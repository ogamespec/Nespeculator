# Nespeculator

A Famicom/NES emulator written in C11 with an SDL3 frontend, implemented
**strictly from the specification set in [`Specs/`](Specs/)** as an experiment in
spec-driven development: the only design input was the Markdown in that
directory. Everything the specification set does not define is recorded in
[`SPEC-GAPS.md`](SPEC-GAPS.md).

## Building

The project builds on Windows with **MSVC** through CMake and the Ninja
generator. Visual Studio 2022 or newer is recommended; the "Desktop development
with C++" workload also provides CMake and Ninja, so nothing else has to be
installed.

```
git clone <repo> && cd Nespeculator
build.bat                 # configure + build (Release)
build\Nespeculator.exe <rom.nes>
```

`build.bat` finds `vcvars64.bat` by itself (via `vswhere`, with a scan of the
standard install roots as a fallback; inside a Developer Command Prompt the
existing environment is used unchanged), configures `build/` with CMake on the
first run and then builds. `build.bat clean` deletes `build/` first, which is
the way to recover from a half-finished configure or a moved checkout.

The SDL3 3.2.10 development files are **vendored** in `third_party/SDL3`
(headers plus `lib\x64\SDL3.lib` and `SDL3.dll`), so a fresh clone builds
without any download; the build copies `SDL3.dll` next to the executable.

### Command line

| Option | Effect |
|---|---|
| `<rom.nes>` | iNES / NES 2.0 image to load |
| `--region ntsc\|pal\|dendy` | override the emulated region |
| `--scale N` | window scale (1..8) |
| `--screenshot FILE` | write a BMP after N frames and exit |
| `--screenshot-frame N` | frame at which to save the screenshot |
| `--trace N` | print emulator state every N frames |
| `--dumpstate N` | print full CPU/PPU/RAM state after N frames and exit |
| `--keys` | print the current input bindings and exit |
| `--step` | single-instruction stepping (start PC from `NES_START_PC`) |
| `--ramhex ADDR COUNT` | hex dump of work RAM |
| `--rombytes ADDR COUNT` | hex dump of cartridge PRG at a CPU address |
| `--ramtrace` | report the first execution below `$8000` and the path taken |
| `--dumpeach` | dump the first 40 framebuffers as PPM into `NES_DUMPDIR` |
| `--help` | usage |

The ROM may also be supplied through the `NES_ROM` environment variable instead of
an argument, which avoids the Windows batch-quoting problems with ROM names that
contain spaces and parentheses.

Starting the emulator **without** a ROM no longer exits: it opens its window with
a short hint screen and waits, and a `.nes` file dropped onto that window is
loaded immediately (dropping a different ROM while a game runs replaces it).
Automation flags (`--screenshot`, `--dumpstate`, `NES_AUTOINPUT`, ...) still
require a ROM and keep failing fast with a usage message.

### Automated test-ROM runs

`NES_AUTOINPUT` drives a scripted controller sequence, which is what makes it
possible to run a test ROM like AccuracyCoin unattended:

| Variable | Meaning |
|---|---|
| `NES_AUTOINPUT` | `frame:button:duration` triples, comma separated, e.g. `200:a:4,320:right:4`. Buttons: `a b select start up down left right` |
| `NES_AUTOFRAMES` | total frames to run before exiting |
| `NES_AUTOSHOT` | frame at which to dump the framebuffer |
| `NES_AUTOSHOTFILE` | destination for that dump (raw 256x240 RGBA) |
| `NES_BUSLOG` | write a full CPU bus trace (cycle, R/W, address, data, PC, `F` for fetch) to a file |

`NES_BUSLOG` is the primary instrument for cycle-level debugging: it shows the
dummy reads and writes that `Specs/02-cpu-instructions.md` section 4 requires.

## Features

- **CPU** — cycle-accurate 2A03/2A07 core: every CPU cycle performs exactly one
  bus access, so the dummy reads, the RMW double write, the branch frames and
  the 7-cycle interrupt frame appear on the bus as `Specs/02-cpu-instructions.md`
  section 4 describes. Official opcodes plus the stable undocumented set.
- **PPU** — dot-accurate RP2C02/RP2C07: the per-dot fetch schedule, sprite
  evaluation with the documented 9th-sprite overflow walk, the sprite pipeline,
  mid-scanline register writes, the `v`/`t`/`x`/`w` scroll model, palette RAM
  aliasing, PPU open bus and the NTSC odd-frame dot skip.
- **APU** — pulse 1/2 with envelope and sweep, triangle with the linear counter,
  the noise LFSR, DMC with sample fetch, the 4/5-step frame counter and the
  AUX A / AUX B non-linear DAC mixing.
- **Cartridges** — NROM (0), MMC1 (1), UxROM (2), CNROM (3), MMC3 (4) and AxROM
  (7); CHR-ROM and CHR-RAM; all four nametable arrangements driven by the
  cartridge's CIRAM A10 as the specification requires.
- **Frontend** — SDL3 video (letterboxed integer-ish scaling), SDL3 audio, and
  an in-emulator HUD.
- **HUD** — an overlay menu (drawn with an embedded 6x8 bitmap font, no external
  font file needed) with: resume, save/load state, ten save-state slots, a
  controls editor for both players on both keyboard and gamepad, region, window
  scale, VSync, FPS display, overlay toggle, audio enable/volume, a system-info
  screen and reset/quit. Bindings are persisted to
  `%APPDATA%\Nespeculator\nespeculator.cfg`.
- **Save states** — a versioned container that serialises the CPU registers and
  in-flight instruction, the whole PPU (registers, `v`/`t`/`x`/`w`, CIRAM, OAM,
  palette, shift registers, sprite lanes, framebuffer), the APU channel state,
  the mapper registers, CHR-RAM and the 2 KB work RAM.

## Controls (defaults)

| Console | Player 1 | Player 2 |
|---|---|---|
| D-pad | arrow keys | I / J / K / L |
| A | X | G |
| B | Z | F |
| Select | Right Shift | T |
| Start | Enter | Y |

| Emulator | Key |
|---|---|
| Menu | Escape |
| Save / load state | F5 / F8 |
| Reset | F2 |
| Pause | P |
| Fast forward | Tab |
| Frame advance | Backspace |
| Quit | F10 |

Gamepads are detected automatically; the first two are player 1 and player 2.
Every binding can be reassigned from the HUD's **Controls** submenu.

## Source layout

```
src/core/      emulation core
  nes.c        clocks, frame loop, reset, DMA scheduling
  cpu.c        per-cycle instruction sequencer
  cpu_ops.c    opcode/addressing tables and operation semantics
  bus.c        CPU address map, open bus, OAM/DMC DMA
  ppu.c        dot-accurate PPU
  apu.c        audio channels and mixing
  cart.c       iNES loading and mappers
  input.c      controller shift registers
  state.c      save-state serialisation
src/frontend/ SDL3 frontend
  main.c       application loop, input, audio
  hud.c        overlay menu
  render.c     video and the bitmap-font text renderer
  hud_font.h   generated 6x8 font
  input_map.c  bindings and settings persistence
```
