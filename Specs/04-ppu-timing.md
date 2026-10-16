# 04-ppu-timing — Clocks, H/V counters, frame timing and power-on state

**Status:** consensus (round 2)
**Scope:** The PPU's timebase: master clock and `PCLK` divider, the H and V
counters, the H/V decoders and the control-signal windows they produce, the
per-region frame tables (NTSC / PAL / Dendy), the odd-frame dot skip, the
CPU:PPU clock relationship, and the power-on / reset state. Colour generation
and the video DAC are in `04-ppu-palette.md`; the pixel pipeline that consumes
these control signals is in `04-ppu-rendering.md`.
**Primary sources:** `BreakingNESWiki/PPU/pclk.md`, `hv.md`, `hv_decoder.md`,
`fsm.md`, `pal.md`, `umc6538.md`, `pads.md`, `waves.md`,
`BreakingNESWiki/Dendy.md`; HDL `HDL/PPU/pclk.v`, `hv.v`, `hv_decoder.v`,
`fsm.v`, `ppu_top.v`; NESdev `PPU_rendering`, `PPU_frame_timing`, `Cycle_reference_chart`,
`PPU_power_up_state`, `PPU_registers`, `NTSC_video`, `PAL_video`.

## 1. Clocks and the dot

| Region | Part | Master `CLK` | Divider | Dot clock `PCLK` | One dot | One scanline (341 dots) |
|---|---|---|---|---|---|---|
| NTSC | RP2C02 | 21.477272 MHz | ÷4 | 5.369318 MHz | 0.18624 µs | 63.51 µs |
| PAL | RP2C07 | 26.601712 MHz | ÷5 | 5.3203424 MHz | 0.18796 µs | 64.09 µs |
| Dendy | UMC UA6538 | 26.601712 MHz | ÷5 | 5.3203424 MHz | 0.18796 µs | 64.09 µs |

Sources: `PPU/pclk.md` (both the frequencies and the ÷4 / ÷5 dividers),
`BreakingNESWiki/Dendy.md` (Dendy uses the PAL video clock).

- One dot = one pixel; the PPU's own state advances one dot per `PCLK` period.
  `CLK` is used by exactly one block, the video phase generator; everything else
  uses the two symmetric halves `PCLK` (high) and `/PCLK` (low), the two states
  of the pixel (`PPU/pclk.md`).
- The divider is a chain of dynamic latches: four in the RP2C02
  (`nor(latch1_in, RES, latch4_out)`, latches on `n_CLK`, `CLK`, `n_CLK`, `CLK`)
  and six in the RP2C07; a `rsff PCLK_FF` splitter produces `/PCLK`/`PCLK`
  (`HDL/PPU/pclk.v`, `PixelClock`).
- `CLK/6` is the colour subcarrier (NTSC 3.579545 MHz, PAL 4.433618 MHz; one
  subcarrier period is 6 `CLK`). One dot is 4 `CLK` on NTSC = 240° of it and
  5 `CLK` on PAL/Dendy = 300°, so the colour phase drifts against the pixel grid
  (`PPU/video_out.md`, `04-ppu-palette.md` §3).

## 2. H and V counters

Both counters are 9-bit and are bounded by the FSM, not by their width
(`PPU/hv.md`):

| Counter | Range | Increment | Clear |
|---|---|---|---|
| H (`HCounter`) | 0–340 | every `PCLK` (`carry_in` bit 0 tied to Vdd) | `HC` from the FSM |
| V (`VCounter`) | 0–261 (NTSC), 0–311 (PAL) | only while `V_IN` is active | `VC` from the FSM |

| Control signal | Source | Behaviour |
|---|---|---|
| `HC` | `HVCounterControl`: `HPLA_23 \| EvenOddOut` | clears H at dot 340, or at 339 on an NTSC odd frame (§8) |
| `V_IN` | `HPLA_23` | one-dot pulse at H = 340 |
| `VC` | `HC AND VPLA_x` (latched) | clears V at the pre-render line; `x` = 2 (NTSC, line 261) or 8 (PAL, line 311) |

A frame is therefore **341 dots × 262 lines (NTSC)** or **341 × 312 (PAL/Dendy)**;
`PPU/hv.md` gives the NTSC geometry directly. The counters' carry-lookahead
"patches" only shorten the carry delay and have no observable effect (`PPU/hv.md`).

## 3. H decoder (NTSC, RP2C02)

**Dot numbering.** This spec uses the NESdev convention, dot = `H` = 0–340.
Normative labels: the vertical (fine-Y) increment and the last coarse-X increment
of the visible line at dot 256, the horizontal `t`→`v` copy at dot 257 (its effect
visible at 258), dummy fetches 337–340. The HDL `PCLK`-edge measurements in
`04-ppu-scrolling.md` §4 observe the same events one dot later (257/258).

The H decoder ("HPLA") is a bank of 24 NOR rows, each active only for the listed
dots (confirmed against `HDL/PPU/hv_decoder.v` `HDecoder`, `RP2C02`; `PPU/hv_decoder.md`):

| Row | Dots active | Meaning / target |
|---|---|---|
| 0 | 279 | FPORCH FF reset |
| 1 | 256 | FPORCH FF set |
| 2 | 65 | `S/EV` — start sprite evaluation |
| 3 | 0–7, 256–263 | `CLIP_O` / `CLIP_B` (left-8 masking) |
| 4 | 0–255 (gated by `VB`) | `CLIP_O` / `CLIP_B` |
| 5 | 339 | `0/HPOS` (also `/EVAL`) |
| 6 | 63 | `/EVAL` |
| 7 | 255 | `E/EV` (also `/EVAL`) |
| 8 | 0–63 | `I/OAM2` — initialise secondary OAM |
| 9 | 256–319 | `OBJ_READ` — sprite pattern fetches |
| 10 | 0–255 (gated by `VB`, `BLNK`) | `/VIS` |
| 11 | H mod 8 = 0, 1 | `#F/NT` — fetch nametable byte |
| 12 | H mod 8 = 6, 7 | `F/TB` — fetch tile B |
| 13 | H mod 8 = 4, 5 | `F/TA` — fetch tile A |
| 14 | 320–335 | `/FO` — fetch output enable |
| 15 | 0–255 | `F/AT` — fetch attribute byte |
| 16 | H mod 8 = 2, 3 | `F/AT` |
| 17 | 270 | BPORCH FF set |
| 18 | 328 | BPORCH FF reset |
| 19 | 279 | HBLANK FF set |
| 20 | 304 | HBLANK FF reset |
| 21 | 323 | BURST FF reset |
| 22 | 308 | BURST FF set |
| 23 | 340 | H counter clear, V counter step |

`VB` (vertical blank) gates rows 4 and 10; `BLNK` (forced blank) gates rows
2, 5–11 and 14–15. Rows 0–1, 3, 12–13 and 16–23 are ungated
(`PPU/hv_decoder.md`).

## 4. H decoder differences on PAL (RP2C07)

Only these rows move; everything else is identical to §3
(`PPU/pal.md`, confirmed in `HDL/PPU/hv_decoder.v` `RP2C07`):

| Row | NTSC dots | PAL dots |
|---|---|---|
| 0 (FPORCH reset) | 279 | 277 |
| 17 (BPORCH set) | 270 | 256 |
| 18 (BPORCH reset) | 328 | 4 |
| 19 (HBLANK set) | 279 | 277 |
| 20 (HBLANK reset) | 304 | 302 |
| 21 (BURST reset) | 323 | 321 |
| 22 (BURST set) | 308 | 306 |
| 23 (H clear / V step) | 340 | 340 |

Rows 1–16 (including FPORCH set at 256) are unchanged.

## 5. V decoders

NTSC (`PPU/hv_decoder.md`; the ±1-dot spread in simulation is the latch skew):

| Row | Scanline | Meaning |
|---|---|---|
| 0 | 247 | VSYNC FF set |
| 1 | 244 | VSYNC FF set |
| 2 | 261 | PICTURE FF reset (picture resumes) |
| 3 | 241 | PICTURE FF set (blanking starts) |
| 4 | 241 | `/VSET` — vblank start, sets `INT_FF` / NMI |
| 5 | 0 | VB FF set (invisible region) |
| 6 | 240 | VB FF, BLNK FF set |
| 7 | 261 | BLNK FF reset |
| 8 | 261 | `VCLR` / `RESCL` — vblank end, clears `INT_FF` |

PAL (`PPU/pal.md`):

| Row | Scanline | Meaning |
|---|---|---|
| 0 | 272 | VSYNC FF set |
| 1 | 269 | VSYNC FF set |
| 2 | 1 | PICTURE FF reset (picture resumes) |
| 3 | 240 | PICTURE FF set (blanking starts) |
| 4 | 241 | `/VSET` — vblank start |
| 5 | 0 | VB FF set |
| 6 | 240 | VB FF, BLNK FF set |
| 7 | 311 | BLNK FF reset |
| 8 | 311 | `VCLR` / `RESCL` |
| 9 | 265 | Even/Odd (2C07: OAM-address workaround) |

The Dendy (UMC 6538) V decoder is identical to the 2C07 **except** that
`VPLA_4` (`/VSET`) fires 50 scanlines later, i.e. at scanline 291
(`PPU/umc6538.md`).

## 6. Per-line control-signal windows

The windows are the **settled outputs** of `HDL/PPU/fsm.v` `HPosLogic` (the
flip-flop dots of §3–§5 plus its output latch; `04-ppu-rendering.md` §1).
`SYNC = ~(sync_latch2_q | VSYNC)`, `VSYNC = ~(n_HB | vsync_ff_q)`: after the
23-dot `/FPORCH` window comes the 25-dot pulse that coincides with `/HB`
(`PPU/fsm.md` maps `/FPORCH` → `SYNC` and `/HB` → `VSYNC`).

| Signal | NTSC | PAL |
|---|---|---|
| FPORCH FF (`/FPORCH`) | high 257–279 | high 257–277 |
| `SYNC` (active low at the `/SYNC` pin) | asserted 280–304 (25 dots, ≈4.7 µs) | asserted 278–302 (25 dots, ≈4.7 µs) |
| BPORCH FF (gates `/PICTURE` off) | high 271–328 | high 257–340 + 0–4 |
| HBLANK FF (`/HB`) | high 280–304 | high 278–302 |
| `BURST` | high 309–323 (15 dots) | high 307–321 (15 dots) |
| H counter clear / V step | 340 | 340 |
| Composite picture window (`/PICTURE` on) | 329–340 + 0–270 | 5–256 |
| Left-8 clip window | 1–8, 257–264 | 1–8, 257–264 |

The composite picture window is the *blanking* gate, not the visible picture:
the TV's active area is fixed by the sync/back-porch timing and the PPU has
pixel data only for dots 0–255 (NESdev `PPU_rendering`). The 2C07 delays
`/PICTURE` by one further D-latch to the video generator (`PPU/pal.md`).

Per-dot schedule an emulator needs (both regions, dots 0–340, settled windows as
in `04-ppu-rendering.md` §2): pixel data is produced on 0–255; 257–320 are the
sprite fetch slots (`OBJ_READ`); 321–336 carry the two-tile prefetch for the next
line; 337–340 are dummy NT fetches; `SYNC` (280–304) and `BURST` (309–323) sit in
the horizontal-blanking interval that follows the front porch. The tile fetch
pattern repeats every 8 dots (`#F/NT` 0–1, `F/AT` 2–3, `F/TA` 4–5, `F/TB` 6–7).

## 7. Frame timing

| Region | Visible lines | Post-render | VBlank lines | Pre-render | Total lines | Total dots |
|---|---|---|---|---|---|---|
| NTSC | 0–239 | 240 | 241–260 | 261 | 262 | 89 342 |
| PAL (RP2C07) | 0–239 | 240 | 241–310 | 311 | 312 | 106 392 |
| Dendy (UA6538) | 0–239 | 240–290 (50 extra) | 291–310 | 311 | 312 | 106 392 |

| Region | VBlank flag / NMI set | VBlank flag cleared | VBlank length | Frame rate |
|---|---|---|---|---|
| NTSC | start of line 241 (dot 1) | start of line 261 | 20 lines (21 with post-render) | ≈60.0988 Hz |
| PAL | start of line 241 | start of line 311 | 70 lines (71 with post-render) | ≈50.007 Hz |
| Dendy | start of line 291 | start of line 311 | 20 lines (50 extra post-render lines) | ≈50.007 Hz |

- The NMI is `VBL_EN AND INT_FF`; `INT_FF` is set by `/VSET` and cleared by
  `RESCL` (line 261 / 311) or by a `$2002` read (`PPU/fsm.md`,
  `04-ppu-registers.md`).
- `VB` (the "invisible region" signal, used only by the H decoder) is active
  from line 240 to the end of vblank and `BLNK` additionally while `BLACK` is
  set; both are checked by `HDL/Framework/Icarus/ppu/fsm_test.v`.

## 8. Odd-frame dot skip (NTSC only)

The Even/Odd block is a two-flip-flop Johnson counter clocked by the V counter
LSB (`V8`); it asserts `EvenOddOut = ~q2 & HPLA_5 & RESCL`, which exists only
while `HPLA_5` (dot 339) is active **and** `RESCL` — active only at V = 261 —
confines the skip to the pre-render line. `EvenOddOut` feeds
`HVCounterControl`, whose inverting `dlatch` presents `HC = HPLA_23 | EvenOddOut`
to the H counter (`~(HPLA_23 | EvenOddOut)` is only its D input); the clear
therefore arrives one dot early on every other frame, so the **pre-render line
of an odd NTSC frame is 340 dots instead of 341** and the odd frame is one dot
shorter (`PPU/fsm.md`; `HDL/PPU/fsm.v` `EvenOdd`, `HVCounterControl`; also
asserted by `HDL/Framework/Icarus/ppu/fsm_test.v`).

- `HPLA_5` is one of the `BLNK`-gated rows of §3, so the skip happens only with
  rendering enabled; with rendering off **every** NTSC frame is
  341 × 262 = 89 342 dots. This agrees with NESdev `PPU_frame_timing` and
  `04-ppu-rendering.md` §7.
- The purpose is to avoid a fixed relationship between `PCLK` and the colour
  subcarrier (the wiki calls it "NTSC Crawl"), so a black-and-white TV does not
  see a crawling pattern (`PPU/fsm.md`, `PPU/pal.md`).
- With rendering enabled the average frame length is 89 341.5 dots; the long-run
  refresh is ≈60.0988 Hz (89 342 dots = 60.0988 Hz when rendering is off).
- **PAL (RP2C07) does not skip a dot.** The Even/Odd block is reused: its
  `EvenOddOut` goes to the sprite logic as the OAM-counter step / `$2003` write
  workaround instead of the H/V counters (`PPU/pal.md`,
  `04-ppu-sprites.md`). The Dendy 6538 inherits the PAL behaviour.

## 9. Dendy timing

The UMC UA6538 keeps the 2C07 341-dot / 312-line video timing and the PAL dot
clock, but `/VSET` (and therefore the vblank flag and NMI) moves from scanline
241 to scanline **291** — 50 lines later (`PPU/umc6538.md`). The frame still
ends at line 311, so the Dendy's vblank is 20 lines (like NTSC) instead of 70.
The Dendy CPU (UMC UA6527P) divides the 26.601712 MHz clock by 15 — 1.773448 MHz,
exactly 3 PPU dots per CPU cycle (`Cycle_reference_chart`, `CPU_variants`; §10) —
so PAL software that expects a 2C07 vblank position or length misbehaves. The
other 6538 differences (brighter DAC, OAM1 write block) are in
`04-ppu-palette.md` §3 and `04-ppu-overview.md` §6.3.

## 10. CPU:PPU phase alignment

| Region | CPU clock | PPU dot clock | `CPU:PPU` | Dots per CPU cycle | CPU cycles per scanline |
|---|---|---|---|---|---|
| NTSC | 1.7897727 MHz (÷12) | 5.369318 MHz (÷4) | 3 : 1 | 3 | 341/3 = 113⅔ |
| PAL | 1.662607 MHz (÷16) | 5.3203424 MHz (÷5) | 16 : 5 | 3.2 | 341/3.2 = 106.5625 |
| Dendy | 1.773448 MHz (÷15) | 5.3203424 MHz (÷5) | 3 : 1 | 3 | 341/3 = 113⅔ |

The `CPU:PPU` column reads as **PPU dots per CPU cycle** (`3:1` = 3, `16:5` =
16/5 = 3.2), the convention of `01-platform-overview.md` §2 — **not** `5:16`.
Dendy figures are rank 4 (`Cycle_reference_chart`: `26.60~ MHz ÷ 15 = 1.773448 MHz`,
dots/cycle 3, `341 × 5÷15 = 113⅔`, `106392 ÷ 3 = 35464` per frame; `CPU_variants`).

- **NTSC:** one CPU cycle is exactly 3 dots. 341 = 3 × 113 + 2, so the
  CPU/PPU phase repeats every **3 scanlines** (1023 dots = 341 CPU cycles). A
  frame is 89 342 dots = 29 780⅔ CPU cycles.
- **PAL:** 16 dots = 5 CPU cycles. 341 × 16 = 5456 dots = 1705 CPU cycles, so
  the phase repeats every **16 scanlines**. A frame is 106 392 dots =
  33 247.5 CPU cycles.
- **Dendy:** exactly 3 dots per CPU cycle; a 312-line frame is 106 392 dots =
  **35 464 CPU cycles** (113⅔ × 312; `PPU_power_up_state` §Dendy).
- Because the CPU interface is asynchronous to `PCLK` (`04-ppu-overview.md`
  §4), a register access lands on whatever dot it occurs on; the ratios above
  tell a cycle-timed routine how many dots a write spans, and how the alignment
  drifts across a frame.

## 11. Power-on and reset

| Item | At power-on | On `/RES = 0` |
|---|---|---|
| `PCLK` divider latches | undefined (`x`); `[0,1,0,1]` if `CLK = 0` and `z` is read as 0 | cleared (RP2C02: `latch1_in = ~(RES \| latch4_out)`; RP2C07 has the reset on `latch2`) |
| H counter | undefined | forced to 0 (`RES` term of `HVCounterBit`) |
| V counter | undefined | forced to 0 |
| Even/Odd flip-flops | undefined | not cleared by `RES`; `RESCL` gates `EvenOddOut` |
| PPU registers (`$2000`, `$2001`, `t`, `w`, `$2007` buffer) | undefined | cleared while the `/RES` latch (`RC`) is set |
| Palette RAM, OAM, nametables, CHR | unspecified | unchanged |

Sources: `PPU/pclk.md` (power-on and reset sections), `PPU/pads.md` (`/RES`
latch → `RC`), `HDL/PPU/pclk.v`, `hv.v`, `fsm.v`, `pads.v`.

After reset the PPU free-runs from `CLK`; the first frame is usable once the
counters run. NESdev adds measured detail not in the Wiki (`PPU_power_up_state`):

- `$2000`/`$2001` read back as `$00`; `$2002[7]` is random at power-on and
  **unchanged by reset** (NESdev: `+0+x xxxx` at power, `U??x xxxx` after
  reset); `t` and the `$2007` buffer are 0; `v` is unspecified.
- Writes to `$2000`, `$2001`, `$2005` and `$2006` are ignored for about
  29 658 NTSC CPU cycles (≈33 132 PAL CPU cycles) after reset; `$2002`, `$2003`,
  `$2004`, `$2007` and OAMDMA work immediately. This is why the standard
  `bit PPUSTATUS` / `bpl` wait loop is needed.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | The vblank flag and NMI arrive at a different scanline on PAL and on Dendy | `/VSET` is `VPLA_4`: line 241 NTSC, 241 PAL, 291 Dendy | Derive the vblank point from the region; never reuse NTSC line numbers |
| 2 | With rendering enabled, NTSC frames alternate between 89 342 and 89 341 dots | The Even/Odd block clears H one dot early on odd frames, and `BLNK` enables the `HPLA_5` term only while rendering runs | Implement the odd-frame dot skip (rendering enabled only); it is what removes the crawl |
| 3 | PAL picture does not flicker the way NTSC does | PAL has no dot skip; the Even/Odd block does the OAM workaround | Do not apply the NTSC skip on PAL |
| 4 | A cycle-timed raster routine works on NTSC but lands on the wrong dot on PAL | 1 CPU cycle is 3 dots NTSC, 3.2 dots PAL | Convert dot targets through the region's ratio; use the 3-/16-scanline phase repeat |
| 5 | A game that waits a fixed number of CPU cycles for vblank hangs on PAL | PAL vblank is 70 lines (241–310), NTSC 20 (241–260) | Wait on the vblank flag/NMI, not on a cycle count |
| 6 | The picture is one dot wider/narrower on alternating NTSC frames | `HC` fires at dot 339 instead of 340 on odd frames, but only while rendering is enabled | Keep the H counter 9-bit and clear it from the decoder signal |
| 7 | The composite picture window does not match the visible pixels | BPORCH/HBLANK are blanking gates; pixel data is dots 0–255 | Render pixels only on dots 0–255 and blank outside the window |
| 8 | Colour phases drift against the pixel grid | The subcarrier is `CLK/6`: one dot is 240° of it on NTSC and 300° on PAL/Dendy | Generate chroma from `CLK`, not per dot |
| 9 | The first frame after power-on/reset has wrong geometry or no stable picture | Divider and H/V counters are undefined at power-on and only cleared by `/RES` | Treat the first frame as invalid; do not sample PPU state before the counters run |
| 10 | Register writes just after reset appear to do nothing | NESdev's ignored-write window (~29 658 NTSC CPU cycles) | Use the standard `$2002` wait loop before writing control registers |
| 11 | A Dendy runs PAL-length frames but its NMI arrives mid-screen compared with a 2C07 | UMC 6538 shifts `VPLA_4` by 50 lines | Use the Dendy `/VSET` line (291), not the 2C07's |
| 12 | Turning rendering off (BLACK) changes the fetch schedule mid-line | `BLNK` gates `/VIS`, `/FO` and the fetch rows | Re-evaluate the per-dot schedule when `$2001` bits 3/4 change |

## Programming / implementation notes

- MUST run the PPU from a single master clock (NTSC ÷4, PAL/Dendy ÷5) and
  implement H as a 0–340 counter and V as 0–261 (NTSC) or 0–311 (PAL/Dendy),
  cleared by the FSM signals of §2.
- MUST implement the H/V decoder rows of §3–§5 as the gating conditions for the
  PPU's internal control lines (sprite evaluation, fetches, clipping) and MUST
  apply the odd-frame one-dot shortening on NTSC only while rendering is enabled
  (`BLNK` gates `HPLA_5`); with rendering off every NTSC frame is 89 342 dots.
- MUST set the vblank flag/NMI at line 241 (NTSC/PAL) or 291 (Dendy) and clear
  it at line 261 (NTSC) or 311 (PAL/Dendy), or on a `$2002` read.
- MUST name the region in any timing constant: NTSC 262 lines, PAL 312 lines,
  Dendy 312 lines with a 291-line vblank start.
- SHOULD keep a per-dot (0–340) resolution model rather than a per-scanline
  one, because `SYNC`, `BURST` and the fetches all sit at fixed dots and
  mid-line register writes are visible.
- SHOULD use the CPU:PPU ratios of §10 to place CPU-cycle-timed writes on the
  right dot (NTSC 3 dots/cycle, PAL 3.2).
- SHOULD treat the first frame after power-on/reset as unstable and the palette
  /OAM/nametable contents as unspecified.
- MAY ignore the carry-lookahead patches of `PPU/hv.md`; they have no behavioural effect.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Scanline/dot structure, 341 dots, fetch schedule | `PPU_rendering` |
| H/V counters and decoder row positions | `PPU_rendering` |
| Frame length, even/odd frames, VBL flag timing | `PPU_frame_timing`, `PPU_rendering` |
| Master clock, CPU:PPU ratios, frame rates | `Cycle_reference_chart` |
| VBlank timing, NMI and the `$2002` flag | `PPU_registers`, `PPU_frame_timing` |
| Odd-frame dot skip ("even/odd frames") | `PPU_frame_timing`, `PPU_rendering` |
| Power-on/reset values and the ignored-write window | `PPU_power_up_state` |
| NTSC/PAL video line structure, sync and burst | `NTSC_video`, `PAL_video` |
| Pinout and clock pin | `PPU_pinout` |
| Per-revision timing (2C07, UMC 6538/Dendy) | `PPU_variants` (standalone page, not a `PPU` section; verified against the 2024-12-25 Wayback snapshot) |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Master clock, PCLK divider, power-on divider state | `PPU/pclk.md` | `pclk.v` |
| H/V counter ranges, clear/increment signals | `PPU/hv.md` | `hv.v` |
| NTSC H decoder rows | `PPU/hv_decoder.md` | `hv_decoder.v` (`HDecoder`, `RP2C02`) |
| PAL H decoder rows | `PPU/pal.md` | `hv_decoder.v` (`HDecoder`, `RP2C07`) |
| NTSC/PAL V decoder rows | `PPU/hv_decoder.md`, `PPU/pal.md` | `hv_decoder.v` (`VDecoder`) |
| H/V control-signal windows, `SYNC`/`BURST`/`BLNK` | `PPU/fsm.md`, `PPU/waves.md` | `fsm.v` (`HPosLogic`, `VPosLogic`) |
| Even/Odd, odd-frame dot skip | `PPU/fsm.md` | `fsm.v` (`EvenOdd`, `HVCounterControl`) |
| VBlank `INT_FF`, `/VSET`, `RESCL` | `PPU/fsm.md` | `fsm.v` (`VBlankInt`) |
| PAL repurposing of Even/Odd | `PPU/pal.md` | `fsm.v` (`EvenOdd_2C07`) |
| Dendy `/VSET` shift and clone differences | `PPU/umc6538.md`, `BreakingNESWiki/Dendy.md` | — |
| `/RES` latch and `RC` | `PPU/pads.md` | `pads.v` |
| Frame geometry (dots, lines, frame length) | `PPU/hv.md`, `PPU/hv_decoder.md` | `hv.v`, `hv_decoder.v` |
| CPU `PHI0` rates, CPU:PPU ratios, frame rates | — (NESdev `Cycle_reference_chart`, rank 4) | — |
| Power-on values and ignored-write window | — (NESdev) | — |

## Open questions

- **PAL picture window.** `PPU/pal.md` gives the 2C07 BPorch FF rows as 256 (set)
  and 4 (reset), a picture gate of dots 5–256 (252 dots) rather than the ~277
  dots of real PAL active video, and adds an extra `/PICTURE` D-latch. The exact
  PAL blanking window needs hardware confirmation; the visible-pixel schedule
  (0–255) is not in doubt.
- **First-frame parity.** The Even/Odd flip-flops have no reset in the HDL
  (`sdff`), and `PPU/fsm.md` does not state their power-on state, so which of
  the first two frames is short is not derivable from ranks 1–2. NESdev treats
  it as unpredictable.
- **`/RES` hold time and the ignored-write window.** `PPU/pads.md` says the reset
  latch stays set until `RESCL = 1` (end of vblank), implying registers stay
  cleared for most of a frame; `HDL/PPU/pads.v` deliberately clears only during
  the pulse. The ~29 658 / ≈33 132 CPU-cycle write-ignore window is NESdev-only —
  no Wiki page or HDL gate implements it. Same conflict as the `RC` semantics
  item in `04-ppu-overview.md` *Open questions*.
- **Dendy CPU clock (rank 4).** No rank 1–3 source gives the UA6527P divider;
  NESdev `Cycle_reference_chart`/`CPU_variants` give ÷15 = 1.773448 MHz and
  3 dots/cycle, adopted by §10 and `01-platform-overview.md` §2.
- **PAL 2C07 reference timing.** The Wiki documents the 2C07 only through
  `PPU/pal.md` and the UMC 6538 differences; the 71-line vblank figure comes
  from `Docs/NES_Famicom_Board_Revisions.md` §4, matching the decoder rows.
- **`VPLA_2` vs `VPLA_8` for `VC`.** The NTSC `HVCounterControl` uses `VPLA_2`
  (line 261) and the PAL version `VPLA_8` (line 311); the V decoder's ±1-dot
  simulation spread makes the frame's exact last dot model-dependent.
- **Dot numbering and the latch shift.** All four PPU spec files now use the
  single convention of the *Dot numbering* note in §3 (NESdev labels 256/257;
  the HDL `PCLK`-edge measurements in `04-ppu-scrolling.md` §4 are one dot
  later). The only residue is the one-`PCLK` latch shift of the derived windows
  in §6.
