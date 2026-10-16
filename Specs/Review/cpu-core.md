# Review: cpu-core — round 2

## Verdict
consensus

Round-1 blocking 1 and major 1 are genuinely fixed and all ten round-1 minors
are addressed. The findings below are minor: none affects the correctness of a
rule, and nothing in the revised file contradicts the sources.

## Blocking
None.

Round-1 blocking 1 (`Specs/02-cpu-core.md` §4, `V` row, line 132) is fixed and
correct in polarity:
`| V | `SO` pin (falling edge; dead on retail parts), ALU overflow `AVR`,
`DB6` on `DB/P` | `CLV` (`0/V`), `AVR` with no overflow, `DB6` on `DB/P` | ...`.
- `BreakingNESWiki/6502/flags.md`, "Флаг V": `0/V` clears, `1/V` forces set via
  `SO`, `AVR/V` follows `AVR`, `DB/V` follows `DB6`.
- `HDL/Core6502/flags.v:104` — `v_latch1_d = ~((~AVR & avr_latch_q) |
  (~DB[6] & DB_V) | hold | Z_V)`. The terms are *clear* conditions (`dlatch`
  with `.nq`, `VOUT = v_latch1_d`), so `Z_V` clears (`0/V` = `CLV`),
  `~AVR & avr_latch_q` clears when there is no overflow (hence `AVR` sets on
  overflow) and `~DB[6] & DB_V` clears when `DB6 = 0`. `AVR` is active-high
  overflow: `alu.v:176-177` drives `AVR` from `.nq`, so `AVR = ~AVRLatch_d`,
  and `AVRLatch_d` built from `cout[6]`/`nands[7]`/`nors[7]` evaluates to
  `C6 XOR C7`.
- `flags.v:111-115` (`so_latch1/2/3` → `vset_latch` → `One_V`) is the `SO`
  falling-edge path behind "dead on retail parts"; `HDL/APU/apu_top.v:136`
  (`.core_SO(1'b1)`) and `BreakingNESWiki/APU/core.md:113` ("Терминал SO всегда
  подсоединен к `1`") support "tied to 1".
- The columns no longer list `CLV` as a setter or `AVR` as an unconditional
  clearer, and they no longer contradict the footnote.

## Major
None.

Round-1 major 1 (the open-bus `$FF` causality) is fixed:
- §8 (lines 207-213) keeps the behavioural rule, labels the initial value
  "NESdev-derived, rank 4; mechanism unresolved", and now reads "The internal
  precharge instead holds undriven *internal* bits at 1, e.g. bit 5 of P".
- The implementation note (line 317-318) says "initialised to `$FF` per NESdev's
  power-on observation", not "by the internal precharge".
- `## Open questions` has "Open-bus `$FF` mechanism" citing `busmux.v:52-55`
  (precharge covers only `SB`/`DB`/`ADL`/`ADH`) and `data_bus.v:38`
  (`bufif0 (DB_Ext, dor_latch_nq, RD)`, enabled only when `RD = 0`). Re-read
  both files: the citations are exact and the conclusion holds.

The ten round-1 minors are also confirmed fixed: 398 lines (limit 400); the §9
vector sentence now separates `0/ADH0`/`0/ADH17` (`ADH → $00`, zero-page) from
the undischarged `$FF` high byte; `n_IPC` is in the `PHI2` row with the
"both half-cycles" note; §9.1 and `## Open questions` restrict "unconfirmed by
ranks 1-2" to `A/X/Y = $00` and `C/Z/D/V/N = 0`; §8 says "last value driven
(read or write)"; the `CPU_variants` row is present; the `(d),y` pointer wrap
carries an inline `CPU_addressing_modes` citation; §2 allows `RDY2` on the
2A07; the `Visual_2A03` wording is "no page or capture is known"; and the `RDY`
Open question names `dispatch.v` `ReadyRW` (`ready_latch2 (.d(WR), .en(PHI1))`,
line 187) alongside `pads.v`.

## Minor
1. **§4 flag table, `Z` row (line 129) is incomplete.** The row gives only the
   `/DBZ` path (`Set by /DBZ (all DB bits zero) | Cleared by /DBZ false`), but
   the sources give `Z` a second, independent load path from `DB1`:
   `BreakingNESWiki/6502/flags.md`, "Флаг Z", lists `DB/P` ("поместить значение
   шины DB на регистр флагов P") and `/DB1` next to `DBZ/Z`, and
   `HDL/Core6502/flags.v:71` implements it as the `(~DB[1] & DB_P)` term of
   `z_latch1_d`. Read alone, the cell would load `Z` from the whole pulled byte
   on `PLP`/`RTI` instead of from bit 1. Every other row spells out its
   `DB<P>` path (`DB0`/`DB2`/`DB3`/`DB6`/`DB7`), so this is a completeness gap.
2. **§4 flag table names the wrong control net for `C`, `N` and `V`.** The rows
   write "`DB0` on `DB/P`", "`DB7` on `DB/P`" and "`DB6` on `DB/P`".
   `flags.md` gives those flags their own controls — `DB/C` ("DB/C … разрядом
   DB0"), `DB/N` ("DB/N … DB7"), `DB/V` ("DB/V … DB6") — and reserves `DB/P`
   for `D` and `I` (and `Z`); `HDL/Core6502/flags.v` declares `DB_C`, `DB_N`,
   `DB_V` and `DB_P` as separate ports. Behaviourally harmless, but the table is
   this section's normative artefact and should name the real nets.
3. **`## Open questions`, `Visual_2A03` item (lines 393-396) is stale.**
   "`02-cpu-instructions.md` still lists `Visual_2A03` and should be corrected in
   a revision round" is no longer true: `Specs/02-cpu-instructions.md` was
   revised after this file — its correspondence row now reads
   `Visual_circuit_tutorial` (line 309) and its remaining `Visual_2A03` mention
   (line 356) says the page does not exist (`Specs/Review/cpu-instructions.md`,
   `## Revision round 2`). The rest of the item ("no page or capture is known")
   is still correct; only the clause about the sibling file needs deleting.

## Checked against
- `BreakingNESWiki/6502/flags.md` — per-flag control signals (`0/V`, `1/V`,
  `AVR/V`, `DB/V`, `DB/C`, `DB/N`, `DB/P`, `DBZ/Z`, `DB[5]` unused), DB↔P bit map.
- `HDL/Core6502/flags.v` — `v_latch1_d` (104), `SO` detector and `vset_latch`
  (111-115), `DB[5] = 1'bz` (133), `z_latch1_d`'s `~DB[1] & DB_P` (71), ports
  `DB_C`/`DB_N`/`DB_V`/`DB_P`.
- `HDL/Core6502/alu.v` — `AVRLatch_d`/`.nq(AVR)` (176-177) ⇒ active-high overflow.
- `HDL/Core6502/busmux.v` — precharge scope (52-55), ConstGen discharge of
  `ADL[0-2]`/`ADH[0]`/`ADH[7:1]` (69-74).
- `HDL/Core6502/data_bus.v` — `bufif0 (DB_Ext, dor_latch_nq, RD)` (38) and the
  `WRLatch` `RD` polarity (42-56); only external driver, write-only.
- `HDL/Core6502/context_control.md` — `n_IPC` in both half-cycles, PC increment
  annotated in PHI2 (line 126); control-signal priority lists.
- `HDL/Core6502/dispatch.v` — `ReadyRW`: `ready_latch2 (.d(WR), .en(PHI1))`
  (187), `n_ready = ~(RDY | ready_latch2_q)` (182, 191), `DORES` gating `WR` (183).
- `HDL/Core6502/brk.v` (`IntVector`, 103-127) — only `Z_ADL0-2` are emitted for
  the vector; `HDL/Core6502/bus_control.v:145-146` derives `Z_ADH0`/`Z_ADH17`
  from `DL/ADL` for zero-page forcing.
- `HDL/APU/apu_top.v:136` (`.core_SO(1'b1)`), `HDL/Core6502/pads.v:54`
  (`assign RDY_frompad = RDY;`).
- `BreakingNESWiki/APU/core.md` — SO tied to 1 (113), SYNC floating, RDY/RDY2
  (32-33, 83-87), DBG→RDY2 on the 2A07 (18). `BreakingNESWiki/APU/pads.md` —
  pin table (M2 pin 31, DBG pin 30, R/W=1 read), A0-A15 `z` during reset,
  M2 1.5 CLK early and `z` during reset. `BreakingNESWiki/6502/pads.md` — RDY,
  SO, R/W polarity. `BreakingNESWiki/6502/regs.md` — S output/input latches in PHI1.
- NESdev cache `/tmp/nesdev-cache/` — `CPU_power_up_state` (§9.1 table matches
  verbatim), `CPU_interrupts` (reset suppresses the three writes; NMI edge;
  hijack window; delayed IRQ), `CPU_pinout` (M2 pin 31, TST pin 30, `/RST`,
  "6 M2 clocks", R/W low = write), `Open_bus_behavior`, `CPU_addressing_modes`,
  `CPU_variants`, `Visual_2A03.html` (Wayback landing page, no article).
- Template/compliance — 398 lines; five fixed headings exact and in order;
  `Status: revised (round 2)`; the correspondence table covers §1-§10.
- Sibling specs — `Specs/02-cpu-instructions.md` (revised round 2: §1 naming,
  §2 cycles, §4 frames, §5 flags all agree with this file) and
  `Specs/02-cpu-dma.md` §2 (`RDY` ignored on write cycles), §7 (2A07 `RDY2`).

## Notes for the next round
- Round-1 notes-for-next-round items were handled as the revision described: the
  `C` row keeps its set/clear columns (defensible) and the §9 reset-priority
  sentence keeps its `Readme.md`/`interrupts.md`/`brk.v` citation. No action.
- §4 `V` row footnote (line 132) still says "only `CLV` and ADC/BIT control V",
  while the same row's columns include `DB6` on `DB/P` (i.e. `PLP`/`RTI`) and
  `Specs/02-cpu-instructions.md` §5 lists `PLP`/`RTI` among the V modifiers.
  A one-word fix would read "`CLV`, ADC, BIT and `PLP`/`RTI`".
- §2 (line 79) cites `(pads.md, NESdev CPU_pinout)` for `RDY2`; neither
  `6502/pads.md` nor `APU/pads.md` mentions `RDY2` — the repository name and the
  `/DBG` mapping are in `BreakingNESWiki/APU/core.md:33,87` (§1 cites that page
  correctly). Harmless, but the citation should name `APU/core.md`.
- §9 (line 244) attaches "NESdev: 6 `M2` clocks" to holding `/RES` low;
  `CPU_pinout` gives those 6 M2 clocks as the delay *after* `/RST` is released
  before the first fetch, while the "several cycles" hold is
  `6502/Readme.md:165`. Worth separating the two if reset timing is tightened.
- Cross-file, not defects of this file:
  `Specs/02-cpu-instructions.md:282` still cites `Specs/02-cpu-core.md §12`;
  this file has content sections §1-§10, and the bit-4/bit-5 rule is §4.
  `Specs/02-cpu-dma.md` (status `adapted (round 1)`, and there is no
  `Specs/Review/cpu-dma.md`) repeats the same now-stale "`02-cpu-instructions.md`
  still lists `Visual_2A03`" clause at lines 326-331; it has not been through a
  Verifier round.

---

# Review: cpu-core — round 1

## Verdict
revise

## Blocking
1. `Specs/02-cpu-core.md` §4 ("The status register P"), the `V` row of the
   flag-source table reads:
   `| V | `CLV`, `SO` pin (falling edge) | `CLV`, ALU overflow `AVR` | ... |`.
   `CLV` is listed as a *setter* of V and the ALU overflow term `AVR` as a
   *clearer*. Both are wrong, and `CLV` appears in both columns.
   Evidence: `BreakingNESWiki/6502/flags.md`, "Флаг V":
   `0/V: Очистить флаг V (применяется во время выполнения инструкций CLV)`,
   `1/V: Установить флаг V. Принудительная установка флага производится с
   помощью контакта SO`, `AVR/V: Изменить значение флага V в соответствии со
   значением AVR`; `HDL/Core6502/flags.v:104,114-115`
   (`v_latch1_d` uses `~AVR & avr_latch_q` to set V via the SO-free path).
   Correct row: **Set by** `SO` falling edge (dead — tied high), ALU overflow
   `AVR`, `DB6` on `DB/P`; **Cleared by** `CLV` (`0/V`), `AVR` with no overflow,
   `DB6` on `DB/P`. The trailing note ("only `CLV` and ADC/BIT control V")
   matches that reading, so the printed columns contradict their own footnote.
   This is the section's core deliverable ("exact set/clear rules"), hence
   blocking rather than major.

## Major
1. §8 and `## Programming / implementation notes` attribute the external
   open-bus default `$FF` to the **internal** bus precharge: "The internal
   precharge does explain the initial value (`$FF` before `D0–D7` has ever been
   driven)" and "initialised to `$FF` by the internal precharge".
   Evidence: `HDL/Core6502/busmux.v:44-57` — `BusPrecharge` drives only
   `SB`, `DB`, `ADL`, `ADH`; `HDL/Core6502/data_bus.v:38` —
   `bufif0 (DB_Ext, dor_latch_nq, RD);` is the **only** driver of the external
   bit, and it is disabled while `RD=1` (read), so no path exists from the
   precharged internal `DB` to `DB_Ext`; `HDL/Core6502/core_top.v:630-640` maps
   `DB_Ext` to the module's `D`. The precharge therefore cannot set the initial
   level of `D0–D7`. The `$FF` default is at best the NESdev observation
   (`CPU_power_up_state` footnote: "This test relies on open bus being
   precharged by these reads"); keep the behavioural rule but label the
   initial-value claim as NESdev/rank-4 and leave the mechanism to
   `## Open questions` (which already contains the related "Open-bus edge
   case" item).
   Note: the Adapter's substantive change *is* correct — `data_bus.md` ("Если
   RD=1 - комплементарные выходные линии с DOR отрубаются … floating") and
   `data_bus.v:38` confirm the DOR cutoff on reads, so "last value driven, not
   a hard `$FF`" is right; only the causal attribution is unsupported.

## Minor
1. **Length rule.** The file is **403 lines**; `Specs/Conventions.md` §6 sets a
   120–400-line target. Trim at least 3 lines before the next round.
2. §9: "`0/ADH0`/`0/ADH17` set the high byte" of the interrupt vector.
   `BreakingNESWiki/6502/context_control.md` and `busmux.md` describe these as
   discharge-to-0 commands, and `HDL/Core6502/busmux.v:73-74` drives
   `ADH[0]=0`, `ADH[7:1]=0` (i.e. `ADH=$00`), with
   `HDL/Core6502/bus_control.v:145-146` deriving `Z_ADH0` from `DL/ADL` — this
   is the zero-page high-byte forcing that §5 already states. The vectors' `$FF`
   high byte comes from the undischarged precharge, not from these two signals.
   Correct the sentence or drop `0/ADH0`/`0/ADH17` from §9.
3. §2 table, `PHI1` row: "the PC is incremented".
   `BreakingNESWiki/6502/context_control.md` lists `n_IPC` in both half-cycles
   but annotates the PHI2 list with "(в этом полуцикле происходит увеличение
   PC)". Move the increment to the PHI2 row (or say the command is asserted in
   both halves).
4. §9.1 / `## Open questions`: the blanket "**unconfirmed** by ranks 1–2" is
   over-cautious for part of the table. NESdev `CPU_interrupts` states the reset
   sequence ("suppresses writes, decrementing the stack pointer thrice without
   modifying memory. This is why the I flag is always set on reset") and
   `HDL/Core6502/dispatch.v:183` (`nor (WR, n_ready, wr_latch_q, DORES)`)
   confirms write suppression, so `S=$FD` and `I=1` are rank-2/4-backed; only
   `A/X/Y=$00` and `C/Z/D/V/N=0` are rank-4-only. The marking is honest —
   narrow it rather than remove it.
5. §8: "a read that no device decodes returns the **last value read**".
   This copies NESdev's wording, but a preceding CPU **write** also leaves its
   value latched (`Open_bus_behavior`, "Electrical basis": the bus behaves as a
   dynamic latch once a voltage is placed on each line). Say "last value driven
   (read or write)".
6. `## NESdev Wiki correspondence` has no row for §10 ("Differences from a
   stock NMOS 6502"). `Status_flags` (D flag) and `CPU_pinout` (TST/SO) partly
   cover it, but `CPU_variants` — present both in `/tmp/nesdev-cache/` and on
   the live wiki, and specifically listing RP2A03/2A03E/G, RP2A07 and pin-30
   behaviour — is the page a reader would consult. Add a `CPU_variants` row.
7. §6, `(Indirect),Y` row: "the zero-page pointer read wraps to `$0000` …" is
   correct per NESdev `CPU_addressing_modes` (`PEEK((arg + 1) % 256)`), but it
   is not in `BreakingNESWiki/6502/Readme.md` (which documents only the `(d,x)`
   wrap). Give it an inline `CPU_addressing_modes` citation rather than leaving
   it under a rank-1 heading.
8. §2: "The processor can be halted only by `RDY`" conflicts with §1, which
   correctly lists `RDY2`/`RDY` on the 2A07. Add "…, or by `RDY2` on the 2A07".
9. `## Open questions`, `Visual_2A03` item: "No such NESdev wiki page exists
   (it was never archived …)" overstates what the evidence proves. The Wayback
   CDX has no capture of `nesdev.org/wiki/Visual_2A03` (my query returned no
   rows; the cached `Visual_2A03.html` is the "has not archived that URL" page),
   while the live `Visual_2C02` header mentions "Visual 2A03" as a hosted
   simulator, not a wiki article. Say "no wiki page/capture is known" instead of
   "does not exist".
10. `## Open questions`, `RDY` implementation item says `pads.v` passes `RDY`
    through "with no such gating". True for `pads.v:54`
    (`assign RDY_frompad = RDY;`), but the read/write dependence does exist
    inside the core: `dispatch.v:181-191` `ReadyRW` includes `WR`
    (`ready_latch2 (.d(WR), .en(PHI1))`). Name `dispatch.v` as well so the
    question is not read as "the HDL has no gating at all".

## Checked against
- `BreakingNESWiki/6502/data_bus.md` — RD=1 cuts the DOR complementary outputs,
  the output side becomes floating; internal `DB` precharge transistor sits next
  to `DL/DB`. Confirms the Adapter's open-bus change, refutes the external-$FF
  attribution.
- `HDL/Core6502/data_bus.v` (`DataBusBit`, `WRLatch`) — `bufif0(DB_Ext,
  dor_latch_nq, RD)` (line 38) only drives the external bus in write mode;
  `RD = ~(PHI2 & WR)` confirms `WR=1` = write.
- `BreakingNESWiki/6502/busmux.md`, `HDL/Core6502/busmux.v` — all four internal
  buses precharge to `$FF` in PHI2 (`SB=DB=ADL=ADH=8'b11111111`, lines 52-55);
  `0/ADL0-2`, `0/ADH0/17` discharge to 0 (lines 69-74).
- `BreakingNESWiki/6502/whatwhen.md` — PHI0/PHI1/PHI2 table, dead time,
  precharge to `$FF` at the start of each half-cycle, "ground wins" conflicts.
- `BreakingNESWiki/6502/clock.md` — PHI1=1 when PHI0=0, PHI2 follows PHI0;
  internal lag and stretched low level (non-overlap).
- `HDL/Core6502/dispatch.v` — **reset stack writes**: line 183
  `nor (WR, n_ready, wr_latch_q, DORES)` forces `WR=0` while `DORES=1`; with
  `WR=0` the write path is off. Adapter's claim verified.
- `HDL/Core6502/brk.v` — `DORES` from the reset FF; unified BRK sequence; B
  latch; `B_OUT = ~(DORES | b_latch2_d)` (B=0 during reset); vector constants
  `Z_ADL0-2`.
- `BreakingNESWiki/6502/interrupts.md`, `Readme.md` — BRK unification, NMI edge
  detection, reset flip-flop and its use for R/W.
- `BreakingNESWiki/6502/flags.md`, `HDL/Core6502/flags.v` — bit↔flag map,
  `DB[5] = 1'bz`, `DBZ` includes DB5, and the V flag control terms (blocking
  finding).
- `BreakingNESWiki/6502/context_control.md` — control-signal tables per
  half-cycle, PHI1/PHI2 execution order, PC-increment note (minor 3).
- `BreakingNESWiki/APU/core.md`, `APU/pads.md`, `Ricoh.md` — core/APU signals,
  `WR=1` = write, `RD=1` = read, `SO` tied high, `SYNC` floating, `RDY2=/DBG`
  on 2A07, M2 leads PHI2 by 1.5 CLK, M2/R/W `z` during reset, BCD cut-out
  (5 polysilicon cuts, patent US3991307).
- `Docs/Famiclones/ChipFeatures.md` — lines 70-72 confirm the famiclone BCD and
  pin-30 notes cited in §10.
- `HDL/Core6502/regs.v` — S input ("Shadow Stack reg") and output latch,
  simultaneous load/output in PHI1.
- `HDL/Core6502/addr_bus.v`, `Readme.md` — address set in PHI1, ABL/ABH hold;
  known PCL load defect matches the Open question.
- NESdev cache `/tmp/nesdev-cache/` — `CPU_power_up_state` (exact §9.1 table:
  `$00-3=$FD`, `I=1`, RAM unreliable; the values *are* the page's measurements),
  `CPU_interrupts` (NMI edge/IRQ level, CLI/SEI/PLP delay, hijacking, reset
  suppresses writes), `CPU_pinout` (divider /12 and /16, M2, TST, RST + "6 M2
  clocks", R/W low = write), `CPU_addressing_modes` (`Implicit` is the NESdev
  name; `(d),y` formula), `Status_flags` (diagram, bits 4/5 ignored on pull,
  D disabled "On the NES"), `Open_bus_behavior`, `Cycle_reference_chart`
  (`Clock_rate` redirects to it), `CPU_variants`.
- Wayback CDX — `Visual_circuit_tutorial` has 200 captures; `Status_register`
  has a 404 capture (so `Status_flags` is the right title); `nesdev.org/wiki/
  Visual_2A03` has no captures. The correspondence table's page names are
  therefore correct; the Adapter already replaced `Visual_2A03` and
  `Status_register`.
- Template/compliance pass — title + `Status` line (`adapted (round 1)`) +
  `Scope` + `Primary sources` present; the five fixed headings are spelled
  exactly (`## Observable phenomena` L299, `## Programming / implementation
  notes` L315, `## NESdev Wiki correspondence` L342, `## Provenance` L363,
  `## Open questions` L381), in order, no number prefix; 403 lines (over the
  400 limit); every content section is cited inline and appears in
  `## Provenance`, with the two gaps noted in Minor 6–7.

## Notes for the next round
- The V-flag row is a one-line fix; after it, re-check the `C` row, whose
  "Set by/ Cleared by" columns also list `ACR` and `DB0` on both sides. That is
  defensible ("change per value"), but it would read better as "changed by".
- §9 "Priority: reset wins over everything" has no inline source in the cited
  pages (`Readme.md` states only NMI > IRQ); either cite the reset FF logic in
  `brk.v` or soften it.
- Re-check the `## Provenance` row "Power-on and post-reset register values, RAM
  unreliability (NESdev-derived) | — (not stated by ranks 1–3) | —" against the
  reset-write-suppression row directly above it; once Minor 4 is applied the two
  rows should agree on which parts are silicon-derived.
- `02-cpu-instructions.md` still lists the non-existent `Visual_2A03` (recorded
  in this file's Open questions) — that is a separate brief.

## Revision round 2

All round-1 findings addressed in `Specs/02-cpu-core.md` (status now
`revised (round 2)`); no other file edited.

- Blocking 1: fixed — §4 `V` row: `SO` falling edge (annotated dead on retail
  parts) and ALU overflow `AVR` are now setters; `CLV` (`0/V`) and `AVR` with no
  overflow are now clearers; `DB6` on `DB/P` is listed on both sides as a
  value-follows term. Matches `BreakingNESWiki/6502/flags.md` ("Флаг V":
  `0/V` clears, `1/V` sets via `SO`, `AVR/V` follows `AVR`, `DB/V` follows DB6)
  and `HDL/Core6502/flags.v:104,114-115`. The trailing footnote is now
  consistent with the columns.
- Major 1: fixed, causality part deferred to Open questions — §8 and the
  implementation notes no longer attribute the external open-bus `$FF` default
  to the internal precharge; the behavioural rule ("last value driven; `$FF`
  only before anything has driven `D0–D7`") is kept, and the initial-value claim
  is labelled NESdev/rank-4. The causality question is now the Open questions
  item "Open-bus `$FF` mechanism", citing `HDL/Core6502/busmux.v:52-55`
  (precharge covers only `SB`/`DB`/`ADL`/`ADH`) and `data_bus.v:38`
  (`bufif0 (DB_Ext, dor_latch_nq, RD)` is the only external driver, disabled
  while `RD=1`).
- Minor 1: fixed — 403 → 398 lines (wording trimmed in §1, §3, §7, §9, §10; no
  sourced fact dropped). Note: `REVISION.md` §5 says 120–300 lines, but the
  binding `Specs/Conventions.md` §6 and this report both use 120–400, so the
  Conventions target was followed.
- Minor 2: fixed — §9 now states `0/ADH0`/`0/ADH17` discharge `ADH` to `$00`
  (the zero-page high-byte forcing of §5), and the vectors' `$FF` high byte is
  the undischarged precharge; `HDL/Core6502/busmux.v` added to the citation.
- Minor 3: fixed — the PC increment moved from the `PHI1` row to the `PHI2` row
  of the §2 table, with the note that `n_IPC` is asserted in both half-cycles
  (`context_control.md`).
- Minor 4: fixed — §9.1 and the Power-on Open question now say only
  `A/X/Y = $00` and `C/Z/D/V/N = 0` are rank-4-only; `S = $FD`/`I = 1` are tied
  to the reset sequence (NESdev `CPU_interrupts`; `dispatch.v` confirms write
  suppression). The `## Provenance` row was updated to agree.
- Minor 5: fixed — §8 now says "last value driven (read or write)".
- Minor 6: fixed — `CPU_variants` row added to the `## NESdev Wiki
  correspondence` table for §10.
- Minor 7: fixed — the `(d),y` pointer-wrap claim now carries an inline
  `CPU_addressing_modes` citation in the §6 table.
- Minor 8: fixed — §2 now reads "halted only by `RDY`, or by `RDY2` on the
  2A07", citing `pads.md` and NESdev `CPU_pinout`.
- Minor 9: fixed — `Visual_2A03` wording changed to "no NESdev wiki page or
  Wayback capture is known" in both the correspondence note and Open questions.
- Minor 10: fixed — the `RDY` Open question now names `dispatch.v` `ReadyRW`
  (`ready_latch2 (.d(WR), .en(PHI1))`) alongside `pads.v`, so it no longer reads
  as "the HDL has no gating at all".
- Notes-for-next-round items not in the finding list: the `C` row keeps its
  set/clear columns (the report calls it defensible), and the reset-priority
  sentence keeps its `Readme.md`/`interrupts.md`/`brk.v` citation as before.

