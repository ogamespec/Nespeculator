# Review: cpu-dma — round 2

**File under review:** `Specs/02-cpu-dma.md` (Status: revised (round 2), 371 lines).
Round 2, re-verification of the Revision round-2 note (kept below).

## Verdict

consensus

## Blocking

None.

## Major

None. Both round-1 Majors are resolved and the fixes are correct:

1. **DMC interrupt flag (round-1 Major 1) — fixed.** `02-cpu-dma.md:151-158` now
   reads: the flag "is set when the fetch that exhausts a non-looping sample
   completes **and the `$4010` IRQ enable (bit 7) is set**; with bit 7 = 0 the flag
   is held cleared, so `$4015` bit 7 reads 0 and no IRQ is raised". Confirmed
   against `HDL/APU/dpcm.v:198` (`rsff_2_4 int_ff (.res1(W4015), .res2(n_IRQEN),
   .res3(RES), .s(AssertInt), .nq(int_ff_nq))`, with `n_IRQEN` = `~$4010[7]` from
   `DPCM_ControlReg`), `HDL/Common/rsff_2_4.v:17-20` (`if (res1|res2|res3) val <= 0;
   else if (s) val <= 1` — reset dominates set) and `nor (DMCINT, int_ff_nq,
   n_IRQEN)`; and against NESdev `APU_DMC` ("bit 7 `I` — IRQ enabled flag. If clear,
   the interrupt flag is cleared."). It agrees with `03-apu-dmc.md` §2 ("0 = IRQ
   disabled and the IRQ flag is held cleared") and §8 ("`DMCINT = flag AND IRQ
   enable`, so with `$4010` bit 7 = 0 the flag is held cleared").
2. **DMC load-fetch schedule (round-1 Major 2) — fixed.** `:131-135` gives the
   schedule with an `(NESdev `DMA`)` marker: a load fetch "is scheduled to halt on a
   get cycle during the 2nd APU cycle after the write, i.e. on the 3rd or 4th CPU
   cycle after it, which matches the 3/4 alignment model below". This is the cached
   NESdev `DMA` wording verbatim ("Load DMAs … are scheduled to halt the CPU on a
   get cycle during the 2nd APU cycle after the write (that is, the 3rd or 4th CPU
   cycle)."), and it is consistent with §2's APU cycle = 2 CPU cycles and with the
   four-row fetch table at `:143-149` (load get→3 / load put→4 / reload put→4 /
   reload get→3).

## Minor

1. **Stale sibling-file clause, two places (also flagged by the `cpu-core`
   Verifier).** `## NESdev Wiki correspondence` (`:317-318`) says
   "`02-cpu-instructions.md` still lists the former and should be corrected (see
   `## Open questions`)", and `## Open questions` (`:341-346`) says "the instruction
   file still needs the same one-line fix". That is no longer true:
   `Specs/02-cpu-instructions.md` was revised after this file — its correspondence
   row now reads `Visual_circuit_tutorial` (`:309`) and its remaining `Visual_2A03`
   mention (`:356`) says the page does not exist (`Specs/Review/cpu-instructions.md`,
   `## Revision round 2`). The rest of the item (no page/capture for `Visual_2A03`)
   is still correct; delete the two clauses about the sibling file. Same defect and
   severity as `Specs/Review/cpu-core.md` Minor 3.
2. **"reset dominates set" is not fully traceable.** `:153-154` cites only
   `HDL/APU/dpcm.v` for "`int_ff` resets on `~$4010[7]` and reset dominates set",
   but the dominance is a property of the instantiated primitive
   `HDL/Common/rsff_2_4.v:17-20`, not of `dpcm.v`. `Primary sources` (`:11`) lists
   `HDL/Common/{dlatch,rsff}.v`; `rsff.v` is the two-gate primitive, while `int_ff`
   uses `rsff_2_4`. Name `rsff_2_4.v` in the sentence or metadata. Low priority.

## Checked against

- **HDLs re-checked this round for the untouched round-1 items** (the claims match
  the round-1 report's quotes; an exact line-for-line diff is not possible because
  `Specs/` is untracked, `git status` → `?? Specs/`):
  `HDL/APU/dma.v:151-162` (`rsff_2_3 StopDMA`/`StartDMA`, `nor(sprdma_rdy, ~NOSPR,
  StartDma)`, `assign RDY = sprdma_rdy & DMCReady`; `DMCRDY` is the port name in
  `dma.v`/`apu_top.v`, wired as `DMCReady`) and `:168-190` (`Address_MUX` ternary
  `SPR_PPU > SPR_CPU > DMC > CPU`); `HDL/APU/regs.v:58-77` (`REGRD`/`REGWR`
  predecode from `CpuA[5..15]` + `RnW`, PLA index `Addr_frommux[4:0]` — the
  `{CPU_A[15:5], bus_A[4:0]}` decode is unchanged); `HDL/APU/io.v:29-30,51-54`
  (`IN_topad = {n_R4017, n_R4016}` only; no APU driver for the D0–D4 return path —
  Minor 1's wording "the controller/expansion device drives bits 4–0" is correct).
  §3's 513/514 model (`:94-112`), the `$4014` page reload (`:85-87`) and the `RDY`
  formula (`:66-70`) are byte-identical to what the round-1 report verified.
- **Round-1 Minor 2 (collision byte indices) — refutation CONFIRMED.** The HDL is
  decisive: `HDL/APU/dpcm.v:551` `CounterBit addr_lo [7:0] (.d({DPA[1:0],
  6'b000000}) …)` puts `$4012[1:0]` in A7–A6, so `addr_lo_q[4:0] = 0` at load;
  `:554` `assign DMC_Addr = {1'b1, addr_hi_q, addr_lo_q}` with `:552`
  `.d({1'b1, DPA[7:2]})` gives `DMC_Addr = {2'b11, DPA[7:2], DPA[1:0], 6'b0}`. The
  round-1 arithmetic (`A[4:0]` = 0/8/16/24 from `{DPA[1:0], 000}`) was wrong.
  Independently reproduced with Icarus Verilog 14.0 (`-D ICARUS`, temp bench, since
  deleted): `$4012`=$00 → `DMC_Addr`=$C000, `A[4:0]`=0; `$FD` → $FF40, 0; `$03` →
  $C0C0, 0; stepping the counter cycles `A[4:0]` through 0,1,…,31 and wraps
  `$FFFF`→`$8000`. So the kept text "byte index 21, 22 or 23 (mod 32)" is right and
  independent of `$4012`, and `:197-201` states exactly that.
- **Round-1 Minors 1, 3–9 — all addressed.** 1: `:193-194`; 3: `:56-58`, `:208-210`
  now cite NESdev `DMA` + `HDL/Core6502/dispatch.v`; 4: `:9-12` now list
  `APU/pal.md` and `HDL/Core6502/{pads,core_top}.v`; 5: `:141` and `:228` carry
  `(NESdev `DMA`)` table captions; 6: `:311` points at `DMA`; 7: `:309` lists
  `Bus_conflict, Open_bus_behavior`; 8: `:110` gives both pairs (286.6/287.2 µs NTSC,
  308.5/309.2 µs PAL; re-computed from 513/514 × 12/21.477272 MHz and 16/26.601712
  MHz); 9: `:303`, `:316`, `:344` use `apu_/clk2` (no `apu_clk2` remains).
- **NESdev cache** — `DMA.html`: the load-DMA scheduling quote, "repeats the last
  read cycle indefinitely, making no forward progress nor handling interrupts … on
  any no-operation DMA cycle", the put/get DMC table ("load DMAs take 3 cycles and
  reload DMAs take 4 unless the halt is delayed by an odd number of cycles") and the
  `/OE` claim ("These output enables remain asserted the entire CPU cycle and even
  across adjacent cycles … controllers only see a single read for each contiguous
  set of reads"). `APU_DMC.html`: the bit-7 "If clear, the interrupt flag is
  cleared." `Standard_controller.html`: the DPCM-conflict pointer and the
  two-identical-reads poll. All match the spec's citations.
- **Template compliance** — `Status` "revised (round 2)"; the five fixed headings are
  spelled exactly and in order: `## Observable phenomena` (`:253`),
  `## Programming / implementation notes` (`:268`), `## NESdev Wiki correspondence`
  (`:296`), `## Provenance` (`:320`), `## Open questions` (`:337`), no number
  prefixes; 371 lines (inside 120–400); every content section appears in the
  correspondence table.

## Notes for the next round

- Verdict is **consensus**: no blocking finding, both round-1 Majors resolved
  inline, and the two new findings are minors that do not affect implementability.
- The stale `02-cpu-instructions.md` clause is the only cross-file defect; it is one
  deletion in two places and will otherwise be re-flagged by every section verifier
  that reads this file (cpu-core already flagged the same clause). Recommend fixing
  it as a tidy-up rather than opening a third revision round.
- The Revision note's phrase "bold `(NESdev DMA)` captions" is slightly off — the
  bold is on the caption title ("**Fetch timing**", "**Collision cost**"), the
  `(NESdev `DMA`)` tag is plain — but the round-1 requirement (inline marking of the
  rank-4 numbers) is met.
- No spec file was edited and no commit was made. Temporary Icarus artifacts
  (`DMC_Addr` bench, `*.run`, `*.vcd`) were created outside the repository under
  `/mnt/c/Work/` and deleted; `git status` is unchanged (`?? Specs/` only).

---

# Review: cpu-dma — round 1

**File under review:** `Specs/02-cpu-dma.md` (Status: adapted (round 1), 354 lines).
Round 1, first report for this brief (no earlier report existed in this file).

## Verdict

revise

## Blocking

None. The load/reload cycle model (3/4), the OAM DMA length (513/514), the
`RDY` formula, the address-multiplexer priority and the `{CPU_A[15:5],
bus_A[4:0]}` register decode all check out against the rank-1 pages, the HDL and
the cached NESdev material. The two Majors below are corrections/gaps, not a
broken specification.

## Major

1. **§4 (L143–147), DMC interrupt flag is written as independent of the `$4010`
   IRQ enable.** The sentence says the flag "is set when the fetch that exhausts
   a non-looping sample completes; it is reported in the `$4015` read (bit 7)
   and, with the `$4010` IRQ enable (bit 7) set, delivered through the APU
   interrupt line". Both sources say the enable also **holds the flag cleared**,
   so `$4015` bit 7 reads 0 while `$4010` bit 7 = 0. Evidence:
   `HDL/APU/dpcm.v:198` `rsff_2_4 int_ff (.res1(W4015), .res2(n_IRQEN),
   .res3(RES), .s(AssertInt), .nq(int_ff_nq))` with `n_IRQEN = ~$4010[7]`
   (`dpcm.v:129`), and reset dominates set in `HDL/Common/rsff_2_4.v:17-20`;
   NESdev `APU_DMC`: "IRQ enabled flag. If clear, the interrupt flag is
   cleared." The sibling spec already gets this right
   (`Specs/03-apu-dmc.md:42` "0 = IRQ disabled and the IRQ flag is held
   cleared", `:210`), so this is also a cross-file inconsistency. Fix is one
   clause: "…is set when the fetch … completes **and `$4010` bit 7 = 1**; with
   bit 7 = 0 the flag is held cleared (`$4015` bit 7 reads 0)".

2. **§4 (L128–133), the DMC *load*-fetch scheduling number is missing.** The
   text gives the halt-slot type ("first attempts to halt on a get slot") but
   not *when* the load fetch is scheduled, which an emulator author needs to
   reproduce `$4015`-write conflicts. NESdev `DMA` states it: "Load DMAs … are
   scheduled to halt the CPU on a get cycle during the 2nd APU cycle after the
   write (that is, the 3rd or 4th CPU cycle)." `Specs/Conventions.md` §7 makes
   implementable cycle numbers mandatory. The existing `## Open questions`
   entry "Load vs reload scheduling is explicit only in NESdev" does not
   resolve it — §4 already adopts the rest of NESdev's `DMA` model (3/4-cycle
   table), so the offset should be stated inline with an `(NESdev DMA)` marker.

## Minor

1. **§5 conflict table (L182–183), "APU drives bits 4–0".** For `$4016`/`$4017`
   the APU only asserts the `/IN0`/`/IN1` output enables; the D4–D0 return
   lines are driven by the controller/expansion hardware. `HDL/APU/io.v:29-30`
   (`assign IN_topad[0] = n_R4016;`) plus `io.v:51-54`: `IOPorts` drives only
   the pads, and no bus driver is gated by `n_R4016`/`n_R4017` anywhere in the
   APU; `BreakingNESWiki/APU/pads.md:112` says the same ("Выходное значение для
   контактов `/IN0-1` - это внутренние сигналы `/R4016` и `/R4017`"); NESdev
   `Controller_reading` ("xxxD DDDD … Input data lines"). Say "the controller/
   expansion device drives bits 4–0; the APU asserts `/IN0`".
2. **§5 (L186–189), "byte index is 21, 22 or 23 (mod 32)".** The collision
   phase is set by `A[4:0]`, which starts at `{$4012[1:0], %000000}` — 0, 8, 16
   or 24 (`HDL/APU/dpcm.v:551-554`, `addr_lo .d({DPA[1:0],6'b000000})`). The
   indices 21/22/23 are therefore correct only for `$4012[1:0] = 0`; in general
   they are `(21, 22, 23 − {$4012[1:0],000}) mod 32`. The OAM-DMA sentence
   directly below is correct (the OAM counter starts at `$00`).
3. **Inline traceability of "the core repeats its last read cycle" (L55–59,
   L196–206).** Both passages cite `APU/dma.md` / `APU/dpcm.md`, but those pages
   only state that the halt is ignored on write cycles (`dma.md:45`) and that
   DMC DMA halts only for the fetch (`dpcm.md:11`) — neither says the read is
   re-issued or externally visible. That is NESdev `DMA` rank-4 ("When RDY is
   deasserted, the 6502 core repeats the last read cycle indefinitely, making no
   forward progress nor handling interrupts. On 2A03 CPUs, these repeated reads
   are externally visible on any no-operation DMA cycle") plus the HDL
   (`HDL/Core6502/dispatch.v:182,187,191` — `n_ready` freezes the cycle
   counters). Add the NESdev `DMA` / `dispatch.v` marker to those sentences as
   the file already does for the get/put parity and the register quirk.
4. **Metadata (L9–11).** `Primary sources` omits two cited sources:
   `BreakingNESWiki/APU/pal.md` (cited at L205, L233 and the Open question at
   L341) and `HDL/Core6502/pads.v` / `HDL/Core6502/core_top.v` (cited in the
   `## Provenance` row at L317). `Specs/Conventions.md` §6.2 requires the
   metadata block to list the file's sources.
5. **Rank-4 cycle numbers are not marked inline.** §4's 3/4-cycle table
   (L135–141) and §6's collision table (L215–219) are NESdev-only numbers (the
   wiki gives no counts; `APU/waves.md` shows only "aligned" vs "1 additional
   CPU cycle" and the DMC/OAM overlap). Only `## Provenance` (L313, L318)
   records `NESdev DMA`. The Adapter asked for this marking; one `(NESdev
   DMA)` tag in each table caption is enough.
6. **`## NESdev Wiki correspondence` (L296), `CPU_interrupts` row.** "Interrupts
   blocked while `RDY` is low" appears in NESdev `DMA` ("making no forward
   progress nor handling interrupts"), not in `CPU_interrupts` — a grep of the
   cached page finds no `RDY`, `DMA` or `halt`. Point the row at `DMA` (or list
   both) so a reader is sent to the page that states it.
7. **`## NESdev Wiki correspondence` (L294), `Open_bus_behavior` row.** "Bus
   conflicts and open bus during DMA": `Open_bus_behavior` covers open bus
   (L25 "A DMC DMA may also alter the last value read …", L90 a "Bus conflict"
   See-also), but the bus-conflict content lives on the separate `Bus_conflict`
   page. Add it to the row.
8. **§3 wall-clock (L107–108).** "513 or 514 CPU cycles ≈ 286.6 µs (NTSC) or
   ≈ 308.5 µs (PAL)" quotes only the 513-cycle duration for both regions
   (513 × 12/21.477272 MHz = 286.6 µs; 513 × 16/26.601712 MHz = 308.5 µs). The
   514-cycle values are ≈ 287.2 µs and ≈ 309.2 µs; give the pair or drop the
   second figure.
9. **Correspondence row (L288) names `apu_clk2`.** The NESdev node names in
   `Visual_circuit_tutorial` are `apu_clk1`, `apu_/clk2` and `apu_clk2_x` (the
   25%/75% duty clocks); there is no `apu_clk2` node. The page-name claim
   itself is verified (see "Checked against").

## Checked against

- `BreakingNESWiki/APU/dma.md` — OAM DMA address low/high counters, `W4014`
  load, `#DMC/AB`, `RUNDMC`, `DMCRDY`, address MUX sources, `PHI1` read-cycle
  detect and the PHI1/ACLK note (L13, L39–45, L54–58, L70–75). Supports §1–§3
  and §5; does **not** state the repeated-read behaviour (Minor 3).
- `BreakingNESWiki/APU/dpcm.md` — `RUNDMC`/`DMCRDY`/`#DMC/AB` meanings (L32–34),
  "DMC DMA halts the CPU only for the sample fetch" (L11), `DMC_A15` tied to
  Vdd (L234), up/down counters. Supports §1, §4 and §7.
- `BreakingNESWiki/APU/core.md` — `DBG`→`RDY2` on 2A07 (L18, L33), `A[15:0]` from
  the address MUX (L37), register-select note (L36), no `RDY` package pin.
- `BreakingNESWiki/APU/pads.md` — pinout (no `RDY`), `/IN0-1` = `/R4016`/`/R4017`
  (L112), internal↔external data bus directly connected (L54).
- `BreakingNESWiki/APU/regs.md` — the ⚠ note that the register index comes from
  the MUX `A0-A4` while the space comes from `CPU_A` (L66); signal table.
- `BreakingNESWiki/APU/clkgen.md` — CLK/PHI/ACLK table (L49–52: 558.73/1117.46 ns
  NTSC, 601.46/1202.93 ns PAL), "ACLK1 and /ACLK2 are not complementary and do
  not overlap" (L45), register ops inactive during PHI1 (L25).
- `BreakingNESWiki/APU/waves.md` — OAM DMA start aligned/unaligned ("requires 1
  additional CPU cycle", L168–174); DMC/OAM overlap and the `RUNDMC` "window"
  (L212). Confirms §3 and §6 qualitatively, no collision counts.
- `BreakingNESWiki/APU/old_2a03.md` — letterless `StartDma` not reset by `RES`
  (OAM DMA section). Confirms §7.
- `BreakingNESWiki/APU/pal.md` — "Test Mode … нет", "Register Select TBD",
  "RDY TBD" (L136–142). Confirms §5/§7's 2A07-mechanism caveat.
- `BreakingNESWiki/6502/pads.md` / `6502/decoder.md` — `RDY` → internal `RDY`
  plus delay chain → `/PRDY`, and the Branch-T0 skip when the previous
  instruction ended not-ready (decoder.md L200). Confirms §2.
- `HDL/APU/dma.v` — `Address_MUX` ternary priority `SPR_PPU > SPR_CPU > DMC >
  CPU` (L187–190); `nor(sprdma_rdy, ~NOSPR, StartDma)` + `RDY = sprdma_rdy &
  DMCReady` (L161–162); `SPR_PPU/SPR_CPU` = `NOR(NOSPR, RUNDMC, ~toggle/toggle)`
  (L155–156); `SPRS` gated by `RUNDMC` (L145); `SPRDMA_AddrLowCounter`
  `EndCount = cc[7]`, `Load(W4014)` clears to 0; `SPRDMA_AddrHigh` register.
  Confirms §1–§3, §5.
- `HDL/APU/dmabuf.v` — `nor(RW_topad, ~RnW_fromcore, SPR_PPU)` forces external
  R/W to write during `SPR_PPU`; `dlatch spr_buf … .en(PHI2)`; tri-state enabled
  by `~SPR_PPU`. Confirms §3.
- `HDL/APU/core.v`, `HDL/APU/apu_top.v`, `HDL/APU/clkgen.v` — delimiter /12,/16,
  PHI0/PHI1/PHI2, ACLK1//ACLK2 derivation; `PHI1 = ~PHI0`, so **HDL `PHI1 = 0`
  is the PHI2 half** — the parenthetical at L51 is correct.
- `HDL/APU/regs.v` — `REGRD`/`REGWR` space predecode from `CpuA[5..15]` +
  core `RnW` (L58–64); PLA index on `Addr_frommux[4:0]` (L72–77); `n_R4015 =
  ~pla[4]`, `n_R4016 = ~pla[23]`, `n_R4017 = ~pla[21]` (→ `$15/$16/$17`) and
  `pla[0..2]` → `$4018–$401A` gated by `DBG`; write selects gated by `PHI1` and
  `nREGWR` (core write). Confirms §2 and §5.
- `HDL/APU/io.v` — `IN_topad = {n_R4017, n_R4016}` only; no `DB` driver for the
  joypad registers (Minor 1).
- `HDL/APU/dpcm.v` — sample counter `{DSC,4'b0000}` 12-bit down, `SOUT =
  cout[11]` (L461–468); address `DMC_Addr = {1'b1, addr_hi, addr_lo}` with
  `{1,DPA[7:2]}`/`{DPA[1:0],6'b0}` (L544–554); `PCM = NOR(PHI1, n_DMCAB)`
  (L326); `DMCINT = NOR(int_ff_nq, n_IRQEN)` and `int_ff.res2 = n_IRQEN`
  (L198–200) — Major 1.
- `HDL/APU/core.v`, `HDL/APU/apu_top.v` — PHI/ACLK wiring and core `RDY` input.
- `HDL/Common/rsff_2_4.v` — reset (res1–3) dominates set. Evidence for Major 1.
- `HDL/Core6502/dispatch.v` — `ReadyRW`: `ready_latch1_d = NOR(RDY,
  ready_latch2_q)`, `ready_latch2.d = WR`, `assign n_ready = ~ready_latch1_nq`
  (L182–191); `n_ready` freezes `TwoCycle`/`RMWCycle` counters. Confirms "no
  forward progress while `RDY = 0`" and the write-cycle gating; supports Minor 3.
- `HDL/Framework/Icarus/apu/oam_dma.v`, `both_dma.v` — the only local DMA
  testbenches; no per-collision assertions, so the numbers must come from NESdev.
  **I compiled and ran `both_dma.v` with Icarus 12.0** (`iverilog -D ICARUS …`,
  then parsed the VCD) and it independently reproduced the §3/§6 model:
  256 OAM reads at `$0200–$02FF` in order, one read + one write per 2 CPU cycles
  (512 cycles), the unaligned second DMA starting exactly one CPU cycle later
  than the aligned first (`SPR_CPU` first edge cycle 2736 vs 5470), a DMC get
  occupying exactly 1 CPU cycle and the mid-OAM collision costing **exactly +2
  cycles** (read gap 3066 → 3070, with `n_DMCAB = 0` on cycle 3068), while the
  OAM source counter stays frozen (next read `$02A6`). Generated
  `both_dma.run`/`both_dma.vcd` were deleted afterwards.
- `/tmp/nesdev-cache/DMA.html` — 513/514 OAM DMA and its two example sequences;
  halt ignored on writes and up-to-3-cycle delays; get/put = `apu_clk1`/
  `apu_clk2`, random power-on parity; repeated reads of `$2002/$2007/$4015`;
  `{A[4:0] bus, CPU_A[15:5]}` register activation and 2A07 fix; DMC 3/4 cycles;
  load DMAs scheduled on the get cycle of the 2nd APU cycle after the write;
  mid-OAM DMC = +2, second-to-last put = +1, last put = +3; joypad `/OE` held
  across contiguous reads; UA6527P sample-end note. Verified §2–§7; source of
  Majors 2 and Minors 3/5/6.
- `/tmp/nesdev-cache/APU_DMC.html` — "stalled for 1-4 CPU cycles … described in
  detail in the DMA article" (L293); "IRQ enabled flag. If clear, the interrupt
  flag is cleared." (L30); "This problem has been fixed on the 2A07" (L318);
  the speculative 4-cycle RDY wording (Open question L332 is accurate).
- `/tmp/nesdev-cache/CPU_pinout.html` — pin 30 `TST` (usually GND), `$4018–$401A`
  on `RP2A03G`, and on `RP2A07` pin 30 high "causes the CPU to stop execution by
  means of activating the embedded 6502's RDY input". Confirms §1 and §7.
- `/tmp/nesdev-cache/CPU_memory_map.html` — "a quirk in the 2A03's register
  decoding can cause DMA to misbehave if the CPU is halted while reading from
  $4000-$401F". Confirms §5's citation.
- `/tmp/nesdev-cache/PPU_OAM.html` — "takes 513 CPU cycles (+1 on odd CPU
  cycles)"; Open question L336 quotes it correctly.
- `/tmp/nesdev-cache/Open_bus_behavior.html` — DMC DMA can alter open bus; the
  bus-conflict content is a separate page (Minor 7).
- Wayback `Visual_circuit_tutorial` (2024 capture) — documents `apu_clk1`
  (25%, high only while φ2 low), `apu_/clk2` (75%) and `apu_clk2_x`, and the
  6502 phase timing. Confirms the correspondence row and the §2 non-complementary
  claim; Minor 9 on the node name.
- Wayback `Controller_reading` (2024 capture) — `$4016`/`$4017` read is
  `xxxD DDDD`, DPCM double-clock/bit-deletion, multiple-read and OAM-synchronized
  workarounds. Confirms §5's rank-4 joypad marking and Minor 1.
- Wayback `Visual_2A03` — "The Wayback Machine has not archived that URL",
  confirming the `## Open questions` / note at L300–303.
- `Cycle_reference_chart` — `Clock_rate` fetch returns title "Cycle reference
  chart" (redirect claim at L300 correct).
- Template/compliance pass — `Status` line "adapted (round 1)", `Scope`,
  `Primary sources` present; the five fixed headings are spelled exactly
  (`## Observable phenomena` L238, `## Programming / implementation notes`
  L253, `## NESdev Wiki correspondence` L281, `## Provenance` L305,
  `## Open questions` L322), in order, no number prefix; 354 lines (inside the
  120–400 limit).

## Notes for the next round

- Both Majors are single-clause edits and do not need a rewrite; after them the
  section is close to consensus. Prefer keeping the simulation-verified wording
  of §3/§6 unchanged.
- NESdev `DMA` also documents the *aborted* DMC DMA (playback stopped on the APU
  cycle before a reload DMA schedules) and the RP2A03G/late-2A03G "unexpected
  reload DMA" bug. These are rank-4 and revision-specific; if they are not added
  to §4, add one `## Open questions` line so the omission is deliberate.
- §5's "the mechanism is not established here" for the 2A07 is correct and
  well-hedged; do not strengthen it from `APU/pal.md` ("RDY TBD",
  "Register Select TBD").
- `Specs/02-cpu-instructions.md` still lists the non-existent `Visual_2A03`
  (already recorded in this file's Open questions at L326–331); that is a
  different brief.

## Revision round 2

- Blocking: none in round 1.
- Major 1: fixed — `Specs/02-cpu-dma.md` §4 now states that the DMC interrupt
  flag is set only while the `$4010` IRQ enable (bit 7) = 1, and is held cleared
  (so `$4015` bit 7 reads 0) while bit 7 = 0, citing `HDL/APU/dpcm.v` (`int_ff`
  reset by `~$4010[7]`, reset dominating set in `HDL/Common/rsff_2_4.v`) and
  NESdev `APU_DMC`. Now agrees with `Specs/03-apu-dmc.md` (§2 bit table, §8).
- Major 2: fixed — §4 now gives the load-fetch schedule: the halt is attempted on
  a get cycle during the 2nd APU cycle after the `$4015` write, i.e. the 3rd or
  4th CPU cycle after it, and is stated to match the 3/4-cycle table, with an
  `(NESdev DMA)` marker. The stale Open question "Load vs reload scheduling is
  explicit only in NESdev" was reworded into a rank-4 provenance note (§4 inline),
  not deleted.
- Minor 1: fixed — §5 `$4016`/`$4017` rows now read "the controller/expansion
  device drives bits 4–0" and keep the `/IN0`/`/IN1` assertion.
- Minor 2: **refuted, not applied** — the finding's arithmetic is wrong.
  `addr_lo.d = {DPA[1:0], 6'b000000}` (`HDL/APU/dpcm.v`) places `$4012[1:0]` in
  A7–A6, so `A[4:0]` loads 0 for every `$4012`, not 0/8/16/24. I recompiled and
  ran `DPCM_AddressCounter` with Icarus Verilog 14.0 (`-D ICARUS`) for
  `$4012` = $00/$01/$02/$03/$FD: `DMC_Addr` = $C000/$C040/$C080/$C0C0/$FF40 and
  `DMC_Addr[4:0]` = 0 in all five cases. The original "byte index 21, 22 or 23
  (mod 32)" text is correct and was kept; §5 gained an inline `HDL/APU/dpcm.v`
  citation making the independence from `$4012` explicit, so the next round can
  re-check it.
- Minor 3: fixed — the repeated-read claims in §2 (Halt rule) and §5 (Repeated
  reads) now cite NESdev `DMA` and `HDL/Core6502/dispatch.v`; the `APU/dma.md`
  citation was removed from those two sentences.
- Minor 4: fixed — `Primary sources` now lists `APU/pal.md` and
  `HDL/Core6502/{pads,core_top}.v`.
- Minor 5: fixed — §4 and §6 tables now carry bold `(NESdev DMA)` captions.
- Minor 6: fixed — correspondence row now points at `DMA` (not `CPU_interrupts`).
- Minor 7: fixed — the open-bus/bus-conflict correspondence row now lists
  `Bus_conflict` alongside `Open_bus_behavior`.
- Minor 8: fixed — §3 gives both wall-clock pairs (≈ 286.6 / 287.2 µs NTSC,
  ≈ 308.5 / 309.2 µs PAL for 513 / 514 cycles).
- Minor 9: fixed — the NESdev node name is `apu_/clk2`; corrected in the
  correspondence row and in the two later mentions of the node names.
- Untouched (per the round-1 verdict): the simulation-verified 513/514 model, the
  `$4014` page reload, the `RDY` formula, the address-mux priority and the
  `{CPU_A[15:5], bus_A[4:0]}` decode.
- Files edited: `Specs/02-cpu-dma.md` only (Status now `revised (round 2)`,
  371 lines). No other file touched, no commit. The refuted Minor 2 is recorded
  here and inline in §5 rather than "fixed" so no incorrect formula is
  introduced.
