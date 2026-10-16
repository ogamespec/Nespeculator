# Global consistency review

**Scope:** the whole `Specs/` set for issue #1392, checked as a *set*.
**Snapshot:** 2026-10-13 23:28 MSK. Several files were being revised *while* this
review ran, so every finding below is anchored to a line number and, where the
file was moving, to its md5 at the snapshot. Re-check a line before acting on it.

| File | md5 (8) | lines | Status at snapshot |
|---|---|---|---|
| `01-platform-overview.md` | `9db0ddcf` | 355 | revised (round 3) |
| `02-cpu-core.md` | `5170c33d` | 398 | revised (round 2) |
| `02-cpu-dma.md` | `feb83b14` | 371 | revised (round 2) |
| `02-cpu-instructions.md` | `cf9f0eee` | 366 | revised (round 2) |
| `03-apu-dmc.md` | `4c2f24ac` | 323 | revised (round 2) |
| `03-apu-frame-counter.md` | `acaf7037` | 279 | revised (round 3) |
| `03-apu-mixer.md` | `b8927fd6` | 253 | revised (round 2) |
| `03-apu-noise.md` | `47327f99` | 317 | revised (round 2) |
| `03-apu-overview.md` | `0147a8a9` | 307 | revised (round 2) |
| `03-apu-pulse.md` | `f73190da` | 319 | adapted (round 1) |
| `03-apu-triangle.md` | `22eedb98` | 301 | adapted (round 1) |
| `04-ppu-memory-map.md` | `0274a39e` | 292 | revised (round 3) |
| `04-ppu-overview.md` | `011cf488` | 399 | revised (round 2) |
| `04-ppu-palette.md` | `2b254b99` | 400 | revised (round 2) |
| `04-ppu-registers.md` | `a215e482` | 357 | revised (round 2) |
| `04-ppu-rendering.md` | `bb6fd79d` | 400 | revised (round 3) |
| `04-ppu-scrolling.md` | `57812389` | 400 | revised (round 3) |
| `04-ppu-sprites.md` | `ab76957f` | 400 | revised (round 3) |
| `04-ppu-timing.md` | `1bdf6e94` | 395 | revised (round 2) |
| `05-boards-and-io.md` | `4530a710` | 400 | revised (round 3) |
| `06-open-questions.md` | `1caccb6a` | 334 | (index, no Status line) |

## Verdict

**revise**

Two independent reasons:

1. **The set still contradicts itself on the PPU dot numbering** (M1) — two files
   put the last coarse-X/Y increment at dot 256 and one at dot 257, and the copy
   at dot 257 vs dot 258. An implementer cannot pick one value.
2. **The verification record does not cover the current revisions.** Every file
   that is at `revised (round 2/3)` was revised *after* its last Verifier report,
   and the latest reports still list blocking/major items that the current text
   has already fixed (see *Unresolved blocking findings*). Under
   `Specs/Conventions.md` §10 a section reaches consensus only when a Verifier
   report shows no blocking findings; stale reports cannot carry that.

Everything else found is major/minor and is listed below for the Final Fixer.

What *is* consistent (checked and found identical everywhere they appear): the
master clocks (`21.477272` / `26.601712` MHz), the /12, /16, /15 and /4, /5
dividers, 341 dots and 262/312 scanlines, the vblank windows
(NTSC 241–260, PAL 241–310, Dendy 291–310), the CPU:PPU *convention* (`3:1`,
`16:5`, Dendy `3:1`), the 4-step LFO intervals (7457 / 14913; PAL 8313 /
16625), the DMC rate table, and the OAM/DMC DMA cycle counts (513/514, 3/4).

## Cross-document mismatches

### M1 (blocking) — PPU dot numbering for the last coarse-X/Y increment and the horizontal copy

Three files give two different answers for the same hardware edge:

| Location | Last coarse-X increment | Vertical increment | Horizontal copy |
|---|---|---|---|
| `04-ppu-scrolling.md:150–152` (also 194–200, 387–391) | dot **257** | dot **257** | dot **258** |
| `04-ppu-rendering.md:84`, `:97` | dot **256** | dot **256** | — |
| `04-ppu-registers.md:251–252` | — | dot **256** | dot **257** |

`04-ppu-scrolling.md:136–140` states its own convention ("each event is named
after the dot in which it is observed; NESdev 'dot N' = this file's dot N+1"),
while `04-ppu-rendering.md:24–29` claims dot = `H` "and uses that number for
every schedule in this file and in `04-ppu-sprites.md`". The disagreement is
recorded (`04-ppu-rendering.md:397–398`, `04-ppu-scrolling.md:385–391`) but not
resolved, and `04-ppu-registers.md` states it as plain fact with no caveat. A
mid-frame-split demo written from `04-ppu-scrolling.md` §7 ("before dot 258")
contradicts one written from `04-ppu-registers.md` §7 ("before dot 257", lines
183–185, which uses 257 consistently inside that file).

### M2 (major) — `01-platform-overview.md` Open questions describe siblings that have since been fixed

`01-platform-overview.md:328–334` ("Cross-file items for a later pass") and
`:70` are now false about three files:

- `:70` and `:329` say `04-ppu-timing.md` §10 "currently disagrees"
  (`≈1.7897727 MHz`, `1 : ≈2.973`). The current `04-ppu-timing.md:244–246` gives
  `1.773448 MHz`, `3 : 1`, `16 : 5` — the platform convention.
- `:330–331` say `05-boards-and-io.md` §1 "still carries the older table
  (`1:3`, `5:16`, `186.2 ns`, Dendy 'not documented here')". The current
  `05-boards-and-io.md:43–45` is cell-for-cell identical to
  `01-platform-overview.md:44–46` (only the `*(NESdev)*` marker differs).
- `:332` says `04-ppu-memory-map.md` §1/§3 presents `$3000–$3EFF` as fact; the
  current `:19`, `:60`, `:222`, `:231` mark it NESdev rank 4. `:332` also cites
  "§14", which does not exist (that file has §1–§9).
- `:333–334` say `03-apu-overview.md` "line 207 gives the letterless `M2` as
  `15/24` without the tie-breaker above". `03-apu-overview.md:75` *does* quote
  NESdev's opposite `17/24`, and `:303` records the conflict.

`05-boards-and-io.md:361–365` carries the reciprocal note naming
`01-platform-overview.md` as the stale file, so the set contradicts itself here
even though the two files' *tables* now agree.

### M3 (minor) — PAL PPU dot clock rounded differently

`04-ppu-overview.md:34–35` prints `5.320342 MHz`; every other file prints
`5.3203424 MHz` (`01-platform-overview.md:45–46`, `05-boards-and-io.md:44–45`,
`04-ppu-timing.md:21–22`, `:245–246`). 26.601712 ÷ 5 = 5.3203424 MHz.

### M4 (minor) — PAL `ACLK` rounded differently

`01-platform-overview.md:60` says `831.304 kHz`; `03-apu-dmc.md:103` and
`03-apu-noise.md:126` say `831.303 kHz`. 26.601712 ÷ 16 ÷ 2 = 831.3035 kHz.

### M5 (minor) — NTSC CPU clock rounded differently

`1.7897727 MHz` (`01-platform-overview.md:44`, `04-ppu-timing.md:244`,
`05-boards-and-io.md:43`) vs `1.789773 MHz` (`02-cpu-core.md:34`,
`03-apu-dmc.md:80`, `03-apu-noise.md:87`, `03-apu-pulse.md:80`, `:281`,
`03-apu-triangle.md:95`, `:241`). 21.477272 ÷ 12 = 1.78977266… MHz.

### M6 (minor) — Dendy post-render line count worded two ways

`04-ppu-overview.md:35` says "50 post-render scanlines instead of 1";
`04-ppu-timing.md:193` says `240–290 (50 extra)`. NTSC has one post-render line
(240) and Dendy 51 (240–290), so both statements are consistent but the first
reads as if Dendy has 50 in total. Align the wording.

## Terminology drift

### T1 (minor) — `Implied` vs NESdev `Implicit`

`02-cpu-core.md:156`, `:170` and `02-cpu-instructions.md:26`, `:41–42` use
`Implied` as the primary term while recording that NESdev writes `Implicit`.
Conventions §4 says the *first* mention uses the NESdev term with the wiki term
in parentheses; here it is reversed and the NESdev spelling is kept only as a
note. Still `pending` in `agents/mechanical-fixes.md` item 8.

### T2 (minor) — Wiki block names without the NESdev mapping

Conventions §4 maps `Object FIFO → sprite pipeline`, `OAM Buffer → secondary
OAM`, `Obj Eval → PPU sprite evaluation`, `PatGen → pattern fetch`,
`COLOR BUFFER → palette output latch`. `04-ppu-overview.md:63–64`, `:78–80`,
`:97–98` uses `Obj Eval`, `OAM Buffer`, `Object FIFO`, `PatGen`, `COLOR BUFFER`
with no parenthetical mapping; `04-ppu-rendering.md:135`, `:241`, `:363` also
says `Object FIFO`; only `04-ppu-sprites.md:8`, `:179` maps it. The correct
pattern is already used for `Color Buffer` in `04-ppu-memory-map.md:162`.

### T3 (minor) — `/NAME` vs HDL `n_NAME` for the same net

`02-cpu-dma.md:30` writes ``/DMCAB` (HDL `n_DMCAB`)``; `03-apu-dmc.md:181`,
`:242` writes `n_DMCAB` as the primary name (likewise `n_NOUT`/`n_BOUT` at
`:22`, `:147`, `:155`, `:169`). Conventions §4: "In the specs write `/NAME` …
Do not mix `/NAME` and `NAME` for the same net."

### T4 (minor) — `Clock_rate` (redirect) still in eight correspondence tables

`nesdev-mapping.md:415–424` declares the rename `Clock_rate →
Cycle_reference_chart`; `03-apu-frame-counter.md:235`, `03-apu-overview.md:268`
and `04-ppu-timing.md:356` use the target. But `Clock_rate` is still cited in
`02-cpu-core.md:338`, `02-cpu-dma.md:302`, `03-apu-mixer.md:220`,
`03-apu-pulse.md:294`, `03-apu-triangle.md:265`, `04-ppu-overview.md:326`,
`:329`, `04-ppu-registers.md:290`, `:323`, `05-boards-and-io.md:319`. Three of
those files give no redirect note (`03-apu-mixer.md`, `03-apu-pulse.md`,
`03-apu-triangle.md`; the others name the redirect).
Orchestrator item C7 records this as still open; the two decisions
(`mechanical-fixes.md` item 4 "cite `Clock_rate` and note the redirect" vs C7
"switch to `Cycle_reference_chart") still contradict each other.

## Coverage gaps

### C1 (major) — `Specs/Review/CONSENSUS.md` is promised but does not exist

`Specs/Readme.md:40` lists it in the "What is here" table and the CONSENSUS role
writes it (`Review/agents/CONSENSUS.md:41`). `ls Specs/Review/` shows no
`CONSENSUS.md`. This is a promised deliverable that is missing at review time; it
is expected to be produced last, so treat it as **pending**, not lost.

### C2 (major) — `06-open-questions.md` is produced but not in the Readme index

`Specs/06-open-questions.md` exists (334 lines, 156 `OQ-*` entries; INDEXER
deliverable 2) and is referenced by `Review/agents/CONSENSUS.md:16`. The Readme
"What is here" table (lines 15–40) and `01-platform-overview.md` §11 index
(lines 211–228) both omit it; `01-platform-overview.md:228` says "All 20 listed
files are present", silently ignoring the 21st.

### C3 (minor) — Breaking NES Wiki pages never cited

A page-by-page check of all 86 `BreakingNESWiki/**/*.md` against `Specs/0*.md`
and `nesdev-mapping.md` found four never cited: `6502/bus_control.md`,
`6502/pc_control.md`, `PPU/youtube.md` and `tentacles.md`. The last two are
non-technical; the first two are the DL/ADL/PCL and bus-control material behind
the `HDL/Core6502` PC-load defect and are a genuine small gap in
`02-cpu-core.md` §8 (which cites the other `6502/` pages, e.g. `regs.md`,
`whatwhen.md`, `data_bus.md`).

### C4 (minor) — undocumented opcodes not enumerable

`02-cpu-instructions.md` §6 (`:250–252`) declares the undocumented set out of
scope; only `$AB` and `$DF` are given. Recorded in Open questions, so not
silent — but an assembler that accepts illegal opcodes cannot be written from
the set (see SDD 3).

### C5 (minor) — no PPU warm-up rule

`04-ppu-registers.md:274` gives the RC ignored-write window (~29658 NTSC CPU
cycles), but no file states the practical rule that rendering must not be
enabled until the reset window has elapsed / the first frame has passed. A demo
author needs it.

## Broken cross-references

Every relative Markdown link between spec files was resolved programmatically;
every `Specs/0*.md` path mentioned in the body exists. Failures:

| # | Location | Target | Problem |
|---|---|---|---|
| B1 | `Specs/Readme.md:40` | `Review/CONSENSUS.md` | file does not exist (see C1) |
| B2 | `02-cpu-instructions.md:282` | `Specs/02-cpu-core.md` §12 | cpu-core has §1–§10; the B/bit-5 rule is §4 (orchestrator C1) |
| B3 | `04-ppu-timing.md:378` | `04-ppu-overview.md` §12 | overview has §1–§7; the `/RES` material is §7 |
| B4 | `01-platform-overview.md:332` | `04-ppu-memory-map.md` §14 | memory-map has §1–§9 |

B2–B4 were found by parsing every `«file» §N` reference in the set and checking
`N` against that file's numbered headings; these are the only three that fail.

## Template / convention violations

| # | Severity | Finding |
|---|---|---|
| V1 | major | **Length target contradicts the role contracts.** `Conventions.md:137` and `VERIFIER.md:52` / `FINAL-FIXER.md:31` say 120–400; `GLOBAL-VERIFIER.md:34` (this role), `COMPILER.md:37` and `REVISION.md:33` say 120–300. 17 of the 20 sections are over 300 lines (355–400); none exceeds 400. The set satisfies the binding contract but violates the drafting role limits. Decide once. |
| V2 | minor | **Title format not uniform** (`Conventions.md` §6 item 1, `# <area> <number> — <name>`): `03-apu-frame-counter.md:1` = `# APU — Frame counter ($4017)` and `03-apu-mixer.md:1` = `# APU — Mixer and DAC outputs` have no number; `03-apu-dmc.md:1` and `03-apu-noise.md:1` are `# APU 03 — …`; the `02-cpu-*` and `04-ppu-*` files use the bare file stem as the area. |
| V3 | minor | **At the 400-line ceiling:** `04-ppu-palette.md`, `04-ppu-rendering.md`, `04-ppu-scrolling.md`, `04-ppu-sprites.md`, `05-boards-and-io.md` are exactly 400 and `04-ppu-overview.md` 399. The Final Fixer's edits must be net-neutral or the files exceed `Conventions.md` §6. |
| V4 | minor | **`06-open-questions.md` has no metadata block and none of the five fixed headings.** Acceptable for an index (INDEXER deliverable), but it should be declared as an exception in the Readme (see C2). |
| V5 | minor | `04-ppu-timing.md` Open questions still says "Length. At the 400-line ceiling"; the file is now 395 lines. |

All 20 sections have `**Status:**`, `**Scope:**`, `**Primary sources:**` and the
five fixed headings, unnumbered and in the required order.

## Unresolved blocking findings from per-section reviews

Latest verdicts at the snapshot: **consensus** for `platform`, `cpu-core`,
`cpu-dma`, `cpu-instructions`, `apu-overview`, `apu-pulse`, `apu-noise`,
`apu-fc-mixer`, `ppu-overview-regs`; **revise** for `ppu-memmap` (round 2),
`ppu-render-sprites` (round 2), `ppu-scroll` (round 2), `ppu-palette-timing`
(round 1) and `boards-io` (round 2).

However, the `revise` reports are **stale relative to the files** — the named
defects are already corrected in the current text:

| Report finding | Current text | State |
|---|---|---|
| `ppu-render-sprites` r2 Blocking 1: `04-ppu-sprites.md` §6 `PAT_ADR[4]` = `OBSEL` | `04-ppu-sprites.md:234` = `tile byte bit 0 (tile index)` | fixed; report not updated |
| `ppu-memmap` r2 Major 1: `04-ppu-memory-map.md:171` Y sign | `:171` = "written value = top scanline − 1, so the sprite first appears on scanline `value + 1`" | fixed |
| `ppu-memmap` r2 Major 2: `$3000–$3EFF` stated as fact | `:19`, `:60`, `:222`, `:231` mark it NESdev rank 4 | fixed |
| `ppu-palette-timing` r1 Blocking 1: `04-ppu-timing.md` §6 `SYNC` window | `04-ppu-timing.md:160` = `SYNC 280–304` / `278–302` | fixed |
| `ppu-palette-timing` r1 Major 2: timing §10 Dendy | `:244–246` = `1.773448 MHz`, `3 : 1` | fixed |
| `ppu-palette-timing` r1 Major 4: odd-frame `HC`/`RESCL` | `04-ppu-timing.md:205–209` correct form (`HC = HPLA_23 \| EvenOddOut` at `:207`) | fixed |
| `ppu-scroll` r2 Blocking 1: §5 coarse-Y pseudocode | `04-ppu-scrolling.md:195–200` has the correct `if / else if / else` | fixed |
| `boards-io` r2 Major 1: "the values agree" | bullet removed; replaced by the cross-file note at `05-boards-and-io.md:361–365` | fixed |

Consequences for the set:

1. There is **no Verifier report for the current revision** of
   `04-ppu-memory-map.md`, `04-ppu-rendering.md`, `04-ppu-scrolling.md`,
   `04-ppu-sprites.md` (round 3), `04-ppu-palette.md`, `04-ppu-timing.md`
   (round 2) or `05-boards-and-io.md` (round 3). Under `Conventions.md` §10 they
   have not reached consensus.
2. The review corpus needs either a re-verification pass or a CONSENSUS note
   that records the round cap and the stale-report condition explicitly.
3. The only *live* review findings carried into this report are M1 (dot
   numbering), which `ppu-render-sprites` r2 Minor 8 and `ppu-scroll` r2 Major 5
   already flag, plus the platform staleness (M2).

## SDD walk-throughs

### 1. Demo with a mid-frame scroll split

Present: `04-ppu-scrolling.md` §3 (write order), §4 (per-dot schedule), §7
(two split mechanisms); `04-ppu-registers.md` §7–§8 (`$2005`/`$2006` semantics);
`04-ppu-rendering.md` §2 (fetch windows); `04-ppu-sprites.md` §7 (sprite 0 hit);
`04-ppu-timing.md` §7 (vblank lines).

Gaps:
- **M1 blocks the one number the demo needs.** §7 says to write `$2005` in the
  line "before dot 258" and, for the full X/Y split, to land the last writes
  "before the dot-257 edge"; but `04-ppu-registers.md:252` puts the copy at dot
  257 and `04-ppu-rendering.md:84` puts the increment at 256. With no single
  convention the split lands one dot differently depending on which file the
  author read.
- The `$2005`/`$2006` bus-conflict dot windows are NESdev-only and not
  enumerated (`04-ppu-registers.md` Open questions; `04-ppu-scrolling.md` Open
  questions) — the "safe" dots for the writes are therefore not fully specified.
- Warm-up (C5) is not stated, so the first-frame behaviour is undefined for the
  author.

### 2. Implement the DMC

Present: `03-apu-dmc.md` §1–§8 (block, registers, NTSC/PAL rate table indexed 0
(428)/NTSC and 0 (398)/PAL, address/length, 7-bit output unit, reader pipeline,
DMA arbitration, `$4015`/IRQ/loop); `02-cpu-dma.md` §4 (`RDY`, 3/4-cycle fetch);
`03-apu-mixer.md` (§2 AUX B weight `DMC[6:0]`); `05-boards-and-io.md` §7
(controller-conflict workaround); `04-ppu-registers.md:235` (`$2007` collision).

Gaps:
- The DMC/OAM arbitration "window" length is not given in ACLK phases
  (`03-apu-dmc.md` Open questions; `02-cpu-dma.md` Open questions).
- `$4011` racing a timer clock is unresolved (`03-apu-dmc.md:300–306`) — an
  emulator must choose.
- PAL index 4 is documented as an exception but the Dendy value is unstated.
- None of these is blocking for a non-cycle-exact DMC; the interleaving cases
  are clearly marked.

### 3. Assembler opcode table

Present: `02-cpu-instructions.md` §1 (mode → bytes, base cycles, extra cycles),
§2 (151 official opcodes with opcode byte and cycles), §3 (extra-cycle rules),
§5 (flag effects), §6 (two undocumented examples).

Gaps:
- **Undocumented opcodes are not enumerated** (`:250–252`), so an assembler that
  accepts illegals cannot be written from the set. The omission is explicit.
- Bytes are not printed per opcode cell; a tool must join §1 by column. Usable
  but not a single lookup table.
- No machine-readable form (CSV/JSON); all tables are Markdown.

Verdict: sufficient for an official-only assembler; not sufficient for one
supporting undocumented opcodes.

## Minor

- `01-platform-overview.md:228` says `04-ppu-scrolling.md` was "created after
  earlier revisions of this index and is now included (Status: adapted (round
  1))"; the file is now `revised (round 3)`. The parenthetical Status is stale.
- `05-boards-and-io.md:47` "This table copies `01-platform-overview.md` §2
  cell-for-cell" is now true again (M2), but the claim is fragile: the two files
  must be edited together or it becomes false a third time.
- `04-ppu-overview.md` §2 block table (`:63–64`, `:78–80`) still calls the
  sprite path `Obj Eval` / `Object FIFO`; see T2.
- `03-apu-overview.md:304` calls the PAL LFO numbers "derived, not quoted";
  `03-apu-pulse.md:123` and `03-apu-triangle.md:136` repeat the same derived
  figures without the caveat. Add the caveat or cross-reference it.
- `06-open-questions.md` header paragraph (`:26`) reads "The per-file counts
  are 9, 6, 10, 8 (CPU area total 24) …". The first `9` is the platform file;
  the arithmetic is correct, but the list omits a "System" label and is easy to
  misread as a mis-sum. Cosmetic.
- `04-ppu-timing.md:378` ("§12", B3) and `:394`'s length note (V5) are the only
  leftovers in that file after the round-2 revision.

## Final fixer pass

Applied by the Final Fixer agent against the 2026-10-13 revisions listed above.
Every change is `file: what changed -> why (evidence)`. Line counts after the
pass: 01 357, 02-cpu-core 398, 02-cpu-dma 364, 02-cpu-instructions 366,
03-apu-dmc 323, 03-apu-frame-counter 279, 03-apu-mixer 253, 03-apu-noise 317,
03-apu-overview 307, 03-apu-pulse 319, 03-apu-triangle 301, 04-ppu-memory-map
292, 04-ppu-overview 397, 04-ppu-palette 400, 04-ppu-registers 357,
04-ppu-rendering 400, 04-ppu-scrolling 394, 04-ppu-sprites 399, 04-ppu-timing
400, 05-boards-and-io 400, 06-open-questions 338, nesdev-mapping 449. All
per-section files keep the five fixed unnumbered headings and are at or below
400 lines.

### M1 — PPU dot numbering (blocking)

- `04-ppu-rendering.md` §1, `04-ppu-registers.md` §10, `04-ppu-scrolling.md` §4,
  `04-ppu-timing.md` §3: added one identical, short **Dot numbering** note (NESdev
  convention, dot = `H` = 0–340; fine-Y and last coarse-X increments at dot 256;
  the horizontal `t`→`v` copy at dot 257, effect visible at 258; dummy fetches
  337–340; the HDL PCLK-edge measurements in `04-ppu-scrolling.md` §4 are one dot
  later, 257/258) -> the three files gave 256 vs 257 and 257 vs 258 for the same
  hardware edges. Evidence: `PPU/hv.md` (H counts 0–340, `HPLA_23` clears at 340)
  plus the NESdev dot labs in `PPU_rendering` ("Cycle 256" increment, "Cycle 257"
  copy); no new measurement invented.
- `04-ppu-rendering.md`: `04-ppu-scrolling.md` §4's tables are now the only
  `(HDL edge)`-labelled ones; §1's old claim that this file's numbers are used
  "in `04-ppu-sprites.md`" removed -> one convention, one set of labels.
- `04-ppu-scrolling.md` §4: replaced the file-local "NESdev dot N = this file's
  dot N+1" paragraph with the shared note; the `t`→`v` schedule table, the two
  measured-transition transcripts and the `$2005`/`$2000`-race prose now carry the
  normative labels, with `(HDL edge)` on the rows/values that were measured 257/258
  -> the file no longer contradicts `04-ppu-registers.md` §7 or
  `04-ppu-rendering.md` §2.
- `04-ppu-registers.md` §7/§10: the dot labels (coarse-X copy at 257, Y increment
  at 256) were already normative and now match the note -> no value change.
- `04-ppu-timing.md` §3: added the same note; the §6 window table already uses the
  HDL `H`-value labels, and the old Open-questions item ("dot = `H − 1`") was
  replaced by a latch-shift residue note -> the file no longer claims a second
  dot-numbering scheme.
- `04-ppu-rendering.md`, `04-ppu-scrolling.md`, `04-ppu-timing.md`, Open
  questions: deleted the obsolete "cross-file dot-256/257 disagreement" entries
  and left the unrelated fine-Y-decrement simulator-artefact item in
  `04-ppu-scrolling.md` untouched -> the disagreement is closed, the artefact
  question is not.
- `04-ppu-rendering.md` Open questions: `/FO` is now described as
  `fo_latch1`/`fo_latch2` → `fo3` → `fo_latch3`, i.e. a two-stage serial depth
  with the second stage re-timed (HDL `fsm.v` `HPosLogic`, lines
  464–466/484/498) -> the earlier "`/FO` three stages" wording contradicted the
  HDL.
- `04-ppu-sprites.md` §2: deleted the file's now-unneeded `Length` note and left
  its existing NESdev-anchored dot paragraph -> the four dot-numbering files are
  the ones that carry the shared note, and this file sits at 399 lines.

### M2 — `01-platform-overview.md` stale cross-file text

- §2 note (line 52): "`05-boards-and-io.md` §1 still carries the older table and
  must be aligned" -> "carries the same table (only local cell: the `*(NESdev)*`
  marker); the two files are edited together". Evidence: `05-boards-and-io.md`
  §1's table is cell-for-cell identical to `01` §2 at the current revisions.
- §2 bullet (line ~70): "`04-ppu-timing.md` §10 currently disagrees
  (≈1.7897727 MHz, 1:≈2.973)" -> "matches (`1.773448 MHz`, `3 : 1`)". Evidence:
  `04-ppu-timing.md` §10 now reads Dendy `1.773448 MHz`, `3 : 1`.
- Open questions "Cross-file items for a later pass" bullet: rewritten to the
  true current state (siblings agree; `04-ppu-memory-map.md` marks
  `$3000–$3EFF` rank 4; `03-apu-overview.md` quotes NESdev's opposite `17/24`)
  -> the old bullet named three fixed files and a non-existent
  `04-ppu-memory-map.md` §14 (B4).
- §11 index: added `06-open-questions.md` and `nesdev-mapping.md` rows and
  replaced the stale "All 20 listed files are present / `04-ppu-scrolling.md` …
  Status: adapted (round 1)" sentence (C2, minor V3-style staleness) -> the index
  now names all 22 documents and the two index files.
- §2 `ACLK`: `831.304 kHz` -> `831.303 kHz`; §2/§10 NTSC CPU stays
  `1.7897727 MHz`. Evidence: `APU/clkgen.md` PHI 601.46 ns = 1.662607 MHz,
  ACLK 1202.93 ns = 831.3035 kHz; 21.477272 ÷ 12 = 1.7897727 MHz.
- NESdev correspondence table: `Cycle_reference_chart` (the `Clock_rate` redirect
  resolves here) -> `Cycle_reference_chart` -> part of the set-wide C7 decision
  below.

### B2–B4 — broken section references

- `02-cpu-instructions.md` (~line 282): `Specs/02-cpu-core.md` §12 -> §4 ->
  `02-cpu-core.md` has §1–§10 and the B/bit-5 rule is in §4 ("The status register
  P"). Evidence: heading inventory of `02-cpu-core.md`; the rule text is at
  `02-cpu-core.md`:319–320.
- `04-ppu-timing.md` (~line 378): "Same conflict as `04-ppu-overview.md` §12" ->
  "the `RC` semantics item in `04-ppu-overview.md` *Open questions*" ->
  `04-ppu-overview.md` has §1–§7 and carries the `RC` conflict in its unnumbered
  Open questions.
- `01-platform-overview.md` (~line 332): the bogus `04-ppu-memory-map.md` §14
  reference disappeared with the M2 rewrite.
- Re-checked every `«file» §N` cross-reference in the set programmatically
  against the target files' numbered headings: no dangling reference remains
  (only the three above ever failed).

### C2 — index coverage

- `01-platform-overview.md` §11: added `06-open-questions.md` (declared as the
  indexer exception with no metadata block) and `nesdev-mapping.md`. Evidence:
  both files exist in `Specs/` (`06-open-questions.md` 334 lines,
  `nesdev-mapping.md` 449 lines). `Specs/Readme.md` was deliberately not touched.

### C7 — `Clock_rate` vs `Cycle_reference_chart`

- Decision: `Cycle_reference_chart` is the one spelling. Replaced the redirect
  name everywhere it appeared in the set: `02-cpu-core.md` (table + prose),
  `02-cpu-dma.md` (table), `03-apu-mixer.md` (table), `03-apu-pulse.md` (table +
  Open questions), `03-apu-triangle.md` (table + Open questions),
  `04-ppu-overview.md` (three table cells + two prose mentions),
  `04-ppu-registers.md` (Observable row + table + Open questions),
  `04-ppu-timing.md` (primary sources, correspondence, provenance),
  `05-boards-and-io.md` (correspondence), `03-apu-overview.md` (Open questions)
  and `06-open-questions.md` (three index rows). Evidence: `Clock_rate` is a wiki
  redirect to `Cycle_reference_chart` (cached `Clock_rate.html` renders the
  "Cycle reference chart" title), and `nesdev-mapping.md`'s own rename table
  already declares `Clock_rate → Cycle_reference_chart`.
- Removed the now-redundant one-line redirect notes from
  `02-cpu-core.md`, `02-cpu-dma.md`, `04-ppu-overview.md` and
  `04-ppu-registers.md`, and the redirect parentheticals in
  `03-apu-frame-counter.md`, `03-apu-mixer.md`, `04-ppu-timing.md` and
  `03-apu-overview.md` -> no mixture and no stale "redirects to X" sentence.
- `nesdev-mapping.md` *Unification notes*: the rename table is retained as the
  historical record and its heading/text now says the rename has also been applied
  to the spec files; its header parenthetical and the Table 1 note now spell the
  canonical token.
- `03-apu-frame-counter.md`: its correspondence-table cell dropped the now-stale
  `(redirect from Clock_rate)` parenthetical -> no table cites the redirect name
  any more (the name survives only in `nesdev-mapping.md`'s own rename record).
- `02-cpu-core.md` prose title "Cycle reference chart" introduced by the rename was
  removed with the redirect note -> consistent with the token spelling.

### Precision minors

- `04-ppu-overview.md` §1: `5.320342 MHz` ×2 -> `5.3203424 MHz`. Evidence:
  `PPU/pclk.md` gives PAL `PCLK` = 5320342.4 Hz; 26.601712 ÷ 5 = 5.3203424 MHz.
- `01-platform-overview.md` §2 `ACLK`: `831.304 kHz` -> `831.303 kHz`. Evidence:
  `APU/clkgen.md` PAL PHI 601.46 ns / ACLK 1202.93 ns; 26.601712 ÷ 16 ÷ 2 =
  831.3035 kHz. `03-apu-dmc.md`/`03-apu-noise.md` already used 831.303 kHz.
- NTSC CPU `1.789773 MHz` -> `1.7897727 MHz` in `02-cpu-core.md` §1,
  `03-apu-dmc.md`:80, `03-apu-noise.md`:87, `03-apu-pulse.md`:80/:281,
  `03-apu-triangle.md`:95/:241, `03-apu-overview.md` §3. Evidence: 21.477272 ÷ 12
  = 1.7897727 MHz; `01-platform-overview.md` §2 and `04-ppu-timing.md` §10 already
  printed that precision. (`05-boards-and-io.md`/`04-ppu-timing.md` already agreed;
  no other occurrence of the truncated forms remains.)

### Outstanding blocking findings whose text was already fixed (re-read, no edit)

- `04-ppu-sprites.md`:234 `PAT_ADR[4]` = tile byte bit 0 for 8×8 and `OV3` for
  8×16 — correct; no change.
- `04-ppu-memory-map.md`:171 OAM Y sign ("written value = top scanline − 1 …
  appears on scanline `value + 1`") and `:19`/`:60`/`:222`/`:231` `$3000–$3EFF`
  rank 4 — correct; no change.
- `04-ppu-timing.md`:160 `SYNC 280–304` / `278–302`, §8 `HC = HPLA_23 |
  EvenOddOut`, §9 Dendy 291 — correct; no change.
- `04-ppu-scrolling.md` §5 coarse-Y pseudocode (`29 → 0` with `NTV` toggle;
  `31 → 0` without) — correct; no change.
- `05-boards-and-io.md` Dendy Open questions item — correct after the M2/C7 edits;
  no further change.

### Residual minors (latest reports)

- `04-ppu-sprites.md` §4: no stale cell range found; `/FO` is not described in this
  file, and the "three stages" wording was the `04-ppu-rendering.md` item fixed
  above. Evidence: `fsm.v` `HPosLogic` (`fo_latch1`/`fo_latch2` → `fo3` →
  `fo_latch3`).
- `05-boards-and-io.md` §1 (~line 50): "the PPU `CLK` input ÷4" -> "the 2C02
  (`CLK` ÷4) / 2C07 (`CLK` ÷5) dot clock". Evidence: `PPU/pclk.md` (÷4 NTSC, ÷5
  PAL) and the table's own PAL column.
- `05-boards-and-io.md` Open questions "Cross-file: `01-platform-overview.md` is
  stale" bullet: rewritten to record that both tables and `04-ppu-timing.md` §10
  agree after this pass -> the reciprocal stale-note contradiction (M2's second
  half) is closed.
- `04-ppu-palette.md` §2: the "Early revisions cannot read back sprite or palette
  memory" quote was attributed to `PPU_variants`; it is on
  `Cycle_reference_chart` (verified in the cached page). Citation corrected.
- `04-ppu-palette.md` §6: the `$D` = black claim for RGB parts is correctly
  attributed to `PPU/rgb.md` ("Эмфазис …"; the RGB decoder masks are given there
  and `video_out.md` has no `$D` row); no change.
- `04-ppu-timing.md` §6: the "`/FPORCH` high" wording describes that FF's own
  level; §6's note already says the table is `HPosLogic`'s settled (latch-shifted)
  outputs, so no edit beyond the dot note.
- `03-apu-overview.md` Observable #8: "DMC DMA asserts `RDY`" -> "pulls `RDY` low"
  -> `RDY` is active-high ready (§7: `0` = core halted), so "asserts" was
  inverted in meaning.
- `03-apu-frame-counter.md` §5: "The same spread appears in mode 4" -> "Mode 4
  shows half that spread". Evidence: the same paragraph gives mode 5 as 37284 vs
  37280 (4 cycles) and mode 4 as 29828 vs 29826 (2 cycles). (The task's
  `03-apu-fc-mixer.md` §5 label names the wrong file; the sentence is in
  `03-apu-frame-counter.md` §5.)

### Cross-file items closed while re-reading (not in the work order)

- `02-cpu-core.md` Open questions, `02-cpu-dma.md` Open questions and
  `06-open-questions.md` `OQ-CPU-05`/`OQ-CPU-08`: the claims that
  `02-cpu-instructions.md` "still lists `Visual_2A03`" were stale (the file cites
  `Visual_circuit_tutorial` at :356). Corrected the two spec notes and marked the
  index entry resolved; the `Visual_2A03` page-name question itself stays recorded.
- `06-open-questions.md`: `OQ-SYS-03` rewritten from the resolved cross-file item
  to the resolved-record (status `open`, still 26 `open`/54 `documentation-gap`
  rows after the `OQ-PPU-48` re-status), and `OQ-PPU-48` re-pointed at the
  genuine latch-shift question now in `04-ppu-scrolling.md` §4. Header status
  counts were recomputed from the 156 entry rows (25 blocking, 51
  needs-hardware-test, 54 documentation-gap, 26 open); the pre-existing
  documentation-gap cell (55) was wrong by one before this pass.
- `06-open-questions.md` rows `OQ-PPU-49`: dropped the "flagged for the sibling's
  next revision" clause that the pass made false.
- `06-open-questions.md`: `OQ-SYS-03` and `OQ-CPU-08` are declared resolved
  records in the header paragraph (their original spec bullets were closed by this
  pass), and the header status counts were recomputed from the 156 entry rows
  (25 blocking, 51 needs-hardware-test, 53 documentation-gap, 27 open) after the
  `OQ-CPU-08` status moved from `documentation-gap` to `open`. The CPU area summary
  was updated to match (gaps 8, open 3).

### Verification

- Programmatic re-check of every `Specs/0*.md` relative path mention: all exist.
- Programmatic re-check of every `«file» §N` reference against the target's
  numbered headings: 0 failures (was 2, plus the B4 prose citation).
- All 20 section files still carry `**Status:**` and the five fixed unnumbered
  headings in order; `06-open-questions.md` remains the declared index exception.
- No file exceeds 400 lines; the four dot-numbering files are 400/357/394/400 and
  `04-ppu-sprites.md` 399.

### Post-consensus fixer pass (six findings from `post-fix-verification.md`)

Applied by the Final Fixer agent after the independent Verifier pass
(`Specs/Review/post-fix-verification.md`), which left condition 1 open. Each line
is `file: what changed -> why (evidence)`. `Specs/Review/CONSENSUS.md` was not
edited. Line counts after the pass: `01-platform-overview` 357,
`03-apu-overview` 307, `04-ppu-rendering` 400, `04-ppu-registers` 357,
`04-ppu-scrolling` 398, `06-open-questions` 345 (was 338).

- `04-ppu-scrolling.md` §4, table dot-257 row, first bullet and the
  "MUST increment the vertical position" note: the claim that the fine-Y step
  repeats "at each tile boundary of the dots 321–336 prefetch" removed -> it is
  **one fine-Y step per scanline at nominal dot 256** (measured 257), and
  321–336 only loads the next line's first two tiles into the shifters. Evidence:
  NESdev `PPU_scrolling` §Y increment ("fine Y is incremented at dot 256 of each
  scanline") and `PPU_rendering` §Cycles 321–336 ("the first two tiles for the
  *next* scanline"); `HDL/PPU/tilecnt.v:52,58` gates `TVSTEP` only by
  `E_EV`/`TSTEP`; the file's own transcripts (:162, :171) already showed the
  step only at 257. The false claim must go even if unexplained; the behaviour is
  fully explained, so nothing was added to `## Open questions`.
- Verification of the above, independent: re-ran the repository self-check with
  Icarus (`tilecnt_test_ntsc.bat`'s compile line, `-D RP2C02 -D ICARUS`) ->
  `tilecnt_test: TEST PASS (66390 checks)`. The VCD it wrote has top-level
  `n_FVO` change exactly three times, each at `H = 256`, 62 744 ns apart, and
  never at `H = 320–340`.
- `04-ppu-rendering.md` §2 table row 256 and the `:101` bullet: "coarse Y
  increments/is incremented" -> **fine Y** incremented (coarse Y only on the
  fine-Y 7→0 wrap) -> the file's own *Dot numbering* note and NESdev
  `PPU_scrolling` §Y increment; coarse Y has no per-line step.
- `04-ppu-registers.md` §10 `v`/`t` mutation table: Coarse Y row "Y increment at
  dot 256 and dots 321–336" -> step on the fine-Y 7→0 wrap; Fine Y row "Reset
  during dots 280–304" -> "`+1` at dot 256, one step per rendered scanline;
  reloaded during dots 280–304" -> same evidence; the Fine Y row previously did
  not mention the only per-line mutation.
- `06-open-questions.md` `OQ-PPU-68`: obsolete cross-file dot-numbering entry
  (asserted `04-ppu-rendering.md` §1 "defines dot = `H − 1`", false) rewritten as
  a resolved record of the one shared convention -> the *Dot numbering* notes in
  the four PPU files plus this pass's record; the `H − 1` sentence is gone.
- `06-open-questions.md` `OQ-BRD-02`: stale claim that `04-ppu-timing.md` §10
  "still writes `1 : 3` / `5 :…`" rewritten as a resolved record of the
  `3:1`/`16:5`/`3:1` "PPU dots per CPU cycle" convention shared by
  `01-platform-overview.md` §2, `04-ppu-timing.md` §10 and
  `05-boards-and-io.md` §1 -> re-read all three tables; no `1 : 3` cell remains.
- `06-open-questions.md` status table and area summaries: the two re-statuses
  (`documentation-gap` -> `open` for `OQ-BRD-02`; `OQ-PPU-68` kept `open` as a
  resolved record) recounted from the 156 rows -> header is now 25 / 51 / 52 / 28
  and the Boards summary "Documentation gaps only (9)" -> "(8)" plus an
  "Open (1)" line. **Discrepancy resolved:** the previous pass recorded the header
  as 53 / 27, but that pass's own table already contained 52 `documentation-gap`
  and 28 `open` rows; this pass sets the header to the count the rows actually
  have.
- `04-ppu-scrolling.md` §4 table header: `| Dot(s) (HDL edge) |` -> `| Dot(s) |`,
  with `(HDL edge; nominal 256)` moved into the measured dot-257 cell, and the
  §4 note now scopes the label to the measured cells/transcripts only -> the
  header covered nominal fetch rows (1–2 … 255–256, 337–340) whose dots are not
  HDL-edge values.
- `01-platform-overview.md` §11 index row for `03-apu-overview.md`:
  "`$4000–$4017` register map" -> "`$4000–$401F` register map" -> that file's
  scope and §4 heading, `Specs/Readme.md:23` and the `$4018–$401A` test
  registers; the remaining `$4000–$4017` mentions name NESdev's APU/I-O block
  and stay.
- Not changed: the Verifier's Finding 7 (`Readme.md` does not list itself) is a
  note, and `Readme.md` is outside this pass's remit.
