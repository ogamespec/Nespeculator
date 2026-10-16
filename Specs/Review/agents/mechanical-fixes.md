# Mechanical cross-file fixes (orchestrator)

Naming/consistency edits that span files owned by different agents. Applied by
the orchestrator after verification, so the per-section agents never race on the
same file. Each entry: what, where, evidence.

| # | Fix | Where | Evidence | Status |
|---|---|---|---|---|
| 1 | `APU_Mixers` → `APU_Mixer` (singular) | any `Specs/0*.md` | `/tmp/nesdev-cache/APU_Mixer.html` = "APU Mixer" (22 KB); `APU_Mixers` is a Wayback soft-404 (4.6 KB). Fixed by the apu-pulse/apu-noise/apu-overview/boards-io Adapters in their own files; re-grep at the end | mostly done |
| 2 | `Visual_2A03` → `Visual_circuit_tutorial` | `Specs/02-cpu-instructions.md`, others | CPU/APU/PPU Adapters independently report no archived `Visual_2A03` article; CDX lists `Visual_2C02` and `Visual_circuit_tutorial` | pending |
| 3 | `Status_register` → `Status_flags` | any `Specs/0*.md` | `cpu-core` Compiler/Adapter: `Status_register` 404s | pending grep |
| 4 | `Clock_rate` vs `Cycle_reference_chart` | all sections | `Clock_rate` redirects to `Cycle reference chart`; both names appear. Decision: cite `Clock_rate` (readers type it; the wiki redirects) and add the redirect note once per table | decided |
| 5 | `PPU_variants` existence | `01-platform-overview.md`, `04-ppu-*` | **Resolved: `PPU_variants` EXISTS** — fetched `https://web.archive.org/web/2024/https://www.nesdev.org/wiki/PPU_variants` = "PPU variants - NESdev Wiki", 68 022 bytes. The `ppu-overview-regs` Adapter wrongly replaced it with `PPU`; restore `PPU_variants` there and keep the `platform` Adapter's usage. `CPU_variants` (56 KB), `Cycle_reference_chart` (31 KB), `PPU_frame_timing` (22 KB), `CPU_Test_Mode` (20 KB), `Famicom` (22 KB) all verified real and cached | pending |
| 6 | `Controller_ports` → `Standard_controller` + `Expansion_port` | any `Specs/0*.md` | apu-overview Adapter: `/wiki/Controller_ports` = HTTP 404 | mostly done |
| 7 | Stack pointer naming `D` → `S` | `Specs/02-cpu-instructions.md` §4 | `BreakingNESWiki/6502/regs.md`/`Readme.md` write `S`; flagged by cpu-core Adapter | pending |
| 8 | "Implied" vs "Implicit" addressing | `Specs/02-cpu-instructions.md` §1 | cached `CPU_addressing_modes` says "Implicit" | pending |

Rule: apply 1–3 and 6–8 mechanically with evidence; for 4 and 5 decide once and
make every file agree. Nothing in this table may be applied silently — each change
goes into the commit message or `Specs/Review/CONSENSUS.md`.

## Adjudications (orchestrator, evidence-based)

Contradictory agent claims resolved here. The Final Fixer must follow these and
must not "re-fix" a file back to the wrong value.

| # | Dispute | Ruling | Evidence |
|---|---|---|---|
| A1 | `apu-pulse` Verifier minor M5: "correspondence cites `APU_Mixer`; the wiki title is `APU_Mixers`" | **M5 is wrong.** `APU_Mixer` (singular) is the real article | fetched `.../wiki/APU_Mixer` → `<title>APU Mixer - NESdev Wiki</title>`, `wgPageName = "APU_Mixer"`, 22 681 bytes; `APU_Mixers` → 4 609-byte Wayback soft-404 |
| A2 | `apu-pulse` Verifier minor M4: "PAL 5-step quarter interval is 14913, not 16625" | **M4 is wrong.** PAL 5-step = 8313, 8313, 8313, 16625 (total 41 564); 14913 is the NTSC value | `BreakingNESWiki/APU/clkgen.md` PLA tables; cached `APU_Frame_Counter.html` gives the PAL half-frame as 16 626/16 626.5 APU cycles |
| A3 | `cpu-instructions` Revision refuted its own "`DEC EA`" instruction | **Refutation accepted** — the only `EA` row is the correct `implied NOP EA 1 2` | being re-checked in the round-2 verification |
| A4 | CPU:PPU ratio column | Cells mean **PPU dots per CPU cycle**: NTSC 3, PAL 3.2 = `16:5`, PAL = 341 × 5÷16 = 106 9/16 CPU cycles/scanline | cached `Cycle_reference_chart.html` ("PPU dots per CPU cycle 3.2") |
| A5 | Dendy clock | **26.601712 MHz ÷ 15 = 1.773448 MHz**, PPU ÷5, **exactly 3 dots per CPU cycle**, 113⅔ CPU cycles/scanline | cached `Cycle_reference_chart.html` (UMC UA6527P) and `CPU_variants.html` |
| A6 | `cpu-dma` §5 collision byte indices 21/22/23 (mod 32) | **Keep them.** The Revision's counter-example assumed `$4012[1:0]` lands in `A[4:0]`; the HDL puts it in A7–A6, so `A[4:0]` is 0 for every `$4012` | Revision ran `DPCM_AddressCounter` under Icarus for `$4012` = $00/$01/$02/$03/$FD → `A[4:0]` = 0 in all cases |
| A7 | NESdev 5-step totals (18 640.5 APU = 37 281 NTSC / 20 782.5 APU = 41 565 PAL) vs repo 37 284/41 564 | Repo figures stay normative; record the difference in `03-apu-frame-counter.md` Open questions (as the 4-step one already is) | `apu-fc-mixer` Verifier major 4 |

## Remaining cross-file items (after the revision rounds)

| # | Item | Where | Evidence |
|---|---|---|---|
| C1 | `02-cpu-core.md §12` cross-reference does not exist (content sections are §1–§10; the B/bit-5 rule is §4) | `02-cpu-instructions.md` (~line 282) | `cpu-core` round-2 Verifier |
| C2 | Stale "instructions still lists `Visual_2A03`" clause | `02-cpu-dma.md` Open questions | `cpu-core` round-2 Verifier; `02-cpu-instructions.md` now uses `Visual_circuit_tutorial` |
| C3 | Stale Dendy clock (`≈1.7897727 MHz`, `1:≈2.973`) and the opposite `CPU:PPU` convention | `04-ppu-timing.md` §10 | `platform` round-2/3 Verifier |
| C4 | Stale clock table (`1:3`, `5:16`, `186.2 ns`, Dendy "not documented here") | `05-boards-and-io.md` §1 | `platform` round-2/3 Verifier |
| C5 | `$3000–$3EFF` / A12 mirror presented as fact rather than rank 4 | `04-ppu-memory-map.md` §1/§3/§14 | `platform` round-1/2 Verifier |
| C6 | Letterless `M2` 15/24 without the 17/24 NESdev tie-breaker | `03-apu-overview.md` (~line 207) | `platform` round-2 Verifier; `apu-overview` states both in §3 |
| C7 | Correspondence still uses the redirect `Clock_rate` | any remaining file | three Adapters switched to `Cycle_reference_chart` |
