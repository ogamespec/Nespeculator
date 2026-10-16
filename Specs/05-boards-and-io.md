# 05 Boards and I/O — motherboards, cartridge interface, controllers and CIC

**Status:** consensus (round 3)
**Scope:** Board- and system-level behaviour of the NES / Famicom family: board and chip
inventory, motherboard revisions, the 60-pin Famicom and 72-pin NES cartridge interfaces,
the motherboard address decode, the CIC lockout (10NES) and the reset it controls, the
standard-controller protocol on `$4016`/`$4017`, the two expansion ports, audio mixing and
the mapper landscape at summary level. Chip internals, clocks and memory maps are in
`01-platform-overview.md` … `04-ppu-*.md`; individual mapper register semantics are out of
scope and left to NESdev.
**Primary sources:** `BreakingNESWiki/MB/{Readme,NES,Famicom}.md`, `Dendy.md`,
`APU/{pads,regs}.md`, `PPU/pads.md`; `Docs/NES_Famicom_Board_Revisions.md`,
`Docs/Famicom/Chipset/Readme.md`, `Docs/Famicom/Famicom_Evo.circ`,
`Docs/Famicom/FamiController/Source.txt`, `Docs/Famiclones/{Readme,ChipFeatures}.md`,
`Docs/NES/NES_HDL_Model/{Readme.md,nes.v}`, `Docs/NES/NESController/`,
`Docs/JIO/Readme.md`; HDL: `HDL/APU/{io,pads,regs}.v`, `HDL/Common/bustris.v`.
**NESdev Wiki material:** the controller, cartridge-edge and expansion-port pages named in
the correspondence table — rank 4, marked *(NESdev)* in place; the repository/Docs material
above remains the specification wherever the two disagree.

## 1. Board inventory and regions

The console is two custom chips (CPU and PPU), two 2 KB×8 SRAMs, glue logic and the
video/RF stage; the Game Pak supplies PRG ROM and usually CHR. A 74LS139 decodes the CPU
address map, a 74LS373 latches the PPU address and two 40H368 inverting tri-state arrays
serve the I/O subsystem (one tri-state is the audio amplifier, one inverts PA13); late
boards fold the 139, the 368s and the RM1 pull-up pack (10 kΩ) into one 32-pin **JIO** chip
(`BU3266`/`BU3270`).

| Board family | Console | Cartridge slot | Lockout | Controllers |
|---|---|---|---|---|
| HVC-CPU-01…08, GPM-01/02a…k | Famicom HVC-001 (1983–1993) | 60-pin edge, top loading | none | two hardwired pads; pad 2 has a microphone |
| NES-CPU-01…11 | NES-001 front-loader (1985–1993) | 72-pin edge, front loading | CIC lock | two 7-pin sockets |
| NESN-CPU-01 / -JIO-01 / -AV-01 / -JIO-02 (PAL) | NES-101 top-loader (1993–1995) | 72-pin edge | none | two 7-pin sockets |
| HVCN-CPU-01/02 | AV Famicom HVC-101 (1993–2003) | 60-pin edge | none | two 7-pin sockets (NES type) |

The 368s are arrays of *inverting* tri-state buffers (`notif0`/`notif1`); on the NES-001
they are U7/U8, with U1/U4 the two 2 KB×8 SRAMs (WRAM / nametable VRAM), U2 the 74LS373,
U3 the 74LS139, U5 the PPU, U6 the CPU and U10 the CIC.

| Region | Chipset | Master clock | CPU divider | CPU `PHI0` | PPU divider | PPU dot (PCLK) | CPU:PPU |
|---|---|---|---|---|---|---|---|
| NTSC (Famicom/NES) | 2A03 + 2C02 | 21.477272 MHz | ÷12 | 1.7897727 MHz (558.73 ns) | ÷4 | 5.369318 MHz (186.24 ns) | 3:1 |
| PAL (European NES) | 2A07 + 2C07 | 26.601712 MHz | ÷16 | 1.6626070 MHz (601.46 ns) | ÷5 | 5.3203424 MHz (187.96 ns) | 16:5 |
| Dendy / PAL famiclones *(NESdev)* | UMC 6527P + 6538 (or NOAC) | like PAL (26.601712 MHz) | ÷15 | 1.773448 MHz (563.9 ns) | ÷5 | 5.3203424 MHz (187.96 ns) | 3:1 |

This table copies `01-platform-overview.md` §2 cell-for-cell; only the *(NESdev)* marker on
the Dendy row is local. On the board the console feeds both chips from one crystal (NTSC)
or the PAL divider chain, so a region swap needs both chips, not just the CPU. The `PCLK`
above is the 2C02 (`CLK` ÷4) / 2C07 (`CLK` ÷5) dot clock (`PPU/pclk.md`), and it is
**not** `SYSTEM CLK`: that signal is the raw master oscillator (21.477272 MHz NTSC /
26.601712 MHz PAL) on the NES 72-pin edge only (§5). `CPU:PPU` is the divider ratio and
numerically the **PPU dots per CPU cycle** (`3:1` = 3 dots/cycle, `16:5` = 16 PPU dots per 5
CPU `PHI0` cycles = 3.2, Dendy `15:5` = exactly `3:1`); the Dendy figures are NESdev rank-4
(`Cycle_reference_chart`, `CPU_variants`) — see *Open questions*.

## 2. Motherboard revision summary

Condensed from `Docs/NES_Famicom_Board_Revisions.md`, which holds the full tables, part
numbers and community sources — do not duplicate that document here.

| Family | Boards (era) | CPU / PPU | Key changes |
|---|---|---|---|
| Famicom launch | HVC-CPU-01…-05 (1983–1984) | RP2A03 (letterless) / RP2C02A…D | Square-button pads; recalled for stability; -05 became the service replacement |
| Famicom main | HVC-CPU-06…-08 (1984–1985) | RP2A03/E, RP2C02D/E | Round buttons; -06 first surface-mount RAM; -07 most common (~13.5 M), later 2A03G/2C02G |
| Famicom GPM | GPM-01, GPM-02a…k (1988–1993) | RP2A03G / RP2C02G-0 | VCCI redesign: RF/power on board, shielding, three-layer PCB; a JIO in any GPM-02 sub-revision is unconfirmed |
| NES front-loader | NES-CPU-01…-11 (1985–1993) | 2A03E/2C02E-0 → 2A03G/2C02G-0; -09…-11 also 2A07/2C07 | -01…-03 test market; -04 first PAL-capable board; -09…-11 add CIC STUN / −5 V countermeasures |
| NES top-loader | NESN-CPU-01, -JIO-01, -JIO-02 (PAL), -AV-01 | 2A03G/2C02G-0 → 2A03H/2C02H-0; PAL 2A07/2C07 | No CIC, no expansion port; common board has "jailbars" and populates only 68 of 72 connector pins |
| AV Famicom | HVCN-CPU-01/02 (1993–2003) | RP2A03H / RP2C02H-0 | JIO chip; -02 relocates C14 and adds a reverse-polarity diode |
| Regional NES | NESE-001 (PAL-B/PAL-A), NESA-001, NES-001 (KOR/BRA) | 2A07 / 2C07 (PAL); KOR is an NTSC console; BRA is PAL-M with a 2C02 PPU | PAL-B 3195A, PAL-A 3197A, 3196A; the Korean "Comboy" is **NTSC** shipped with a PAL-B lock; the Brazilian unit is **PAL-M** and uses a 2C02 PPU plus colour-conversion circuitry, with a 6113B1 lock |

RF/video: early Famicoms use a separate RF box on a ribbon cable, GPM boards integrate it;
the NES front-loader has an internal modulator (channel 3/4) plus composite jacks, the
French NES has no modulator (RGB on a proprietary connector), and the NES-101 is RF-only
except the rare AV-01 Multi-Out board.

## 3. Reset, the CIC lockout and its clock

The CIC (NESdev: **CIC lockout chip**, 10NES) runs from a **separate ≈4 MHz clock** (`nes.v`
`CIC_CLK`; exposed as pin 24 of the NES expansion port, "4.00 MHz CIC CLK"), independent of
the video crystal.

| Lockout part | Used in |
|---|---|
| 3193 (non-A), 3193A | NTSC NES-001 (3193 on NES-CPU-01…-04, 3193A from -04/-05) |
| 6113, 6113A, 6113B1 | Cartridge *keys* on most carts; listed NTSC lock options; Brazilian NES uses 6113B1 |
| 3195A (PAL-B) | Europe/Spain/France NESE-001; Korean "Comboy" |
| 3197A (PAL-A) | Mattel UK/AUS/NZ/ITA NESE-001 |
| 3196A | HK / Asian / Indian NESA-001 |
| 3198 / 3199 | FamicomBox cartridge key / coin-timer variant |

The console CIC is the **lock** and the cartridge CIC the **key**; they are the same IC
wired differently. LOCK/KEY is pulled to +5 V in the console and grounded on the cartridge,
both use the same 4 MHz clock, the key's RESET input comes from the lock's "SLAVE CIC
RESET" output, and the lock drives the CPU+PPU `/RES` line. A correct handshake releases
the console; otherwise the lock holds `/RES` low with a **1 Hz square wave**, so an
unlicensed cartridge resets the console about once per second, forever. Locks ground pins
11–15 (used for CIC addressing only in multi-game systems). NES-CPU-09/-10/-11 add series
resistors (two 1 kΩ on -11) and clamp diodes on the CIC data lines, because Camerica, Color
Dreams, AVE and AGCI carts used a charge pump generating −5 V to freeze the CIC; the
protection covers the data pins, not the reset pins. The lock is defeated either by
reconfiguring the console CIC as a key (disconnect pin 4) or by the reversible two-wire
method that lets the reset button drive `/RES` directly — the two-wire method is a
community technique with no rank-1 source here, so treat it as **unconfirmed**.

The Famicom, NES-101 and AV Famicom have **no** lockout chip. *(Unconfirmed — not in the
cached NESdev pages:)* the two CIC pins left on the NES-101 cartridge edge are tied to +5 V
and PPU D4, the other two float. On the Famicom the RESET button pulls `/RES` low for an
RC-defined period, and the pull-up pack RM1 (10 kΩ×11) holds `/IRQ` and `/NMI` at 1 when
undriven.

## 4. Motherboard address decode (74LS139)

One dual 2-to-4 decoder (`nes.v`; `Famicom_Evo.circ` `LS139`) produces every on-board chip
select; its second half is enabled by an output of the first, so all on-board devices
respond only while `M2` = 1.

| Half | Enable | Inputs | Output low = | Range |
|---|---|---|---|---|
| 1 | GND | `{A15, M2}` | `nY1[3]` = `/ROMSEL`; `nY1[1]` = half-2 enable (A15 = 0, M2 = 1) | `$8000–$FFFF` |
| 2 | `nY1[1]` | `{A14, A13}` | `nY2[0]` = WRAM `/CE`; `nY2[1]` = PPU `/DBE` | `$0000–$1FFF` / `$2000–$3FFF` |

WRAM (2 KB, A0–A10) is therefore mirrored four times, and the PPU register file every
8 bytes, because only A0–A2 (`RS0–RS2`) reach the PPU; `$4000–$401F` is decoded inside the
2A03/2A07. `/ROMSEL` is the logical NAND of `M2` and A15 and the only way a cartridge sees
A15, which is not carried on the cartridge edge (`01-platform-overview.md` §4 has the
full map).

## 5. Cartridge interface

This section has **no rank-1 source**: the repository documents the board side of the slot
(`nes.v` `CartPort`, `Famicom_Evo.circ` `P1`) but the wiki has no cartridge-connector page.
Signals are named from the console's point of view; pin numbers and the pinout rows below
follow NESdev `Cartridge_connector` and are marked *(NESdev)* where the repository does not
corroborate them.

| Signal group | Famicom 60-pin | NES 72-pin | Notes |
|---|---|---|---|
| CPU A0–A14, D0–D7, R/W, M2, `/ROMSEL`, `/IRQ` | yes | yes | A15 decoded on the board *(observed)*; `/IRQ` has an internal pull-up and is open-collector only on PlayChoice-10 *(NESdev)* |
| PPU A0–A13, D0–D7, `/RD`, `/WR`, `/A13` | yes | yes | PA13 is normally strapped to CHR `/CE`; the inverted `/A13` goes to CIRAM `/CE` *(NESdev)* |
| CIRAM A10, CIRAM `/CE` | yes | yes | Generated **by the cartridge** *(observed)*: it owns nametable mirroring and whether board VRAM is enabled at all |
| CIC CLK, +RST, data | no | pins 71/70/35/34 *(NESdev)* | Handshake pair; on the NES-101 tied to +5 V and PPU D4 *(unconfirmed — see Open questions)* |
| Audio from CPU, audio to RF | pins 45/46 *(NESdev)* | — | NES carts cannot inject audio; the NES mix input sits on the expansion port |
| SYSTEM CLK | no | pin 37 *(NESdev)* | Raw 21.477272 MHz (NTSC) / 26.601712 MHz (PAL) oscillator output |
| EXP0–EXP9 | no | pins 16–20, 51–55 *(NESdev)* | Wired through to the bottom expansion port; EXP6 became the de-facto expansion-audio line |
| +5 V, GND | yes | yes | Famicom pitch 2.54 mm, NES 2.50 mm *(NESdev `Famicom`)*; NES pins 62/63 are PPU A11/A10 out of sequence *(NESdev `Cartridge_connector`)* |

The PPU never drives CIRAM A10. A CHR-ROM cartridge decodes only `/RD` and PA13; a CHR-RAM
cartridge must also decode PPU `/WR` or writes through `$2007` are lost *(NESdev)*. The multiplexed
`AD0–AD7` bus is latched on the board: `ALE` = 1 presents the low address byte and the
74LS373 holds it as VRAM A0–A7, `ALE` = 0 turns the bus into the data byte; PA13 reaches the
cartridge inverted through one 368 tri-state (`Docs/Famicom/Chipset/Readme.md`).

**NMI is not on either cartridge connector.** The only NMI source is the PPU `/INT`
(vblank) pin wired to the CPU `/NMI`; a cartridge can pull `/IRQ` only.

## 6. Controller protocol (`$4016` / `$4017`)

The **standard controller** (Breaking NES Wiki: joypad) is a 4021 parallel-in/serial-out
shift register in the pad, clocked from the CPU through the port. Write `$4016`; read
`$4016` (pad 1) or `$4017` (pad 2). Writing `$4017` is the APU frame counter and does not
affect the pads.

```
$4016 write  xxxx x210   bit 0 = OUT0 pad latch (both ports), bits 1-2 = OUT1/OUT2 to the
                         expansion port; bits 3-7 ignored
$4016 read   xxxD DDDD   D0 pad serial, D1 Famicom expansion controller, D2 Famicom
                         microphone, D3 Zapper light, D4 Zapper trigger, D5-D7 open bus
$4017 read   xxxD DDDD   same D0/D1/D3/D4 layout for pad 2; D2 is the microphone on
                         $4016 only
```

| Step | Action | Hardware effect |
|---|---|---|
| 1 | write `$4016` bit 0 = 1 | OUT0 high; both pads' 4021 registers continuously reload their parallel inputs |
| 2 | write `$4016` bit 0 = 0 | OUT0 low; the registers hold the latched buttons in serial mode |
| 3 | read `$4016` or `$4017` eight times | each read sends one clock pulse to that port and returns one bit on D0 |

- Report order A, B, Select, Start, Up, Down, Left, Right; **1 = pressed**. Pad inputs are
  pulled to +5 V and shorted to GND by the buttons, so the 4021 serial output is 0 when
  pressed and the board's 40H368 inverts it on the way to the CPU data bus. After 8 bits an
  official pad returns 1 forever (its serial input is tied to GND).
- Clock: `R/W NAND (address = $4016/$4017)`, low only during the read. The console latches
  the pad data on the high→low transition and the pad shifts on the low→high transition at
  the end of the read, so **every read of the port is also a shift**. On the Famicom the pad
  data passes through the inverting 40H368 tri-state arrays U7/U8
  (`Docs/Famicom/Famicom_Scheme_IO_Annotated.png`; `nes.v` `CtrlPort`, `LS368`).
- Unconnected lines read 0 on the NES-001 and Famicom (pull-ups plus inverter); the NES-101
  leaves D2–D4 as open bus *(NESdev)*. D5–D7 are never driven and hold the previous bus byte,
  normally `$40` *(NESdev)*; Paperboy and similar games test for `$40`/`$41`.
- Line sharing: NES ports expose D0 (pad serial), D3 (Zapper light) and D4 (Zapper trigger);
  Famicom pads use D0, pad 2 supplies the microphone on `$4016` D2, and D1–D4 appear on the
  15-pin expansion port. Famicom pad 2 has no Select/Start (bits read 0).
- **DMC (DPCM) DMA collision.** A DMC fetch that starts a read in the same cycle as the program's
  `$4016`/`$4017` read double-clocks the pad and deletes one bit; the trailing 1 bit then
  takes the 8th position, showing up as a spurious Right press. Fixed in the 2A07 (PAL).
  Read repeatedly and keep a result only when two consecutive reports agree, or trigger an
  OAM DMA last in vblank so the reads land on even CPU cycles (`02-cpu-dma.md` §4; NESdev
  `Controller_reading`, `APU_DMC`).
- Some PAL consoles (FRA/HOL/NOE) have port diodes, so only pads with 3.6 kΩ pull-ups on
  CLK and LATCH (NES-004E) can drive them; the NESE-034 Four Score is likewise protected
  *(NESdev `Controller_port_pinout`#Protection_Diodes, `Standard_controller`#PAL)*.

## 7. Expansion ports

| | Famicom | NES |
|---|---|---|
| Connector | 15-pin male DA-15, front edge | 48-pin card edge, underside, under a plastic tab; 2.6 mm thick |
| Cartridge-edge wiring | separate port only (`Famicom_Evo.circ` `P2`) | EXP0–EXP9 also appear on the 72-pin edge (`MB/NES.md`) |
| Audio | SOUND pin (before/after expansion-audio mixing on RF/AV Famicom) | audio mix input, pin 3 |

Famicom DA-15 pinout (NESdev `Expansion_port`; corroborated pin-for-pin by
`Famicom_Evo.circ` `P2`): 1 GND, 2 SOUND, 3 `/IRQ`, 4–8 joypad 2 `/D4`,`/D3`,`/D2`,`/D1`,`/D0`,
9 `/OE` joypad 2 (`$4017` read strobe), 10–12 OUT2, OUT1, OUT0, 13 joypad 1 `/D1`, 14 `/OE`
joypad 1 (`$4016` read strobe), 15 +5 V. `/OE` is also usable as a shift clock. The port was
used commercially (third-party pads, 3rd/4th players). *(NESdev:)* on the RF Famicom, Twin
Famicom and Titler, joypad 2 `/D0` is driven by the permanently connected pad; on the AV
Famicom the detachable pad makes it an input, which the Multi Adapter AX-1 does not expect.

The NES 48-pin port (never used commercially) carries +5 V/GND, the audio mix input
(pin 3), open-collector `/NMI`, A15, EXP0–EXP9, both `/OE` strobes, joypad 1
`/D0`,`/D1`,`/D3`,`/D4`, joypad 2 `/D0`–`/D4`, CPU D0–D7, `/IRQ`, video out, amplified
audio, unregulated adapter voltage and the 4 MHz CIC clock (pin 24). The repository has no
NES expansion-port page, so this row is *(NESdev `Expansion_port`, `Hardware_pinout`)*. Most
cartridge ASICs drive `/IRQ` push-pull, so an expansion device should insert a series 1 kΩ
resistor.

## 8. Audio mixing

- Famicom: the CPU `AUX A`/`AUX B` DAC outputs, the cartridge audio in (edge pin 45), the
  cartridge audio return (pin 46, to the RF/AV output) and the pad-2 microphone are summed
  into the input of the 368 tri-state used as an inverting amplifier (`Famicom_Evo.circ`;
  `MB/Famicom.md` shows the same stage).
- JIO-based boards (AV Famicom, late Famicom/top-loader revisions) mix expansion
  (cartridge) audio quieter than pre-GPM boards, so games written for an early Famicom
  sound different on a GPM or AV Famicom.
- The NES 72-pin edge has no audio pins, so NES carts cannot mix extra channels without
  modification; the NES mix input exists only on the expansion port (pin 3)
  *(NESdev `Expansion_port`; NESdev `Category:Expansion_audio` lists the boards that use the
  Famicom path)*.

## 9. Mapper landscape

Mirroring is *always* cartridge-controlled through CIRAM A10 / CIRAM `/CE`; banking, save
RAM, interrupts and extra sound are added by the cartridge. Mappers are **discrete logic**
(named after a board, e.g. UNROM) or **ASIC** (Nintendo's MMC, Konami's VRC), and the
community identifies them by the iNES header mapper number; some discrete-logic boards
suffer **bus conflicts** when the ROM output fights a mapper register write, while ASICs
usually gate PRG `/CE` with `/ROMSEL` and A14.

| iNES | Common name | Type | Characteristic capability |
|---|---|---|---|
| 0 | NROM | discrete | no banking; 32 KB PRG / 8 KB CHR maximum |
| 1 | MMC1 | ASIC | serial PRG/CHR banking, switchable mirroring, battery RAM, CPU-cycle IRQ |
| 2 | UxROM (UNROM/UOROM) | discrete | 16 KB PRG banks switched at `$8000`, CHR-RAM |
| 3 | CNROM | discrete | 8 KB CHR banks, fixed PRG |
| 4 | MMC3 | ASIC | 8 KB PRG / 1 KB CHR banking, scanline IRQ, 4-screen option |
| 5 | MMC5 | ASIC | large PRG/CHR, extended attributes, vertical split, extra audio, multipliers |
| 7 | AxROM | discrete | 32 KB PRG banks, single-screen mirroring selected by the mapper |
| 24 / 26 | VRC6 | ASIC | Konami banking plus pulse/saw expansion audio |
| 69 | Sunsoft FME-7 / 5B | ASIC | banking, CPU IRQ; the 5B adds expansion audio |
| 85 | VRC7 | ASIC | Konami banking plus FM (OPLL) expansion audio |

Konami VRC1/VRC2/VRC4, Namco 163, MMC2/MMC4, Color Dreams, Camerica, GxROM and dozens of
pirate/Asian mappers follow the same pattern; NES 2.0 headers extend the numbering beyond
255. This table is background *(NESdev `Mapper`, which links one `INES_Mapper_NNN` page per
number)* — this repository contains no mapper documentation, and per-mapper limits MUST be
taken from the NESdev pages.

## Observable phenomena

| # | Phenomenon (how it shows up) | Cause | What to do |
|---|---|---|---|
| 1 | An unlicensed cartridge resets the console about once per second, forever | The console CIC lock never completes the key exchange and holds `/RES` with a 1 Hz square wave | Emulate the lock as a reset generator (or its defeat); the screen shows a repeating reset-vector fetch |
| 2 | Reading the pad in a tight loop returns the A button eight times | The 4021 stays in parallel-load mode because OUT0 (the latch) was never brought low | Always write 1 then 0 to `$4016` before reading |
| 3 | Pressed buttons read as 1, an unplugged pad reads as 0 | Pad inputs are pulled up and grounded by the buttons; the 40H368 inverts the serial line | Invert the raw pad bit once, not once per read |
| 4 | `$4016`/`$4017` reads return `$40` in bits 5–7 | Those bits are not driven and hold the previous bus byte, normally the high byte of the port address | Model open bus for bits 5–7 only |
| 5 | A spurious Right press appears while a DMC sample plays | A DMC fetch double-clocks the shift register and deletes one bit, so the trailing 1 lands in the 8th position | Re-read until two consecutive reports match, or use the OAM-DMA alignment trick; 2A07 (PAL) has no glitch |
| 6 | An NTSC pad does nothing on some PAL consoles | Protection diodes on FRA/HOL/NOE ports block CLK/LATCH unless the pad pulls them up (3.6 kΩ, NES-004E) | Present the pad as a pull-up-equipped PAL type on those models |
| 7 | Famicom pad 2 cannot press Start or Select | The hardwired pad 2 has no such buttons; the bits read 0 | Do not require Start/Select on Famicom pad 2 |
| 8 | A cartridge cannot generate NMI | `/NMI` is not on either cartridge connector; the only source is the PPU `/INT` | Restrict cartridge interrupts to `/IRQ` |
| 9 | Expansion audio is silent on an NES but works on a Famicom | The 72-pin edge has no audio pins; the NES mix input lives on the unused expansion port | Gate expansion audio by console type |
| 10 | The top-loader NES has no CIC, and its CIC cartridge pins sit at +5 V / PPU D4 | The NES-101 omitted the lockout chip | Do not require CIC authentication on the NES-101 or AV Famicom |

Rows 4, 5, 6 and the `$40` detail rest on NESdev rather than the repository; row 10's CIC
pin tie-off is unconfirmed — see *Open questions*.

## Programming / implementation notes

- An emulator MUST implement the board decode as `M2`-gated: WRAM at `$0000–$1FFF` (four
  mirrors), the PPU register file at `$2000–$3FFF` (8-byte mirrors), `/ROMSEL` = NAND(A15,
  M2) over `$8000–$FFFF`, and the APU at `$4000–$401F` from inside the CPU. The cartridge
  MUST own CIRAM A10 and CIRAM `/CE`; the PPU MUST NOT select mirroring.
- The pad protocol MUST be reproduced exactly: latch write 1 → write 0, then one bit per
  read in the order A, B, Select, Start, Up, Down, Left, Right, inverted so that 1 =
  pressed, returning 1 after the eighth bit on official pads. Every `$4016`/`$4017` read
  MUST clock the port, so any extra read (in particular during a DMC DMA) shifts it.
- `$4016` write bits 0–2 MUST be latched and held, bits 3–7 ignored; `$4017` writes MUST go
  to the frame counter only. Bits 5–7 of both reads MUST be open bus; unconnected D0–D4
  SHOULD read 0 on the NES-001/Famicom and open bus (D2–D4) on the NES-101.
- The DMC/controller collision SHOULD be emulated for NTSC and Dendy parts and MUST NOT be
  applied to the 2A07 (PAL). Region-dependent behaviour MUST be per machine: CIC only on
  the NES-001 front-loader, PAL-A and PAL-B locks mutually incompatible, SYSTEM CLK on the
  NES cartridge edge but not the Famicom, and Dendy MUST NOT be clocked as PAL.
- A tool needing mapper limits (PRG/CHR size, IRQ, audio) MUST consult the NESdev page for
  that mapper number; this spec gives the landscape only.

## NESdev Wiki correspondence

All page names below were checked against the local wiki cache (`/tmp/nesdev-cache/`) or
fetched through the Wayback Machine; `Famicom`, `NES` and `Four_score` are redirects, so the
canonical target is given in each case.

| Topic | NESdev Wiki page |
|---|---|
| Console models, region variants, revision history | `Famicom`, `NES_reference_guide` (the `NES` title redirects here) |
| CPU and PPU chip variants | `CPU_variants`, `PPU_variants` |
| Board chip inventory, JIO integration, logic ICs, chip list | `Hardware_pinout`, `CPU_variants`, `PPU_variants` |
| Master clock, CPU:PPU ratio, PAL and Dendy division | `Cycle_reference_chart`, `NES_reference_guide` |
| CIC lockout chip, lock/key handshake, reset loop, defeat, pinout | `CIC_lockout_chip`, `CIC_lockout_chip_pinout` |
| CPU address decode, mirrors, `/ROMSEL`, WRAM and PPU selects | `CPU_memory_map`, `Cartridge_connector` |
| Cartridge edge signals, bus ownership, CIRAM A10 / CIRAM `/CE` | `Cartridge_connector`, `Mirroring` |
| Cartridge-edge and expansion-port `EXP0–EXP9` pins | `EXP_pins`, `Cartridge_connector` |
| Standard controller protocol, `$4016` write bits, D0–D4 line use | `Standard_controller`, `Controller_reading`, `Controller_port_pinout` |
| Zapper, Four Score and other controller hardware | `Zapper`, `Four_player_adapters` (the `Four_score` title redirects here) |
| DPCM/DMC-vs-controller conflict and the software workarounds | `Controller_reading_code`, `APU_DMC` (`#Conflict_with_controller_and_PPU_read`), `Errata` |
| Famicom DA-15 and NES 48-pin expansion ports | `Expansion_port`, `Hardware_pinout` |
| Expansion audio and board-level mixing | `APU_Mixer`, `Category:Expansion_audio` |
| iNES/NES 2.0 mapper numbers, families, discrete vs ASIC, bus conflicts | `Mapper` (one `INES_Mapper_NNN` page per number), `INES`, `NES_2.0`, `Bus_conflict` |
| Open bus on the controller ports | `Controller_reading`, `Open_bus_behavior` |

## Provenance

A cell naming a `Docs/…` or `HDL/…` path is a rank-3/repository source, not a
`BreakingNESWiki/` page; `nes.v` and `Famicom_Evo.circ` are the board-level HDL/schematic
under `Docs/NES/NES_HDL_Model/` and `Docs/Famicom/`.

| Claim area | BreakingNES Wiki page | HDL module |
|---|---|---|
| Board inventory, glue logic, JIO, 368/139/373 roles | `MB/{Readme,NES,Famicom}.md`, `Docs/Famicom/Chipset/Readme.md`, `Docs/JIO/Readme.md` | `Docs/NES/NES_HDL_Model/nes.v` |
| Motherboard revisions, region clocks, RF/video differences | `Docs/NES_Famicom_Board_Revisions.md`, `MB/{NES,Famicom}.md` | `HDL/APU/core.v`, `HDL/PPU/pclk.v` |
| Dendy (UMC 6527P/6538) ÷15 CPU divider, `1.773448 MHz` and dot clocks | (not in this repository; `Dendy.md` says only "close to NTSC") NESdev `Cycle_reference_chart`, `CPU_variants` | — |
| LS139 decode, `M2` gating, `/ROMSEL`, WRAM/PPU selects | — | `nes.v`, `Famicom_Evo.circ` (`LS139`) |
| Cartridge-edge signals, bus ownership, CIRAM A10 `/CE` | `PPU/pads.md`, `APU/pads.md`, `Docs/NES/NES_HDL_Model/Readme.md`, `PPU/vram_ctrl.md` | `nes.v` (`CartPort`), `Famicom_Evo.circ` (`P1`) |
| CIC behaviour, reset loop, region parts, defeat | `MB/NES.md`, `Docs/NES_Famicom_Board_Revisions.md` §5.1 | `nes.v` (`CIC`) |
| Famicom reset button, RM1 pull-ups, audio summing | `MB/Famicom.md`, `Docs/Famicom/Famicom_Scheme_IO_Annotated.png` | `Famicom_Evo.circ`, `nes.v` (`AudioMix`) |
| `$4016`/`$4017` register bits, OUT0–2 latches, `/IN0`,`/IN1` | `APU/{pads,regs}.md` | `HDL/APU/{io,pads,regs}.v` |
| Pad protocol (4021, latch, clock, inversion, line use) | `Docs/NES/NESController/{con_schem.jpg,timing.jpg,CD4021.pdf}`, `Docs/Famicom/FamiController/Source.txt` | `nes.v` (`CtrlPort`, `LS368`), `Famicom_Evo.circ` (`P4`,`P5`,`U_40H368`) |
| DPCM/pad double-clock conflict | (not in the wiki) NESdev `Controller_reading`, `Errata` | `02-cpu-dma.md` §4 |
| Expansion-port pinouts and `EXP` wiring | `MB/NES.md` | `Famicom_Evo.circ` (`P2`), `nes.v` (`ExpPort`) |
| Mapper landscape, discrete vs ASIC, bus conflicts | (not in this repository) NESdev `Mapper` | — |
| Dendy/famiclone and Vs./PlayChoice-10/FamicomBox notes | `Dendy.md`, `Docs/Famiclones/{Readme,ChipFeatures}.md`, `Docs/VS_UniSystem/Readme.md`, `Docs/PlayChoice-10/Readme.md`, `Docs/NES_Famicom_Board_Revisions.md` | — |

## Open questions

Section marking convention used above: *(observed)* = rank 1–3 repository source,
*(NESdev)* = the fact comes from the NESdev Wiki only and is not corroborated here.

- **The controller protocol has no rank-1 source.** The wiki has no controller or `$4016`
  prose page; the protocol above comes from the `Docs/` schematics plus NESdev and needs a
  die-level re-check. Specifically NESdev-only and unconfirmed here: the `$40` value in
  read bits 5–7, the NES-101 open-bus row, and the PAL protection diodes.
- **Cross-file: `01-platform-overview.md` §2.** The final consistency pass
  re-checked both clock tables and the dot numbering: `04-ppu-timing.md` §10 agrees
  with §1 here and `01` §2 (Dendy `1.773448 MHz`, `3:1`); `01` line 52 now says so,
  and the old `1:3`/`5:16`/`186.2 ns` / `1 : ≈2.973` wordings are gone.
- **Dendy clocks are rank-4.** `Dendy.md` (rank 1) says only that the CPU runs "close to
  the NTSC NES" with PAL video; the §1 ÷15 / `1.773448 MHz (563.9 ns)` / `3:1` cells are
  NESdev (`Cycle_reference_chart`: `26.60~ MHz ÷ 15 = 1.773448 MHz`; `CPU_variants`: ÷15
  on parts whose date stamp has a `-`). No rank 1–3 source here documents the 6527P divider.
- **Which 40H368 tri-state is which.** The wiki text ("one tri-state is the audio
  amplifier, one inverts PA13") disagrees with the `Famicom_Evo.circ` comment that "A6/Y6
  are used as an inverter for sound"; the `n_G1`/`n_G2` grouping of Y1–Y4 / Y5–Y6 is the
  74HC368 datasheet grouping, not proven for that model.
- **Shift order vs the repository diagram.** `Docs/NES/NESController/timing.jpg` labels the
  3rd and 4th shifts "Start, Select" while NESdev (and the cc65 convention) says Select
  then Start; the diagram is assumed wrong until verified on hardware.
- **Famicom pad-2 microphone address.** NESdev places the microphone on `$4016` D2 and
  `Famicom_Evo.circ` models a MIC line on the pad-2 subcircuit, but the wiki gives no
  address and no die-level source exists here. The spec therefore states only that D2 is
  the microphone on `$4016`, not which port pin a `$4017` read samples.
- **CIC defeat methods.** NESdev documents only the "disconnect pin 4" conversion; the
  reversible two-wire method that lets the reset button drive `/RES` directly has no
  rank-1 source here and is marked **unconfirmed** in §3.
- **Famicom pad clock/latch drive.** An `M2`-gated inverting 368 driving the pad
  `CLK`/`LATCH` lines is not in `Famicom_Evo.circ` (`P4`/`P5` expose only `OUT_0`,
  `S4016_*`/`S4017_*`, `MIC`) or `nes.v` (`CtrlPort`/`LS368` model the data path only).
- **NES-101 CIC tie-off.** The §3 claim that two of the four CIC edge pins sit at +5 V and
  PPU D4 (the rest floating) is in none of the cached NESdev pages inspected
  (`Cartridge_connector`, `CIC_lockout_chip`, `CIC_lockout_chip_pinout`, `Famicom`); unconfirmed.
- **PAL controller protection diodes.** Which models and board revisions carry the port
  diodes is undocumented here; NESdev names only FRA/HOL/NOE.
- **Mapper documentation is absent.** No repository file describes mappers, iNES numbers,
  bus conflicts or mapper mirroring, so §9 is NESdev background, not silicon truth.
- **Expansion-audio mixing levels.** `nes.v` calls the summing a black box ("some kind of
  magic") and the GPM/AV Famicom level change is qualitative; no gain figures exist.
- **Famicom 60-pin audio pins.** The wiki has no cartridge-connector page; pins 45/46 as
  "audio from CPU"/"audio to RF" come from NESdev plus `Famicom_Evo.circ` `P1.SoundIn`/`SoundOut`.
- **Vs. System / PlayChoice-10 / FamicomBox detail.** The repository has stub pages only
  (one link each), so these platforms are not specified beyond the revision tables.
