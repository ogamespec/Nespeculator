# Review: cpu-instructions — round 2

## Verdict

**consensus**

Both round-1 findings are resolved and the five minors are fixed as claimed. I
re-ran the source checks independently rather than trusting the revision note:
the full 151-cell matrix still matches `Docs/6502/6502 Instruction Set.htm` and
`MOS6502_Opcodes/Readme.md` cell for cell (0 differences apart from the two
appendix misprints the spec already overrides), and the Revision agent's
refutation of the round-1 `DEC EA` item is **correct** — the only `EA` token in
the htm is the correct `implied NOP EA 1 2`. The revision did introduce one new,
small inaccuracy (the CDX capture count in the Open questions); it is a Minor
and does not change the verdict.

## Blocking

None. Round-1 Blocking 1 (`Visual_2A03`) is resolved.

## Major

None. Round-1 Major 1 (the §4 stack-pointer `D` claim) is resolved.

## Minor

1. **`## Open questions`, "NESdev source coverage" (line 358): "ten captures" is
   not the number of Wayback CDX captures for `CPU_unofficial_opcodes`.** The
   substantive claim (the page name is corroborated) and the earliest date are
   both right, but the count does not reproduce. Evidence:
   `curl "http://web.archive.org/cdx/search/cdx?url=nesdev.org/wiki/CPU_unofficial_opcodes&output=text"`
   returned **86 rows / 86 distinct timestamps**, of which 83 are HTTP 200 (one
   301, two 403), earliest `20220314185308` = 2022-03-14, latest `20260830000155`.
   Fix: delete the parenthetical count (or say "many captures"); an exact number
   is stale the moment the crawler runs again. The same sentence's `4.6 kB`
   `CPU_instructions` claim and the "only that row is unverified" conclusion are
   correct.

## Checked against

- **Round-1 Blocking 1 — `Visual_2A03`.** `grep` on
  `Specs/02-cpu-instructions.md`: the only remaining occurrence is line 356,
  "`Visual_2A03` does not exist", inside the sentence that names
  `Visual_circuit_tutorial` as the actual die-level source; the correspondence
  table row is now `| Per-cycle bus behaviour on the real die |
  Visual_circuit_tutorial |` (line 309). Independent CDX:
  `nesdev.org/wiki/Visual_2A03` returns HTTP 200 with an **empty body** (0 rows,
  no captures at all), while `nesdev.org/wiki/Visual_circuit_tutorial` returns 30
  captures, all HTTP 200, earliest `20220423170616`.
- **Round-1 Major 1 — stack pointer.** §4 line 148–149 now reads "The stack
  pointer is S (NESdev `S`; the repository's `regs.md` and `Readme.md` also write
  `S`)". Verified in the sources: `BreakingNESWiki/6502/regs.md` line 5
  ("Регистр S представляет собой указатель стека"), lines 18–21 (`S/ADL`,
  `S/SB`, `SB/S`, `S/S`), line 32; `BreakingNESWiki/6502/Readme.md` line 33
  ("S: указатель стека"); `HDL/Core6502/regs.v` line 5 (`S_SB, S_ADL, S_S,
  SB_S`), line 28 (`SRegBit sreg[7:0]`), line 52. No `D` stack-pointer naming
  exists; `grep -n '`D`'` on the spec returns nothing.
- **Minor 1 — `$DF` compare cycle.** `MOS6502_Opcodes/DF.md` sequence is T2, T3,
  T4, T5+T6, T7, T0, T1; T7 writes `DOR = DL` (original), T0 writes `DOR = ADD`
  (`DL + 0xFF` = original − 1), and **T1** is `Addr = PC++; ADD = AC + ~DL + 1`
  with decoder line `116: CMP (T1)` and T1 PHI2 commands `DBZ_Z, DB_N, ACR_C`.
  §6 line 248 now matches exactly; size/cycles 3 / 7 and the two-write RMW shape
  are unchanged.
- **Minor 2 — Implied/Implicit.** §1 line 41 now says "Implicit (NESdev's
  spelling; written `Implied` here and in `02-cpu-core.md`, no operand)". The
  cache `/tmp/nesdev-cache/CPU_addressing_modes.html` has the "Other addressing"
  row `Implicit  Instructions like RTS or CLC have no address operand…`;
  `02-cpu-core.md` line 170 records the same mapping.
- **Minor 3 — `4*` legend.** §2 lines 56–58 now state "`a,x`, `a,y`, `(d),y` —
  so `4*` is 4 base cycles, 5 when the index crosses a page", consistent with §3
  ("one extra cycle") and §4 ("4 cycles, 5 on a page cross"). No cell changed:
  still 151 cells, 16 `4*`, 7 `5*`, 8 `2†`.
- **Minor 4 — `6502 Instruction Set.htm` defect list.** Reproduced every listed
  defect: `DEC absolute  CE  3  3` (must be 6); `implied DEC CA 1 2` under "DEX
  Decrement Index X by One" (line 455); `implied DEC 88 1 2` under DEY (line
  465); `implied PHP 28 1 4` under PLP (line 663); LSR row `- + + - - -` (N
  unmodified); TXS row `+ + - - - -`. The dropped claim is also correct: `ASL
  Shift Left One Bit` and `ROL  Rotate One Bit Left` both have correct flag rows
  (`+ + + - - -`) and tables, so the spec rightly no longer calls them wrong.
- **Refutation of the round-1 `DEC EA` instruction — UPHELD.** A word-boundary
  search of the whole htm (`(^|[^A-Za-z0-9])EA([^A-Za-z0-9]|$)`) returns exactly
  one line, 606: `implied NOP EA 1 2` under `NOP  No Operation` (flag row
  `- - - - - -`). There is no `DEC EA` entry; the round-1 item does not
  reproduce. The reproducible single-line misprints are `DEC CA`, `DEC 88` and
  `PHP 28`, exactly as the spec now says. (Note the trap for the next verifier: a
  naive `grep -i EA` matches substrings like `Clear`/`head` and is meaningless.)
- **Minor 5 — `CPU_unofficial_opcodes`.** The spec no longer lists it as
  uncorroborated (line 357). CDX confirms the title is real (83 HTTP-200
  captures, earliest 2022-03-14) even though the local cache has no copy; only
  the `CPU_instructions` row is left unverified, which matches the 4 639-byte
  Wayback landing page (title `Wayback Machine`, no article body) and the live
  page's HTTP 403 (`curl https://www.nesdev.org/wiki/CPU_instructions` → 403).
- **§2 matrix unchanged and clean (full re-verification, not a spot check).**
  Parsed all 151 cells and compared against `Docs/6502/6502 Instruction Set.htm`
  (151/151 appendix rows parsed): 0 mnemonic differences (the htm's
  `relative BVC oper 70` is the appendix's own misprint; the spec correctly
  writes `BVS`), 0 cycle differences (the htm's `CE = 3` is its own defect; the
  spec correctly writes 6), 0 byte-length differences, 0 `*`/`†` marker
  differences. Compared against `MOS6502_Opcodes/Readme.md`'s 151-entry table: 0
  mnemonic/mode differences, no repo-only and no spec-only opcodes. Counts: 151
  cells, 151 unique opcode values, 56 unique official mnemonics, 16 `4*`,
  7 `5*`, 8 `2†` — identical to the numbers in the revision note, so the matrix
  did not change.
- **Structure.** 366 lines (inside 120–400); `**Status:** revised (round 2)`
  accurate for the round; the five fixed headings are present in order, exactly
  spelled and without number prefixes (`## Observable phenomena` L253,
  `## Programming / implementation notes` L269, `## NESdev Wiki correspondence`
  L299, `## Provenance` L311, `## Open questions` L327).
- **HDL citations re-spot-checked:** `bus_control.v` lines 131–132
  (`nor (temp8, X[71], X[72])`, `nand (PGX, temp8, ~BR0)`); `core_top.v` line 255
  (`assign BR0 = Decoder_out[73] & ~n_PRDY;`); `regs.v` line 5/28 `S…` nets.

## Notes for the next round

- The single Minor is a wording fix in one Open question (drop "ten captures");
  nothing else in the revised passages is wrong, so no re-derivation is needed.
- Stale cross-references elsewhere — **not** defects of this file, but they now
  contradict it: `Specs/02-cpu-core.md` line 395, `Specs/02-cpu-dma.md` lines
  301–302 and 327–330 still say "`02-cpu-instructions.md` still lists
  `Visual_2A03` and should be corrected in a revision round", and
  `Specs/Review/agents/mechanical-fixes.md` row 2 is still "pending". This file
  has now done its part; whoever owns those texts should update them (I did not
  edit them).
- The round-1 caution still stands: do not rewrite `4*` cells to `4+1` or `5*` to
  `5+1`; the base-cycle convention is intentional and matches NESdev.

---

# Review: cpu-instructions — round 1

## Verdict

**revise**

The mathematical core of this file is unusually clean: I machine-compared all
151 opcode cells (mnemonic, addressing mode, page-cross marker, base cycles)
against `Docs/6502/6502 Instruction Set.htm` and against
`MOS6502_Opcodes/Readme.md`, and found **zero** arithmetic, mapping or
marker errors. The cycle rules are internally consistent and every one of the
four known appendix typos is correctly *not* inherited. The two open cross-file
issues are real and are the reason this is not a consensus verdict: the §9
`Visual_2A03` entry names a page that does not exist (verified against the
Wayback CDX index, and already corrected in `02-cpu-core.md` and
`02-cpu-dma.md`), and §4 asserts something demonstrably false about
`regs.md`/`Readme.md`.

## Blocking

1. **`## NESdev Wiki correspondence` (line 306) and `## Open questions`
   (line 351): `Visual_2A03` is not a NESdev Wiki page.** The correspondence
   table cites `Visual_2A03` for "Per-cycle bus behaviour on the real die".
   Evidence: the local cache `/tmp/nesdev-cache/Visual_2A03.html` is a 4 634-byte
   Wayback landing page (`<title>Wayback Machine</title>`) with no article body,
   whereas `/tmp/nesdev-cache/Visual_2C02.html` is a 27 761-byte real article
   (`<title>Visual 2C02 - NESdev Wiki</title>`); a Wayback CDX query
   (`http://web.archive.org/cdx/search/cdx?url=nesdev.org/wiki/Visual*`) returns
   HTTP 200 for `Visual_2C02`, `Visual_circuit_tutorial` and `Visual6502wiki/…`
   and **no row at all** for `Visual_2A03`. The correct page is
   `Visual_circuit_tutorial`. This is not a new finding: `Specs/02-cpu-core.md`
   §9 already uses `Visual_circuit_tutorial` and its Open questions say
   "`02-cpu-instructions.md` still lists `Visual_2A03` and should be corrected in
   a revision round"; `Specs/02-cpu-dma.md` says the same; and
   `Specs/Review/agents/mechanical-fixes.md` row 2 lists the replacement as
   "pending". The file under review is now the only caller of a page the rest of
   the set has rejected, so the §9 table row cannot stand as "verified".
   Minimal fix: replace the page name in the table and drop the corresponding
   clause from the Open question.

## Major

1. **§4 (lines 146–147): the stack pointer is `S` in the repository sources, not
   `D`.** The text reads "The stack pointer is S (NESdev `S`; the repository's
   `regs.md` writes it `D`), so the stack address is `$0100 + S`." The
   parenthetical is factually wrong and nothing in the file depends on it.
   Evidence: `BreakingNESWiki/6502/regs.md` writes `S` throughout ("Регистр S
   представляет собой указатель стека", "S/ADL: поместить старое значение
   регистра S на шину ADL", "SB/S", "S/S"); `BreakingNESWiki/6502/Readme.md`
   line 33 writes "S: указатель стека"; `BreakingNESWiki_DeepL/6502/regs.md`
   translates the same sentence as "Register S is a stack pointer";
   `HDL/Core6502/regs.v` names the port and module `S_SB`, `S_ADL`, `S_S`,
   `SB_S`, `sreg[7:0]`, `SRegBit`, with debug nets `s`/`ss`. A `grep` for `D` as
   a stack pointer across `BreakingNESWiki/6502/*.md`, `Docs/` and
   `HDL/Core6502/*.v` returns nothing. `02-cpu-core.md` §3 already uses `S`
   ("| S | 8 | Stack pointer… | `$0100 + S` |"), so this file is also
   self-inconsistent with the sibling section. The likely origin is a
   transcription slip: the 6502 **D** flag (decimal) and the stack pointer were
   confused. Fix: delete the parenthetical or change it to "the repository's
   `regs.md` and `Readme.md` also write it `S`".

## Minor

1. **§6 `$DF` (line 245): the compare is attributed to the wrong cycle.** The
   row says "RMW frame; computes the original value − 1, writes it, and also
   compares A with the original value, setting C/Z/N". `MOS6502_Opcodes/DF.md`
   shows the comparison in the **last** cycle, after both writes: T2 `Addr =
   PC++`, T3 `Addr = PC++; ADD = X + DL`, T4 `Addr = (DL << 8) | ADD`,
   T5+T6 read, T7 `Write DOR = DL`, T0 `Write DOR = ADD`, **T1 `Addr = PC++;
   ADD = AC + ~DL + 1`** — i.e. the DCP-vs-A subtraction is the T1 ALU operation,
   not part of the write pair. The byte/cycle figure (3 / 7) and the RMW
   two-write pattern are both correct, so this is a wording defect only.

2. **§1 (lines 24–46) / `## Open questions`: "Implied" is a deliberate,
   documented deviation — say so here.** The cached NESdev page
   (`/tmp/nesdev-cache/CPU_addressing_modes.html`) uses **"Implicit"** in its
   table of "Other addressing" modes ("Instructions like `RTS` or `CLC` have no
   address operand…"). This file uses "Implied" in the §1 table and in the §2
   column header, with no note. The convention is intentional and is already
   recorded — `02-cpu-core.md` §6 says "NESdev's names are `Implicit` (written
   `Implied` here and in `02-cpu-instructions.md`)" — but a reader who lands on
   `02-cpu-instructions.md` first cannot tell whether "Implied" is a mistake.
   Add the one-line mapping (the existing sentence in §1 naming NESdev's
   spellings is the right place).

3. **§2: the `*` convention is only stated in terms of NESdev's `+`, never as a
   number.** §2 says `*` "marks the modes that take NESdev's '`+`'" and §1 says
   the mode takes "+1 if the index crosses a page", but §3's rule and §4's
   example ("4 cycles, 5 on a page cross") are the only places that say out loud
   that `4*` means 4 base / 5 maximum. Since this table is the one an emulator
   author will copy, add "(`4*` = 4 base, 5 across a page)" once to the legend
   so the two readings of the cell cannot be confused.

4. **`## Open questions`, "Docs/6502/6502 Instruction Set.htm errors"
   (lines 338–342): the list is right but the signpost is slightly off, and one
   defect is understated.** I confirmed in the Appendix A text: `DEC absolute`
   prints `CE  3  3` (must be 6); the single-line entries are mislabelled —
   `implied  DEC  CA  1  2` under the "Decrement Index X" heading,
   `implied  DEC  88  1  2` under "Decrement Index Y", `implied  PHP  28  1  4`
   under "Pull Processor Status from Stack", and `DEC  EA  1  2` under "No
   Operation"; the `LSR` flag row is `- + + - - -`, leaving N unmodified; and the
   `TXS` flag row is `+ + - - - -`. Two corrections for the wording: (a) the
   `ASL`/`ROL` sections are *not* mislabelled in the htm — `ASL  Shift Left One
   Bit`, `ROL  Rotate One Bit Left` and their flag rows/tables are all correct;
   if an Adapter saw a mislabel it is not reproducible from this file, so cite
   the `DEC CA`/`DEC 88`/`PHP 28`/`DEC EA` transpositions instead. (b) The DEC/DEX
   mislabel is a property of the **single-line** appendix entries, not of the
   `DEC` section header (which reads `DEC  Decrement Memory by One`).

5. **`## Open questions`, "NESdev source coverage" (lines 345–353): stet, with
   one wording fix.** The claim is right — `wc -c /tmp/nesdev-cache/CPU_instructions.html`
   = 4 639 bytes and it is a Wayback landing page — and it correctly refuses to
   claim NESdev verification for the §2 table. But the same paragraph then calls
   `CPU_unofficial_opcodes` "uncorroborated"; it is corroborated. A CDX query for
   `nesdev.org/wiki/CPU_unofficial_opcodes` returns ten archived 200 captures
   (earliest 2022-03-14), so the name is real even though the local cache has no
   copy. Split the two names: keep `CPU_instructions` as unverifiable, drop
   `CPU_unofficial_opcodes` from the uncorroborated list or note that CDX proves
   the title.

## Checked against

- `Docs/6502/6502 Instruction Set.htm` — parsed the 256-cell matrix and the
  Appendix A per-instruction tables. All 151 official cells (mnemonic, mode,
  byte length, base cycles, `*`/`**`) were compared cell-by-cell with §2; the
  only two textual disagreements are the appendix's own `CE` = 3 and `$70` =
  `BVC`, both of which the spec correctly overrides (`$70` = BVS). The
  §2 figures 2/3/4/4*/5/6/7 for read groups, 5/5/6 for indexed stores, 5/6/6/7
  for RMW, 3/5 for `JMP`, 6 for `JSR`/`RTS`/`RTI`, 7 for `BRK`, 5 for `BIT
  absolute` all match the appendix and/or the main matrix.
- `MOS6502_Opcodes/Readme.md` — machine-compared its 151-entry opcode table with
  §2: **0 semantic differences** in mnemonic and addressing mode; the tables
  cover the same 151 values (the repo table leaves `$00` as bare `BRK`, the spec
  fills mode `imp`).
- `MOS6502_Opcodes/{A9,8D,20,EE,DF,AB,EA}.md`, `brk.md` — frame tables. `A9`
  confirms the 2-cycle immediate frame; `8D` confirms `T0` = write A and `T1` =
  fetch next; `20` confirms the JSR order low, dummy stack read, PCH, PCL, high;
  `EE` confirms the read-modify-write Table shape (read, write, write); `DF`
  confirms the DCP cycle list; `AB` confirms `SB = DL & AC`, `X = SB`,
  `SetFlags(N,Z)`; `brk.md`'s "BRK Software Model" T0–T6 matches §4's 7-cycle
  frame row for row, including "Increment PC if a BRK instruction is executed
  directly".
- `BreakingNESWiki/6502/{decoder,dispatch,extra_counter,predecode,random_logic,alu_control,branch_logic,flags,flags_control,context_control,regs,regs_control,bus_control,interrupts,Readme}.md`
  — `decoder.md` line 35 and 200 confirm `/PRDY` → decoder line 73 "Branch T0"
  and the T0-skip-after-RDY rule; `extra_counter.md` confirms the three counters
  and "one T = PHI1 + PHI2"; `flags.md` confirms DB4 = B and DB5 = unused, that
  DB5 is not driven when P→DB, and that Z is tested on the whole DB; `interrupts.md`
  confirms the B flag forces opcode `$00` (unified BRK sequence);
  `random_logic.md`/`alu_control.md` map `PGX` = "Page X"; `regs.md` confirms the
  S naming (Major 1).
- `HDL/Core6502/{decoder,dispatch,bus_control,alu_control,flags,flags_control,regs,brk,core_top}.v`
  — `bus_control.v` lines 131–132 implement `nor (temp8, X[71], X[72])` and
  `nand (PGX, temp8, ~BR0)`, confirming §3's "PGX formed from X71/X72" and the
  `/PRDY` interaction; `core_top.v` line 255 `assign BR0 = Decoder_out[73] &
  ~n_PRDY`; `alu_control.v` lines 74–75 use `INC_SB` in the SB/ADH path;
  `regs.v` names every stack net `S…` (Major 1).
- `/tmp/nesdev-cache/CPU_addressing_modes.html` — confirms the abbreviation set,
  the `4+`/`5+` and `+` "oops" wording, the store-always-reads-writes
  explanation ("Store instructions always have this 'oops' cycle: the CPU first
  reads from the partially added address and then writes to the correct
  address"), the `(d),y` pointer high byte wrapping to `$0000`, and the term
  **Implicit** (Minor 2).
- `/tmp/nesdev-cache/{CPU_interrupts,Status_flags}.html` — confirm the
  `CLI`/`SEI`/`PLP` one-instruction IRQ delay vs `RTI`'s immediate effect,
  NMI-during-the-first-four-tics BRK hijacking, the B flag table
  (NMI 0 / IRQ 0 / BRK 1 / PHP 1), "The two bits with no CPU effect are ignored
  when pulling flags from the stack", and "always pushed as 1" for bit 5.
- `/tmp/nesdev-cache/CPU_instructions.html` (4 639 B, Wayback landing page) and
  `/tmp/nesdev-cache/Visual_2A03.html` (4 634 B, same) — confirm the two
  "unavailable" claims; CDX queries confirm `Visual_circuit_tutorial` (200) vs
  no `Visual_2A03` row (Blocking 1) and `CPU_unofficial_opcodes` (200, ten
  captures) (Minor 5).
- Structure: 359 lines (inside 120–400); headings are exactly
  `## Observable phenomena`, `## Programming / implementation notes`,
  `## NESdev Wiki correspondence`, `## Provenance`, `## Open questions` with no
  number prefix and in the required order, after the numbered content sections;
  `**Status:** adapted (round 1)` matches the rest of the set; the §6 table
  contains 151 unique opcodes, no duplicates, and exactly 56 unique official
  mnemonics as claimed.

## Notes for the next round

- The two fixes are one line each and neither touches a number: replace
  `Visual_2A03` with `Visual_circuit_tutorial` in §9 (and trim the Open
  question), and rewrite the §4 parenthetical. Everything else in this file that
  I checked is correct, so the revision round should be mechanical and should
  not re-derive the matrix.
- Do **not** "fix" `4*` cells to `4+1` or `5*` cells to `5+1` — the base
  convention ("base cycles" in §1, `*` for the conditional extra read) is
  consistent across §1–§4 and matches NESdev's own table; only the legend
  (Minor 3) needs the clarifying clause.
- If a second opinion on the §2 matrix is wanted, the cheapest independent path
  is `Visual6502wiki/6502_State_Machine` and `Visual6502wiki/6502_Timing_States`
  (both archived 200, per the CDX output), not `CPU_instructions`, whose capture
  is empty.

## Revision round 2

- Blocking 1: fixed — `Visual_2A03` replaced with `Visual_circuit_tutorial` in the
  §9 correspondence table and in the `## Open questions` "NESdev source coverage"
  entry (`Specs/02-cpu-instructions.md`); the remaining mention of `Visual_2A03`
  now only records that the page does not exist.
- Major 1: fixed — the §4 parenthetical now reads "the repository's `regs.md` and
  `Readme.md` also write `S`". Re-read `BreakingNESWiki/6502/regs.md` (S/ADL,
  S/SB, SB/S, S/S) and `BreakingNESWiki/6502/Readme.md`; no `D` stack-pointer
  naming exists.
- Minor 1: fixed — the §6 `$DF` row now places the compare of A with the original
  value in T1 (`ADD = AC + ~DL + 1`), after the two writes; size/cycles 3 / 7 and
  the RMW shape are unchanged (`MOS6502_Opcodes/DF.md` T1 row).
- Minor 2: fixed — §1's NESdev naming sentence now reads "Implicit (NESdev's
  spelling; written `Implied` here and in `02-cpu-core.md`, no operand)", per
  `/tmp/nesdev-cache/CPU_addressing_modes.html` (the "Other addressing" table).
- Minor 3: fixed — §2's legend adds "so `4*` is 4 base cycles, 5 when the index
  crosses a page". No cell changed: the matrix still holds 151 unique opcodes,
  16 `4*`, 7 `5*`, 8 `2†`.
- Minor 4: fixed — the Open question now lists only the reproducible appendix
  defects: `DEC absolute` (`CE`) printed as 3 cycles, the single-line misprints
  `DEC CA` (DEX), `DEC 88` (DEY), `PHP 28` (PLP), the `LSR` N row
  (`- + + - - -`) and the `TXS` flag row (`+ + - - - -`); the `ASL`/`ROL` claim is
  dropped and the text now states those sections are correct.
  **Refuted:** the round-1 `DEC EA` item does not reproduce —
  `Docs/6502/6502 Instruction Set.htm` contains exactly one `EA` row, the correct
  `implied  NOP  EA  1  2` under `NOP  No Operation` (line 606); its `EA` count is
  1 in the whole file. The reproducible single-line misprints are the three above.
- Minor 5: fixed — `CPU_unofficial_opcodes` removed from the uncorroborated list.
  The Open question now records that it is corroborated by the Wayback CDX index
  (ten captures, earliest 2022-03-14) and that only the `CPU_instructions` title
  is unconfirmed, so just that one row of §9 is unverified.
- No other file was edited; no `git commit` was run.
