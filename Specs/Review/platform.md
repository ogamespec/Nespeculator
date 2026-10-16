# Review: platform — round 4

Reviewed revision: `/mnt/c/Work/breaks/Specs/01-platform-overview.md`,
**Status:** revised (round 3), 355 lines, md5
`9db0ddcfe21dcac0dde7220448d84889` (mtime 2026-10-13 22:47:56). Re-verification of
the round-2 report kept below, after Revision round 3 reworded the §2
`CPU:PPU` definition.

## Verdict

consensus

## Blocking

None. The round-2 Major 1 and Minor 1 are genuinely fixed; the round-1 findings
remain fixed (spot-checked, not re-litigated). The only remaining defects are in
the cross-file list this file correctly delegates (recorded below) plus the
unchanged length note.

## Major

None.

Round-2 Major 1 (the `CPU:PPU` column definition) is resolved:

- §2 lines 48–51 now define the column as "the CPU:PPU divider ratio (`12:4`,
  `16:5`, `15:5`), numerically the **PPU dots per CPU cycle**", with `3:1` = 3
  dots per CPU cycle and `16:5` = 16 dots per 5 CPU cycles = 3.2. Both false
  "16 CPU cycles per 5 dots" phrases are gone (`grep "16 CPU cycles per 5 dots"`
  → no hit); the only surviving order statement is the correct "16 dots per 5
  CPU cycles". This is the minimal fix round 2 prescribed.
- The cells re-derive consistently with the definition: NTSC ÷12/÷4, 12÷4 = 3 →
  `3:1`; PAL ÷16/÷5, 16÷5 = 3.2 → `16:5`; Dendy ÷15/÷5, 15÷5 = 3 → `3:1`.
  Cached `Cycle_reference_chart.html`, "PPU dots per CPU cycle" row, gives `3`
  (Ricoh 2C02), `3.2` (Ricoh 2C07) and "Like NTSC" (UMC UA6538), and its
  "CPU cycles per scanline" row gives `341 × 5÷16 = 106 9⁄16` for PAL — both
  match the file's text.
- Observation only, no action: the header `CPU:PPU` reads CPU-first while the
  values are written PPU-first (`3:1` = 3 dots per CPU cycle); the definition
  sentence removes the ambiguity, and this is the wording round 2 itself
  prescribed.

## Minor

1. **`## Open questions`, line 332 — the cross-file bullet cites
   `04-ppu-memory-map.md` §14, which does not exist.** That sibling has nine
   numbered sections (`§1. The 14-bit PPU address space` … `§9. VRAM controller
   and the external PPU address bus`). The `$3000–$3EFF`-as-fact text is at §1
   (line 19), §3 (line 60), `## Observable phenomena` row 8 (line 218) and
   `## Programming / implementation notes` (line 227). The substantive claim is
   true; the section reference is stale (already wrong in the round-2 report,
   which recorded the same lines as "§1/§3/§14"). Fix: `§1, §3 and the two
   fixed-heading tables`.
2. **Same bullet, line 333 — `03-apu-overview.md` "without the tie-breaker
   above".** Line 207 does give the letterless `M2` as `15/24`, but that sibling
   (now `Status: revised (round 2)`, mtime 22:49:24, i.e. written after this
   file) itself quotes NESdev `CPU_pinout`'s opposite `17/24` in its `### M2`
   section (line 75) and records the conflict in its own Open questions
   (line 303), deliberately keeping the schematic assignment. The bullet is
   right about the value and the divergence, but "resolves the conflict the
   other way" is the accurate wording; as written it implies the sibling lacks
   the tie-breaker.
3. **Length (round-2 Minor 2, unchanged).** 355 lines is inside
   `Specs/Conventions.md` §6 (120–400) but still above the REVISION role's
   120–300 limit (`agents/REVISION.md` step 5). No content defect; flag it for
   the Consensus record.

## Cross-file items (residual — not defects of this file)

All four verified against the current siblings (mtimes 22:27–22:49):

1. `04-ppu-timing.md` (**Status:** adapted (round 1)) §10 "CPU:PPU phase
   alignment", lines 245–249, still uses the opposite order `Ratio (CPU cycles :
   dots)` (`1 : 3`, `5 : 16`) with a separate "Dots per CPU cycle 3 / 3.2"
   column, and gives Dendy as `≈1.7897727 MHz` / `1 : ≈2.973`. The table is
   internally arithmetically correct; it disagrees with §2's convention and
   Dendy figure. Recorded accurately at `## Open questions` lines 328–330.
2. `05-boards-and-io.md` (**Status:** adapted (round 1)) §1 lines 43–45 still
   carries NTSC `1:3`, PAL `5:16`, `186.2 ns` and Dendy "not documented here";
   line 47 still claims to be "byte-identical to `01-platform-overview.md` §2",
   which is now false. This file no longer repeats that claim: line 52 says the
   sibling "still carries the older table and must be aligned by a later
   cross-file pass", which is accurate.
3. `04-ppu-memory-map.md` (**Status:** adapted (round 1)) §1/§3 and the
   fixed-heading tables still present the `$3000–$3EFF` mirror as fact
   (lines 19, 60, 218, 227), not as NESdev rank 4. Recorded accurately (modulo
   the section reference in Minor 1).
4. `03-apu-overview.md` resolves the letterless `M2` conflict the other way
   (`15/24`), recorded in the same bullet.

## Checked against

- `/tmp/nesdev-cache/Cycle_reference_chart.html` — CPU columns `Ricoh 2A03`
  ÷12 / `Ricoh 2A07` ÷16 / `UMC UA6527P` ÷15; PPU columns `Ricoh 2C02` /
  `Ricoh 2C07` / `UMC UA6538`; "Master clocks per PPU dot" `4` / `5` / `5`;
  "PPU dots per CPU cycle" `3` / `3.2` / `Like NTSC`; "CPU cycles per scanline"
  `341 × 4÷12` / `341 × 5÷16 = 106 9⁄16` / `341 × 5÷15`. Confirms §2's
  definition, all three cells, the PAL arithmetic and both Open-questions
  bullets.
- `/mnt/c/Work/breaks/Specs/01-platform-overview.md` — full re-read: §2
  lines 44–52, §5 line 132, §9 lines 187/191–196, §11 lines 228–229,
  `## Open questions` lines 315–355.
- Siblings read for the cross-file bullet: `04-ppu-timing.md` headings + §10
  lines 245–249; `05-boards-and-io.md` §1 lines 43–53; `04-ppu-memory-map.md`
  headings + lines 19/60/218/227/280; `03-apu-overview.md` lines 73–75/207/303.
- Round-1 findings spot-checked as still fixed (not re-litigated): `$3000–$3EFF`
  row `NESdev; …` + dedicated Provenance row with `—`/`—` (line 304); Dendy row
  full figures (line 46) and the 3-dots-per-cycle bullets (54–59); index note
  (228–229); APU I/O pin count/direction (85); `186.24 ns` (44);
  `**Primary sources:**` (10–19); readability "In this and all previous
  revisions" (194); letterless `M2` `17/24` with the conflict flagged (187).
- Template check: `Status: revised (round 3)`; 355 lines (120–400); the five
  fixed headings present, exact, unnumbered and in order at line 231
  `## Observable phenomena`, 247 `## Programming / implementation notes`, 274
  `## NESdev Wiki correspondence`, 295 `## Provenance`, 313 `## Open questions`;
  all 11 numbered content sections precede them; no orphan section.

## Notes for the next round

- **Consensus:** no blocking finding, and the one outstanding major from round 2
  is closed. The file's remaining dependencies are cross-file and are correctly
  delegated in `## Open questions`; they are recorded here as residual/minor.
- Optional cleanup for a later revision: the two wording fixes to the cross-file
  bullet (Minor 1–2) and the length note (Minor 3). None changes a technical
  fact in the spec.
- A set-wide pass must still align `04-ppu-timing.md` §10 and
  `05-boards-and-io.md` §1 with §2's convention and figures; `05` §1 line 47's
  "byte-identical" claim is the one statement in the set that is now definitely
  false.
- No spec file was edited by this role; no `git commit` was made.

---

# Review: platform — round 2

Reviewed revision: `/mnt/c/Work/breaks/Specs/01-platform-overview.md`,
**Status:** revised (round 2), 355 lines, md5
`c422c0e18cc707cbbc861a13274172f6` (mtime 2026-10-13 22:38:51). Re-verification of
the round-1 report kept below (2 major, 6 minor, plus the task's PAL-cell item),
re-read against the current `Specs/0*.md` siblings and the `/tmp/nesdev-cache`
Wayback copies.

## Verdict

revise

## Blocking

None. All eight round-1 findings are genuinely fixed (see the disposition table
below); nothing was found that is factually wrong against a rank 1–3 source.

## Major

1. **§2 "Master clocks and CPU:PPU ratio" — the column definition contradicts the
   cells and states the PAL relationship backwards.** The paragraph under the
   table (lines 48–50) reads:

   > The `CPU:PPU` column is CPU `PHI0` cycles : PPU dots ... `3:1` means 3 dots
   > per CPU cycle, `16:5` means 16 CPU cycles per 5 dots. The PAL cell is `16:5`,
   > **not** `5:16` — the ÷16 and ÷5 dividers give 16 CPU cycles per 5 dots.

   Three statements cannot all hold:
   - Under the stated order *CPU cycles : PPU dots*, PAL is
     1.662607 : 5.3203424 = **5 : 16** and NTSC is **1 : 3**; the printed cells
     (`3:1`, `16:5`) are the reciprocals of the definition the same sentence
     gives.
   - "`16:5` means 16 CPU cycles per 5 dots" is arithmetically false. PAL has
     3.2 PPU dots per CPU cycle, i.e. **16 dots per 5 CPU cycles** (5 CPU cycles
     per 16 dots). NESdev encodes exactly that in the page this row cites:
     `CPU cycles per scanline = 341 × 5÷16 = 106 9⁄16` and `PPU dots per CPU
     cycle = 3.2` (`/tmp/nesdev-cache/Cycle_reference_chart.html`, "Clock rates"
     / "CPU cycle counts"). The printed reading would put 341÷5×16 ≈ 1091 CPU
     cycles on a PAL scanline instead of 106.5625.
   - The sentence "the ÷16 and ÷5 dividers give 16 CPU cycles per 5 dots"
     repeats the inversion; the dividers give 16 *dots* per 5 CPU cycles (the
     divider ratio 16:5 equals the dots per CPU cycle).

   The numeric cells are correct if the column means **PPU dots per CPU cycle**
   (= CPU÷PPU divider ratio, 12:4 / 16:5 / 15:5), which is what the following
   gloss "`3:1` means 3 dots per CPU cycle" and line 66 ("one CPU cycle is
   exactly 3 PPU dots on NTSC and 3.2 PPU dots on PAL") already say. The defect
   is the definition sentence and the two "16 CPU cycles per 5 dots" phrases.
   Round 1 graded the same class of defect in this very table as major
   ("Both majors below are labelling / currency defects, not wrong silicon
   behaviour"), so this is major, not minor: the section's own definition
   sentence contradicts its table.
   Minimal fix: state the convention once, e.g. "The `CPU:PPU` column is the
   CPU:PPU divider ratio (12:4, 16:5, 15:5), numerically the PPU dots per CPU
   cycle: `3:1` = 3 dots per CPU cycle, `16:5` = 16 dots per 5 CPU cycles", and
   delete both "16 CPU cycles per 5 dots" phrases. The surviving convention must
   also be reconciled with `04-ppu-timing.md` §10, which uses the opposite order
   with a matching header — see the cross-file items.

## Minor

1. **§2 line 51 — the claim about the sibling is stale and false.** "...
   `05-boards-and-io.md` §1 reproduces this table byte-for-byte and still carries
   the inverted cell ..." The two tables are no longer byte-identical: 05 §1
   (mtime 22:31:30, i.e. before this file's 22:38:51 revision) has NTSC `1:3`,
   PAL `5:16`, `186.2 ns`, and a Dendy row of "not documented here / ≈ NTSC rate
   / NTSC-like, nominally 1:3". 05 §1 line 50 still claims to be "byte-identical
   to `01-platform-overview.md` §2". The substantive disagreement is a cross-file
   item (05 already documents its side in its own Open questions); the defect
   here is only the false "byte-for-byte" assertion.
2. **Process / length.** 355 lines is inside `Specs/Conventions.md` §6 (120–400)
   but above the REVISION role's 120–300 limit (`agents/REVISION.md` step 5); the
   revision grew the file 318 → 355. Not a content defect; flag it for the
   Consensus record.

## Round-1 finding disposition (round 2)

| Round-1 finding | Status | Evidence in the reviewed revision |
|---|---|---|
| Major 1 — `$3000–$3EFF` presented as a board-decode fact | fixed | §5 line 132 now `NESdev; the board decode that ignores PPU A12 is not documented in this repository's rank 1–3 sources`. Provenance line 304 is a dedicated `$3000–$3EFF nametable mirror` row with `—` in both columns and the empty `CartPort` body noted. Open questions lines 335–342 state the mirror is rank-4-only and the palette alias rank-1–3-derived. `nes.v` is no longer cited for the decode; line 303 re-points to the top-level `LS139`/`RAM` wiring, which exists (`LS139 DMX`, `RAM WRAM`, `RAM VRAM` in `Docs/NES/NES_HDL_Model/nes.v`). |
| Major 2 — Dendy clock row and Open question | fixed | §2 line 46 `like PAL (26.601712 MHz) / ÷15 / 1.773448 MHz (563.9 ns) / ÷5 / 5.3203424 MHz (187.96 ns) / 3:1`; bullet lines 54–59 cite `Cycle_reference_chart` (UMC UA6527P, `26.60~ MHz ÷ 15 = 1.773448 MHz`, 113⅔ CPU cycles per scanline) and `CPU_variants` (÷15, ÷16 without `-` in the date stamp) and state **exactly 3 PPU dots per CPU cycle**. Open questions lines 322–334 replace the false "undocumented" reason with the rank-4 provenance and the `04-ppu-timing.md` §10 disagreement. All figures confirmed in the cache; 1/1.773448 MHz = 563.87 ns and 4×15/5 = 3 exactly. |
| Minor 1 — stale §11 index note | fixed | §11 lines 228–229: "All 20 listed files are present in `Specs/`; `04-ppu-scrolling.md` ... is now included (Status: adapted (round 1))". The index lists exactly the 20 existing `Specs/0*.md` files and `04-ppu-scrolling.md` indeed carries `Status: adapted (round 1)`. |
| Minor 2 — APU I/O pin count/direction | fixed | §3 line 85: `5 (3 outputs OUT0–OUT2, 2 read strobes /IN0, /IN1); 4`, direction `APU → board`, note "All five APU pins are driven by the APU (`APU =>` in `APU/pads.md`)". `APU/pads.md` lines 23–27 mark `/IN1`, `/IN0`, `OUT2..OUT0` all `APU =>`. |
| Minor 3 — 186.2 ns | fixed | §2 line 44 `5.369318 MHz (186.24 ns)`; 1/5 369 318 = 186.242 ns. The PAL/Dendy dot clock is given in full (`5.3203424 MHz`, `187.96 ns`). |
| Minor 4 — incomplete `**Primary sources:**` | fixed | Lines 10–19 add `PPU/{vram_ctrl,par}`, `Docs/UMC6527P/div_umc.v`, `HDL/APU/{regs,dma,pads}.v`, `HDL/PPU/{vram_ctrl,cram,regs,pads,hv,hv_decoder}.v`, `HDL/Common/bustris.v`. Every listed path exists on disk; every `.md`/`.v` cited in the body appears in the list (remaining diffs are `Specs/*.md` cross-references); Provenance line 303 records that `NES_HDL_Model/Readme.md` is a title stub. |
| Minor 5 — `$2004`/palette readability scope | fixed | §9 line 194: "**In this and all previous revisions** OAMDATA (`$2004`) and palette RAM are **not readable** — i.e. letterless, A, B, C, D, D-0 and E too (`Docs/...` §4, NESdev `PPU_variants`)". Line 193 adds `RP2C02D`, `RP2C02D-0`, `RP2C02E`; line 192 adds `RC2C02C`; line 191 attributes the PPUMASK/PPUCTRL asynchrony to A alone and keeps B at the X=255 bug. Confirmed by `Docs/NES_Famicom_Board_Revisions.md` §4 (lines 358–365) and `PPU_variants` ("In this and all previous revisions...", "RP2C02A ... PPUMASK and PPUCTRL seem to be entirely asynchronous", "RP2C02B ... just like 2C02A"). |
| Minor 6 — M2 conflict rank wording | fixed | Open questions lines 315–321 call it a rank-1 (`APU/old_2a03.md`, 15/24) vs rank-3 (`Docs/...`, 17/24) conflict and cite the tie-breakers `CPU_pinout` and `CPU_variants`; §9 line 187 states 17/24 with the Wiki's 15/24 flagged. Sources confirm both sides (`old_2a03.md` line 59; Docs §3; `CPU_pinout` 17/24 letterless, 15/24 E/G/H, 19/32 2A07). |
| Task item — PAL cell `5:16` → `16:5` | partly fixed | The cell is now `16:5` and matches NESdev's `PPU dots per CPU cycle = 3.2`, but the column definition and gloss are inverted — see Major 1. |

## Cross-file items (recorded here, not defects of this file)

1. `04-ppu-timing.md` §10 (Status still `adapted (round 1)`) gives Dendy as
   `≈1.7897727 MHz` / `1 : ≈2.973` — the round-1 Major 2 sibling defect; §2's
   Open questions (lines 331–334) records it.
2. `04-ppu-timing.md` §10 uses the **opposite convention with a matching
   header**: "Ratio (CPU cycles : dots)", NTSC `1 : 3`, PAL `5 : 16`, with
   "Dots per CPU cycle 3 / 3.2". That table is internally correct; this file's
   §2 cells use the reciprocal order under a header that reads CPU-first. The
   two files must agree on one convention, and neither currently cross-references
   the other's choice.
3. `05-boards-and-io.md` §1 still carries the round-1 table (NTSC `1:3`, PAL
   `5:16`, `186.2 ns`, Dendy "not documented here") and still claims to be
   byte-identical to §2 here. Its Open questions (lines 360–362) already document
   the PAL disagreement; its Dendy row is now the stale one.
4. `04-ppu-memory-map.md` §1/§3/§14 still presents `$3000–$3EFF` and "the board
   decode ignores PPU A12" as fact and not as rank 4 (lines 19, 60, 218, 227),
   so the set disagrees about the rank of that mirror even though this file is
   now correct. Expected: that sibling predates the round-2 revision.
5. `03-apu-overview.md` line 207 states the letterless 2A03 `M2` high time as
   `15/24` (the rank-1 value) without the 17/24 tie-breaker that this file's
   Open questions now cites — the same conflict resolved differently.

## Checked against

- `/tmp/nesdev-cache/Cycle_reference_chart.html` (the `Clock_rate` redirect —
  verified: the page header is "Cycle reference chart ... (Redirected from Clock
  rate)") — Dendy column `Like PAL`, UMC `UA6527P`, `26.60~ MHz ÷ 15 =
  1.773448 MHz`, master clocks per PPU dot `5`, PPU dots per CPU cycle `3`, CPU
  cycles per scanline `341 × 5÷15 = 113 2⁄3`, frame `35464`; PAL `341 × 5÷16 =
  106 9⁄16` and `PPU dots per CPU cycle 3.2` (evidence for Major 1).
- `/tmp/nesdev-cache/CPU_variants.html` — `UA6527P`: "Revisions without '-' in
  the date stamp have a ÷16 CPU divider"; "Revisions with '-' ... ÷15".
- `/tmp/nesdev-cache/PPU_memory_map.html` — `$3000-$3EFF` "Unused"; "usually a
  mirror of the 2kB region from $2000-2EFF".
- `/tmp/nesdev-cache/PPU_variants.html` — readability note on E-0 ("In this and
  all previous revisions, OAMDATA and palette RAM are not readable"), RP2C02A
  X=255 + asynchronous PPUMASK/PPUCTRL, RP2C02B X=255 only, `RC2C02C` ceramic.
- `/tmp/nesdev-cache/CPU_pinout.html` — letterless 17/24, E/G/H 15/24,
  2A07 19/32.
- `BreakingNESWiki/APU/pads.md` — all five I/O pads (`/IN1`, `/IN0`,
  `OUT2..OUT0`) marked `APU =>`.
- `BreakingNESWiki/APU/old_2a03.md` (rank-1 side: early 2A03 `M2` 15/24),
  `BreakingNESWiki/Dendy.md` (CPU "close to the NTSC NES", PAL video; 6527/6538
  clones; no divider stated), `BreakingNESWiki/PPU/umc6538.md` (6538 only).
- `Docs/NES_Famicom_Board_Revisions.md` §3/§4 — letterless `M2` 17/24 vs 15/24,
  2A07 19/32; PPU rows D/D-0/E/E-0 and the "this and all previous revisions"
  readability note; RP2C02A/B behaviour; 2C07 rows.
- `Docs/NES/NES_HDL_Model/nes.v` — top-level `LS139 DMX` (`First_A({CPU_A[15],
  M2})`, `Second_A({CPU_A[14],CPU_A[13]})`, `nROMSEL`/`WRAM_nCE`/`PPU_nCE`),
  `RAM WRAM` (`A[10:0]`), `RAM VRAM` (`{VRAM_A10,PPU_A[9:0]}`), `LS373`, `PPU`,
  `CIC`, `CartPort` with `CPU_A[14:0]` ("not using A15") and an **empty body**;
  `Readme.md` is a title + image stub.
- `Docs/UMC6527P/div_umc.v` — 185-line gate netlist, no stated divider ratio.
- `HDL/` — existence of all 13 listed modules confirmed; the changed §2/§9 claims
  they back were already re-read in round 1 (`HDL/PPU/pclk.v`, `cram.v`,
  `HDL/APU/core.v`, `pads.v`), and the current siblings quote the same gates.
- Siblings re-read for cross-file contradictions: `04-ppu-timing.md` §1/§10,
  `05-boards-and-io.md` §1 + Open questions, `04-ppu-memory-map.md` §1/§3/§14,
  `03-apu-overview.md` §4/§9, `02-cpu-core.md` §…, `04-ppu-registers.md` §9 —
  all CPU map sizes, PPU map sizes and the LS139 decode agree with §4/§5 here.
- Template check: `Status: revised (round 2)`; 355 lines (120–400); the five
  fixed headings present, in order, exact and unnumbered; all 11 content sections
  appear in the correspondence table.

## Notes for the next round

- Only §2 of `01-platform-overview.md` needs a wording change (Major 1); the
  numeric cells, the Dendy row, the memory maps and the index are correct. No
  spec file was edited by this role.
- If the two-revision-round cap (`Specs/Conventions.md` §10) is enforced, the
  alternative disposition for Major 1 is to record the chosen column convention
  in `## Open questions` with the reason. The section cannot be called
  `consensus` while §2's definition sentence ("CPU `PHI0` cycles : PPU dots",
  "16 CPU cycles per 5 dots") contradicts its own `3:1`/`16:5` cells.
- The sibling files named in the revision note were all last written before this
  file's 22:38:51 revision, so their round-1 content is expected; re-read them
  before acting on the cross-file list.

---

# Review: platform — round 1

Reviewed revision: `/mnt/c/Work/breaks/Specs/01-platform-overview.md`,
**Status:** adapted (round 1), 318 lines, md5 `45d3282489a192219d56b13fc6aeb3f2`
(mtime 2026-10-13 22:20:05). No previous report existed in this file.

## Verdict

revise

## Blocking

None. No claim was found that is factually wrong against a rank 1–3 source, and
the CPU/PPU memory-map tables are numerically correct and agree with every
dedicated section. Both majors below are labelling / currency defects, not
wrong silicon behaviour.

## Major

1. **§5 PPU memory map — the `$3000–$3EFF` row is presented as a board-decode
   fact, not as a rank-4 (NESdev) claim.** Line 115 reads
   `Unused; on the standard board a mirror of $2000–$2EFF | NESdev and the board
   decode, which ignores PPU A12 (...)`. No rank 1–3 source in this repository
   supports "the board decode ignores PPU A12":
   - `grep -rn "A12" BreakingNESWiki/ Docs/ HDL/` returns **zero** hits for a PPU
     A12 decode; the only `A12` is the 6502 address pin in
     `Logisim/Evolution/6502_Evo.circ`, and `Logisim/2C0x_PPU.circ` has only the
     pad name `PA12` (13 occurrences, no CIRAM logic).
   - `BreakingNESWiki/PPU/vram_ctrl.md` and `PPU/cram.md` describe the VRAM and
     CRAM decoders but never mention `$3000–$3EFF`, `$3F20–$3FFF` or A12.
   - `Docs/NES/NES_HDL_Model/nes.v` declares
     `module CartPort (... VRAM_A10, VRAM_nCE ...)` with an **empty body**, and
     its `Readme.md` is a two-line stub — the Provenance row
     `CPU memory map and board decode | Docs/NES/NES_HDL_Model/Readme.md | nes.v`
     therefore cites nothing for this claim.
   - `Docs/NES_Famicom_Board_Revisions.md` never mentions A12 or the `$3000` range.
   - The only source is NESdev: cached `PPU_memory_map` lists `$3000-$3EFF` as
     "Unused" in the table and, under *Hardware mapping*, "`$3000-3EFF` is
     **usually** a mirror of the 2kB region from `$2000-2EFF`" — rank 4, and
     explicitly conditional.
   The sibling `$3F20–$3FFF` row (line 118) is correctly marked `NESdev; ...`,
   so the two mirror rows of the same table disagree about their own rank.
   Fix: label line 115 the same way as line 118 (`NESdev; ...`), keep the CRAM /
   board-decode derivation in `## Open questions` (where it already is), and drop
   the `nes.v` attribution for a decode that file does not contain. Note that
   `04-ppu-memory-map.md` §1/§3/§14 has the same problem; fixing only this file
   will not remove the conflict.

2. **Dendy clock row and its Open question.** §2 line 43 gives
   `CPU PHI0 ≈ NTSC rate` and `CPU:PPU NTSC-like, nominally 1:3`; the Open
   question (lines 297–299) says "the crystal and CPU divider of the 6527P/NOAC
   parts are undocumented". Three problems:
   - The divider **is** documented on a page this file already cites: line 261
     lists `Cycle_reference_chart` for "PAL/Dendy". Cached
     `Cycle_reference_chart` gives the Dendy column as CPU `UMC UA6527P`,
     `26.60~ MHz ÷ 15 = 1.773448 MHz`, "Master clocks per PPU dot = 5",
     `CPU cycles per scanline = 341 × 5÷15 = 113⅔`, `CPU cycles per frame =
     106392 ÷ 3 = 35464`. Cached `CPU_variants` independently records the
     UMC UA6527P divider as ÷15 (÷16 on runs whose date stamp has no `-`).
     So the value is undocumented in the *BreakingNES Wiki*, not undocumented
     — the Open question's reason is inaccurate as written.
   - With that CPU clock the ratio is **exactly** 3 dots/CPU cycle, i.e. 1:3 —
     the platform's figure is right and the sibling is wrong: `04-ppu-timing.md`
     §10 (line 250) states `Dendy | ≈1.7897727 MHz | 5.3203424 MHz | 1 : ≈2.973`,
     which is 5.3203424 ÷ 1.7897727 and uses the NTSC CPU rate. The two spec
     files therefore disagree (1:3 vs 1:2.973) with no reconciling note;
     `05-boards-and-io.md` line 42 repeats the platform's looser wording.
   - The row is internally loose even without the sibling: `≈NTSC rate`
     (1.7897727 MHz) with a 5.3203424 MHz dot clock is 2.973, not 3.
   Fix: give the Dendy CPU clock as ≈1.7734 MHz (÷15, NESdev
   `Cycle_reference_chart`/`CPU_variants`; note some 6527P runs use ÷16), state
   "exactly 3 PPU dots per CPU cycle", and cross-reference the sibling, whose
   §10 needs the same correction.

## Minor

1. **§11 index — the table is now exact, but the trailing note is stale.**
   Directory listing gives 20 `Specs/0*.md` files, and the index lists all 20
   (19 pre-existing + `04-ppu-scrolling.md`); no existing file is omitted and no
   extra file is listed. However lines 209–212 still say "One listed file,
   `04-ppu-scrolling.md`, is planned but was not yet present in `Specs/` when
   this index was written". That file was created during this review
   (mtime 22:26:48, Status: draft), so the note is now false. The note was
   accurate when written; it just needs refreshing or deleting.
2. **§3 bus table, "APU I/O and PPU EXT | 5 in / 3 out; 4"** (line 68). The
   APU I/O pins are `OUT0–OUT2`, `/IN0`, `/IN1`, and `APU/pads.md` marks the
   direction of **all five** as `APU =>` (`/IN1` pin 35, `/IN0` pin 36,
   `OUT2..OUT0` pins 37–39); `03-apu-overview.md` §2 lines 45–46 likewise list
   all five as `out`. The cell also sits in a "Width" column where every other
   row gives a bit width. Use e.g. `5 (3 outputs, 2 read strobes)`.
3. **§2 line 41, NTSC PCLK period "186.2 ns".** 1 ÷ 5,369,318 Hz = 186.24 ns;
   `PPU/pclk.md` says "~0,186 µs" and `04-ppu-timing.md` §1 says `0.18624 µs`.
   The PAL figure in the same row is given to two decimals (187.96 ns), so
   either round both or state both exactly. (Dendy line 43's `≈5.320342 MHz` is
   likewise truncated against the PAL row's `5.3203424 MHz`.)
4. **Metadata `**Primary sources:**` (lines 10–16) is incomplete relative to the
   body and the `## Provenance` table.** Cited but unlisted: `PPU/vram_ctrl.md`
   (§5, Provenance), `PPU/par.md` (Provenance), `HDL/PPU/{vram_ctrl,cram,regs,
   pads,hv,hv_decoder}.v`, `HDL/APU/{regs,dma,pads}.v`, `HDL/Common/bustris.v`,
   `Docs/UMC6527P/div_umc.v`. `Docs/NES/NES_HDL_Model/Readme.md` is listed as a
   source for the board decode but contains only a title and an image link.
5. **§9 revision table, `RP2C02E-0` row (line 175).** "OAMDATA (`$2004`) and
   palette RAM are not readable" is placed only on E-0, but
   `Docs/NES_Famicom_Board_Revisions.md` §4 and NESdev `PPU_variants` both say
   "in this and **all previous** revisions" — it equally covers the
   letterless/A/B/C row above and the D revisions, which the table omits
   (`RP2C02D`/`-0`, `RC2C02C` are missing). Also, the asynchrony of
   PPUMASK/PPUCTRL is documented for `RP2C02A` (B shares only the X=255 bug),
   whereas line 174 attributes it to "A and B".
6. **Open questions line 296 calls the letterless M2 duty cycle "a rank-1 source
   conflict".** It is rank 1 (`APU/old_2a03.md`, early part 15/24) vs rank 3
   (`Docs/NES_Famicom_Board_Revisions.md`, letterless 17/24). The tie-breaker is
   already available and should be cited: NESdev `CPU_pinout` ("the original NTSC
   2A03 (no letter), M2 has a duty cycle of 17/24"; "2A03E, G, and H ... 15/24")
   and `CPU_variants` ("M2 duty cycle is 17/24 instead of 15/24") both side with
   the Docs file, against `old_2a03.md`.

## Checked against

- `BreakingNESWiki/APU/clkgen.md` — CLK/PHI/ACLK table (2A03 21477272 Hz,
  ~558.73 ns, ~1117.46 ns; 2A07 26601712 Hz, ~601.46 ns, ~1202.93 ns); ACLK1 /
  `/ACLK2` from PHI÷2 and non-overlapping; LFO 240/192 Hz and 120/96 Hz; SoftCLK
  PLA sequence. Confirms §2's ACLK = PHI0÷2, 894.886 kHz / 831.304 kHz.
- `BreakingNESWiki/PPU/pclk.md` — CLK 21.477272 / 26.601712 MHz, PCLK 5369318 /
  5320342.4 Hz, ÷4 / ÷5, `CLK` only in the phase generator, power-on latch state
  `x`. Confirms §2 and §7.
- `HDL/PPU/pclk.v` — RP2C02 four-latch ÷4, RP2C07 six-latch ÷5, frequency
  comments. Confirms §2.
- `HDL/APU/core.v` — `CLK_Divider`, `PHI0_tocore = ~sout[5]`, M2 NOR from
  `nval_4` and `PHI2`. Confirms §2's divider and M2 description.
- `HDL/APU/clkgen.v` — `ACLKGen` PHI1/PHI2 latch pair; `SoftTimer`.
- `BreakingNESWiki/APU/pads.md` — M2 high 1.5 CLK before core PHI2; `z` during
  reset in retail mode; A0–A15 and R/W `z` on reset; pin 30 `DBG`; all five I/O
  pins direction `APU =>`. Confirms §2/§7/§9 and contradicts Minor 2.
- `BreakingNESWiki/PPU/pads.md` — EXT0–3 direction from `$2000[6]`; `/RES`
  asynchronous latch held until `RESCL`, drives `RC`, power-on `x`; `/INT` → CPU
  `/NMI`; AD0–AD7/ALE/A8–A13 and `/RD` complement of `/WR`. Confirms §3/§6/§7.
- `BreakingNESWiki/6502/Readme.md` — PHI1 address+R/W / PHI2 data, PHI2 precharge
  to `$FF`, NMI falling-edge sensitive, `/RES` held several cycles. Confirms
  §2/§3/§6/§7.
- `BreakingNESWiki/PPU/fsm.md` — `/VSET` = VBlank Set, INT_FF, `$2002[7]` read
  clears it; EVEN/ODD logic implements the "NTSC Crawl". Confirms §2 and
  Observable 4.
- `BreakingNESWiki/PPU/pal.md` — "В PAL PPU отсутствует логика NTSC Crawl";
  **Regs**: `$2005`/`$2006` shared latch cleared by `RC` or `$2002` read; CPU
  register access is independent of PCLK. Confirms §2 and §6.
- `BreakingNESWiki/PPU/cram.md` + `HDL/PPU/cram.v` — rows `{CGA4,CGA1,CGA0}`,
  rows 0/4 one word line, `cram[CGA]` with a 5-bit `CGA`. Supports the
  `$3F10/$3F14/$3F18/$3F1C` alias and the `$3F20–$3FFF` mirror (line 118).
- `BreakingNESWiki/PPU/umc6538.md` — `VPLA_4` fires 50 lines later, drives
  `/VSET`. Confirms Observable 5.
- `BreakingNESWiki/APU/{core,old_2a03,pal,regs}.md` — `/DAA`+`/DSA` cut (Observable
  9); letterless: LFO not reloaded by reset, ACLK divider and OAM-DMA start FF
  not reset, no tonal noise, shorter noise table, no debug registers (Open
  question); 2A07 `DBG`→`RDY2`; `$401A` bit 7 `LOCK` freezes the generators.
- `BreakingNESWiki/MB/NES.md`, `MB/Famicom.md` — CIC failure ⇒ endless reset;
  Famicom RM1 pull-ups on `/IRQ`/`/NMI` and capacitor-defined `/RES`. Confirms
  §7 and Observable 10.
- `Docs/NES_Famicom_Board_Revisions.md` §1–§5 — Famicom/NES board tables, CIC
  table (3193/3193A, 3195A PAL-B incl. Korean "Comboy", 3197A PAL-A, 3196A,
  6113/6113B1 Brazil, 3198/3199 FamicomBox), glue logic (74LS139, 74LS373,
  40H368/74HC368 audio amp + PA13 invert, JIO BU3266/BU3270), PPU revision
  table, letterless M2 17/24. Confirms §1, §8, §9, §10.
- `Docs/NES/NES_HDL_Model/nes.v` — `CIC_CLK` 4 MHz comment; `.nHOST_RST(nRST)`
  feeds CPU `nRST` and PPU `nRST`. `CartPort` body empty; `Readme.md` a stub
  (evidence for Major 1).
- `Specs/04-ppu-memory-map.md` §1/§3/§14, `04-ppu-palette.md` §6,
  `04-ppu-registers.md` §9, `04-ppu-timing.md` §2/§10, `03-apu-overview.md`
  §2/§4/§9, `05-boards-and-io.md` §2/§4/§5 — every CPU map size/range
  (`$0000–$07FF` 2 KB; `$0800–$1FFF` 6 KB; `$2000–$2007` 8 B; `$2008–$3FFF`
  8,184 B; `$4000–$4013` 20 B; `$4014`/`$4015`/`$4016–$4017`; `$4018–$401F`
  8 B; `$4020–$5FFF` 8,160 B; `$6000–$7FFF` 8 KB; `$8000–$FFFF` 32 KB) and every
  PPU map size (`$0000–$0FFF`, `$1000–$1FFF` 4 KB; four 1 KB nametables;
  `$3000–$3EFF` 3.75 KB; `$3F00–$3F1F` 32 B; four aliases; `$3F20–$3FFF` 224 B)
  was re-derived by arithmetic and cross-read; all agree with the dedicated
  sections. Exception: the Dendy ratio (Major 2).
- NESdev cache `/tmp/nesdev-cache/`: `PPU_memory_map`, `CPU_memory_map`,
  `Cycle_reference_chart` (confirms `Clock_rate` redirects here),
  `CPU_variants`, `PPU_variants`, `CPU_Test_Mode`, `CPU_pinout`,
  `CIC_lockout_chip`, `Clock_rate`. **All 18 page names in §"NESdev Wiki
  correspondence" resolve to real pages** — including `PPU_variants`.

### The `PPU_variants` question

`PPU_variants` **exists**; the Adapter who believed otherwise is wrong. No
network probe was needed: `/tmp/nesdev-cache/PPU_variants.html` (68 KB) is the
real article "PPU variants" (official NTSC/PAL/RGB tables, RP2C02…RP2C02H-0,
RP2C07/2C07-0/2C07A-0, unofficial clones) and links back from `CPU_variants`'s
"See also". The platform's use of the name (correspondence line 270 and
Provenance) is correct, and `CPU_variants`, `CPU_Test_Mode`, `Famicom` and
`Cycle_reference_chart` are likewise real.

## Notes for the next round

- The file meets the structural contract: `Status: adapted (round 1)`, 318 lines
  (within 120–400), and the five fixed headings are present, in order, spelled
  exactly and unnumbered. The 11 numbered content sections each map to at least
  one row of the correspondence table; no orphan content section.
- **Concurrency warning.** The sibling specs were being rewritten while this
  review ran (`03-apu-frame-counter.md` 22:27, `03-apu-mixer.md` 22:27,
  `04-ppu-scrolling.md` created 22:26, `04-ppu-memory-map.md`/`04-ppu-sprites.md`
  /`04-ppu-timing.md`/`05-boards-and-io.md` 22:30). Cross-file findings above are
  pinned to that window; a revision round should re-read the siblings before
  acting on Major 2 or Minor 1. `01-platform-overview.md` itself was stable
  (22:20:05) throughout.
- Major 2 is a spec-set issue, not only a platform issue: the same Dendy ratio
  appears in `04-ppu-timing.md` §10 and, looser, in `05-boards-and-io.md` line
  42. Correcting the platform alone will leave the set contradictory.
- Minor 1 will resolve itself if the index note is deleted now that
  `04-ppu-scrolling.md` exists (Status still `draft`, so the index line is not
  yet "verified" either).

## Revision round 2

Revision of `Specs/01-platform-overview.md`, **Status:** revised (round 2),
355 lines, md5 `c422c0e18cc707cbbc861a13274172f6`. No blocking findings existed. All majors and
all minors addressed in place; no finding refuted, none deferred as unresolvable.

- Major 1: fixed — §5 `$3000–$3EFF` row now reads `NESdev; the board decode that
  ignores PPU A12 is not documented in this repository's rank 1–3 sources`, matching
  the sibling `$3F20–$3FFF` row. The `nes.v` attribution was dropped from the row
  and from the `## Provenance` table, which now has a dedicated
  `$3000–$3EFF nametable mirror` row with `—` in both source columns (noting the
  empty `CartPort` body of `nes.v`); the CPU-map/decode row was re-pointed to the
  top-level `LS139`/`RAM` wiring in `nes.v` and no longer cites the stub
  `NES_HDL_Model/Readme.md` as a source for the decode. `## Open questions` now
  states plainly that the `$3000` mirror is **rank-4 only** and that the palette
  mirror is rank 1–3-derived, and the "both are inferred" wording is gone.
- Major 2: fixed — §2 Dendy row gives the real figures: master clock like PAL
  (26.601712 MHz), `÷15`, `PHI0` 1.773448 MHz, PPU `÷5`, dot 5.3203424 MHz,
  `CPU:PPU` 3:1, with a bullet citing `Cycle_reference_chart` (UMC `UA6527P`,
  `26.60~ MHz ÷ 15 = 1.773448 MHz`, 113⅔ CPU cycles per scanline) and
  `CPU_variants` (÷15, ÷16 on some date stamps), and stating **exactly 3 PPU dots
  per CPU cycle**. The Dendy rows are marked NESdev rank-4 in the table bullet and
  in `## Open questions`, because no rank 1–3 source here documents the 6527P
  divider (`Docs/UMC6527P/div_umc.v` is a netlist with no stated ratio). The stale
  "crystal and CPU divider ... undocumented" Open question was replaced by two
  bullets: the rank-4 provenance of the Dendy figures, and an explicit note that
  `Specs/04-ppu-timing.md` §10 still states `≈1.7897727 MHz` / `1 : ≈2.973` and
  needs the same correction. The §2 ratio bullet also cross-references that §10
  row.
- Minor 1: fixed — the stale index note is replaced by "All 20 listed files are
  present in `Specs/`; `04-ppu-scrolling.md` ... is now included (Status: adapted
  (round 1))".
- Minor 2: fixed — §3 row width cell is now `5 (3 outputs OUT0–OUT2, 2 read
  strobes /IN0, /IN1); 4`, direction `APU → board`, and the note states that all
  five pins are driven by the APU (`APU =>` in `APU/pads.md`), superseding
  "5 in / 3 out".
- Minor 3: fixed — NTSC PCLK is `5.369318 MHz (186.24 ns)`; the Dendy dot clock
  is stated as `5.3203424 MHz (187.96 ns)`, no longer truncated.
- Minor 4: fixed — `**Primary sources:**` now adds `PPU/{vram_ctrl,par}`,
  `Docs/UMC6527P/div_umc.v`, `HDL/APU/{regs,dma,pads}.v`,
  `HDL/PPU/{vram_ctrl,cram,regs,pads,hv,hv_decoder}.v` and
  `HDL/Common/bustris.v`, and the Provenance note records that
  `NES_HDL_Model/Readme.md` is a title stub.
- Minor 5: fixed — §9 no longer puts the readability loss on E-0 alone: the row
  reads "**In this and all previous revisions** OAMDATA (`$2004`) and palette RAM
  are **not readable** — i.e. letterless, A, B, C, D, D-0 and E too"; the missing
  `RP2C02D`/`-0`/`RP2C02E` and `RC2C02C` markings are now present, the
  letterless/A row is split from B/C, and the PPUMASK/PPUCTRL asynchrony is
  attributed to A only (B keeps only the X=255 bug).
- Minor 6: fixed — the Open question now calls the letterless M2 duty a rank-1
  (`APU/old_2a03.md`) vs rank-3 (`Docs/NES_Famicom_Board_Revisions.md`) conflict
  and cites the tie-breakers NESdev `CPU_pinout` and `CPU_variants` (17/24);
  §9's letterless row states 17/24 with the conflict flagged.
- Extra (task assignment): the PAL `CPU:PPU` cell was `5:16`; it is now **`16:5`**
  (÷16 and ÷5 give 16 CPU cycles per 5 dots), with the NTSC cell normalised to
  `3:1` and a paragraph defining the column. A note in §2 records that
  `05-boards-and-io.md` §1 reproduces this table byte-for-byte and still carries
  the inverted cell, so that sibling — which already documents the same defect in
  its own Open questions — must follow.
- Sibling impact for the next round: `05-boards-and-io.md` §1 must copy the
  revised table (ratio `16:5`, Dendy row, `186.24 ns`) or deliberately diverge;
  `04-ppu-timing.md` §10 must correct the Dendy clock/ratio;
  `04-ppu-memory-map.md` §1/§3/§14 share the rank-4 `$3000–$3EFF` labelling issue
  noted in Major 1 and are untouched by this role.
- Scope: only `Specs/01-platform-overview.md` and this report were edited. No
  commit was made. Length is 355 lines, inside the `Specs/Conventions.md` §6
  120–400 target but above the REVISION role's 120–300 note; the growth is the
  added Dendy provenance, the split revision row and the two new Open questions.

## Revision round 3

Revision of `Specs/01-platform-overview.md`, **Status:** revised (round 3), 355 lines
(unchanged), md5 `9db0ddcfe21dcac0dde7220448d84889`. No blocking findings existed.

- Major 1 (round 2): fixed — §2's column definition now reads "the CPU:PPU divider
  ratio (`12:4`, `16:5`, `15:5`), numerically the **PPU dots per CPU cycle**", with
  `3:1` = 3 dots per CPU cycle and `16:5` = 16 dots per 5 CPU cycles = 3.2, citing
  NESdev `Cycle_reference_chart` ("PPU dots per CPU cycle 3.2"; the cached copy is
  `/tmp/nesdev-cache/Cycle_reference_chart.html`) and PAL `341 × 5÷16 = 106 9⁄16`
  CPU cycles per scanline. Both false "16 CPU cycles per 5 dots" phrases are deleted
  (the ÷16/÷5 divider sentence and the "`16:5` means …" gloss). The numeric cells
  (`3:1`, `16:5`, `3:1`), the Dendy row, the `04-ppu-timing.md` §10 cross-reference
  and the §2 consequence bullet are unchanged; the surviving convention is now the
  one the Verifier's minimal fix prescribed.
- Minor 1: fixed — the byte-identity claim is gone. §2 now says
  `05-boards-and-io.md` §1 "still carries the older table and must be aligned by a
  later cross-file pass", which matches the sibling's actual (stale) state instead of
  asserting a byte-for-byte reproduction.
- Minor 2: kept — the file remains exactly 355 lines, inside `Specs/Conventions.md`
  §6 (120–400); the added cross-file Open-questions bullet (7 lines) was offset by
  compacting the Dendy-clocks (−2 lines) and letterless-M2 (−1 line) bullets, so the
  count did not grow.
- Cross-file items recorded in `## Open questions` (new **Cross-file items for a later
  pass** bullet): `04-ppu-timing.md` §10 stale Dendy clock **and** its opposite ratio
  convention, `05-boards-and-io.md` §1 stale table, `04-ppu-memory-map.md`
  §1/§3/§14 `$3000–$3EFF` marking, `03-apu-overview.md` line 207 letterless `M2`
  wording. The round-2 "Dendy ratio disagrees with a sibling" bullet is subsumed by
  it; no new fact was introduced.
- Scope: only `Specs/01-platform-overview.md` and this report were edited. No other
  file was touched and no commit was made.
