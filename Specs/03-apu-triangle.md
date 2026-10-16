# APU 03-triangle — Triangle channel

**Status:** consensus (round 1)
**Scope:** The triangle channel of the 2A03/2A07 APU as an implementable unit: the
11-bit timer (clocked at the CPU clock rate), the 5-bit output counter and its 32-step
sequence, the linear counter with its control/reload flags, and the interaction with
the shared length counter. The channel has no volume control and no sweep; the mixer
is specified in `03-apu-mixer.md`.
**Primary sources:** `BreakingNESWiki/APU/triangle.md`, `.../length.md`, `.../sound.md`,
`.../common.md`, `.../waves.md`, `.../clkgen.md`, `.../regs.md`; HDL `APU/triangle.v`,
`APU/length.v`, `APU/common.v`, `APU/regs.v`; Icarus benches `triangle_output.v`,
`length_counter.v`.

## 1. Channel structure

The triangle channel (Sound C, `sound.md`) is a fixed-amplitude generator: its 4-bit
output goes to the mixer without a volume or envelope stage. It contains
(`triangle.md`, `TRIANGLE` schematic):

| Block | Function | HDL |
|---|---|---|
| Control | derives load/step strobes for the three counters | `TRIANGLE_Control` |
| Linear counter | 7-bit down counter, extra duration timer | `TRIANGLE_LinearCounter` |
| Frequency counter | 11-bit down counter | `TRIANGLE_FreqCounter` |
| Frequency register | 11-bit timer period | `RegisterBit` array in `TRIANGLE_FreqCounter` |
| Output counter | 5-bit up counter, drives the saw direction | `TRIANGLE_Output` |
| Length counter | shared 8-bit down counter instance | `LengthCounter` |

The developers used the CPU phase `PHI1` instead of `ACLK1` for parts of the triangle
channel specifically to smooth the "stepped" waveform (`triangle.md`, `sound.md`).
The frequency register, the frequency counter and the output counter are therefore
clocked at the CPU clock rate, not at the APU clock rate (one APU cycle = 2 CPU
cycles). The linear counter uses the frame counter's quarter-frame signal `/LFO1`
(active low).

The output counter only advances when the whole channel is allowed to sound:

```
TTSTEP = LFO-independent pulse from the timer
         AND linear counter not zero (TCO = 0)
         AND length counter counting (NOTRI = 0)
         AND LOCK = 0
```

(`TRIANGLE_Control`: `nor(TCO, LOCK, PHI1, NOTRI, n_FOUT)`), so gating the channel
freezes the output counter at its current value instead of resetting it.

## 2. Registers

All channel registers are write-only; reads return open bus (`regs.md`).

```
$4008
7  bit  0
---- ----
CRRR RRRR
  bit 7    control flag: 0 = the linear counter expires and does not reload;
           1 = it reloads with R at every quarter frame, and the length
           counter is halted (same bit)
  bits 6:0 linear counter reload value R (0…127)
```

```
$400B
7  bit  0
---- ----
LLLL LHHH

bits 7:3   length counter index L (see §5)
bits 2:0   timer high 3 bits (period bits 10:8)
```

| Register | NESdev bitfield | Contents |
|---|---|---|
| `$4008` | `CRRR.RRRR` | control flag (bit 7) and linear counter reload value (bits 6:0) |
| `$400A` | `LLLL.LLLL` | timer low 8 bits (period bits 7:0) |
| `$400B` | `LLLL.LHHH` | length counter index (bits 7:3) and timer high (bits 2:0) |
| `$401A` | — (test register) | 2A03 test register: loads the 5-bit output counter with bits 4:0 (`regs.md`) |

Side effects of `$400B`: it loads the length counter, sets the linear counter reload
flag (§4) and updates the timer high bits. Writing `$4008` does not set the reload
flag.

## 3. Timer and output sequencer

The timer period is `t = %HHH_LLLLLLLL` from `$400B` bits 2:0 and `$400A`. The 11-bit
down counter is stepped once per **CPU cycle** (unlike the pulse timer, which is
stepped once per APU cycle); on borrow it reloads with `t` and advances the output
counter by one step.

- timer period = `t + 1` CPU cycles
- 32 steps per waveform
- `f = fCPU / (32*(t + 1))`, `t = fCPU / (32*f) - 1`

`fCPU` is 1.7897727 MHz on **NTSC** (2A03, 21.477272 MHz ÷ 12) and 1.662607 MHz on
**PAL** (2A07, 26.601712 MHz ÷ 16, `clkgen.md`). **Dendy** uses an NTSC-like CPU:PPU
ratio with a different clock and must not be treated as a 2A07 (see *Open questions*).

| t | 0 | 1 | 3 | 253 | 511 |
|---|---|---|---|---|---|
| NTSC f | 55.93 kHz | 27.97 kHz | 13.98 kHz | 220.2 Hz | 109.2 Hz |
| PAL f | 51.96 kHz | 25.98 kHz | 12.99 kHz | 204.6 Hz | 101.5 Hz |

The output counter is a 5-bit up counter `T[4:0]`. `T[4]` selects the direction and
bits 3:0 give the level:

```
TRI_Out = NOT( T[4] ? NOT(T[3:0]) : T[3:0] )      // TRIANGLE_Output, triangle.v
```

which produces the 32-step pseudo-triangle sequence sent to the mixer
(`triangle.md` "5-разрядный UpCounter. Старший разряд управляет направлением"):

```
15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
```

The duplicated 0 and 15 steps give the sequence an average of 7.5, and the channel
has no mute for very small periods:

- with `t = 0` or `t = 1` the timer period is 1–2 CPU cycles, so the channel emits an
  ultrasonic tone of `fCPU/32` … `fCPU/64` (about 55.9 kHz down to 28.0 kHz NTSC,
  52.0 kHz to 26.0 kHz PAL). The schematic has no frequency-based mute for the
  triangle (`TRIANGLE_Control` has no `SW_UVF` equivalent); after the output filter
  the level settles around 7.5, so writing such a period is heard as a click followed
  by a DC offset rather than as silence.

`$400B` does not reset the timer divider; the new period takes effect at the next
reload.

## 4. Linear counter

The linear counter is a 7-bit down counter that gates the output counter for a
programmable time (`triangle.md` "7-разрядный DownCounter"). It is clocked by the
quarter-frame signal `/LFO1` (about 240 Hz in 4-step mode, 192 Hz in 5-step mode
NTSC; 7457 CPU cycles per quarter frame in 4-step mode, `clkgen.md`). Two flags
control it:

| Flag | Stored in | Set by | Cleared by |
|---|---|---|---|
| control flag | `$4008` bit 7 | write to `$4008` | write to `$4008` |
| reload flag | internal flip-flop | write to `$400B` | a quarter-frame clock, **only if** the control flag is clear |

At every quarter-frame clock, in this order:

1. if the reload flag is set, the counter is loaded with `R` (`$4008` bits 6:0) and
   is not decremented on this clock;
2. otherwise, if the counter is non-zero, it is decremented;
3. if the control flag is clear, the reload flag is cleared.

The channel is gated open while the counter is non-zero, so a single reload with
value `R` (control flag clear) holds the channel open for exactly `R` quarter-frame
clocks (`R = 127` is the maximum, about 0.53 s NTSC in 4-step mode and 0.66 s in
5-step mode). With the control flag set, the reload flag is never cleared while the
flag remains set, so the counter is reloaded with `R` at every quarter frame and the
linear counter never expires.

`$400B` sets the reload flag, so the reload does not happen instantly: it happens at
the next quarter-frame clock. A `$4008` write does not clear the reload flag either,
so writing `$4008` while both flags are already set changes the value that will be
reloaded at the next quarter-frame clock (`triangle.md`, `length.md`; the `Reload_FF`
stage in `TRIANGLE_Control`).

## 5. Length counter interaction

The triangle uses the same length counter module as the other tone channels
(`length.md`); its input carry is `TRI_LC = ~$4008` bit 7 and its loaded value comes
from the PLA indexed by `$400B` bits 7:3. The PLA reload value is one less than the
community length table because the counter is silenced when it is clocked *while
already zero* (`length.v`; verified on the `length_counter.v` bench). NESdev
`APU_Length_Counter` (section "Length counter internals") documents the same
relationship: the published table entries are the values the counter is loaded with
plus one.

```
index   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   (hex)
reload  9  FD  13   1  27   3  4F   5  9F   7  3B   9   D   B  19   D
length 10 254  20   2  40   4  80   6 160   8  60  10  14  12  26  14
index  10  11  12  13  14  15  16  17  18  19  1A  1B  1C  1D  1E  1F
reload  B   F  17  11  2F  13  5F  15  BF  17  47  19   F  1B  1F  1D
length 12  16  24  18  48  20  96  22 192  24  72  26  16  28  32  30
```

`reload` is the internal PLA value the counter is loaded with; `length` is the
community (NESdev) table entry for the same index, equal to `reload + 1`, and is the
number of `/LFO2` half-frame clocks (`$4015` bit 2 must be set); at 120 Hz the shortest
note is 83 ms and the longest 2.12 s (NTSC, 4-step mode). Implementations need both:
load `reload`, silence one clock after it reaches zero, and quote `length` as the note
duration.

The two counters are AND-ed at the output gate: the channel sounds only while the
linear counter is non-zero **and** the length counter is counting. Consequences:

- the effective duration is the shorter of the linear-counter time (`R` quarter
  frames) and the length-counter time (`length` half frames);
- setting `$4008` bit 7 (length counter halt) also keeps the linear counter reloading
  forever, so a halted triangle plays until `$4015` disables it or the period is
  changed;
- `$4015` bit 2 = 0 disables the channel immediately; re-enabling it has no effect
  until `$400B` is written again.

Common ways to silence the channel and their side effects (`triangle.md` behaviour
plus `TRIANGLE_Control`):

| Method | Effect |
|---|---|
| write `$80` to `$4008` | control flag set, `R = 0`: the next reload loads the counter with 0 (the reload flag must already be set from a `$400B` write) and the output counter holds its current level (clean pause, no click) |
| write `$FF` to `$4008` afterwards | the next reload gives `R = 127` and the tone resumes from the held level (the output counter was never reset) |
| `$4015` bit 2 = 0 | immediate silence and length counter reset |
| write a period of 0 or 1 | ultrasonic output; the filtered level sits between 7 and 8, so a click is heard when a normal period is restored |

## 6. Output gating and test mode

`TRIANGLE_Output` is loaded by `$401A` (2A03 only, `regs.md`) and cleared by the
internal `RES`. `LOCK` (asserted by the 2A03 test readout of `$4019`) stops the
output counter, holding the currently generated level so the debug interface can read
`TRI[3:0]`; the triangle contributes `TRI[3:0]` to `$4019` bits 3:0 and the noise
channel to bits 7:4 (`regs.md`). Normal programs never assert `LOCK`.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | Writing a period of 0 or 1 gives a click and a quiet DC offset instead of silence | the triangle has no frequency mute; the output runs at up to `fCPU/32` and the filter averages it to about 7.5 | silence via `$4008`/`$4015` instead |
| 2 | Writing `$80` to `$4008` stops the tone at the next quarter frame with no click | `R = 0` closes the gate while the output counter holds its position | use this for pause/resume |
| 3 | Writing `$FF` after `$80` resumes exactly where the tone stopped | the output counter is gated, not reset | exploit for click-free note gaps |
| 4 | The tone continues after the linear counter value was changed while both flags were set; the new value is applied one quarter frame later | the reload flag is only cleared when the control flag is clear | treat `$4008` writes as taking effect at the next quarter-frame clock |
| 5 | A triangle note with `$400B` length index 0 lasts 10 half frames even though the PLA holds 9 | the counter is silenced one clock after it reaches zero | use the community length table for durations, not the reload value |
| 6 | Setting `$4008` bit 7 makes the note play forever even with a length written | bit 7 is both the length counter halt flag and the linear counter control flag; the linear counter now reloads every quarter frame | clear bit 7 to let either counter expire |
| 7 | Writing `$400A`/`$400B` does not restart the waveform from 15 | the timer divider is not reset by a register write and the output counter keeps its phase | write `$401A` (2A03 test register) only if an exact phase is required |
| 8 | A period update takes effect only at the next timer borrow | the frequency counter is loaded from the register on borrow | do not expect an immediate pitch change |
| 9 | The triangle reaches much higher frequencies than the pulse channels | its timer is clocked at the CPU rate (not the APU rate) and there is no fixed mute | clamp only if the audio path requires it, not in the channel model |
| 10 | `$4015` bit 2 = 0 silences the channel at once and the tone does not return when the bit is set again | the length counter is forced to non-counting and must be reloaded | re-enable, then write `$400B` |

## Programming / implementation notes

- The triangle timer MUST be stepped once per CPU cycle; using the APU clock halves
  all frequencies and is a defect.
- `f = fCPU/(32*(t+1))` MUST be used for pitch; `fCPU` is region-specific
  (NTSC 1.7897727 MHz, PAL 1.662607 MHz).
- The output counter MUST be a 5-bit up counter with the direction taken from bit 4,
  producing the 32-step sequence in §3; it MUST NOT be reset when the gate closes.
- The linear counter MUST implement the three ordered actions in §4 and MUST clear
  the reload flag only when the control flag is clear.
- The reload flag MUST be set by `$400B` and MUST NOT be set by `$4008`.
- The length counter MUST be loaded by `$400B` bits 7:3 with the reload values in §5
  and MUST silence the channel one half-frame after those reloads reach zero.
- The output MUST be gated by `(linear counter non-zero) AND (length counter
  counting)`; the gate MUST NOT reset the output counter.
- `$401A` (2A03 test register) MAY be implemented as a direct load of the output
  counter; it SHOULD be disabled on 2A07.
- A tool that needs exact note lengths SHOULD use the `length` row of §5 divided by
  the region's half-frame rate, and MUST NOT assume the two counters are independent.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Triangle block structure, CPU-rate timer, sequencer, linear counter | `APU_Triangle` |
| 32-step sequence and the triangle silencing methods (`$80`/`$FF`, `$4015`, period 0/1) | `APU_Triangle` |
| Length counter table, halt flag, `$4015` enable/disable | `APU_Length_Counter` |
| `$4008`–`$400B` register layout and bit fields | `APU_registers` |
| Quarter-frame and half-frame clocks | `APU_Frame_Counter` |
| CPU clock rates (NTSC 2A03, PAL 2A07, Dendy) | `APU_Triangle`, `Cycle_reference_chart` |
| Triangle output contribution to the non-linear mixer | `APU_Mixer` |
| 2A03 test registers `$4018`–`$401A` and `LOCK` | `CPU_pinout` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Channel blocks, PHI1 clocking, counter widths | `APU/triangle.md`, `APU/sound.md` | `APU/triangle.v` |
| Timer formula, output counter, 32-step sequence | `APU/triangle.md` | `TRIANGLE_FreqCounter`, `TRIANGLE_Output` |
| Linear counter algorithm and flags | `APU/triangle.md`, `APU/length.md` | `TRIANGLE_LinearCounter`, `TRIANGLE_Control` |
| `$400B` reload-flag side effect and quirk | `APU/length.md`, `APU/waves.md` | `Reload_FF` in `TRIANGLE_Control` |
| Length counter reload table and counting | `APU/length.md`, `APU/waves.md` | `APU/length.v` |
| LFO1 rate and quarter-frame counts | `APU/clkgen.md` | `APU/clkgen.v` |
| `$401A` output-counter load, `LOCK`, `$4019` readout | `APU/regs.md` | `TRIANGLE_Output`, `APU/regs.v` |
| Register write example (test vector) | `APU/waves.md` | `Framework/Icarus/apu/triangle_output.v` |

Verification performed while compiling this section: Icarus scratch bench of
`TriangleChan` with `$4008 = $85`, `$400A = $03`, `$400B = $00` reproduced a timer step
every 4 CPU cycles and the sequence 15,14,…; the `length_counter.v` bench supplied the
length-table verification shared with `03-apu-pulse.md`.

## Open questions

- The exact boundary behaviour when `$400B` is written in the same CPU cycle as the
  quarter-frame clock is modelled by the `Reload_FF` pipeline in `TRIANGLE_Control`;
  whether the community model's "reload at the next quarter-frame clock" is
  cycle-accurate was not verified on hardware.
- `$401A` is documented in `regs.md` only as a 2A03 test write that loads the triangle
  output counter; its interaction with `LOCK` and with the debug readout is not
  specified here.
- The analog result of the ultrasonic `t = 0`/`t = 1` period (the "halfway between 7
  and 8" level) is a filter effect, not a digital one; the schematic does not model it.
- **Dendy CPU clock.** Dendy (PAL famiclone with an NTSC-like CPU:PPU ratio) is not
  covered by `APU/clkgen.md`. NESdev `Cycle_reference_chart` (and `APU_Triangle`/`APU_Pulse`) give
  `fCPU = 1.773448 MHz` for Dendy (26.601712 MHz ÷ 15) — a community value,
  **unconfirmed** by ranks 1–3; the platform overview should state which value it uses.
