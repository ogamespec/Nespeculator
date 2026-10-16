# Review: ppu-palette-timing — round 2

## Verdict
consensus

## Blocking
None. Round-1 blocking 1 is resolved (see Major 1); no new blocking defect was
found. All five round-1 majors are resolved, and the nine minors are addressed;
the items under *Minor* below are residual and do not keep the section open.

## Major

1. **`SYNC` window (`04-ppu-timing.md` §6) — resolved.** §6 now has separate
   rows: `/FPORCH` 257–279 (23 dots) and `SYNC` **280–304** (25 dots, ≈4.7 µs)
   NTSC / 257–277 and **278–302** PAL, with the preamble giving
   `SYNC = ~(sync_latch2_q | VSYNC)`, `VSYNC = ~(n_HB | vsync_ff_q)`. I re-ran
   Icarus 14.0 with a temporary per-dot probe on `HDL/PPU/fsm.v` (V = 100,
   RP2C02 and RP2C07) and measured exactly those settled outputs: NTSC
   `fporch_ff_nq` 257–279, `hblank_ff_q`/`SYNC` 280–304, `bporch_ff_q` 271–328,
   `burst_ff_nq` 309–323, `n_PICTURE` 0–270 + 329–340; PAL `fporch_ff_nq`
   257–277, `hblank_ff_q`/`SYNC` 278–302, `bporch_ff_q` 257–340 + 0–4,
   `burst_ff_nq` 307–321, `n_PICTURE` 5–256. **Every** §6 row matches, including
   the ones round 1 did not single out. `VidOut` forces the sync level while
   `SYNC = 1` (`or (black_latch_d, pic_out_latch_d, SYNC, BURST)`, `video_out.v`),
   so the composite sync pulse is genuinely 280–304 / 278–302.
2. **Dendy row (`04-ppu-timing.md` §10) — resolved.** The row is now
   `1.773448 MHz (÷15) | 5.3203424 MHz (÷5) | 3 : 1 | 3 | 341/3 = 113⅔`, the §10
   Dendy bullet gives 35 464 CPU cycles/frame (113⅔ × 312; 106 392 ÷ 3), and the
   Open question is relabelled "(rank 4)" citing
   `Cycle_reference_chart`/`CPU_variants`. Verified against the cached
   `Cycle_reference_chart` ("26.60~ MHz ÷ 15 = 1.773448 MHz", "PPU dots per CPU
   cycle 3", "341 × 5÷15 = 113 2⁄3", "106392 ÷ 3 = 35464") and against
   `01-platform-overview.md` §2 and `05-boards-and-io.md` §1, which now carry the
   same Dendy and `CPU:PPU` cells.
3. **`CPU:PPU` convention — resolved.** §10's column is `CPU:PPU` with a separate
   "Dots per CPU cycle" column, and the text states it "reads as **PPU dots per
   CPU cycle** (`3:1` = 3, `16:5` = 16/5 = 3.2) … — **not** `5:16`", matching
   `01-platform-overview.md` §2 ("numerically the PPU dots per CPU cycle … the
   PAL cell is `16:5`, **not** `5:16`") and `05-boards-and-io.md` §1. The old
   `1 : 3` / `5 : 16` cells are gone (checked by grep).
4. **`HC` / `EvenOddOut` / `RESCL` — resolved.** §2 and §8 now write
   `HC = HPLA_23 | EvenOddOut`, state that `~(HPLA_23 | EvenOddOut)` is only the
   inverting `dlatch` D input, add
   `EvenOddOut = ~q2 & HPLA_5 & RESCL`, and say `RESCL` (active only at V = 261)
   confines the skip to the pre-render line. Matches `fsm.v`
   (`nor (EvenOddOut, q2, nHPLA_5, ~RESCL)`,
   `dlatch ctrl_latch1 (.d(~(HPLA_23 | EvenOddOut)), .en(n_PCLK), .nq(HC))`) and
   `HDL/Common/dlatch.v` (`nq = ~d`).
5. **Dot-numbering Open question — resolved.** The bullet now states that both
   files use dot = `H`, that `HPLA_23` clears at dot 340, that
   `04-ppu-rendering.md` §1 is correct, and records only the one-`PCLK` latch
   shift of the derived windows. `04-ppu-rendering.md` §1 is unchanged ("dot =
   `H`", dot 340).

## Minor
Residual only (non-blocking); round-1 minors 6–14 are all addressed.

1. **`04-ppu-palette.md` §2 (lines 132–135) and correspondence — the quotation
   is mis-sourced.** The quoted sentence "Early revisions cannot read back sprite
   or palette memory" is not on `PPU_variants`; it is the "Other quirks" cell of
   NESdev `Cycle_reference_chart` (the `Clock_rate` redirect, cached).
   `PPU_variants` states the same fact as "In this and all previous revisions,
   OAMDATA and palette RAM are not readable" / "OAMDATA and PPU palette are not
   readable" (RP2C03B). Cite `Cycle_reference_chart` for the quote and keep
   `PPU_variants` for the per-revision list.
2. **`04-ppu-palette.md` §6 — citation nuance.** "(NESdev `PPU_palettes`
   §2C03/2C04)" covers two facts; the page attributes hue `$D` = **black** to the
   2C03 and to the 2C05-99, and the "completely different palettes as a copy
   protection measure" sentence to the 2C04. Split the citation.
3. **`04-ppu-timing.md` §6 — `FPORCH` row polarity wording.** The row
   "FPORCH FF (`/FPORCH`) | high 257–279" pairs an active-low name with "high";
   the FF's active-high `q` is high 257–279 and `/FPORCH` (`nq`) is low. Cosmetic
   (the window is correct); e.g. "FPORCH FF active 257–279".
4. **`04-ppu-timing.md` Open questions ("Length")** says the file is "At the
   400-line ceiling"; it is 395 lines. Cosmetic.

## Checked against

- **Rank 2 (HDL, Icarus 14.0, fresh runs).** `fsm_test.v` — TEST PASS 1586
  (RP2C02) / 1886 (RP2C07); `vidout_test.v` — TEST PASS 32930 (RP2C02) / 56055
  (RP2C07). A temporary probe (deleted afterwards) dumped one V = 100 line of
  `PPU_FSM` for both revisions and measured `fporch_ff_nq`, `hblank_ff_q`,
  `SYNC`, `bporch_ff_q`, `burst_ff_nq`, `n_PICTURE`, `OBJ_READ`, `CLIP_O/CLIP_B`,
  `n_FO`, `VSYNC`, `n_HB`.
- `HDL/PPU/fsm.v` (`EvenOdd`, `HVCounterControl`, `HVCounterControl_2C07`,
  `HPosLogic`, `VPosLogic`), `HDL/Common/{dlatch,rsff}.v`,
  `HDL/PPU/{hv_decoder,pclk,video_out,cram,ppu_top}.v`. I re-decoded **all** 24
  NTSC H rows, 24 PAL H rows and 10+10 V rows programmatically from the
  `HDecoder`/`VDecoder` masks; §3, §4 and §5 reproduce them exactly, including
  the gating lists (VB gates rows 4, 10; BLNK gates 2, 5–11, 14–15; rows 0–1, 3,
  12–13, 16–23 ungated). `cram.v` `CRAM_Decoder` uses `{CGA4,CGA1,CGA0}` with
  physical rows 0 and 4 sharing `ROW0_4`; `n_CC = ~cram[CGA][3:0]`,
  `n_LL = ~cram[CGA][5:4]`; the array is initialised to 0. `video_out.v`
  `ChromaDecoder` has two latches on the chroma path, `LumaDecoder` is fed
  `n_LL` directly, `n_PR = PZ[0]`/`n_PG = PZ[9]` are revision-independent, and
  `Emphasis` has no `RP2C07` branch.
- **Rank 1 (wiki).** `PPU/hv_decoder.md` (H/V row tables), `PPU/hv.md`,
  `PPU/fsm.md` (signal table: `/FPORCH`→`SYNC`, `/HB`→`VSYNC`, `/FO`→Data
  Reader), `PPU/umc6538.md` (`VPLA_4` +50 lines), `PPU/video_out.md` (phase
  decoder table; `$8` is decoder row 5 = burst phase 0°), `PPU/pal.md`.
- **Rank 3.** `Docs/PPU/VideoSignal/VDAC.TXT` (2C02G/2C07/UA6538 rows reproduced
  cell-for-cell by palette §3; "no BLACK level"; "TINT don't affect SYNC and
  BURST"), `Docs/NES_Famicom_Board_Revisions.md` §4 (RP2C02E-0 palette not
  readable; 2C07 71-line vblank; red/green emphasis swapped).
- **Rank 4 (`/tmp/nesdev-cache/`).** `Cycle_reference_chart` (Dendy ÷15,
  1.773448 MHz, 3 dots/cycle, 113 2⁄3, 35464, 50.0070 Hz; the readability
  quirk), `CPU_variants`, `PPU_palettes` (2C07 `$x7` and −15°; hue `$D` black on
  2C03/2C05-99; 2C04 scrambled), `PPU_power_up_state` (`+0+x xxxx` at power,
  `U??x xxxx` after reset, ~29658/33132 ignored-write window, working-register
  list), `PPU_variants`, `NTSC_video`.
- **Cross-file.** `01-platform-overview.md` §2 (3:1 / 16:5 / ÷15, 1.773448,
  35464), `05-boards-and-io.md` §1 (same table cell-for-cell),
  `04-ppu-rendering.md` §1–§2 (dot = `H`, dot 340, fetch schedule 257–320 /
  321–336 / 337–340, clip 1–8 / 257–264).
- **Template.** Both files `**Status:** revised (round 2)`; the five fixed
  headings are present in order with no number prefix (timing
  291/308/330/345/364; palette 297/316/339/353/372); lengths 395 and 400 lines
  (within the 120–400 target).

## Notes for the next round

- **Consensus reached — final round.** No blocking findings; every major is
  resolved; no major was deferred to Open questions.
- One cross-file follow-up, **outside** these two files:
  `01-platform-overview.md` §2 still says "`05-boards-and-io.md` §1 still carries
  the older table and must be aligned by a later cross-file pass", and its Open
  question *Cross-file items for a later pass* still claims `04-ppu-timing.md`
  §10 gives Dendy as `≈1.7897727 MHz` / `1 : ≈2.973` and that
  `05-boards-and-io.md` §1 carries `1:3` / `5:16` / `186.2 ns`. All three claims
  are now false (timing §10 and boards §1 are fixed). Route a stale-text fix to
  `01-platform-overview.md`; **do not** edit the timing/palette files for it.
- The files under review did not change during this review (`04-ppu-timing.md`
  md5 `1bdf6e94882ac16425ed8d7f1e810257`, `04-ppu-palette.md` md5
  `2b254b99bdc99f853103632c216b330a`); no spec file was edited and the temporary
  probe testbench was deleted.

---

# Review: ppu-palette-timing — round 1

## Verdict
revise

## Blocking

1. **`04-ppu-timing.md` §6 — the `SYNC` window is wrong; it is the `/FPORCH`
   window, not the sync pulse.** Line 161 says "`SYNC` (active low) | asserted
   256–279 (4.5 µs) | asserted 256–277 (4.1 µs)". The HSync pulse is the ~25-dot
   HBlank window, i.e. **≈279/280–303/304 NTSC and ≈277/278–301/302 PAL**, not
   256–279/256–277.
   - Rank 1: `BreakingNESWiki/imgstore/ppu/ppu_ntsc_line.png` and
     `ppu_pal_line.png` (linked from `PPU/fsm.md`, `PPU/waves.md`) draw `/FPORCH`
     first, then `/HB` and `SYNC` together, then `BURST` — the sync is *after* the
     front porch, not inside it.
   - Rank 2: I re-ran Icarus on `HDL/PPU/fsm.v` (`fsm_test.v`, `-D RP2C02`,
     `-D ICARUS`: TEST PASS 1586 checks; `-D RP2C07`: TEST PASS 1886 checks) and
     probed `PPU_FSM` on a visible line. The settled outputs are
     `fporch_ff.q` 257–279, `hblank_ff.q` 280–304, `SYNC` **280–304**,
     `BURST` 309–323 (NTSC), and `SYNC` **278–302**, `BURST` 307–321 (PAL).
     In `HPosLogic`, `SYNC = ~(sync_latch2_q | VSYNC)` and
     `VSYNC = ~(n_HB | vsync_ff_q)` (`fsm.v` lines 545–562), so `SYNC` is
     deasserted for the whole FPORCH window and asserted for the HBlank window.
   - Rank 4: NESdev `NTSC_video` §"Scanline Timing" (via `/tmp/nesdev-cache/`):
     "horizontal sync | 277 | 25" followed by "back porch 302 | 4", "colorburst
     306 | 15", "front porch 268 | 9" — a 25-dot sync after the front porch.
   The rest of the §6 table is consistent with this (HBLANK FF 279–303,
   BURST 308–322), so the `SYNC` row also contradicts its own table. The
   follow-on sentence "`SYNC` and `BURST` fall in the 256–322 blanking interval"
   (line 179) must move with it. This is the one window a video-signal
   implementer will copy verbatim, and as written it produces a broken line.

## Major

2. **`04-ppu-timing.md` §10 — the Dendy CPU clock / ratio / cycles-per-scanline
   are wrong** (line 249: `≈1.7897727 MHz`, `1 : ≈2.973`, `≈2.973`,
   `≈114.7`). The authoritative figures:
   - `Cycle_reference_chart.html` (cached; `Clock_rate` redirects here):
     Dendy CPU "26.60~ MHz ÷ 15 = **1.773448 MHz**", "PPU dots per CPU cycle
     **3**", CPU cycles per scanline "341 × 5÷15 = **113 2⁄3**", total CPU
     cycles per frame "106392 ÷ 3 = **35464**", frame rate "50.0070 Hz".
   - `CPU_variants.html`: UA6527P "Divider is 15"; `PPU_power_up_state.html`
     §Dendy: "a frame of exactly **113.667 × 312 = 35464** cycles".
   - `Specs/01-platform-overview.md` §2 (`revised (round 3)`) already states
     Dendy ÷15 = 1.773448 MHz, `CPU:PPU 3:1`, "exactly 3 PPU dots per CPU
     cycle", and explicitly names this file's row as the outlier (its Open
     questions, lines 328–330).
   So the correct row is `1.773448 MHz | 5.3203424 MHz | 1 : 3 | 3 | 341/3 =
   113⅔`. `1.7897727 MHz` is the *NTSC 2A03* rate (21.477272 ÷ 12), which the
   ÷15 divider rules out. The `## Open questions` bullet "Dendy CPU clock"
   (lines 383–385) is therefore wrong to say "no rank 1–3 source … gives an
   exact frequency, so the … row is approximate" — a rank-4 figure exists and is
   adopted by `01-platform-overview.md`; rewrite the bullet to cite
   `Cycle_reference_chart`/`CPU_variants` and mark it rank-4.
   (Derived check: 106,392 dots ÷ 2.973 = 35,786 CPU cycles, not the measured
   35,464.)

3. **`04-ppu-timing.md` §10 — the file's `CPU:PPU` convention contradicts
   `01-platform-overview.md`.** The platform file fixes the convention:
   "The `CPU:PPU` column is … numerically the **PPU dots per CPU cycle**:
   `3:1` … `16:5` = 16 dots per 5 CPU cycles = 3.2 … so the PAL cell is `16:5`,
   **not** `5:16`". `04-ppu-timing.md` §10 instead heads the column "Ratio
   (CPU cycles : dots)" and writes `1 : 3`, `5 : 16`. Same physics, opposite
   reading; a reader who carries `5:16` across files as "dots per CPU cycle"
   gets 0.3125 dots/cycle. Use the platform convention (`3:1`, `16:5`, Dendy
   `3:1`) or print both directions explicitly.

4. **`04-ppu-timing.md` §8 — the odd-frame clear equation is inverted and drops
   the `RESCL` gate** (lines 209–215; the same inverted form is in the §2 table,
   line 55). The file writes `HC = ~(HPLA_23 | EvenOddOut)`. In
   `HDL/PPU/fsm.v` `HVCounterControl` (line 324) `~(HPLA_23 | EvenOddOut)` is
   the *D input* of an **inverting** `dlatch`, so the net asserted on the
   counter is `HC = HPLA_23 | EvenOddOut` (`dlatch.nq = ~d`,
   `HDL/Common/dlatch.v`). Taken literally, the file's equation would clear H
   whenever the strobes are *inactive*. More seriously, §8 never states that
   `EvenOddOut = ~q2 & HPLA_5 & RESCL` (`fsm.v` line 250): `RESCL` is active
   only at V=261, and it is the term that confines the skip to the pre-render
   line. An implementer copying §8 shortens **every** scanline of an odd frame.
   The observable claim (340-dot pre-render line, rendering enabled only) is
   otherwise correct. Note `Specs/Review/ppu-render-sprites.md` major 3 already
   found the mirror-image wording in `04-ppu-rendering.md` §7.

5. **`04-ppu-timing.md` Open questions (lines 394–397) — the cross-file
   dot-numbering claim about `04-ppu-rendering.md` §1 is false.** It says the
   rendering file "defines dot = `H − 1` and prints that row at dot 339". The
   current `04-ppu-rendering.md` §1 says "**dot = H**", notes H counts 0–340 and
   `HPLA_23` clears at H = 340, and prints that row at **dot 340** (table rows
   `| 5 | 339 | … | 340 |` and `| 23 | 340 | … | 340 |`). My re-run of
   `fsm_test.v` confirms H ∈ 0–340 with the 340/339 wrap, and `PPU/hv.md` says
   "H тикает от 0 до 340 (включая)". **The rendering file is right and the
   timing file's Open question is stale.** The two files now agree on the clear
   dot; the only real residue is the +1-dot FSM latch shift of the *derived*
   windows (see Minor 7). Rewrite the bullet so a Revision agent does not "fix"
   the correct sibling.

## Minor

6. **`04-ppu-timing.md` §6 — every measured output window is one dot earlier
   than the HDL's settled outputs.** Probe (NTSC): FPORCH FF 257–279 (file
   256–278), BPORCH 271–328 (file 270–327), HBLANK 280–304 (file 279–303),
   BURST 309–323 (file 308–322), `/PICTURE` 329–340+0–270 (file
   328–340+0–269). The preamble promises "the final output carries one or two
   extra `PCLK` latch delays", so either state that the table lists the
   decoder/FF set-reset dots or add the +1.

7. **`04-ppu-timing.md` §6 (line 178–179) vs `04-ppu-rendering.md` §2 — the
   emulator-facing fetch schedule is off by one between the two files:**
   timing "256–319 the sprite fetch slots (`OBJ_READ`); 320–335 … (`/FO`);
   336–340 dummy" vs rendering "dots 257–320", "321–336", "337–340 dummy".
   Both are self-consistent with their own convention (raw `HPLA` value vs
   latched window), but the cross-file numbers disagree; pick one and say which.

8. **`04-ppu-timing.md` §11 (line 281) — "`$2002[7]` is random".** NESdev
   `PPU_power_up_state` gives PPUSTATUS as `+0+x xxxx` at power and
   `U??x xxxx` after reset: the VBL flag is random at power and **unchanged**
   by reset ("random at power, and unchanged by reset"). Say so.

9. **`04-ppu-timing.md` Provenance (line 361) over-attributes the CPU clocks.**
   "Frame geometry and CPU:PPU ratios | `PPU/hv.md`, `PPU/pclk.md` | `hv.v`,
   `pclk.v`": `hv.md`/`pclk.md` give the dot geometry and CLK/PCLK only; the CPU
   PHI0 rates, CPU:PPU ratios and the 60.0988/50.007 Hz frame rates come from
   NESdev `Clock_rate`/`Cycle_reference_chart` (already listed in the
   correspondence table). Split the row.

10. **`04-ppu-palette.md` §2 (lines 133–135) cites the wrong NESdev page.** The
    revision-gated palette readability ("2C02E and earlier … not readable",
    RGB 2C04) is not on `PPU_palettes`; it is on `PPU_variants` ("Early
    revisions cannot read back sprite or palette memory") and in the cited
    `Docs/NES_Famicom_Board_Revisions.md` §4 ("RP2C02E-0 … OAMDATA and palette
    RAM are **not readable**"). The correspondence table already lists
    `PPU_variants` for this.

11. **`04-ppu-palette.md` Provenance (line 364) claims HDL support for a swap
    the HDL does not implement.** "PAL doubled decoder, V0 alternation, R/G
    swap | `PPU/pal.md` | `video_out.v` (RP2C07)": in `HDL/PPU/video_out.v`
    the emphasis wiring is revision-independent — `assign n_PR = PZ[0]; assign
    n_PG = PZ[9];` (lines 219–220) and `Emphasis` (lines 289–308) have no
    `ifdef RP2C07`; only the 25-row decoder and the V0 bank select are gated.
    The claim itself is sound (rank 1 `PPU/pal.md` "перепутано соединение …
    /PR и /PG"; rank 4 `Cycle_reference_chart`: PAL "Blue (D7), red (D6),
    green (D5)"); fix only the HDL column (e.g. "not modelled").

12. **`04-ppu-palette.md` §5 (line 258) omits the PAL burst reference.** NESdev
    `PPU_palettes` §2C07: the PAL PPU "has a -15 degree hue shift relative to
    the 2C02 due to a different colorburst reference phase generated by the PPU
    (`$x7` rather than `$x8`)". The file's "Colour burst | 0° every line | 0° …"
    is right for the 2C02's internal phase-vs-burst relationship
    (`PPU/video_out.md` ties the burst to chroma 8), but the PAL difference
    should be flagged (silicon source is rank 4 here; the 25-row `pal.md` mask
    table does not map rows to colours).

13. **`04-ppu-palette.md` §6 — RGB differences incomplete.** NESdev
    `PPU_palettes` §2C03 and 2C05: the RGB parts render hue `$D` as **black**,
    not dark grey (the file's §3 renders `$D` as the `$00` grey at its luma,
    which is correct for the 2C02 only). Also worth a clause: the RGB palette
    is scrambled on the 2C04 ("completely different palettes as a copy
    protection measure").

14. **`04-ppu-palette.md` §2 (lines 116–119) — "emits a pixel one dot late" is
    not quantified by any source.** `PPU/cram.md` only says the CB outputs pass
    through "unequal D-latch chains" so the video generator outputs "pixels from
    the past"; the HDL delays only the chroma path (`video_out.v`
    `ChromaDecoder`, two latches) and feeds `n_LL` straight to `LumaDecoder`.
    Mark the exact one-dot figure as a model choice (or move it to Open
    questions) rather than a sourced fact.

## Checked against

- **Rank 1 (Breaking NES Wiki):** `PPU/hv.md` (H 0–340, V 0–261, carry patches),
  `PPU/hv_decoder.md` (NTSC H/V PLA rows, VB/BLNK gating), `PPU/pal.md`
  (2C07 HPLA rows 0/17–22, V rows, doubled chroma decoder, V0, R/G swap,
  `/PICTURE` delay), `PPU/fsm.md` (signal direction, EVEN/ODD, front/back porch
  names), `PPU/pclk.md` (CLK/PCLK rates, ÷4/÷5, power-on/reset latch state),
  `PPU/umc6538.md` (VPLA_4 +50 lines, DAC, OAM1 write block, H decoder
  unchanged), `PPU/cram.md` (CB, ROW0_4, COL/ROW decode, /BW chroma clear,
  output delay), `PPU/video_out.md` (12-phase decoder table, luma levels, TINT
  ×0.746, DAC L0–L9), `PPU/rgb.md` (pin table, 16×12 decoders, emphasis,
  no palette/OAM read, Even/Odd grounded), `PPU/mux.md` (PAL0–4, EXT,
  direct colour via `$2006`), `PPU/vram_ctrl.md` (read buffer, `TH/MUX`),
  `BreakingNESWiki/Dendy.md`, `PPU/waves.md`; the rank-1 line diagrams
  `imgstore/ppu/ppu_ntsc_line.png` and `ppu_pal_line.png`.
- **Rank 2 (HDL, re-run under Icarus 14.0):** `PPU/hv_decoder.v` — I
  re-decoded all 24 H rows (NTSC and RP2C07) and all 10 V rows programmatically
  and reproduced the file's §3/§4/§5 tables exactly, including the VB/BLNK
  gating lists; `PPU/fsm.v` (`HVCounterControl`, `EvenOdd`, `HPosLogic`,
  `VPosLogic`, `VBlankInt`) for the §2/§6/§8 signal equations; `PPU/pclk.v`
  (4/6 latch divider, inverting `dlatch` semantics); `PPU/cram.v`
  (`CRAM_Decoder` ROW0_4/COL decode, `cram[index]` 6-bit, `n_CC = BW ? … :
  1111`, `n_LL`); `PPU/video_out.v` (decoder masks, `Emphasis`, `VidOut`,
  `PhaseShifter`) and `PPU/dac.v`; `HDL/Common/{dlatch,rsff}.v`.
  `HDL/Framework/Icarus/ppu/fsm_test.v` — **TEST PASS, 1586 checks (RP2C02) /
  1886 checks (RP2C07)**; `vidout_test.v` — **TEST PASS, 32930 (RP2C02) /
  56055 (RP2C07)**. A temporary probe testbench was used to measure the
  §6/§8 windows and deleted afterwards; no spec file was touched.
- **Rank 3 (Docs):** `Docs/PPU/VideoSignal/VDAC.TXT` (2C02G/2C07/UA6538
  measured levels — reproduced exactly by the palette §3 table; "PPU has no
  BLACK level"; TINT does not affect SYNC/BURST),
  `Docs/NES_Famicom_Board_Revisions.md` §4 (RP2C02E-0 palette not readable,
  2C07 71-line vblank, red/green emphasis swapped).
- **Rank 4 (NESdev cache, `/tmp/nesdev-cache/`):** `Cycle_reference_chart`
  (Dendy ÷15 = 1.773448 MHz, dots/cycle 3, 113 2⁄3, 35464, 50.0070 Hz; PAL
  16:5/3.2; NTSC 60.0988 Hz), `CPU_variants` (UA6527P ÷15), `PPU_frame_timing`
  (odd-frame skip from (339,261), rendering-on condition, VBL clear = 20
  scanlines), `PPU_rendering` (VBL set at tick 1 of 241; (339,261)→(0,0)),
  `PPU_registers` ("scanline 241, dot 1"; ignored-write window 29658/33132),
  `PPU_power_up_state` (reset table, `U??x xxxx`, Dendy 35464-cycle frame),
  `PPU_palettes` (SAAPP index, entry-0 sharing, mirrors, `VV HHHH`, hue $0/$D,
  $E/$F = black, 2C03 $D = black, 2C07 burst $x7, RGB full-on emphasis),
  `PPU_variants`, `PPU_pinout` (RGB pins 14–16/17/21), `NTSC_video`
  (scanline timing: hsync 277+25, back porch 302, colorburst 306+15).
- **Cross-file (workspace):** `Specs/01-platform-overview.md` §2 + Open
  questions (round 3) for the Dendy ÷15 / 3:1 convention and the recorded
  disagreement; `Specs/04-ppu-memory-map.md` §6–§7 (alias table
  `$3F10…/$3F00…`, `$3F20–$3FFF` mirror, palette-read bypass, per-revision
  readability) — **matches `04-ppu-palette.md` §1/§2**; `Specs/04-ppu-rendering.md`
  §1–§2 (dot = H, dot 340 clear, fetch windows); `Specs/Conventions.md` §5–§7;
  `Specs/_template.md`.
- **Template/length:** both files carry `**Status:** adapted (round 1)`; the
  five fixed headings are present in the correct order with no number prefix
  (timing 288/305/331/346/364; palette 296/315/338/352/371); both are exactly
  400 lines (within the 120–400 target; note they are *at* the ceiling, so any
  correction must not grow them — several fixes below can be net-neutral).

## Notes for the next round

1. Fix the three blocking/major timing items first: the §6 `SYNC` row
   (use HBlank-window numbers), the Dendy row in §10, and the §8 `HC`/`RESCL`
   equations. The Dendy fix is a three-cell edit; the `SYNC` fix is two cells
   plus line 179.
2. `04-ppu-palette.md` has **no blocking defect** — every checked table
   (32×6-bit RAM, ROW0_4/COL decode, `%LLCCCC`, greyscale-chroma-only, 12-phase
   table, `TINT` ×0.746, PAL V0 alternation, RGB pins/emphasis) matches ranks
   1–4. Its findings are traceability/precision minors only.
3. Reconcile the `CPU:PPU` column direction with `01-platform-overview.md` §2
   (`3:1`, `16:5`) rather than re-deriving it; the platform file already warns
   that `05-boards-and-io.md` carries the older `5:16`-style table, and this
   file would be the third variant if it keeps "CPU cycles : dots".
4. When rewriting the dot-numbering Open question, state that both files use
   `dot = H` / dot 340 for the clear, and record only the FSM latch shift
   (Minor 7). `04-ppu-rendering.md` §1 must not be changed to match this file.
5. The `SYNC` window is the only §6 row that another spec quotes indirectly
   (`04-ppu-palette.md` §5 quotes the FPORCH/BPORCH/HBLANK/BURST decoder dots
   only and stays valid); after fixing §6, no other spec needs an edit.

## Revision round 2

- Blocking 1: fixed — `04-ppu-timing.md` §6 now lists the settled `HPosLogic`
  outputs (preamble states the +1 `PCLK` latch stage, citing
  `04-ppu-rendering.md` §1). `SYNC` is **280–304 NTSC / 278–302 PAL** (25 dots,
  ≈4.7 µs) and `/FPORCH` is kept as a separate row (257–279 / 257–277); the
  preamble gives `SYNC = ~(sync_latch2_q | VSYNC)`,
  `VSYNC = ~(n_HB | vsync_ff_q)`. Every derived line moved with it: the per-dot
  paragraph is now 257–320 `OBJ_READ` / 321–336 prefetch / 337–340 dummy (same
  as `04-ppu-rendering.md` §2) and places `SYNC` 280–304, `BURST` 309–323 in the
  blanking interval after the front porch. The whole table was re-measured with
  an Icarus probe on `fsm.v` (NTSC FPORCH 257–279, HBLANK 280–304, BPORCH
  271–328, BURST 309–323, `/PICTURE` 329–340+0–270; PAL FPORCH 257–277, HBLANK
  278–302, BPORCH 257–340+0–4, BURST 307–321, `/PICTURE` 5–256).
- Major 2: fixed — §10 Dendy row is now `1.773448 MHz (÷15) | 5.3203424 MHz |
  3 : 1 | 3 | 341/3 = 113⅔`; §9 and a new §10 Dendy bullet give 35 464 CPU
  cycles per frame (`Cycle_reference_chart`, `CPU_variants`,
  `PPU_power_up_state` §Dendy). The "Dendy CPU clock" Open question no longer
  calls the row unsourced/approximate: it cites those rank-4 pages and
  `01-platform-overview.md` §2.
- Major 3: fixed — §10's column is renamed `CPU:PPU` and states the platform
  convention (numerically **PPU dots per CPU cycle**: `3:1`, `16:5`,
  explicitly "**not** `5:16`"), matching `01-platform-overview.md` §2; the
  `1 : 3` / `5 : 16` cells are gone.
- Major 4: fixed — §2 and §8 now use `HC = HPLA_23 | EvenOddOut`, noting that
  `~(HPLA_23 | EvenOddOut)` is only the inverting `dlatch`'s D input; §8 adds
  `EvenOddOut = ~q2 & HPLA_5 & RESCL` and says `RESCL` (active only at V = 261)
  confines the skip to the pre-render line.
- Major 5: fixed — the dot-numbering Open question now states that both files use
  dot = `H`, that `HPLA_23` clears at dot 340, and that
  `04-ppu-rendering.md` §1 is correct; only the one-`PCLK` latch shift of the
  derived windows is recorded. That sibling file was **not** changed.
- Minor 6: fixed — every §6 row now carries the +1 latch shift.
- Minor 7: fixed — §6's per-dot schedule uses the rendering file's settled
  numbers (257–320 / 321–336 / 337–340).
- Minor 8: fixed — §11 says `$2002[7]` is random at power-on and **unchanged by
  reset**.
- Minor 9: fixed — Provenance split into "Frame geometry" (two files/HDL) and
  "CPU `PHI0` rates, CPU:PPU ratios, frame rates" (NESdev rank 4, no Wiki/HDL).
- Minor 10: fixed — `04-ppu-palette.md` §2 now cites NESdev `PPU_variants`
  ("Early revisions cannot read back sprite or palette memory") instead of
  `PPU_palettes`, keeping `Docs/NES_Famicom_Board_Revisions.md` §4.
- Minor 11: fixed — palette Provenance HDL cell now reads "decoder/V0 in
  `video_out.v` (`RP2C07`); R/G swap **not modelled**" (`n_PR = PZ[0]`,
  `n_PG = PZ[9]` are unconditional and `Emphasis` has no `RP2C07` branch).
- Minor 12: fixed — §5 burst row is code `$8` on NTSC vs `$7` on PAL with the
  −15° hue shift, plus a rank-4 note that the 25-row mask table maps no rows to
  colours.
- Minor 13: fixed — §3's `$D` row, `Observable phenomena` 6 and §6 now state
  that RGB parts render hue `$D` as black (2C02 grey is 2C02-only); §6 also adds
  the 2C04 scrambled-palette copy protection.
- Minor 14: fixed — §2 states the HDL delays only the chroma path (two
  `ChromaDecoder` latches) and feeds `n_LL` straight to `LumaDecoder`, marking
  "one dot late" a **model choice**; `Observable phenomena` 3 and the SHOULD note
  follow.
- Refutations: none. No finding was deferred.
- Verification/limits: the Icarus probe testbench was temporary and deleted; no
  HDL file was touched. Lengths after the round: `04-ppu-timing.md` 395 lines,
  `04-ppu-palette.md` 400 lines (both ≤ 400). `04-ppu-rendering.md`,
  `01-platform-overview.md` and every other file were left untouched.
