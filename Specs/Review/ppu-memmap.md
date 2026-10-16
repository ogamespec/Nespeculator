# Review: ppu-memmap — round 3

Re-verification (final round) of `/mnt/c/Work/breaks/Specs/04-ppu-memory-map.md`,
**Status:** revised (round 3), 292 lines, md5
`0274a39e167923d6cdcfb813a0857945`, mtime 2026-10-13 23:24:56 — identical to the
`## Revision round 3` note kept below. Rounds 1–2 history is preserved. No spec
file was edited; no `git commit`.

## Verdict

consensus

## Blocking

None. Round-2 Major 1 is resolved: §8's byte `+0` row (line 171) now reads
``Sprite Y coordinate (top edge; written value = top scanline − 1, so the sprite
first appears on scanline `value + 1`)``. Cached NESdev `PPU_OAM` says verbatim
"Sprite data is delayed by one scanline; you must subtract 1 from the sprite's Y
coordinate before writing it here", so written value = top − 1 and top =
value + 1; the sibling `04-ppu-sprites.md` agrees both ways (§"One-line offset"
line 83 "a sprite whose Y byte is `Y` first appears on scanline `Y+1`", observable
#1 line 267 "appears one scanline lower than its Y byte … Model the +1 line
offset"). All three now say the same thing. A file-wide grep finds this as the
only Y-offset statement in the file, so no internal contradiction was introduced.

## Major

None. Round-2's three Minors (2–4) are also applied:
- Open questions (line 285) no longer calls `04-ppu-scrolling.md` §2 stale. The
  sibling's current §2 prints `AT_ADR[9:6] = %1111  constant $3C0 attribute-table
  offset` (line 70) and "`AT_ADR[9:6]` is a constant field, not a function of
  `TVO`/`THO`"; a grep of `Specs/04-ppu-scrolling.md` for `TVO[2:1]`/`AT_ADR[9:8]`
  finds nothing (rc=1). `04-ppu-rendering.md` §3 (line 120) prints
  `%10NN 1111 yyy xxx — … bits [9:6] = 1111`. The bullet correctly records
  agreement and leaves only the `HDL/PPU/tilecnt.v` tie-off as the inconsistency.
- `**Primary sources:**` (line 5) now adds `PPU/umc6538.md` (cited line 186,
  provenance line 278) and HDL `mux.v` (provenance 272), `bgcol.v` (provenance
  269) and `fsm.v` (line 207, `DelayedH`); all four files exist in the checkout.
- §7 (line 162) reads "on the 2C02E and all earlier NTSC parts and on the RGB
  2C03, 2C04 and 2C05 series (`04-ppu-registers.md` §9, `PPU_variants`:
  RC2C05-01…04 behave like the 2C03, RC2C05-99 like the 2C02E)". Cached
  `PPU_variants` has the 2C03 family "OAMDATA and PPU palette are not readable",
  RC2C05-01…04 "believed to behave like 2C03", RC2C05-99 "An RGB PPU with
  RP2C02E behavior: OAM and palettes are unreadable". The programming note
  (line 235) now lists the same set with the same RC2C05-99 exception, matching
  `04-ppu-registers.md` §9 ("on 2C02E and earlier, and on the 2C03, 2C04 and the
  whole 2C05 series"). The former self-contradiction is gone and §7 agrees with
  the note and the sibling.

## Minor

Residual only; nothing blocking, nothing that should keep the section open.

1. **§7 "the read returns the shadowed VRAM byte instead" (line 162) is loose
   phrasing, not an error.** On a revision whose palette is unreadable the read
   goes through the ordinary `$2007` buffer path (`04-ppu-registers.md` §9
   "behaves like ordinary VRAM"), so what the CPU gets is the *buffered* byte,
   i.e. the previously shadowed value. Pre-existing wording carried from round 2,
   correctly not changed in round 3; an implementer is not misled.
2. **§1 line 21 gives the palette mirror as "224 bytes" while §6 (line 150) and
   observable #4 (line 218) describe it functionally.** Both are correct
   (`$3F20–$3FFF` = 0xE0 = 224); no reconcile needed, noted only because the
   rank-2 citation and the byte count live on the same row.

## Checked against

- `/mnt/c/Work/breaks/Specs/04-ppu-memory-map.md` — full re-read (292 lines).
  §1 map, §2 PAR table/diagram, §4 `AT_ADR` formula, §5 `v`/`t` field tables,
  §6 palette index and mirror, §7 read buffer/bypass, §8 OAM tables, §9 VRAM
  controller rows all unchanged from the round-2-reviewed text; the round-3
  edits are confined to lines 5, 162, 171, 235, 285.
- Cached NESdev `PPU_OAM` (Wayback) — byte 0 "you must subtract 1 … before
  writing it here"; "Bits that have decayed also read back as 0".
- Cached `PPU_variants` — 2C02E-and-earlier/palette unreadable; RP2C03B
  "OAMDATA and PPU palette are not readable"; RC2C05-01…04 like 2C03;
  RC2C05-99 RP2C02E behaviour, "OAM and palettes are unreadable".
- Siblings: `04-ppu-sprites.md` §"One-line offset" (line 83) + observable #1
  (line 267); `04-ppu-scrolling.md` §2 (lines 69–75) `AT_ADR[9:6] = %1111`
  constant field; `04-ppu-rendering.md` §3 (line 120) `%10NN 1111 yyy xxx`;
  `04-ppu-registers.md` §9 (line 232) palette-read revision list.
- HDL: `HDL/PPU/tilecnt.v` 459–462 (`AT_adr[6..9] = 1'd0` — the defect note in
  §4 line 92 and Open questions line 284 is still present and accurate);
  `HDL/PPU/cram.v` 31/107 `input [4:0] CGA`, 138/142 `cram[CGA]` — the
  `$3F20–$3FFF` mirror is still rank 2 (Open questions line 287, §1 line 21,
  §6 line 154) and was not relabelled; `HDL/PPU/fsm.v` `DelayedH`,
  `mux.v`, `bgcol.v`, `umc6538.md` all present.
- Template: `Status: revised (round 3)` accurate; five fixed headings present,
  in order, exact and unnumbered (lines 211, 227, 245, 260, 282); 292 lines
  (120–400); md5 matches the `## Revision round 3` note. No `git commit`.

## Notes for the next round

- No further round is required. All round-1 and round-2 blocking/major findings
  are resolved; the only open item that remains is the `HDL/PPU/tilecnt.v`
  `AT_adr[6..9]` tie-off, which is an HDL defect correctly recorded in the
  spec's `## Open questions` (§4 cross-references it), not a spec error.
- The two residual Minors above are wording-level and are not a reason to keep
  the section open.
- Round-2 Minors 2–4 and Major 1 are all confirmed fixed as the revision note
  claims; no finding is refuted and no new error was introduced.

---

# Review: ppu-memmap — round 2

Re-verification of `/mnt/c/Work/breaks/Specs/04-ppu-memory-map.md`,
**Status:** revised (round 2), 292 lines, md5
`d5b3c646421ff56b6081f3af6e1cb1d7`, mtime 23:10:26 — identical to the
`## Revision round 2` note kept below. Round-1 history is preserved. No spec
file was edited; no `git commit`.

## Verdict

revise

## Blocking

None. Round-1 Blocking 1 is resolved and the replacement text is correct. §9 now
has separate `/RD` (pin 24) / `/WR` (pin 23) rows for the **VRAM controller** and
`/RD_CPU`/`/WR_CPU` rows (HDL `n_CpuRD`/`n_CpuWR`) for the R/W decoder; the note
after the table (line 207) and the programming note (line 242) repeat the split.
Re-derived from HDL: `vram_ctrl.v` 92–100/113 gives
`RD = (latched H0_D & ~BLNK) | PD_RB` with **no** `/DBE` term (and
`WR = DB_PAR & ~TH_MUX`); `regs.v` `RWDecoder` (305–320) is the complementary /
both-`1`-at-`/DBE = 1` pair; `ppu_top.v` 202–203 routes it to `n_CpuRD`/`n_CpuWR`,
`pads.v` 51–53 sends those to the D0–D7 pads only, and `pads.v` 59–60
(`n_RDPad = ~RD_topad`) takes the VRAM pins from `VRAM_Control` (`ppu_top.v`
483–484). This agrees with `04-ppu-overview.md` §4 (122–131) and its programming
note (287–291) and with `pads.md` `/RD`=24 / `/WR`=23 (whose own complementarity
sentence the HDL refutes — correctly not carried over).

## Major

1. **§8 (line 171) gives the sprite Y-coordinate offset with the wrong sign.**
   The row reads ``Sprite Y coordinate (top edge; value = top scanline + 1)``.
   NESdev `PPU_OAM`: "Byte 0: Y position of top of sprite ... you must subtract 1
   from the sprite's Y coordinate before writing it here" → written value =
   top − 1, i.e. `top = value + 1`. `04-ppu-sprites.md`, the sibling this table
   is required to agree with, states it correctly and the other way round:
   §"One-line offset" (line 83) "a sprite whose Y byte is `Y` first appears on
   scanline `Y+1`", observable #1 (line 267) "A sprite appears one scanline
   lower than its Y byte ... Model the +1 line offset". The file's equation is
   the inverse of both. Fix to `value = top scanline − 1` (or "top scanline =
   value + 1"). Pre-existing (round 1 checked the +2 attribute row, not the +0
   row), so it does not disturb the round-1 findings.

## Minor

2. **Open questions (line 285) is stale about `04-ppu-scrolling.md` §2.** The
   bullet says that sibling "still gives an `AT_ADR` composition
   (`AT_ADR[9:8] = TVO[2:1]`, …)" and "needs the same correction". The current
   `04-ppu-scrolling.md` §2 (lines 68–72, mtime 23:09:20, **earlier** than this
   file's 23:10:26) prints `AT_ADR[9:6] = %1111` and states "`AT_ADR[9:6]` is a
   constant field, not a function of `TVO`/`THO`"; a grep of
   `Specs/04-ppu-scrolling.md` for `TVO[2:1]`/`AT_ADR[9:8]` finds nothing. Keep
   the `04-ppu-rendering.md` half and drop or rewrite the `04-ppu-scrolling.md`
   half. (Same class as round-1 Minor 3.)
3. **`**Primary sources:**` (line 5) still omits files cited in the body or
   provenance:** `PPU/umc6538.md` (line 186, provenance 278), `HDL/PPU/mux.v`
   (provenance 272), `HDL/PPU/bgcol.v` (provenance 269) and `HDL/PPU/fsm.v`
   (line 207). Round-1 Minor 9 was only partly fixed: the six named files were
   added and the never-cited `Docs/PPU/USPatent/OAM_FIFO.txt` dropped.
4. **§7 (line 162) over-generalises the 2C05 palette-read behaviour.**
   "the whole 2C05 series behaves like the 2C03, the RC2C05-99 like the 2C02E" is
   self-contradictory; NESdev `PPU_variants` has RC2C05-01…04 "believed to behave
   like 2C03" and RC2C05-99 with "RP2C02E behavior: OAM and palettes are
   unreadable", as does `04-ppu-registers.md` §9 ("RC2C05-01…04 behave like the
   2C03, RC2C05-99 like the 2C02E"). Say "RC2C05-01…04". Related: the programming
   note (line 235) still lists only "2C02E-and-earlier/2C04", contradicting §7's
   2C02E + 2C03 + 2C04 (+2C05) list in the same file.

## Checked against

- `HDL/PPU/vram_ctrl.v` — `RD = (latched H0_D & ~BLNK) | PD_RB` (w38/w34/w39/
  w40, lines 92–100, latch `u10` line 113), `WR = DB_PAR & ~TH_MUX` (95),
  `TH_MUX = ~(w12 | nPA[0..5])` (78/81), no `/DBE` term.
- `HDL/PPU/regs.v` `RWDecoder` (305–320); `HDL/PPU/ppu_top.v` 30–31, 202–203,
  472–490 (`n_CpuRD`/`n_CpuWR`; `.RD`/`.WR`; `.nPA(n_PA_out[13:8])`);
  `HDL/PPU/pads.v` 51–60; `HDL/PPU/fsm.v` 217–220 (`H0_D = ~h0_latch1_nq`).
- `HDL/PPU/tilecnt.v` 459–462 (`AT_adr[6..9] = 1'd0`), 463–464
  (`AT_adr[13] = FVO[1]|~BLNK`, `AT_adr[12] = 0` while rendering).
- `Logisim/Evolution/PPU_Evo.circ`, circuit `TileCounters_All`: exactly one
  `Constant`, bare (no `value`), at `(1120,300)`; its net is the splitter
  `(1160,230)` fanout pins `(1140,300)…(1140,330)`, i.e. `AT_ADR[6..9]` — the
  §4/Open-questions defect note is confirmed and unchanged.
- `HDL/PPU/cram.v` — `input [4:0] CGA` (31/107), `cram[0:31]` (116),
  `cram[CGA]` (138/142), `ROW0_4` (65/73–75): the `$3F20–$3FFF` mirror stays
  rank 2 and only its citation changed.
- Cached NESdev pages: `PPU_memory_map` ("usually a mirror of the 2kB region
  from `$2000-2EFF`; The PPU does not render from this address range");
  `PPU_OAM` (byte 0 subtract 1; attribute bits 2–4 "always read back as 0";
  "Bits that have decayed also read back as 0 through OAMDATA");
  `PPU_variants` (2C02E-and-earlier and RP2C03B have unreadable palettes;
  RP2C04 "behaves like 2C03C"; RC2C05-01…04 like 2C03; RC2C05-99 RP2C02E
  behaviour).
- Siblings: `04-ppu-overview.md` §4 + programming note; `04-ppu-scrolling.md`
  §1 (field bits) and §2 (`AT_ADR[9:6] = %1111`); `04-ppu-rendering.md` §3
  (`%10NN 1111 yyy xxx`); `04-ppu-registers.md` §9 (palette-read revisions);
  `04-ppu-sprites.md` §1 + §"One-line offset"; `04-ppu-palette.md` §4/mirrors.
- `BreakingNESWiki/PPU/par.md` bit table (memory-map §2 matches row-for-row);
  `PPU/pads.md` pins 23/24; `PPU/oam.md` (2112 = 1856 + 256, 29 bits/sprite);
  `Docs/PPU/OAM_RowCol.txt` (34+32 rows; groups 8,8,5,8,8,8,5,8,8 = 66 rows;
  quoted `1792 + 320`).
- Template: five fixed headings present, in order, exact and unnumbered (211,
  227, 245, 260, 282); 292 lines (120–400); `Status: revised (round 2)` matches
  the repo-wide convention. Core map/formulas re-read and otherwise unchanged:
  §2 PAR table, §4 attribute formula + `AT_ADR`, §5 field/fetch tables, §6
  palette index, §7 read buffer/bypass, §8 byte order/bit fields (§8's +0 gloss
  excepted, Major 1) — all agree with the siblings and HDL. The `$3000–$3EFF`
  rank-4 marking is present in §1, §3, observable #8, the programming note (now
  SHOULD) and the provenance row (`—`/`—`); the palette mirror is never
  relabelled rank 4.

## Notes for the next round

- Round-1 Blocking 1, Major 2 and Minors 3–9 are all addressed as the revision
  note claims; no fix is refuted and no new blocking item exists. The palette
  mirror was correctly left rank 2 and only re-cited.
- Remaining work is the Major 1 one-line Y-sign fix plus Minors 2–4; all are
  local text edits and none re-opens a round-1 finding.
- With Major 1 fixed and Minors 2–4 applied, the file should be ready for
  consensus on the next pass.

---

# Review: ppu-memmap — round 1

File under review: `/mnt/c/Work/breaks/Specs/04-ppu-memory-map.md`,
**Status:** adapted (round 1), 285 lines, mtime 22:31:55. No earlier report
existed in this file. No spec file was edited.

## Verdict

revise

## Blocking

1. **§9 `/RD` row (line 194) conflates the two `/RD`/`/WR` pairs and states the
   opposite of the VRAM controller's behaviour.** The row reads
   ``VRAM read strobe; complementary to `/WR` while `/DBE = 0`, both inactive
   when `/DBE = 1` `` and is sourced to `PPU/vram_ctrl.md`. That sentence is the
   *CPU-interface* R/W-decoder claim in `BreakingNESWiki/PPU/regs.md` §"R/W
   Декодер" ("Сигналы /RD и /WR комплементарны когда /DBE = 0. Если /DBE = 1
   ... оба сигнала равны `1`") — it is about the D0–D7 direction pair
   (`HDL/PPU/ppu_top.v` `n_CpuRD`/`n_CpuWR`, matched to `pads.v`), **not** the
   VRAM-controller outputs this table documents. `PPU/vram_ctrl.md` says only
   "RD: на выход /RD", "WR: на выход /WR"; it says nothing about `/DBE`.
   - `HDL/PPU/vram_ctrl.v` lines 96–100: `w39 = w38 & w34` (`w34 = ~BLNK`),
     `w40 = ~(w39 | PD_RB)`, `RD = ~w40` → `/RD = (latched H0_D & ~BLNK) | PD_RB`.
     There is **no `/DBE` term**. `H0_D = ~h0_latch1_nq` is the H-counter LSB
     delayed one latch (`HDL/PPU/fsm.v` `DelayedH`, lines 217–220), i.e. it
     toggles on alternate dots, so `/RD` is asserted on alternate dots of every
     rendering scanline *while `/DBE = 1`*. (`VRAM_Control`'s `.nPA` is
     `n_PA_out[13:8]`, `HDL/PPU/ppu_top.v` line 480.)
   - `Specs/04-ppu-overview.md` §"Programming / implementation notes" lines
     287–291 states the correct split explicitly: the decoder's `/RD_CPU`/
     `/WR_CPU` are the complementary/`1`-when-`/DBE` pair, while "the VRAM
     `/RD`/`/WR` pins 24/23 come from the VRAM controller and stay active during
     rendering with `/DBE = 1`". That sibling (mtime 23:01:04) was corrected
     after this file (22:31:55); the same defect was round 1's Blocking 1 in
     `Specs/Review/ppu-overview-regs.md`.
   Impact: an implementer following §9 would suppress the VRAM read strobe
   during rendering and break every fetch. Fix: split the row (name the decoder
   pair `/RD_CPU`/`/WR_CPU`, cite `PPU/regs.md`; state that the VRAM `/RD`/`/WR`
   pins are active during rendering, cite `PPU/vram_ctrl.md` + `vram_ctrl.v`).

## Major

2. **The `$3000–$3EFF` mirror is presented as a board-decode fact, but the
   repository has no rank 1–3 source for it.** It appears as fact in §1
   (line 19, "on the standard board a mirror of `$2000–$2EFF` (PPU A12 not
   decoded)"), §3 (line 60, "the board decode ignores `PA12`"), Observable
   phenomena #8 (line 218), and Programming notes (line 227, "MUST mirror
   `$3000–$3EFF` onto `$2000–$2EFF` on standard boards"). The file's own Open
   questions (line 280) concedes no named source states it, and the Provenance
   row (line 262) claims `pamux.v`/`Docs/NES/NES_HDL_Model/nes.v` support.
   Evidence that they do not:
   - `Docs/NES/NES_HDL_Model/nes.v`: `CartPort` (line 280) and `LS139` (line 353)
     have **empty bodies**; `VRAM_nCE` is only declared. The file contains no
     A12/PA12 decode at all, so it cannot be cited for "the board decode ignores
     PA12". `HDL/PPU/pamux.v` only muxes the address; it has no decode either.
   - The rank-4 source, cached `/tmp/nesdev-cache/PPU_memory_map.html`, says
     "`$3000-3EFF` is **usually** a mirror of the 2kB region from `$2000-2EFF`"
     and does not mention A12/PA12.
   - The corrected sibling `Specs/01-platform-overview.md` §5 (mtime 22:47:56)
     now marks exactly this claim "NESdev; the board decode that ignores PPU A12
     is not documented in this repository's rank 1–3 sources", and its round-2
     report (`Specs/Review/platform.md` lines 239–242, 523–531) flags
     `04-ppu-memory-map.md` §1/§3/§218/§227 as still presenting it as fact.
   Fix: mark the `$3000–$3EFF` mirror **NESdev (rank 4) / not documented in this
   repository's rank 1–3 material**, drop `nes.v`/`pamux.v` from that Provenance
   row (or set both source columns to `—`), and soften the Programming-notes
   `MUST` accordingly.

## Minor

3. **Open questions (line 278) is stale about `04-ppu-rendering.md` §3.** It
   claims that sibling "prints the attribute address as `%0010 3NN YYYY YXXX`
   and says 'bits 5–6 forced to `11`'". The current `04-ppu-rendering.md` §3
   (line 119, mtime 23:03:54, later than this file) prints
   `%10NN 1111 yyy xxx` and says bits `[9:6]` = `1111` — i.e. it now agrees with
   this file. Keep the `04-ppu-scrolling.md` §2 half of the bullet (that sibling
   still prints `AT_ADR[9:8] = TVO[2:1]` … at lines 69–71, which the HDL does
   not support), but drop the rendering half.

4. **Open questions (line 280) is wrong to put the `$3F20–$3FFF` mirror in the
   same "not stated" bucket as `$3000–$3EFF`.** The palette mirror is not
   rank-4-only: `HDL/PPU/cram.v` `CRAM_Block` takes a 5-bit `CGA` and indexes a
   32-entry `cram[CGA]` (lines 107, 113, 138, 142), and in direct-colour mode the
   index is the low 5 address bits (`TH`). Every address in `$3F00–$3FFF`
   therefore lands on `cram[v[4:0]]`, so the mirror follows from rank-2 HDL
   (as §6 line 154 already says). `HDL/PPU/cram.v` *is* one of the sources named
   for this brief. Fix the wording; add the HDL citation to the §1 row (line 21)
   so both mirrors are not left with the same provenance. Do **not** relabel the
   palette mirror "NESdev-only".

5. **§7 (line 162) contradicts the sibling it cites about which revisions can
   read palette RAM.** It says the range is unreadable "on 2C02E and earlier NTSC
   parts, and on the RGB 2C04", citing `04-ppu-registers.md` §9 — but that §9
   (line 232) lists "2C02E and earlier, and on the **2C03** and 2C04". Cached
   `/tmp/nesdev-cache/PPU_variants.html` confirms the 2C03 family ("RP2C03B …
   OAMDATA and PPU palette are not readable"; 2C04 "behaves like 2C03C";
   2C05-99 "OAM and palettes are unreadable"). Add the 2C03 (and the 2C05-99
   note) or cross-reference §9 without re-listing the parts.

6. **§8 gives two mutually inconsistent partitions of the same 2112 cells.**
   Line 181: "1856 bits of sprite data plus 256 bits (32 bytes) of secondary
   OAM" (64 × 29 = 1856). Line 183 quotes `Docs/PPU/OAM_RowCol.txt` verbatim:
   "= `1792 + 320 = 2112` bits" while also glossing the last group as "8
   secondary-OAM rows" (8 × 32 = 256, not 320; 1792 = 64 × 28, which contradicts
   the 29 bits/sprite claim). Both sums are 2112 but they are different splits.
   Keep the doc's figure only with a note that it disagrees with its own row
   grouping (58 primary rows + 8 secondary rows = 1856 + 256), or drop it.

7. **Open questions (line 283) misstates the NESdev evidence it relies on.**
   "it is not stated whether a decayed cell also reads back as 0" — cached
   `/tmp/nesdev-cache/PPU_OAM.html` states "Bits that have decayed also read
   back as 0 through OAMDATA." The rank 1–3 gap is real; the claim about what
   NESdev says is not.

8. **Open questions (line 285) calls the `TH/MUX` gating unstated, but the HDL
   in the same file's sources states it.** `HDL/PPU/vram_ctrl.v` lines 78–81,
   103: `w12 = ~BLNK` (latched), `TH_MUX = ~(w12 | nPA[0] | … | nPA[5])`, and
   `.nPA(n_PA_out[13:8])` (`ppu_top.v` line 480) → `TH_MUX = BLNK & PA[13:8] =
   %111111`, i.e. any `$3Fxx` address while blanking, independent of read vs
   write. The Wiki gap can stay, but the item can be resolved from
   `vram_ctrl.v` rather than left open.

9. **`**Primary sources:**` (line 5) omits sources cited in the body:**
   `PPU/dataread.md`, `PPU/regs.md`, `PPU/pads.md`, `HDL/PPU/scroll_regs.v`,
   `Docs/NES/NES_HDL_Model/nes.v`, and NESdev `PPU_registers`, `PPU_scrolling`,
   `CHR_ROM_vs._CHR_RAM` (the last one verified to exist:
   `.../wiki/CHR_ROM_vs._CHR_RAM`, oldid 20817). Only `Docs/PPU/USPatent/
   OAM_FIFO.txt` is listed but never cited.

## Checked against

- `/tmp/nesdev-cache/PPU_memory_map.html` — 14-bit `$0000–$3FFF`; per-range sizes
  (`$0F00`, `$0020`, `$00E0`); "usually a mirror of the 2kB region from
  `$2000-2EFF`"; "The PPU does not render from this address range"; "not
  configurable, always mapped to the internal palette control". §1 matches;
  the `$3000` "usually" is not carried over (Major 2).
- `/tmp/nesdev-cache/PPU_nametables.html`, `Mirroring.html` — 960 + 64 bytes,
  64×30 / 32×60 / 32×30 / 64×60, CIRAM A10 taps A10 (vertical/H pad) and A11
  (horizontal/V pad), $2000=$2800 / $2400=$2C00 etc. §3's four rows and the
  silkscreen note are correct.
- `/tmp/nesdev-cache/PPU_attribute_tables.html` — 8×8 bytes at `$23C0/$27C0/
  $2BC0/$2FC0`, quadrant formula `(br<<6)|(bl<<4)|(tr<<2)|(tl)`; §4's diagram and
  formula match; the right-edge/MMC5 notes match "Glitches"/"Expansion".
- `/tmp/nesdev-cache/PPU_scrolling.html` — `attribute address = 0x23C0 |
  (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07)`, `NN 1111 YYY XXX`,
  `tile address = 0x2000 | (v & 0x0FFF)`; coarse Y out-of-bounds wrap and
  Y=31→0 without nametable switch. §3/§4/§5 formulas match.
- `/tmp/nesdev-cache/PPU_pattern_tables.html` — `0HNNNN NNNNPyyy`, `%00`
  transparent, CHR-ROM/CHR-RAM; §2's diagram and
  `PTSEL·$1000 + tile·$10 + plane·8 + fineY` match.
- `/tmp/nesdev-cache/PPU_palettes.html` — `SAAPP` index, entry-0 aliasing
  ("writing to either one updates the same internal storage"), "mirrored through
  the entire $3F00-$3FFF region"; §6 matches; the palette mirror has rank-2 HDL
  backing too (Minor 4).
- `/tmp/nesdev-cache/PPU_registers.html` — read buffer "updated only on PPUDATA
  reads … not affected by … rendering"; palette reads return the entry
  immediately with a normal shadowed VRAM read landing in the buffer; PPUADDR
  bit 14 forced 0. §7 and §5 agree.
- `/tmp/nesdev-cache/PPU_OAM.html` — byte 2 `Palette(1–0)/Unimplemented(4–2)/
  Priority/Flip`, "always read back as 0 … decayed also read back as 0"; §8's
  byte table and the no-cells claim match (Minor 7).
- `/tmp/nesdev-cache/PPU_variants.html`, `CHR_ROM_vs._CHR_RAM` (Wayback, oldid
  20817) — palette-read revision list (Minor 5); CHR page exists.
- `BreakingNESWiki/PPU/vram_ctrl.md`, `par.md`, `pamux.md`, `tilecnt.md`,
  `cram.md`, `oam.md`, `mux.md`, `regs.md`, `umc6538.md` — signal lists, PAR bit
  table (bits 0–2 / 3 / 4 / 5–11 / 12 match par.md row-for-row), CRAM
  columns `{CGA3,CGA2}` / rows `{CGA4,CGA1,CGA0}` with rows 0 and 4 merged,
  OAM 2112 cells / 1856 + 256 / 29 bits per sprite / column-skip on attribute
  bits 2–4 / row-decode inversion on NTSC, TH/MUX direct colour with rendering
  off, R/W decoder claim (Blocking 1).
- `Docs/PPU/OAM_RowCol.txt` — 34 + 32 rows, groups 8,8,5,8,8,8,5,8,8, column and
  row one-hot maps; §8 reproduces it (Minor 6). `Docs/PPU/USPatent/OAM_FIFO.txt`
  read but not cited in the body.
- `Docs/NES_Famicom_Board_Revisions.md` §1.4, §5.3 — two 2K×8 SRAMs, U4 VRAM
  `$2000–$2FFF`, cartridge supplies CHR, 74LS373 low-address latch, 368 inverts
  PA13; §1/§9 claims supported. `Docs/NES/NES_HDL_Model/nes.v` — `CartPort` and
  `LS139` bodies are empty (Major 2).
- `HDL/PPU/tilecnt.v`, `par.v`, `pamux.v`, `vram_ctrl.v`, `cram.v`, `oam.v`,
  `pads.v`, `ppu_top.v`, `fsm.v`, `scroll_regs.v` — address assembly
  (`THO = {AT_adr[2],AT_adr[1],AT_adr[0],NT_adr[1],NT_adr[0]}`,
  `TVO = {AT_adr[5..3],NT_adr[6],NT_adr[5]}`), `AT_adr[13]=FVO[1]|~BLNK`,
  `AT_adr[12]=~(...)`, `AT_adr[6..9]=1'd0`; `/PA` active-low, `PAPads=~n_PA[13:8]`;
  ReadBuffer gated by `XRB`/`PD_RB`; OAM modelled as a plain 256×8 array with no
  attribute mask; `/RD` equation (Blocking 1); `TH_MUX` gating (Minor 8).
- `Logisim/Evolution/PPU_Evo.circ`, circuit `TileCounters_All` (parsed with the
  union-find/net method) — exactly one `Constant` in the circuit, at
  `(1120,300)`, a bare `<comp lib="0" name="Constant"/>` with **no** `value`
  attribute; its net joins the `AT_ADR` splitter fanout points
  `(1140,300)…(1140,330)` = `AT_ADR[6..9]`. File-wide, 45 of 70 Constants are
  bare and 25 carry `value="0x0"`, so the "bare Constant = Vdd" convention the
  spec relies on (as `pamux.v`/`cram.v` comments do) is consistent. §4's
  `AT_ADR[9:6] = %1111` and the Open-questions HDL-defect item are **verified**.
  `Docs`/HDL cannot be blamed here: `tilecnt.v` really does tie 6–9 to 0.
- Template: `Status: adapted (round 1)` correct; five fixed headings present, in
  order, exact and unnumbered (lines 207/223/240/255/275); 285 lines (120–400);
  no `0x` outside the quoted Verilog attribute; all 9 content sections appear in
  the NESdev correspondence table.

## Notes for the next round

- One blocking item and one major item to fix; the rest are wording/attribution
  and can be handled in the same revision. All three top findings are cases where
  a *later* sibling revision (22:47–23:03) already fixed the same issue and this
  22:31 draft still carries the old text, so cross-reading `04-ppu-overview.md`
  §Programming notes, `01-platform-overview.md` §5 and `04-ppu-rendering.md` §3
  gives the exact target wording.
- Do **not** relabel the `$3F20–$3FFF` palette mirror "NESdev-only": it is
  structurally guaranteed by `HDL/PPU/cram.v` (5-bit `CGA` into a 32-entry
  array), which is a named source. Only the `$3000–$3EFF` mirror is rank-4-only.
- The `AT_ADR[6..9]` defect is confirmed as an HDL bug, not a spec bug: the
  schematic's single bare Constant does drive those four bits. Keep §4 and the
  Programming note as written.
- Items deferred by the file (OAM decay timings TBD in `PPU/oam.md`; CHR write
  decoding per board; exact right-edge attribute fetch dots) are legitimately
  open and correctly recorded.

## Revision round 2

Revision of `Specs/04-ppu-memory-map.md`, **Status:** revised (round 2), 292
lines, md5 `d5b3c646421ff56b6081f3af6e1cb1d7`. No finding refuted; all nine
addressed in place, none left silently open.

- Blocking 1: fixed — §9 no longer conflates the pairs. The table now has
  separate `/RD` (pin 24) and `/WR` (pin 23) rows for the **VRAM controller**
  outputs plus `/RD_CPU`/`/WR_CPU` rows for the R/W decoder
  (`HDL/PPU/ppu_top.v` `n_CpuRD`/`n_CpuWR`), and the note after the table states
  that only the decoder pair is complementary/tied inactive by `/DBE`, that the
  VRAM `/RD` is `/RD = (latched H0_D & ~BLNK) OR PD_RB` (no `/DBE` term, so
  asserted on alternate dots of every rendering scanline while `/DBE = 1`), and
  that an emulator must track the two pairs separately. Both the public
  `04-ppu-overview.md` §4 table and `PPU/regs.md` §"R/W Декодер" / `RWDecoder`
  were re-read; the programming note now carries the same rule explicitly.
- Major 2: fixed — the `$3000–$3EFF` mirror is marked NESdev rank 4 everywhere:
  §1 rows (table now quotes "usually a mirror of the 2kB region from
  `$2000-2EFF`" and names the A12 decode as undocumented in rank 1–3), §3's
  closing paragraph, Observable phenomenon 8, and the programming note (now
  SHOULD, not MUST, "as NESdev describes"). The `nes.v`/`pamux.v` citation was
  dropped from the provenance row, which is split into a `Nametable mirroring,
  CIRAM A10` row (`PPU/pamux.md`, `pamux.v`) and a `$3000–$3EFF nametable mirror`
  row with `—` in both source columns (noting the empty `CartPort`/`LS139`
  bodies). Open questions now spells out the rank difference.
- Minor 3: fixed — the Open-questions attribute-fetch bullet drops the stale
  `%0010 3NN YYYY YXXX` claim; it records that `04-ppu-rendering.md` §3 now
  prints `%10NN 1111 yyy xxx` (constant field `[9:6] = %1111`) and agrees, and
  that `04-ppu-scrolling.md` §2 is the sibling still needing correction.
- Minor 4: fixed — the `$3F20–$3FFF` **rank was not changed**: §1's palette row
  now cites `HDL/PPU/cram.v` `CRAM_Block` (5-bit `CGA` into `cram[0:31]`), the
  Open-questions bullet states the mirror is rank 2 and the `$3000` one rank 4
  only.
- Minor 5: fixed — §7 now lists the 2C02E-and-earlier parts **and the 2C03 and
  2C04** (plus the 2C05 series / RC2C05-99 note), citing `04-ppu-registers.md` §9
  and NESdev `PPU_variants`.
- Minor 6: fixed — §8 keeps the doc's `1792 + 320` only as a quoted, annotated
  discrepancy: the spec's own partition is `1856 + 256 = 2112` (64 sprites × 29
  bits + 8 secondary rows × 32), which matches `PPU/oam.md` and the row grouping,
  while `1792 + 320` implies 56 primary rows and disagrees with the 29-bit
  sprite.
- Minor 7: fixed — the read-back bullet now cites NESdev `PPU_OAM` correctly
  ("always read back as 0"; "bits that have decayed also read back as 0") and
  recommends returning 0, keeping the genuine rank 1–3 gap.
- Minor 8: fixed — the backdrop-override bullet is now settled from HDL:
  `TH_MUX = BLNK & PA[13:8] = %111111` (`vram_ctrl.v` `w13`/`w12` with
  `.nPA(n_PA_out[13:8])`), independent of read vs write; §6 was re-read and
  still matches.
- Minor 9: fixed — `**Primary sources:**` now adds `PPU/dataread.md`,
  `PPU/regs.md`, `PPU/pads.md`, `HDL/PPU/scroll_regs.v`, `HDL/PPU/regs.v`,
  `Docs/NES/NES_HDL_Model/nes.v` and NESdev `PPU_registers`, `PPU_scrolling`,
  `CHR_ROM_vs._CHR_RAM`; the never-cited `Docs/PPU/USPatent/OAM_FIFO.txt` was
  dropped.
- Kept: the `HDL/PPU/tilecnt.v` `AT_adr[6..9]` tie-off stays in Open questions
  and §4 as the HDL defect (schematic ties those bits high via the bare
  Constant), per the Verifier's confirmation.
- Not touched: any other file; no `git commit`.

## Revision round 3

Revision of `Specs/04-ppu-memory-map.md`, **Status:** revised (round 3), 292
lines, md5 `0274a39e167923d6cdcfb813a0857945`. No finding refuted; all four
addressed in place.

- Major 1: fixed — §8's OAM byte `+0` row now reads "written value = top
  scanline − 1, so the sprite first appears on scanline `value + 1`". This
  matches NESdev `PPU_OAM` ("you must subtract 1 from the sprite's Y coordinate
  before writing it here") and `04-ppu-sprites.md` §"One-line offset" ("a sprite
  whose Y byte is `Y` first appears on scanline `Y+1`"), so all three agree.
- Minor 2: fixed — the Open-questions attribute-fetch bullet no longer calls
  `04-ppu-scrolling.md` §2 stale. It now records that both siblings print the
  constant field at bits `[9:6]` (`04-ppu-scrolling.md` line 70
  `AT_ADR[9:6] = %1111`, "a constant field, not a function of `TVO`/`THO`";
  `04-ppu-rendering.md` §3 `%10NN 1111 yyy xxx`) and agree with §4 here; the
  only remaining inconsistency is the `HDL/PPU/tilecnt.v` tie-off (previous
  item), so §4's "reconciled in Open questions" cross-reference still holds.
- Minor 3: fixed — `**Primary sources:**` adds `BreakingNESWiki/PPU/umc6538.md`
  (cited at line 186, provenance row "OAM row-order NTSC/PAL difference, UMC 6538
  write block") and the HDL `mux.v` (provenance row "Palette index path,
  backdrop override"), `bgcol.v` (provenance row "Attribute table layout …") and
  `fsm.v` (line 207, `DelayedH`).
- Minor 4: fixed — §7 now reads "on the 2C02E and all earlier NTSC parts and on
  the RGB 2C03, 2C04 and 2C05 series (`04-ppu-registers.md` §9,
  `PPU_variants`: RC2C05-01…04 behave like the 2C03, RC2C05-99 like the
  2C02E)", removing the self-contradiction; the programming note's revision
  list was expanded to the same set (2C02E-and-earlier + 2C03 + 2C04 + 2C05,
  with the RC2C05-99 exception), so §7 and the note no longer disagree.
- Kept: the `$3F20–$3FFF` palette mirror stays **rank 2** (`HDL/PPU/cram.v`
  5-bit `CGA` into `cram[0:31]`) and the `AT_adr[6:9]` HDL-defect note stays in
  §4 and Open questions, per the round-2 Verifier's confirmation.
- Not touched: any other file; no `git commit`.
