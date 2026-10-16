# APU — Frame counter ($4017)

**Status:** consensus (round 3)
**Scope:** The APU frame counter (Breaking NES Wiki: `SoftCLK`, "programmable timer"): its clock
source, the 4-step and 5-step sequences with NTSC/PAL cycle counts, what each step clocks, the frame
interrupt, and the `$4017` / `$4015` interface. Envelope, sweep and length-counter internals belong
to the channel specs; the DMC interrupt is covered by `03-apu-dmc.md`.
**Primary sources:** `BreakingNESWiki/APU/clkgen.md`, `.../waves.md`, `.../cycles.md`, `.../regs.md`,
`.../common.md`, `.../length.md`, `.../square.md`, `.../triangle.md`, `.../noise.md`, `.../dpcm.md`,
`.../pal.md`, `.../pads.md`, `.../visual2a03.md`; `HDL/APU/clkgen.v`, `HDL/APU/regs.v`,
`HDL/APU/apu_top.v`, `HDL/APU/common.v`, `HDL/APU/env.v`, `HDL/APU/square.v`, `HDL/APU/triangle.v`,
`HDL/APU/noise.v`, `HDL/APU/length.v`, `HDL/APU/dpcm.v`.

## 1. Role and clock source

The frame counter (Breaking NES Wiki: `SoftCLK`, "programmable timer") lives in the timing generator
block of the CPU (2A03/2A07). It provides:

- a periodic interrupt (mode 0 only), once per sequence;
- two low-frequency clocks for the sound generators, `/LFO1` (quarter frame) and `/LFO2` (half frame);
- the daisy-chain point where the DMC interrupt (`DMCINT`) is OR-ed with the frame interrupt into the
  single internal `INT` line that drives `/IRQ` (`HDL/APU/clkgen.v`, `SoftCLK_Control`).

| Signal | Visual 2A03 name | Rate | Target |
|---|---|---|---|
| `/LFO1` | `frm_quarter` | 240 Hz (NTSC, mode 0) | pulse envelopes, noise envelope, triangle linear counter |
| `/LFO2` | `frm_half` | 120 Hz (NTSC, mode 0) | four length counters, two pulse sweep units |

Clock chain (`clkgen.md`, `waves.md`, `ACLKGen`):

| Signal | NTSC 2A03 | PAL 2A07 | Relation |
|---|---|---|---|
| master CLK | 21.477272 MHz (46.56 ns) | 26.601712 MHz (37.59 ns) | chip input, pin 29 |
| PHI0 / CPU cycle | ~558.73 ns | ~601.46 ns | NTSC CLK ÷ 12, PAL CLK ÷ 16; also the unit of every cycle count below |
| APU cycle (ACLK) | ~1117.46 ns | ~1202.93 ns | PHI ÷ 2, i.e. **2 CPU cycles** |

`ACLK1` and `/ACLK2` are non-overlapping (not complementary). The sequencer is stepped once per APU
cycle, i.e. every 2 CPU cycles. A CPU cycle is one PHI0 period; an APU cycle is 2 CPU cycles.

## 2. Programming interface

`$4017` write (write-only; the register selector drives `W4017` only during PHI2):

```
7  bit  0
---- ----
MI-- ----
M = mode: 0 = 4-step sequence, 1 = 5-step sequence
I = interrupt inhibit: 1 = clear the frame interrupt flag and prevent it being set
```

`$4015` read status (bits driven by three blocks; bit 5 is not driven by the APU):

```
7  bit  0
---- ----
IF-D NT21

I = DMC interrupt flag ($4015[7])       N = noise length counter active
F = frame interrupt flag ($4015[6])     T = triangle length counter active
- = not driven (open bus)               2 = pulse 2 length counter active
D = DMC sample active                  1 = pulse 1 length counter active
```

- `$4015` write: `%0000_1111` enables all four length counters; bit 4 additionally enables the DMC.
- Frame IRQ flag (`$4015[6]`): set by the frame counter, cleared by **reading `$4015`** or by writing
  `$4017` with bit 6 = 1 (`SoftCLK_Control`: `int_ff` reset inputs `R4015_clear`, `mask_clear`, `RES`).
- DMC IRQ flag (`$4015[7]`): a different flip-flop, cleared by a `$4015` **write** or by clearing
  `$4010[7]` (`HDL/APU/dpcm.v`, `DPCM_IntControl`) — not by reading `$4015`.
- `Timer_Int = int_ff OR DMCINT`; the CPU's `/IRQ` is the logical OR of both sources.

## 3. Sequencer structure

The sequencer is a 15-bit shift register (LFSR) followed by a PLA (`SoftCLK_LFSR`, `SoftCLK_PLA`):

- The shift register is clocked by `F2` (one step per APU cycle) and reset by `F1`.
- The PLA decodes the shift-register state into 6 outputs. Output 5 is the anti-zero term: it forces a
  `1` into the shift-in while the register holds all zeros, so the all-zero state cannot persist.
- Outputs 0, 1, 2 and 3 (mode 0) or 0, 1, 2 and 4 (mode 1) are activated in turn; output 3 is the
  interrupt output and is not used in mode 1, so **the frame interrupt exists only in mode 0**.
- The last activation of a sequence (PLA output 3 in mode 0, output 4 in mode 1) is marked
  *SET LFSR*: it resets the shift register and restarts the sequence.

## 4. 4-step sequence (mode 0, `$4017[7]` = 0)

Intervals are the number of CPU cycles (PHI1 cycles) between successive PLA activations, measured by
the repository (`clkgen.md`); cumulative columns are the sum from the start of the sequence.

| Step | PLA out | Act. | NTSC interval (CPU) | NTSC cumulative (CPU / APU) | PAL interval (CPU) | PAL cumulative (CPU / APU) | Quarter frame | Half frame | Frame IRQ |
|---|---|---|---|---|---|---|---|---|---|
| 1 | PLA0 | 0 | 7457 | 7457 / 3728.5 | 8313 | 8313 / 4156.5 | yes | — | — |
| 2 | PLA1 | 1 | 7457 | 14914 / 7457 | 8313 | 16626 / 8313 | yes | yes | — |
| 3 | PLA2 | 2 | 7457 | 22371 / 11185.5 | 8313 | 24939 / 12469.5 | yes | — | — |
| 4 | PLA3 | 3 | 7457 | 29828 / 14914 | 8313 | 33252 / 16626 | yes | yes | set |
| restart | — | — | — | 29828 / 14914 (next step 1) | — | 33252 / 16626 | — | — | — |

Rates (NTSC / PAL): quarter frame ~240 Hz / ~200 Hz, half frame ~120 Hz / ~100 Hz, frame interrupt
~60 Hz / ~50 Hz. PAL interval counts are 8313 CPU cycles per step; note that the last interval is not
exactly equal to the others ("the table does not take into account the slightly floating number of
cycles for the last output", `clkgen.md`).

> NESdev says the 4-step sequence sets the interrupt every 29830 CPU cycles (NTSC) and every 33254
> CPU cycles (PAL); this repository's PLA intervals sum to **29828** and **33252**. This spec uses the
> repository figures; the difference is kept in Open questions.

## 5. 5-step sequence (mode 1, `$4017[7]` = 1)

| Step | PLA out | Act. | NTSC interval (CPU) | NTSC cumulative (CPU / APU) | PAL interval (CPU) | PAL cumulative (CPU / APU) | Quarter frame | Half frame | Frame IRQ |
|---|---|---|---|---|---|---|---|---|---|
| 1 | PLA0 | 0 | 7457 | 7457 / 3728.5 | 8313 | 8313 / 4156.5 | yes | — | — |
| 2 | PLA1 | 1 | 7457 | 14914 / 7457 | 8313 | 16626 / 8313 | yes | yes | — |
| 3 | PLA2 | 2 | 7457 | 22371 / 11185.5 | 8313 | 24939 / 12469.5 | yes | — | — |
| — | — | 3 | (no activation) | 29828 / 14914 | — | 33252 / 16626 | — | — | — |
| 5 | PLA4 | 4 | 14913 | 37284 / 18642 | 16625 | 41564 / 20782 | yes | yes | — |
| restart | — | — | — | 37284 / 18642 | — | 41564 / 20782 | — | — | — |

NESdev describes the same sequence as five steps; its step 4 (at 14914.5 APU cycles ≈ 29829 CPU
cycles NTSC, 16626.5 ≈ 33253 PAL) produces no clock at all. (NESdev's own 4-step table rounds the
interrupt total to 29830 / 33254 CPU cycles; see Open questions.) The PLA4 interval therefore spans
two nominal steps. The wiki's PLA activation numbering (0, 1, 2, 4) is the hardware's output index,
not the NESdev step number; both are given above.

Rates (NTSC / PAL): quarter frame ~192 Hz / ~160 Hz, half frame ~96 Hz / ~80 Hz, and **no frame
interrupt in mode 1**. The wiki marks the count of the last output as slightly floating; the table
above uses its `nLFO1` row (`clkgen.md`, PLA4 = 14913), while the parallel `nLFO2` row gives
PLA4 = 22367 measured from PLA1. The two mode-5 rows therefore describe the last output 4 CPU cycles
apart (37284 vs 37280). Mode 4 shows half that spread: the `nLFO2` row gives PLA1 (14913) and
then PLA3 (14913) (SET LFSR), i.e. **29826** CPU cycles for the mode-0 sequence instead of the
`nLFO1` row's 29828 — the "slightly floating" last output the wiki's own caveat leaves unreconciled.

## 6. What each step clocks

`/LFO1` = quarter frame = `frm_quarter`; `/LFO2` = half frame = `frm_half` (`visual2a03.md`).

| Consumer | Clock | Where it is used |
|---|---|---|
| Pulse 1 / Pulse 2 envelope | `/LFO1` | envelope divider decrement / reload (`env.v`) |
| Noise envelope | `/LFO1` | same shared envelope unit (`env.v`) |
| Triangle linear counter | `/LFO1` | linear-counter reload / decrement (`triangle.v`) |
| Length counters (all four) | `/LFO2` | decrement when the counter is enabled (`length.v`) |
| Pulse 1 / Pulse 2 sweep | `/LFO2` | sweep divider / `DO_SWEEP` generation (`square.v`) |

The envelope unit is shared between the two pulse channels and the noise channel; the triangle uses
`/LFO1` for its linear counter only, with its own step gating. The wiki defines the two LFO outputs at
~240 Hz (mode 0) / ~192 Hz (mode 1) for LFO1 and ~120 Hz / ~96 Hz for LFO2, all NTSC figures, matching
the step tables above.

## 7. Frame interrupt

- The flag is set at the **mode-0 step 4** point (PLA3), i.e. at 29828 CPU cycles (NTSC) or 33252
  (PAL) after the sequence start, provided `$4017[6]` is clear (`int_ff` set input is
  `n_mdout & PLA_in[3]`; `n_mdout` is `1` only in mode 0).
- The flag is a level: it stays set and keeps `/IRQ` asserted until cleared.
- Clearing: a read of `$4015` clears it; writing `$4017` with bit 6 = 1 also clears it and prevents it
  being set while bit 6 remains 1 (`mask_clear`).
- The DMC flag is independent and is not cleared by the `$4015` read (`DPCM_IntControl`).

## 8. `$4017` write timing and alignment

- `W4017` is asserted by the register decoder for the whole PHI2 half of the write cycle
  (`regs.v`: `nor(W4017, PHI1, ~pla[27])`). The mode and inhibit bits are captured by transparent
  register latches while `W4017` is high, so the register content changes during the write cycle.
  Writing `%1000_0000` (`$80`) selects the 5-step sequence with the interrupt enabled; `%0100_0000`
  (`$40`) selects the 4-step sequence and inhibits the interrupt; `%1100_0000` (`$C0`) inhibits the
  interrupt while staying in 5-step mode (`clkgen.v` latches `mode` from `DB[7]` and `mask` from
  `DB[6]`, both enabled by `W4017`).
- `W4017` also resets the `z` flip-flop in the LFSR control; on the next `/ACLK2` phase this forces
  the LFSR reset (`F1`). A reset started by a write also emits a quarter and a half frame signal if
  the written mode bit is 1.
- NESdev describes the observable result as: **"after 3 or 4 CPU clock cycles the timer is reset"**,
  the choice depending on whether the write falls inside or between APU cycles. NESdev provides the
  3–4-cycle range; no committed testbench in this repository measures which of the two applies (see
  Open questions).
- Because the write restarts the sequencer, **any `$4017` write** moves every following step and can
  suppress or delay a pending frame interrupt by up to one full sequence: a write shortly before the
  step-4 point replaces the pending IRQ with a fresh sequence, which reaches its own step 4 one
  mode-0 period later (29828 CPU cycles NTSC / 33252 PAL), counted from the 3–4-cycle-later restart
  rather than from the write cycle. This is the "IRQ delayed by one step / one frame" behaviour of
  test ROMs that write `$4017` late in the frame. The only run of this measurement is ad-hoc and not
  reproducible from the tree (see Open questions), so no concrete cycle offset is quoted here.

## 9. Reset and power-on state

- `/RES` (pin 3; NESdev pin name `/RST`) produces the internal `RES` signal; `RES` resets the frame
  interrupt flip-flop and holds the sequencer's shift register in its reset state.
- The mode and inhibit registers have no reset input and power up as 0 (their latches initialise to
  0), so after reset the frame counter runs the **4-step sequence with the interrupt enabled**.
- After `RES` is released the shift register self-starts: `F1` loads every bit with `1`
  (`SoftCLK_LFSR_Bit`), so the reset state is all ones, and the anti-zero PLA output (5) only prevents
  the register locking at all zeros after the first shifts. The first mode-0 activations then appear
  after roughly 7457 CPU cycles from the start of the sequence.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | `$4015[6]` becomes 1 about 60 times/s (NTSC) or 50 times/s (PAL) | mode-0 step 4 sets `int_ff` | poll `$4015` or service the IRQ; clear by reading `$4015` |
| 2 | Reading `$4015` returns the flag and clears it | `int_ff` reset input driven by `/R4015` | record the flag before reading if the value matters |
| 3 | Setting `$4017[6]` silences the frame IRQ permanently until cleared | `mask_clear` resets `int_ff` and blocks the set term | use it to disable the frame IRQ cleanly |
| 4 | The frame IRQ never fires in 5-step mode | PLA3 (the interrupt output) is skipped in mode 1 | use mode 1 for VBL-only timing |
| 5 | An IRQ is late or missing after a `$4017` write | the write restarts the sequencer; the pending step is replaced | do not write `$4017` near the step-4 point if the IRQ timing matters |
| 6 | A `$4017` write with bit 7 set produces an immediate envelope step / audible click | reset with the mode bit set generates quarter and half frame signals | expect an extra envelope/linear clock on mode switch |
| 7 | Music speeds up/slows when the mode changes | quarter frame is 240 Hz (mode 0) vs 192 Hz (mode 1); half frame 120 Hz vs 96 Hz (NTSC) | pick the mode before starting playback; the LFO rates differ by design |
| 8 | Envelope decay and sweep rates change together between mode 0 and mode 1 | both are driven by the same two LFO signals | compute envelope/sweep periods from the active mode |
| 9 | The IRQ rate drifts slightly against the video frame | 29828 CPU cycles vs 29780.5 CPU cycles per NTSC frame | for frame-locked timing use the PPU NMI, not the frame IRQ |
| 10 | PAL runs the sequences 8313 CPU cycles per step, not 7457 | the 2A07 has a different PLA and master clock | select the step table from the region; do not reuse NTSC counts |

## Programming / implementation notes

- An emulator MUST count the sequencer in APU cycles (2 CPU cycles) and MUST use the region's step
  table: NTSC 7457 CPU cycles per nominal step, PAL 8313.
- The mode-0 sequence MUST span 29828 CPU cycles (NTSC) / 33252 (PAL) from one step-4 interrupt to
  the next; NESdev quotes 29830 / 33254, so do not mix the two totals in one implementation.
- The frame IRQ flag MUST be a level flag, not a one-cycle pulse, and MUST be cleared by a read of
  `$4015` and by `$4017[6] = 1`.
- A write to `$4017` MUST reset the sequence and MUST take effect 3–4 CPU cycles after the write
  cycle (mode 1 writes additionally emitting an immediate quarter and half frame clock).
- Mode 0 MUST clock `$4015[6]` at step 4; mode 1 MUST NOT clock the flag.
- Implementations SHOULD drive the quarter frame from `/LFO1` and the half frame from `/LFO2` so
  envelopes/linear counter and length/sweep share the same step phase.
- The frame IRQ and the DMC IRQ MUST be treated as one CPU `/IRQ` input, with independent flags.
- Writing `$4017` SHOULD not be modelled as an immediate register change without the sequence reset;
  test ROMs exist that depend on the delay.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Frame counter registers, write side effects, 4/5-step tables | `APU_Frame_Counter` |
| Sequencer structure (LFSR, PLA outputs, anti-zero/SET LFSR) | `APU_Frame_Counter` (observable sequence only; the LFSR itself is internal and not described) |
| `$4017` / `$4015` bitfields and addresses | `APU_registers` |
| Quarter frame consumers (envelope, linear counter) | `APU_Envelope`, `APU_Triangle` |
| Half frame consumers (length counter, sweep) | `APU_Length_Counter`, `APU_Sweep` |
| DMC interrupt flag and its clearing rules | `APU_DMC` |
| APU clock tree, divider and APU cycle rate | `Cycle_reference_chart` |
| Reset and power-on state of `$4017` and the frame counter LFSR | `CPU_power_up_state` |
| PAL 2A07 step counts and frame rate | `APU_Frame_Counter`, `Cycle_reference_chart` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| SoftCLK purpose, modes, `$4017` bits | `APU/clkgen.md` | `clkgen.v` (`SoftCLK_Control`) |
| 4-step / 5-step PLA activation intervals, SET LFSR | `APU/clkgen.md` | `clkgen.v` (`SoftCLK_PLA`, `SoftCLK_LFSR_Control`) |
| LFSR structure and anti-zero output 5 | `APU/clkgen.md`, `APU/common.md` | `clkgen.v` (`SoftCLK_LFSR`, `SoftCLK_LFSR_Bit`) |
| `/LFO1`, `/LFO2` routing | `APU/clkgen.md`, `APU/visual2a03.md`, `APU/length.md` | `env.v`, `triangle.v`, `noise.v`, `square.v`, `length.v` |
| `$4015` bit assignments | `APU/clkgen.md`, `APU/regs.md`, `APU/length.md`, `APU/dpcm.md` | `length.v`, `dpcm.v`, `clkgen.v` |
| Register decode and PHI2 write gating | `APU/regs.md` | `regs.v` (`ApuRegsDecoder`) |
| Interrupt daisy-chain with DMC | `APU/clkgen.md`, `APU/dpcm.md` | `clkgen.v`, `dpcm.v` (`DPCM_IntControl`) |
| ACLK clock tree | `APU/clkgen.md`, `APU/waves.md` | `clkgen.v` (`ACLKGen`) |
| 2A07 PLA and divider differences | `APU/pal.md` | `clkgen.v` (PLA masks in `APU/pal.md`) |
| Cycle counts (NTSC/PAL) | `APU/clkgen.md` (PLA interval tables — rank 1) | `HDL/Framework/Icarus/apu/clkgen.v` checks clock-tree transitions only, so the write latency is NESdev rank 4 and the ad-hoc interval run is not reproducible |

## Open questions

- 4-step / 5-step totals: the repository gives **29828** CPU cycles for mode 0 (four uniform
  7457-cycle PLA intervals; PAL 33252) and **37284** for mode 1 (7457 + 7457 + 7457 + 14913; PAL
  41564 = 8313 + 8313 + 8313 + 16625); these are the specification here. NESdev's 4-step table puts
  the step boundaries at cumulative 7457 / 14913 / 22371 / **29828** CPU cycles (deltas 7457, 7456,
  7458, 7457), adds an extra quarter+half clock at 29829, resets the counter at 29830 and therefore
  quotes the 4-step interrupt period as 29830 CPU cycles (PAL 33254). Its mode-1 table gives **37281**
  (NTSC) and **41565** (PAL), i.e. 18640.5 / 20782.5 APU cycles; NESdev's 29829 is that mode-1 table's
  step-4 time (14914.5 APU), not a 4-step boundary, and 29830 is its reset/period figure. An ad-hoc
  (uncommitted) Icarus run of `clkgen.v` produced the first three 4-step intervals as 7457, 7456,
  7458, matching the NESdev deltas rather than the uniform repository table. Which is exact, and is
  the difference the "slightly floating" last interval? Both values are kept; this file uses the
  repository figures.
- NESdev says the `$4017` write takes effect after 3 **or 4** CPU cycles depending on APU-cycle
  alignment. No committed testbench measures the latency (`HDL/Framework/Icarus/apu/softclk.v` says it
  "is not a unit test right now", and `.../clkgen.v` only scans clock-tree transitions); an ad-hoc run
  suggested 3 cycles for a write asserted at the start of PHI2, but that is not reproducible from the
  tree. The 4-cycle case needs a dedicated committed test.
- Dendy famiclones are stated elsewhere to use an NTSC-like CPU:PPU ratio. No source in this
  repository documents the Dendy frame-counter PLA, so whether a Dendy uses the 2A03 step table is
  unconfirmed.
- `cycles.md` documents 13 internal timing loops as images only; no textual description exists, so
  the loops involving SoftCLK (if any) could not be transcribed.
- The exact mode/mask register value at power-on is inferred from the latch initialisation in
  `HDL/APU/common.v` (`RegisterBit`), not stated in the wiki.
