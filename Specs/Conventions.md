# Specification conventions (issue #1392)

This document is the **binding contract** for every spec file under `Specs/`.
Compiler, Adapter and Verifier agents MUST follow it. If a rule here has to be
broken for a good reason, note the reason in the section's *Open questions*.

---

## 1. Purpose and audience

The specs are practical, mid-level specifications of the NES/Famicom hardware.
They must be:

- **Useful to programmers** who build demos, games, emulators, assemblers,
  editors and test tools (Spec-Driven Development). A reader who knows only
  registers, instructions and clock cycles MUST be able to act on them.
- **Faithful to the silicon.** The *Breaking NES Wiki* (transistor/logic level,
  this repository) is the primary source of truth.
- **Aligned with the community.** Register names, memory maps, timing units and
  page topics follow the *NESdev Wiki* conventions, so the community can use the
  specs without a translation layer.

The "golden mean": **no transistor-level narration, no vague hand-waving.**
Digital-logic detail appears only where it explains an observable phenomenon
(a bug, a race, an unusual timing), and then it is condensed into a rule a
programmer or emulator author can implement.

## 2. Normative language

- **MUST / MUST NOT** — required for correctness. Violating it is a defect.
- **SHOULD / SHOULD NOT** — recommended; deviations are allowed if documented.
- **MAY** — optional.
- **Observed on hardware / by the schematic** — a statement derived from the
  Breaking NES Wiki or its HDL, not from documentation.

Do not present folklore as fact. Anything not traceable (section 7) either goes
to *Open questions* or is explicitly marked **unconfirmed**.

## 3. Sources of truth and conflict resolution

| Rank | Source | Role |
|---|---|---|
| 1 | `BreakingNESWiki/**/*.md` (Russian original) and `BreakingNESWiki_DeepL/**` (EN translation) | Silicon truth: what the chip actually does |
| 2 | `HDL/**/*.v` (Icarus-verified, die-perfect) | Executable restatement of rank 1; use to confirm/refute |
| 3 | `Docs/**` (board revisions, chipset, patents, video signal) | Board/system-level facts |
| 4 | NESdev Wiki (via the Wayback Machine, section 9) | Naming, register layout, community-expected behaviour |

Rules:

- If ranks 1–3 and rank 4 disagree about **naming or presentation**, follow
  NESdev (that is the Adapter's job) and record the mapping.
- If they disagree about **behaviour**, the silicon wins. Describe the silicon
  behaviour and add a note: `> NESdev says X; this repository's schematic shows Y`.
- Never silently drop a Breaking NES Wiki behaviour because NESdev does not
  mention it. It is exactly those behaviours that justify this experiment.

## 4. Terminology mapping (Breaking NES Wiki → this spec)

First mention of a concept uses the NESdev term, with the Breaking NES Wiki term
in parentheses. Afterwards the NESdev term is used.

| Breaking NES Wiki | This spec / NESdev |
|---|---|
| Square 0 / Square 1 | Pulse 1 / Pulse 2 (APU pulse channels) |
| DPCM | DMC (delta modulation channel) |
| SoftCLK, "programmable timer" | Frame counter (APU frame sequencer) |
| `$4017` LFO modes 4/5 | 4-step / 5-step frame counter sequence |
| Object FIFO, OB bus, Object Buffer | Sprite pipeline, sprite shifters, secondary OAM |
| Object Evaluation (Obj Eval) | PPU sprite evaluation |
| OAM Buffer | Secondary OAM |
| SPR0HIT (actually `/SPR0HIT`) | Sprite 0 hit flag |
| SPR_OV / M4_OVZ | Sprite overflow flag |
| CLIP_B / CLIP_O | Background / sprite left-column masking |
| Pattern Generator (PatGen) | Pattern fetch + pattern shift registers |
| COLOR BUFFER | Palette output latch (index → emphasis/colour) |
| CRAM, Color Generator | Palette RAM, PPU palettes |
| MUX | Pixel priority multiplexer |
| PAR (Picture Address Register) | part of the internal `v` scroll/address register |
| FV/FH/TV/TH paired registers | `x` (fine X), coarse X / fine Y / coarse Y inside `v` and `t` |
| H/V counters, HV decoder | PPU dot counter / scanline counter, `HVLATCH` |
| PCLK / `/PCLK` | PPU dot clock (`PCLK` high/low state), 1 dot = 1 pixel |
| ACLK1, /ACLK2, ACLK3A…ACLK5 | APU clock phases (see `Specs/03-apu-overview.md`) |
| PHI0/PHI1/PHI2, M2 | CPU clock and its phases; `M2` is the external CPU clock |
| RDY | `RDY` input (DMA halt) |
| DBE | Data bus enable |
| T0…T7 (6502), "extra counter" | 6502 cycle within instruction / T-state |
| SB, DB, ADH, ADL | 6502 internal buses — keep the repository names, mark as internal |
| APU (the chip) | CPU (2A03/2A07) when talking about the processor |
| Cartridge | Cartridge / Game Pak |
| CIC | CIC lockout (10NES) |
| Debug registers `$4018`–`$401A` | 2A03 test registers |

Signal polarity: the Breaking NES Wiki writes active-low names as `/NAME` (or
occasionally `#NAME`); the HDL uses `n_NAME`. In the specs write `/NAME` and
state the active level once (`/RD` — active low). Do not mix `/NAME` and
`NAME` for the same net.

## 5. Notation

- Hexadecimal `$2000`, binary `%1010_1010`, decimal plain. Address ranges
  `$2000–$2007`. Never `0x2000` (that is a source-code convention, not a spec
  convention) — except inside Verilog/C snippets.
- Registers: use the NESdev full names with the address, e.g. `PPUCTRL ($2000)`.
  Bit fields are shown as a named diagram:

  ```
  7  bit  0
  ---- ----
  VPHB SINN
  ```

- Bits are numbered from 0 (LSB) to 7 (MSB). "Bit 7 set" means value `%1xxxxxxx`.
- Timing units: **CPU cycle** (one PHI0 period), **APU cycle** (ACLK, 2 CPU
  cycles), **PPU dot** (one PCLK period), **scanline** (341 dots on both NTSC
  and PAL), **frame**. State the master clock (NTSC 21.477272 MHz,
  PAL 26.601712 MHz) wherever a frequency is given.
- A statement about NTSC/PAL/Dendy MUST name the region; Dendy is a PAL
  famiclone with an NTSC-like CPU:PPU ratio and MUST NOT be lumped in with PAL.
- Tables beat prose for anything enumerable (register bits, memory maps, cycle
  counts, frame timing).

## 6. Required document structure

Every spec file uses the template in `Specs/_template.md`:

1. Title `# <area> <number> — <name>`
2. Metadata block: `Status`, `Scope`, `Primary sources`
3. Content sections, numbered, each one self-contained
4. `## Observable phenomena` — the "why a programmer cares" list: each item is a
   phenomenon, its cause in one or two sentences, and what to do about it
5. `## Programming / implementation notes` — normative checklist for an
   emulator author or tool writer
6. `## NESdev Wiki correspondence` — table `Topic | NESdev Wiki page`
7. `## Provenance` — table `Claim area | BreakingNES Wiki page | HDL module`
8. `## Open questions` — anything unverified, contradictory or blocking

Length target: **120–400 lines** per file. "Concise" means no filler, no
transistor-level narration and no duplicated material — it does not mean fewer
tables. Go below 120 lines only for a genuinely small interface; split a file
above ~450 lines.

## 7. Traceability (hard requirement)

- Every non-obvious factual claim MUST be attributable to a rank 1–3 source.
  The `## Provenance` table covers this at section granularity; individual
  surprising claims get an inline citation like `(PPU/scroll_regs.md)`.
- Every content section MUST appear in the NESdev correspondence table with the
  NESdev page(s) a reader would otherwise consult.
- Numbers that a reader will implement (timer periods, scanline counts, cycle
  costs, bit positions) MUST be stated explicitly, never as "see the schematic".

## 8. Language and style

- English (the community language; the Russian original stays in
  `BreakingNESWiki/`).
- Short declarative sentences. No marketing, no filler, no "it is worth noting
  that". Tables and code blocks over paragraphs.
- Addressed to an engineer implementing the behaviour, not to a reader of the
  schematic. Wording like "look at the schematic" is forbidden.
- Include a minimal code snippet only when it removes ambiguity (e.g. the
  palette-corruption workaround, the $2006/$2005 write order).

## 9. Accessing the NESdev Wiki

`www.nesdev.org` is behind a bot check and returns HTTP 403 to non-browser
clients. Use the Wayback Machine form, which works:

```
https://web.archive.org/web/2024/https://www.nesdev.org/wiki/<Page_Name>
```

Use NESdev only for naming, layout and cross-checking. Do **not** paste NESdev
text into the specs: the specs are derived from the Breaking NES Wiki and link
out to NESdev instead.

## 10. Agent roles (process)

- **Compiler** — drafts a section from Breaking NES Wiki + HDL + Docs. Owns
  technical content. Writes the files.
- **Adapter** — makes the draft match NESdev conventions: terminology, register
  naming, document shape, correspondence tables. Edits the files. Must not
  weaken silicon-derived behaviour.
- **Verifier** — independent. Checks fidelity, internal consistency, NESdev
  alignment, traceability and practical usability. Writes a report under
  `Specs/Review/` and returns a verdict. **Never edits the spec files.**
- **Revision** — addresses a Verifier's blocking/major findings. Edits the
  files, appends to the file's `Status` line and to
  `Specs/Review/<section>.md` a `## Revision round N` note.

Consensus for a section = Verifier reports no *blocking* findings and the
remaining *major* findings are all resolved or moved to *Open questions* with a
reason. Maximum two revision rounds per section; anything still open after that
is recorded as a known limitation, not silently dropped.
