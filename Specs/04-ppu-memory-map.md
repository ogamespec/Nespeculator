# 04-ppu-memory-map — PPU address space, nametables, palettes and OAM

**Status:** consensus (round 3)
**Scope:** The 14-bit PPU address space `$0000–$3FFF` as reached by the rendering pipeline and by the CPU through PPUADDR (`$2006`) / PPUDATA (`$2007`): pattern tables, nametables and mirroring, attribute tables, palette RAM, the `$2007` read buffer, OAM, and the VRAM controller / external address bus. The per-dot fetch schedule is in `04-ppu-rendering.md`, scrolling in `04-ppu-scrolling.md`, sprite details in `04-ppu-sprites.md`, register bit fields in `04-ppu-registers.md`.
**Primary sources:** `BreakingNESWiki/PPU/vram_ctrl.md`, `oam.md`, `umc6538.md`, `cram.md`, `par.md`, `scroll_regs.md`, `pamux.md`, `tilecnt.md`, `mux.md`, `regs.md`, `dataread.md`, `pads.md`, `Readme.md`; `Docs/PPU/OAM_RowCol.txt`, `Docs/NES_Famicom_Board_Revisions.md` §5.3, the board wiring stubs in `Docs/NES/NES_HDL_Model/nes.v` (their `CartPort`/`LS139` bodies are empty — see `## Open questions`); HDL `HDL/PPU/vram_ctrl.v`, `mux.v`, `oam.v`, `cram.v`, `pamux.v`, `tilecnt.v`, `bgcol.v`, `par.v`, `scroll_regs.v`, `pads.v`, `fsm.v`, `ppu_top.v`, `regs.v`; schematic `Logisim/Evolution/PPU_Evo.circ` (`TileCounters_All`); NESdev `PPU_memory_map`, `PPU_nametables`, `PPU_attribute_tables`, `PPU_palettes`, `PPU_pattern_tables`, `Mirroring`, `PPU_OAM`, `PPU_registers`, `PPU_scrolling`, `CHR_ROM_vs._CHR_RAM` (via the Wayback Machine).

## 1. The 14-bit PPU address space

The PPU addresses 14 bits (16 KB), `$0000–$3FFF`, on a bus wholly separate from the CPU address bus (`PPU/pads.md`, `PPU/Readme.md`). Rendering builds the address internally; the CPU reaches the same space through PPUADDR (`$2006`) / PPUDATA (`$2007`). The internal address register `v` (NESdev name; the Wiki's tile-counter chain, `PPU/tilecnt.md`) has 15 bits, `v[14:0]`. `v[14]` is the top fine-Y bit, is forced to 0 by the first `$2006` write, and is never driven on the address bus; the bus carries `v[13:0]` (`PPU/scroll_regs.md`, NESdev `PPU_registers`).

| Address range | Size | Contents | Mapped by |
|---|---|---|---|
| `$0000–$0FFF` | 4 KB | Pattern table 0 (256 tiles × 16 bytes) | Cartridge |
| `$1000–$1FFF` | 4 KB | Pattern table 1 | Cartridge |
| `$2000–$23FF` | 1 KB | Nametable 0: 960-byte tile field + 64-byte attribute table at `$23C0–$23FF` | Cartridge / board VRAM |
| `$2400–$27FF` | 1 KB | Nametable 1 | Cartridge / board VRAM |
| `$2800–$2BFF` | 1 KB | Nametable 2 | Cartridge / board VRAM |
| `$2C00–$2FFF` | 1 KB | Nametable 3 | Cartridge / board VRAM |
| `$3000–$3EFF` | 3.75 KB | Unused; the cartridge mirrors `$2000–$2EFF` here | Cartridge (NESdev `PPU_memory_map`: "usually a mirror of the 2kB region from `$2000-2EFF`"; the board decode that ignores PPU A12 is not documented in this repository's rank 1–3 sources — see `## Open questions`) |
| `$3F00–$3F1F` | 32 bytes | Palette RAM indexes (16 background + 16 sprite entries) | Internal to the PPU |
| `$3F20–$3FFF` | 224 bytes | Mirrors of `$3F00–$3F1F` (only `CGA0–CGA4` are decoded; `HDL/PPU/cram.v` `CRAM_Block` indexes a 32-entry `cram[CGA]`) | Internal to the PPU |

The board supplies 2 KB of VRAM (U4, "CIRAM") for `$2000–$2FFF`; the cartridge supplies the pattern tables as CHR-ROM or CHR-RAM, not nametable VRAM (`Docs/NES_Famicom_Board_Revisions.md` §5.3). The cartridge returns CIRAM A10 (the bank select for each nametable slot) and `/VRAM CE`; the PPU does not drive CIRAM A10 (`01-platform-overview.md` §3).

## 2. Pattern tables (`$0000–$1FFF`) — CHR

Each pattern table is 4 KB = 256 tiles of 16 bytes = two 8-byte bit planes. Plane 0 holds bit 0 of each pixel's colour index, plane 1 holds bit 1; `%00` is transparent (background) and `%11` is colour 3 (NESdev `PPU_pattern_tables`; the fetch is modelled in `PPU/par.md`, `PPU/dataread.md`). The address is assembled in the pattern-address portion of `v` (the Wiki's Picture Address Register, PAR, `PPU/par.md`; NESdev treats it as part of the `v` register):

```
13 12 11 10  9  8  7  6  5  4  3  2  1  0
 0  P  N  N  N  N  N  N  N  N  B  y  y  y
    |  +----------------------+  |  +----- fine Y (row within the tile)
    |       tile number          |  +------ plane (0 = bit 0, 1 = bit 1)
    +---------------------------- pattern table select
```

| PAR bit | Background | 8×8 sprite | 8×16 sprite | Role |
|---|---|---|---|---|
| 0–2 | Fine-Y counter `FV0–FV2` | OAM2 `OV0–OV2` | OAM2 `OV0–OV2` | Row within the tile |
| 3 | `/H1'` | `/H1'` | `/H1'` | Bit plane (A/B byte of the row) |
| 4 | Nametable bit 0 | OAM2 tile-index bit 0 | OAM2 `OV3` | Pattern-table index |
| 5–11 | Nametable bits 1–7 | OAM2 tile-index bits 1–7 | OAM2 tile-index bits 1–7 | Tile index |
| 12 | `BGSEL` (`$2000[4]`) | `OBSEL` (`$2000[3]`) | OAM2 tile-index bit 0 | Pattern-table select |

The sprite columns read from **secondary OAM** (`OAM2` in the Wiki, §8), the per-scanline copy of the sprite bytes, not OAMDATA (`$2004`) directly.

So `address = PTSEL·$1000 + tile·$10 + plane·8 + fineY`. Reads and writes are both electrically possible — the PPU emits `/RD`, `/WR` and `ALE` for VRAM accesses (`PPU/vram_ctrl.md`, `HDL/PPU/vram_ctrl.v`). On a CHR-ROM board the memory ignores the write strobe and many boards do not decode PPU `/WR` for this range, so `$2007` writes have no effect; on a CHR-RAM board writes are stored (NESdev `PPU_pattern_tables`, `CHR_ROM_vs._CHR_RAM`; `Docs/NES_Famicom_Board_Revisions.md` §5.3). Mappers may bank-switch the whole range or 1/2/4 KB parts of it.

## 3. Nametables (`$2000–$2FFF`) and mirroring

A nametable is 1 KB: 960 bytes of 8×8 tile indexes (32 columns × 30 rows) at `+$000–$3BF`, then a 64-byte attribute table at `+$3C0–$3FF` (NESdev `PPU_nametables`). The four logical nametables form a 2×2 arrangement, but the board has only 2 KB of VRAM, so the cartridge drives CIRAM A10 to fold two logical nametables onto each physical 1 KB bank.

| Arrangement (NESdev) | Tile map | CIRAM A10 source | Consequence |
|---|---|---|---|
| Vertical mirroring (board pad "H") | 64×30 | PPU A10 | `$2000`=`$2800`, `$2400`=`$2C00` |
| Horizontal mirroring (board pad "V") | 32×60 | PPU A11 | `$2000`=`$2400`, `$2800`=`$2C00` |
| One-screen | 32×30 | Mapper output (or tied GND/VCC) | All four slots show one bank, switched mid-frame |
| Four-screen | 64×60 | Extra 2 KB cartridge VRAM, not CIRAM | All four nametables distinct |

(Source: NESdev `Mirroring`, `PPU_nametables`; cartridge wiring in `Docs/NES_Famicom_Board_Revisions.md` §5.3.) The silkscreen names the tile layout, not the scroll direction: the "H" pad gives vertical mirroring and the "V" pad horizontal mirroring (NESdev `Mirroring`). The PPU itself knows nothing about mirroring — it emits `PA0–PA13` and the cartridge returns CIRAM A10. `$3000–$3EFF` is usually the same 2 KB repeated (NESdev `PPU_memory_map`); no rank 1–3 source in this repository states the board-level decode that produces it, so treat it as NESdev-only (see `## Open questions`). The PPU never draws from it (§1). Scrolling past the 32-column field toggles nametable bit 10 and wraps coarse X 31→0; passing coarse Y 29 toggles bit 11. Coarse Y = 30/31 do not wrap (§4).

## 4. Attribute tables

Each nametable's last 64 bytes are an 8×8 array of attribute bytes; one byte covers a 32×32-pixel (4×4 tile) area split into four 2×2-tile quadrants (NESdev `PPU_attribute_tables`):

```
7  bit  0
---- ----
BB AA CC DD
|| || || ++- Top-left quadrant palette (2 bits)
|| || ++---- Top-right quadrant palette
|| ++------- Bottom-left quadrant palette
++---------- Bottom-right quadrant palette
```

`byte = (bottomright << 6) | (bottomleft << 4) | (topright << 2) | topleft`; the bits select one of four background palettes. The Wiki calls the fetch address bus `AT_ADR` (`PPU/tilecnt.md`, `PPU/pamux.md`); it is assembled as:

```
AT_ADR[2:0]  = coarse X / 4   (THO[4:2])
AT_ADR[5:3]  = coarse Y / 4   (TVO[4:2])
AT_ADR[11:10]= nametable bits (NTH, NTV)
AT_ADR[13:12]= %10            (constant while rendering)
AT_ADR[9:6]  = %1111          ($3C0 attribute-table offset)
```

which is exactly the NESdev render-time formula:

```
AT address = $23C0 | (nametable << 10) | ((coarse Y >> 2) << 3) | (coarse X >> 2)
```

The constant field `AT_ADR[9:6]` is one Vdd tie in the schematic (`Logisim/Evolution/PPU_Evo.circ`, circuit `TileCounters_All`); the generated `HDL/PPU/tilecnt.v` currently ties those four bits to 0 instead, which is a defect — see Open questions. The corresponding nametable tile address is `$2000 | (nametable << 10) | (coarse Y << 5) | coarse X` (`HDL/PPU/tilecnt.v` `NT_adr`); the pattern address for the same tile is in §2. `04-ppu-rendering.md` §3 drives this address during the AT fetch slot; the two files' notation for the constant field is reconciled in Open questions. Quirks a programmer can see:

- **Bottom rows.** The visible field is 30 tile rows but the attribute map covers 32. Coarse Y can be set out of bounds (> 29, e.g. a `$2005` value ≥ 240); the PPU then fetches **tile data out of the attribute table region**, so one or two rows of attribute bytes appear as tiles. Incrementing coarse Y from 31 wraps to 0 without switching the nametable (NESdev `PPU_scrolling`).
- **Right edge / seam.** Attribute bits are re-fetched per 8-pixel sliver and shared by the four 8-pixel columns of a 16-pixel area, so a scroll seam can fall inside an attribute cell and half of a 16-pixel block gets the wrong colour — the classical edge "attribute glitch" (NESdev `PPU_attribute_tables` §"Glitches", `Mirroring`). The dot schedule of the redundant right-edge fetches is in `04-ppu-rendering.md`.
- **Redundant fetches.** The attribute byte is re-fetched for every tile row and for the two unused prefetch tiles, so a mapper (MMC5) can return a different byte per fetch (NESdev `PPU_attribute_tables` §"Expansion").

## 5. PPU address bit fields

`v` and `t` (NESdev names; the Wiki's tile counters and paired scroll registers) share one 15-bit layout, bits 14–0:

```
yyy NN YYYYY XXXXX
||| || ||||| +++++-- coarse X scroll      (bits 4–0, 0–31)
||| || +++++-------- coarse Y scroll      (bits 9–5, 0–29 visible)
||| ++-------------- nametable select     (bits 11–10; bit 10 = NTH, bit 11 = NTV)
+++----------------- fine Y scroll        (bits 14–12, 0–7)
```

`v[14]` (fine Y bit 2) is forced to 0 by the `$2006` first write and is never driven on the 14-bit address bus; the `$2007` fetch places `v[13:0]` on it (§7). The bit assignments are confirmed bit-for-bit by `04-ppu-scrolling.md` §1 and `HDL/PPU/scroll_regs.v`.

| Field | Bits | Written by | Used by |
|---|---|---|---|
| Coarse X (Wiki `TH`) | 4–0 | `$2005` 1st write bits 7–3; `$2006` 2nd write bits 4–0 | `NT_adr` bits 0–4, `AT_adr` bits 0–2 |
| Coarse Y (Wiki `TV`) | 9–5 | `$2005` 2nd write bits 7–3; `$2006` writes | `NT_adr` bits 5–9, `AT_adr` bits 3–5 |
| Nametable X (Wiki `NTH`) | 10 | `$2000[0]`; `$2006` 1st write bit 2 | `NT_adr`/`AT_adr` bit 10 |
| Nametable Y (Wiki `NTV`) | 11 | `$2000[1]`; `$2006` 1st write bit 3 | `NT_adr`/`AT_adr` bit 11 |
| Fine Y (Wiki `FV`) | 14–12 | `$2005` 2nd write bits 2–0; `$2006` 1st write bits 5–4, bit 14 cleared | Pattern row; **not** in the NT/AT address |
| Fine X (Wiki `FH`) | outside `v`, 3 bits | `$2005` 1st write bits 2–0 | Background shifter pixel offset |

The full `$2005`/`$2006` → field mapping and the shared first/second latch `w` are in `04-ppu-registers.md` §7–§8 (`PPU/scroll_regs.md`, `HDL/PPU/scroll_regs.v`).

| Fetch | Address placed on the bus |
|---|---|
| `$2007` (CPU) | `v[13:0]` (bit 14 is not on the bus), then increment by 1 or 32 |
| Nametable | `$2000 \| (v[11:10] << 10) \| (v[9:5] << 5) \| v[4:0]` |
| Attribute | `$23C0 \| (v[11:10] << 10) \| ((v[9:5] >> 2) << 3) \| ((v[4:0] >> 2) & 7)` |
| Pattern | `PTSEL << 12 \| tile << 4 \| plane << 3 \| fineY` (§2) |

## 6. Palette RAM (`$3F00–$3FFF`)

32 entries of 6 bits: 16 background entries and 16 sprite entries, i.e. four 4-entry palettes for each layer (`PPU/Readme.md`, `PPU/cram.md`). A palette index is 5 bits, produced by the pixel-priority multiplexer:

```
4 bit 0
-------
S A A P P
| | | | |
| | | +-+- Pixel value from the pattern data (0-3)
| +-+----- Palette number from the attribute bits (0-3)
+--------- Background (0) / sprite (1) select
```

(NESdev `PPU_palettes`; the Wiki equivalent is the multiplexer output `PAL0–PAL4` in `PPU/mux.md`, where colour bit 4 is the layer select.)

| Address | Contents |
|---|---|
| `$3F00–$3F0F` | 4 background palettes × 4 entries |
| `$3F10–$3F1F` | 4 sprite palettes × 4 entries |
| `$3F20–$3FFF` | Mirrors of `$3F00–$3F1F` |

- Entry 0 of each palette is transparent for rendering, but entry 0 of palette 0 is the **backdrop colour**, shown wherever both layers are transparent (NESdev `PPU_palettes`).
- **Backdrop alias.** `$3F10` is the *same cell* as `$3F00`, and likewise `$3F14`/`$3F04`, `$3F18`/`$3F08`, `$3F1C`/`$3F0C`. The cell-array decoder uses column `{CGA3,CGA2}` and row `{CGA4,CGA1,CGA0}`, and physical rows 0 and 4 share one word line, so any pair differing only in `CGA4` with `CGA1=CGA0=0` is one cell (`PPU/cram.md`; `HDL/PPU/cram.v` `CRAM_Decoder` asserts `ROW0_4` for both `%000` and `%100`). `CGA4` is exactly the background/sprite select bit.
- Only `CGA0–CGA4` are decoded (`HDL/PPU/cram.v` `CRAM_Block` indexes `cram[CGA]` with a 5-bit `CGA`), producing the `$3F20–$3FFF` mirror.
- **Backdrop override (the Wiki's "direct colour").** A `$2006` address in `$3F00–$3FFF` makes the VRAM controller assert `TH/MUX`, routing the `TH` counter (the low 5 address bits) into the pixel multiplexer as the palette index (`PPU/vram_ctrl.md`, `PPU/mux.md`). This is how palette reads/writes through `$2007` work, and why setting a `$3Fxx` address with rendering off makes that colour fill the screen (NESdev `PPU_palettes` §"Backdrop override").

## 7. PPUDATA (`$2007`) and the read buffer

Accesses use `v`, then increment it by 1 or 32 (`$2000[2]`); the increment and rendering-time side effects are in `04-ppu-registers.md` §9 and `04-ppu-scrolling.md`.

- **Read buffer.** A `$2007` read returns the internal read buffer (the Wiki's READ BUFFER, RB), then loads the addressed byte into it, so reads lag by one access; the buffer is updated only by `$2007` reads (`PPU/vram_ctrl.md` §"Read Buffer", `HDL/PPU/vram_ctrl.v` `ReadBuffer`). Prime it with one dummy read after setting `$2006`.
- **Palette bypass.** A read in `$3F00–$3FFF` returns the palette entry immediately through the palette output latch (the Wiki's Color Buffer, signals `#CB/DB`), with no priming read, while the ordinary VRAM read at the same address still occurs and is what lands in the buffer (`PPU/cram.md`, signals `#CB/DB`/`#DB/CB`; NESdev `PPU_registers`). The range is unreadable — the read returns the shadowed VRAM byte instead — on the 2C02E and all earlier NTSC parts and on the RGB 2C03, 2C04 and 2C05 series (`04-ppu-registers.md` §9, `PPU_variants`: RC2C05-01…04 behave like the 2C03, RC2C05-99 like the 2C02E).
- Reads of unmapped VRAM normally return the low address byte rather than a decaying latch, because the PPU data bus `PD` is multiplexed with the address bus (`PPU/Readme.md`). The palette-corruption hazard of leaving `v` inside `$3F00–$3FFF` while rendering, and its workaround, are in `04-ppu-registers.md` §8–§9.

## 8. OAM (sprite memory)

The CPU sees OAM as 256 bytes: 64 sprites × 4 bytes, addressed by OAMADDR (`$2003`) and read/written through OAMDATA (`$2004`) or OAMDMA (`$4014`).

| Byte | Bits | Meaning |
|---|---|---|
| +0 | 7–0 | Sprite Y coordinate (top edge; written value = top scanline − 1, so the sprite first appears on scanline `value + 1`) |
| +1 | 7–0 | Tile index (bit 0 is the pattern-table select in 8×16 mode) |
| +2 | 7–6 | Vertical flip (bit 7), horizontal flip (bit 6) |
| +2 | 5 | Priority (0 = in front of background) |
| +2 | 4–2 | **Not stored** in the cell array; reads return 0 |
| +2 | 1–0 | Palette number (sprite palettes `$3F10–$3F1F`) |
| +3 | 7–0 | Sprite X coordinate (left edge) |

(Byte order: NESdev `PPU_OAM`, `PPU_memory_map`, which also states that unimplemented attribute bits 2–4 read back as 0 on revisions that allow `$2004` reads; the attribute bit layout is detailed in `04-ppu-sprites.md`.) Physical organisation (`PPU/oam.md`, `Docs/PPU/OAM_RowCol.txt`):

- The array holds **2112 cells (264 bytes)**: 1856 bits of sprite data plus **256 bits (32 bytes) of secondary OAM** — the Wiki's "temporary OAM" / `OAM2` — for the eight sprites selected on the current scanline. This is not the 8-bit OAM buffer (`OB`) that stages each byte between array and bus.
- Sprite data is **29 bits per sprite**, not 32: the three missing bits are attribute bits 2–4, which have no cells. The column decoder skips those bit-line columns for OAM-buffer bits 2–4, and the skipped columns land exactly on attribute bits 2–4 (`PPU/oam.md` §"Декодер колони"); the secondary-OAM copy does not skip columns.
- Row/column map (`Docs/PPU/OAM_RowCol.txt`): top block 34 rows × 32 cells, bottom block 32 rows × 32 cells; rows grouped `8, 8, 5, 8, 8, 8, 5, 8, 8` (Y, tile, attribute, X for two 32-sprite halves, then 8 secondary-OAM rows), i.e. 58 primary rows plus the 8 secondary rows = `1856 + 256 = 2112`. Columns are `/~OAM0–2` plus `OAM8` (`OAM8 = 1` selects secondary OAM); rows are `/~OAM3–7`, a 1-of-32 one-hot decode. During `PCLK = 1` all ROW outputs are 0 (no cell selected). (`Docs/PPU/OAM_RowCol.txt` quotes the same map as `1792 + 320 = 2112 bits`; that partition disagrees with its own row grouping — 58 primary rows + 8 secondary rows is `1856 + 256`, and 1792 + 320 implies 56 primary rows — and with the 29-bit sprite. The `1856 + 256` split is the one `PPU/oam.md` and the HDL support.)
- **OAM decay.** The cells are 4T dynamic RAM: the value is a charge on two cross-coupled gates and leaks. The array is refreshed during precharge (`PCLK = 1` recharge, `PCLK = 0` write-back to the selected word line), so an unselected word line decays. Games keep a CPU-RAM copy (usually `$0200`) and copy all 256 bytes every vblank with OAMDMA (`$4014`) (`PPU/oam.md`; the timings are marked TBD there).
- The row decode order differs by region: the NTSC PPU drives the OAM address inverted (`/OAM0–7`), permuting the logical row numbering, while the PAL PPU drives it non-inverted so the decoder's right-to-left 0–31 order holds (`PPU/oam.md` §"Декодер ряда").
- A `$2004` write reaches the array only while `BLNK` is active (rendering off or vblank); the UMC 6538 clone has that vblank write-block disconnected (`PPU/oam.md`, `PPU/umc6538.md`).

## 9. VRAM controller and the external PPU address bus

The VRAM controller ("Контроллер VRAM", `PPU/vram_ctrl.md`) is the PPU's lower-half bus controller and contains the read buffer:

| Signal | Dir | Meaning |
|---|---|---|
| `/RD` | out | VRAM read strobe, pin 24, from the VRAM controller: asserted by the latched H-counter LSB while rendering and by `PD/RB`, active low (`HDL/PPU/vram_ctrl.v`) |
| `/WR` | out | VRAM write strobe, pin 23, from the VRAM controller, active low |
| `/RD_CPU` (`n_CpuRD`) | int | CPU-interface read strobe from the R/W decoder, for the D0–D7 pads only; `0` on a read while `/DBE = 0`, `1` otherwise |
| `/WR_CPU` (`n_CpuWR`) | int | CPU-interface write strobe from the same decoder; complementary to `/RD_CPU` while `/DBE = 0`, both `1` while `/DBE = 1` |
| `ALE` | out | 1 while `AD0–AD7` carries the low address byte, 0 while it carries data |
| `A8–A13` (`/PA8–PA13`) | out | High address bits (active-low internally, inverted at the pad) |
| `AD0–AD7` | bidir | Multiplexed low address byte / data byte; the board latches the address into a 74LS373 |
| `TSTEP` | int | Steps the `TH`/`TV` tile counters (to the Data Reader) |
| `DB/PAR` | int | Connects internal DB to the PAR address latch to load a CPU-written address/scroll value |
| `PD/RB` | int | Connects external PPU data `PD` to the read buffer to load the next value |
| `TH/MUX` | int | Sends the `TH` counter to the pixel multiplexer as the palette index (§6) |
| `XRB` | int | Tri-states the read buffer off internal DB except during a `$2007` read |

(Source: `PPU/vram_ctrl.md`; `HDL/PPU/vram_ctrl.v` `VRAM_Control`/`ReadBuffer`; `HDL/PPU/pads.v` for the pads.) The `/RD` and `/WR` rows of the table are the **VRAM-controller** outputs on pins 24/23. The complementary pair that goes inactive when `/DBE = 1` is the R/W decoder's `/RD_CPU`/`/WR_CPU` (HDL `n_CpuRD`/`n_CpuWR`), which drives the D0–D7 pads only and never reaches the VRAM pins (`PPU/regs.md` §"R/W Декодер", `HDL/PPU/regs.v` `RWDecoder`, `HDL/PPU/ppu_top.v`); `04-ppu-overview.md` §4 tabulates it. `H0_D` is the H-counter LSB delayed by one latch (`HDL/PPU/fsm.v` `DelayedH`), so `/RD` is asserted on alternate dots of every rendering scanline **while `/DBE = 1`**. An emulator must therefore track the two pairs separately: `/DBE` gates the CPU register interface, not VRAM fetch.

The final external address is produced by the **address multiplexer** (PAMUX, `PPU/pamux.md`), a 14-bit latch bank selecting per bit between the PAR pattern address (`PAT_ADR0–13`), the tile-counter `AT_ADR`/`NT_ADR`, and the CPU data bus `DB0–7` for the low eight bits (`HDL/PPU/pamux.v`). Its output `/PA0–13` is active-low; `PA13` is also returned to the cartridge inverted (`01-platform-overview.md` §3).

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | The first `$2007` read after `$2006` returns the previous byte | One-byte read buffer ahead of VRAM | Perform one dummy read to prime it |
| 2 | Palette reads return the colour immediately, with no priming read | Palette RAM is read through the palette output latch (Color Buffer), bypassing the read buffer | Implement the palette bypass for `$3F00–$3FFF` |
| 3 | The backdrop changes when writing the first sprite palette entry | `$3F10` and `$3F00` are the same cell (rows 0/4 merged) | Alias `$3F10/$3F14/$3F18/$3F1C` to `$3F00/$3F04/$3F08/$3F0C` |
| 4 | Palette writes beyond `$3F1F` hit existing entries | Only `CGA0–CGA4` are decoded in the CRAM array | Mirror `$3F00–$3F1F` through `$3F20–$3FFF` |
| 5 | The right edge shows wrong colours while scrolling | Attribute bits are shared per 16×16 area; right-edge fetches cross the nametable seam | Accept, hide with clipping/sprites, or use one-screen/4-screen mirroring |
| 6 | Scrolling down past the last tile row shows garbage tiles | Coarse Y > 29 fetches tile data from the attribute table region; Y=31 wraps without a nametable switch | Keep scroll in range, or use the effect for the overscan area |
| 7 | The same nametable appears at two of the four `$2000` slots | Cartridge CIRAM A10 wiring (horizontal/vertical mirroring) | Implement mirroring per cartridge; one-screen needs a mapper-driven CIRAM A10 |
| 8 | `$3000–$3EFF` reads return nametable data | NESdev: the cartridge "usually" mirrors `$2000–$2EFF` into this range; the board decode behind it is not documented in this repository's rank 1–3 sources | Mirror `$3000–$3EFF` onto `$2000–$2EFF` for standard boards (NESdev, rank 4) |
| 9 | Pattern data cannot be changed / writes do nothing | CHR-ROM ignores the write strobe; some boards do not decode PPU `/WR` | Bank-switch or write CHR-RAM only; treat CHR-ROM as read-only |
| 10 | Sprites fade or corrupt if left alone | OAM is 4T dynamic RAM and decays unless refreshed | Refresh all 256 bytes with OAMDMA (`$4014`) every frame |
| 11 | With rendering off, a `$3Fxx` `$2006` address fills the screen with that colour (backdrop override) | `TH/MUX` routes the `TH` counter into the pixel multiplexer as the palette index | Use for palette tests; do not leave `v` in the palette range when enabling rendering |

## Programming / implementation notes

- MUST model a 14-bit PPU address space `$0000–$3FFF`, separate from the CPU bus, with the map of §1.
- MUST drive only `v[13:0]` on the PPU address bus; `v[14]` is a scroll-only bit and is forced to 0 by the first `$2006` write.
- MUST implement the four nametable slots and the cartridge-supplied CIRAM A10 (vertical, horizontal, one-screen, four-screen); SHOULD mirror `$3000–$3EFF` onto `$2000–$2EFF` as NESdev describes for standard boards (rank 4 — the board decode is not documented in this repository's rank 1–3 sources; see `## Open questions`).
- MUST implement the 960-byte tile field plus the 64-byte attribute table per nametable, with the attribute byte/quadrant decomposition of §4.
- MUST place background fetches at `$2000 | …`, attributes at `$23C0 | …` and patterns at `PTSEL·$1000 + tile·$10 + plane·8 + fineY`; MUST NOT fetch background from `$3000–$3FFF`.
- MUST hard-wire the attribute-fetch address bits `[9:6]` to 1 (the `$3C0` offset) and bits `[13:12]` to `%10`; do not derive them from the tile address or the fetch reads tile data instead of attribute bytes.
- MUST implement the `$2007` one-byte read buffer and the palette bypass on revisions that can read palette RAM; on the 2C02E and earlier parts and on the RGB 2C03, 2C04 and 2C05 series the palette range MUST behave as ordinary VRAM (RC2C05-01…04 like the 2C03, RC2C05-99 like the 2C02E; `04-ppu-registers.md` §9).
- MUST alias `$3F10/$3F14/$3F18/$3F1C` to `$3F00/$3F04/$3F08/$3F0C` and mirror the palette through `$3F20–$3FFF`.
- MUST increment the VRAM address after every `$2007` access by 1 or 32 (`$2000[2]`); rendering-time increment rules are in `04-ppu-scrolling.md`.
- MUST implement backdrop override (`TH/MUX` "direct colour") so a `$3Fxx` address reaches the palette logic with rendering off.
- MUST model OAM as 256 CPU-visible bytes (64 × 4, order Y/tile/attribute/X) plus 32 bytes of secondary OAM; SHOULD model attribute bits 2–4 as absent (writes dropped, `$2004` reads return 0) and SHOULD model OAM decay.
- SHOULD gate `$2004` writes on `BLNK`; MAY ignore the NTSC/PAL row-decode permutation unless die-accurate decay is modelled.
- MUST expose the external bus: 14 address bits, multiplexed `AD0–AD7` with `ALE`, the VRAM controller's `/RD`/`/WR` strobes and the 8-bit `PD` data bus.
- MUST keep the CPU-interface R/W decoder (`/RD_CPU`/`/WR_CPU`) separate from the VRAM `/RD`/`/WR` pins: the former is gated by `/DBE`, the latter is not, and both VRAM strobes stay active during rendering (`PPU/regs.md` §"R/W Декодер", `HDL/PPU/vram_ctrl.v`).
- SHOULD treat CHR-ROM as read-only and CHR-RAM as writable per cartridge configuration, not per PPU state.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| 16 KB PPU memory map, mapper responsibilities, the `$3000–$3EFF` mirror | `PPU_memory_map` |
| `v`/`t` address bit fields, PPUADDR/PPUSCROLL writes, nametable/attribute fetch addresses | `PPU_scrolling`, `PPU_registers` |
| Nametable layout, 960 + 64 bytes, four screens | `PPU_nametables` |
| Mirroring modes and CIRAM A10 wiring | `Mirroring` |
| Attribute table layout, quadrants, glitches | `PPU_attribute_tables` |
| Pattern tables, bit planes, CHR address format | `PPU_pattern_tables`, `CHR_ROM_vs._CHR_RAM` |
| Palette RAM, backdrop, backdrop override, colour values | `PPU_palettes` |
| `$2007` read buffer, palette reads, increment | `PPU_registers`, `PPU_scrolling` |
| OAM structure, `$2003`/`$2004`/`$4014` | `PPU_OAM` |
| PPU bus pins, `ALE`, the VRAM `/RD`/`/WR` strobes, the CPU-interface R/W decoder, VRAM controller | `PPU_pinout`, `PPU_registers` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| 14-bit address space; pattern/nametable/palette map | `PPU/Readme.md`, `PPU/pads.md` | `ppu_top.v`, `pads.v` |
| Pattern address assembly (PAR), bit roles | `PPU/par.md`, `PPU/dataread.md` | `par.v` |
| Attribute/nametable address generation | `PPU/tilecnt.md`, `PPU/pamux.md` | `tilecnt.v` (`AT_adr`, `NT_adr`), `pamux.v` |
| Nametable mirroring, CIRAM A10 | `PPU/pamux.md` | `pamux.v` |
| `$3000–$3EFF` nametable mirror | — (NESdev only; the board decode is not in the Wiki) | — (the `CartPort`/`LS139` bodies of `Docs/NES/NES_HDL_Model/nes.v` are empty) |
| Attribute table layout, quadrant mapping, `AT_adr` constant field | — (NESdev) | `tilecnt.v` (`AT_adr`), `bgcol.v` |
| `v`/`t` field layout and `$2005`/`$2006` mapping | `PPU/scroll_regs.md` | `scroll_regs.v` |
| Palette RAM organisation, row 0/4 merge, mirrors | `PPU/cram.md` | `cram.v` (`CRAM_Decoder`, `CRAM_Block`) |
| Palette index path, backdrop override (`TH/MUX` direct colour) | `PPU/mux.md`, `PPU/vram_ctrl.md` | `mux.v`, `cram.v`, `vram_ctrl.v` |
| Read buffer, one-byte delay, palette bypass | `PPU/vram_ctrl.md`, `PPU/cram.md` | `vram_ctrl.v` (`ReadBuffer`) |
| Control signals `/RD`, `/WR`, `ALE`, `DB/PAR`, `PD/RB`, `XRB`, `TSTEP` | `PPU/vram_ctrl.md` | `vram_ctrl.v`, `pads.v` |
| CPU-interface R/W decoder (`/RD_CPU`/`/WR_CPU`, `/DBE` gating) | `PPU/regs.md` §"R/W Декодер" | `regs.v` (`RWDecoder`), `ppu_top.v` (`n_CpuRD`/`n_CpuWR`) |
| OAM 256-byte view, byte meanings, 2112-cell array, decay, row/column decode | `PPU/oam.md` | `oam.v`, `Docs/PPU/OAM_RowCol.txt` |
| OAM attribute bits 2–4 absent; `$2004` read-back as 0 | `PPU/oam.md` (column skip) | — (the read-back value is NESdev `PPU_OAM` only) |
| OAM row-order NTSC/PAL difference, UMC 6538 write block | `PPU/oam.md`, `PPU/umc6538.md` | — |
| Cartridge pattern tables, CHR-ROM vs CHR-RAM | — (Docs/NESdev) | `Docs/NES_Famicom_Board_Revisions.md` §5.3 |
| CPU-side `$2000–$2007` semantics, `$3F00` workaround | `PPU/regs.md` | `regs.v` |

## Open questions

- **`AT_ADR` constant field: HDL defect and cross-spec notation.** `HDL/PPU/tilecnt.v` ties `AT_adr[6]`, `[7]`, `[8]` and `[9]` to `1'd0`, dropping the `$3C0` attribute-table offset; every attribute fetch would then read tile data instead of an attribute byte. In the schematic (`Logisim/Evolution/PPU_Evo.circ`, circuit `TileCounters_All`) those four adjacent bits share one net driven by a single bare `Constant` with no `value` attribute; the repository's documented convention is that such a Constant reads as Vdd = 1 (`HDL/PPU/pamux.v`, which relies on the same convention to un-freeze its latches, and other Constants in the file that do carry `value="0x0"`). This spec follows the schematic and NESdev, so §4 hard-wires `AT_ADR[9:6] = %1111`; the HDL needs those four bits corrected to 1 and an attribute-fetch test added.
- **Attribute-fetch notation across specs.** `04-ppu-rendering.md` §3 (`%10NN 1111 yyy xxx`, constant field at bits `[9:6]`) and `04-ppu-scrolling.md` §2 (`AT_ADR[9:6] = %1111`, "a constant field, not a function of `TVO`/`THO`") both agree with §4 here; the only remaining inconsistency is the `HDL/PPU/tilecnt.v` tie-off recorded in the previous item.
- **Mirroring and CIRAM A10.** No Breaking NES Wiki page describes nametable mirroring, CIRAM A10 or four-screen cartridges; §3 is assembled from NESdev `Mirroring` and the board/HDL material. A board- or die-level decode page would remove the rank-4 dependency.
- **`$3000–$3EFF` and `$3F20–$3FFF` mirrors: different ranks.** NESdev states both, but they do not have the same evidence. The `$3F20–$3FFF` mirror is rank 2: `HDL/PPU/cram.v` `CRAM_Block` takes a 5-bit `CGA` and indexes a 32-entry `cram[CGA]`, so any address in `$3F00–$3FFF` lands on `cram[v[4:0]]` and the mirror is structurally guaranteed (§6). The `$3000–$3EFF` mirror is **rank 4 only**: NESdev says `$3000-3EFF` is "usually" a mirror of `$2000-2EFF` and never mentions `A12`/`PA12`; no rank 1–3 source in this repository states the board decode that produces it — `Docs/NES/NES_HDL_Model/nes.v` has empty `CartPort`/`LS139` bodies and `HDL/PPU/pamux.v` only muxes the address. §1, §3 and the programming notes mark it accordingly; a board- or die-level decode page would remove the rank-4 dependency.
- **Attribute right-edge behaviour.** The exact dots at which the redundant right-edge fetches cross into the next nametable, and whether the last-row fetch ever uses the merged row-0/4 word line, are not documented in the Wiki.
- **OAM decay timings.** `PPU/oam.md` leaves the degradation timing as TBD; an emulator cannot reproduce decay without them, so model OAM as static for now.
- **OAM attribute bits 2–4 read-back.** The cell array has no storage for them (§8; rank 1 `PPU/oam.md` documents the column skip) and NESdev `PPU_OAM` states that they "always read back as 0" through OAMDATA, adding that bits which have decayed also read back as 0 (a side effect of the DRAM controller precharging the bus). No rank 1–3 source in this repository confirms the read-back value, so an emulator should follow NESdev and return 0.
- **CHR write decoding.** "Many cartridges only decode reads" is a board-level generalisation; this repository has no per-board table of which boards ignore PPU `/WR`.
- **Backdrop override (direct colour) gating.** `HDL/PPU/vram_ctrl.v` resolves the condition: `TH_MUX = ~(latched ~BLNK | nPA[0] | … | nPA[5])` with `.nPA(n_PA_out[13:8])` (`ppu_top.v`), i.e. `TH_MUX = BLNK & PA[13:8] = %111111` — any address in `$3F00–$3FFF` (read or write) while blanking is active, independent of `DB_PAR`. NESdev `PPU_palettes` describes the visible effect; the Wiki does not state the gating equation, but it is derivable from the HDL, so this item is settled and §6 reflects it.
