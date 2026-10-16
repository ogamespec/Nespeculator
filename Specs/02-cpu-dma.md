# 02-cpu-dma — OAM DMA, DMC DMA, `RDY` halting and bus conflicts

**Status:** consensus (round 2)
**Scope:** The two DMA engines inside the 2A03/2A07 CPU as seen from the CPU and
from the cartridge: OAM DMA (`$4014`), DMC (DPCM) sample DMA, the `RDY` halt
handshake, the address multiplexer and the resulting bus conflicts. APU sound
generation, the DMC sample format and the PPU's OAM internals are excluded
(`03-apu-dmc.md`, `04-ppu-sprites.md`).
**Primary sources:** `BreakingNESWiki/APU/{dma,dpcm,core,cycles,regs,pads,common,waves,old_2a03,pal,Readme}.md`,
`BreakingNESWiki/6502/{pads,decoder}.md`; `HDL/APU/{dma,dmabuf,core,dpcm,regs,clkgen,pads,io}.v`,
`HDL/Common/{dlatch,rsff}.v`, `HDL/Core6502/{pads,core_top}.v`,
`HDL/Framework/Icarus/apu/{oam_dma,both_dma}.v`.

## 1. Where DMA sits in the 2A03

The CPU contains two independent DMA engines plus the arbitration logic that lets
them share the 6502 core's external bus (`APU/dma.md`, `HDL/APU/dma.v`). The core
is one bus master among several (`APU/Readme.md`), and `RDY` is internal — the chip
has no `RDY` pin (`APU/pads.md`, `6502/pads.md`).

| Unit | Trigger | Transfer |
|---|---|---|
| OAM DMA (sprite DMA) | write to OAMDMA (`$4014`) | 256 bytes, source page `$xx`, destination hardwired to OAMDATA (`$2004`) |
| DMC DMA (DPCM DMA) | buffer empty, channel enabled, bytes remain | 1 byte from `$8000–$FFFF` into the DMC sample buffer |

| Signal | Meaning | Level |
|---|---|---|
| `RDY` | core readiness; `0` = core halted | low |
| `RUNDMC` | DMC owns DMA; OAM DMA in standby | high |
| `/DMCAB` | DMC owns the external address bus (HDL `n_DMCAB`) | low |
| `DMCRDY` | DMC ready; `0` forces `RDY` to `0` | high |
| `SPR/CPU`, `SPR/PPU` | OAM DMA read phase / write-to-`OAMDATA` phase | high |
| `W4014` | write to `OAMDMA` | high |

The signal names above are the repository's internal names; none of them is a
package pin (`APU/pads.md`). NESdev's `CPU_pinout` likewise has no `RDY` pin — on
the 2A07 the halt function is reached through pin 30 (`TST`, the repository's
`DBG`). NESdev has no separate names for the DMA control signals.

## 2. Clocks, get/put phases and the `RDY` halt handshake

- **CPU cycle** = one PHI0 period: NTSC 21.477272 MHz ÷ 12 ≈ 558.73 ns; PAL
  26.601712 MHz ÷ 16 ≈ 601.46 ns (`APU/clkgen.md`).
- **APU cycle** = ACLK = PHI0 ÷ 2 = **2 CPU cycles**: NTSC ≈ 1117.46 ns, PAL ≈
  1202.93 ns. `ACLK1` is the first phase, `/ACLK2` the second; they do not overlap
  and are not exact complements (`APU/clkgen.md`).
- DMA can only **get** (read) on `ACLK1` and **put** (write) on `/ACLK2`: the
  NESdev *get*/*put* slots. Which CPU-cycle parity is a get depends on the
  power-on CPU/APU alignment (`HDL/APU/clkgen.v`; NESdev reports it is random).
- All OAM DMA registers, counters and control are clocked by `ACLK`; `PHI1` only
  detects a core read cycle and `PHI2` only latches the DMA buffer. APU register
  **writes** are gated to `PHI1 = 0` (the `PHI2` half) while reads are not
  (`APU/dma.md`, `HDL/APU/regs.v`), so a `$4014` write is captured late and the
  transfer cannot begin before the next read cycle.

**Halt rule.** With `RDY = 0` the core repeats its last read cycle, makes no
forward progress and services no interrupts (NESdev `DMA`;
`HDL/Core6502/dispatch.v`). It ignores a low `RDY` on write
cycles, so a request arriving on a write waits for the next read cycle: OAM DMA
from the `$4014` write (in `PHI2`), DMC DMA at or after the scheduled fetch
(`APU/dma.md`, `APU/dpcm.md`). A wait of up to 3 CPU cycles is possible: an RMW
instruction has two consecutive write cycles and an interrupt frame three
(NESdev `DMA`).

```
sprdma_rdy = NOR(StopDma, StartDma)   // either FF set -> 0
RDY        = sprdma_rdy AND DMCRDY    // DMCRDY=0 always halts the core
StartDma   : set by W4014, cleared when the transfer starts
StopDma    : set when the transfer starts (DOSPR), cleared by the counter end
```

`RDY` therefore stays low for the whole OAM DMA, and a DMC fetch forces it low for
its own duration even during an OAM DMA (`HDL/APU/dma.v`). `/PRDY` ("Previous
Ready", HDL `n_PRDY`) is a delayed copy of `RDY`: if the core was not ready when
the previous instruction ended and the next instruction is a conditional branch,
its first cycle (T-state T0) is skipped (`6502/decoder.md`).

## 3. OAM DMA (`$4014`)

- Write the source page number to OAMDMA (`$4014`, write-only); bits 7–0 become
  A15–A8 of the source and A7–A0 are cleared. The destination is hardwired to
  OAMDATA (`$2004`) and cannot be configured (`APU/dma.md`). The engine reads
  `$xx00–$xxFF` in order; the low byte is an 8-bit **up counter** whose end is the
  carry out of bit 7 (256 reads + 256 writes, `HDL/APU/dma.v`).
- The high byte is a register loaded by `W4014`. A second write before the core
  halts (the second write of an RMW such as `INC $4014`) reloads the page and
  clears the counter: the transfer copies from the **last** page written.
- `SPR/CPU` and `SPR/PPU` alternate: read into the DMA buffer, then write it to
  `$2004`. The byte is latched on `PHI2`; during `SPR/PPU` the buffer drives the
  unbuffered external data bus and the external `R/W` is forced to write
  (`HDL/APU/dmabuf.v`). The `$2004` address is a constant, so the byte lands at the
  PPU's current `OAMADDR ($2003)` (`04-ppu-sprites.md`).

**Per-cycle timing** (513-cycle case; the halt cycle falls on a put slot):

| CPU cycle | Slot | Master | R/W | Address | Event |
|---|---|---|---|---|---|
| 0 | — | CPU | write | `OAMDMA ($4014)` | `W4014` latched in `PHI2`; page loaded, counter cleared, `RDY` low |
| 1 | put | CPU | read | A | halt cycle; core repeats its read, no transfer |
| 2 | get | OAM DMA | read | `$xx00` | byte 1 into the DMA buffer |
| 3 | put | OAM DMA | write | `OAMDATA ($2004)` | byte 1 written to OAM |
| … | | | | | bytes 2–255: 254 read/write pairs |
| 512 | get | OAM DMA | read | `$xxFF` | byte 256 into the DMA buffer |
| 513 | put | OAM DMA | write | `OAMDATA ($2004)` | byte 256 written; end carry clears `StopDma`, `RDY` released |
| 514 | — | CPU | read | A | core completes the read it was halted on |

If the halt cycle falls on a get slot, the next (put) slot cannot be used for a
read, so one no-op **alignment** cycle is spent, giving **514** CPU cycles. OAM
DMA therefore costs **513 or 514 CPU cycles** = 256.5 or 257 APU cycles:
≈ 286.6 / 287.2 µs on NTSC or ≈ 308.5 / 309.2 µs on PAL. The CPU-cycle counts are
the same on NTSC, PAL and Dendy; only wall-clock duration changes with the
region's clock.

## 4. DMC DMA

A fetch is requested when the channel is enabled (`$4015` write, bit 4 = 1), the
sample byte counter is not exhausted and the sample buffer is empty
(`APU/dpcm.md`, `HDL/APU/dpcm.v`). While `RUNDMC = 1` the OAM DMA is in standby,
and `/DMCAB = 0` takes the address bus.

- Sample address (`$4012`) = `$C000 + ($4012 × $40)`: load value `{1, $4012[7:2]}`
  into A14–A8 and `{$4012[1:0], %000000}` into A7–A0.
- A15 is hardwired to `1`, so DMC addresses are always `$8000–$FFFF`; the 15-bit
  address counter increments per byte and wraps `$FFFF → $8000`.
- Sample length (`$4013`) = `($4013 × 16) + 1` bytes, held in a 12-bit down
  counter loaded with `{$4013, %0000}` (`HDL/APU/dpcm.v`).
- A fetched byte fills the sample buffer, which the output unit then empties; an
  empty buffer requests the next fetch. This is the sample buffer of
  `03-apu-dmc.md`; only the fetch path is specified here.

DMC DMA halts the core only for the fetch. The two kinds are scheduled
differently (NESdev `DMA`): a **load** fetch — after `$4015` bit 4 is set with an
empty buffer — is scheduled to halt on a get cycle during the 2nd APU cycle after
the write, i.e. on the 3rd or 4th CPU cycle after it, which matches the 3/4
alignment model below; a **reload** fetch follows the buffer emptying and is
scheduled to halt on a put slot. Once halted, every fetch performs a dummy cycle
and, on the wrong phase, an alignment cycle, so the fetch itself costs 3 or 4 CPU
cycles. "Halt slot" below is the slot on which the halt succeeds; a failed attempt
(the CPU was writing) is retried on the next CPU cycle:

**Fetch timing** (NESdev `DMA`):

| DMA event | CPU cycles | Halt slot | Cycle roles |
|---|---|---|---|
| load fetch, halt on get | 3 | get | halt, dummy, get |
| load fetch, halt on put | 4 | put | halt, dummy, alignment, get |
| reload fetch, halt on put | 4 | put | halt, dummy, alignment, get |
| reload fetch, halt on get | 3 | get | halt, dummy, get |
| halt ignored on a write | +1 per ignored halt | — | RMW has 2 consecutive writes, interrupts 3 |

The DMC interrupt flag is set when the fetch that exhausts a non-looping sample
completes **and the `$4010` IRQ enable (bit 7) is set**; with bit 7 = 0 the flag is
held cleared, so `$4015` bit 7 reads 0 and no IRQ is raised (`HDL/APU/dpcm.v`:
`int_ff` resets on `~$4010[7]` and reset dominates set; NESdev `APU_DMC`:
clearing the enable clears the flag). With the enable set, the flag is reported in
the `$4015` read (bit 7) and delivered through the APU interrupt line. While `RDY`
is low the core cannot take it, so it is delayed to the first instruction boundary
after the fetch.

## 5. Address multiplexer and mid-DMA register decode

The address multiplexer selects one of four sources, in this priority
(`HDL/APU/dma.v`):

| Priority | Source | Selected when | Address driven |
|---|---|---|---|
| 1 | `SPR/PPU` | OAM DMA write phase | constant `$2004` |
| 2 | `SPR/CPU` | OAM DMA read phase | OAM source counter `$xx00–$xxFF` |
| 3 | DMC | `/DMCAB = 0` | DMC address counter `$8000–$FFFF` |
| 4 | CPU | none of the above | core address `CPU_A[15:0]` |

`RUNDMC = 1` forces `SPR/CPU = SPR/PPU = 0`, so a DMC fetch wins the bus even
during an OAM DMA, and the OAM counter is frozen because its step signal is gated
by `RUNDMC`.

**The register decoder does not see a single address.** APU register *space*
(`$4000–$401F`) is predecoded from the core address, but the *register index* is
decoded from the multiplexer output (`APU/regs.md`, `HDL/APU/regs.v`):

```
register address = { CPU_A[15:5] , bus_A[4:0] }
```

While halted the core keeps repeating the same read address, so a DMA bus address
can unintentionally activate an APU register if that read address is in
`$4000–$401F`. Because the halt is accepted only on read cycles, only read
registers can be activated (write selects are gated by the core's `R/W`). NESdev
documents the same quirk in its CPU memory map (NESdev `CPU_memory_map`):

| Bus `A[4:0]` | Register activated | Effect |
|---|---|---|
| `%10101` (`$xx15`) | APU status (`$4015`) | APU drives the status byte; conflicts with the DMA read; frame counter (`SoftCLK`) IRQ flag cleared |
| `%10110` (`$xx16`) | JOYPAD1 (`$4016`) | `/IN0` (joypad 1 `/OE`) asserted → controller 1 clocked; the controller/expansion device drives bits 4–0 |
| `%10111` (`$xx17`) | JOYPAD2 (`$4017`) | `/IN1` (joypad 2 `/OE`) asserted → controller 2 clocked; the controller/expansion device drives bits 4–0 |
| `%11000–%11010` | `$4018–$401A` test registers | 2A03 test mode only (`DBG = 1`); absent on 2A07 |

A DMC fetch starts at `$C000 + ($4012 × $40)`; the low byte loads
`{$4012[1:0], %000000}` (`HDL/APU/dpcm.v`), whose bits 4–0 are zero, so `A[4:0]`
runs through all 32 values from 0 regardless of `$4012` as the sample plays: a
fetch whose byte index is 21, 22 or 23 (mod 32) collides when the core is halted
in `$4000–$401F`. OAM DMA source offsets behave the same way (byte offset mod 32).

**Bus conflicts.** The internal and external data buses are directly connected
with no buffering (`APU/pads.md`), so whenever the register selector activates a
register during a DMA read both the APU and the cartridge drive the bus: the APU
output is corrupted and the byte the DMA latches is unreliable.

**Repeated reads.** On every DMA no-op cycle (halt, dummy, alignment) the core's
read address is still on the bus, so the rejected read is performed again and is
externally visible on the 2A03 (NESdev `DMA`; `HDL/Core6502/dispatch.v`). This
corrupts the PPU status (`PPUSTATUS ($2002)`), VRAM data (`PPUDATA ($2007)`), APU
status (`$4015`) and joypad (`$4016`/`$4017`) reads that collide with a DMC fetch.
On the joypad
registers NESdev adds a rank-4 detail not stated by the schematic pages read: the
`/IN0`/`/IN1` output enables stay asserted across adjacent reads of the same
register, so a contiguous block of repeated joypad reads clocks the controller
once (NESdev `DMA`, `Standard_controller`). NESdev reports the 2A07 fixes the
extra reads; `APU/pal.md` leaves the 2A07 `RDY` connection and register select as
TBD, so the mechanism is not established here.

## 6. DMC DMA and OAM DMA together

Nothing synchronises the OAM and DMC engines, so a DMC fetch can land anywhere
during an OAM DMA: the fetch runs, the OAM transfer is paused and re-aligns
afterwards (`APU/waves.md`, `APU/dpcm.md`). The OAM transfer is never restarted
and its counter never steps during the fetch (§5).

**Collision cost** (NESdev `DMA`):

| Collision point | Extra cost |
|---|---|
| DMC fetch in the middle of OAM DMA | +2 CPU cycles (1 DMC get, 1 OAM re-align) |
| DMC fetch on the second-to-last OAM put | +1 CPU cycle total (the DMC get falls after OAM ends) |
| DMC fetch on the last OAM put | +3 CPU cycles beyond the end of the OAM DMA |

The wiki waveform set shows the same arbitration: part of the DMC transfer is
reserved for a possible OAM transfer, and from a "window" after the start of the
DMC transfer an active OAM DMA is suppressed by `RUNDMC` (`APU/waves.md`,
`HDL/Framework/Icarus/apu/both_dma.v`).

## 7. Region and revision notes

- Cycle counts (513/514, 3/4, +1/+2/+3) are CPU-cycle counts, identical on NTSC,
  PAL and Dendy; only the clock period differs (APU cycle ≈ 1117.46 ns NTSC vs
  ≈ 1202.93 ns PAL). Dendy MUST NOT be clocked as PAL (NTSC-like CPU:PPU ratio).
- 2A07 (PAL): debug registers absent, `DBG` (pin 30, NESdev `TST`) becomes an
  extra readiness input `RDY2` (`APU/core.md`, NESdev `CPU_pinout`), and the DMC
  extra-read conflict is reported fixed (NESdev `APU_DMC`); `APU/pal.md` leaves
  2A07 `RDY`/register decode as TBD.
- Letterless (early) 2A03: the `StartDma` flip-flop is not cleared by `RES`
  (`APU/old_2a03.md`).

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | Sprite data rotated or at the wrong Y after OAM DMA | the 256 writes go through `OAMDATA ($2004)`, whose position is the PPU's `OAMADDR ($2003)` | set `OAMADDR ($2003)` = `$00` before writing `OAMDMA ($4014)` (`04-ppu-sprites.md`) |
| 2 | OAM DMA occasionally takes one cycle longer | the halt cycle can land on a get slot, adding an alignment cycle | budget 514 CPU cycles, not 513 |
| 3 | The read at the start of the instruction after the `$4014` write happens more than once | while halted the core repeats the read it was on, so every DMA no-op cycle (halt, and any alignment cycle) shows that address again | apply the read's side effects each time; avoid side-effect register reads at a DMA boundary |
| 4 | `INC $4014` copies the wrong page | both writes of the RMW reload the page register before the halt | treat the last `$4014` write as authoritative |
| 5 | Garbled controller reads while DMC DMA runs | the DMC fetch repeats the core's `$4016`/`$4017` read and can clock the other joypad via the mixed address | read controllers repeatedly until two results match |
| 6 | A `PPUDATA ($2007)` read returns the wrong byte and the VRAM address jumps | the PPU sees the repeated `$2007` reads on the halt/dummy/alignment cycles and increments each time | do not read `$2007` during DMC DMA, or resynchronise `PPUADDR ($2006)` afterwards |
| 7 | A vblank poll is lost | repeated `PPUSTATUS ($2002)` reads during DMA no-op cycles clear the flag more than once | read `$2002` once per poll; prefer NMI |
| 8 | Audible click or wrong DMC sample byte | bus conflict: fetch address bits 4–0 select `$4015`/`$4016`/`$4017` while the core is halted in `$4000–$401F` | avoid colliding sample alignments, or stop DMC playback around critical reads |
| 9 | DMC IRQ arrives 3–4 cycles late | `RDY` low blocks the core from taking interrupts | do not time critical code across a DMC fetch |
| 10 | The frame counter (`SoftCLK`) IRQ flag disappears unobserved | repeated `$4015` reads during a DMA fetch clear the flag | read `$4015` once and cache the value |

## Programming / implementation notes

- To run an OAM DMA: set `OAMADDR ($2003)` to `$00`, then write the source page to
  `OAMDMA ($4014)`. `OAMADDR` handling is the PPU's (`04-ppu-sprites.md`).
- An emulator MUST halt the core via `RDY`, MUST ignore the halt on write cycles,
  MUST retry the halt on the following cycles (up to 3 consecutive writes) and
  MUST re-perform the interrupted read when the halt ends.
- An emulator MUST model get/put as `ACLK1`/`/ACLK2` and MUST accept either
  power-on parity: OAM DMA 513 or 514 CPU cycles, a DMC fetch 3 or 4.
- The address multiplexer MUST use the priority in §5; during a DMC fetch the DMC
  address MUST win over the OAM source and the OAM counter MUST NOT step.
- The register decoder MUST be `{CPU_A[15:5], bus_A[4:0]}`, with APU write selects
  gated by the core's `R/W`; otherwise the §5 conflicts cannot be reproduced.
- Every DMA no-op cycle MUST repeat the core's read, including its side effects on
  `$2002`, `$2007`, `$4015`, `$4016` and `$4017`; on cycles where the DMA unit
  actually drives the bus the core's read MUST NOT be repeated.
- OAM DMA MUST write all 256 bytes to `$2004`; a second `$4014` write before the
  halt MUST reload the full source address. DMC DMA MUST halt the core only for
  the fetch, not the whole sample; sample setup is `$C000 + $4012×$40`, length
  `$4013×16 + 1`, started by `$4015` bit 4.
- A DMC fetch inside an OAM DMA MUST pause the OAM transfer and re-align it (about
  +1 CPU cycle), not restart it; the collision costs in §6 MUST be applied on top
  of the 513/514-cycle base.
- The DMC IRQ MUST be deferred while the core is halted and taken at the first
  instruction boundary after the fetch.
- Emulators SHOULD apply the 2A07 and early-2A03 differences only for those
  revisions, and MUST NOT infer Dendy clocking from PAL.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| The two DMA units, `RDY` halt, get/put cycles, alignment, dummy and repeated reads | `DMA` |
| 2A03/2A07 pinout, pin 30 `TST`, no `RDY` package pin | `CPU_pinout` |
| CPU cycle period and the NTSC /12, PAL /16 clock division | `Cycle_reference_chart` |
| APU clock phases (NESdev `apu_clk1`/`apu_/clk2`) and die-level signal names | `Visual_circuit_tutorial` |
| OAM layout, `OAMDATA ($2004)`, the 256-byte `OAMDMA ($4014)` copy | `PPU_OAM` |
| DMC registers `$4010`–`$4013`, `$4015` enable/status, sample address and length, load/reload DMA | `APU_DMC` |
| APU register map `$4000–$4017`, `$4015` status bits, joypad registers | `APU_registers`, `APU` |
| PPU registers and the read side effects of `PPUDATA ($2007)` / `PPUSTATUS ($2002)` | `PPU_registers` |
| CPU memory map; the `$4000–$401F` register-decode quirk that lets DMA activate registers | `CPU_memory_map` |
| Bus conflicts and open bus during DMA | `Bus_conflict`, `Open_bus_behavior` |
| DMC-vs-joypad conflict and the redundant-read workaround | `Controller_reading`, `Standard_controller` |
| Interrupts blocked while `RDY` is low | `DMA` |
| 2A07/PAL and clone differences | `APU_DMC`, `CPU_pinout` |

Page names were resolved against the local NESdev Wiki cache / Wayback Machine.
There is **no** `Visual_2A03` wiki page — the die-level material (and the `apu_clk1`/`apu_/clk2`
names) is at `Visual_circuit_tutorial`; `02-cpu-instructions.md` still lists the
former and should be corrected (see `## Open questions`).

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| OAM DMA counters, control, end of transfer | `APU/dma.md`, `APU/common.md`, `APU/waves.md` | `HDL/APU/dma.v` |
| Address multiplexer and priority | `APU/dma.md`, `APU/dpcm.md` | `HDL/APU/dma.v` |
| `RDY` handshake, `DMCRDY`, `RUNDMC` | `APU/dma.md`, `APU/core.md`, `APU/dpcm.md` | `HDL/APU/dma.v` |
| DMA buffer, forced `R/W`, bus direction | `APU/core.md`, `APU/pads.md` | `HDL/APU/dmabuf.v`, `HDL/APU/pads.v` |
| DMC fetch control, counters, 3/4-cycle timing | `APU/dpcm.md`, `APU/common.md`; NESdev `DMA` | `HDL/APU/dpcm.v` |
| DMC interrupt flag and its delivery | `APU/dpcm.md` | `HDL/APU/dpcm.v` |
| Register select from the mixed address | `APU/regs.md` | `HDL/APU/regs.v` |
| APU clock phases and periods | `APU/clkgen.md`, `APU/cycles.md` | `HDL/APU/clkgen.v` |
| Internal `RDY` / `/PRDY` (`n_PRDY`) and the Branch-T0 skip | `6502/pads.md`, `6502/decoder.md` | `HDL/Core6502/pads.v`, `HDL/Core6502/core_top.v` |
| OAM/DMC concurrent waveforms, collision cost | `APU/waves.md`; NESdev `DMA` | `HDL/Framework/Icarus/apu/oam_dma.v`, `both_dma.v` |
| Early-2A03 `StartDma` reset difference | `APU/old_2a03.md` | — |
| Illegal register activation, conflict outcome | — (not stated by ranks 1–3; NESdev `DMA`, `CPU_memory_map`) | `HDL/APU/regs.v` |

## Open questions

- **Get/put parity.** The wiki derives `ACLK1`/`/ACLK2` from `PHI1`/`PHI2` with a
  reset-defined state; NESdev reports random power-on parity. Unresolved.
- **NESdev's own DMC timing wording.** `APU_DMC` says (explicitly as speculation)
  that the DMC unit holds `RDY` low for a fixed 4 cycles; `DMA` gives 3 or 4
  depending on alignment. This file follows the 3/4 model, which matches the
  schematic's dummy + optional alignment structure (§4).
- **OAM extra-cycle wording.** `PPU_OAM` writes "513 CPU cycles (+1 on odd CPU
  cycles)"; `DMA` attributes the +1 to the random power-on get/put parity. The
  random-parity model is the one used here (§2, §3); the two are only reconciled
  if the CPU-cycle parity is fixed, which NESdev denies.
- **2A07 mechanism.** NESdev reports the 2A07 has no extra-read conflict, but
  `APU/pal.md` marks 2A07 `RDY` and register select as TBD, so the fix cannot be
  stated from rank 1–2 evidence.
- **Conflict resolution electrically.** The wiki shows both drivers enabled but not
  whether the result is wired-AND, OR or undefined, so the latched byte is called
  unreliable.
- **Load vs reload scheduling.** The load fetch's 2nd-APU-cycle halt schedule and
  the 3/4-cycle fetch table (§4) are NESdev rank-4 numbers; the wiki schematics
  (`APU/dpcm.md` images) show the control latches but no textual labels, and the
  silicon does not state a cycle count on its own.
- **Joypad `/OE` across adjacent reads.** NESdev's claim that a contiguous block of
  repeated `$4016`/`$4017` reads clocks the controller once (rank 4) is not
  confirmed by the schematic pages read (`APU/regs.md`, `APU/pads.md`).
- **DMC/OAM arbitration window** length: `APU/waves.md` describes the "window"
  without giving a number of `ACLK` phases.
- **Dendy/clone DMA timing** is not covered by the wiki; NESdev reports UA6527P
  clones end samples 1 APU cycle later, so no Dendy rule is given here.
