# Review: apu-overview — round 2

## Verdict
consensus

## Blocking
None. The round-1 blocking finding (`RDY` polarity and model inverted) is
**resolved**:
- `03-apu-overview.md:178` now reads "`RDY` is active-high ready: `0` = the core
  is halted and repeats its last read cycle. It is pulled low for the whole OAM
  DMA and whenever the DMC is not ready, and released only when the sprite DMA
  is idle **and** `DMCRDY` = 1: `sprdma_rdy = NOR(StopDma, StartDma)` … and
  `RDY = sprdma_rdy AND DMCRDY`".
- `03-apu-overview.md:243` states the same as a MUST ("active-high ready with
  `0` = core halted … `sprdma_rdy` … is `0` for the whole OAM DMA and
  `DMCRDY` = 0 … MUST force `RDY` low"). The two halves no longer contradict.
- Verified against the silicon: `HDL/APU/dma.v:148` `dlatch nospr_latch
  (.d(StopDma), .en(ACLK1), .nq(NOSPR))` makes `NOSPR = ~StopDma`, so
  `nor (sprdma_rdy, ~NOSPR, StartDma)` (`dma.v:161`) is exactly
  `NOR(StopDma, StartDma)`, and `assign RDY = sprdma_rdy & DMCReady`
  (`dma.v:162`). `StopDma` is set for the transfer (`dma.v:151`) and `StartDma`
  by `$4014` (`dma.v:152`), so `RDY` = 0 throughout; it is released when both are
  clear and `DMCReady` = 1.
- Agrees with `Specs/02-cpu-dma.md:28` (`RDY`; `0` = core halted), `:66-67`
  (identical formulas) and `:72` ("`RDY` therefore stays low for the whole OAM
  DMA"). `BreakingNESWiki/APU/dma.md:42` ("Если DMC не готов - … RDY … в 0")
  and `BreakingNESWiki/APU/dpcm.md:11` ("RDY = 0") match.

## Major
None. The round-1 major finding (`$401A` write bits 4–0 missing) is
**resolved**:
- `03-apu-overview.md:200` (§9) now lists "bits 4–0 = triangle output counter
  parallel load (`TRIANGLE_Output`, `triangle.v`, `apu_top.v`)".
- `03-apu-overview.md:124` (§4) now routes `$4018–$401A` to "Test logic,
  Triangle".
- Verified: `HDL/APU/triangle.v:146` `CounterBit out_cnt [4:0] (.ACLK1(PHI1),
  .d(DB[4:0]), .load(W401A), …)` inside `TRIANGLE_Output`, and
  `HDL/APU/apu_top.v:323` `.W401A(W401A)` on `TriangleChan`;
  `BreakingNESWiki/APU/regs.md:41` "`W401A`|Reg Select|Test, Triangle".
- §6 was updated consistently (`03-apu-overview.md:166`): "`$4018–$401A` are
  read-only except for `$401A` (write: `LOCK` bit 7 and the triangle output
  counter bits 4–0, test mode only)".

## Minor
1. **Residual inverse-polarity wording for `RDY`** —
   `03-apu-overview.md:231` (Observable phenomena #8) says "DMC DMA asserts
   `RDY` only for the byte fetch (`dpcm.md`)". `RDY` is active-high *ready*
   (`03-apu-overview.md:178`, `:243`), and the cited source says the opposite
   level: `BreakingNESWiki/APU/dpcm.md:11` — "DMC DMA прерывает процессор
   (RDY = 0) только на время выбора очередного сэмпла". The other post-fix
   bullets say "pulled low"; align this row to "pulls `RDY` low only for the
   byte fetch".
2. **New `triangle.v` citation is not in the metadata / Provenance lists** —
   `03-apu-overview.md:200` attributes the `$401A` bits 4–0 counter load to
   `triangle.v` (and `apu_top.v`), but the `**Primary sources:**` HDL list
   (`03-apu-overview.md:5`) omits `triangle.v`, and the Provenance row
   "Test registers `$4018–$401A` and `LOCK`" (`03-apu-overview.md:288`) lists
   only `HDL/APU/test.v`, `HDL/APU/regs.v`. Add `triangle.v` to both.
   (`square.v` and `noise.v` are cited in the §1 block table with the same
   omission, so this is pre-existing apart from the new §9 line.) The inline
   citation means traceability is not broken — completeness only.

## Checked against
- `HDL/APU/regs.v` — recomputed all 29 PLA rows against `$4000–$401F`. The PLA
  input is the *complemented* low address (`ApuRegs_PLA`: `.A(Addr_frommux[4:0]),
  .nA(~Addr_frommux[4:0])`), so the raw NOR minterms look reversed; after
  inversion they decode exactly as §4: writes `$4000–$4014` (nothing at `$4009`
  or `$400D`), read+write `$4015`/`$4016`/`$4017`, read `$4018`/`$4019`/`$401A`
  and write `$401A` all `DBG`-gated (`regs.v:82-83,98-101`), nothing at
  `$401B–$401F`. The register map and directions are unchanged from round 1 and
  match the HDL.
- `HDL/APU/{length,dpcm,clkgen,test,noise,square,triangle,io}.v` — bit fields
  re-spot-checked and unchanged: `$4015` write `---D NT21` (`dpcm.v:222`
  `ena_ff .d(DB[4])`; `length.v:47` `dbit_ena {DB[3],DB[2],DB[1],DB[0]}`),
  `$4015` read `IF-D NT21` (`dpcm.v:199` DB[7], `clkgen.v:153` DB[6],
  `dpcm.v:224` DB[4], `length.v:141` bits 0–3), `$4010 IL-- RRRR`
  (`dpcm.v:127-129`), `$4011` 7-bit (`dpcm.v:573-574`, DB[6:1]+DB[0], bit 7
  unused), `$400E M--- PPPP` (`noise.v:54,189`), `$4001 EPPP NSSS`
  (`square.v:126,275-277`), `$4000 DDLC VVVV` (`square.v:306`), `$4008
  CRRR RRRR` (`triangle.v:79,106`), `$4018`/`$4019`/`$401A` read drivers
  (`test.v:36-57`), `$401A` bit 7 `LOCK` FF clocked by `ACLK1` and reset by
  `RES` (`test.v:27-31`), `$4016` write bits 0–2 (`io.v:32`).
- `HDL/APU/dma.v` — `RDY` model (above); 8-bit low counter `EndCount = cc[7]`
  (`dma.v:76-97`) = 256 pairs; read-cycle-only start
  `nand (read_cyc, ~PHI1, RnW)` (`dma.v:167`); `$2004` hard-wired destination;
  mux priority `SPR/PPU > SPR/CPU > /DMCAB > CPU`.
- `Specs/02-cpu-dma.md` — `RDY` active level and formulas agree (`:28`, `:66-67`,
  `:72`); its OAM DMA total of 513 or 514 CPU cycles (`:108-109`) matches
  `03-apu-overview.md:176`.
- `BreakingNESWiki/APU/{dma,dpcm,regs}.md` — `RDY` = 0 while the DMC is unready
  and during the fetch (`dma.md:42`, `dpcm.md:11`); `W401A` → "Test, Triangle"
  (`regs.md:41`); `/R4015` → `SoftCLK, DMABuffer, Length, DPCM`.
- `HDL/APU/apu_top.v:167-173` — `n_R4015` wired into `DMABuffer`, confirming the
  `$4015` Reaches row fix (`03-apu-overview.md:120`).
- `HDL/APU/clkgen.v` — `R4015_clear = nor (n_R4015, PHI1)` (`:146`) is active
  when `PHI1` = 0, i.e. during `PHI2`, matching Observable phenomena #3
  (`03-apu-overview.md:226`) and §3's write-timing rule; `Z2` ORed into both
  `/LFO1` and `/LFO2` (`:107`, `:113`) and latched from `n_mode` (`:198-200`),
  matching §3 (`03-apu-overview.md:79`).
- NESdev cache `/tmp/nesdev-cache/` — `Cycle_reference_chart.html` title is
  "Cycle reference chart" and `Clock_rate.html` redirects there;
  `APU_Mixer.html` title is "APU Mixer"; `APU_Noise.html` gives `M---.PPPP`;
  `APU_registers.html` gives `---D NT21` and `IF-D NT21`; `PPU_OAM.html`,
  `Standard_controller.html`, `Expansion_port.html` are present. `APU_Mixers` is
  not cited as a page anywhere in the spec.
- Round-1 minors 1–11 — all confirmed applied as described in the
  `## Revision round 2` note: PHI2 clear wording (`:226`), `dma.v`-sourced 256
  pairs (`:176`), DMA buffer in the `$4015` Reaches row (`:120`), `Z2` in both
  LFO terms (`:79`), `$400E` bit 7 = `M` (`:139`), `$4015` read bit 4 = "DMC
  active" (`:147`, `:156`), stale `APU_Mixers` bullet removed (only the
  legitimate page-name note at `:299` remains), `Cycle_reference_chart` (`:268`),
  `CLK ÷ 12` (÷ 16 on the 2A07) (`:14`, `:55`), H1
  `# 03-apu-overview — Overview and register interface` (`:1`), §4 heading
  `$4000–$401F` (`:95`).
- Template / `Specs/Conventions.md` §6 — `Status: revised (round 2)` present and
  accurate; the five fixed headings spelled exactly and in order at `:220`
  (`## Observable phenomena`), `:234` (`## Programming / implementation notes`),
  `:250` (`## NESdev Wiki correspondence`), `:273` (`## Provenance`), `:297`
  (`## Open questions`), no number prefix; file is 307 lines, inside the binding
  120–400 range (`Specs/Conventions.md:137`). `Specs/Review/agents/REVISION.md:33`
  still says 120–300; Conventions §6 is binding, so 307 is compliant and this is
  not a finding.
- Adjudication A1 — `APU_Mixer` (singular) is used in the correspondence table
  (`03-apu-overview.md:264`); not flagged, per
  `Specs/Review/agents/mechanical-fixes.md:29`.

## Notes for the next round
- The `$4015` clear half-cycle is stated only here; `03-apu-frame-counter.md`
  describes the clear as "a read of `$4015`" without naming `PHI2` (`:67`,
  `:149`). They do not contradict, but if that file is ever revised, state the
  `PHI2` half-cycle there too.
- `$4015` read bit 4 is the DMC enable FF (`dpcm.v:222-224`); NESdev defines `D`
  as "DMC active". "DMC active" is a fair compromise, but a one-word
  parenthetical ("enable FF, cleared when the sample finishes") in §5 would
  remove any doubt.
- Minor 2 (add `triangle.v` to Primary sources / Provenance) is the only
  attribution bookkeeping left; it does not block consensus.

---

# Review: apu-overview — round 1

## Verdict
revise

## Blocking
1. **`RDY` polarity and model are inverted** — `03-apu-overview.md:178` (§7
   bullet) and `03-apu-overview.md:243` (normative bullet) say:

   > "`RDY` to the core is asserted only while the DMA is active **and** the DMC
   > is ready (`RDY = sprdma_rdy AND DMCRDY`)" …
   > "`RDY` MUST be modelled as `DMA active AND DMC ready`".

   The two halves of the first sentence cannot both hold, and read at signal
   level both are wrong. `RDY` is active-high "ready" (`0` = core halted, the
   convention the sibling `Specs/02-cpu-dma.md:27` uses), and the hardware pulls
   it **low** for the whole OAM DMA:
   - `HDL/APU/dma.v:160-162`: `nor (sprdma_rdy, ~NOSPR, StartDma); assign RDY =
     sprdma_rdy & DMCReady;` with `StopDma`/`StartDma` (`dma.v:145-156`) set
     while the transfer runs — so `sprdma_rdy = 0` and `RDY = 0` throughout the
     DMA, not "asserted while the DMA is active".
   - `BreakingNESWiki/APU/dma.md`: "Если DMC не готов - то сигнал RDY также
     принудительно устанавливается в 0"; `BreakingNESWiki/APU/dpcm.md:11`:
     "DMC DMA прерывает процессор (RDY = 0) только на время выбора очередного
     сэмпла".
   - `Specs/02-cpu-dma.md:55-71` states it correctly: "`RDY` … `0` = core
     halted"; "`RDY` therefore stays low for the whole OAM DMA, and a DMC fetch
     forces it low for its own duration".

   The file never states `RDY`'s active level, and the MUST would make an
   implementer run the core exactly while the OAM DMA is transferring. Correct
   rule: `RDY` is released (high) only when the sprite DMA is idle **and**
   `DMCRDY` = 1; it is pulled low (core halted / repeats its last read cycle)
   while the sprite DMA runs and whenever `DMCRDY` = 0. Match the wording to
   `02-cpu-dma.md`.

## Major
1. **`$401A` write bits 4–0 are missing** — the register map row
   (`03-apu-overview.md:124`, "`$4018–$401A` … Reaches: Test logic
   (`/R4018`, `/R4019`, `/R401A`, `W401A`)") and the §9 table
   (`03-apu-overview.md:200`: "`$401A` write | bit 7 = `LOCK` flip-flop") list
   only `LOCK`. `W401A` also parallel-loads the triangle output counter:
   - `BreakingNESWiki/APU/regs.md`: "`W401A`|Reg Select|Test, Triangle|1: Запись
     отладочного регистра $401A".
   - `HDL/APU/triangle.v:132-146`: `TRIANGLE_Output … .W401A(W401A)`,
     `CounterBit out_cnt [4:0] (… .d(DB[4:0]), .load(W401A) …)`.
   - `HDL/APU/apu_top.v:323`: `.W401A(W401A)` on `TriangleChan`.
   Add "bits 4–0 = triangle output counter parallel load (test mode)" to §9 and
   `Triangle` to the "Reaches" column of §4.

## Minor
1. **Wrong half-cycle for the `$4015` read side effect** —
   `03-apu-overview.md:226` (Observable phenomena #3) says "the read (during
   `PHI1`) resets the frame-IRQ flip-flop (`clkgen.v`)". `HDL/APU/clkgen.v:146`
   is `nor (R4015_clear, n_R4015, PHI1);` — the clear is active only when
   `PHI1` = 0, i.e. during `PHI2` (the second half). This contradicts the
   file's own §3 rule ("writes … active only while `PHI1` = 0, i.e. during
   `PHI2`", `03-apu-overview.md:67`) and `square.md:87`. Drop the parenthetical
   or write "during `PHI2` (`PHI1` = 0)".
2. **`256 times` is attributed to `dma.md`, which does not state it** —
   `03-apu-overview.md:176` ends "… 256 times (`dma.md`)". `dma.md` gives the
   alternation but no count; the number comes from the 8-bit low address
   counter (`HDL/APU/dma.v:76-99`, `EndCount = cc[7]`; `APU/common.md` counter
   table lists the OAM DMA low counter as 8-bit). Cite `dma.v` instead of
   `dma.md`.
3. **`$4015` "Reaches" row omits the DMA buffer** — `03-apu-overview.md:120`
   lists "Length counters, DMC, frame counter (`W4015`, `/R4015`)". `regs.md`
   routes `/R4015` to SoftCLK, **DMABuffer**, Length and DPCM, and
   `HDL/APU/apu_top.v:173` wires `n_R4015` into `DMABuffer` (it suppresses
   `WR` on an APU read: `HDL/APU/dmabuf.v:19`). §6 already records this
   (`03-apu-overview.md:166`), but the §4 row is the table a reader scans;
   add "DMA buffer".
4. **LFO terms omit `Z2`** — `03-apu-overview.md:79` enumerates "`/LFO1` is
   asserted while any of PLA[0..4] is active, `/LFO2` while PLA[1], PLA[3] or
   PLA[4] is active". The HDL also ORs `Z2` into both terms
   (`HDL/APU/clkgen.v:107`: `nor (tmp1, PLA_out[0..4], Z2)`;
   `clkgen.v:113`: `nor (tmp3, PLA_out[1], PLA_out[3], PLA_out[4], Z2)`), and
   `Z2` is live in 5-step mode (`clkgen.v:192-200`, `z2_out` latches `n_mode`).
   `clkgen.md` does not describe `Z2`, so the sentence is faithful to rank 1 but
   not to the module it also cites; either add "or the `Z2` phase" or mark it a
   rank-1 simplification (the derived 5-step 192/96 Hz rates still agree with
   NESdev `APU_Frame_Counter`).
5. **`$400E` bit 7 is labelled `L`** (`03-apu-overview.md:139`), but `L`
   everywhere else in the same table means "length-counter halt / loop"
   (`$4000`, `$4003`, `$400C`, `$400F`). NESdev `APU_Noise` and
   `APU_registers` name it `M` (Mode flag); `waves.md:158` calls it "Loop".
   Use `M` to avoid the collision inside one table.
6. **`$4015` read bit 4 described two ways** — the diagram row says
   "D DMC active" (`03-apu-overview.md:147`) while the read-bits table says
   "DMC enable / sample-active state" (`03-apu-overview.md:156`). NESdev
   `APU`/`APU_registers` define `D` as "DMC active"; the HDL driver is the
   enable FF (`HDL/APU/dpcm.v:222-224`), reset when the sample ends. Harmonise
   on "DMC active (enable FF, cleared when the sample finishes)".
7. **Stale `APU_Mixers` bullet** — `03-apu-overview.md:300` claims
   "`03-apu-dmc.md`, `03-apu-noise.md` and `05-boards-and-io.md` also cite
   `APU_Mixers` … those files need the same `APU_Mixer` correction". They no
   longer do: they already cite `APU_Mixer` (`03-apu-dmc.md:269`,
   `03-apu-noise.md:269`, `05-boards-and-io.md:326`). Reword or delete.
8. **`Clock_rate` vs the canonical title** — the correspondence row
   (`03-apu-overview.md:268`) uses the redirect `Clock_rate`; the canonical page
   is `Cycle_reference_chart` (`/tmp/nesdev-cache/Clock_rate.html` header
   "(Redirected from Clock rate)"), and `01-platform-overview.md:281` /
   `03-apu-frame-counter.md:224` already use the canonical name. The redirect is
   recorded in Open questions, so this is cosmetic; align if convenient.
9. **`CLK ÷ 12` unqualified in the block table** — `03-apu-overview.md:14`
   says "Clock divider | `CLK ÷ 12` → `PHI0`" without naming the region; the
   2A07 divides by 16 (NESdev `CPU_pinout`; the file's own §3 PAL column gives
   26.601712 MHz ÷ 16). Add "(÷ 16 on the 2A07)" or name the region.
10. **Title wording** — `# APU 03 — Overview and register interface`
    (`03-apu-overview.md:1`) inverts the sibling pattern
    (`# 01 Platform — …`, `# 02-cpu-core — …`, `# 02-cpu-instructions — …`).
    Cosmetic.
11. **Heading vs content width** — §4 is titled "Register map `$4000–$4017`"
    (`03-apu-overview.md:95`) but the table also carries `$4018–$401A` and
    `$401B–$401F`; widen the heading to `$4000–$401F`.

## Checked against
- `BreakingNESWiki/APU/regs.md` — register-operation table and the
  `CPU_Ax` (space) vs mux `A0–A4` (register) split; `W401A → Test, Triangle`;
  `/R4015 → SoftCLK, DMABuffer, Length, DPCM`; `/DBGRD`; `LOCK`; 2A07 note.
- `HDL/APU/regs.v` — recomputed all 29 PLA rows: `$4018/$4019/$401A` read,
  `$401A` write, `$4015` read, `$4003/$4002/$4007/$4001/$4004/$4005/$400C/
  $4006/$4000/$4008/$4015/$400A/$4011/$400B/$400F/$400E/$4017/$4013/$4016/
  $4012/$4010/$4014` writes. Every address in the §4 map matches; `$4009`,
  `$400D`, `$401B–$401F` are undecoded; space select `A15=0, A14=1, A13–A5=0`
  = `$4000–$401F`; all write strobes are `nor(…, PHI1, …)`; reads are not
  `PHI1`-gated.
- `BreakingNESWiki/APU/waves.md` — test vectors confirm the `$4000` (`DDLC
  VVVV`), `$4001` (`EPPP NSSS`), `$4008` (`CRRR RRRR`), `$400C` (`--LC VVVV`),
  `$400E` (`L--- PPPP`), `$400F` (`LLLL L---`), `$4015` enable-bit ordering
  (`NT21`), and the `$4014`/ACLK1 alignment penalty.
- `HDL/APU/{square,triangle,noise,dpcm,length,env}.v` — bit-field bit
  assignments: `$4000` duty `DB[7:6]`, `$4001` `SWDIS = ~DB[7]`, negate
  `DB[3]`, shift `DB[2:0]`, `$4003` `{DB[2:0],DB[7:0]}` = timer, `$400C`
  `DB[5:4]`, `$400E` `DB[7]`/`DB[3:0]`, `$4010` `DB[7]`/`DB[6]`/`DB[3:0]`,
  `$4011` `DB[6:0]` (bit 7 unused), `$4015` read drivers bits 0–3
  (`length.v:141`), 4 (`dpcm.v:224`), 6 (`clkgen.v:153`), 7 (`dpcm.v:199`).
- `HDL/APU/io.v` — `ACLK5 = ~nACLK2`; `OUT0-2` from `DB[2:0]`; `/IN0`, `/IN1`
  are `n_R4016`, `n_R4017` pad outputs, no data-bus driver.
- `HDL/APU/pads.v` + `BreakingNESWiki/APU/pads.md` — 40-pin map, `RES =
  ~/RES`, `A0–A15` and IN/OUT `z` during reset, `R/W` `z` during reset,
  `M2` leads `PHI2` by 1.5 CLK, `M2` driven during reset only with `DBG` = 1.
- `BreakingNESWiki/APU/core.md`, `HDL/APU/core.v`, `apu_top.v` — `/DAA`/`/DSA`
  cut, D flag normal, `SO` = 1, `SYNC` floating, `RDY2 = /DBG` on the 2A07 (no
  `RDY2` net in `apu_top.v`), `/NMI` direct, `/IRQ_INT = /IRQ NOR INT`.
- `BreakingNESWiki/APU/clkgen.md`, `HDL/APU/clkgen.v` — divider chain, PHI/ACLK
  periods (NTSC 21.477272 MHz ÷ 12, 558.73 ns, 1117.46 ns; PAL 26.601712 MHz
  ÷ 16, 601.46 ns, 1202.93 ns), 6-stage Johnson divider, non-overlapping
  `ACLK1`/`nACLK2`, other-ACLK2 copies 2/3A/3B/4/5, 15-bit LFSR + PLA, PLA[3]
  interrupt / skipped in mode 1, PLA[5] anti-lockup, `INT = frame-IRQ FF OR
  DMCINT`, `DMCINT = flag AND $4010 bit 7`, frame IRQ only in 4-step.
- `BreakingNESWiki/APU/clkgen.md` activation tables — 4-step 7457 CPU cycles
  (240 Hz) / 14913 (120 Hz); 5-step 3×7457+14913 (192 Hz) /
  14913+22367 (96 Hz); PAL 8313 / 16625 / 24939 → derived 200/100/160/80 Hz
  (checked arithmetically; the PAL figures are correctly marked as derived).
- `BreakingNESWiki/APU/dma.md`, `HDL/APU/dma.v` — `$4014` low-byte clear /
  high-byte load / start, hard-wired `$2004`, read-cycle-only start, mux
  priority `SPR/PPU > SPR/CPU > /DMCAB > CPU`, `RDY = sprdma_rdy AND DMCRDY`
  (blocking finding), 8-bit low counter.
- `BreakingNESWiki/APU/dpcm.md`, `HDL/APU/dpcm.v`, `HDL/APU/dmabuf.v` — DMC DMA
  priority (`RUNDMC`), `/DMCAB`, halt only for the fetch, `$4015` read does
  **not** clear the DMC IRQ flag (`int_ff .res1(W4015)`, `.res2(n_IRQEN)` only),
  `$4015` write / `$4010` bit 7 = 0 do clear it.
- `BreakingNESWiki/APU/{pal,old_2a03,test_pattern}.md`, `HDL/APU/test.v`,
  `HDL/APU/regs.v` — 2A07 register decoder shortened, `W401A` grounded, no
  debug logic, different SoftCLK/DPCM/noise decoders; letterless 2A03 list
  (no `LOCK`/test regs, no tonal noise, shorter noise LFSR, reset omissions,
  `M2` = 15/24 from SR3, disabled PIT); `$4018` `SQA`/`SQB`, `$4019`
  `TRI`/`RND`, `$401A` `DMC[6:0]`, `LOCK` FF.
- `BreakingNESWiki/APU/dac.md`, `HDL/APU/dac.v` — `AUX_A = {SQB,SQA}` (8),
  `AUX_B = {DMC[6:0],RND,TRI}` (15), ~100 Ω pull-downs, 272 mV at 75 Ω with
  `SQA = SQB = $F`, no separate analogue supplies.
- **Both Adapter-recorded conflicts are honest and the silicon was kept:**
  (a) `M2` — §3 (`03-apu-overview.md:73-75`) states the NESdev assignment
  (15/24 for 2A03E/G/H, 17/24 letterless, 19/32 2A07) and then keeps the
  repository assignment (`old_2a03.md`: 17/24 on the 2A03G from SR4, 15/24 on
  the letterless from SR3), with the PAL figure marked unconfirmed — matches
  `CPU_pinout` and `APU/old_2a03.md` exactly. (b) test-mode `$4016`/`$4017` —
  Open questions (`03-apu-overview.md:303`) records NESdev `CPU_pinout`
  ("$4016 and $4017 become open bus" with pin 30 high on the RP2A03G), states
  that `regs.v` does **not** gate the strobes with `DBG`
  (`not(n_R4016, pla[23])`, `not(n_R4017, pla[21])` — verified), and keeps the
  schematic behaviour (strobes still asserted).
- NESdev cache `/tmp/nesdev-cache/` — `APU_registers` (`---D NT21` write,
  `IF-D NT21` read, `$4000`–`$4013` diagrams), `APU` (`$4015` read clears the
  frame flag but not the DMC flag; bit 5 open bus; `$4017 MI-- ----`),
  `APU_Frame_Counter` (3–4 CPU cycle `$4017` delay, 240/120/192/96 Hz),
  `APU_DMC` (`IL--.RRRR`, `$C000 + A*64`, `L*16 + 1`, IRQ cleared by `$4010`
  bit 7 = 0), `APU_Pulse` (`DDLC VVVV`, `EPPP NSSS`), `APU_Noise`
  (`--lc.vvvv`, `M---.PPPP`), `APU_Triangle` (`CRRR.RRRR`, `llll.lHHH`),
  `CPU_pinout` (÷12/÷16, `M2` duties, `TST`, `/RST` reset state), `Clock_rate`
  (redirect header to `Cycle_reference_chart`), `APU_Mixer` (article 54) vs
  `APU_Mixers` (Wayback soft-404). Every page name in the correspondence table
  exists in the cache; `APU_Mixer` is the correct singular title.
- Template/compliance pass — title, `Status: adapted (round 1)`, `Scope`,
  `Primary sources` present; the five fixed headings are spelled exactly
  `## Observable phenomena` (L220), `## Programming / implementation notes`
  (L234), `## NESdev Wiki correspondence` (L250), `## Provenance` (L273),
  `## Open questions` (L297), in order, no number prefix; **308 lines**
  (within 120–400); every content section has a correspondence row and a
  Provenance row, with inline citations for the non-obvious claims.

## Notes for the next round
- Fix the `RDY` bullets against `Specs/02-cpu-dma.md`; keep the same active-level
  convention in both files and state `0` = halt once.
- Add the `$401A` bits 4–0 triangle-counter load to §4 and §9 (Major 1) and then
  re-read §6's "Direction" bullet, which says `$4018–$401A` are read-only
  "except for `$401A` bit 7" — with bits 4–0 also writable in test mode, say
  "except `$401A` (write: `LOCK` bit 7, triangle counter bits 4–0)".
- The `$4015`/PHI1 clause (Minor 1) should be checked against
  `03-apu-frame-counter.md`, which owns the frame-IRQ timing; keep the two files
  consistent about which half-cycle clears the flag.
- Minor 7 (stale `APU_Mixers` bullet) is a moving target — re-grep
  `Specs/*.md` for `APU_Mixers` immediately before the revision is written.
- Not a defect, but worth a one-line cross-reference: §7 gives the 256-byte
  transfer but never the 513/514-CPU-cycle total; `02-cpu-dma.md` has the
  per-cycle table. An overview reader looking for the DMA cost will not find it
  here.
- DMC `$4015` read bit 4 is driven by the enable FF in the HDL, while NESdev
  defines `D` as "bytes remaining > 0"; the wording in Minor 6 should not
  over-claim either implementation.

## Revision round 2
- Blocking 1: fixed — `03-apu-overview.md` §7 now states the active level
  (`RDY` active high, `0` = core halted, core repeats its last read cycle) and
  the formula the way `Specs/02-cpu-dma.md` does:
  `sprdma_rdy = NOR(StopDma, StartDma)` with `StopDma` set for the whole
  transfer, `RDY = sprdma_rdy AND DMCRDY`. It is pulled low for the whole OAM
  DMA and whenever `DMCRDY` = 0 (a DMC fetch), and released only when the
  sprite DMA is idle **and** `DMCRDY` = 1. The `RDY` normative bullet in
  Programming / implementation notes was rewritten to the same rule, so §7 and
  the normative bullet no longer invert the polarity
  (`HDL/APU/dma.v:160-162`, `BreakingNESWiki/APU/dma.md`).
- Major 1: fixed — §4 `$4018–$401A` Reaches row now reads "Test logic,
  Triangle"; §9 `$401A` write row now lists "bits 4–0 = triangle output
  counter parallel load (`TRIANGLE_Output`, `triangle.v`, `apu_top.v`)";
  §6 Direction bullet updated to "`$401A` (write: `LOCK` bit 7 and the triangle
  output counter bits 4–0, test mode only)".
- Minor 1: fixed — Observable phenomena #3 now says the `$4015` read clears the
  frame-IRQ flip-flop during `PHI2` (`PHI1` = 0), matching `clkgen.v`
  `nor(R4015_clear, n_R4015, PHI1)` and this file's own §3 write-timing rule.
- Minor 2: fixed — §7 now says "256 read/write pairs; the low address is an
  8-bit up counter whose end is the carry out of bit 7 (`dma.v`)" instead of
  attributing the count to `dma.md`.
- Minor 3: fixed — §4 `$4015` Reaches row now includes "DMA buffer".
- Minor 4: fixed — §3 LFO sentence now includes the `Z2` term in both `/LFO1`
  and `/LFO2`, described as the fifth-step qualifier latched from the mode bit
  (`HDL/APU/clkgen.v:107,113,192-200`).
- Minor 5: fixed — `$400E` bit 7 renamed `M` (diagram now `M--- PPPP`, field
  "M noise mode") so `L` means only length-counter halt in §5.
- Minor 6: fixed — `$4015` read bit 4 now reads "DMC active" in both the
  `$4015` (R) diagram row and the read-bits table.
- Minor 7: fixed — deleted the stale Open-questions bullet about `APU_Mixers`
  in other specs. Re-grepped `Specs/*.md` immediately before writing: the only
  occurrences are `APU_Mixer` (singular) in `03-apu-dmc.md:269`,
  `03-apu-noise.md:269`, `05-boards-and-io.md:326` and this file.
- Minor 8: fixed — correspondence row now uses `Cycle_reference_chart`; the
  Open-questions page-names bullet records that `Clock_rate` redirects there.
- Minor 9: fixed — §1 block table now reads "`CLK ÷ 12` (÷ 16 on the 2A07)";
  the §3 ordered chain names NTSC/PAL division too.
- Minor 10: fixed — H1 normalised to
  `# 03-apu-overview — Overview and register interface`.
- Minor 11: fixed — §4 heading widened to "Register map `$4000–$401F`".
- Adjudicated, not applied — the `apu-pulse` Verifier's contrary minor:
  `APU_Mixer` (singular) is kept. The canonical page was fetched and is
  `<title>APU Mixer - NESdev Wiki`, `wgPageName = APU_Mixer`, 22 681 bytes;
  `APU_Mixers` is a Wayback soft-404. No occurrence of `APU_Mixers` remains as
  a citation in this file.
- Also added (round-1 "Notes for the next round" gap, not a finding): §7 now
  gives the OAM DMA total cost, 513 or 514 CPU cycles, cross-referencing
  `02-cpu-dma.md`; the round-1 note that §7 gives the byte count but not the
  cycle cost.
- Length disclosure: `03-apu-overview.md` is 307 lines. `Specs/Conventions.md`
  §6 (the binding contract) allows 120–400; `REVISION.md` says 120–300. No
  sourced content was deleted to reach 300 — the file was already 308 lines at
  round 1, which that round's Verifier measured as "within 120–400".
