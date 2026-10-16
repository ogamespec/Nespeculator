# Review: ppu-scroll — round 3

File: `Specs/04-ppu-scrolling.md` — 400 lines, `Status: revised (round 3)`.
Brief: `ppu-scroll` (`Specs/Review/agents/items.md`). Round 2 (kept below)
returned `revise` with 1 blocking, 1 major and 4 minor findings; the
`## Revision round 3` note at the end records the fixes. This round re-verifies
every fix against the primary sources (HDL, the test bench and the NESdev
cache) and re-reads the whole file for anything the revision disturbed. No spec
file was edited.

## Verdict

consensus

## Blocking

None.

## Major

None. Round-2 Major 1 (the stale `AT_ADR` request in
`04-ppu-memory-map.md` Open questions) is resolved: that sibling's item
"Attribute-fetch notation across specs" now reads that
`04-ppu-rendering.md` §3 and `04-ppu-scrolling.md` §2 "both agree with §4
here", and this file's `## Open questions` records that outcome and keeps the
one residual sibling wording (`04-ppu-memory-map.md` §3 "Coarse Y = 30/31 do
not wrap") for the sibling's next revision.

## Minor

Residual only; none of these blocks consensus.

1. **§5 (line 214), load-only wording for coarse Y = 30.** "`Z_TV` does not
   fire for those values, so they persist until the next vertical step, which
   runs 31 → 0 with the nametable *not* toggled" reads as if the next step from
   either loaded value is 31 → 0; for coarse Y = 30 the next step is the
   `else` branch 30 → 31 before 31 → 0. The pseudocode directly above and
   Observable #9 both state the correct rule, so this is cosmetic.
2. **§4 measurement vs. the §4 table (lines 151, 164, 173).** The measured fine
   Y still *decreases* at the step edge (5 → 4; 1 → 0) while the table says fine
   Y `+1`; the file records this honestly in `## Open questions` ("Fine-Y
   decrement on the pre-render line", suspected simulator artefact). Recorded,
   not new — listed here only so the residual is not lost.
3. **Review-file bookkeeping (not the spec).** The `## Revision round 3` note's
   body attributes one compensating deletion to "the §7 VRAM-restore sentence",
   while its addendum says §7 was left at its original wording and the only
   changes are §4 (+1) and §5 (−1). The net is 0 either way and the file is
   exactly 400 lines; no action needed.

## Checked against

- Round-2 Blocking 1 — **fixed.** §5 lines 195–201 now test the current value:
  `if coarse Y == 29: coarse Y = 0, v[11] ^= 1 / else if coarse Y == 31:
  coarse Y = 0 / else coarse Y += 1`, so 28 → 29, 29 → 0 + toggle, 30 → 31,
  31 → 0. Step-for-step identical to NESdev `PPU_scrolling` §Y increment
  (`if (y == 29) { y = 0; v ^= 0x0800 } else if (y == 31) { y = 0 } else
  y += 1`), to the file's prose (lines 204–210, "a step from 29 goes to 0 with
  `NTV` toggled", measured "25, 26, …, 29, 0") and to the Programming notes
  (lines 304–305). HDL: `w0 = ~(BLNK | w17 | n_TVO[0] | w20 | n_TVO[2] |
  n_TVO[3] | n_TVO[4])` with `w17 = ~TVIN`, `w20 = ~n_TVO[1]` (`tilecnt.v`
  lines 128, 132, 134) is `(TVO == 29) & TVIN & ~BLNK`; `Z_TV = ~(w14 | w15)`
  (line 122, registered through lines 135–137); `TileCounterBitReset`
  `val_out = w6 & (~Reset)` (line 296) with every `Tile_TV_Counter` bit taking
  `Reset(Z_TV)` (lines 356–360).
- Round-2 Minor 2 — **fixed.** No "initial value of 29" run remains anywhere in
  the file (`grep` for `initial value`, `29 → 30`, `30 → 31` finds nothing);
  §5 lines 211–217 attribute coarse Y 30/31 solely to the `$2005` second write
  with Y = 240–255 (240–247 → 30, 248–255 → 31 through `DB[7:3]`) loaded by the
  next vertical copy via `TVLOAD`, which is the correct load path.
- Round-2 Minor 3 — **fixed.** §4 lines 184–186 now reads "the next increment
  in the 8-dot chain would fall at dot 4 of the next line (dot 345), but that
  line restarts its fetches, so the next increment is at its dot 9": with this
  file's 341 dots (0–340), 337 + 8 = 345 = dot 4 of the next line, and dot 9 is
  the first increment of the restarted line. Arithmetic correct.
- Round-2 Minor 4 — **fixed.** Provenance line 361 now says "NESdev
  `PPU_variants` documents no scroll-register differences"; the string "brief"
  no longer appears in the file at all, so the misattribution is gone.
- Round-2 Minor 5 — **fixed.** §5 line 210 cites "`tilecnt_test.v` check 3
  models it". `HDL/Framework/Icarus/ppu/tilecnt_test.v` header property 3
  (lines 16–19) is exactly the fine-Y/coarse-Y step with "TVO 29 -> 0 together
  with a toggle of the vertical nametable bit"; the functional model
  (lines 149–158) implements the same `29 → 0 + toggle / 31 → 0 / else += 1`
  rule. Property 5 (lines 23–27) is the `NT_adr` decode and is no longer cited.
- Cross-file Open question (lines 394–397) — **accurate.** `Specs/04-ppu-memory-map.md`
  Open questions now says "...`04-ppu-rendering.md` §3 ... and
  `04-ppu-scrolling.md` §2 (`AT_ADR[9:6] = %1111`, "a constant field, not a
  function of `TVO`/`THO`") both agree with §4 here"; the quote matches §2
  lines 68–77 verbatim. The sibling's §3 line 60 does still say "Coarse Y =
  30/31 do not wrap (§4)", which this file's Open question names correctly.
- New-error sweep of the round-3 edits (§4 lines 184–189, §5 lines 203–219,
  Open questions lines 394–397): no contradiction introduced. §4 dot labels
  (increments 9…257, 329, 337; copy dot 258) still agree with §5, §6, §7,
  Observable #5 and the Programming notes; §5's step path and load path do not
  overlap.
- Template/Conventions: 400 lines exactly (120–400 ✓); `Status: revised
  (round 3)` accurate; five fixed headings exact, unnumbered, in order
  (lines 274, 290, 328, 343, 365); content sections 1–7 numbered; no conflict
  markers; file ends with a complete sentence.
- Cross-references resolve: `04-ppu-rendering.md` §1 (0-based dots), §2
  (line 84 still "coarse X increments (last tile); coarse Y increments" at dot
  256 — the recorded disagreement), §3 (lines 97, 104, 119–120; its own Open
  question records the same dot-label difference); `04-ppu-registers.md` §7
  (line 23 "copied into `v` until dot 257"), §10 (line 251 "Y increment at dot
  256 and dots 321–336", line 252 "Copy at dot 257 (H) and dot 280 (V)");
  `04-ppu-memory-map.md` §3, §4, §5.
- `BreakingNESWiki/PPU/tilecnt.md` — Tile Vertical limit 30 and reset "carry
  TVZ + 1 TVSTEP" (line 16) confirm the Open-questions wording exactly.
- NESdev cache `/tmp/nesdev-cache/PPU_scrolling.html` §Y increment (line 435
  `if (y == 29)`, the 31 branch and the "negative scroll" paragraph).

## Notes for the next round

- No further round is needed for this section: no blocking findings remain and
  the one major is resolved in the sibling and recorded here.
- The three minors above are residual and cosmetic; none should reopen
  `04-ppu-scrolling.md`.
- The file is still at the 400-line cap, so any future edit needs a
  compensating deletion.

---

# Review: ppu-scroll — round 2

File: `Specs/04-ppu-scrolling.md` — 400 lines, `Status: revised (round 2)`.
Brief: `ppu-scroll` (`Specs/Review/agents/items.md`). Round 1 (kept below)
returned `revise` with 2 blocking, 3 major and 5 minor findings; the
`## Revision round 2` note at the end of this file records the fixes. This
round re-verifies every fix against the primary sources (HDL and the NESdev
cache), then re-reads the revised text for anything the revision missed or
introduced. No spec file was edited.

## Verdict

revise

## Blocking

1. **§5, the coarse-Y pseudocode (lines 194–200) is wrong for the four values
   it exists to describe and contradicts the prose directly below it.** The
   right-hand column reads

   ```
   Y increment (fine Y wraps 7 -> 0):
     coarse Y += 1
     if coarse Y == 29: coarse Y = 0, v[11] ^= 1
     if coarse Y == 31: coarse Y = 0, v[11] unchanged
   ```

   Read literally (increment first, then test the *new* value) it gives
   coarse Y 28 → 29 → 0 + toggle (28 should step to 29), 29 → 30 (29 should
   become 0 + toggle), 30 → 31 → 0 without toggle (30 should become 31),
   31 → 32 (31 should become 0). The wrap tests must be made on the current
   value, exactly as the left-hand (coarse-X) column does.
   Evidence:
   - NESdev `PPU_scrolling` §Wrapping around → §Y increment:
     `int y = (v & 0x03E0) >> 5; if (y == 29) { y = 0; v ^= 0x0800 } else if
     (y == 31) { y = 0 } else y += 1` — the increment is the `else` branch,
     never an unconditional `+= 1` before the test.
   - The file's own prose, lines 202–209: "a step from 29 goes to 0 with `NTV`
     toggled", "the chain is cleared to 0 on the step out of 29", and the
     measured run "25, 26, …, 29, 0"; Programming notes lines 304–305: "the
     29→0 wrap toggling nametable Y".
   - `HDL/PPU/tilecnt.v`: `Z_TV = ~(w14 | w15)` (line 122) is built from
     `w0 = ~(BLNK | w17 | n_TVO[0] | w20 | n_TVO[2] | n_TVO[3] | n_TVO[4])`
     (line 134, i.e. `(TVO == 29) & ~BLNK & TVIN`), and every `TileCounterBitReset`
     in `Tile_TV_Counter` takes `Reset(Z_TV)` (lines 356–360) with
     `val_out = w6 & (~Reset)` (line 296): while the counter holds 29 the
     pending vertical step clears the output, so the step path never registers
     30.
   Fix: move `coarse Y += 1` into an `else` (or write NESdev's `if / else if /
   else`). One line; the prose needs no change.

## Major

1. **Cross-file: `04-ppu-memory-map.md` Open questions (line 285) still says
   this file's §2 is wrong, and the fix points the reader there.** §2 line
   80–82 now says the `tilecnt.v` zero tie is "an HDL defect documented in
   `04-ppu-memory-map.md` Open questions"; that section does document the HDL
   defect (line 284, correct), but its next item, "Attribute-fetch notation
   across specs", still states "`04-ppu-scrolling.md` §2 **still gives** an
   `AT_ADR` composition (`AT_ADR[9:8] = TVO[2:1]`, …) that has no constant in
   bits 6–9 … that sibling needs the same correction". §2 no longer contains
   that composition, so the sibling's request is stale and a reader following
   the pointer is told the file is still wrong. Either the sibling's line is
   updated in the cross-file pass or this file records the stale request in
   `## Open questions` (as it did for the dot-numbering disagreement, round-1
   finding 5). The round-2 revision was scoped to this file, so this is a
   final-pass item, not a new in-file error.

## Minor

2. **§5 line 215–217: "The round-1 29 → 30 → 31 → 0 run had an initial value
   of 29 and is a load-path observation."** Under the rule the same paragraph
   states (and the HDL, above), a counter that holds 29 and steps clears to 0;
   a sequence starting at 29 cannot reach 30 by stepping. The run is consistent
   with the load path only if 30 was *loaded* (`$2005` Y ≥ 240 → `t` = 30/31 →
   vertical copy → `TVLOAD`), which is not what "initial value of 29" says.
   Name the load, or drop the run.
3. **§4 line 184: "the increment that would fall at dot 345 happens on the
   *next* scanline's dot 9 instead."** With the file's own 341 dots per line
   (0–340, §4 line 130), dot 345 is dot 4 of the next line, not dot 9, so the
   two numbers do not reconcile as written. Say "the next increment in the
   8-dot chain (dot 4 of the next line)" or drop the counterfactual dot.
4. **Provenance line 361–363: "The brief's `PPU_variants` documents no
   scroll-register differences."** The brief (`items.md` §ppu-scroll) lists
   `PPU_scrolling`, `PPU_registers`, `PPU_rendering`, `PPU_nametables`; it does
   not name `PPU_variants`. Attribute the page as "NESdev `PPU_variants`" (as
   the sibling specs do) or drop the sentence.
5. **§5 line 209: "`tilecnt_test.v` (checks 3, 5) models it."** Property 3 of
   that test is the fine-Y/coarse-Y step with "TVO 29 -> 0 together with a
   toggle of the vertical nametable bit"; property 5 is the NT fetch-address
   decode (`NT_adr[13:12]`/`NT_adr[9:5] === TVO`), which checks the counter
   taps in the address, not the 29→0 step. Cite 3 (optionally "3, and 5 for
   the address wiring").

## Checked against

- Round-1 findings 1–10, direct re-check of the revised text:
  - **Blocking 1 (fine-Y clamp)** — fixed. §3 lines 114–121 scope `FV[2] = 0`
    to the "`$2006` first-write path … `W6_1` path only", state that the second
    `$2005` write sets all three bits, and add "The counter is not clamped".
    Observable #8 (line 285) and Programming bullet (lines 296–298) carry the
    same scope. `grep -i "clamp|0-3"` finds only "not clamped" and the scoped
    "fine Y 0–3" of the first `$2006` write; no residual 0–3 claim.
  - **Blocking 2 (`AT_ADR`)** — fixed. §2 lines 68–72 now give
    `[2:0]=THO[4:2]`, `[5:3]=TVO[4:2]`, `[9:6]=%1111`, `[11:10]=NTH/NTV`,
    `[13:12]=%10`, matching the `$23C0` formula on lines 75–80,
    `04-ppu-memory-map.md` §4 verbatim, and the HDL routing (`THO =
    {AT_adr[2:0], NT_adr[1:0]}`, `TVO = {AT_adr[5:3], NT_adr[6:5]}`,
    `AT_adr[10]=NTHOut`, `AT_adr[11]=NTVOut`; `tilecnt.v` lines 466–483). The
    `AT_adr[6..9] = 1'd0` tie (lines 459–462) is recorded in-line as an HDL
    defect with a pointer to the memory-map Open questions — Major 1 above is
    only about the staleness of the sibling's companion item.
  - **Major 3 (dot convention)** — fixed. §4 lines 132–136 state it exactly
    once (0-based dot in which the event is observed; NESdev "dot N" = the edge
    starting this file's dot N+1), and §6 (246–248), §7 (267–268), Observable
    #5 (282) and the Programming notes (302–310) all use it: vertical + last
    coarse-X increment at dot 257 = NESdev "dot 256", copy on the
    dot-257→258 edge = NESdev "dot 257". §4's own trace (line 164, "dot 257:
    X 4->5, fineY 5->4"; line 165, "dot 258: X 5->3 (copy)") matches. NESdev
    text confirmed: "At dot 256 … increments the vertical position", "At dot
    257 … copies all bits related to horizontal position", "begins at dots 328
    and 336 … at 8, 16, … 256".
  - **Major 4 (coarse Y / `Z_TV`)** — fixed. §5 lines 202–219 give one account:
    step path 29 → 0 with `NTV` toggled; 30/31 only via a `$2005` Y load
    (240–255) through `TVLOAD`, then 31 → 0 without the nametable switch; the
    old Open-questions item is replaced by the narrow `PPU/tilecnt.md`
    "limit 30" question. HDL: `w0` = `(TVO==29) & ~BLNK & TVIN` (line 134),
    `NTVIN = ~w13` with `w13 = ~(w6|w0)` and `w6 = NTHO & BLNK` (lines 112,
    120, 124) — so `NTV` toggles at 29 while rendering and not at 31;
    `Reset(Z_TV)` clears on the step out of 29 (lines 296, 356–360). NESdev
    §Y increment confirmed (29 → 0 + switch, 31 → 0 without switch, ">= 240 …
    'negative' scroll"). Minor 2 above is the only residual wording.
  - **Major 5 (cross-file dot 256/257)** — recorded. Open questions lines
    384–393 name `04-ppu-rendering.md` §2 ("both increments at dot 256") and
    `04-ppu-registers.md` §10 ("Y increment at dot 256" / "Copy at dot 257
    (H)"), both verified in the siblings.
  - **Minors 6–10** — fixed. §4 lines 186–188 now describe `THSTEP =
    ~(~(TSTEP | (F_TB & H0_DD)) | PCLK)` opening because `F_TB` is high at an
    even-H edge (`tilecnt.v` lines 41, 48); §6 lines 238–245 no longer say
    "wrong dot window" and match NESdev `PPU_registers` §Bus conflict ("the
    only safe time … is during blanking"); the Programming bullet (299–301)
    lists `9, 17, …, 249, 257` plus `329, 337`; Provenance (357–360) cites
    §2, §3–§6 only, matching the actual NESdev citations (none in §7); §2
    line 65 cites `04-ppu-memory-map.md` §4 as well as §3.
- Template/Conventions: five fixed headings present, exact spelling, no number
  prefix, in order; `Status: revised (round 2)` accurate; 400 lines
  (120–400 ✓, exactly at the cap — any further addition needs a compensating
  deletion); file ends with a complete sentence, no conflict markers, no
  duplicated sections.
- Cross-references resolve: `04-ppu-registers.md` §7–§10, `§7` (dot 257 copy /
  vertical copy wording), `§10` ("Y increment at dot 256", "Copy at dot 257
  (H)"); `04-ppu-rendering.md` §1 (0-based dots, +1-dot latch latency), §2
  (dots 1–256 = 32 tile periods; "256 | coarse X increments …; coarse Y
  increments"), §3; `04-ppu-memory-map.md` §3, §4, §5.
- HDL: `HDL/PPU/scroll_regs.v` `Fine_V` (lines 42–62: `w1 = W5_2` line 57,
  `FVx[0..1] <- DB[4..5]`, `FVx[2] <- 1'b0` only on the `W6_1` select, line
  61), `NT_Select`, `Tile_V` (`TVx[0..2] <- W6_2 DB[5..7]`, `TVx[3..4] <- W6_1
  DB[0..1]`); `HDL/PPU/tilecnt.v` `TileCountersControl`, `TileCountersControl2`,
  `TileCounterBitReset`, `Tile_TH_Counter`/`Tile_TV_Counter` taps;
  `HDL/Framework/Icarus/ppu/tilecnt_test.v` properties 3 and 5.
- NESdev cache: `PPU_scrolling` (§PPU internal registers, §Register controls,
  §At dot 256 / §At dot 257, §Between dot 328 and 256, §Wrapping around →
  §Coarse X increment / §Y increment, §$2007 reads and writes),
  `PPU_registers` (§Bus conflict, §Bit 0 race condition), plus the cached
  `PPU_rendering` material quoted in round 1.
- No new inconsistency found other than Blocking 1 (the §5 body is the section
  rewritten for Major 4 this round, so the pseudocode is the one place the
  revision can have disturbed) and the stale sibling line in Major 1.

## Notes for the next round

- Blocking 1 is a one-line reordering in §5; if it is fixed and Major 1 is
  either fixed in `04-ppu-memory-map.md` (one sentence) or recorded in this
  file's `## Open questions`, the next verdict can be `consensus`.
- Minors 2–5 are cosmetic but Minor 2 is a factual loose end in the paragraph
  that carries Major 4's fix; fix it in the same edit.
- The file is at the 400-line cap: at least as many lines must be removed as
  added.
- Do not re-open the settled items: Blocking 1 is now scoped only to the first
  `$2006` write; the `AT_ADR` composition, the single dot convention, the
  single coarse-Y account, and the recorded dot-256/257 disagreement are all
  verified consistent with the HDL and the siblings.

---

<!-- Rounds 1 and the Revision agent's round-2 note, kept verbatim below. -->

# Review: ppu-scroll — round 1

File: `Specs/04-ppu-scrolling.md` — 384 lines, `Status: adapted (round 1)`.
Brief: `ppu-scroll` (`Specs/Review/agents/items.md`). Gold: `Specs/Conventions.md`,
`Specs/_template.md`. Round 1 — no previous report existed, this file is new.

## Verdict

revise

## Blocking

1. **§3, §6, §8 and Observable #8 state a fine-Y clamp that the HDL does not
   implement and that the file's own measurement refutes.** §3 line 108–110:
   "`$2006` can never set fine Y above 3. The first `$2006` write puts `0` into
   `FV[2]` (`v[14]`)… *because the fine-Y counter is clamped to 0–3* so that the
   address cannot leave the PPU address space." §8 line 269 repeats "`$2006`
   cannot scroll to fine Y 4–7", and the Programming notes (line 280) tell an
   implementer to "MUST" include `FV[2] = 0` on the first `$2006` write — all
   correct — but the explanatory clause "the fine-Y counter is clamped to 0–3"
   is false and, taken as normative, is catastrophic: clamping fine Y to 0–3
   means pattern rows 4–7 of every tile are unreachable and vertical scrolling
   shows only half of each tile row.
   Evidence:
   - `HDL/PPU/scroll_regs.v` `Fine_V` (lines 53–62): `FVx[2]` gets `DB[2]` when
     the *second* `$2005` write is the active source (`w1 = W5_2`, line 57, 61)
     and `1'b0` only on the first `$2006` write path. So `$2005` second write
     sets all three bits; only `$2006` is limited to 0–3.
   - `BreakingNESWiki/PPU/scroll_regs.md` Fine VScroll table (lines 40–46) gives
     `W5/2 DB2 -> FV2` and `W6/1 0 -> FV2`; the clamp explanation ("иначе его
     значение бы выходило за предел адресного пространства PPU") is attached
     to the `$2006` path only.
   - NESdev `PPU_scrolling` §Register controls: `$2005` second write is
     "`t: FGH..AB CDE..... <- d: ABCDEFGH`" i.e. all 3 fine-Y bits;
     `$2006` first write is "`t: Z...... ........ <- 0`" i.e. that one bit only.
   - The file's own §4 measurement (lines 148 and 152) starts the scanline at
     "fine Y = 5" and logs "dot 257: fineY 5->4" — impossible if the counter
     were clamped to 0–3.
   Fix: delete the clamp sentence; say that the *first `$2006` write* writes
   fine Y bits 0–1 and forces bit 2 to 0 (fine Y 0–3 through `$2006`), while the
   *second `$2005` write* writes all three bits (fine Y 0–7). `04-ppu-registers.md`
   §8 line 210 and `04-ppu-memory-map.md` §5 line 110 are scoped to `$2006` and
   are correct; do not copy this file's wording.

2. **§2 (lines 68–71) gives an `AT_ADR` composition with no constant field, and
   it contradicts the `$23C0` formula printed three lines below it.** The block
   reads
   ```
   AT_ADR[9:8] = TVO[2:1]      AT_ADR[7:5] = TVO[0], THO[4:3]
   AT_ADR[4:0] = THO[2:0]      AT_ADR[12] = 0          AT_ADR[13] = 1
   ```
   There is no `AT_ADR[9:6] = %1111`. Line 75 then states the correct NESdev
   formula `attribute address = $23C0 | (v & $0C00) | ((v >> 4) & $38) | ((v >> 2) & $07)`,
   which hard-wires bits 9–6 to `1111`. The two statements are mutually
   exclusive for any coarse Y above 7 (`TVO[2:1]` is 0 until coarse Y ≥ 8).
   Evidence:
   - `HDL/PPU/tilecnt.v` lines 459–462: `AT_adr[6..9] = 1'd0` (a known
     translation defect, see below); the tile-counter bits are wired to
     `AT_adr[13:10]`, `AT_adr[5:3]`, `AT_adr[2:0]` (lines 463–473) — exactly the
     memory-map composition, not the §2 one.
   - `04-ppu-memory-map.md` §4 (lines 78–84) and §5 line 127 give
     `AT_ADR[9:6] = %1111`, `AT_ADR[13:12] = %10`, and §5 line 110 states the
     bit assignments are "confirmed bit-for-bit by `04-ppu-scrolling.md` §1" —
     that reconciliation is now false.
   - `04-ppu-memory-map.md` Open questions (line 278) explicitly names this
     file's composition as "not supported by the schematic, which drives exactly
     those four bits from the Vdd tie" and asks for a correction.
   - `PPU/tilecnt.md` only names `AT_ADR`/`NT_ADR` as the counters' outputs, so
     the composition must come from the schematic/HDL, where bits 6–9 are a
     constant field (one bare `Constant` fanning to four bits in
     `Logisim/Evolution/PPU_Evo.circ`, circuit `TileCounters_All`).
   Fix: `AT_ADR[9:6] = %1111` (and `AT_ADR[13:12] = %10`), with the tile bits at
   `[5:3]`/`[2:0]`/`[11:10]` as in `04-ppu-memory-map.md` §4; record the
   `tilecnt.v` zero-tie as the HDL defect the sibling file already documents.
   Note the value of the schematic Constant is not settled by this repository
   (bare `<comp lib="0" name="Constant"/>`, no `value` attribute; the HDL
   translated it as 0, the sibling file argues the convention is Vdd): the
   *statement* that bits 9–6 are a constant field is what §2 must not lose.

## Major

3. **§4 and Open questions: the NESdev↔HDL dot-label reconciliation is stated
   twice and the two statements disagree, so the file assigns NESdev's "dot
   256" to two different edges.** §4 line 120–123 maps NESdev's numbers one dot
   earlier ("its '8, 16, … 256' and 'dots 328 and 336' are this file's dots 9,
   17, … 257 and 329, 337"). Under that rule NESdev's "dot 256" is this file's
   dot 257. But the §4 table (lines 139 and 152) and Open questions (lines
   369–371) put
   the *vertical* increment on this file's dot 256 and say "NESdev names the
   same edges 'at dot 256' and 'at dot 257'". The same NESdev label therefore
   maps to this file's 257 (for the horizontal increment) and to this file's 256
   (for the vertical increment). Corroboration that the two events are on the
   same edge: the §4 measurement itself logs `dot 257: X 4->5, fineY 5->4` —
   X increment and fine-Y increment together, which matches NESdev's "increases
   … until 256" plus "fine Y is incremented at dot 256" treated as one edge.
   Also, the file's own gate explanation (lines 173–176) and HDL confirm the
   coarse-X increment happens on the edge that starts dot 257
   (`TileCountersControl`: `THSTEP = ~(~(TSTEP | (F_TB & H0_DD)) | PCLK)`), so
   the vertical increment must carry the same label.
   Fix: state the mapping once (keep it in §4), make it explicit which edge each
   label denotes (NESdev tick N = the edge starting this file's dot N+1 for the
   increment list 8…256/328/336 *and* for the vertical increment, while NESdev's
   copy "at dot 257" is this file's dot-257→258 edge), and move the duplicate
   note in Open questions to a cross-reference so a later edit cannot change one
   without the other. Verify the pair in the HDL (simulate `TSTEP`, `F_TB`,
   `H0_DD`) instead of quoting NESdev prose for the vertical edge.

4. **§5 and Open questions give two contradictory descriptions of the
   coarse-Y/Z_TV behaviour, and the test-bench numbers are not the ones listed.**
   §5 lines 198–200 say the HDL "keeps the NESdev wrapping (31→0, nametable
   unchanged) *and* adds a reset term, `Z_TV`, that forces the coarse-Y output
   to 0 whenever `TVO == 29`", i.e. it asserts a divergence from NESdev. Open
   questions lines 348–357 (item "Coarse-Y limit") then says the counter "was
   seen running 29 → 30 → 31 → 0", *and* says whether `$2005` 240–255 really
   reaches 30/31 is "unconfirmed" — while §5 line 191–193 states flatly that
   "30 and 31 are reachable only through the load path". Evidence:
   - `HDL/PPU/tilecnt.v` `TileCountersControl2`: `Z_TV = ~(w14 | w15)`; `w0`/`w1`
     (lines 117–118, 133) decode the coarse-Y outputs as `TVO == 29`
     (`n_TVO = {…}` line 361, and `w1 = ~(~BLNK | ~TVIN | n_TVO[0..4])`), and
     `Tile_TV_Counter` applies `Reset=Z_TV` to all five `TileCounterBitReset`
     bits (`val_out = w6 & ~Reset`, line 296). So the counter's non-loading
     sequence is 0…28, holds 29 while `Z_TV` is registered, then goes to 0: the
     net stepping behaviour (29→0 with `NTV` toggled by `NTVIN`, since
     `TVO == 29` also drives `NTVIN`) is NESdev's row-29 wrap, not a divergence.
     Loading 30/31 does bypass the reset, which is why the 29→30→31→0 run is
     possible — and that is the NESdev 31→0 "nametable unchanged" case.
   - §5's own measured trace (line 191–193) shows "25, 26, …, 29, then 0 with
     `NTV` toggling", which is the `Z_TV` sequence, not the 29→30→31→0 run.
   - `PPU/tilecnt.md` line 16 gives the Tile Vertical limit as 30 and the reset
     source as "carry TVZ + 1 TVSTEP" — the file's Open questions correctly
     records this discrepancy, but §5 then presents the 31→0 rule as what the
     HDL implements.
   Fix: one description. Recommend: the derived counter steps 29→0 with the
   nametable toggle (NESdev row 29, and what the test bench measured); coarse Y
   30/31 exist only after a `$2005` Y load of 240–255, and stepping from 31
   wraps to 0 without a nametable toggle (NESdev §Wrapping around). Say which
   measurement produced 29→30→31→0 and with which initial value, or drop the
   claim; do not leave both in the file.

5. **Cross-file conflict: the dot of the coarse-X increment at the end of the
   visible region.** This file §4 puts the coarse-X increment for the
   first sprite-fetch window at dot 257 (line 138) and the vertical increment at
   dot 256 (line 139); `Specs/04-ppu-rendering.md` §2 line 83 puts *both* coarse
   X and coarse Y increments at dot 256 ("`| 256 | — | coarse X increments (last
   tile); coarse Y increments |`"), and `Specs/04-ppu-registers.md` §10 line 250
   says "Coarse X increment at each tile fetch" / "Y increment at dot 256".
   `04-ppu-rendering.md` §2 also says dots 1–256 hold 32 tile periods, which
   fits this file's 31 increments inside 1–256 plus the dot-257 one only if the
   boundary is read as inclusive at 256; the two files must use one convention
   for that dot. Report as a cross-file finding: whichever wording is chosen,
   `04-ppu-rendering.md` §2/§3 and `04-ppu-scrolling.md` §4 must agree, because
   a reader implementing the schedule from `04-ppu-rendering.md` alone gets the
   32nd increment one dot early.

## Minor

6. **§4 line 172–176, `F_TB` wording.** "At dot 337 the step gate … is satisfied
   because `F_TB` has just fallen, while the fetch window before it is blocked."
   In `HDL/PPU/fsm.v` `F_TB = ~(ftb_latch2_q | fo_latch3_q)` and
   `TileCountersControl` gates on `TSTEP | (F_TB & H0_DD)`; the increment fires
   because `F_TB` is high at an even-H edge inside the dummy window, not because
   it "has just fallen". The conclusion (one increment, at dot 337, next at the
   next line's dot 9) is right; reword the mechanism or drop the explanation.

7. **§6 line 228, "wrong dot window" for the `$2006` bus conflict.** NESdev
   `PPU_registers` §Bus conflict says "if the second write to PPUADDR happens at
   specific times, at most one axis of scrolling will be set to the bitwise AND
   of the written value and the current value. The only safe time to finish the
   second write is during blanking". The trigger is a rendering-time second
   write (a bus-timing collision), not "the wrong dot window", and the safe
   window is blanking, not a dot range. This file already says "the safe window
   is blanking" in the next sentence — make the two consistent.

8. **§4 table row "329, 337 | two extra coarse X increments" vs the Programming
   notes.** The table is right (dots 329 and 337 each add one). The normative
   bullet (line 282–284) says "coarse X at the dots listed in §4 (9 + 8k for
   k = 0…30, then 257, 329, 337)", but `9 + 8k` for k = 0…30 is 31 dots ending
   at 249 — the file's own table stops the 8k series at 249 and adds 257. Write
   the list explicitly (`9, 17, …, 249, 257, 329, 337`, 32 increments per line)
   so the bullet cannot be read as "increment at 257 as well as 249+8k".

9. **Provenance (line 339–344) is self-contradictory on NESdev scope.** "no
   NESdev page was used to establish them [the dot numbers]. NESdev pages are
   cited in §2, §6, §7 and §10 only" — §2 does cite NESdev (the `NT_ADR`/`AT_ADR`
   formulas and `Mirroring`), and the file has no §10. Say "§2 (naming and the
   address formulas), §3, §6 and §7" or drop the section list. The substantive
   claim (dot numbers are HDL-measured) is correct.

10. **§2 should cite `04-ppu-memory-map.md` §4 as well as §3.** The `AT_ADR`
    composition belongs to that file's §4 (attribute tables), while line 83
    points only at §3 for the mirroring/board wiring. Traceability, not
    correctness.

## Checked against

- `BreakingNESWiki/PPU/scroll_regs.md` — all five tables (`Fine HScroll`,
  `Fine VScroll`, `Name Table select`, `Tile V`, `Tile H`) checked against §3's
  write table and §1's widths: the `$2005` 1st/2nd, `$2006` 1st/2nd and `$2000`
  data-bit → field mappings agree; the `W5/2 DB2 → FV2` row is the one the
  clamp sentence misreads.
- `HDL/PPU/scroll_regs.v` — `Fine_H`, `Fine_V`, `NT_Select`, `Tile_V`, `Tile_H`,
  `ScrollRegs`: bit-for-bit agreement with §3, including the shared `NTH`/`NTV`
  latches fed by `W0`/`W6_1` and `FVx[2] <- 0` only on the `W6_1` path.
- `HDL/PPU/tilecnt.v` — `TileCounterBit`/`TileCounterBitReset`, `Tile_TH_Counter`,
  `Tile_TV_Counter`, `Tile_FV_Counter`, `Tile_NT_Counters`,
  `TileCountersControl`/`2`, `TileCnt` `AT_adr`/`NT_adr`: §4's step/copy gates
  (`THSTEP`, `THLOAD`, `TVSTEP`, `TVLOAD`), §5's wrap logic, `Z_TV`, and the
  `AT_adr` composition (finding 2).
- `BreakingNESWiki/PPU/tilecnt.md` — counter table (widths, limits 32/30/2/2/8,
  carry/reset sources); confirms `TSTEP`-driven TV/TH increments indirectly via
  `PPU/vram_ctrl.md`.
- `BreakingNESWiki/PPU/regs.md` — shared first/second latch, `RC`/`$2002` clear,
  "Специальное примечание" (CPU interface not synchronised to `PCLK`) — §3 and
  §6 statements confirmed.
- `BreakingNESWiki/PPU/fsm.md`, `HDL/PPU/fsm.v` — `SC_CNT = ~(hblank | BLACK)`,
  `F_TB = ~(ftb_latch2_q | fo_latch3_q)`, `DelayedH` chain (`H0_DD`); used for
  findings 3, 6 and the "counters frozen while rendering disabled" claim.
- `HDL/PPU/bgcol.v` — `BGC_0`/`BGC_1` colour tap `disp[FH]`, FH shared across
  all four `BGC_*` cells: §1/§2 fine-X statements confirmed.
- `BreakingNESWiki/PPU/pamux.md`, `vram_ctrl.md` — PAMUX sources, `TSTEP`
  ("позволяет TV/TH счётчикам выполнить инкремент"), read buffer: §2/§6
  confirmed; `$2007`-during-rendering remains rank-4 only, as the file says.
- `Specs/04-ppu-registers.md` §7–§10 — write tables, `w` latch, `v`/`t` layout,
  dot-257/280 copy statements: agree with §1/§3; §10 line 252 "dot 280 (V)" vs
  this file's dots 280–304 is a wording difference, not a contradiction.
- `Specs/04-ppu-memory-map.md` §3, §4, §5 — "Coarse Y = 30/31 do not wrap"
  (line 60) is the only occurrence of that wording there; §3 line 94, §4 line 94
  and Observable #6 (line 216) all give the NESdev 31→0-without-nametable-switch
  rule, and Open questions line 278 flags this file's `AT_ADR` composition. The
  file's Open-questions note on the snippet is fair; the snippet is an
  incomplete sentence rather than a contradictory rule.
- `Specs/04-ppu-rendering.md` §1, §2, §3 — dot convention, fetch windows,
  "dot 256 = coarse X + coarse Y" (finding 5) and the 337–340 dummy fetches.
- NESdev cache `/tmp/nesdev-cache/`: `PPU_scrolling` (internal registers,
  register-control writes, dot 256/257/280–304/328–336 lists, §Wrapping around
  pseudocode, §Split X/§Split X-Y, §$2007), `PPU_registers` (Bit 0 race
  condition, Bus conflict), `PPU_rendering` (cycles 1–256, 257–320, 321–336,
  337–340, Cycle 0). Findings 1, 3, 4, 5, 7 rest on these.
- `Logisim/Evolution/PPU_Evo.circ` circuit `TileCounters_All` — one bare
  `Constant` fanning to four adjacent `AT_ADR` bits (10, 9, 8, 7 at the splitter
  routing) and the `TVO`/`THO`/`NTV`/`NTH`/`FVO` bit routes; used as evidence for
  finding 2 only (the constant's value is not settled here).
- Template/Conventions: five fixed headings present, exact spelling, in order,
  no numbering; 384 lines (120–400 ✓); `Status: adapted (round 1)` matches the
  round; `## NESdev Wiki correspondence` and `## Provenance` complete.

## Notes for the next round

- Findings 1 and 2 are one-sentence/short-block fixes but they are wording that
  a reader implements literally, so re-check the exact replacement text, not the
  section as a whole.
- Finding 3 is worth resolving by simulating the edge (Icarus) rather than by
  editing prose twice; the existing measurement numbers in §4 already contain
  the answer (the same dot shows both increments).
- Findings 4 and 5 are the two places where this file and a sibling currently
  cannot both be right; the revision round should touch the sibling's line or
  record the difference in this file's Open questions with the sibling named.
- `04-ppu-memory-map.md` Open questions line 278 asks *this* file to fix its
  `AT_ADR` composition; that request should not stay unanswered in the next
  round.

## Revision round 2

File: `Specs/04-ppu-scrolling.md` — 400 lines, `Status: revised (round 2)`.
Only that file was edited (no sibling touched, no commit).

- Blocking 1: fixed — §3 scopes the `FV[2] = 0` clamp to the *first `$2006`
  write* (`scroll_regs.v` `Fine_V`, `W6_1` path only) and states that the
  second `$2005` write sets `FV[2:0]` (fine Y 0–7); the counter is explicitly
  not clamped. Observable #8's cause column and the Programming bullet ("MUST
  use the field mapping") were rewritten to the same scope. No other clamp
  sentence exists in the file (the round-1 "§6" reference had none).
- Blocking 2: fixed — §2 composition replaced with `AT_ADR[2:0] = THO[4:2]`,
  `AT_ADR[5:3] = TVO[4:2]`, `AT_ADR[9:6] = %1111`, `AT_ADR[11:10] = NTH/NTV`,
  `AT_ADR[13:12] = %10`, consistent with the `$23C0` formula on the next lines
  and with `04-ppu-memory-map.md` §4. The `tilecnt.v` `AT_adr[6..9] = 1'd0`
  zero-tie is recorded in-line as an HDL defect with a pointer to the
  memory-map Open-questions entry.
- Major 3: fixed — §4 states the dot convention exactly once (events named after
  the 0-based dot they are observed in; NESdev "dot N" = the edge starting this
  file's dot N+1), makes the vertical and last coarse-X increments both dot 257
  (NESdev "dot 256") and the copy the dot-257→258 edge (dot 258), merges the
  table rows accordingly, and keeps the §4 trace (which logs `dot 257: X 4->5,
  fineY 5->4`).
- Major 4: fixed — §5 gives one account: `Z_TV` decodes `TVO == 29`, so the
  step path wraps 29 → 0 with the `NTV` toggle (measured 25…29, 0; modelled by
  `tilecnt_test.v` checks 3/5); 30/31 exist only through a `$2005` Y load of
  240–255 (`TVLOAD` bypasses `Z_TV`) and step 31 → 0 without a toggle. The
  round-1 29 → 30 → 31 → 0 run is stated with its initial value of 29 and
  attributed to the load path. The old Open-questions "Coarse-Y limit" item was
  replaced by a narrower one covering only the `PPU/tilecnt.md` "limit 30"
  wording.
- Major 5: recorded — new Open-questions item "Dot numbering and the cross-file
  dot-256/257 disagreement" names `04-ppu-rendering.md` §2 (both increments at
  dot 256) and `04-ppu-registers.md` §10 ("Y increment at dot 256" / "Copy at
  dot 257 (H)") for the final cross-file pass.
- Minor 6: fixed — §4 `F_TB` wording now says the gate `THSTEP =
  ~(~(TSTEP | (F_TB & H0_DD)) | PCLK)` opens because `F_TB` is high at an
  even-H (`H0_DD`) edge inside the dummy window.
- Minor 7: fixed — §6 bus conflict no longer says "wrong dot window": it is a
  rendering-time second write colliding with the PPU's own register access, and
  the only safe time is blanking.
- Minor 8: fixed — Programming bullet now lists `9, 17, …, 249, 257` (the 32
  tile-period increments) plus `329, 337` explicitly.
- Minor 9: fixed — Provenance now says §2 (naming, address formulas), §3–§6,
  drops the non-existent §10 and the incorrect §7.
- Minor 10: fixed — §2 cites `04-ppu-memory-map.md` §4 as well as §3.
- No findings refuted. Related dot-number wording in §6 (the `$2000` bit 0 race),
  §7 (full X/Y split) and Observable #5 was aligned to the same convention so
  the file has no second numbering scheme.

## Revision round 3

File: `Specs/04-ppu-scrolling.md` — 400 lines, `Status: revised (round 3)`.
Only that file was edited (the sibling `04-ppu-memory-map.md` was not touched,
no commit). Final round.

- Blocking 1: fixed — §5's coarse-Y pseudocode now tests the current value
  (`if coarse Y == 29: … else if coarse Y == 31: … else coarse Y += 1`,
  NESdev's `if / else if / else`), so it gives 28 → 29, 29 → 0 + `v[11]`
  toggle, 30 → 31, 31 → 0 without toggle. It again matches the prose below it
  ("the chain is cleared to 0 on the step out of 29"), the Programming notes,
  NESdev `PPU_scrolling` §Y increment and `tilecnt.v` (`Z_TV` decodes
  `TVO == 29`; `TileCounterBitReset` gates `val_out` with `~Reset`).
- Major 1 (cross-file staleness in `04-ppu-memory-map.md` Open questions):
  recorded, sibling not edited — §2 was already correct (`AT_ADR[9:6] = %1111`,
  `[5:3] = TVO[4:2]`, `[2:0] = THO[4:2]`, `[11:10] = NTH/NTV`,
  `[13:12] = %10`, matching the `$23C0` formula). The Open-questions item
  "Coarse-Y wrapping wording in `04-ppu-memory-map.md`" was merged into a new
  "Stale requests in `04-ppu-memory-map.md` Open questions", which now names
  both the sibling §3 "Coarse Y = 30/31 do not wrap" wording and the sibling's
  stale claim that this file's §2 "still gives" the old `AT_ADR` composition,
  with both marked for the sibling's next revision.
- Minor 2: fixed — the impossible "round-1 29 → 30 → 31 → 0 run had an initial
  value of 29" sentence is dropped. §5 now says only that the step path out of
  29 always clears to 0, so 30/31 appear only through the `TVLOAD` load of a
  `$2005` Y ≥ 240 already named in the same paragraph.
- Minor 3: fixed — §4 no longer says the missed increment happens at "the next
  scanline's dot 9" after naming dot 345. It now reads "the next increment in
  the 8-dot chain would fall at dot 4 of the next line (dot 345), but that line
  restarts its fetches, so the next increment is at its dot 9".
- Minor 4: fixed — Provenance now attributes the page as "NESdev
  `PPU_variants`"; the incorrect "The brief's `PPU_variants`" is gone (the
  brief `items.md` §ppu-scroll lists only `PPU_scrolling`, `PPU_registers`,
  `PPU_rendering`, `PPU_nametables`).
- Minor 5: fixed — §5 now cites "`tilecnt_test.v` check 3" only; property 3 is
  the 29 → 0 wrap with the `NTV` toggle, property 5 is the `NT_adr`
  fetch-address decode.
- No findings refuted.
- Length: still exactly 400 lines. The §4 dot-345 rewording (+1) and the
  Open-questions merge (+1) are offset by the §5 load-path sentence (−1) and
  the §7 VRAM-restore sentence (−1); no other section was reorganised. The
  template headings are unchanged and `Status: revised (round 3)` is set.
- Addendum (re-checked before finishing): the sibling `04-ppu-memory-map.md`
  was revised in parallel during this round; its Open-questions item now reads
  that `04-ppu-rendering.md` §3 and `04-ppu-scrolling.md` §2 "both agree with
  §4 here", so the stale `AT_ADR` request against this file is resolved in the
  sibling. The `## Open questions` item here was narrowed accordingly
  ("`04-ppu-memory-map.md` cross-references", 4 lines): it keeps the remaining
  sibling §3 "Coarse Y = 30/31 do not wrap" wording and records that the
  `AT_ADR` request is resolved. §7 was left at its original wording, so the
  only line-count changes are §4 (+1) and §5 (−1); the file is still exactly
  400 lines.
