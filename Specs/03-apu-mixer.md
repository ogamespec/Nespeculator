# APU — Mixer and DAC outputs

**Status:** consensus (round 2)
**Scope:** The APU's two digital-to-analog outputs (AUX A for the pulse channels, AUX B for
triangle/noise/DMC): the digital codes each DAC receives, the binary-weighted resistor network and the
resulting non-linear transfer curve, the output pins, and the board-level load. Downstream board
mixing/filtering is treated only as far as the community documentation describes it.
**Primary sources:** `BreakingNESWiki/APU/dac.md`, `.../pads.md`, `.../sound.md`, `.../common.md`,
`.../regs.md`, `.../pal.md`, `.../Readme.md`; `HDL/APU/dac.v`, `HDL/APU/apu_top.v`, `HDL/APU/pads.v`,
`HDL/APU/regs.v`; `Scripts/APU_DAC_Generator.py`, `Scripts/Readme.md`;
`HDL/Framework/Icarus/apu/aux_test.v`, `HDL/Framework/Icarus/apu/auxout.v`,
`HDL/Framework/Icarus/apu/auxa.mem`, `HDL/Framework/Icarus/apu/auxb.mem`.

## 1. Two DACs, two pins

The APU (2A03/2A07) does not produce a digital audio word. It drives two analog current-summing
networks whose outputs leave the chip on dedicated pins; the board sums them into the audio output.

| Pin | Die net (NESdev pin name) | Direction | Digital input bus | Contents |
|---|---|---|---|---|
| 1 | AUX A (NESdev `AD1`) | APU → | `AUX_A[7:0]` | `{SQB[3:0], SQA[3:0]}` — Pulse 2 (Square 1) and Pulse 1 (Square 0) |
| 2 | AUX B (NESdev `AD2`) | APU → | `AUX_B[14:0]` | `{DMC[6:0], RND[3:0], TRI[3:0]}` — DMC (DPCM), Noise, Triangle |

The die-level pad names are AUX A / AUX B (`APU/pads.md`); NESdev names the package pins `AD1` (audio
out, both pulse waves) and `AD2` (audio out, triangle, noise and DPCM).

The two bargraph codes are formed in `HDL/APU/dac.v` (`DAC_Square`, `DAC_Others`). Each channel's
4-bit (DMC: 7-bit) output is the volume/envelope-controlled amplitude of that channel; the DAC pins
carry the *summed current* of the enabled stages.

The DACs share the chip's normal VDD/GND (no separate analog supply pins); the DAC power routing uses
a star topology (`dac.md`).

## 2. Digital code weights

The network is binary-weighted. Each digital stage switches one MOSFET whose on-resistance is
`39 kΩ / weight`; all enabled stages conduct into the same external load, so the pin sees the sum of
the stage conductances (`Scripts/APU_DAC_Generator.py`).

| Bus | Bit *i* | Conductance weight (× 1/39 kΩ) |
|---|---|---|
| `SQA[3:0]` | i | 2^i |
| `SQB[3:0]` | i | 2^i |
| `TRI[3:0]` | i | 2^(i+1) |
| `RND[3:0]` | i | 2^i |
| `DMC[6:0]` | i | 2^(i-1) |

This collapses the two 8-bit / 15-bit input vectors onto a single conductance value:

```
g_A = SQA + SQB                         (0 … 30)
g_B = 4*TRI + 2*RND + DMC               (0 … 217)
```

The per-bit table is stated in `1/39 kΩ` units, which are the AUX A units: its AUX A rows sum to
`SQA + SQB = g_A`. `g_B` is instead scaled ×2, so that `R_int = 78 kΩ / g_B` holds for AUX B (§3); an
implementer who combines the AUX B rows with `39 kΩ` gets a factor-of-2 error.

The collapse is exact for the repository's DAC tables: `auxa.mem` is constant for a given `SQA+SQB`,
and `auxb.mem` is constant for a given `4*TRI + 2*RND + DMC` (verified for all 256 / 32768 entries).

> NESdev says the triangle : noise : DMC per-LSB weights are 3 : 2 : 1 (its `tnd_table` is indexed by
> `3*triangle + 2*noise + dmc`); this repository's DAC model gives 4 : 2 : 1
> (`Scripts/APU_DAC_Generator.py`: `tri_reciprocal = [2, 4, 8, 16]`, `rnd_reciprocal = [1, 2, 4, 8]`,
> `dmc_reciprocal = [0.5, 1, 2, 4, 8, 16, 32]`). This spec uses the repository's 4 : 2 : 1; the
> difference is kept in Open questions.

## 3. Transfer curve

The internal conductance works against the external pull-down resistor on the motherboard
(`ExtRes`, 100 Ω nominal on each AUX pin):

```
R_int = 39 kΩ / g_A          (AUX A)
R_int = 78 kΩ / g_B          (AUX B)
V_pin = 5.0 V * ExtRes / (R_int + ExtRes)
```

With `g = 0` all stages are off; the model uses an effectively open internal resistance
(`IntUnloaded = 999999 Ω`), giving ≈ 0.5 mV rather than exactly 0 V.

The curve is concave: each additional unit of *g* contributes less than the previous one. Because the
same load resistor is shared, the channels inside one group interact — the pin voltage is a
non-linear function of the weighted sum, not a sum of independent channel voltages. This is why
writing `$4011` (the DMC level) changes the DC level seen by the triangle and noise channels.

Full-scale values with the nominal 100 Ω load:

| DAC | Max code | Max `V_pin` | Min `V_pin` |
|---|---|---|---|
| AUX A | `SQA = SQB = 15` | 357.14 mV | 0.50 mV |
| AUX B | `DMC = 127, RND = TRI = 15` | 1088.26 mV | 0.50 mV |

`dac.md` measures 272 mV maximum on AUX A with a 75 Ω external resistor (RP2A03G, `dac_square.nes`,
`SQA = SQB = 15`); the model above gives 272.7 mV for 75 Ω, which confirms the network model.

## 4. AUX A transfer table (Pulse 1 + Pulse 2)

The table is indexed by `g_A = SQA + SQB`. The "normalized" column is the value stored in
`HDL/Framework/Icarus/apu/auxa.mem`, i.e. `(V/V_max − 0.5) × 2`, in the range −0.9972 … +1.0000.

| `SQA+SQB` | mV (100 Ω) | Normalized | | `SQA+SQB` | mV (100 Ω) | Normalized |
|---|---|---|---|---|---|---|
| 0 | 0.50 | −0.9972 | | 16 | 197.04 | +0.1034 |
| 1 | 12.79 | −0.9284 | | 17 | 208.85 | +0.1695 |
| 2 | 25.51 | −0.8571 | | 18 | 220.59 | +0.2353 |
| 3 | 38.17 | −0.7863 | | 19 | 232.27 | +0.3007 |
| 4 | 50.76 | −0.7157 | | 20 | 243.90 | +0.3659 |
| 5 | 63.29 | −0.6456 | | 21 | 255.47 | +0.4307 |
| 6 | 75.76 | −0.5758 | | 22 | 266.99 | +0.4951 |
| 7 | 88.16 | −0.5063 | | 23 | 278.45 | +0.5593 |
| 8 | 100.50 | −0.4372 | | 24 | 289.86 | +0.6232 |
| 9 | 112.78 | −0.3684 | | 25 | 301.20 | +0.6867 |
| 10 | 125.00 | −0.3000 | | 26 | 312.50 | +0.7500 |
| 11 | 137.16 | −0.2319 | | 27 | 323.74 | +0.8129 |
| 12 | 149.25 | −0.1642 | | 28 | 334.93 | +0.8756 |
| 13 | 161.29 | −0.0968 | | 29 | 346.06 | +0.9379 |
| 14 | 173.27 | −0.0297 | | 30 | 357.14 | +1.0000 |
| 15 | 185.19 | +0.0370 | | | | |

Step size falls from 12.29 mV (0 → 1) to 11.08 mV (29 → 30): the pulse pair is *not* linear in the sum.

## 5. AUX B transfer table (triangle + noise + DMC)

The table is indexed by `g_B = 4*TRI + 2*RND + DMC` and has 218 entries; `auxb.mem` holds them
normalized to −0.9991 … +1.0000. Representative entries (100 Ω load):

| `g_B` | mV | Norm. | `g_B` | mV | Norm. | `g_B` | mV | Norm. |
|---|---|---|---|---|---|---|---|---|
| 0 | 0.50 | −0.9991 | 64 | 379.15 | −0.3032 | 160 | 851.06 | +0.5641 |
| 1 | 6.40 | −0.9882 | 72 | 422.54 | −0.2235 | 168 | 886.08 | +0.6284 |
| 2 | 12.79 | −0.9765 | 80 | 465.12 | −0.1452 | 176 | 920.50 | +0.6917 |
| 4 | 25.51 | −0.9531 | 88 | 506.91 | −0.0684 | 184 | 954.36 | +0.7539 |
| 8 | 50.76 | −0.9067 | 96 | 547.95 | +0.0070 | 192 | 987.65 | +0.8151 |
| 16 | 100.50 | −0.8153 | 104 | 588.24 | +0.0811 | 200 | 1020.41 | +0.8753 |
| 24 | 149.25 | −0.7257 | 112 | 627.80 | +0.1538 | 208 | 1052.63 | +0.9345 |
| 32 | 197.04 | −0.6379 | 120 | 666.67 | +0.2252 | 216 | 1084.34 | +0.9928 |
| 40 | 243.90 | −0.5518 | 128 | 704.85 | +0.2954 | 217 | 1088.26 | +1.0000 |
| 48 | 289.86 | −0.4673 | 136 | 742.36 | +0.3643 | | | |
| 56 | 334.93 | −0.3845 | 152 | 815.45 | +0.4986 | | | |

Per-LSB weight ratios inside AUX B are triangle : noise : DMC = 4 : 2 : 1 g-units, so near zero:
- one triangle LSB (4 g) ≈ 25 mV, one noise LSB (2 g) ≈ 12 mV, one DMC LSB (1 g) ≈ 6 mV;
- DMC reaches full scale over 127 steps and therefore dominates the group's loudest output;
- the triangle has only 15 steps but is the loudest *per step*.

The complete 218-entry curve is regenerated by `Scripts/APU_DAC_Generator.py` (which also writes
`Scripts/auxa.csv`, `Scripts/auxb.csv`, `auxa.mem`, `auxb.mem`, `auxa.hex`, `auxb.hex`).

## 6. Pins and board network

- Both AUX pads are plain pads inside the chip; nothing internal loads them (`dac.md`).
- The motherboard places a small pull-down resistor to GND on each AUX pin, typically 100 Ω.
- The pin is therefore a current output whose voltage depends on the external resistor: the same
  digital code gives a different level on a board with a different resistor.
- AUX A and AUX B are separate pins; the APU never sums them internally.

## 7. What the APU does not do

- There is no on-chip summing of AUX A and AUX B. The character of the mixed output (and the
  high-pass/low-pass filter chain) lives on the board, downstream of these pins.
- `dac.md` describes the AUX B (triangle/noise/DMC) transistor network as "TBD"; only the resistor
  model in `Scripts/APU_DAC_Generator.py` and the normalized `auxb.mem` table exist for it.
- The DAC output is a level, not a stream: there is no sample rate, only the channel update rates.

## 8. Region differences (2A07)

- The repository's DAC model, tables and generator are region-independent: the same `IntRes = 39 kΩ`,
  `ExtRes = 100 Ω`, `Vdd = 5 V` and the same `auxa.mem` / `auxb.mem` are used for NTSC and PAL.
- What does differ on the 2A07 (`pal.md`): the master clock (26.601712 MHz), the SoftCLK PLA, the DPCM
  and noise decoders, the shortened register decoder and the absence of the debug registers. None of
  these is inside the DAC network, but they change the *rate* at which the DAC codes are updated.
- No source in this repository documents a 2A07 DAC resistor or level difference (see Open questions).

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | Two pulse channels at volume 15 do not give twice the voltage of one | the shared load makes `V` concave in `SQA+SQB` | use the `g_A` table, not a linear sum |
| 2 | The first pulse step (12.29 mV) is larger than the last (11.08 mV) | the shared load makes `V` concave in `SQA+SQB` | emulate the divider, not `k × sum` |
| 3 | Writing `$4011` shifts the level of triangle and noise | DMC, noise and triangle share one node and one load | model `$4011` as a DC offset on AUX B |
| 4 | DMC samples sound louder than the triangle at full scale | DMC has 7 bits (127 steps) on the same network | weight triangle : noise : DMC = 4 : 2 : 1 |
| 5 | Changing the AUX pull-down resistor changes the level | the pin is a current output into the board resistor | state the external resistor when quoting mV |
| 6 | The same code measures ~272 mV on a 75 Ω board and ~357 mV on a 100 Ω board | `V = 5 V × R_ext/(R_int+R_ext)` | parameterise the load, do not hard-code a voltage |
| 7 | Code 0 is not exactly 0 V | the off-state internal resistance is finite (~1 MΩ) | expect a small DC offset at rest |
| 8 | Ultrasonic channel periods average to a mid-level DC value rather than a tone | the DAC plus downstream filters act as a low-pass | emulate the channel at its own rate and let the filter average it |
| 9 | The APU has no audio input pin; only AUX A and AUX B leave the chip | the two DAC pins are the only analog outputs in the 40-pin package | mix AUX A and AUX B (and any cartridge audio) at board level |
| 10 | The two AUX pins must be scaled differently when summed | AUX B's full scale (1088 mV) is ~3× AUX A's (357 mV) | apply separate gains before adding the two outputs |

## Programming / implementation notes

- An emulator MUST compute each channel's digital code first (pulse 0–15, triangle 0–15, noise 0–15,
  DMC 0–127) and MUST combine them as `g_A = pulse1 + pulse2` and
  `g_B = 4*triangle + 2*noise + dmc`.
- The pin voltages MUST be derived from the weighted sum; adding per-channel voltages is wrong for
  this hardware.
- A 31-entry AUX A table and a 218-entry AUX B table reproduce the repository's exact curves; the
  tables MUST be indexed by `g_A` and `g_B` respectively.
- For cross-checking only, `APU_Mixer` publishes a 203-entry `tnd_table` indexed by
  `3*triangle + 2*noise + dmc` (within 4% of the exact formula on that page). It uses the NESdev
  3 : 2 : 1 triangle weight and MUST NOT be mixed with the 4 : 2 : 1 model above (see Open
  questions).
- Implementations SHOULD apply the board chain after the DACs (NES: high-pass at 90 Hz, high-pass at
  440 Hz, low-pass at 14 kHz; Famicom: high-pass at 37 Hz) — these are board components, not on-chip.
- `$4011` (DMC level) MUST be applied to the AUX B code even when no sample is playing; it is a
  documented crude volume control for triangle and noise.
- Tools that report "volume" SHOULD quote the digital code as well as millivolts, since the millivolt
  value depends on the external resistor.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Non-linear mixing formulas and lookup tables | `APU_Mixer` |
| Channel output values fed to the DACs | `APU_Pulse`, `APU_Triangle`, `APU_Noise`, `APU_DMC` |
| `$4011` direct DMC level load | `APU_DMC` |
| Output filter chain after the DACs (NES vs Famicom) | `APU_Mixer` |
| CPU/APU package pins (`AD1`/`AD2`) | `CPU_pinout` |
| APU register map | `APU_registers` |
| 2A07 (PAL) region differences relevant to the DAC output | `CPU_variants`, `Cycle_reference_chart` |

## Provenance

| Claim area | BreakingNES Wiki | HDL / repo |
|---|---|---|
| AUX A/B pins and contents | `APU/pads.md`, `APU/dac.md` | `HDL/APU/apu_top.v`, `HDL/APU/dac.v` |
| Channel output words (4/4/4/7 bit) | `APU/regs.md`, `APU/sound.md` | `HDL/APU/dac.v`, `HDL/APU/regs.v` |
| Resistor network, weights, 39 kΩ / 100 Ω / 5 V | `APU/dac.md`, `Scripts/Readme.md` | `Scripts/APU_DAC_Generator.py` |
| Normalized transfer tables | — | `HDL/Framework/Icarus/apu/auxout.v`, `auxa.mem`, `auxb.mem` |
| Measured 272 mV max on AUX A (75 Ω) | `APU/dac.md` | — |
| Star-topology DAC supply | `APU/dac.md` | — |
| AUX B network "TBD" | `APU/dac.md` | — |
| 2A07 differences relevant to output rate | `APU/pal.md` | `HDL/APU/clkgen.v` |

## Open questions

- The assignment brief mentions an `/AUX` pin, but `APU/pads.md` documents only AUX A (pin 1) and
  AUX B (pin 2). No `/AUX` net or pin exists in this repository's APU sources — the reference needs
  clarification.
- Per-LSB weights inside AUX B: this spec uses the repository's **4 : 2 : 1** (triangle : noise : DMC)
  from `Scripts/APU_DAC_Generator.py`; NESdev's `APU_Mixer` lookup table uses **3 : 2 : 1**
  (`tnd_table` indexed by `3*triangle + 2*noise + dmc`). Both values are kept. Which matches silicon
  is unresolved: `Scripts/Readme.md` warns that the model uses a simplified FET with linear
  resistance, so the 4 : 2 : 1 ratio may be a model artefact.
- The board-level mixing network that sums AUX A and AUX B (gains, filters) is not described in the
  repository; `dac.md` marks the AUX B network "TBD". The filter figures quoted in
  *Programming / implementation notes* come from NESdev, not from this repository.
- `ExtRes`: `dac.md` says 100 Ω is "usual", while its own measurement used 75 Ω. Whether the real
  board value is 100 Ω or varies by board revision is unresolved.
- `IntRes = 39 kΩ` is a single lumped value for a MOSFET operated in the linear region; the true
  device has a cubic V–I relationship (`Scripts/Readme.md`). The error at high codes is unknown.
- No 2A07 (PAL) DAC or level difference is documented; the tables are reused unchanged. Whether the
  2A07 die has different DAC transistor sizes is unconfirmed.
