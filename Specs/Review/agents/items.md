# Section briefs (issue #1392)

One brief per spec section. Compiler / Adapter / Verifier agents are given one
brief id and read the matching section here. Paths are relative to the
repository root `/mnt/c/Work/breaks` unless absolute.

Legend:
- **files** — deliverable path(s) under `Specs/`.
- **sources** — primary Breaking NES Wiki / Docs material (Russian original; the
  spec is written in English).
- **hdl** — die-perfect Verilog to confirm behaviour.
- **nesdev** — candidate NESdev Wiki page names for the Adapter.
- **focus** — the section's required technical content.

---

## platform

- files: `01-platform-overview.md`
- sources: `README.md`, `BreakingNESWiki/Readme.md`, `BreakingNESWiki/MOS.md`,
  `BreakingNESWiki/Ricoh.md`, `BreakingNESWiki/nmos.md`, `BreakingNESWiki/Dendy.md`,
  `BreakingNESWiki/MB/Readme.md`, `BreakingNESWiki/MB/NES.md`, `BreakingNESWiki/MB/Famicom.md`,
  `BreakingNESWiki/APU/Readme.md`, `BreakingNESWiki/PPU/Readme.md`,
  `BreakingNESWiki/6502/Readme.md`, `Docs/NES_Famicom_Board_Revisions.md`,
  `Docs/Famicom/Chipset/Readme.md`, `Docs/NES/NES_HDL_Model/Readme.md`
- hdl: (none)
- nesdev: `NES_reference_guide`, `CPU_memory_map`, `PPU_memory_map`, `Clock_rate`, `Glossary`, `PPU_pinout`
- focus: Platform-level specification of the NES/Famicom: the chipset (2A03/2A07
  CPU+APU, 2C02/2C07 PPU, SRAM, CIC, glue logic), NTSC vs PAL vs Dendy master
  clocks and CPU:PPU clock ratios, the three physical buses (CPU address/data,
  PPU address/data, cartridge), CPU↔PPU coupling through registers and NMI,
  reset/power-on overview, region/config variants (Famicom vs NES, VS. System,
  PlayChoice-10, Famiclones) at summary level, and an index table pointing to the
  other spec files. Include a complete CPU memory map table ($0000–$FFFF with
  mirrors) and a PPU memory map table ($0000–$3FFF), each row naming the region
  and access rules. This file is the entry point: it must let a reader navigate
  the rest of `Specs/`.

## cpu-core

- files: `02-cpu-core.md`
- sources: `BreakingNESWiki/6502/Readme.md`, `.../regs.md`, `.../pc.md`,
  `.../flags.md`, `.../interrupts.md`, `.../ir.md`, `.../whatwhen.md`,
  `.../busmux.md`, `.../address_bus.md`, `.../data_bus.md`, `.../pads.md`,
  `.../clock.md`, `.../context_control.md`
- hdl: `HDL/Core6502` (whole directory — open the sources listed there)
- nesdev: `CPU_registers`, `CPU_addressing_modes`, `Status_flags`,
  `CPU_interrupts`, `CPU_power_up_state`, `Visual_2A03`
- focus: The 2A03/2A07 CPU core as seen by a programmer: registers A/X/Y/S/P/PC,
  addressing modes, flags and their exact set/clear rules, stack behaviour, the
  PHI1 (address + R/W setup) / PHI2 (data) cycle structure, RDY, reset and
  interrupt vectors, power-on state, and the differences from a stock NMOS 6502
  (decimal mode present in silicon but disconnected in Ricoh parts; 2A07 vs
  2A03). Document internal buses (ADH/ADL, SB, DB) only where they explain
  observable bus behaviour (precharge to $FF, dummy reads).

## cpu-instructions

- files: `02-cpu-instructions.md`
- sources: `BreakingNESWiki/6502/decoder.md`, `.../predecode.md`,
  `.../random_logic.md`, `.../dispatch.md`, `.../alu.md`, `.../alu_control.md`,
  `.../flags_control.md`, `.../branch_logic.md`, `.../extra_counter.md`,
  `.../context_control.md`, `.../whatwhen.md`, `Docs/6502/6502 Instruction Set.htm`
- hdl: `HDL/Core6502`
- nesdev: `CPU_instructions`, `CPU_addressing_modes`, `CPU_registers`, `Visual_2A03`
- focus: The instruction set as an implementable table: every official opcode
  with mnemonic, addressing mode, byte length, base cycle count, and the
  extra-cycle rules (page crossing for absolute indexed read, branch taken/page
  crossing, RMW, stack ops). Include each addressing mode with a concrete operand
  example, the flag effects of ADC/SBC/CMP/CPX/CPY/BIT/PLP/PHP/RTI/BRK, the RMW
  read-modify-write bus sequence, the JMP ($xxFF) page-wrap bug if the sources
  support it, and interrupt/BRK sequence frames. Cover undocumented opcodes only
  if the sources support them; otherwise state they are out of scope and add to
  Open questions.

## cpu-dma

- files: `02-cpu-dma.md`
- sources: `BreakingNESWiki/APU/dma.md`, `.../dpcm.md`, `.../core.md`,
  `.../cycles.md`, `.../regs.md`, `.../pads.md`, `.../common.md`
- hdl: `HDL/APU/dma.v`, `HDL/APU/dmabuf.v`, `HDL/APU/core.v`, `HDL/APU/dpcm.v`
- nesdev: `DMA`, `APU_DMC`, `PPU_OAM`, `Open_bus_behavior`, `CPU_interrupts`
- focus: DMA as seen from the CPU: OAM DMA ($4014) length (513/514 CPU cycles),
  DMC sample DMA, the RDY halt handshake and its timing, dummy reads, bus
  conflicts, the DMA controller's internal address multiplexer (register select
  uses the DMA-side address, not CPU_A — this explains mid-DMA register
  behaviour), and the known conflicts: DMC DMA stealing the cycle of a $2007 read
  (extra increment/skipped byte), DMC DMA vs controller reads at $4016/$4017, and
  DMC DMA vs OAM DMA. Include a per-cycle timing table.

## apu-overview

- files: `03-apu-overview.md`
- sources: `BreakingNESWiki/APU/Readme.md`, `.../contents.md`, `.../regs.md`,
  `.../pads.md`, `.../common.md`, `.../core.md`, `.../clkgen.md`, `.../sound.md`,
  `.../waves.md`, `.../cycles.md`, `.../visual2a03.md`, `.../test_pattern.md`,
  `.../old_2a03.md`
- hdl: `HDL/APU/apu_top.v`, `HDL/APU/regs.v`, `HDL/APU/pads.v`, `HDL/APU/io.v`,
  `HDL/APU/common.v`
- nesdev: `APU`, `APU_registers`, `APU_Frame_Counter`, `APU_DMC`, `CPU_pinout`
- focus: APU overview and register interface: the block list (CPU core, clock
  generator, frame counter, pulse x2, triangle, noise, DMC, DAC, OAM DMA, I/O
  ports, debug/test logic), the complete $4000–$4017 register map with R/W
  direction, bit fields, mirroring and open-bus behaviour, the APU clocking
  hierarchy (CLK → PHI → ACLK1 / /ACLK2 → LFO1/LFO2) with NTSC/PAL periods, IRQ
  routing (frame counter + DMC through the daisy chain), the RDY/DMA interaction,
  the 2A03 debug/test registers ($4018–$401A) and what they expose, and 2A07
  differences (no debug logic).

## apu-pulse

- files: `03-apu-pulse.md`, `03-apu-triangle.md`
- sources: `BreakingNESWiki/APU/square.md`, `.../triangle.md`, `.../length.md`,
  `.../sound.md`, `.../common.md`, `.../waves.md`
- hdl: `HDL/APU/square.v`, `HDL/APU/triangle.v`, `HDL/APU/env.v`, `HDL/APU/length.v`
- nesdev: `APU_Pulse`, `APU_Triangle`, `APU_Length_Counter`, `APU_Envelope`,
  `APU_Sweep`, `APU_registers`
- focus: Pulse channels 1 and 2 and the triangle channel as implementable units.
  Pulse: timer period formula (CPU cycles per step = 2*(t+1)), 8-step duty
  sequences, envelope generator (divider, decay level, loop, constant volume),
  sweep unit (divider, target period formula, negate behaviour differing between
  pulse 1 and pulse 2, mute conditions), length counter (load table, halt,
  reload), $4000–$4007 bit fields. Triangle: linear counter (control flag, reload
  value, reload/halt, $400B write reload quirk), 32-step sequence, timer,
  240 Hz/120 Hz clocking, ultrasonic behaviour when the period is tiny, and the
  interaction between linear counter and length counter.

## apu-noise

- files: `03-apu-noise.md`, `03-apu-dmc.md`
- sources: `BreakingNESWiki/APU/noise.md`, `.../dpcm.md`, `.../dma.md`,
  `.../dac.md`, `.../length.md`, `.../common.md`, `.../waves.md`
- hdl: `HDL/APU/noise.v`, `HDL/APU/dpcm.v`, `HDL/APU/dac.v`, `HDL/APU/length.v`
- nesdev: `APU_Noise`, `APU_DMC`, `APU_Envelope`, `APU_Length_Counter`, `APU_registers`
- focus: Noise and DMC as implementable units. Noise: period table (NTSC and PAL,
  all 16 entries with concrete values), 15-bit LFSR tap configuration for mode 0
  (bit 1) and mode 1 (bit 6), output bit, envelope and length interaction,
  $400C/$400E/$400F bit fields. DMC: rate table (NTSC/PAL values), 7-bit output
  level, $4011 direct load and its masking/behaviour, sample address
  ($C000 + $40*addr) and length ($10*len+1) formulas, memory reader state machine,
  loop and IRQ enable/flag, sample buffer, and the interaction with CPU DMA
  ($4015 bit 4).

## apu-fc-mixer

- files: `03-apu-frame-counter.md`, `03-apu-mixer.md`
- sources: `BreakingNESWiki/APU/clkgen.md`, `.../waves.md`, `.../cycles.md`,
  `.../dac.md`, `.../pads.md`, `.../common.md`, `.../sound.md`
- hdl: `HDL/APU/clkgen.v`, `HDL/APU/dac.v`, `HDL/APU/apu_top.v`, `HDL/APU/pads.v`
- nesdev: `APU_Frame_Counter`, `APU_Mixers`, `APU_registers`, `APU_DMC`
- focus: Frame counter and mixer. Frame counter: 4-step and 5-step sequences with
  concrete NTSC and PAL CPU/APU cycle counts for each step, what each step clocks
  (envelope, linear counter, length counter, sweep), IRQ assert/clear timing, the
  $4017 write delay quirk (write takes effect after 3–4 CPU cycles; writing with
  the reset bit at certain alignments delays the IRQ by one step), and the frame
  IRQ flag in $4015. Mixer: the DAC structure, the non-linear mixing formula (or
  exact lookup behaviour) for pulse/triangle/noise/DMC, the output pins (AUX A /
  AUX B, /AUX), and 2A07 differences.

## ppu-overview-regs

- files: `04-ppu-overview.md`, `04-ppu-registers.md`
- sources: `BreakingNESWiki/PPU/Readme.md`, `.../regs.md`, `.../pads.md`,
  `.../fsm.md`, `.../mux.md`, `.../visual2c02.md`, `.../umc6538.md`, `.../rails.md`
- hdl: `HDL/PPU/ppu_top.v`, `HDL/PPU/regs.v`, `HDL/PPU/pads.v`, `HDL/PPU/fsm.v`
- nesdev: `PPU`, `PPU_registers`, `PPU_power_up_state`, `Open_bus_behavior`,
  `Visual_2C02`, `PPU_pinout`
- focus: PPU overview and the CPU-facing register file. Overview: block diagram,
  the internal buses DB / PD / OB (masters, users, and the capacitance/open-bus
  semantics of DB that must be modelled as a transparent latch), the CPU
  interface pins (RS0-2, /DBE, R/W), PPU-side register indices 0–7 vs the external
  $2000–$2007 mapping, the 2C05 identifier, and a revision overview table
  (2C02A..H, 2C03/2C04/2C05, 2C07, UMC 6538). Registers: each of
  $2000/$2001/$2002/$2003/$2004/$2005/$2006/$2007 with full bit-field diagrams,
  side effects (PPUSTATUS read clears w; OAMADDR write corrupts OAM on 2C02G;
  $2004 readability by revision), the shared w latch between $2005 and $2006, and
  the internal v/t/x registers with their relationship to $2005/$2006 writes.

## ppu-memmap

- files: `04-ppu-memory-map.md`
- sources: `BreakingNESWiki/PPU/vram_ctrl.md`, `.../oam.md`, `.../cram.md`,
  `.../par.md`, `.../scroll_regs.md`, `.../pamux.md`, `Docs/PPU/OAM_RowCol.txt`,
  `Docs/PPU/USPatent/OAM_FIFO.txt`
- hdl: `HDL/PPU/vram_ctrl.v`, `HDL/PPU/oam.v`, `HDL/PPU/cram.v`, `HDL/PPU/pamux.v`
- nesdev: `PPU_memory_map`, `PPU_nametables`, `PPU_attribute_tables`,
  `PPU_palettes`, `PPU_pattern_tables`, `Mirroring`, `PPU_OAM`
- focus: The 14-bit PPU address space as seen through $2006/$2007 and by the
  rendering pipeline: pattern tables $0000–$1FFF (CHR read/write rules plus the
  fact that many cartridges only decode reads), nametables $2000–$2FFF with the
  four arrangement modes (horizontal, vertical, four-screen, one-screen) and
  CIRAM A10 selection, attribute table layout (2 bits per 16x16 pixel area,
  quadrant mapping, last row/column attribute quirk), palette RAM $3F00–$3FFF
  with its 32 entries, the $3F10/$3F14/$3F18/$3F1C mirrors and the backdrop
  mirror, PPUDATA read buffer behaviour (one-byte delay, palette reads bypassing
  the buffer), OAM organization (256 bytes, 4 bytes per sprite, byte meanings,
  row/column layout from `Docs/PPU/OAM_RowCol.txt`) and the VRAM controller
  signals. Also give the table of PPU address bit fields (coarse/fine X and Y,
  nametable bits).

## ppu-render-sprites

- files: `04-ppu-rendering.md`, `04-ppu-sprites.md`
- sources: `BreakingNESWiki/PPU/dataread.md`, `.../bgcol.md`, `.../fifo.md`,
  `.../mux.md`, `.../fsm.md`, `.../tilecnt.md`, `.../pamux.md`, `.../obj_eval.md`,
  `.../oam.md`, `.../hv.md`, `.../hv_decoder.md`, `Docs/PPU/OAM_RowCol.txt`
- hdl: `HDL/PPU/fsm.v`, `HDL/PPU/bgcol.v`, `HDL/PPU/fifo.v`,
  `HDL/PPU/mux.v`, `HDL/PPU/obj_eval.v`, `HDL/PPU/oam.v`, `HDL/PPU/hv.v`,
  `HDL/PPU/hv_decoder.v`, `HDL/PPU/tilecnt.v`, `HDL/PPU/par.v`, `HDL/PPU/pamux.v`,
  `HDL/PPU/scroll_regs.v` — note: there is **no** `HDL/PPU/dataread.v` in this
  checkout; the Data Reader logic lives in `par.v` / `tilecnt.v` / `bgcol.v` /
  `pamux.v` / `scroll_regs.v`
- nesdev: `PPU_rendering`, `PPU_sprite_evaluation`, `PPU_OAM`,
  `PPU_sprite_priority`, `PPU_pattern_tables`, `PPU_scrolling`
- focus: Pixel pipeline and sprites. Rendering: the per-dot schedule as a table
  (which dots fetch nametable/attribute/pattern-low/pattern-high, the 2-tile
  prefetch at 321–336, the dummy fetches at 337–340, the first dot of a scanline,
  idle dots 257–320 and what happens at 256/257/328/336), the background shift
  registers and fine-X mux, per-pixel priority resolution between background and
  sprites (priority bits, behind-background, backdrop), rendering enable/disable,
  forced blank, the odd-frame skipped dot, and left-column clipping. Sprites: OAM
  layout, the sprite evaluation algorithm (scanline comparison, 8-sprite limit,
  secondary OAM), the exact sprite overflow bug algorithm (incrementing n/m, the
  false positives/negatives), sprite 0 hit conditions and exclusions, sprite
  priority ordering, 8x16 sprite layout, and the sprite pipeline/shifters.

## ppu-scroll

- files: `04-ppu-scrolling.md`
- sources: `BreakingNESWiki/PPU/scroll_regs.md`, `.../par.md`, `.../tilecnt.md`,
  `.../pamux.md`, `.../hv.md`, `.../fsm.md`, `.../regs.md`, `.../dataread.md`
- hdl: `HDL/PPU/scroll_regs.v`, `HDL/PPU/par.v`, `HDL/PPU/tilecnt.v`,
  `HDL/PPU/pamux.v`, `HDL/PPU/hv.v`, `HDL/PPU/fsm.v`
- nesdev: `PPU_scrolling`, `PPU_registers`, `PPU_rendering`, `PPU_nametables`
- focus: Scrolling as an implementable model: the v/t/x/w internal registers with
  bit layouts, how the Breaking NES Wiki paired registers (PAR, FV/FH/TV/TH, tile
  counters) map onto them, the $2005 first/second write and $2006 high/low write
  sequences, the exact increment schedule (coarse X increment at each tile fetch,
  Y increment at dot 256, horizontal/vertical copy at dot 257, X/Y reset at dot
  280, Y increment during 321–336), vertical scroll applying on the next frame
  when written during rendering, mid-frame split techniques, the $2006
  second-write AND conflict, the dot-257 PPUCTRL bit 0 race, and the effects of
  $2007 reads/writes during rendering on the scroll registers.

## ppu-palette-timing

- files: `04-ppu-palette.md`, `04-ppu-timing.md`
- sources: `BreakingNESWiki/PPU/cram.md`, `.../pal.md`, `.../rgb.md`, `.../tv.md`,
  `.../video_out.md`, `.../pclk.md`, `.../hv.md`, `.../hv_decoder.md`,
  `.../fsm.md`, `Docs/PPU/VideoSignal/composite_signal.txt`,
  `Docs/PPU/VideoSignal/colorburst.txt`, `Docs/PPU/VideoSignal/SMPTE_bars.txt`,
  `Docs/PPU/VideoSignal/VDAC.TXT`
- hdl: `HDL/PPU/cram.v`, `HDL/PPU/video_out.v`, `HDL/PPU/dac.v`,
  `HDL/PPU/pclk.v`, `HDL/PPU/hv.v`
- nesdev: `PPU_palettes`, `PPU_rendering`, `NTSC_video`, `PAL_video`,
  `PPU_power_up_state`, `Colour_emphasis`, `PPU_pinout`
- focus: Colour generation and frame timing. Palette: the 32-entry palette RAM
  (16 background + 16 sprite palettes), palette index → colour path (emphasis
  bits, greyscale AND with $30, the 6-bit output), NTSC colour generation
  (12 hues x 4 levels + greys/blacks, the phase relationship to the colour burst,
  the role of PCLK and the phase generator), PAL colour generation, RGB PPU
  differences, and the $3F00 backdrop rules. Timing: master clock and PCLK
  divider, scanline structure (341 dots) with a per-region frame table (visible
  240, vblank 20/70, pre-render, total 262 NTSC / 312 PAL scanlines, odd frame -1
  dot), Dendy timing, CPU:PPU phase alignment (3:1 NTSC and 16:5 PAL, and the
  resulting 341/3 phases), and PPU power-on/reset state.

## boards-io

- files: `05-boards-and-io.md`
- sources: `BreakingNESWiki/MB/Readme.md`, `.../NES.md`, `.../Famicom.md`,
  `BreakingNESWiki/Dendy.md`, `BreakingNESWiki/APU/pads.md`,
  `BreakingNESWiki/PPU/pads.md`, `Docs/NES_Famicom_Board_Revisions.md`,
  `Docs/Famicom/Chipset/Readme.md`, `Docs/Famiclones/ChipFeatures.md`,
  `Docs/Famiclones/Readme.md`, `Docs/NES/NES_HDL_Model/Readme.md`,
  `Docs/PlayChoice-10/Readme.md`, `Docs/VS_UniSystem/Readme.md`, `Docs/JIO/Readme.md`
- hdl: (none)
- nesdev: `Cartridge_connector`, `Mapper`, `CIC_lockout_chip`,
  `Standard_controller`, `Expansion_port`, `PPU_pinout`, `CPU_pinout`
- focus: Board/system level: the CPU and PPU bus connections to the cartridge
  slot (PRG data bus, CHR bus, /ROMSEL, CIRAM A10, PPU /RD and /WR, M2, IRQ, NMI),
  the CIC lockout chip and reset behaviour, the standard controller protocol on
  $4016/$4017 (strobe, 8-bit shift order, the DMC-DMA first-read corruption and
  the workaround), the expansion port, Famicom vs NES differences (CIC, controller
  sockets, audio on the cartridge edge, RF), region configurations (NTSC/PAL
  crystal and dividers) and the mappers landscape at summary level with mapper
  numbers and a pointer to NESdev. Include a summary motherboard revision table
  from `Docs/NES_Famicom_Board_Revisions.md` (summarise and link; do not duplicate
  the whole document).
