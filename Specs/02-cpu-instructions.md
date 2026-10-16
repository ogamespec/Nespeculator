# 02-cpu-instructions — instruction set, addressing modes and cycle costs

**Status:** consensus (round 2)
**Scope:** The official 6502 instruction set as executed by the 2A03/2A07 core:
the addressing modes; every official opcode with mnemonic, mode, length and base
cycle count; the extra-cycle rules; the per-cycle bus sequences hardware can
observe; and the flag effects. Undocumented opcodes are summarised in §6. The
registers, fetch/decode hardware and interrupt vectors are in
`Specs/02-cpu-core.md`; DMA stalls are in `Specs/02-cpu-dma.md`.
**Primary sources:** `BreakingNESWiki/6502/{decoder,predecode,random_logic,dispatch,alu,alu_control,flags_control,branch_logic,extra_counter,context_control,whatwhen}.md`,
`Docs/6502/6502 Instruction Set.htm`, `Docs/6502/6502 - Integration.pdf`,
`MOS6502_Opcodes/{Readme,A9,A2,8D,20,EE,DF,AB,brk}.md`;
`HDL/Core6502/{decoder,dispatch,brk,branch_logic,extra_counter,pc_control,alu,alu_control,flags,flags_control,bus_control,core_top}.v`.

## 1. Addressing modes

The opcode byte alone determines the mnemonic and the addressing mode. There are
**56 official mnemonics** and **151 official opcodes**; the other 105 values are
undocumented (§6). The shortest official instruction is 2 cycles, the longest 7,
and cycle 1 is always the opcode fetch (`Readme.md`, `decoder.md`). Cycle counts
are CPU cycles (`PHI0` periods), identical for **NTSC** (RP2A03), **PAL** (RP2A07)
and **Dendy**; only wall-clock duration differs (`Specs/02-cpu-core.md` §1).

| Mode (NESdev) | Assembler example | Operand / effective address | Bytes | Base cycles | Extra cycles |
|---|---|---|---|---|---|
| Implied | `CLC` | — | 1 | 2 | — |
| Accumulator | `ASL A` | A | 1 | 2 | — |
| Immediate | `LDA #$1C` | the byte `$1C` follows the opcode | 2 | 2 | — |
| Zero page | `LDA $56` | `[$0056]`, high byte forced to `$00` | 2 | 3 | — |
| Zero page,X | `LDA $33,X` | `[$0033 + X]`, wraps inside `$00xx` | 2 | 4 | — |
| Zero page,Y | `LDX $33,Y` | `[$0033 + Y]`, wraps inside `$00xx` | 2 | 4 | — |
| Absolute | `LDA $1234` | `[$1234]` | 3 | 4 | — |
| Absolute,X | `LDA $1234,X` | `[$1234 + X]` | 3 | 4 | +1 if the index crosses a page (reads) |
| Absolute,Y | `LDA $1234,Y` | `[$1234 + Y]` | 3 | 4 | +1 if the index crosses a page (reads) |
| Indirect | `JMP ($1234)` | PC ← `[$1234]` | 3 | 5 | — |
| (Indirect,X) | `LDA ($34,X)` | `[[$0034 + X]]`; the `+X` wraps inside `$00xx` | 2 | 6 | — |
| (Indirect),Y | `LDA ($2A),Y` | `[[$002A] + Y]`; for a pointer at `$FF` the pointer high byte comes from `$0000` | 2 | 5 | +1 if the index crosses a page (reads) |
| Relative | `BNE $FC` | PC ← PC + signed 8-bit displacement | 2 | 2 | +1 if taken; +1 more if taken across a page |

NESdev naming: the NESdev Wiki gives the abbreviation, the name and the operand
formula for each mode (`CPU_addressing_modes`): Implicit (NESdev's spelling;
written `Implied` here and in `02-cpu-core.md`, no operand), Accumulator (`A`),
Immediate (`#v`), Zero page (`d`), Zero page
indexed (`d,x` and `d,y`), Absolute (`a`), Absolute indexed (`a,x` and `a,y`),
Relative (`label`), Indirect (`(a)`, `JMP` only), Indexed indirect (`(d,x)`) and
Indirect indexed (`(d),y`). The mode column above keeps this repository's
spellings; the opcode matrix in §2 uses the NESdev abbreviations.

Overrides: stores (`STA`/`STX`/`STY`) never take the page-cross cycle
(`STA $1234,X` = 5, `STA ($2A),Y` = 6, `STY $12,X` = 4), and read-modify-write
has its own costs (accumulator 2, zp 5, zp,X 6, abs 6, abs,X 7). `BIT` is zero
page (3 cycles, 2 bytes) or absolute (4 cycles, 3 bytes).

## 2. Opcode matrix (all 151 official opcodes)

Each cell is `` `$opcode` cycles ``; empty means no official opcode. `*` marks
the modes that take NESdev's "`+`" (the page-crossing "oops") cycle on reads:
`a,x`, `a,y`, `(d),y` — so `4*` is 4 base cycles, 5 when the index crosses a
page. `†` marks `rel`: +1 cycle if the branch is taken, +1 more
if it is taken across a page, so a branch costs 2 (not taken), 3 or 4. The header
row is the NESdev mode abbreviation, the row below it the repository's spelling;
both are defined in §1, as are the byte lengths.

| Inst | imp | acc | #v | d | d,x | d,y | a | a,x | a,y | (d,x) | (d),y | (a) | rel |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| | | | Imm | ZP | ZP,X | ZP,Y | Abs | Abs,X | Abs,Y | (Ind,X) | (Ind),Y | Ind | Rel |
| LDA |  |  | `$A9` 2 | `$A5` 3 | `$B5` 4 |  | `$AD` 4 | `$BD` 4* | `$B9` 4* | `$A1` 6 | `$B1` 5* |  |  |
| LDX |  |  | `$A2` 2 | `$A6` 3 |  | `$B6` 4 | `$AE` 4 |  | `$BE` 4* |  |  |  |  |
| LDY |  |  | `$A0` 2 | `$A4` 3 | `$B4` 4 |  | `$AC` 4 | `$BC` 4* |  |  |  |  |  |
| STA |  |  |  | `$85` 3 | `$95` 4 |  | `$8D` 4 | `$9D` 5 | `$99` 5 | `$81` 6 | `$91` 6 |  |  |
| STX |  |  |  | `$86` 3 |  | `$96` 4 | `$8E` 4 |  |  |  |  |  |  |
| STY |  |  |  | `$84` 3 | `$94` 4 |  | `$8C` 4 |  |  |  |  |  |  |
| TAX | `$AA` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| TAY | `$A8` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| TXA | `$8A` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| TYA | `$98` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| TSX | `$BA` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| TXS | `$9A` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| PHA | `$48` 3 |  |  |  |  |  |  |  |  |  |  |  |  |
| PHP | `$08` 3 |  |  |  |  |  |  |  |  |  |  |  |  |
| PLA | `$68` 4 |  |  |  |  |  |  |  |  |  |  |  |  |
| PLP | `$28` 4 |  |  |  |  |  |  |  |  |  |  |  |  |
| AND |  |  | `$29` 2 | `$25` 3 | `$35` 4 |  | `$2D` 4 | `$3D` 4* | `$39` 4* | `$21` 6 | `$31` 5* |  |  |
| EOR |  |  | `$49` 2 | `$45` 3 | `$55` 4 |  | `$4D` 4 | `$5D` 4* | `$59` 4* | `$41` 6 | `$51` 5* |  |  |
| ORA |  |  | `$09` 2 | `$05` 3 | `$15` 4 |  | `$0D` 4 | `$1D` 4* | `$19` 4* | `$01` 6 | `$11` 5* |  |  |
| BIT |  |  |  | `$24` 3 |  |  | `$2C` 4 |  |  |  |  |  |  |
| ADC |  |  | `$69` 2 | `$65` 3 | `$75` 4 |  | `$6D` 4 | `$7D` 4* | `$79` 4* | `$61` 6 | `$71` 5* |  |  |
| SBC |  |  | `$E9` 2 | `$E5` 3 | `$F5` 4 |  | `$ED` 4 | `$FD` 4* | `$F9` 4* | `$E1` 6 | `$F1` 5* |  |  |
| CMP |  |  | `$C9` 2 | `$C5` 3 | `$D5` 4 |  | `$CD` 4 | `$DD` 4* | `$D9` 4* | `$C1` 6 | `$D1` 5* |  |  |
| CPX |  |  | `$E0` 2 | `$E4` 3 |  |  | `$EC` 4 |  |  |  |  |  |  |
| CPY |  |  | `$C0` 2 | `$C4` 3 |  |  | `$CC` 4 |  |  |  |  |  |  |
| INC |  |  |  | `$E6` 5 | `$F6` 6 |  | `$EE` 6 | `$FE` 7 |  |  |  |  |  |
| INX | `$E8` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| INY | `$C8` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| DEC |  |  |  | `$C6` 5 | `$D6` 6 |  | `$CE` 6 | `$DE` 7 |  |  |  |  |  |
| DEX | `$CA` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| DEY | `$88` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| ASL |  | `$0A` 2 |  | `$06` 5 | `$16` 6 |  | `$0E` 6 | `$1E` 7 |  |  |  |  |  |
| LSR |  | `$4A` 2 |  | `$46` 5 | `$56` 6 |  | `$4E` 6 | `$5E` 7 |  |  |  |  |  |
| ROL |  | `$2A` 2 |  | `$26` 5 | `$36` 6 |  | `$2E` 6 | `$3E` 7 |  |  |  |  |  |
| ROR |  | `$6A` 2 |  | `$66` 5 | `$76` 6 |  | `$6E` 6 | `$7E` 7 |  |  |  |  |  |
| JMP |  |  |  |  |  |  | `$4C` 3 |  |  |  |  | `$6C` 5 |  |
| JSR |  |  |  |  |  |  | `$20` 6 |  |  |  |  |  |  |
| RTS | `$60` 6 |  |  |  |  |  |  |  |  |  |  |  |  |
| RTI | `$40` 6 |  |  |  |  |  |  |  |  |  |  |  |  |
| BRK | `$00` 7 |  |  |  |  |  |  |  |  |  |  |  |  |
| BCC |  |  |  |  |  |  |  |  |  |  |  |  | `$90` 2† |
| BCS |  |  |  |  |  |  |  |  |  |  |  |  | `$B0` 2† |
| BEQ |  |  |  |  |  |  |  |  |  |  |  |  | `$F0` 2† |
| BMI |  |  |  |  |  |  |  |  |  |  |  |  | `$30` 2† |
| BNE |  |  |  |  |  |  |  |  |  |  |  |  | `$D0` 2† |
| BPL |  |  |  |  |  |  |  |  |  |  |  |  | `$10` 2† |
| BVC |  |  |  |  |  |  |  |  |  |  |  |  | `$50` 2† |
| BVS |  |  |  |  |  |  |  |  |  |  |  |  | `$70` 2† |
| CLC | `$18` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| CLD | `$D8` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| CLI | `$58` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| CLV | `$B8` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| SEC | `$38` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| SED | `$F8` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| SEI | `$78` 2 |  |  |  |  |  |  |  |  |  |  |  |  |
| NOP | `$EA` 2 |  |  |  |  |  |  |  |  |  |  |  |  |

## 3. Extra-cycle rules

- **Page cross (reads).** For absolute indexed (`a,x`, `a,y`) and indirect
  indexed (`(d),y`) *reads*, the index is added to the low byte first; if that
  addition carries out of bit 7, one extra cycle is taken. NESdev calls this
  extra read the "oops" cycle (`CPU_addressing_modes`). The page-cross term is
  the `PGX` ("Page X") signal, formed from decoder outputs `X71` ("OP abs,XY")
  and `X72` ("OP ind,Y"), and the carry is propagated into the high byte by
  `INC_SB` ("Increment SB") (`decoder.md`, `alu_control.md`, `bus_control.v`).
- **No penalty** for absolute indexed / indirect indexed *stores* (`STA`/`STX`/
  `STY`, fixed 5/5/6), for absolute indexed read-modify-write (fixed 7), or for
  any zero-page mode: `d,x`, `d,y` and `(d,x)` wrap inside `$00xx`. NESdev states
  the store case as "add a cycle for write instructions" — the store always
  performs the un-carried read, but the cycle is already counted in the base cost
  (`CPU_addressing_modes`).
- **Branches**: not taken 2 cycles; taken on the same page 3; taken across a page
  4 (`Docs/6502/6502 Instruction Set.htm`, footnote `**`).
- **The extra cycle is a real bus cycle**, not a wait state; its result is
  discarded (or corrected by the following read) and it is visible to mappers and
  I/O registers.

## 4. Execution frames (bus sequences)

One cycle = `PHI1` (address and `R/W` set) + `PHI2` (data transferred); see
`Specs/02-cpu-core.md` §2. The stack pointer is S (NESdev `S`; the repository's
`regs.md` and `Readme.md` also write `S`), so the stack address is `$0100 + S`.
The byte read in an instruction's last cycle is the opcode of the *next*
instruction (`MOS6502_Opcodes/Readme.md`), so the frames count from the opcode
fetch.

**Read with index (`LDA $1234,X`: 4 cycles, 5 on a page cross)**

| Cycle | R/W | Address | Notes |
|---|---|---|---|
| 1 | R | PC | opcode; PC++ |
| 2 | R | PC | address low byte; PC++ |
| 3 | R | PC | address high byte; PC++ |
| 4 | R | base + X without the carry | data (dummy if the carry is set) |
| 5 | R | base + X with the carry | real data read; only on a page cross |

**Read-modify-write (`INC $1234`: 6 cycles, `INC $1234,X`: 7)**

| Cycle | R/W | Address | Notes |
|---|---|---|---|
| 1–3 | R | PC | opcode, address low, address high |
| 4 | R | effective address | read the original value into `DL`/`ADD` |
| 5 | W | effective address | **write the original (unmodified) value back** |
| 6 | W | effective address | write the modified value |

**Store (`STA $1234,X`: 5 cycles)** is the read frame with cycle 4 changed to an
always-performed dummy read at `base + X without the carry` and cycle 5 a write
of A at `base + X with the carry`.

**Branch (2 / 3 / 4 cycles)**: opcode (PC++), then the signed displacement
(PC++); if taken, one fetch-and-discard cycle at the uncorrected PC while the
displacement is added; if taken across a page, one more read at the corrected PC
before the real fetch.

**Stack, returns, interrupt entry.** `PHA`/`PHP` (3): opcode, dummy read of the
next byte, write `$0100 + S`, S−−. `PLA`/`PLP` (4): opcode, dummy read, read
`$0100 + S` (dummy, S++), read `$0100 + S` (value, S++). `JSR` (6): address low,
dummy read, write `PCH`, write `PCL`, address high. `RTS` (6): pull `PCL`, pull
`PCH`, one more read at PC with PC++. `RTI` (6): pull `P`, `PCL`, `PCH` (PC
unchanged).

The 7-cycle interrupt frame is common to `BRK`, `/IRQ`, `/NMI` and `/RES`; only
the first two cycles and bit 4 of the pushed P differ:

| Cycle | `BRK` `$00` | `/IRQ`, `/NMI` |
|---|---|---|
| 1 | PC: fetch opcode `$00`; PC++ | PC: fetch opcode, discarded; `$00` forced into the opcode register; PC **not** incremented |
| 2 | PC: read the next byte and discard it; PC++ | PC: same fetch again, discarded |
| 3 | `$0100 + S`, W: push `PCH`, S−− | same |
| 4 | `$0100 + S`, W: push `PCL`, S−− | same |
| 5 | `$0100 + S`, W: push P (B flag = 1), S−− | `$0100 + S`, W: push P (B flag = 0), S−− |
| 6 | read vector low (`$FFFE` / `$FFFA`), set I | same |
| 7 | read vector high (`$FFFF` / `$FFFB`) | same |

Vectors: `/NMI` `$FFFA`/`$FFFB`, `/RES` `$FFFC`/`$FFFD`, `BRK`/`/IRQ`
`$FFFE`/`$FFFF` (`CPU_interrupts`, `MOS6502_Opcodes/20.md`, `brk.md`,
`Docs/6502/6502 - Integration.pdf`). `/RES` runs the same sequence with the three
stack writes suppressed (three dummy S decrements) and therefore always ends with
I = 1 (`CPU_interrupts`).

## 5. Flag effects

`+` = modified, `–` = unchanged. N/Z come from the 8-bit value placed on the
internal `DB` bus, C from `ACR`, V from `AVR` (`flags.md`, `flags_control.md`).

| Instruction(s) | N | Z | C | V | Rule |
|---|---|---|---|---|---|
| `ADC` | + | + | + | + | A = A + M + C; C = carry out; V = signed overflow |
| `SBC` | + | + | + | + | A = A − M − (1 − C); C = no borrow. **Binary only**: D is ignored for arithmetic on 2A03/2A07 |
| `CMP`/`CPX`/`CPY` | + | + | + | – | reg − M; C = 1 when reg ≥ M (no borrow); N/Z from the difference |
| `BIT` (zero page/absolute) | + | + | – | + | N = M7, V = M6, Z = 1 when (A & M) = 0 |
| `ASL`/`LSR`/`ROL`/`ROR` | + | + | + | – | C = the bit shifted out; N/Z from the result (so `LSR` always clears N) |
| `INC`/`INX`/`INY`/`DEC`/`DEX`/`DEY` | + | + | – | – | N/Z from the result |
| `AND`/`ORA`/`EOR`/`LDA`/`LDX`/`LDY`/`TAX`/`TAY`/`TSX`/`TXA`/`TYA` | + | + | – | – | N/Z from the loaded value |
| `TXS` | – | – | – | – | transfers X to S over `SB`; the value never reaches `DB`, so N/Z are **not** updated |
| `PLP`/`RTI` | + | + | + | + | all flags replaced from the stack byte (bit 4 has no stored flag; bit 5 reads as 1) |
| `PHP` | – | – | – | – | pushes P with bit 4 (B) = 1 and bit 5 = 1 |
| `BRK` | – | – | – | – | pushes `PCH`, `PCL`, P (bit 4 (B) = 1); `I` is set by the sequence |
| `CLC`/`SEC`/`CLD`/`SED`/`CLI`/`SEI`/`CLV` | – | – | C/D/I/V | – | writes exactly one flag |
| Branches, `STA`/`STX`/`STY`, `JMP`, `JSR`, `RTS`, `NOP` | – | – | – | – | no flags affected |

NESdev names bit 4 of the pushed P the **B flag** (`Status_flags`): it is not a
register in the core, only the transient signal that distinguishes an instruction
entry (`BRK`, `PHP` — pushed as 1) from a hardware interrupt (`/NMI`, `/IRQ` —
pushed as 0). `PLP` and `RTI` ignore bits 4 and 5 when pulling, and bit 5 always
reads (and is pushed) as 1.

The 2A03/2A07 cut the BCD correction path (`/DAA`, `/DSA`), so `SED` changes only
the stored D flag and `ADC`/`SBC` stay binary even with D = 1 (`APU/core.md`).

## 6. Undocumented opcodes

The decode PLA covers all 256 opcode values; an unofficial opcode is not trapped
— it is decoded by whatever `X` lines its bit pattern matches and runs through
the same `T`-cycle machinery (`decoder.md`). Two undocumented opcodes are
documented in this repository and are representative:

| Opcode | Repo name | Size / cycles | Behaviour |
|---|---|---|---|
| `$AB` | `ATX` (NESdev `LAX #`) | 2 / 2 | `SB` and `DB` are driven simultaneously (A on one, the immediate on the other) and joined by `SB/DB`; the "ground wins" conflict rule makes the shared bus `A & M`, loaded into X **and** back into A, with N/Z set from it (`MOS6502_Opcodes/AB.md`) |
| `$DF` | `DCP` absolute indexed (`a,x`) | 3 / 7 | RMW frame; the two writes store the original value and then the value − 1, after which T1 compares A with the original value (`ADD = AC + ~DL + 1`), setting C/Z/N (`MOS6502_Opcodes/DF.md`) |

The full undocumented set (aliases, `NOP` variants, unstable behaviour, DMA
interaction) is **not enumerated by these sources** and is out of scope.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | An `INC`/`DEC`/`ASL`/`LSR`/`ROL`/`ROR` at an I/O address causes the write side effect **twice** | The RMW frame performs two consecutive write cycles: the original value, then the modified value (`MOS6502_Opcodes/EE.md`) | Model both writes; never treat an RMW as read + one write. Avoid RMW on `$2000–$2007` and `$4014` in demos |
| 2 | Extra reads appear on the bus that the program did not ask for | Address is set in `PHI1`, data read in `PHI2`; indexed stores always do a dummy read, indexed reads do one when the index crosses a page | Reproduce dummy reads; mapper and `$2007`/`$4016`/`$4017` side effects depend on them |
| 3 | An indexed read takes one cycle longer when the index crosses a page | The carry out of the low-byte addition gates the `PGX` term, adding the correcting read | Implement the +1; for cycle-accurate traces also emit the extra read |
| 4 | A taken branch costs 1 cycle more, 2 more across a page | The displacement is added after the fetch-and-discard cycle, and the fetch is redone when `PCH` changes | Cost branches 2/3/4; charge nothing for a not-taken branch |
| 5 | The instruction after a DMA stall behaves differently when it is a branch | `/PRDY` ("Previous Ready") is wired into the decoder's Branch T0 line: if the CPU was not ready at the end of the previous instruction, a following branch has its `T0` cycle skipped (`decoder.md`) | Account for the skipped cycle after `RDY` stalls; the purpose is undocumented |
| 6 | `BRK` returns to the address after the opcode plus one, not to the address after the opcode | The `BRK` frame reads and discards the byte after the opcode and increments PC, so the pushed return address is PC+2 (`CPU_interrupts`) | Push PC+2 for `BRK`; leave a padding byte after it. `/IRQ` and `/NMI` suppress PC increment during their two fetch cycles, so they return to the interrupted instruction |
| 7 | `BIT` sets N and V from the *memory* byte but Z from `A & M` | N/V are latched from `DB7`/`DB6`; Z comes from the `ZTST` term of the AND result | Implement the split source; `BIT #` does not exist officially |
| 8 | `LSR` clears N even though it is a "shift", and `TXS` changes no flags | Flags follow the value placed on `DB`; `LSR`'s result has bit 7 = 0, `TXS` never drives `DB` | Derive N/Z from the loaded/result byte, not from the mnemonic class |
| 9 | Any undocumented opcode runs instead of hanging the CPU | The PLA decodes all 256 values; there is no illegal-opcode trap (`decoder.md`) | Do not raise an "invalid opcode" fault on the 2A03/2A07 |
| 10 | A `BRK` can jump to the `/NMI` vector (interrupt hijacking) | If `/NMI` is asserted during the first four cycles of the `BRK` frame, the vector selected at cycle 5 changes; P is still pushed with B = 1 (`CPU_interrupts`) | Use the vector status sampled at cycle 5; keep the pushed B bit from the instruction, not from the interrupt that wins |
| 11 | `CLI`/`SEI`/`PLP` do not affect a pending `/IRQ` until one instruction later, while `RTI` does | `CLI`/`SEI`/`PLP` change I after the interrupt poll of their first cycle; `RTI` restores I before polling (`CPU_interrupts`) | Delay the effect of `CLI`/`SEI`/`PLP` on IRQ masking by one instruction; apply `RTI`'s immediately. NMI is edge-sensitive and unaffected |

## Programming / implementation notes

- MUST decode instruction and addressing mode from the opcode byte alone; there
  are 151 official opcodes.
- MUST charge the base cycles of §2 plus: +1 for an indexed read page cross,
  +1/+1 for a taken branch (same/other page), and nothing for stores or RMW.
- MUST perform the RMW double write (original, then modified) at the effective
  address, and the dummy read of an indexed store and of the
  `JSR`/`PHA`/`PHP`/`PLA`/`PLP`/`RTS`/`RTI` frames, because it is visible on the
  bus.
- MUST execute `BRK`/`/IRQ`/`/NMI` through the 7-cycle interrupt frame, pushing
  `PCH`, `PCL`, P in that order, and MUST set `I` on entry.
- MUST push P with bit 4 (the B flag) = 1 for `BRK`/`PHP` and 0 for `/IRQ`/`/NMI`,
  and bit 5 = 1 (`Specs/02-cpu-core.md` §4).
- MUST increment PC twice during `BRK`'s first two cycles (so `BRK` returns to
  PC+2) and MUST NOT increment it during `/IRQ` or `/NMI` (`CPU_interrupts`).
- MUST delay the effect of `CLI`/`SEI`/`PLP` on a pending `/IRQ` by one
  instruction, while applying `RTI`'s restored `I` immediately
  (`CPU_interrupts`).
- MUST treat `ADC`/`SBC` as binary on RP2A03/RP2A07 even with D = 1.
- MUST implement `BIT`'s split flag sources (N/V from M, Z from A & M).
- SHOULD select the interrupt vector from the status sampled at cycle 5 of the
  frame, so that `/NMI` arriving during `BRK` hijacks the vector while the pushed
  B bit still reflects `BRK` (`CPU_interrupts`).
- SHOULD implement `TXS` as flag-neutral, `LSR`/`ROR`/`ROL`/`ASL` as setting N
  from the result byte, and undocumented opcodes as executing their decoded
  behaviour rather than trapping (`$AB` and `$DF` above are a starting point).
- MAY ignore the `/PRDY` Branch-T0 skip unless cycle-exact behaviour after a DMA
  stall is required.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| Official opcode table, mnemonics, lengths, cycle counts | `CPU_instructions` |
| Addressing modes: names, operand syntax and byte/cycle length | `CPU_addressing_modes` |
| A/X/Y/S/P registers and the architectural register set | `CPU_registers` |
| Status flag meanings, the B flag, bit 4/5 behaviour | `Status_flags` |
| `/IRQ`/`/NMI` frames, vectors, interrupt hijacking, delayed IRQ response | `CPU_interrupts` |
| Undocumented/illegal opcodes | `CPU_unofficial_opcodes` |
| Per-cycle bus behaviour on the real die | `Visual_circuit_tutorial` |

## Provenance

| Claim area | BreakingNES Wiki / Docs | HDL |
|---|---|---|
| Opcode↔mnemonic/mode mapping, instruction classes, all-256 decode | `6502/decoder.md`, `6502/predecode.md`, `6502/random_logic.md` | `decoder.v`, `predecode.v` |
| Addressing modes, operand wraparound, zero-page forcing | `6502/Readme.md` | `bus_control.v`, `addr_bus.v` |
| Base cycle costs, page-cross/branch penalties, `T0–T7` framework | `Docs/6502/6502 Instruction Set.htm`, `MOS6502_Opcodes/Readme.md`, `6502/extra_counter.md`, `6502/dispatch.md` | `extra_counter.v`, `dispatch.v` |
| Per-cycle frames: read, RMW, store, JSR, stack, BRK | `MOS6502_Opcodes/{A9,A2,8D,20,EE,DF,brk}.md` | `alu.v`, `bus_control.v`, `core_top.v` |
| Branch direction/taken logic, `/PRDY` Branch-T0 skip | `6502/branch_logic.md`, `6502/decoder.md` | `branch_logic.v`, `decoder.v` |
| Flag effects, N/Z/C/V paths, shifts and compares | `6502/flags.md`, `6502/flags_control.md`, `6502/alu.md`, `6502/alu_control.md` | `flags.v`, `flags_control.v`, `alu.v`, `alu_control.v` |
| Interrupt/BRK entry, vectors `$FFFA/$FFFC/$FFFE` | `6502/interrupts.md`, `MOS6502_Opcodes/brk.md`, `Docs/6502/6502 - Integration.pdf` | `brk.v`, `core_top.v` |
| Interrupt hijacking, delayed `/IRQ` response after `CLI`/`SEI`/`PLP`, B flag (NESdev-derived) | — (NESdev `CPU_interrupts`, `Status_flags`; not covered by the ranges 1–3 sources) | — |
| Undocumented `$AB` (ATX/LAX) and `$DF` (DCP) | `MOS6502_Opcodes/AB.md`, `MOS6502_Opcodes/DF.md` | `decoder.v` |
| Precharged buses and "ground wins" conflicts behind `$AB` | `6502/whatwhen.md`, `6502/Readme.md` | `busmux.v` |
| BCD disabled in the 2A03/2A07 core | `APU/core.md`, `Ricoh.md` | `alu_control.v` (`/DAA`, `/DSA`) |

## Open questions

- **`JMP ($xxFF)` page-wrap.** Neither the Wiki pages, the `Docs/6502`
  instruction list nor the HDL walkthroughs document the indirect-JMP page-wrap
  bug (high byte fetched from the same page when the pointer low byte is `$FF`).
  It is **unconfirmed** by ranks 1–3 and is deliberately not specified above; a
  `MOS6502_Opcodes/6C.md` walkthrough or an HDL test would settle it.
- **Full undocumented opcode set.** Only `$AB` (ATX) and `$DF` (DCP) have
  walkthroughs; the rest (aliases, `NOP` variants, unstable instructions, DMA
  interaction) is out of scope until the repository documents it.
- **Dummy-read address on a page cross.** The sources state the +1 cycle (`PGX`)
  but not the address presented during it. §4 uses the un-carried address (the
  community-documented behaviour); confirm against the schematic before treating
  it as silicon-derived.
- **`Docs/6502/6502 Instruction Set.htm` errors.** Its appendix lists
  `DEC absolute` (`CE`) as 3 cycles instead of 6, repeats the preceding
  instruction's mnemonic in the single-line entries for `DEX` (`DEC CA`), `DEY`
  (`DEC 88`) and `PLP` (`PHP 28`), leaves `LSR`'s N column as unmodified
  (`- + + - - -`), and marks N/Z as modified for `TXS` (`+ + - - - -`). Its `ASL`
  and `ROL` sections are correct. §2 uses the opcode matrix plus the shared
  RMW/shift decode lines; which source is authoritative?
- **`/PRDY` Branch-T0 skip.** Its purpose is unknown (`decoder.md`) and it is not
  stated whether it also applies to interrupt entries.
- **NESdev source coverage.** The local NESdev cache's `CPU_instructions.html`
  capture is an empty Wayback landing page (4.6 kB, no article body), the live
  page returns HTTP 403, and the Internet Archive was offline for the cache date,
  so the opcode and cycle counts of §2 are **not** textually cross-checked against
  that page. `CPU_addressing_modes`, `CPU_registers`, `Status_flags` and
  `CPU_interrupts` *were* available and are the basis of §1, §3, §4, §5 and §9.
  The die-level material of §9 is `Visual_circuit_tutorial` (`Visual_2A03` does
  not exist), and `CPU_unofficial_opcodes` is corroborated by the Wayback CDX
  index (ten captures, earliest 2022-03-14); only the `CPU_instructions` title
  remains unconfirmed, so treat that one row of §9 as unverified until a capture
  is available.
- **Interrupt hijacking and the delayed `/IRQ` response** (phenomena 10 and 11)
  are documented by NESdev but have no rank 1–3 source in this repository; they
  need a schematic/HDL check (the `brk.v` and `ir.v` sampling points) before they
  can be called silicon-derived.
- The per-register RMW consequences for `$2000–$2007`/`$4014` belong to the PPU
  and DMA sections.
