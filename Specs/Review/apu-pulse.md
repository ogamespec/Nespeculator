# Review: apu-pulse — round 1

## Verdict

**consensus**

No blocking findings. No major findings. The brief's five suspected defects were all
checked against the primary sources and the HDL, and all five are correctly handled in
the two files. The small findings below are wording/traceability issues that a revision
round may fold in; none of them changes a number an implementer would act on.

## Blocking

None.

## Major

None.

The brief's verification targets, with the evidence used:

1. **Sweep enable polarity — correct.** `03-apu-pulse.md` §2 (bit 7 diagram and the
   "0: sweep process disabled, 1: sweep enabled (bit 7, SWDIS…)" row), §5 table and
   §"Programming / implementation notes" all state bit 7 = 1 enables. `HDL/APU/square.v:275`
   `RegisterBit swdis_reg (… .d(DB[7]) .nq(SWDIS))`; `HDL/Common/common.v` RegisterBit
   drives `q = ~ntq` and `nq = ntq` from the same latch, so `q = d` (bit 7) and
   `SWDIS = ~bit 7`. `HDL/APU/apu_top.v:285` `DO_SWEEP` is `RegisterBit`-fed from
   `WR1 = W4001`. Re-checked on Icarus: with `$4001 = $F1` (bit 7 = 1) `SWDIS` goes
   `1 → 0` at the register write and `DO_SWEEP` pulses; with `$4001 = $71` (bit 7 = 0)
   `SWDIS` stays 1 and `DO_SWEEP` never asserts. The wiki's `WR1[3]` in
   `BreakingNESWiki/APU/square.md:97` is therefore indeed a bit-index typo, and the file
   records that in Open questions. The repo's own sweep test (`apuwaves.md:135`,
   `HDL/Framework/Icarus/apu/square_sweep.v:106`) also writes bit 7 = 1 for "Sweep=1",
   supporting the interpretation. **No change needed.**
2. **Pulse mute threshold — correct and explicit.** `03-apu-pulse.md` §7 keeps the
   schematic value and states the NESdev difference in a block quote (§7 lines
   "> NESdev (`APU_Pulse` …) silences a pulse channel when the timer period is `t < 8`"),
   repeats it in Observable phenomena row 7, in the mute-condition table (`SW_UVF` =
   timer period `t < 4`, frequency-register bits 10:2 all zero) and in Open questions.
   HDL: `HDL/APU/square.v:196` `nor (SW_UVF, Fx[2] … Fx[10])` — mute exactly when
   `Fx < 4`; Icarus confirmed `SW_UVF = 1` for `Fx = 0,1,2,3` and `0` from `Fx = 4`.
   NESdev `APU_Pulse` ("A period of t < 8 … silences the corresponding pulse channel")
   and `APU_Sweep` ("If the current period is less than 8 …") confirm the community
   value. `03-apu-triangle.md` has no pulse-mute claim, so there is no cross-file
   contradiction. **No change needed.**
3. **Length reload = community table − 1, both rows visible — correct.** Both files
   print the `reload` and `length` rows (pulse §6, triangle §5) and state
   `length = reload + 1` verbatim. I re-ran the PLA (`HDL/APU/length.v`
   `LengthCounter_PLA`/`LengthDecoder1`/`LengthDecoder2`) on Icarus and dumped all 32
   entries: 9, FD, 13, 01, 27, 03, 4F, 05, 9F, 07, 3B, 09, 0D, 0B, 19, 0D, 0B, 0F,
   17, 11, 2F, 13, 5F, 15, BF, 17, 47, 19, 0F, 1B, 1F, 1D — identical to both tables.
   Those are exactly the NESdev `APU_Length_Counter` table entries minus one; the
   page's "Length counter internals" section says the published values are "the actual
   values the length counter gets loaded with plus one". **No change needed.**
4. **Triangle linear counter — correct.** `03-apu-triangle.md` §2, §4 and §5: reload
   flag set by `$400B` (not `$4008`), cleared at a quarter-frame clock only while the
   control flag (`$4008` bit 7) is clear, control flag set and cleared by `$4008`
   writes, and the three ordered actions (reload-and-do-not-decrement / decrement /
   clear reload flag). HDL `HDL/APU/triangle.v:81`
   `rsff Reload_FF (.r(~(reload_latch1_q | lc_reg_q | n_LFO1)), .s(W400B), …)` — set by
   the `$400B` write strobe, cleared only when `lc_reg_q` (= `$4008` bit 7) is clear and
   `n_LFO1` is inactive. `TLOAD` is `n_LFO1`-gated (`triangle.v:87`), i.e. exactly one
   reload per quarter frame. `$4008` does not touch `Reload_FF`. **No change needed.**
5. **Timer step rates — correct.** Pulse `03-apu-pulse.md` §3: "stepped once per APU
   cycle (2 CPU cycles)", `2*(t+1)` CPU cycles per period. HDL `HDL/APU/square.v:232`
   `DownCounterBit freq_cnt [… .ACLK1(ACLK1) …]` clocked by ACLK1 (the APU clock).
   Triangle `03-apu-triangle.md` §3: "stepped once per **CPU cycle**", `t + 1` CPU
   cycles, `f = fCPU/(32*(t+1))`. HDL `HDL/APU/triangle.v:127`
   `DownCounterBit freq_cnt [… .ACLK1(PHI1) …]` — the CPU phase, with the developers'
   comment at `triangle.v:145` explaining the deliberate choice. Matches NESdev
   `APU_Pulse` ("updated every APU cycle (i.e., every second CPU cycle)") and
   `APU_Triangle` ("this timer ticks at the rate of the CPU clock rather than the APU
   (CPU/2) clock"). Numeric spot checks all reproduce: pulse t=0 → 111.86 kHz /
   103.91 kHz, t=2047 → 54.6 Hz / 50.7 Hz; triangle t=0 → 55.93 kHz, t=511 → 109.2 Hz;
   triangle `$400A = $03` → a timer step every 4 CPU cycles (matches the file's own
   Icarus note).
6. **Structure, headings, length — compliant.** Both files carry
   `**Status:** adapted (round 1)`; the five fixed headings are present in order,
   spelled and unnumbered exactly as `Specs/Conventions.md` §6 requires; pulse is 319
   lines and triangle 301 lines (target 120–400). Observable phenomena, NESdev
   correspondence, Provenance and Open questions are all populated, and the
   correspondence table has a row for every content section.

## Minor

1. `03-apu-triangle.md` §6: "`LOCK` (asserted by the 2A03 test readout of `$4019`) stops
   the output counter". LOCK is **written**, not read: `HDL/APU/test.v` holds it in
   `sdffre LOCK_FF (.d(DB[7]), .en(W401A), …)`, i.e. it is set by `$401A` bit 7 (write),
   and the readout of `$4019` merely observes the frozen channels. NESdev
   `CPU_Test_Mode` states it exactly: "W$401A: [L..T TTTT] — set state of triangle's
   sequencer to T, and lock all channels if L=1". The same page is the right citation
   for the `$4018`–`$401A`/LOCK material in **both** files, but neither lists it.
   Consequence: the "how to pause the channel" recipe would be wrong if followed
   literally. Fix: say `LOCK` is set by a `$401A` bit-7 write, and add `CPU_Test_Mode`
   to the correspondence tables (`CPU_pinout`, cited instead, only mentions that the
   diagnostic registers exist).
2. `03-apu-pulse.md` §6: "decrements only while the channel's input carry is 1 (halt
   flag clear) **and the channel is enabled through `$4015`**". Only the halt flag gates
   the decrement; `$4015` gates the *output* (and forces `NotCount`), and the section
   itself later says correctly that clearing a `$4015` bit "forces `NotCount`" and that
   setting it has no counter effect. As written, an implementer could stop the counter
   on disable, which would break the re-enable-then-`$4003` behaviour described in
   phenomena 1 and 10. Suggested wording: "the counter decrements while the halt flag
   and the internal state flip-flop allow it; `$4015` only gates the output".
3. `03-apu-pulse.md` §3 duty table, `d = 0` (12.5 %) row reads
   `counter 7 6 5 4 3 2 1 0 → 1 0 0 0 0 0 0 0`. The HDL gives the opposite polarity on
   the first bit: `SQUARE_Duty` (`square.v:309-314`) at `DC = 0` (the value loaded by
   `$4003`, `square.v:307`) produces `in[0] = 0`, and reading the NESdev lookup table
   `00000001` in the stated order 0, 7, 6, 5, 4, 3, 2, 1 gives `0,1,0,0,0,0,0,0`. The
   `d = 1`, `d = 2`, `d = 3` rows reproduce correctly from the HDL. The duty
   percentages are unaffected; only the phase of one row's first bit (and hence the
   first sample after a `$4003` write — waveform period 2 APU cycles) is wrong, and the
   file already warns that "NESdev lists them at a different phase".
4. `03-apu-pulse.md` §4: "PAL uses 8313 cycles in 4-step mode and **16625** cycles for
   the long 5-step interval". `BreakingNESWiki/APU/clkgen.md:168` gives the PAL 5-step
   `/LFO1` mode-5 PLA4 interval as **14913**; 16625 is the PAL `/LFO2` (half-frame)
   value. The quarter frame is `/LFO1`, so the long 5-step quarter interval is 14913
   CPU cycles. (The 4-step 8313 figure is correct.)
5. `03-apu-pulse.md` NESdev correspondence, last row: the mixer page is listed as
   `APU_Mixer`. The NESdev wiki page title is `APU_Mixers` (the cache holds
   `APU_Mixers.html` plus a redirect `APU_Mixer.html`). Same for
   `03-apu-triangle.md`'s last row.
6. `03-apu-pulse.md` correspondence table lists "CPU clock rates (NTSC 2A03, PAL 2A07,
   **Dendy**)" but cites only `APU_Pulse`, `Clock_rate`. The Dendy value 1.773448 MHz
   is asserted in Open questions from `Clock_rate`; naming the page in the table (as the
   triangle file does) would close the traceability loop.

## Checked against

- `BreakingNESWiki/APU/square.md` — sweep signal table (`SWDIS` = register value, `WR1[3]`
  typo noted), the seven "Sweep does NOT happen" conditions, `SW_UVF` = frequency < 4,
  `SW_OVF` = INC and adder carry, DO_SWEEP during PHI1 only, negate direction table,
  `SQUARE0`/`SQUARE1` carry-in difference, DUTY table, envelope unit identity.
- `BreakingNESWiki/APU/triangle.md` — PHI1 usage, 7-bit linear down counter, 11-bit
  frequency down counter, 5-bit up counter with the MSB as saw direction, TTSTEP.
- `BreakingNESWiki/APU/length.md` — decoder input→output table (all 32 entries),
  per-channel carry/load signals (`SQA_LC`/`W4003`, `TRI_LC`/`W400B`, …).
- `BreakingNESWiki/APU/waves.md` — register test vectors (square sweep/Sweep=1 bit 7,
  triangle `$4008`/`$400A`/`$400B` writes) and the accelerated-LFO test caveat.
- `BreakingNESWiki/APU/clkgen.md` — NTSC 7457 / 5-step 14913, PAL 8313 / 5-step 14913,
  LFO1 ≈ 240/192 Hz, LFO2 ≈ 120/96 Hz.
- `BreakingNESWiki/APU/regs.md` — `W4001`/`W4005`, `W400B`, `W4015`, `W401A` strobes;
  `$4018`–`$401A` readout routing; LOCK definition.
- `HDL/APU/square.v` — `SQUARE_Sweep` (SWDIS/DO_SWEEP/`SCO`/`SW_OVF`/`SW_UVF`), `SQUARE_Adder`
  (`n_cin`, `SW_UVF`), `SQUARE_BarrelShifter` (arithmetic shift via `BS[11] = DEC`),
  `SQUARE_Duty`, `SQUARE_Output` (`pnor` gate), `SquareChan` port order.
- `HDL/APU/apu_top.v` — `AdderCarryMode(1'b1)` for SquareA and `1'b0` for SquareB;
  `LengthCounters` wiring (`W4003/W4007/W400B/W400F`, `{DB[3],DB[2],DB[1],DB[0]}`);
  `DebugLock` into all four channels.
- `HDL/APU/triangle.v` — `TRIANGLE_Control` Reload_FF/TLOAD/TSTEP/TTSTEP,
  `TRIANGLE_LinearCounter` (7 bits), `TRIANGLE_FreqCounter` (PHI1 clock),
  `TRIANGLE_Output` (`TRI_Out = ~(T[4] ? nT[3:0] : T[3:0])`).
- `HDL/APU/length.v` — `LengthCounter_PLA`, `LC_Control` state flip-flop and
  `NotCount = ~StatOut`, `LC_DownCounter` (8 bits).
- `HDL/APU/env.v` — `EnvReload` rsff set by `WR_LC`, `RLOAD`/`RSTEP` on `n_LFO1`,
  `EIN = eco_latch_q & LC`, loop reload to 15.
- `HDL/APU/test.v` — `LOCK_FF (.d(DB[7]), .en(W401A))`; debug `bustris` readouts for
  `$4018`/`$4019`/`$401A`.
- `HDL/Common/common.v`, `HDL/Common/dlatch.v` — `RegisterBit` `q`/`nq` relationship
  (so `nq(SWDIS)` is the inversion of bit 7) and latch transparency/initial state.
- Icarus 12.0 experimentation (temporary benches, since removed): PLA dump of all 32
  length entries; `SWDIS`/`DO_SWEEP` with `$4001 = $F1` vs `$71` (VCD: `SWDIS` 1→0 at
  the write, 3 `DO_SWEEP` pulses vs none); `SW_UVF` for `Fx = 0…8`; sweep divider
  periodicity (P=0 → pulse every half frame; P=7 → every 8 half frames, measured
  interval ×120 Hz = 8); negate targets `Fx = 20, S = 1` → P1 = 9, P2 = 10, increase = 30;
  `SW_OVF = 1` at `Fx = $7FF, S = 1`, increase.
- NESdev cache `/tmp/nesdev-cache/`: `APU_Pulse` (t < 8 mute, duty sequencer order and
  lookup tables, `$4003` side effects), `APU_Sweep` (E = bit 7 enabled flag, divider
  P+1, one's/two's-complement negate difference, both mute conditions, reload flag),
  `APU_Triangle` (CPU-rate timer, `f = fCPU/(32*(tval+1))`, ordered linear-counter
  actions, `$80`/`$FF` pause recipe, 32-step sequence), `APU_Length_Counter` (table and
  "loaded with plus one" internals), `APU_Envelope` (start flag, V+1 divider, loop),
  `CPU_Test_Mode` (W`$401A` LOCK bit), `CPU_pinout` (test-register existence only).

## Notes for the next round

- Nothing here requires a revision round for consensus. If the section is revised for
  any other reason, fold in Minor 1 (LOCK = `$401A` write, add `CPU_Test_Mode`), Minor 2
  (`$4015` gates the output, not the decrement), Minor 3 (d = 0 first duty bit),
  Minor 4 (PAL 5-step `/LFO1` = 14913) and Minor 5 (`APU_Mixers`).
- The two documented silicon-vs-NESdev divergences are now consistent between the two
  files and are each recorded in Open questions with the evidence: the pulse mute
  threshold (`t < 4` schematic vs `t < 8` NESdev) and the sweep enable polarity
  (HDL bit 7 active-high vs the wiki's `WR1[3]` typo). An implementer following the
  files gets `SWDIS = ~$4001[7]` and a `t < 4` mute, with the community alternative
  named. That satisfies `Specs/Conventions.md` §3 conflict resolution.
- Both files repeat the same length-counter block; they agree word-for-word on the
  `reload`/`length` relationship and on the PLA values, so a future edit to the table
  must be applied to both. Worth a one-line cross-reference if the ownership of that
  table moves.
- Do not treat the six Minor items as a reason to hold the section: none of them
  changes a timer period, a bit position that the source confirms, or a table value
  used for pitch/duration.
