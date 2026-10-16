# Open questions — cross-section index (issue #1392)

This file aggregates the `## Open questions` sections of all 20 spec files under
`Specs/`, plus the cross-file conflicts those sections record. Each entry keeps a
stable id of the form `OQ-<AREA>-NN`; the *Section* column names the spec file it
came from, so the entry can always be traced back to its full text.

**What "unresolved" means here.** An entry is unresolved when no rank 1–3 source
(`BreakingNESWiki/`, `HDL/`, `Docs/`) settles it, or when two sources disagree
and the spec deliberately kept both readings. It does not mean the spec is
wrong: it means a reader must not treat that statement as silicon-confirmed
(`Conventions.md` §3, §7). NESdev-only facts, HDL defects, missing repository
pages and behaviour that needs a hardware measurement all appear here; the spec
itself notes the working assumption it uses in each case.

**Status values.**

| Status | Meaning | Count |
|---|---|---|
| `blocking-emulation` | An emulator or tool cannot be made cycle-/behaviour-accurate without a decision or a missing number. | 25 |
| `needs-hardware-test` | The answer needs a measurement, a scope capture or a test ROM on real hardware. | 51 |
| `documentation-gap` | A source, page or signal description is missing from this repository or the wiki; the behaviour may be known but is not traceable. | 52 |
| `open` | Recorded as unresolved, with no immediate emulation consequence (includes editorial/scope notes). | 28 |

Two entries (`OQ-SYS-03`, `OQ-CPU-08`) are kept as resolved records: the
final consistency pass closed their original spec bullets, and their rows say so
rather than being dropped and renumbering the rest of the index. The post-consensus
fixer pass re-statused two more stale rows the same way: `OQ-PPU-68` (dot
numbering — all four PPU files share one convention) and `OQ-BRD-02` (`CPU:PPU`
column convention — the three tables agree).

All 20 section files have an `## Open questions` section; none is empty. The
per-file counts are 9, 6, 10, 8 (CPU area total 24), 7, 5, 6, 5, 9, 4, 4 (APU
area total 40), 9, 9, 8, 10, 8, 6, 10, 9 (PPU area total 69) and 14 (boards),
giving 156 entries.

## The entries

| ID | Area | Section | Question | Why it matters | Status |
|---|---|---|---|---|---|
| `OQ-SYS-01` | System / platform | `01-platform-overview.md` | Letterless RP2A03 M2 duty cycle | `APU/old_2a03.md` (rank 1) says the early part is 15/24 vs 17/24 later; `Docs/NES_Famicom_Board_Revisions.md` (rank 3) says letterless is 17/24. The tie-breakers side with the Docs file: NESdev `CPU_pinout`… | `needs-hardware-test` |
| `OQ-SYS-02` | System / platform | `01-platform-overview.md` | Dendy clocks | `Dendy.md` (rank 1) only says the CPU runs "close to the NTSC NES" with PAL video; the §2 figures are NESdev rank 4: `Cycle_reference_chart` gives the UMC `UA6527P` as `26.60~ MHz ÷ 15 = 1.773448 MHz` (113⅔… | `needs-hardware-test` |
| `OQ-SYS-03` | System / platform | `01-platform-overview.md` | Cross-file items (resolved in the final pass) | The four PPU spec files now share one dot-numbering convention (increments at dot 256, the horizontal copy at dot 257), and the sibling cross-references of `01-platform-overview.md`… | `open` |
| `OQ-SYS-04` | System / platform | `01-platform-overview.md` | `$3000–$3EFF` nametable mirror and `$3F20–$3FFF` palette mirror | NESdev states both. The palette mirror is strongly implied by rank 1–3: the CRAM address is 5 bits (`{CGA4,CGA1,CGA0}` × `{CGA3,CGA2}`), so rows `$3F20–$3FFF` alias the 32 entries (`04-ppu-palette.md`). The… | `documentation-gap` |
| `OQ-SYS-05` | System / platform | `01-platform-overview.md` | CPU:PPU phase relationship | No source states whether the CPU/PPU phase offset after `/RES` is deterministic, which a cycle-accurate scheduler needs. | `blocking-emulation` |
| `OQ-SYS-06` | System / platform | `01-platform-overview.md` | Interrupt vector addresses | The wiki documents the vector-generation logic but not the addresses (`$FFFA–$FFFF`); those above are NESdev/6502 knowledge. | `documentation-gap` |
| `OQ-SYS-07` | System / platform | `01-platform-overview.md` | `$4018–$401A` access on revision E | The wiki describes the 2A03 debug interface in general and its absence in the letterless part; whether all `DBG`-capable revisions behave identically is not stated. | `documentation-gap` |
| `OQ-SYS-08` | System / platform | `01-platform-overview.md` | PPU revision gaps | `2C02H` is only "fixes some glitches"; the rumoured 2C01/2C06 debug PPUs are unconfirmed and 2C02A/B/C X=255 sprite behaviour is not fully characterised. | `needs-hardware-test` |
| `OQ-SYS-09` | System / platform | `01-platform-overview.md` | PAL NES CPU/PPU pairing | The board-revision document marks several board/chip combinations as undocumented (e.g. which NTSC boards shipped 6113-family locks); not resolvable from this repository's sources. | `documentation-gap` |
| `OQ-CPU-01` | CPU (2A03/2A07) | `02-cpu-core.md` | Power-on register values | The Breaking NES Wiki describes the reset *sequence*, not the initial contents of A/X/Y/S/P. Post-reset `S`/`I` follow from that sequence; the `A/X/Y = $00` and `C/Z/D/V/N = 0` values are NESdev… | `needs-hardware-test` |
| `OQ-CPU-02` | CPU (2A03/2A07) | `02-cpu-core.md` | HDL defect in the PC load path | `HDL/Core6502/Readme.md` records that the core never loads a memory-sourced low byte into `PCL` (reset PC becomes `04FF` instead of the vector), traced to the `DL→ADL→PCL` timing in… | `open` |
| `OQ-CPU-03` | CPU (2A03/2A07) | `02-cpu-core.md` | `RDY` implementation | `dma.md` says `RDY` is only effective on read cycles. `pads.v` passes `RDY` through combinationally (`assign RDY_frompad = RDY;`), but the read/write dependence does exist in `dispatch.v` `ReadyRW`… | `blocking-emulation` |
| `OQ-CPU-04` | CPU (2A03/2A07) | `02-cpu-core.md` | Open-bus `$FF` mechanism | `busmux.v:52-55` precharges only the internal `SB`/`DB`/`ADL`/`ADH`, and `data_bus.v:38` (`bufif0 (DB_Ext, dor_latch_nq, RD)`) is the only external driver, disabled while `RD=1` — so the HDL gives no path for… | `blocking-emulation` |
| `OQ-CPU-05` | CPU (2A03/2A07) | `02-cpu-core.md` | `Visual_2A03` page name | No NESdev wiki page or Wayback capture is known (the CDX index lists only `Visual_2C02` and `Visual_circuit_tutorial`). `02-cpu-instructions.md` still lists `Visual_2A03` and should be corrected in a revision… | `documentation-gap` |
| `OQ-CPU-06` | CPU (2A03/2A07) | `02-cpu-core.md` | Undocumented opcodes | The Wiki describes the decoder and random logic but this section does not enumerate illegal opcodes; see `02-cpu-instructions.md`. | `documentation-gap` |
| `OQ-CPU-07` | CPU (2A03/2A07) | `02-cpu-dma.md` | Get/put parity | The wiki derives `ACLK1`/`/ACLK2` from `PHI1`/`PHI2` with a reset-defined state; NESdev reports random power-on parity. Unresolved. | `blocking-emulation` |
| `OQ-CPU-08` | CPU (2A03/2A07) | `02-cpu-dma.md` | NESdev page name for the die-level material (resolved in the final pass) | `Visual_2A03` is not a NESdev Wiki page — the Wayback Machine has no archived article for that URL; `Visual_circuit_tutorial` is the page that documents the `apu_clk1`/`apu_/clk2` node names. `02-cpu-dma.md`, `02-cpu-core.md` and `02-cpu-instructions.md` now all cite it, so the no-bullet record stands resolved. | `open` |
| `OQ-CPU-09` | CPU (2A03/2A07) | `02-cpu-dma.md` | NESdev's own DMC timing wording | `APU_DMC` says (explicitly as speculation) that the DMC unit holds `RDY` low for a fixed 4 cycles; `DMA` gives 3 or 4 depending on alignment. This file follows the 3/4 model, which matches the schematic's… | `blocking-emulation` |
| `OQ-CPU-10` | CPU (2A03/2A07) | `02-cpu-dma.md` | OAM extra-cycle wording | `PPU_OAM` writes "513 CPU cycles (+1 on odd CPU cycles)"; `DMA` attributes the +1 to the random power-on get/put parity. The random-parity model is the one used here (§2, §3); the two are only reconciled if… | `blocking-emulation` |
| `OQ-CPU-11` | CPU (2A03/2A07) | `02-cpu-dma.md` | 2A07 mechanism | NESdev reports the 2A07 has no extra-read conflict, but `APU/pal.md` marks 2A07 `RDY` and register select as TBD, so the fix cannot be stated from rank 1–2 evidence. | `needs-hardware-test` |
| `OQ-CPU-12` | CPU (2A03/2A07) | `02-cpu-dma.md` | Conflict resolution electrically | The wiki shows both drivers enabled but not whether the result is wired-AND, OR or undefined, so the latched byte is called unreliable. | `needs-hardware-test` |
| `OQ-CPU-13` | CPU (2A03/2A07) | `02-cpu-dma.md` | Load vs reload scheduling | The load fetch's 2nd-APU-cycle halt schedule and the 3/4-cycle fetch table (§4) are NESdev rank-4 numbers; the wiki schematics (`APU/dpcm.md` images) show the control latches but no textual labels, and the… | `needs-hardware-test` |
| `OQ-CPU-14` | CPU (2A03/2A07) | `02-cpu-dma.md` | Joypad `/OE` across adjacent reads | NESdev's claim that a contiguous block of repeated `$4016`/`$4017` reads clocks the controller once (rank 4) is not confirmed by the schematic pages read (`APU/regs.md`, `APU/pads.md`). | `needs-hardware-test` |
| `OQ-CPU-15` | CPU (2A03/2A07) | `02-cpu-dma.md` | DMC/OAM arbitration window | The window length is unspecified: `APU/waves.md` describes the "window" without giving a number of `ACLK` phases. | `documentation-gap` |
| `OQ-CPU-16` | CPU (2A03/2A07) | `02-cpu-dma.md` | Dendy/clone DMA timing | Dendy/clone DMA timing is not covered by the wiki; NESdev reports UA6527P clones end samples 1 APU cycle later, so no Dendy rule is given here. | `documentation-gap` |
| `OQ-CPU-17` | CPU (2A03/2A07) | `02-cpu-instructions.md` | `JMP ($xxFF)` page-wrap | Neither the Wiki pages, the `Docs/6502` instruction list nor the HDL walkthroughs document the indirect-JMP page-wrap bug (high byte fetched from the same page when the pointer low byte is `$FF`). It is… | `needs-hardware-test` |
| `OQ-CPU-18` | CPU (2A03/2A07) | `02-cpu-instructions.md` | Full undocumented opcode set | Only `$AB` (ATX) and `$DF` (DCP) have walkthroughs; the rest (aliases, `NOP` variants, unstable instructions, DMA interaction) is out of scope until the repository documents it. | `documentation-gap` |
| `OQ-CPU-19` | CPU (2A03/2A07) | `02-cpu-instructions.md` | Dummy-read address on a page cross | The sources state the +1 cycle (`PGX`) but not the address presented during it. §4 uses the un-carried address (the community-documented behaviour); confirm against the schematic before treating it as… | `needs-hardware-test` |
| `OQ-CPU-20` | CPU (2A03/2A07) | `02-cpu-instructions.md` | `Docs/6502/6502 Instruction Set.htm` errors | Its appendix lists `DEC absolute` (`CE`) as 3 cycles instead of 6, repeats the preceding instruction's mnemonic in the single-line entries for `DEX` (`DEC CA`), `DEY` (`DEC 88`) and `PLP` (`PHP 28`), leaves… | `documentation-gap` |
| `OQ-CPU-21` | CPU (2A03/2A07) | `02-cpu-instructions.md` | `/PRDY` Branch-T0 skip | Its purpose is unknown (`decoder.md`) and it is not stated whether it also applies to interrupt entries. | `documentation-gap` |
| `OQ-CPU-22` | CPU (2A03/2A07) | `02-cpu-instructions.md` | NESdev source coverage | The local NESdev cache's `CPU_instructions.html` capture is an empty Wayback landing page (4.6 kB, no article body), the live page returns HTTP 403, and the Internet Archive was offline for the cache date, so… | `documentation-gap` |
| `OQ-CPU-23` | CPU (2A03/2A07) | `02-cpu-instructions.md` | Interrupt hijacking and the delayed `/IRQ` response | Phenomena 10 and 11 have no rank 1–3 source in this repository; they need a schematic/HDL check (the `brk.v` and `ir.v` sampling points) before they can be called silicon-derived. | `needs-hardware-test` |
| `OQ-CPU-24` | CPU (2A03/2A07) | `02-cpu-instructions.md` | What are the per-register RMW consequences for `$2000–$2007` and `$4014`? | A scope note: those effects belong to the PPU and DMA sections and are only referenced here. | `open` |
| `OQ-APU-01` | APU | `03-apu-dmc.md` | Are the DMC rate tables and the `$10*L + 1` byte count exactly right? | They are derived from the `dpcm.md`/`pal.md` PLA masks and the `DPCM_FreqLFSR` model (all 32 NTSC entries match the community tables) plus the community length formula and the HDL load value; a test-ROM run would pin them. | `needs-hardware-test` |
| `OQ-APU-02` | APU | `03-apu-dmc.md` | `$4011` racing a timer clock (NESdev vs schematic) | NESdev `APU_DMC` notes that when `$4011` is written at the same time as the timer outputs a clock, the output level is occasionally not changed properly. The HDL gives the direct load priority over the delta… | `blocking-emulation` |
| `OQ-APU-03` | APU | `03-apu-dmc.md` | Power-on output level (NESdev vs schematic) | NESdev says the DMC level is loaded with 0 on power-up. The HDL clears the 6-bit counter on `RES` (`RevCounterBit .clear(RES)`, `dpcm.v`), but the LSB is a plain `RegisterBit` with no reset (`out_reg`), so the… | `blocking-emulation` |
| `OQ-APU-04` | APU | `03-apu-dmc.md` | Why does playback end one byte after the counter reaches 0? | The `SOUT` latch phase behind it is inferred from the HDL; cycle-level hardware evidence is missing, so end-of-sample timing may be off by one byte. | `documentation-gap` |
| `OQ-APU-05` | APU | `03-apu-dmc.md` | What does `$4015` bit 4 do when a DMC DMA fetch is already in flight? | Aborting a fetch mid-flight is not documented in the wiki, so disabling DMC during an active fetch has no specified outcome. | `documentation-gap` |
| `OQ-APU-06` | APU | `03-apu-dmc.md` | Do the 2A07 DPCM `/RDY` address-bus glitch and the late-RP2A03G DMC DMA bug have signal-level descriptions? | `Docs/NES_Famicom_Board_Revisions.md` mentions both, but the wiki describes neither, so neither can be modelled. | `documentation-gap` |
| `OQ-APU-07` | APU | `03-apu-dmc.md` | Does the DMC's effect on `$4016`/`$4017` controller reads belong here or in `02-cpu-dma.md`? | A scope note: the CPU-side conflict is specified in `02-cpu-dma.md` §4 and only referenced here. | `open` |
| `OQ-APU-08` | APU | `03-apu-frame-counter.md` | Are the 4-step / 5-step totals 29828 / 37284 CPU cycles (repository) or 29830 / 37281 (NESdev)? | An ad-hoc (uncommitted) Icarus run produced 7457/7456/7458 intervals, matching the NESdev deltas rather than the uniform repository table; a committed testbench would settle the exact period, which drives frame-IRQ timing. | `blocking-emulation` |
| `OQ-APU-09` | APU | `03-apu-frame-counter.md` | Does a `$4017` write take effect after 3 or 4 CPU cycles? | It depends on APU-cycle alignment; no committed testbench measures the latency (`HDL/Framework/Icarus/apu/softclk.v` says it 'is not a unit test right now'), so emulators may be off by one cycle. | `blocking-emulation` |
| `OQ-APU-10` | APU | `03-apu-frame-counter.md` | Does a Dendy famiclone use the 2A03 frame-counter step table? | No repository source documents the Dendy frame-counter PLA, so Dendy frame-IRQ timing is unconfirmed. | `needs-hardware-test` |
| `OQ-APU-11` | APU | `03-apu-frame-counter.md` | What do the 13 timing-loop images in `cycles.md` say about SoftCLK? | The appendix is images only, so any internal timing loop involving SoftCLK could not be transcribed. | `documentation-gap` |
| `OQ-APU-12` | APU | `03-apu-frame-counter.md` | What is the exact frame-counter mode/mask register value at power-on? | It is inferred from the `RegisterBit` latch initialisation in `HDL/APU/common.v`, not stated in the wiki. | `open` |
| `OQ-APU-13` | APU | `03-apu-mixer.md` | Is there an `/AUX` pin or net? | The assignment brief mentions one, but `APU/pads.md` documents only AUX A (pin 1) and AUX B (pin 2); the reference needs clarification. | `documentation-gap` |
| `OQ-APU-14` | APU | `03-apu-mixer.md` | Are the AUX B per-LSB weights 4:2:1 or 3:2:1 (triangle:noise:DMC)? | The repository model uses 4:2:1, NESdev's `APU_Mixer` table uses 3:2:1; `Scripts/Readme.md` warns the 4:2:1 ratio may be a simplified-FET model artefact, so the mixed output level is unresolved. | `blocking-emulation` |
| `OQ-APU-15` | APU | `03-apu-mixer.md` | What are the board-level gains and filters that sum AUX A and AUX B? | `dac.md` marks the AUX B network TBD and the filter figures in the notes are NESdev, so an emulator's output stage is not repository-derived. | `documentation-gap` |
| `OQ-APU-16` | APU | `03-apu-mixer.md` | Is `ExtRes` 100 Ω, 75 Ω, or board-revision dependent? | `dac.md` says 100 Ω is 'usual' while its own measurement used 75 Ω, so the DAC transfer scale is uncertain. | `needs-hardware-test` |
| `OQ-APU-17` | APU | `03-apu-mixer.md` | How large is the error from modelling the 39 kΩ MOSFET as linear? | The real device has a cubic V–I relationship (`Scripts/Readme.md`), and the error at high codes is unknown. | `documentation-gap` |
| `OQ-APU-18` | APU | `03-apu-mixer.md` | Does the 2A07 die have different DAC transistor sizes or output levels? | No 2A07 DAC or level difference is documented; the 2A03 tables are reused unchanged. | `needs-hardware-test` |
| `OQ-APU-19` | APU | `03-apu-noise.md` | Should the noise period-table derivation be re-run in CI? | The wiki prose does not print the tables; the 32 NTSC/PAL values come from the PLA masks plus the `NOISE_FreqLFSR` model and reproduce the community tables, but the derivation is not automated. | `open` |
| `OQ-APU-20` | APU | `03-apu-noise.md` | What is the letterless RP2A03's full noise period table? | Only the period-15 value (2046 CPU cycles) is quoted from `Docs/NES_Famicom_Board_Revisions.md`; the decoder mask is not given in the wiki, so the rest is unverified. | `needs-hardware-test` |
| `OQ-APU-21` | APU | `03-apu-noise.md` | Power-on random-LFSR state (NESdev vs schematic) | NESdev `APU_Noise` states the 15-bit shift register is loaded with the value 1 on power-up. The schematic gives it no reset (`NOISE_RandomLFSRBit` uses the plain `RegisterBit`, `noise.v`), so ranks 1–2 leave… | `blocking-emulation` |
| `OQ-APU-22` | APU | `03-apu-noise.md` | Mode-1 sequence length (NESdev vs schematic) | NESdev `APU_Noise` says mode 1 produces a **93- or 31-step** sequence, depending on the shift-register state when bit 7 is set; §4 quotes 93 steps. Simulating the HDL feedback (`RIN = sout[0] XOR sout[6]`)… | `open` |
| `OQ-APU-23` | APU | `03-apu-noise.md` | Is mode 1 on the 2A07 different from the 2A03? | `pal.md` shows a change to the period PLA only and the tap-selection logic appears identical, but no page documents 2A07 mode 1 separately. | `documentation-gap` |
| `OQ-APU-24` | APU | `03-apu-overview.md` | NESdev page names | All titles in the correspondence table were checked against the local Wayback cache (`/tmp/nesdev-cache/`, snapshots of December 2024 – January 2025). Two draft names were wrong and are corrected here:… | `documentation-gap` |
| `OQ-APU-25` | APU | `03-apu-overview.md` | `$4015` bit 5 | `$4015` bit 5 is undriven in the HDL, so this spec calls it open bus; the wiki never states its behaviour explicitly, so the whole read value stays uncertain. | `blocking-emulation` |
| `OQ-APU-26` | APU | `03-apu-overview.md` | `$4016`/`$4017` read value | The wiki describes only the `/IN0`/`/IN1` strobes; how many data bits the external device drives onto the bus is board-level and is not in the wiki pages read here. | `documentation-gap` |
| `OQ-APU-27` | APU | `03-apu-overview.md` | `$4016`/`$4017` in test mode | NESdev `CPU_pinout` states that with pin 30 (`TST`/`DBG`) high, `$4016`/`$4017` read as open bus on the 2A03G. This repository's HDL does not gate the strobes with `DBG` (`regs.v`: `not(n_R4016, pla[23])`,… | `needs-hardware-test` |
| `OQ-APU-28` | APU | `03-apu-overview.md` | `M2` duty-cycle conflict | `old_2a03.md` gives 17/24 for the 2A03G and 15/24 for the letterless 2A03 (SR4 vs SR3); NESdev `CPU_pinout` assigns the fractions the other way round (15/24 for the 2A03E/G/H, 17/24 for the letterless part)… | `needs-hardware-test` |
| `OQ-APU-29` | APU | `03-apu-overview.md` | PAL LFO frequencies | The PAL figures are derived here from the 8313 / 16625 / 24939 CPU-cycle intervals in `clkgen.md`; the wiki states frequencies (240/120/192/96 Hz) only for the 2A03, so the PAL 200/100/160/80 Hz numbers are derived, not quoted. | `documentation-gap` |
| `OQ-APU-30` | APU | `03-apu-overview.md` | `cycles.md` | `cycles.md` is an appendix of 13 images ("uroboros" feedback loops) with no text; it yielded no citable statements, a source limitation rather than a section defect. | `documentation-gap` |
| `OQ-APU-31` | APU | `03-apu-overview.md` | `RDY2` | `RDY2` appears only in `core.md`; there is no `RDY2` net in `HDL/APU/apu_top.v`, so the 2A07 "debug WAIT" behaviour is wiki-only and unconfirmed. | `documentation-gap` |
| `OQ-APU-32` | APU | `03-apu-overview.md` | `$401B–$401F` | `$401B–$401F` are not decoded by the APU; whether the CPU-side read returns pure open bus on retail hardware is not documented in the pages read. | `documentation-gap` |
| `OQ-APU-33` | APU | `03-apu-pulse.md` | Pulse mute threshold | The schematic mutes a pulse channel at `t < 4` (`SW_UVF` = timer bits 10:2 all zero, `square.v`); NESdev documents `t < 8` (`APU_Pulse`, and the same "current period less than 8" mute under `APU_Sweep`). This… | `blocking-emulation` |
| `OQ-APU-34` | APU | `03-apu-pulse.md` | Sweep enable polarity and bit index | `square.md` lists `SWDIS` as fed by register `WR1[3]`; the HDL feeds it from `$4001` bit 7 through an inverter (`swdis_reg .d(DB[7]) .nq(SWDIS)`, `square.v`), so bit 7 = 1 enables the sweep process. Icarus… | `open` |
| `OQ-APU-35` | APU | `03-apu-pulse.md` | Does the first sweep update really happen at the first half-frame borrow? | Whether the community model's 'divider starts at P' is equivalent was not verified on hardware. | `needs-hardware-test` |
| `OQ-APU-36` | APU | `03-apu-pulse.md` | Dendy CPU clock | Dendy (PAL famiclone, NTSC-like CPU:PPU ratio) is not covered by `APU/clkgen.md`. NESdev `Cycle_reference_chart` (and `APU_Pulse`) give `fCPU = 1.773448 MHz` for Dendy (26.601712 MHz ÷ 15) — a community value,… | `needs-hardware-test` |
| `OQ-APU-37` | APU | `03-apu-triangle.md` | Is the `$400B` same-cycle boundary handling cycle-accurate? | It is modelled by the `Reload_FF` pipeline in `TRIANGLE_Control`; the community model's 'reload at the next quarter-frame clock' was not verified on hardware. | `needs-hardware-test` |
| `OQ-APU-38` | APU | `03-apu-triangle.md` | How do the `$401A` test write, `LOCK` and the debug readout interact? | `regs.md` documents `$401A` only as a test write that loads the triangle output counter; the interaction is unspecified. | `documentation-gap` |
| `OQ-APU-39` | APU | `03-apu-triangle.md` | Is the analog `t = 0`/`t = 1` level a filter effect the schematic does not model? | It is a filter effect, not a digital one, so the 'halfway between 7 and 8' level cannot be reproduced from the digital model. | `open` |
| `OQ-APU-40` | APU | `03-apu-triangle.md` | Dendy CPU clock | Dendy (PAL famiclone with an NTSC-like CPU:PPU ratio) is not covered by `APU/clkgen.md`. NESdev `Cycle_reference_chart` (and `APU_Triangle`/`APU_Pulse`) give `fCPU = 1.773448 MHz` for Dendy (26.601712 MHz ÷ 15) — a… | `needs-hardware-test` |
| `OQ-PPU-01` | PPU | `04-ppu-memory-map.md` | `AT_ADR` constant field: HDL defect and cross-spec notation | `HDL/PPU/tilecnt.v` ties `AT_adr[6]`, `[7]`, `[8]` and `[9]` to `1'd0`, dropping the `$3C0` attribute-table offset; every attribute fetch would then read tile data instead of an attribute byte. In the… | `open` |
| `OQ-PPU-02` | PPU | `04-ppu-memory-map.md` | Attribute-fetch notation across specs | `04-ppu-rendering.md` §3 now prints the attribute address as `%10NN 1111 yyy xxx` with the constant field at bits `[9:6]`, agreeing with §4 here. `04-ppu-scrolling.md` §2 still gives an `AT_ADR` composition… | `open` |
| `OQ-PPU-03` | PPU | `04-ppu-memory-map.md` | Mirroring and CIRAM A10 | No Breaking NES Wiki page describes nametable mirroring, CIRAM A10 or four-screen cartridges; §3 is assembled from NESdev `Mirroring` and the board/HDL material. A board- or die-level decode page would remove… | `documentation-gap` |
| `OQ-PPU-04` | PPU | `04-ppu-memory-map.md` | `$3000–$3EFF` and `$3F20–$3FFF` mirrors: different ranks | NESdev states both, but they do not have the same evidence. The `$3F20–$3FFF` mirror is rank 2: `HDL/PPU/cram.v` `CRAM_Block` takes a 5-bit `CGA` and indexes a 32-entry `cram[CGA]`, so any address in… | `documentation-gap` |
| `OQ-PPU-05` | PPU | `04-ppu-memory-map.md` | Attribute right-edge behaviour | The exact dots at which the redundant right-edge fetches cross into the next nametable, and whether the last-row fetch ever uses the merged row-0/4 word line, are not documented in the Wiki. | `needs-hardware-test` |
| `OQ-PPU-06` | PPU | `04-ppu-memory-map.md` | OAM decay timings | `PPU/oam.md` leaves the degradation timing as TBD; an emulator cannot reproduce decay without them, so model OAM as static for now. | `blocking-emulation` |
| `OQ-PPU-07` | PPU | `04-ppu-memory-map.md` | OAM attribute bits 2–4 read-back | The cell array has no storage for them (§8; rank 1 `PPU/oam.md` documents the column skip) and NESdev `PPU_OAM` states that they "always read back as 0" through OAMDATA, adding that bits which have decayed… | `needs-hardware-test` |
| `OQ-PPU-08` | PPU | `04-ppu-memory-map.md` | CHR write decoding | "Many cartridges only decode reads" is a board-level generalisation; this repository has no per-board table of which boards ignore PPU `/WR`. | `documentation-gap` |
| `OQ-PPU-09` | PPU | `04-ppu-memory-map.md` | Backdrop override (direct colour) gating | `HDL/PPU/vram_ctrl.v` resolves the condition: `TH_MUX = ~(latched ~BLNK \\| nPA[0] \\| … \\| nPA[5])` with `.nPA(n_PA_out[13:8])` (`ppu_top.v`), i.e. `TH_MUX = BLNK & PA[13:8] = %111111` — any address in… | `open` |
| `OQ-PPU-10` | PPU | `04-ppu-overview.md` | `RC` semantics | `PPU/pads.md` describes an `/RES` latch held until `RESCL = 1`, implying registers stay cleared until the end of vblank; `HDL/PPU/pads.v` deliberately changed `RC` to `RES` (clear only during the reset pulse)… | `blocking-emulation` |
| `OQ-PPU-11` | PPU | `04-ppu-overview.md` | OAMADDR corruption on the 2C02G | The Wiki has no page describing `$2003` write corruption; it is documented only in `Docs/NES_Famicom_Board_Revisions.md` §4 and NESdev `PPU_registers` (source address unclear, "usually $20", taken from the CPU… | `needs-hardware-test` |
| `OQ-PPU-12` | PPU | `04-ppu-overview.md` | `$2004` readability | Neither `PPU/oam.md` nor `PPU/regs.md` states which revisions can read OAMDATA. The claim (added on 2C02G) rests on `Docs/NES_Famicom_Board_Revisions.md` §4 and NESdev; the `/R4` path exists in the HDL because… | `needs-hardware-test` |
| `OQ-PPU-13` | PPU | `04-ppu-overview.md` | 2C05 identification | The `PPUSTATUS` bits 4–0 identifier is NESdev (`PPU_registers` §"2C05 identifier"), which notes that no data has been taken directly from the PPUs — only what arcade games expect — so §6.1 is **unconfirmed**… | `needs-hardware-test` |
| `OQ-PPU-14` | PPU | `04-ppu-overview.md` | PAL 2C07 details | The Wiki documents only the UMC 6538 clone, not the reference 2C07: its 71-scanline vblank and red/green emphasis swap come from `Docs/NES_Famicom_Board_Revisions.md` §4 and NESdev. A 2C07 die page is missing. | `documentation-gap` |
| `OQ-PPU-15` | PPU | `04-ppu-overview.md` | Pins 20 and 40 | `PPU/pads.md` does not list the ground and supply pins; the pinout table takes them from NESdev `PPU_pinout`, which draws the composite part with pin 20 = GND and pin 40 = +5V. Worth a second source if the pin… | `documentation-gap` |
| `OQ-PPU-16` | PPU | `04-ppu-overview.md` | Power-on values | `PPU/pads.md` states the `Reset FF` is `x` at power-on; NESdev supplies measured initial values. Emulators should treat PPUSTATUS bit 7 and OAM/palette contents as unspecified rather than fixed. | `open` |
| `OQ-PPU-17` | PPU | `04-ppu-overview.md` | Emphasis order on PAL/Dendy | The bit swap is not stated by any Breaking NES Wiki page; NESdev `Cycle_reference_chart` ("the color emphasis bits on the PAL NES have their red and green bits in PPUMASK swapped") and `Errata` agree, so it is documented… | `needs-hardware-test` |
| `OQ-PPU-18` | PPU | `04-ppu-overview.md` | HDL coverage | RGB and clone revisions are not implemented in `HDL/PPU/` (only `RP2C02`/`RP2C07` macros), so §6.2/§6.3 rest on `PPU/rgb.md` and `PPU/umc6538.md`. | `documentation-gap` |
| `OQ-PPU-19` | PPU | `04-ppu-palette.md` | Greyscale vs. colour burst | `PPU/cram.md`'s signal table describes `B/W` as "disable Colour Burst"; the HDL clears only the chroma bits and leaves `BURST` running. Which is silicon? A TV without burst switches to monochrome by itself, so… | `blocking-emulation` |
| `OQ-PPU-20` | PPU | `04-ppu-palette.md` | Phase table direction | The phase-per-colour table in §3 is derived from the `PPU/video_out.md` decoder-table permutation; the wiki itself never lists phases per colour code. Confirm against a hardware phase measurement or the Visual… | `needs-hardware-test` |
| `OQ-PPU-21` | PPU | `04-ppu-palette.md` | Emphasis level per revision | `PPU/video_out.md` gives 0.746 for the 2C02; `VDAC.TXT` shows slightly different ratios for 2C07/6538/6528. A per-part emphasis factor is not stated by any source. | `needs-hardware-test` |
| `OQ-PPU-22` | PPU | `04-ppu-palette.md` | DAC level labels | `PPU/video_out.md` labels the levels with strings such as `0D`, `1D`, `2D`, `3D`, `00`, `10` and `20 / 30` but never defines how those map onto the 64 palette values, so the exact level-per-entry mapping is… | `documentation-gap` |
| `OQ-PPU-23` | PPU | `04-ppu-palette.md` | Palette contents at power-on | No Breaking NES Wiki page states the initial CRAM contents; the HDL initialises all 32 entries to 0. NESdev treats them as unspecified. | `open` |
| `OQ-PPU-24` | PPU | `04-ppu-palette.md` | RGB PPU completeness | `PPU/rgb.md` is reconstructed from top-layer die photographs only and states that parts are guessed; the RGB decoders' masks are given but not the resulting colours. | `documentation-gap` |
| `OQ-PPU-25` | PPU | `04-ppu-palette.md` | Direct colour during rendering | The Wiki says the trick requires rendering to be off; the HDL gates `TH/MUX` with the palette-range and rendering-off condition, but the exact CPU/PPU alignment sensitivity is untested. | `needs-hardware-test` |
| `OQ-PPU-26` | PPU | `04-ppu-palette.md` | Length | This file is 400 lines, at the 120–400-line target of `Conventions.md` §6; the address/alias, phase and DAC tables fill it, and the brief assigns all three colour generators to one file. | `open` |
| `OQ-PPU-27` | PPU | `04-ppu-registers.md` | Ignored writes after reset | NESdev states that writes to `$2000`, `$2001`, `$2005`, `$2006` are ignored for ~29658 (NTSC) / ≈33132 (PAL) CPU cycles after reset. No Breaking NES Wiki page states this window and the HDL has no such gate —… | `blocking-emulation` |
| `OQ-PPU-28` | PPU | `04-ppu-registers.md` | `RC` timing | `PPU/pads.md` implies registers stay cleared until `RESCL` (vblank end); `HDL/PPU/pads.v` deliberately clears only during the `/RES` pulse and documents the change as a bug fix. NESdev `PPU_power_up_state`… | `blocking-emulation` |
| `OQ-PPU-29` | PPU | `04-ppu-registers.md` | 2C02G OAMADDR corruption | Documented only in `Docs/NES_Famicom_Board_Revisions.md` §4 and NESdev; the Wiki has no page for it, the copied row's source address is only described as "usually `$20`", and NESdev states it is not known… | `needs-hardware-test` |
| `OQ-PPU-30` | PPU | `04-ppu-registers.md` | `$2004` readability revision | The Wiki does not state which parts can read OAMDATA; the "added on 2C02G" claim comes from rank-3/rank-4 sources (`PPU_variants` gives 2C02E and earlier plus the 2C05 series as unreadable), and the HDL models… | `needs-hardware-test` |
| `OQ-PPU-31` | PPU | `04-ppu-registers.md` | `$2005`/`$2006` bus conflict | The exact dot windows in which the second `$2006` write is ANDed with the current value are known from NESdev only; `PPU/par.md` shows the shared-bus coupling but not the dot numbers. | `documentation-gap` |
| `OQ-PPU-32` | PPU | `04-ppu-registers.md` | Palette read reliability | NESdev reports corruption on one of the four CPU/PPU alignments depending on the console; no schematic finding explains it, and `PPU/cram.md` describes the Color Buffer without it. | `needs-hardware-test` |
| `OQ-PPU-33` | PPU | `04-ppu-registers.md` | Palette-read bits 7–6: open bus vs the HDL | NESdev says the byte returned for a `$3F00–$3FFF` read carries PPU open bus in bits 7–6, but `HDL/PPU/cram.v` drives `{2'b00, cram[CGA]}`. An implementation must choose: model the NESdev open bus (accurate to… | `blocking-emulation` |
| `OQ-PPU-34` | PPU | `04-ppu-registers.md` | 2C05 register swap | `PPU_variants` states the swap directly ("PPUCTRL and PPUMASK swap locations", RC2C05-01), so its existence is documented; no source, rank 1–3 or NESdev, states its polarity (which register answers which… | `needs-hardware-test` |
| `OQ-PPU-35` | PPU | `04-ppu-registers.md` | DMC/`$2007` read conflict | The extra increment is NESdev-documented; `PPU/vram_ctrl.md` is short and does not cover CPU/DMA arbitration. | `documentation-gap` |
| `OQ-PPU-36` | PPU | `04-ppu-registers.md` | Length | This file is inside the 120–400-line range of `Conventions.md` §6. The extra material is the eight per-register tables and the `t` field maps; splitting the register file from the internal `v`/`t`/`x`/`w`… | `open` |
| `OQ-PPU-37` | PPU | `04-ppu-rendering.md` | `dataread.v` does not exist | The Data Reader is spread over `par.v`, `tilecnt.v`, `bgcol.v`, `pamux.v` and `scroll_regs.v` (the wiki splits it the same way). | `documentation-gap` |
| `OQ-PPU-38` | PPU | `04-ppu-rendering.md` | H-decoder strobe vs NESdev dot number | The `HPLA` strobes of `PPU/hv_decoder.md` are 0–1 dot earlier than the NESdev dot window of the signal they drive: `I/OAM2` H 0–63 (clear cycles 1–64), `OBJ_READ` H 256–319 (sprite fetches 257–320), `/FO` H… | `open` |
| `OQ-PPU-39` | PPU | `04-ppu-rendering.md` | `F/AT` and `#F/NT` placement | The masks give `#F/NT` at H ≡ 0,1 (mod 8) and `F/AT` at H ≡ 2,3 (mod 8), while the NESdev NT and AT accesses occupy dots 1–2 and 3–4; the latch stage that aligns each strobe with its access dot is in no source… | `open` |
| `OQ-PPU-40` | PPU | `04-ppu-rendering.md` | Shift-register decay | NESdev describes the pattern/attribute registers decaying to 0 within about two frames and the mid-frame artefacts that causes; the HDL `BGC_*` cells hold their last value, and the decay rate is unverified by… | `blocking-emulation` |
| `OQ-PPU-41` | PPU | `04-ppu-rendering.md` | `EXT`-pin erasure trick | `PPU/mux.md` marks the grounding-`EXT` + slave mode trick "not confirmed experimentally". Recorded as unconfirmed. | `needs-hardware-test` |
| `OQ-PPU-42` | PPU | `04-ppu-rendering.md` | Sprite 0 hit on the pre-render scanline | Neither the wiki nor the HDL forbids the hit there; it depends on whether `/VIS` and the residual shift-register content satisfy §6. Needs a test ROM. | `needs-hardware-test` |
| `OQ-PPU-43` | PPU | `04-ppu-rendering.md` | `/FO` semantics | Whether `/FO` is "output enable" or "the segment in which the pattern byte is valid" is not stated; the HDL gates `F_TA`/`F_TB` with it, so the pattern phase is modelled as two adjacent 2-dot windows. | `documentation-gap` |
| `OQ-PPU-44` | PPU | `04-ppu-rendering.md` | Length | 400 lines, inside the 120–400-line target of `Conventions.md` §6; the extra material is the `HPLA` dot table and the per-stage provenance. | `open` |
| `OQ-PPU-45` | PPU | `04-ppu-scrolling.md` | Coarse-Y 30/31 and the `PPU/tilecnt.md` count limit | §5 gives one rule: the step path wraps 29 → 0 with the nametable toggle (measured; modelled by `tilecnt_test.v`), while 30/31 exist only after a `$2005` Y load of 240–255. `PPU/tilecnt.md` gives the Tile… | `open` |
| `OQ-PPU-46` | PPU | `04-ppu-scrolling.md` | `$2007` during rendering | NESdev documents a simultaneous coarse X and Y increment; `PPU/vram_ctrl.md` describes the same coupling in prose, but the generated `tilecnt.v` produces only the per-scanline step, so the affected dot windows… | `documentation-gap` |
| `OQ-PPU-47` | PPU | `04-ppu-scrolling.md` | Fine-Y decrement on the pre-render line | The measurement shows fine Y dropping between dots 256 and 257 of the pre-render line (1 → 0) before the dot-281 reload; NESdev documents only the reload. The `TVSTEP` gate in `tilecnt.v` has two… | `open` |
| `OQ-PPU-48` | PPU | `04-ppu-scrolling.md` | Dot numbering and the latch shift | The four PPU spec files now use one convention (the `Dot numbering` note; increments at dot 256, the horizontal copy at dot 257); §4's own measured transitions are `(HDL edge)`, one dot later (257/258). Whether the HDL's `dlatch` phase also shifts the other `HPLA`-derived windows is not stated by any page. | `needs-hardware-test` |
| `OQ-PPU-49` | PPU | `04-ppu-scrolling.md` | Coarse-Y wrapping wording in `04-ppu-memory-map.md` | That file's §3 says "Coarse Y = 30/31 do not wrap"; the intended meaning is that 30 and 31 do not trigger the 29→0 nametable toggle, not that the counter stops. The final pass confirmed the two files agree on the stepping rule. | `documentation-gap` |
| `OQ-PPU-50` | PPU | `04-ppu-scrolling.md` | Sprite pipeline and the dot-258 copy | Whether the horizontal copy and the sprite pipeline can skew by more than one dot (affecting sprite 0 hit on the following line) was not verified. | `needs-hardware-test` |
| `OQ-PPU-51` | PPU | `04-ppu-sprites.md` | `dataread.v` is not in the checkout | The sprite pattern addressing the brief assigns to it lives in `par.v` (V inversion) and `ppu_top.v`. | `documentation-gap` |
| `OQ-PPU-52` | PPU | `04-ppu-sprites.md` | Overflow walk vs the HDL model | `PPU/obj_eval.md` states only that `SPR_OV` is set when more than 8 sprites are on the line and that copying then stops; `HDL/PPU/obj_eval.v` stops evaluation at the 9th in-range sprite outright. The diagonal… | `blocking-emulation` |
| `OQ-PPU-53` | PPU | `04-ppu-sprites.md` | Exact 9th-sprite detection window | Whether the overflow check consumes the rest of the evaluation window or is cut short by `E/EV` at dot 256, and how many `n`/`m` steps fit in the remaining dots, is not stated by any source here; the… | `needs-hardware-test` |
| `OQ-PPU-54` | PPU | `04-ppu-sprites.md` | OAM decay timings | `PPU/oam.md` explicitly marks the cell degradation timing as TBD ("Calculate or measure cell degradation timings"). No refresh-rate rule can be derived from ranks 1–3. | `blocking-emulation` |
| `OQ-PPU-55` | PPU | `04-ppu-sprites.md` | `$2004` read behaviour during rendering | `PPU/oam.md` describes the buffer and its `R4` latch but not the evaluator's ownership of the address during evaluation; the behaviour in §1/§2 is NESdev-observed. The HDL drives `n_OAM` from the evaluator… | `needs-hardware-test` |
| `OQ-PPU-56` | PPU | `04-ppu-sprites.md` | Sprite overflow / OAMADDR interaction on 2C02G-2C02H | NESdev reports that starting evaluation with OAMADDR ≠ 0 copies the bytes at `OAMADDR & $F8` over OAM row 0 (`PPU_sprite_evaluation` notes, `Errata`); no wiki page covers it. The related behaviour — an… | `needs-hardware-test` |
| `OQ-PPU-57` | PPU | `04-ppu-sprites.md` | Decoder strobe vs dot number in the evaluation window | `PPU/hv_decoder.md` puts `S/EV` at H = 65 and `E/EV` at H = 255, and the H counter equals the dot number (0–340, `PPU/hv.md`). Both pass the two-latch `HPosLogic` chain, so the `S/EV` and `E/EV` edges reach… | `open` |
| `OQ-PPU-58` | PPU | `04-ppu-sprites.md` | Sprite X past the right edge | Sprites at X ≥ 249 keep only the pixels that fall in the visible window (dots 0–255), and the HDL lane stops when `n_VIS` goes high (`HDL/PPU/fifo.v`; NESdev `PPU_OAM`). Whether the die re-arms and drops the… | `documentation-gap` |
| `OQ-PPU-59` | PPU | `04-ppu-sprites.md` | High Y bytes and wrap-around | Writing `$EF`–`$FF` in the Y byte hides a sprite: the in-range test `Y ≤ V < Y + h` selects it only for post-render scanlines (240 and later) when `Y ≥ $EF` (`HDL/PPU/obj_eval.v` compares an 8-bit `V`).… | `needs-hardware-test` |
| `OQ-PPU-60` | PPU | `04-ppu-sprites.md` | Length | This file is 400 lines, inside the 120–400-line target of `Conventions.md` §6; the extra material is the OAM/attribute tables and the per-item provenance. | `open` |
| `OQ-PPU-61` | PPU | `04-ppu-timing.md` | PAL picture window | `PPU/pal.md` gives the 2C07 BPorch FF rows as 256 (set) and 4 (reset), which produces a picture gate of dots 4–255 (252 dots) rather than the ~277 dots of real PAL active video. `PPU/pal.md` also says… | `needs-hardware-test` |
| `OQ-PPU-62` | PPU | `04-ppu-timing.md` | First-frame parity | The Even/Odd flip-flops have no reset in the HDL (`sdff`), and `PPU/fsm.md` does not state their power-on state, so which of the first two frames is short is not derivable from ranks 1–2. NESdev treats it as… | `open` |
| `OQ-PPU-63` | PPU | `04-ppu-timing.md` | `/RES` hold time | `PPU/pads.md` says the reset latch stays set until `RESCL = 1` (end of vblank), implying registers stay cleared for most of a frame; `HDL/PPU/pads.v` deliberately clears only during the pulse. This is the same… | `blocking-emulation` |
| `OQ-PPU-64` | PPU | `04-ppu-timing.md` | Ignored-write window | The ~29 658 / ≈33 132 CPU-cycle window is NESdev-only; no Wiki page or HDL gate implements it. | `blocking-emulation` |
| `OQ-PPU-65` | PPU | `04-ppu-timing.md` | Dendy CPU clock | `BreakingNESWiki/Dendy.md` says the CPU is "close to NTSC" but gives no exact frequency, so the Dendy row of §10 is approximate. The video timing (312 lines, `/VSET` at 291) is from `PPU/umc6538.md`. | `needs-hardware-test` |
| `OQ-PPU-66` | PPU | `04-ppu-timing.md` | PAL 2C07 reference timing | The Wiki documents the 2C07 only through `PPU/pal.md` and the UMC 6538 differences; the 71-line vblank figure comes from `Docs/NES_Famicom_Board_Revisions.md` §4 and matches the decoder rows here. | `documentation-gap` |
| `OQ-PPU-67` | PPU | `04-ppu-timing.md` | `VPLA_2` vs `VPLA_8` for `VC` | The NTSC `HVCounterControl` uses `VPLA_2` (line 261) and the PAL version uses `VPLA_8` (line 311); the V decoder's ±1-dot simulation spread means the exact last dot of the frame is model dependent. A hardware… | `needs-hardware-test` |
| `OQ-PPU-68` | PPU | `04-ppu-timing.md` | Dot numbering across the PPU specs (resolved in the final pass) | All four PPU spec files now use one convention, recorded by the *Dot numbering* note: dot = `H` = 0–340, the fine-Y and last coarse-X increments at dot 256, the `t`→`v` copy at dot 257 (effect visible at 258), dummy fetches 337–340; the `(HDL edge)` rows of `04-ppu-scrolling.md` §4 are the same events one `PCLK` edge later (257/258). No file defines dot = `H − 1`. | `open` |
| `OQ-PPU-69` | PPU | `04-ppu-timing.md` | Length | This file is 400 lines, at the 120–400-line target of `Conventions.md` §6; the four H/V decoder tables and the per-line window table are the enumerable data the brief requires. | `open` |
| `OQ-BRD-01` | Boards & I/O | `05-boards-and-io.md` | The controller protocol has no rank-1 source | The wiki has no controller or `$4016` prose page; the protocol above comes from the `Docs/` schematics plus NESdev and needs a die-level re-check. Specifically NESdev-only and unconfirmed here: the `$40` value… | `needs-hardware-test` |
| `OQ-BRD-02` | Boards & I/O | `05-boards-and-io.md` | `CPU:PPU` column convention (resolved in the final pass) | `01-platform-overview.md` §2, `04-ppu-timing.md` §10 and `05-boards-and-io.md` §1 now write the column the same way — numerically **PPU dots per CPU cycle** (`3:1`, `16:5`, `3:1`) — and each states the convention explicitly. The old "CPU cycles : dots" rows (`1 : 3`, `5 :…`) are gone; the entry is kept as a resolved record. | `open` |
| `OQ-BRD-03` | Boards & I/O | `05-boards-and-io.md` | Dendy clocks are rank-4 | `Dendy.md` (rank 1) says only that the CPU runs "close to the NTSC NES" with PAL video; the §1 ÷15 / `1.773448 MHz (563.9 ns)` / `3:1` cells are NESdev (`Cycle_reference_chart`: `26.60~ MHz ÷ 15 = 1.773448… | `needs-hardware-test` |
| `OQ-BRD-04` | Boards & I/O | `05-boards-and-io.md` | Which 40H368 tri-state is which | The wiki text ("one tri-state is the audio amplifier, one inverts PA13") disagrees with the `Famicom_Evo.circ` comment that "A6/Y6 are used as an inverter for sound"; the `n_G1`/`n_G2` grouping of Y1–Y4 /… | `documentation-gap` |
| `OQ-BRD-05` | Boards & I/O | `05-boards-and-io.md` | Shift order vs the repository diagram | `Docs/NES/NESController/timing.jpg` labels the 3rd and 4th shifts "Start, Select" while NESdev (and the cc65 convention) says Select then Start; the diagram is assumed wrong until verified on hardware. | `needs-hardware-test` |
| `OQ-BRD-06` | Boards & I/O | `05-boards-and-io.md` | Famicom pad-2 microphone address | NESdev places the microphone on `$4016` D2 and `Famicom_Evo.circ` models a MIC line on the pad-2 subcircuit, but the wiki gives no address and no die-level source exists here. The spec therefore states only… | `documentation-gap` |
| `OQ-BRD-07` | Boards & I/O | `05-boards-and-io.md` | CIC defeat methods | NESdev documents only the "disconnect pin 4" conversion; the reversible two-wire method that lets the reset button drive `/RES` directly has no rank-1 source here and is marked **unconfirmed** in §3. | `documentation-gap` |
| `OQ-BRD-08` | Boards & I/O | `05-boards-and-io.md` | Famicom pad clock/latch drive | An `M2`-gated inverting 368 driving the pad `CLK`/`LATCH` lines is not in `Famicom_Evo.circ` (`P4`/`P5` expose only `OUT_0`, `S4016_*`/`S4017_*`, `MIC`) or `nes.v` (`CtrlPort`/`LS368` model the data path only). | `documentation-gap` |
| `OQ-BRD-09` | Boards & I/O | `05-boards-and-io.md` | NES-101 CIC tie-off | The §3 claim that two of the four CIC edge pins sit at +5 V and PPU D4 (the rest floating) is in none of the cached NESdev pages inspected (`Cartridge_connector`, `CIC_lockout_chip`, `CIC_lockout_chip_pinout`,… | `needs-hardware-test` |
| `OQ-BRD-10` | Boards & I/O | `05-boards-and-io.md` | PAL controller protection diodes | Which models and board revisions carry the port diodes is undocumented here; NESdev names only FRA/HOL/NOE. | `documentation-gap` |
| `OQ-BRD-11` | Boards & I/O | `05-boards-and-io.md` | Mapper documentation is absent | No repository file describes mappers, iNES numbers, bus conflicts or mapper mirroring, so §9 is NESdev background, not silicon truth. | `documentation-gap` |
| `OQ-BRD-12` | Boards & I/O | `05-boards-and-io.md` | Expansion-audio mixing levels | `nes.v` calls the summing a black box ("some kind of magic") and the GPM/AV Famicom level change is qualitative; no gain figures exist. | `needs-hardware-test` |
| `OQ-BRD-13` | Boards & I/O | `05-boards-and-io.md` | Famicom 60-pin audio pins | The wiki has no cartridge-connector page; pins 45/46 as "audio from CPU"/"audio to RF" come from NESdev plus `Famicom_Evo.circ` `P1.SoundIn`/`SoundOut`. | `documentation-gap` |
| `OQ-BRD-14` | Boards & I/O | `05-boards-and-io.md` | Vs. System / PlayChoice-10 / FamicomBox detail | The repository has stub pages only (one link each), so these platforms are not specified beyond the revision tables. | `documentation-gap` |

## By area

Each subsection separates the entries that need a hardware measurement (with the
measurement suggested) from the documentation gaps; `open` and
`blocking-emulation` entries are named where they are not in either group.

### System / platform (OQ-SYS, 9 entries)

- **Needs hardware measurement (3).** `OQ-SYS-01` — scope `M2` on a letterless
  RP2A03 and on a 2A03E/G/H and count the duty in 24ths. `OQ-SYS-02` — measure
  the 6527P CPU clock and its divider on a Dendy board. `OQ-SYS-08` — run an
  X = 255 sprite test on 2C02A/B/C parts.
- **Documentation gaps only (4).** `OQ-SYS-04` (`$3000–$3EFF` is NESdev rank 4
  only, unlike the structurally guaranteed `$3F20–$3FFF` palette mirror),
  `OQ-SYS-06` (interrupt vector addresses), `OQ-SYS-07` (`$4018–$401A` on
  revision E), `OQ-SYS-09` (PAL CPU/PPU board pairings).
- **Open (1).** `OQ-SYS-03`, kept as the record of the cross-file dot-numbering
  and Dendy-clock drift that the final consistency pass resolved.
- **Blocking-emulation (1).** `OQ-SYS-05`, the CPU:PPU phase after reset.

### CPU (OQ-CPU, 24 entries)

- **Needs hardware measurement (8).** `OQ-CPU-01` power-on A/X/Y/S/P (meter or
  test ROM after a cold power cycle), `OQ-CPU-11` the 2A07 extra-read
  difference, `OQ-CPU-12` the electrical outcome of a DMC/OAM bus conflict
  (die-level analysis), `OQ-CPU-13` the DMC load-vs-reload halt schedule
  (committed Icarus testbench), `OQ-CPU-14` joypad `/OE` across adjacent reads
  (test ROM), `OQ-CPU-17` `JMP ($xxFF)` (test ROM), `OQ-CPU-19` the dummy-read
  address on a page cross (HDL probe), `OQ-CPU-23` interrupt hijacking and the
  delayed `/IRQ` response (schematic check or test ROM).
- **Documentation gaps only (8).** `OQ-CPU-05` (the `Visual_2A03` page name),
  `OQ-CPU-06`, `OQ-CPU-18` (undocumented opcodes), `OQ-CPU-15` (DMC/OAM
  arbitration window), `OQ-CPU-16` (Dendy DMA timing), `OQ-CPU-20` (errors in
  `Docs/6502/6502 Instruction Set.htm`), `OQ-CPU-21` (`/PRDY` Branch-T0 skip),
  `OQ-CPU-22` (empty `CPU_instructions` capture).
- **Open (3).** `OQ-CPU-02` (HDL defect in the PC load path), `OQ-CPU-08`
  (resolved-record page-name note), `OQ-CPU-24` (RMW scope note).
- **Blocking-emulation (5).** `OQ-CPU-03` (`RDY` authority), `OQ-CPU-04`
  (open-bus `$FF` mechanism), `OQ-CPU-07` (get/put parity), `OQ-CPU-09`
  (NESdev's fixed-4-cycle DMC wording), `OQ-CPU-10` (OAM extra-cycle wording).

### APU (OQ-APU, 40 entries)

- **Needs hardware measurement (11).** `OQ-APU-01` (a test-ROM run pinning the
  derived DMC rate/length tables), `OQ-APU-10` (Dendy frame-counter PLA),
  `OQ-APU-16` (`ExtRes` value, via the DAC transfer curve), `OQ-APU-18` (2A07
  DAC transistor sizes), `OQ-APU-20` (letterless noise period table),
  `OQ-APU-27` (`$4016`/`$4017` in test mode), `OQ-APU-28` (`M2` duty
  fractions), `OQ-APU-35` (first sweep update), `OQ-APU-36`/`OQ-APU-40` (Dendy
  CPU clock) and `OQ-APU-37` (`$400B` same-cycle boundary).
- **Documentation gaps only (15).** `OQ-APU-04`, `OQ-APU-05`, `OQ-APU-06`
  (DMC oddities), `OQ-APU-11`, `OQ-APU-30` (`cycles.md` is images only),
  `OQ-APU-13` (`/AUX` pin), `OQ-APU-15` (board mixing network), `OQ-APU-17`
  (39 kΩ linearity error), `OQ-APU-23`
  (2A07 mode 1), `OQ-APU-24` (page-name corrections), `OQ-APU-26`
  (`$4016`/`$4017` read value), `OQ-APU-29` (derived PAL LFO figures),
  `OQ-APU-31` (`RDY2`), `OQ-APU-32` (`$401B–$401F`), `OQ-APU-38` (`$401A`
  interaction).
- **Open (6).** `OQ-APU-07` (DMC/controller scope note), `OQ-APU-12` (power-on
  mode/mask), `OQ-APU-19` (CI re-run), `OQ-APU-22` (93- vs 31-step mode 1),
  `OQ-APU-34` (sweep enable polarity), `OQ-APU-39` (analog `t = 0`/`t = 1`).
- **Blocking-emulation (8).** `OQ-APU-02`, `OQ-APU-03`, `OQ-APU-08`,
  `OQ-APU-09`, `OQ-APU-14`, `OQ-APU-21`, `OQ-APU-25`, `OQ-APU-33`.

### PPU (OQ-PPU, 69 entries)

- **Needs hardware measurement (24).**
  Revision-specific behaviour — `OQ-PPU-11`, `OQ-PPU-12`, `OQ-PPU-13`,
  `OQ-PPU-17`, `OQ-PPU-29`, `OQ-PPU-30`, `OQ-PPU-32`, `OQ-PPU-34`: a test ROM
  (or die photograph) per revision for OAMADDR corruption, `$2004` readability,
  the 2C05 identifier/register swap, 2C07 emphasis order and palette-read
  reliability.
  Timing and edge windows — `OQ-PPU-05`, `OQ-PPU-07`, `OQ-PPU-41`,
  `OQ-PPU-42`, `OQ-PPU-48`, `OQ-PPU-50`, `OQ-PPU-53`, `OQ-PPU-55`,
  `OQ-PPU-56`, `OQ-PPU-59`, `OQ-PPU-61`, `OQ-PPU-65`, `OQ-PPU-67`: a test ROM
  or an H/V-decoder edge measurement for the attribute right edge, OAM read-back
  bits, the latch shift of the dot numbering, the 9th-sprite window, the `EXT`
  erasure trick, pre-render sprite 0 hit, dot-258 skew, high-Y wrap, the PAL
  picture window and `VPLA_2` (`VC`).
  Colour and DAC — `OQ-PPU-20`, `OQ-PPU-21`: a hardware phase/level
  measurement per revision.
  Direct colour during rendering — `OQ-PPU-25`: a CPU/PPU alignment test ROM.
- **Documentation gaps only (17).** `OQ-PPU-03` (mirroring/CIRAM A10),
  `OQ-PPU-04` (mirror evidence ranks), `OQ-PPU-08` (CHR write decoding),
  `OQ-PPU-14` (2C07 die page), `OQ-PPU-15` (pins 20/40), `OQ-PPU-18` (RGB/clone
  HDL coverage), `OQ-PPU-22` (DAC level labels), `OQ-PPU-24` (RGB PPU masks),
  `OQ-PPU-31` (`$2005`/`$2006` dot windows), `OQ-PPU-35` and `OQ-PPU-46`
  (`$2007` during rendering), `OQ-PPU-37`, `OQ-PPU-43` and `OQ-PPU-51`
  (`dataread.v`, `/FO` semantics), `OQ-PPU-49` (sibling-file wording),
  `OQ-PPU-58` (sprite X past the right edge), `OQ-PPU-66` (2C07 reference
  timing).
- **Open (17).** Section/notation and "Length" notes plus the partially settled
  items: `OQ-PPU-01`, `OQ-PPU-02`, `OQ-PPU-09`, `OQ-PPU-16`, `OQ-PPU-23`,
  `OQ-PPU-26`, `OQ-PPU-36`, `OQ-PPU-38`, `OQ-PPU-39`, `OQ-PPU-44`,
  `OQ-PPU-45`, `OQ-PPU-47`, `OQ-PPU-57`, `OQ-PPU-60`,
  `OQ-PPU-62`, `OQ-PPU-68`, `OQ-PPU-69`. These are recorded but do not block
  an emulator; several are cross-file wording conflicts that a single
  convention would close. Three of the 17 are editorial records, not live
  questions: `OQ-PPU-68` (dot numbering, now a resolved record) plus the
  `OQ-PPU-60`/`OQ-PPU-69` file-length notes.
- **Blocking-emulation (11).** `OQ-PPU-06`, `OQ-PPU-10`, `OQ-PPU-19`,
  `OQ-PPU-27`, `OQ-PPU-28`, `OQ-PPU-33`, `OQ-PPU-40`, `OQ-PPU-52`,
  `OQ-PPU-54`, `OQ-PPU-63`, `OQ-PPU-64`.

### Boards & I/O (OQ-BRD, 14 entries)

- **Needs hardware measurement (5).** `OQ-BRD-01` (controller protocol die
  re-check), `OQ-BRD-03` (Dendy clock), `OQ-BRD-05` (Select-then-Start shift
  order), `OQ-BRD-09` (NES-101 CIC tie-off), `OQ-BRD-12` (expansion-audio
  levels).
- **Documentation gaps only (8).** `OQ-BRD-04`, `OQ-BRD-06`, `OQ-BRD-07`,
  `OQ-BRD-08`, `OQ-BRD-10`, `OQ-BRD-11` (mappers), `OQ-BRD-13`, `OQ-BRD-14`.
- **Open (1).** `OQ-BRD-02` — the `CPU:PPU` column convention, re-statused when
  the final pass aligned `01-platform-overview.md` §2, `04-ppu-timing.md` §10 and
  `05-boards-and-io.md` §1; the row now records the agreement.
- No blocking-emulation entries; the board spec is a rank-3/rank-4 summary and
  states its assumptions.

## How to close these

The concrete evidence that would resolve each `blocking-emulation` entry. Almost
all of them need the same two instruments: a committed Icarus testbench over the
named HDL module, or a test ROM on a real console (ideally a scope capture for
the analog cases).

| Entry | Evidence that closes it |
|---|---|
| `OQ-SYS-05` | A logic-analyzer capture of `M2` vs `PCLK` from `/RES` release on NTSC and PAL, or a statement in `PPU/pclk.md`/`APU/clkgen.md` that fixes the reset phase. |
| `OQ-CPU-03` | Reconcile `dispatch.v` `ReadyRW` with `pads.v`; a committed testbench that holds `RDY` low during a write cycle and shows whether the write completes. |
| `OQ-CPU-04` | A die/HDL trace of the external data-bus driver (`data_bus.v:38`) plus a scope measurement of D0–D7 while reading an undriven address. |
| `OQ-CPU-07` | A committed Icarus test that resets the APU and records `ACLK1`/`/ACLK2` phase, plus a cold-boot parity measurement. |
| `OQ-CPU-09` | A committed DMC-fetch testbench measuring `RDY` low duration at each of the four APU-cycle alignments; a test ROM counting the stall cycles. |
| `OQ-CPU-10` | The same testbench started on odd and even CPU cycles; a scope capture of OAM DMA length. |
| `OQ-APU-02` | An HDL testbench that asserts a `$4011` write in the same ACLK phase as `DSTEP`; a test ROM that writes `$4011` on a timer edge and records AUX B. |
| `OQ-APU-03` | Measure AUX B immediately after power-on, before any `$4011`/`$4010` write. |
| `OQ-APU-08` | A committed `clkgen.v` testbench measuring the full 4-step and 5-step periods (NTSC and PAL), plus a test ROM timing the frame IRQ; this decides 29828/37284 vs 29830/37281. |
| `OQ-APU-09` | A committed `softclk.v` test that asserts `$4017` at each APU-cycle alignment and counts CPU cycles to the first step. |
| `OQ-APU-14` | A hardware AUX B transfer-curve measurement, or a transistor-level model that reproduces it, to choose 4:2:1 vs 3:2:1. |
| `OQ-APU-21` | Capture the first noise samples after power-on with a short period; or die analysis of `NOISE_RandomLFSRBit`. |
| `OQ-APU-25` | A test ROM reading `$4015` with a known byte left on the open bus. |
| `OQ-APU-33` | A test ROM sweeping the pulse timer across 4/7/8 and recording muting; a die check of the `SW_UVF` bits in `square.v`. |
| `OQ-PPU-06`, `OQ-PPU-54` | A hardware OAM cell-degradation measurement (decay vs refresh interval), or a documented refresh-rate rule usable by an emulator. |
| `OQ-PPU-10`, `OQ-PPU-28`, `OQ-PPU-63` | A die trace of the `RC`/`RESCL` net, plus a test ROM writing PPU registers during the reset window on a 2C02G. |
| `OQ-PPU-19` | A scope capture of composite video with greyscale set, checking whether the colour burst survives. |
| `OQ-PPU-27`, `OQ-PPU-64` | A test ROM writing `$2000`/`$2001`/`$2005`/`$2006` inside the ~29658 (NTSC) / ≈33132 (PAL) CPU-cycle window and reading back the effect. |
| `OQ-PPU-33` | A test ROM reading `$3F00` after a known open-bus byte has been placed on the PPU bus. |
| `OQ-PPU-40` | A hardware measurement of pattern/attribute shift-register decay over frames, or a test ROM showing the mid-frame artefact. |
| `OQ-PPU-52` | Implement the NESdev `n`/`m` overflow walk and validate it against a test ROM that produces documented false positives and negatives. |
