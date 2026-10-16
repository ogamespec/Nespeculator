# Spec gaps found while building a spec-driven NES emulator

This emulator was implemented strictly from the Markdown files under `Specs/*.md`, and anything the spec set does not state had to be decided outside it. The list below records every place where that spec set is silent, ambiguous, self-contradictory, or explicitly marked open, grouped by hardware area. Each entry says what an implementer must consequently invent, guess, or leave configurable. The specs themselves flag many of these as open questions, so this list overlaps their own "Open questions" sections, but it is oriented to what an implementer has to do rather than to what a researcher might still confirm.

## 1. Cartridge header and mapper register interfaces (the largest gap)

- **The iNES/NES 2.0 magic and PRG/CHR size fields are never described.** No file under `Specs/*.md` defines the header magic, the 16 KB PRG-unit count or the 8 KB CHR-unit count. An implementer must adopt a header format from outside the spec set before a single ROM can be opened.
- **The mirroring flag and the four-screen flag are not described.** Nothing states where a header records vertical/horizontal/one-screen/four-screen, or how the flag relates to the CIRAM A10 value the cartridge is supposed to generate; the emulator invents the loader's mirroring decode.
- **The battery, trainer and submapper bits are absent.** No spec text names a battery-backed save, a 512-byte trainer, or the NES 2.0 submapper nibble. Persistence and trainer-skip behaviour are therefore emulator-defined.
- **CHR-ROM vs CHR-RAM is never determined by a rule.** The boards file distinguishes "CHR-ROM" from "CHR-RAM" boards and says CHR-ROM ignores the write strobe, but no header bit, size convention or board table tells a loader which one a given image uses; the emulator guesses from the CHR size field.
- **Mapper register semantics are explicitly out of scope.** `05-boards-and-io.md` opens with the scope statement that "individual mapper register semantics are out of scope", and its §9 closes: "this repository contains no mapper documentation, and per-mapper limits MUST be taken from the NESdev pages." No register address, bit field or banking granularity appears anywhere.
- **The mapper table is background, not spec.** §9 lists iNES numbers 0, 1, 2, 3, 4, 5, 7, 24/26, 69 and 85 with one-line capability summaries, tagged "background *(NESdev `Mapper`…)*"; the emulator must take the actual register layout for each from outside the repo.
- **The unnumbered mapper families have no registers either.** VRC1/VRC2/VRC4, Namco 163, MMC2/MMC4, Color Dreams, Camerica and GxROM are named in prose, and "NES 2.0 headers extend the numbering beyond 255", but no register interface exists for any of them, so none can be implemented from spec.
- **No bank-switching granularity or PRG/CHR window layout.** The table says MMC3 does "8 KB PRG / 1 KB CHR banking" and UxROM switches 16 KB banks at `$8000`, but which register values select which banks, where the fixed bank lives, and in what order the switchable windows appear are all unstated.
- **Bus conflicts are named but not characterised.** §9 says "some discrete-logic boards suffer **bus conflicts** when the ROM output fights a mapper register write, while ASICs usually gate PRG `/CE`", but not which writes conflict on which board, what value results, or how open bus participates.
- **CIRAM A10 and CIRAM `/CE` polarity are only sketched.** The spec states the cartridge "MUST own CIRAM A10 and CIRAM `/CE`; the PPU MUST NOT select mirroring", but the polarity of `/CE`, the A10 value per arrangement, and the mapper-driven one-screen case are left to the implementer.
- **The four-screen mechanism is a one-line entry.** `04-ppu-memory-map.md` gives "Four-screen | 64×60 | Extra 2 KB cartridge VRAM, not CIRAM | All four nametables distinct" and nothing about how a board enables it or which header bit selects it.
- **Mapper IRQ behaviour is not specified.** MMC3's scanline IRQ and the FME-7/VRC CPU IRQ are listed as capabilities, but no counter source, clock edge, reload/latch order or acknowledge behaviour is given; the only timing hint is `04-ppu-rendering.md`'s note that MMC5 clocks a scanline counter from the three consecutive nametable fetches at dots 337–340 and 1–2.
- **Expansion-audio register interfaces are absent.** VRC6, the FME-7 5B and VRC7 are named as adding "pulse/saw", "expansion audio" and "FM (OPLL)", but no register addresses or write protocol exist, so board audio cannot be mixed from these files.
- **Save RAM, battery backing and WRAM sizing are unaddressed.** The battery bit, PRG-RAM size and any save-file format are not mentioned for any board; the implementer chooses a persistence model with no spec basis.

## 2. CPU core

- **Power-on register values are rank-4 and explicitly unconfirmed.** §9.1 gives A/X/Y = `$00` and C/Z/D/V/N = 0, then states they "are NESdev measurements, **unconfirmed** by ranks 1–2". The spec's own Open questions repeat the doubt.
  > "The Breaking NES Wiki pages describe the reset *sequence*, not the initial register contents."
- **The `RDY` read/write authority conflict is unresolved.** `dma.md` says `RDY` is effective only on read cycles, while `pads.v` passes `RDY` through combinationally and the read/write dependence lives in `dispatch.v` `ReadyRW`. The spec asks "Which is authoritative for the spec?", so DMA-stall timing has two possible models.
- **Open bus as a hard `$FF` has no HDL path.** `busmux.v` precharges only the internal buses and the only external driver is disabled while `RD=1`; "the HDL gives no path for the precharge to set the external `$FF`; that value is a rank-4 NESdev observation". The emulator must choose a latched last-driven value or a constant `$FF` before the bus has ever been driven.
- **The HDL PC-load defect is recorded but not resolved.** The repo records that "the core never loads a memory-sourced low byte into `PCL` (reset PC becomes `04FF` instead of the vector)"; the spec follows `pc.md` and asks for the HDL fix, so the emulator chooses documented hardware over the HDL model.
- **`Visual_2A03` does not exist as a wiki page.** "No `Visual_2A03` page is known" and the index "lists only `Visual_2C02` and `Visual_circuit_tutorial`"; the die-level reference is a substitute a from-scratch implementer cannot verify.
- **The CPU:PPU phase relationship after reset is not stated.** The specs give the clock ratios (NTSC /12, PAL /16, Dendy /15) and the `PHI1`/`PHI2` structure, but no file says where in the PPU dot grid the CPU's first cycle after reset lands; every cycle-stepped bus interleaving depends on that offset.
- **Interrupt vector addresses are not in the wiki sources.** The vectors `$FFFA`/`$FFFC`/`$FFFE` are credited to NESdev `CPU_interrupts`, `MOS6502_Opcodes` and `Docs`; the instruction file notes the hijacking and delayed-`/IRQ` rows have "no rank 1–3 source in this repository".
- **Reset hold time and the write-suppression model are not pinned.** `/RES` "must be held low for several cycles (NESdev: 6 `M2` clocks)", the three stack pushes are suppressed while S still decrements, and the reset flip-flop is held "because the `R/W` pin control needs it"; the exact hold count and when registers clear are emulator choices.
- **Initial RAM state is unspecified.** Internal RAM and cartridge RAM "have an unreliable state at power-on"; the spec says SHOULD use "a deterministic or random initial pattern" and MUST NOT trust it, leaving the emulator to pick the pattern.

## 3. Instruction set

- **The `JMP ($xxFF)` page-wrap bug is deliberately not specified.**
  > "It is **unconfirmed** by ranks 1–3 and is deliberately not specified above; a `MOS6502_Opcodes/6C.md` walkthrough or an HDL test would settle it."
  The emulator must choose the community-documented same-page high byte or the non-wrapping behaviour with no spec support for either.
- **The undocumented/illegal opcode set is out of scope.** Only `$AB` (ATX/LAX) and `$DF` (DCP) have walkthroughs; "The full undocumented set (aliases, `NOP` variants, unstable behaviour, DMA interaction) is **not enumerated by these sources** and is out of scope." Commercial ROMs use them, so the emulator imports a table from outside.
- **The `$DF` DCP row contradicts silicon.** §6 says T1 "compares A with the original value (`ADD = AC + ~DL + 1`)", but DCP is DEC-then-CMP: the comparison is against `M − 1`. The spec's reading makes every DCP addressing mode fail AccuracyCoin's flags check (error code 5). The emulator follows silicon and AccuracyCoin; the spec row is recorded here as wrong.
- **The unstable stores (`$93/$9F` SHA, `$9B` SHS, `$9C` SHY, `$9E` SHX) and `$BB` LAE/LAS are not specified at all.** Implemented from outside: value = `reg & (H+1)`, high-byte corruption `H & reg` when the index carries, SHS also loading S, and LAE loading `M & S` into A/X/S. AccuracyCoin's SH* suite pins the variant ("behaviour 1"); the DMA interaction that drops the `AND (H+1)` cannot be derived from the spec set because get/put parity is itself a gap (section 8).
- **The dummy-read address on a page cross is not silicon-derived.** §4 uses the un-carried address and warns "confirm against the schematic before treating it as silicon-derived"; because dummy reads are visible to mappers and I/O, the choice changes observable bus traffic.
- **`/PRDY` Branch-T0 skip has no stated purpose.** "Its purpose is unknown (`decoder.md`) and it is not stated whether it also applies to interrupt entries"; the spec only says the emulator "MAY ignore" it, so post-DMA-stall branch timing is a judgement call.
- **`Docs/6502/6502 Instruction Set.htm` contains known errors.** The spec lists `DEC absolute` (`CE`) as 3 cycles instead of 6, wrong mnemonics for `DEX`/`DEY`/`PLP`, a wrong `LSR` N column and N/Z marked modified for `TXS`, then asks "which source is authoritative?" The emulator picks the opcode matrix over the document.
- **NESdev `CPU_instructions` could not be cross-checked.** The local capture "is an empty Wayback landing page (4.6 kB, no article body)", the live page returns HTTP 403 and the Internet Archive was offline, so "the opcode and cycle counts of §2 are **not** textually cross-checked against that page".
- **Interrupt hijacking has no rank 1–3 source.** The `BRK`-hijacked-by-`/NMI` vector switch with the pushed B bit staying 1 is NESdev-only and "needs a schematic/HDL check … before it can be called silicon-derived".
- **The delayed `/IRQ` response has no rank 1–3 source.** `CLI`/`SEI`/`PLP` delaying a pending `/IRQ` by one instruction while `RTI` applies immediately is likewise NESdev-only and unverified against the schematic.
- **Per-register RMW side effects are cross-referenced out.** "The per-register RMW consequences for `$2000–$2007`/`$4014` belong to the PPU and DMA sections", so no single file defines them and the emulator must reconcile the double write with the register semantics itself.

## 4. PPU rendering, sprites and scrolling

- **The H-decoder strobe is one dot earlier than the NESdev dot window.**
  > "`I/OAM2` H 0–63 (clear cycles 1–64), `OBJ_READ` H 256–319 (sprite fetches 257–320), `/FO` H 320–335 (prefetch 321–336)"
  The spec notes it "is not a units conversion but the `dlatch` delay of `HPosLogic`", and no page states the delay in dots.
- **Which latch stage aligns each signal is unstated.** `F/AT` is single-stage while `S/EV`, `E/EV`, `I/OAM2`, `OBJ_READ`, `/VIS`, `#F/NT`, `F/TA` and `F/TB` pass two, and `/FO` is a re-timed two-stage depth; the spec asks whether "one stage also costs a dot". Every fetch, evaluation and clip window shifts by one dot depending on the answer.
- **Shift-register decay is unverified.** NESdev says the dynamic `BGC_*` registers decay to 0 within about two frames, but "the HDL `BGC_*` cells hold their last value, and the decay rate is unverified by ranks 1–3"; the residual content shown by a mid-frame enable is "unspecified".
- **The `EXT`-pin erasure trick is unconfirmed.** "`PPU/mux.md` marks the grounding-`EXT` + slave mode trick 'not confirmed experimentally'." The emulator either treats `EXT` as grounded (backdrop) or implements an unverified mode.
- **Sprite 0 hit on the pre-render scanline is undefined.** "Neither the wiki nor the HDL forbids the hit there; it depends on whether `/VIS` and the residual shift-register content satisfy §6. Needs a test ROM."
- **The sprite overflow walk is NESdev-only and the HDL disagrees.** `PPU/obj_eval.md` says only that the flag sets beyond 8 sprites, while `HDL/PPU/obj_eval.v` "stops evaluation at the 9th in-range sprite outright"; the diagonal `n`/`m` false-positive/false-negative walk is "not restated by ranks 1–2".
- **The exact 9th-sprite detection window and dot budget are not stated.** "Whether the overflow check consumes the rest of the evaluation window or is cut short by `E/EV` at dot 256, and how many `n`/`m` steps fit in the remaining dots, is not stated by any source here."
- **OAM decay timings are explicitly TBD.** `PPU/oam.md` says "Calculate or measure cell degradation timings", and the memory-map file adds "an emulator cannot reproduce decay without them, so model OAM as static for now."
- **`$2004` read behaviour during rendering is NESdev-observed only.** `PPU/oam.md` "describes the buffer and its `R4` latch but not the evaluator's ownership of the address during evaluation"; the HDL drives `n_OAM` from the evaluator, "consistent but not documented as silicon truth".
- **The 2C02G OAMADDR corruption details are absent from the specs.** NESdev reports that an unaligned OAMADDR at evaluation start reinterprets the bytes as Y/tile/attribute/X and treats the sprite at OAMADDR as sprite 0; "no wiki page covers it" and the HDL models it not at all.
- **The OAMADDR&`$F8` row-0 copy is NESdev-only.** "Starting evaluation with OAMADDR ≠ 0 copies the bytes at `OAMADDR & $F8` over OAM row 0"; the HDL drives its own counter from 0, so the emulator must import the behaviour from NESdev and needs a test ROM.
- **Sprite X past the right edge has no re-arm rule.** Sprites at X ≥ 249 keep only the pixels that fall inside dots 0–255; "Whether the die re-arms and drops the remainder on the next line is not stated by the wiki", so "discarded" is only the modelled choice.
- **High-Y wrap-around comparator behaviour is undescribed.** "Whether the die's comparator can wrap and match a high Y byte on the first visible scanlines is not described by `PPU/obj_eval.md` and is not modelled by the HDL."
- **Whether coarse Y = 31 can be held for a full scanline is unsettled.** The measured step path wraps 29 → 0 with the nametable toggle, and 30/31 appear only through a `$2005` Y load of 240–255; "whether silicon can *hold* coarse Y = 31 for a full scanline is not settled by ranks 1–2."
- **`PPU/tilecnt.md`'s count-limit wording conflicts with the measured rule.** The wiki "gives the Tile Vertical limit as 30 and the reset source as 'carry TVZ + 1 TVSTEP'"; read literally that stops the counter at 30, "which the HDL and the `$2005` mapping (coarse Y = `DB[7:3]`) do not support."
- **`$2007` accesses during rendering have no dot windows.** NESdev documents a simultaneous coarse X and Y increment, but "the generated `tilecnt.v` produces only the per-scanline step, so the affected dot windows come from NESdev only."
- **The `$2000` bit-0 race has no reproducible window.** The spec says only that a write straddling the dot-257→258 copy edge "can load open bus into `NTH`"; the emulator invents the exact window and the open-bus value it loads.
- **The `$2006` second-write bus conflict has no reproducible window.** The write "can also be ANDed with the current `v` value on one axis" and "the only safe time to finish the second write is blanking", but no dot range is given.
- **Fine-Y decrement on the pre-render line is probably a simulator artefact.** The measurement shows fine Y dropping 1 → 0 between dots 256 and 257 before the dot-281 reload; `TVSTEP` in `tilecnt.v` has "two mutually-driving `assign` statements, so this is likely a simulator artefact with no program-visible consequence."
- **Cross-file dot labels still need interpreting.** `04-ppu-scrolling.md` §4's `(HDL edge)` rows are one dot later (257/258), and "whether the horizontal copy and the sprite pipeline can skew by more than one dot (affecting sprite 0 hit on the following line) was not verified."
- **`dataread.v` does not exist.** "The Data Reader is spread over `par.v`, `tilecnt.v`, `bgcol.v`, `pamux.v` and `scroll_regs.v`"; an implementer following the named file finds nothing, and the pixel pipeline has no single normative source.

## 5. PPU palette and colour generation

- **The level-per-entry mapping is incomplete.**
  > "the exact level-per-entry mapping is incomplete in ranks 1–2. Emulators should use the canonical NES palette (NESdev `PPU_palettes`) plus the measured levels of §3."
  The emulator must adopt an external 64-colour table to render anything.
- **Greyscale vs colour burst is contradictory.** "`PPU/cram.md`'s signal table describes `B/W` as 'disable Colour Burst'; the HDL clears only the chroma bits and leaves `BURST` running. Which is silicon?" The difference is observable on a TV without burst.
- **The phase table is derived, not stated.** "The phase-per-colour table in §3 is derived from the `PPU/video_out.md` decoder-table permutation; the wiki itself never lists phases per colour code", so absolute hue angles are an unverified reconstruction.
- **Per-revision emphasis factor is not stated.** "`PPU/video_out.md` gives 0.746 for the 2C02; `VDAC.TXT` shows slightly different ratios for 2C07/6538/6528. A per-part emphasis factor is not stated by any source."
- **The DAC level labels are never mapped onto the 64 values.** Levels are named `0D`, `1D`, `2D`, `3D`, `00`, `10` and `20 / 30`, but the spec "never defines how those map onto the 64 palette values", so a composite-accurate renderer cannot be built from spec alone.
- **Palette contents at power-on are unspecified.** "No Breaking NES Wiki page states the initial CRAM contents; the HDL initialises all 32 entries to 0. NESdev treats them as unspecified."
- **RGB PPU colours are not derivable.** "`PPU/rgb.md` is reconstructed from top-layer die photographs only and states that parts are guessed; the RGB decoders' masks are given but not the resulting colours."
- **Direct colour during rendering is untested.** "The Wiki requires rendering off; the HDL gates `TH/MUX` on palette range and rendering-off, but alignment is untested", so the gating and alignment for a `$3Fxx` access made while rendering are emulator choices.
- **The palette write latency is a model choice.** The output latch passes through unequal D-latch chains and "the common 'one dot late' figure is a **model choice**, not a sourced number"; the spec's "apply the new colour on the next dot" is therefore convention, not silicon.
- **Palette readability is revision-gated but the revision is not identified.** The spec says palette RAM is not readable on the 2C02E and earlier and on the 2C03/2C04/2C05 series, and gives `RC2C05-01…04` vs `RC2C05-99` distinctions, but no rule for identifying a revision at runtime; the emulator fixes one by configuration.

## 6. APU

- **`$4015` bit 5 open bus.** The specs use bit 5 both as the DMC-active flag and as open bus, and no file states what a read returns for it; the emulator hard-codes a convention.
- **`$4016`/`$4017` read width.** No source states how many bits the ports drive or which bits are open bus for the whole byte read; only the `$40` detail in bits 5–7 is given, and that itself is NESdev.
- **Test-mode gating.** Whether test mode gates individual channels and which registers it affects is not defined; the emulator either ignores test mode or guesses a mask.
- **`$401B`–`$401F`.** These addresses have no defined read or write behaviour anywhere; the emulator must choose open bus or discard.
- **`LOCK` assertion source contradiction.** The specs name the `LOCK` signal for the channel-lock mechanism but disagree about what asserts it; the emulator picks one source.
- **No PHI2 sub-cycle landing point.** No file says where in the CPU's `PHI1`/`PHI2` half-cycles the APU's timer edges fall, so CPU/APU race outcomes are emulator-defined.
- **No ACLK-to-PHI phase relationship.** The APU channel clock's phase relative to the CPU cycle is unstated; the emulator invents a sampling instant, which changes frame-counter and timer/CPU interactions.
- **Pulse mute threshold t<4 vs t<8.** Two different mute conditions appear in different places with no resolution; the emulator picks one, audibly changing short-period behaviour.
- **Sweep enable polarity.** Whether the sweep enable bit is active high or low (and how it gates the divider) is stated ambiguously; the emulator chooses.
- **First-sweep-update timing.** Whether the first sweep update occurs on the first or second sweep clock is not stated; the emulator fixes it.
- **Sweep divider semantics.** Whether the sweep divider is a period reload or a countdown, and how the target period is computed, is not stated precisely; the emulator implements one reading.
- **The S=0 gloss.** The spec's treatment of the `S = 0` case is a gloss rather than a rule, so the emulator must define what happens for a zero sweep period.
- **Duty-table phase difference.** The duty sequences are given with a phase offset relative to the sequencer start that the specs do not explain; the emulator chooses the alignment.
- **Missing pulse reset/power-on values.** The pulse register file (duty, envelope, sweep, length, timer) has no stated power-on or reset contents; the emulator initialises them arbitrarily.
- **Envelope clock edge and order.** Which ACLK edge clocks the envelope, and whether decay/length clocks precede or follow it, is not stated; the emulator fixes an order.
- **Literal envelope sharing.** The specs describe envelope hardware shared "literally" between functions in a way that cannot be taken as written; the emulator must reinterpret what is shared.
- **Length-counter power-on value.** The initial length is unstated; the emulator chooses it.
- **Discarded length writes.** Writes that arrive when the length counter is disabled or reloading are described inconsistently; the emulator chooses when writes are ignored.
- **Triangle `$400B`/quarter-frame race.** The interaction between a `$400B` write and the quarter-frame clock is not specified; the emulator produces one of several possible results.
- **The ultrasonic analog result.** The analog consequence of an ultrasonic triangle period is not modelled or stated; the emulator either ignores it or approximates it.
- **Missing triangle reset values.** The triangle's linear counter and sequence state have no stated power-on or reset values; the emulator invents them.
- **Noise period table is derived, not printed.** The spec reconstructs the timer periods instead of reproducing a table, so the emulator inherits the derivation and cannot check it.
- **Noise/DMC LFSR feedback taps are absent.** Because the tap positions are not printed, "load-value-to-period mapping cannot be re-derived" from the spec; the emulator takes the LFSR configuration from outside.
- **The letterless noise table is unverified.** The alternate noise table has no entries labelled in the source, so the spec cannot confirm it and the emulator uses it on trust.
- **Noise power-on seed.** The LFSR initial value at power-on is unstated; the emulator picks a seed, changing startup noise.
- **Mode-1 noise sequence length 93 vs 31.** The short-mode sequence length appears as both figures; the emulator picks one.
- **DMC rate tables are derived.** The DMC timer periods are reconstructions rather than a printed table, so the emulator inherits any derivation error.
- **DMC length formula is derived.** The sample-length computation is likewise reconstructed, not sourced, and the emulator cannot verify it.
- **`$4011`/timer race.** The race between a direct load through `$4011` and the DMC timer is unresolved; the emulator fixes a winner.
- **DMC power-on output LSB.** The initial value of the DMC output level's least significant bit is unstated; the emulator chooses.
- **One-byte-late end of playback.** The spec notes playback ends one byte late but gives no rule; the emulator decides the exact end condition.
- **`$4015` bit 4 mid-fetch.** The DMC-active bit's value in the middle of a fetch is not defined; the emulator chooses.
- **Known silicon bugs are not described.** The specs acknowledge that DMC/APU errata exist but do not enumerate them, so bug-compatible behaviour is impossible from spec.
- **DMC disabled-state.** What the DMC channel outputs and how its registers behave when disabled is not defined; the emulator invents a disabled model.
- **ACLK1/ACLK2 half.** The specs refer to a half-rate APU clock without defining which channel edges it drives; the emulator assigns it.
- **Frame-counter total 29828 vs 29830.** Two different total cycle counts appear; the emulator must pick one frame length.
- **Internal 5-step table contradiction (37284 vs 37280 / 29826).** The internal 5-step frame-counter table is given with conflicting values; the emulator chooses one.
- **`$4017` write latency 3 vs 4.** The reset/IRQ latency after a `$4017` write is given as both 3 and 4 cycles; the emulator chooses.
- **IRQ delay offset.** The extra delay between the frame-counter IRQ condition and the CPU observing it is not stated; the emulator invents an offset.
- **Z2 semantics.** The meaning of the `Z2` term in the frame counter is not defined; the emulator assigns it a role.
- **PLA/LFSR structure absent.** The relevant PLA and LFSR internal structure is not documented, so the emulator reimplements behaviour rather than structure and cannot verify corner cases.
- **Dendy frame-counter PLA is unknown.** No source gives the Dendy PAL variant of the frame-counter decode; the emulator reuses PAL or guesses.
- **`cycles.md` images are unusable.** The referenced cycle images could not be read, so the timing they were meant to convey is unavailable and the emulator works from prose.
- **Power-on mode/mask is inferred.** Whether the frame counter starts in 4-step or 5-step mode at power-on, and the initial IRQ mask, are inferred rather than stated.
- **First-activation delay is "roughly".** The spec qualifies the first frame-counter activation delay with "roughly" and gives no exact cycle count; the emulator rounds.
- **PAL LFO frequencies are derived.** The PAL low-frequency oscillator values are reconstructions, so the emulator inherits any derivation error.
- **AUX B table is incomplete.** Only representative entries are given, so a full mixer input table cannot be built from spec.
- **No PCM sample format or rate is specified anywhere.** No spec file defines the output sample format, rate or resampling; the emulator defines its own audio interface.
- **triangle:noise:DMC weight conflict 4:2:1 vs 3:2:1.** Two different weightings appear with no resolution; the emulator picks one and the output balance is arbitrary.
- **Board mixing network absent.** The resistor network that sums expansion audio with the internal channels is not documented, so cartridge audio levels are guessed.
- **ExtRes 100 vs 75 ohm.** Two external-resistor values appear with no resolution; the emulator picks one and accepts output-level error.
- **IntRes lumped approximation.** The internal resistance is only approximated; the emulator inherits the approximation and cannot correct it.
- **No 2A07 DAC difference.** No file states how the PAL 2A07 output stage differs; the emulator reuses the NTSC model.
- **The `/AUX` pin does not exist.** A referenced `/AUX` signal has no pin or net in the described hardware; the emulator drops or renames it.
- **AUX B network TBD.** One AUX mixing network is explicitly marked to-be-determined; the emulator invents its resistor values.
- **AUX A vs B scaling not given.** No relative scaling between the two AUX inputs is given, so expansion-audio level is invented.
- **Dendy is unsupported everywhere.** The APU files provide no Dendy-specific register, clock or frame-counter behaviour; the emulator reuses another region or leaves Dendy unimplemented.
- **M2 duty-cycle conflict.** The `M2` duty cycle is given inconsistently between files; the emulator picks one.
- **`RDY2` is wiki-only.** The `RDY2`/`DBG` debug-wait path exists only in NESdev-derived material; the emulator decides whether to model it.
- **No overall APU power-on state.** Beyond the individual missing register values, there is no defined overall power-on/reset state for the APU; the emulator defines one.
- **PAL tables are derived from mask images.** The PAL APU tables are reconstructions from mask photography, so the emulator inherits unverifiable values.

## 7. Controllers

- **No rank-1 source for the pad protocol at all.** The 4021/40H368 serial protocol appears only through NESdev-derived prose; the spec set gives no HDL or die source for the shift sequence, so the emulator implements it on trust.
- **Select/Start shift order is contested.** The order of Select and Start in the serial stream is disputed between sources; the emulator picks one, changing which button a game reads as which.
- **Famicom pad-2 microphone address.** The address and bit that read the Famicom controller-2 microphone are not given; the emulator invents a mapping or omits it.
- **Famicom pad clock/latch drive.** How the Famicom's hardwired pad is clocked and latched, and whether `$4016` and `$4017` both drive it, is not defined; the emulator chooses a wiring model.
- **PAL protection diodes per model.** `05-boards-and-io.md` lists this under NESdev (`3.6 kΩ`, NES-004E) with an unconfirmed pin tie-off, so which models have the diodes is a model-by-model guess.
- **Which 40H368 is which.** The specs name multiple 40H368 packages without mapping each to a port or signal; the emulator must assign them.
- **How many data bits external devices drive.** The number of bits an external device may drive on the expansion port is not stated; the emulator picks a width.
- **`$4016`/`$4017` in test mode.** Whether the ports behave differently in test mode is undefined; the emulator chooses.
- **Joypad `/OE` across adjacent reads.** The output-enable behaviour when two reads occur back-to-back is not described, so the value returned on a double read is emulator-defined.

## 8. DMA

- **No OAM DMA abort path.** The specs describe the OAM DMA transfer but never how (or whether) it can be aborted or restarted mid-transfer; the emulator picks a model.
- **Get/put parity is indeterminable.** The read/write ordering the DMA controller uses is not derivable from the spec set, yet it fixes the exact cycle on which each byte moves.
- **Conflicting OAM cycle wording.** OAM DMA is given as 513/514 CPU cycles in one place and described with different internal phasing elsewhere; the emulator adopts one count.
- **`RDY` authority.** The read/write-authority conflict of §2 reappears for DMA stalls: the spec does not settle which declaration governs when `RDY` halts the core.
- **Open-bus value during DMA.** What the CPU data bus reads during a DMA cycle is not defined; the emulator invents the value.
- **DMC 3-vs-fixed-4 cycles.** DMC DMA is described both as a 3-cycle and as a fixed 4-cycle operation; the emulator chooses.
- **Load-vs-reload schedule is rank 4.** The DMC address/length load/reload timing is NESdev-derived only; the emulator inherits it unverified.
- **DMC/OAM arbitration window length.** The window in which the DMA unit decides between a DMC and an OAM request is not stated; stall insertion is therefore emulator-defined.
- **Bus-conflict resolution is undefined.** The rules for resolving a CPU/cartridge drive conflict are not characterised (see also §1), so conflicts resolve by emulator choice.
- **2A07 fix mechanism.** The PAL 2A07's remedy for the DMC glitch is named but not described; the emulator must guess how `RDY`/`DBG` produce it.
- **Dendy DMA timing.** No Dendy-specific DMA timing exists in the specs; the emulator reuses another region's model.
- **Aborting a DMC fetch mid-flight.** What happens when a DMC fetch is interrupted by reset, by another DMA, or by disabling the channel is not defined; the emulator chooses.
- **2A07 DPCM glitch and late-G DMC bug.** Both are mentioned but neither is specified in enough detail to reproduce; the emulator either ignores or approximates them.
- **End-of-sample one byte late.** The observed one-byte-late end is noted without a rule, so the emulator must decide the exact end condition.
- **DMC rate table derivation.** The rate table is reconstructed rather than printed; the emulator cannot verify it against a source.
- **DMC length formula derivation.** The sample-length computation is likewise reconstructed, and the emulator inherits any error.
- **`$4011` race and power-on level.** The direct-load/timer race and the initial output level are unresolved; the emulator fixes both.
- **`$4015` bit 5.** The DMA view of the DMC-active bit conflicts with the open-bus use of the same bit; the emulator picks a reading.
- **`$401B`–`$401F`.** These addresses have no defined behaviour, so a DMA or CPU access to them is emulator-defined.
- **No OAM-DMA/DMC-DMA overlap rule.** The specs describe each DMA path separately but never state what happens when both are pending in the same cycle beyond the undefined arbitration window.

## 9. PPU memory map and timing

- **`$3000–$3EFF` mirror is rank 4 only.**
  > "The `$3000–$3EFF` mirror is **rank 4 only**: NESdev says `$3000-3EFF` is 'usually' a mirror of `$2000-2EFF` and never mentions `A12`/`PA12`"
  No rank 1–3 source gives the board decode, so the emulator mirrors it on trust.
- **Mirroring / CIRAM A10 has no wiki source.** "No Breaking NES Wiki page describes nametable mirroring, CIRAM A10 or four-screen cartridges"; the emulator's four nametable modes come from NESdev and board wiring only.
- **`AT_ADR` HDL defect.** `HDL/PPU/tilecnt.v` "ties `AT_adr[6]`, `[7]`, `[8]` and `[9]` to `1'd0`, dropping the `$3C0` attribute-table offset; every attribute fetch would then read tile data instead of an attribute byte"; the spec follows the schematic, so the emulator chooses the schematic over the HDL.
- **Attribute right-edge dots are undocumented.** "The exact dots at which the redundant right-edge fetches cross into the next nametable, and whether the last-row fetch ever uses the merged row-0/4 word line, are not documented in the Wiki."
- **OAM decay is repeated as an open item here too.** `PPU/oam.md` leaves degradation timing TBD, so the memory-map file's instruction to "model OAM as static for now" is a fallback, not a rule.
- **Attribute bits 2–4 read-back is unconfirmed.** The cells have no storage, but "No rank 1–3 source in this repository confirms the read-back value, so an emulator should follow NESdev and return 0"; the HDL models a plain 256-byte array without the `$E3` mask.
- **Palette-read bits 7–6 are open bus vs HDL zeros.** The upper bits of a palette read are described as open bus in one place and modelled as zeros in the HDL; the spec does not resolve which value the emulator returns.
- **`$2005`/`$2006` bus-conflict windows are not quantified.** The write "can also be ANDed with the current `v` value on one axis" and "the only safe time to finish the second write is blanking", but no dot window is given, so the emulator invents one.
- **DMC/`$2007` read conflict has no defined winner.** The VRAM port conflict between a DMC fetch and a `$2007` read is described only as a coupling, and the spec does not say which access wins or what the CPU reads.
- **`$2007` during rendering dot windows come from NESdev only.** The generated `tilecnt.v` "produces only the per-scanline step", so the corrupting coarse-X-and-Y increment and its timing are an outside import.
- **Shift-register decay.** The dynamic background registers decay in about two frames per NESdev, but the HDL holds its last value and the rate is unverified; the emulator chooses whether to decay.
- **`$2004` readability and OAMADDR corruption.** Readable OAM is revision-gated, the 2C02G OAMADDR&`$F8` corruption is NESdev-only, and OAM decay timing is TBD; all three leave revision-accurate OAM undefined.
- **2C05 register-swap polarity.** The swapped PPU register mapping on the RC2C05 is given inconsistently; the emulator picks a mapping.
- **2C05 identifier conflict.** How a 2C05 is identified (and the `RC2C05-99` behaves-like-2C02E case) is not resolved into a rule; the emulator fixes one by configuration.
- **Power-on contents.** The PPU's initial register, VRAM and palette contents are undefined; the emulator defines initial state.
- **Backdrop-override alignment sensitivity.** The direct-colour gate is derivable from the HDL, but "alignment is untested", so the emulator defines an alignment convention for `$3Fxx` accesses.
- **Sprite evaluation window.** The `S/EV`/`E/EV` stage on which evaluation starts and ends "is not stated in dots by any page"; the emulator follows the NESdev 65–256 anchor.
- **Vertical `t`→`v` copy dots are absent from the timing spec.** The copy at dots 280–304 appears only in the scrolling and rendering files, so an implementer reading only the timing file misses a program-visible event.
- **H-decoder strobe vs dot number.** The derived windows are one `PCLK` later than the NESdev dots; the emulator resolves the mapping by convention.
- **`/FO` semantics.** Whether `/FO` means "output enable" or "the segment in which the pattern byte is valid" is unstated; "the pattern phase is modelled as two adjacent 2-dot windows."
- **PAL picture window contradiction (4–255 vs 5–256).** `PPU/pal.md` gives "a picture gate of dots 5–256 (252 dots) rather than the ~277 dots of real PAL active video", while the visible-pixel schedule is 0–255; "the exact PAL blanking window needs hardware confirmation."
- **First-frame parity.** "The Even/Odd flip-flops have no reset in the HDL (`sdff`), and `PPU/fsm.md` does not state their power-on state, so which of the first two frames is short is not derivable from ranks 1–2. NESdev treats it as unpredictable."
- **`VPLA_2` vs `VPLA_8` for `VC`.** The NTSC and PAL counter-clear decodes differ and "the V decoder's ±1-dot simulation spread makes the frame's exact last dot model-dependent"; the emulator cannot pin the frame length exactly.
- **`/RES` hold time and the register-clearing conflict.** `PPU/pads.md` says the reset latch "stays set until `RESCL = 1` (end of vblank)", the HDL "deliberately clears only during the pulse", and the two models disagree; the emulator picks one.
- **The ignored-write window is NESdev-only.** "The ~29 658 / ≈33 132 CPU-cycle write-ignore window is NESdev-only — no Wiki page or HDL gate implements it."
- **Dendy CPU clock is rank 4.** "No rank 1–3 source gives the UA6527P divider"; the emulator adopts NESdev's ÷15 = 1.773448 MHz and 3 dots/cycle.
- **PAL 2C07 reference timing is undocumented.** "The Wiki documents the 2C07 only through `PPU/pal.md` and the UMC 6538 differences; the 71-line vblank figure comes from `Docs/NES_Famicom_Board_Revisions.md` §4."
- **CPU:PPU phase after reset.** No timing file states where the CPU clock sits relative to the dot grid after reset (the same gap as §2); the emulator exposes or invents a phase.
- **Dot-numbering residue.** "The only residue is the one-`PCLK` latch shift of the derived windows in §6", so the emulator must decide whether the accepted convention or the measured edges drive its schedule.
- **Frame-counter/IRQ intervals are not pinned.** The CPU-side frame-counter and IRQ periods sit outside the PPU files read here and are not reconciled with PPU timing; the emulator defines the interleaving.
- **CHR write decoding has no per-board table.** "'Many cartridges only decode reads' is a board-level generalisation; this repository has no per-board table of which boards ignore PPU `/WR`", so write-through behaviour is emulator policy.
- **Interrupt vectors and undocumented opcodes are defined outside the files read.** The vectors and illegal-opcode behaviour live in the CPU/instruction specs (themselves partly NESdev-derived), so a reader of only the memory-map and timing files has no basis for them.

## 10. Frontend / user interface (not covered by the specs at all)

- **Host input binding.** The specification set is a hardware description and says nothing about keyboard/gamepad binding, deadzones or hot-plugging; the emulator defines its own input map.
- **Menu/HUD design.** No spec describes an on-screen menu, pause state, debug overlay or notifications; all of it is emulator-side design.
- **Save-state contents and format.** No file defines what a save state contains, in what order, or how it is encoded. The emulator's format is its own and must be versioned so later changes can be detected; no spec guidance constrains it.
- **Audio sample rate, format and resampling.** Nothing in the spec set fixes an output rate, sample type or resampling filter (the APU files define the chip, not the host interface); the emulator chooses all three.
- **Window/present settings.** Video scaling, filters, aspect ratio, fullscreen and vsync behaviour are not addressed anywhere; the emulator defines them.
- **Configuration persistence.** Where settings are stored, in what format and how they are migrated is undefined; the emulator invents it.
- **Default region and PPU revision.** The specs describe NTSC, PAL and Dendy and several PPU revisions, but give no guidance on which to default to or how to auto-detect; the emulator picks a default.
- **ROM database / compatibility policy.** No spec defines header-vs-database precedence, bad-dump handling or per-ROM overrides; these are emulator policy.
- **Expansion-port peripherals are undefined.** The Zapper, Four Score, Famicom microphone, keyboard and other peripherals have no register-level spec, so host mapping and device behaviour are invented.

## 11. Summary table

Counts are the number of gap entries listed in the corresponding section; some entries bundle several closely related unknowns (the APU section in particular).

| Area | Count of gaps | Highest-impact gap for a from-scratch implementer |
|---|---|---|
| Cartridge/mapper | 14 | iNES/NES 2.0 header layout and mapper register interfaces: without them no ROM can be loaded or banked. |
| CPU core | 9 | Power-on/reset register state and the `RDY` halt authority: both gate the first instruction and every DMA stall. |
| Instruction set | 9 | The undocumented opcode set is out of scope, so real commercial code has no defined behaviour. |
| PPU rendering/sprites/scrolling | 21 | H-decoder strobe vs NESdev dot numbers: every fetch, evaluation and clip window is off by one dot until resolved. |
| PPU palette | 10 | Level-per-entry mapping from the 6-bit value to a colour: only an external canonical palette is available. |
| APU | 61 | No defined mixer weights or board mixing network, so output level and balance are arbitrary. |
| Controllers | 9 | No rank-1 source for the pad protocol at all; even the shift order is contested. |
| DMA | 20 | Get/put parity and OAM-DMA cycle timing: read/write order and DMA duration are not derivable. |
| PPU memory map/timing | 32 | CIRAM A10 / mirroring has no rank 1–3 source, so nametable mapping comes from NESdev alone. |
| Frontend | 9 | Save-state contents/format: entirely emulator-defined and versioned by the emulator. |
