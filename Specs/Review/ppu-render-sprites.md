# Review: ppu-render-sprites — round 3

## Verdict
consensus

## Blocking

None. Round-2 blocking 1 is fixed and now agrees with all four cited sources:
`04-ppu-sprites.md` §6 (lines 234–236) gives the 8×8 column bit 4 = "tile byte
bit 0 (tile index)", bits 5–11 = "tile byte bits 1–7", bit 12 = `OBSEL`
(PPUCTRL bit 3) — `OBSEL` now appears exactly once in the table — and keeps the
8×16 column bit 4 = `OV3`, bit 12 = tile byte bit 0. Evidence:
- `BreakingNESWiki/PPU/par.md` rows 4 / 5–11 / 12 (identical in
  `BreakingNESWiki_DeepL/PPU/par.md`): bit 4 OB 8×8 = "OAM2, Tile Index Byte,
  бит 0", OB 8×16 = "Схема сравнения спрайтов OV3"; bit 12 = `OBSEL`.
- `HDL/PPU/par.v`: `ParBit4` (96–102) selects `val_OB` on `PADx` for
  `OBJ_READ = 1, O8_16 = 0` (instantiation 211–219, `.val_OB(OB[0])`,
  `.val_OBPrev(OV[3])`), i.e. bit 4 = tile bit 0 in 8×8 and `OV[3]` in 8×16;
  `ParControl` (49–53) puts `OBSEL` only on `PAD12`.
- `04-ppu-memory-map.md` §2 (lines 41–43) and `04-ppu-rendering.md` §3 (lines
  131–133) print the same three columns; the two files under review no longer
  contradict each other on the common 8×8 case.

## Major

None. All round-1 majors (rendering §3 BG `PAT_ADR` source, §1 latch rationale,
§7 odd-frame mechanism, sprites §1 attribute diagram, sprite-0 exclusions,
overflow walk) remain resolved in substance, and round 2 recorded no majors.

## Minor

Residual, non-blocking (final round — recorded, not used to keep the section
open):

1. **`04-ppu-rendering.md` *Open questions* line 380 — the `/FO` stage count.**
   The bullet singles `/FO` out as passing "three stages", which disagrees with
   §1 line 33 ("one or two `dlatch` stages") and with the +1-dot shift the §1
   table applies to `/FO` (rows 14–15: H 320–335 / 0–255 → dots 321–336 /
   1–256). In `HDL/PPU/fsm.v`, three `dlatch` cells feed `n_FO` (`fo_latch1`
   464, `fo_latch2` 465, `fo_latch3` 466), but the serial depth is two:
   `HPLA_14`/`HPLA_15` each pass `fo_latch1`/`fo_latch2`, then the shared
   `fo_latch3` latches the combinational `fo3 = nor(fo_latch1_q, fo_latch2_q)`
   (484) and `n_FO = fo_latch3_nq` (498). "three `dlatch` cells" would be
   accurate; "three stages" and "one or two" cannot both stand. The same
   citation is stale by one line: the three `fo_latch*` cells are 464–466 (467
   is `fat_latch1`), not 465–467. Effect: the implementable schedule (§1 table,
   §2) is unaffected — rationale/traceability precision only.

## Round-2 findings — round-3 confirmation

- **Blocking 1 (`PAT_ADR` 8×8 bit 4)** — fixed; see above.
- **Minor 1 (`/VIS` exception)** — fixed. §1 lines 36–39 now apply the +1 dot to
  "every multi-latch signal **except** `/VIS`" and state the exception inline
  (NESdev anchor 0–255; a sprite at X = 0 must show a pixel on dot 0), so the
  rule no longer contradicts the `/VIS` row (line 52) or the OQ (387–388).
- **Minor 2 (completed multi-latch list)** — fixed. §1 line 37 and the OQ
  (378–380) now name `0/HPOS`, `/EVAL` and `/FO` alongside `S/EV`, `E/EV`,
  `CLIP_O`/`CLIP_B`, `I/OAM2`, `OBJ_READ`, `#F/NT`, `F/TA`, `F/TB`; HDL
  confirmed (`hpos_latch1/2` 443–444, `eval_latch1/2` 447–448). Residual: the
  `/FO` stage count (minor 1 above).
- **Minor 3 (one evaluation-window statement)** — fixed. Rendering §1 line 40
  and OQ 381–382, sprites §2 line 73 and OQ 384–385 all say NESdev cycles
  65–256 with the latched `S/EV` edge at dot 66 and `E/EV` dot 256; the stale
  "`S/EV` H 65 / `E/EV` H 255 (65–256)" pairing is gone, and the two OAMADDR
  mentions read dot 66 / NESdev cycle 65 (sprites lines 86, 278).
- **Minor 4 (`/EVAL` is a level)** — fixed. Sprites §2 lines 78–80 describe
  `n_EVAL` as a level, low only at H = 63/255/339, whose rising edge ends the
  window; matches `fsm.v` 447–449 (`eval_latch2` input
  `~(hpos_latch1_q | eval_latch1_q | eev_latch1_q)`) and `obj_eval.v` 233–245
  (`n_eval_d == 0 && n_EVAL == 1`).
- **Minor 5 (either clip)** — fixed. Sprites §7 item 5 line 258: "With either
  left-column clip active (bit 1 or bit 2 = 0) the earliest possible hit column
  is x = 8"; matches NESdev `PPU_OAM` §"Sprite 0 hits" ("At x=0 to x=7 if the
  left-side clipping window is enabled (if bit 2 or bit 1 of PPUMASK is 0)").
- **Minor 6 (dropped dot)** — fixed. Rendering §7 lines 277–280: the counter
  clears at H = 339, so the pre-render line is 340 dots and the dropped dot is
  dot 340, the dot on which `HPLA_23` would clear. Matches `EvenOdd`
  (`fsm.v` 250) and `HVCounterControl` (`HC = HPLA_23 | EvenOddOut`, 324).
- **Minor 7 (bug-ending signal)** — fixed. Sprites §4 line 172 quotes the NESdev
  cause section's ending: "another signal prevents the comparison from
  influencing the sprite address incrementation logic"; the write-disable is no
  longer named as the end (`PPU_sprite_evaluation` §"Cause of the sprite
  overflow bug").
- **Minor 8 (cross-file dot 256/257)** — recorded. Rendering *Open questions*
  lines 397–399 state the §2 (dot 256) vs `04-ppu-scrolling.md` §4 (its dot 257,
  explicitly = NESdev dot 256) disagreement for the final global pass.
- **Housekeeping** — the folded OQ material is preserved (the `#F/NT`/`F/AT`
  placement and `/FO`-semantics sentences, lines 378–386), and the compressed
  PAL note still holds: I re-diffed `hv_decoder.v` RP2C02 vs RP2C07 and only
  `dec_out[0]` and `dec_out[17]–[22]` differ (`HPLA_0` and `HPLA_17–22`).

## Checked against

- `BreakingNESWiki/PPU/par.md` and `BreakingNESWiki_DeepL/PPU/par.md` — full
  bit-usage table rows 4, 5–11, 12, BG / OB 8×8 / OB 8×16.
- `HDL/PPU/par.v` — `ParControl` 49–53, `ParBit4` 96–102, PAR instantiations
  206–228; `OBJ_READ`/`O8_16` selection and `PAD12`.
- `Specs/04-ppu-memory-map.md` §2 (lines 41–43), §4, §5 (address formulas) —
  same `PAT_ADR` bit sources.
- `Specs/04-ppu-rendering.md` §1 (31–61), §2 (75–105), §3 (127–135), §6, §7
  (251–286), programming notes, *Open questions* (367–400).
- `Specs/04-ppu-sprites.md` §1 (33–52), §2 (65–87), §3, §4 (158–174), §5.2, §6
  (226–238), §7 (240–261), phenomena, *Open questions* (353–400).
- `HDL/PPU/fsm.v` — `HPosLogic` latch block 439–468 (`sev`, `clpo/clpb`,
  `hpos`, `eval`, `eev`, `ioam`, `objrd`, `nvis`, `fnt`, `ftb`, `fta`, `fo`,
  `fat`) and assignments 493–504; `EvenOdd` 235–251; `HVCounterControl`
  312–330 and `_2C07` 333–349.
- `BreakingNESWiki/PPU/hv_decoder.md` — HPLA table rows 2, 3, 5, 6, 7, 8, 9,
  10, 11, 12, 13, 14, 15, 16, 23 values and signal meanings (`/EVAL` also on
  HPLA_5/6/7).
- `HDL/PPU/hv_decoder.v` — RP2C02 vs RP2C07 `HDecoder` diff (only `HPLA_0`,
  `HPLA_17–22`).
- `HDL/PPU/obj_eval.v` — `n_EVAL` 0→1 edge as end-of-window (233–245).
- NESdev via the Wayback Machine (Dec 2024): `PPU_sprite_evaluation` (cycles
  1–64 / 65–256 / 257–320, steps 3a/3b, "Cause of the sprite overflow bug"),
  `PPU_OAM` (Byte 1/2 layout, `$EF`–`$FF`, X `$F9`–`$FF`, sprite 0 exclusions
  incl. x = 255 and "bit 2 or bit 1 ... is 0", `$E3`).
- Sibling specs: `04-ppu-memory-map.md` §2/§4/§5/§8; `04-ppu-registers.md`
  §2–§5, §9, §12; `04-ppu-scrolling.md` §4 and *Open questions* — every cited
  section number resolves.
- Template/Conventions: both files `Status: revised (round 3)`, exactly 400
  lines (inside the 120–400 target), the five fixed headings present,
  unnumbered and in order; `04-ppu-palette.md`/`04-ppu-timing.md` exist.

## Notes for the next round

- **Consensus reached.** No blocking findings; all majors are resolved. The one
  residual minor above (`/FO` stage count / stale HDL line range) does not
  misdirect an implementer and is recorded here rather than reopening the
  section.
- `04-ppu-sprites.md` §7 item 4 line 257 pairs "`BLACK = 0`" with "PPUMASK bits
  3 *and* 4 set". The operative requirement (both layers) is correct and fixes
  the round-2 major-5 wording; note only that bits 3 and 4 set already imply
  `BLACK = 0` (`04-ppu-rendering.md` §7 line 253), so the clause is redundant
  rather than wrong — a future edit could read "bits 3 and 4 set, hence
  `BLACK = 0`".
- The cross-file dot-256/257 item is correctly parked in
  `04-ppu-rendering.md` *Open questions*; it belongs to the final global pass
  together with the sibling `04-ppu-scrolling.md` §4 note.
- This round edited only this review file; neither spec file was modified and no
  `git commit` was made.

---

# Review: ppu-render-sprites — round 2

## Verdict
revise

## Blocking

1. **`04-ppu-sprites.md` §6 — the `PAT_ADR` table's 8×8 column gives bit 4 as
   `OBSEL`; it must be the tile-index byte's bit 0.** Lines 230–236 print
   `| 4 | OBSEL (PPUCTRL bit 3) | OV3 … |` and `| 12 | OBSEL (PPUCTRL bit 3) |
   tile byte bit 0 |`, with bits 5–11 = "tile byte bits 1–7". The 8×8 column
   therefore uses `OBSEL` twice and never places tile-index bit 0 anywhere,
   although the same file's §5.1/§6 prose (lines 181, 238) and the rendering
   file's table say the tile byte supplies bits 4–11. Sources:
   - `BreakingNESWiki/PPU/par.md` line 32: `|4|Name Table, бит 0|OAM2, Tile
     Index Byte, бит 0|Схема сравнения спрайтов OV3|Индекс в Pattern Table|`
     and line 34: `|12|BGSEL ($2000)|OBSEL ($2000)|OAM2, Tile Index Byte,
     бит 0|Выбор Pattern Table|`; identical in
     `BreakingNESWiki_DeepL/PPU/par.md` lines 32–34.
   - `HDL/PPU/par.v`: `ParBit4 u6 (… .val_OB(OB[0]), .val_PD(PD[0]),
     .OBJ_READ(OBJ_READ), .O8_16(O8_16), .val_OBPrev(OV[3]),
     .PADx(PAddr_out[4]))` (lines 211–219) — with `OBJ_READ = 1, O8_16 = 0`
     the cell selects `OB[0]`; `ParControl` lines 49–50 drive
     `PAD12 = OBJ_READ ? (O8_16 ? OB[0] : OBSEL) : BGSEL`.
   - The sibling specs already state it correctly:
     `04-ppu-rendering.md` §3 line 131 (`secondary-OAM tile byte bit 0` on
     bit 4) and `04-ppu-memory-map.md` §2 line 41 (`OAM2 tile-index bit 0`).
   Effect: an implementer copying §6 addresses every 8×8 sprite pattern with
   PPUCTRL bit 3 on `PAT_ADR[4]` and drops the tile index's low bit, so the two
   files under review contradict each other on the common case. This is not a
   round-2 regression, and round 1 missed it (its *Checked against* recorded
   "spec §6 correct", which holds only for the 8×16 column); it is the same
   class of one-cell `PAT_ADR` error as round-1 blocking 1.

## Major

None. All five round-1 majors are resolved in substance (confirmation below);
the residual defects behind major 2 are recorded as minors 1–3.

## Minor

1. **`04-ppu-rendering.md` §1 lines 36–37 vs the table rows.** The paragraph
   says the dot column "applies that +1 dot to every two-latch signal (…
   `/VIS` …); `F/AT` alone is single-stage", but the `/VIS` row (line 51) is
   left at 0–255 while the single-stage `F/AT` row (line 56) is shifted from
   H ≡ 2,3 (mod 8) to dots ≡ 3,4 (mod 8). *Open questions* lines 379–383
   explains each exception individually, but the stated rule does not match
   the table. Either drop `/VIS` from the list or say the column mixes the
   NESdev anchor with the per-signal latch shift.
2. **`04-ppu-rendering.md` §1 line 37 — the two-latch list is incomplete.**
   `Z_HPOS` (0/HPOS, line 47), `/EVAL` (line 48) and `/FO` (line 55) are also
   multi-latch: `hpos_latch1/2` (`fsm.v` 443–444), `eval_latch1/2` (447–448),
   `fo_latch1/2/3` (465–467). Their rows are shifted correctly, but the
   parenthetical reads as exhaustive.
3. **`04-ppu-rendering.md` *Open questions* lines 373–374 — the `S/EV`
   example is stale after the §1 change.** The bullet still says "`S/EV` H 65 /
   `E/EV` H 255 (evaluation 65–256)" — a 0-dot offset for `S/EV` — while §1
   line 45 now puts the `S/EV` edge at dot 66, and `04-ppu-sprites.md` OQ
   line 384 says 66. Related: `04-ppu-sprites.md` §2 lines 73/86 and
   `04-ppu-rendering.md` *Programming notes* lines 310–311 still say the
   evaluation window is 65–256, so the three statements need one explicit
   sentence (NESdev's anchor 65 vs this spec's latched edge 66).
4. **`04-ppu-sprites.md` §2 lines 78–79 — `/EVAL` is not a one-dot pulse.**
   `n_EVAL = ~(hpos_latch1_q | eval_latch1_q | eev_latch1_q)` (`fsm.v`
   447–449) is high for 338 of the 341 dots and goes low for one dot at
   H = 63, 255 and 339; `ppu_top.v` line 82 labels it "0: Sprite Evaluation in
   Progress" — a level. The actionable half is right: `obj_eval.v` lines
   233–236 detect the 0→1 edge as the end-of-window signal. Delete "is a
   one-dot pulse too, not a level" and say it is a level whose rising edge ends
   the window.
5. **`04-ppu-sprites.md` §7 item 5, line 258.** "With both clipping bits set
   the earliest possible hit column is x = 8" is too strong: NESdev `PPU_OAM`
   excludes x = 0–7 "if bit 2 **or** bit 1 of PPUMASK is 0", so either clip
   alone already moves the earliest hit to x = 8. Say "with either left-column
   clip active (bit 1 or bit 2 = 0)".
6. **`04-ppu-rendering.md` §7 line 279 — the dropped dot is misidentified.**
   "…the pre-render scanline is **340 dots** instead of 341; the dropped dot is
   the one before the normal clear." The counter clears while
   `HC = HPLA_23 | EvenOddOut` is high, i.e. during H = 339 on odd frames, so
   dot 339 is kept (last dot) and dot 340 — the dot during which `HPLA_23`
   normally fires — is the one that never happens. "the one before the normal
   clear" points at 339 instead.
7. **`04-ppu-rendering.md` §4 line 172 — wrong signal named as the end of the
   bug.** NESdev `PPU_sprite_evaluation` §"Cause of the sprite overflow bug"
   makes the *write-disable* signal the cause and says "another signal prevents
   the comparison from influencing the sprite address incrementation logic";
   the spec has "a write-disable signal stops influencing the counter logic and
   the bug ends".
8. **Cross-file dot labels for the last increment (pre-existing).**
   `04-ppu-rendering.md` §2 lines 84/97 put the final coarse-X and the coarse-Y
   increment at dot 256; `04-ppu-scrolling.md` §4 (measured) puts the same edge
   at its dot 257 and its *Open questions* explicitly asks that "the three
   files must use one convention". The rendering file records no such caveat.
   Not touched by this revision; the final pass must pick one convention.

## Round-1 findings — round-2 confirmation

- **Blocking 1 (rendering §3 BG `PAT_ADR` source) — fixed.** Lines 131–132 now
  read "name-table byte (tile index) bit 0" / "bits 1–7"; matches `par.md`
  rows 4–5, `par.v` (`ParBit4` from `PD[0]`, `ParBit` from `PD[1..7]`) and
  `04-ppu-memory-map.md` §5 (`tile << 4`); the citation now points at §5.
- **Major 2 (+1 `dlatch` shift) — fixed in substance.** `S/EV` 65 → 66
  (line 45), `CLIP_O`/`CLIP_B` 0–7/256–263 → 1–8/257–264 (line 46), `E/EV`
  already 256; the paragraph (36–41) and the OQ bullet (376–378) now name the
  same ten signals, and §2's schedule is unchanged. Residual: minors 1–3.
- **Major 3 (odd-frame skip) — fixed.** §7 lines 273–284 now state the extra
  clear: `EvenOddOut` asserts at H = 339
  (`nor (EvenOddOut, q2, nHPLA_5, ~RESCL)`, `fsm.v` 250, `.nHPLA_5(~HPLA_out[5])`
  line 94) and `HVCounterControl` clears with `HC = HPLA_23 | EvenOddOut`
  (`fsm.v` 324), giving a 340-dot NTSC pre-render line; the 2C07 block omits the
  term (`fsm.v` 344), and the PAL/Dendy contrast plus the 340-dot observable
  (phenomenon 9) are kept.
- **Major 4 (attribute diagram) — fixed.** Lines 35–44 are `VHPx --pp` with
  palette bits 1–0, priority bit 5, H flip 6, V flip 7 and bits 4–2 "no
  cells"/read back 0, matching NESdev `PPU_OAM` Byte 2 and
  `04-ppu-memory-map.md` §8; the `$E3` sentence no longer claims §8 leaves the
  value open.
- **Major 5 (sprite 0 exclusions) — fixed.** x = 255 added
  (`04-ppu-sprites.md` §7 item 6 line 259; `04-ppu-rendering.md` §6 line 230)
  and both layers required (`04-ppu-sprites.md` item 4 line 257; rendering
  line 229), consistent with `regs.v` `Clipper` 205–209
  (`n_CLPB = ~(w2|w3|~BGE)`, `CLPO = w2|CLIP_O|~OBE`) and NESdev `PPU_OAM`
  §"Sprite 0 hits".
- **Major 6 (overflow walk) — fixed.** §3 hit branch (lines 143–146) sets the
  flag and advances three entries with `m` wrap into `n`; §4 prose (line 162)
  reads `OAM[n+1][1]`…`OAM[n+3][3]`; the wrap row (line 170) is `OAM[n+4][0]`;
  all match NESdev `PPU_sprite_evaluation` steps 3a/3b.
- **Minors 7–17 — all fixed** (verified line by line): §4 16-pixel quadrant
  (`bgcol.v` `at[{TVO[1], H01}]`, 361–362); §5 no `CLIP_B` in `MUX_Control`
  (clip lives in `BGCOL` `n_CLPB`); §1 `F/AT` single-latch + `/FO` gate
  (`F_AT = ~(fo3 | fat_latch1_nq)`, `fsm.v` 499); sprites §2 `/EVAL` used as an
  edge (wording issue — minor 4); X ≥ 249 / ≤ 248; sticky `$2002[5]` cited to
  `obj_eval.md`/`obj_eval.v`; `PD_SEL` = one-dot-delayed `F_AT` (`bgcol.v` 87);
  Y threshold `$EF`; `T` = first displayed tile and "first fetched = third"
  NESdev-only; §1 `$2004`-during-rendering cause; §7 `SC_CNT` gated by `BLACK`
  (`fsm.v` 501) and the VRAM read path by `BLNK` (`vram_ctrl.v` 69).

## Checked against

- `BreakingNESWiki/PPU/par.md` and `BreakingNESWiki_DeepL/PPU/par.md` — full
  bit table rows 0–2, 3, 4, 5–11, 12 for BG / OB 8×8 / OB 8×16.
- `HDL/PPU/par.v` — `ParBit`, `ParBit4`, `ParBitInv`, `ParControl`, `PAR`
  instantiations; the 8×8 bit-4 source is `OB[0]`, `OBSEL` only reaches bit 12.
- `HDL/PPU/fsm.v` — `HPosLogic` latch block 439–467 and assignments 493–504,
  `EvenOdd` 235–251, `HVCounterControl` 312–330 and `_2C07` 333–349,
  `VPosLogic` `RESCL`/`BLNK`.
- `HDL/PPU/hv_decoder.v` — re-simulated `HDecoder` with Icarus
  (`-D ICARUS -D RP2C02`) over H = 0…341: `HPLA_2` 65, `_3` 0–7/256–263, `_5`
  339, `_6` 63, `_7` 255, `_8` 0–63, `_9` 256–319, `_10` 0–255, `_11` ≡0,1,
  `_12` ≡6,7, `_13` ≡4,5, `_14` 320–335, `_15` 0–255, `_16` ≡2,3, `_23` 340.
  The raw `HPLA` column of §1 matches `PPU/hv_decoder.md` exactly.
- `HDL/PPU/regs.v` — `Clipper` gating of `n_CLPB`/`CLPO` by `BGE`/`OBE`.
- `HDL/PPU/mux.v` — `MUX_Control` inputs (no clip term),
  `Spr0Hit` `Strike = ~(PCLK|bg|n_SPR0HIT|n_SPR0_EV|n_VIS)`, `STRIKE_FF.r(RESCL)`.
- `HDL/PPU/bgcol.v` — `PD_SR`/`SRLOAD`/`PD_SEL`/`STEP`/`NEXTS`/`H01` (83–89),
  quadrant select `at[{TVO[1], H01}]` (361–362, 395).
- `HDL/PPU/fifo.v` — `code2lane` lane table, `p_code` byte types 2/3/5/7,
  `Z_HPOS` arming and the `n_VIS` emit window.
- `HDL/PPU/tilecnt.v`, `HDL/PPU/vram_ctrl.v`, `HDL/PPU/oam.v`,
  `HDL/PPU/obj_eval.v` — `SC_CNT`/`BLNK` gating, `ovf_reg`/`RESCL`
  (101, 144–145), `n_SPR0_EV` (121, 186), `n_EVAL` edge (233–236).
- NESdev via the Wayback Machine (2024/2025 snapshots): `PPU_sprite_evaluation`
  (cycles 1–64 / 65–256 / 257–320, steps 1–4 incl. 3a/3b, bug cause),
  `PPU_OAM` (attribute Byte 2, `$EF`–`$FF`, `$F9`–`$FF`, sprite 0 exclusions
  incl. x = 255 and "if background or sprite rendering is disabled", `$E3`),
  `PPU_rendering` (cycle 0, cycles 1–256 / 257–320 / 321–336 / 337–340,
  "first tile fetched is Tile 3", sprite 0 "image starts at cycle 2", odd-frame
  skipped cycle, priority table).
- Sibling specs: `04-ppu-memory-map.md` §2, §4, §5, §8;
  `04-ppu-registers.md` §2–§5, §9, §12; `04-ppu-scrolling.md` §4 and
  *Open questions*; `Specs/Conventions.md` §6–§10; `Specs/_template.md`.
- Template: both files `Status: revised (round 2)`, exactly 400 lines, the five
  fixed headings present, unnumbered and in order.

## Notes for the next round

- One blocking item remains and it is a one-cell fix: `04-ppu-sprites.md` §6's
  8×8 bit-4 source must become "tile byte bit 0" (the 8×16 column and the rest
  of the table, and rendering §3, are correct).
- The round-2 revision itself introduced no new source-fidelity error; the
  minors above are wording/model-consistency (1–4), one pre-existing wording
  slip (6–7) and the sibling-owned dot-convention item (8). Minor 5 is a
  one-phrase fix.
- Round 2 is the last permitted revision round (`Conventions.md` §10). If
  minors 1–4 and 6–8 are not rewritten, they should be recorded in the files'
  *Open questions* as known limitations rather than silently dropped; minors 3,
  4 and 8 are traceability-relevant.
- The `04-ppu-scrolling.md` §4 measured one-dot disagreement (minor 8) belongs
  in the final global pass, together with the render/register files'
  "dot 256 vs 257" labels.

---

# Review: ppu-render-sprites — round 1

## Verdict
revise

## Blocking

1. **`04-ppu-rendering.md` §3 — the background `PAT_ADR` source column is wrong.**
   Lines 130–132 label `PAT_ADR[4]` as "nametable X bit" and `PAT_ADR[5:11]` as
   "nametable Y + coarse X". `BreakingNESWiki/PPU/par.md`
   ("Таблица использования разрядов в адресации") and the English translation
   `BreakingNESWiki_DeepL/PPU/par.md` both give the BG source of bit 4 as
   "Name Table, bit 0" and of bits 5–11 as "Name Table, bits 1–7", with the role
   column reading "Index in Pattern Table" for both. `HDL/PPU/par.v` confirms it:
   for `OBJ_READ = 0`, `PAddr_out[4]` comes from `PD[0]` and `PAddr_out[5..11]`
   from `PD[1..7]` (`ParBit`/`ParBit4` instances) — i.e. bits 0–7 of the
   *fetched name-table byte* (the tile index), not scroll/nametable-select bits.
   The sibling `04-ppu-memory-map.md` §5 (line 128) already uses the correct
   form: ``Pattern | `PTSEL << 12 | tile << 4 | plane << 3 | fineY` ``.
   As written the table would wire the nametable select / coarse X into the
   pattern address and break every background fetch. Fix is a one-cell
   correction: bit 4 = "name-table byte (tile index) bit 0"; bits 5–11 =
   "name-table byte (tile index) bits 1–7".

## Major

2. **`04-ppu-rendering.md` §1 — the "+1 dot" rationale is applied selectively and
   the signal list in *Open questions* is incomplete.** §1 (lines 31–39) states
   that `HPosLogic` passes each strobe through "one or two `dlatch` stages", so
   the observable window is "the strobe's H value or one dot later", and it then
   shifts `I/OAM2`, `OBJ_READ`, `/FO`, `#F/NT`, `F/TA`, `F/TB` and `F/AT` by
   +1 dot. But `S/EV` (line 43, "65"), `E/EV`, `CLIP_O`/`CLIP_B` (line 44,
   "0–7, 256–263") and `/VIS` (line 49, "0–255") are listed at their raw HPLA
   values. In `HDL/PPU/fsm.v`, `S_EV`, `E_EV`, `I_OAM2`, `OBJ_READ`, `n_VIS`,
   `CLIP_O`/`CLIP_B` all use the identical two-latch path
   (`sev_latch1`/`sev_latch2` lines 440–441, `eev_latch1/2` 450–451,
   `ioam_latch1/2` 452–453, `objrd_latch1/2` 454–455, `nvis_latch1/2` 456–457,
   `clpo_latch1/2` 442–443); only `F/AT` is single-stage (`fat_latch1`, line
   467). The *Open questions* bullet (lines 368–382) lists only
   "`I/OAM2`, `OBJ_READ`, `/VIS`, `#F/NT`, `F/TA` and `F/TB` pass two stages,
   `F/AT` one" and omits `S/EV`, `E/EV` and the two clip signals, so the choice
   to leave `S/EV` at dot 65 while shifting `OBJ_READ` to 257–320 (same path) is
   unexplained. The schedule itself (dots 1–256 / 257–320 / 321–336 / 337–340) is
   self-consistent and matches NESdev `PPU_rendering`, so this is a model/rationale
   defect, not a wrong schedule.

3. **`04-ppu-rendering.md` §7 — the odd-frame skip mechanism is described
   backwards.** Lines 272–276 say the circuit "suppresses the H-counter clear at
   H = 340 (dot 340), so the pre-render scanline is 340 dots instead of 341".
   Suppressing a clear cannot shorten a line. `HDL/PPU/fsm.v` shows the opposite:
   `EvenOdd` asserts `EvenOddOut` at H = 339 (`nor (EvenOddOut, q2, nHPLA_5,
   ~RESCL)`, line 250, instantiated with `.nHPLA_5(~HPLA_out[5])`, line 94), and
   `HVCounterControl` ORs it into the clear:
   `HC = nq(dlatch(d = ~(HPLA_23 | EvenOddOut))) = HPLA_23 | EvenOddOut`
   (line 324). So the odd frame gets an *extra* clear at H = 339 (one dot
   early); the 2C07 variant omits it (`HVCounterControl_2C07`, line 344). The
   observable result quoted (340-dot NTSC pre-render line, absent on PAL/Dendy)
   is correct; the mechanism is inverted and contradicts the module it cites.

4. **`04-ppu-sprites.md` §1 — the attribute-byte bit diagram is shifted by one
   bit and shows a 4-bit palette.** Lines 36–45 render:
   `VHPp PPPP` with the `+` markers under bits 3–0 ("palette (palette bits
   0–1)"), bit 4 ("priority"), bit 5 ("horizontal flip"), bit 6 ("vertical
   flip"), bit 7 ("unused"). NESdev `PPU_OAM` §Byte 2 and the sibling
   `04-ppu-memory-map.md` §8 table (lines 173–176) give bits 7–6 = V/H flip,
   bit 5 = priority, bits 4–2 = not stored, bits 1–0 = palette. The diagram also
   contradicts the sentence three lines below it ("Bits 4–2 of the attribute
   byte have no cells"). An implementer copying the diagram gets priority, both
   flips and the palette bit positions wrong. Suggested diagram:
   `VHPx --pp` (or the memory-map §8 table).

5. **`04-ppu-sprites.md` §7 / `04-ppu-rendering.md` §6 — the sprite 0 hit
   exclusions are incomplete.** The brief and NESdev `PPU_OAM` §"Sprite 0 hits"
   require the x = 255 exclusion ("At x=255, for an obscure reason related to
   the pixel pipeline"); it is absent from both files (no occurrence of 255 as a
   pixel in either sprite-0 list). Add it to the §7 operational list and to the
   rendering §6 practical form. Related precision: §7 item 4 says rendering must
   be enabled "(`BLACK = 0`; either PPUMASK bit 3 or 4)". NESdev says the hit
   does not happen "if background or sprite rendering is disabled"; the HDL gates
   each layer separately (`regs.v` `Clipper`: `CLPO` includes `~OBE`,
   `n_CLPB` includes `~BGE`), so the operational rule is "both layers must be
   enabled", which happens to be implied by conditions 2–3 but is not what the
   sentence says.

6. **`04-ppu-sprites.md` §3–§4 — the overflow walk pseudo-code does not match the
   cited NESdev algorithm, and the walk table is off by one.** NESdev
   `PPU_sprite_evaluation` step 3a ("If the value is in range, set the sprite
   overflow flag ... and read the next 3 entries of OAM (incrementing 'm' after
   each byte and incrementing 'n' when 'm' overflows)") is not what the §3 code
   does: the hit branch (lines 143–146) advances one byte and adds a second `n`
   step only when `m` wraps (`n=(n+1); m=(m+1); if m==0: n=n+1`). The miss
   branch (lines 139–142) does match step 3b ("increment n and m (without
   carry)"), but then the §4 table's wrap row ("m wraps | `OAM[n+5][0]` again",
   line 170) is wrong: starting at `(n0,0)` the walk is `(n0,0)`, `(n0+1,1)`,
   `(n0+2,2)`, `(n0+3,3)`, `(n0+4,0)` — i.e. `OAM[n+4][0]`, four sprites ahead,
   not five. The §4 prose (line 162) also says the walk reads "`OAM[n][1]`,
   `OAM[n][2]`, `OAM[n][3]`" while its own table (lines 167–169) says
   `OAM[n+1][1]`, `OAM[n+2][2]`, `OAM[n+3][3]`. The bug's *explanation* (false
   positives/negatives, write-disable, end of bug on the next match) is correct
   and honestly marked NESdev-only in *Open questions*; the algorithm an
   implementer would copy is not.

## Minor

7. `04-ppu-rendering.md` §4 line 154: "Attribute bits change once per tile
   period" contradicts the same table's "`H01` toggles every two tiles"
   (line 148) and NESdev/`bgcol.v`, where the selected pair is constant for a
   16-pixel quadrant (`BGC_2`: `at[{TVO[1], H01}]`, lines 361–362).

8. `04-ppu-rendering.md` §5 line 176 lists "background left-column clip
   (`CLIP_B`)" as a stage-1 `PictureMUX` control. `HDL/PPU/mux.v` `MUX_Control`
   has no clip input; the clip is applied in `BGCOL` (`n_CLPB`), as this file's
   own §6 table says.

9. `04-ppu-rendering.md` §1 line 54 gives `F/AT` as "dots ≡ 3,4 (mod 8)", which
   includes dots 339–340, while §2 line 88 says dots 337–340 are two nametable
   fetches. `F_AT = ~(fo3 | fat_latch1_nq)` is additionally gated off outside
   the `/FO` window (dots 321–336), which is what removes the conflict; the
   §1 row does not mention that gate.

10. `04-ppu-sprites.md` §2 line 79: "evaluation runs while `/EVAL` = 0". In
    `HDL/PPU/fsm.v` `n_EVAL` is a one-dot pulse
    (`eval_latch2_nq = hpos_latch1_q | eval_latch1_q | eev_latch1_q`, lines
    448–449), and `obj_eval.v` uses its rising edge as the end-of-window signal
    (lines 234–243), not as a level.

11. `04-ppu-sprites.md` §5.2 line 218 / phenomenon 10 (line 275): "X ≥ 248"
    should be X ≥ 249. NESdev `PPU_OAM` says X = `$F9`–`$FF` (249–255) loses
    pixels; a sprite at X = 248 covers dots 248–255 and is fully visible under
    this file's own `/VIS` = dots 0–255.

12. `04-ppu-sprites.md` §4 line 160 cites `PPU/fsm.md` and `PPU/mux.md` for the
    sticky `$2002[5]`; neither page mentions `SPR_OV` (only `PPU/obj_eval.md`
    and `obj_eval.v` do, as the Provenance row correctly states).

13. `04-ppu-rendering.md` §4 line 146 gives `PD_SEL` as "`F_AT=1` and `H0_DD`";
    `bgcol.v` line 87 uses the one-dot-delayed `F_AT` (`w6`) and the phase:
    `PD_SEL = ~(w6 | PCLK | ~H0_DD)`.

14. `04-ppu-sprites.md` §3 line 107: "A Y byte ≥ `$F0` ... hides a sprite" —
    NESdev `PPU_OAM` and the file's own table (line 118) put the threshold at
    `$EF` (`$EF` is selected on scanline 239 and drawn on 240–247, off-screen).

15. `04-ppu-rendering.md` §2 line 82/92: "tiles T … T+31" uses T both for the
    first tile *fetched* on the line (tile 3) and for the tile displayed at
    dots 1–8; the NESdev "first tile fetched is Tile 3" claim (correct) is
    attributed to `PPU/fsm.md`, which does not discuss tiles.

16. `04-ppu-sprites.md` §1 line 53: "It is invisible through `$2004`/`$4014`
    ... but it explains why OAM reads during rendering return other addresses" —
    the latter is caused by the evaluator driving the OAM address, not by the
    NTSC row-decode permutation, which is what the sentence is about.

17. `04-ppu-rendering.md` §7 line 256 says "the pattern loads are gated by
    `BLACK`"; `bgcol.v` `PD_SR`/`SRLOAD` contain no `BLACK` term (the VRAM read
    path is gated by `BLNK` in `vram_ctrl.v`).

## Checked against

- `BreakingNESWiki/PPU/hv.md`, `hv_decoder.md`, `fsm.md` — H counter 0–340,
  HPLA strobe values and meanings, even/odd description; all §1 table values
  0–23, 65–340 confirmed. `HDL/PPU/hv_decoder.v` RP2C02 vs RP2C07 masks differ
  only at `HPLA_0` and `HPLA_17–22` (verified line by line), so the spec's PAL
  claim is right.
- `HDL/PPU/fsm.v` — `HPosLogic` latch stages (`sev`, `clpo`, `clpb`, `hpos`,
  `eval`, `eev`, `ioam`, `objrd`, `nvis`, `fnt`, `ftb`, `fta`, `fo`, `fat`),
  `EvenOdd`/`EvenOdd_2C07`, `HVCounterControl` (HC polarity), `SC_CNT`, `BLACK`
  gating.
- NESdev `PPU_rendering` (cached) — cycle 0 idle address, cycles 1–256 fetch
  order, 257–320 sprite fetches, 321–336 prefetch, 337–340 dummy NT fetches,
  "first tile fetched is Tile 3", shifters reloaded on ticks 9…257, sprite 0
  earliest cycle 2, odd-frame dot skip, priority decision table.
- NESdev `PPU_sprite_evaluation` (cached) — cycles 1–64 secondary-OAM clear,
  65–256 evaluation, 257–320 fetch, step 3a/3b overflow algorithm, write-disable
  cause, no evaluation on the pre-render line, evaluation with either layer on.
- NESdev `PPU_OAM` (cached) — attribute byte bits 7–6/5/4–2/1–0, `$EF`–`$FF`
  hiding, 8×16 tile bit 0 bank, sprite 0 hit exclusions incl. x = 255 and
  "background or sprite rendering disabled", X = `$F9`–`$FF`, OAM DMA 513(+1).
- NESdev `PPU_sprite_priority` (cached) — sprite-vs-sprite priority resolved
  before background priority.
- `BreakingNESWiki/PPU/par.md` + `BreakingNESWiki_DeepL/PPU/par.md`,
  `HDL/PPU/par.v` — PAT_ADR bit sources, 8×16 `OV3` → bit 4 and tile-byte bit 0
  → bit 12 (spec §6 correct).
- `BreakingNESWiki/PPU/bgcol.md`, `HDL/PPU/bgcol.v` — `PD_SR`/`SRLOAD`/`PD_SEL`/
  `STEP`/`NEXTS`/`H01` equations, reversed load, `disp[FH]` tap, `{TVO[1],H01}`
  quadrant select, bit-reversal comments.
- `BreakingNESWiki/PPU/mux.md`, `HDL/PPU/mux.v` (`MUX_Control`, `PictureMUX`,
  `Spr0Hit`) — three-stage cascade, `OCOL` rule, 5-bit index, `PAL4` = sprite
  selector, `Strike = ~(PCLK|bg|/SPR0HIT|/SPR0_EV|/VIS)`, `STRIKE_FF.r(RESCL)`.
- `BreakingNESWiki/PPU/tilecnt.md`, `HDL/PPU/tilecnt.v` — counter widths/limits
  (TH 5/32, TV 5/30, NT 1/2 each, FV 3/8); spec §3 item 1 correct.
- `BreakingNESWiki/PPU/pamux.md`, `HDL/PPU/pamux.v` — NT/AT/PAT source
  selection and 14-bit `/PA`.
- `BreakingNESWiki/PPU/fifo.md`, `HDL/PPU/fifo.v` — lane decode `H3''..H5''`,
  byte types `H0''..H2''` = 2/3/5/7, `Z_HPOS` arming, priority encoder,
  `HINV`/`HDIR`, `PD/FIFO` zero-fill.
- `BreakingNESWiki/PPU/obj_eval.md`, `HDL/PPU/obj_eval.v` — 8-sprite limit,
  secondary OAM, `OV`, `/SPR0_EV`, `PD/FIFO`, `ovf_reg` cleared by `RESCL`, and
  that the HDL does **not** model the diagonal walk (spec's caveat is accurate).
- `BreakingNESWiki/PPU/oam.md`, `Docs/PPU/OAM_RowCol.txt` — 2112 cells, 29
  bits/sprite, `OAM8`, row/column decode, NTSC inversion, `BLNK` write gate.
- `BreakingNESWiki/PPU/scroll_regs.md` — `FH`/`FV`/`NTH`/`NTV`/`TV`/`TH`
  mappings, `$2005`/`$2006` dual writes.
- `Specs/04-ppu-registers.md` §4–§6, §12 — `$2003`/`$2004`/`$4014` semantics,
  revision gating, OAMADDR forced 0 on dots 257–320, 513/514 DMA, `RESCL`
  clearing of bits 6/5. All consistent with the two files under review.
- `Specs/04-ppu-memory-map.md` §4, §5, §8 — attribute formula
  `$23C0 | (v&$0C00) | ((v>>4)&$38) | ((v>>2)&7)` (matches rendering §3's
  `%10NN 1111 yyy xxx`), pattern formula, OAM attribute table (contradicts
  sprites §1 diagram).
- `Specs/04-ppu-scrolling.md` §1–§2 and Open questions — `v` bit fields
  (`v[14:12]`→`PAT_ADR[2:0]`, NT/AT addresses) and the 0–340 dot convention.
- Template/Conventions: both files have `Status: adapted (round 1)`, the five
  fixed headings exactly and in order, and are exactly 400 lines (inside
  120–400). Cross-references to sibling § numbers (`04-ppu-memory-map.md` §4,
  §5; `04-ppu-registers.md` §3, §5, §9, §12) resolve to the right sections.

## Notes for the next round

- The `04-ppu-rendering.md` §3 attribute notation (`%10NN 1111 yyy xxx`) is now
  correct and matches NESdev, but `04-ppu-memory-map.md` *Open questions*
  (line 278) still describes the rendering file as printing
  `%0010 3NN YYYY YXXX`. That sibling note is stale and should be dropped when
  the memory-map file is next revised.
- `04-ppu-scrolling.md` *Open questions* (lines 369–375) records a one-dot
  offset between its measured edges and NESdev's; the rendering file's dot = H
  model and the "+1 for two-latch signals" narrative should be reconciled with
  that note explicitly (currently only the `F/AT` case is discussed).
- Fixing items 1 and 3–6 is enough for a consensus re-review; items 2 and 7–17
  are correctness/clarity of rationale and can be recorded if not all rewritten.

## Revision round 2

- Blocking 1: fixed — `04-ppu-rendering.md` §3 `PAT_ADR` table: bit 4 BG source
  is now "name-table byte (tile index) bit 0" and bits 5–11 "name-table byte
  (tile index) bits 1–7", matching `PPU/par.md` ("Name Table, бит 0/1-7"),
  `HDL/PPU/par.v` (`ParBit4`/`ParBit` from `PD[0]`/`PD[1..7]`) and
  `04-ppu-memory-map.md` §5 (`tile << 4`). The citation line now also points to
  §5.
- Major 2: fixed — `04-ppu-rendering.md` §1: the +1 `dlatch` shift is now
  applied to every two-latch signal in the "dot window" column (`S/EV` 65 → 66,
  `CLIP_O`/`CLIP_B` 0–7/256–263 → 1–8/257–264; `E/EV` 256 was already shifted),
  the paragraph names the full two-stage list (`S/EV`, `E/EV`, `CLIP_O`/
  `CLIP_B`, `I/OAM2`, `OBJ_READ`, `/VIS`, `#F/NT`, `F/TA`, `F/TB`; `F/AT`
  single-stage), and the *Open questions* H-decoder bullet now lists all of
  them. Functional clip wording stays x = 0–7 (§6, programming notes). The
  §2 schedule (1–256 / 257–320 / 321–336 / 337–340) is unchanged.
- Major 3: fixed — `04-ppu-rendering.md` §7: the odd-frame skip is now stated as
  an *extra* clear at H = 339 (`EvenOdd` asserts `EvenOddOut`; `HVCounterControl`
  `HC = HPLA_23 | EvenOddOut`), so the line clears one dot early and the
  pre-render line is 340 dots. The 340-dot NTSC observable and PAL/Dendy
  contrast are kept.
- Major 4: fixed — `04-ppu-sprites.md` §1 attribute diagram is now `VHPx --pp`
  (palette bits 1–0, priority bit 5, H flip bit 6, V flip bit 7, bits 4–2 not
  stored), matching NESdev `PPU_OAM` and `04-ppu-memory-map.md` §8; the
  read-back note no longer claims §8 leaves the value open.
- Major 5: fixed — sprite 0 hit now excludes x = 255 (`04-ppu-sprites.md` §7
  item 6, `04-ppu-rendering.md` §6 practical form) and requires **both** PPUMASK
  bits 3 and 4 (`Clipper` gates `n_CLPB` with `~BGE` and `CLPO` with `~OBE`).
- Major 6: fixed — `04-ppu-sprites.md` §3 hit branch now sets the flag and
  advances over the next 3 OAM entries (m with wrap into n), per NESdev step 3a;
  §4 prose reads `OAM[n+1][1]`…`OAM[n+3][3]` and the wrap row is `OAM[n+4][0]`.
  The NESdev-only marking and the false positive/negative explanation are kept.
- Minor 7: fixed — §4 quadrant select and attribute bullet now say 16-pixel
  (two-tile) quadrant; programming note likewise.
- Minor 8: fixed — §5 stage-1 control no longer lists `CLIP_B`; the clip is
  noted as applied in `BGCOL` (`n_CLPB`), not `MUX_Control`.
- Minor 9: fixed — §1 `F/AT` row now says single-latch and gated off outside `/FO`.
- Minor 10: fixed — `04-ppu-sprites.md` §2: `/EVAL` (`n_EVAL`) is a one-dot
  pulse used as an edge, not a level.
- Minor 11: fixed — X ≥ 248 → X ≥ 249 (§5.2, phenomenon 10, Open questions);
  X ≤ 247 → X ≤ 248 in the remedy column.
- Minor 12: fixed — sticky `$2002[5]` now cites `PPU/obj_eval.md`/`obj_eval.v`.
- Minor 13: fixed — §4 `PD_SEL` now "one-dot-delayed `F_AT=1` and `H0_DD` and `/PCLK`".
- Minor 14: fixed — Y hiding threshold `$F0` → `$EF` (§3 and Open questions).
- Minor 15: fixed — §2 tile symbol clarified (T = first displayed tile) and the
  "first fetched tile is the third" claim is now NESdev-only.
- Minor 16: fixed — §1 no longer attributes `$2004` rendering-time addresses to
  the NTSC row permutation; the evaluator owns the address.
- Minor 17: fixed — §7 forced-blank cell now says `SC_CNT` is gated by `BLACK`
  and the VRAM read path by `BLNK`; the `BGCOL` pattern loads have no `BLACK` term.
- Both files re-checked at exactly 400 lines, `Status: revised (round 2)`. Only
  `04-ppu-rendering.md`, `04-ppu-sprites.md` and this report were touched; no
  `git commit`.

## Revision round 3

- Blocking 1: fixed — `04-ppu-sprites.md` §6 `PAT_ADR` table, 8×8 column: bit 4
  is now "tile byte bit 0 (tile index)" (was `OBSEL`), so the 8×8 column sources
  are bits 4–11 = tile byte bits 0–7 and bit 12 = `OBSEL` only. Matches
  `BreakingNESWiki/PPU/par.md` row 4 ("OAM2, Tile Index Byte, бит 0"), row 12
  (`OBSEL`), `HDL/PPU/par.v` (`ParBit4` takes `OB[0]` when `O8_16 = 0`;
  `ParControl` puts `OBSEL` on `PAD12`), and the sibling files
  `04-ppu-memory-map.md` §2 and `04-ppu-rendering.md` §3.
- Minor 1: fixed — `04-ppu-rendering.md` §1: the list now says "every multi-latch
  signal **except** `/VIS`", and the `/VIS` exception is stated inline (NESdev
  anchor 0–255; a sprite at X = 0 must show a pixel on dot 0), so the rule no
  longer contradicts the `/VIS` row.
- Minor 2: fixed — the same list now also names `0/HPOS`, `/EVAL` and `/FO`
  (`fsm.v` `hpos_latch1/2`, `eval_latch1/2`, `fo_latch1/2/3`); the Open-questions
  bullet likewise lists them and notes `/FO` is three-stage.
- Minor 3: fixed — one explicit statement now stands in `04-ppu-rendering.md` §1
  (and is repeated in its Open questions): NESdev's evaluation window is cycles
  65–256, while the latched `S/EV` edge is dot 66 and `E/EV` dot 256. The stale
  "`S/EV` H 65 / `E/EV` H 255 (evaluation 65–256)" pairing is gone;
  `04-ppu-sprites.md` §2's table is annotated "(NESdev anchor; latched edges
  66–256)", and the two bare "dot 65" OAMADDR mentions now read dot 66 / NESdev
  cycle 65.
- Minor 4: fixed — `04-ppu-sprites.md` §2: `/EVAL` (`n_EVAL`) is now described as
  a *level*, low only at H = 63/255/339, whose rising edge ends the window
  (`fsm.v` 447–449, `obj_eval.v` 233–236).
- Minor 5: fixed — `04-ppu-sprites.md` §7 item 5: "With either left-column clip
  active (bit 1 or bit 2 = 0) the earliest possible hit column is x = 8" (NESdev
  `PPU_OAM`: x = 0–7 excluded "if bit 2 or bit 1 of PPUMASK is 0").
- Minor 6: fixed — `04-ppu-rendering.md` §7: the dropped dot is now identified as
  dot 340, the dot on which `HPLA_23` normally clears the counter (dot 339 is
  kept, so 340 never happens).
- Minor 7: fixed — `04-ppu-sprites.md` §4: the bug-ending signal is no longer
  called a write-disable signal; it is "another signal [that] prevents the
  comparison from influencing the sprite address incrementation logic" (NESdev
  `PPU_sprite_evaluation` §"Cause of the sprite overflow bug", which makes the
  write-disable the *cause*, not the end).
- Minor 8: moved to `04-ppu-rendering.md` *Open questions* as an explicit
  cross-file item: §2 labels the final coarse-X/coarse-Y increment dot 256 while
  `04-ppu-scrolling.md` §4 measures the same edge at its dot 257; the final global
  pass must pick one convention.
- Housekeeping: the `F/AT`+`#F/NT`-placement and `/FO`-semantics Open-questions
  bullets were folded into the H-decoder bullet (content preserved, no finding
  dropped); the §1 PAL note was compressed by one line. Both files are exactly
  400 lines with `Status: revised (round 3)`, the five fixed headings unnumbered
  and in order. Only `04-ppu-rendering.md`, `04-ppu-sprites.md` and this report
  were touched; no `git commit`.
