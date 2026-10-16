# Review: apu-fc-mixer — round 3

**Files under review:** `Specs/03-apu-frame-counter.md` (279 lines, `Status: revised (round 3)`) and
`Specs/03-apu-mixer.md` (253 lines, `Status: revised (round 2)` — unchanged this round). This report
is the only file written; no spec file was edited, nothing was committed.

## Verdict

consensus

## Blocking

None.

## Major

None. Round-2 Major 1 and Major 2 are resolved; both re-verified independently below.

1. *(round-2 Major 1 — resolved)* `03-apu-frame-counter.md` Open questions (lines 256–267) now gives
   NESdev's mode-0 step boundaries as cumulative 7457 / 14913 / 22371 / **29828** CPU cycles with
   deltas 7457, 7456, 7458, 7457 (sum = **29828**), an extra quarter+half clock at 29829, the counter
   reset at 29830, the mode-1 totals **37281** / **41565** CPU (18640.5 / 20782.5 APU), and identifies
   29829 as the mode-1 step-4 time rather than a mode-0 boundary. Re-derived from
   `/tmp/nesdev-cache/APU_Frame_Counter.html` (Wayback 20250113123532): mode-0 APU rows
   3728.5 / 7456.5 / 11185.5 / 14914 / 14914.5 / 0 (14915) → ×2 =
   7457 / 14913 / 22371 / 29828 / 29829 / 29830 CPU; mode-1 APU rows … / 14914.5 / 18640.5 / 0 (18641)
   → step 4 = 29829, step 5 = 37281; PAL mode-1 16626.5 / 20782.5 → 33253 / 41565; the page's
   "the interrupt flag is set every 29830 CPU cycles" and PAL 33254 match the quoted period figures.
   The repository totals stay normative (4 × 7457 = 29828; 4 × 8313 = 33252; 3 × 7457 + 14913 = 37284;
   3 × 8313 + 16625 = 41564 — all re-checked). The extra clause "NESdev's 29829 is that mode-1 table's
   step-4 time … not a 4-step boundary" is accurate; it sits next to the also-correct mode-0
   extra-clock sentence because both NESdev rows really are 14914.5 APU.

2. *(round-2 Major 2 — resolved)* §8 (lines 174–180) no longer quotes a concrete cycle offset. It now
   states only that a write shortly before step 4 restarts the sequencer, which reaches its own step 4
   "one mode-0 period later (29828 CPU cycles NTSC / 33252 PAL), counted from the 3–4-cycle-later
   restart rather than from the write cycle" — consistent with the 3–4-cycle latency at lines 170–173.
   `29600`, `59428` and `59430` no longer occur anywhere in either spec (a grep over `Specs/` finds
   them only in this review file's history).

## Minor

1. *(new, cosmetic)* `03-apu-frame-counter.md` §5, line 127: "The same spread appears in mode 4"
   follows "4 CPU cycles apart (37284 vs 37280)", but the mode-4 spread it then quotes is **2** CPU
   cycles (29828 vs 29826). The figures on both sides are correct; only the word "same" overstates the
   match. Reword to "A similar spread appears in mode 4" if the paragraph is touched again. No effect
   on any number, table or rule.

2. *(round-2 Minor 1 — resolved)* `Provenance`, line 252: the wiki cell now reads
   "`APU/clkgen.md` (PLA interval tables — rank 1)" and the HDL cell confines rank 4 to the write
   latency and the unreproducible ad-hoc run ("`HDL/Framework/Icarus/apu/clkgen.v` checks clock-tree
   transitions only, so the write latency is NESdev rank 4 and the ad-hoc interval run is not
   reproducible"). Matches the committed benches: `.../apu/clkgen.v` is the "checking APU Clock Trees"
   test; `.../apu/softclk.v` still says "This is not a unit test right now".

3. *(round-2 Minor 2 — resolved)* §5, lines 123–129 now also cites the mode-4 `nLFO2` case
   (`clkgen.md:159`: PLA1 14913, then PLA3 14913 → **29826**, vs the `nLFO1` row's 29828), alongside
   the mode-5 `nLFO2` PLA4 = 22367 case. The §5 "no activation" row (line 113) and the exact
   cumulatives no longer carry "~": `29828 / 14914` and `33252 / 16626`. The remaining "~" characters
   are only on genuinely approximate quantities (clock periods at lines 34–35, LFO rates at 97–98,
   123, 145).

## Checked against

- `/tmp/nesdev-cache/APU_Frame_Counter.html` (Wayback 20250113123532) — full text: `$4017` `MI--.----`,
  bit 7 mode / bit 6 inhibit, "After 3 or 4 CPU clock cycles*, the timer is reset" with the
  during/between-APU-cycle footnote, mode-0 rows 3728.5 / 7456.5 / 11185.5 / 14914 / 14914.5 /
  0 (14915) APU and "every 29830 CPU cycles" (PAL 33254), mode-1 rows through 14914.5 / 18640.5 /
  0 (18641) APU with "the frame interrupt flag is never set". All round-2/round-3 reconciliation
  arithmetic re-derived from these rows.
- `/tmp/nesdev-cache/APU_registers.html` — `$4015` write `---D NT21`, read `IF-D NT21`; `$4017`
  `SD-- ----`. Matches §2/§8.
- `/tmp/nesdev-cache/APU_Mixer.html` — 31-entry pulse table, 203-entry `tnd_table` indexed
  `3*triangle + 2*noise + dmc`, "within 4%", NES filters 90 Hz / 440 Hz / 14 kHz, Famicom 37 Hz.
  `APU_Mixers` has no Wayback snapshot, so the singular `APU_Mixer` in the mixer's notes and
  correspondence (lines 199, 214) is the right page name.
- `BreakingNESWiki/APU/clkgen.md` — `:94–97` LFO rates mode 0 / mode 1; `:154–170` NTSC/PAL
  `nLFO1`/`nLFO2` activation tables (mode-4 `nLFO2` = PLA1 14913, PLA3 14913 → 29826; mode-5 `nLFO2`
  = PLA1 14913, PLA4 22367 → 37280 vs `nLFO1` 37284); `:45–52` ACLK periods; `:149–152` output 3
  interrupt/mode-0 only, output 4 mode-1 only, output 5 anti-zero, activation order 0,1,2,3 / 0,1,2,4;
  `:170` floating-last-output caveat.
- `HDL/APU/clkgen.v` — `:148–149` `mode`←`DB[7]`, `mask`←`DB[6]`, both `ena(W4017)`; `:146`
  `R4015_clear = nor(n_R4015, PHI1)`; `:150` `int_ff` set = `n_mdout & PLA_in[3]`, resets
  `RES`/`R4015_clear`/`mask_clear`; `:155–156` `Timer_Int` = OR with `DMCINT`; `:107`/`:113`
  `nLFO1` = PLA0–4, `nLFO2` = PLA1/PLA3/PLA4; `:186–188` `F1` from PLA3/PLA4 + `nACLK2`; `:199`
  `z_ff` reset by `W4017`; `:226` `F1 ? 1'b1` (reset state all ones). Confirms §2, §3, §6, §8, §9 and
  the `$40`/`$C0` rule.
- `HDL/APU/regs.v:113` — `nor(W4017, PHI1, ~pla[27])`; `HDL/APU/common.v` `RegisterBit` — transparent
  while `ena`, no reset port (mode/mask power up at 0).
- `HDL/Framework/Icarus/apu/clkgen.v`, `softclk.v` — the only committed clkgen benches; neither
  measures frame-counter intervals or `$4017` write latency, so the round-1 relabelling and the
  round-3 rank-1/rank-4 split are honest.
- `Scripts/auxa.mem` and `HDL/Framework/Icarus/apu/auxa.mem` (byte-identical) — decoded all 256
  big-endian floats; exactly 31 constant `SQA+SQB` groups, range −0.9972 … +1.0000; **all 31** §4 rows
  (mV and normalized) match, including the five round-1 corrections 0.1034 / 0.4951 / 0.6867 / 0.8129
  / 0.9379.
- `Scripts/auxb.mem` and `HDL/Framework/Icarus/apu/auxb.mem` (byte-identical) — decoded all 32768
  floats with index `{DMC[6:0],RND[3:0],TRI[3:0]}`; exactly 218 constant `4*TRI + 2*RND + DMC` groups,
  range −0.9991 … +1.0000; **all 31** representative §5 rows match (mV and normalized); max
  `g_B` = 217 (DMC 127 / RND 15 / TRI 15) → 1088.26 mV.
- `Scripts/APU_DAC_Generator.py` — `IntRes = 39 kΩ`, `IntUnloaded = 999999 Ω`, `ExtRes = 100 Ω`,
  `Vdd = 5 V`, `tri_reciprocal = [2,4,8,16]`, `rnd_reciprocal = [1,2,4,8]`,
  `dmc_reciprocal = [0.5…32]`; re-derived `R_int = 39 kΩ / g_A`, `R_int = 78 kΩ / g_B`, 357.14 mV
  (AUX A, 100 Ω), 1088.26 mV (AUX B, 100 Ω), 272.7 mV (AUX A, 75 Ω) and the 4 : 2 : 1 per-LSB
  relation.
- `Specs/Conventions.md` §3 (silicon wins / record the mapping), §6 (fixed headings, 120–400 lines),
  §7 (traceability), §10 (round cap and consensus definition).
- Structure: both files carry exactly the five fixed headings — `## Observable phenomena`,
  `## Programming / implementation notes`, `## NESdev Wiki correspondence`, `## Provenance`,
  `## Open questions` — unnumbered and in order, plus a `Status` line, at 279 / 253 lines (120–400).

## Notes for the next round

- Round-2 Major 1, Major 2, Minor 1 and Minor 2 are all resolved **and independently re-verified**;
  no blocking or major finding remains, so this section reaches **consensus**. Only the cosmetic
  "same spread" wording (Minor 1) is open, and it changes no value.
- Round-1 Blocking 1, Majors 3–4 and all nine Minors are still fixed in the current text (item map in
  the round-2 report below): `$40` selects the 4-step sequence (lines 163–166), the Icarus evidence
  stays labelled ad-hoc/unreproducible (lines 172–173, 179–180, 252, 268–272), NESdev's mode-1 totals
  are recorded (lines 261–263), §9 states the all-ones reset (lines 188–191), and the mixer's AUX A/B
  tables, 4 : 2 : 1 weighting and singular `APU_Mixer` are unchanged.
- Nothing else changed this round: the normative tables (29828 / 33252 / 37284 / 41564), the 4 : 2 : 1
  weighting and both AUX transfer tables still match their sources exactly.

---

# Review: apu-fc-mixer — round 2

**Files under review:** `Specs/03-apu-frame-counter.md` (273 lines) and
`Specs/03-apu-mixer.md` (253 lines); both `Status: revised (round 2)`. This
report is the only file written; no spec file was edited, nothing was committed.

## Verdict

revise

## Blocking

None. The round-1 blocking finding is resolved: `03-apu-frame-counter.md` §8
lines 161–164 now states `$80` = 5-step/interrupt enabled, `$40` = 4-step
sequence + interrupt inhibited, `$C0` = inhibit while in 5-step mode. It agrees
with §2 line 48 and with `HDL/APU/clkgen.v:148–149`
(`RegisterBit mode (.d(DB[7]), .ena(W4017), ...)` /
`RegisterBit mask (.d(DB[6]), .ena(W4017), ...)`, enabled by
`HDL/APU/regs.v:113` `nor(W4017, PHI1, ~pla[27])`). The false "keeps the current
mode" sentence is gone.

## Major

1. **`03-apu-frame-counter.md` Open questions, lines 255–257 — the NESdev
   reconciliation still does not add up and misstates the source's mode-0
   table.** The bullet says "NESdev's 4-step table puts the step boundaries at
   cumulative 7457 / 14913 / 22371 / 29829 CPU cycles (deltas 7457, 7456, 7458,
   7458), i.e. **29830** total for NTSC". The four listed deltas sum to
   **29829**, so "i.e. 29830" does not follow. Concretely, NESdev
   `APU_Frame_Counter` (Wayback 20250113123532) mode-0 rows are 3728.5 / 7456.5
   / 11185.5 / **14914** APU cycles, then 14914.5 and "0 (14915)" — i.e. CPU
   7457 / 14913 / 22371 / **29828** / 29829 / 29830. The quoted "29829" is
   NESdev's **mode-1** step-4 time (its mode-1 row is 14914.5 APU = 29829 CPU —
   the same figure §5, line 117 of this file quotes), not a mode-0 step
   boundary; NESdev's 29830 is the period after which the counter resets
   ("the interrupt flag is set every 29830 CPU cycles"), not the sum of four
   mode-0 deltas. Correct text: "NESdev's mode-0 steps fall at cumulative
   7457 / 14913 / 22371 / **29828** CPU cycles (deltas 7457, 7456, 7458, 7457);
   it puts an extra quarter+half clock at 29829 and resets the counter at
   29830, and quotes the interrupt period as 29830 / 33254." This is the
   remaining half of round-1 Major 2.

2. **`03-apu-frame-counter.md` §8, lines 173–177 — the ad-hoc observation
   contradicts the write latency stated immediately above it, and the recorded
   value changed between rounds.** §8: "a mode-0 write at CPU cycle 29600 …
   moved the next frame IRQ to CPU cycle 59428 — one whole 29828-cycle sequence
   later". `59428 = 29600 + 29828` requires the sequencer to restart in the
   same CPU cycle as the write, but lines 165–171 (and NESdev) say the effect
   lands **3–4 CPU cycles after the write cycle**: a restart at 29603/29604
   puts the next step-4 IRQ at 59431/59432. The round-1 report of this same
   sentence recorded the observation as **59430** (59430 − 29600 = 29830); the
   round-2 file states 59428. The run is explicitly uncommitted and, per this
   file's Open questions, unreproducible from the tree —
   `HDL/Framework/Icarus/apu/clkgen.v` scans only PHI0/PHI1/PHI2/ACLK
   transitions, `softclk.v` says it "is not a unit test right now", and
   `lfo.v` is a deliberately bogus accelerated LFO source, not a frame-counter
   bench — so neither figure can be checked. Re-tuning an unverifiable
   measurement to the normative 29828 removes the evidence trail and also
   deletes the interesting tension it carried (an HDL run implying the 29830
   period). Fix: drop the concrete 3-cycle / 29600 → 59428 figures until a
   committed bench exists, or restore the value actually measured and state the
   discrepancy it implies; the observable claim needs no number.

## Minor

3. **`03-apu-frame-counter.md` Provenance, line 249 — "interval figures are
   NESdev rank 4" misclassifies the normative numbers.** The cell reads
   "`HDL/Framework/Icarus/apu/clkgen.v` checks clock-tree transitions only;
   write-latency and interval figures are NESdev rank 4, with no committed
   bench". The interval figures are rank 1: the same row's wiki column names
   `APU/clkgen.md` (PLA interval tables), and §4/§5 derive
   29828/33252/37284/41564 from it. Only the write latency (3–4 cycles) and the
   uncommitted run are NESdev rank 4. Reword, e.g. "…; the committed bench
   checks clock-tree transitions only, so the write latency is NESdev rank 4
   and the ad-hoc interval run is not reproducible".

4. **`03-apu-frame-counter.md` §5, line 113 — only one of the two `nLFO2`
   floating figures was added (round-1 Minor 10).** The mode-5 `nLFO2`
   PLA4 = 22367 is now cited (lines 124–127), but the mode-4 `nLFO2` row
   (`APU/clkgen.md:159`) gives PLA1 (14913) and PLA3 (14913), i.e. 29826 for
   the mode-0 sequence — a second instance of the "slightly floating … last
   output" the §4 caveat quotes. Optional, but if the caveat names one figure
   it should name both or neither. Related nit: the exact NTSC cumulative in
   the line-113 cell is written "~29828" while the PAL cell is "~33252".

## Checked against

- `HDL/APU/clkgen.v:146–153` — `R4015_clear = nor(n_R4015, PHI1)`,
  `RegisterBit mode`←`DB[7]`, `mask`←`DB[6]`, both `ena(W4017)`;
  `int_ff` set = `n_mdout & PLA_in[3]`, reset by `RES` / `R4015_clear` /
  `mask_clear`; `Timer_Int = ~(intff_out | DMCINT)`. Confirms `$40`/`$C0`, the
  mode-0-only interrupt and the level flag.
- `HDL/APU/regs.v:113` — `nor(W4017, PHI1, ~pla[27])` (PHI2 write gating);
  `HDL/APU/common.v` `RegisterBit` (no reset port, transparent while `ena`).
- `HDL/APU/clkgen.v:191–211` — `F1`/`F2` from `PLA_in[3]`/`PLA_in[4]` and
  `nACLK2`; `z_ff` reset by `W4017`; `SoftCLK_LFSR_Bit` loads `1` on `F1`.
- `HDL/APU/dpcm.v` `DPCM_IntControl` — DMC flag reset by `W4015`, `n_IRQEN`,
  `RES` (not by `n_R4015`), `DB[7]` driven on a `$4015` read.
- `HDL/APU/dac.v` — `AUX_A = {SQB,SQA}`, `AUX_B = {DMC,RND,TRI}`; matches §1.
- `BreakingNESWiki/APU/clkgen.md:154–170` — NTSC/PAL `nLFO1`/`nLFO2`
  mode4/mode5 PLA activation tables (7457×4; mode-5 7457,7457,7457,14913; PAL
  8313×4 and 8313,8313,8313,16625; `nLFO2` mode-5 PLA4 = 22367) and the
  floating-last-output caveat; `:94–97` LFO1/LFO2 Mode=0/Mode=1 rates;
  `:45–52` ACLK non-overlap and 21477272/26601712 Hz.
- `BreakingNESWiki/APU/dac.md`, `pads.md`, `pal.md` — star topology, 100 Ω
  board pull-down, 272 mV with 75 Ω and `SQA = SQB = 0xf` on RP2A03G, AUX B
  "TBD", AUX A pin 1 / AUX B pin 2, 2A07 clock/PLA/DPCM/noise/register-decoder/
  debug differences.
- `Scripts/auxa.mem` / `HDL/Framework/Icarus/apu/auxa.mem` — byte-identical;
  decoded all 256 big-endian floats: `(V/V_max − 0.5) × 2` for all 31 `g_A`
  groups, and **all 31** §4 rows (mV and normalized) match. The five corrected
  values re-derived independently: 16 → +0.1034, 22 → +0.4951, 25 → +0.6867,
  27 → +0.8129, 29 → +0.9379 (round-1 Minor 5 resolved).
- `Scripts/auxb.mem` — 32768 floats; exactly 218 distinct
  `4*TRI + 2*RND + DMC` groups; all 31 representative §5 rows (mV and
  normalized) match, including the −0.9991 … +1.0000 range.
- `Scripts/APU_DAC_Generator.py` — `IntRes = 39 kΩ`, `IntUnloaded = 999999 Ω`,
  `ExtRes = 100 Ω`, `Vdd = 5 V`, `tri_reciprocal = [2,4,8,16]`,
  `rnd_reciprocal = [1,2,4,8]`, `dmc_reciprocal = [0.5 … 32]`; confirms the §2
  unit note (the per-bit table sums to `g_B/2`, `g_B` is ×2, so
  `R_int = 78 kΩ / g_B`) — round-1 Minor 6 resolved.
- `HDL/Framework/Icarus/apu/clkgen.v`, `softclk.v`, `lfo.v` — no committed
  bench measures frame-counter intervals or `$4017` write latency; the
  round-1 Major 3 relabelling is honest.
- NESdev `APU_Frame_Counter` (Wayback 20250113123532) — `$4017` `MI--.----`,
  bit 7 mode / bit 6 inhibit, "After 3 or 4 CPU clock cycles, the timer is
  reset", mode-0 rows 3728.5 / 7456.5 / 11185.5 / 14914 / 14914.5 / 0 (14915)
  APU and "every 29830 CPU cycles", mode-1 rows through 18640.5 / 20782.5 APU
  with no interrupt.
- NESdev `APU_Mixer` (Wayback 20241229000148) — 31-entry pulse table,
  203-entry `tnd_table` indexed `3*triangle + 2*noise + dmc`, "within 4%",
  filters 90 Hz / 440 Hz / 14 kHz (NES) and 37 Hz (Famicom).
- Structure: both files carry exactly the five fixed headings
  (`## Observable phenomena`, `## Programming / implementation notes`,
  `## NESdev Wiki correspondence`, `## Provenance`, `## Open questions`),
  unnumbered and in order, a `Status` line, and are within 120–400 lines
  (273 / 253). Normative totals 29828 / 33252 / 37284 / 41564 and the
  4 : 2 : 1 weighting are unchanged; `APU_Mixer` (singular) is kept.

## Notes for the next round

- Round-1 Blocking 1, Majors 3 and 4 and all nine Minors are resolved as
  claimed; `03-apu-mixer.md` is unchanged in substance except the five
  corrected AUX A values and the §2 unit note, both verified independently.
- The only substantive opens are the two Major items above, both confined to
  `03-apu-frame-counter.md`'s NESdev reconciliation and its ad-hoc-run
  sentence. Neither changes the normative tables or any implementable rule.
- Do not "fix" Major 1 by adopting NESdev's totals: the repository's uniform
  7457/8313 intervals remain the stated normative basis (Conventions §3, the
  silicon wins) and already sum correctly to 29828 / 33252.
- Round-1's note about §9 (reset state is all ones via `F1`) was acted on at
  lines 185–188 and is correct.

---

# Review: apu-fc-mixer — round 1

**Files under review:** `Specs/03-apu-frame-counter.md` (258 lines),
`Specs/03-apu-mixer.md` (248 lines); both `Status: adapted (round 1)`.
This report is the only file written. No spec file was edited, nothing was
committed.

## Verdict

revise

## Blocking

1. **`03-apu-frame-counter.md` §8, line 158 — a `$4017` write with bit 7 clear
   does *not* "keep the current mode"; `$40` selects the 4-step sequence.**
   The sentence "Writing `%1000_0000` (`$80`) selects the 5-step sequence with
   the interrupt enabled; `%0100_0000` (`$40`) keeps the current mode and
   inhibits the interrupt" is false. `W4017` enables both register latches
   simultaneously for the whole byte, and the mode latch takes `DB[7]`:
   `HDL/APU/clkgen.v:148–149`
   `RegisterBit mode (.d(DB[7]), .ena(W4017), ...)` /
   `RegisterBit mask (.d(DB[6]), .ena(W4017), ...)`, enabled by
   `HDL/APU/regs.v:113` `nor(W4017, PHI1, ~pla[27])`. Writing `$40` therefore
   captures `DB[7] = 0` and puts the sequencer in 4-step mode; to inhibit the
   interrupt while staying in 5-step mode you must write `$C0`. The file
   contradicts itself: §2 line 48, §5 and §7 all say `$4017[7]` selects the
   mode. NESdev `APU_Frame_Counter` gives `$4017` as `MI--.----`, "Bit 7 M:
   Sequencer mode: 0 selects 4-step sequence, 1 selects 5-step", and NESdev
   `APU_registers` gives `$4017` as `SD-- ----`. An emulator implemented from
   this sentence would ignore the mode change on a `$40` write. Rewrite as
   "`%0100_0000` (`$40`) selects the 4-step sequence and inhibits the
   interrupt".

## Major

2. **`03-apu-frame-counter.md` — the file's own numbers do not add up to its
   normative 29828 total in two places.**
   - Open questions, lines 245–248: "the repository gives **29828** CPU cycles
     (four 7457-cycle PLA intervals; Icarus reproduces 7457/7456/7458/7458)".
     7457 + 7456 + 7458 + 7458 = **29829**, not 29828. Those four figures are
     in fact NESdev's cumulative step deltas (NESdev's step boundaries
     7457 / 14913 / 22371 / 29829 CPU cycles), so the parenthetical reproduces
     the NESdev total while claiming to corroborate the repository's. Either
     the interval list or the total is wrong.
   - §8, lines 168–172: "a mode-0 write at CPU cycle 29600 … moved the next
     frame IRQ to CPU cycle 59430 — one whole 29828-cycle sequence later".
     59430 − 29600 = **29830**; 29600 + 29828 = 59428. The example therefore
     implies the NESdev period, not the 29828 asserted in the same paragraph.
   The tables themselves are correct: 4 × 7457 = 29828 NTSC, 4 × 8313 = 33252
   PAL, 3 × 7457 + 14913 = 37284 NTSC, 3 × 8313 + 16625 = 41564 PAL. Only the
   prose sums are off; state whichever simulation values were actually
   measured and make them consistent with the 29828/33252 choice.

3. **`03-apu-frame-counter.md` §8 and Open questions — the Icarus evidence
   cannot be reproduced from the repository, and the `Provenance` row
   overstates the HDL source.** §8 (lines 163–172) reports "In the Icarus model,
   a write asserted at the start of PHI2 produced `F1` … exactly 3 CPU cycles
   later" and the 29600 → 59430 observation; Open questions (line 246) reports
   "Icarus reproduces 7457/7456/7458/7458". The only committed frame-counter
   testbenches are `HDL/Framework/Icarus/apu/clkgen.v`, which checks
   PHI0/PHI1/PHI2/ACLK1/nACLK2 transitions only (it never touches `$4017` or
   the LFSR), and `HDL/Framework/Icarus/apu/softclk.v`, whose own header says
   "This is not a unit test right now, we are just doing some preliminary
   SoftCLK fiddling". No log, `.run` or `.vcd` in the tree carries these
   results (the only text occurrence of 59430 is the spec itself). The
   `Provenance` row "Cycle counts (NTSC/PAL) and write latency | `APU/clkgen.md`
   | Icarus simulation of `clkgen.v` (see §8)" therefore attributes to HDL
   evidence that does not exist. The normative 3–4 CPU-cycle latency is safe
   because NESdev `APU_Frame_Counter` states it, but the sentence about
   "the Icarus model" MUST either cite a committed testbench/log or be relabelled
   as an ad-hoc local measurement (Conventions §7).

4. **`03-apu-frame-counter.md` §5 — the 5-step disagreement with NESdev is not
   recorded.** §5 uses the repository figures (last step 37284 CPU cycles NTSC,
   41564 PAL), but NESdev `APU_Frame_Counter`'s mode-1 table puts the last step
   at 18640.5 APU cycles = 37281 CPU cycles (NTSC) and 20782.5 = 41565 CPU
   cycles (PAL). The file gives NESdev's step-4 position (29829 / 33253) but
   never its step-5 total, and Open questions only carries the 4-step
   discrepancy. Conventions §3 requires a "> NESdev says X; this repository
   shows Y" note whenever the two disagree about behaviour; add the mode-1
   totals to the Open-questions bullet so both reconciliations are visible.

## Minor

5. **`03-apu-mixer.md` §4, lines 100/106/109/111/113 — five normalized AUX A
   entries are off by one in the 4th decimal.** The table states these are
   "the value stored in `HDL/Framework/Icarus/apu/auxa.mem`", and that file
   matches `(V/V_max − 0.5) × 2` exactly for all 256 entries (checked). The
   stored/model values round to: `g_A`=16 → 0.1034 (not 0.1035), 22 → 0.4951
   (not 0.4952), 25 → 0.6867 (not 0.6868), 27 → 0.8129 (not 0.8130),
   29 → 0.9379 (not 0.9380). All mV values and all AUX B normalized values
   match.
6. **`03-apu-mixer.md` §2, lines 40–52 — the per-bit table and the `g_B`
   collapse use different conductance units.** The table header says
   "Conductance weight (× 1/39 kΩ)" and its per-bit weights sum to
   2·TRI + RND + DMC/2, but the collapse is written
   `g_B = 4*TRI + 2*RND + DMC`, i.e. twice that (equivalently, the AUX B rows
   are in units of 1/78 kΩ). This matches `Scripts/APU_DAC_Generator.py:73–75`
   (`tri_reciprocal`, `rnd_reciprocal`, `dmc_reciprocal`) and the
   `R_int = 78 kΩ / g_B` formula is correct, so no number in the file is wrong;
   but an implementer who combines the per-bit table with `39 kΩ` gets a
   factor-of-2 conflict. State that `g_B` is scaled ×2 (or head the AUX B rows
   `× 1/78 kΩ`).
7. **`03-apu-frame-counter.md` §2, line 65 — "%0001_1111 enables all four" is
   a misleading example.** `$1F` sets bit 4 as well, which enables the DMC;
   the four length counters alone are `%0000_1111`. NESdev `APU_registers`
   gives the write field as `---D NT21` (bit 4 = DMC enable). Say
   "`%0000_1111` enables all four length counters; bit 4 additionally enables
   the DMC".
8. **`03-apu-frame-counter.md` §6, lines 140–141 — the two LFO rate pairs are
   mode 0 / mode 1, but everywhere else in the file "/" means NTSC / PAL.**
   "~240 Hz / ~192 Hz (LFO1) and ~120 Hz / ~96 Hz (LFO2)" comes from
   `APU/clkgen.md`'s mode table (Mode=0 | Mode=1) and is an NTSC figure.
   Name the modes and the region, otherwise §4/§5's "~240 Hz / ~200 Hz" reads
   as the same kind of pair.
9. **`03-apu-frame-counter.md` §5, line 114 — the 5-step "no activation" row
   mixes sources.** The PAL cumulative is "~33253", but the row is otherwise
   derived from the repository intervals and 24939 + 8313 = 33252 (the NTSC
   twin is 22371 + 7457 = 29828). 33253 is NESdev's step-4 value; mark the
   provenance of that one cell or use ~33252 for consistency.
10. **`03-apu-frame-counter.md` §5, lines 118–122 — the wiki's second,
    conflicting last-interval figure is not mentioned.** `APU/clkgen.md`'s
    NTSC table gives the mode-5 `nLFO1` PLA4 interval as 14913 (the spec's
    basis, total 37284) but the `nLFO2` row gives PLA4 = 22367 measured from
    PLA1, which implies 14913 + 22367 = 37280. That 4-cycle spread is exactly
    the "slightly floating … last output" caveat quoted in §4/§5, yet only the
    `nLFO1` row is transcribed. Add the `nLFO2` figure to the caveat.
11. **`03-apu-frame-counter.md` NESdev correspondence, lines 215–226 — no row
    covers §3 (sequencer structure).** §3 (15-bit LFSR, PLA outputs 0–5,
    anti-zero term, SET LFSR) has no NESdev counterpart row, which Conventions
    §7 requires for every content section; `APU_Frame_Counter` describes the
    observable sequence but not the LFSR. Add a row (even if the value is
    "internal, not described by NESdev").
12. **`03-apu-mixer.md` Open questions, line 242 — dangling cross-reference.**
    "The filter figures quoted in §10 come from NESdev" — the file has no §10;
    the NESdev filter chain is in *Programming / implementation notes*.
13. **`03-apu-mixer.md` §1/§8 — no NESdev correspondence row for the 2A07
    region differences.** §8 is the only content section with no entry in the
    correspondence table (`APU_Mixer` and `CPU_pinout` cover the rest);
    `Cycle_reference_chart`/`CPU_variants` would be the pages a reader would
    otherwise consult.

## Checked against

- `BreakingNESWiki/APU/clkgen.md` — SoftCLK purpose, `$4017[7:6]`, the LFO
  routing table (LFO1 → Square/Noise/Triangle, LFO2 → Square/Length), the PLA
  feature list (out 3 interrupt/mode-0 only, out 4 mode 1 only, out 5 anti-zero,
  activation order 0,1,2,3 / 0,1,2,4), the NTSC/PAL activation tables
  (7457×4 / 8313×4; mode-5 7457,7457,7457,14913 and 8313,8313,8313,16625) and
  the "floating last output" caveat; clock table 21477272/26601712 Hz,
  46.56/37.59 ns, 558.73/601.46 ns, 1117.46/1202.93 ns and the non-overlapping
  ACLK1 //ACLK2 statement.
- `HDL/APU/clkgen.v` — `ACLKGen` (PHI1/PHI2 latches), `SoftCLK_Control`
  (`R4015_clear = nor(n_R4015, PHI1)`, `mask`/`mode` latches, `int_ff` set =
  `n_mdout & PLA_in[3]`, reset inputs `RES`/`R4015_clear`/`mask_clear`,
  `Timer_Int = int_ff OR DMCINT`), `SoftCLK_LFSR_Control` (`z` flip-flop reset
  by `W4017`, `F1`/`F2` from PLA3/PLA4 + `nACLK2`, `sin` anti-zero term),
  `SoftCLK_LFSR_Bit` (`F1 ? 1'b1`, so the reset state is all ones),
  `SoftCLK_LFSR` (15 bits), `SoftCLK_PLA` (6 outputs, `md` term in PLA3), and
  `nLFO1`/`nLFO2` terms (quarter = PLA0–4, half = PLA1, PLA3, PLA4).
- `HDL/APU/regs.v` — `nor(W4017, PHI1, ~pla[27])`, `not(n_R4015, pla[4])`,
  `nor(W4015, PHI1, ~pla[15])`, PHI1 write gating.
- `HDL/APU/common.v` — `RegisterBit` is transparent while `ena` is high
  (`latch_in = ena ? d : (ACLK1 ? q : 1'bz)`), no reset port → mode/mask power
  up at 0.
- `HDL/APU/dpcm.v:186–202` — `DPCM_IntControl`: DMC flag reset by `W4015`,
  `n_IRQEN` (i.e. `$4010[7]` cleared) and `RES`; **not** by `n_R4015`;
  `DB[7]` driven during a `$4015` read.
- `HDL/APU/dac.v` — `DAC_Square` = `{SQB,SQA}` (8 bit), `DAC_Others` =
  `{DMC,RND,TRI}` (15 bit).
- `HDL/APU/apu_top.v`, `HDL/APU/pads.v` — `AUX_A[7:0]`, `AUX_B[14:0]`,
  `Timer_Int` = "Timer interrupt (combined with DMC interrupt)" → `n_IRQ_tocore`.
- `HDL/Framework/Icarus/apu/clkgen.v`, `softclk.v` — the only committed
  clkgen/SoftCLK testbenches; neither measures frame-counter intervals or write
  latency.
- `Scripts/APU_DAC_Generator.py` — `IntRes = 39 kΩ`, `IntUnloaded = 999999 Ω`,
  `ExtRes = 100 Ω`, `Vdd = 5 V`, `stage_reciprocal = [1,2,4,8]`,
  `tri_reciprocal = [2,4,8,16]`, `rnd_reciprocal = [1,2,4,8]`,
  `dmc_reciprocal = [0.5,1,2,4,8,16,32]`, `gain = 2`, normalization
  `(V/V_max − 0.5) × 2`; reproduced all AUX A (31) and AUX B (218) values.
- `Scripts/auxa.mem`, `Scripts/auxb.mem`, `HDL/Framework/Icarus/apu/auxa.mem`,
  `auxb.mem`, `auxout.v` — 256/32768 entries, byte-identical between the two
  copies; `auxa` is constant for every `SQA+SQB` (31 distinct values) and
  `auxb` for every `4*TRI+2*RND+DMC` (218 distinct values); NESdev's 3:2:1
  index collides (203 distinct values), confirming the file's Open question.
- `Scripts/Readme.md` — "A simplified FET model with linear resistance is used.
  Actually there is some tricky formula with cubic relationship to input
  voltage".
- `BreakingNESWiki/APU/dac.md` — star-topology VDD/GND, AUX pads unloaded
  internally, on-board pull-down "usually 100 Ω", 272 mV / 75 Ω measurement with
  `SQA = SQB = 0xF` (model gives 272.7 mV), triangle/noise/DPCM network "TBD".
- `BreakingNESWiki/APU/pads.md` — AUX A pin 1, AUX B pin 2, `/RES` pin 3, CLK
  pin 29; **no `/AUX` net anywhere** (confirms the mixer's Open question).
- `BreakingNESWiki/APU/regs.md`, `.../pal.md` — `SQA/SQB/TRI/RND/DMC` bus
  widths, SoftCLK PLA differs on 2A07, debug registers absent on 2A07.
- NESdev (local cache): `APU_Frame_Counter` (4/5-step tables, 29830/33254,
  3–4-cycle write latency, inhibit behaviour), `APU_registers` (`$4015`
  `IF-D NT21` / `---D NT21`, `$4017` `SD-- ----`), `APU_Mixer` (exact formula,
  31-entry pulse table, 203-entry `tnd_table` indexed `3*triangle + 2*noise +
  dmc`, "within 4%", filter chain 90 Hz/440 Hz/14 kHz NES, 37 Hz Famicom),
  `CPU_pinout` (`AD1` = "Audio out pin (both pulse waves)", `AD2` = "Audio out
  pin (triangle, noise, and DPCM)", `/RST`, CLK ÷12 / ÷16),
  `Cycle_reference_chart` (confirmed it is the target of the `Clock_rate`
  redirect, as the file states), `CPU_power_up_state`.
- Structure: both files carry the exact five fixed headings in order with no
  number prefix, a `Status` line, and are within 120–400 lines (258 / 248).

## Notes for the next round

- The frame counter's substantive tables are sound. Every interval sums to the
  repository totals, the PLA activation order and the "output 3 = interrupt,
  mode 0 only" rule match `SoftCLK_PLA`/`SoftCLK_Control`, the `$4015`/`$4017`
  bit diagrams match NESdev `APU_registers`, the read/write asymmetry between
  the frame and DMC interrupt flags matches `DPCM_IntControl`, and the
  quarter/half routing matches `env.v`, `triangle.v`, `square.v`, `length.v`
  and `clkgen.v`.
- The 4-step blockquote (lines 103–105) is honest and correct: NESdev
  `APU_Frame_Counter` really does say 29830 / 33254, and the repository PLA
  really does sum to 29828 / 33252. The normative choice of the repository
  figures follows Conventions §3 ("the silicon wins").
- §9 line 180–181, "the shift register self-starts through the anti-zero PLA
  output", is imprecise: `F1` loads every bit with `1`
  (`SoftCLK_LFSR_Bit`, `F1 ? 1'b1`), so the reset state is all ones and the
  anti-zero term only prevents the register locking at all zeros after the
  first shifts. The observable claim (first activation ~7457 CPU cycles after
  the sequence starts) is unaffected; reword if touched in round 2.
- Mixer numbers are otherwise exact: the 218-entry `g_B` table, the 4:2:1 /
  3:2:1 blockquote, the 272.7 mV cross-check for 75 Ω, the −0.9972…+1.0000 /
  −0.9991…+1.0000 normalized ranges, and the pin naming (AUX A/`AD1`,
  AUX B/`AD2`, no `/AUX`) all verify.
- Do not "fix" the Open-questions 29828-vs-29830 entry by adopting NESdev's
  totals: the repository's uniform 7457 intervals are the stated normative
  basis and sum correctly to 29828. Only the two prose sums and the
  Icarus-attribution wording need correction.

## Revision round 2

- Blocking 1: fixed — `03-apu-frame-counter.md` §8 now says `$40` selects the 4-step sequence and
  inhibits the interrupt, and adds `$C0` for "inhibit while staying in 5-step". Cites `clkgen.v`
  (`mode` ← `DB[7]`, `mask` ← `DB[6]`, both enabled by `W4017`) and agrees with the file's §2.
- Major 2: fixed — the Open-questions bullet no longer sums NESdev's deltas as if they corroborated
  the repository; it now lists NESdev's cumulative boundaries 7457 / 14913 / 22371 / 29829 (deltas
  7457, 7456, 7458, 7458). The §8 example is now "29600 → 59428", consistent with 29828. The
  normative tables (29828 / 33252 / 37284 / 41564) are unchanged.
- Major 3: fixed by relabelling — the `Provenance` row no longer attributes interval/write-latency
  figures to an Icarus simulation of `clkgen.v`. §8 marks the 3-cycle and 29600 → 59428 observations
  as an ad-hoc, uncommitted simulation; the new Open-questions bullet states that
  `HDL/Framework/Icarus/apu/clkgen.v` only scans clock-tree transitions and `softclk.v` says it "is
  not a unit test right now", so no committed bench reproduces them. The NESdev 3–4-cycle range is
  retained. No evidence was fabricated.
- Major 4: fixed — the combined Open-questions bullet now records NESdev's mode-1 totals
  (18640.5 / 20782.5 APU cycles = 37281 NTSC / 41565 PAL) next to the 4-step 29830/33254 note, with
  the repository's 37284 / 41564 stated as normative (per adjudication A7).
- Minor 5: fixed — `03-apu-mixer.md` §4 normalized values corrected to 0.1034 / 0.4951 / 0.6867 /
  0.8129 / 0.9379 (decoded from `Scripts/auxa.mem`; the file was the authority, not the spec).
- Minor 6: fixed — `03-apu-mixer.md` §2 adds a unit note: the per-bit table is in AUX A's `1/39 kΩ`
  units while `g_B` is scaled ×2 for `R_int = 78 kΩ / g_B`.
- Minor 7: fixed — `%0000_1111` is now the length-counter example, with bit 4 = DMC named separately.
- Minor 8: fixed — §6 names the LFO rate pair as mode 0 / mode 1 (NTSC), not NTSC / PAL.
- Minor 9: fixed — the 5-step "no activation" PAL cell is now `~33252`.
- Minor 10: fixed — §5's floating-last-output caveat now also cites the `nLFO2` PLA4 = 22367 row.
- Minor 11: fixed — NESdev correspondence gains a `§3` sequencer-structure row.
- Minor 12: fixed — mixer Open questions now points to *Programming / implementation notes*, not `§10`.
- Minor 13: fixed — mixer correspondence gains a 2A07 row (`CPU_variants`, `Cycle_rate` redirect).
- Not changed: the 4:2:1 (repository) vs 3:2:1 (NESdev) adjudication and the `APU_Mixer` spelling.
- Also reworded §9 (report "Notes"): the reset state is all ones (`F1` loads `1`); the anti-zero
  output only prevents a lock at all zeros — the observable first-activation claim is unchanged.

## Revision round 3

- Major 1: fixed — `03-apu-frame-counter.md` Open questions now gives NESdev's mode-0 step boundaries
  as 7457 / 14913 / 22371 / **29828** CPU (deltas 7457, 7456, 7458, 7457), an extra quarter+half clock
  at 29829 and the counter reset at 29830, and states that 29829 is NESdev's **mode-1** step-4 time
  (14914.5 APU = 29829) while 29830 is its reset/period figure. Evidence: `BreakingNESWiki/APU/clkgen.md`
  and the cached NESdev `APU_Frame_Counter` (mode-0 rows 3728.5 / 7456.5 / 11185.5 / 14914 / 14914.5 /
  0 (14915) APU; mode-1 step 4 = 14914.5; "the interrupt flag is set every 29830 CPU cycles"). The
  repository totals 29828 / 33252 / 37284 / 41564 remain normative and unchanged.
- Major 2: fixed by dropping the figures — §8 no longer carries the "29600 → 59428" ad-hoc run. It now
  states that the restart lands 3–4 CPU cycles after the write and that the next step-4 IRQ follows one
  full mode-0 period later, counted from that restart, so the example cannot contradict the stated
  latency; no concrete cycle offset is quoted because the run is uncommitted, unreproducible and had
  two different recorded values across rounds.
- Minor 1: fixed — the Provenance row now calls the `clkgen.md` PLA interval tables rank 1 and confines
  rank 4 to the write latency and the unreproducible ad-hoc interval run.
- Minor 2: fixed — §5's floating-last-output paragraph now also cites the mode-4 `nLFO2` case
  (PLA1 14913, then PLA3 14913 → **29826** for the mode-0 sequence), and the §5 "no activation" cell
  drops the `~` from the exact repository cumulatives (29828 / 14914 and 33252 / 16626).
- Not changed: the normative tables and totals, the 4 : 2 : 1 weighting, the `APU_Mixer` spelling and
  `03-apu-mixer.md`. Only `Specs/03-apu-frame-counter.md` and this report were written; no commit.
