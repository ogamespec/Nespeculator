# 03-apu-overview — Overview and register interface

**Status:** consensus (round 2)
**Scope:** The Ricoh 2A03 (NTSC) / 2A07 (PAL) as a component: block list, pinout, clock hierarchy, the `$4000–$4017` register interface with directions and bit fields, interrupt routing, OAM/DMC DMA and `RDY`, the I/O ports, the 2A03 test registers and the 2A07 / early-revision differences. Channel internals (timers, duty, envelope, sweep, length, LFSR, DMC reader, mixer numbers) are specified in `03-apu-pulse.md`, `03-apu-triangle.md`, `03-apu-noise.md`, `03-apu-dmc.md`, `03-apu-frame-counter.md` and `03-apu-mixer.md`.
**Primary sources:** `BreakingNESWiki/APU/` — `Readme.md`, `contents.md`, `regs.md`, `pads.md`, `common.md`, `core.md`, `clkgen.md`, `sound.md`, `waves.md`, `cycles.md`, `visual2a03.md`, `test_pattern.md`, `old_2a03.md`, `pal.md`, `dma.md`, `dpcm.md`, `square.md`, `triangle.md`, `noise.md`, `length.md`, `dac.md`; `HDL/APU/` — `apu_top.v`, `regs.v`, `pads.v`, `io.v`, `common.v`, `clkgen.v`, `core.v`, `dma.v`, `dmabuf.v`, `dpcm.v`, `length.v`, `test.v`, `dac.v`; `Docs/APU/2A03_ACLK_Distrib.png`.

## 1. What the 2A03/2A07 is

The 2A03 (NTSC) and 2A07 (PAL) are Ricoh CPUs, RP2A03 / RP2A07, in a 40-pin DIP. Each integrates a MOS 6502 core, the APU sound generators, DMA logic, I/O ports and (2A03 only) test logic. The Breaking NES Wiki calls the whole chip "APU"; this spec follows NESdev and calls the chip the CPU, reserving "APU" for the audio and register section (`APU/Readme.md`).

| Block | Function | HDL |
|---|---|---|
| 6502 core | Instruction execution; BCD disabled | `apu_top.v` (`Core`), `core.v` |
| Clock divider | `CLK ÷ 12` (÷ 16 on the 2A07) → `PHI0`; `PHI1`/`PHI2`; `M2` | `core.v` (`CLK_Divider`) |
| ACLK generator | `PHI0 ÷ 2` → `ACLK1`, `/ACLK2` | `clkgen.v` (`ACLKGen`) |
| Frame counter (wiki: Soft CLK) | `$4017`, `/LFO1`, `/LFO2`, frame IRQ, IRQ daisy chain | `clkgen.v` (`SoftTimer`) |
| Pulse 1 / Pulse 2 (wiki: Square 0/1) | Two pulse channels | `square.v` |
| Triangle (wiki: Sound C) | Triangle channel | `triangle.v` |
| Noise | Noise channel | `noise.v` |
| DMC (wiki: DPCM / Sound E) | Sample playback + sample DMA | `dpcm.v` |
| Length counters | Shared 4× length counter block | `length.v` |
| OAM/sprite DMA | DMA controller, address mux, `RDY` control | `dma.v`, `dmabuf.v` |
| I/O ports | `$4016`/`$4017` pads | `io.v` |
| DACs | Digital outputs on AUX A / AUX B | `dac.v` |
| Debug/test logic | `$4018–$401A`, `LOCK` (2A03 only) | `test.v` |
| Register decoder | APU register predecode, PLA, strobes | `regs.v` |

The embedded 6502 differs from a stock 6502 in one respect: the `/DAA` and `/DSA` decimal-correction control signals are cut (five missing polysilicon links), so the core always adds and subtracts in binary even when the D flag is set. The instruction decoder, the D flag itself (set/cleared by `SED`/`CLD`), its use by `PHP`/`PLP`/`RTI` and its stacking during interrupts are unchanged (`core.md`). The core is one bus participant among several: the DMA controller can stop it with `RDY`, and the DMC competes for the address bus (`Readme.md`, `dma.md`).

## 2. Pinout

| Pin | Name | Dir | Function |
|---|---|---|---|
| 1, 2 | AUX A, AUX B | out | DAC buses, see §10 |
| 3 | `/RES` | in | Reset input; internal `RES` is its inverse |
| 4–19 | A0–A15 | out | Address bus, unbuffered; outputs `z` while `RES` = 1 |
| 20 | GND | — | Ground |
| 21–28 | D0–D7 | bidir | Data bus, unbuffered; `RD` = 1 input, `WR` = 1 output |
| 29 | CLK | in | Master clock |
| 30 | DBG | in | 1 = test mode (2A03), `RDY2` control (2A07) |
| 31 | M2 | out | External CPU clock (modified `PHI2`) |
| 32 | `/IRQ` | in | Maskable interrupt, active low |
| 33 | `/NMI` | in | Non-maskable interrupt, active low |
| 34 | R/W | out | Data-bus direction, 1 = read; `z` while `RES` = 1 |
| 35, 36 | `/IN1`, `/IN0` | out | Read strobes for the `$4017` / `$4016` ports |
| 37–39 | OUT2, OUT1, OUT0 | out | `$4016` bits 2, 1, 0 |
| 40 | VCC | — | +5 V |

`SO` is tied to 1 (the input is a falling-edge detector) and `SYNC` is left floating (`core.md`). All `IN`/`OUT` pads and A0–A15 go high-impedance during reset (`pads.md`).

NESdev `CPU_pinout` uses different names for several pins: `AD1`/`AD2` for AUX A / AUX B, `/OE1`/`/OE2` for `/IN0` / `/IN1` (controller 1 / 2), `/RST` for `/RES`, and `TST` (tentative name) for `DBG`. This spec keeps the repository / Visual 2A03 names; the NESdev names are the community alternative.

## 3. Clock hierarchy

Ordered chain: `CLK` pad → `/CLK` → 6-stage Johnson divider (`CLK ÷ 12` on the NTSC 2A03; the 2A07 divides by 16) → `PHI0` → core → `PHI1`, `PHI2` → ACLK generator (`PHI0 ÷ 2`) → `ACLK1`, `/ACLK2` → frame-counter LFSR/PLA → `/LFO1`, `/LFO2` (`clkgen.md`, `core.v`, `clkgen.v`).

| Signal | Relation | NTSC 2A03 | PAL 2A07 |
|---|---|---|---|
| `CLK` | master clock | 21.477272 MHz, ~46.56 ns | 26.601712 MHz, ~37.59 ns |
| `PHI0` | `CLK ÷ 12` | ~1.7897727 MHz, ~558.73 ns | ~1.662607 MHz, ~601.46 ns |
| `ACLK` | `PHI0 ÷ 2` | ~0.894886 MHz, ~1117.46 ns | ~0.831304 MHz, ~1202.93 ns |

One **CPU cycle** is one `PHI0` period (1.79 MHz NTSC); one **APU cycle** is one `ACLK` period = 2 CPU cycles. `PHI1` is the first half of a CPU cycle ("set R/W mode and address"), `PHI2` the second ("read/write data"); `ACLK1` and `/ACLK2` are neither complementary nor overlapping (`clkgen.md`).

Rules that follow from the clocking:

- Register writes are active only while `PHI1` = 0, i.e. during `PHI2`; every write strobe in `regs.v` is a NOR with `PHI1`. This is why the pulse sweep unit asserts `DO_SWEEP` only during `PHI1`, to avoid clashing with CPU register writes (`square.md`).
- The triangle channel uses `PHI1` instead of `ACLK1` in places, to smooth the steps of its output (`sound.md`, `triangle.md`).
- "Other ACLK2" is a local inversion of `/ACLK2`. The copies are: `ACLK2` (DMC and OAM DMA), `ACLK3A`/`ACLK3B` (pulse `$4002`/`$4003`/`$4006`/`$4007` registers), `ACLK4` (noise channel and length-counter control), `ACLK5` (I/O output latches) (`clkgen.md`, `io.v`). Older wiki pages wrote `ACLK1` as `/ACLK` and `/ACLK2` as `ACLK`; the current names follow Visual 2A03 (`Readme.md`, `visual2a03.md`).

### M2

`M2` is the external CPU clock derived from `PHI2`. Its high level becomes active 1.5 `CLK` (3 half-cycles) before the core's `PHI2` goes high, so external devices can drive or release the bus before the internal core does (`pads.md`). The high time is 17/24 of the divider cycle on the 2A03G and 15/24 on the letterless 2A03 (SR4 vs SR3, `old_2a03.md`). In retail mode (`DBG` = 0) `M2` is `z` during reset; with `DBG` = 1 it keeps being driven during reset (`pads.md`, `pads.v`).

> NESdev says 15/24 for the NTSC 2A03E/G/H and 17/24 for the letterless part (and 19/32 for the PAL 2A07); this repository's schematic assigns 17/24 to the 2A03G and 15/24 to the letterless part (`old_2a03.md`). This spec keeps the schematic assignment; the PAL 19/32 figure is NESdev-only and **unconfirmed** here. See *Open questions*.

### Low-frequency oscillator (frame-counter LFO)

The frame counter is a 15-bit LFSR plus a PLA. PLA outputs 0, 1, 2 and (mode-dependent) 3 or 4 activate in turn; PLA[5] exists only to reload the LFSR out of the all-zero state and does not affect the LFO outputs. Both LFO outputs are active low and are qualified by the `ACLK2` phase and by the `Z2` term (a fifth-step qualifier latched from the mode bit, so it is live in 5-step mode): `/LFO1` is asserted while any of PLA[0..4] is active or `Z2` is active, `/LFO2` while PLA[1], PLA[3] or PLA[4] is active or `Z2` is active (`clkgen.md`, `clkgen.v`).

| Signal | 4-step (`$4017` bit 7 = 0), NTSC | 5-step (bit 7 = 1), NTSC | Destinations |
|---|---|---|---|
| `/LFO1` | ~240 Hz (7457 CPU cycles) | ~192 Hz (3×7457 then 14913) | Pulse 1/2 envelope, triangle linear counter, noise envelope |
| `/LFO2` | ~120 Hz (14913 CPU cycles) | ~96 Hz (14913 then 22367) | Pulse 1/2 sweep, all four length counters |

In NESdev terms `/LFO1` is the quarter-frame clock and `/LFO2` the half-frame clock. In 5-step mode one slot is skipped, so the intervals are no longer uniform (for `/LFO1` at NTSC: 7457, 7457, 7457, then 14913 CPU cycles), which lowers the average rates to the values shown (`clkgen.md`). For the PAL 2A07 the same outputs are spaced 8313 CPU cycles in 4-step mode; dividing the PAL `PHI0` rate by those intervals gives ~200 Hz (`/LFO1`) and ~100 Hz (`/LFO2`), and ~160 Hz / ~80 Hz in 5-step mode (derived from the 8313 / 16625 / 24939 cycle counts in `clkgen.md`; the wiki states these frequencies only for the 2A03).

### Interrupt routing

- `/NMI` reaches the core almost directly, through intermediate inverters only (`core.md`).
- `/IRQ` from the pad is NORed with the internal `INT`, so `/IRQ_INT` at the core is asserted when either the external `/IRQ` or `INT` is asserted (`pads.v`, `core.md`).
- The frame-counter block is the daisy chain for the DMC: `INT` = frame-IRQ flag OR `DMCINT` (`clkgen.v`, `clkgen.md`).
- `DMCINT` = `$4015` bit 7 AND `$4010` bit 7; the frame IRQ is `$4015` bit 6, inhibited and cleared by `$4017` bit 6, and is generated only in 4-step mode (`dpcm.v`, `clkgen.v`, `clkgen.md`).

## 4. Register map `$4000–$401F`

| Address | R/W | Name | Reaches |
|---|---|---|---|
| `$4000` | W | Pulse 1 duty / volume / envelope | `Square 0` (`W4000`) |
| `$4001` | W | Pulse 1 sweep | `Square 0` (`W4001`) |
| `$4002` | W | Pulse 1 timer low | `Square 0` (`W4002`) |
| `$4003` | W | Pulse 1 length / timer high | `Square 0` + length counter (`W4003`) |
| `$4004` | W | Pulse 2 duty / volume / envelope | `Square 1` (`W4004`) |
| `$4005` | W | Pulse 2 sweep | `Square 1` (`W4005`) |
| `$4006` | W | Pulse 2 timer low | `Square 1` (`W4006`) |
| `$4007` | W | Pulse 2 length / timer high | `Square 1` + length counter (`W4007`) |
| `$4008` | W | Triangle linear counter / control | Triangle (`W4008`) |
| `$4009` | — | not decoded | — |
| `$400A` | W | Triangle timer low | Triangle (`W400A`) |
| `$400B` | W | Triangle length / timer high | Triangle + length counter (`W400B`) |
| `$400C` | W | Noise volume / envelope | Noise (`W400C`) |
| `$400D` | — | not decoded | — |
| `$400E` | W | Noise period / mode | Noise (`W400E`) |
| `$400F` | W | Noise length | Noise + length counter (`W400F`) |
| `$4010` | W | DMC IRQ enable / loop / rate | DMC (`W4010`) |
| `$4011` | W | DMC direct load (7-bit level) | DMC (`W4011`) |
| `$4012` | W | DMC sample address | DMC (`W4012`) |
| `$4013` | W | DMC sample length | DMC (`W4013`) |
| `$4014` | W | OAM DMA page | Sprite DMA (`W4014`) |
| `$4015` | R/W | Channel enable / status | Length counters, DMC, frame counter, DMA buffer (`W4015`, `/R4015`) |
| `$4016` | R/W | I/O port 0 (controller 1) | I/O ports (`W4016`, `/R4016`) |
| `$4017` | W | Frame counter (write) | Frame counter (`W4017`) |
| `$4017` | R | I/O port 1 (controller 2) | I/O ports (`/R4017`) |
| `$4018–$401A` | R (W for `$401A`) | 2A03 test registers, `DBG` = 1 only | Test logic, Triangle (`/R4018`, `/R4019`, `/R401A`, `W401A`) |
| `$401B–$401F` | — | not decoded | — |

## 5. Register bit fields

Bits 0 (LSB) to 7 (MSB). Fields follow the test vectors in `waves.md` and the HDL register bits (`regs.v`, `clkgen.v`, `dpcm.v`, `length.v`).

| Register | Diagram | Fields |
|---|---|---|
| `$4000`, `$4004` | `DDLC VVVV` | DD duty (0–3); L length-counter halt / envelope loop; C constant volume; VVVV volume or envelope divider |
| `$4001`, `$4005` | `EPPP NSSS` | E sweep enable (1 = enabled, 0 = disabled — the NESdev enabled flag; `SWDIS` in the HDL is its inverse); PPP divider period; N negate; SSS shift count |
| `$4002`, `$4006`, `$400A` | `LLLL LLLL` | Timer bits 7–0 (pulse 1, pulse 2, triangle) |
| `$4003`, `$4007`, `$400B` | `LLLL LHHH` | LLLLL length index; HHH timer bits 10–8 |
| `$4008` | `CRRR RRRR` | C control flag (halt / reload); RRRRRRR linear-counter reload value |
| `$400C` | `--LC VVVV` | L length-counter halt / envelope loop; C constant volume; VVVV volume or envelope divider |
| `$400E` | `M--- PPPP` | M noise mode (0 = random, 1 = tonal/periodic); PPPP period index |
| `$400F` | `LLLL L---` | LLLLL length index |
| `$4010` | `IL-- RRRR` | I 1 = DMC IRQ enabled; L loop; RRRR rate index |
| `$4011` | `-DDD DDDD` | DDDDDDD 7-bit DAC level; bit 7 is not latched (`dpcm.v`) |
| `$4012` | `AAAA AAAA` | Sample address `$C000 + $40 × A` |
| `$4013` | `LLLL LLLL` | Sample length `$10 × L + 1` bytes |
| `$4014` | `AAAA AAAA` | OAM DMA source page |
| `$4015` (W) | `---D NT21` | D DMC enable; N noise; T triangle; 2 pulse 2; 1 pulse 1 |
| `$4015` (R) | `IF-D NT21` | I DMC IRQ flag; F frame-counter IRQ flag; D DMC active; NT21 length-counter status; bit 5 is not driven |
| `$4016` (W) | `----- 210` | Bits 0, 1, 2 drive OUT0, OUT1, OUT2; bits 3–7 ignored |
| `$4017` (W) | `MI-- ----` | M 1 = 5-step sequence, 0 = 4-step; I 1 = frame IRQ inhibit (also clears the flag) |

`$4015` read bits, from the HDL drivers (`length.v`, `clkgen.v`, `dpcm.v`); NESdev writes the field as `IF-D NT21`:

| Bit | Meaning |
|---|---|
| 0–3 | Length counter status: pulse 1, pulse 2, triangle, noise (1 = counter active) |
| 4 | DMC active |
| 5 | Not driven by any APU block (open bus) |
| 6 | Frame-counter IRQ flag, active high; mask/inhibit it with `$4017` bit 6 |
| 7 | DMC IRQ flag, active high; enable it with `$4010` bit 7 |

## 6. Register decoding, direction and open bus

- **Address window.** The predecoder selects the APU register space when CPU address bits A15 = 0, A14 = 1 and A13–A5 = 0 — that is, `$4000–$401F` exactly, 32 bytes (`regs.v`). A0–A4 do not take part in the space selection.
- **Register index.** The register is chosen by A0–A4 *after the DMA address multiplexer*, not directly from the CPU address bus. The space is chosen from the CPU address bus (`CPU_Ax`), so a register access is decoded only when the multiplexer presents the CPU address (`regs.md`).
- **No mirroring.** Within `$4000–$401F` each of the 32 addresses selects one register or nothing; there is no repetition of the register file (there is no bit above A4 in the decode). `$4009`, `$400D` and `$401B–$401F` have no register.
- **Direction.** `$4000–$4014` are write-only; `$4015`, `$4016` and `$4017` have both a write and a read path; `$4018–$401A` are read-only except for `$401A` (write: `LOCK` bit 7 and the triangle output counter bits 4–0, test mode only). `/R4015` is additionally routed to the DMA buffer, which tracks `$4015` reads (`regs.md`).
- **Open bus.** A read of `$4000–$4014` enables no bus driver, so the CPU sees whatever the data bus last held (open bus). On a `$4015` read only bits 0–4, 6 and 7 are driven, so bit 5 is open bus. On `$4016`/`$4017` reads the APU drives nothing at all: it only asserts `/IN0` or `/IN1`, and the byte is supplied by the controller/expansion hardware on the external bus (`regs.md`, `io.v`, `length.v`, `clkgen.v`, `dpcm.v`).
- **Debug reads.** `$4018`, `$4019` and `$401A` are only selected when the `DBG` pad is 1; with `DBG` = 0 they read as open bus (`regs.v`, §9).
- **Write timing.** All write strobes are gated with `PHI1`, so register writes take effect in `PHI2` (§3).

## 7. DMA, the address bus and `RDY`

The OAM ("sprite") DMA block is also the bus arbiter and the `RDY` controller (`dma.md`).

- A write to `$4014` clears the low byte of the DMA address, loads the written value into the high byte, and starts the transfer (`dma.md`, `dma.v`). The destination is hard-wired: the address multiplexer emits `$2004` (the PPU OAM data register) for every write phase (`dma.md`, `dma.v`).
- The DMA alternates a read phase (`SPR/CPU`: fetch the next byte from `$XX00 + n`) and a write phase (`SPR/PPU`: store it to `$2004`), 256 read/write pairs; the low address is an 8-bit up counter whose end is the carry out of bit 7 (`dma.v`). With the halt cycle and the possible alignment cycle the transfer costs 513 or 514 CPU cycles (`02-cpu-dma.md`).
- The DMA can start only on a CPU **read** cycle (`PHI1` = 0, R/W = 1), because the 6502 ignores `RDY` on write cycles. A `$4014` write that is not aligned to `ACLK1` therefore costs one extra CPU cycle (`dma.md`, `dma.v`, `waves.md`).
- `RDY` is active-high ready: `0` = the core is halted and repeats its last read cycle. It is pulled low for the whole OAM DMA and whenever the DMC is not ready, and released only when the sprite DMA is idle **and** `DMCRDY` = 1: `sprdma_rdy = NOR(StopDma, StartDma)` (with `StopDma` set for the duration of the transfer) and `RDY = sprdma_rdy AND DMCRDY` (`dma.v`, `dma.md`).
- DMC sample DMA has priority. While `RUNDMC` = 1 the sprite DMA is held in standby, and `/DMCAB` takes the address bus for the sample fetch; the DMC stops the CPU only for the duration of that fetch, not for the whole sample (`dpcm.md`, `dma.md`). At the start of a DMC transfer there is a window after which `RUNDMC` suppresses an already-active OAM DMA, because two reads cannot be performed at once (`waves.md`).
- Address-multiplexer priority: `SPR/PPU` (`$2004`) > `SPR/CPU` (DMA address) > `/DMCAB` (DMC address) > CPU address (`dma.v`, `dma.md`). The DMA buffer also produces the external `RW`, `WR` and `RD` signals for the address/data pads and holds the byte being copied (`core.md`, `dmabuf.v`).
- `RDY2` is a second core-ready input. On the 2A03 it is tied to 1 (no effect). On the 2A07 it equals `/DBG`, so driving `DBG` high in the 2A07 halts the CPU — an external "test WAIT" (`core.md`).

## 8. I/O ports `$4016` / `$4017`

- Write `$4016`: bits 0, 1, 2 are latched on `ACLK1` and then transferred to the OUT0/OUT1/OUT2 pins by the `ACLK5` = `/ACLK2` latch, i.e. on the following `ACLK2` phase. Bits 3–7 are ignored (`io.v`).
- Read `$4016` / `$4017`: the register select asserts `/IN0` or `/IN1`, which are outputs to the controller/expansion ports; the APU drives no data-bus bits (`regs.md`, `io.v`, `pads.md`).
- Write `$4017` goes to the frame counter only; the I/O port has no write path from `$4017` (`regs.md`).
- The pads `/IN0`, `/IN1`, `OUT0–OUT2` are high-impedance during reset (`pads.md`).
- On the NES/Famicom the three outputs are the standard controller-port latch/clock lines (NESdev `Standard_controller`); OUT0 is the controller strobe and OUT1/OUT2 go to the expansion port (`Expansion_port`).

## 9. Test mode (`$4018–$401A`) and revision differences

With `DBG` = 1, the 2A03 exposes the live generator outputs (`regs.md`, `test.v`):

| Register | Bits |
|---|---|
| `$4018` read | 3–0 = pulse 1 DAC output (`SQA`); 7–4 = pulse 2 DAC output (`SQB`) |
| `$4019` read | 3–0 = triangle output (`TRI`); 7–4 = noise output (`RND`) |
| `$401A` read | 6–0 = DMC output level (`DMC`); bit 7 not driven |
| `$401A` write | bit 7 = `LOCK` flip-flop (clocked by `ACLK1`, cleared by `RES`); bits 4–0 = triangle output counter parallel load (`TRIANGLE_Output`, `triangle.v`, `apu_top.v`) |

`LOCK` freezes / disconnects the sound generators so that their current values stay stable while they are read back (`regs.md`, `test.v`). With `DBG` = 0 the three registers are not selected at all, so they read as open bus. `/DBGRD` (active low) marks "an APU register read while `DBG` = 1" and is consumed by the DMA buffer (`regs.md`, `regs.v`, `test.v`).

Revision differences:

- **2A07 (PAL):** no debug logic at all — only `RDY2` remains. The register decoder is shortened, `W401A` is tied to ground, and the SoftCLK PLA, the DMC decoder and the noise decoder have different transistor patterns (`pal.md`, `regs.md`, `core.md`).
- **Letterless 2A03 (early revision):** no test registers and no `LOCK`; `W401A` tied to ground; no tonal-noise mode (`$400E` bit 7 latch and its multiplexer are absent, feedback comes straight from noise LFSR bit R13); the noise frequency LFSR is one stage shorter with a different feedback and a different period table; `RES` does not reload the DPCM frequency LFSR, the ACLK divider or the OAM DMA start flip-flop; the LFO is not reloaded by reset; `M2` high time is 15/24 (SR3); the test-pattern area (like the 2A03G/2A07A test patterns, `test_pattern.md`) contains nothing of interest apart from a disabled programmable interval timer (`old_2a03.md`).

## 10. DAC outputs

The digital channel outputs leave the chip on two buses (`dac.v`, `dac.md`):

| Pin | Content |
|---|---|
| AUX A | `{SQB[3:0], SQA[3:0]}` — 8 bits (pulse 1, pulse 2) |
| AUX B | `{DMC[6:0], RND[3:0], TRI[3:0]}` — 15 bits (DMC, noise, triangle) |

Inside the chip the AUX pads are bare (no load); the board terminates them, usually with ~100 Ω pull-downs to GND per line (`dac.md`). Measured AUX A maximum is 272 mV with a 75 Ω external load at `SQA` = `SQB` = `%1111` (`dac.md`). The DACs use the common VDD/GND with star routing; there are no separate analog supply pins (`dac.md`). This is the "semi-analog" part of the chip.

## Observable phenomena

| # | Phenomenon | Cause | What to do |
|---|---|---|---|
| 1 | Reading `$4000–$4014` returns changing, unrelated values | these addresses are write-only; no bus driver is enabled (`regs.md`) | treat the result as open bus; never read channel registers for state |
| 2 | `$4015` bit 5 follows bus activity | only bits 0–4, 6, 7 have drivers (`length.v`, `clkgen.v`, `dpcm.v`) | mask bit 5 off before testing status |
| 3 | Reading `$4015` acknowledges the frame IRQ | the read (during `PHI2`, `PHI1` = 0) resets the frame-IRQ flip-flop (`clkgen.v`) | read `$4015` exactly once per handler and cache the value |
| 4 | The frame IRQ never fires in 5-step mode | PLA output 3, which generates the interrupt, is skipped in mode 1 (`clkgen.md`) | use the 4-step sequence for frame-IRQ timing, or poll `$4015` |
| 5 | Writing `$4017` clears a pending frame IRQ | the `$4017` bit-6 write also resets the IRQ flip-flop (`clkgen.v`) | a mode change is also an acknowledge |
| 6 | An OAM DMA may start one CPU cycle late | it can start only on a CPU read cycle, because `RDY` is ignored on write cycles (`dma.md`, `dma.v`) | model the read-cycle wait and the `ACLK1` alignment penalty (`waves.md`) |
| 7 | An OAM DMA stalls while a DMC byte is fetched | `RUNDMC` holds the sprite DMA and the DMC owns the address bus (`dma.md`, `dma.v`) | expect a longer DMA while DPCM is playing |
| 8 | The CPU is stopped only for the DMC sample fetch | DMC DMA pulls `RDY` low only for the byte fetch (`dpcm.md`) | do not halt the core for a whole sample |
| 9 | Envelope, linear-counter, sweep and length rates change with the frame-counter mode | `/LFO1` and `/LFO2` rates differ between 4-step and 5-step (`clkgen.md`) | a mode change alters music speed, pitch and decay rates |

## Programming / implementation notes

- The APU register window is `$4000–$401F`; only `$4000–$4017` and (2A03, `DBG` = 1) `$4018–$401A` are populated. Writes to `$4009`, `$400D` and `$401B–$401F` MUST be treated as no-ops.
- Register writes MUST be applied in `PHI2` (they are gated with `PHI1`); reads MUST be applied when the core performs the read cycle.
- `$4000–$4014` MUST be modelled as write-only; reads return the last value on the data bus (open bus).
- `$4015` reads MUST clear the frame IRQ flag (bit 6) but MUST NOT clear the DMC IRQ flag (bit 7) — only a `$4015` write (or `$4010` bit 7 = 0) does that; a `$4017` write with bit 6 = 1 MUST also clear the frame IRQ, and the frame IRQ MUST NOT be generated in 5-step mode. See `03-apu-dmc.md`.
- A `$4017` write MUST reset the sequence and take effect 3–4 CPU cycles later, not on the write cycle (`03-apu-frame-counter.md`).
- `$4015` write bit 4 MUST control the DMC enable, and clearing it MUST stop the current DMC sample.
- `$4014` MUST clear the low DMA address byte, load the high byte from the written value, and start a 256-byte transfer to `$2004`.
- `RDY` MUST be modelled as active-high ready with `0` = core halted: `RDY = sprdma_rdy AND DMCRDY`, where `sprdma_rdy` (`NOR(StopDma, StartDma)`) is `0` for the whole OAM DMA and `DMCRDY` = 0 (a DMC fetch) MUST force `RDY` low. The DMA start MUST wait for a CPU read cycle, and the core MUST re-perform the interrupted read when the halt ends.
- DMC DMA MUST take priority over OAM DMA (via `RUNDMC`) and MUST halt the CPU only during the sample-byte fetch.
- `$4016` write bits 0–2 MUST drive OUT0–OUT2; `$4016`/`$4017` reads MUST NOT return APU data, only assert `/IN0`/`/IN1`.
- `/LFO1` and `/LFO2` periods MUST follow the region and the current frame-counter mode (NTSC 240/120/192/96 Hz; PAL 200/100/160/80 Hz).
- An emulator targeting a 2A03 SHOULD accept `DBG` = 1 and expose `$4018–$401A`; an emulator targeting a 2A07 MUST NOT (only `RDY2` remains).
- Statements about clock rates MUST name the region: NTSC 2A03 (21.477272 MHz master clock) or PAL 2A07 (26.601712 MHz); Dendy is a PAL famiclone with an NTSC-like CPU:PPU ratio and is not covered here.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| APU block overview, channels, register summary | `APU` |
| Complete `$4000–$4017` register map and bit fields | `APU_registers` |
| Frame counter, `$4017` modes, frame IRQ, LFO rates | `APU_Frame_Counter` |
| DMC: registers `$4010–$4013`, DMA, IRQ flag | `APU_DMC` |
| Pulse channel bit fields (`$4000–$4007`) | `APU_Pulse` |
| Triangle channel bit fields (`$4008–$400B`) | `APU_Triangle` |
| Noise channel bit fields (`$400C–$400F`) | `APU_Noise` |
| Envelope (loop / constant-volume bits) | `APU_Envelope` |
| Sweep (`$4001`/`$4005`) | `APU_Sweep` |
| Length-counter enables and `$4015` status bits | `APU_Length_Counter`, `APU_registers` |
| DAC outputs AUX A / AUX B | `APU_Mixer` |
| OAM DMA via `$4014` | `PPU_OAM` |
| `$4016`/`$4017` controller ports | `Standard_controller`, `Expansion_port` |
| Open-bus behaviour of the write-only and undriven registers | `Open_bus_behavior` |
| Clock hierarchy, APU/CPU clock rates and dividers | `Cycle_reference_chart` |
| Pinout, `M2`, `RDY`, `/IRQ`, `/NMI`, 2A03 vs 2A07 | `CPU_pinout` |
| Memory map of the `$4000` region | `CPU_memory_map` |
| 2A03 test registers `$4018–$401A`, `DBG`, PAL/early revision differences | `CPU_pinout`, `APU` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Chip identity, block list, BCD removal, semi-analog DAC | `APU/Readme.md`, `APU/contents.md`, `APU/core.md` | `HDL/APU/apu_top.v`, `HDL/APU/core.v` |
| Pinout, pad tri-state, `M2` lead and reset behaviour | `APU/pads.md` | `HDL/APU/pads.v` |
| Clock hierarchy, divider, ACLK, `M2` duty | `APU/clkgen.md`, `APU/core.md`, `APU/pads.md`, `APU/old_2a03.md` | `HDL/APU/core.v`, `HDL/APU/clkgen.v` |
| LFO periods, PLA, quarter/half-frame routing | `APU/clkgen.md`, `APU/pal.md` | `HDL/APU/clkgen.v` |
| Register map, directions, predecode, `DBG` gating | `APU/regs.md` | `HDL/APU/regs.v`, `HDL/APU/apu_top.v` |
| Register bit fields | `APU/waves.md`, `APU/square.md`, `APU/triangle.md`, `APU/noise.md`, `APU/dpcm.md`, `APU/clkgen.md` | `HDL/APU/regs.v`, `HDL/APU/dpcm.v`, `HDL/APU/length.v` |
| `$4015` read bits and side effects | `APU/regs.md`, `APU/clkgen.md`, `APU/dpcm.md`, `APU/waves.md` | `HDL/APU/length.v`, `HDL/APU/clkgen.v`, `HDL/APU/dpcm.v` |
| Interrupt routing and IRQ daisy chain | `APU/core.md`, `APU/clkgen.md`, `APU/pads.md` | `HDL/APU/clkgen.v`, `HDL/APU/pads.v`, `HDL/APU/dpcm.v` |
| OAM DMA, `$4014`, address mux, `RDY` | `APU/dma.md`, `APU/waves.md` | `HDL/APU/dma.v`, `HDL/APU/dmabuf.v` |
| DMC DMA priority and arbitration | `APU/dpcm.md`, `APU/dma.md`, `APU/waves.md` | `HDL/APU/dpcm.v`, `HDL/APU/dma.v` |
| I/O ports `$4016`/`$4017` | `APU/pads.md`, `APU/regs.md` | `HDL/APU/io.v` |
| Test registers `$4018–$401A` and `LOCK` | `APU/regs.md` | `HDL/APU/test.v`, `HDL/APU/regs.v` |
| 2A07 differences | `APU/pal.md`, `APU/regs.md`, `APU/core.md` | `HDL/APU/regs.v` (gating), `HDL/APU/test.v` |
| Letterless 2A03 differences | `APU/old_2a03.md` | — |
| Test-pattern areas | `APU/test_pattern.md` | `HDL/APU/test.v` |
| DAC outputs AUX A / AUX B | `APU/dac.md`, `APU/sound.md` | `HDL/APU/dac.v` |
| Common register/counter cells | `APU/common.md` | `HDL/APU/common.v`, `HDL/Common/*.v` |
| Signal names vs Visual 2A03 | `APU/visual2a03.md`, `APU/Readme.md` | — |
| Non-textual cycle appendix | `APU/cycles.md` | — |

## Open questions

- **NESdev page names.** All titles in the correspondence table were checked against the local Wayback cache (`/tmp/nesdev-cache/`, snapshots of December 2024 – January 2025). Two draft names were wrong and are corrected here: `APU_Mixers` does not exist (the `/wiki/APU_Mixers` URL returns the archive error page; the mixer page is `APU_Mixer`, article 54) and `Controller_ports` 404s (the `$4016`/`$4017` interface is documented under `Standard_controller` and `Expansion_port`). A few redirects resolve to their canonical titles (`Cycle reference chart` → `Cycle_reference_chart`). The `$4015` mnemonics are confirmed on `APU_registers` as `---D NT21` (write) and `IF-D NT21` (read).
- **`$4015` bit 5** is undriven in the HDL, so this spec calls it open bus. The wiki never states its behaviour explicitly — confirm whether the die leaves it floating for the whole read.
- **`$4016`/`$4017` read value.** The wiki describes only the `/IN0`/`/IN1` strobes; how many data bits the external device drives onto the bus is board-level and is not in the wiki pages read here.
- **`$4016`/`$4017` in test mode.** NESdev `CPU_pinout` states that with pin 30 (`TST`/`DBG`) high, `$4016`/`$4017` read as open bus on the 2A03G. This repository's HDL does not gate the strobes with `DBG` (`regs.v`: `not(n_R4016, pla[23])`, `not(n_R4017, pla[21])`), so the APU still asserts `/IN0`/`/IN1` in test mode; whether a controller answers is board-level and unverified here. This spec follows the schematic (strobes still asserted).
- **`M2` duty-cycle conflict.** `old_2a03.md` gives 17/24 for the 2A03G and 15/24 for the letterless 2A03 (SR4 vs SR3); NESdev `CPU_pinout` assigns the fractions the other way round (15/24 for the 2A03E/G/H, 17/24 for the letterless part) and adds 19/32 for the 2A07. This spec keeps the schematic assignment and notes the conflict in §3. Neither source defines the 24-unit reference period; the derivation should be re-checked against `DIV_2A03` / `div_letterless`.
- **PAL LFO frequencies** are derived here from the 8313 / 16625 / 24939 CPU-cycle intervals in `clkgen.md`; the wiki states frequencies (240/120/192/96 Hz) only for the 2A03. Treat the PAL 200/100/160/80 Hz figures as derived, not quoted.
- **`cycles.md`** is an appendix of 13 images ("uroboros" feedback loops) with no text; it yielded no statements that could be cited. The Verifier should treat its absence as a limitation of the source, not of this section.
- **`RDY2`** appears only in `core.md`; there is no `RDY2` net in `HDL/APU/apu_top.v`, so the 2A07 "debug WAIT" behaviour is wiki-only and unconfirmed by HDL.
- **`$401B–$401F`** are not decoded by the APU; whether the CPU-side read returns pure open bus on retail hardware is not documented in the pages read.
