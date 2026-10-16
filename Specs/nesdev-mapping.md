# NESdev Wiki mapping index (issue #1392)

This index connects the section specs under `Specs/` to the NESdev Wiki. Use
Table 1 to find the source material behind a section, Table 2 to jump from a
topic in a section to the community page a reader would otherwise consult, and
Table 3 to find every section that leans on a given page.

`www.nesdev.org` returns HTTP 403 to non-browser clients (a bot check), so read
every page through the Wayback Machine: the access form is

```
https://web.archive.org/web/2024/https://www.nesdev.org/wiki/<Page_Name>
```

Page names below are the wiki titles as the spec files wrote them, unified where
two files used different spellings for the same page (see *Unification notes* at
the end; the only rename applied is `Clock_rate` → `Cycle_reference_chart`, cited
below as `Cycle_reference_chart`).
Names are not invented here: any page a spec file does not name is not in this
index. Two `Visual6502wiki/...` rows live on the separate Visual6502 wiki, not
on nesdev.org (footnote to Table 3).

The `## NESdev Wiki correspondence` tables of all 20 section files were
combined: 301 topic→page instances, 202 distinct topics, 72 distinct pages.

## Table 1 — Spec file → source material

One row per spec file. The Breaking NES Wiki and HDL columns are the file's own
`Primary sources`; a handful of files also draw on `Docs/`, `MOS6502_Opcodes/`,
`Scripts/` and the Logisim schematic, which are folded into the HDL column where
they are HDL/schematic material and otherwise noted in the cell.

| Spec file | Breaking NES Wiki pages | HDL modules |
|---|---|---|
| `01-platform-overview.md` | `Readme.md`, `MOS.md`, `Ricoh.md`, `nmos.md`, `Dendy.md`, `MB/{Readme,NES,Famicom}.md`, `6502/Readme.md`, `APU/{Readme,pads,regs,core,clkgen,old_2a03,pal}.md`, `PPU/{Readme,pads,regs,pclk,hv,hv_decoder,fsm,pal,cram,vram_ctrl,par,umc6538}.md` | `HDL/APU/{core,clkgen,regs,dma,pads}.v`, `HDL/PPU/{pclk,vram_ctrl,cram,regs,pads,hv,hv_decoder}.v`, `HDL/Common/bustris.v` |
| `02-cpu-core.md` | `6502/{Readme,regs,pc,flags,interrupts,ir,whatwhen,busmux,address_bus,data_bus,pads,clock,context_control,regs_control,extra_counter,dispatch}.md`, `APU/core.md`, `APU/pads.md`, `Ricoh.md` | `HDL/Core6502/{core_top,pads,clock,regs,flags,flags_control,pc,pc_control,addr_bus,data_bus,busmux,bus_control,brk,ir,branch_logic,dispatch,alu_control,extra_counter}.v` |
| `02-cpu-dma.md` | `APU/{dma,dpcm,core,cycles,regs,pads,common,waves,old_2a03,pal,Readme}.md`, `6502/{pads,decoder}.md` | `HDL/APU/{dma,dmabuf,core,dpcm,regs,clkgen,pads,io}.v`, `HDL/Common/{dlatch,rsff}.v`, `HDL/Core6502/{pads,core_top}.v`, `HDL/Framework/Icarus/apu/{oam_dma,both_dma}.v` |
| `02-cpu-instructions.md` | `6502/{decoder,predecode,random_logic,dispatch,alu,alu_control,flags_control,branch_logic,extra_counter,context_control,whatwhen}.md`; Docs `6502/6502 Instruction Set.htm`, `6502/6502 - Integration.pdf`, `MOS6502_Opcodes/{Readme,A9,A2,8D,20,EE,DF,AB,brk}.md` | `HDL/Core6502/{decoder,dispatch,brk,branch_logic,extra_counter,pc_control,alu,alu_control,flags,flags_control,bus_control,core_top}.v` |
| `03-apu-dmc.md` | `APU/{dpcm,dma,dac,common,clkgen,pal,waves}.md` | `HDL/APU/{dpcm,dac,common,dma,regs}.v` |
| `03-apu-frame-counter.md` | `APU/{clkgen,waves,cycles,regs,common,length,square,triangle,noise,dpcm,pal,pads,visual2a03}.md` | `HDL/APU/{clkgen,regs,apu_top,common,env,square,triangle,noise,length,dpcm}.v` |
| `03-apu-mixer.md` | `APU/{dac,pads,sound,common,regs,pal,Readme}.md` | `HDL/APU/{dac,apu_top,pads,regs}.v`, `HDL/Framework/Icarus/apu/{aux_test,auxout}.v`, `auxa.mem`, `auxb.mem`; model `Scripts/APU_DAC_Generator.py` |
| `03-apu-noise.md` | `APU/{noise,length,common,clkgen,pal,waves}.md` | `HDL/APU/{noise,length,env,common}.v` |
| `03-apu-overview.md` | `APU/` — `{Readme,contents,regs,pads,common,core,clkgen,sound,waves,cycles,visual2a03,test_pattern,old_2a03,pal,dma,dpcm,square,triangle,noise,length,dac}.md` | `HDL/APU/{apu_top,regs,pads,io,common,clkgen,core,dma,dmabuf,dpcm,length,test,dac}.v` |
| `03-apu-pulse.md` | `APU/{square,length,sound,common,waves,clkgen}.md` | `HDL/APU/{square,env,length,common,apu_top,regs}.v`; Icarus `{square_output,square_duty,square_sweep,env_unit,length_counter}.v` |
| `03-apu-triangle.md` | `APU/{triangle,length,sound,common,waves,clkgen,regs}.md` | `HDL/APU/{triangle,length,common,regs}.v`; Icarus `{triangle_output,length_counter}.v` |
| `04-ppu-memory-map.md` | `PPU/{vram_ctrl,oam,cram,par,scroll_regs,pamux,tilecnt,mux,regs,dataread,pads,Readme}.md` | `HDL/PPU/{vram_ctrl,oam,cram,pamux,tilecnt,par,scroll_regs,pads,ppu_top,regs}.v`; schematic `Logisim/Evolution/PPU_Evo.circ` |
| `04-ppu-overview.md` | `PPU/{Readme,regs,pads,fsm,mux,hv,pclk,visual2c02,umc6538,rgb,rails}.md`, `Ricoh.md` | `HDL/PPU/{ppu_top,pads,regs,fsm,hv,pclk}.v` |
| `04-ppu-palette.md` | `PPU/{cram,pal,rgb,video_out,tv,mux,pads,umc6538,visual2c02}.md` | `HDL/PPU/{cram,video_out,dac,pclk,ppu_top}.v` |
| `04-ppu-registers.md` | `PPU/{regs,scroll_regs,oam,cram,vram_ctrl,par,fsm,mux,pads,rails}.md` | `HDL/PPU/{regs,pads,scroll_regs,oam,vram_ctrl,ppu_top}.v` |
| `04-ppu-rendering.md` | `PPU/{dataread,bgcol,mux,fsm,tilecnt,par,pamux,hv_decoder,hv,regs,fifo,obj_eval}.md` | `HDL/PPU/{fsm,bgcol,tilecnt,mux,hv,hv_decoder,par,pamux,fifo,obj_eval,ppu_top}.v` |
| `04-ppu-scrolling.md` | `PPU/{scroll_regs,tilecnt,pamux,par,hv,hv_decoder,fsm,regs,dataread,vram_ctrl}.md` | `HDL/PPU/{scroll_regs,par,tilecnt,pamux,hv,hv_decoder,fsm,regs,vram_ctrl}.v` |
| `04-ppu-sprites.md` | `PPU/{oam,obj_eval,fifo,mux,fsm,par,hv_decoder,regs}.md` | `HDL/PPU/{oam,obj_eval,fifo,mux,fsm,ppu_top}.v` |
| `04-ppu-timing.md` | `PPU/{pclk,hv,hv_decoder,fsm,pal,umc6538,pads,waves}.md`, `Dendy.md` | `HDL/PPU/{pclk,hv,hv_decoder,fsm,ppu_top}.v` |
| `05-boards-and-io.md` | `MB/{Readme,NES,Famicom}.md`, `Dendy.md`, `APU/{pads,regs}.md`, `PPU/pads.md` | `HDL/APU/{io,pads,regs}.v`, `HDL/Common/bustris.v`; board-level `Docs/NES/NES_HDL_Model/nes.v`, `Docs/Famicom/Famicom_Evo.circ` |

## Table 2 — Topic → NESdev Wiki

Collected and de-duplicated from the `## NESdev Wiki correspondence` tables of all
20 section files, in spec order (01 → 05). One row per distinct topic; the page
cell lists every unified page that topic maps to, and the last cell lists every
spec file that states the row. **202 rows.**

| Topic | NESdev Wiki page | Spec file |
|---|---|---|
| System overview, terminology | `NES_reference_guide`, `Glossary` | `01-platform-overview.md` |
| CPU memory map, mirrors, `$4014`/`$4015`, `$4018–$401F` test mode | `CPU_memory_map`, `CPU_Test_Mode` | `01-platform-overview.md` |
| PPU memory map, nametables, attribute tables, mirroring, palettes | `PPU_memory_map`, `PPU_nametables`, `PPU_attribute_tables`, `Mirroring`, `PPU_palettes` | `01-platform-overview.md` |
| Master clock, CPU:PPU ratio, PAL/Dendy, vblank timing | `Cycle_reference_chart`, `PPU_frame_timing` | `01-platform-overview.md` |
| Chip pinouts and VRAM bus | `CPU_pinout`, `PPU_pinout` | `01-platform-overview.md` |
| CPU-facing PPU registers | `PPU_registers` | `01-platform-overview.md` |
| APU register map and frame counter | `APU_registers`, `APU_Frame_Counter` | `01-platform-overview.md` |
| Interrupts (NMI/IRQ) | `CPU_interrupts` | `01-platform-overview.md` |
| DMA/OAM | `DMA`, `PPU_OAM` | `01-platform-overview.md` |
| Controllers and expansion port | `Standard_controller`, `Expansion_port` | `01-platform-overview.md` |
| CIC lockout | `CIC_lockout_chip` | `01-platform-overview.md` |
| Cartridge connector and mappers | `Cartridge_connector`, `Mapper` | `01-platform-overview.md` |
| Region/model variants, revisions | `Famicom`, `CPU_variants`, `PPU_variants` | `01-platform-overview.md` |
| Motherboard revisions and glue logic | `NES_reference_guide` | `01-platform-overview.md` |
| Power-on state | `CPU_power_up_state`, `PPU_power_up_state` | `01-platform-overview.md` |
| Palette RAM and emphasis | `PPU_palettes`, `Colour_emphasis` | `01-platform-overview.md` |
| 2A03/2A07 pinout, `M2`, `/RST`, `TST`, `RDY`, `/NMI`/`/IRQ`, φ1/φ2 half-cycles | `CPU_pinout` | `02-cpu-core.md` |
| Clock divider and CPU:PPU ratios (NTSC /12, PAL /16, Dendy /15) | `Cycle_reference_chart` | `02-cpu-core.md` |
| Architectural registers A/X/Y/S/P/PC | `CPU_registers` | `02-cpu-core.md` |
| Status flags, bit layout, B flag, D flag, SO not exposed | `Status_flags` | `02-cpu-core.md` |
| Address map and mirrors (`$0000–$FFFF`) | `CPU_memory_map` | `02-cpu-core.md` |
| Addressing modes with operand examples | `CPU_addressing_modes` | `02-cpu-core.md` |
| T-state cycle framework and per-cycle bus behaviour | `Visual6502wiki/6502_Timing_States` | `02-cpu-core.md` |
| CPU open bus vs internal precharge (capacitive data bus) | `Open_bus_behavior` | `02-cpu-core.md` |
| Interrupts, vectors, BRK sequence, hijacking, reset, B bit | `CPU_interrupts`, `Visual6502wiki/6502_BRK_and_B_bit` | `02-cpu-core.md` |
| Power-on and post-reset register values | `CPU_power_up_state` | `02-cpu-core.md` |
| 2A03/2A07 revisions, pin-30 `/RDY`, BCD cut-out, clones (§10) | `CPU_variants` | `02-cpu-core.md` |
| Die-level visual reference for the 2A03 core | `Visual_circuit_tutorial` | `02-cpu-core.md` |
| The two DMA units, `RDY` halt, get/put cycles, alignment, dummy and repeated reads | `DMA` | `02-cpu-dma.md` |
| 2A03/2A07 pinout, pin 30 `TST`, no `RDY` package pin | `CPU_pinout` | `02-cpu-dma.md` |
| CPU cycle period and the NTSC /12, PAL /16 clock division | `Cycle_reference_chart` | `02-cpu-dma.md` |
| APU clock phases (NESdev `apu_clk1`/`apu_/clk2`) and die-level signal names | `Visual_circuit_tutorial` | `02-cpu-dma.md` |
| OAM layout, `OAMDATA ($2004)`, the 256-byte `OAMDMA ($4014)` copy | `PPU_OAM` | `02-cpu-dma.md` |
| DMC registers `$4010`–`$4013`, `$4015` enable/status, sample address and length, load/reload DMA | `APU_DMC` | `02-cpu-dma.md` |
| APU register map `$4000–$4017`, `$4015` status bits, joypad registers | `APU_registers`, `APU` | `02-cpu-dma.md` |
| PPU registers and the read side effects of `PPUDATA ($2007)` / `PPUSTATUS ($2002)` | `PPU_registers` | `02-cpu-dma.md` |
| CPU memory map; the `$4000–$401F` register-decode quirk that lets DMA activate registers | `CPU_memory_map` | `02-cpu-dma.md` |
| Bus conflicts and open bus during DMA | `Bus_conflict`, `Open_bus_behavior` | `02-cpu-dma.md` |
| DMC-vs-joypad conflict and the redundant-read workaround | `Controller_reading`, `Standard_controller` | `02-cpu-dma.md` |
| Interrupts blocked while `RDY` is low | `DMA` | `02-cpu-dma.md` |
| 2A07/PAL and clone differences | `APU_DMC`, `CPU_pinout` | `02-cpu-dma.md` |
| Official opcode table, mnemonics, lengths, cycle counts | `CPU_instructions` | `02-cpu-instructions.md` |
| Addressing modes: names, operand syntax and byte/cycle length | `CPU_addressing_modes` | `02-cpu-instructions.md` |
| A/X/Y/S/P registers and the architectural register set | `CPU_registers` | `02-cpu-instructions.md` |
| Status flag meanings, the B flag, bit 4/5 behaviour | `Status_flags` | `02-cpu-instructions.md` |
| `/IRQ`/`/NMI` frames, vectors, interrupt hijacking, delayed IRQ response | `CPU_interrupts` | `02-cpu-instructions.md` |
| Undocumented/illegal opcodes | `CPU_unofficial_opcodes` | `02-cpu-instructions.md` |
| Per-cycle bus behaviour on the real die | `Visual_circuit_tutorial` | `02-cpu-instructions.md` |
| DMC registers, rates, sample address/length | `APU_DMC` | `03-apu-dmc.md` |
| DMA cycles, `RDY`, conflicts with `$2007`/`$4016` | `DMA`, `Open_bus_behavior` | `03-apu-dmc.md` |
| APU register map `$4000`–`$4017` | `APU_registers`, `APU` | `03-apu-dmc.md`, `03-apu-noise.md` |
| Length-counter/status bits of `$4015` | `APU_Length_Counter` | `03-apu-dmc.md` |
| APU IRQ routing | `CPU_interrupts` | `03-apu-dmc.md` |
| DAC and mixing (`AUX_B`) | `APU_Mixer` | `03-apu-dmc.md` |
| OAM DMA (`$4014`) | `PPU_OAM` | `03-apu-dmc.md` |
| 2A03/2A07 revision differences | `CPU_variants` | `03-apu-dmc.md`, `03-apu-noise.md` |
| Frame counter registers, write side effects, 4/5-step tables | `APU_Frame_Counter` | `03-apu-frame-counter.md` |
| Sequencer structure (LFSR, PLA outputs, anti-zero/SET LFSR) | `APU_Frame_Counter` | `03-apu-frame-counter.md` |
| `$4017` / `$4015` bitfields and addresses | `APU_registers` | `03-apu-frame-counter.md` |
| Quarter frame consumers (envelope, linear counter) | `APU_Envelope`, `APU_Triangle` | `03-apu-frame-counter.md` |
| Half frame consumers (length counter, sweep) | `APU_Length_Counter`, `APU_Sweep` | `03-apu-frame-counter.md` |
| DMC interrupt flag and its clearing rules | `APU_DMC` | `03-apu-frame-counter.md` |
| APU clock tree, divider and APU cycle rate | `Cycle_reference_chart` | `03-apu-frame-counter.md` |
| Reset and power-on state of `$4017` and the frame counter LFSR | `CPU_power_up_state` | `03-apu-frame-counter.md` |
| PAL 2A07 step counts and frame rate | `APU_Frame_Counter`, `Cycle_reference_chart` | `03-apu-frame-counter.md` |
| Non-linear mixing formulas and lookup tables | `APU_Mixer` | `03-apu-mixer.md` |
| Channel output values fed to the DACs | `APU_Pulse`, `APU_Triangle`, `APU_Noise`, `APU_DMC` | `03-apu-mixer.md` |
| `$4011` direct DMC level load | `APU_DMC` | `03-apu-mixer.md` |
| Output filter chain after the DACs (NES vs Famicom) | `APU_Mixer` | `03-apu-mixer.md` |
| CPU/APU package pins (`AD1`/`AD2`) | `CPU_pinout` | `03-apu-mixer.md` |
| APU register map | `APU_registers` | `03-apu-mixer.md` |
| 2A07 (PAL) region differences relevant to the DAC output | `CPU_variants`, `Cycle_reference_chart` | `03-apu-mixer.md` |
| Noise channel registers and period table | `APU_Noise` | `03-apu-noise.md` |
| Envelope unit | `APU_Envelope` | `03-apu-noise.md` |
| Length counter, halt, reload | `APU_Length_Counter` | `03-apu-noise.md` |
| Frame counter quarter/half-frame clocks | `APU_Frame_Counter` | `03-apu-noise.md` |
| DAC and non-linear mixing (`AUX_B`) | `APU_Mixer` | `03-apu-noise.md` |
| APU block overview, channels, register summary | `APU` | `03-apu-overview.md` |
| Complete `$4000–$4017` register map and bit fields | `APU_registers` | `03-apu-overview.md` |
| Frame counter, `$4017` modes, frame IRQ, LFO rates | `APU_Frame_Counter` | `03-apu-overview.md` |
| DMC: registers `$4010–$4013`, DMA, IRQ flag | `APU_DMC` | `03-apu-overview.md` |
| Pulse channel bit fields (`$4000–$4007`) | `APU_Pulse` | `03-apu-overview.md` |
| Triangle channel bit fields (`$4008–$400B`) | `APU_Triangle` | `03-apu-overview.md` |
| Noise channel bit fields (`$400C–$400F`) | `APU_Noise` | `03-apu-overview.md` |
| Envelope (loop / constant-volume bits) | `APU_Envelope` | `03-apu-overview.md` |
| Sweep (`$4001`/`$4005`) | `APU_Sweep` | `03-apu-overview.md` |
| Length-counter enables and `$4015` status bits | `APU_Length_Counter`, `APU_registers` | `03-apu-overview.md` |
| DAC outputs AUX A / AUX B | `APU_Mixer` | `03-apu-overview.md` |
| OAM DMA via `$4014` | `PPU_OAM` | `03-apu-overview.md` |
| `$4016`/`$4017` controller ports | `Standard_controller`, `Expansion_port` | `03-apu-overview.md` |
| Open-bus behaviour of the write-only and undriven registers | `Open_bus_behavior` | `03-apu-overview.md` |
| Clock hierarchy, APU/CPU clock rates and dividers | `Cycle_reference_chart` | `03-apu-overview.md` |
| Pinout, `M2`, `RDY`, `/IRQ`, `/NMI`, 2A03 vs 2A07 | `CPU_pinout` | `03-apu-overview.md` |
| Memory map of the `$4000` region | `CPU_memory_map` | `03-apu-overview.md` |
| 2A03 test registers `$4018–$401A`, `DBG`, PAL/early revision differences | `CPU_pinout`, `APU` | `03-apu-overview.md` |
| Channel block structure, timer, sequencer, output muting threshold | `APU_Pulse` | `03-apu-pulse.md` |
| Duty cycle sequences and the downward sequencer counter | `APU_Pulse` | `03-apu-pulse.md` |
| Envelope generator: start/loop flags, decay level, constant volume | `APU_Envelope` | `03-apu-pulse.md` |
| Sweep divider, target period, negate difference, sweep muting | `APU_Sweep` | `03-apu-pulse.md` |
| Length counter table, halt flag, `$4015` enable/disable | `APU_Length_Counter` | `03-apu-pulse.md`, `03-apu-triangle.md` |
| `$4000`–`$4007` register layout and bit fields | `APU_registers` | `03-apu-pulse.md` |
| Quarter-frame and half-frame clocks | `APU_Frame_Counter` | `03-apu-pulse.md`, `03-apu-triangle.md` |
| CPU clock rates (NTSC 2A03, PAL 2A07, Dendy) | `APU_Pulse`, `Cycle_reference_chart`, `APU_Triangle` | `03-apu-pulse.md`, `03-apu-triangle.md` |
| 2A03 test registers `$4018`–`$401A` and `LOCK` | `CPU_pinout` | `03-apu-pulse.md`, `03-apu-triangle.md` |
| Pulse output contribution to the non-linear mixer | `APU_Mixer` | `03-apu-pulse.md` |
| Triangle block structure, CPU-rate timer, sequencer, linear counter | `APU_Triangle` | `03-apu-triangle.md` |
| 32-step sequence and the triangle silencing methods (`$80`/`$FF`, `$4015`, period 0/1) | `APU_Triangle` | `03-apu-triangle.md` |
| `$4008`–`$400B` register layout and bit fields | `APU_registers` | `03-apu-triangle.md` |
| Triangle output contribution to the non-linear mixer | `APU_Mixer` | `03-apu-triangle.md` |
| 16 KB PPU memory map, mapper responsibilities, the `$3000–$3EFF` mirror | `PPU_memory_map` | `04-ppu-memory-map.md` |
| `v`/`t` address bit fields, PPUADDR/PPUSCROLL writes, nametable/attribute fetch addresses | `PPU_scrolling`, `PPU_registers` | `04-ppu-memory-map.md` |
| Nametable layout, 960 + 64 bytes, four screens | `PPU_nametables` | `04-ppu-memory-map.md` |
| Mirroring modes and CIRAM A10 wiring | `Mirroring` | `04-ppu-memory-map.md` |
| Attribute table layout, quadrants, glitches | `PPU_attribute_tables` | `04-ppu-memory-map.md` |
| Pattern tables, bit planes, CHR address format | `PPU_pattern_tables`, `CHR_ROM_vs._CHR_RAM` | `04-ppu-memory-map.md` |
| Palette RAM, backdrop, backdrop override, colour values | `PPU_palettes` | `04-ppu-memory-map.md` |
| `$2007` read buffer, palette reads, increment | `PPU_registers`, `PPU_scrolling` | `04-ppu-memory-map.md` |
| OAM structure, `$2003`/`$2004`/`$4014` | `PPU_OAM` | `04-ppu-memory-map.md` |
| PPU bus pins, `ALE`, the VRAM `/RD`/`/WR` strobes, the CPU-interface R/W decoder, VRAM controller | `PPU_pinout`, `PPU_registers` | `04-ppu-memory-map.md` |
| PPU chip overview, block roles | `PPU` | `04-ppu-overview.md` |
| Internal data buses, capacitive DB, PPU open bus | `Open_bus_behavior`, `PPU` | `04-ppu-overview.md` |
| Register map, mirroring, open bus, 2C05 identifier | `PPU_registers`, `Open_bus_behavior` | `04-ppu-overview.md` |
| Power-on / reset values and write suppression | `PPU_power_up_state` | `04-ppu-overview.md` |
| Pinout and signal names | `PPU_pinout` | `04-ppu-overview.md` |
| Visual 2C02 node names | `Visual_2C02` | `04-ppu-overview.md` |
| Revision list and per-revision bugs | `PPU_variants` | `04-ppu-overview.md` |
| Region clock ratios and frame geometry | `Cycle_reference_chart`, `PPU_rendering` | `04-ppu-overview.md` |
| PAL red/green emphasis swap on 2C07/Dendy | `Cycle_reference_chart`, `Errata` | `04-ppu-overview.md` |
| Palette-read support and palette corruption | `PPU_registers` | `04-ppu-overview.md` |
| 2C05 `$2000`/`$2001` swap seen from the board side | `PPU_variants`, `Cycle_reference_chart` | `04-ppu-overview.md` |
| Palette RAM layout, mirrors, backdrop, entry format | `PPU_palettes` | `04-ppu-palette.md` |
| Backdrop/priority rules, transparent colour 0 | `PPU_rendering` | `04-ppu-palette.md` |
| `$3Fxx` accesses: direct colour, palette reads and the `$2007` bypass | `PPU_registers` | `04-ppu-palette.md` |
| NTSC colour generation, phases, 12 hues | `NTSC_video` | `04-ppu-palette.md` |
| PAL colour generation and phase alternation | `PAL_video` | `04-ppu-palette.md` |
| Colour emphasis bits and levels | `Colour_emphasis` | `04-ppu-palette.md` |
| RGB PPU pin changes and behaviour | `PPU_pinout`, `PPU_variants` | `04-ppu-palette.md` |
| Palette readability per revision | `PPU_variants` | `04-ppu-palette.md` |
| Palette contents at power-on (unspecified) | `PPU_power_up_state` | `04-ppu-palette.md` |
| Register summary, bit fields, mirroring | `PPU_registers` | `04-ppu-registers.md` |
| `$2005`/`$2006`/`v`/`t`/`x`/`w`, dot schedule, splits | `PPU_scrolling` | `04-ppu-registers.md` |
| Power-on/reset values and the ignored-write window | `PPU_power_up_state` | `04-ppu-registers.md`, `04-ppu-timing.md` |
| PPU open bus and reads of write-only registers | `Open_bus_behavior` | `04-ppu-registers.md` |
| OAM layout, `$2004`, `$4014` | `PPU_OAM` | `04-ppu-registers.md` |
| OAM corruption, $2004 readability, per-revision differences | `PPU_registers`, `Errata`, `PPU_variants` | `04-ppu-registers.md` |
| Vblank flag race and NMI | `NMI`, `PPU_rendering` | `04-ppu-registers.md` |
| 2C05 `$2000`/`$2001` swap on arcade boards | `PPU_variants`, `Cycle_reference_chart` | `04-ppu-registers.md` |
| Per-dot background fetch schedule, prefetch, dummy fetches | `PPU_rendering` | `04-ppu-rendering.md` |
| Scanline dot numbering 0–340 and the `/VIS` window | `PPU_rendering`, `PPU_frame_timing` | `04-ppu-rendering.md` |
| Pattern shift registers, fine X, attribute latches | `PPU_rendering`, `PPU_scrolling` | `04-ppu-rendering.md` |
| Priority multiplexer decision table, backdrop/EXT | `PPU_rendering`, `PPU_palettes` | `04-ppu-rendering.md` |
| Sprite priority quirk | `PPU_sprite_priority` | `04-ppu-rendering.md` |
| Sprite 0 hit conditions | `PPU_rendering`, `PPU_registers` | `04-ppu-rendering.md`, `04-ppu-sprites.md` |
| Left-column clipping (PPUMASK bits 1–2) | `PPU_registers` | `04-ppu-rendering.md` |
| Forced blank / rendering enable | `PPU_registers`, `PPU_rendering` | `04-ppu-rendering.md` |
| Odd-frame skipped dot, pre-render line length | `PPU_rendering`, `PPU_frame_timing` | `04-ppu-rendering.md` |
| Pattern table layout and background pattern address | `PPU_pattern_tables` | `04-ppu-rendering.md` |
| `v` updates during rendering | `PPU_scrolling` | `04-ppu-rendering.md` |
| Direct colour / backdrop override | `PPU_rendering`, `PPU_palettes` | `04-ppu-rendering.md` |
| `v`/`t`/`x`/`w` register names, sizes and roles | `PPU_scrolling` | `04-ppu-scrolling.md` |
| `$2005` first/second, `$2006` high/low and `$2000` bit 0–1 write sequences | `PPU_scrolling`, `PPU_registers` | `04-ppu-scrolling.md` |
| Per-dot schedule, prefetch, dummy fetches | `PPU_rendering` | `04-ppu-scrolling.md` |
| Coarse X and coarse Y wrapping rules | `PPU_scrolling` | `04-ppu-scrolling.md` |
| Nametable/attribute address formulas and the seam | `PPU_scrolling`, `PPU_nametables`, `PPU_attribute_tables` | `04-ppu-scrolling.md` |
| Nametable arrangement / mirroring and the CIRAM A10 select | `Mirroring`, `PPU_nametables` | `04-ppu-scrolling.md` |
| Mid-frame scroll splits | `PPU_scrolling` | `04-ppu-scrolling.md` |
| `$2000` bit 0 race | `PPU_registers` | `04-ppu-scrolling.md` |
| `$2006` second-write bus conflict | `PPU_registers` | `04-ppu-scrolling.md` |
| `$2007` accesses during rendering | `PPU_scrolling` | `04-ppu-scrolling.md` |
| OAM layout, attributes, `$2003`/`$2004`, OAM DMA | `PPU_OAM` | `04-ppu-sprites.md` |
| Scanline dot numbering 0–340 and the sprite windows | `PPU_rendering`, `PPU_sprite_evaluation` | `04-ppu-sprites.md` |
| Sprite evaluation schedule, secondary OAM, 8-sprite limit | `PPU_sprite_evaluation`, `PPU_rendering` | `04-ppu-sprites.md` |
| Sprite overflow bug and its cause | `PPU_sprite_evaluation` | `04-ppu-sprites.md` |
| Sprite priority and the priority quirk | `PPU_sprite_priority` | `04-ppu-sprites.md` |
| 8×16 sprite tile layout and pattern table selection | `PPU_OAM`, `PPU_pattern_tables` | `04-ppu-sprites.md` |
| OAM decay and refresh practice | `PPU_OAM`, `PPU_power_up_state` | `04-ppu-sprites.md` |
| Sprite rendering dots and pipeline | `PPU_rendering` | `04-ppu-sprites.md` |
| OAM corruption per revision (OAMADDR writes, evaluation start) | `PPU_variants`, `Errata` | `04-ppu-sprites.md` |
| Scanline/dot structure, 341 dots, fetch schedule | `PPU_rendering` | `04-ppu-timing.md` |
| H/V counters and decoder row positions | `PPU_rendering` | `04-ppu-timing.md` |
| Frame length, even/odd frames, VBL flag timing | `PPU_frame_timing`, `PPU_rendering` | `04-ppu-timing.md` |
| Master clock, CPU:PPU ratios, frame rates | `Cycle_reference_chart` | `04-ppu-timing.md` |
| VBlank timing, NMI and the `$2002` flag | `PPU_registers`, `PPU_frame_timing` | `04-ppu-timing.md` |
| Odd-frame dot skip ("even/odd frames") | `PPU_frame_timing`, `PPU_rendering` | `04-ppu-timing.md` |
| NTSC/PAL video line structure, sync and burst | `NTSC_video`, `PAL_video` | `04-ppu-timing.md` |
| Pinout and clock pin | `PPU_pinout` | `04-ppu-timing.md` |
| Per-revision timing (2C07, UMC 6538/Dendy) | `PPU_variants` | `04-ppu-timing.md` |
| Console models, region variants, revision history | `Famicom`, `NES_reference_guide` | `05-boards-and-io.md` |
| CPU and PPU chip variants | `CPU_variants`, `PPU_variants` | `05-boards-and-io.md` |
| Board chip inventory, JIO integration, logic ICs, chip list | `Hardware_pinout`, `CPU_variants`, `PPU_variants` | `05-boards-and-io.md` |
| Master clock, CPU:PPU ratio, PAL and Dendy division | `Cycle_reference_chart`, `NES_reference_guide` | `05-boards-and-io.md` |
| CIC lockout chip, lock/key handshake, reset loop, defeat, pinout | `CIC_lockout_chip`, `CIC_lockout_chip_pinout` | `05-boards-and-io.md` |
| CPU address decode, mirrors, `/ROMSEL`, WRAM and PPU selects | `CPU_memory_map`, `Cartridge_connector` | `05-boards-and-io.md` |
| Cartridge edge signals, bus ownership, CIRAM A10 / CIRAM `/CE` | `Cartridge_connector`, `Mirroring` | `05-boards-and-io.md` |
| Cartridge-edge and expansion-port `EXP0–EXP9` pins | `EXP_pins`, `Cartridge_connector` | `05-boards-and-io.md` |
| Standard controller protocol, `$4016` write bits, D0–D4 line use | `Standard_controller`, `Controller_reading`, `Controller_port_pinout` | `05-boards-and-io.md` |
| Zapper, Four Score and other controller hardware | `Zapper`, `Four_player_adapters` | `05-boards-and-io.md` |
| DPCM/DMC-vs-controller conflict and the software workarounds | `Controller_reading_code`, `APU_DMC`, `Errata` | `05-boards-and-io.md` |
| Famicom DA-15 and NES 48-pin expansion ports | `Expansion_port`, `Hardware_pinout` | `05-boards-and-io.md` |
| Expansion audio and board-level mixing | `APU_Mixer`, `Category:Expansion_audio` | `05-boards-and-io.md` |
| iNES/NES 2.0 mapper numbers, families, discrete vs ASIC, bus conflicts | `Mapper`, `INES`, `NES_2.0`, `Bus_conflict` | `05-boards-and-io.md` |
| Open bus on the controller ports | `Controller_reading`, `Open_bus_behavior` | `05-boards-and-io.md` |

## Table 3 — NESdev Wiki page → spec files (reverse index)

Every distinct page named in Table 2, sorted by page name. **72 pages.**
`Visual6502wiki/*`[^v6502] and `Category:Expansion_audio`[^cat] are included for
completeness but are not nesdev.org article pages.

| NESdev Wiki page | Spec files |
|---|---|
| `APU` | `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-noise.md`, `03-apu-overview.md` |
| `APU_DMC` | `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-frame-counter.md`, `03-apu-mixer.md`, `03-apu-overview.md`, `05-boards-and-io.md` |
| `APU_Envelope` | `03-apu-frame-counter.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md` |
| `APU_Frame_Counter` | `01-platform-overview.md`, `03-apu-frame-counter.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md` |
| `APU_Length_Counter` | `03-apu-dmc.md`, `03-apu-frame-counter.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md` |
| `APU_Mixer` | `03-apu-dmc.md`, `03-apu-mixer.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md`, `05-boards-and-io.md` |
| `APU_Noise` | `03-apu-mixer.md`, `03-apu-noise.md`, `03-apu-overview.md` |
| `APU_Pulse` | `03-apu-mixer.md`, `03-apu-overview.md`, `03-apu-pulse.md` |
| `APU_registers` | `01-platform-overview.md`, `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-frame-counter.md`, `03-apu-mixer.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md` |
| `APU_Sweep` | `03-apu-frame-counter.md`, `03-apu-overview.md`, `03-apu-pulse.md` |
| `APU_Triangle` | `03-apu-frame-counter.md`, `03-apu-mixer.md`, `03-apu-overview.md`, `03-apu-triangle.md` |
| `Bus_conflict` | `02-cpu-dma.md`, `05-boards-and-io.md` |
| `Cartridge_connector` | `01-platform-overview.md`, `05-boards-and-io.md` |
| `Category:Expansion_audio` [^cat] | `05-boards-and-io.md` |
| `CHR_ROM_vs._CHR_RAM` | `04-ppu-memory-map.md` |
| `CIC_lockout_chip` | `01-platform-overview.md`, `05-boards-and-io.md` |
| `CIC_lockout_chip_pinout` | `05-boards-and-io.md` |
| `Colour_emphasis` | `01-platform-overview.md`, `04-ppu-palette.md` |
| `Controller_port_pinout` | `05-boards-and-io.md` |
| `Controller_reading` | `02-cpu-dma.md`, `05-boards-and-io.md` |
| `Controller_reading_code` | `05-boards-and-io.md` |
| `CPU_addressing_modes` | `02-cpu-core.md`, `02-cpu-instructions.md` |
| `CPU_instructions` | `02-cpu-instructions.md` |
| `CPU_interrupts` | `01-platform-overview.md`, `02-cpu-core.md`, `02-cpu-instructions.md`, `03-apu-dmc.md` |
| `CPU_memory_map` | `01-platform-overview.md`, `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-overview.md`, `05-boards-and-io.md` |
| `CPU_pinout` | `01-platform-overview.md`, `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-mixer.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md` |
| `CPU_power_up_state` | `01-platform-overview.md`, `02-cpu-core.md`, `03-apu-frame-counter.md` |
| `CPU_registers` | `02-cpu-core.md`, `02-cpu-instructions.md` |
| `CPU_Test_Mode` | `01-platform-overview.md` |
| `CPU_unofficial_opcodes` | `02-cpu-instructions.md` |
| `CPU_variants` | `01-platform-overview.md`, `02-cpu-core.md`, `03-apu-dmc.md`, `03-apu-mixer.md`, `03-apu-noise.md`, `05-boards-and-io.md` |
| `Cycle_reference_chart` | `01-platform-overview.md`, `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-frame-counter.md`, `03-apu-mixer.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md`, `04-ppu-overview.md`, `04-ppu-registers.md`, `04-ppu-timing.md`, `05-boards-and-io.md` |
| `DMA` | `01-platform-overview.md`, `02-cpu-dma.md`, `03-apu-dmc.md` |
| `Errata` | `04-ppu-overview.md`, `04-ppu-registers.md`, `04-ppu-sprites.md`, `05-boards-and-io.md` |
| `EXP_pins` | `05-boards-and-io.md` |
| `Expansion_port` | `01-platform-overview.md`, `03-apu-overview.md`, `05-boards-and-io.md` |
| `Famicom` | `01-platform-overview.md`, `05-boards-and-io.md` |
| `Four_player_adapters` | `05-boards-and-io.md` |
| `Glossary` | `01-platform-overview.md` |
| `Hardware_pinout` | `05-boards-and-io.md` |
| `INES` | `05-boards-and-io.md` |
| `Mapper` | `01-platform-overview.md`, `05-boards-and-io.md` |
| `Mirroring` | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-scrolling.md`, `05-boards-and-io.md` |
| `NES_2.0` | `05-boards-and-io.md` |
| `NES_reference_guide` | `01-platform-overview.md`, `05-boards-and-io.md` |
| `NMI` | `04-ppu-registers.md` |
| `NTSC_video` | `04-ppu-palette.md`, `04-ppu-timing.md` |
| `Open_bus_behavior` | `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-overview.md`, `04-ppu-overview.md`, `04-ppu-registers.md`, `05-boards-and-io.md` |
| `PAL_video` | `04-ppu-palette.md`, `04-ppu-timing.md` |
| `PPU` | `04-ppu-overview.md` |
| `PPU_attribute_tables` | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-scrolling.md` |
| `PPU_frame_timing` | `01-platform-overview.md`, `04-ppu-rendering.md`, `04-ppu-timing.md` |
| `PPU_memory_map` | `01-platform-overview.md`, `04-ppu-memory-map.md` |
| `PPU_nametables` | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-scrolling.md` |
| `PPU_OAM` | `01-platform-overview.md`, `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-overview.md`, `04-ppu-memory-map.md`, `04-ppu-registers.md`, `04-ppu-sprites.md` |
| `PPU_palettes` | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-palette.md`, `04-ppu-rendering.md` |
| `PPU_pattern_tables` | `04-ppu-memory-map.md`, `04-ppu-rendering.md`, `04-ppu-sprites.md` |
| `PPU_pinout` | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-timing.md` |
| `PPU_power_up_state` | `01-platform-overview.md`, `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-registers.md`, `04-ppu-sprites.md`, `04-ppu-timing.md` |
| `PPU_registers` | `01-platform-overview.md`, `02-cpu-dma.md`, `04-ppu-memory-map.md`, `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-registers.md`, `04-ppu-rendering.md`, `04-ppu-scrolling.md`, `04-ppu-sprites.md`, `04-ppu-timing.md` |
| `PPU_rendering` | `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-registers.md`, `04-ppu-rendering.md`, `04-ppu-scrolling.md`, `04-ppu-sprites.md`, `04-ppu-timing.md` |
| `PPU_scrolling` | `04-ppu-memory-map.md`, `04-ppu-registers.md`, `04-ppu-rendering.md`, `04-ppu-scrolling.md` |
| `PPU_sprite_evaluation` | `04-ppu-sprites.md` |
| `PPU_sprite_priority` | `04-ppu-rendering.md`, `04-ppu-sprites.md` |
| `PPU_variants` | `01-platform-overview.md`, `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-registers.md`, `04-ppu-sprites.md`, `04-ppu-timing.md`, `05-boards-and-io.md` |
| `Standard_controller` | `01-platform-overview.md`, `02-cpu-dma.md`, `03-apu-overview.md`, `05-boards-and-io.md` |
| `Status_flags` | `02-cpu-core.md`, `02-cpu-instructions.md` |
| `Visual6502wiki/6502_BRK_and_B_bit` [^v6502] | `02-cpu-core.md` |
| `Visual6502wiki/6502_Timing_States` [^v6502] | `02-cpu-core.md` |
| `Visual_2C02` | `04-ppu-overview.md` |
| `Visual_circuit_tutorial` | `02-cpu-core.md`, `02-cpu-dma.md`, `02-cpu-instructions.md` |
| `Zapper` | `05-boards-and-io.md` |

[^v6502]: These two pages are on the Visual6502 wiki (visual6502.org/wiki), not on
    nesdev.org; the Wayback form above does not apply. The specs use the
    `Visual6502wiki/<Page>` convention to keep them apart from NESdev titles.
[^cat]: A MediaWiki category, not an article; it has no single Wayback capture.

## Pages shared by more than one spec file (the seams)

51 of the 72 pages are cited by more than one section. These are
where two specs can drift apart: if a behaviour changes on one side, every file
listed must be re-checked together. The count is the number of spec files.

| Page | Files | Spec files |
|---|---|---|
| `Cycle_reference_chart` | 12 | `01-platform-overview.md`, `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-frame-counter.md`, `03-apu-mixer.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md`, `04-ppu-overview.md`, `04-ppu-registers.md`, `04-ppu-timing.md`, `05-boards-and-io.md` |
| `PPU_registers` | 10 | `01-platform-overview.md`, `02-cpu-dma.md`, `04-ppu-memory-map.md`, `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-registers.md`, `04-ppu-rendering.md`, `04-ppu-scrolling.md`, `04-ppu-sprites.md`, `04-ppu-timing.md` |
| `APU_registers` | 9 | `01-platform-overview.md`, `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-frame-counter.md`, `03-apu-mixer.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md` |
| `APU_Mixer` | 7 | `03-apu-dmc.md`, `03-apu-mixer.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md`, `05-boards-and-io.md` |
| `CPU_pinout` | 7 | `01-platform-overview.md`, `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-mixer.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md` |
| `Open_bus_behavior` | 7 | `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-overview.md`, `04-ppu-overview.md`, `04-ppu-registers.md`, `05-boards-and-io.md` |
| `PPU_OAM` | 7 | `01-platform-overview.md`, `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-overview.md`, `04-ppu-memory-map.md`, `04-ppu-registers.md`, `04-ppu-sprites.md` |
| `PPU_rendering` | 7 | `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-registers.md`, `04-ppu-rendering.md`, `04-ppu-scrolling.md`, `04-ppu-sprites.md`, `04-ppu-timing.md` |
| `PPU_variants` | 7 | `01-platform-overview.md`, `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-registers.md`, `04-ppu-sprites.md`, `04-ppu-timing.md`, `05-boards-and-io.md` |
| `APU_DMC` | 6 | `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-frame-counter.md`, `03-apu-mixer.md`, `03-apu-overview.md`, `05-boards-and-io.md` |
| `APU_Frame_Counter` | 6 | `01-platform-overview.md`, `03-apu-frame-counter.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md` |
| `APU_Length_Counter` | 6 | `03-apu-dmc.md`, `03-apu-frame-counter.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md`, `03-apu-triangle.md` |
| `CPU_variants` | 6 | `01-platform-overview.md`, `02-cpu-core.md`, `03-apu-dmc.md`, `03-apu-mixer.md`, `03-apu-noise.md`, `05-boards-and-io.md` |
| `PPU_power_up_state` | 6 | `01-platform-overview.md`, `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-registers.md`, `04-ppu-sprites.md`, `04-ppu-timing.md` |
| `CPU_memory_map` | 5 | `01-platform-overview.md`, `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-overview.md`, `05-boards-and-io.md` |
| `PPU_pinout` | 5 | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-overview.md`, `04-ppu-palette.md`, `04-ppu-timing.md` |
| `APU` | 4 | `02-cpu-dma.md`, `03-apu-dmc.md`, `03-apu-noise.md`, `03-apu-overview.md` |
| `APU_Envelope` | 4 | `03-apu-frame-counter.md`, `03-apu-noise.md`, `03-apu-overview.md`, `03-apu-pulse.md` |
| `APU_Triangle` | 4 | `03-apu-frame-counter.md`, `03-apu-mixer.md`, `03-apu-overview.md`, `03-apu-triangle.md` |
| `CPU_interrupts` | 4 | `01-platform-overview.md`, `02-cpu-core.md`, `02-cpu-instructions.md`, `03-apu-dmc.md` |
| `Errata` | 4 | `04-ppu-overview.md`, `04-ppu-registers.md`, `04-ppu-sprites.md`, `05-boards-and-io.md` |
| `Mirroring` | 4 | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-scrolling.md`, `05-boards-and-io.md` |
| `PPU_palettes` | 4 | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-palette.md`, `04-ppu-rendering.md` |
| `PPU_scrolling` | 4 | `04-ppu-memory-map.md`, `04-ppu-registers.md`, `04-ppu-rendering.md`, `04-ppu-scrolling.md` |
| `Standard_controller` | 4 | `01-platform-overview.md`, `02-cpu-dma.md`, `03-apu-overview.md`, `05-boards-and-io.md` |
| `APU_Noise` | 3 | `03-apu-mixer.md`, `03-apu-noise.md`, `03-apu-overview.md` |
| `APU_Pulse` | 3 | `03-apu-mixer.md`, `03-apu-overview.md`, `03-apu-pulse.md` |
| `APU_Sweep` | 3 | `03-apu-frame-counter.md`, `03-apu-overview.md`, `03-apu-pulse.md` |
| `CPU_power_up_state` | 3 | `01-platform-overview.md`, `02-cpu-core.md`, `03-apu-frame-counter.md` |
| `DMA` | 3 | `01-platform-overview.md`, `02-cpu-dma.md`, `03-apu-dmc.md` |
| `Expansion_port` | 3 | `01-platform-overview.md`, `03-apu-overview.md`, `05-boards-and-io.md` |
| `PPU_attribute_tables` | 3 | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-scrolling.md` |
| `PPU_frame_timing` | 3 | `01-platform-overview.md`, `04-ppu-rendering.md`, `04-ppu-timing.md` |
| `PPU_nametables` | 3 | `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-scrolling.md` |
| `PPU_pattern_tables` | 3 | `04-ppu-memory-map.md`, `04-ppu-rendering.md`, `04-ppu-sprites.md` |
| `Visual_circuit_tutorial` | 3 | `02-cpu-core.md`, `02-cpu-dma.md`, `02-cpu-instructions.md` |
| `Bus_conflict` | 2 | `02-cpu-dma.md`, `05-boards-and-io.md` |
| `Cartridge_connector` | 2 | `01-platform-overview.md`, `05-boards-and-io.md` |
| `CIC_lockout_chip` | 2 | `01-platform-overview.md`, `05-boards-and-io.md` |
| `Colour_emphasis` | 2 | `01-platform-overview.md`, `04-ppu-palette.md` |
| `Controller_reading` | 2 | `02-cpu-dma.md`, `05-boards-and-io.md` |
| `CPU_addressing_modes` | 2 | `02-cpu-core.md`, `02-cpu-instructions.md` |
| `CPU_registers` | 2 | `02-cpu-core.md`, `02-cpu-instructions.md` |
| `Famicom` | 2 | `01-platform-overview.md`, `05-boards-and-io.md` |
| `Mapper` | 2 | `01-platform-overview.md`, `05-boards-and-io.md` |
| `NES_reference_guide` | 2 | `01-platform-overview.md`, `05-boards-and-io.md` |
| `NTSC_video` | 2 | `04-ppu-palette.md`, `04-ppu-timing.md` |
| `PAL_video` | 2 | `04-ppu-palette.md`, `04-ppu-timing.md` |
| `PPU_memory_map` | 2 | `01-platform-overview.md`, `04-ppu-memory-map.md` |
| `PPU_sprite_priority` | 2 | `04-ppu-rendering.md`, `04-ppu-sprites.md` |
| `Status_flags` | 2 | `02-cpu-core.md`, `02-cpu-instructions.md` |

## Unification notes

### Rename applied to Tables 2 and 3 (now applied to the spec files)

Rename applied while building Tables 2 and 3 (`old` → `new`), and applied to
every spec file in the final consistency pass (Tables 2/3 and the spec
correspondence tables now cite only the canonical name):

| # | Old name (as written) | New (canonical) name | Spec files that wrote the old name |
|---|---|---|---|
| 1 | `Clock_rate` | `Cycle_reference_chart` | `02-cpu-core.md`, `02-cpu-dma.md`, `03-apu-mixer.md`, `03-apu-pulse.md`, `03-apu-triangle.md`, `04-ppu-overview.md`, `04-ppu-registers.md`, `04-ppu-timing.md`, `05-boards-and-io.md` |

`Clock_rate` is a wiki redirect; `Cycle_reference_chart` is its target (the
`Cycle reference chart` title), and 01-platform-overview, 03-apu-frame-counter,
03-apu-overview and 06-open-questions already cited, or now cite, the canonical
name. This is the only table-level variant found.

### Adjudicated names that need no table rename

The following wrong or redirect names occur only in spec prose (usually
`## Open questions`), not in any correspondence table, so Tables 2 and 3 needed
no change; a mechanical pass over the spec files should still fix them:

| Old name | New (canonical) name | Where it appears |
|---|---|---|
| `APU_Mixers` | `APU_Mixer` | `03-apu-overview.md` (open questions; every table already uses `APU_Mixer`) |
| `Visual_2A03` | `Visual_circuit_tutorial` | `02-cpu-core.md`, `02-cpu-dma.md` (open questions; every table already uses `Visual_circuit_tutorial`) |
| `Status_register` | `Status_flags` | `02-cpu-core.md` (prose; every table already uses `Status_flags`) |
| `Controller_ports` | `Standard_controller` + `Expansion_port` | `03-apu-overview.md` (open questions only) |
| `NES` (title) | `NES_reference_guide` | redirect — already recorded canonically inside the table |
| `Four_score` | `Four_player_adapters` | redirect — already recorded canonically inside the table |
| `Cycle reference chart` (with a space) | `Cycle_reference_chart` | `02-cpu-core.md` line 352 prose (the wiki title, not a page token) |

Two redirects are already recorded canonically inside the correspondence tables
themselves (`NES_reference_guide` for the `NES` title, `Four_player_adapters`
for `Four_score`), so they need no rename.
