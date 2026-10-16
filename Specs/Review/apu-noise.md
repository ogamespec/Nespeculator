# Review: apu-noise — round 2

**Files under review:** `Specs/03-apu-noise.md` (317 lines), `Specs/03-apu-dmc.md`
(323 lines); both `Status: revised (round 2)`. This report is the only file
written; neither spec was edited and nothing was committed.

## Verdict

consensus

## Blocking

None. Round-1 Blocking 1 is resolved. `03-apu-noise.md` §6 (lines 185–190) no
longer claims a decay restart on `$400C`: it states that `$400C` "does **not**
restart the envelope: no path from `WR_Reg` reaches the envelope reload
flip-flop", and the next bullet says `$400F` "is the only noise-channel write
that does". Verified in `HDL/APU/env.v`: line 49
`rsff EnvReload (.r(WR_LC), .s(~(n_LFO1 | erld_latch_q)), .q(EnvReload_q), .nq(RELOAD));`
— only `WR_LC` (`$400F`) resets it; `$400C` (`WR_Reg`) drives only `envdis_reg`,
`lc_reg`, `vol_reg` (lines 40–42), and `RLOAD`/`ERES` derive from
`n_LFO1`/`rco_latch_q`/`eco_reload` (lines 56–61) with no `WR_Reg` path. §6 now
agrees with Observable #9 (line 246) and Programming notes lines 258–259.

## Major

None. Round-1 Major 2 is resolved. `03-apu-dmc.md` §7 (lines 192–200) now
presents the `Address_MUX` order `SPR_PPU > SPR_CPU > DMC_AB > CPU` as static and
explicitly says that order "alone would put OAM DMA first"; the arbitration
inference is reworded to `RUNDMC` forcing both sprite selects low
(`HDL/APU/dma.v:155–156`), after which "the DMC owns the address bus and a DMC
get pre-empts a pending OAM DMA". Verified against `dma.v:187–190`
(`AddrOut = SPR_PPU ? $2004 : SPR_CPU ? SPR_Addr : DMC_AB ? DMC_Addr : CPU_Addr`)
and `nor(SPR_PPU, NOSPR, RUNDMC, ~toggle)` / `nor(SPR_CPU, NOSPR, RUNDMC, toggle)`;
the end behaviour is unchanged and matches NESdev `DMA` ("OAM DMA has a lower
priority than DMC DMA").

## Minor

1. **`03-apu-dmc.md` §3, lines 103–106 — residual wording from round-1 Minor 9.**
   "the PAL load values are the NTSC values scaled by that ratio" scales the
   *periods in CPU cycles*, not the `FR` load values printed just below (`$08A` =
   138 → `$1CC` = 460; the `FR` LFSR period is not linear in the load value).
   The parenthetical then lists `380 / 1.0765 = 353.0 → 354` under "most entries
   are nearest-integer", but that is exactly the entry round 1 named as *not*
   nearest-integer (353.0 rounds to 353; 354 is only the nearest achievable
   *even* value — every PAL period is even). All numbers are correct; reword to
   "the PAL **periods** are the NTSC periods scaled by that ratio … (14 of 16
   entries round to the nearest integer; rate 1 is the 353.0 → 354 one-off, and
   rate 4 is the documented exception)". Severity minor; verdict unaffected.

Minors 3–9 of round 1 are otherwise all fixed (evidence below).

## Checked against

- **Blocking 1** — `HDL/APU/env.v:40–42,49,56–61` and `HDL/Common/rsff.v`
  (`rsff .r` resets `q`); NESdev `APU_registers` / `APU_Noise`: "`$400F` …
  Length counter load and envelope restart (write)".
- **Major 2** — `HDL/APU/dma.v:145,155–156,162,187–190`; `BreakingNESWiki/APU/waves.md`
  ("Совместная работа спрайтовой DMA и DPCM", final paragraph: part of the DPCM
  DMA cycles are reserved for OAM DMA, and after that window `RUNDMC` suppresses
  an active OAM DMA); NESdev `DMA`.
- **Minor 3 (NTSC-relative percentages)** — recomputed all 32 pitches from the §3
  periods and the CPU clocks (NTSC 1.789773 MHz, PAL 1.662607 MHz): PAL −7.10 %
  at indices 0/1, +6.17 % at index 2, +1.34 % at index 5 (spec: −7.1 %, +6.2 %,
  1.3 %), all other indices within ±0.92 %; the "relative to the NTSC value" note
  is present. The 7.65 % figure is gone.
- **Minor 4 ("one 31-state cycle")** — simulated `RIN = sout[0] XOR sout[tap]`
  over all 32767 non-zero seeds: tap 1 = one 32767-step cycle; tap 6 = 352
  cycles × 93 steps + 1 cycle × 31 states. Open questions now says "one 31-state
  cycle" and keeps 93 steps for seed 1.
- **Minor 5 (mode-bit phase)** — `03-apu-noise.md` §4 lines 154–157 now derives
  the next-`RSTEP` phase from the combinational `mux_out`
  (`noise.v:191,194`); the contradictory Open-questions "not documented" bullet
  is gone.
- **Minor 6 (length-counter label)** — §1 line 24 is now "Noise length counter";
  `HDL/APU/length.v:39–50` instantiates one `length_cnt [3:0]` array.
- **Minor 7 (PAL provenance)** — Provenance line 286 cites `APU/pal.md` (masks)
  + `APU/clkgen.md` with `noise.v` qualified "(shared counter/LFSR model)"; the
  NTSC masks are in `noise.v`, the PAL masks only in `pal.md` "Noise Channel
  Decoder".
- **Minor 8 (LFSR width)** — §2 line 59 now "1 = 15-bit LFSR with tap bit 6";
  `noise.v:191` shifts `{RIN,sout[14:1]}` through all 15 stages in both modes.
- **Minor 9** — "not because of the clock ratio" is gone from both files and
  both now say the PLA must compensate for the clock ratio; "quantised to the
  next counter reload" is gone. Residual wording is Minor 1 above.
- **Derived tables unchanged and correct (independent re-derivation).** Parsed
  the NOR-plane masks directly from `HDL/APU/noise.v` (`NOISE_Decoder2`) and
  `HDL/APU/dpcm.v` (`DPCM_Decoder2`) and decoded the `pal.md` "Noise Channel
  Decoder"/"DPCM Decoder" masks with the same convention round 1 calibrated
  (printed in reverse index order; mask '1' = transistor → output bit 0).
  Simulating the exact `NOISE_FreqLFSR` / `DPCM_FreqLFSR` feedback reproduces all
  32 entries of each table — NTSC noise 4…4068, PAL noise 4…3778, NTSC DMC
  428…54, PAL DMC 398…50 — identical to the spec tables and to the cached NESdev
  `APU_Noise` / `APU_DMC` tables. All 32 `NNF` (`$002`…`$140` / `$002`…`$24E`)
  and 32 `FR` (`$173`…`$156` / `$1D7`…`$1D5`) values match the derivation and
  the values recorded in the round-1 report (which were written before the
  revision), so no table entry changed in round 2.
- **`APU_Mixer` (singular, adjudication A1)** — kept in both correspondence
  tables (`03-apu-noise.md:275`, `03-apu-dmc.md:273`). Cache check:
  `APU_Mixer.html` = 22 681 bytes, `wgPageName = APU_Mixer`, title "APU Mixer";
  `APU_Mixers.html` = 4 609 bytes with no article body (soft-404). No plural
  remains in either file.
- **Template compliance** — both Status lines are `revised (round 2)`; the five
  fixed headings are spelled exactly, without a number prefix and in order; the
  files are 317 / 323 lines (inside 120–400).
- **No new errors introduced** in the revised passages: DMC §7's
  register-index-vs-range claim re-checked against `HDL/APU/regs.v:58–77` (PLA
  index from `Addr_frommux[4:0]`, range check on `CpuA = Addr_fromcore`); every
  rate/pitch column in both §3 tables recomputed from the clocks (all rounded
  values correct); §5 gates (`noise.v:196`, `noise.v:42`) and the `AUX_B` bit
  assignments (`RND[3:0]` at `AUX_B[7:4]`, `DMC[6:0]` at `AUX_B[14:8]`)
  re-checked against `dac.v`.

## Notes for the next round

1. No blocking or major findings; the round-1 blocking and major are resolved
   and all four derived tables are verified. Consensus.
2. Optional wording tidy for a future revision (Minor 1): in `03-apu-dmc.md` §3
   say "periods" rather than "load values", and do not present `353.0 → 354` as
   a nearest-integer example.
3. Remaining unverified areas are only the honest ones already in
   `## Open questions`: the letterless-RP2A03 period table, the power-on LFSR
   seed, the `SOUT` latch phase, and PAL mode 1.

---

# Review: apu-noise — round 1

**Files under review:** `Specs/03-apu-noise.md`, `Specs/03-apu-dmc.md`
(Status: adapted (round 1)). This report is the only file written; the specs were
not edited and nothing was committed.

## Verdict

revise

## Blocking

1. **`03-apu-noise.md` §6, lines 181–183 — `$400C` does not restart the envelope.**
   The bullet claims that writing `$400C` "restarts the decay sequence
   (`EnvReload` flip-flop reset)". The start flag is reset only by `WR_LC`
   (`$400F`): `HDL/APU/env.v:49` `rsff EnvReload (.r(WR_LC), .s(~(n_LFO1 |
   erld_latch_q)), ...)`; `$400C` (`WR_Reg`) only loads `envdis_reg`, `lc_reg`
   and `vol_reg` (`env.v:40–42`), and no path from `WR_Reg` reaches `RLOAD` or
   `ERES`. NESdev `APU_registers` marks `$400F` "(also starts envelope)" /
   `$400C` without that side effect, and `APU_Envelope` lists "Sets start flag"
   only for `$4003/$4007/$400F`. The file contradicts itself: Programming notes
   line 253 ("if shared, `$400F` MUST restart it") and Observable phenomenon 9
   (line 240, "`$400F` write restarts the envelope decay") are correct, and the
   sibling `03-apu-pulse.md:275` states the envelope "MUST NOT restart on
   `$4000`". An implementer following §6 adds a spurious retrigger on every
   `$400C` write.

## Major

2. **`03-apu-dmc.md` §7, lines 191–193 — the quoted mux order does not yield the
   stated conclusion.**
   The order `SPR_PPU > SPR_CPU > DMC_AB > CPU` is correct
   (`HDL/APU/dma.v:187–190`), but in that mux `SPR_CPU` (OAM read) has priority
   over `DMC_AB`, so the list by itself gives *OAM* DMA priority. The DMC wins
   only because `RUNDMC` forces the sprite selects low: `dma.v:155–156`
   `nor(SPR_PPU, NOSPR, RUNDMC, ~toggle)` and `nor(SPR_CPU, NOSPR, RUNDMC,
   toggle)`. The following sentence (and Observable #10, and the normative note
   at line 256) describe the correct mechanism, but the sentence "so DMC DMA has
   priority over OAM DMA" must be rewritten as "RUNDMC gates the sprite selects,
   so once the DMC is running it owns the address bus". The end behaviour (a DMC
   get pre-empts a pending OAM DMA) is right and matches NESdev `DMA`: "OAM DMA
   has a lower priority than DMC DMA".

## Minor

3. **`03-apu-noise.md` §3, lines 124–128 — the percentages are measured against
   the PAL value, not NTSC.** Index 0/1: (NTSC−PAL)/PAL = 7.65 %, but the PAL
   pitch is 7.1 % lower relative to NTSC; index 2: (PAL−NTSC)/PAL = 5.8 %, but
   6.2 % higher relative to NTSC. The 7.65 % figure is numerically identical to
   the ACLK ratio − 1 (1.0765 − 1), so it can be mistaken for the inverted
   clock-ratio error this round fixed. The directions (PAL lower at 0/1, higher
   at 2, within ~1 % elsewhere) are correct; index 5 is 1.34 %, slightly more
   than "~1 %". State the reference or use −7.1 % / +6.2 %.
4. **`03-apu-noise.md` Open questions, lines 307–308 — "one single state" is
   wrong.** Cycle decomposition of the mode-1 map (`RIN = sout[0] XOR sout[6]`)
   gives one 31-*state* cycle plus 352 cycles of 93 states (32736 states). It is
   a 31-step cycle, not one state. Seed 1 → 93 steps and mode 0 → a single
   32767-step cycle were both confirmed.
5. **`03-apu-noise.md` §4, lines 152–153 vs Open questions 309–311.** §4 states
   the mode-bit switch "takes effect at the next `RSTEP`"; Open questions says
   the phase is "not documented". In the HDL `mux_out` is combinational off
   `rmod_q` (`noise.v:194`) and is sampled by `NOISE_RandomLFSRBit` on `RSTEP`
   (`noise.v:191`), so next-`RSTEP` is well defined by the schematic. Keep one
   statement.
6. **`03-apu-noise.md` §1, line 24 — `LengthCounter` #3.** The source instantiates
   four identical counters in one array `length_cnt [3:0]` (`length.v:39–50`);
   "#3" is an array index, not a module name. Prefer "the noise length counter".
7. **`03-apu-noise.md` Provenance, line 280 — PAL row cites the wrong HDL.** The
   PAL decoder masks are only in `APU/pal.md` ("Noise Channel Decoder");
   `HDL/APU/noise.v` contains the NTSC decoder. Cite `pal.md` (masks) +
   `noise.v` (shared counter/LFSR model).
8. **`03-apu-noise.md` §2, line 59 — "1 = 6-bit LFSR (tonal noise)".** The shift
   register is 15 stages in both modes (`noise.v:191`; NESdev `APU_Noise`).
   Mode 1 changes only the tap (bit 6 instead of bit 1), which shortens the
   effective sequence. §4 corrects this, but §2 read alone is misleading.
9. **`03-apu-dmc.md` §3, lines 101–104 — self-contradictory ratio wording.**
   "not because of the clock ratio: the PAL periods are the NTSC periods divided
   by the APU clock ratio" — the PLA differs *because* it must compensate for
   the clock ratio. The same wording appears in `03-apu-noise.md` §3, line 109.
   Also "quantised to the next counter reload" is imprecise: PAL entry 1 is 354
   where 380 / 1.0765 = 353.0, while most entries are nearest-integer
   (397.6→398, 176.5→176, 50.2→50). "Rounded to the nearest achievable LFSR
   count" is accurate.

## Checked against

- **Independent re-derivation of all region tables (all values match the
  specs).** Parsed the topological PLA masks from `BreakingNESWiki/APU/noise.md`,
  `pal.md` and `dpcm.md`, calibrated the mask convention against the explicit
  NTSC "Decoder In/Out" tables (rows/columns are printed in reverse index order;
  '1' = connected), and simulated the exact `NOISE_FreqLFSR` / `DPCM_FreqLFSR`
  feedback in Python. Result: all 16 NTSC noise periods (4…4068), all 16 PAL
  noise periods (4…3778), all 16 NTSC DMC periods (428…54) and all 16 PAL DMC
  periods (398…50) reproduce the spec tables and the NESdev tables exactly; the
  32 noise `NNF` values and 32 DMC `FR` values in the specs match the derived
  masks ($002/$00A/$0AA/$2BB/$139/$173/$063/$067/$1A9/$106/$227/$062/$642/$20F/$300/$140
  and PAL $002/$00A/$055/$55D/$64E/$797/$6C3/$7C1/$433/$51C/$3AB/$648/$3F9/$5A4/$4F3/$24E;
  FR NTSC $173/$08A/…/$156, PAL $1D7/$1CC/…/$1D5). The PAL/NTSC relationship in
  §3 is correct in direction (see Minor 3 for the percentage base).
- **`HDL/APU/noise.v`** — taps (`mux_out = rmod_q ? sout[6] : sout[1]`, line 194),
  RIN XOR + all-zero escape (195), 15-stage shift direction (191), output gate
  `RNDOUT = (sout[0] | NORND) & ~LOCK` (196) and `RND_out = Vol & ~RNDOUT` (42),
  `RSTEP = NFLOAD` on state `%000_0000_0001` (144–150), 11-bit decoder/`NNF`
  (101–121), `$400E` reg bits (54). Mode-1 cycle structure (1×31, 352×93) and
  power-on seed 1 → 93 steps simulated.
- **`HDL/APU/dpcm.v`** — `$C000 + $40*A` load (`{1'b1,DPA[7:2]}` / `{DPA[1:0],6'b0}`,
  551–554), 15-bit counter + `DMC_A15`=Vdd and `$FFFF→$8000` wrap (554),
  `{DSC,4'b0000}` sample load (466), 9-bit `FR` decoder (377–395), output
  6-bit reversible counter + `$4011` LSB (573–577), ±2 / LSB fixed (502, 573),
  saturation via `DOUT=cout[5]` inhibiting `DSTEP` (328, 577), LSB-first shift
  (500) and `PCM = ~PHI1 & DMC_AB` (326), `start_set` read-cycle gate (250),
  `ED1`/IRQ set on sample-counter empty (223), `$4015` clear (198), loop
  suppression (223), `RUNDMC`/`n_DMCAB`/`DMCRDY` (248–253).
- **`HDL/APU/dma.v`** — `RDY = sprdma_rdy & DMCReady` (162), `Address_MUX`
  ordering (187–190) and the `RUNDMC` gating of the sprite selects (155–156) —
  basis of Major 2.
- **`HDL/APU/regs.v`** — writes disabled in PHI1 (`nor(W400x, PHI1, ~pla[n])`,
  85–113), register index from `Addr_frommux[4:0]` vs range check on
  `Addr_fromcore` (58–77) — DMC §7 confirms.
- **`HDL/APU/env.v`** — `LC = ~$400C[5]` (41), `V = ENVDIS ? VOL : ENV` (63),
  `EnvReload` reset by `WR_LC` only (49) — basis of Blocking 1.
- **`HDL/APU/length.v`** / **`length.md`** — 32 load values equal to the wiki
  decoder table; audible = load + 1 matches NESdev.
- **`HDL/APU/test.v`**, **`dac.v`** — LOCK from `$401A` bit 7, noise `RND[3:0]` on
  `$4019[7:4]`, `AUX_B = {DMC[6:0], RND[3:0], TRI[3:0]}`.
- **`HDL/APU/common.v`**, **`HDL/Common/rsff.v`** — `RevCounterBit` load-over-step
  priority (`latch_in`, 138) and `rsff .r` resets q.
- **`BreakingNESWiki/APU/clkgen.md`** — 2A03/2A07 `/CLK`/PHI/ACLK periods,
  `PHI ÷ 2` ACLK, LFO1/LFO2 240/192 and 120/96 Hz; **`dma.md`**/**`waves.md`** —
  sprite DMA subordinate to `RUNDMC`, the DPCM/OAM overlap window; **`Dendy.md`**
  — UMC 6527 NTSC-rate CPU with PAL video.
- **`Docs/NES_Famicom_Board_Revisions.md`** — letterless RP2A03 (no looped noise,
  2046 vs 4068, frame counter not reset on reset), Vs. System, RP2A03E/G, 2A07
  ÷16 divider and different tables, 2A07 DPCM RDY glitches and the late-RP2A03G
  DMC DMA bug — all quoted correctly in §7 / Open questions.
- **NESdev cache (`/tmp/nesdev-cache/`)** — `APU_Noise` (period tables, taps,
  32767 / "93 or 31", power-up = 1), `APU_DMC` (rate tables, `$C000 + A*64`,
  `$10*L + 1`, power-up level 0, `$4011` race, "IRQ when the last byte is read",
  PAL `$4`/`$C` off A-440), `APU_registers` (`$400F` starts envelope; `$4015`
  `IF-D NT21`/`---D NT21`), `APU_Envelope`, `DMA` (DMC > OAM), `APU_Mixer`
  (title "APU Mixer", 3934 chars) vs `APU_Mixers` (Wayback soft-404, 647 chars),
  `CPU_variants`.
- **Template compliance** — both files have the Status line `adapted (round 1)`,
  the five fixed headings spelled exactly with no number prefix and in order,
  and are 314 / 319 lines (inside 120–400). The `APU_Mixers` → `APU_Mixer`
  correction has landed in both correspondence tables (only `03-apu-overview.md`
  and the review notes still mention the plural). All four NESdev-vs-schematic
  differences requested (power-on LFSR, mode-1 93/31, `$4011` race, DMC power-on
  level) are present in Open questions, with the schematic behaviour kept in the
  body (§4/§5) and the community alternative named there.

## Notes for the next round

1. Fix Blocking 1: delete the "restarts the decay sequence (`EnvReload`
   flip-flop reset)" clause from the `$400C` bullet in §6 and say explicitly
   that `$400C` does **not** restart the envelope; only `$400F` (`WR_LC`) does.
2. Fix Major 2: separate the static `Address_MUX` ordering from the arbitration
   outcome (`RUNDMC` gates `SPR_CPU`/`SPR_PPU`); keep the NESdev-aligned end
   result that a DMC get pre-empts OAM DMA.
3. Apply the Minor 3–9 wording corrections. Minor 3 must keep the corrected
   direction; only the reference base/percentages change.
4. Do not change the derived tables, the register bit fields, the address/length
   formulas, the ±2/LSB/saturation rules or the open questions except Minor 4;
   they were verified against the PLA masks, the HDL models and the NESdev
   tables.
5. After the edit, re-check that §6, Programming notes line 253 and Observable
   phenomenon 9 all say `$400F` for the envelope restart, and re-run a grep for
   `APU_Mixers` in these two files.

## Revision round 2

- Blocking 1: fixed — `03-apu-noise.md` §6: the `$400C` bullet no longer claims a
  decay restart and now states explicitly that `$400C` does **not** restart the
  envelope (no path from `WR_Reg` to the reload flip-flop); the following bullet
  says `$400F` (`WR_LC`) is the only noise write that restarts it. §6 now agrees
  with Programming notes, Observable #9 and `03-apu-pulse.md` ("MUST NOT restart
  on `$4000`").
- Major 2: fixed — `03-apu-dmc.md` §7: the static `Address_MUX` order is now
  presented as such and explicitly noted not to decide the outcome; the
  inference is reworded to `RUNDMC` forcing `SPR_PPU`/`SPR_CPU` low (`dma.v`),
  after which the DMC owns the address bus. End behaviour unchanged (DMC get
  pre-empts pending OAM DMA).
- Minor 3: fixed — `03-apu-noise.md` §3 percentages are now relative to NTSC
  (−7.1 % at indices 0/1, +6.2 % at index 2) and index 5 is stated as 1.3 %.
- Minor 4: fixed — `03-apu-noise.md` Open questions: "one single state" →
  "one 31-state cycle".
- Minor 5: fixed — `03-apu-noise.md` §4 now gives the next-`RSTEP` phase as
  defined by the HDL (`mux_out` combinational, sampled on `RSTEP`); the
  contradictory Open-questions bullet was deleted.
- Minor 6: fixed — `03-apu-noise.md` §1: "`LengthCounter` #3" → "Noise length
  counter".
- Minor 7: fixed — `03-apu-noise.md` Provenance: PAL row cites `APU/pal.md`
  (masks) + `clkgen.md`, with `noise.v` qualified as the shared counter/LFSR
  model.
- Minor 8: fixed — `03-apu-noise.md` §2 bit-7 meaning: "6-bit LFSR" →
  "15-bit LFSR with tap bit 6".
- Minor 9: fixed — `03-apu-dmc.md` §3 and `03-apu-noise.md` §3: the ratio
  wording now says the PLA differs *because* it must compensate for the clock
  ratio, and "quantised to the next counter reload" → "rounded to the nearest
  achievable counter value" with the nearest-integer examples.
- `APU_Mixer` (singular) kept in both correspondence tables — confirmed the
  Wayback page name is `APU_Mixer` (`wgPageName = APU_Mixer`, 22 673 bytes) while
  `APU_Mixers` is a soft-404 (4 609 bytes with no article body). No change made.
- No derived table, register bit field, address/length formula or ±2/LSB rule
  was touched. Files edited: `Specs/03-apu-noise.md` (Status → revised round 2),
  `Specs/03-apu-dmc.md` (Status → revised round 2), and this report (appended).
  Nothing committed.
