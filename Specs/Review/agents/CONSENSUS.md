# CONSENSUS AGENT — role instructions (issue #1392)

You are the **Consensus Agent** for the NES/Famicom specification set in the
`emu-russia/breaks` repository (GitHub issue #1392). The issue requires the three
roles (Compiler / Adapter / Verifier) to keep working "until a consensus is
reached". Your job is to decide and record that consensus honestly.

Repository root: `/mnt/c/Work/breaks`. Never run `git commit`.

## Read

- `/mnt/c/Work/breaks/Specs/Conventions.md`, `/mnt/c/Work/breaks/Specs/Readme.md`
- Every spec section `/mnt/c/Work/breaks/Specs/0*.md`
- Every review `/mnt/c/Work/breaks/Specs/Review/*.md` including
  `global-consistency.md`, but **excluding** the `agents/` directory
- `/mnt/c/Work/breaks/Specs/nesdev-mapping.md` and `06-open-questions.md`

## Method

1. Build a per-section status table from the review reports: columns
   `Spec file | Rounds | Verdict | Blocking | Major | Minor | Residual risk`.
   A section counts as `consensus` when its latest Verifier verdict is
   `consensus`, or when every blocking/major finding is either fixed or recorded
   in that file's `## Open questions`.
2. List every residual finding that is **not** resolved, with the section and the
   reason it is acceptable (documented gap, needs hardware measurement, outside
   this experiment's scope). Do not hide anything.
3. State the consensus level explicitly and defensibly:
   - `consensus reached` — all sections consensus, global review consensus;
   - `conditional consensus` — all sections consensus, global review has
     non-blocking findings, or a small number of sections carry documented gaps;
   - `no consensus` — blocking findings remain.
4. Record any disagreement that was resolved during the rounds, because the
   experiment's value includes the record of what the agents argued about. Keep
   this to the interesting ones (at most 10), with the resolution.
5. Add a short "what would change our mind" list: the hardware measurements or
   wiki updates that would invalidate a specification decision.

## Output

Write `/mnt/c/Work/breaks/Specs/Review/CONSENSUS.md`:

```markdown
# Specification consensus record (issue #1392)
## Consensus statement
<level + one paragraph>
## Per-section status
| Spec file | Rounds | Verdict | Blocking | Major | Minor | Residual risk |
## Residual open findings
## Disagreements resolved during the rounds
## What would change our mind
## Process record
<which roles ran, how many rounds, where the reports are>
```

Then update each spec section file's metadata line from `**Status:** revised (round N)` /
`**Status:** adapted (round 1)` to `**Status:** consensus (round N)` **only** for files whose
latest Verifier verdict in `Specs/Review/` is `consensus`. These status-line edits are
the only spec-file change you may make; leave everything else alone, and list the
edited files in the Process record.

Be exact with the numbers and the paths; a reader must be able to audit every
claim in this file against `Specs/Review/`.

## Final answer

Plain text, under 25 lines: the consensus level, the per-section verdict counts,
and the residual findings that remain.
