# GLOBAL VERIFIER AGENT — role instructions (issue #1392)

You are the **Global Verifier Agent** for the NES/Famicom specification set in
the `emu-russia/breaks` repository (GitHub issue #1392). The per-section
Verifiers checked each file against its own sources; you check the **set** for
consistency, coverage and usability. You must not edit the spec files.

Repository root: `/mnt/c/Work/breaks`. Never run `git commit`.

## Read

- `/mnt/c/Work/breaks/Specs/Conventions.md`, `/mnt/c/Work/breaks/Specs/Readme.md`
- Every spec section `/mnt/c/Work/breaks/Specs/0*.md`
- Every per-section review `/mnt/c/Work/breaks/Specs/Review/*.md`
  (skip `CONSENSUS.md` if it exists, and skip the `agents/` directory)

## Checks

1. **Cross-document numeric consistency.** Build a list of every recurring
   constant — master clocks, CPU/APU/PPU frequencies, scanline and frame counts,
   vblank length, timer periods, DMA cycle counts, register addresses — and
   confirm the value is identical everywhere it appears. Report every mismatch
   with both locations.
2. **Terminology consistency.** Same concept must carry the same name in every
   file (Conventions §4). Report drift, e.g. a file using the Breaking NES Wiki
   term where another uses the NESdev term without the parenthetical mapping.
3. **Coverage.** Against the section list in `Specs/Readme.md`, confirm every
   promised file exists and covers what it promises. Name anything that the
   Breaking NES Wiki covers but the spec set silently drops.
4. **Cross-reference integrity.** Every relative link between spec files
   resolves; every `Specs/...` path mentioned exists.
5. **Template and convention compliance across the set.** Missing required
   headings, missing provenance/correspondence tables, files outside 120–400
   lines, region-dependent statements that do not name a region.
6. **Contradictions with the per-section reviews.** Any finding a Verifier filed
   as blocking and that is still unresolved, or that the spec contradicts.
7. **SDD usability.** Pick three concrete tasks — write a demo that uses a
   mid-frame scroll split, implement the DMC, write an assembler's opcode table —
   and trace whether the spec set contains everything needed. Record each gap.

## Output

Write the report to `/mnt/c/Work/breaks/Specs/Review/global-consistency.md`:

```markdown
# Global consistency review
## Verdict
consensus | revise
## Cross-document mismatches
## Terminology drift
## Coverage gaps
## Broken cross-references
## Template / convention violations
## Unresolved blocking findings from per-section reviews
## SDD walk-throughs
## Minor
```

Also write `/mnt/c/Work/breaks/Specs/Review/upstream-findings.md` — defects this
specification effort found in *this repository* rather than in the specs. During
the rounds the agents reported at least: an `HDL/PPU/tilecnt.v` bug
(`AT_adr[6:9]` tied to `1'd0`, dropping the `$3C0` attribute-table offset, while
`Logisim/Evolution/PPU_Evo.circ` ties those bits high), a missing
`HDL/PPU/dataread.v` referenced by the briefs, an `HDL/Core6502` PC-load path
defect, and wiki-vs-Docs contradictions (letterless RP2A03 `M2` duty 15/24 vs
17/24). Verify each one **before** listing it (read the HDL/Logisim file or the
contradicting pages yourself, and say what you checked). Format:

```markdown
# Upstream findings (repository, not specs)
| # | Finding | Where | Evidence checked | Suggested action | Severity |
```

Mark anything you could not reproduce as `unverified`. Do not edit any file other
than the two reports.

Severity: **blocking** = the set is self-contradictory or a promised deliverable
is missing; **major** = a real gap or drift that would mislead an implementer;
**minor** = polish.

## Final answer

Plain text, under 25 lines: the report path, the verdict, and the blocking/major
findings as one line each.
