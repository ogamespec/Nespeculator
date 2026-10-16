# APU 03-pulse — Pulse 1 and Pulse 2 channels

**Status:** consensus (round 1)
**Scope:** The two pulse (square) channels of the 2A03/2A07 APU as implementable
units: timer, duty sequencer, envelope, sweep and length counter, and the output gate.
The frame counter and mixer are covered by `03-apu-frame-counter.md` and `03-apu-mixer.md`.
**Primary sources:** `BreakingNESWiki/APU/square.md`, `.../length.md`, `.../sound.md`,
`.../common.md`, `.../waves.md`, `.../clkgen.md`; HDL `APU/square.v`, `APU/env.v`,
`APU/length.v`, `APU/common.v`, `APU/apu_top.v`, `APU/regs.v`; Icarus benches
`square_output.v`, `square_duty.v`, `square_sweep.v`, `env_unit.v`, `length_counter.v`.

## 1. Channel structure

Pulse 1 (Square 0) and Pulse 2 (Square 1) are two copies of one generator (`SQUARE`,
`square.md`). Each channel contains:

| Block | Function | HDL |
|---|---|---|
| Frequency register `Fx[10:0]` / counter | period storage and 11-bit down counter, drives the sequencer | `SQUARE_FreqReg`, `SQUARE_FreqCounter` |
| Duty sequencer | 3-bit down counter + 4-way output select | `SQUARE_Duty` |
| Envelope generator | 4-bit decay level + 4-bit divider | `Envelope_Unit` |
| Sweep unit | 3-bit divider, barrel shifter, adder | `SQUARE_Sweep`, `SQUARE_Adder` |
| Length counter | 8-bit down counter, shared module | `LengthCounter` |
| Output gate | mutes the channel and selects volume | `SQUARE_Output` |

The envelope unit is shared with the noise channel; the length counter is one of four
identical instances (`length.md`). The channels are identical except for the sweep
adder carry-in, which is the only difference between Pulse 1 and Pulse 2 (§5). The
4-bit generator output `SQA[3:0]` / `SQB[3:0]` goes to the mixer as the channel DAC.
Envelope and sweep iterate on the frame counter's `/LFO1` (quarter frame) and `/LFO2`
(half frame), both active low; the timer and duty sequencer use `ACLK1`/`ACLK2` (one APU
cycle = 2 CPU cycles, `clkgen.md`). Register writes are active in the CPU `PHI2` phase only.

## 2. Registers

All registers are write-only; reads return open bus (`regs.md`).

```
$4000 / $4004
7  bit  0
---- ----
DDLC VVVV
|||| ||||
|||| ++++- volume / envelope divider reload value V (bits 3:0)
|||+------ 0: envelope, 1: constant volume (bit 4)
||+------- 0: length counter counts, 1: halt counter / loop envelope (bit 5)
++-------- duty select: 0=12.5 %, 1=25 %, 2=50 %, 3=75 % inverted (bits 7:6)

$4001 / $4005
7  bit  0
---- ----
EPPP NSSS
|||| ||||
|||| ++++- shift count S for the sweep adder (bits 2:0); S = 0 behaves like bit 7 = 0
|||+------ 0: increase period, 1: decrease period (negate, bit 3)
|+++------ sweep divider reload value P (bits 6:4); the divider period is P + 1 half frames
+--------- 0: sweep process disabled, 1: sweep enabled (bit 7, SWDIS; NESdev enabled flag E)
```

| Register | NESdev bitfield | Contents |
|---|---|---|
| `$4002` / `$4006` | `LLLL.LLLL` | timer low 8 bits (period bits 7:0) |
| `$4003` / `$4007` | `LLLL.LHHH` | bits 7:3 length counter index; bits 2:0 timer high (period bits 10:8) |

Writing `$4003`/`$4007` loads the length counter, restarts the envelope (§4) and
restarts the duty sequencer; it does **not** reset the timer divider. `$4003` bits 7:3
index a PLA (§6).

## 3. Timer and duty sequencer

The timer period is the 11-bit value `t = %HHH_LLLLLLLL` formed by `$4003` bits 2:0
(high) and `$4002` (low). The down counter is stepped once per APU cycle (2 CPU
cycles); on borrow it is reloaded with `t` and the duty sequencer advances one step.

- timer period = `t + 1` APU cycles = `2*(t + 1)` CPU cycles
- duty step rate = one step per `t + 1` APU cycles
- waveform period = `8*(t + 1)` APU cycles = `16*(t + 1)` CPU cycles
- Pulse frequency `f = fCPU / (16*(t + 1))`, `t = fCPU / (16*f) - 1`

`fCPU` is 1.7897727 MHz on **NTSC** (2A03, 21.477272 MHz ÷ 12) and 1.662607 MHz on
**PAL** (2A07, 26.601712 MHz ÷ 16, `clkgen.md`). **Dendy** uses an NTSC-like CPU:PPU
ratio with a third CPU clock and must not be treated as a 2A07 (see *Open questions*). The very low
periods are not usable: see the mute rules in §7.

| t | 0 | 3 | 4 | 7 | 8 | 253 | 2047 |
|---|---|---|---|---|---|---|---|
| NTSC f | 111.86 kHz | 27.97 kHz | 22.37 kHz | 13.98 kHz | 12.43 kHz | 440.4 Hz | 54.6 Hz |
| PAL f | 103.91 kHz | 25.98 kHz | 20.78 kHz | 12.99 kHz | 11.55 kHz | 409.1 Hz | 50.7 Hz |

`$4003`/`$4007` loads the 3-bit duty counter with 0 (phase reset); each timer reload
(`FLOAD`) steps it down. `DUTY` is a function of the counter value and `$4000`/`$4004`
bits 7:6 (`d`), `square.md`. The counter counts downward, so each line below is read
left to right:

```
counter  7 6 5 4 3 2 1 0
d=0      1 0 0 0 0 0 0 0      12.5 %
d=1      1 1 0 0 0 0 0 0      25 %
d=2      1 1 1 1 0 0 0 0      50 %
d=3      0 0 1 1 1 1 1 1      75 % (25 % negated)
```

These are the community duty sequences (NESdev lists them at a different phase).
Writing `$4000`/`$4004` changes the duty without resetting the counter phase.

## 4. Envelope generator

One `Envelope_Unit` per pulse channel (same unit as noise). It contains a 4-bit down
divider and the 4-bit decay level:

- **divider**: loaded with `V` and stepped on the quarter-frame clock; it reloads with
  `V` when it borrows, so its period is `V + 1` quarter frames.
- **decay level**: 4-bit down counter, decrements each time the divider borrows, 15 → 0.
- **start flag**: set by a write to `$4003`/`$4007`. At the next quarter-frame clock the
  flag is cleared, the decay level is loaded with 15 and the divider with `V`; so a
  retriggered note always starts at volume 15.
- **loop flag** (`$4000` bit 5): when the decay level reaches 0 at a divider borrow it
  is loaded with 15 if the flag is set, otherwise it stays 0.
- **constant volume** (bit 4): selects the volume source only. With bit 4 set the output
  is `V`; clear, it is the decay level. The decay counter keeps running either way.

The quarter frame is `/LFO1`: NTSC 4-step mode fires every 7457 CPU cycles (about
240 Hz), 5-step mode averages about 192 Hz; PAL uses 8313 cycles in 4-step mode and
16625 cycles for the long 5-step interval (`clkgen.md`). The decay level therefore
steps every `V + 1` quarter frames; for `V = 15` that is 16 quarter frames (measured on
the HDL). Internal signals `RLOAD`/`RSTEP`, `ERES`/`ESTEP` and
`EIN = ~(decay level zero AND bit 5 clear)`; a reload with `EIN = 1` sets the decay level to 15 (`env.v`).

## 5. Sweep unit

The sweep applies a constant-in-time period change while active (`sound.md`); it exists
only in the pulse channels and works on the 11-bit value `Fx`:

| `$4001` bits | Meaning |
|---|---|
| 7 | `SWDIS` (NESdev: enabled flag `E`, active low): 0 = sweep process disabled, 1 = sweep enabled |
| 6:4 | divider reload value `P` (0…7); the divider period is `P + 1` half frames |
| 3 | negate (`DEC`): 1 = decrease period (raise pitch), 0 = increase |
| 2:0 | shift count `S` (`SR`); the shifter shifts `Fx` right by `S`; `S = 0` behaves as if bit 7 = 0 |

`SWDIS` is the *inverted* bit 7 (`swdis_reg .d(DB[7]) .nq(SWDIS)`, `square.v`): a set
bit 7 enables the sweep process, a clear bit 7 disables it. This matches NESdev's
"enabled flag" polarity and was re-checked with Icarus (`RegisterBit` with `d = 1`
drives `SWDIS = 0`); the BreakingNES Wiki signal table names `WR1[3]` as the source,
which the HDL refutes (see *Open questions*).

The divider is a 3-bit down counter loaded with `P` whenever it borrows; a write to
`$4001`/`$4005` reloads it at the next half-frame clock. While non-zero it is stepped
once per half frame (`/LFO2`, about 120 Hz NTSC 4-step), so `DO_SWEEP` — the pulse that
rewrites the frequency register — is generated every `P + 1` half frames (`P = 0`: every
half frame). `DO_SWEEP` is asserted during the CPU `PHI1` phase so it cannot collide with
a CPU write to `$4002`/`$4003` (`square.md`). Target period on a sweep step (barrel
shifter + complementary adder, `square.v`; the carry-in is the only channel difference,
`apu_top.v`):

```
increase (bit 3 = 0):   Fx' = Fx + (Fx >> S)
decrease (bit 3 = 1):   Fx' = Fx - (Fx >> S) - 1   for Pulse 1 (Square 0)
                        Fx' = Fx - (Fx >> S)       for Pulse 2 (Square 1)
```

The result is truncated to 11 bits; `S = 0` performs no sweep (`SRZ`). `DO_SWEEP` is
suppressed, and the period left unchanged, when any of these holds (`square.md`; the
conditions are OR-ed into one gate):

| Signal | Condition |
|---|---|
| `SWDIS` | `$4001` bit 7 clear (sweep process disabled) |
| `NOSQ` | the channel's length counter reached 0 or is disabled |
| `SRZ` | shift count `S = 0` |
| `SW_OVF` | increasing and the adder carried out (`Fx + (Fx >> S) > $7FF`) |
| `SW_UVF` | current period `Fx < 4` (bits 10:2 all zero) |
| `SCO` | the sweep divider has not borrowed yet |
| `/LFO2` | half-frame clock not active |

`SW_OVF` and `SW_UVF` additionally silence the channel itself (§7). No sweep is applied
while the length counter is halted, so halting the length counter also freezes pitch.

## 6. Length counter

All four tone channels share one PLA and four identical 8-bit down counters
(`length.md`, `length.v`); only the signals differ:

| Channel | Input carry | Not-counting output | Loaded by |
|---|---|---|---|
| Pulse 1 | `SQA_LC` (= `~$4000` bit 5) | `NOSQA` | `$4003` |
| Pulse 2 | `SQB_LC` (= `~$4004` bit 5) | `NOSQB` | `$4007` |
| Triangle | `TRI_LC` (= `~$4008` bit 7) | `NOTRI` | `$400B` |
| Noise | `RND_LC` (= `~$400C` bit 5) | `NORND` | `$400F` |

The counter is clocked by `/LFO2` (half frame) and decrements only while the channel's
input carry is 1 (halt flag clear) and the channel is enabled through `$4015`. The PLA
maps the 5-bit length index (`$4003` bits 7:3) to a reload value that is **one less**
than the community length table: the counter is silenced when it is clocked *while
already zero*, so the audible duration is one half frame longer than the reload
(`length.v`; the Icarus `length_counter.v` bench: index 9 reloads 7 and runs 8 half
frames). NESdev `APU_Length_Counter` (section "Length counter internals") documents the
same relationship: the published table entries are the values the counter is loaded
with plus one.

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
duration in `/LFO2` half-frame clocks. An implementation needs both: load `reload`,
silence the channel one clock after it reaches zero, and quote `length` as the note
duration. At 120 Hz (NTSC 4-step mode) index 0 lasts 83 ms and index 1 lasts 2.12 s;
PAL half frames are about 20 % longer (100 Hz, `clkgen.md`). Control:

- a length-register write loads the counter and starts counting (sets the state
  flip-flop); it only takes effect if the channel is enabled via `$4015`;
- the halt flag (bit 5 for pulse/noise, `$4008` bit 7 for triangle) stops the decrement
  but not the sound: the note plays until disabled or retriggered;
- `$4015` bit 0 (Pulse 1) / bit 1 (Pulse 2) = 1 enables the channel; 0 disables it
  immediately (`NotCount` forced). Enabling has no immediate counter effect — the
  counter is (re)loaded only by the next length-register write.

## 7. Output gate and muting

The 4-bit output is the volume value when the duty output is 1 and no mute condition is
true, otherwise 0 (`SQUARE_Output` in `square.v`):

```
d   = DUTY AND NOT(SW_UVF OR SW_OVF OR NOSQ)   // latched on ACLK1
sqv = d AND NOT(LOCK)                          // LOCK = 2A03 test readout
SQA = sqv ? V : 0                              // V = constant volume or decay level
```

| Mute condition | Cause |
|---|---|
| duty output 0 | sequencer's low phase (normal part of the waveform) |
| `NOSQ` | length counter disabled or already at zero |
| `SW_UVF` | timer period `t < 4` (frequency register bits 10:2 all zero) |
| `SW_OVF` | sweep increasing and the target period overflowed `$7FF` |
| `LOCK` | 2A03 test readout holding the generator (`regs.md`) |

> NESdev (`APU_Pulse`; the same threshold appears as the sweep unit's mute in
> `APU_Sweep`) silences a pulse channel when the timer period is `t < 8`. This
> repository's schematic silences it only for `t < 4` (`square.md`: "Frequency value
> is less than 4"; `square.v`: `nor (SW_UVF, Fx[2], Fx[3], …, Fx[10])`, i.e. timer
> bits 10:2 all zero). Both are output mutes, not timer changes. With the schematic
> rule the highest audible tone is `fCPU/(16*4)` — about 22.4 kHz NTSC / 20.8 kHz PAL
> — as against about 12.4 kHz / 11.5 kHz for the community rule. This specification
> keeps the schematic value; see *Open questions*.

The `SW_UVF`/`SW_OVF` mutes are computed from the current frequency register value
rather than stored, so writing a period that clears the condition restores the channel.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | A note stays silent although `$4000`/`$4002` were written | the length counter holds 0 or the channel is disabled in `$4015` | write `$4015` bit n = 1, then a length register to sound the note |
| 2 | A note plays one decay and stops; retriggering needs a length write | envelope loop flag (bit 5) clear: the decay level stops at 0 | set bit 5 for a repeating decay, or retrigger with `$4003` |
| 3 | Writing `$4000` mid-note changes the timbre without a click | the duty counter phase is not reset by `$4000` | use a `$4003` retrigger when a phase reset is wanted |
| 4 | Two channels set up identically do not sweep identically | Pulse 1 subtracts `(Fx >> S) + 1`, Pulse 2 subtracts `(Fx >> S)` | implement the two carry-in modes separately |
| 5 | A pitch slide steps every `P + 1` half frames and its timing restarts when `$4001` is written | the sweep divider reloads on borrow, and a `$4001` write reloads it at the next half frame | do not apply the target period immediately |
| 6 | An upward sweep suddenly mutes the channel | target period exceeded `$7FF` (`SW_OVF`) | the mute stays until a smaller period is written |
| 7 | A very short period (`t < 4`) is silent here but audible on some emulators | `SW_UVF` mute at `t < 4`; the community reference mutes at `t < 8` | follow the schematic (discrepancy in §7) |
| 8 | A note written with halt (bit 5) + constant volume never decays and never stops | halt stops the length counter; constant volume bypasses the envelope | this is the standard sustained-tone setting |
| 9 | A retriggered note starts at full volume regardless of `V` | the start flag loads the decay level with 15 at the next quarter frame | account for the one-quarter-frame start latency |
| 10 | `$4015` bit n = 0 silences a channel instantly and the previous length is lost | the state flip-flop forces `NotCount` | re-enable and write a length register to resume |

## Programming / implementation notes

- The timer MUST count `t, t-1, …, 0, t` at one step per APU cycle and MUST advance the sequencer on reload.
- The duty table MUST be a function of a 3-bit down counter; the phase reset comes only from `$4003`/`$4007`.
- The envelope MUST restart on `$4003`/`$4007`, MUST take effect at the next quarter-frame clock, and MUST NOT restart on `$4000`.
- The envelope divider period MUST be `V + 1` quarter frames; the loop MUST reload the decay level with 15.
- The sweep MUST be enabled by `$4001`/`$4005` bit 7 = 1 and disabled by bit 7 = 0 (`SWDIS` is the inverted bit); with the sweep disabled no period update occurs, but the overflow/underflow mute still applies.
- The sweep MUST use the two different negate formulas and MUST suppress updates for all seven conditions in §5; `DO_SWEEP` MUST NOT reset the timer divider.
- The length counter MUST use the reload values in §6 and MUST silence the channel one half-frame after those reloads reach zero.
- A `$4015` write with a channel bit clear MUST close that channel's gate immediately; setting the bit MUST have no immediate counter effect.
- Region-dependent frequencies MUST be computed from `fCPU` (NTSC 1.7897727 MHz, PAL 1.662607 MHz), never from a single constant; an emulator SHOULD keep `SW_UVF`/`SW_OVF` separate from the duty gate so debug output matches the schematic.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Channel block structure, timer, sequencer, output muting threshold | `APU_Pulse` |
| Duty cycle sequences and the downward sequencer counter | `APU_Pulse` |
| Envelope generator: start/loop flags, decay level, constant volume | `APU_Envelope` |
| Sweep divider, target period, negate difference, sweep muting | `APU_Sweep` |
| Length counter table, halt flag, `$4015` enable/disable | `APU_Length_Counter` |
| `$4000`–`$4007` register layout and bit fields | `APU_registers` |
| Quarter-frame and half-frame clocks | `APU_Frame_Counter` |
| CPU clock rates (NTSC 2A03, PAL 2A07, Dendy) | `APU_Pulse`, `Cycle_reference_chart` |
| 2A03 test registers `$4018`–`$401A` and `LOCK` | `CPU_pinout` |
| Pulse output contribution to the non-linear mixer | `APU_Mixer` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Channel block structure, duty principle | `APU/square.md`, `APU/sound.md` | `APU/square.v` |
| Register bit fields and write strobes | `APU/common.md`, `APU/regs.md` | `APU/regs.v`, `APU/square.v` |
| Timer, duty counter, duty table | `APU/square.md`, `APU/waves.md` | `SQUARE_FreqCounter`, `SQUARE_Duty` |
| Envelope divider/decay/loop/constant volume | `APU/square.md`, `APU/waves.md` | `APU/env.v` |
| Sweep conditions, negate, carry-in difference | `APU/square.md` | `APU/square.v`, `APU/apu_top.v` |
| Length counter reload table and counting | `APU/length.md`, `APU/waves.md` | `APU/length.v` |
| `$4015` enable/disable, halt gating | `APU/length.md`, `APU/regs.md` | `APU/length.v` |
| LFO1/LFO2 rates, quarter/half frame counts | `APU/clkgen.md` | `APU/clkgen.v` |
| Mute conditions (`SW_UVF`, `SW_OVF`, `NOSQ`) | `APU/square.md` | `SQUARE_Output` in `APU/square.v` |

Icarus checks made while compiling this section: `length_counter.v` (index 9 reloads 7, `NotCount` after 8 LFO2 clocks); a scratch `Envelope_Unit` bench (period `V + 1`, `$4003` restart to 15 mid-decay, loop only with bit 5 set).

## Open questions

- **Pulse mute threshold.** The schematic mutes a pulse channel at `t < 4` (`SW_UVF` = timer bits 10:2 all zero, `square.v`); NESdev documents `t < 8` (`APU_Pulse`, and the same "current period less than 8" mute under `APU_Sweep`). This specification keeps the schematic value and notes the difference in §7. Which threshold the production 2A03/2A07 dies use is not settled by the sources available here; an emulator that must match the community model should make the threshold configurable.
- **Sweep enable polarity and bit index.** `square.md` lists `SWDIS` as fed by register `WR1[3]`; the HDL feeds it from `$4001` bit 7 through an inverter (`swdis_reg .d(DB[7]) .nq(SWDIS)`, `square.v`), so bit 7 = 1 enables the sweep process. Icarus re-check: `RegisterBit` with `d = 1` produces `SWDIS = 0`. The wiki's bit index is treated as a typo; the HDL polarity is used here and it agrees with NESdev's "enabled flag".
- The first sweep update after a channel is enabled happens at the first half-frame borrow; whether the community model's "divider starts at `P`" is equivalent was not verified on hardware.
- **Dendy CPU clock.** Dendy (PAL famiclone, NTSC-like CPU:PPU ratio) is not covered by `APU/clkgen.md`. NESdev `Cycle_reference_chart` (and `APU_Pulse`) give `fCPU = 1.773448 MHz` for Dendy (26.601712 MHz ÷ 15) — a community value, **unconfirmed** by ranks 1–3; the platform overview should state which value it uses.
