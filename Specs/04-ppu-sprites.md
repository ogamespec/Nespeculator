# 04-ppu-sprites — OAM, sprite evaluation and sprite rendering

**Status:** consensus (round 3)
**Scope:** The sprite subsystem: OAM (Object Attribute Memory) layout and
addressing, the sprite/DMA view of `$2003`/`$2004`/`$4014`, the per-scanline
sprite evaluation algorithm and its 8-sprite limit, the exact sprite overflow
bug (the incrementing `n`/`m` pair and its false positives/negatives), the
8×16 sprite layout, the sprite pipeline (the Wiki's Object FIFO): lane loading,
the X down-counter, the two pattern shifters, horizontal/vertical flip, sprite
priority ordering and the conditions for sprite 0 hit. The background pipeline,
the priority multiplexer and the rendering-enable rules are in
`04-ppu-rendering.md`; the frame/scanline structure is in `04-ppu-timing.md`;
the register bits are in `04-ppu-registers.md`.
**Primary sources:** `BreakingNESWiki/PPU/oam.md`, `obj_eval.md`, `fifo.md`,
`mux.md`, `fsm.md`, `par.md`, `hv_decoder.md`, `regs.md`;
`Docs/PPU/OAM_RowCol.txt`; HDL `HDL/PPU/oam.v`, `obj_eval.v`, `fifo.v`,
`mux.v`, `fsm.v`, `ppu_top.v`; NESdev `PPU_sprite_evaluation`, `PPU_OAM`,
`PPU_sprite_priority`, `PPU_rendering`, `PPU_pattern_tables` (via the Wayback
Machine).

## 1. OAM: size, layout and addressing

OAM is 2112 bits = 264 bytes — 64 sprites × 4 bytes (256 bytes, 1856 of the 2112 cells) plus a 256-bit (32-byte) secondary area holding the sprites selected for the current line, called **secondary OAM** ("temporary OAM", "OAM2") here (`PPU/oam.md`).

```
OAM byte address   n = 0..63 sprites, m = 0..3 bytes
4*n + 0   Y coordinate
4*n + 1   tile index (pattern number)
4*n + 2   attributes
4*n + 3   X coordinate (left edge of the sprite)
```

Attributes (`OAM[4n+2]`):

```
7  bit  0
---- ----
VHPx --pp
|||   ||
|||   ++- palette (palette bits 0–1)
||+------ priority: 0 = in front of background, 1 = behind background
|+------- horizontal flip
+-------- vertical flip
```

Bits 4–2 of the attribute byte have no cells in the OAM array; on revisions
that can read `$2004` they read back as 0, i.e. the byte behaves as if it were
ANDed with `$E3` (`Docs/PPU/OAM_RowCol.txt`, `PPU/oam.md`; NESdev `PPU_OAM`;
`04-ppu-memory-map.md` §8 gives the same layout; the HDL `oam.v` models a plain
256-byte array without the `$E3` mask).

The wiki's on-die address arithmetic differs from the byte-address view: OAM columns are `/OAM0–2` (plus `OAM8 = 1` to select secondary OAM) and rows are `/OAM3–7`, and on NTSC the address is inverted, so the *logical* row order is scrambled relative to the topological layout (`PPU/oam.md` "Row Decoder", `Docs/PPU/OAM_RowCol.txt`: 34+32 rows in 9 groups of 8, 8, 5, 8, 8, 8, 5, 8, 8 = 2112 cells). It is invisible through `$2004`/`$4014`, which use the same scrambled mapping; `$2004` reads during rendering return other addresses because the evaluator drives the OAM address (§2), not because of this permutation.

| Access | Behaviour |
|---|---|
| `$2003` write | loads OAMADDR (8 bits); also arms the OAM buffer. OAMADDR is forced to 0 during dots 257–320 of every visible and pre-render scanline (`04-ppu-registers.md` §5, NESdev `PPU_registers`) |
| `$2004` write | stores the byte at OAMADDR, then increments OAMADDR by 1 |
| `$2004` read | returns OAM[OAMADDR] through the `R4` latch; does **not** increment OAMADDR; not available before the 2C02G |
| `$4014` write (CPU) | OAM DMA: 256 bytes from a CPU page to OAM through `$2004`, 513/514 CPU cycles (`04-ppu-registers.md` §12, `02-cpu-dma.md`) |

Secondary OAM is not CPU-addressable. During dots 1–64 of a rendering scanline the PPU writes `$FF` into all 32 bytes (the `I/OAM2` window, `HPLA_8` = H 0–63 plus one FSM latch dot); a CPU `$2004` read during an evaluation window returns the internal evaluator address, not OAMADDR (NESdev `PPU_sprite_evaluation`; `04-ppu-rendering.md` §1).

OAM is dynamic RAM: a cell is a 4T-DRAM cell whose value decays, and refresh only happens for cells selected by the row decoder during the PCLK precharge cycle. That is "OAM decay"; games keep a shadow copy in CPU RAM and refresh all 256 bytes with OAM DMA every vblank (`PPU/oam.md` "Memory Cell"). The wiki leaves the decay timings TBD — see *Open questions*.

## 2. When sprites are evaluated

Dot numbers are the NESdev numbers used throughout this spec and in
`04-ppu-rendering.md`, i.e. they equal the H counter value (0–340).

| Event | Dots / lines | Notes |
|---|---|---|
| Secondary OAM clear (`I/OAM2`) | dots 1–64 of every visible and pre-render scanline | writes `$FF` to the 32 secondary bytes |
| Sprite evaluation (`S/EV` … `/EVAL`) | dots 65–256 (NESdev anchor; latched edges 66–256) | scans the 64 primary-OAM sprites for the **next** scanline |
| Sprite fetch (`OBJ_READ`) | dots 257–320 | loads the 8 selected sprites into the pipeline |
| Transfer for the next line | dots 321–336 | background prefetch runs at the same time |

`S/EV` is a one-dot pulse (`HPLA_2` at H = 65) and `E/EV` ends the window
(`HPLA_7` at H = 255, i.e. dot 256); `/EVAL` (`n_EVAL`) is a *level*, low only at
H = 63/255/339, not a one-dot pulse, and `obj_eval.v` uses its rising edge as the
end-of-window signal (`PPU/fsm.md`, `PPU/hv_decoder.md`, `HDL/PPU/fsm.v`; the
decoder-vs-dot offset is in `04-ppu-rendering.md` §1 and *Open questions*). Consequences:

- **One-line offset.** Evaluation for scanline `L` fills secondary OAM with the sprites to show on scanline `L+1`, so a sprite whose Y byte is `Y` first appears on scanline `Y+1` (NESdev `PPU_sprite_evaluation`).
- **The pre-render scanline does not evaluate.** It fetches sprites from the secondary OAM left over from the previous frame's scanline 239.
- Evaluation runs whenever *either* PPUMASK bit 3 or bit 4 is set (`BLACK = 0`); clearing both stops it and leaves secondary OAM stale (`PPU/fsm.md`, `04-ppu-rendering.md` §7).
- During evaluation the OAM interface is driven by the evaluator's counters, so `$2004` reads observe the evaluator's address and writes only partly reach OAM — the write enable is gated by `BLNK` (`HDL/PPU/oam.v`: `/WE` requires `BLNK`), which is why OAM DMA belongs in vblank. OAMADDR is sampled as the evaluation start address when evaluation begins (`S/EV`, dot 66 here; NESdev cycle 65), so a non-zero OAMADDR makes the evaluator start mid-list (NESdev `PPU_registers`); it is forced to 0 during the fetch window (dots 257–320, `04-ppu-registers.md` §5).

## 3. The sprite evaluation algorithm

The evaluator walks primary OAM and copies each in-range sprite into the next
secondary-OAM slot. It has two counters (`PPU/obj_eval.md`,
`HDL/PPU/obj_eval.v`):

| Counter | Size | Modes |
|---|---|---|
| main OAM counter `n` | 8 bits | +4 during the search, +1 during the copy, +1 in the `$2003`/`$2004` view |
| secondary OAM counter | 5 bits | +1 per copied byte |

In-range test for scanline `V`, sprite with Y byte `OAM[n][0]`, sprite height
`h` (8 or 16 from PPUCTRL bit 5, signal `O8/16`):

```
in_range  ⇔  OAM[n][0] ≤ V  <  OAM[n][0] + h        (unsigned 8-bit, wraps)
row       =  V − OAM[n][0]                          (0 … h−1)
```

The comparator is a signed subtractor of `V − Y` with a borrow chain (`PPU/obj_eval.md` "Comparator"). Evaluation runs on scanline `V` and fills secondary OAM for `V + 1`, so the test `Y ≤ V < Y + h` selects a sprite for display on scanlines `Y + 1 … Y + h`. A Y byte of `$EF`–`$FF` selects the sprite only for post-render scanlines (240 and later), which is why that range hides it (NESdev `PPU_OAM`). The evaluator also needs "a sprite is on the line" for the sprite-0-hit gate: it raises `/SPR0_EV` when sprite index 0 is copied (`PPU/obj_eval.md`).

Visible scanlines for a few Y bytes (8×8 sprites), showing the +1 offset:

| OAM Y byte | Scanlines where the sprite is drawn | `OV` (row) |
|---|---|---|
| `$00` | 1–8 | 0–7 |
| `$10` | 17–24 | 0–7 |
| `$98` | 153–160 | 0–7 |
| `$E7` | 232–239 | 0–7 |
| `$E8` | 233–239 visible, 240 post-render (not shown) | 0–6 visible, 7 not |
| `$EF` | 240–247, all post-render (not shown) | 0–7 |
| `$F8` | 249–256, post-render / vblank (not shown) | 0–7 |

With `h = 16` (8×16 sprites) each range above is 16 lines long, so `Y = $E7`
covers scanlines 232–247 and the last eight rows fall outside the visible
region.

Algorithm as the hardware performs it (NESdev `PPU_sprite_evaluation`; `HDL/PPU/obj_eval.v` models the same loop):

```
n = 0; m = 0; found = 0
# 1. clear secondary OAM (dots 1..64): every byte = $FF
# 2. search phase
while n < 64:
    if found < 8:
        if not in_range(OAM[n][0]): n = n + 1; continue
        copy OAM[n][0..3] to secondary OAM[found*4 .. found*4+3]
        if n == 0: sprite0_on_line = true
        found = found + 1; n = n + 1
    else:
        # 3. overflow check, with the hardware bug
        if not in_range(OAM[n][m]):
            n = (n + 1) mod 64; m = (m + 1) mod 4   # <-- bug: m follows n
            if n == 0: break
            continue
        set sprite overflow flag ($2002[5])         # NESdev step 3a
        # read the next 3 entries: m after each byte, n when m wraps
        repeat 3: m = (m + 1) mod 4; if m == 0: n = (n + 1) mod 64
        if n == 0: break
# 4. copy phase: keep attempting copies into a full secondary OAM until HBLANK
```

Rules a programmer can rely on:

- **At most 8 sprites per scanline.** Once 8 in-range sprites are found, no later sprite is displayed, wherever it sits in OAM; shuffling OAM every frame is the standard workaround.
- **Order matters.** Secondary-OAM slots are filled in OAM order, so a lower-numbered sprite is drawn "in front" of a higher-numbered one (§5.3).
- **Evaluation stops at 8 plus the overflow check.** A 9th in-range sprite is not copied; it can only set the overflow flag.
- Copied bytes are raw; the evaluator does not transform them. The in-sprite row offset `OV = V − Y` is computed during evaluation and passed to the pattern address generator (`PPU/par.md`, `PPU/obj_eval.md` "OV0-3").
- Any unfinished evaluation is cut short when `E/EV` arrives, and the secondary-OAM bytes not yet written keep the `$FF` from the clear phase.

## 4. The sprite overflow bug

The sprite overflow flag is PPUSTATUS bit 5 (`$2002[5]`), set when the evaluator looks for a 9th in-range sprite. It is sticky: cleared only at the end of vblank (`RESCL`), never by a `$2002` read (`PPU/obj_eval.md`; `HDL/PPU/obj_eval.v` keeps `ovf_reg`).

The bug is the `m` increment in step 3 above (NESdev `PPU_sprite_evaluation` §"Sprite overflow bug"): after 8 sprites have been found, the overflow check does not advance to the next sprite's Y byte for every candidate. On a *miss* it increments `n` and `m` together, so after the first comparison it reads `OAM[n+1][1]`, `OAM[n+2][2]`, `OAM[n+3][3]` — tile index, attribute and X coordinate of successive sprites — and treats them as Y coordinates. The result is a "diagonal" walk through OAM:

| Step | Comparable read | If it matches the line |
|---|---|---|
| 1 | `OAM[n][0]` (a real Y byte) | overflow set (correct) |
| 2 | `OAM[n+1][1]` (a tile index) | overflow set (false positive) |
| 3 | `OAM[n+2][2]` (attributes) | overflow set (false positive) |
| 4 | `OAM[n+3][3]` (an X coordinate) | overflow set (false positive) |
| 5 | `OAM[n+4][0]` (a real Y byte, four sprites ahead) | sprites n+1…n+3 were skipped (false negatives) |

Because the walk consumes at most 64 steps and terminates when `n` wraps to 0, the flag shows both false positives (only 8 sprites on screen but `$2002[5]` sets because some X coordinate falls in the scanline range) and false negatives (a 9th in-range sprite never reached). Once one more in-range value is found, another signal prevents the comparison from influencing the sprite address incrementation logic, and the bug is no longer in effect for that line (NESdev `PPU_sprite_evaluation`).

Practical rules: **never** use `$2002[5]` as a "more than 8 sprites" test (`PPU_sprite_evaluation` documents it as a bug/gameplay quirk); the OAM counter keeps running through the copy phase and terminates on wrap, so evaluation consumes the whole 64-sprite walk unless `E/EV` cuts it short at dot 256; and because the flag is sticky until the next vblank end, clearing it by reading `$2002` does not work.

## 5. Sprite rendering: the sprite pipeline

The pipeline that produces sprite pixels is the **sprite pipeline** (the Wiki's
"Object FIFO"): eight parallel lanes, one per secondary-OAM slot. Each lane
holds the two pattern bytes of the sprite row, the palette (attribute bits 0–1)
and priority and H-flip bits, and the X coordinate (`PPU/fifo.md`,
`HDL/PPU/fifo.v` `ObjectFIFO`).

### 5.1 Lane loading (dots 257–320)

For every dot with `OBJ_READ` active, the lane is selected by the high bits of
the delayed H counter, `H3''..H5''` (a 3-to-8 decode), and the byte type by
`H0''..H2''`:

| `H0''..H2''` | Byte loaded | Source bus |
|---|---|---|
| 2 | attribute byte (palette, priority, H flip) | OB (secondary OAM) |
| 3 | X coordinate | OB |
| 5 | pattern plane A (low) | PD (VRAM) |
| 7 | pattern plane B (high) | PD |

| Lane (sprite) | `H3'' H4'' H5''` |
|---|---|
| 0 | `H3 H4 H5` |
| 1 | `/H3 H4 H5` |
| 2 | `H3 /H4 H5` |
| 3 | `/H3 /H4 H5` |
| 4 | `H3 H4 /H5` |
| 5 | `/H3 H4 /H5` |
| 6 | `H3 /H4 /H5` |
| 7 | `/H3 /H4 /H5` |

Two details the die adds: `PD/FIFO` (from the evaluator) forces the pattern bytes of a lane with no real sprite to `$00` instead of the leftover bus value, so a partial sprite list cannot show the last byte on the bus; and the attribute byte is loaded during the second garbage nametable fetch of the sprite's 8-dot slot with the X byte on the following dot (NESdev `PPU_rendering` §257–320), matching the `H0''..H2''` = 2, 3 mapping above.

### 5.2 Pixel output

- `0/HPOS` ("clear HPos") at dot 340 (`HPLA_5` at H = 339, latched) clears the per-lane X down-counters and arms the lanes: `Z_HPOS` loads each lane's counter with its X coordinate (`HDL/PPU/fifo.v`).
- On each subsequent dot with `n_VIS = 0`, every armed lane counts down; when a lane's counter reaches 0 it emits its 8 pixels, one per dot, starting at the sprite's X coordinate.
- The two pattern bytes shift out as a pair, one bit per plane per dot, bit 7 first (leftmost pixel); the **H-flip** bit mirrors the order (`HINV`/`HDIR`, `PPU/fifo.md` "H. Inversion").
- **Vertical flip** is in the pattern address generator, not the FIFO: `OV = V − Y` selects the row and the flip bit is folded into the row calculation (`PPU/par.md`, `PPU/dataread.md`).
- A lane that has emitted all 8 pixels is done for the line; other lanes keep going independently, so a later sprite can still appear after an earlier one finished.
- Sprite pixels are suppressed at x = 0–7 when PPUMASK bit 2 = 0 (`CLPO`); the X counters still count down (`04-ppu-rendering.md` §6).
- A sprite whose X is ≥ 249 (X = `$F9`–`$FF`) runs past the end of the line. The lane emits while `n_VIS = 0` (dots 0–255) and stops there, so only the part of the 8-pixel window that falls inside the visible line is shown and the remainder is dropped, not re-emitted on the next line (`HDL/PPU/fifo.v`; NESdev `PPU_OAM`).

### 5.3 Sprite priority

The eight lanes feed a priority encoder (`PPU/fifo.md` "Sprite Priority", `HDL/PPU/fifo.v`): the winner is the **lowest-numbered OAM sprite** that emits an opaque pixel on the current dot. Its pattern bits go out as `/ZCOL0`/`/ZCOL1`, its palette as `ZCOL2`/`ZCOL3`, its priority bit as `/ZPRIO` (0 = in front of the background).

The order of operations matters: sprite-vs-sprite priority is resolved **first**, and only the winner is offered to the background/priority multiplexer. A foreground sprite that loses to an earlier "behind background" sprite stays hidden even though its own priority would have put it in front — the NESdev "sprite priority quirk" (`04-ppu-rendering.md` §5). The tie-break is strictly the OAM index, also in 8×16 mode (not the tile number).

## 6. 8×16 sprites

PPUCTRL (`$2000`) bit 5 (`O8/16`) selects the sprite size for the whole frame (`04-ppu-registers.md` §2). For 8×16 sprites the OAM tile index is interpreted differently (`PPU/par.md` "Source for OB (sprites 8x16)"):

| `PAT_ADR` bit | 8×8 source | 8×16 source |
|---|---|---|
| 0–2 | `OV0–2` (row within the sprite) | `OV0–2` |
| 3 | `/H1'` — A/B plane selection within the tile | `/H1'` |
| 4 | tile byte bit 0 (tile index) | `OV3` — second 8-pixel tile of the 16-row sprite |
| 5–11 | tile byte bits 1–7 | tile byte bits 1–7 |
| 12 | `OBSEL` (PPUCTRL bit 3) | tile byte bit 0 |

Consequences: the OAM tile byte loses its low bit, which selects the pattern table (0 = `$0000`, 1 = `$1000`), and bit 3 of `OV` (the row) becomes `PAT_ADR` bit 4, selecting the second tile of the 16-row sprite; PPUCTRL bit 3 (`OBSEL`) is ignored. The two stacked 8×8 tiles are `tile & $FE` and `tile | $01` from the table selected by `tile & $01`, and a vertical flip also swaps which of the two is drawn first. The evaluator's in-range test uses height 16 (`Y ≤ V < Y + 16`, with wrap-around); `OV` is 0–15 and its bit 3 feeds `PAT_ADR[4]`.

## 7. Sprite 0 hit

PPUSTATUS bit 6 (`$2002[6]`) is a set-only latch (`STRIKE_FF`), cleared only by `RESCL` (end of vblank), not by a `$2002` read (`PPU/mux.md`, `HDL/PPU/mux.v` `Spr0Hit`). It is set on a dot where all of the following hold (exact HDL condition):

```
STRIKE  ⇔  PCLK phase
         ∧ (BGC0 = 1 ∨ BGC1 = 1)          ; background pixel is opaque
         ∧ /SPR0HIT                        ; lane 0 emits an opaque pixel now
         ∧ /SPR0_EV                        ; sprite 0 was found on this scanline
         ∧ /VIS                            ; the dot is in the visible region
```

Operationally:

1. Sprite 0 (OAM index 0) must have been selected during evaluation of the previous scanline (in range and among the first 8), otherwise `/SPR0_EV` blocks the hit for the whole line.
2. Sprite 0 must emit a non-transparent pixel on this dot (its pattern bits are not both 0 at the current position, after flips).
3. The background pixel on the same dot must be non-transparent (`BGC1:BGC0 ≠ %00`): a background pattern colour of 0 blocks the hit even if the attribute selects a non-zero palette.
4. **Both** layers must be enabled (`BLACK = 0`; PPUMASK bits 3 *and* 4 set), because the clipper gates each layer separately (`regs.v` `Clipper`: `n_CLPB` includes `~BGE`, `CLPO` includes `~OBE`); the dot must also be inside `/VIS`.
5. Clipping applies: with background clipping on (`$2001` bit 1 = 0) the background reads transparent at x = 0–7, and with sprite clipping on (`$2001` bit 2 = 0) lane 0 emits nothing there. With either left-column clip active (bit 1 or bit 2 = 0) the earliest possible hit column is x = 8.
6. The hit is never reported at x = 255, for reasons internal to the pixel pipeline (NESdev `PPU_OAM`).

The latch is not cleared when the hit dot ends, and it holds the *first* qualifying dot; polling `$2002[6]` in the visible region therefore detects that dot. Reset/`RESCL` behaviour is in `04-ppu-registers.md` §4.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | A sprite appears one scanline lower than its Y byte | Evaluation fills secondary OAM for the next scanline | Model the +1 line offset; sprite Y is not a direct screen row |
| 2 | 9th and later sprites on a line simply vanish | Secondary OAM holds only 8 sprites; further in-range sprites are not copied | Shuffle OAM every frame, or split sprites across lines |
| 3 | `$2002[5]` sets with 8 or fewer sprites on screen (false positive) | The overflow walk reads tile/attribute/X bytes as Y after the 8th sprite | Never use `$2002[5]` as a sprite-count check |
| 4 | `$2002[5]` stays clear with 9+ sprites on screen (false negative) | The `n`/`m` diagonal walk skips sprites | Same; rely on OAM bookkeeping in the game |
| 5 | `$2002[5]` and sprite 0 hit cannot be cleared by reading `$2002` | Both flags are sticky until the vblank-end event (`RESCL`) | Wait for vblank start (or the next frame) before trusting the flags |
| 6 | A foreground sprite hides behind the background because an earlier sprite was marked "behind" | Sprite priority is resolved before background priority; the winner alone is compared with the background | Order sprites in OAM by desired priority; remember the quirk for the foreground case |
| 7 | Sprite 0 hit never fires on a line | Sprite 0 was not selected by evaluation (Y out of range or later than the 8th in-range sprite), so `/SPR0_EV` stays high | Place sprite 0 first in OAM and inside the target line's range |
| 8 | Sprite 0 hit does not fire over a blank background area | The hit also requires an opaque *background* pixel | Put a non-zero background tile under sprite 0 |
| 9 | Sprite 0 hit is missing in the leftmost 8 pixels | Left-column clips force one of the two layers transparent there | Move split-timing sprites to X ≥ 8, or enable the left-column bits in `$2001` |
| 10 | The right-hand part of a sprite at X ≥ 249 is never shown | The lane emits only while `n_VIS = 0` (dots 0–255) and then stops for the line | Keep sprites at X ≤ 248 when all 8 pixels are needed |
| 11 | Sprites flicker randomly between frames | OAM is dynamic RAM and decays; the whole 256 bytes must be refreshed every vblank | Keep an OAM shadow in CPU RAM and OAM-DMA it every vblank |
| 12 | `$2004` reads during rendering return unexpected addresses | The evaluator drives the OAM address; OAMADDR only selects the evaluation start address (sampled at dot 66, the `S/EV` edge) and is forced to 0 during dots 257–320 | Treat `$2004` reads outside vblank/forced blank as undefined; keep OAMADDR = `$00` |
| 13 | An 8×16 sprite shows the wrong two tiles | In 8×16 mode the tile byte's bit 0 selects the pattern table and bit 3 of the row selects the second tile | Mask the tile byte with `$FE`/`$01` and ignore PPUCTRL bit 3 |
| 14 | A sprite's pattern shows the wrong half when V-flipped | Vertical flip is done in the pattern address (`OV`), horizontal flip in the FIFO | Apply the two flips in different stages, or the pixels mirror on the wrong axis |

## Programming / implementation notes

- MUST lay out OAM as 64 × 4 bytes (`Y`, tile, attributes, X) plus 32 bytes of
  secondary OAM that is not CPU-visible.
- MUST implement `$2004` write as store-then-increment and `$2004` read as
  no-increment; MUST gate readable OAMDATA on the revision (2C02G and later).
- MUST implement the secondary-OAM clear to `$FF` on dots 1–64 and make `$2004`
  reads during evaluation/fetching return the internal evaluator address.
- MUST run evaluation for the *next* scanline during dots 65–256 and the sprite fetch during dots 257–320; MUST NOT evaluate on the pre-render scanline.
- MUST apply the 8-sprite limit in OAM order, and MUST use `O8/16` for the
  in-range height test (8 or 16 rows) with 8-bit wrap-around.
- MUST reproduce the overflow walk of §4 (`n` and `m` both incrementing after a
  miss, `m` wrapping to 0 then stepping `n` once more) if the emulator exposes
  `$2002[5]`; a "count the sprites on the line" shortcut is wrong.
- MUST clear `$2002[5]` and `$2002[6]` only on the vblank-end event, never on a
  `$2002` read.
- MUST implement 8×16 addressing with `tile bit 0` → pattern table and
  `OV bit 3` → `PAT_ADR bit 4`; PPUCTRL bit 3 is ignored in 8×16 mode.
- MUST resolve sprite-vs-sprite priority before background priority, using the
  OAM index as the tie-break, and MUST treat a pixel as transparent when both
  pattern bits are 0.
- MUST implement V flip in the pattern row address and H flip in the pixel
  shifter, and MUST force the pattern bytes of empty lanes to `$00`.
- MUST implement the sprite 0 hit condition of §7, including the `/SPR0_EV`,
  `/VIS` and background-opacity terms; SHOULD assert it no earlier than dot 2 of
  the line.
- SHOULD model OAM decay as a configurable option (the decay time is not
  documented by the Wiki) and MUST NOT rely on wiped OAM at power-on.
- SHOULD keep sprite writes (OAM DMA) inside vblank: the OAM write enable is
  gated by `BLNK`, so writes during rendering do not behave like normal writes.
- SHOULD keep OAMADDR at `$00` before enabling rendering, so evaluation starts
  at sprite 0 and the sprite-0-hit and priority behaviour is predictable.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| OAM layout, attributes, `$2003`/`$2004`, OAM DMA | `PPU_OAM` |
| Scanline dot numbering 0–340 and the sprite windows | `PPU_rendering`, `PPU_sprite_evaluation` |
| Sprite evaluation schedule, secondary OAM, 8-sprite limit | `PPU_sprite_evaluation`, `PPU_rendering` |
| Sprite overflow bug and its cause | `PPU_sprite_evaluation` |
| Sprite 0 hit conditions | `PPU_rendering`, `PPU_registers` |
| Sprite priority and the priority quirk | `PPU_sprite_priority` |
| 8×16 sprite tile layout and pattern table selection | `PPU_OAM`, `PPU_pattern_tables` |
| OAM decay and refresh practice | `PPU_OAM`, `PPU_power_up_state` |
| Sprite rendering dots and pipeline | `PPU_rendering` |
| OAM corruption per revision (OAMADDR writes, evaluation start) | `PPU_variants`, `Errata` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| OAM size, 64×4 layout, secondary OAM, cell map | `PPU/oam.md`, `Docs/PPU/OAM_RowCol.txt` | `oam.v` |
| OAM attribute bits 2–4 not stored, `$E3` read-back | `PPU/oam.md`, `Docs/PPU/OAM_RowCol.txt` | `oam.v` (plain 256-byte model) |
| Dot numbering, `S/EV`/`E/EV`/`I/OAM2`/`OBJ_READ` strobe dots | `PPU/hv.md`, `PPU/hv_decoder.md`, `PPU/fsm.md` | `hv.v`, `hv_decoder.v`, `fsm.v` |
| OAM row/column decoder, inverted NTSC addressing | `PPU/oam.md` | `oam.v` |
| OAM buffer, `R4` latch, write enable gating | `PPU/oam.md` | `oam.v` |
| OAM decay and refresh | `PPU/oam.md` | — |
| Evaluation window, `S/EV`, `E/EV`, `/EVAL`, `I/OAM2` | `PPU/fsm.md`, `PPU/hv_decoder.md` | `fsm.v`, `hv_decoder.v` |
| Evaluation counters, comparator, copy loop, `OV`, `/SPR0_EV`, `PD/FIFO` | `PPU/obj_eval.md` | `obj_eval.v` |
| Sprite overflow flag and the sticky `$2002[5]` | `PPU/obj_eval.md` | `obj_eval.v` |
| Overflow `n`/`m` diagonal walk (bug detail) | partially `PPU/obj_eval.md` | `obj_eval.v` (loop), NESdev for the bug |
| FIFO lanes, lane select decode, byte types | `PPU/fifo.md` | `fifo.v` |
| Down-counter, `0/HPOS`, lane arming | `PPU/fifo.md` | `fifo.v` |
| Paired shift register, H inversion, priority encoder | `PPU/fifo.md` | `fifo.v` |
| Sprite fetch dots 257–320, attribute/X load order | `PPU/fsm.md`, `PPU/hv_decoder.md` | `fsm.v`, `fifo.v` |
| 8×16 pattern addressing | `PPU/par.md` | `par.v` |
| Sprite 0 hit condition and `$2002[6]` | `PPU/mux.md` | `mux.v` (`Spr0Hit`) |
| Left-column sprite clipping | `PPU/fsm.md`, `PPU/regs.md` | `fsm.v`, `regs.v` (`Clipper`) |
| Evaluation schedule and overflow-bug description | — (NESdev) | — |

## Open questions

- **`dataread.v` is not in the checkout.** The sprite pattern addressing the
  brief assigns to it lives in `par.v` (V inversion) and `ppu_top.v`.
- **Overflow walk vs the HDL model.** `PPU/obj_eval.md` states only that
  `SPR_OV` is set when more than 8 sprites are on the line and that copying then
  stops; `HDL/PPU/obj_eval.v` stops evaluation at the 9th in-range sprite
  outright. The diagonal `n`/`m` walk (false positives and negatives) is
  documented by NESdev `PPU_sprite_evaluation` and is not restated by ranks 1–2
  in this repository. An emulator that wants bug-compatible `$2002[5]` should
  follow the NESdev algorithm of §4.
- **Exact 9th-sprite detection window.** Whether the overflow check consumes the rest of the evaluation window or is cut short by `E/EV` at dot 256, and how many `n`/`m` steps fit in the remaining dots, is not stated by any source here; the observable effect (unpredictable `$2002[5]`) is documented, the dot budget is not.
- **OAM decay timings.** `PPU/oam.md` explicitly marks the cell degradation
  timing as TBD ("Calculate or measure cell degradation timings"). No
  refresh-rate rule can be derived from ranks 1–3.
- **`$2004` read behaviour during rendering.** `PPU/oam.md` describes the
  buffer and its `R4` latch but not the evaluator's ownership of the address
  during evaluation; the behaviour in §1/§2 is NESdev-observed. The HDL drives
  `n_OAM` from the evaluator unconditionally, which is consistent but not
  documented as silicon truth.
- **Sprite overflow / OAMADDR interaction on 2C02G-2C02H.** NESdev reports
  that starting evaluation with OAMADDR ≠ 0 copies the bytes at `OAMADDR & $F8`
  over OAM row 0 (`PPU_sprite_evaluation` notes, `Errata`); no wiki page
  covers it. The related behaviour — an unaligned OAMADDR at evaluation start makes the
  bytes at and after it be reinterpreted as Y/tile/attribute/X, and the sprite
  at OAMADDR be treated as sprite 0 — is also NESdev-only
  (`PPU_registers` §OAMADDR). The HDL drives `n_OAM` from the evaluator's own
  counter starting at 0, so neither is modelled. Needs a test ROM.
- **Decoder strobe vs dot number in the evaluation window.** `PPU/hv_decoder.md`
  puts `S/EV` at H = 65 and `E/EV` at H = 255, and the H counter equals the dot
  number (0–340, `PPU/hv.md`). Both pass the two-latch `HPosLogic` chain, so the
  `S/EV` and `E/EV` edges reach the datapath at dots 66 and 256; the NESdev
  evaluation window is 65–256. The exact stage (`S/EV` vs `/EVAL`) on which
  evaluation starts and ends is not stated in dots by any page; the schedule in
  §2 follows NESdev. See `04-ppu-rendering.md` *Open questions*.
- **Sprite X past the right edge.** Sprites at X ≥ 249 keep only the pixels that
  fall in the visible window (dots 0–255), and the HDL lane stops when `n_VIS`
  goes high (`HDL/PPU/fifo.v`; NESdev `PPU_OAM`). Whether the die re-arms and
  drops the remainder on the next line is not stated by the wiki; "discarded" is
  the behaviour modelled here.
- **High Y bytes and wrap-around.** Writing `$EF`–`$FF` in the Y byte hides a
  sprite: the in-range test `Y ≤ V < Y + h` selects it only for post-render
  scanlines (240 and later) when `Y ≥ $EF` (`HDL/PPU/obj_eval.v` compares an
  8-bit `V`). Whether the die's comparator can wrap and match a high Y byte on
  the first visible scanlines is not described by `PPU/obj_eval.md` and is not
  modelled by the HDL; the one-line offset alone explains the observed
  behaviour.
