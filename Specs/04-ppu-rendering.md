# 04-ppu-rendering — Background pipeline, pixel priority and rendering control

**Status:** consensus (round 3)
**Scope:** The per-dot rendering schedule of the PPU (which memory access happens
on which dot of a scanline), the background "Data Reader" pipeline (tile
counters, pattern fetch, pattern shift registers, fine-X pixel selection), the
pixel priority multiplexer that combines background and sprite pixels, the
sprite 0 hit latch, and the rendering enable/forced-blank rules. OAM layout,
sprite evaluation and the sprite pipeline itself are in `04-ppu-sprites.md`;
the frame-level scanline structure and vblank are in `04-ppu-timing.md`;
mutation of the `v`/`t` scroll registers by the fetch schedule is in
`04-ppu-scrolling.md`; palette lookup and colour output are in
`04-ppu-palette.md`.
**Primary sources:** `BreakingNESWiki/PPU/dataread.md`, `bgcol.md`, `mux.md`,
`fsm.md`, `tilecnt.md`, `par.md`, `pamux.md`, `hv_decoder.md`, `hv.md`,
`regs.md`, `fifo.md`, `obj_eval.md`;
HDL `HDL/PPU/fsm.v`, `bgcol.v`, `tilecnt.v`, `mux.v`, `hv.v`,
`hv_decoder.v`, `par.v`, `pamux.v`, `fifo.v`, `obj_eval.v`, `ppu_top.v`;
NESdev `PPU_rendering`, `PPU_scrolling`, `PPU_sprite_priority`,
`PPU_pattern_tables` (via the Wayback Machine).

## 1. Scanline structure and dot numbering

Every scanline is 341 dots on NTSC and PAL alike. The PPU's H counter (`H0–H8`)
counts 0–340 inclusive and `HPLA_23` clears it at H = 340, so **dot = H**
(`PPU/hv.md`, `HDL/PPU/hv.v`, `HDL/PPU/hv_decoder.v`). Dot 340 is the last dot
of the line; dot 0 is the idle dot that follows the clear.

**Dot numbering.** This spec uses the NESdev convention, dot = `H` = 0–340.
Normative labels: the vertical (fine-Y) increment and the last coarse-X increment
of the visible line at dot 256, the horizontal `t`→`v` copy at dot 257 (its effect
visible at 258), dummy fetches 337–340. The HDL `PCLK`-edge measurements in
`04-ppu-scrolling.md` §4 observe the same events one dot later (257/258).

The H decoder (`PPU/hv_decoder.md`, `HDL/PPU/hv_decoder.v` `HDecoder`) emits
the strobes `HPLA_0–23` at the H values below. `HPosLogic` (`PPU/fsm.md`,
`HDL/PPU/fsm.v`) passes each strobe through one or two `dlatch` stages before it
becomes a control signal, and the consuming logic samples it at its own phase,
so the observable dot window is the strobe's H value or one dot later — never
earlier. The last column applies that +1 dot to every multi-latch signal except
`/VIS` (`S/EV`, `E/EV`, `0/HPOS`, `/EVAL`, `/FO`, `I/OAM2`, `OBJ_READ`, `#F/NT`,
`F/TA`, `F/TB`, `CLIP_O`/`CLIP_B`); `F/AT` alone is single-stage, and `/VIS` keeps
the NESdev anchor 0–255 (a sprite at X = 0 must show a pixel on dot 0). NESdev's
evaluation window is cycles 65–256; the latched `S/EV` edge is dot 66 and `E/EV`
dot 256. Emulators MUST anchor on the last column, not the raw `HPLA` values;
the per-signal stage is in *Open questions*.

| HPLA | HPLA strobe (H = dot) | Signal produced | Dot window used in §2 |
|---|---|---|---|
| 2 | 65 | `S/EV` start of sprite evaluation | 66 (`S/EV` edge; window 65–256) |
| 3 | 0–7, 256–263 | `CLIP_O`/`CLIP_B` left-column clip windows | 1–8, 257–264 |
| 5 | 339 | `0/HPOS` clear/arm the sprite lane counters | 340 |
| 6, 7 | 63, 255 | `/EVAL` set at 63, `E/EV` end at 255 | 64 … 256 |
| 8 | 0–63 | `I/OAM2` secondary-OAM clear window | 1–64 |
| 9 | 256–319 | `OBJ_READ` sprite fetch window | 257–320 |
| 10 | 0–255 | `/VIS` visible region (deasserted on the pre-render line) | 0–255 — see *Open questions* |
| 11 | H ≡ 0,1 (mod 8) | `#F/NT` nametable fetch | dots ≡ 1,2 (mod 8) |
| 12 | H ≡ 6,7 (mod 8) | `F/TB` pattern-high fetch | dots ≡ 7,0 (mod 8) |
| 13 | H ≡ 4,5 (mod 8) | `F/TA` pattern-low fetch | dots ≡ 5,6 (mod 8) |
| 14, 15 | 320–335, 0–255 | `/FO` pattern fetch output enable; `F/AT` gating window | 321–336, 1–256 |
| 16 | H ≡ 2,3 (mod 8) | `F/AT` attribute fetch (single-latch, gated off outside `/FO`) | dots ≡ 3,4 (mod 8) — see *Open questions* |
| 23 | 340 | H counter clear, V counter step | 340 |

(NTSC: `PPU/hv_decoder.md`, `HDL/PPU/hv_decoder.v`. The PAL `RP2C07` decoder differs
only at `HPLA_0` and `HPLA_17–22` (porch/blank/burst), so §1–§2 hold for PAL and Dendy too.)

## 2. Per-dot fetch schedule

While rendering is enabled, the PPU drives the VRAM address/data pins on almost
every dot. One background tile needs four 2-dot accesses — nametable (NT),
attribute (AT), pattern low (TA), pattern high (TB) — and the eight dots of a
tile period are filled by them in that order. A sprite fetch reuses the same
circuitry with the NT/AT accesses replaced by "garbage" nametable fetches.

Dot numbers below are the NESdev numbers of §1, i.e. they equal the H counter
value. Visible scanlines 0–239 (same pipeline on NTSC, PAL and Dendy; the
region only changes which lines exist):

| Dot (= H) | Phase | Bytes fetched |
|---|---|---|
| 0 | idle | the PPU address bus carries the pattern-low address that dot 5 fetches (NESdev `PPU_rendering`, "Cycle 0") |
| 1–2 | NT | nametable byte, tile T |
| 3–4 | AT | attribute byte, tile T |
| 5–6 | TA | pattern low, tile T |
| 7–8 | TB | pattern high, tile T |
| 9–16 | NT/AT/TA/TB | tile T+1 |
| … | … | dots 1–256 cover 32 tile periods (tiles T … T+31; T = first displayed tile) |
| 256 | — | coarse X increments (last tile); fine Y increments (coarse Y only on a 7→0 wrap) |
| 257–264 | sprite slot 0 | garbage NT, garbage NT, pattern low, pattern high |
| 265–272 | sprite slot 1 | sprite #1 of the next line |
| … | … | 8 sprite slots, 8 dots each (dots 257–320) |
| 321–336 | two-tile prefetch | NT/AT/TA/TB for the first two tiles of the *next* line |
| 337–340 | dummy | two nametable fetches of the same tile |

Facts this table encodes:

- Dots 1–256 hold **32 tile periods**. The data for the first two tiles of the line was already loaded during dots 321–336 of the previous line, so the first tile *fetched* on the line is the third one (NESdev `PPU_rendering`).
- Every byte occupies a 2-dot access: the first dot places the full address on the bus and the board latches the low byte (`ALE`), the second performs the read (NESdev `PPU_rendering`; §3, `PPU/pamux.md`).
- Dots 257–320 are **idle with respect to the background**: they are the sprite fetch window (64 dots = 8 sprites × 8 dots). `OBJ_READ` is the fetch event and is active for exactly these dots (`HPLA_9` strobes at H 256–319 and the FSM latch chain shifts it one dot — §1).
- Dots 337–340 are two more nametable fetches of the tile that dots 1–2 of the next line fetch. The Breaking NES Wiki does not state their purpose; NESdev notes that MMC5 uses the resulting string of three consecutive nametable fetches (dots 337–340 and 1–2 of the next line) to clock a scanline counter.
- Dot 256 is not idle: coarse X wraps and fine Y is incremented (coarse Y advances only when fine Y wraps 7→0). The register-level detail is in `04-ppu-scrolling.md`.
- Dots 0–255 are the `/VIS` window (`HPLA_10` strobes at H 0–255; the latch alignment is in *Open questions*). Outside it the sprite logic is disabled (`n_VIS`); the background pipeline keeps shifting but the pixels are not visible (§6).

Pre-render scanline (NTSC 261, PAL 311):

- The background fetch schedule of §2 runs unchanged, so the shift registers are pre-filled with the first two tiles of the next line.
- The sprite fetch schedule of dots 257–320 also runs, but with whatever secondary OAM content was left over from scanline 239 of the previous frame (sprite evaluation does not run on the pre-render line; NESdev `PPU_sprite_evaluation`).
- The vertical copy of `t` into `v` happens during dots 280–304 (`04-ppu-scrolling.md`).
- NTSC only: on odd frames one dot is dropped at the end of this scanline (the line is 340 dots). PAL and Dendy do not skip a dot. See §7.

## 3. Background data path ("Data Reader")

The Data Reader is not a separate HDL module: the wiki splits it into PAR, scroll registers, tile counters, PAMUX and BGCOL (`PPU/dataread.md`). Three blocks generate an address, in this order:

1. **Tile counters** hold the coarse position — the `v` register's coarse fields: coarse X `THO0–4` (5 bits, max 31), coarse Y `TVO0–4` (5 bits, max 29), nametable `NTHO`/`NTVO` (1 bit each, max 1) and fine Y `FVO0–2` (3 bits, max 7) (`PPU/tilecnt.md` counter table, `HDL/PPU/tilecnt.v`).
2. **PAMUX** selects the address source for the current dot — `NT_ADR`, `AT_ADR` or `PAT_ADR` — and drives the 14-bit `/PA0–13` latches (`PPU/pamux.md`, `HDL/PPU/pamux.v`).
3. **PAR** holds the pattern (CHR) address `PAT_ADR0–13` (`PPU/par.md`).

The address fields as fetched:

| Access | Address bits (PPU address space, bit 13 down to bit 0) |
|---|---|
| Nametable (NT) | `%10NN YYYYY XXXXX` — `$2000` + nametable `[11:10]` + coarse Y `[9:5]` + coarse X `[4:0]` (`04-ppu-memory-map.md` §5) |
| Attribute (AT) | `%10NN 1111 yyy xxx` — `$23C0` + nametable `[11:10]`, bits `[9:6]` = `1111`, `yyy` = coarse Y `>> 2` at `[5:3]`, `xxx` = coarse X `>> 2` at `[2:0]` (`04-ppu-memory-map.md` §4) |
| Pattern low (TA) in the fetch | `PAT_ADR` with address bit 3 = 0 (plane A) |
| Pattern high (TB) in the fetch | `PAT_ADR` with address bit 3 = 1 (plane B, +8) |

The pattern address is assembled from two sources, selected by `OBJ_READ`
(`PPU/par.md` "Table of bits usage in addressing", `HDL/PPU/par.v`; `04-ppu-memory-map.md` §5):

| `PAT_ADR` bit | Background source | Sprite source (8×8) | Sprite source (8×16) |
|---|---|---|---|
| 0–2 | fine Y `FVO0–2` | sprite row offset `OV0–2` | `OV0–2` |
| 3 | `/H1'` (A/B plane) | `/H1'` | `/H1'` |
| 4 | name-table byte (tile index) bit 0 | secondary-OAM tile byte bit 0 | `OV3` |
| 5–11 | name-table byte (tile index) bits 1–7 | tile byte bits 1–7 | tile byte bits 1–7 |
| 12 | PPUCTRL `BGSEL` bit 4 | PPUCTRL `OBSEL` bit 3 | tile byte bit 0 |

Two consequences a programmer sees: the background pattern table comes from PPUCTRL (`$2000`) bit 4 and is sampled when the pattern address is loaded, so a mid-scanline `$2000` write can split the pattern table across the line; and sprite V flip is in the address generator (`OV` is the row offset, `PPU/dataread.md` "getting sprite patterns and their V inversion") while sprite H flip is in the Object FIFO (`PPU/fifo.md` "H. Inversion", `04-ppu-sprites.md`).

## 4. Background shift registers and fine X

Two 8-bit shift registers hold the pattern planes of the current tile
(`BGC_0`, `BGC_1`) and two 1-bit latches hold the attribute bits 0 and 1
(`BGC_2`, `BGC_3`) (`PPU/bgcol.md`, `HDL/PPU/bgcol.v`).

| Event | When | Effect |
|---|---|---|
| `PD_SR` (plane A staging) | `F_TA=1` and `H0_DD` and `/PCLK` | pattern-low byte captured into a staging latch |
| `SRLOAD` | `F_TB=1` and `H0_DD` and `/PCLK` | both planes load into the shift registers in **reversed** bit order |
| `PD_SEL` | one-dot-delayed `F_AT=1` and `H0_DD` and `/PCLK` | attribute byte captured; its four 2-bit quadrant pairs stored |
| `STEP` + `NEXTS` | once per dot (`STEP2`/`NEXTS` derived from `/FO`) | register rotates one pixel |
| quadrant select | constant per 16-pixel quadrant | `{TVO1, H01}` picks a captured attribute pair; `H01` toggles every two tiles |

Rules that follow from this structure:

- **Loading is bit-reversed.** NES pattern bit 7 is the leftmost pixel and is the first shifted out (`PPU/bgcol.md`: "The value is fed to SR8 in reversed form, because the pixels are output backwards"; `HDL/PPU/bgcol.v` `BGC_SR8`).
- **Fine X selects the pixel.** The 3-bit fine X register `FH` (`x`) is the tap index into the 8-bit pattern registers: the pixel on the current dot is `disp[FH]`. With the reversed load this reproduces the NESdev rule "the first pixel of the first tile is pattern bit (7 − fine X)".
- **Attribute bits are constant for a 16-pixel (two-tile) quadrant**, selected by `{TVO1, H01}` and changed on the same dot as the pattern reload, so colour bits 3:2 and pattern bits 1:0 never skew by a pixel (`HDL/PPU/bgcol.v` `BGC_2`).
- Writes to fine X during rendering take effect on the next dot; writes to the vertical scroll take effect on the next frame (`04-ppu-scrolling.md`, NESdev `PPU_scrolling`).
- The registers are dynamic storage; NESdev notes they decay towards 0 in a few frames and that enabling rendering mid-frame can show the decay (see *Open questions*).

The background pixel presented to the multiplexer is the 4-bit value
`BGC3 BGC2 BGC1 BGC0` — two pattern bits and two palette bits — gated by the
background left-column clip `CLIP_B` (§6). Colour 0 (`BGC1:BGC0 = %00`) is
transparent.

## 5. Pixel priority multiplexer

`PictureMUX` produces the 5-bit palette index `PAL0–4` (HDL `CGA_out`) and the
4-bit `EXT` output (`PPU/mux.md`, `HDL/PPU/mux.v` `PictureMUX`,
`MUX_Control`). Inputs: background colour `BGC0–3`, sprite colour
`/ZCOL0`, `/ZCOL1`, `ZCOL2`, `ZCOL3`, sprite priority `/ZPRIO`, the external
`EXT0–3` pins, and the direct palette index from the tile-H counter (`THO0–4`,
`TH/MUX`).

The selection is a three-stage cascade:

| Stage | Selects | Control |
|---|---|---|
| 1 | background vs sprite colour | background transparent (`BGC1:BGC0 = %00`), sprite transparent (`ZCOL1:ZCOL0 = %00`), sprite priority behind background (`/ZPRIO` = 1) |
| 2 | previous result vs `EXT0–3` | when neither background nor sprite pixel is opaque |
| 3 | previous result vs `THO0–4` | `TH/MUX` = 1 (CPU access to `$3F00–$3FFF`) |

The stage-1 rule reproduces the standard NESdev priority table exactly
(NESdev `PPU_rendering` "Priority multiplexer decision table"); the background
left-column clip is applied earlier, in `BGCOL` (`n_CLPB`), not by `MUX_Control`
(§6, `HDL/PPU/mux.v`):

| Background pixel | Sprite pixel | Sprite priority | Output |
|---|---|---|---|
| transparent | transparent | x | `EXT` in (on a stock console: `$3F00` backdrop) |
| transparent | 1–3 | x | sprite |
| 1–3 | transparent | x | background |
| 1–3 | 1–3 | in front (attr bit 5 = 0) | sprite |
| 1–3 | 1–3 | behind (attr bit 5 = 1) | background |

- The sprite side already resolves sprite-vs-sprite priority **before** this
  mux: only the winning sprite pixel reaches it, and the winner is the
  lowest-numbered OAM sprite with an opaque pixel on this dot — even when its
  own priority bit says "behind background" (`04-ppu-sprites.md` §5.3, NESdev `PPU_sprite_priority`).
- Bit 4 of the index is the background/sprite selector (`0` background or
  backdrop, `1` sprite), so the final index is `$3F00` + bit4·16 + palette·4 +
  colour (`04-ppu-palette.md`).
- The **colour buffer** (`PPU/mux.md`, `PPU/dataread.md`) latches the index, so
  a change of `BGC`, sprite output or `$2001`/`$2000` bit at dot N appears at
  dot N+1 at the earliest; sampling combo values without the latch yields a
  one-pixel-early swap on raster splits.
- When `TH/MUX` is active the index comes straight from the tile-H counter.
  With rendering disabled this is "direct colour": pointing `$2006` into
  `$3F00–$3FFF` displays that entry with no VRAM access (`PPU/mux.md`,
  `04-ppu-registers.md` §9).
- The `EXT0–3` path is a further colour source (stage 2, when neither pixel is
  opaque); on every known console the pins are grounded, so it reads as black.
  Using it to erase the screen needs slave mode and is marked unconfirmed by the wiki (`PPU/mux.md` "Exploiting EXT Pins").

## 6. Sprite 0 hit and clipping

Sprite 0 hit is latched in `STRIKE_FF`, exported as PPUSTATUS bit 6
(`$2002[6]`), cleared only by `RESCL` (the vblank-end event) and never by a
`$2002` read (`PPU/mux.md` §"Sprite 0 Hit", `HDL/PPU/mux.v` `Spr0Hit`).

```
hit at dot N  ⇔  PCLK phase
               ∧ either background pattern bit is 1  (BGC1:BGC0 ≠ %00)
               ∧ /SPR0HIT   (sprite 0 lane emits an opaque pixel)
               ∧ /SPR0_EV   (sprite 0 was found on this scanline)
               ∧ /VIS       (dot is inside the visible region)
```

Practical form: the flag sets on the first visible dot where an **opaque
sprite-0 pixel** overlaps an **opaque background pixel**, with **both** PPUMASK
bits 3 and 4 set (the clipper gates each layer: `regs.v` `n_CLPB` includes
`~BGE`, `CLPO` includes `~OBE`), never at x = 255 (NESdev `PPU_OAM`), and no
earlier than dot 2 (NESdev `PPU_rendering`); the shifters hold no valid data
before then. It is a set-only latch cleared at vblank end, so it holds the
*first* such dot.

Clipping (left-column masking) is generated in the H/V FSM
(`PPU/fsm.md` outputs `CLIP_O`/`CLIP_B`, `HDL/PPU/fsm.v` `HPosLogic`) and
applied independently to the two layers:

| Signal | Source bit | Effect |
|---|---|---|
| `CLIP_O` (`CLPO` into the Object FIFO) | PPUMASK `$2001` bit 2 = 0 | sprite pixels at x = 0–7 are not emitted |
| `CLIP_B` (`/CLPB` into BGCOL) | PPUMASK `$2001` bit 1 = 0 | background pixels at x = 0–7 are forced transparent |

Both come from the same decoder slot (`HPLA_3`), which strobes at H = 0–7 and
256–263; the `dlatch` pair in `HPosLogic` moves the active window to dots 1–8
(the leftmost 8 visible pixels, x = 0–7) and dots 257–264 (the line-end tail,
where the shifters of the line just finished are still running).

Consequences: with background clipping on, sprite 0 hit cannot occur in the leftmost 8 pixels (the background reads transparent); with sprite clipping on there are no sprite pixels there at all, so likewise no hit; and clipping stops only the pixel contribution, not the fetch logic, so the address schedule of §2 is unchanged. The tail occurrence of `HPLA_3` at dots 256–263 does not silently clip two tiles of the *next* line's picture: the shift-register content that would be visible there is the last tile of the current line, and the leading window (x = 0–7) governs the next line's left column.

## 7. Enable, forced blank and the odd-frame skip

`BLACK = ~(BGE | OBE)` where `BGE`/`OBE` are PPUMASK bits 3/4
(`HDL/PPU/regs.v`, `04-ppu-registers.md` §3). Rendering counts as **enabled**
when *either* bit is set.

| State | Background pixels | Sprite pixels | Memory accesses |
|---|---|---|---|
| both bits 0 (**forced blank**) | none; the pixel is the `EXT` input (backdrop `$3F00`) | none | the fetch schedule stops: `SC_CNT` is gated by `BLACK` and the VRAM read path by `BLNK` (`vram_ctrl.v`); PPU address pins show `v` instead |
| only bit 3 set | shown | none | full schedule runs |
| only bit 4 set | none | shown | full schedule runs; the background shift register keeps shifting |
| both set | shown | shown | full schedule runs |

- `BLACK` reaches the sprite logic as `BLNK`, which also gates the VRAM controller and the OAM write path. Sprite evaluation stops, but the pipeline keeps being loaded and shifted, so turning bits 3/4 on in the middle of a line shows the current shift-register content for the rest of the line (the "enable rendering mid-frame" raster trick; the residual content is unspecified) (`PPU/fsm.md`, NESdev `PPU_rendering`).
- Forced blank shows the current palette entry when `v` points into
  `$3F00–$3FFF` (direct colour), otherwise the `EXT` input — normally the
  backdrop (`04-ppu-palette.md`).
- **Odd-frame dot skip (NTSC only).** The Even/Odd circuit is a two-latch
  Johnson counter driven by the V counter and the horizontal logic
  (`HDL/PPU/fsm.v` `EvenOdd`). On odd frames it asserts `EvenOddOut` at H = 339
  (`EvenOdd` gates `nHPLA_5` with `RESCL`), and `HVCounterControl` ORs it into
  the clear: `HC = HPLA_23 | EvenOddOut`. The H counter therefore clears one dot
  early, at H = 339 instead of H = 340, so the pre-render scanline is **340
  dots** instead of 341; the dropped dot is dot 340, the dot on which `HPLA_23`
  would normally clear the counter.
  The skip needs rendering enabled (`RESCL`/`V8` conditions of the circuit) and
  is absent on PAL/Dendy, where the 2C07 replaces the block with the
  OAM-corruption workaround (`HDL/PPU/fsm.v` `EvenOdd_2C07`). Practical effect:
  with rendering enabled and a stationary screen the "dot crawl" disappears on
  NTSC.
- The H/V counters and vertical logic (`VSYNC`, `/PICTURE`, `VB`, `BLNK`, `RESCL`) are in `04-ppu-timing.md`.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | The whole screen shifts by one pixel (scroll) | Fine X `FH` selects the pattern-register tap that is output on this dot | Apply `x` as a 3-bit offset into the background shifter state, not as a pipeline delay |
| 2 | Sprite over background looks wrong at 4-colour boundaries | Stage-1 mux compares only the low two bits of each pixel, not the full 4-bit colour | Test transparency as `(pixel & %11) == 0` for both layers |
| 3 | A sprite that is behind the background makes a *nearer* sprite disappear behind the background | Sprite-vs-sprite priority is resolved before background priority; the winner then loses to the background | Reproduce the two-step priority; do not re-test later sprites |
| 4 | Mid-screen raster split shows the change one pixel late | The colour buffer latches the 5-bit index at `PCLK` | Add the one-dot pixel-output latch to the model |
| 5 | Screen becomes solid backdrop when both PPUMASK bits 3 and 4 are cleared | `BLACK` stops the fetch schedule and the pixel source becomes `EXT`/`$3F00` | Gate fetching and pixel output on `BLACK`, and show the backdrop while blanked |
| 6 | Sprite 0 hit fires later than expected by 1–2 dots | The flag is set only once a background pixel is opaque and `/VIS` is asserted; the shifters are empty until dot 2 | Compare the hit dot with tolerance for the 2-dot pipeline, and never poll before dot 8 when the left column is clipped |
| 7 | Sprite 0 hit never fires in the leftmost 8 pixels | `CLIP_B`/`CLIP_O` force the layers transparent at x = 0–7 (and the 257–264 tail) | Keep sprites used for split timing at X ≥ 8 |
| 8 | Mid-frame PPUCTRL writes split the pattern table across a line | Pattern table select is sampled into the pattern address when it is loaded, per tile | Sample `BGSEL`/`OBSEL` per fetched tile, not per frame |
| 9 | Extra or missing dot at the top of NTSC frames | The Even/Odd circuit drops one dot at the end of the pre-render line on odd frames, only with rendering on | Model the 340-dot pre-render line for NTSC odd frames; keep 341 dots on PAL/Dendy |
| 10 | Arbitrary palette colour fills the screen with rendering disabled | `TH/MUX` feeds the tile-H counter content (loaded from `$2006`) into the palette index path | Implement direct colour: `v` inside `$3F00–$3FFF` plus blanking shows that entry |
| 11 | The same tick produces a different address on two runs | The NT/AT/TA/TB phases are 2-dot accesses and the address bus is multiplexed with the data bus | Sequence address-then-read (ALE) per access; do not model the bus as a byte-wide single-cycle read |
| 12 | Sprite pixels appear one line lower than the sprite's Y value | Evaluation selects sprites for the *next* scanline, and rendering uses the result one line later | Apply the standard +1 line offset to sprite Y |
| 13 | Background flickers/garbage for the first frame after enabling rendering | The internal shift registers are dynamic and start undefined, and are only pre-filled on the pre-render line | Treat the pipeline state as unspecified at power-on; do not show it before the first pre-render fill |

## Programming / implementation notes

- MUST implement the visible-line schedule of §2: 32 background tile periods on
  dots 1–256, the two-tile prefetch on dots 321–336, and the two dummy
  nametable fetches on dots 337–340.
- MUST treat sprite evaluation of the *next* line as running in parallel with
  the background fetches of dots 65–256; the two share OAM but not the pixel pipeline.
- MUST implement each VRAM access as two dots (address + ALE, then read/write).
- MUST select the background pixel by fine X out of an 8-bit, bit-reversed
  pattern register, and keep the attribute bits constant for the whole 16-pixel
  quadrant (two tiles).
- MUST implement the priority table of §5, including: transparency is the low
  two bits, sprite-vs-sprite priority is resolved before background priority,
  and bit 4 of the resulting index marks a sprite pixel.
- MUST latch the 5-bit palette index in the colour buffer for one dot.
- MUST set sprite 0 hit only when background and sprite-0 pixels are both
  opaque, `/VIS` is asserted and the sprite was found on the line; MUST NOT clear it except at the vblank-end event (`RESCL`).
- MUST apply the left-column clips to the leftmost 8 pixels (x = 0–7) from PPUMASK bits 1 and 2, and MUST NOT let a clipped layer produce sprite 0 hit.
- MUST stop the fetch schedule and show the `EXT`/backdrop pixel when both PPUMASK bits 3 and 4 are clear; SHOULD model the unspecified shift-register content when rendering is enabled mid-frame.
- SHOULD model the NTSC odd-frame 340-dot pre-render line only when rendering is enabled, and MUST NOT apply it on PAL or Dendy.
- SHOULD implement the direct-colour (`TH/MUX`) path so that `$2006` pointing into `$3F00–$3FFF` is visible with rendering disabled.
- MUST keep `$2007` accesses off the visible line unless rendering is disabled, because they drive the same address path and mutate `v`.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Per-dot background fetch schedule, prefetch, dummy fetches | `PPU_rendering` |
| Scanline dot numbering 0–340 and the `/VIS` window | `PPU_rendering`, `PPU_frame_timing` |
| Pattern shift registers, fine X, attribute latches | `PPU_rendering`, `PPU_scrolling` |
| Priority multiplexer decision table, backdrop/EXT | `PPU_rendering`, `PPU_palettes` |
| Sprite priority quirk | `PPU_sprite_priority` |
| Sprite 0 hit conditions | `PPU_rendering`, `PPU_registers` |
| Left-column clipping (PPUMASK bits 1–2) | `PPU_registers` |
| Forced blank / rendering enable | `PPU_registers`, `PPU_rendering` |
| Odd-frame skipped dot, pre-render line length | `PPU_rendering`, `PPU_frame_timing` |
| Pattern table layout and background pattern address | `PPU_pattern_tables` |
| `v` updates during rendering | `PPU_scrolling` |
| Direct colour / backdrop override | `PPU_rendering`, `PPU_palettes` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Per-dot fetch windows (`F/NT`, `F/AT`, `F/TA`, `F/TB`, `/FO`, `OBJ_READ`) | `PPU/fsm.md`, `PPU/hv_decoder.md` | `fsm.v` (`HPosLogic`), `hv_decoder.v` |
| H/V counters, H clear at 340, V step | `PPU/hv.md`, `PPU/fsm.md` | `hv.v`, `fsm.v` (`HVCounterControl`) |
| Dot numbering 0–340, `HPLA` strobe values, FSM latch delay | `PPU/hv.md`, `PPU/hv_decoder.md`, `PPU/fsm.md` | `hv.v`, `hv_decoder.v` (`HDecoder`), `fsm.v` (`HPosLogic`) |
| Tile counters, `AT_ADR`/`NT_ADR`, counter maxima | `PPU/tilecnt.md` | `tilecnt.v`, `ppu_top.v` |
| PAMUX address sources and 14-bit `/PA0–13` | `PPU/pamux.md` | `pamux.v` |
| `PAT_ADR` bit assembly, background/sprite sources, V inversion | `PPU/par.md`, `PPU/dataread.md` | `par.v` |
| Background shift registers, reversed load, fine-X tap | `PPU/bgcol.md` | `bgcol.v` (`BGC_0`…`BGC_3`, `BGC_SR8`, `BGC_Control`) |
| Priority mux cascade and decision table | `PPU/mux.md` | `mux.v` (`PictureMUX`, `MUX_Control`) |
| Sprite 0 hit latch, `$2002[6]` | `PPU/mux.md` | `mux.v` (`Spr0Hit`) |
| Left/right clip windows `CLIP_O`/`CLIP_B` | `PPU/fsm.md`, `PPU/regs.md` | `fsm.v`, `regs.v` (`Clipper`) |
| `BLACK`/`BLNK` gating of rendering and fetches | `PPU/fsm.md`, `PPU/regs.md` | `fsm.v`, `regs.v`, `vram_ctrl.v` |
| Even/Odd circuit, odd-frame skip | `PPU/fsm.md` | `fsm.v` (`EvenOdd`, `HVCounterControl`) |
| Direct colour `TH/MUX` | `PPU/mux.md`, `PPU/vram_ctrl.md` | `mux.v`, `vram_ctrl.v` |
| Object FIFO pixel output, H inversion, priority encoder | `PPU/fifo.md` | `fifo.v` |
| Sprite pipeline / lane loading | `PPU/fifo.md`, `PPU/obj_eval.md` | `fifo.v`, `obj_eval.v` |
| Fetch order NT → AT → TA → TB, dot numbers, sprite 0 earliest dot, mapper use of dots 337–340 | — (NESdev only) | — |

## Open questions

- **`dataread.v` does not exist.** The Data Reader is spread over `par.v`,
  `tilecnt.v`, `bgcol.v`, `pamux.v` and `scroll_regs.v` (the wiki splits it the same way).
- **H-decoder strobe vs NESdev dot number.** The `HPLA` strobes of
  `PPU/hv_decoder.md` are 0–1 dot earlier than the NESdev dot window of the signal
  they drive: `I/OAM2` H 0–63 (clear cycles 1–64), `OBJ_READ` H 256–319 (sprite
  fetches 257–320), `/FO` H 320–335 (prefetch 321–336); the masks also give
  `#F/NT` at H ≡ 0,1 (mod 8) and `F/AT` at H ≡ 2,3 (mod 8) against the NESdev NT
  and AT accesses on dots 1–2 and 3–4. The H counter equals the dot number
  (`PPU/hv.md`: H ticks 0–340), so this is not a units conversion but the `dlatch`
  delay of `HPosLogic` (`PPU/fsm.md`, `HDL/PPU/fsm.v`): `S/EV`, `E/EV`, `0/HPOS`,
  `/EVAL`, `CLIP_O`/`CLIP_B`, `I/OAM2`, `OBJ_READ`, `/VIS`, `#F/NT`, `F/TA` and
  `F/TB` pass two stages, `F/AT` one. `/FO` is `fo_latch1`/`fo_latch2` combined by
  `fo3` and re-latched in `fo_latch3` (`fsm.v` `HPosLogic`), i.e. a two-stage
  serial depth with the second stage re-timed. NESdev's
  evaluation window is cycles 65–256; the latched `S/EV` edge is dot 66 and `E/EV`
  dot 256 (§1), so §2 keeps 65–256 as the NESdev anchor and the `HPLA` values as
  internal. No page states the delay in dots, and the single-latch `F/AT` reaches
  the NESdev attribute dots 3–4 only if one stage also costs a dot; whether `/FO`
  means "output enable" or "the segment in which the pattern byte is valid" is
  likewise unstated, so the pattern phase is modelled as two adjacent 2-dot windows.
  **Unconfirmed** at the dot level by ranks 1–2; the `/VIS` window (`HPLA_10`, two
  stages) is listed at the NESdev anchor for the reason in §1.
- **Shift-register decay.** NESdev describes the pattern/attribute registers
  decaying to 0 within about two frames and the mid-frame artefacts that causes;
  the HDL `BGC_*` cells hold their last value, and the decay rate is unverified by ranks 1–3.
- **`EXT`-pin erasure trick.** `PPU/mux.md` marks the grounding-`EXT` + slave
  mode trick "not confirmed experimentally". Recorded as unconfirmed.
- **Sprite 0 hit on the pre-render scanline.** Neither the wiki nor the HDL
  forbids the hit there; it depends on whether `/VIS` and the residual
  shift-register content satisfy §6. Needs a test ROM.
- **Cross-file dot labels.** The four PPU spec files use the single convention of
  the *Dot numbering* note in §1: the increments at dot 256, the copy edge at dot
  257. `04-ppu-scrolling.md` §4's `(HDL edge)` rows label the same edges 257/258.
