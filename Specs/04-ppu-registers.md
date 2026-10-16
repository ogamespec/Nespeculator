# 04-ppu-registers — PPU register file ($2000–$2007) and the internal v/t/x/w

**Status:** consensus (round 2)
**Scope:** The eight CPU-visible PPU registers `$2000–$2007`: bit fields, read and write side effects, per-revision differences, the shared first/second write latch, and how `$2005`/`$2006` writes load the internal scroll/address register `t`. The rendering-time use of `v`/`x`, the per-dot update schedule and mid-frame splits are in `04-ppu-scrolling.md`; the VRAM space reached through `$2006`/`$2007` is in `04-ppu-memory-map.md`; OAM and sprite evaluation are in `04-ppu-sprites.md`.
**Primary sources:** `BreakingNESWiki/PPU/regs.md`, `scroll_regs.md`, `oam.md`, `cram.md`, `vram_ctrl.md`, `par.md`, `fsm.md`, `mux.md`, `pads.md`, `rails.md`; `Docs/NES_Famicom_Board_Revisions.md`; HDL `HDL/PPU/regs.v`, `pads.v`, `scroll_regs.v`, `oam.v`, `vram_ctrl.v`, `ppu_top.v`; NESdev `PPU_registers`, `PPU_scrolling`, `PPU_power_up_state`, `Open_bus_behavior`, `PPU_OAM`, `PPU_variants`.

## 1. Register map

| CPU address | Name | Access | Bits | Main effect |
|---|---|---|---|---|
| `$2000` | PPUCTRL | W | `VPHB SINN` | NMI enable, pattern select, VRAM increment, nametable select |
| `$2001` | PPUMASK | W | `BGRs bMmG` | Rendering enable, left-column masking, greyscale, emphasis |
| `$2002` | PPUSTATUS | R | `VSOx xxxx` | Vblank / sprite 0 / overflow flags; read clears the write latch |
| `$2003` | OAMADDR | W | `AAAA AAAA` | OAM read/write address |
| `$2004` | OAMDATA | R/W | `DDDD DDDD` | OAM data; writes increment OAMADDR |
| `$2005` | PPUSCROLL | W×2 | `XXXX XXXX` / `YYYY YYYY` | X then Y scroll |
| `$2006` | PPUADDR | W×2 | `..AA AAAA AAAA AAAA` | VRAM address, high byte first |
| `$2007` | PPUDATA | R/W | `DDDD DDDD` | VRAM/palette data; auto-increments the address |
| `$4014` | OAMDMA | W (CPU) | `AAAA AAAA` | 256-byte CPU→OAM DMA (§12) |

The registers are mirrored every 8 bytes through `$3FFF` because `A3–A12` are not decoded, and all eight are reached through the same internal index 0–7, decoded from `RS0–RS2` (NESdev `PPU_registers`, `04-ppu-overview.md` §5). The CPU interface is **not** synchronised to `PCLK`, so every register can be written at any dot of any scanline (`PPU/regs.md`, "Специальное примечание"); writes to `$2000`, `$2001`, `$2005`, `$2006` are ignored during the reset window (`04-ppu-overview.md` §7).

## 2. PPUCTRL ($2000 write)

```
7  bit  0
---- ----
VPHB SINN
|||| ||||
|||| ||++- Base nametable address (0=$2000, 1=$2400, 2=$2800, 3=$2C00)
|||| |+--- VRAM address increment per $2007 access (0: +1, 1: +32)
|||| +---- Sprite pattern table for 8x8 sprites (0=$0000, 1=$1000; ignored in 8x16)
|||+------ Background pattern table (0=$0000, 1=$1000)
||+------- Sprite size (0: 8x8, 1: 8x16)
|+-------- PPU master/slave (0: read backdrop colour from EXT pins, 1: output colour on EXT pins)
+--------- Vblank NMI enable (0: off, 1: on)
```

| Bit | Internal signal | Effect |
|---|---|---|
| 0 | `NTH` | Nametable X; also `t[10]` |
| 1 | `NTV` | Nametable Y; also `t[11]` |
| 2 | `I1/32` | `$2007` increment 1 or 32 (PAL uses inverted `#I1/32`) |
| 3 | `OBSEL` | Sprite pattern table for 8x8 |
| 4 | `BGSEL` | Background pattern table |
| 5 | `O8/16` | Sprite size (PAL uses inverted `#O8/16`) |
| 6 | `/SLAVE` | 0 = master (input from EXT), 1 = slave (drive EXT) |
| 7 | `VBL` | Vblank NMI enable |

Sources: `PPU/rails.md`, `HDL/PPU/regs.v` `RegCTRL0`; the Wiki's official name is `CTLR0`.

- Bits 0–1 drive the **same latches** as `t[10]`/`t[11]` written by the first `$2006` write (`PPU/scroll_regs.md` §"Выбор Name Table"), so a `$2000` write changes the nametable part of the pending scroll/address and must be paired with `$2005`.
- Setting bit 7 while the `$2002[7]` flag is still set asserts `/INT` immediately: the NMI output is `VBL AND INT_FF` (`HDL/PPU/fsm.v` `VBlankInt`, `INT = NOR(~VBL_EN, ~int_ff_q)`). Read `$2002` first.
- A write that starts on dot 257 and continues through dot 258 (only that CPU/PPU
  alignment, only with a horizontal nametable arrangement / vertical mirroring,
  or 4-screen VRAM) writes the *open bus* value into PPUCTRL, so only the next
  scanline is drawn from the left nametable; horizontal and one-screen mirroring
  are immune because both nametables are identical. Workaround: write `$2000`
  (or its mirror at `$2100`, which selects the intended bit 0) with the desired
  nametable (NESdev `PPU_registers` §"Bit 0 race condition"; not covered by the
  Wiki).

## 3. PPUMASK ($2001 write)

```
7  bit  0
---- ----
BGRs bMmG
|||| ||||
|||| |||+- Greyscale
|||| ||+-- Show background in the leftmost 8 pixels
|||| |+--- Show sprites in the leftmost 8 pixels
|||| +---- Enable background rendering
|||+------ Enable sprite rendering
||+------- Emphasize red (green on PAL/Dendy)
|+-------- Emphasize green (red on PAL/Dendy)
+--------- Emphasize blue
```

| Bit | Internal signal | Effect |
|---|---|---|
| 0 | `B/W` | Greyscale: forces the colour into the grey column |
| 1 | `/BGCLIP` | 0 = clip the left 8 background pixels |
| 2 | `/OBCLIP` | 0 = clip the left 8 sprite pixels |
| 3 | `BGE` | Background rendering enable |
| 4 | `OBE` | Sprite rendering enable |
| 5–7 | `/TR`, `/TG`, `/TB` | Emphasis (tint) |

Sources: `PPU/rails.md`, `HDL/PPU/regs.v` `RegCTRL1`; the Wiki's name is `CTLR1`.

- `BLACK = ~(BGE | OBE)`: rendering counts as "off" only when both enables are 0 (`HDL/PPU/regs.v`). `BLACK` gates the whole rendering/VRAM-access schedule.
- Clipping chain: `/BGCLIP` → FSM → `CLIP_B` → `Clipper` → `/CLPB` → BGCOL, and `/OBCLIP` → FSM → `CLIP_O` → `Clipper` → `CLPO` → Obj FIFO (`PPU/regs.md`, `HDL/PPU/regs.v` `Clipper`).
- Red and green emphasis are swapped on PAL/Dendy (`Docs/NES_Famicom_Board_Revisions.md` §4; NESdev `PPU_registers`); RGB parts instead drive an emphasized channel fully on (`PPU/rgb.md`).

## 4. PPUSTATUS ($2002 read)

```
7  bit  0
---- ----
VSOx xxxx
|||| ||||
|||+-++++- PPU open bus (2C05 series: fixed identifier, §4.1)
||+------- Sprite overflow flag
|+-------- Sprite 0 hit flag
+--------- Vblank flag
```

| Bit | Signal | Set by | Cleared by |
|---|---|---|---|
| 7 | `INT_FF` (vblank) | `/VSET` at the start of vblank (scanline 241, dot 1) | `$2002` read, or `RESCL` (dot 1 of the pre-render scanline) |
| 6 | `STRIKE_FF` (sprite 0 hit) | sprite 0 opaque over opaque background | `RESCL` only |
| 5 | sprite overflow | buggy 9th-sprite check during sprite evaluation; false positives and negatives (see `04-ppu-sprites.md`) | `RESCL` only |
| 4–0 | DB open bus | — | — |

Sources: `PPU/fsm.md` §"Обработка прерывания VBlank", `PPU/mux.md` §"Sprite 0 Hit", `HDL/PPU/fsm.v` `VBlankInt`, `HDL/PPU/mux.v` `Spr0Hit`.

- Reading `$2002` clears the shared first/second write latch `w` used by `$2005`/`$2006` (`PPU/regs.md`): the standard way to force the next scroll or address write to be a "first" write.
- The same read clears the vblank flag (`R2_EN` resets the vblank FF in `HDL/PPU/fsm.v`) and therefore any pending NMI. Bits 6 and 5 are **not** cleared by the read.
- Reading the flag on the dot before it is set returns 0 and clears it, so polling can miss vblank; use NMI (NESdev `PPU_registers`).
- Bits 4–0 are undriven on a normal PPU and return the current DB (PPU I/O bus) value; a `$2002` read loads bits 7–5 from the flags and leaves bits 4–0 unchanged (NESdev `Open_bus_behavior`). `$2000`, `$2001`, `$2003`, `$2005` and `$2006` likewise have no read path, so reading any of them returns the capacitive DB value — PPU open bus, never `$00` (`PPU/Readme.md`, `PPU/pads.md`). NESdev adds that the latched value begins to decay after a frame or so, faster once the PPU is warm.

### 4.1 2C05 identifier

On the arcade 2C05-02/03/04 the low five bits return a constant instead of open bus; games check it as copy protection. The consumer 2C05-99 is normal.

| PPU | Mask | Expected |
|---|---|---|
| 2C05-02 | `$3F` | `$3D` |
| 2C05-03 | `$1F` | `$1C` |
| 2C05-04 | `$1F` | `$1B` |

Source: NESdev `PPU_registers` §"2C05 identifier" — not covered by the Breaking NES Wiki; see *Open questions*.

## 5. OAMADDR ($2003 write)

```
7  bit  0
---- ----
AAAA AAAA
```

- The written byte is the OAM read/write address (`PPU/oam.md`).
- During rendering OAMADDR is forced to 0 on dots 257–320 of every visible and pre-render scanline (NESdev `PPU_registers`).
- **2C02G bug:** a write to OAMADDR corrupts OAM. The symptom is that the 8-byte row at the target address is overwritten with the 8 bytes at `$20` (sprites 8–9); the copy source comes from the previous value on the CPU bus, which is usually `$20` because that is the `$2003` operand. Starting rendering with OAMADDR ≥ 8 similarly copies the bytes at `OAMADDR & $F8` over OAM row 0 (`Docs/NES_Famicom_Board_Revisions.md` §4; NESdev `PPU_registers` §"OAMADDR precautions"). NESdev adds that the exact corruption is not fully described, that other behaviours may exist, that it is **not known** whether the bug affects every 2C02 revision, and that it was required on the Dendy for 2C02 compatibility. The affected list is revision-dependent: `Errata` names the 2C02G, 2C03G and 2C02H, while `PPU_variants` has the 2C07 working correctly ("OAMADDR works correctly, not corrupting OAM contents") and does not name the 2C04/2C05.
- Practical rule: write `$00` to `$2003` and refresh all of OAM with OAMDMA.

## 6. OAMDATA ($2004 read/write)

```
7  bit  0
---- ----
DDDD DDDD
```

- Write: stores the byte into OAM at OAMADDR, then increments OAMADDR.
- Read: returns OAM[OAMADDR] through the `R4` latch; reading does **not** increment OAMADDR (`PPU/oam.md`, "Защёлка R4").
- Per revision (`Docs/NES_Famicom_Board_Revisions.md` §4; NESdev `PPU_registers` §OAMDATA, `PPU_variants`): OAMDATA is **not readable** on the oldest PPUs, i.e. 2C02E and all earlier NTSC parts, and on the 2C05 series (RC2C05-01…04 behave like the 2C03; RC2C05-99 has 2C02E behaviour); readability was added on the RP2C02G, found on most NESes and later Famicoms. The RGB 2C04 has no `/R4` register operation at all (`PPU/rgb.md`).
- Reads during vertical or forced blanking return OAM[OAMADDR]; reads **while rendering** instead expose whatever internal OAM access sprite evaluation and sprite loading are performing, which some games use deliberately (NESdev `PPU_registers`).
- Writes while rendering (pre-render line and visible lines 0–239, with either background or sprite rendering enabled) do **not** change OAM but do perform a glitchy increment that bumps only the high 6 bits of OAMADDR; OAMDMA is affected too because it is built from `$2004` writes. Emulating the increment as "no change" is the safer approximation (NESdev `PPU_registers`). The Wiki's OAM Buffer shows external writes enabled only while `BLNK` is active (`PPU/oam.md`). OAM is dynamic RAM and decays unless refreshed, which is why games keep a CPU-RAM copy and DMA it every vblank.

## 7. PPUSCROLL ($2005 write) and the shared write latch

`$2005` takes two writes. A single toggle (`w`) is shared with `$2006`: it remembers only the *order* of access, not which register was written, so a `$2005` write followed by a `$2006` write is treated as the second and first halves of the same pair (`PPU/regs.md`). It is cleared by `RC` (reset) and by a `$2002` read (`PPU/regs.md`, `HDL/PPU/regs.v` `SCCXFirstSecond`).

```
1st write        2nd write
7  bit  0        7  bit  0
---- ----        ---- ----
XXXX XXXX        YYYY YYYY
```

| Field (Wiki) | Storage | Data bits |
|---|---|---|
| Fine X `FH0–FH2` | `x` (3 bits, outside `t`) | 1st write `DB2–DB0` |
| Coarse X `TH0–TH4` | `t[4:0]` | 1st write `DB7–DB3` |
| Fine Y `FV0–FV2` | `t[14:12]` | 2nd write `DB2–DB0` |
| Coarse Y `TV0–TV4` | `t[9:5]` | 2nd write `DB7–DB3` |

Source: `PPU/scroll_regs.md`, confirmed bit-for-bit by `HDL/PPU/scroll_regs.v` (`Fine_H`, `Fine_V`, `Tile_H`, `Tile_V`). `FH` is the fine-X value applied to the background shifter; `TH`/`TV` are the coarse counters and `FV` the fine-Y counter. The NESdev names for the four fields are fine X (`x`), coarse X (`t[4:0]`), fine Y (`t[14:12]`) and coarse Y (`t[9:5]`) (NESdev `PPU_scrolling`).

Timing of mid-frame writes:

- The first `$2005` write updates fine X `x` immediately, but the coarse-X part of `t` is not copied into `v` until dot 257, so it must be written before hblank starts to affect the next scanline (NESdev `PPU_scrolling`).
- A `$2005` second write during a split only loads `t`; it is the `t → v` copy at dots 280–304 (vertical) and 257 (horizontal) that makes vertical scroll changes visible, so a vertical write made while rendering is active generally takes effect on the next frame (NESdev `PPU_registers`).
- A known-safe split trick: write the combined old-fine-X/new-coarse-X value to a `$2005` mirror (e.g. `$2105`) before dot 257, then the full new X to `$2005` after dot 257; the mirror's low 3 address bits select which fine-X value open bus leaves behind (NESdev `PPU_scrolling`).

## 8. PPUADDR ($2006 write)

`$2006` takes two writes, high byte first, tracked by the same `w` latch (`PPU/regs.md`, `PPU/scroll_regs.md`).

```
1st write        2nd write
15 bit  8        7  bit  0
---- ----        ---- ----
..AA AAAA        AAAA AAAA
```

The two writes load `t` as follows (all from `PPU/scroll_regs.md`, confirmed by `HDL/PPU/scroll_regs.v`):

| `t` field (Wiki) | `t` bits | 1st write (high) | 2nd write (low) |
|---|---|---|---|
| Coarse X `TH0–TH4` | 4–0 | — | `DB4–DB0` |
| Coarse Y `TV0–TV4` | 9–5 | `DB1–DB0` (t9–t8) | `DB7–DB5` (t7–t5) |
| Nametable X `NTH` | 10 | `DB2` | — |
| Nametable Y `NTV` | 11 | `DB3` | — |
| Fine Y `FV0–FV1` | 13–12 | `DB5–DB4` | — |
| Fine Y `FV2` | 14 | forced to `0` | — |
| (unused) | 15 | `DB7`, `DB6` ignored | — |

- Bits 14–15 of the address are therefore not writable: a `$2006` first write can set fine Y only to 0–3, because `FV2` (`t[14]`) is forced to 0 — the fine-Y counter is clamped so the address cannot leave the PPU address space (`PPU/scroll_regs.md`).
- **Palette corruption workaround.** After palette writes, finish by pointing the address at a palette mirror (`$3F00`, `$3F10`, `$3F20`, …) before leaving the palette range (NESdev `PPU_registers`):

  ```
  lda #$3F : sta PPUADDR   ; any palette mirror
  lda #$00 : sta PPUADDR
             sta PPUADDR   ; then any non-palette address
             sta PPUADDR
  ```

- If the second write lands on specific dots during rendering, one axis of `t` becomes the bitwise AND of the written and current value (NESdev `PPU_registers` §"Bus conflict"). The Wiki's PAR material (`PPU/par.md`) shows the same shared-bus coupling from the schematic side.

## 9. PPUDATA ($2007 read/write)

```
7  bit  0
---- ----
DDDD DDDD
```

- Access uses the current VRAM address `v` (the PAR pseudo-register), then increments `v` by 1 or 32 according to `$2000[2]` (`PPU/vram_ctrl.md`, `PPU/par.md`).
- **Read buffer.** A read returns the contents of the internal READ BUFFER and then loads `VRAM[v]` into it, so reads are delayed by one byte; the buffer is updated only by `$2007` reads (`PPU/vram_ctrl.md` §"Read Buffer", `HDL/PPU/vram_ctrl.v` `ReadBuffer`). Prime it with one dummy read after setting `$2006`.
- **Palette reads bypass the buffer.** For `$3F00–$3FFF` the palette entry is returned immediately through the Color Buffer, while a normal VRAM read at the same address still happens underneath and is what lands in the read buffer (`PPU/cram.md` signals `#CB/DB`/`#DB/CB`; NESdev `PPU_registers` §"Reading palette RAM"). Two details an implementer needs: the returned byte carries PPU open bus in bits 7–6, and it is returned **after** greyscale is applied (greyscale ANDs the colour with `$30`, preserving the underlying palette data). Palette-read support is revision-dependent: supported on the 2C02G, 2C02H and PAL parts; on 2C02E and earlier, and on the 2C03, 2C04 and the whole 2C05 series, the range behaves like ordinary VRAM (`PPU_variants`: RC2C05-01…04 behave like the 2C03, RC2C05-99 like the 2C02E). NESdev also reports that on some consoles palette reads are corrupted for one of the four CPU/PPU alignments, so software should not rely on them. The repository HDL returns `00` in bits 7–6 instead of open bus (`HDL/PPU/cram.v`), a divergence recorded in *Open questions*.
- `TH/MUX` forces the `TH` counter value onto the palette index path whenever the VRAM address is in `$3F00–$3FFF` and `BLNK` is active — "direct colour". `BLNK` is broader than "rendering disabled": it is also active during vblank, and the HDL condition is `BLNK & PA[13:8] = $3F` (`HDL/PPU/vram_ctrl.v` `TH_MUX`). Setting a `$3Fxx` address with `$2006` is enough to make the index appear (`PPU/mux.md`, `PPU/vram_ctrl.md`).
- Using `$2007` while rendering changes `v`, and therefore the rest of the frame's scroll; the access triggers a coarse-X increment **and** a Y increment simultaneously (with normal wrapping), regardless of the `$2000[2]` increment bit, so writes can land at unpredictable addresses. Keep accesses in vblank or forced blank and reload the scroll afterwards (`04-ppu-scrolling.md`).
- A DMC sample fetch colliding with a `$2007` read adds an extra read cycle, so the address is incremented twice and one byte is skipped (NESdev `PPU_registers` §"Read conflict with DPCM samples"; `02-cpu-dma.md`).

## 10. The internal registers v, t, x, w

**Dot numbering.** This spec uses the NESdev convention, dot = `H` = 0–340.
Normative labels: the vertical (fine-Y) increment and the last coarse-X increment
of the visible line at dot 256, the horizontal `t`→`v` copy at dot 257 (its effect
visible at 258), dummy fetches 337–340. The HDL `PCLK`-edge measurements in
`04-ppu-scrolling.md` §4 observe the same events one dot later (257/258).

| NESdev name | Breaking NES Wiki name / block | Size | Role |
|---|---|---|---|
| `t` | paired scroll registers (`FV`/`TH`/`TV`/`NT` latches) | 15 bits | Scroll/address preload; source for rendering copies and for `$2006` |
| `v` | PAR counters + tile counters (`THO`/`TVO`, `FV`/`NTV`/`NTH`) | 15 bits | Current VRAM address; used by `$2007` and by rendering |
| `x` | `FH` fine-H register | 3 bits | Fine X offset applied to the background shifter |
| `w` | first/second write latch | 1 bit | Alternates `$2005`/`$2006` halves |

`v` and `t` share the same layout (field sources: `PPU/scroll_regs.md`, `HDL/PPU/scroll_regs.v`; the timeline that mutates `v` is in `04-ppu-scrolling.md`):

| bits | Field | Written by | Mutated during rendering |
|---|---|---|---|
| 4–0 | Coarse X | `$2005` 1st, `$2006` 2nd | Coarse X increment at each tile fetch |
| 9–5 | Coarse Y | `$2005` 2nd, `$2006` 1st/2nd | Step on the fine-Y 7→0 wrap of the dot-256 increment; reload at the pre-render vertical copy (dots 280–304) |
| 10–11 | Nametable X/Y | `$2000` bits 0–1, `$2006` 1st | Copy at dot 257 (H) and dot 280 (V) |
| 14–12 | Fine Y | `$2005` 2nd all 3 bits; `$2006` 1st writes bits 12–13 only, bit 14 forced 0 | `+1` at dot 256, one step per rendered scanline; reloaded during dots 280–304 |
| 15 | unused | — | — |

`$2007` accesses are the only CPU-side operation that changes `v` directly; `t` changes only through `$2000`, `$2005` and `$2006` writes. The 14-bit address on the VRAM pins is the `PAT_ADR`/`PA` output built from these fields (`PPU/par.md`, `PPU/rails.md`).

## 11. Reset and power-on

| Item | At power-on | After reset |
|---|---|---|
| `$2000`, `$2001` | `$00` | `$00` |
| `$2002[7]` | random | `U??x xxxx` |
| `$2003` | `$00` | unchanged |
| `w` | cleared | cleared |
| fine X `x` | `$00` | `$00` |
| `t` | `$0000` | `$0000` |
| `v`, OAM, palette | unspecified | unchanged |
| `$2007` read buffer | `$00` | `$00` |

`RC` clears `$2000`, `$2001`, `t`, the fine-X register `x`, `w` and the `$2007` read buffer while `RC` is active (held until `RESCL`), and does **not** clear `v`, OAM or palette RAM (NESdev `PPU_power_up_state`; `HDL/PPU/scroll_regs.v` `Fine_H`). Writes to `$2000`, `$2001`, `$2005`, `$2006` are ignored while `RC` is active: about 29658 NTSC CPU cycles (≈33132 PAL CPU cycles) after reset, i.e. until the pre-render scanline of the next frame (NESdev `PPU_power_up_state`; the Wiki describes `RC` but not the ignored-write window — see *Open questions*).

## 12. OAMDMA ($4014, CPU side)

`$4014` is a CPU register, not a PPU register: writing a page number suspends the CPU and copies 256 bytes from CPU memory to OAM as 256 read/write pairs through `$2004`. Not counting the OAMDMA write tick this takes 513 CPU cycles, plus one more when the write landed on an odd CPU cycle (`02-cpu-dma.md`, NESdev `PPU_OAM`). Because every `$2004` write increments OAMADDR, set `$2003 = $00` first. On PAL/2C07 the copy must finish within the first 24 scanlines of vblank (~scanline 265), because sprite evaluation then starts and grabs OAM; on the Dendy UA6538 those writes are not blocked, so the same code can corrupt OAM there (NESdev `PPU_OAM`, `PPU_registers`).

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | A read of `$2000`/`$2001`/`$2003`/`$2005`/`$2006` returns the last byte written to any PPU port | Those registers have no read driver; DB floats with the last value | Implement the DB open-bus latch and return it for undriven reads |
| 2 | Setting the scroll after a `$2006` write silently shifts the screen | `w` is shared by `$2005`/`$2006` and remembers only the order of writes | Read `$2002` before every `$2005`/`$2006` sequence |
| 3 | A `$2002` read clears the vblank flag and cancels a pending NMI | The vblank latch is reset by the `$2002` read strobe | Read `$2002` before enabling NMI in `$2000[7]` |
| 4 | Sprite data written with `$2003`/`$2004` comes out shifted or duplicated on a 2C02G | OAMADDR writes corrupt OAM; the copy source is the previous bus value | Write `$2003 = $00` and use OAMDMA for the whole 256 bytes |
| 5 | OAMDATA reads return garbage on an early console | `$2004` is not readable on 2C02E and earlier; a 2C04 has no `/R4` | Gate OAM reads on the revision; do not use OAMDATA reads for collision |
| 6 | The first `$2007` read after setting `$2006` returns stale data | One-byte read-buffer delay | Perform one dummy read to prime the buffer |
| 7 | Palette reads return the colour immediately, with no priming read | Palette RAM bypasses the `$2007` read buffer; the returned byte carries open bus in bits 7–6 and has greyscale applied | Return the palette entry at once but still fill the read buffer from shadowed VRAM |
| 8 | `$2006` cannot set fine Y above 3 | `t[14]` (fine Y bit 2) is forced to 0 by the first `$2006` write | Set fine Y through the `$2005` second write when 4–7 is needed |
| 9 | Turning rendering off mid-screen with `v` inside `$3F00–$3FFF` corrupts palette RAM | `v` is shared with rendering and is left in the palette range when rendering stops (mechanism not in the Wiki; NESdev observation) | Point at a palette mirror, then move `v` outside the palette before disabling rendering |
| 10 | The 2C03/2C04/2C05 arcade boards need extra glue logic around the control registers | The 2C05 swaps `$2000`/`$2001` (`PPU_variants`); NESdev `Cycle_reference_chart` reports the boards "unswap" them | Detect the 2C05 via the `$2002` bits 4–0 identifier before writing control registers |
| 11 | Re-enabling NMI in vblank runs the handler immediately | NMI is `VBL AND INT_FF`; setting the enable with the flag set asserts `/INT` at once | Clear the vblank flag (`$2002` read) before setting `$2000[7]` |
| 12 | `$2007` reads skip a byte while DMC samples play | A DMC DMA fetch colliding with the read adds an extra increment | Accept the extra increment or avoid `$2007` reads during DMC playback |
| 13 | Register writes mid-scanline produce raster effects | The CPU interface is asynchronous to `PCLK` | Allow mid-scanline writes and their consequences |

## Programming / implementation notes

- MUST implement `$2000`, `$2001`, `$2003`, `$2005`, `$2006` as write-only: reading them returns the DB open-bus value.
- MUST implement one shared `w` latch for `$2005`/`$2006`, cleared by a `$2002` read and by reset.
- MUST apply the `$2005`/`$2006` field mapping of §7/§8, including fine Y bit 2 forced to 0 by a `$2006` first write and address bits 14–15 ignored.
- MUST clear the vblank flag (and pending NMI) when `$2002` is read, and MUST NOT clear the sprite 0 hit or sprite overflow flags from a read.
- MUST drive `$2002[7]` and `/INT` from the same vblank latch, and model the immediate NMI when `$2000[7]` is set while the flag is set.
- SHOULD model the sprite overflow flag as the buggy 9th-sprite check of `04-ppu-sprites.md`, not as a plain "> 8 sprites on the line" test: the flag produces false positives and false negatives.
- MUST implement the `$2007` one-byte read buffer and its palette bypass on revisions that support palette reads.
- MUST increment the VRAM address after every `$2007` access by 1 or 32 (`$2000[2]`); SHOULD also do so for the colliding DMC fetch.
- MUST increment OAMADDR after every `$2004` write and never after a read.
- SHOULD keep OAMADDR at `$00`, use OAMDMA for OAM updates, and model the 2C02G OAMADDR corruption only when a 2C02G is emulated.
- MUST reproduce the palette-corruption behaviour: turning rendering off with `v` in `$3F00–$3FFF` corrupts the palette entry at `v`.
- MUST make `RC` clear `$2000`, `$2001`, `t`, the fine-X register `x`, `w` and the `$2007` read buffer, but not `v`, OAM or palette RAM.
- MUST hold the reset-clear signal until the end of vblank, so that writes to `$2000`, `$2001`, `$2005` and `$2006` are ignored for the ~29658 NTSC (≈33132 PAL) CPU cycles after reset; `$2002`, `$2003`, `$2004`, `$2007` and OAMDMA must work immediately. The standard `bit PPUSTATUS`/`bpl` double wait loop passes the window.
- SHOULD finish OAM updates within the first 24 scanlines of PAL/2C07 vblank (before scanline 265), because sprite evaluation then starts and contends for OAM.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Register summary, bit fields, mirroring | `PPU_registers` |
| `$2005`/`$2006`/`v`/`t`/`x`/`w`, dot schedule, splits | `PPU_scrolling` |
| Power-on/reset values and the ignored-write window | `PPU_power_up_state` |
| PPU open bus and reads of write-only registers | `Open_bus_behavior` |
| OAM layout, `$2004`, `$4014` | `PPU_OAM` |
| OAM corruption, $2004 readability, per-revision differences | `PPU_registers` (§OAMADDR precautions, §OAMDATA), `Errata`, `PPU_variants` |
| Vblank flag race and NMI | `NMI`, `PPU_rendering` |
| 2C05 `$2000`/`$2001` swap on arcade boards | `PPU_variants`, `Cycle_reference_chart` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| `$2000`/`$2001` bit fields and internal signal names | `PPU/regs.md`, `PPU/rails.md`, `PPU/visual2c02.md` | `regs.v` (`RegCTRL0`, `RegCTRL1`) |
| Shared `$2005`/`$2006` first/second latch | `PPU/regs.md` | `regs.v` (`SCCXFirstSecond`) |
| R/W decoder and per-register strobes | `PPU/regs.md`, `PPU/rails.md` | `regs.v` (`RWDecoder`, `RegSelect`) |
| `$2005` → fine/coarse X/Y mapping | `PPU/scroll_regs.md` | `scroll_regs.v` |
| `$2006` → `t` mapping, fine Y bit 2 forced 0 | `PPU/scroll_regs.md` | `scroll_regs.v` (`Fine_V`) |
| Vblank flag, `$2002[7]`, read clears flag | `PPU/fsm.md` | `fsm.v` (`VBlankInt`) |
| Sprite 0 hit flag and `$2002[6]` | `PPU/mux.md` | `mux.v` (`Spr0Hit`) |
| `$2004` OAM read/write path, `R4` latch | `PPU/oam.md` | `oam.v` |
| `$2004` readability and OAMADDR corruption | — (Docs/NESdev) | — |
| `$2007` read buffer, `TH/MUX`, VRAM access | `PPU/vram_ctrl.md`, `PPU/par.md` | `vram_ctrl.v` (`ReadBuffer`) |
| Palette read bypass and direct colour | `PPU/cram.md`, `PPU/mux.md` | `vram_ctrl.v`, `cram.v` |
| Clipping chain `/BGCLIP`…`CLPO` | `PPU/regs.md`, `PPU/rails.md` | `regs.v` (`Clipper`) |
| Reset `RC` and power-on state | `PPU/pads.md`, `PPU/fsm.md` | `pads.v` |
| Mirroring, `$4014`, dot-257 race, palette workaround | — (NESdev) | — |

## Open questions

- **`RC` semantics and the ignored-write window.** NESdev states that writes to `$2000`, `$2001`, `$2005`, `$2006` are ignored for ~29658 (NTSC) / ≈33132 (PAL) CPU cycles after reset; `PPU/pads.md` implies registers stay cleared until `RESCL` (vblank end), while `HDL/PPU/pads.v` deliberately clears only during the `/RES` pulse and documents the change as a bug fix. NESdev `PPU_power_up_state` supports the held interpretation but describes a ~29658-cycle window rather than a register file wiped every frame; no Wiki page states that window and the HDL has no such gate — **unconfirmed** by ranks 1–2. Which is silicon, and where does `RC` end on the 2C02G?
- **OAMADDR corruption and `$2004` readability.** The Wiki has no page for `$2003` write corruption; it is documented only in `Docs/NES_Famicom_Board_Revisions.md` §4 and NESdev (source address unclear, "usually `$20`"), and NESdev adds that it is *not known* whether the bug is present in all 2C02 revisions, so key it on the 2C02G. Likewise neither `PPU/oam.md` nor `PPU/regs.md` states which revisions can read OAMDATA; the "added on 2C02G" claim rests on `Docs/NES_Famicom_Board_Revisions.md` §4 and NESdev (`PPU_variants` gives 2C02E and earlier plus the 2C05 series as unreadable), and the HDL models the `/R4` path unconditionally.
- **`$2005`/`$2006` bus conflict.** The exact dot windows in which the second `$2006` write is ANDed with the current value are known from NESdev only; `PPU/par.md` shows the shared-bus coupling but not the dot numbers.
- **Palette reads: reliability and bits 7–6.** NESdev reports corruption on one of the four CPU/PPU alignments depending on the console (no schematic finding explains it), and says the byte returned for a `$3F00–$3FFF` read carries PPU open bus in bits 7–6, whereas `HDL/PPU/cram.v` drives `{2'b00, cram[CGA]}`. An implementation must choose: model the NESdev open bus (accurate to the console) or follow the HDL's zeroes; neither is resolved by ranks 1–3.
- **2C05 register swap.** `PPU_variants` states the swap directly ("PPUCTRL and PPUMASK swap locations", RC2C05-01), so its existence is documented; no source, rank 1–3 or NESdev, states its polarity (which register answers which address). `Cycle_reference_chart` independently reports the arcade boards "use either the 2C03 or a 2C05 with glue logic to unswap `$2000` and `$2001`". The `$2002` bits 4–0 identifier is NESdev `PPU_registers` with the caveat that no data has been taken directly from the PPUs. Both are **unconfirmed** by ranks 1–3.
- **DMC/`$2007` read conflict.** The extra increment is NESdev-documented; `PPU/vram_ctrl.md` is short and does not cover CPU/DMA arbitration.
