# 04-ppu-scrolling — Scroll registers, `v`/`t`/`x`/`w` and per-dot scrolling

**Status:** consensus (round 3)
**Scope:** How the PPU positions the background: the Breaking NES Wiki paired
scroll registers (`FH`/`FV`/`TH`/`TV`/`NT`), their exact mapping to the NESdev
`v`/`t`/`x`/`w` model, the per-dot update schedule of the coarse/fine counters
during rendering, mid-frame split techniques and the register-access side
effects that disturb scrolling. The `$2000–$2007` CPU-side write sequences
themselves are in `04-ppu-registers.md` §7–§10 and are only summarised here;
VRAM layout is in `04-ppu-memory-map.md`; the fetch pipeline is in
`04-ppu-rendering.md`.
**Primary sources:** `BreakingNESWiki/PPU/scroll_regs.md`, `.../tilecnt.md`,
`.../pamux.md`, `.../par.md`, `.../hv.md`, `.../hv_decoder.md`, `.../fsm.md`,
`.../regs.md`, `.../dataread.md`, `.../vram_ctrl.md`; `HDL/PPU/scroll_regs.v`,
`par.v`, `tilecnt.v`, `pamux.v`, `hv.v`, `hv_decoder.v`, `fsm.v`, `regs.v`,
`vram_ctrl.v`.

## 1. The programmer's model: `t`, `v`, `x`, `w`

The NESdev names are used below; the Breaking NES Wiki name is given in
parentheses on first mention.

| Register | Breaking NES Wiki | Width | Role |
|---|---|---|---|
| `v` | tile counters (`TH`/`TV`/`NT`/`FV`, `tilecnt.md`) | 15 bits | The address the rendering pipeline actually walks |
| `t` | scroll registers (`FH`/`FV`/`TH`/`TV`/`NT`, `scroll_regs.md`) | 15 bits | Preload value written by `$2000`/`$2005`/`$2006` |
| `x` | `FH` (fine H scroll) | 3 bits | Sub-tile X offset, never changes during rendering |
| `w` | first/second write latch (`regs.md`) | 1 bit | Selects which half of `$2005`/`$2006` a write is |

```
15-bit t / v:
yyy NN YYYYY XXXXX
||| || ||||| +++++-- coarse X (5 bits)
||| || +++++-------- coarse Y (5 bits)
||| ++-------------- nametable select (NX, NY)
+++----------------- fine Y (3 bits)
```

Bit positions in the HDL: coarse X = `v[4:0]` (`TH`), coarse Y = `v[9:5]`
(`TV`), nametable X = `v[10]` (`NTH`), nametable Y = `v[11]` (`NTV`), fine Y =
`v[14:12]` (`FV`). Fine X (`FH`) lives outside `v` because it is applied to the
background shift register instead of the address (`HDL/PPU/bgcol.v`,
`BGC_0/BGC_1`: the colour tap is `disp[FH]`). `v` has 15 bits (NESdev
`PPU_scrolling`) but the PPU address space is 14 bits, so `v[14]` is never
driven on `/PA0-13`; the CPU-visible VRAM address is `v[13:0]`.

`t` and `v` are physically different circuits. `t` is the five banks of level
latches in `HDL/PPU/scroll_regs.v`; `v` is the tile-counter chain in
`HDL/PPU/tilecnt.v` (coarse X, coarse Y, name-table X/Y and fine Y). The
counters are *loaded* from the scroll latches and *stepped* by the rendering
timing signals; they are never loaded from `$2005` directly except through the
copy events of §4.

## 2. Where the scroll fields are used

| Field | Background | Sprites |
|---|---|---|
| coarse X `v[4:0]` | nametable address `NT_ADR[4:0]`, bits 0–1 also feed the attribute quadrant select | not used |
| coarse Y `v[9:5]` | nametable address `NT_ADR[9:5]`, bits 0–1 also feed the attribute quadrant select | not used |
| fine Y `v[14:12]` | pattern-row bits `PAT_ADR[2:0]` | replaced by `OV[2:0]` (row within the sprite), inverted when the vertical-flip attribute is set |
| nametable X/Y `v[11:10]` | `NT_ADR[11:10]` | not used |
| fine X `FH` | selects the output bit of the background shift register | not used |

The attribute table address is built independently of the sprite/background
path (`04-ppu-memory-map.md` §4; `HDL/PPU/tilecnt.v`, `TileCnt`):

```
AT_ADR[2:0]   = THO[4:2]      coarse X / 4
AT_ADR[5:3]   = TVO[4:2]      coarse Y / 4
AT_ADR[9:6]   = %1111         constant $3C0 attribute-table offset
AT_ADR[11:10] = NTH, NTV      nametable select
AT_ADR[13:12] = %10           constant while rendering
```

Like the NESdev formula it must satisfy (NESdev `PPU_scrolling` §Tile and
attribute fetching), `AT_ADR[9:6]` is a constant field, not a function of
`TVO`/`THO`:
`attribute address = $23C0 | (v & $0C00) | ((v >> 4) & $38) | ((v >> 2) & $07)`;
`AT_ADR[5:3]`/`[2:0]` give the coarse-Y/coarse-X quotients, and the nametable
formula is `tile address = $2000 | (v & $0FFF)`. `HDL/PPU/tilecnt.v` currently
ties `AT_adr[6..9]` to `1'd0` instead of the schematic's Vdd tie — an HDL
defect documented in `04-ppu-memory-map.md` Open questions, not the spec.
`AT_ADR` and `NT_ADR` are fed to the PPU address multiplexer (PAMUX),
which also takes the pattern address `PAT_ADR` from the Picture Address
Register (PAR) and the CPU data bus for `$2007` accesses (`PPU/pamux.md`).
`v[11:10]` selects one of the four logical nametables; which physical VRAM it
lands in is decided by the cartridge's CIRAM A10 select, not by the PPU
(NESdev `Mirroring`, `PPU_nametables`; board wiring in
`04-ppu-memory-map.md` §3).

## 3. Writing `t`: the `$2005`/`$2006`/`$2000` sequences

`$2005` and `$2006` share one toggle `w`; it records only the *order* of
accesses, not which register was written, and is cleared by a `$2002` read or
by reset (`PPU/regs.md`, `HDL/PPU/regs.v` `SCCXFirstSecond`). A `$2005` first
write followed by a `$2006` write is therefore the second half of the same
pair. The field mapping (from the tables in `PPU/scroll_regs.md`, confirmed
bit-for-bit in `HDL/PPU/scroll_regs.v`) is:

| Write | `FH` | `FV` | `TH` | `TV` | `NT` |
|---|---|---|---|---|---|
| `$2005` 1st (X) | `DB[2:0]` | – | `DB[7:3]` | – | – |
| `$2005` 2nd (Y) | – | `DB[2:0]` | – | `DB[7:3]` | – |
| `$2006` 1st (addr high) | – | `DB[5:4]`, bit 2 forced `0` | – | `DB[1:0]` | `DB[3:2]` |
| `$2006` 2nd (addr low) | – | – | `DB[4:0]` | `DB[7:5]` | – |
| `$2000` (`PPUCTRL`) | – | – | – | – | `DB[1:0]` |

Three consequences that programs rely on:

- **The second `$2006` write copies the whole of `t` into `v` immediately**
  (`HDL/PPU/tilecnt.v`: the `W6_2_Ena` pulse loads `TH`/`TV` from `THx`/`TVx`;
  `NT` and `FV` follow from the same write's other bits) — the mechanism behind
  every mid-frame `$2006`-based scroll change.
- **The first `$2006` write cannot set fine Y above 3; the second `$2005`
  write sets all three bits.** On the `$2006` first-write path `FV[0]`/`FV[1]`
  come from `DB[4]`/`DB[5]` and `FV[2]` (`v[14]`) is forced to `0`
  (`HDL/PPU/scroll_regs.v` `Fine_V`, `W6_1` path only), keeping the address in
  the 14-bit PPU space (`PPU/scroll_regs.md`). The second `$2005` write takes
  `DB[2:0]` → fine Y 0–7 (`t: FGH..AB CDE..... <- d: ABCDEFGH`, NESdev
  `PPU_scrolling` §Register controls). The counter is not clamped: it counts
  0–7, and §4's measurement starts from fine Y 5.
- **`$2000`, `$2005` and `$2006` write the *same* `NT` latches.** `NTH`/`NTV`
  are loaded either from `$2000` bits 0/1 or from the first `$2006` write's
  bits 2/3 (`scroll_regs.v`, `NT_Select`). `$2006` therefore overwrites the
  nametable part of the pending scroll, and the scroll must be re-written
  after VRAM updates.

## 4. The per-dot update schedule during rendering

**Dot numbering.** This spec uses the NESdev convention, dot = `H` = 0–340.
Normative labels: the vertical (fine-Y) increment and the last coarse-X increment
of the visible line at dot 256, the horizontal `t`→`v` copy at dot 257 (its effect
visible at 258), dummy fetches 337–340. The HDL `PCLK`-edge measurements in §4
observe the same events one dot later (257/258); the `(HDL edge)` label below
marks only the cells and transcripts that carry the measured values, not the
nominal fetch dots of the first four rows. "Rendering enabled" means at least one
of `$2001` bits 3/4 is set; with rendering disabled (`BLNK` active,
`PPU/fsm.md` `SC_CNT = ~(HBLANK | BLACK)`) the counters do not step, which is
what makes `$2007` addressing linear. The transitions quoted below were measured
by simulating `PixelClock + HVCounters + HVDecoder + PPU_FSM + TileCnt` with the
same wiring as `ppu_top.v` (Icarus Verilog, `-D RP2C02`).

| Dot(s) | Event | `t` → `v` field |
|---|---|---|
| 1–2, 9–10, … , 249–250 | nametable byte fetch | – |
| 3–4, 11–12, … , 251–252 | attribute byte fetch | – |
| 5–6, 13–14, … , 253–254 | pattern low fetch | – |
| 7–8, 15–16, … , 255–256 | pattern high fetch | – |
| 9, 17, 25, … , 249, 257 | coarse X increment, one per 8-dot tile period (32 per line); the dot-257 one is the last of the visible-region tile periods (HDL edge; dot 256) | `v[4:0] += 1`, wrap 31→0 toggles `v[10]` |
| 257 (HDL edge; nominal 256) | vertical increment, on the same edge as the last coarse X increment; **one fine-Y step per scanline** | fine Y `+1`; the coarse Y step of §5 only on the fine-Y 7→0 wrap |
| 258 | horizontal copy | `v[10:0]` horizontal part ← `t` (coarse X, nametable X) |
| 258–320 | eight sprite fetch windows ("garbage" name table, pattern low/high; `OBJ_READ`) | – |
| 321–336 | two-tile prefetch of the next scanline | – |
| 329, 337 | two extra coarse X increments | `v[4:0] += 1` |
| 280–304 (pre-render line only) | vertical copy, repeated | coarse Y, nametable Y, fine Y ← `t` |
| 337–340 | two dummy nametable fetches (same byte as the next line's first fetch; nominal dots 337–340) | one extra coarse X increment |

The measured transitions on one NTSC scanline (coarse X = 5, coarse Y = 28,
fine Y = 5, NX = 1, NY = 0 at dot 0) were:

```
dot   9: X 5->6      dot 217: X 31->0, NX 1->0     dot 329: X 3->4
dot  17: X 6->7      dot 257: X 4->5, fineY 5->4   dot 337: X 4->5   (HDL edge)
...                  dot 258: X 5->3 (copy), NX -> 1
dot 249: X 3->4
```

and on the pre-render line of the same frame (coarse X = 5, coarse Y = 25,
fine Y = 1, NX = 1, NY = 1):

```
dot 257: fineY 1->0 (HDL edge)   dot 281: fineY 0->1, NY 1->0 (vertical copy)
dot 258: X 5->3, NX -> 1  (HDL edge)
```

Four details differ from a naive "increment every 8 dots" model:

- The vertical increment is **once per scanline**, at nominal dot 256 (measured
  at the dot-257 HDL edge), and it is a *fine-Y* step: coarse Y advances only on
  the fine-Y 7→0 wrap of §5. The dots 321–336 prefetch does **not** step the
  vertical position — it loads the next line's first two tiles into the shifters
  at the position already advanced at dot 256 (NESdev `PPU_scrolling` §Y
  increment, "fine Y is incremented at dot 256 of each scanline"; NESdev
  `PPU_rendering` §Cycles 321–336, "the first two tiles for the *next*
  scanline"). An extra step there would fetch every line one row too low.
- The dummy fetches at dots 337–340 add only **one** further coarse X
  increment, at dot 337; the next increment in the 8-dot chain would fall at
  dot 4 of the next line (dot 345), but that line restarts its fetches, so the
  next increment is at its dot 9. At dot 337 `THSTEP =
  ~(~(TSTEP | (F_TB & H0_DD)) | PCLK)` opens because `F_TB` is high at an
  even-H (`H0_DD`) edge inside the dummy window: exactly one increment.
- The counters always walk coarse X, fine Y, coarse Y and the nametable bits;
  while rendering is disabled they hold the `$2006`-loaded value.

## 5. Coarse X and Y wrapping

```
coarse X increment:                       Y increment (fine Y wraps 7 -> 0):
  if v[4:0] == 31:                          if coarse Y == 29: coarse Y = 0, v[11] ^= 1
      v[4:0] = 0                            else if coarse Y == 31: coarse Y = 0
      v[10] ^= 1                            else coarse Y += 1
  else v[4:0] += 1
```

The coarse-Y counter has one stepping sequence and one load-only sequence.
Stepping (rendering enabled): fine Y wraps 7→0, the carry `TVIN` steps the
chain, and a step from 29 goes to 0 with `NTV` toggled — NESdev's row-29 wrap,
not a divergence. `Z_TV` (`TileCountersControl2`) decodes `TVO == 29` while
rendering and a vertical step is pending, and the `TileCounterBitReset` cells
in `tilecnt.v` gate `val_out` with `~Reset`, so the chain is cleared to 0 on the
step out of 29: starting at coarse Y 25 the measurement produced 25, 26, …,
29, 0 with `NTV` toggling, and `tilecnt_test.v` check 3 models it.
Load-only: a `$2005` second write with Y = 240–255 puts 30 or 31 into the
coarse-Y part of `t`, which the next vertical copy loads into the counter
through `TVLOAD`; `Z_TV` does not fire for those values, so they persist until
the next vertical step, which runs 31 → 0 with the nametable *not* toggled and
the PPU reading the attribute table as tile data ("negative" scroll) (NESdev
`PPU_scrolling` §Wrapping around). The step path out of 29 always clears to 0,
so 30/31 appear only through the `TVLOAD` load above.
`PPU/tilecnt.md` gives the count limit as 30 and the reset source as "carry
TVZ + 1 TVSTEP"; that wording is the unresolved part — see *Open questions*.

## 6. Accessing PPU registers during rendering

The CPU interface is **not** synchronised to `PCLK`, so a program may write any
register at any dot (`PPU/regs.md`, "Специальное примечание"). The observable
consequences for scrolling:

| Access | Effect on scrolling |
|---|---|
| `$2005` first write (X) | fine X `FH` changes immediately and affects the current scanline; coarse X takes effect at the horizontal copy (measured dot 258, HDL edge), i.e. on the next scanline |
| `$2005` second write (Y) | fine Y and coarse Y in `t` change immediately, but the counters are only reloaded at the pre-render line's vertical copy, so the change applies to the **next frame** |
| `$2000` write | nametable bits of `t` change immediately; applied at the next horizontal copy, or at the vertical copy for `NTV` |
| `$2006` second write | the whole of `t` is copied into `v` immediately, mid-scanline |
| `$2007` read/write during rendering | the VRAM port is busy with fetches, and the access corrupts the live scroll/address: NESdev `PPU_scrolling` documents a coarse X increment **and** a Y increment together. `PPU/vram_ctrl.md` describes the coupling in prose (`TSTEP` "allows the TV/TH counters to perform an increment", and `TSTEP`/`E_EV` gate `THSTEP`/`TVSTEP` in `tilecnt.v`), but the generated HDL does not reproduce the `$2007`-driven pulse — see *Open questions* |
| `$2002` read | clears `w`; does not touch `v`/`t`/`x` |

Two documented hardware races:

- **`$2006` second-write vs. the background pipeline.** A mid-scanline second
  `$2006` write reloads `v` at once, but the visible effect is delayed by one
  to two tiles because the pattern/attribute shift registers are already
  loaded (NESdev `PPU_rendering`, "Tile and attribute fetching"). The write
  can also be ANDed with the current `v` value on one axis: a second write
  made while rendering is active can collide with the PPU's own register
  access (NESdev `PPU_registers` §"Bus conflict"); the only safe time to
  finish the second write is blanking.
- **`$2000` bit 0 across the copy edge (the dot-257 copy).** Because bit 0
  targets the same nametable latch as the horizontal copy, a write that
  straddles the dot-257→258 edge can load open bus into `NTH`, drawing the next
  scanline from the wrong nametable (NESdev `PPU_registers` §"Bit 0 race
  condition"). This matters with horizontal mirroring / four-screen VRAM; with
  vertical or one-screen mirroring both nametables are the same.

## 7. Mid-frame splits

The PPU offers two mechanisms, because horizontal and vertical scroll update
at different moments:

- **X split (cheap):** write `$2005` (and `$2000` if the nametable changes)
  during the scanline, before the dot-257 copy edge. The coarse X/name-table copy
  (dot 257, effect at 258) picks up the new value for the *next* scanline; fine X
  applies to the current one. Landing the write late in the line leaves a one- or two-tile
  glitch, which is normally hidden with a solid status bar or a sprite.
- **Full X/Y split:** four writes in the order `$2006`, `$2005` (Y), `$2005`
  (X), `$2006` (low byte). The second `$2006` write reloads all of `v`
  immediately, including the vertical bits, so this need not wait for the next
  frame. Do the last two writes in horizontal blanking; if they land before the
  dot-256 increment edge (measured 257, HDL edge), set Y to the intended value
  minus one (the vertical increment there happens right after).

After any VRAM access through `$2006`/`$2007`, the scroll must be restored:
`t` and `v` share the address fields, so a `$2006`-based VRAM update destroys
the pending scroll.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | Screen jumps sideways after a nametable update | `$2006` writes the same `NT` latches as `$2000`/`$2005` | Re-write `$2000` bits 0–1 and both `$2005` writes after every VRAM update, before rendering resumes |
| 2 | The first `$2005` after a `$2006` scrolls wrongly | One shared `w` toggle for `$2005`/`$2006` | Read `$2002` before each scroll sequence to force the next write to be a "first" write |
| 3 | Vertical split in mid-screen works, horizontal split needs a different write | Coarse X/name-table X copy at dot 257 of *every* scanline (visible at 258); vertical bits reload only at the pre-render line | For X use `$2005`+`$2000` before dot 257; for Y use the four-write `$2006` sequence so `v` is reloaded at once |
| 4 | Status-bar split appears one or two tiles late | `$2006` reloads `v` immediately, but the shift registers for the current two tiles are already loaded | Accept the 1–2 tile delay or place the split in blanking |
| 5 | One scanline drawn from the wrong nametable | `$2000` bit 0 write straddling the dot-257 copy edge (visible at 258) loads open bus into `NTH` | Do not write `$2000` across the copy edge; arrange horizontal mirroring or mark the line with a blank/sprite |
| 6 | Reading `$2007` during rendering wrecks the picture | The VRAM port is busy with fetches; the access increments the live counters | Never touch `$2007` while rendering is enabled; use vblank |
| 7 | Fine Y written mid-frame only shows up next frame | Vertical bits are copied from `t` to `v` only in the pre-render line's dots 280–304 | Apply vertical scroll changes in vblank; or use the `$2006` second write for an immediate reload |
| 8 | `$2006` cannot scroll to fine Y 4–7 | Only the first `$2006` write forces `FV[2]` to 0; the second `$2005` write sets all three fine-Y bits | Set fine Y through the `$2005` second write |
| 9 | Coarse Y beyond 239 shows attribute data as tiles | `$2005` Y = 240–255 loads coarse Y 30/31; the counter wraps 31→0 without switching nametable | Treat 240–255 as "negative" scroll and expect one or two rows of attribute garbage |
| 10 | Rendering the left column after scrolling leaves a seam | Coarse X increments at dot 9 of the line, and the first two tiles come from the previous line's prefetch | Use `$2001` bit 1 (background left-column mask) to hide the seam, or keep the seam off-screen |
| 11 | Mapper IRQ counters drift by one dot | The last dummy nametable fetch of a line produces a coarse X increment at dot 337, and the next line's first at dot 9 | Model the dot-337 increment; MMC5 uses the three consecutive nametable fetches at the pre-render/visible line boundary |

## Programming / implementation notes

- MUST implement `t`, `v`, `x` and `w` as separate state; `$2005`/`$2006`
  write `t` (except the second `$2006`, which also copies `t` → `v`), and only
  the copy events of §4 move data from `t` into `v`.
- MUST implement the shared `w` toggle, cleared by `$2002` reads and reset.
- MUST use the field mapping of §3: the first `$2006` write forces `FV[2] = 0`
  (fine Y 0–3), the second `$2005` write writes `FV[2:0]` (fine Y 0–7), and
  `NTH`/`NTV` are shared latches between `$2000`, `$2005` and `$2006`.
- MUST increment coarse X at the dots listed in §4 — 9, 17, …, 249, 257 (the
  32 tile-period increment dots) and 329, 337 — when rendering is enabled,
  with the 31→0 wrap toggling nametable X.
- MUST increment the vertical position once per rendered scanline, at the dot
  256 edge (measured at 257, HDL edge): one fine-Y step, with the fine-Y 7→0
  wrap stepping coarse Y and the 29→0 wrap toggling nametable Y. The dots
  321–336 prefetch fires no vertical step (NESdev `PPU_scrolling` §Y increment).
  (`04-ppu-rendering.md` §3, `04-ppu-memory-map.md` §5.)
- MUST copy the horizontal bits of `t` into `v` at the dot-257 edge of every
  rendered scanline (effect visible at 258) and the vertical bits during dots
  280–304 of the pre-render line. `04-ppu-registers.md` §7 quotes the same
  events with the same dot numbers.
- SHOULD treat all writes to `$2000`/`$2001`/`$2005`/`$2006` as taking effect
  on the CPU cycle they occur (no CPU-side synchronisation), and implement the
  `$2000` bit 0 race at the dot-257 copy edge and the `$2006` second-write
  bus conflict if the emulator targets raster-effect-heavy software.
- SHOULD keep the scroll counters frozen while rendering is disabled, so that
  `$2007` addressing is the linear 1/32 increment; while rendering is enabled a
  `$2007` access MUST use the counter path instead (coarse X **and** Y increment
  together, `$2000[2]` ignored — NESdev `PPU_scrolling`; not reproduced by the
  generated HDL, see *Open questions*).
- MUST preserve the shared `w` across a mid-frame split: the four writes
  `$2006`, `$2005` (Y), `$2005` (X), `$2006` must be issued in that order with
  `w` starting at "first write" (read `$2002` first), otherwise each write lands
  in the wrong half of its pair.
- MAY implement the dummy-fetch increments (dots 337–340) as a single
  increment at dot 337 if only mapper IRQ timing (not fetch addresses) is
  affected.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| `v`/`t`/`x`/`w` register names, sizes and roles | `PPU_scrolling` §PPU internal registers |
| `$2005` first/second, `$2006` high/low and `$2000` bit 0–1 write sequences | `PPU_scrolling` §Register controls, `PPU_registers` |
| Per-dot schedule, prefetch, dummy fetches | `PPU_rendering` |
| Coarse X and coarse Y wrapping rules | `PPU_scrolling` §Wrapping around |
| Nametable/attribute address formulas and the seam | `PPU_scrolling` §Tile and attribute fetching, `PPU_nametables`, `PPU_attribute_tables` |
| Nametable arrangement / mirroring and the CIRAM A10 select | `Mirroring`, `PPU_nametables` |
| Mid-frame scroll splits | `PPU_scrolling` §Split X scroll / §Split X/Y scroll |
| `$2000` bit 0 race | `PPU_registers` §Bit 0 race condition |
| `$2006` second-write bus conflict | `PPU_registers` §Bus conflict |
| `$2007` accesses during rendering | `PPU_scrolling` §$2007 reads and writes |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| `FH`/`FV`/`TH`/`TV`/`NT` layouts and write tables | `PPU/scroll_regs.md` | `scroll_regs.v` (`Fine_H`, `Fine_V`, `NT_Select`, `Tile_H`, `Tile_V`) |
| Shared first/second write latch, `RC`/`$2002` clear | `PPU/regs.md` | `regs.v` (`SCCXFirstSecond`, `RegSelect`) |
| Tile counter modes, count limits, carry/reset sources | `PPU/tilecnt.md` | `tilecnt.v` (`TileCountersControl`, `TileCountersControl2`, `Tile_TH_Counter`, `Tile_TV_Counter`, `Tile_FV_Counter`, `Tile_NT_Counters`) |
| `AT_ADR`/`NT_ADR` composition and PAMUX source selection | `PPU/pamux.md` | `pamux.v`, `tilecnt.v` |
| Pattern-row and sprite/background selection in the pattern address | `PPU/par.md` | `par.v` |
| H/V counter limits (0–340, 0–261 NTSC) and clear/step signals | `PPU/hv.md`, `PPU/fsm.md` | `hv.v`, `fsm.v` (`HVCounterControl`) |
| Per-dot control signals (`SC_CNT`, `F_AT`, `F_TA`, `F_TB`, `#F/NT`, `/FO`, `OBJ_READ`) and the dot numbers | `PPU/hv_decoder.md` (HPLA table), `PPU/fsm.md` | `hv_decoder.v`, `fsm.v` (`HPosLogic`) |
| `TSTEP` driving the TV/TH counters, `DB/PAR`, read buffer | `PPU/vram_ctrl.md` | `vram_ctrl.v` |
| Fine X applied to the background shift register | `PPU/dataread.md` | `bgcol.v` (`BGC_0`/`BGC_1`, tap `disp[FH]`) |

The dot numbers in §4 were confirmed by simulation of the HDL (see §4); no
NESdev page was used to establish them. Elsewhere NESdev is cited only for
naming, layout and community-observed behaviour: §2 (naming, the address
formulas), §3–§6 (write mappings, dot labels, wrapping rules, register races).
NESdev `PPU_variants` documents no scroll-register differences
(`04-ppu-registers.md` already carries the revision-dependent behaviour), so it
is not cited here.

## Open questions

- **Coarse-Y 30/31 and the `PPU/tilecnt.md` count limit.** §5 gives one rule:
  the step path wraps 29 → 0 with the nametable toggle (measured; modelled by
  `tilecnt_test.v`), while 30/31 exist only after a `$2005` Y load of 240–255.
  `PPU/tilecnt.md` gives the Tile Vertical limit as 30 and the reset source as
  "carry TVZ + 1 TVSTEP"; read literally that stops the counter at 30, which
  the HDL and the `$2005` mapping (coarse Y = `DB[7:3]`) do not support.
  Whether silicon can *hold* coarse Y = 31 for a full scanline is not settled
  by ranks 1–2; emulators follow the NESdev rule either way.
- **`$2007` during rendering.** NESdev documents a simultaneous coarse X and Y
  increment; `PPU/vram_ctrl.md` describes the same coupling in prose, but the
  generated `tilecnt.v` produces only the per-scanline step, so the affected
  dot windows come from NESdev only.
- **Fine-Y decrement on the pre-render line.** The measurement shows fine Y
  dropping between dots 256 and 257 of the pre-render line (1 → 0) before the
  dot-281 reload; NESdev documents only the reload. The `TVSTEP` gate in
  `tilecnt.v` has two mutually-driving `assign` statements, so this is likely a
  simulator artefact with no program-visible consequence.
- **Dot numbering.** The four PPU spec files now use the single convention of the
  *Dot numbering* note in §4: the vertical and last coarse-X increments at dot
  256, the horizontal copy at the dot-257 edge (effect at 258). §4's own two
  measured transitions are `(HDL edge)`, i.e. one dot later (257/258); whether
  the HDL's `dlatch` phase also shifts the other `HPLA`-derived windows is a
  separate open item in `04-ppu-rendering.md`.
- **`04-ppu-memory-map.md` cross-references.** Its §3 still says "Coarse Y =
  30/31 do not wrap" (meaning: 30/31 do not trigger the 29→0 nametable toggle);
  its Open-questions `AT_ADR` request against this file's §2 is resolved — the
  sibling now agrees with §2.
- **Sprite pipeline and the dot-258 copy.** Whether the horizontal copy and the
  sprite pipeline can skew by more than one dot (affecting sprite 0 hit on the
  following line) was not verified.
