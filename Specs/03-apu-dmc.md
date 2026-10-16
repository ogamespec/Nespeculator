# APU 03 — DMC (delta modulation channel)

**Status:** consensus (round 2)
**Scope:** The 2A03/2A07 DMC channel (Breaking NES Wiki: DPCM, delta modulation,
`DPCM`): registers `$4010`–`$4013`, the `$4015` enable/status/IRQ bits, the
7-bit output unit, the regional rate tables, the sample address/length formulas,
the memory reader (byte pipeline) and the APU-side DMA arbitration. The
CPU-visible cycle cost of DMC DMA, the `$2007`/`$4016` conflicts and the RDY
handshake timing are in `02-cpu-dma.md`; the DAC/mixer in `03-apu-mixer.md`.
**Primary sources:** `BreakingNESWiki/APU/dpcm.md`, `dma.md`, `dac.md`,
`common.md`, `clkgen.md`, `pal.md`, `waves.md`; `HDL/APU/dpcm.v`, `dac.v`,
`common.v`, `dma.v`, `regs.v`; `Docs/NES_Famicom_Board_Revisions.md`.

## 1. Block structure

| Block | HDL | Function |
|---|---|---|
| Control register `$4010` and control logic | `DPCM_ControlReg`, `DPCM_Control` and sub-blocks | 4-bit rate index `Fx`, loop bit, IRQ-enable bit; emits `DSLOAD`, `DSSTEP`, `BLOAD`, `BSTEP`, `NSTEP`, `DSTEP`, `PCM` |
| Rate decoder PLA | `DPCM_Decoder1`, `DPCM_Decoder2` | 4-to-16 demultiplexer + NOR plane: rate index → 9-bit counter load `FR` |
| Rate counter | `DPCM_FreqLFSR` | 9-bit LFSR down-counter; reloads itself and emits `DFLOAD` once per bit period |
| Sample counter | `DPCM_SampleCounter` | 12-bit down-counter, loaded with `$4013` × 16; `SOUT` = finished |
| Sample buffer | `DPCM_SampleBuffer`, `DPCM_SampleBitCounter` | 8-bit register (`PCM` load) + shift register (`BLOAD`/`BSTEP`); 3-bit bit counter, `n_NOUT` = byte exhausted |
| Address register/counter | `DPCM_AddressReg`, `DPCM_AddressCounter` | `$4012`; 15-bit up-counter with `DMC_A15` tied to Vdd |
| Output unit | `DPCM_Output` | 6-bit reversible counter + 1-bit register → 7-bit DAC value |

Unlike the sprite DMA, the DMC has **its own** sample buffer and only takes over
DMA control with `RUNDMC` (`APU/dpcm.md`); all its counters run on `ACLK1`/`ACLK2`,
with `PHI1` used only to identify a read cycle and to latch the fetched byte.

## 2. Register interface

`$4010` (write only) — control:

```
7  bit  0
---- ----
IL-- RRRR
```

| Bit | Name | Meaning |
|---|---|---|
| 7 | `I` | 1 = DMC IRQ enabled, 0 = IRQ disabled and the IRQ flag is held cleared |
| 6 | `L` | 1 = loop the sample |
| 3–0 | `RRRR` | Rate index 0–15 into the region table (§3) |
| 5–4 | — | Unused |

`$4011` (write only) — direct load of the 7-bit output level:

```
7  bit  0
---- ----
-DDD DDDD
```

Bit 7 is ignored; bits 6–0 are loaded immediately into the output counter.
`$4012` (write only) = sample address `AAAAAAAA`; `$4013` (write only) = sample
length `LLLLLLLL`.

`$4015` (read/write):

```
7  bit  0
---- ----
IF-D NT21
```

Read: bit 7 = DMC IRQ flag, bit 6 = frame IRQ flag, bit 5 = open bus, bit 4 =
DMC active (enable flip-flop set; cleared when the sample ends or bit 4 is
written 0), bits 3–0 = length-counter status of noise/triangle/pulse 2/pulse 1.
Write: `---D NT21`, bit 4 = DMC enable, bits 3–0 = channel enables; **any**
write to `$4015` clears the DMC IRQ flag.

## 3. Rate table

The rate counter is a 9-bit LFSR **down-counter** (`DPCM_FreqLFSR`). It is
loaded from the region decoder PLA, steps once per APU cycle (`ACLK2` phase),
reloads itself when its value reaches `%0_0000_0001` and asserts `DFLOAD` on the
reload. One `DFLOAD` shifts one sample bit out of the buffer. Period in CPU
cycles = 2 × APU cycles (ACLK = PHI0/2 on both 2A03 and 2A07; `APU/clkgen.md`).
NTSC CPU 1.7897727 MHz, PAL CPU 1.662607 MHz.

| Index | NTSC period (CPU cycles) | NTSC bit rate | PAL period (CPU cycles) | PAL bit rate |
|---|---|---|---|---|
| 0 | 428 | 4181.7 Hz | 398 | 4177.4 Hz |
| 1 | 380 | 4709.9 Hz | 354 | 4696.6 Hz |
| 2 | 340 | 5264.0 Hz | 316 | 5261.4 Hz |
| 3 | 320 | 5593.0 Hz | 298 | 5579.2 Hz |
| 4 | 286 | 6258.0 Hz | 276 | 6023.9 Hz |
| 5 | 254 | 7046.4 Hz | 236 | 7044.9 Hz |
| 6 | 226 | 7919.4 Hz | 210 | 7917.2 Hz |
| 7 | 214 | 8363.4 Hz | 198 | 8397.0 Hz |
| 8 | 190 | 9419.9 Hz | 176 | 9446.6 Hz |
| 9 | 160 | 11186.1 Hz | 148 | 11233.8 Hz |
| 10 | 142 | 12604.0 Hz | 132 | 12595.5 Hz |
| 11 | 128 | 13982.6 Hz | 118 | 14089.9 Hz |
| 12 | 106 | 16884.7 Hz | 98 | 16965.4 Hz |
| 13 | 84 | 21306.8 Hz | 78 | 21315.5 Hz |
| 14 | 72 | 24858.0 Hz | 66 | 25191.0 Hz |
| 15 | 54 | 33143.9 Hz | 50 | 33252.1 Hz |

The NTSC and PAL tables differ because the 2A07 has a **different rate decoder
PLA** (`APU/pal.md` "DPCM Decoder") that must compensate for the clock ratio
(894.886 kHz / 831.303 kHz = 1.0765): the PAL load values are the NTSC values
scaled by that ratio and rounded to the nearest achievable counter value (most
entries are nearest-integer: 380 / 1.0765 = 353.0 → 354, 397.6 → 398, 176.5 →
176, 50.2 → 50), so absolute bit rates are almost identical on both regions.
Index 4 is the exception: PAL uses 276 CPU cycles (6023.9 Hz) where the ratio
predicts ≈ 266, and `APU_DMC` flags PAL `$4` (and `$C`) as off the intended
tuning. Load values (9-bit `FR`):

| Index | NTSC `FR` | PAL (2A07) `FR` | Index | NTSC `FR` | PAL (2A07) `FR` |
|---|---|---|---|---|---|
| 0 | `$173` | `$1D7` | 8 | `$009` | `$0A4` |
| 1 | `$08A` | `$1CC` | 9 | `$0DC` | `$18B` |
| 2 | `$143` | `$136` | 10 | `$0F1` | `$1E7` |
| 3 | `$0DB` | `$185` | 11 | `$0F9` | `$167` |
| 4 | `$1EE` | `$10F` | 12 | `$08D` | `$0A8` |
| 5 | `$03F` | `$1BC` | 13 | `$189` | `$071` |
| 6 | `$07D` | `$074` | 14 | `$18E` | `$1B1` |
| 7 | `$1D1` | `$099` | 15 | `$156` | `$1D5` |

(The NTSC/PAL tables above are the `FR` values fed to `DPCM_FreqLFSR`; they
reproduce the community NTSC and PAL DMC period tables for all 32 entries.)

## 4. Sample address and length

- Address: `$4012` = `A`; the address counter loads `$C000 + $40*A` (HDL: low
  byte = `{DPA[1:0], 6'b0}`, high byte = `{1'b1, DPA[7:2]}`). `$00` → `$C000`,
  `$FF` → `$FFC0`. The counter is 15 bits with `DMC_A15` tied to Vdd
  (`APU/dpcm.md`), so reads stay in `$8000–$FFFF` and wrap from `$FFFF` to
  `$8000` for as long as bytes remain.
- Length: `$4013` = `L`; the number of bytes played is `$10*L + 1`
  (community/NESdev formula). HDL: `DPCM_SampleCounter` is loaded with
  `{DSC[7:0], 4'b0000}` = `16*L`, and `SOUT` ("counter empty") is sampled by
  the next `ACLK1` phase, after the byte-complete pulse that emptied it, so
  playback ends one byte later — hence `16*L + 1` bytes.
- Both counters are loaded together by `DSLOAD`, which is asserted on the
  `$4015` bit 4 enable transition and on a loop wrap. `DSSTEP` (byte consumed)
  decrements the sample counter and increments the address counter.

## 5. Output unit (7-bit DAC value)

`DPCM_Output` = 6-bit reversible counter (`DMC_Out[6:1]`) + 1-bit register
(`DMC_Out[0]`):

- `DSTEP` (one per `DFLOAD`) increments the counter when the shifted-out sample
  bit is 1 and decrements it when the bit is 0 (`CountDown = n_BOUT`).
- The counter is a binary up/down counter, so one sample bit changes the 7-bit
  level by ±2. The LSB (`DMC_Out[0]`) is written only by `$4011` and is
  **never** changed by playback.
- The reversible counter saturates: `DOUT` (carry/borrow out of the 6-bit
  counter) inhibits further `DSTEP`, so the level clamps to 0 or 126 when the
  LSB is 0, and to 1 or 127 when the LSB is 1.
- Bits are shifted out **LSB first**: the buffer shift register shifts toward
  bit 0 (`sin = {1'b0, sout[7:1]}`) and `n_BOUT = NOT sout[0]` is used by the
  next `DSTEP`.
- `$4011` loads all 7 bits immediately and regardless of playback (bit 7 is
  ignored), and when the sample ends or the channel is disabled the level is
  **held** — nothing resets the counter except the chip reset.

The 7-bit result is the DAC input `DMC[6:0]`, occupying `AUX_B[14:8]`
(`AUX_B = {DMC[6:0], RND[3:0], TRI[3:0]}`); `$4011` = `$80`/`$00` silences it.

## 6. Memory reader (byte pipeline)

| Event | Signals | Effect |
|---|---|---|
| Bit period elapsed | `DFLOAD` | `BSTEP` shifts one bit out of the buffer; `DSTEP` steps the output ±2; `NSTEP` increments the sample bit counter |
| Byte exhausted (8th bit) | `n_NOUT` | The shift register is reloaded from the 8-bit sample buffer (`BLOAD`); `DSSTEP` decrements the sample counter and increments the address counter |
| Buffer empty and bytes remain | `PCM` (load request) | A DMA fetch is requested; the fetched byte is latched into the 8-bit sample buffer during PHI2 while the DMC owns the address bus (`PCM = NOT PHI1 AND DMC_AB`) |
| Enable transition / loop wrap | `DSLOAD` | Sample counter and address counter reloaded from `$4013`/`$4012` |
| Sample counter reaches 0 | `SOUT` | Latched; with `PCMDone` and `LOOP = 0` it resets the enable flip-flop (`ED1`), stopping playback and setting the IRQ flag |
| Loop enabled | `LOOPMode` | At the wrap, `DSLOAD` reloads the counters and playback continues; no IRQ is raised |

The byte rate is the bit rate of §3 divided by 8 (index 0 NTSC: 522.7 bytes/s).

## 7. DMA arbitration and the CPU

| Signal | Direction | Meaning |
|---|---|---|
| `n_DMCAB` | DMC → address mux | 0 = the DMC owns the CPU address bus for a sample fetch |
| `RUNDMC` | DMC → sprite DMA | 1 = the DMC is busy and holds the sprite DMA in standby |
| `DMCRDY` | DMC → sprite DMA | 0 = the DMC is not ready; `RDY` to the 6502 is forced to 0 |
| `DMC_Addr` | DMC → address mux | `$8000–$FFFF` sample address |

Rules observed in the silicon:

- `RDY = sprdma_rdy AND DMCRDY` (`HDL/APU/dma.v`): a DMC fetch halts the 6502
  through the same `RDY` input as OAM DMA. The fetch begins only on a CPU
  **read** cycle (`start_set` requires `PHI1 = 0` and `R/W = 1`), because `RDY`
  is ignored during write cycles.
- Address-multiplexer priority (`Address_MUX`): `SPR_PPU` ($2004 write) >
  `SPR_CPU` (OAM DMA read) > DMC (`DMC_AB`) > CPU. That static order alone would
  put OAM DMA first, but while the DMC is running `RUNDMC` forces both sprite
  selects low (`HDL/APU/dma.v`: `nor(SPR_PPU, NOSPR, RUNDMC, ~toggle)`,
  `nor(SPR_CPU, NOSPR, RUNDMC, toggle)`), so the DMC owns the address bus and a
  DMC get pre-empts a pending OAM DMA. Because two reads cannot overlap, the DMC
  reserves a window at the start of its transfer during which an already-active
  OAM DMA can still complete a cycle; after that window `RUNDMC` suppresses it
  (`APU/waves.md`).
- The APU register **index** is taken from the DMA address multiplexer output
  `A0–A4`, while the `$4000–$401F` range check uses the CPU address
  (`HDL/APU/regs.v`). A DMC fetch that presents register-like low address bits
  while the CPU is reading `$401x` can therefore select another APU register;
  the cycle-level consequences are in `02-cpu-dma.md`.

## 8. IRQ, loop and `$4015`

- The IRQ flag is an RS flip-flop set by `ED1` (`LOOP = 0`, bytes-remaining
  counter empty, last byte latched into the buffer — i.e. when the last byte is
  **read**, not when it finishes playing) and cleared by any `$4015` write, by
  `$4010` bit 7 = 0 and by reset. Reading `$4015` bit 7 returns the flag and does
  **not** clear it (unlike the frame IRQ flag, which a `$4015` read clears).
- `DMCINT = flag AND IRQ enable`, so with `$4010` bit 7 = 0 the flag is held
  cleared, no IRQ reaches the daisy chain, and with the loop flag set `ED1` is
  suppressed, so looping samples never set the flag.
- `$4015` bit 4 read = "DMC active" (enable flip-flop set); it clears when the
  sample ends without looping.
- Writing 1 to `$4015` bit 4 while the channel is disabled reloads `$4012`/
  `$4013` into the counters and starts a **load** DMA if the buffer is empty.
  Writing 0 stops playback; a later 0→1 transition restarts the sample from the
  beginning, while a repeat write of 1 does not retrigger.
- Rewriting `$4012`/`$4013` during playback does not affect the running sample
  (loaded at the next enable/loop wrap); rewriting `$4010` does not restart or
  stop the sample and its new rate is used from the next rate-counter reload.
- Disabling the channel or finishing a sample does not reset the output level
  (§5).

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | CPU runs measurably slower while a DMC sample plays | Each byte needs a fetch that holds `RDY` low (3–4 CPU cycles) | Budget ≈ 4 cycles/byte: ~7.4 % at rate 15 (54 cycles/byte), ~0.9 % at rate 0 |
| 2 | A constant DC offset/click remains after a sample ends | The 7-bit output counter holds its last value; nothing resets it | Write `$4011` with `$80`/`$00` (or the desired level) to silence |
| 3 | DMC waveforms look quantised in steps of 2, and the bottom bit never moves | Only the 6-bit counter is stepped by playback; the LSB comes from `$4011` | Write `$4011` once to choose the LSB, then let playback run |
| 4 | Writing `$4011` mid-sample produces a click or a step | Direct load of the whole output counter | Use it deliberately for PCM/click tricks |
| 5 | Sample plays at a slightly different pitch on PAL | 2A07 uses a different rate decoder PLA | Pick the region table by console region; absolute bit rates are nearly equal |
| 6 | Clearing and setting `$4015` bit 4 restarts the sample | The 0→1 transition reloads the counters (`DSLOAD`) | Use it as the retrigger mechanism; expect the address/length to be re-read |
| 7 | DMC IRQ fires when the last byte is fetched — one 8-bit buffer before the sound stops — and reading `$4015` does not clear it | `ED1` sets the flag at bytes-remaining = 0 with loop clear and the last byte latched; any `$4015` write clears it | Write `$4015` in the IRQ handler; set `$4010` bit 7 = 0 to disable |
| 8 | A looping sample never raises the IRQ | `ED1` requires `LOOP = 0` | Do not rely on the IRQ for looped playback |
| 9 | Rewriting `$4012`/`$4013` during playback appears to be ignored | The counters load only on `DSLOAD` (enable or loop wrap) | Rewrite the registers before enabling, or at the loop point |
| 10 | A DMC fetch during OAM DMA stalls or truncates sprite DMA | `RUNDMC`/`n_DMCAB` give the DMC priority on the address bus | Expect OAM DMA to extend by the DMC's stolen cycles; see `02-cpu-dma.md` |
| 11 | A `$4011` value with bit 7 set produces the same level as the value masked to 7 bits | Bit 7 is not connected to the output counter | Mask to `$7F`; do not expect bit 7 to matter |
| 12 | A sample that runs past `$FFFF` continues at `$8000` | `DMC_A15` is tied to Vdd and the 15-bit counter wraps | Keep samples below `$FFFF` or embrace the wrap as documented |

## Programming / implementation notes

- An emulator MUST implement the rate counter as a down-counter loaded from the
  region table and reloaded every period; one reload = one sample bit.
- The output unit MUST be a saturating reversible counter stepped by ±1 in the
  upper 6 bits (±2 in the 7-bit value), with the LSB owned by `$4011`; the bit
  order MUST be LSB first from each fetched byte, and `$4011` MUST load the full
  7-bit level immediately and ignore bit 7.
- The sample address MUST be `$C000 + $40*A` with the `$8000–$FFFF` wrap, and
  the byte count MUST be `$10*L + 1`.
- The enable path MUST reload both counters on the `$4015` bit 4 0→1 transition
  and on a loop wrap.
- Reading `$4015` MUST NOT clear the DMC IRQ flag; writing `$4015` MUST clear
  it; `$4010` bit 7 = 0 MUST hold the flag cleared.
- DMC DMA MUST have priority over OAM DMA and MUST halt the CPU only on a read
  cycle; the fetch MUST latch the byte during PHI2 while the DMC owns the bus.
- A region-select switch SHOULD choose the NTSC or PAL rate table.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| DMC registers, rates, sample address/length | `APU_DMC` |
| DMA cycles, `RDY`, conflicts with `$2007`/`$4016` | `DMA`, `Open_bus_behavior` |
| APU register map `$4000`–`$4017` | `APU_registers`, `APU` |
| Length-counter/status bits of `$4015` | `APU_Length_Counter` |
| APU IRQ routing | `CPU_interrupts` |
| DAC and mixing (`AUX_B`) | `APU_Mixer` |
| OAM DMA (`$4014`) | `PPU_OAM` |
| 2A03/2A07 revision differences | `CPU_variants` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| DMC block list, signal names, `DSLOAD`/`DSSTEP`/`BLOAD`/`BSTEP`/`NSTEP`/`DSTEP`/`PCM` | `APU/dpcm.md` | `dpcm.v` |
| Control register `$4010`, `$4015` status and IRQ behaviour | `APU/dpcm.md` | `dpcm.v` |
| Rate decoder PLA (NTSC) and rate counter | `APU/dpcm.md` | `dpcm.v` |
| PAL (2A07) rate decoder PLA | `APU/pal.md` | `dpcm.v` |
| Sample address counter, `DMC_A15` = Vdd, wrap | `APU/dpcm.md` | `dpcm.v` |
| Output counter, `$4011` direct load, bit order | `APU/dpcm.md` | `dpcm.v` |
| DMA arbitration (`RUNDMC`, `n_DMCAB`, `DMCRDY`, address mux, register select) | `APU/dpcm.md`, `APU/dma.md` | `dpcm.v`, `dma.v`, `regs.v` |
| Counter primitives (reversible/down counters) | `APU/common.md` | `common.v` |
| DMC/OAM overlap window | `APU/waves.md` | (test bench) |
| APU clock periods | `APU/clkgen.md` | `clkgen.v` |
| Revision-specific DMC behaviour | `Docs/NES_Famicom_Board_Revisions.md` | — |

## Open questions

- The wiki does not print the DMC rate tables or the length formula; §3's values
  were derived from the `dpcm.md` / `pal.md` PLA masks and the `DPCM_FreqLFSR`
  model (all 32 entries match the community tables), and the `$10*L + 1` byte
  count is the community formula plus the HDL load value. Should this be pinned
  by a test-ROM run?
- **`$4011` racing a timer clock (NESdev vs schematic).** NESdev `APU_DMC` notes
  that when `$4011` is written at the same time as the timer outputs a clock, the
  output level is occasionally not changed properly. The HDL gives the direct
  load priority over the delta step (`RevCounterBit`:
  `latch_in = load ? d : (clear ? 0 : (step ? cgnq : …))`, `common.v`), so the
  `DSTEP` is dropped in that clock. §5 describes the immediate load but not the
  lost step; an emulator matching the community model should let `$4011` win and
  discard a simultaneous `DSTEP`.
- **Power-on output level (NESdev vs schematic).** NESdev says the DMC level is
  loaded with 0 on power-up. The HDL clears the 6-bit counter on `RES`
  (`RevCounterBit .clear(RES)`, `dpcm.v`), but the LSB is a plain `RegisterBit`
  with no reset (`out_reg`), so the LSB's power-on value is undefined in the
  schematic. §5's "nothing resets the counter except the chip reset" is exact for
  the upper 6 bits only.
- The exact reason playback ends one byte after the counter reaches 0 (the
  `SOUT` latch phase) is inferred from the HDL; cycle-level hardware evidence is
  missing.
- `$4015` bit 4 behaviour when a DMC DMA fetch is already in flight (aborting
  mid-fetch) is not documented in the wiki.
- `Docs/NES_Famicom_Board_Revisions.md` mentions fixed DPCM RDY address-bus
  glitches in the 2A07 and a DMC DMA bug in late RP2A03G; neither is described
  at signal level in the wiki.
- The DMC's interaction with controller reads at `$4016`/`$4017` is a
  CPU-side effect and is left to `02-cpu-dma.md`.
