# NES / Famicom specifications

Mid-level, practical specifications of the NES/Famicom chipset, compiled from
the **Breaking NES Wiki** in this repository and aligned with **NESdev Wiki**
conventions.

This directory is the deliverable of
[issue #1392](https://github.com/emu-russia/breaks/issues/1392): an experiment in
producing specifications that are usable for **Spec-Driven Development** — demos,
games, emulators, assemblers, editors and test tools — without forcing the
reader to first master the transistor-level material.

## What is here

| File | Contents |
|---|---|
| [`Conventions.md`](Conventions.md) | Binding rules for the specs: normative language, terminology mapping, notation, traceability, agent roles |
| [`_template.md`](_template.md) | The document template every section follows |
| [`01-platform-overview.md`](01-platform-overview.md) | Console/chipset overview, clocks, buses, CPU↔PPU relationship, memory maps |
| [`02-cpu-core.md`](02-cpu-core.md) | 2A03/2A07 CPU core: programmer model, registers, addressing, PHI1/PHI2 |
| [`02-cpu-instructions.md`](02-cpu-instructions.md) | All 151 official opcodes, cycle costs, page-crossing rules |
| [`02-cpu-dma.md`](02-cpu-dma.md) | OAM DMA, DMC DMA, `RDY` halting, bus conflicts |
| [`03-apu-overview.md`](03-apu-overview.md) | APU blocks, clocks, register map `$4000–$401F` |
| [`03-apu-pulse.md`](03-apu-pulse.md) | Pulse 1/2: timers, duty, envelope, sweep, length |
| [`03-apu-triangle.md`](03-apu-triangle.md) | Triangle channel and linear counter |
| [`03-apu-noise.md`](03-apu-noise.md) | Noise channel, LFSR, period tables |
| [`03-apu-dmc.md`](03-apu-dmc.md) | DMC (DPCM): sample fetch, DMA, IRQ, output |
| [`03-apu-frame-counter.md`](03-apu-frame-counter.md) | Frame counter, 4/5-step sequences, frame IRQ |
| [`03-apu-mixer.md`](03-apu-mixer.md) | DAC, non-linear mixing, output pins |
| [`04-ppu-overview.md`](04-ppu-overview.md) | PPU architecture, internal buses DB/PD/OB, revisions |
| [`04-ppu-registers.md`](04-ppu-registers.md) | `$2000–$2007`, `$4014`, internal `v`/`t`/`x`/`w` |
| [`04-ppu-memory-map.md`](04-ppu-memory-map.md) | VRAM space, nametables, mirroring, CHR, palettes, OAM |
| [`04-ppu-rendering.md`](04-ppu-rendering.md) | Per-dot background pipeline, priority mux, odd-frame skip |
| [`04-ppu-sprites.md`](04-ppu-sprites.md) | OAM, sprite evaluation, sprite 0 hit, overflow bug |
| [`04-ppu-scrolling.md`](04-ppu-scrolling.md) | Scroll registers, increment schedule, mid-frame splits |
| [`04-ppu-palette.md`](04-ppu-palette.md) | Palette RAM, colour generation, emphasis, NTSC/PAL/RGB |
| [`04-ppu-timing.md`](04-ppu-timing.md) | Frame timing, H/V decoders, NTSC/PAL/Dendy, power-on |
| [`05-boards-and-io.md`](05-boards-and-io.md) | Motherboards, cartridge interface, controllers, CIC |
| [`nesdev-mapping.md`](nesdev-mapping.md) | Index: every spec section ↔ Breaking NES Wiki ↔ HDL ↔ NESdev page |
| [`06-open-questions.md`](06-open-questions.md) | Every unresolved question, with status and the evidence that would close it |

## How the specs were produced

The specs were compiled by a swarm of cooperating agent roles, iterating until
consensus (see [`Conventions.md`](Conventions.md) §10):

- **Compiler** — drafted each section from the Breaking NES Wiki + HDL + Docs.
- **Adapter** — aligned terminology, register naming, memory maps and document
  shape with the NESdev Wiki (via the Wayback Machine, since `nesdev.org` blocks
  non-browser clients).
- **Verifier** — independently checked fidelity to the sources, internal
  consistency, traceability and SDD usability, and filed numbered findings.
- **Revision** — resolved the findings and recorded what changed, what was
  refuted and what was deferred.
- **Indexer / Global Verifier / Final Fixer / Consensus** — built the cross-cutting
  index and open-question register, checked the set for cross-document
  consistency, reconciled the remaining mismatches, and recorded the consensus.

The complete audit trail is in [`Review/`](Review/): one report per section (all
rounds preserved, each verdict anchored to a line number and file hash), the
global consistency review, the [upstream findings](Review/upstream-findings.md)
(this effort also found defects in the repository's HDL and Docs), and the
[consensus record](Review/CONSENSUS.md). The agent role contracts and section
briefs are in [`Review/agents/`](Review/agents/).

## Using the specs

- **Emulator authors**: `02-cpu-*`, `03-apu-*` and `04-ppu-*` are written as
  implementable rules; the *Observable phenomena* tables are the test list.
- **Demo/game developers**: `01-platform-overview.md`,
  `02-cpu-instructions.md`, `03-apu-overview.md` and `04-ppu-*.md` describe the
  programming interface; the *NESdev Wiki correspondence* tables link out to the
  community documentation for everything that is not hardware-specific.
- **Tool authors** (assemblers, editors, test ROMs):
  `02-cpu-instructions.md`, `04-ppu-memory-map.md` and `05-boards-and-io.md`
  carry the enumerable tables.

Everything here is traceable: each section's `## Provenance` table names the
Breaking NES Wiki page and HDL module behind its claims, and the *Open questions*
sections say plainly what is still unproven. Where the silicon and the community
documentation disagree, the schematic's behaviour is kept as the specification
and the difference is recorded rather than smoothed over.

## Status

All 20 sections reached **consensus** between their Compiler, Adapter and
Verifier agents. Residual open questions (156 at the time of writing, 25 of them
relevant to bit-exact emulation) are listed in
[`06-open-questions.md`](06-open-questions.md) and summarised in
[`Review/CONSENSUS.md`](Review/CONSENSUS.md).
