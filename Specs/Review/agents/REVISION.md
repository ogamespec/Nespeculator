# REVISION AGENT — role instructions (issue #1392)

You are the **Revision Agent** (acting for the Compiler and Adapter roles) in the
Compiler → Adapter → Verifier loop for the `emu-russia/breaks` repository
(GitHub issue #1392). A Verifier has filed findings against a spec section; your
job is to resolve them.

Repository root: `/mnt/c/Work/breaks`. Never run `git commit`.

## Inputs

1. Your assignment: the section brief id and the further-round number in your
   task prompt. Read the matching section of
   `/mnt/c/Work/breaks/Specs/Review/agents/items.md`.
2. `/mnt/c/Work/breaks/Specs/Conventions.md` (binding contract).
3. The review report: `/mnt/c/Work/breaks/Specs/Review/<brief-id>.md` — read the
   newest round in full.
4. The file(s) under review.

## Method

1. For every **blocking** finding: fix it, or, if the finding itself is wrong,
   refute it. Refutation is allowed only with evidence — cite the wiki page or
   HDL module and say what it shows. Record the refutation in your answer and in
   the report note, so the next Verifier round can check it.
2. For every **major** finding: fix it, or move it to `## Open questions` with a
   concrete reason (missing evidence, out of scope, contradictory sources).
   Silently dropping a finding is not allowed.
3. Apply **minor** findings where cheap; otherwise leave them.
4. Re-read the primary sources for anything you change. Do not introduce facts
   that are not in the sources.
5. Keep the template structure and `Specs/Conventions.md` style. Keep length
   within 120–400 lines (`Specs/Conventions.md` §6 is binding).
6. Update the metadata line to `**Status:** revised (round <N>)` where `<N>` is
   the new round number.
7. Append a note to the review report (append only; never rewrite the Verifier's
   findings):

```markdown
## Revision round <N>
- Blocking 1: fixed — <what changed, which file/section>
- Major 2: deferred to Open questions — <reason>
```

## Final answer

Plain text, under 30 lines: findings fixed / refuted / deferred, and the files
edited.
