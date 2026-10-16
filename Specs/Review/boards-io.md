# Review: boards-io — round 3

File under review: `Specs/05-boards-and-io.md` (Status: revised (round 3), 400 lines).
Round 2 (below) returned `revise` with 1 major and 3 minor findings; the Revision round 3
note is at the foot of this file. This is the final round.

Baseline limitation: `Specs/` is untracked in this checkout (`git status` → `?? Specs/`), so
"not altered" below means the current text was re-verified against the sources, not
mechanically diffed.

## Verdict
consensus

## Blocking
None.

## Major
None. Round-2 major 1 (the Open-questions `04-ppu-timing.md` §10 claim) is resolved — see
*Resolved from round 2*.

## Minor (residual, non-blocking)
1. **§1 line 50 applies the `÷4` PCLK divider to the 2C07 as well as the 2C02.** Quote:
   "The `PCLK` above is the 2C02/2C07 dot clock (the PPU `CLK` input ÷4, `PPU/pclk.md`)".
   That is correct for the RP2C02 (21.477272 ÷ 4 = 5.369318 MHz) but not for the RP2C07, whose
   dot clock is `CLK ÷ 5` (26.601712 ÷ 5 = 5.3203424 MHz) — as this file's own §1 table says on
   line 44 (`÷5`), as `01-platform-overview.md` §2 line 45 says, as `04-ppu-timing.md` §1 line 21
   says (`RP2C07 | 26.601712 MHz | ÷5`), and as the cited page's own timing table shows
   (`PPU/pclk.md`: `PAL | 26601712 Hz | … | 5320342.4 Hz`) — even though that page's prose
   (line 9) names the ÷4 only with the 21.48 MHz NTSC clock. Classified minor, not major: the
   authoritative per-region dividers are correct in the §1 table two lines above and in
   `01`/`04`; only this redundant prose parenthetical is region-generic. A line-neutral fix
   would be "the PPU `CLK` input (NTSC ÷4, PAL/Dendy ÷5, `PPU/pclk.md`)".
2. **`PPU/pclk.md` is cited inline in §1 but is not in the metadata `Primary sources` block
   (lines 11–16) nor in the `## Provenance` "region clocks" row (line 341, whose wiki column
   names `Docs/NES_Famicom_Board_Revisions.md` / `MB/{NES,Famicom}.md` and whose HDL column
   names `HDL/PPU/pclk.v`).** Conventions §7 is satisfied by the inline citation, so this is a
   completeness nit only, and the file is at the 400-line ceiling — no action needed this round.

## Resolved from round 2
- Major 1 — **fixed and true.** The Open-questions bullet (lines 363–366) now reads
  "**Cross-file: `01-platform-overview.md` is stale about §1 and `04` §10.** `04-ppu-timing.md`
  §10 now agrees with §1 here and `01` §2 (Dendy `1.773448 MHz`, `3:1`, PPU dots per CPU cycle).
  But `01` line 52 still calls §1's table "older", and its cross-file bullet (~line 331)
  repeats `1:3`/`5:16`/`186.2 ns` and `04`'s old `1 : ≈2.973`; the fix belongs to `01`."
  `04-ppu-timing.md` (`revised (round 2)`) §10 line 246 now gives Dendy
  `1.773448 MHz (÷15) | 5.3203424 MHz (÷5) | 3 : 1 | 3 | 341/3 = 113⅔`, and lines 248–249 state
  that the column is "**PPU dots per CPU cycle** … the convention of `01-platform-overview.md`
  §2 — **not** `5:16`". So it really does agree with `05` §1 line 45 (`÷15`,
  `1.773448 MHz (563.9 ns)`, `5.3203424 MHz (187.96 ns)`, `3:1`) and with `01` §2 line 46. The
  change is `04`'s own documented revision (`Specs/Review/ppu-palette-timing.md`,
  `## Revision round 2`: "Major 2: fixed — §10 Dendy row is now `1.773448 MHz (÷15) |
  5.3203424 MHz | 3 : 1 | 3 | 341/3 = 113⅔`"), not a round-3 edit inside `05`. The false
  "the values agree / presentation differs" sentence and the stale `1 : ≈2.973`
  characterisation are gone.
- Minor 1 — **recorded as a cross-file item, accurately.** `01-platform-overview.md`
  (`revised (round 3)`) line 52 still says "`05-boards-and-io.md` §1 still carries the older
  table and must be aligned by a later cross-file pass", and its cross-file bullet (lines
  328–331) still repeats `04`'s old `≈1.7897727 MHz` / `1 : ≈2.973` and `05`'s old
  `1:3`/`5:16`/`186.2 ns` cells. `05`'s bullet names exactly those two places and assigns the
  fix to `01`; no edit to `01` was made from this brief.
- Minor 2 — **fixed.** `## Observable phenomena` note (lines 285–286): "Rows 4, 5, 6 and the
  `$40` detail rest on NESdev rather than the repository; row 10's CIC pin tie-off is
  unconfirmed — see *Open questions*." Row 10 (line 283) claims the NES-101 "CIC cartridge pins
  sit at +5 V / PPU D4"; §3 lines 106–108 mark that *(Unconfirmed — not in the cached NESdev
  pages:)* and Open questions lines 388–390 call it unconfirmed and list the cached pages
  inspected. NESdev attribution and the unconfirmed marking are now separated correctly; no row
  is over- or under-attributed.
- Minor 3 — **fixed (with the ÷4 scope caveat in Minor 1 above).** The unsupported "`PCLK` pin
  of the external video stage" is gone; line 50 now cites `PPU/pclk.md` and the raw `CLK` is
  correctly described as feeding only the video phase generator: `PPU/pclk.md` line 21
  "Входной тактовый сигнал `CLK` используется исключительно в фазогенераторе видеотракта PPU";
  `Docs/Famicom/Famicom_Evo.circ` has no `PCLK` net (`grep -c -i pclk` = 0).

## Checked against
- `Specs/05-boards-and-io.md` — full read. 400 lines (`wc -l`); `Status` line 3 =
  `revised (round 3)`; the five fixed headings appear exactly once each, in order
  (lines 270, 288, 308, 332, 354), exact spelling, no number prefix.
- `Specs/01-platform-overview.md` §2 (lines 42–52) and its cross-file/Open-questions block
  (lines 321–334) — table diff and the still-stale cross-reference.
- `Specs/04-ppu-timing.md` §1 (lines 18–25) and §10 (lines 240–251) — Dendy row, ÷5 PAL
  divider, the `PPU dots per CPU cycle` convention and the agreement statement.
- `Specs/Review/ppu-palette-timing.md` `## Revision round 2` (lines 246–268) — the origin of
  `04` §10's corrected Dendy row.
- `Specs/Conventions.md` §3, §5, §6, §7 — rank rules, the `PCLK` = PPU dot terminology, the
  five required headings and the `Claim area | BreakingNES Wiki page | HDL module` header,
  the 120–400 line target, traceability.
- `BreakingNESWiki/PPU/pclk.md` — line 9 `PCLK` = input `CLK` ÷ 4 (21.48 MHz), line 21 raw
  `CLK` only in the video phase generator, lines 25–29 timing table
  (NTSC 21477272→5369318, PAL 26601712→5320342.4).
- `HDL/PPU/pclk.v` lines 1–60 — `RP2C02` 4-latch divider vs `RP2C07` 6-latch branch; header
  comment `PCLK` NTSC 5369318 Hz / PAL 5320342.4 Hz.
- `Docs/Famicom/Famicom_Evo.circ` — `grep -c -i pclk` = 0 (no `PCLK` net).
- `Docs/NES/NES_HDL_Model/nes.v` lines 139–149 — `LS139` `First_nE(gnd)`,
  `First_A({CPU_A[15],M2})`, `First_nY(nY1)`, `Second_nE(nY1[1])`,
  `Second_A({CPU_A[14],CPU_A[13]})`, `nROMSEL = nY1[3]`, `WRAM_nCE = nY2[0]`,
  `PPU_nCE = nY2[1]`; line 22 `assign SYSTEM_CLK = CLK;`.
- `/tmp/nesdev-cache/Cartridge_connector.html.txt` — 72-pin rows: `|01 37| <- SYSTEM CLK` and
  "only available on 72-pin connectors"; `|35 71|` / `|34 70|` CIC pins; audio 45/46; EXP0–4 =
  pins 16–20 and EXP5–9 = pins 51–55; pins 62/63 = PPU A11/A10 reversed; `/IRQ` internal
  pull-up and the PlayChoice-10 open-collector note.
- `/tmp/nesdev-cache/Standard_controller.txt` — report order A,B,Select,Start,Up,Down,Left,Right
  (line 126); 1 = pressed; post-8-bit `1` (line 100); 4021 `SerIn` = GND (line 146); open-bus
  `$40`/Paperboy (line 127); PAL NES-004E 3.6 kΩ (line 152).
- `/tmp/nesdev-cache/CIC_lockout_chip.txt` — lock/key same IC, 4 MHz on pin 6, SLAVE CIC RESET,
  1 Hz square wave, pins 11–15 grounded, pin-4 defeat (lines 43, 44, 47);
  Camerica/Color Dreams/AVE/AGCI −5 V charge pump and data-not-reset protection (line 50).
- `/tmp/nesdev-cache/APU_DMC.txt` line 495 and `Errata.txt` line 84 — DMC double-clock on
  NTSC/Famicom, fixed on the 2A07.
- `Docs/NES_Famicom_Board_Revisions.md` §2.4 / §5.1 (lines 203–205, 220–224) — NES-CPU-09/-10/-11
  CIC hardening, "-11 … two 1 kΩ series resistors and two clamp diodes".

## Notes for the next round
- No blocking or major findings remain; the section reaches **consensus** and needs no further
  revision round. The two residual minors are wording/metadata-level and the file is at the
  400-line ceiling, so address them only if the ceiling is relaxed or a line is freed.
- Do not touch §3 (CIC), §4 (LS139), §5 pin rows or §6 (controller protocol): re-verified this
  round against `CIC_lockout_chip`, `nes.v`, `Cartridge_connector` and `Standard_controller`;
  no error found.
- The still-stale cross-reference lives in `01-platform-overview.md` (lines 52 and 328–331) and
  is correctly recorded as a cross-file item in `05`'s Open questions; it is not a `05` defect.

---

# Review: boards-io — round 2

File under review: `Specs/05-boards-and-io.md` (Status: revised (round 2), 400 lines).
Round 1 (below) returned `revise` with 2 blocking, 4 major and 6 minor findings; the
Revision round 2 note is at the foot of this file.

Baseline limitation: `Specs/` is untracked in this checkout (`git status` → `?? Specs/`), so
there is no git baseline to diff the round-1 text against. "Not altered" below means the
current text was re-verified against the sources, not mechanically diffed.

## Verdict
revise

## Blocking
None. Round-1 blocking 1 (clock table) and blocking 2 (ratio prose / Open question) are both
fixed — see *Resolved from round 1*.

## Major
1. **`## Open questions` line 366 "The values agree" is false for the Dendy row it cites, and
   it hides a real cross-file conflict.** The bullet lists `04-ppu-timing.md` §10's
   `1 : ≈2.973` as an instance of a merely different *presentation*, but `04-ppu-timing.md`
   §10 (lines 247–249) gives Dendy as `≈1.7897727 MHz` — the NTSC 2A03 rate — and `≈2.973`
   dots per CPU cycle, while `05` §1 (line 45) gives `1.773448 MHz` and `3:1` (exactly 3).
   `3 ≠ ≈2.973`, so the values do **not** agree; `01-platform-overview.md` Open questions
   (line 328 ff.) likewise calls `04`'s Dendy rate "the NTSC 2A03 rate, ruled out by the ÷15
   divider". The dedicated Dendy bullet immediately below (lines 367–370) records the facts
   correctly, so the fix is to qualify this sentence — e.g. "the NTSC/PAL values agree; the
   Dendy rate is a genuine numeric conflict, see the Dendy bullet" — leaving the cross-file
   fix with `04`. Major because the traceability section currently mis-states the state of
   the sibling file and contradicts `05` §1.

## Minor
1. **`01-platform-overview.md` still calls this file's §1 stale** (not a defect in `05`).
   `01` line 52: "`05-boards-and-io.md` §1 still carries the older table and must be aligned
   by a later cross-file pass", and its Open questions line 331 repeats `1:3`, `5:16`,
   `186.2 ns`, Dendy "not documented here". `05` §1 now copies `01` §2 cell-for-cell, so both
   `01` statements are false and must be removed in the same cross-file pass.
2. **Observable phenomena note (line 285) over-attributes row 10.** It says "Rows 4, 5, 6 and
   10 … rest on NESdev", but the NES-101 CIC tie-off in row 10 is marked *(unconfirmed — not
   in the cached NESdev pages)* in §3 (line 106) and in Open questions (lines 388–390). Name
   it as unconfirmed rather than NESdev.
3. **§1 line 50 "not the `PCLK` pin of the external video stage" is unsupported** (carried
   over; round 1 did not raise it). `BreakingNESWiki/PPU/pclk.md` shows `PCLK` is generated
   inside the PPU by ÷4 from the `CLK` pin and the raw `CLK` feeds only the video-out phase
   generator; `Famicom_Evo.circ` has no `PCLK` net, and no source inspected names a `PCLK`
   pin on any external video stage. Reword to "not the PPU `CLK` input pin".

## Resolved from round 1
- Blocking 1 — **fixed.** §1's table (lines 41–45) is cell-for-cell identical to
  `01-platform-overview.md` §2 (lines 42–46) across all 5 rows × 8 columns: NTSC
  `186.24 ns` / `3:1`, PAL `16:5`, Dendy `UMC 6527P + 6538 (or NOAC)` /
  `like PAL (26.601712 MHz)` / `÷15` / `1.773448 MHz (563.9 ns)` / `÷5` /
  `5.3203424 MHz (187.96 ns)` / `3:1`. The only difference is the documented local
  `*(NESdev)*` marker on the Dendy region cell.
- Blocking 2 — **fixed.** §1 lines 52–55 now read "**PPU dots per CPU cycle** (`3:1` = 3
  dots/cycle, `16:5` = 16 PPU dots per 5 CPU `PHI0` cycles = 3.2, Dendy `15:5` = exactly
  `3:1`)"; no "16 CPU cycles per 5 dots" and no claim that `01` says `5:16` remain.
- Major 1 — **fixed.** §1 lines 50–52: `SYSTEM CLK` is "the raw master oscillator
  (21.477272 MHz NTSC / 26.601712 MHz PAL) on the NES 72-pin edge only", explicitly "**not**"
  the dot clock; §5 line 144 agrees. `nes.v` line 22 `assign SYSTEM_CLK = CLK;` with line 5
  `input CLK; // 21.477272 MHz` and line 282 `input SYSTEM_CLK; // NES Board specific`;
  NESdev `Cartridge_connector`: "Main oscillator frequency output. It is only available on
  72-pin connectors", pin 37.
- Major 2 — **fixed.** §2 line 70 chipset cell now reads "2A07 / 2C07 (PAL); KOR is an NTSC
  console; BRA is PAL-M with a 2C02 PPU", matching the row's Notes and
  `Docs/NES_Famicom_Board_Revisions.md` §2.4 lines 306–307.
- Major 3 — **fixed.** §5 line 140: "PA13 is normally strapped to CHR `/CE`; the inverted
  `/A13` goes to CIRAM `/CE` *(NESdev)*"; consistent with §5 line 148 and NESdev
  `Cartridge_connector` ("Most boards tie PA13 directly to the /CE of CHR ROM or CHR RAM …
  `/A13` … usually directly connected with PPU /A13 [CIRAM /CS]").
- Major 4 — **fixed.** §1 line 45 carries `*(NESdev)*`; §1 lines 54–55 and the Open questions
  bullet (lines 367–370) cite `Cycle_reference_chart` (`26.60~ MHz ÷ 15 = 1.773448 MHz`) and
  `CPU_variants` (÷15 on parts whose date stamp has a `-`); the new Provenance row (line 342)
  names both NESdev pages and records that `Dendy.md` says only "close to NTSC"
  (`BreakingNESWiki/Dendy.md` verified: "процессор работал на частоте приближенной к NTSC
  NES, а видеосистема работала на частотах PAL").
- Minor 7 — fixed (line 285 now includes row 5). Minor 8 — fixed (§6 lines 184–187 cite the
  U7/U8 40H368 arrays and `Famicom_Scheme_IO_Annotated.png`; the `M2`-gated `CLK`/`LATCH`
  claim is now Open questions lines 385–387). Minor 9 — fixed (pitch cites `Famicom`; verified
  in `/tmp/nesdev-cache/Famicom.html`: "2.54 mm (0.1 in) on the Famicom vs. a non-standard
  2.50 mm on the NES"; pins 62/63 cite `Cartridge_connector`; CHR-RAM `/WR` is marked).
  Minor 10 — fixed (both pages named, line 202). Minor 11 — fixed (marked unconfirmed in §3
  line 106 and §5 line 142; Open questions lines 388–390 match §3's "the other two float").
  Minor 12 — fixed (Provenance header line 338 = `Claim area | BreakingNES Wiki page | HDL
  module`, Conventions §6).

## Checked against
- `Specs/05-boards-and-io.md` — full read. 400 lines (`wc -l`), Status `revised (round 2)`;
  the five fixed headings appear exactly once each, in order after §9, with exact spelling
  and no number prefix.
- `Specs/01-platform-overview.md` §2 (lines 42–52) and Open questions (lines 328–334) — table
  diff and the now-stale cross-reference.
- `Specs/04-ppu-timing.md` §10 (lines 243–256) — `04`'s Dendy row and CPU-cycles : dots
  convention.
- `Specs/Conventions.md` §3, §6, §7 — heading order/spelling, Provenance header, 120–400 line
  target, marking convention.
- `Docs/NES/NES_HDL_Model/nes.v` lines 3–23, 139–205, 278–307 — `SYSTEM_CLK = CLK`,
  `input CLK // 21.477272 MHz`, `input SYSTEM_CLK // NES Board specific`, LS139
  (`First_A={A15,M2}`, `Second_nE=nY1[1]`, `Second_A={A14,A13}`, `nROMSEL=nY1[3]`,
  `WRAM_nCE=nY2[0]`, `PPU_nCE=nY2[1]`), `PPU_nA13(~PPU_A[13])`, `CartPort` (`CPU_A[14:0]`,
  "not using A15"), `CtrlPort`/`LS368` data path.
- `Docs/NES_Famicom_Board_Revisions.md` §2.4 (lines 296–308), §1 (lines 203–223), NESN table
  (lines 265–268) — KOR NTSC / BRA PAL-M on 2C02, the -11 two 1 kΩ resistors, NESN-CPU-JIO-02
  PAL 2A07/2C07.
- `/tmp/nesdev-cache/Cartridge_connector.html` + `.txt` — pin 37 SYSTEM CLK "only available on
  72-pin connectors"; PA13 → CHR `/CE` and `/A13` → CIRAM `/CE`; pins 62/63 = PPU A11/A10
  reversed; CIC 71/70/35/34; audio 45/46; EXP0–4 = pins 16–20 and EXP5–9 = pins 51–55; `/IRQ`
  internal pull-up and the PlayChoice-10 open-collector note.
- `/tmp/nesdev-cache/Standard_controller.txt` — report order A,B,Select,Start,Up,Down,Left,
  Right; 1 = pressed; post-8-bit 1; 4021 `SerIn` = GND; open-bus `$40`/Paperboy; PAL NES-004E
  3.6 kΩ.
- `/tmp/nesdev-cache/CIC_lockout_chip.txt` — lock/key same IC, 4 MHz on pin 6, SLAVE CIC
  RESET, 1 Hz square wave, pins 11–15 grounded, pin-4 defeat.
- `/tmp/nesdev-cache/Famicom.html`, `Expansion_port.html`, `Cycle_reference_chart.html`,
  `CPU_variants.html` — pin pitch; 48-pin 2.6 mm port, pin 24 "4.00MHz CIC CLK",
  open-collector `/NMI` and push-pull `/IRQ`; Dendy ÷15 / `1.773448 MHz`.
- `BreakingNESWiki/Dendy.md`, `BreakingNESWiki/PPU/pclk.md` — "close to NTSC" + PAL video,
  UMC 6527/6538; `PCLK` = `CLK` ÷ 4 with the raw `CLK` used only by the video-out phase
  generator.

## Notes for the next round
- One line-level fix (Major 1) plus the two wording items. **The file is at the 400-line
  ceiling**, so every fix must replace text line-for-line; adding a bullet or a table row
  would break Conventions §6.
- Do not touch §3 (CIC), §4 (LS139 table) or §6 (controller protocol) — re-verified against
  `CIC_lockout_chip`, `nes.v` and `Standard_controller`; no error found.
- `01-platform-overview.md` lines 52 and 331 must be corrected in the same pass; they still
  assert that `05` §1 is stale.

---

# Review: boards-io — round 1

File under review: `Specs/05-boards-and-io.md` (Status: adapted (round 1), 390 lines).
No previous report existed for this brief; this is the first round.

## Verdict
revise

## Blocking

1. **`§1` clock table is stale and the "byte-identical to `01-platform-overview.md` §2" claim is
   false.** `05` line 47 states: "This table is byte-identical to `01-platform-overview.md` §2,
   kept in step deliberately." `01-platform-overview.md` §2 (lines 42–52) now reads:

   | Cell | `05` §1 (stale) | `01` §2 (current) |
   |---|---|---|
   | NTSC PPU dot (PCLK) | `5.369318 MHz (186.2 ns)` | `5.369318 MHz (186.24 ns)` (= 4 ÷ 21.477272 MHz) |
   | NTSC `CPU:PPU` | `1:3` | `3:1` |
   | PAL `CPU:PPU` | `5:16` | `16:5` |
   | Dendy master clock | `not documented here` | `like PAL (26.601712 MHz)` |
   | Dendy CPU divider | `not documented here` | `÷15` |
   | Dendy CPU `PHI0` | `≈ NTSC rate` | `1.773448 MHz (563.9 ns)` |
   | Dendy PPU divider | `2C07-class` | `÷5` |
   | Dendy PCLK | `≈5.320342 MHz` | `5.3203424 MHz (187.96 ns)` |
   | Dendy `CPU:PPU` | `NTSC-like, nominally 1:3` | `3:1` |

   Evidence: `01-platform-overview.md` lines 44–46 and 52 ("`05-boards-and-io.md` §1 still carries
   the older table and must be aligned by a later cross-file pass"), plus its Open questions lines
   331–332, which name this file's stale cells verbatim (`1:3`, `5:16`, `186.2 ns`,
   Dendy "not documented here"). NESdev `/tmp/nesdev-cache/Cycle_reference_chart.html`: "CPU clock
   speed … 26.60~ MHz ÷ 15 = 1.773448 MHz", "Master clocks per PPU dot … 5", "PPU dots per CPU
   cycle … 3 / 3.2 / Like NTSC"; `/tmp/nesdev-cache/CPU_variants.html` (UA6527P): "Revisions with
   \"-\" in the date stamp have a ÷15 CPU divider".

   Note for the Revision agent: `1:3` and `5:16` are numerically correct if read as *CPU cycles :
   PPU dots*, but `01` uses the inverse convention (*PPU dots : CPU cycle* / reduced divider ratio,
   `12:4`, `16:5`, `15:5`) under the identical column heading `CPU:PPU`. The two files therefore
   contradict each other; the fix is to copy `01` §2 verbatim (`3:1`, `16:5`, the full Dendy row,
   `186.24 ns`), not merely to swap the digits.

2. **`§1`'s own explanation of the ratio (lines 51–54) and the corresponding Open question
   (lines 360–363) invert the arithmetic and mis-describe the sibling file.** Both state that
   "from the dividers the arrangement is **16 CPU `PHI0` cycles per 5 PPU dots**, i.e. NESdev's
   `16:5`". That is backwards: the ÷16 CPU (1.6626070 MHz) and ÷5 dot (5.3203424 MHz) clocks give
   5.3203424 ÷ 1.6626070 = 3.2 = **16 PPU dots per 5 CPU cycles**. NESdev
   `/tmp/nesdev-cache/Cycle_reference_chart.html`: "PPU dots per CPU cycle … PAL 3.2" and
   "CPU cycles per scanline … 341 × 5÷16 = 106 9/16". Both places also assert "the source table
   reads `5:16`" about `01-platform-overview.md` §2 — `01` line 45 now reads `16:5`, so the Open
   question asks the reader to "fix" a cell that is already correct. This is self-contradictory
   with the file's own `5:16` cell and will mislead the Revision agent.

## Major

3. **`§1` line 50 mis-states `SYSTEM CLK` (wrong console and wrong signal).**
   Quote: "the `PCLK` above is the 2C02/2C07 dot clock, not the `PCLK` pin of the external video
   stage (the Famicom routes the PPU dot clock to the cartridge edge as `SYSTEM CLK`, §5)."
   (a) `SYSTEM CLK` is not on the Famicom 60-pin edge: `05`'s own §5 table says Famicom "no" /
   NES "pin 37", and the Program notes line 302 say "SYSTEM CLK on the NES cartridge edge but not
   the Famicom"; NESdev `Cartridge_connector.html`: "SYSTEM CLK: Main oscillator frequency output.
   It is only available on 72-pin connectors"; `Famicom_Evo.circ` `P1` has no SYSTEM CLK net (only
   `M2`). (b) It is the raw master oscillator, not the PPU dot clock: `nes.v` line 22
   `assign SYSTEM_CLK = CLK;` with `CLK` = 21.477272 MHz, wired to `CartPort` line 178.

4. **`§2` line 69 "Regional NES … `2A07 / 2C07`" is wrong for two of the consoles listed in the
   same row.** `Docs/NES_Famicom_Board_Revisions.md` line 306: "Korean NES ("Comboy") | NES-001
   (KOR) | **NTSC** | 3195A … NTSC console with a PAL-B lockout chip"; line 307: "Brazilian NES |
   NES-001 (BRA) | **PAL-M** | 6113B1 | … colour-conversion circuitry because the **2C02** PPU
   outputs NTSC colour". The CPU/PPU cell therefore contradicts the row's own Notes cell and the
   cited source. Split the row or qualify the chipset per console.

5. **`§5` table line 138: "`/A13` is inverted PA13, normally strapped to CHR `/CE`" — wrong
   signal.** NESdev `Cartridge_connector.html` signal descriptions: "PPU A0..A13 … Most boards tie
   **PA13** directly to the **/CE of CHR ROM or CHR RAM** to map it into pattern table space
   ($0000–$1FFF) without any extra logic"; "PPU /A13: The inverted form of PPU A13. Typically used
   to map **nametables** and attribute tables to $2000–$3FFF"; "CIRAM /CE … usually directly
   connected with **PPU /A13**". `05`'s own line 146 already says the opposite ("A CHR-ROM
   cartridge decodes only `/RD` and PA13"), so the table contradicts the body text. Correct to:
   PA13 → CHR `/CE`; `/A13` → CIRAM `/CE`.

6. **Dendy clocks are NESdev rank 4 but carry no `*(NESdev)*` marker, and the `## Provenance` row
   mis-attributes them.** `05` lines 17–19 and 353–354 define `*(NESdev)*` as the marker for facts
   "from the NESdev Wiki only and not corroborated here"; the `§1` Dendy cells have no marker.
   `01-platform-overview.md` Open questions lines 321–327 records that rank-1 `Dendy.md` only says
   the CPU runs "close to the NTSC NES" and that the ÷15/`1.773448 MHz` figures are NESdev; `05`'s
   Provenance line 339 attributes "region clocks" to `Docs/NES_Famicom_Board_Revisions.md`,
   `MB/{NES,Famicom}.md`, `Dendy.md` — none of which documents ÷15. `05` also has no Dendy-clocks
   entry in `## Open questions`.

## Minor

7. `## Observable phenomena` line 283 ("Rows 4, 6, 10 and the `$40` detail rest on NESdev rather
   than the repository") under-enumerates: row 5 (DMC fetch double-clocking the pad) is NESdev-only
   by the file's own §6 Open question and its Provenance row at line 346 ("(not in the wiki) NESdev
   `Controller_reading`, `Errata`").

8. `§6` line 184: "On the Famicom the pulse is an inverting 368 buffer whose input is `M2` and whose
   enable is the port read strobe (`/IN0`, `/IN1`)." The cited `Famicom_Evo.circ` `P4`/`P5`
   subcircuits expose only `OUT_0`, `S4016_CUP`/`S4017_CUP`, `S4016_D0`/`S4017_D0` and (P5) `MIC`
   — no CLK net and no `M2`; `nes.v` `CtrlPort`/`LS368` model only the data path
   (`nY` → `CPU_D[0..4]`). `Docs/Famicom/Famicom_Scheme_IO_Annotated.png` does show the U7/U8
   40H368 arrays around the pad/expansion connectors — cite that file (or move the detail to Open
   questions).

9. Inline `*(NESdev)*` facts in `§5` do not name a page: line 144 "Famicom pitch 2.54 mm, NES
   2.50 mm" is not in the cached `Cartridge_connector`/`Expansion_port`, and line 146 "a CHR-RAM
   cartridge must also decode PPU `/WR` or writes through `$2007` are lost" is unmarked even though
   it is a NESdev-derived rule. Name the page (`Hardware_pinout` / `Famicom_cartridge_dimensions`)
   or mark.

10. `§6` line 198–200 attributes the PAL port diodes and the pad models to `Controller_port_pinout`.
    The diodes are on `Controller_port_pinout#Protection_Diodes`, but the model numbers and the value
    are on `Standard_controller#PAL`: "PAL controllers for these regions (model NES-004E) each
    contain a 3.6KΩ resistor between these two inputs and 5V" and "Model NESE-034 ver1.1 is also
    diode protected". The citation should name both pages.

11. `§3` line 105–106 / Open question line 378–379: the NES-101 CIC tie-off ("two CIC pins … tied
    to +5 V and PPU D4; the other two float") is labelled NESdev-only but is not present in any
    cached NESdev page inspected (`Cartridge_connector`, `CIC_lockout_chip`,
    `CIC_lockout_chip_pinout`, `NES_reference_guide`, `Famicom`). Name the exact page/revision.
    (It is already recorded in Open questions, so it does not block consensus.) The Open question
    wording ("the remaining CIC pins are tied to +5 V and PPU D4") also differs from `§3` ("the
    other two float").

12. `## Provenance` header (line 336) is `| Claim area | BreakingNES Wiki | HDL |`; Conventions §6
    specifies `Claim area | BreakingNES Wiki page | HDL module`.

## Checked against

- `Specs/05-boards-and-io.md` — full read; structure: 390 lines (within 120–400), `Status` line
  present, the five fixed headings present exactly once each, in order, with no number prefix.
- `Specs/01-platform-overview.md` §2, §8, External/Open questions — clock table and the explicit
  cross-file stale note (lines 42–52, 328–334).
- `/tmp/nesdev-cache/Cycle_reference_chart.html`, `CPU_variants.html` — NTSC/PAL/Dendy clocks,
  ÷15, "PPU dots per CPU cycle 3 / 3.2".
- `/tmp/nesdev-cache/Cartridge_connector.html` — 60-pin and 72-pin pinouts, `/ROMSEL` = NAND(M2,
  A15), SYSTEM CLK "only available on 72-pin connectors", CIC pins 71/70/35/34, audio pins 45/46,
  EXP 16–20/51–55, PA13 → CHR `/CE`, `/A13` → CIRAM `/CE`, pins 62/63 mirrored.
- `/tmp/nesdev-cache/Standard_controller.html` — report order A,B,Select,Start,Up,Down,Left,Right;
  1 = pressed; "all subsequent bits will report 1"; `$4016` write `xxxx xxxS`; open-bus `$40`
  (Paperboy); Famicom pad 2 has no Select/Start and its microphone is `$4016` D2; PAL diodes,
  NES-004E 3.6 kΩ, NESE-034; 4021 `SerIn` = GND.
- Wayback `Controller_reading` — CLK = `R/W nand (ADDRESS == $4016/$4017)`, latched on the
  high→low and shifted on the low→high edge; DMC "double clock"/bit-deletion → spurious Right;
  "This glitch is fixed in the 2A07"; multiple-read and OAM-DMA-synchronised workarounds; the
  NES-001/NES-101/Famicom unconnected-line and D5–D7 open-bus table (NES-101 D2–D4 open bus).
- `/tmp/nesdev-cache/APU_DMC.html` — "fixed on the 2A07 and PAL NES is exempt of this bug";
  `Errata.html` — "[2A03] DMC DMA during a controller read … double clocking".
- Wayback `Controller_port_pinout`, `CIC_lockout_chip_pinout`, `EXP_pins`, `Category:Expansion_audio`,
  `Four_player_adapters` — all real; `/wiki/NES` redirects to `NES_reference_guide`; `Four_score`
  resolves to `Four_player_adapters`; `APU_Mixers` redirects to `APU_Mixer` (singular correct).
  Every content section of the draft appears in the correspondence table.
- `/tmp/nesdev-cache/CIC_lockout_chip.html` — lock/key same IC, LOCK/KEY wiring, 4 MHz on pin 6,
  slave-CIC reset, 1 Hz reset square wave, pins 11–15 grounded, pin-4 defeat, Camerica/Color
  Dreams/AVE/AGCI −5 V charge pump, diodes on data not reset pins.
- `/tmp/nesdev-cache/Expansion_port.html` — DA-15 pinout (1 GND, 2 SOUND, 3 /IRQ, 4–8 joypad-2
  D4–D0, 9 /OE, 10–12 OUT2–0, 13 joypad-1 D1, 14 /OE, 15 +5 V), 48-pin NES port incl. pin 24
  "4.00 MHz CIC CLK", pin 3 audio mix input, open-collector /NMI, series 1 kΩ /IRQ note.
- `Docs/NES/NES_HDL_Model/nes.v` — `LS139` `First_A = {A15, M2}`, `nY1[3]` = `/ROMSEL`,
  `Second_nE = nY1[1]`, `Second_A = {A14, A13}`, `nY2[0]` = WRAM `/CE`, `nY2[1]` = PPU `/DBE`;
  `CPU_A[14:0]` to `CartPort`; `PPU_nA13 = ~PPU_A[13]`; `VRAM_A10`/`VRAM_nCE` inputs from the cart;
  `SYSTEM_CLK = CLK`; `LS368` P4/P5 inverting the pad data; pull-ups on `P1_D`/`P2_D`, `/IRQ`,
  `/NMI`. §4's decode table verified correct.
- `Docs/Famicom/Famicom_Evo.circ` — `P1` labels (M2, n_PA13, n_ROMSEL, SoundIn/SoundOut,
  VRAM_A10, n_VRAM_CS; no SYSTEM CLK), `P2` labels match the DA-15 pinout, `P4`/`P5`/`LS139`/
  `U_40H368`, `RM1` comment, 11 `Pull Resistor` components.
- `Docs/Famicom/Famicom_Scheme_IO_Annotated.png` — "RM1: 10K×11"; P4/P5/P2 pin-by-pin;
  U7/U8 40H368 arrays; P1 Φ2 (M2) on the 60-pin edge, no SYSTEM CLK.
- `Docs/Famicom/Chipset/Readme.md` — two 40H368 inverting tri-state arrays, one as the audio
  amplifier, one inverting PA13; LS139 "bridge"; LS373 latches the PPU address.
- `Docs/NES/NESController/{con_schem.jpg,timing.jpg,CD4021.pdf}` — `con_schem.jpg` shows the 4021
  with `SERIN` (pin 11) to GND and the button labels; `timing.jpg` labels the 3rd/4th shifts
  "Start, Select" (confirming the draft's Open question). `Docs/Famicom/FamiController/Source.txt`
  is only a Console5 URL plus "Note: Up/Down mixed up", so it supports the pad wiring only weakly.
- `Docs/NES_Famicom_Board_Revisions.md` — §1.3–1.5, §2.2–2.4, §5.1–5.4: board revisions, GPM
  three-layer PCB and JIO, 72-pin connector populating 68 pins, NES-CPU-09/-10/-11 CIC hardening,
  CIC part table (3193/3193A, 6113 family, 3195A, 3197A, 3196A, 3198/3199), regional NES table,
  HVCN-CPU-02 diode, RF/video differences.
- `BreakingNESWiki/MB/{NES,Famicom}.md`, `BreakingNESWiki/APU/pads.md`,
  `HDL/APU/{io,pads,regs}.v` — CIC/TBD text, expansion-port routing only to CPU I/O terminals,
  RM1 pull-ups on `/IRQ`/`/NMI`, reset RC, `/IN0`/`/IN1` = `$4016`/`$4017` read strobes, OUT0–2
  latches.

## Notes for the next round

- Fix the two blocking items first: replace `§1`'s table with `01-platform-overview.md` §2 verbatim
  (NTSC `3:1` / `186.24 ns`, PAL `16:5`, full Dendy row `like PAL (26.601712 MHz)` / `÷15` /
  `1.773448 MHz (563.9 ns)` / `÷5` / `5.3203424 MHz (187.96 ns)` / `3:1`), then rewrite the
  paragraph at lines 51–54 and the Open question at 360–363 so they read "16 PPU dots per 5 CPU
  `PHI0` cycles (3.2 dots per CPU cycle)" and no longer claim `01`'s PAL cell is `5:16`. Add the
  `*(NESdev)*` marker and the `Cycle_reference_chart`/`CPU_variants` citation to the Dendy row
  (model: `01` Open questions lines 321–327).
- Three factual cells to correct: `§1` line 50 (`SYSTEM CLK` is NES-only and is the raw oscillator),
  `§2` line 69 (KOR is NTSC, BRA is PAL-M/2C02), `§5` line 138 (PA13 → CHR `/CE`, `/A13` → CIRAM
  `/CE`).
- Do **not** weaken `§6`: the controller protocol verified exactly against `Controller_reading`,
  `Standard_controller` and `APU_DMC`, including the strobe, bit order, inversion, the post-8-bits
  `1`, the open-bus `$40`, the DMC double-clock/bit-deletion and the 2A07 fix. `§4`'s 74LS139 table
  also verified cell-for-cell against `nes.v`.

## Revision round 2

- Blocking 1: fixed — `Specs/05-boards-and-io.md` §1 clock table replaced with
  `01-platform-overview.md` §2 cell-for-cell: NTSC PCLK `186.24 ns` and ratio `3:1`, PAL
  `16:5`, full Dendy row `UMC 6527P + 6538 (or NOAC)` / `like PAL (26.601712 MHz)` / `÷15` /
  `1.773448 MHz (563.9 ns)` / `÷5` / `5.3203424 MHz (187.96 ns)` / `3:1`. The false
  "byte-identical" sentence now reads "copies `01-platform-overview.md` §2 cell-for-cell"
  and names the one local difference (the *(NESdev)* marker).
- Blocking 2: fixed — §1 prose and the former Open question no longer say "16 CPU cycles per
  5 dots": both now read 16 **PPU dots** per 5 CPU `PHI0` cycles = 3.2 dots per CPU cycle,
  and no longer claim `01`'s PAL cell is `5:16`; the Open question is replaced by a
  `CPU:PPU` convention note (the `04-ppu-timing.md` §10 `CPU cycles : dots` presentation).
- Major 1 (SYSTEM CLK): fixed — §1 prose now says `SYSTEM CLK` is the raw master oscillator
  (21.477272 MHz NTSC / 26.601712 MHz PAL), present on the NES 72-pin edge only, and **not**
  the PPU dot clock; consistent with §5 and `nes.v` (`assign SYSTEM_CLK = CLK;`).
- Major 2 (KOR/BRA): fixed — §2 Regional NES chipset cell now reads `2A07 / 2C07 (PAL); KOR
  is an NTSC console; BRA is PAL-M with a 2C02 PPU`, matching the row's Notes and
  `Docs/NES_Famicom_Board_Revisions.md` §2.4.
- Major 3 (`/A13` → CHR `/CE`): fixed — §5 table row now reads `PA13` → CHR `/CE`,
  inverted `/A13` → CIRAM `/CE` *(NESdev)*, consistent with §5's body text.
- Major 4 (Dendy clocks): fixed — `*(NESdev)*` marker added to the Dendy row, a Dendy
  Open-questions bullet added (`Cycle_reference_chart` `26.60~ MHz ÷ 15 = 1.773448 MHz`,
  `CPU_variants` ÷15), a new Provenance row added, and `Dendy.md` removed from the "region
  clocks" provenance row (it says only "close to NTSC").
- Minor 7: fixed — the `Observable phenomena` note now lists row 5 alongside rows 4, 6, 10.
- Minor 8: fixed — the unevidenced "inverting 368 whose input is `M2`" claim in §6 is
  replaced by the corroborated U7/U8 40H368 data-path statement; the `M2`-gated
  `CLK`/`LATCH` claim moved to Open questions.
- Minor 9: fixed — pin pitch now cites NESdev `Famicom`, pins 62/63 cite
  `Cartridge_connector`, and the CHR-RAM `/WR` rule is marked *(NESdev)*.
- Minor 10: fixed — PAL diode citation now names both
  `Controller_port_pinout`#Protection_Diodes and `Standard_controller`#PAL.
- Minor 11: fixed — the NES-101 CIC tie-off is now marked *(unconfirmed — not in the cached
  NESdev pages)* in §3 and §5, and the Open question names the cached pages inspected and
  matches §3's "the other two float" wording.
- Minor 12: fixed — Provenance header is now `Claim area | BreakingNES Wiki page | HDL module`.
- Kept as is: §3 CIC body, §4 LS139 decode, §6 controller protocol, pin numbers and §5 body
  text (only the two minor markers above changed). File is 400 lines; `Status` = revised
  (round 2). No refutations were needed.

## Revision round 3

- Major 1 (Open questions, `04-ppu-timing.md` §10 "the values agree" is false for Dendy):
  **fixed** — the Open-questions bullet (`Specs/05-boards-and-io.md` line 363) no longer says
  `04` §10 writes CPU cycles : dots (`1 : 3`, `5 : 16`, `1 : ≈2.973`). `04-ppu-timing.md` §10
  was revised after round 2 and now gives Dendy `1.773448 MHz (÷15)` / `3 : 1` / 3 dots per CPU
  cycle under the `PPU dots per CPU cycle` convention, i.e. it agrees with §1 here. The
  rewritten bullet states that agreement explicitly and the stale "the presentation does not
  [agree]" claim is deleted. Line-neutral (4 lines replaced by 4).
- Minor 1 (`01-platform-overview.md` lines 52 and ~331 still call §1 stale): **recorded as a
  cross-file item** — the same rewritten Open-questions bullet now names `01` line 52 ("older
  table") and its cross-file bullet (~line 331, which repeats `1:3`/`5:16`/`186.2 ns` and
  `04`'s old `1 : ≈2.973`) as stale, and assigns the fix to the final cross-file pass over
  `01`. No edit to `01` (not this brief's file). Line-neutral.
- Minor 2 (Observable-phenomena note, line ~285, over-attributes row 10): **fixed** — the note
  now reads "Rows 4, 5, 6 and the `$40` detail rest on NESdev … row 10's CIC pin tie-off is
  unconfirmed — see *Open questions*", matching §3 line 106 and the NES-101 Open question.
  Line-neutral (2 lines replaced by 2).
- Minor 3 (§1 "not the `PCLK` pin of the external video stage" is unsupported): **fixed** by
  citing support and dropping the unsupported pin — line 50 now reads "the 2C02/2C07 dot clock
  (the PPU `CLK` input ÷4, `PPU/pclk.md`)". `BreakingNESWiki/PPU/pclk.md`: "`PCLK` получается
  путем замедления (деления) входного тактового сигнала `CLK` (21.48 MHz) в 4 раза" and the
  raw `CLK` "используется исключительно в фазогенераторе видеотракта"; `Famicom_Evo.circ` has
  no `PCLK` net (grep count 0). Line-neutral (1 line replaced by 1).
- Kept as is: §3 CIC, §4 LS139 decode, §5 body, §6 controller protocol, §7 — not touched
  (round 3 made no change there). File is 400 lines (`wc -l`); `Status` = revised (round 3).
  All four edits were line-neutral; no refutations were needed.
