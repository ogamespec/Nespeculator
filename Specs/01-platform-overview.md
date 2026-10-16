# 01 Platform — NES / Famicom system overview

**Status:** consensus (round 3)
**Scope:** Platform-level description of the Nintendo Entertainment System and the
Family Computer: chipset inventory, master clocks and CPU:PPU clock ratios, the
three physical buses, the complete CPU (`$0000–$FFFF`) and PPU (`$0000–$3FFF`)
memory maps, CPU↔PPU / CPU↔APU coupling, reset and power-on, and the region and
configuration variants. Instruction, APU-channel and PPU-pipeline detail is
**not** covered here; it is referenced to the other files of `Specs/`.
**Primary sources:** `BreakingNESWiki/`: `Readme.md`, `MOS.md`, `Ricoh.md`,
`nmos.md`, `Dendy.md`, `MB/{Readme,NES,Famicom}.md`, `6502/Readme.md`,
`APU/{Readme,pads,regs,core,clkgen,old_2a03,pal}.md`,
`PPU/{Readme,pads,regs,pclk,hv,hv_decoder,fsm,pal,cram,vram_ctrl,par,umc6538}.md`;
`Docs/`: `NES_Famicom_Board_Revisions.md`, `Famicom/Chipset/Readme.md`,
`Famiclones/{ChipFeatures,Readme}.md`, `NES/NES_HDL_Model/{Readme.md,nes.v}`,
`UMC6527P/div_umc.v`;
HDL: `HDL/APU/{core,clkgen,regs,dma,pads}.v`,
`HDL/PPU/{pclk,vram_ctrl,cram,regs,pads,hv,hv_decoder}.v`,
`HDL/Common/bustris.v`.

## 1. Platform at a glance

The console is two large custom chips plus discrete memory and glue logic. The
Game Pak supplies the program and, in most configurations, the graphics.

| Part | Device | Role |
|---|---|---|
| CPU (the Breaking NES Wiki calls this chip the **APU**) | Ricoh RP2A03 (NTSC) / RP2A07 (PAL) | MOS 6502 core with decimal correction disconnected, plus the APU: clock divider, frame counter (SoftCLK), pulse 1/2 (Square 0/1), triangle, noise, DMC (DPCM), DAC, OAM DMA and I/O ports |
| PPU | Ricoh RP2C02 (NTSC) / RP2C07 (PAL); RGB RP2C03/2C04/2C05 for arcade hardware | 14-bit VRAM interface, background/sprite pipeline, palette RAM (CRAM), composite video output |
| Internal RAM (the board documentation calls the U1 chip **WRAM**) | 2 KB×8 SRAM (U1) | CPU work RAM, `$0000–$07FF` |
| VRAM / CIRAM | 2 KB×8 SRAM (U4) | PPU nametables, `$2000–$2FFF`; selected by the cartridge's `CIRAM /CE` (NESdev aliases: `VRAM /CS`, board net `/VRAM CE`) |
| CIC lockout (10NES) | Sharp SM590-family 4-bit MCU: 3193/3193A (NTSC), 3195A / 3197A / 3196A (PAL), 6113 family | Cartridge authentication; absent on the Famicom and on the NES-101 top-loader |
| Glue logic | 74LS139 dual 2-to-4 decoder, 74LS373 8-bit transparent latch, 40H368/74HC368 inverting tristate buffers | CPU address decode, PPU address latch, controller/expansion buffers; one 368 tristate is the audio amplifier and one inverts PA13. Late boards integrate the 139, the 368s and the pull-up pack into the **JIO** chip (BU3266/BU3270) |
| Clock | one crystal | NTSC 21.477272 MHz, PAL 26.601712 MHz. The CIC runs from a separate ≈4 MHz clock |
| Video/audio out | | Composite video from the PPU `VOUT` pin; audio is the sum of the CPU `AUX A`/`AUX B` DAC outputs and the cartridge audio input |

## 2. Master clocks and CPU:PPU ratio

Both the CPU and the PPU divide the same board clock, but by different
coefficients and independently of each other.

| Region | Chipset | Master clock | CPU divider | CPU `PHI0` | PPU divider | PPU dot (PCLK) | CPU:PPU |
|---|---|---|---|---|---|---|---|
| NTSC (Famicom/NES) | 2A03 + 2C02 | 21.477272 MHz | ÷12 | 1.7897727 MHz (558.73 ns) | ÷4 | 5.369318 MHz (186.24 ns) | 3:1 |
| PAL (European NES) | 2A07 + 2C07 | 26.601712 MHz | ÷16 | 1.6626070 MHz (601.46 ns) | ÷5 | 5.3203424 MHz (187.96 ns) | 16:5 |
| Dendy / PAL famiclones | UMC 6527P + 6538 (or NOAC) | like PAL (26.601712 MHz) | ÷15 | 1.773448 MHz (563.9 ns) | ÷5 | 5.3203424 MHz (187.96 ns) | 3:1 |

The `CPU:PPU` column is the CPU:PPU divider ratio (`12:4`, `16:5`, `15:5`), numerically the
**PPU dots per CPU cycle**: `3:1` = 3 dots per CPU cycle, `16:5` = 16 dots per 5 CPU cycles
= 3.2 (NESdev `Cycle_reference_chart`, "PPU dots per CPU cycle 3.2"; PAL `341 × 5÷16` =
106 9⁄16 CPU cycles per scanline), so the PAL cell is `16:5`, **not** `5:16`.
`05-boards-and-io.md` §1 carries the same table (its only local cell is a
`*(NESdev)*` marker on the Dendy row); the two files are edited together.

- Dendy figures are NESdev rank-4: `Cycle_reference_chart` gives the CPU as UMC
  `UA6527P`, `26.60~ MHz ÷ 15 = 1.773448 MHz` (113⅔ CPU cycles per scanline), and
  `CPU_variants` records the same ÷15 divider (`÷16` on runs whose date stamp has
  no `-`). With the 6538's ÷5 dot clock the ratio is **exactly 3 PPU dots per CPU
  cycle** — PAL video, NTSC-like 3:1 CPU ratio. No rank 1–3 source here states
  the 6527P divider; rank 1–3 `PPU/umc6538.md` covers only the 6538.
- APU clock `ACLK` = `PHI0` ÷ 2: 894.886 kHz NTSC, 831.303 kHz PAL; `ACLK1` and
  `/ACLK2` are non-overlapping (`APU/clkgen.md`). The CPU core uses both
  halftimes of `PHI0`: `PHI1` = "set address and R/W", `PHI2` = "read/write
  data" (`6502/Readme.md`). The external `M2` pin is a modified `PHI2` that goes
  high 1.5 CLK periods before the core's `PHI2`, so external devices can
  drive/undrive the bus early (`APU/pads.md`).
- Consequence of the ratio: one CPU cycle is exactly 3 PPU dots on NTSC and
  3.2 PPU dots on PAL, i.e. 341 ÷ 3 = 113.67 CPU cycles per scanline NTSC and
  341 ÷ 3.2 = 106.5625 CPU cycles per scanline PAL. Dendy's PAL frame is
  312 × 341 = 106,392 dots ÷ 3 = 35,464 CPU cycles. `04-ppu-timing.md` §10
  matches (`1.773448 MHz`, `3 : 1`).
- Frame budget (derived): 262 × 341 = 89,342 PPU dots NTSC and 312 × 341 =
  106,392 PAL per frame, i.e. ≈60.099 Hz NTSC and ≈50.007 Hz PAL. The NTSC PPU
  skips one dot on alternate frames (the odd-frame skipped dot; the wiki's
  EVEN/ODD "NTSC Crawl", `PPU/fsm.md`); the PAL PPU has no such logic
  (`PPU/pal.md`).

## 3. Physical buses

| Bus | Width | Direction | Connected to | Notes |
|---|---|---|---|---|
| CPU address `ADDR` | 16 | CPU → system | Internal RAM, PPU registers, APU, cartridge | One-way, unbuffered at the pads (`6502/Readme.md`, `APU/pads.md`); the cartridge edge sees A0–A14 only, A15 is decoded on the motherboard |
| CPU data `DATA` | 8 | bidirectional | CPU, internal RAM, PPU registers, APU, cartridge | Connected directly to the core (no buffer). During `PHI2` the core precharges its internal buses to `$FF` (`6502/Readme.md`) |
| PPU address/data | 14 address / 8 data | PPU ↔ VRAM and cartridge | `AD0–AD7` (multiplexed), `A8–A13`, `ALE`, `/RD`, `/WR` | `ALE=1` drives the low address byte on `AD0–AD7`; the board latches it into the 74LS373 to form VRAM `A0–A7`; `ALE=0` turns `AD0–AD7` into the data byte (`PPU/pads.md`, `Docs/Famicom/Chipset/Readme.md`) |
| Cartridge (Game Pak) | | both chips | `CPU_D`, `CPU_A0–A14`, `R/W`, `M2`, `/ROMSEL`, `/IRQ`; `PPU_D`, `PPU_A0–A13` + inverted PA13, `/RD`, `/WR`; CIRAM A10, CIRAM `/CE`; CIC clock/data; audio in | The cartridge generates CIRAM A10 and CIRAM `/CE`, i.e. it decides nametable mirroring and whether the PPU sees the board VRAM at all (`Docs/NES/NES_HDL_Model/nes.v`) |
| APU I/O and PPU EXT | 5 (3 outputs `OUT0–OUT2`, 2 read strobes `/IN0`, `/IN1`); 4 | APU → board; PPU ↔ PPU | `OUT0–OUT2`, `/IN0`, `/IN1`; `EXT0–EXT3` | All five APU pins are driven by the APU (`APU =>` in `APU/pads.md`), not inputs to it; the `/IN0`/`/IN1` strobes read back `$4016`/`$4017`. EXT is multi-PPU video mixing, direction set by `$2000` bit 6 (`PPU/pads.md`) |

The PPU does **not** drive CIRAM A10: the cartridge returns the bank select.

## 4. CPU memory map (`$0000–$FFFF`)

Decoded on the motherboard by the 74LS139 pair. The first half is gated by `M2`,
so on-board devices respond only during `PHI2` (`nes.v`).

| Address | Size | Device | Region | Access rules |
|---|---|---|---|---|
| `$0000–$07FF` | 2 KB | Internal RAM (U1; board docs: WRAM) | all | Read/write; RAM address inputs are A0–A10, so A11–A15 are not decoded |
| `$0800–$1FFF` | 6 KB | Internal RAM mirrors | all | Mirrors of `$0000–$07FF` (three repeats) |
| `$2000–$2007` | 8 B | PPU registers | all | The PPU receives only `RS0–RS2` = A0–A2 plus `R/W` and `/DBE`; which register is readable/writable is per register (see `04-ppu-registers.md`) |
| `$2008–$3FFF` | 8,184 B | PPU register mirrors | all | Mirror every 8 bytes because the PPU decodes only A0–A2 |
| `$4000–$4013` | 20 B | APU channel registers | all | Write-only (pulse 1/2, triangle, noise, DMC) |
| `$4014` | 1 B | OAM DMA trigger | all | Write-only; starts a 256-byte copy into PPU `$2004` |
| `$4015` | 1 B | APU status / channel enable | all | Read (status) and write (enable) |
| `$4016`, `$4017` | 2 B | I/O ports (controllers, expansion) | all | Read (pad data) and write (strobe / frame counter) |
| `$4018–$401F` | 8 B | 2A03 test registers / unmapped | NTSC 2A03, only with `DBG=1` for `$4018–$401A` | Debug reads of the sound generators (`$4018`/`$4019`), the `$401A` write bit 7 (`LOCK`, which freezes the sound generators for read-back); `$401B–$401F` open bus; the APU decodes only `$4000–$401F` |
| `$4020–$5FFF` | 8,160 B | cartridge / expansion | all | Not decoded on the motherboard; mapper or expansion hardware may respond |
| `$6000–$7FFF` | 8 KB | cartridge PRG-RAM (NESdev: WRAM) / expansion | all | Not decoded on the motherboard; the cartridge supplies `/CE` |
| `$8000–$FFFF` | 32 KB | cartridge PRG ROM (+ mapper registers) | all | `/ROMSEL` is active when A15 = 1 and M2 = 1; reset/IRQ/NMI vectors are fetched from the top of this range (NESdev: `$FFFA/$FFFC/$FFFE`) |

NESdev groups `$4000–$4017` into a single *APU and I/O registers* block and
`$4018–$401F` as normally disabled APU and I/O functionality (the page's *CPU
test mode*); the rows above split them because their directions and side effects
differ.

Two decode rules matter to an emulator: the APU selects its **address space**
from the CPU address bus but the **individual register** from the DMA
controller's address multiplexer (A0–A4), not `CPU_A` (`APU/regs.md`); and
`$4018–$401A` exist only in the 2A03, while the 2A07 has no debug logic and uses
`DBG` as an extra `RDY` source (`APU/pal.md`, `APU/core.md`).

## 5. PPU memory map (`$0000–$3FFF`)

The PPU address space is 14 bits. It is reached by the CPU only through
`$2006`/`$2007` and by the rendering pipeline through its own VRAM controller
(`PPU/vram_ctrl.md`). This table is the entry-point summary; the detailed map,
bit fields and mirroring rules are in `04-ppu-memory-map.md`.

| Address | Size | Contents | Access rules |
|---|---|---|---|
| `$0000–$0FFF` | 4 KB | Pattern table 0 (CHR) | Supplied by the cartridge (CHR-ROM or CHR-RAM); the motherboard has no CHR memory. `$2000` bit 4 selects this table for backgrounds, bit 3 for 8×8 sprites |
| `$1000–$1FFF` | 4 KB | Pattern table 1 (CHR) | Same; 8×16 sprites use bit 0 of the tile index to select the table |
| `$2000–$23FF`, `$2400–$27FF`, `$2800–$2BFF`, `$2C00–$2FFF` | 4 KB | Nametables 0–3 (1 KB each: a 960-byte tile-index field plus a 64-byte attribute table) | The board provides 2 KB VRAM (U4, CIRAM) for the four slots; the cartridge's `CIRAM /CE` enables it and `CIRAM A10` selects which 1 KB bank each slot sees |
| `$3000–$3EFF` | 3.75 KB | Unused; on the standard board a mirror of `$2000–$2EFF` | NESdev; the board decode that ignores PPU A12 is not documented in this repository's rank 1–3 sources (see `## Open questions`). The cartridge mirroring logic uses PPU A10/A11 |
| `$3F00–$3F1F` | 32 B | Palette RAM (CRAM), 32 entries | Read/write through `$2007`; each entry is 6 bits. 16 background + 16 sprite entries, index bit 4 selecting the background/sprite half. Reads are revision-dependent and return PPU open bus in bits 7–6 (`04-ppu-registers.md` §9) |
| `$3F10/$3F14/$3F18/$3F1C` | 4 B | Mirrors of `$3F00/$3F04/$3F08/$3F0C` | Hardware cause: in the CRAM cell array the row is `{CGA4,CGA1,CGA0}` and **rows 0 and 4 share the same word** (`PPU/cram.md`), so the background "backdrop" entry is aliased by the first sprite entry |
| `$3F20–$3FFF` | 224 B | Mirror of `$3F00–$3F1F` | NESdev; the PPU's palette decoder uses only `CGA0–CGA4` (`HDL/PPU/cram.v`, `04-ppu-memory-map.md` §1) |

The PPU's CPU interface is independent of PCLK: a write can land at any dot and
the pipeline observes it at its own phase (`PPU/regs.md`), enabling mid-frame
register splits.

## 6. CPU ↔ PPU and CPU ↔ APU coupling

There is no handshake between the chips; all coupling is through registers and a
few dedicated signals (NMI, IRQ, RDY).

| Path | Mechanism | Notes |
|---|---|---|
| CPU → PPU registers | Writes/reads in `$2000–$2007` | The PPU sees only the register index A0–A2; the `$2000` mapping is made by the board |
| PPU → CPU NMI | PPU `/INT` (vblank) is wired to the CPU `/NMI` | NMI is falling-edge sensitive (`6502/Readme.md`); the vblank flag is `$2002` bit 7 and reading `$2002` clears both the flag and the `$2005/$2006` write toggle (`PPU/fsm.md`, `PPU/regs.md`) |
| CPU → OAM | `$4014` write starts the APU's DMA controller, which copies 256 bytes from CPU memory into PPU `$2004` | The APU halts the 6502 core with `RDY` for the duration (`APU/Readme.md`, `APU/core.md`); this is the normal bulk sprite upload path (`$2003`/`$2004` writes are the alternative) |
| APU → CPU (DMA and IRQ) | DMC sample fetch uses the same DMA machinery and also raises `RDY`; frame-counter (SoftCLK) and DMC IRQs are daisy-chained into the core `/IRQ` | The external `/IRQ` pin is combined with the internal one (`APU/core.md`, `APU/clkgen.md`); see `02-cpu-dma.md` |
| Controllers | `$4016`/`$4017` are APU I/O ports | `OUT0` is the strobe, `/IN0` and `/IN1` return the pad bits (`APU/pads.md`) |
| CPU ↔ cartridge | `/ROMSEL` + M2 selects PRG ROM; mapper registers sit in the same range | |
| PPU ↔ cartridge | PPU drives A0–A13 + `/RD`/`/WR`/`ALE`; cartridge returns CHR data, CIRAM A10 and CIRAM `/CE` | |

## 7. Reset and power-on

| Block | Behaviour |
|---|---|
| CPU | `/RES` must be held low for several cycles after power-on before the core starts (`6502/Readme.md`); the core then fetches its reset vector from the cartridge ROM. The `RES` signal also forces the external `R/W` pin to high impedance |
| APU pads | On reset (`RES=1`) the address pads A0–A15 are disconnected (`z`), and in retail parts `M2` is `z` as well (open-drain); `R/W` is `z` (`APU/pads.md`) |
| APU state | The letterless RP2A03 does not reload the APU frame counter (the wiki's LFO) on reset and does not reset the ACLK divider or the OAM-DMA start flip-flop (`APU/old_2a03.md`) |
| PPU | The `/RES` latch is fully asynchronous and independent of CLK; it holds the reset request until `RESCL` clears it, and it drives the register-clear (`RC`) logic (`PPU/pads.md`). At power-on the latch itself takes an undefined value (`x`) |
| PPU counters | The H/V counters and the PCLK divider latch state at power-on are logically undefined; the wiki can only predict one assumed pattern if `CLK=0` and floating gates are read as 0 (`PPU/pclk.md`) |
| Memory | Internal RAM and VRAM contents at power-on are undefined; the cartridge ROM is the only defined memory |
| Reset source | On the NES the CIC holds the CPU/PPU in reset (`nHOST_RST` → `/RES`) until the cartridge's CIC completes the key exchange; on failure the console resets forever (`MB/NES.md`, `nes.v`). The Famicom has no CIC: the RESET button pulls `/RES` to 0 with an RC-defined low period, and pull-up pack RM1 holds `/IRQ` and `/NMI` at 1 when undriven (`MB/Famicom.md`) |

## 8. Region and configuration variants

| Configuration | CPU / PPU | Notes |
|---|---|---|
| Famicom (HVC-001) | 2A03 / 2C02 | 60-pin top-loading cartridge slot, hardwired controllers, microphone in controller 2, cartridge audio mixed on the board, no CIC |
| NES (NES-001) | 2A03 / 2C02 | 72-pin front-loading slot, detachable controllers, CIC lockout present, expansion port at the bottom also routed to cartridge pins |
| PAL NES (NESE-001 / NESA-001) | 2A07 / 2C07 | PAL-B 3195A (Europe/Spain/France), PAL-A 3197A (Mattel UK/AUS/NZ/ITA), 3196A (HK/Asian/Indian; Indian units branded "Samurai"). The French unit has no RF modulator and carries RGB via a proprietary connector |
| Korean ("Comboy") / Brazilian NES | 2A03 / 2C02 | Korean NTSC console shipped with the PAL-B 3195A lockout; the Brazilian unit is PAL-M with colour-conversion circuitry because the 2C02 outputs NTSC colour (6113B1) |
| Vs. System / PlayChoice-10 / Titler / FamicomBox | 2A03 (letterless) + RGB PPU 2C03/2C04/2C05, 2C05-99, CIC 3198–3199 | Arcade and special-purpose hardware; the Vs. System uses an RP2A04 pin-compatible "CPU" jumper on some boards, and letterless CPUs lack looped noise |
| Dendy & famiclones | UMC 6527P (2A03-class) + 6538 (2C07-class), later UM6561 NOAC | Pin-compatible clone chips. Dendy runs the CPU near the NTSC rate with PAL video timing (`Dendy.md`), so PAL software runs slightly fast; the 6538 delays `/VSET` by 50 scanlines (`PPU/umc6538.md`) |

The Famicom, NES and famiclone cartridge slots and mappers are described at board
level in `05-boards-and-io.md`; mapper behaviour itself is out of scope for
`Specs/` and is covered by NESdev (see the correspondence table).

## 9. CPU and PPU revision summary

| Marking | Era | Distinguishing behaviour |
|---|---|---|
| RP2A03 (letterless) | 1983–1984 | M2 duty cycle 17/24 (NESdev + `Docs/NES_Famicom_Board_Revisions.md`; the Wiki's `APU/old_2a03.md` says 15/24 — see `## Open questions`); no looped/tone noise mode; noise period table shorter; the APU frame counter (LFO) is not restarted by reset; no debug registers; pin 30 unused |
| RP2A03E | 1984–1986 | "Fixed" CPU that shipped with the round-button Famicom; pin 30 is `/RDY` |
| RP2A03G / H | 1987–2002 | Reference part; pin 30 enables the CPU test mode (debug registers `$4018–$401A`); late G production has a DMC DMA bug; H has no known difference from late G |
| RP2A07 / RP2A07A | 1987–1992 | PAL: ÷16 CPU divider, different DMC/noise/frame-counter tables, M2 duty 19/32, no debug logic |
| RP2C02 (letterless/A) | 1983 | Earliest NTSC PPUs. A (and B) can emit erroneous sprite pixels at X=255; on A, PPUMASK/PPUCTRL appear asynchronous |
| RP2C02B / C (`RC2C02C`) | 1983–1984 | B shares the X=255 sprite bug with A; C briefly replaced B, then B resumed; C also exists in a ceramic package |
| RP2C02D, RP2C02D-0, RP2C02E | 1984–1985 | No documented functional difference in the revision table; they precede E-0 and therefore share its unreadable OAMDATA/palette RAM |
| RP2C02E-0 | 1985–1987 | **In this and all previous revisions** OAMDATA (`$2004`) and palette RAM are **not readable** — i.e. letterless, A, B, C, D, D-0 and E too (`Docs/NES_Famicom_Board_Revisions.md` §4, NESdev `PPU_variants`); several sprite-evaluation bugs |
| RP2C02G-0 / H-0 | 1987–2003 | G: writes to OAMADDR (`$2003`) corrupt OAM; susceptible to signal reflections, fixed with series resistors on cartridge lines. H fixes some G glitches |
| RP2C07 / RP2C07-0 / RP2C07A-0 | 1985–1992 | PAL: 71-scanline vblank, sprite evaluation can never be fully disabled, red/green colour-emphasis bits swapped |

## 10. Motherboard revision summary

Condensed from `Docs/NES_Famicom_Board_Revisions.md` (which carries the full
tables and sources).

| Family | Boards | Summary |
|---|---|---|
| Famicom | HVC-CPU-01 … -08, HVC-CPU-GPM-01, GPM-02a…k | 1983 launch boards recalled; -05 became the replacement; -07 the most common; GPM (1988–1993) integrated the RF module and added shielding |
| NES (front-loader and top-loader) | NES-CPU-01 … -11; NESN-CPU-01, -JIO-01, -JIO-02 (PAL), -AV-01 | -01…-03 test market; -05 switched to G revisions; -09…-11 added CIC STUN / −5 V countermeasures. The 1993 top-loader has no CIC and no expansion port; the JIO boards replace the discrete glue logic and fix the common board's jailbar video |
| AV Famicom | HVCN-CPU-01, HVCN-CPU-02 | 1993–2003; JIO chip, 2A03H/2C02H, no CIC |

## 11. Index of the specification

| File | Contents |
|---|---|
| `01-platform-overview.md` | This file: chipset, clocks, buses, memory maps, reset, variants |
| `02-cpu-core.md` | 2A03/2A07 programmer model: registers, addressing, flags, stack, PHI1/PHI2, reset and interrupt vectors |
| `02-cpu-instructions.md` | Instruction set: opcodes, cycle costs, page-crossing rules |
| `02-cpu-dma.md` | OAM DMA, DMC DMA, `RDY` halting, bus conflicts |
| `03-apu-overview.md` | APU blocks, clocks and the `$4000–$401F` register map |
| `03-apu-pulse.md`, `03-apu-triangle.md` | Pulse 1/2 and triangle channels |
| `03-apu-noise.md`, `03-apu-dmc.md` | Noise and DMC channels |
| `03-apu-frame-counter.md`, `03-apu-mixer.md` | Frame counter and DAC/mixer |
| `04-ppu-overview.md`, `04-ppu-registers.md` | PPU architecture and the CPU-facing registers |
| `04-ppu-memory-map.md` | VRAM space, nametables, mirroring, CHR, palettes |
| `04-ppu-rendering.md`, `04-ppu-sprites.md` | Per-dot pipeline, OAM, sprite evaluation |
| `04-ppu-scrolling.md` | Scroll registers and mid-frame splits |
| `04-ppu-palette.md`, `04-ppu-timing.md` | Colour generation, frame timing, power-on state |
| `05-boards-and-io.md` | Motherboards, cartridge interface, controllers, CIC |
| `06-open-questions.md` | Consolidated `OQ-*` open-question index (indexer exception: no metadata block) |
| `nesdev-mapping.md` | NESdev Wiki mapping index (spec file → page, page → spec file) |

All 20 listed files are present in `Specs/`; `06-open-questions.md` and
`nesdev-mapping.md` are the two index documents (not per-section specs).

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | Writes to `$2008–$3FFF` change PPU registers | The PPU decodes only A0–A2 as `RS0–RS2` | Mirror the PPU register file every 8 bytes; do not treat `$2008+` as open bus |
| 2 | Only 2 KB of internal RAM is usable; `$0800–$1FFF` repeats | 2 KB SRAM addressed by A0–A10, board decode ignores A11–A15 | Implement the four 2 KB mirrors |
| 3 | A PAL game's fixed-cycle loop takes a different wall-clock time than on NTSC | CPU divider ÷16 and PPU divider ÷5 (PAL) vs ÷12 and ÷4 (NTSC) | Never hard-code NTSC cycles per frame (≈29,780) for PAL (≈33,247) |
| 4 | NMI fires at the start of vblank and the flag can be lost | PPU `/INT` is wired to `/NMI`; `$2002` bit 7 is set by the `/VSET` event and cleared by reading `$2002` | Read `$2002` once per NMI; remember it also resets the `$2005`/`$2006` write toggle |
| 5 | On Dendy, raster splits keyed to NMI land ~50 scanlines late | The UMC 6538 delays `VPLA_4` (`/VSET`) by 50 lines (`PPU/umc6538.md`) | Treat Dendy as its own timing variant, not as a PAL NES |
| 6 | A sprite update blocks the CPU until the transfer finishes | `$4014` starts the APU DMA controller, which holds the core with `RDY` (`APU/core.md`) | Budget the OAM upload inside vblank; exact stall cycles are in `02-cpu-dma.md` |
| 7 | Writing `$2003` mid-frame corrupts sprites | On the 2C02G-0 a write to OAMADDR corrupts OAM (`Docs/NES_Famicom_Board_Revisions.md`) | Avoid OAMADDR writes outside vblank on G-revision hardware |
| 8 | Writing `$3F10` also changes the background backdrop colour at `$3F00` | Palette RAM rows 0 and 4 share a word (`PPU/cram.md`) | Model the `$3F10/$3F14/$3F18/$3F1C` aliases |
| 9 | `SED` has no effect; BCD routines compute binary results | The `/DAA` and `/DSA` control signals are physically cut in the Ricoh core (`APU/core.md`) | Do not implement decimal mode on 2A03/2A07 |
| 10 | An unlicensed cartridge keeps the console in an endless reset loop | The NES CIC lock never completes the key exchange (`MB/NES.md`) | Provide the CIC (or its defeat) in board-level emulation; the loop shows as a repeating reset vector fetch |
| 11 | A cart without CHR memory renders garbage / reads back the last bus value | Pattern tables come from the cartridge; the board has no CHR RAM | Model CHR-ROM/CHR-RAM presence per cartridge |

## Programming / implementation notes

- An emulator MUST derive the CPU clock (NTSC ÷12, PAL ÷16, Dendy ÷15) and the
  PPU dot clock (NTSC ÷4, PAL and Dendy ÷5) independently from the region's
  master clock; tying them to one constant breaks at least one region. Note the
  NTSC and Dendy ratios both give 3 dots per CPU cycle at different absolute
  rates.
- An emulator MUST implement the CPU memory-map mirrors and the `M2` gating of
  the on-board decode (internal RAM, PPU registers and PRG ROM respond in `PHI2`
  only).
- An emulator MUST take nametable mirroring and the VRAM enable from the
  cartridge (`CIRAM A10`, `CIRAM /CE`), never from the PPU: the PPU drives
  A0–A13 but returns neither signal (`05-boards-and-io.md`).
- An emulator MUST mirror the PPU register file every 8 bytes over
  `$2008–$3FFF` and MUST implement the `$3F10/$3F14/$3F18/$3F1C` palette aliases.
- An emulator MUST wire the PPU vblank output to the CPU NMI input and MUST
  clear the flag and the `w` toggle on a `$2002` read.
- An emulator MUST implement OAM upload as the APU DMA transfer triggered by
  `$4014`, including the `RDY` stall (`02-cpu-dma.md`).
- An emulator SHOULD start from an undefined power-on state and let software
  initialise it, exposing code that relies on unspecified reset values.
- An emulator MUST NOT implement decimal mode for 2A03/2A07; the letterless
  2A03's missing looped-noise mode is modelled only for that revision.
- A tool that computes frame budgets MUST name the region: NTSC 262 scanlines ×
  341 dots, PAL 312 × 341, Dendy a PAL frame (312 × 341 = 106,392 dots) with an
  NTSC-like 3 dots-per-CPU-cycle ratio.

## NESdev Wiki correspondence

| Topic | NESdev Wiki page |
|---|---|
| System overview, terminology | `NES_reference_guide`, `Glossary` |
| CPU memory map, mirrors, `$4014`/`$4015`, `$4018–$401F` test mode | `CPU_memory_map`, `CPU_Test_Mode` |
| PPU memory map, nametables, attribute tables, mirroring, palettes | `PPU_memory_map`, `PPU_nametables`, `PPU_attribute_tables`, `Mirroring`, `PPU_palettes` |
| Master clock, CPU:PPU ratio, PAL/Dendy, vblank timing | `Cycle_reference_chart`, `PPU_frame_timing` |
| Chip pinouts and VRAM bus | `CPU_pinout`, `PPU_pinout` |
| CPU-facing PPU registers | `PPU_registers` |
| APU register map and frame counter | `APU_registers`, `APU_Frame_Counter` |
| Interrupts (NMI/IRQ) | `CPU_interrupts` |
| DMA/OAM | `DMA`, `PPU_OAM` |
| Controllers and expansion port | `Standard_controller`, `Expansion_port` |
| CIC lockout | `CIC_lockout_chip` |
| Cartridge connector and mappers | `Cartridge_connector`, `Mapper` |
| Region/model variants, revisions | `Famicom`, `CPU_variants`, `PPU_variants` |
| Motherboard revisions and glue logic | `NES_reference_guide` |
| Power-on state | `CPU_power_up_state`, `PPU_power_up_state` |
| Palette RAM and emphasis | `PPU_palettes`, `Colour_emphasis` |

## Provenance

| Claim area | BreakingNES Wiki | HDL |
|---|---|---|
| Chipset inventory, roles, Ricoh part numbers | `APU/Readme.md`, `PPU/Readme.md`, `Ricoh.md`, `Readme.md` | — |
| CPU core = 6502 with BCD cut | `APU/core.md`, `Ricoh.md`, `6502/Readme.md` | `HDL/APU/core.v` |
| Master clocks, dividers, `ACLK`, LFO rates | `APU/clkgen.md`, `PPU/pclk.md` | `HDL/APU/core.v`, `HDL/APU/clkgen.v`, `HDL/PPU/pclk.v` |
| Frame layout (341 dots, 262/312 lines, `/VSET`, `VCLR`, EVEN/ODD) | `PPU/hv.md`, `PPU/hv_decoder.md`, `PPU/fsm.md`, `PPU/pal.md` | `HDL/PPU/hv.v`, `HDL/PPU/hv_decoder.v` |
| CPU memory map and board decode (`M2` gating, 74LS139 selects, A0–A10 RAM) | `6502/Readme.md`, `APU/pads.md` | `Docs/NES/NES_HDL_Model/nes.v` — top-level `LS139`/`RAM`/`PPU` wiring; the `Readme.md` beside it is a title stub |
| `$3000–$3EFF` nametable mirror | — (NESdev only; the board decode behind it is not in the Wiki) | — (the `CartPort` body of `nes.v` is empty) |
| APU/IO register space and DMA register select | `APU/regs.md`, `APU/pads.md` | `HDL/APU/regs.v`, `HDL/APU/dma.v` |
| PPU register interface (`RS0–2`, `/DBE`, `w` latch) | `PPU/regs.md`, `PPU/pads.md` | `HDL/PPU/regs.v`, `HDL/PPU/pads.v` |
| PPU memory map, VRAM controller, palettes | `PPU/vram_ctrl.md`, `PPU/par.md`, `PPU/cram.md` | `HDL/PPU/vram_ctrl.v`, `HDL/PPU/cram.v` |
| Physical buses and pinouts | `6502/Readme.md`, `APU/pads.md`, `PPU/pads.md`, `Docs/Famicom/Chipset/Readme.md` | `nes.v`, `HDL/Common/bustris.v` |
| Reset and power-on | `6502/Readme.md`, `APU/pads.md`, `APU/old_2a03.md`, `PPU/pads.md`, `PPU/pclk.md`, `MB/Famicom.md`, `MB/NES.md` | `HDL/APU/pads.v`, `HDL/PPU/pads.v` |
| Region/model variants, CIC, motherboard and glue logic | `Docs/NES_Famicom_Board_Revisions.md`, `PPU/Readme.md`, `Ricoh.md`, `MB/NES.md`, `MB/Famicom.md`, `MB/Readme.md`, `Docs/Famicom/Chipset/Readme.md` | `nes.v` |
| Dendy and famiclones | `Dendy.md`, `PPU/umc6538.md`, `Docs/Famiclones/ChipFeatures.md`, `Docs/Famiclones/Readme.md` | `Docs/UMC6527P/div_umc.v` |

## Open questions

- **Letterless RP2A03 M2 duty cycle.** `APU/old_2a03.md` (rank 1) says the early part
  is 15/24 vs 17/24 later; `Docs/NES_Famicom_Board_Revisions.md` (rank 3) says
  letterless is 17/24. The tie-breakers side with the Docs file: NESdev `CPU_pinout`
  ("the original NTSC 2A03 (no letter), M2 has a duty cycle of 17/24"; "2A03E, G, and
  H ... 15/24") and `CPU_variants` ("17/24 instead of 15/24"). This rank-1 vs rank-3
  conflict still needs a die-level re-check of the letterless part.
- **Dendy clocks.** `Dendy.md` (rank 1) only says the CPU runs "close to the NTSC
  NES" with PAL video; the §2 figures are NESdev rank 4: `Cycle_reference_chart`
  gives the UMC `UA6527P` as `26.60~ MHz ÷ 15 = 1.773448 MHz` (113⅔ CPU cycles per
  scanline) and `CPU_variants` records the same ÷15 divider (`÷16` on some date
  stamps), so the figures are quoted but not silicon-confirmed. With the 6538's ÷5
  dot clock the ratio is **exactly 3 PPU dots per CPU cycle**; no rank 1–3 source
  here documents the 6527P divider (`Docs/UMC6527P/div_umc.v` states no ratio).
- **Cross-file items (re-checked at the final pass).** The siblings now agree with
  this file: `04-ppu-timing.md` §10 gives Dendy as `1.773448 MHz` / `3 : 1` (PPU
  dots per CPU cycle), `05-boards-and-io.md` §1 is cell-for-cell the table of §2
  here, `04-ppu-memory-map.md` marks `$3000–$3EFF` NESdev rank 4 throughout, and
  `03-apu-overview.md` §3 quotes NESdev's opposite `17/24` `M2` figure and records
  the conflict. Nothing is left pending, and §2's table is the normative one.
- **`$3000–$3EFF` nametable mirror and `$3F20–$3FFF` palette mirror.** NESdev
  states both. The palette mirror is strongly implied by rank 1–3: the CRAM
  address is 5 bits (`{CGA4,CGA1,CGA0}` × `{CGA3,CGA2}`), so rows
  `$3F20–$3FFF` alias the 32 entries (`04-ppu-palette.md`). The `$3000` mirror is
  **rank-4 only**: no rank 1–3 page states the `PA12`/A12 decode, the Wiki's
  `vram_ctrl.md`/`cram.md` describe the VRAM and CRAM decoders without naming the
  `$3000–$3EFF` range, and `nes.v`'s `CartPort` body is empty (NESdev notes the
  mirror as "usually" true). Both are therefore marked NESdev in §5.
- **CPU:PPU phase relationship.** No source states whether the CPU/PPU phase
  offset after `/RES` is deterministic, which a cycle-accurate scheduler needs.
- **Interrupt vector addresses.** The wiki documents the vector-generation logic
  but not the addresses (`$FFFA–$FFFF`); those above are NESdev/6502 knowledge.
- **`$4018–$401A` access on revision E.** The wiki describes the 2A03 debug
  interface in general and its absence in the letterless part; whether all
  `DBG`-capable revisions behave identically is not stated.
- **PPU revision gaps.** `2C02H` is only "fixes some glitches"; the rumoured
  2C01/2C06 debug PPUs are unconfirmed and 2C02A/B/C X=255 sprite behaviour is
  not fully characterised.
- **PAL NES CPU/PPU pairing.** The board-revision document marks several
  board/chip combinations as undocumented (e.g. which NTSC boards shipped
  6113-family locks); not resolvable from this repository's sources.
