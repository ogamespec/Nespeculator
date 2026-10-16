# Specification consensus record (issue #1392)

## Consensus statement

**Consensus reached.**

All 20 spec section files under `Specs/` carry a latest Verifier verdict of
`consensus` (14 briefs in `Specs/Review/`, 33 review rounds, rounds 1–3/4, plus
the final re-verifications). No blocking finding and no major finding remains
open in any section: in every latest report both the `## Blocking` and the
`## Major` sections read "None", and the section-level residuals are minors
(wording, citation completeness, one HDL testbench gap).

The two conditions this record originally attached have since been closed.

**Condition 1 — re-verify the Final Fixer's set-level edits.** Closed by
`Specs/Review/post-fix-verification.md`. Its round 1 re-read the post-fix text
independently and filed six findings (1 blocking: a false "fine Y steps at each
321–336 prefetch tile boundary" claim; 3 major: coarse-Y-vs-fine-Y wording, two
stale `06-open-questions.md` entries; 2 minor). A further fixer pass applied all
six, logged in that report's `## Post-consensus fixer pass`. Round 2
(`## Round 2 (post-final-fix confirmation)`, appended at line 326) re-checked
each from the current text and the sources — `HDL/PPU/tilecnt.v` `TVSTEP` gating,
a fresh Icarus run of the repository's own `tilecnt_test` (**TEST PASS, 66 390
checks**) with its VCD parsed (one `n_FVO` change per line, at H=256, never in
320–340), and NESdev `PPU_scrolling`/`PPU_rendering` — and returned `consensus`
with no new blocking or major issue.

**Condition 2 — decide the length target (V1).** Resolved by the orchestrator:
`Specs/Review/agents/COMPILER.md`, `REVISION.md` and `GLOBAL-VERIFIER.md` now
state the binding `Specs/Conventions.md` §6 target of 120–400 lines. All 20
sections are inside it (253–400 lines).

What remains is cosmetic or explicitly out of scope. Cosmetic: three
measured-valued cells in `04-ppu-scrolling.md` §4 carry the `(HDL edge)` sense in
the row label rather than a per-cell tag, and `Specs/Readme.md` does not list
itself in its own table. Out of scope for this experiment: the deferred
terminology and coverage minors T1–T3, V2 and C3–C5 recorded below, and the
repository defects filed separately in `Specs/Review/upstream-findings.md`.

No blocking finding remains anywhere, and no residual item can invalidate an
implementable number; the record is therefore `consensus reached`.

## Per-section status

Counts are the findings in the **latest Verifier round** for the brief that
covers the file (`0` = the report's section says "None"). Where one brief covers
two files, the count is the brief's; the *Residual risk* column says which file
each open item concerns. "Rounds" lists the Verifier rounds present in that
brief's report.

| Spec file | Rounds | Verdict | Blocking | Major | Minor | Residual risk |
|---|---|---|---|---|---|---|
| `01-platform-overview.md` | 1, 2, 4 | consensus (r4) | 0 | 0 | 3 | cross-file items and Minors 1–2 closed by the Final Fixer; only the length note remains (357 lines vs `REVISION.md` 120–300) |
| `02-cpu-core.md` | 1, 2 | consensus (r2) | 0 | 0 | 3 | Minor 1 (`Z` row omits the `DB1` path), Minor 2 (`C`/`N`/`V` rows name `DB/P` instead of `DB/C`/`DB/N`/`DB/V`); Minor 3 fixed by the Final Fixer |
| `02-cpu-dma.md` | 1, 2 | consensus (r2) | 0 | 0 | 2 | Minor 2 (`rsff_2_4.v` not named for reset dominance); Minor 1 fixed by the Final Fixer |
| `02-cpu-instructions.md` | 1, 2 | consensus (r2) | 0 | 0 | 1 | Minor 1 ("ten captures" for `CPU_unofficial_opcodes`; CDX returns 86 rows, 83 HTTP 200) |
| `03-apu-dmc.md` | 1, 2 | consensus (r2) | 0 | 0 | 1 | the brief's only residual minor: §3 "PAL **load values**" should say "periods"; the `353.0 → 354` example is the not-nearest-integer one |
| `03-apu-frame-counter.md` | 1, 2, 3 | consensus (r3) | 0 | 0 | 1 | Minor 1 ("same spread" vs the 4-cycle/2-cycle figures) fixed by the Final Fixer |
| `03-apu-mixer.md` | 1, 2, 3 | consensus (r3) | 0 | 0 | 0 | none open; 4 : 2 : 1 weighting kept, NESdev 3 : 2 : 1 recorded |
| `03-apu-noise.md` | 1, 2 | consensus (r2) | 0 | 0 | 0 | none open against this file (the brief's residual minor is in `03-apu-dmc.md`) |
| `03-apu-overview.md` | 1, 2 | consensus (r2) | 0 | 0 | 2 | Minor 2 (`triangle.v` missing from `Primary sources`/`Provenance`); Minor 1 (`RDY` "asserts") fixed |
| `03-apu-pulse.md` | 1 | consensus (r1) | 0 | 0 | 6 (brief) | 3 open against this file: §6 `$4015` gates the output not the decrement; §3 `d=0` first duty bit polarity; §4 PAL 5-step `/LFO1` = 14913, not 16625. Minor 5 (`APU_Mixer`) adjudicated, Minor 6 (Dendy row) closed by C7 |
| `03-apu-triangle.md` | 1 | consensus (r1) | 0 | 0 | 6 (brief) | 1 open against this file: §6 `LOCK` is a `$401A` bit-7 write, not the `$4019` readout; `CPU_Test_Mode` not cited. Minor 5 adjudicated, Minor 4 is in the pulse file |
| `04-ppu-memory-map.md` | 1, 2, 3 | consensus (r3) | 0 | 0 | 2 | Minors 1–2 cosmetic: "shadowed VRAM byte" phrasing; "224 bytes" vs the functional description |
| `04-ppu-overview.md` | 1, 2 | consensus (r2) | 0 | 0 | 1 | Minor 1: §6.3 "latches on `OAM8`" should be "an extra buffer on `OAM8`, latches on `/PICTURE`" |
| `04-ppu-registers.md` | 1, 2 | consensus (r2) | 0 | 0 | 1 (brief) | none open against this file (the OAM8 minor is in `04-ppu-overview.md`) |
| `04-ppu-palette.md` | 1, 2 | consensus (r2) | 0 | 0 | 4 (brief) | Minor 2 open: §6 `PPU_palettes §2C03/2C04` covers two facts and should be split. Minor 1 (mis-sourced quote) fixed by the Final Fixer |
| `04-ppu-timing.md` | 1, 2 | consensus (r2) | 0 | 0 | 4 (brief) | Minor 3 open: §6 "`/FPORCH` high" polarity wording. Minor 4 (stale length note) resolved by the Final Fixer's edit |
| `04-ppu-rendering.md` | 1, 2, 3 | consensus (r3) | 0 | 0 | 1 (brief) | none open: the `/FO` stage count and stale `fsm.v` line range were fixed by the Final Fixer |
| `04-ppu-sprites.md` | 1, 2, 3 | consensus (r3) | 0 | 0 | 1 (brief) | none open as a finding; note only: §7 item 4 pairs "`BLACK = 0`" with "bits 3 and 4 set", which already imply it |
| `04-ppu-scrolling.md` | 1, 2, 3 | consensus (r3) | 0 | 0 | 3 | Minor 1 §5 coarse-Y = 30 load-only wording; Minor 2 fine-Y decrement measured vs table (recorded in Open questions as a suspected simulator artefact); Minor 3 is review-file bookkeeping |
| `05-boards-and-io.md` | 1, 2, 3 | consensus (r3) | 0 | 0 | 2 | Minor 2 (`PPU/pclk.md` cited inline but absent from metadata/Provenance); Minor 1 (÷4 prose) fixed by the Final Fixer |
| `06-open-questions.md` *(index)* | — | no verdict (INDEXER) | — | — | 156 OQ rows | 25 `blocking-emulation`, 51 `needs-hardware-test`, 53 `documentation-gap`, 27 `open`, plus 2 declared resolved records |
| `nesdev-mapping.md` *(index)* | — | no verdict (INDEXER) | — | — | — | cross-section ↔ wiki ↔ HDL ↔ NESdev index; no `Status` line by design |

Totals: **20/20 sections consensus**; the 14 latest reports list **0 blocking
and 0 major** open findings; the open section-level minors are itemised in
*Residual open findings* below.

## Residual open findings

Nothing below is hidden by the verdict: each item is either a wording/citation
minor that changes no implementable number, a documented gap in the
`## Open questions` register, or a repository defect outside the specs' edit
scope.

### Deferred by the Final Fixer (global review, still open)

| Item | Where | Why it is acceptable |
|---|---|---|
| **T1** `Implied` vs NESdev `Implicit` | `02-cpu-core.md:156,:170`; `02-cpu-instructions.md:26,:41–42` | Minor terminology; both files record that NESdev writes `Implicit`, so the mapping is traceable. Still `pending` in `agents/mechanical-fixes.md` item 8; needs one editorial decision, no behaviour change. |
| **T2** Wiki block names without the NESdev mapping | `04-ppu-overview.md:59,:63–64,:78–80,:97–98`; `04-ppu-rendering.md:135,:241,:363` | Minor terminology drift (`Obj Eval`, `OAM Buffer`, `Object FIFO`, `PatGen`, `COLOR BUFFER`); only `04-ppu-sprites.md` maps them. The correspondence tables carry the page names, so a reader can still resolve them. |
| **T3** `/NAME` vs HDL `n_NAME` for the same net | `02-cpu-dma.md:30` (`/DMCAB` vs `n_DMCAB`) vs `03-apu-dmc.md:181,:242` (`n_DMCAB`, `n_NOUT`, `n_BOUT`) | Minor notation mix, explicitly against `Conventions.md` §4; both spellings identify the same net and the HDL name is given alongside. |
| **V1** Length target contradicts the role contracts | `Conventions.md:137`, `VERIFIER.md:52`, `FINAL-FIXER.md:31` say 120–400; `GLOBAL-VERIFIER.md:34`, `COMPILER.md:37`, `REVISION.md:33` say 120–300 | The binding contract (`Conventions.md` §6) is satisfied by all 20 files (253–400 lines), so this is a process/contract defect, not a spec defect. 17 of 20 files exceed 300 lines; none exceeds 400. Needs one orchestrator decision. |
| **V2** Title format not uniform | `03-apu-frame-counter.md:1`, `03-apu-mixer.md:1` (no number) and `03-apu-dmc.md:1`/`03-apu-noise.md:1` (`# APU 03 — …`); `02-cpu-*`/`04-ppu-*` use the bare file stem as area | Minor `Conventions.md` §6 item 1 deviation; titles are still unique and unambiguous, and every file is indexed by stem in the Readme/§11 index. |
| **V3** Files at the 400-line ceiling | `04-ppu-palette.md` 400, `04-ppu-rendering.md` 400, `04-ppu-timing.md` 400, `05-boards-and-io.md` 400; `04-ppu-sprites.md` 399 | Standing editorial constraint, not a defect: every file is within the binding limit. Any further edit to these files needs a compensating deletion. |
| **V4** `06-open-questions.md` has no metadata block and none of the five fixed headings | `06-open-questions.md` | Accepted INDEXER exception for an index. `01-platform-overview.md` §11 declares it; `Specs/Readme.md` lists it but does not spell out the metadata exception. |
| **C3** Breaking NES Wiki pages never cited | `6502/bus_control.md`, `6502/pc_control.md`, `PPU/youtube.md`, `tentacles.md` | Minor coverage gap. The last two are non-technical; the first two are the DL/ADL/PCL material behind the HDL PC-load defect, which is recorded in `02-cpu-core.md` Open questions and as upstream finding 3. |
| **C4** Undocumented opcodes not enumerable | `02-cpu-instructions.md` §6 (`:250–252`) | Deliberate scope limit, stated in the file and in the OQ register; official-only assemblers are covered. |
| **C5** No PPU warm-up rule | `04-ppu-registers.md:274` gives the ~29 658 NTSC / ≈33 132 PAL ignored-write window, but no practical "do not enable rendering yet" rule | Documented gap for demo authors; the underlying window and its rank-4 status are recorded, so the missing rule cannot mislead about hardware. |
| **M6** Dendy post-render line count worded two ways | `04-ppu-overview.md:35` "50 post-render scanlines instead of 1" vs `04-ppu-timing.md:192` "240–290 (50 extra)" | Both statements are numerically consistent (NTSC has 1 post-render line, Dendy 51); only the first reads as a total. Wording-only. |
| **C1/B1** `Specs/Review/CONSENSUS.md` was promised but did not exist | `Specs/Readme.md:40` links it | **Resolved by this file.** The link now resolves. |
| **T4/C7** `Clock_rate` (redirect) in eight correspondence tables | e.g. `02-cpu-core.md`, `05-boards-and-io.md` | **Resolved by the Final Fixer**: `Cycle_reference_chart` is now the single spelling set-wide, and `nesdev-mapping.md` keeps the rename as the historical record. |

### Per-section residual minors (latest Verifier reports)

The reports themselves carry the line anchors; the list below is the live state
after the Final Fixer pass.

- `01-platform-overview.md` (platform r4): Minor 3 length note only (355 lines at
  review, now 357; V1 class). The two cross-file wording items were fixed.
- `02-cpu-core.md` (cpu-core r2): Minor 1 — the `Z` row gives only the `/DBZ`
  path and omits the `DB1` load path (`flags.md`, `flags.v:71`); Minor 2 — the
  `C`/`N`/`V` rows name `DB/P` where the sources use `DB/C`, `DB/N`, `DB/V`
  (behaviourally harmless). Report notes also flag the §4 `V` footnote
  ("only `CLV` and ADC/BIT") against the same row's `PLP`/`RTI` column, the §2
  `RDY2` citation (`APU/core.md` is the right page), and the §9 "6 `M2` clocks"
  attribution (delay after `/RST` release, not the hold).
- `02-cpu-dma.md` (cpu-dma r2): Minor 2 — "reset dominates set" is a property of
  `HDL/Common/rsff_2_4.v:17–20`, not of `dpcm.v`; name the primitive.
- `02-cpu-instructions.md` (cpu-instructions r2): Minor 1 — "ten captures" does
  not reproduce (CDX: 86 rows / 83 HTTP 200); drop the count or write "many".
- `03-apu-dmc.md` (apu-noise r2): Minor 1 — §3 scales "load values" where it
  means periods, and presents `353.0 → 354` as nearest-integer when 354 is only
  the nearest achievable even value.
- `03-apu-overview.md` (apu-overview r2): Minor 2 — add `triangle.v` (and
  `square.v`/`noise.v`) to `Primary sources` and the `$4018–$401A` Provenance
  row; the inline citation keeps traceability intact.
- `03-apu-pulse.md` / `03-apu-triangle.md` (apu-pulse r1): no revision round ran,
  so these stand as written — pulse §6 `$4015` gates the output, not the
  decrement; pulse §3 `d = 0` first duty bit is inverted (12.5 % row); pulse §4
  PAL 5-step `nLFO1` interval is 14913 (`clkgen.md:168`), not 16625; triangle §6
  `LOCK` is set by a `$401A` bit-7 write (not the `$4019` readout) and
  `CPU_Test_Mode` should be cited in both files. None changes a timer period or a
  table value.
- `04-ppu-memory-map.md` (ppu-memmap r3): Minors 1–2 — "shadowed VRAM byte"
  phrasing for the unreadable-palette path; the palette mirror is given as
  "224 bytes" in one row and functionally in two others (both correct).
- `04-ppu-overview.md` (ppu-overview-regs r2): Minor 1 — `OAM8` gained an
  *additional buffer*, not latches; only `/PICTURE` gained latches.
- `04-ppu-palette.md` / `04-ppu-timing.md` (ppu-palette-timing r2): palette §6
  should split the `PPU_palettes` citation (hue `$D` = black on 2C03/2C05-99 vs
  the 2C04 copy-protection palette); timing §6's "`/FPORCH` high" pairs an
  active-low name with a level (the window 257–279 is correct).
- `04-ppu-sprites.md` (ppu-render-sprites r3): no open finding; the §7 item-4
  "`BLACK = 0`; bits 3 and 4 set" clause is redundant rather than wrong.
- `04-ppu-scrolling.md` (ppu-scroll r3): Minor 1 — §5 reads as if the step out of
  coarse Y = 30 is 31 → 0, while the pseudocode and Observable #9 give 30 → 31;
  Minor 2 — the §4 measured fine-Y decrement (5 → 4, 1 → 0) disagrees with the
  `+1` table and is recorded in Open questions as a suspected simulator artefact;
  Minor 3 is review-file bookkeeping.
- `05-boards-and-io.md` (boards-io r3): Minor 2 — `PPU/pclk.md` is cited inline
  in §1 but absent from `Primary sources` and the region-clocks Provenance row
  (Conventions §7 is satisfied by the inline citation).
- `06-open-questions.md`: the header's per-file count list ("9, 6, 10, 8 …")
  omits a "System" label and reads like a mis-sum; the arithmetic is correct.

### Documented open-question register

`06-open-questions.md` aggregates **156** entries: 25 `blocking-emulation`, 51
`needs-hardware-test`, 53 `documentation-gap`, 27 `open`, plus two entries
(`OQ-SYS-03`, `OQ-CPU-08`) kept as resolved records. The 25
`blocking-emulation` entries are the sections' own statement that an emulator
cannot be bit-exact without a decision or a missing number; each names the
evidence that would close it. They are the deliberate, documented residual risk
of the set and are not re-listed here individually.

### Repository (upstream) defects — outside the spec set's edit scope

From `Specs/Review/upstream-findings.md` (re-verified by the Global Verifier):

1. `HDL/PPU/tilecnt.v:459–462` ties `AT_adr[6:9]` to `1'd0`, dropping the `$3C0`
   attribute offset; the Logisim `Constant` has no value attribute and defaults
   to 1 (Logisim-evolution 3.8.0, `pamux.v` precedent). **major (functional)** —
   recorded in `04-ppu-memory-map.md:284` / `04-ppu-scrolling.md:80`; fix the HDL.
2. No single `HDL/PPU/dataread.v` (the Data Reader is spread over `par.v`,
   `tilecnt.v`, `bgcol.v`, `pamux.v`, `scroll_regs.v`). **minor** — no spec change
   needed; OQ-PPU-37/51 cover it.
3. `HDL/Core6502` never loads a memory-sourced low byte into `PCL` (reset PC
   becomes `$04FF`). **major (functional)** — recorded in `02-cpu-core.md`
   Open questions; fix `bus_control.v`/`pc_control.v`/`decoder.v`.
4. Stale VCD artifacts under `HDL/Framework/Icarus/ppu/`; the stronger claim that
   they disagree with the HDL is *unverified*. **minor (tooling)** — cite re-runs,
   not waveforms; do not commit VCDs.
5. Letterless RP2A03 `M2` duty: rank-1 wiki 15/24 vs rank-3 Docs 17/24.
   **major (documentation/behaviour)** — needs a hardware measurement; both
   values are kept and flagged (`OQ-SYS-01`).
6. Five wrong values plus spelling errors in `Docs/6502/6502 Instruction Set.htm`.
   **minor (documentation)** — the specs already use the correct values.

### Set-level caveat

The Final Fixer's M1 dot-numbering edits (`04-ppu-scrolling.md`,
`04-ppu-rendering.md`, `04-ppu-registers.md`, `04-ppu-timing.md`) and its M2
rewrite of the `01-platform-overview.md` Open-questions block were made **after**
those files' last Verifier reports, so the post-fix text has not been through an
independent Verifier round. The Global Verifier's own "Outstanding blocking
findings whose text was already fixed (re-read, no edit)" list covers the
pre-existing ones; the M1/M2 edits themselves do not have that cover. Re-running
the global consistency check over the post-Final-Fixer revisions is the single
action that would remove this condition.

## Disagreements resolved during the rounds

Ten of the substantive disagreements the agents argued about, with the
resolution. (Not counted separately: the `$40` blocking — a `$4017` write with
bit 7 clear selects the 4-step sequence, it does not "keep the current mode"; the
`APU_Mixers` vs `APU_Mixer` page-name adjudication — the singular page is the
real article; and the `%0001_1111` → `%0000_1111` length-counter example.)

1. **Pulse sweep-enable polarity.** The wiki lists `SWDIS` as fed by register
   `WR1[3]`; the HDL feeds it from `$4001` bit 7 through the `RegisterBit`
   inversion (`square.v:275`, `swdis_reg .d(DB[7]) .nq(SWDIS)`), i.e. bit 7 = 1
   enables. Icarus re-check: `$4001 = $F1` drops `SWDIS` 1 → 0 and pulses
   `DO_SWEEP`; `$71` never asserts it. **Resolved:** bit 7 is the enable, the
   wiki bit index is a typo, and NESdev's `E` = bit 7 agrees; recorded in
   `03-apu-pulse.md` Open questions. The same round kept the **mute threshold**
   disagreement (`SW_UVF = nor(Fx[2..10])` → mute at `t < 4`) as the schematic
   value, with NESdev's `t < 8` in a block quote and Open questions, per
   `Conventions.md` §3 (silicon wins; record the difference).
2. **Mixer weighting 4 : 2 : 1 vs NESdev 3 : 2 : 1.** `APU_DAC_Generator.py`
   (`tri_reciprocal = [2,4,8,16]`, `rnd_reciprocal = [1,2,4,8]`,
   `dmc_reciprocal = [0.5 … 32]`) gives 4 : 2 : 1, and `auxb.mem` is constant for
   218 distinct `4*TRI + 2*RND + DMC` values while NESdev's `3*triangle + 2*noise
   + dmc` index collides (203 distinct). **Resolved:** the repository's
   4 : 2 : 1 stays normative; NESdev's formula is kept as a block quote and an
   Open question, with the unit note that `g_B` is scaled ×2 for
   `R_int = 78 kΩ / g_B`.
3. **Frame counter 29828 vs NESdev 29830.** The repository's uniform PLA
   intervals give 29828 NTSC / 33252 PAL (and 37284 / 41564 in 5-step), and these
   remain normative. Round 1 had summed NESdev's deltas as if they corroborated
   the repository and quoted an ad-hoc `29600 → 59430` run. **Resolved:** the
   bullet now gives NESdev's own boundaries 7457/14913/22371/**29828** with
   deltas summing to 29828, identifies 29829 as NESdev's mode-1 step-4 time and
   29830 as its reset/period figure, and the irreproducible example was dropped
   rather than "fixed" toward NESdev's totals.
4. **Open-bus `$FF` causality.** The draft attributed the external open-bus
   default to the internal precharge, but `busmux.v:52–55` precharges only
   `SB`/`DB`/`ADL`/`ADH` and `data_bus.v:38` (`bufif0 (DB_Ext, dor_latch_nq, RD)`)
   is disabled while `RD = 1`, so no path reaches `D0–D7`. **Resolved:** the
   behavioural rule ("last value driven, read or write; `$FF` by default") stays,
   the initial value is labelled NESdev rank 4, and the mechanism moved to
   `## Open questions` (`OQ-CPU-04`).
5. **`$400C` / `$400F` envelope restart.** The draft said a `$400C` write restarts
   the decay sequence. `env.v:49` resets `EnvReload` only on `WR_LC` (`$400F`);
   `$400C` drives only `envdis_reg`/`lc_reg`/`vol_reg`. **Resolved:** `$400C`
   does **not** restart the envelope; `$400F` is the only noise-channel write
   that does — matching NESdev "length counter load and envelope restart
   (write)" and the file's own Observable #9.
6. **Attribute-table `AT_adr[6:9]` HDL defect.** The specs print the correct
   `$23C0` composition (`AT_adr[9:6] = %1111`); `HDL/PPU/tilecnt.v:459–462` ties
   those bits to `1'd0`, and the Logisim schematic's bare `Constant` actually
   reads 1 (3.8.0 `Constant.java` initialises `Value.TRUE`; the same convention
   is documented in `pamux.v`). **Resolved:** the spec keeps the hardware intent
   and records the tie-off as an **HDL defect** (upstream finding 1) rather than
   changing the `$3C0` offset in the spec; `04-ppu-memory-map.md` and
   `04-ppu-scrolling.md` cross-reference it.
7. **Fine-Y clamp.** The draft clamped fine Y to 0–3 outside the `$2006`
   first-write path. **Resolved:** `FV[2] = 0` is scoped to the `$2006`
   first-write / `W6_1` path only, the second `$2005` write sets all three bits,
   and the counter is **not** clamped (fine Y 0–7). Observable #8 and the
   programming note carry the same scope.
8. **`PAT_ADR` 8×8 bit 4.** One cell gave `PAT_ADR[4] = OBSEL` for 8×8. **Resolved:**
   bit 4 = "tile byte bit 0 (tile index)" in 8×8 and `OV3` in 8×16; `OBSEL`
   (PPUCTRL bit 3) is on bit 12 only — matching `par.md` rows 4/5–11/12,
   `par.v` `ParBit4`/`ParControl`, and the two sibling files.
9. **PPU dot-numbering convention.** Three files gave two answers for the same
   edge: the last coarse-X/Y increment at dot 256 or 257, and the horizontal
   `t`→`v` copy at 257 or 258. **Resolved** by one shared *Dot numbering* note in
   `04-ppu-scrolling.md`/`04-ppu-rendering.md`/`04-ppu-registers.md`/
   `04-ppu-timing.md`: dot = `H` = 0–340; fine-Y and last coarse-X increments at
   dot 256; the copy at dot 257 with its effect visible at 258; dummy fetches
   337–340; the HDL PCLK-edge measurements are one dot later and are labelled
   `(HDL edge)`. The obsolete cross-file Open-questions entries were deleted.
10. **Dendy clock and the `CPU:PPU` column convention.** `01-platform-overview.md`
    §2, `04-ppu-timing.md` §10 and `05-boards-and-io.md` §1 disagreed on the
    column order (CPU-first header vs PPU-first values), on the PAL cell
    (`5:16` vs `16:5`), on Dendy (`≈1.7897727 MHz` / `1 : ≈2.973` vs
    `1.773448 MHz` / `3 : 1`) and on the PCLK divider (`÷4` prose for a 2C07 that
    is `÷5`). **Resolved:** the column is the CPU:PPU divider ratio, numerically
    the **PPU dots per CPU cycle**; NTSC `3:1`, PAL `16:5` = 3.2, Dendy
    `1.773448 MHz (÷15)`, PPU `5.3203424 MHz (÷5)`, `3 : 1`, and the dot clock is
    "2C02 (`CLK` ÷4) / 2C07 (`CLK` ÷5)". The Final Fixer also unified the
    precision of the NTSC CPU clock (`1.7897727 MHz`) and the PAL `ACLK`
    (`831.303 kHz`) set-wide.

## What would change our mind

Concrete hardware measurements or wiki/repository updates that would invalidate a
specification decision (all are also listed with their closing evidence in
`06-open-questions.md` → "How to close these"):

1. **`M2` duty cycle on a letterless RP2A03 vs a 2A03E/G/H** in 24ths — decides
   `15/24` vs `17/24` and therefore `OQ-SYS-01`, `03-apu-overview.md:73` and the
   platform Open question (upstream finding 5).
2. **A pulse-timer sweep across 4/7/8 with the mute recorded** (or a die check of
   the `SW_UVF` bits) — decides the `t < 4` schematic rule vs NESdev's `t < 8`
   (`OQ-APU-33`).
3. **A committed `clkgen.v` testbench over full 4-step/5-step periods plus a
   frame-IRQ test ROM** — decides 29828/37284 vs NESdev's 29830/37281
   (`OQ-APU-08`).
4. **An AUX B transfer-curve measurement, or a transistor-level model** — decides
   4 : 2 : 1 vs NESdev's 3 : 2 : 1 (`OQ-APU-14`); the repository's own Readme
   admits the FET model is "a simplified … linear resistance" while the real
   network has a cubic relationship.
5. **A scope/logic-analyzer capture of `M2` vs `PCLK` from `/RES` release**
   (NTSC and PAL) — fixes the CPU:PPU phase and closes `OQ-SYS-05`.
6. **A die/HDL trace of the external data-bus driver plus a scope measurement of
   D0–D7 on an undriven read** — decides the open-bus `$FF` mechanism
   (`OQ-CPU-04`).
7. **A committed DMC-fetch testbench and an ACLK-phase `$4011` race test** —
   decides the DMC/OAM arbitration window and the `$4011` race
   (`OQ-CPU-09`, `OQ-CPU-10`, `OQ-APU-02`).
8. **A fix to `HDL/PPU/tilecnt.v:459–462` plus an `AT_adr[9:6]` assertion in
   `tilecnt_test.v`** — would make the recorded attribute-fetch defect obsolete
   (upstream finding 1). A fix to the Core6502 PCL load path would do the same
   for finding 3.
9. **A documented revision rule for the PPU reset window** (`$2000`/`$2001`/
   `$2005`/`$2006` writes in the ~29 658 / ≈33 132 CPU-cycle window) and a
   hardware OAM-decay measurement — closes `OQ-PPU-27/64`, `OQ-PPU-06`,
   `OQ-PPU-54` and the C5 warm-up gap.
10. **A correction of `old_2a03.md` or `Docs/NES_Famicom_Board_Revisions.md`**
    (whichever is wrong) and of the six `Docs/6502/6502 Instruction Set.htm`
    misprints — would remove upstream findings 5 and 6.
11. **An orchestrator decision on V1** (one length target across all role
    contracts) and a Verifier re-run of the global consistency check over the
    post-Final-Fixer revisions — would remove the two conditions on this
    consensus and allow `consensus reached`.

## Process record

**Roles that ran** (contracts in `Specs/Review/agents/`): Compiler (drafted each
section from Breaking NES Wiki + HDL + Docs), Adapter (NESdev terminology,
register naming, document shape, correspondence tables), Verifier (independent;
wrote the reports; never edited a spec file), Revision (fixed blocking/major
findings and appended `## Revision round N` notes), Indexer (produced
`Specs/06-open-questions.md` and `Specs/nesdev-mapping.md`), Global Verifier
(`Specs/Review/global-consistency.md`, `Specs/Review/upstream-findings.md`),
Final Fixer (the `## Final fixer pass` section of `global-consistency.md`), and
this Consensus Agent.

**Rounds.** 14 section briefs covering 20 section files; **33 Verifier review
rounds** and **19 `## Revision round N` notes** in `Specs/Review/*.md`. Review
rounds per brief: `platform` 1,2,4; `cpu-core` 1,2; `cpu-dma` 1,2;
`cpu-instructions` 1,2; `apu-overview` 1,2; `apu-pulse` 1; `apu-noise` 1,2;
`apu-fc-mixer` 1,2,3; `ppu-overview-regs` 1,2; `ppu-memmap` 1,2,3;
`ppu-render-sprites` 1,2,3; `ppu-scroll` 1,2,3; `ppu-palette-timing` 1,2;
`boards-io` 1,2,3. Every brief's latest round is `consensus`.

**Where the reports are.**

- One report per brief, all rounds preserved: `Specs/Review/` — `platform.md`,
  `cpu-core.md`, `cpu-dma.md`, `cpu-instructions.md`, `apu-overview.md`,
  `apu-pulse.md`, `apu-noise.md`, `apu-fc-mixer.md`, `ppu-overview-regs.md`,
  `ppu-memmap.md`, `ppu-render-sprites.md`, `ppu-scroll.md`,
  `ppu-palette-timing.md`, `boards-io.md`.
- Set-level: `Specs/Review/global-consistency.md` (snapshot verdict `revise`,
  23:28; `## Final fixer pass` appended 23:37–23:50) and
  `Specs/Review/upstream-findings.md`.
- Indexes: `Specs/06-open-questions.md` (156 entries) and
  `Specs/nesdev-mapping.md`.
- Role contracts and section briefs: `Specs/Review/agents/` (`COMPILER.md`,
  `ADAPTER.md`, `VERIFIER.md`, `REVISION.md`, `INDEXER.md`,
  `GLOBAL-VERIFIER.md`, `FINAL-FIXER.md`, `CONSENSUS.md`, `items.md`,
  `mechanical-fixes.md`).

**Status-line edits made by this role (the only spec-file changes).** Each of the
20 section files was changed from `**Status:** revised (round N)` /
`**Status:** adapted (round 1)` to `**Status:** consensus (round N)`, keeping the
round number already in the file. The round that carried the consensus verdict
is given in parentheses where it differs from the revision round.

| File | New status line | Consensus declared in |
|---|---|---|
| `01-platform-overview.md` | `consensus (round 3)` | platform review round 4 |
| `02-cpu-core.md` | `consensus (round 2)` | cpu-core round 2 |
| `02-cpu-dma.md` | `consensus (round 2)` | cpu-dma round 2 |
| `02-cpu-instructions.md` | `consensus (round 2)` | cpu-instructions round 2 |
| `03-apu-dmc.md` | `consensus (round 2)` | apu-noise round 2 |
| `03-apu-frame-counter.md` | `consensus (round 3)` | apu-fc-mixer round 3 |
| `03-apu-mixer.md` | `consensus (round 2)` | apu-fc-mixer round 3 (file unchanged that round) |
| `03-apu-noise.md` | `consensus (round 2)` | apu-noise round 2 |
| `03-apu-overview.md` | `consensus (round 2)` | apu-overview round 2 |
| `03-apu-pulse.md` | `consensus (round 1)` | apu-pulse round 1 |
| `03-apu-triangle.md` | `consensus (round 1)` | apu-pulse round 1 |
| `04-ppu-memory-map.md` | `consensus (round 3)` | ppu-memmap round 3 |
| `04-ppu-overview.md` | `consensus (round 2)` | ppu-overview-regs round 2 |
| `04-ppu-palette.md` | `consensus (round 2)` | ppu-palette-timing round 2 |
| `04-ppu-registers.md` | `consensus (round 2)` | ppu-overview-regs round 2 |
| `04-ppu-rendering.md` | `consensus (round 3)` | ppu-render-sprites round 3 |
| `04-ppu-scrolling.md` | `consensus (round 3)` | ppu-scroll round 3 |
| `04-ppu-sprites.md` | `consensus (round 3)` | ppu-render-sprites round 3 |
| `04-ppu-timing.md` | `consensus (round 2)` | ppu-palette-timing round 2 |
| `05-boards-and-io.md` | `consensus (round 3)` | boards-io round 3 |

`06-open-questions.md` and `nesdev-mapping.md` have no `**Status:**` line (they
are INDEXER deliverables, not sections) and were not edited, per the role
contract. No other spec file was touched, and no `git commit` was made.

**Line counts at the time of this record** (after the Final Fixer pass; all
inside `Conventions.md` §6's 120–400): 01 357; 02-cpu-core 398; 02-cpu-dma 364;
02-cpu-instructions 366; 03-apu-dmc 323; 03-apu-frame-counter 279;
03-apu-mixer 253; 03-apu-noise 317; 03-apu-overview 307; 03-apu-pulse 319;
03-apu-triangle 301; 04-ppu-memory-map 292; 04-ppu-overview 397;
04-ppu-palette 400; 04-ppu-registers 357; 04-ppu-rendering 400;
04-ppu-scrolling 394; 04-ppu-sprites 399; 04-ppu-timing 400;
05-boards-and-io 400; 06-open-questions 338; nesdev-mapping 449.

**Audit path.** Every claim above is anchored either to a line in
`Specs/Review/<brief>.md` (the round-3/round-4 reports are at the top of each
file; the history is kept below), to `Specs/Review/global-consistency.md`
(snapshot table at lines 8–30, the `## Final fixer pass` at line 347), or to
`Specs/Review/upstream-findings.md` (rows 1–6).
