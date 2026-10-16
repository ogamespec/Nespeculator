# 02-cpu-core — 2A03/2A07 CPU core

**Status:** consensus (round 2)
**Scope:** The Ricoh 2A03 (NTSC) / 2A07 (PAL) embedded MOS 6502 core as seen by a
programmer: the register set, the status register and its exact flag controls,
the stack, the address map visible from the core, the address modes, the
`PHI1`/`PHI2` cycle structure, `RDY`, the interrupt/reset vectors and the
BRK sequence, the power-on state, and the differences from a stock NMOS 6502.
Instruction-by-instruction opcode tables, cycle costs and page-crossing rules
are in `02-cpu-instructions.md`; DMA and the `RDY` handshake as used by OAM/DMC
are in `02-cpu-dma.md`; the chip-level pins are in `01-platform-overview.md`.
**Primary sources:** `BreakingNESWiki/6502/Readme.md`, `regs.md`, `pc.md`,
`flags.md`, `interrupts.md`, `ir.md`, `whatwhen.md`, `busmux.md`,
`address_bus.md`, `data_bus.md`, `pads.md`, `clock.md`, `context_control.md`,
`regs_control.md`, `extra_counter.md`, `dispatch.md`; `BreakingNESWiki/APU/core.md`,
`BreakingNESWiki/APU/pads.md`, `BreakingNESWiki/Ricoh.md`;
`HDL/Core6502/` (`core_top.v`, `pads.v`, `clock.v`, `regs.v`, `flags.v`,
`flags_control.v`, `pc.v`, `pc_control.v`, `addr_bus.v`, `data_bus.v`,
`busmux.v`, `bus_control.v`, `brk.v`, `ir.v`, `branch_logic.v`, `dispatch.v`,
`alu_control.v`, `extra_counter.v`).

## 1. The core inside the 2A03/2A07

The CPU of the NES/Famicom is not a separate chip: the MOS 6502 core is
integrated into the Ricoh APU, `RP2A03` for NTSC and `RP2A07` for PAL
(`BreakingNESWiki/Ricoh.md`). The die photos show the core as a reduced copy of
the standalone processor, in Ricoh mask style (`BreakingNESWiki/APU/core.md`).

The core is clocked by `PHI0`, produced on-chip by the APU clock divider from
the master clock:

| Region | APU part | Master clock | `PHI0` = CPU cycle |
|---|---|---|---|
| NTSC | RP2A03 | 21.477272 MHz | 21.477272 / 12 = 1.7897727 MHz |
| PAL | RP2A07 | 26.601712 MHz | 26.601712 / 16 = 1.662607 MHz |

All cycle counts in the specs are `PHI0` periods; the divider is described in
`01-platform-overview.md` and `03-apu-overview.md`, and the core only sees `PHI0`.

The core exposes only these signals to the rest of the APU
(`BreakingNESWiki/APU/core.md`, `BreakingNESWiki/APU/pads.md`):

| Core signal | Dir | Meaning on the 2A03/2A07 |
|---|---|---|
| `A0–A15` | out | Address bus (`CPU_A` into the APU address multiplexer); disconnected (`z`) during reset. |
| `D0–D7` | bidir | Data bus, connected directly to the APU internal data bus. |
| `R/W` | out | Core data-bus direction; the external `R/W` pin is driven by the APU DMA buffer logic, not directly. |
| `SYNC` | out | `T1` (opcode fetch) indicator. **Not connected — floating.** |
| `SO` | in | Set-overflow input, **tied to 1**; its falling-edge detector therefore never fires. |
| `RDY`, `RDY2` | in | Ready terms. `RDY2 = 1` on the 2A03; on the 2A07 `RDY2 = /DBG`, so `DBG` = 1 forces a debug WAIT (`DBG` is pin 30, NESdev `TST`). |
| `/IRQ`, `/NMI`, `/RES` | in | Interrupts and reset. `/IRQ` is combined on-chip with the frame-counter and DMC interrupts; `/NMI` goes straight to the core. NESdev names the reset pin `/RST` (`CPU_pinout`); this spec keeps the repository name. |

`SYNC` and `SO` therefore have no observable effect on a real NES/Famicom; the
2A07's debug WAIT is the only other core-visible difference besides the clock
ratio. `M2` (pin 31) is a modified `PHI2`, active 1.5 `CLK` (three half-cycles)
*before* the core's `PHI2`, so external devices start driving/releasing the bus
first. During reset, `M2` and `R/W` are `z` (open drain) in retail mode
(`BreakingNESWiki/APU/pads.md`).

## 2. The cycle: PHI1 and PHI2

Each CPU cycle is one `PHI0` period split into two half-cycles (NESdev writes
them φ1/φ2), a "dual-rail" clock (`BreakingNESWiki/6502/clock.md`, `whatwhen.md`):

| `PHI0` | `PHI1` | `PHI2` | Core activity |
|---|---|---|---|
| 0 | 1 | 0 | **Set address and R/W mode.** Registers drive the internal buses, the ALU is loaded, and the address is latched into the external address registers. |
| 1 | 0 | 1 | **Read/write data.** Data is transferred on the data bus, the PC is incremented (`n_IPC` is asserted in both half-cycles; `context_control.md`), and the internal buses are precharged. |

- `PHI1` and `PHI2` are never high together: the internal versions lag `PHI0`
  slightly and the low level is stretched by an asymmetric inverter chain, so the
  two are guaranteed non-overlapping with a small dead time (`clock.md`).
- The address is set during `PHI1` and held by the `ABL`/`ABH` output registers
  for the whole cycle; they are re-loaded from `ADL`/`ADH` only when `ADL/ABL` /
  `ADH/ABH` are active (`address_bus.md`, `HDL/Core6502/addr_bus.v`).
- The `R/W` mode is latched from the dispatcher during `PHI1`; `R/W = 1` means
  the processor reads and `R/W = 0` means it writes (`pads.md`,
  `HDL/Core6502/data_bus.v`).
- The processor can be halted only by `RDY`, or by `RDY2` on the 2A07 (`pads.md`,
  NESdev `CPU_pinout`); the halt mechanism is described in `02-cpu-dma.md`.

## 3. Programmer-visible registers

| Register | Width | Description |
|---|---|---|
| A | 8 | Accumulator (`AC`). |
| X | 8 | Index register X. |
| Y | 8 | Index register Y. |
| S | 8 | Stack pointer. The stack lives at `$0100–$01FF`; the effective address is `$0100 + S`. Decrements on push, increments on pull. |
| P | 8 | Status register (NESdev also calls it processor status), physically distributed as individual flag latches. |
| PC | 16 | Program counter, split into `PCL` (PC\[7:0\]) and `PCH` (PC\[15:8\]). |

The core also holds non-programmer registers: `IR` (opcode), `PD` (predecoded
opcode), `AI`/`BI`/`ADD` (ALU), `ABH`/`ABL` (output address), `DL` (data latch),
`DOR` (data output), `PCHS`/`PCLS` and the stack shadow latch (`Readme.md`,
`pc.md`, `HDL/Core6502/regs.v`).

S is special: during `PHI1` its output latch drives the old S onto `SB`/`ADL`
while its input ("shadow") latch takes the new value from `SB`; `S/S` refreshes
the input latch when `SB/S` is inactive. Both happen in the same half-cycle,
which is what makes the push/pull sequences work (`regs.md`, `regs_control.md`,
`HDL/Core6502/regs.v`).

## 4. The status register P

NESdev calls the P bits the **status flags** (`Status_flags`). Bit order as pushed (bit 7 left, bit 0 right):

```
7  bit  0
---- ----
NV1B DIZC
|||| ||||
|||| |||+- C  Carry
|||| ||+-- Z  Zero
|||| |+--- I  Interrupt disable
|||| +---- D  Decimal (present, but not connected in the 2A03/2A07)
|||+------ B  Break (the B flag; not a stored flag — see below)
||+------- 1  Unused — no CPU effect; always pushed as 1
|+-------- V  Overflow
+--------- N  Negative (sign)
```

Flag update sources, from the flag latches (`BreakingNESWiki/6502/flags.md`,
`HDL/Core6502/flags.v`):

| Flag | Set by | Cleared by | Notes |
|---|---|---|---|
| C | `SEC` (`IR5`), ALU carry `ACR`, `DB0` on `DB/P` | `CLC` (`IR5`), `ACR`, `DB0` on `DB/P` | ADC/SBC/CMP/CPX/CPY/ASL/LSR/ROL/ROR/PLP/RTI. |
| Z | `/DBZ` (all `DB` bits zero) | `/DBZ` false | Latched on `PHI1`; also computed for the ALU result via `ZTST`. |
| I | `SEI` (`IR5`), `DB2` on `DB/P`, BRK sequence (`BRK6E`) | `CLI` (`IR5`), `DB2` on `DB/P` | Forced to 1 by the interrupt sequence during cycle 6. |
| D | `SED` (`IR5`), `DB3` on `DB/P` | `CLD` (`IR5`), `DB3` on `DB/P` | The flag itself works normally; the BCD correction circuits are disconnected (§10). |
| V | `SO` pin (falling edge; dead on retail parts), ALU overflow `AVR`, `DB6` on `DB/P` | `CLV` (`0/V`), `AVR` with no overflow, `DB6` on `DB/P` | On the 2A03/2A07 `SO` is tied high, so only `CLV` and ADC/BIT control V. |
| N | `DB7` on `DB/P` | `DB7` on `DB/P` | Bit 7 of the loaded/computed value. |
| B | BRK sequence when the interrupt source is BRK | — | Not a stored flag. It is generated by the BRK sequencer and appears only as bit 4 of the byte pushed on the stack. |

Bit 5 is not connected to any flag: when P is placed on the internal `DB` the
`DB5` line is left undriven and (because `DB` is precharged, §8) reads as 1.
`/DBZ` nevertheless includes `DB5` in the all-zero test (`flags.md`,
`HDL/Core6502/flags.v`). NESdev states the same rule as "always pushed as 1" and
notes that bits 5 and 4 are ignored when pulling (`Status_flags`).

## 5. Address map visible from the core

`$0000–$00FF` is the zero page, `$0100–$01FF` the stack and `$0200–$07FF` the
rest of the 2 KB internal RAM; on retail hardware `$0800–$1FFF` mirrors it,
`$2000–$401F` is PPU/APU/expansion I/O and `$4020–$FFFF` is cartridge/expansion
space (`01-platform-overview.md` §4; NESdev `CPU_memory_map`). The core does no
decoding: it drives every computed address onto `A0–A15` and leaves the board to
map it. Zero-page addressing always produces a `$00xx` address — the high byte is
forced to `$00` by the core, not read from memory (`Readme.md`).

## 6. Address modes

| Mode (NESdev) | Syntax example | Effective address / operand |
|---|---|---|
| Implied | `CLC` | none |
| Accumulator | `ASL A` | A |
| Immediate | `LDA #$1C` | A ← `$1C` |
| Zero page | `LDA $56` | `[$0056]` — high byte forced to `$00` |
| Zero page,X | `LDA $33,X` | `[$0033 + X]`, wrapped inside `$00xx` |
| Zero page,Y | `LDX $33,Y` | `[$0033 + Y]`, wrapped inside `$00xx` |
| Absolute | `LDA $1234` | `[$1234]` |
| Absolute,X | `LDA $1234,X` | `[$1234 + X]` |
| Absolute,Y | `LDA $1234,Y` | `[$1234 + Y]` |
| Indirect | `JMP ($1234)` | PC ← `[$1234]` |
| (Indirect,X) | `LDA ($34,X)` | `[[$0034 + X]]`; the `$0034 + X` sum wraps around 256 bytes (`$FF + $02 = $0001`, not `$0101`) |
| (Indirect),Y | `LDA ($2A),Y` | `[[$002A] + Y]`; the zero-page pointer read wraps to `$0000` when the pointer is at `$00FF` (`CPU_addressing_modes`) |
| Relative | `BNE $FC` | PC ← PC + signed displacement |

NESdev's names are `Implicit` (written `Implied` here and in
`02-cpu-instructions.md`), `Accumulator`, `Immediate`, `Zero page`,
`Zero page indexed` (`d,x`/`d,y`), `Absolute`, `Absolute indexed` (`a,x`/`a,y`),
`Relative`, `Indirect` (`(a)`), `Indexed indirect` (`(d,x)`) and
`Indirect indexed` (`(d),y`) (`CPU_addressing_modes`).

Page-crossing cycle penalties for absolute indexed reads and taken branches are
specified in `02-cpu-instructions.md`.

## 7. Instruction timing framework

- The opcode is always fetched first (`T1`, driven out on `SYNC`); the decoder
  classifies instruction and mode entirely from the opcode byte, so the minimum
  instruction is 2 cycles (`Readme.md`).
- Three nested cycle counters produce the `T` states (`extra_counter.md`,
  `dispatch.md`, `HDL/Core6502/extra_counter.v`): `T0–T1` short instructions,
  `T2–T5` the extended counter (shift register clocked from `T1`, reset by
  `TRES2` at instruction end), `T6–T7` read-modify-write and interrupt/BRK.
- Within a half-cycle the control signals fire in a fixed order (buses, ALU,
  flags, PC, external address); the order is listed in `context_control.md`.
- Branch decisions compare one of C/V/N/Z (selected by `IR6`/`IR7`) against `IR5` (branch-if-set/clear) (`HDL/Core6502/branch_logic.v`).

## 8. Internal buses and their observable effects

The core has four precharged internal buses (`Readme.md`, `busmux.md`,
`whatwhen.md`): `ADH`/`ADL` (address halves, feeding `ABH`/`ABL` → `A0–A15`),
`SB` (side bus, register transfers and ALU operand A) and `DB` (internal data
bus, flags/`DL`/AC/PC halves and ALU operand B).

During `PHI2` **all four buses are precharged to `$FF`** ("1" is the fast
transition on NMOS); the value survives into the following `PHI1` by bus
capacitance. Constants are then formed during `PHI1` by discharging selected
bits through the `0/ADL*`, `0/ADH*` gates (`busmux.md`, `context_control.md`,
`HDL/Core6502/busmux.v`).

This is directly observable:

- **Open bus is not a hard `$FF`.** During a read the `DOR` output drivers are
  cut off (`data_bus.md`), so the external `D0–D7` traces float at the last
  value driven on them: a read that no device decodes returns the **last value
  driven** (read or write; `Open_bus_behavior`). Before `D0–D7` has ever been
  driven the observed value is `$FF` (NESdev-derived, rank 4; mechanism
  unresolved — see `## Open questions`). The internal precharge instead holds
  undriven *internal* bits at 1, e.g. bit 5 of P (§4).
- Reads performed by instructions that ignore the result (dummy reads) still
  put a real address on `A0–A15`; they are not suppressed.
- If two sources drive one bus at the same time the conflict is resolved in
  favour of the low level ("ground wins"), as NMOS transistors pull down and
  cannot pull up (`whatwhen.md`).

## 9. Interrupts, BRK and reset

The core unifies all exceptions and `BRK` into one 7-cycle "BRK sequence": an
arriving interrupt forces opcode `$00` through the `B` latch, executing `BRK`
(`interrupts.md`, `HDL/Core6502/brk.v`).

| Source | Detection | Vector | `I` on entry | B in pushed P |
|---|---|---|---|---|
| `/NMI` | falling edge; the pad latch samples `/NMI` every `PHI2`, and the `BRK6E` window (`PHI2` of cycle 6 → `PHI1` of cycle 7) fixes the vector during the sequence (`interrupts.md`; NESdev `CPU_interrupts`) | `$FFFA`/`$FFFB` | set | 0 |
| `/IRQ` | level (active low), sampled through the pad latch each cycle; `CLI`/`SEI`/`PLP` affect a pending `/IRQ` only one instruction later (`CPU_interrupts`, `02-cpu-instructions.md`) | `$FFFE`/`$FFFF` | set | 0 |
| `BRK` | opcode `$00` | `$FFFE`/`$FFFF` | set | 1 |
| `/RES` | level (active low), held on the reset flip-flop for the random logic | `$FFFC`/`$FFFD` | set | 0 |

Priority: reset wins over everything; NMI wins over IRQ/BRK (`Readme.md`,
`interrupts.md`, `brk.v`). If `/NMI` is asserted during the first four cycles of
a `BRK`, the vector chosen at cycle 5 is the NMI vector while P is still pushed
with B = 1 — interrupt hijacking (`CPU_interrupts`, `02-cpu-instructions.md` §4).

The sequence pushes `PCH`, then `PCL`, then `P` (three S decrements) and reads
the vector low byte then high byte. During `/RES` the three pushes are
**suppressed**: `DORES` forces the core's internal `WR` to 0 (read), so S
decrements three times but memory is not written and `I` ends up set
(`HDL/Core6502/dispatch.v`, `brk.v`; NESdev `CPU_interrupts`,
`CPU_power_up_state`) — that is what puts S at `$FD` rather than `$00` (§9.1).
`/RES` must be held low for several cycles (NESdev: 6 `M2` clocks, `CPU_pinout`);
the pulse is held on the `RESET FLIP/FLOP` because the `R/W` pin control needs it
(`Readme.md`, `interrupts.md`).

The vector address is built by a constant generator: `0/ADL0`, `0/ADL1`,
`0/ADL2` discharge bits 0–2 of `ADL`, while `0/ADH0`/`0/ADH17` discharge `ADH`
to `$00` (zero-page forcing, §5) — the `$FF` high byte is the undischarged
precharge, giving `$FFFA`/`$FFFC`/`$FFFE` per source (`interrupts.md`, `busmux.md`, `HDL/Core6502/busmux.v`, `brk.v`).

### 9.1 Power-on and reset register state

The Breaking NES Wiki pages describe the reset *sequence*, not the initial
register contents. NESdev's `CPU_power_up_state` documents the measured values:

| Register | At power-on | After `/RES` |
|---|---|---|
| A, X, Y | `$00` | unchanged |
| PC | `($FFFC)` | `($FFFC)` |
| S | `$00 − 3 = $FD` | S −= 3 |
| C, Z, D, V, N | 0 | unchanged |
| I | 1 | 1 |

The `$FD` value and `I = 1` follow from the reset sequence above (NESdev
`CPU_interrupts`; `HDL/Core6502/dispatch.v` confirms write suppression). Internal
RAM (`$0000–$07FF`) and cartridge RAM have an unreliable state at power-on and
are unchanged by reset, so a program MUST NOT trust their initial contents.
`A/X/Y = $00` and `C/Z/D/V/N = 0` are NESdev measurements, **unconfirmed** by
ranks 1–2 (`Open questions`).

## 10. Differences from a stock NMOS 6502

| Item | Stock NMOS 6502 | 2A03 (NTSC) | 2A07 (PAL) |
|---|---|---|---|
| BCD correction | enabled by D=1 | **disconnected**: five pieces of polysilicon are cut, so `/DAA` and `/DSA` never reach the ALU correction circuits. Boolean ADC/SBC only, even with D=1 | same as 2A03 |
| D flag | normal | still settable/clearable, still saved in P, still restored by `PLP`/`RTI` | same |
| `SO` | external pin sets V on falling edge | tied to 1, no effect | tied to 1, no effect |
| `SYNC` | opcode-fetch output | not connected (floating) | not connected |
| `RDY` | halt | driven by OAM/DMC DMA | driven by OAM/DMC DMA, and forced low when the `DBG` pin is high (`RDY2`) |

The BCD cut-out is explained by MOS patent US3991307: Ricoh removed transistors
rather than pay royalties, leaving the correction circuit in the mask. The 2A07
runs from the PAL divider ratio (§1) and exposes a debug WAIT through pin 30
(`DBG`, NESdev `TST`) instead of the 2A03 test registers (`APU/core.md`,
`Ricoh.md`, `Docs/Famiclones/ChipFeatures.md`). NESdev adds that the RP2A03E
also stops on pin 30 high (rank 4, unstated by the schematics; `CPU_pinout`).

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | `SED` has no effect on `ADC`/`SBC`; `D=1` still changes the value pushed by `PHP` | `/DAA` and `/DSA` are cut from the ALU correction cells in the Ricoh mask | Do not implement decimal arithmetic on the 2A03/2A07. Keep D as a normal stored flag. Add a per-chip switch for famiclone CPUs that re-enable BCD. |
| 2 | Reading an address no device decodes returns the last value left on `D0–D7` (open bus) | During a read the core's `DOR` outputs are cut off and the external bus floats at its last driven value (`data_bus.md`, `Open_bus_behavior`) | Latch the last value driven on the data bus and return it; do not hard-code `$FF`. The value is `$FF` only before the bus has been driven since power-on. DMC DMA can change it (`02-cpu-dma.md`). |
| 3 | Extra "dummy" reads appear on the bus during indexed and RMW instructions | The address is set in `PHI1` and the bus is read in `PHI2` even when the result is discarded | Reproduce the dummy read addresses (they matter for mapper and PPU register side effects). |
| 4 | `RDY` low during a write cycle does not stop the CPU | The DMA controller only asserts `RDY` after the core enters a read cycle (PHI1=0, R/W=1) | Implement `RDY` halting only on read cycles; see `02-cpu-dma.md`. |
| 5 | `BRK` and `/IRQ` push different bit 4 values but use the same vector | The unified BRK sequencer drives the B latch only for the BRK source | Push P with B=1 for `BRK`, B=0 for `/IRQ` and `/NMI`; both use `$FFFE`. |
| 6 | `SO` cannot set V; `CLV` still works | The 2A03/2A07 ties the `SO` pin to 1 and the detector is falling-edge | Ignore the SO input; only `CLV` and the ALU clear/set V. |
| 7 | `SYNC` is dead on the 2A03/2A07 | The core's `SYNC` output is not bonded/connected inside the APU | Do not rely on `SYNC`; use the address/M2 timing instead. |
| 8 | Bit 5 of P is not driven by any flag | No flag is connected to `DB5`; the `DB` precharge holds it at 1 | Treat bit 5 as always pushed as 1; `PLP`/`RTI` ignore bits 4 and 5 when pulling. |
| 9 | `/NMI` triggers once per falling edge even if it stays low | The NMI path is an edge detector (two RS latches) clocked around `BRK6E` | Latch NMI on the falling edge; a held-low line must not re-trigger. |
| 10 | Reset takes several cycles, leaves S three lower and writes nothing to the stack | `/RES` is latched on a reset flip-flop and `DORES` forces `WR` to read; the BRK sequence still walks S | Model the reset sequence explicitly (suppressed pushes, vector fetch from `$FFFC`, S = `$FD`, `I` = 1), not as a single-cycle jump. |
| 11 | The pushed stack address and value come from the *old* S while S is already updating | The S register has separate input and output latches that operate in the same `PHI1` | Track S as "old value on the bus, new value latched" so push/pull addresses match hardware. |

## Programming / implementation notes

- MUST implement `PHI1` and `PHI2` as two non-overlapping half-cycles: address
  and `R/W` in `PHI1`, data transfer in `PHI2`.
- MUST implement S as an 8-bit pointer into `$0100–$01FF` with the push
  (decrement) and pull (increment) order used by the hardware.
- MUST force the high byte of every zero-page address to `$00`, including the
  `(Indirect,X)` base (`$0034 + X` wraps inside `$00xx`) and the zero-page
  pointer read for `(Indirect),Y`.
- MUST disable BCD correction for RP2A03/RP2A07, while keeping D as a
  storable/restorable flag. SHOULD make BCD a per-chip option for clones.
- MUST model open bus on `D0–D7` as a latch of the last value driven on the data
  bus (initialised to `$FF` per NESdev's power-on observation), not a constant `$FF`.
- MUST push P with bit 4 (the B flag) = 1 for `BRK`/`PHP` and 0 for
  `/IRQ`/`/NMI`, and MUST push bit 5 = 1.
- MUST suppress the three stack writes during `/RES` while still decrementing S
  three times (S = `$FD` after reset), and MUST set `I` on reset.
- MUST use `$FFFA` (NMI), `$FFFC` (reset) and `$FFFE` (IRQ/BRK) as the vectors.
- MUST treat `/NMI` as edge-sensitive and `/IRQ`/`/RES` as level-sensitive.
- MUST apply `RDY` halting only in read cycles (`02-cpu-dma.md`).
- MUST NOT assume any particular RAM contents at power-on; SHOULD use a
  deterministic or random initial pattern and MUST initialise the CPU registers
  (§9.1).
- SHOULD NOT expose `SYNC`/`SO` in an emulator (dead on both parts). MAY model
  the address bus as driven from `ABL`/`ABH`, latched in `PHI1`, for
  cycle-accurate bus traces.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| 2A03/2A07 pinout, `M2`, `/RST`, `TST`, `RDY`, `/NMI`/`/IRQ`, φ1/φ2 half-cycles | `CPU_pinout` |
| Clock divider and CPU:PPU ratios (NTSC /12, PAL /16, Dendy /15) | `Cycle_reference_chart` |
| Architectural registers A/X/Y/S/P/PC | `CPU_registers` |
| Status flags, bit layout, B flag, D flag, SO not exposed | `Status_flags` |
| Address map and mirrors (`$0000–$FFFF`) | `CPU_memory_map` |
| Addressing modes with operand examples | `CPU_addressing_modes` |
| T-state cycle framework and per-cycle bus behaviour | `Visual6502wiki/6502_Timing_States` |
| CPU open bus vs internal precharge (capacitive data bus) | `Open_bus_behavior` |
| Interrupts, vectors, BRK sequence, hijacking, reset, B bit | `CPU_interrupts`, `Visual6502wiki/6502_BRK_and_B_bit` |
| Power-on and post-reset register values | `CPU_power_up_state` |
| 2A03/2A07 revisions, pin-30 `/RDY`, BCD cut-out, clones (§10) | `CPU_variants` |
| Die-level visual reference for the 2A03 core | `Visual_circuit_tutorial` |

Page names were resolved against the local NESdev Wiki cache / Wayback Machine.
`Status_flags` is the canonical title (there is no `Status_register` page). No
`Visual_2A03` page is known, so the table uses `Visual_circuit_tutorial` (see
`## Open questions`).

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Core integration, APU pin map, debug WAIT on 2A07 | `APU/core.md`, `APU/pads.md`, `Ricoh.md` | `core_top.v`, `pads.v` |
| `PHI1`/`PHI2` half-cycle semantics, dead time | `6502/Readme.md`, `6502/clock.md`, `6502/whatwhen.md` | `clock.v` |
| Registers A/X/Y/S/P/PC, S dual latch | `6502/Readme.md`, `6502/regs.md`, `6502/pc.md`, `6502/regs_control.md` | `regs.v`, `pc.v`, `pc_control.v` |
| Status register bit map and flag controls | `6502/flags.md`, `6502/flags_control.md`, `6502/ir.md` | `flags.v`, `flags_control.v` |
| Address map, zero-page forcing, stack page | `6502/Readme.md`, `6502/regs.md` | `regs.v`, `pc.v` |
| Address modes and wraparound rules | `6502/Readme.md` | `bus_control.v`, `addr_bus.v` |
| Cycle/state counters `T0–T7` | `6502/extra_counter.md`, `6502/dispatch.md` | `extra_counter.v` |
| Internal buses, precharge to `$FF`, ground-wins conflicts | `6502/Readme.md`, `6502/busmux.md`, `6502/regs.md`, `6502/whatwhen.md` | `busmux.v`, `data_bus.v`, `bus_control.v` |
| CPU open bus on `D0–D7` (capacitive bus, not a forced `$FF`) | `6502/data_bus.md`, `6502/whatwhen.md` | `data_bus.v`, `pads.v` (model cross-checked against NESdev `Open_bus_behavior`) |
| Interrupts, BRK sequence, vectors, reset flip-flop, reset write suppression | `6502/interrupts.md`, `6502/pads.md`, `6502/Readme.md` | `brk.v`, `pads.v`, `dispatch.v` (`DORES` gates `WR`) |
| BCD cut-out, 2A07 vs 2A03 | `APU/core.md`, `Ricoh.md`, `Docs/Famiclones/ChipFeatures.md` | `alu_control.v` (the `/DAA`/`/DSA` outputs that are cut) |
| Control-signal execution order within a half-cycle | `6502/context_control.md` | `bus_control.v`, `alu_control.v`, `pc_control.v` |
| Power-on register values, RAM unreliability (NESdev-derived); post-reset `S`/`I` from the reset sequence | `6502/interrupts.md`, `6502/Readme.md` | `dispatch.v`, `brk.v` (`DORES`) |

## Open questions

- **Power-on register values.** The Breaking NES Wiki describes the reset
  *sequence*, not the initial contents of A/X/Y/S/P. Post-reset `S`/`I` follow
  from that sequence; the `A/X/Y = $00` and `C/Z/D/V/N = 0` values are NESdev
  `CPU_power_up_state` measurements, **unconfirmed** by ranks 1–2.
- **HDL defect in the PC load path.** `HDL/Core6502/Readme.md` records that the
  core never loads a memory-sourced low byte into `PCL` (reset PC becomes `04FF`
  instead of the vector), traced to the `DL→ADL→PCL` timing in
  `bus_control.v`/`pc_control.v`; the spec follows `pc.md`, and the HDL needs
  the fix.
- **`RDY` implementation.** `dma.md` says `RDY` is only effective on read
  cycles. `pads.v` passes `RDY` through combinationally (`assign RDY_frompad =
  RDY;`), but the read/write dependence does exist in `dispatch.v` `ReadyRW`
  (`ready_latch2 (.d(WR), .en(PHI1))`). Which is authoritative for the spec?
- **Open-bus `$FF` mechanism.** `busmux.v:52-55` precharges only the internal
  `SB`/`DB`/`ADL`/`ADH`, and `data_bus.v:38` (`bufif0 (DB_Ext, dor_latch_nq,
  RD)`) is the only external driver, disabled while `RD=1` — so the HDL gives no
  path for the precharge to set the external `$FF`; that value is a rank-4
  NESdev observation (`Open_bus_behavior`). The override question is open too.
- **`Visual_2A03` page name.** No NESdev wiki page or Wayback capture is known
  (the CDX index lists only `Visual_2C02` and `Visual_circuit_tutorial`).
  `02-cpu-instructions.md` has since been corrected to `Visual_circuit_tutorial`,
  so all three CPU files now agree on that page.
- **Undocumented opcodes.** The Wiki describes the decoder and random logic but
  this section does not enumerate illegal opcodes; see `02-cpu-instructions.md`.
