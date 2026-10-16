# Post-fix verification — issue #1392 (independent Verifier pass)

**Assignment.** Re-verify condition 1 of `Specs/Review/CONSENSUS.md`: the
set-wide blocking finding **M1** (PPU dot numbering) and the cross-file items
fixed by the Final Fixer pass have not been through an independent Verifier
round. This pass reads the post-fix text, the four PPU sources, the HDL and the
indexes, and files findings. **No spec file was edited and no `git commit` was
made.** (The only file written is this report.)

## Verdict

**revise**

Condition 1 **cannot be closed.** M1's numbering itself is now consistent and
physically correct, and the four *Dot numbering* notes match, but the post-fix
text still carries (a) a wrong statement about the vertical increment in the
scrolling fetch schedule, (b) the old coarse-Y/fine-Y field mislabel in two of
the four dot-numbering files, and (c) two stale *Open questions* index rows —
including `OQ-PPU-68`, which is exactly the obsolete "cross-file dot
disagreement" entry the M1 fix was supposed to remove.

## Findings

### 1. `04-ppu-scrolling.md` — the vertical increment does not repeat during the 321–336 prefetch. **blocking**

- `04-ppu-scrolling.md:149` (schedule table, dot-257 row): "…the same increment
  fires at each tile boundary of the dots 321–336 prefetch".
- `04-ppu-scrolling.md:177–181` (bullet): "The vertical increment is not once
  per scanline: the same fine-Y step … fires at dot 256 … and again at each
  tile boundary of the dots 321–336 prefetch, i.e. the prefetch tiles advance
  the vertical position as if they were ordinary tile periods".
- **False**, by four independent lines of evidence:
  - NESdev `PPU_scrolling` (Wayback 2025-01-08) §"Y increment": "fine Y is
    incremented **at dot 256 of each scanline**, overflowing to coarse Y";
    the only other vertical event in that page is the 280–304 reload.
    §"Between dot 328 … and 256 of the next scanline" lists **horizontal**
    increments only.
  - NESdev `PPU_rendering` (Wayback 2025-01-10) §"Cycles 321-336": the two
    prefetched tiles are "the first two tiles for the _next_ scanline", fetched
    at the vertical position already advanced at dot 256 — an extra vertical
    step would fetch them one row too low on every line.
  - The repository's own self-check `HDL/Framework/Icarus/ppu/tilecnt_test.v`
    property 3 (`:16–19`) states "The fine-Y counter (FVO) advances by **one
    per scanline** (mod 8)", and its mid-line model (`:147–160`, checked at
    `:187–193`) would fail if the counter stepped again at 329/337. I ran it:
    `tilecnt_test: TEST PASS (66390 checks)` (Icarus 14.0, `-D RP2C02 -D ICARUS`).
  - `HDL/PPU/tilecnt.v:52,58`: `w13 = ~(E_EV | TSTEP)`,
    `TVSTEP = ~(w13 | PCLK)`. The vertical chain (FV + TV + NTV,
    `tilecnt.v:353–360, 482–484`) therefore steps only on `E_EV` — the HPLA_7
    (H = 255) strobe through two latches (`fsm.v:450–451, 491`) — or on the
    `$2007` `TSTEP` pulse (`vram_ctrl.v:97`). Neither exists in 321–336.
  - Direct parse of the `tilecnt_test.vcd` produced by that run: the top-level
    `n_FVO[2:0]` changes once per scanline (spacing ≈ 62.7 µs) at H ≈ 256;
    there is no transition at H = 321–340.
- The claim also contradicts the file's **own** measured transcripts: `:161–165`
  and `:171–173` show the fine-Y change only at dot 257 (and the 281 reload),
  and only coarse-X changes at 329 and 337.
- Not introduced by the M1 renumbering (the bullet and the table cell predate
  it), but it is unrecorded in `## Open questions` and is in the same schedule
  table the M1 fix rewrote.

### 2. `04-ppu-rendering.md` and `04-ppu-registers.md` — the dot-256 vertical increment is mislabelled "coarse Y". **major**

The new normative notes say the dot-256 event is the **fine-Y** increment
(`04-ppu-rendering.md:29–33`, `04-ppu-registers.md:239–243`,
`04-ppu-timing.md:62–66`, `04-ppu-scrolling.md:130–134`), but three table cells
call it a coarse-Y increment:

- `04-ppu-rendering.md:88`: `| 256 | — | coarse X increments (last tile); coarse Y increments |`
- `04-ppu-rendering.md:101`: "Dot 256 is not idle: coarse X wraps and coarse Y is incremented."
- `04-ppu-registers.md:257` (Coarse Y row of the §10 field table): "Y increment at dot 256 and dots 321–336", while the Fine Y row (`:259`) does not mention the per-tile increment at all.

Evidence: NESdev `PPU_scrolling` §"Y increment" — "fine Y is incremented at dot
256 …, overflowing to coarse Y"; coarse Y changes only on a fine-Y 7→0 wrap
(`04-ppu-scrolling.md:194–199, 202–208`). HDL: `Tile_FV_Counter` and
`Tile_TV_Counter` share `TVSTEP`, and the coarse-Y counter's `carry_in` is the
fine-Y carry (`HDL/PPU/tilecnt.v:482–484`, `TileCountersControl2` TVIN at
`tilecnt.v:110–130`), so only fine Y changes on a normal dot-256 step. The
pre-fix round-3 report had already quoted the same rendering wording
(`Specs/Review/ppu-scroll.md:99–101, 117`); the M1 note now makes it a genuine
contradiction rather than a recorded label difference.

### 3. `Specs/06-open-questions.md:178` (`OQ-PPU-68`) — obsolete cross-file dot entry survives. **major**

The row still reads "…while `04-ppu-rendering.md` §1 defines dot = `H − 1` and…"
and is still `open`. No file defines `dot = H − 1`: all four notes read
"dot = `H` = 0–340" (`04-ppu-rendering.md:25, 29`; `04-ppu-registers.md:239`;
`04-ppu-timing.md:62`; `04-ppu-scrolling.md:130`). The Final Fixer replaced that
Open-questions item inside `04-ppu-timing.md` only (its log claims so at
`Specs/Review/global-consistency.md:382–385`); the aggregated index row was not
updated. This is precisely the "obsolete cross-file dot disagreement entry"
condition 1 asked to confirm gone.

### 4. `Specs/06-open-questions.md:181` (`OQ-BRD-02`) — stale `CPU:PPU` row (same class). **major**

The row says `04-ppu-timing.md` §10 "still writes CPU cycles : dots
(`1 : 3`, `5 :…`)". Post-fix §10 (`04-ppu-timing.md:246–252`) writes
`3 : 1` / `16 : 5` / `3 : 1` (PPU dots per CPU cycle) — the same convention as
`01-platform-overview.md` §2 and `05-boards-and-io.md` §1. The Final Fixer
aligned the files but not the index row, so the index asserts a contradiction
that no longer exists. Same "fixer edit not propagated" defect as Finding 3.

### 5. `04-ppu-scrolling.md:142` — `(HDL edge)` header covers nominal rows. **minor**

The §4 table header is `| Dot(s) (HDL edge) |`, but the fetch rows
(`:144–147`, dots 1–2 … 255–256) and the dummy-fetch row (`:155`, dots
337–340) use the NESdev nominal dots, not the +1 HDL-edge values. The §4 note
(`:133–134`) says the label marks the *measured* rows only; per-cell labelling
(or a split header) would remove the ambiguity.

### 6. `01-platform-overview.md:218` — §11 index row describes the wrong APU range. **minor**

The index row for `03-apu-overview.md` says "the `$4000–$4017` register map",
while that file's scope and §4 heading say `$4000–$401F`
(`03-apu-overview.md:4, 95`) and `Specs/Readme.md:23` likewise says
`$4000–$401F`. The `$4018–$401A` test registers are covered by the file, so the
index row understates it.

### 7. `Specs/Readme.md` file table — self-row absent. **minor / note**

The 24-row table matches the 25-file inventory except that it does not list
`Readme.md` itself; no stale or phantom rows. All 31 relative links in
`Specs/Readme.md` resolve (including the directory links `Review/` and
`Review/agents/`); the only external link is the issue URL.

## Checked against

**1. M1 dot numbering (all four files).**

- Wording: `04-ppu-rendering.md:29–33`, `04-ppu-registers.md:239–243` and
  `04-ppu-timing.md:62–66` are **verbatim identical**; `04-ppu-scrolling.md:130–134`
  differs only by the local self-reference ("in §4") and the extra sentence
  about the `(HDL edge)` label. Materially identical. ✓
- Normative labels: all four say dot = `H` = 0–340, fine-Y + last coarse-X
  increments at dot 256, `t`→`v` copy at 257 (effect 258), dummy fetches
  337–340. NESdev `PPU_scrolling` §"At dot 256" / §"At dot 257" and §"Y
  increment" give the same 256/257/258 sequence. ✓
- `HPLA_23` = 340 claim: decoded from `HDL/PPU/hv_decoder.v:58`
  (`dec_out[23] = ~|{d[3],d[5],d[6],d[9],d[10],d[13],d[14],d[17],d[18]}` →
  H8=1,H7=0,H6=1,H5=0,H4=1,H3=0,H2=1,H1=0,H0=0 = 340); matches
  `BreakingNESWiki/PPU/hv_decoder.md:47`. ✓
- `(HDL edge)` labelling: present on the measured rows/values in
  `04-ppu-scrolling.md:148–149, 162, 171–172, 178, 227, 265`; the rendering §2
  table is explicitly nominal (`:75`, "Dot (= H)"); timing §6's settled windows
  are not labelled but their one-`PCLK` latch residue is declared in its Open
  questions (`04-ppu-timing.md:398–400`). Finding 5 covers the scrolling header.
- Self-consistency of the each file's own tables: registers §10 agrees with the
  note on the copy dot `257` (`:258`) and the H copy (H) / V copy (280) split;
  rendering §2's dot numbers agree with the note except for the field name
  (Finding 2); timing §3 rows are raw `HPLA` H values and match the note;
  scrolling §4 agrees except for Finding 1. The obsolete file-level
  "cross-file disagreement" Open-questions entries are replaced by current-state
  entries (`04-ppu-rendering.md:398–400`, `04-ppu-scrolling.md:382–386`,
  `04-ppu-timing.md:396–400`); the index row is the exception (Finding 3).

**2. Physics of the renumbering.**

- Increment edge: `THSTEP = (TSTEP | (F_TB & H0_DD)) & ~PCLK`
  (`HDL/PPU/tilecnt.v:41–48`), with `F_TB` the HPLA_12 (H mod 8 = 6,7) strobe
  through the two-latch chain (`fsm.v:460–461, 496`), i.e. the coarse-X step is
  the F/TB tile boundary at H mod 8 = 0; the counter update is observed at the
  next PCLK edge (H ≡ 1 mod 8), which is exactly the file's measured
  9, 17, …, 249, 257 sequence (`04-ppu-scrolling.md:148, 161–162`). Nominal
  NESdev dots are 8, 16, …, 256 (`PPU_scrolling` §"Between dot 328 …").
- Vertical step: `TVSTEP = (E_EV | TSTEP) & ~PCLK` (`tilecnt.v:52, 58`), with
  `E_EV` = HPLA_7 (H = 255) through two latches (`fsm.v:450–451, 491`), i.e.
  one vertical step per line at the dot-256 boundary; measured at the following
  edge (`04-ppu-scrolling.md:162, 171`). Nominal: NESdev dot 256.
- Copy: `THLOAD` is derived from `E_EV` (two stages) gated by `SC_CNT`
  (`tilecnt.v:47, 51, 57, 61–64`), i.e. the horizontal `t`→`v` load lands one
  edge after the step; measured at H = 258 (`04-ppu-scrolling.md:150, 163, 172`).
  Nominal: NESdev dot 257.
- Conclusion: the relative order — vertical + last coarse-X increment, then the
  horizontal copy on the next dot — is preserved; the normative labels are the
  NESdev cycles 256/257 and the +1 HDL-edge annotation is coherent. The
  renumbering did not break the physics; the only physics defect is Finding 1.
- `BreakingNESWiki/PPU/fsm.md` and `hv_decoder.md` were read directly (both are
  prose/mask tables with no dot statements beyond the HPLA rows; rows 1 = 256,
  11 = H mod 8 ∈ {0,1}, 12 = {6,7}, 23 = 340 in `hv_decoder.md:25, 35–36, 47`,
  matching the decoded `hv_decoder.v` equations).

**3. Indexes.**

- `Specs/06-open-questions.md` and `Specs/nesdev-mapping.md` both exist.
  `01-platform-overview.md` §11 (`:210–232`) lists both plus all 20 sections and
  states the indexer exception correctly; the only text defect is Finding 6.
- `Specs/Readme.md`: 24 table rows vs 25 `Specs/*.md` files (only `Readme.md`
  itself is unlisted); every relative link resolves (31 checked
  programmatically); the `## Status` paragraph matches the consensus record
  (20 sections, 156 OQ entries, 25 `blocking-emulation`).

**4. Statuses and review verdicts.**

- All 20 per-section files carry exactly one `**Status:** consensus (round N)`
  line (`01` r3, `02-cpu-core` r2, `02-cpu-dma` r2, `02-cpu-instructions` r2,
  `03-apu-dmc` r2, `03-apu-frame-counter` r3, `03-apu-mixer` r2,
  `03-apu-noise` r2, `03-apu-overview` r2, `03-apu-pulse` r1,
  `03-apu-triangle` r1, `04-ppu-memory-map` r3, `04-ppu-overview` r2,
  `04-ppu-palette` r2, `04-ppu-registers` r2, `04-ppu-rendering` r3,
  `04-ppu-scrolling` r3, `04-ppu-sprites` r3, `04-ppu-timing` r2,
  `05-boards-and-io` r3). None missed; `06-open-questions.md`,
  `nesdev-mapping.md`, `Readme.md` and `Conventions.md` have none by design.
- The newest round at the top of each of the 14 `Specs/Review/*.md` section
  reports is `consensus` (`platform` r4, `cpu-core` r2, `cpu-dma` r2,
  `cpu-instructions` r2, `apu-overview` r2, `apu-pulse` r1, `apu-noise` r2,
  `apu-fc-mixer` r3, `ppu-overview-regs` r2, `ppu-memmap` r3,
  `ppu-render-sprites` r3, `ppu-scroll` r3, `ppu-palette-timing` r2,
  `boards-io` r3). No section file claims consensus while its latest brief
  verdict is not consensus. `Specs/Review/global-consistency.md:32–34` is the
  set-level `revise` snapshot and remains the condition this pass was to
  discharge.
- The statuses of the four M1 files predate the Final Fixer pass (the stale
  report condition); with Findings 1–2 unrecorded, those statuses are not
  presently supported by the post-fix text.

**Sources read for this pass:** `BreakingNESWiki/PPU/fsm.md`,
`BreakingNESWiki/PPU/hv_decoder.md`, `BreakingNESWiki/PPU/tilecnt.md`,
`BreakingNESWiki/PPU/hv.md`; `HDL/PPU/fsm.v`, `hv_decoder.v`, `hv.v`,
`tilecnt.v`, `scroll_regs.v`, `vram_ctrl.v`, `ppu_top.v`; the Icarus
testbenches `tilecnt_test.v` (+ one executed run) and the parsed
`tilecnt_test.vcd`; NESdev `PPU_rendering` and `PPU_scrolling` (Wayback
snapshots 2025-01-10 / 2025-01-08); all four post-fix PPU files, the two
indexes, `Specs/Readme.md`, and the 14 section reviews plus
`global-consistency.md`.

## Post-consensus fixer pass

Applied by the Final Fixer agent against every finding above. Each line is
`file: what changed -> why (evidence)`. No `Specs/Review/CONSENSUS.md` edit and no
`git commit`; `Specs/Readme.md`, `Specs/Conventions.md`, `Specs/_template.md` and
the role contracts are untouched. Line counts after the pass: `01-platform-overview`
357, `03-apu-overview` 307, `04-ppu-rendering` 400, `04-ppu-registers` 357,
`04-ppu-scrolling` 398, `06-open-questions` 345.

**Finding 1 (blocking) — `04-ppu-scrolling.md`, the vertical increment.**

- §4 table, dot-257 row: "the same increment fires at each tile boundary of the
  dots 321–336 prefetch" -> "**one fine-Y step per scanline**", nominal 256 /
  measured 257 -> NESdev `PPU_scrolling` §Y increment: fine Y is incremented
  *at dot 256 of each scanline*; no other vertical event exists in 321–336.
- §4, first bullet: "The vertical increment is not once per scanline … again at
  each tile boundary of the dots 321–336 prefetch" -> once per scanline at dot
  256 (measured 257, HDL edge), and the 321–336 prefetch loads the next line's
  first two tiles at the already-advanced position -> NESdev `PPU_rendering`
  §Cycles 321–336 (the two tiles are "for the *next* scanline"); an extra step
  would fetch every line one row too low.
- §4, bullet lead-in "Three details" -> "Four details" -> the corrected bullet
  is a genuine deviation from the naive 8-dot model.
- Programming notes, "MUST increment the vertical position" bullet: removed "and
  at the tile boundaries of the dots 321–336 prefetch" -> same evidence; this
  occurrence was not in the finding list but is the same false claim.
- Evidence re-verified, not just quoted: I re-ran
  `HDL/Framework/Icarus/ppu/tilecnt_test_ntsc.bat`'s compile line with Icarus
  (`iverilog -g2012 -D RP2C02 -D ICARUS -o /tmp/tilecnt_fixer.run ../../../Common/*.v
  ../../../PPU/{pclk,hv,hv_decoder,fsm,tilecnt}.v tilecnt_test.v`; `vvp`) and got
  `tilecnt_test: TEST PASS (66390 checks)`. Parsing the VCD it wrote,
  top-level `n_FVO` changes exactly three times, each at `H = 256`, spaced
  62 744 ns (one scanline), and **never** in `H = 320–340`; `tilecnt.v:52,58`
  gates `TVSTEP` only by `E_EV`/`TSTEP`, so no step can exist in the prefetch.
- The file's own transcripts already agreed (`:162` fineY only at 257; `:171`
  only the 281 reload) and are unchanged. The claim was removed, not softened,
  so no `## Open questions` entry was needed for it.

**Finding 2 (major) — coarse-Y/fine-Y field name.**

- `04-ppu-rendering.md` §2 table, dot-256 row: "coarse Y increments" -> "fine Y
  increments (coarse Y only on a 7→0 wrap)" -> NESdev `PPU_scrolling` §Y
  increment; the file's own §1 note already said fine-Y.
- `04-ppu-rendering.md` §2 bullet (`:101`): "coarse X wraps and coarse Y is
  incremented" -> "… and fine Y is incremented (coarse Y advances only when fine
  Y wraps 7→0)" -> same.
- `04-ppu-registers.md` §10 `v`/`t` table: Coarse Y row "Y increment at dot 256
  and dots 321–336" -> "Step on the fine-Y 7→0 wrap of the dot-256 increment;
  reload at the pre-render vertical copy" -> coarse Y has no per-tile step;
  HDL `Tile_TV_Counter` `carry_in` is the fine-Y carry (`tilecnt.v:482–484`).
- `04-ppu-registers.md` §10 Fine Y row: "Reset during dots 280–304" -> "`+1` at
  dot 256, one step per rendered scanline; reloaded during dots 280–304" -> the
  row no longer omits the only per-line mutation -> Finding 2's second half.

**Finding 3 (major) — `06-open-questions.md:178`, `OQ-PPU-68`.**

- Row re-statused `open` (was `open`) and rewritten from the obsolete "cross-file
  dot disagreement" text (`… while 04-ppu-rendering.md §1 defines dot = H − 1
  and…`), which no file supports, into a resolved record: all four PPU files use
  one convention (dot = `H` = 0–340; fine-Y + last coarse-X increments at 256;
  copy at 257, effect 258; dummy fetches 337–340; `04-ppu-scrolling.md` §4's
  `(HDL edge)` rows are the same events one `PCLK` edge later) -> the Final
  Fixer's own record in `Specs/Review/global-consistency.md` and the four
  *Dot numbering* notes; the `H − 1` sentence was simply false.

**Finding 4 (major) — `06-open-questions.md:181`, `OQ-BRD-02`.**

- Row rewritten and re-statused to `open` as a resolved record: the old claim
  that `04-ppu-timing.md` §10 "still writes CPU cycles : dots (`1 : 3`, `5 :…`)"
  replaced by the current agreement (`01-platform-overview.md` §2,
  `04-ppu-timing.md` §10, `05-boards-and-io.md` §1 all write numerically PPU dots
  per CPU cycle, `3:1`/`16:5`/`3:1`, and each states the convention) -> re-read
  all three tables; no `1 : 3` or `5 : 16` cell survives.
- `06-open-questions.md` status table and area summaries corrected to match the
  two re-statuses: header now 25 / 51 / **52** / **28** (was 53 / 27) and the
  Boards summary "Documentation gaps only (9)" -> "(8)" plus an "Open (1)"
  entry -> the header counts must equal the 156 index rows; recount in prose.

**Finding 5 (minor) — `04-ppu-scrolling.md:142`, `(HDL edge)` header scope.**

- §4 table header `| Dot(s) (HDL edge) |` -> `| Dot(s) |`; the measured dot-257
  vertical row is now labelled `257 (HDL edge; nominal 256)` in its own cell ->
  the header covered the nominal fetch rows (1–2 … 255–256, 337–340), whose dots
  were never HDL-edge values (finding's own per-cell suggestion).
- §4 note (`:133–135`): now says the `(HDL edge)` label marks only the cells and
  transcripts carrying measured values, not the nominal fetch dots.

**Finding 6 (minor) — `01-platform-overview.md:218`, §11 index row.**

- `03-apu-overview.md` row: "the `$4000–$4017` register map" -> "the
  `$4000–$401F` register map" -> that file's scope and §4 heading
  (`03-apu-overview.md:4, 236`), `Specs/Readme.md:23`, and the `$4018–$401A` test
  registers it covers. The other `$4000–$4017` mentions in the set
  (`01-platform-overview.md:110`, `02-cpu-dma.md:306`, `03-apu-overview.md:236,
  255`, `nesdev-mapping.md:99, 143`) are correct as written — they name NESdev's
  APU/I-O register block, not this file's scope.

Not changed: Finding 7 (the `Readme.md` table does not list `Readme.md` itself)
is a note, not a defect, and `Readme.md` is outside this pass's remit.

## Round 2 (post-final-fix confirmation)

**Verdict: consensus.** All six findings are confirmed fixed. The fixer's log
(`## Post-consensus fixer pass`) was not trusted: every item was re-read in the
current text and re-checked against `HDL/PPU/tilecnt.v` and `fsm.v`, NESdev
`PPU_scrolling` and `PPU_rendering` (Wayback), and a fresh independent Icarus run
of `tilecnt_test.v`. No spec file was edited and no `git commit` was made; the
only file written is this report.

### Finding 1 (`04-ppu-scrolling.md`, vertical increment) — CONFIRMED FIXED

- No occurrence of "tile boundary"/"321–336 step" remains (`grep` over
  `Specs/*.md`). `:149` (coarse-X row) no longer mentions the vertical counter;
  `:150` now reads "vertical increment … **one fine-Y step per scanline**" with
  the coarse step "only on the fine-Y 7→0 wrap"; `:153` gives the 321–336
  prefetch `t`→`v` field "–"; `:178–185` states the prefetch "does **not** step
  the vertical position"; `:304–307` repeats it normatively.
- Table/prose/transcripts agree on *timing*: `:162–163` and `:171–172` show the
  vertical event only at dot 257 (HDL edge) plus the 281 reload, and only coarse
  X at 329/337.
- HDL: `tilecnt.v:52,58` `w13 = ~(E_EV | TSTEP)`, `TVSTEP = ~(w13 | PCLK)`, so
  the vertical chain (`Tile_FV_Counter`/`Tile_TV_Counter`, `:482–484`) steps on
  `E_EV` or `TSTEP` only; `E_EV` is `HPLA_7` (H = 255) via two latches
  (`fsm.v:450–451, 491`). No HPLA_7 and no `TSTEP` exists in 321–336.
- Fresh run: `iverilog … -D RP2C02 -D ICARUS … tilecnt_test.v` →
  `tilecnt_test: TEST PASS (66390 checks)`; parsing my own VCD, top-level
  `n_FVO` changes exactly 3 times, each at `H = 256`, spaced 62 744 ns, and never
  in `H = 320–340`.
- NESdev `PPU_scrolling` §Y increment: "fine Y is incremented at dot 256 of each
  scanline, overflowing to coarse Y"; §Between dot 328 … lists horizontal
  increments only. NESdev `PPU_rendering` §Cycles 321–336: "the first two tiles
  for the *next* scanline", at the position already advanced at dot 256.

### Finding 2 (`04-ppu-rendering.md`, `04-ppu-registers.md` field names) — CONFIRMED FIXED

- `04-ppu-rendering.md:88` now: "coarse X increments (last tile); fine Y
  increments (coarse Y only on a 7→0 wrap)". `:101`: "coarse X wraps and fine Y
  is incremented (coarse Y advances only when fine Y wraps 7→0)". No other
  dot-256 coarse-Y claim exists in the file.
- `04-ppu-registers.md:257` (Coarse Y row): "Step on the fine-Y 7→0 wrap of the
  dot-256 increment". `:259` (Fine Y row): "`+1` at dot 256, one step per
  rendered scanline". Both match the file's own *Dot numbering* note (`:239–243`)
  and NESdev's Y-increment rule; coarse Y is driven only by the fine-Y carry
  (`tilecnt.v:482–484`, `TileCountersControl2` `TVIN`).

### Finding 3 & 4 (`06-open-questions.md`, `OQ-PPU-68` / `OQ-BRD-02`) — CONFIRMED FIXED

- `:181` `OQ-PPU-68` is now a resolved record, `open`: it states the single
  convention (dot = `H` = 0–340, increments at 256, copy 257/258, dummy 337–340)
  and explicitly "No file defines dot = `H − 1`." No current-tense `H − 1` claim
  survives in `Specs/*.md`.
- `:184` `OQ-BRD-02` states the three tables agree on numerically PPU dots per
  CPU cycle (`3:1`, `16:5`, `3:1`) and that the old "`1 : 3`, `5 :…`" rows "are
  gone"; `04-ppu-timing.md:246–252` now writes `3 : 1` / `16 : 5` / `3 : 1`.
  The old values appear only as quoted history, not as an assertion about the
  timing file.
- **Header counts verified by recount**: statuses are
  `blocking-emulation` 25, `needs-hardware-test` 51, `documentation-gap` 52,
  `open` 28 = 156 rows, matching the `:20–23` table and the area sums
  (1+5+8+11+0 = 25; 3+8+11+24+5 = 51; 4+8+15+17+8 = 52; 1+3+6+17+1 = 28). The
  per-file list at `:33–35` also sums to 9+24+40+69+14 = 156.

### Finding 5 (`04-ppu-scrolling.md`, `(HDL edge)` scope) — CONFIRMED FIXED

- `:142` header is now `| Dot(s) |` (no blanket `(HDL edge)`), and the measured
  vertical row is labelled in its own cell: `:150` "257 (HDL edge; nominal
  256)", `:149` "(HDL edge; dot 256)". `:133–136` scopes the label to measured
  cells/transcripts, "not the nominal fetch dots of the first four rows", and
  `:156` marks 337–340 as "nominal dots". No nominal cell carries the label.
- Residual (minor, not the filed defect): the measured-valued cells `:151` "258"
  (nominal 257), `:152` "258–320" and `:154` "329, 337" (nominal 328/336) still
  carry no per-cell label. Cosmetic only; the dot-257/258 mapping is defined in
  `:150` and the note, so no reader can misdate the event.

### Finding 6 (`01-platform-overview.md` §11) — CONFIRMED FIXED

- `:218` now reads "APU blocks, clocks and the `$4000–$401F` register map",
  matching `03-apu-overview.md`'s scope and `Specs/Readme.md:23`.

### Sanity sweep — PASS

- All 20 section files: exactly one `**Status:** consensus (round N)` line, all
  five fixed headings present in the required order and unnumbered, length
  253–400 lines (all ≤ 400, all ≥ 120).
- `Specs/Readme.md` table: 24 rows against 25 `Specs/*.md` files, the only
  unlisted file being `Readme.md` itself; no stale or phantom rows. Its
  `## Status` figures (156 open questions, 25 blocking) still match the index.
- `04-ppu-timing.md:62–66` and `:396–400` carry the same single *Dot numbering*
  convention as the other three PPU files; its §10 no longer writes `1 : 3` or
  `5 :…`.

**New blocking or major issues: none.** The only open residuals are the two
cosmetic notes above (unlabelled measured cells in `04-ppu-scrolling.md` §4; the
`Readme.md` self-row), neither of which affects implementability, fidelity or
traceability. **Condition 1 (M1 and the six cross-file items) can be closed.**
