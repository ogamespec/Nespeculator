# APU 03 — Noise channel

**Status:** consensus (round 2)
**Scope:** The 2A03/2A07 noise channel (Breaking NES Wiki: "noise generator",
`NOISE`): period timer, 15-bit pseudo-random LFSR, the NTSC and PAL period
tables, the envelope/length interaction and the `$400C`/`$400E`/`$400F`
interface. The shared envelope unit, the length counters and the frame counter
(`/LFO1`, `/LFO2`) are specified in `03-apu-pulse.md` and
`03-apu-frame-counter.md`; the DAC/mixer in `03-apu-mixer.md`.
**Primary sources:** `BreakingNESWiki/APU/noise.md`, `length.md`, `common.md`,
`clkgen.md`, `pal.md`, `waves.md`; `HDL/APU/noise.v`, `length.v`, `env.v`,
`common.v`; `Docs/NES_Famicom_Board_Revisions.md`.

## 1. Block structure

| Block | HDL | Function |
|---|---|---|
| Period register | `NOISE_FreqReg` | 4 bits written from `$400E` bits 3–0 |
| Mode register | `NOISE_RandomLFSR.rmod_reg` | 1 bit written from `$400E` bit 7 |
| Decoder PLA | `NOISE_Decoder1`, `NOISE_Decoder2` | 4-to-16 demultiplexer + NOR plane: period index → 11-bit counter load value `NNF` |
| Frequency counter | `NOISE_FreqLFSR` | 11-bit LFSR down-counter; reloads itself and emits `RSTEP` once per period |
| Random LFSR | `NOISE_RandomLFSR` | 15-bit shift register, tap selected by the mode bit, clocked by `RSTEP` |
| Envelope unit | `Envelope_Unit` (`HDL/APU/env.v`) | Shared with pulse 1/2 (`$400C` write port) |
| Noise length counter | `LengthCounter` (`HDL/APU/length.v`) | Shared logic, loaded by `$400F`, carry-in `RND_LC` |
| Output gate | `NoiseChan` | `RND_out = RNDOUT ? 0 : Vol` (4 bits) |

Clock domains: everything except the CPU-facing registers runs on `ACLK1` /
`/ACLK2` (APU cycles); `RSTEP` shifts the 15-bit LFSR; `nLFO1` (quarter-frame,
~240 Hz NTSC mode 0) clocks the envelope; `nLFO2` (half-frame, ~120 Hz) clocks
the length counter.

## 2. Register interface

`$400C` (write only) — noise control:

```
7  bit  0
---- ----
--LC VVVV
```

| Bit | Name | Meaning |
|---|---|---|
| 5 | `L` | Length counter halt (1 = stop) and envelope loop (1 = loop) |
| 4 | `C` | 0 = envelope decay volume, 1 = constant volume |
| 3–0 | `VVVV` | Constant volume, or envelope decay period (0 = fastest) |
| 7–6 | — | Unused |

`$400E` (write only) — noise period:

```
7  bit  0
---- ----
M--- PPPP
```

| Bit | Name | Meaning |
|---|---|---|
| 7 | `M` | 0 = 15-bit LFSR (white noise), 1 = 15-bit LFSR with tap bit 6 (short, tonal sequence) |
| 3–0 | `PPPP` | Period index 0–15 into the region table (§3) |
| 6–4 | — | Unused; only bit 7 and bits 3–0 are latched (`NOISE_FreqReg`, `rmod_reg`) |

`$400F` (write only) — length counter load:

```
7  bit  0
---- ----
llll L---
```

Bits 7–3 are a 5-bit index into the length table (§6); bits 2–0 are unused.
`$4015` write bit 3 enables the channel, `$4015` read bit 3 reads the length
counter status. All APU register writes are disabled during PHI1
(`HDL/APU/regs.v`: `W400x = nor(PHI1, ~pla[...])`), so a write lands in PHI2.

## 3. Period timer and the region tables

The frequency counter is an 11-bit LFSR **down-counter** (`NOISE_FreqLFSR`). It
is loaded with the value decoded from `$400E` bits 3–0, counts one step per APU
cycle (`ACLK2` phase), reloads itself when its value reaches `%000_0000_0001`,
and asserts `RSTEP = NFLOAD` on that reload. `RSTEP` shifts the random LFSR by
one step, so one timer period = one LFSR shift. The all-zero state is escaped by
the feedback term (`NSIN`), so the counter can never lock up.

Timer period in CPU cycles = 2 × APU cycles (ACLK = PHI0/2 on both 2A03 and
2A07; `APU/clkgen.md`: 2A03 `/CLK` 46.56 ns, PHI 558.73 ns, ACLK 1117.46 ns;
2A07 `/CLK` 37.59 ns, PHI 601.46 ns, ACLK 1202.93 ns). NTSC CPU 1.7897727 MHz,
PAL CPU 1.662607 MHz.

| Index | NTSC period (CPU cycles) | NTSC LFSR step rate | PAL period (CPU cycles) | PAL LFSR step rate |
|---|---|---|---|---|
| 0 | 4 | 447.4 kHz | 4 | 415.7 kHz |
| 1 | 8 | 223.7 kHz | 8 | 207.8 kHz |
| 2 | 16 | 111.9 kHz | 14 | 118.8 kHz |
| 3 | 32 | 55.93 kHz | 30 | 55.42 kHz |
| 4 | 64 | 27.97 kHz | 60 | 27.71 kHz |
| 5 | 96 | 18.64 kHz | 88 | 18.89 kHz |
| 6 | 128 | 13.98 kHz | 118 | 14.09 kHz |
| 7 | 160 | 11.19 kHz | 148 | 11.23 kHz |
| 8 | 202 | 8.860 kHz | 188 | 8.844 kHz |
| 9 | 254 | 7.046 kHz | 236 | 7.045 kHz |
| 10 | 380 | 4.710 kHz | 354 | 4.697 kHz |
| 11 | 508 | 3.523 kHz | 472 | 3.522 kHz |
| 12 | 762 | 2.349 kHz | 708 | 2.348 kHz |
| 13 | 1016 | 1.762 kHz | 944 | 1.761 kHz |
| 14 | 2034 | 879.9 Hz | 1890 | 879.7 Hz |
| 15 | 4068 | 440.0 Hz | 3778 | 440.1 Hz |

The two region tables are produced by two **different decoder PLAs** on the
2A03 and the 2A07 (`APU/pal.md` "Noise Channel Decoder"), chosen to compensate
for the region clock ratio. The load values (11-bit `NNF`, bit *n* drives LFSR
stage *n*) are:

| Index | NTSC `NNF` | PAL (2A07) `NNF` | Index | NTSC `NNF` | PAL (2A07) `NNF` |
|---|---|---|---|---|---|
| 0 | `$002` | `$002` | 8 | `$1A9` | `$433` |
| 1 | `$00A` | `$00A` | 9 | `$106` | `$51C` |
| 2 | `$0AA` | `$055` | 10 | `$227` | `$3AB` |
| 3 | `$2BB` | `$55D` | 11 | `$062` | `$648` |
| 4 | `$139` | `$64E` | 12 | `$642` | `$3F9` |
| 5 | `$173` | `$797` | 13 | `$20F` | `$5A4` |
| 6 | `$063` | `$6C3` | 14 | `$300` | `$4F3` |
| 7 | `$067` | `$7C1` | 15 | `$140` | `$24E` |

Consequence: for most indices the PAL tick counts are reduced roughly in
proportion to the APU clock ratio (ACLK NTSC 894.886 kHz, PAL 831.303 kHz;
NTSC/PAL = 1.0765), so the absolute periods and pitches agree to within ~1 %
(index 5 is 1.3 %). The compensation is not complete: indices 0 and 1 keep the
NTSC tick count (4/8 CPU cycles), so the PAL pitch is 7.1 % **lower**, and
index 2 quantises to 14 CPU cycles, 6.2 % **higher**. Select the table by
region; do not assume the two tables agree at the low indices. (Percentages are
relative to the NTSC value. Derivation: the `NNF` tables above fed to
`NOISE_FreqLFSR` reproduce all 16 NTSC and all 16 PAL periods exactly.)

## 4. Random LFSR (the noise source)

15 stages `sout[14:0]`, no reset (power-on value undefined). On each `RSTEP`:

- feedback `RIN = sout[0] XOR sout[tap]`, where `tap = 1` for `$400E` bit 7 = 0
  (mode 0) and `tap = 6` for `$400E` bit 7 = 1 (mode 1);
- shift direction: `sout[i] := sout[i+1]` for *i* = 0…13, `sout[14] := RIN`;
- if all 15 stages are 0, `RIN` is forced to 1 (lock-up escape).

| `$400E` bit 7 | Tap | Sequence length | Character |
|---|---|---|---|
| 0 | bit 1 | 32767 steps | White noise (long sequence) |
| 1 | bit 6 | 93 steps | Short repeating sequence — buzzy tone |

The LFSR output used by the channel is stage 0 (`sout[0]`); at index 15 mode 0
a full sequence lasts 32767 × 4068 CPU cycles = 74.5 s (NTSC), so it is
effectively aperiodic for sound purposes. Mode 1 repeats every 93 × 4068 =
378324 CPU cycles = 211 ms (NTSC).

The mode bit (`rmod_reg`) is latched on a `$400E` write; the switch takes
effect at the next `RSTEP`, i.e. the next timer reload. In the HDL `mux_out` is
combinational off `rmod_q` (`noise.v`) and is sampled by `NOISE_RandomLFSRBit`
on `RSTEP`, so the first shift after the write already uses the new tap.

## 5. Output path

```
RNDOUT = (sout[0] OR NORND) AND NOT LOCK
RND    = RNDOUT ? %0000 : Vol
```

- `Vol` is the 4-bit envelope output: either the constant volume `$400C[3:0]`
  (`$400C[4]` = 1) or the decay-envelope level.
- `NORND` (length counter "not counting", active when the counter is 0 or the
  channel is disabled) forces the output to 0.
- `LOCK` (2A03 debug only; set by writing bit 7 of the test register `$401A`)
  freezes the output so that the test registers can read it (noise `RND[3:0]` =
  `$4019` bits 7–4). The 2A07 has no debug logic (`APU/pal.md`).
- The 4-bit result is DAC input `RND[3:0]`, which occupies `AUX_B[7:4]` of the
  DAC output (`DAC_Others`: `AUX_B = {DMC[6:0], RND[3:0], TRI[3:0]}`).

## 6. Envelope and length counter

The envelope is the same circuit as for pulse 1/2 (`APU/noise.md`: "the circuit
is identical to the square channels' envelope"), driven by `/LFO1`. Signal map:
`WR_Reg` = `$400C`, `WR_LC` = `$400F`, `LC` = `RND_LC` (carry-in to the length
counter).

Rules observed in the silicon:

- Writing `$400C` sets the constant-volume flag, the 4-bit volume/period, the
  envelope loop and the length halt bit only. It does **not** restart the
  envelope: no path from `WR_Reg` reaches the envelope reload flip-flop
  (`EnvReload` is reset by `WR_LC` only, `HDL/APU/env.v`).
- Writing `$400F` loads the length counter and also restarts the envelope (this
  is the only noise-channel write that does).
- `$400C` bit 5 is shared: 1 = envelope loop and length-counter halt.
- The length counter's carry-in is `RND_LC = NOT $400C[5]`, so with the halt bit
  set the counter's borrow chain is blocked and the counter does not decrement.
- Writing `$4015` bit 3 = 0 disables the length counter and silences the channel;
  writing 1 re-enables it, but the counter stays disabled until a `$400F` load.

Length counter load values (`LengthDecoder2`, input = `$400F` bits 7–3). The
value loaded is always odd; the channel counts down that many half-frames and
the borrow-out (silencing) occurs one half-frame later, so the audible length is
the community's table (load + 1):

| Index | Load | Audible half-frames | Index | Load | Audible half-frames |
|---|---|---|---|---|---|
| 0 | 9 | 10 | 16 | 11 | 12 |
| 1 | 253 | 254 | 17 | 15 | 16 |
| 2 | 19 | 20 | 18 | 23 | 24 |
| 3 | 1 | 2 | 19 | 17 | 18 |
| 4 | 39 | 40 | 20 | 47 | 48 |
| 5 | 3 | 4 | 21 | 19 | 20 |
| 6 | 79 | 80 | 22 | 95 | 96 |
| 7 | 5 | 6 | 23 | 21 | 22 |
| 8 | 159 | 160 | 24 | 191 | 192 |
| 9 | 7 | 8 | 25 | 23 | 24 |
| 10 | 59 | 60 | 26 | 71 | 72 |
| 11 | 9 | 10 | 27 | 25 | 26 |
| 12 | 13 | 14 | 28 | 15 | 16 |
| 13 | 11 | 12 | 29 | 27 | 28 |
| 14 | 25 | 26 | 30 | 31 | 32 |
| 15 | 13 | 14 | 31 | 29 | 30 |

The same 32 loads apply to pulse 1/2 and triangle (`APU/length.md`: all four
counters are identical); see `03-apu-pulse.md` for the half-frame clock timing.

## 7. Region and revision differences

| Part | Difference that affects the noise channel |
|---|---|
| 2A07 (PAL) | Different noise decoder PLA: all 16 periods except index 0/1 differ from NTSC (§3). The clock divider is ÷16 instead of ÷12 (`Docs/NES_Famicom_Board_Revisions.md`). Absolute pitches match NTSC within ~1 % except indices 0–2 (§3). No debug/test logic. |
| RP2A03 (letterless, 1983–1984) | **No tonal (looped) noise mode** — `$400E` bit 7 has no effect (mode 1 is not available); lowest noise period is 2046 CPU cycles instead of 4068. Frame counter is not restarted on reset. |
| Vs. System (letterless 2A03) | Additionally does not support looped noise. |
| RP2A03E onwards | Looped noise present; reference noise behaviour. |
| Dendy (famiclone) | PAL video with an NTSC-rate CPU (`Dendy.md`); the CPU is a UMC 6527 (RP2A03 clone), so the NTSC table and NTSC CPU clock apply. The wiki notes further undocumented APU differences in clones. |

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | Channel goes silent after its length expires even though `$400C` still holds a volume | `NORND` forces the LFSR output gate high when the length counter reaches 0 | Keep `$400C` bit 5 set (halt), or reload `$400F`; check `$4015` bit 3 to detect expiry |
| 2 | Index 15 gives a clean, low buzz; index 0 gives a high hiss | Timer period table §3; the LFSR step rate spans 440 Hz…447 kHz NTSC | Use index 0–5 for hiss/percussion, 10–15 for low rumble |
| 3 | `$400E` bit 7 = 1 does not sound like noise but a steady buzz | Mode-1 tap (bit 6) shortens the sequence to 93 steps | Use mode 1 for tonal/percussive effects; expect a fixed pitch tied to the period index |
| 4 | Changing `$400E` mid-playback does not change the pitch until one period later | The frequency counter reloads itself from the decoder output | Accept one-period latency; do not expect an immediate jump |
| 5 | Same `$400E` index sounds slightly different on PAL and NTSC | 2A07 uses a different decoder PLA; the CPU-cycle counts differ (§3) | Select the region table by console region, not a single table |
| 6 | Letterless Famicom/Vs. board ignores `$400E` bit 7 and index 15 is an octave higher | No looped-noise mode; noise period 15 is 2046 not 4068 CPU cycles | Detect the revision, or avoid mode 1 and index 15 for compatibility |
| 7 | Writing `$400C` with a new volume clicks | `Vol` feeds the DAC combinationally; the level changes on the write | Ramp the volume over frames if a click is unwanted |
| 8 | Writing `$4015` bit 3 = 0 silences the channel and a later write of 1 leaves it silent | The length counter is cleared to 0, `NORND` stays 1 | Reload `$400F` after re-enabling |
| 9 | `$400F` write restarts the envelope decay | `WR_LC` resets the envelope reload flip-flop | Use `$400F` writes for note retriggering |
| 10 | Power-on noise waveform differs between chips but is stable afterwards | The 15-bit LFSR has no reset; only the all-zero state is escaped | Do not rely on a defined power-on LFSR seed; it is not architecturally specified |

## Programming / implementation notes

- An emulator MUST implement the timer as a down-counter reloaded with the
  region table value, shifting the LFSR once per period.
- The LFSR MUST be a 15-bit shift register with the feedback tap selected by
  `$400E` bit 7 (bit 1 / bit 6) and the all-zero lock-up escape.
- `$400C` bits 7–6, `$400E` bits 6–4 and `$400F` bits 2–0 MUST be ignored.
- Emulators MUST silence the channel (output 0) when the length counter is 0 or
  `$4015` bit 3 is 0.
- The envelope MAY be shared with the pulse channels; if shared, `$400F` MUST
  restart it (see `03-apu-pulse.md`).
- Implementation order of the timer and the LFSR shift MUST match the region
  table exactly, or the output will be one period off.
- A region-select switch SHOULD choose between the NTSC and PAL period tables;
  a fixed NTSC table is a known PAL incompatibility.
- `$400E` bit 7 SHOULD default to mode 0 for maximum compatibility.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Noise channel registers and period table | `APU_Noise` |
| Envelope unit | `APU_Envelope` |
| Length counter, halt, reload | `APU_Length_Counter` |
| APU register map `$4000`–`$4017` | `APU_registers`, `APU` |
| Frame counter quarter/half-frame clocks | `APU_Frame_Counter` |
| DAC and non-linear mixing (`AUX_B`) | `APU_Mixer` |
| 2A03/2A07 revision differences | `CPU_variants` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Noise block list, register-bit ownership, envelope identity | `APU/noise.md` | `noise.v` |
| Envelope behaviour and `$400C`/`$400F` ports | `APU/noise.md`, `APU/common.md` | `env.v` |
| Decoder PLA, LFSR structure, mode tap, output gate | `APU/noise.md` | `noise.v` |
| NTSC period load values | `APU/noise.md` | `noise.v` |
| PAL (2A07) decoder PLA and clock divider | `APU/pal.md`, `APU/clkgen.md` | `noise.v` (shared counter/LFSR model) |
| Length counter load values and halt/carry-in | `APU/length.md` | `length.v` |
| Counter types (down counters) | `APU/common.md` | `common.v` |
| APU clock periods, LFO rates | `APU/clkgen.md`, `APU/waves.md` | `clkgen.v` |
| Noise timing diagrams | `APU/waves.md` | (test bench) |
| Revision-specific noise behaviour | `Docs/NES_Famicom_Board_Revisions.md` | — |

## Open questions

- The wiki prose does not print the noise period tables; §3's NTSC/PAL values
  were derived from the `noise.md` / `pal.md` PLA masks plus the
  `NOISE_FreqLFSR` model, and reproduce the community tables for all 32 entries.
  Should the derivation be re-run in CI?
- The letterless RP2A03's period-15 value (2046 CPU cycles) is quoted from
  `Docs/NES_Famicom_Board_Revisions.md`; the corresponding decoder mask is not
  given in the wiki, so the whole letterless period table is unverified.
- **Power-on random-LFSR state (NESdev vs schematic).** NESdev `APU_Noise`
  states the 15-bit shift register is loaded with the value 1 on power-up. The
  schematic gives it no reset (`NOISE_RandomLFSRBit` uses the plain
  `RegisterBit`, `noise.v`), so ranks 1–2 leave the seed undefined. §4 keeps the
  schematic position; an emulator that wants the community power-on waveform
  should seed with 1. Only all-zero is escaped, and hardware/emulator test
  evidence for the real power-on seed is still missing.
- **Mode-1 sequence length (NESdev vs schematic).** NESdev `APU_Noise` says mode
  1 produces a **93- or 31-step** sequence, depending on the shift-register state
  when bit 7 is set; §4 quotes 93 steps. Simulating the HDL feedback
  (`RIN = sout[0] XOR sout[6]`) gives a 93-step cycle from every non-zero state
  except one 31-state cycle, so both figures are real and the 93-step value
  holds for the community power-up seed 1.
- Mode 1 on the 2A07 is not separately documented; `pal.md` shows a change to
  the period PLA only, and the tap-selection logic of `NOISE_RandomLFSR` appears
  identical.
