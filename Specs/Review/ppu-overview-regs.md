# Review: ppu-overview-regs — round 2

Files: `Specs/04-ppu-overview.md` (399 lines, `Status: revised (round 2)`) and
`Specs/04-ppu-registers.md` (357 lines, `Status: revised (round 2)`).

## Verdict

consensus

Round-1 blocking 1 and majors 2–4 are resolved, and every round-1 minor is
resolved except one wording nit (`OAM8`, Minor 1 below, does not affect
implementability). No blocking finding remains and no major finding is open.

## Blocking

None.

## Major

None. Resolved since round 1:

- **Blocking 1 (decoder vs VRAM strobes).** `04-ppu-overview.md` §4 now names the
  R/W-decoder outputs `/RD_CPU`/`/WR_CPU`, states they drive the D0–D7 pads only,
  and replaces the wrong normative bullet with "MUST route the R/W decoder's
  `/RD_CPU`/`/WR_CPU` only to the D0–D7 direction logic … the VRAM `/RD`/`/WR`
  pins 24/23 come from the VRAM controller and stay active during rendering with
  `/DBE = 1`". Confirmed against `PPU/regs.md` §"R/W Декодер", `PPU/pads.md`
  lines 57/111–129, `HDL/PPU/regs.v` `RWDecoder`, `HDL/PPU/ppu_top.v`
  (`n_CpuRD`/`n_CpuWR` → `pads.v`; pad `RD`/`WR` ← `VRAM_Control`), and
  `HDL/PPU/vram_ctrl.v` (`WR = ~(w28|TH_MUX)`, `RD = ~w40`, both independent of
  `n_DBE`). Decoder truth table re-derived from `RWDecoder`: read/`/DBE=0` →
  `0/1`; write/`/DBE=0` → `1/0`; `/DBE=1` → `1/1` — matches the §4 table. The
  §4 pin table (23/24 = VRAM strobes) agrees.
- **Major 2 (`PPU_variants`).** Restored in both files (overview Primary sources /
  §6 source note / correspondence; registers Primary sources / §5 / §6 / §9 /
  correspondence / Open questions). `/tmp/nesdev-cache/PPU_variants.html` is the
  real article (68 022 bytes, `<title>PPU variants - NESdev Wiki</title>`) and
  contains "PPUCTRL and PPUMASK swap locations" (RC2C05-01), "OAMADDR works
  correctly, not corrupting OAM contents" (RP2C07), "OAM and palettes are
  unreadable" (RC2C05-99), and the UA6528/UA6538 rows. Both 2C05 Open questions
  now cite it for the swap's existence and reserve only the polarity as unconfirmed.
- **Major 3 (fine X on reset).** `x` is now in both reset tables
  (`04-ppu-overview.md` §7 line 244, `04-ppu-registers.md` §11 line 266) and in
  both normative `RC` bullets. NESdev `PPU_power_up_state`: the reset signal
  clears "the VRAM address latch (T) and the fine X scroll"; `HDL/PPU/scroll_regs.v`
  `Fine_H` drives all three `SCC_FF` bits with `.RC(RC)` (lines 37–39).
- **Major 4 (length / false claim).** `awk 'END{print NR}'` = 399 ≤ 400
  (`Conventions.md` §6); the stale "This file is 398 lines" bullet and the
  `COMPILER.md` 120–300 target are gone; the registers' Length bullet (line 357)
  no longer states a stale number.

## Minor

1. **`04-ppu-overview.md` §6.3 (lines 222–223) over-corrects the 6538 wording.**
   It now says "optional latches were added on `OAM8` and `/PICTURE`", but
   `PPU/umc6538.md` §"Sprite Eval" gives `OAM8` an *additional buffer*
   ("дополнительный буфер"), and only `/PICTURE` is listed with *extra latches*
   ("есть дополнительные защёлки"). Round-1 minor 15 was about `/PICTURE`; the fix
   applied "latches" to both nets. Suggested: "an extra buffer on `OAM8` and extra
   latches on `/PICTURE`". Cosmetic only — no behaviour depends on it.
2. **4 fixed headings, order and line counts all pass** (recorded here as a check,
   not a finding): both files carry exactly `## Observable phenomena`,
   `## Programming / implementation notes`, `## NESdev Wiki correspondence`,
   `## Provenance`, `## Open questions`, unnumbered and in order; 399 and 357 lines.

## Checked against

- `BreakingNESWiki/PPU/regs.md` — §"R/W Декодер": `/RD`/`/WR` "используются для
  контактов внешней шины D0-D7", complementary when `/DBE = 0`, both `1` when
  `/DBE = 1`; §"Специальное примечание" (register access not tied to `PCLK`);
  `CTLR0`/`CTLR1`.
- `BreakingNESWiki/PPU/pads.md` — line 57 (D0–D7 pair from RW Decoder), lines
  111–129 (`/RD`/`/WR` pins from VRAM controller), full pin table (1–19, 21–39;
  no 20/40), reset latch held until `RESCL` (lines 176–178).
- `BreakingNESWiki/PPU/vram_ctrl.md` — "RD: на выход /RD", "WR: на выход /WR".
- `BreakingNESWiki/PPU/scroll_regs.md` + `HDL/PPU/scroll_regs.v` — `$2005`/`$2006`
  field mappings re-derived bit-for-bit (`Fine_H`, `Fine_V`, `Tile_H`, `Tile_V`,
  `NT_Select`) including `t[14] ← 0`, `t[13:12] ← DB5/DB4`, `t[9:8] ← DB1/DB0`,
  `t[7:5] ← DB7/DB5`, `t[4:0] ← DB4/DB0`; `FH` outside `t`; all `.RC(RC)`.
- `HDL/PPU/regs.v` — `RWDecoder`, `RegSelect`, `SCCXFirstSecond`, `RegCTRL0`
  (`I_1_32`/`O_8_16`/`n_SLAVE`/`VBL`), `RegCTRL1` (`BLACK = ~(BGE|OBE)`,
  `/BGCLIP`/`/OBCLIP`/`n_TR`/`n_TG`/`n_TB`), `Clipper`.
- `HDL/PPU/ppu_top.v`, `pads.v` — `n_CpuRD`/`n_CpuWR` only to the D-pad tristate
  logic; pad `RD`/`WR` only from `VRAM_Control`; `assign RC = RES` with the
  deliberate-change comment.
- `HDL/PPU/fsm.v` `VBlankInt` — `res1(VCLR)`, `res2(R2_EN)`,
  `INT = NOR(~VBL_EN, ~int_ff_q)`; `mux.v` `Spr0Hit` and `obj_eval.v` `ovf_reg`
  cleared by `RESCL` only.
- `HDL/PPU/cram.v` line 142 — `{2'b00, cram[CGA]}` (bits 7–6 divergence), now in
  registers *Open questions*.
- `BreakingNESWiki/PPU/rgb.md` — pin table 14→`RED`, 15→`GREEN`, 16→`BLUE`,
  17→`AGND`, 21→`/SYNC`; no `/R4`, no CRAM read; emphasis drives the channel full.
- `BreakingNESWiki/PPU/umc6538.md` — `VPLA_4` +50 lines (`/VSET`), Even/Odd OAM1
  block disabled (265–311), brighter DAC, `OAM8` buffer, `/PICTURE` latches.
- `BreakingNESWiki/PPU/pclk.md`, `Readme.md` (block/bus tables), `Dendy.md`,
  `Docs/NES_Famicom_Board_Revisions.md` §4 (dates incl. 2C02B 1983-12…1984-05,
  OAMDATA readability, OAMADDR corruption, 2C07 emphasis swap).
- NESdev cache `/tmp/nesdev-cache/`: `PPU_variants` (all revision notes above;
  2C05 identifier). `PPU_power_up_state` (fine X cleared, `U??x xxxx`, 29658/33132
  CPU clocks, write suppression). `PPU_registers` (§"Bus conflict" = 2.7.3,
  §"Bit 0 race condition", §"2C05 identifier" table `$3F→$3D`/`$1F→$1C`/`$1F→$1B`,
  §"Reading palette RAM" incl. bits 7–6 open bus and greyscale, §"OAMADDR
  precautions", §OAMDATA rendering writes, §"Read conflict with DPCM samples",
  open-bus decay "after a frame or so"). `PPU_scrolling` (`grep -ci conflict` = 0;
  `$2007`-during-rendering coarse-X + Y increment). `Errata`
  (`[2C02G, 2C03G, and 2C02H]` OAMADDR corruption; PAL emphasis swap).
  `Clock_rate`/`Cycle_reference_chart` (`mw-redirect`; "unswap `$2000` and
  `$2001`"). `Open_bus_behavior` (status port loads bits 7–5 only). `PPU` (links
  once to `PPU_variants`).
- Cross-file: inbound refs to these sections resolve (§2/§3/§4/§5/§7–§10/§12 used
  by `01-platform-overview.md`, `04-ppu-memory-map.md`, `04-ppu-palette.md`,
  `04-ppu-rendering.md`, `04-ppu-scrolling.md`, `04-ppu-sprites.md`); outbound refs
  to `04-ppu-scrolling.md`/`04-ppu-memory-map.md`/`04-ppu-rendering.md` resolve to
  existing files. Register bit fields unchanged and re-verified against the
  Wiki/HDL, so the sections that cite them as authority are unaffected.

## Notes for the next round

- Consensus reached; the residual Minor 1 (`OAM8` buffer vs latch) is cosmetic and
  can be folded into any later wording pass. Per `Conventions.md` §10 this was the
  second and final revision round, so no further round is required.
- **Out-of-scope observation (not a finding against these files):**
  `04-ppu-timing.md` line 379 cites "the same conflict as in `04-ppu-overview.md`
  §12"; `04-ppu-overview.md` has no §12 — the `RC` conflict is in its
  *Open questions* (last, unnumbered, section). The timing file's cross-ref should
  be corrected in its own pass.
- No spec file was edited and no commit was made by this Verifier.

---

# Review: ppu-overview-regs — round 1

Files: `Specs/04-ppu-overview.md` (405 lines, `Status: adapted (round 1)`) and
`Specs/04-ppu-registers.md` (352 lines, `Status: adapted (round 1)`).

## Verdict

revise

## Blocking

1. **`04-ppu-overview.md` §"Programming / implementation notes" (lines 287–288)
   confuses the two `/RD`/`/WR` pairs.** The bullet says "SHOULD implement
   `/RD` and `/WR` as complementary VRAM strobes that are both inactive when
   `/DBE = 1`." The `/RD`/`/WR` of the R/W decoder are the *CPU-interface
   D0–D7 bus enables*, not the VRAM strobes:
   - `BreakingNESWiki/PPU/regs.md` §"R/W Декодер": the decoder's `/RD` and
     `/WR` "используются для контактов внешней шины D0-D7" and are both `1`
     when `/DBE = 1`.
   - `BreakingNESWiki/PPU/pads.md` (/RD, /WR): "Значение контакта /RD
     получается с внутреннего сигнала `RD`, который выходит из
     `[контроллера VRAM](vram_ctrl.md)`".
   - `BreakingNESWiki/PPU/vram_ctrl.md`: "RD: на выход /RD", "WR: на выход
     /WR".
   - HDL: `HDL/PPU/ppu_top.v` names the decoder outputs `n_CpuRD`/`n_CpuWR`
     ("`/RD` to D0-D7 pads (CPU I/F)", lines 30–31, 202–203) and routes
     `vram_ctrl`'s `RD`/`WR` to the pads (lines 174–177, 483–484);
     `HDL/PPU/vram_ctrl.v` asserts them during rendering, i.e. while
     `/DBE = 1`.
   The §4 text and the §4 pin table are correct; only this normative bullet is
   wrong, and it contradicts the file's own pin table. Fix: give the decoder
   pair a distinct name (e.g. `/RD_CPU`/`/WR_CPU`), state that it is the
   D0–D7 direction pair gated by `/DBE`, and state separately that the VRAM
   `/RD`/`/WR` (pins 24/23) come from vram_ctrl and stay active during
   rendering.

## Major

2. **`PPU_variants` exists and was dropped from both files.**
   `/tmp/nesdev-cache/PPU_variants.html` is the real article
   "PPU variants - NESdev Wiki" (68 022 bytes, `oldid=22290`); the `PPU` page
   only *links* to it: `href="/web/20241217204057/.../wiki/PPU_variants"
   title="PPU variants"` (link text "List of known PPU versions and variants").
   The Adapter therefore replaced a real page with `PPU`:
   - `04-ppu-overview.md` line 15–16 ("NESdev `PPU` (its 'List of known PPU
     versions and variants' section)"), line 184–188 (source note for §6), and
     the correspondence row line 322 ("Revision list and per-revision bugs |
     `PPU` (section "List of known PPU versions and variants")").
   - The revision table's RGB dates, `2C03/2C04/2C05` notes, the 2C05
     `$2000`/`$2001` swap, the 2C07 "OAMADDR works correctly" and the
     `UA6528`/`UA6538` notes are all on `PPU_variants`, not on `PPU`.
   - Consequence for *Open questions*: `04-ppu-overview.md` lines 371–380 and
     `04-ppu-registers.md` line 350 claim the swap is only "implied by
     `Clock_rate`". `PPU_variants` states it directly ("PPUCTRL and PPUMASK
     swap locations", RC2C05-01), so that open question must be rewritten
     (polarity is still undetermined, existence is not).
   - `04-ppu-registers.md` §5/§6 (lines 144, 157) also need the page for
     per-revision OAMADDR/OAMDATA/palette readability.
   Sibling sections already use the correct name (`04-ppu-timing.md` line 344,
   `04-ppu-palette.md` line 349), so this is an isolated oversight; see
   `Specs/Review/agents/mechanical-fixes.md` item 5 (still "pending").
   *Other page names checked, none dropped for the same reason:* `PPU`,
   `PPU_registers`, `PPU_power_up_state`, `Open_bus_behavior`, `PPU_pinout`,
   `Visual_2C02`, `PPU_rendering`, `Errata`, `PPU_scrolling`, `PPU_OAM`,
   `Clock_rate` all resolve in the cache; `NMI` is real (linked from the `PPU`
   page) though not cached locally.

3. **Fine X (`x`) is missing from the reset side effects in both files.**
   `04-ppu-overview.md` §7 table (lines 238–247) has rows for `w`, `t`, `v`
   but none for `x`; its normative note line 299–301 lists `$2000`, `$2001`,
   `t`, `w`, read buffer. `04-ppu-registers.md` §11 table (lines 260–268) and
   line 270 ("`RC` clears `$2000`, `$2001`, `t`, `w` and the `$2007` read
   buffer") do the same. Evidence that `x` *is* cleared:
   - NESdev `PPU_power_up_state`: the internal reset signal "clears PPUCTRL,
     PPUMASK, PPUSCROLL, PPUADDR, the PPUSCROLL/PPUADDR latch, and the PPUDATA
     read buffer. (Clearing PPUSCROLL and PPUADDR corresponds to clearing the
     VRAM address latch (T) **and the fine X scroll**.)"
   - `HDL/PPU/scroll_regs.v` `Fine_H`: all three `SCC_FF` bits take `.RC(RC)`.
   Both files' own §10 tables assign `FH` (fine X) to `x`, three bits outside
   `t`, so §7/§11 must add `x` to the cleared set.

4. **`04-ppu-overview.md` is 405 lines and its self-report is wrong.**
   `awk 'END{print NR}'` = 405; `Conventions.md` §6 sets 120–400 lines.
   Line 401 of the same file says "This file is 398 lines, inside the 120–400
   range" — stale and false, and it cites a `COMPILER.md` 120–300 target that
   is not the binding contract. Trim ≥6 lines or split §6, and correct/remove
   the note.

## Minor

5. `V??x xxxx` should be `U??x xxxx` (U = unchanged) in both power-on tables:
   `04-ppu-overview.md` line 241 and `04-ppu-registers.md` line 263. NESdev
   `PPU_power_up_state` table row `PPUSTATUS ($2002) | +0+x xxxx | U??x xxxx`
   with the legend "U = unchanged".
6. `04-ppu-registers.md` line 220 cites `PPU_scrolling` §"Bus conflict". There
   is no such text on `PPU_scrolling` (`grep -ci conflict` on the cached page =
   0); the subsection is `PPU_registers` §2.7.3 "Bus conflict".
7. `04-ppu-registers.md` line 270 says `RC` clears registers "while `/RES` is
   asserted", which contradicts the held-until-`RESCL` model used by the next
   sentence and by the file's own normative note (line 307) and *Open
   questions*. Reword to "while `RC` is active (held until `RESCL`)".
8. `04-ppu-overview.md` line 46: "The only signal the PPU sends to the CPU is
   `/INT`". The PPU also drives D0–D7 during register reads (the file's own §4
   pin table marks D0–D7 bidirectional; `PPU_pinout` names pins 2–9 "CPU
   D0..D7"; `01-platform-overview.md` §3 connects the CPU data bus to the PPU
   registers). Say "the only dedicated control/interrupt line".
9. `04-ppu-registers.md` §10 table line 241 lists `FH` inside the `t` row while
   §7 line 174 assigns fine X (`FH`) to `x`, "3 bits, outside `t`". Keep `FH`
   out of the `t` row.
10. Revision-dependent claims should name `PPU_variants` (see Major 2), and
    `04-ppu-registers.md` §5 line 144 ("The 2C03, 2C04, 2C05 and 2C07 are not
    affected") conflicts with `Errata` (already cited by the file), which lists
    "2C02G, 2C03G, and 2C02H" for OAMADDR corruption; §9 line 232's
    non-readable list omits the 2C05 series (`PPU_variants`: RC2C05-01…04
    behave like 2C03; RC2C05-99 has 2C02E behaviour).
11. `04-ppu-registers.md` line 233: TH/MUX is qualified "with rendering
    disabled", but the HDL condition is `BLNK & (PA13:8 = $3F)`
    (`HDL/PPU/vram_ctrl.v` `TH_MUX`), and `BLNK` is also active during vblank;
    the sibling `04-ppu-memory-map.md` §6 omits the qualifier. Align the three.
12. `04-ppu-registers.md` line 232 claims palette reads return PPU open bus in
    bits 7–6 (NESdev `PPU_registers` §"Reading palette RAM"), but
    `HDL/PPU/cram.v` line 142 drives `{2'b00, cram[CGA]}`. The Provenance row
    lists `cram.v` for palette reads; either model the open bus or record the
    HDL divergence in *Open questions* per `Conventions.md` §3.
13. The capacitive-DB model is stated as "stays there" (`04-ppu-overview.md`
    §3 lines 84–90, `04-ppu-registers.md` line 120). NESdev `PPU_registers`
    adds that the value "begins to decay after a frame or so"; one sentence
    makes the latch-vs-decay tradeoff explicit for implementers.
14. `Clock_rate` is a redirect to `Cycle_reference_chart` (`PPU_variants` links
    it as `class="mw-redirect"`); the mechanical-fixes decision is to cite
    `Clock_rate` with a one-line redirect note. Neither file has it
    (`04-ppu-overview.md` line 323, `04-ppu-registers.md` line 321).
15. Fidelity nits: `04-ppu-overview.md` line 24–26 says the RGB parts "replace
    `VOUT` and the four EXT pins with `RED`, `GREEN`, `BLUE` and `/SYNC`" — per
    `PPU/rgb.md` EXT3 becomes AGND (not a signal) and pin 21 becomes `/SYNC`;
    §2 line 64 labels the Data Reader `("PatGen")` although the same table
    lists PatGen separately as a DB user; §6.3 line 222 calls the 6538
    `/PICTURE` extra latches "buffers" (`PPU/umc6538.md`: "дополнительные
    защёлки").

## Checked against

- `BreakingNESWiki/PPU/Readme.md` — block list, DB/PD/OB bus table and the
  capacitive-DB warning; revision list. Bus table and DB rule reproduced
  correctly.
- `BreakingNESWiki/PPU/regs.md` — R/W decoder (CPU D0–D7 pair), register select,
  shared `$2005`/`$2006` latch cleared by `RC`/$2002, `CTLR0`/`CTLR1`, clipping
  chain, "access is not tied to PCLK". All confirmed; the `/RD`/`/WR`
  distinction is the Blocking 1 finding.
- `BreakingNESWiki/PPU/scroll_regs.md` — Fine H/V, NT select, Tile V/H tables.
  Every bit mapping in `04-ppu-registers.md` §7/§8/§10 matches bit-for-bit,
  including `$2006` 1st write setting `t[9:8]` from DB1–DB0, `t[11:10]` from
  DB3–DB2, `FV[2]` forced 0.
- `BreakingNESWiki/PPU/pads.md` — pin table (pins 1–19, 21–39; 20/40 absent),
  `/RES` latch held until `RESCL`, `RC`, PD-as-address-byte float.
- `BreakingNESWiki/PPU/fsm.md` — `/VSET` edge detector, `INT_FF`, `RESCL/VCLR`,
  read-clears-flag, EVEN/ODD.
- `BreakingNESWiki/PPU/rails.md` — register strobes `/W0 /W1 /R2 /W3 /W4 /R4
  /W5/1 /W5/2 /W6/1 /W6/2 /R7 /W7`; `I1/32`, `#I1/32`, `O8/16`, `#O8/16`;
  `OBSEL`, `BGSEL`, `/TR /TG /TB`, `B/W`, `BLACK`, `TH/MUX`, `DB/PAR`.
- `BreakingNESWiki/PPU/visual2c02.md` — all quoted node names (`io_rw` 1224,
  `io_db0-7`, `_io_db0-7`, `_db0-7`, `spr_d0-7`, `pclk0`/`pclk1`,
  `/ZCOL0` 1680, `pal_ptr0-4`).
- `BreakingNESWiki/PPU/oam.md` — 2112-bit OAM (64×29 + 256), `R4` latch, OB
  write source `BLNK=1`; `R4`/`/W4` signals.
- `BreakingNESWiki/PPU/cram.md`, `vram_ctrl.md`, `mux.md`, `par.md`,
  `pclk.md`, `rgb.md`, `umc6538.md` — read buffer, TH/MUX, palette bypass,
  RGB differences, 6538 `/VSET`/Even-Odd/DAC/OAM8 differences.
- `HDL/PPU/regs.v` — `RWDecoder`, `RegSelect`, `SCCXFirstSecond` (`res1(RC)`,
  `res2(R2)`), `RegCTRL0` (all 8 bits incl. `I_1_32`/`O_8_16` polarity),
  `RegCTRL1` (`BLACK = ~(BGE|OBE)`), `Clipper`.
- `HDL/PPU/scroll_regs.v` — `FH`/`FV`/`TH`/`TV`/`NT` write sources and `RC`.
- `HDL/PPU/fsm.v` — `VBlankInt`: `res1(VCLR)`, `res2(R2_EN)`,
  `INT = NOR(~VBL_EN, ~int_ff_q)`, `DB7Out` gated by `R2_EN`. Confirms a `$2002`
  read clears `w` **and** the vblank/NMI latch but not the sprite flags.
- `HDL/PPU/mux.v` `Spr0Hit`, `HDL/PPU/obj_eval.v` (`ovf_reg` cleared by
  `RESCL` only) — confirms `$2002[6]`/`[5]` are not cleared by a read.
- `HDL/PPU/vram_ctrl.v` — read buffer (`ReadBuffer`, `PD_RB`/`XRB`),
  `TH_MUX = BLNK & PA[13:8]=$3F`, `ALE`, `RD`/`WR` (VRAM strobes).
- `HDL/PPU/pads.v` — D0–D7 tristate direction from the CPU `/RD`,`/WR` pair;
  `RC = RES` and its deliberate-change comment.
- `HDL/PPU/ppu_top.v` — `n_CpuRD`/`n_CpuWR` vs pad `RD`/`WR` routing; block
  instantiations.
- `Docs/NES_Famicom_Board_Revisions.md` §4 (RP2C02 1983-06 … RP2C07A-0, OAMDATA
  readability, OAMADDR corruption, emphasis swap), §2.2 (74LS139/74HC139 at
  U3), §5.2 (139 decoder roles), §5.3.
- NESdev cache `/tmp/nesdev-cache/`: `PPU`, `PPU_variants`, `PPU_registers`,
  `PPU_scrolling`, `PPU_power_up_state`, `PPU_pinout`, `Open_bus_behavior`,
  `PPU_OAM`, `Errata`, `Clock_rate`, `Visual_2C02`.
- **Dot-257 PPUCTRL bit-0 race — confirmed fixed.** `04-ppu-registers.md`
  lines 54–61 now match `PPU_registers` §"Bit 0 race condition": "using a
  horizontal nametable arrangement (a.k.a. vertical mirroring) or 4-screen
  VRAM", "Only writes that start on dot 257 and continue through dot 258",
  "writes the value of open bus to the register", "only the next scanline",
  "no effect in horizontal or one-screen mirroring", and the `$2100` mirror
  workaround "with the desired nametable". (The NESdev wording itself contrasts
  "horizontal nametable arrangement" with "horizontal ... mirroring"; the spec
  reproduces both faithfully, so this is only a readability nit.)
- **v/t/x layout cross-read.** `04-ppu-scrolling.md` §1 (`v[4:0]`, `v[9:5]`,
  `v[11:10]`, `v[14:12]`, fine X outside `v`; full 15-bit address, `v[14]`
  never on `/PA0-13`), `04-ppu-memory-map.md` §5/§6/§7, and
  `04-ppu-registers.md` §7/§8/§10 agree field-for-field; `$2006` fine-Y clamp,
  coarse-Y 29→0 nametable toggle, `$2007`-during-rendering coarse-X+Y increment
  and palette bypass are consistent across all three.
- `Conventions.md` §6/§7 and `_template.md` — five fixed headings present,
  exact, unnumbered and in order in both files; correspondence and provenance
  tables present and populated.

## Notes for the next round

- Fix the Blocking item first (one bullet + a signal rename); it is the only
  finding that would make an implementer build the wrong hardware.
- The `PPU_variants` rename is mechanical and already tracked globally
  (`Specs/Review/agents/mechanical-fixes.md` item 5). While there, rewrite the
  2C05 *Open questions* in both files: `PPU_variants` documents the swap's
  existence (not its polarity) and the 2C05 identifier caveat.
- Adding `x` to the reset tables changes both files' line counts;
  `04-ppu-overview.md` needs a net trim of ≥6 lines anyway (Major 4), so do the
  two edits together.
- Re-read `04-ppu-scrolling.md` / `04-ppu-memory-map.md` after editing if the
  wording of the vertical-copy and TH/MUX sentences changes; the three files
  must keep the same dot/qualifier wording.
- No spec file was edited and no commit was made by this Verifier.

## Revision round 2

Files: `Specs/04-ppu-overview.md` (399 lines, `Status: revised (round 2)`) and
`Specs/04-ppu-registers.md` (357 lines, `Status: revised (round 2)`).

- **Blocking 1: fixed** (`04-ppu-overview.md` §4 + implementation notes). The
  decoder table now names its outputs `/RD_CPU`/`/WR_CPU`, states they drive the
  D0–D7 pads only (`HDL/PPU/regs.v` `RWDecoder`; `HDL/PPU/ppu_top.v` names them
  `n_CpuRD`/`n_CpuWR` and routes them to `pads.v`), and the normative bullet is
  replaced by "MUST route the R/W decoder's `/RD_CPU`/`/WR_CPU` only to the
  D0–D7 direction logic … the VRAM `/RD`/`/WR` pins 24/23 come from the VRAM
  controller and stay active during rendering with `/DBE = 1`" (`PPU/regs.md`
  §"R/W Декодер", `PPU/pads.md`, `PPU/vram_ctrl.md`, `HDL/PPU/ppu_top.v`).
  The §4 pin table (pins 23/24) was already correct and now agrees.
- **Major 2: fixed.** `PPU_variants` restored in `04-ppu-overview.md` (Primary
  sources, §6 source note, correspondence row) and in `04-ppu-registers.md`
  (Primary sources, §5, §6, §9, correspondence, Open questions). The 2C05
  `$2000`/`$2001` Open question in both files now says `PPU_variants` states the
  swap directly ("PPUCTRL and PPUMASK swap locations", RC2C05-01) and that only
  the polarity is undetermined; `Clock_rate` is cited only as the board-side
  corroboration.
- **Major 3: fixed.** Fine X `x` added to both reset tables (overview §7,
  registers §11), to the overview's normative `RC` bullet, and to the registers'
  §11 sentence plus a new implementation-note bullet (`NESdev
  PPU_power_up_state`; `HDL/PPU/scroll_regs.v` `Fine_H`, `.RC(RC)`).
- **Major 4: fixed.** `04-ppu-overview.md` trimmed 405 → 399 lines by
  compressing §1/§6.2/§6.3/§7 and several Open questions; no sourced fact
  removed. The false "This file is 398 lines" bullet was deleted (the stale
  `COMPILER.md` 120–300 target went with it). `04-ppu-registers.md`'s own
  length bullet was reworded to drop its now-stale number.
- **Minors: applied.** `V??x xxxx` → `U??x xxxx` (both); `PPU_scrolling`
  §"Bus conflict" → `PPU_registers` §"Bus conflict" (registers §8); "while
  `/RES` is asserted" → "while `RC` is active (held until `RESCL`)" (registers
  §11); "the only signal the PPU sends to the CPU" → "the only dedicated control
  line" plus a D0–D7-reads clause (overview §1); `FH` removed from the `t` row
  in registers §10; revision-dependent claims now name `PPU_variants` and the
  OAMADDR affected list follows `Errata` (2C02G, 2C03G, 2C02H) while the 2C07 is
  excluded; the `$2004`/palette non-readable lists now include the 2C05 series;
  TH/MUX is stated against the HDL condition `BLNK & PA[13:8] = $3F`, not just
  "rendering disabled"; the `cram.v` `{2'b00, …}` palette-read bits 7–6
  divergence is recorded in registers *Open questions*; the DB "begins to decay
  after a frame or so" note added to both files; a one-line `Clock_rate`
  → `Cycle_reference_chart` redirect note added below both correspondence
  tables; RGB pins corrected to `RED`/`GREEN`/`BLUE`/`AGND` + `/SYNC` on pin 21
  (`PPU/rgb.md`), the Data Reader's bogus `("PatGen")` label removed, and the
  6538 `/PICTURE` "buffers" reworded to "latches" (`PPU/umc6538.md`
  "дополнительные защёлки").
- No change was made to any file other than the two specs above and this report
  (append only). No commit was made.
