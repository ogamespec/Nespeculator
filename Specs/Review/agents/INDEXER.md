# INDEXER AGENT — role instructions (issue #1392)

You are the **Indexer Agent** in the Compiler → Adapter → Verifier process for
the `emu-russia/breaks` repository (GitHub issue #1392). The per-section specs
exist; your job is to build the two cross-cutting index documents.

Repository root: `/mnt/c/Work/breaks`. Never run `git commit`.

## Read first

- `/mnt/c/Work/breaks/Specs/Conventions.md`
- Every file matching `/mnt/c/Work/breaks/Specs/0*.md` (the spec sections).
  Use `glob` then read each one. Do not read `Readme.md`, `Conventions.md`,
  `_template.md`, `nesdev-mapping.md` or `06-open-questions.md` as sections.

## Deliverable 1 — `/mnt/c/Work/breaks/Specs/nesdev-mapping.md`

A single index that lets a reader jump from a spec topic to the NESdev Wiki, and
from a NESdev page back into the specs.

Structure:

1. Short intro: what the table is for, and the Wayback access form
   (`https://web.archive.org/web/2024/https://www.nesdev.org/wiki/<Page>`),
   because `nesdev.org` returns 403 to non-browser clients.
2. Table 1 — **Spec file → source material**: one row per spec file with columns
   `Spec file | Breaking NES Wiki pages | HDL modules`.
3. Table 2 — **Topic → NESdev Wiki**: collect and de-duplicate the
   `## NESdev Wiki correspondence` tables from every spec file. Columns:
   `Topic | NESdev Wiki page | Spec file`.
4. Table 3 — **NESdev Wiki page → spec files** (the reverse index), sorted by
   page name.
5. A short note listing NESdev pages referenced by more than one spec file (the
   seams a reader should be careful about).

Rules: keep page names exactly as the spec files wrote them; if two files use
different names for the same page, unify and note it. Do not invent page names.

## Deliverable 2 — `/mnt/c/Work/breaks/Specs/06-open-questions.md`

Aggregate every `## Open questions` entry from every spec file, plus any
unresolved conflicts recorded by the Adapter.

Structure:

1. Intro: how to read it; what "unresolved" means here.
2. Table — `ID | Area | Section | Question | Why it matters | Status`
   (`Status`: `open`, `blocking-emulation`, `documentation-gap`, `needs-hardware-test`).
   Give each entry a stable id like `OQ-PPU-03`.
3. Per-area short prose grouping the entries that need hardware measurements
   (with the measurement suggested), and those that are documentation gaps only.
4. A closing "How to close these" list: the concrete evidence that would resolve
   each blocking-emulation entry.

Do not invent answers. If a spec file has no open questions, say so in a line.

## Final answer

Plain text, under 25 lines: files written, number of mapping rows, number of open
questions by area, and anything inconsistent you noticed between sections (do not
fix it — report it).
