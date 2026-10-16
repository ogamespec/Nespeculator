# 04-ppu-palette — Palette RAM, colour generation and the video DAC

**Status:** consensus (round 2)
**Scope:** How the PPU turns a 5-bit pixel colour index into a composite (or RGB)
video level: the 32-entry palette RAM ("Color Generator", CRAM) and its address
decoder, the 6-bit palette entry format, the palette output latch
(Color Buffer) path, greyscale
and colour emphasis, the NTSC/PAL chroma phase generator and the video DAC
levels. The pixel-priority multiplexer that produces the index is in
`04-ppu-rendering.md`; the register bits that control all of this are in
`04-ppu-registers.md`; the dot/frame timing of `BURST` and `/PICTURE` is in
`04-ppu-timing.md`.
**Primary sources:** `BreakingNESWiki/PPU/cram.md`, `pal.md`, `rgb.md`,
`video_out.md`, `tv.md`, `mux.md`, `pads.md`, `umc6538.md`, `visual2c02.md`;
`Docs/PPU/VideoSignal/composite_signal.txt`, `colorburst.txt`, `SMPTE_bars.txt`,
`VDAC.TXT`; HDL `HDL/PPU/cram.v`, `video_out.v`, `dac.v`, `pclk.v`,
`ppu_top.v`; NESdev `PPU_palettes`, `NTSC_video`, `PAL_video`,
`Colour_emphasis`, `PPU_pinout`, `PPU_variants`.

## 1. Palette RAM

The palette is a 32 × 6-bit static RAM inside the PPU: 16 background (BG) and
16 sprite colours in four 4-colour palettes each (`PPU/cram.md`, `PPU/Readme.md`).
Each entry is the final colour; the 5-bit index from the pixel multiplexer selects it.

Palette entry bit layout (the value written to `$3F00–$3F1F` through `$2007`):

```
5  bit  0
---- --
LLCCCC
```

| Bits | Breaking NES Wiki name | Role |
|---|---|---|
| 3–0 | `CC0–CC3` (chroma) | Selects 1 of 12 colour phases, or a grey/black code |
| 5–4 | `LL0–LL1` (luma) | Selects 1 of 4 brightness levels |

The colour index itself is 5 bits, `PAL0–PAL4`, built by the pixel multiplexer
(`PPU/mux.md`, `HDL/PPU/mux.v`):

| Index bit | Source | Meaning |
|---|---|---|
| 0–1 | background or sprite pixel pattern bits | colour 0–3 inside the palette |
| 2–3 | attribute (BG) or sprite attribute bits | palette 0–3 |
| 4 | `CGA4` = palette type | 0 = background palette, 1 = sprite palette |

A pixel colour of 0 is transparent. When both background and sprite pixels are
transparent the multiplexer selects the external `EXT` colour; on a normal
console `EXT0–EXT3` are grounded, so the index becomes 0 and the universal
backdrop colour `$3F00` is shown (`PPU/mux.md`, `PPU/pads.md`).

### 1.1 Address decoder

The 5-bit index is wired to the array as designed columns and rows, not as a
linear address (`PPU/cram.md`):

| Field | Index bits | Function |
|---|---|---|
| Column | `CGA3` (msb), `CGA2` | one of 4 cells per word line (`COL0–COL3`) |
| Row | `CGA4` (msb), `CGA1`, `CGA0` | one of 8 codes on 7 word lines |
| Palette type | `CGA4` | 0 = background, 1 = sprite |

| `{CGA4,CGA1,CGA0}` | Word line |
|---|---|
| 0 | `ROW0_4` |
| 1 | `ROW1` |
| 2 | `ROW2` |
| 3 | `ROW3` |
| 4 | `ROW0_4` (physical row 0) |
| 5 | `ROW5` |
| 6 | `ROW6` |
| 7 | `ROW7` |

Rows 0 and 4 share word line `ROW0_4` (`PPU/cram.md`), so each background
palette's colour-0 entry is the same cell as the corresponding sprite palette's:

| Background address | Sprite mirror | Entry |
|---|---|---|
| `$3F00` | `$3F10` | backdrop / BG palette 0 colour 0 |
| `$3F04` | `$3F14` | BG palette 1 colour 0 |
| `$3F08` | `$3F18` | BG palette 2 colour 0 |
| `$3F0C` | `$3F1C` | BG palette 3 colour 0 |

Palette address ranges (the NESdev `PPU_palettes` layout, produced by this
decoder):

| Address | Contents |
|---|---|
| `$3F00–$3F03` | background palette 0 (colour 0 = backdrop) |
| `$3F04–$3F07` | background palette 1 |
| `$3F08–$3F0B` | background palette 2 |
| `$3F0C–$3F0F` | background palette 3 |
| `$3F10–$3F13` | sprite palette 0 (colour 0 = backdrop) |
| `$3F14–$3F17` | sprite palette 1 |
| `$3F18–$3F1B` | sprite palette 2 |
| `$3F1C–$3F1F` | sprite palette 3 |

Only the low 5 address bits reach the array, so `$3F20–$3FFF` mirror
`$3F00–$3F1F` every 32 bytes (`HDL/PPU/cram.v`, `CRAM_Decoder`).

## 2. The colour output path

The current palette entry is held in a 6-bit **palette output latch**
(Color Buffer, CB) and driven to the video generator as two active-low groups
(`PPU/cram.md`, `HDL/PPU/cram.v`):

| Signal | Value | Meaning |
|---|---|---|
| `/CC0–/CC3` | `~cram[index][3:0]` | chroma of the current pixel |
| `/LL0–/LL1` | `~cram[index][5:4]` | luma of the current pixel |

The latch also serves the CPU path: `#DB/CB` loads it on a `$2007` palette write
and `#CB/DB` drives it back on a palette read (`PPU/cram.md`). Its outputs pass
through unequal D-latch chains, so the video generator emits pixels "from the
past" (`PPU/cram.md`); the HDL delays only the chroma path (two latches in
`ChromaDecoder`) and feeds `n_LL` straight to `LumaDecoder` (`video_out.v`), so
the common "one dot late" figure is a **model choice**, not a sourced number.

Two control signals modify the output before it leaves this block:

- **Greyscale** (`$2001[0]`, `B/W`). The chroma field is forced to 0 while luma
  passes unchanged — the NESdev "AND with `$30`" effect. In the HDL `/CC` is
  `%1111` (chroma 0 = grey) whenever the colour path is inactive, i.e. outside
  the visible picture or with `$2001[0]` set; `/LL0–/LL1` are never gated
  (`HDL/PPU/cram.v`).
- **Emphasis** (`$2001[5:7]`, `/TR`, `/TG`, `/TB`). See §4.

The palette read path bypasses the `$2007` read buffer: a read of
`$3F00–$3FFF` returns the palette entry from the palette output latch immediately while
the shadowed VRAM byte is what lands in the read buffer (`PPU/cram.md`,
`04-ppu-registers.md` §9). On the 2C02E and earlier NTSC parts (and on the RGB
2C04) palette RAM is **not readable** and the range behaves like ordinary VRAM
(`Docs/NES_Famicom_Board_Revisions.md` §4; NESdev `Cycle_reference_chart`: "Early
revisions cannot read back sprite or palette memory").

### 2.1 Direct colour

When the CPU addresses `$3Fxx` with rendering disabled, the VRAM controller
signals `TH/MUX`, which puts the low 5 bits of the current PPU address (`TH`
counter `THO0–THO4`) onto the palette index instead of a pixel colour
(`PPU/mux.md`, `PPU/vram_ctrl.md`). Merely setting `$2006` to `$3Fxx` is enough
to make that index appear, because only the `TH` counter is used in this mode.
This is the "direct colour" / full-palette trick (`PPU/mux.md`).

## 3. NTSC colour generation

The video phase generator is the only part of the PPU clocked by the master
clock `CLK`; everything else runs on the dot clock `PCLK` = `CLK`/4 (NTSC) or
`CLK`/5 (PAL/Dendy). `CLK`/6 is the colour subcarrier (NTSC: 21.477272/6 =
3.579545 MHz; PAL: 26.601712/6 = 4.433618 MHz), so one subcarrier period is
6 `CLK`, and the phase shifter's D-latch chain advances 30° per half-`CLK`
(`PPU/video_out.md`, `PPU/pclk.md`; subcarrier frequency from
`Docs/PPU/VideoSignal/composite_signal.txt`).

The chroma decoder picks one of 12 phases from the current chroma code. All
phases are measured relative to the colour burst, which is generated with the
phase of chroma code 8 (`PPU/video_out.md`):

| Chroma code (CC) | Generated colour | Phase vs. burst | Note |
|---|---|---|---|
| `$0` | grey shade of `LL` | — | phase row not connected |
| `$1` | hue 1 | 150° | |
| `$2` | hue 2 | 180° | |
| `$3` | hue 3 | 210° | |
| `$4` | hue 4 | 240° | |
| `$5` | hue 5 | 270° | |
| `$6` | hue 6 | 300° | |
| `$7` | hue 7 | 330° | |
| `$8` | hue 8 | 0° | same phase as the colour burst |
| `$9` | hue 9 | 30° | |
| `$A` | hue 10 | 60° | |
| `$B` | hue 11 | 90° | |
| `$C` | hue 12 | 120° | |
| `$D` | grey shade of `LL` | — | no phase row; luma still used (RGB parts: black, §6) |
| `$E`, `$F` | black | — | forced by `PBLACK` regardless of `LL` |

The phases are monotonic in the chroma code; the decoder's topological output
numbering is not (`PPU/video_out.md`). `PBLACK` is asserted for chroma codes
`$E` and `$F` and overrides the luma path, so those two entries always render
black (`HDL/PPU/video_out.v`, `ChromaDecoder`).

The luma decoder selects one of 4 levels from `LL0–LL1`; outside the picture it
is forced to 0 (`HDL/PPU/video_out.v`, `LumaDecoder`). The composite DAC is a
current-summing ladder whose level `L0–L9` is the highest phase-swing bit
asserted (`HDL/PPU/dac.v`). Unloaded levels at the `VOUT` pin (`PPU/video_out.md`):

| Level `L` | V (unloaded) | Description |
|---|---|---|
| 0 | 0.781 | sync |
| 1 | 1.000 | colour burst L |
| 2 | 1.131 | colour level 0D |
| 3 | 1.300 | colour 1D (black / blank) |
| 4 | 1.712 | colour burst H |
| 5 | 1.743 | colour level 2D |
| 6 | 1.875 | colour level 00 |
| 7 | 2.287 | colour level 10 |
| 8 | 2.331 | colour level 3D |
| 9 | 2.743 | colour levels 20 / 30 |

Each hued colour is a two-level square wave at the subcarrier rate alternating
between its luma row's two levels; the average sets brightness and the
difference sets saturation. There is no separate black level: blank and black
are level `L3` (`Docs/PPU/VideoSignal/VDAC.TXT`, `PPU/video_out.md`).

Measured levels (volts, source cells are `a / b`; `H`/`L` are the high and low
excursions of the colour waveform, `Docs/PPU/VideoSignal/VDAC.TXT`):

| Part (region) | Row | SYNC | BURST | LUMA0 | LUMA1 | LUMA2 | LUMA3 |
|---|---|---|---|---|---|---|---|
| RP2C02G (NTSC) | H | 0.525 | 0.841 | 1.091 / 0.825 | 1.500 / 1.133 | 1.941 / 1.466 | 1.941 / 1.466 |
| | L | 0.000 | 0.300 | 0.366 / 0.266 | 0.525 / 0.391 | 0.966 / 0.733 | 1.558 / 1.166 |
| RP2C07 (PAL) | H | 0.566 | 0.900 | 1.175 / 0.900 | 1.566 / 1.208 | 2.033 / 1.566 | 2.033 / 1.566 |
| | L | 0.000 | 0.300 | 0.391 / 0.300 | 0.566 / 0.433 | 1.041 / 0.791 | 1.633 / 1.266 |
| UMC UA6538 (Dendy) | H | 0.458 | 0.700 | 0.916 / 0.700 | 1.400 / 1.066 | 1.916 / 1.475 | 1.916 / 1.475 |
| | L | 0.000 | 0.300 | 0.341 / 0.258 | 0.466 / 0.350 | 0.816 / 0.625 | 1.441 / 1.100 |

## 4. Emphasis (`$2001[5:7]`)

Emphasis attenuates the picture when the pixel's phase drives a colour-difference
channel and the matching `$2001` bit is set (`PPU/video_out.md`, `video_out.v`):

| `$2001` bit | Signal | Channel (NTSC) | Channel (PAL/Dendy) |
|---|---|---|---|
| 5 | `/TR` | red | **green** |
| 6 | `/TG` | green | **red** |
| 7 | `/TB` | blue | blue |

`TINT` is applied only inside the visible picture (`/POUT`), never to sync or
burst. When active, the DAC output is multiplied by ≈0.746
(`PPU/video_out.md`). The effect is a dimmer version of the *whole* pixel, not
a colour rotation, which is why emphasised backgrounds darken as a whole.

Region and revision differences:

- **NTSC (RP2C02):** red/green connections as above.
- **PAL (RP2C07) and Dendy (UA6538):** the `/PR` and `/PG` connections are
  crossed, so `$2001` bit 5 emphasises green and bit 6 emphasises red
  (`PPU/pal.md`, `PPU/umc6538.md`; `Docs/NES_Famicom_Board_Revisions.md` §4).
- **RGB parts (2C03/2C04/2C05):** emphasis does not attenuate; it drives the
  selected RGB channel fully on (`PPU/rgb.md`, §6).

## 5. PAL colour generation (RP2C07)

The 2C07 uses the same palette RAM and the same 12 phases, but alternates the
chroma phase on every scanline (the PAL delay-line colour system expects the V
component to invert each line). Differences from NTSC
(`PPU/pal.md`, `HDL/PPU/video_out.v`):

| Item | RP2C02 (NTSC) | RP2C07 (PAL) |
|---|---|---|
| Chroma decoder | 13 rows | 25 rows (two banks) |
| Bank selection | — | V counter bit 0 (`V0`), one bank per scanline |
| Phase alternation | none | phase inverted on alternate lines |
| Colour burst | 0° every line, reference code `$8` | reference code `$7`: −15° hue shift, phase inverted on alternate lines |
| `/PICTURE` | direct | extra D-latch delay before the video generator |
| Emphasis red/green | normal | swapped (§4) |
| H decoder windows | FPorch 279, BPorch 270/328, HBlank 279/304, BURST 308/323 | FPorch 277, BPorch 256/4, HBlank 277/302, BURST 306/321 (`04-ppu-timing.md` §4) |

`PPU/pal.md` contains the full 25-row mask table for the doubled decoder, and
notes that the NTSC "crawl" Even/Odd circuit is absent on 2C07 (it is reused for
an OAM-address workaround, see `04-ppu-sprites.md`). NESdev `PPU_palettes` §2C07
puts the PAL burst reference at `$x7` rather than the 2C02's `$x8`, a −15° hue
shift (rank 4; the mask table maps no rows to colours).

## 6. RGB PPUs (RP2C03 / RP2C04 / RC2C05)

The RGB parts (RP2C03, RP2C04, RC2C05) replace the composite output with
component video. Only the 2C04-0003 die was photographed, so the pinout below is
that part's; the other revisions are assumed similar (`PPU/rgb.md`):

| Pin (2C02G) | Pin (2C04-0003) |
|---|---|
| `EXT0` (14) | `RED` |
| `EXT1` (15) | `GREEN` |
| `EXT2` (16) | `BLUE` |
| `EXT3` (17) | `AGND` |
| `VOUT` (21) | `/SYNC` |

NESdev `PPU_pinout` calls those pins, at the same numbers, `R` (14), `G` (15),
`B` (16), `GND` (17) and `CSYNC` (21): pins 14–16 and 21 become video outputs
and pin 17 ground, replacing the `EXT` bus and composite `VOUT`.

Other differences (`PPU/rgb.md`):

- The composite phase generator is replaced by per-channel 16 × 12 decoders and
  three current-summing DACs; the colour-burst logic still exists but its output
  is unused.
- Emphasis drives the corresponding channel fully on instead of attenuating.
- There is no OAM read (`/R4`) and no palette read; the palette output latch
  and secondary OAM are simplified.
- Hue `$D` renders as **black**, not as the 2C02's grey of the same luma, and
  the 2C04 palette is scrambled as a copy-protection permutation (NESdev
  `PPU_palettes` §2C03/2C04).
- The Even/Odd circuit is disabled (its input is tied to ground).

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | Writing `$3F10` also changes the backdrop `$3F00` | Rows 0 and 4 share one CRAM word line | Model `$3F00/$3F10`, `$3F04/$3F14`, `$3F08/$3F18`, `$3F0C/$3F1C` as aliases |
| 2 | `$3F20–$3FFF` behave like `$3F00–$3F1F` | Only `CGA0–CGA4` reach the palette decoder | Mask the palette address to 5 bits |
| 3 | A palette write during rendering appears on a later pixel | The palette output latch passes through unequal D-latch delay chains; the exact lag is a model choice | Apply the new colour on the next dot, not the current one |
| 4 | Greyscale leaves brightness unchanged but removes all hue | `$2001[0]` clears `CC0–CC3`, keeping `LL0–LL1` | Force the chroma nibble to 0, not the whole entry |
| 5 | Chroma `$0E`/`$0F` are black at any luma | `PBLACK` overrides the luma path for codes `$E`/`$F` | Hard-code those entries to the black level |
| 6 | Chroma `$0D` is a grey, not black | No decoder row matches `$D`; only the phase path is absent | Render `$0D` as the `$00` grey at its own luma (2C02; RGB parts: black, §6) |
| 7 | Emphasised pixels look darker but keep their hue | `TINT` scales the whole composite level by ≈0.746 | Multiply the output level; do not rotate the hue |
| 8 | A game written for NTSC shows red and green emphasis swapped on PAL/Dendy | `/PR` and `/PG` are crossed on the 2C07/6538 | Swap the `$2001` bits 5/6 mapping when the region is PAL/Dendy |
| 9 | Palettes look different between NTSC, PAL and Dendy even for the same values | The DAC ladder differs per part (VDAC measurements) | Select a per-part voltage/palette table |
| 10 | An arcade RGB board shows solid saturated colours where a consumer PPU would dim | RGB PPUs drive the emphasised channel fully on | Model RGB emphasis as full-on, not attenuation |
| 11 | Palette reads return the colour with no priming read while VRAM reads need one | Palette data bypasses the `$2007` read buffer | Return the palette entry immediately on `$3Fxx` reads |
| 12 | On a 2C02E or a 2C04, `$3F00` reads return stale VRAM instead of a colour | Palette RAM is not readable on those revisions | Gate palette reads on the revision |
| 13 | With rendering off, writing `$2006 = $3Fxx` paints the screen with that palette entry | `TH/MUX` routes the `TH` counter onto the palette index ("direct colour") | Implement the direct-colour path when rendering is disabled |
| 14 | Composite colours drift in phase against the pixel grid | The subcarrier is `CLK`/6, so one dot is 240° of it on NTSC (dot = 4 `CLK`) and 300° on PAL/Dendy (dot = 5 `CLK`) | Generate the subcarrier from the master clock, not per pixel |

## Programming / implementation notes

- MUST implement palette RAM as 32 entries of 6 bits and mask the PPU palette
  address to 5 bits (`$3F00–$3FFF` all alias into `$3F00–$3F1F`).
- MUST alias `$3F00/$3F10`, `$3F04/$3F14`, `$3F08/$3F18`, `$3F0C/$3F1C`.
- MUST treat a palette entry as 6 bits: bits 5–4 luma, bits 3–0 chroma.
- MUST render chroma `$0`, `$D` as greys of the entry's luma, `$E`/`$F` as
  black, and `$1–$C` as the 12 phases of §3.
- MUST generate the 12 phases from the master clock (`CLK/6` subcarrier) so that
  the colour burst phase is code 8's phase and the phase drifts against the
  pixel grid.
- MUST apply emphasis by scaling the composite level (NTSC: ≈0.746) and
  MUST swap the red/green emphasis bits on PAL/Dendy.
- MUST apply greyscale by clearing only the chroma nibble.
- SHOULD drive the universal backdrop from palette index 0 (`$3F00`) when both
  background and sprite pixels are transparent.
- SHOULD model a one-dot palette output latch delay (a model choice, §2) and the
  direct-colour (`TH/MUX`) path for `$3Fxx` accesses with rendering disabled.
- MAY use the canonical NES 64-colour table (NESdev `PPU_palettes`); the 6-bit
  value is a colour index, not RGB.
- RGB parts: MUST output RGB levels with emphasis as full-on and MUST NOT
  provide palette or OAM reads.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Palette RAM layout, mirrors, backdrop, entry format | `PPU_palettes` |
| Backdrop/priority rules, transparent colour 0 | `PPU_rendering` |
| `$3Fxx` accesses: direct colour, palette reads and the `$2007` bypass | `PPU_registers` |
| NTSC colour generation, phases, 12 hues | `NTSC_video` |
| PAL colour generation and phase alternation | `PAL_video` |
| Colour emphasis bits and levels | `Colour_emphasis` |
| RGB PPU pin changes and behaviour | `PPU_pinout`, `PPU_variants` |
| Palette readability per revision | `PPU_variants` (standalone page, not a `PPU` section; verified against the 2024-12-25 Wayback snapshot) |
| Palette contents at power-on (unspecified) | `PPU_power_up_state` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Palette size, 16 BG + 16 sprite, 6-bit entries | `PPU/cram.md`, `PPU/Readme.md` | `cram.v` |
| Row/column decoder, `ROW0_4` aliasing | `PPU/cram.md` | `cram.v` (`CRAM_Decoder`) |
| Palette output latch, CPU read/write paths, output delay | `PPU/cram.md` | `cram.v` (`CB_Control`) |
| Greyscale chroma clear | `PPU/cram.md`, `PPU/regs.md` | `cram.v` (`n_CC`) |
| Direct colour (`TH/MUX`) | `PPU/mux.md`, `PPU/vram_ctrl.md` | `mux.v`, `vram_ctrl.v` |
| Chroma phases, burst phase, decoder rows | `PPU/video_out.md` | `video_out.v` (`ChromaDecoder`, `PhaseShifter`) |
| Luma levels, phase swing, DAC levels | `PPU/video_out.md` | `video_out.v`, `dac.v` |
| Emphasis, `TINT`, 0.746 scaling | `PPU/video_out.md`, `PPU/regs.md` | `video_out.v` (`Emphasis`) |
| PAL doubled decoder, V0 alternation, R/G swap | `PPU/pal.md` | decoder/V0 in `video_out.v` (`RP2C07`); R/G swap **not modelled** |
| PAL/Dendy H/V differences | `PPU/pal.md`, `PPU/umc6538.md` | `hv_decoder.v`, `fsm.v` |
| RGB PPU pins, decoders, emphasis | `PPU/rgb.md` | — (not implemented) |
| Measured DAC levels | — (`Docs/PPU/VideoSignal/VDAC.TXT`) | `dac.v` |
| Composite signal structure, burst | `PPU/tv.md` | — (`Docs/PPU/VideoSignal/`) |
| Backdrop universality, palette-read bypass | `PPU/mux.md`, `PPU/cram.md` | `mux.v`, `vram_ctrl.v` |

## Open questions

- **Greyscale vs. colour burst.** `PPU/cram.md`'s signal table describes `B/W`
  as "disable Colour Burst"; the HDL clears only the chroma bits and leaves
  `BURST` running. Which is silicon? A TV without burst switches to monochrome
  by itself, so the distinction is observable (`cram.v` vs `cram.md`).
- **Phase table direction.** The phase-per-colour table in §3 is derived from
  the `PPU/video_out.md` decoder-table permutation; the wiki itself never lists
  phases per colour code. Confirm against a hardware phase measurement or the
  Visual 2C02 waveform before treating the absolute angles as final.
- **Emphasis level per revision.** `PPU/video_out.md` gives 0.746 for the 2C02;
  `VDAC.TXT` shows slightly different ratios for 2C07/6538/6528. A per-part
  emphasis factor is not stated by any source.
- **DAC level labels.** `PPU/video_out.md` labels the levels with strings such
  as `0D`, `1D`, `2D`, `3D`, `00`, `10` and `20 / 30` but never defines how
  those map onto the 64 palette values, so the exact level-per-entry mapping is
  incomplete in ranks 1–2. Emulators should use the canonical NES palette
  (NESdev `PPU_palettes`) plus the measured levels of §3.
- **Palette contents at power-on.** No Breaking NES Wiki page states the initial
  CRAM contents; the HDL initialises all 32 entries to 0. NESdev treats them as
  unspecified.
- **RGB PPU completeness.** `PPU/rgb.md` is reconstructed from top-layer die
  photographs only and states that parts are guessed; the RGB decoders' masks
  are given but not the resulting colours.
- **Direct colour during rendering.** The Wiki requires rendering off; the HDL
  gates `TH/MUX` on palette range and rendering-off, but alignment is untested.
- **Length.** This file is 400 lines, at the 120–400-line target of
  `Conventions.md` §6; the address/alias, phase and DAC tables fill it, and the
  brief assigns all three colour generators to one file.
