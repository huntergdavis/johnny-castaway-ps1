# Transition-Zero Plan (post red-team v1)

## W4 landed (2026-06-09 late) — island constants survive custom backdrops

The worst staged hit (walkstuf2 after suzy1: 171 setup_vb, 361 KB of
CD re-reads) was the island constants being dropped on every
custom-backdrop teardown: the SCR cache slot admitted (and a per-scene
toggle freed) suzy1's one-shot screen, and fgBackdropRelease(0)
dropped the BACKGRND sprite sheet. The cache now admits only
OCEAN/NIGHT and lives for the proof's lifetime; slot 0 stays loaded
through custom scenes (waves still stop). walkstuf2: 171 -> 51
setup_vb; every staged hit passes the 60 vb ratchet gate. Also landed
earlier: oversize stage windows publish in place (johnny3 42 -> 26).

Residual cold costs, documented and accepted for now: suzy1-class
scenes pay their own custom SCR load (~97 vb screen — per-scene
content, would need a second staged file class); the
undrained-predecessor no-walk miss (building6 49 vb, suzy1's
pack_start 74 vb); boot (200 vb, once per session). W3's parse-work
items are absorbed: full hits already run pack_start 1-5 vb.

## MILESTONE 1 — 2026-06-09 (branch ps1-transition-zero-20260609, 22 commits)

W0+W2+W1+W5 landed, W4 first win landed. Staged transitions: 13-26
setup_vb (0.22-0.43 s, best with ZERO setup CD reads) vs ~250 vb
(~4.2 s) at session start; boundaries fully animated (visible walk +
moving ocean) vs 5-8 s frozen screen; 20/20-scene soaks deterministic
and BSOD-free with the CACHE relief valve recovering all pressure
events; staged-hit rate 85-89%. Normal-mode canary green throughout.

Next session entry points: setup-prime SEGMENT staging
(walkstuf2-class, ~360 KB bypasses the stream window — segment offsets
are already in the staged metadata), suzy1 position-reroll rebuild
profile, waves during the setup window, >=2 h soak, real-hardware
burn-in for the CD spin removals.

Goal: drive scene transition latency from ~1.5s (common path, `setup_vb=86-87`)
toward perceptual zero, and pull the cold path (`setup_vb=187-283`) down with it.
Drafted 2026-06-09, red-teamed against code at 358ccda5bd (v0.9.3-ps1).

## Ground truth (verified)

- The 86-87vb "common path" is gated behind the `loading-waves` boot token
  (`gFgLoadingWaveProofEnabled` defaults 0, `foreground_pilot.c:398`,
  `jc_reborn.c:1325-1328`) and is explicitly **not release-ready**: it retains
  300-345KB CACHE across boundaries and skips `memCacheRewindIfEmpty`
  (`async-wave-loading-branch-plan.md:131-145`). The shipping default path is
  the ~4s cold path.
- The perf matrix (126 explicit-scene `noloop` boots) structurally cannot
  measure staged transitions: `fgLoopPrimeStageLookahead()` bails on
  `explicitScene` (`jc_reborn.c:487-490`). Every matrix `setup_vb` is a
  guaranteed staged miss. `setup_vb` is logged (`ps1_perf.c:1200`, parsed at
  `ps1-perf-iterate.sh:708,1367`) but never gated.
- Best case ever observed with reads nearly eliminated: `setup_vb=43`
  (`pack_start_vb=34`). **CPU-side setup work (~30-50vb of metadata parse,
  entry-table build, TRANSIENT reset, first-frame prime) is the real floor**,
  not CD bytes.
- Five unconditional seek-settle busy-waits: `cdrom_ps1.c:787` (1M iters),
  `:845` (500K), `:944` (200K), `:1289` (100K per chunk), `:2061` (100K).
  ~33.8MHz CPU → tens of ms each; `:1289` runs per stream chunk. The async
  path (`cdrom_ps1.c:128-136`) does Setloc→Read with no spin and soaks clean,
  so they are probably cargo cult — but only DuckStation has ever gated them.
- SPU budget at the adoption instant: audio ~227KB; WALK_CLEAN (144KB) and
  WALK_PSB (48KB) are **live** during the inter-scene walk
  (`foreground_pilot.c:1545-1552`, `walk_pilot.c:157-198`); MRAFT 12KB.
  Actually reclaimable for staging: FG_METADATA+FG_PAYLOAD = 72KB + ~9KB slack.
  Plans assuming 276KB of SPU staging space double-count.
- Structural staged-miss population: tide/raft/position reroll every 8-12
  scenes (`jc_reborn.c:843-866`), pause-menu/freeplay/scene-set changes clear
  lookahead (`fgLoopClearStageLookahead` callsites), boot is always cold, and
  staging can't start until current-scene payload reads drain
  (`stream_runtime.c.inc:542-546`). Worst-case-zero is unachievable;
  gate median (staged) and cold paths as separate metrics.
- Failed-experiments constraints that bind any design here:
  - No extra CACHE allocation may live across the boundary above the rewind
    floor (side-buffer BSOD at 522s; R33 BSOD at 790s). Safe shapes: SPU,
    VRAM, or a staged sub-region at the *bottom* of CACHE with a proof that
    `memCacheRewindIfEmpty` still fires (soak grep: `CACHE-rewind-skip` = 0).
  - Overlap must *remove* setup from the transition, not animate over it.
  - One memory-shape change per branch, each behind its own boot token, each
    with its own ≥2h soak before the next stacks on it.

## W5 landed (2026-06-09 night) — boundaries are content now

Frame-dump archaeology found the real perceptual story: setup_vb was
the tip of the iceberg. The full boundary (scene_end -> next
scene_start) froze the screen for 5.2 s on the proof path and 8.3 s on
defaults — every transition, every mode — because the scene-end
fgRuntimeReset wiped TRANSIENT (where R33 put the bg tiles) BEFORE the
walk ran. Johnny has been walking invisibly since R33. Fixes:
1. Deferred TRANSIENT wipe (fgRuntimeResetCore(keepBackdropTiles)):
   scene-end keeps tiles; the next setup-top reset wipes as before.
   Walks paint again.
2. VBlank-based walk pacing: visible walks cost 2+ VBlanks/frame and
   iteration-counted pose timing doubled every pose (9-17 s walks).
   Poses and the runaway cap now charge elapsed VBlanks.
3. Wave thread survives keepBackgrnd boundaries: islandAnimate uses
   only slot 0 + grBackgroundSfc, both boundary-safe now. Ocean keeps
   moving through the walk and Johnny's arrival pause.
4. gap_vb on the setup line + harness: the full-boundary metric that
   setup_vb structurally missed; this is the regression tripwire.
Result: transitions are scene -> ~1 s final pose (authentic content)
-> animated walk with moving ocean -> 0.25-0.8 s setup -> scene. The
only remaining frozen-wave window is the setup phase itself.

## W1 soak confirmation (2026-06-09, final stack)

Second independent 20-scene soak on the committed stack: 20/20 scenes,
zero BSODs, 89% staged hits, per-scene results byte-identical to the
first soak (deterministic under seed 1). First ratchet gate active:
`--gate-setup-hit 60` passes every transition except the documented
custom-backdrop class (walkstuf2 171 vb), which is W4's work item.
Normal-mode canary (fishing1-low explicit) green throughout.

## W1 landed (2026-06-09 evening) — staged-path results

20-scene soak, `fgpilot perf-log spu-stage loading-waves seed 1`:
20/20 scenes, zero BSODs, 89% staged hits, CACHE relief valve fired 3x
and recovered each time. Transition distribution: best 13-15 setup_vb
(~0.22 s, ZERO setup CD reads), typical 14-47, known heavy classes:
custom-backdrop scenes (suzy1 cold 177, walkstuf2 hit 171 — backdrop
reload class, W4 territory) and the no-walk-after-undrained-scene miss
(johnny5->building6 cold 49). Mechanisms, in landing order:
1. stage_adopt classification + --transitions harness (W0).
2. Seek-spin removal (W2): ~8 vb per cold transition.
3. Metadata-only stage publish + hard-release survival: adoption works.
4. Slab retention (clean rects, stream window, walk PSB caller-owned
   slab): fragmentation BSODs eliminated.
5. Walk-window payload staging + walk-start: adopt=7 full hits common.
6. memSetCacheReliefHook pressure valve: retention is droppable, BSOD
   becomes one transition of churn.
Remaining before release: >=2h soak, real-hardware burn-in (CD spins +
async), pause-menu/freeplay-entry paths still clear lookahead (cold by
design), boot scene is inherently cold (200 vb).

## Measured baselines (2026-06-09, W0 harness, seed 1, verbose schema)

- Shipping default (`fgpilot perf-log seed 1`, 4 scenes): boot `setup_vb=205`;
  cold transitions `214 / 245 / 215` (~3.6-4.1 s). Every one pays
  `screen_vb=84` + `backdrop_vb=64-67` — the scene-independent reloads are
  ~60% of the cold cost (W4's target).
- Proof path (`spu-stage loading-waves`): first transition `setup_vb=23`
  (~0.38 s, the loading-waves reuse win) but `stage_adopt=0` — staged data
  was NOT adopted even with tide unchanged (silent precondition failure,
  see W1). BSOD on the 3rd transition: `CACHE exhausted req=98276`,
  `cacheUsed=521208`. The W0 harness reproduces the branch-plan release
  blocker in under 2 minutes of emulated time.
- The 126-row matrix path is unaffected: explicit-scene noloop boots remain
  the cold-path reference.

## Workstreams (priority order)

### W0. Measurement: scene-pair harness + transition gates  (prereq, ~days)
- New harness mode: seeded loop boot that plays scene pairs with staging
  enabled and parses the *second* scene's `JCPERF2 setup` line.
- Add `stage_adopt=hit|miss` (new field) to the setup line; today nothing
  distinguishes adoption from cold setup.
- Gate `setup_vb` two ways: staged-hit budget (tight, e.g. start at 90 and
  ratchet down) and cold budget (looser). Add columns to the scene matrix for
  the cold numbers it already produces.
- Blank/non-animating-frame detector in regtest frame dumps (needed before any
  perceptual-zero claim). frame_mismatch is intra-scene only today.
- Without W0, every later phase is gated against the wrong path.

### W1. Land the loading-waves path properly  (the unfinished prerequisite)
- ROOT CAUSE of silent adoption misses (FG_STAGE_DIAG_LOGS run, 2026-06-09):
  staging for N+1 only starts once scene N's payload reads fully drain
  (`stream_runtime.c.inc:542-546`), which for streaming scenes is the last
  few held frames — `JCSTAGE S 65536` fires ~0.5 s before adoption, the read
  is still in flight, `AdoptWindow` invalidates, `stage_adopt=0`. Fix
  directions: start staging once the stream window covers the remaining
  frames; make the bounded tail wait cover an in-flight stage at adoption;
  or shrink the staged unit so it can complete in the available slack.
- The 4s→1.5s win is currently a proof flag. Release blocker per branch plan:
  restore scene-boundary CACHE rewind or prove bounded single-owner lifetime
  for every retained buffer. This is a mem-region design round (red-team it),
  not a flag flip.
- Until W1 lands, default users still see ~4s.

### W2. Dead-wait removal  (low risk, cold-path medicine, ~days)
- Remove the five seek-settle spins **one per commit**, soak each; real
  hardware (or low-level-accurate emulator) burn-in before release — the
  `cd_fail` gate is DuckStation-only and the CD layer has one proven ordering
  hazard (pause-IRQ FIFO race, `cdrom_ps1.c:756-765`).
- `:1289` first (per-chunk, multiplies), then `:845`, `:944`, `:787`, `:2061`.
- Skip: idle hooks for sound loads (all audio loads are boot-time only);
  bounce-memcpy elimination (microseconds, and naive direct-read overruns
  exact-size buffers on the final partial sector); async SPU writes for
  staging (they hide inside held slack already — only the CD-read→SPU-write
  pipelining is worth anything, and only if W4 needs it).

### W3. Parse-during-slack: adoption becomes a pointer swap  (new, highest leverage on the floor)
- Profile what `pack_start_vb=34` actually is (extend ps1_perf sub-marks).
- Staged metadata sits idle in SPU/RAM for the whole of scene N while the CPU
  idles in `fgRuntimeWaitHeldVBlank`. Parse it then: build the entry table,
  sound-event table, draw bounds, and clean-rect estimate during scene-N held
  slack; park the built structures; adoption = validate + pointer swap.
- Target: staged-hit `setup_vb` from ~43-86 to <20. This attacks the CPU
  floor that no amount of CD work touches.

### W4. Stage the cold-path constants speculatively  (covers misses too)
- BACKGRND.PSB (93KB) + OCEAN/NIGHT SCR clean state dominate the cold
  breakdown (`backdrop_vb=83-100`, `screen_vb=84`) and are scene-independent
  modulo night/tide (session/sequence-pinned). Prefetch them regardless of
  which scene comes next — benefits staged hits AND structural misses.
- Staging surfaces, in order of preference:
  1. **VRAM**: audit spare framebuffer space; `grCaptureBgRectToSpu` proves
     rect capture works — a VRAM-parked clean backdrop restores with a GPU
     blit, costs zero CACHE/CD, can't block the rewind. Do the VRAM budget
     audit first; nobody has.
  2. **Inter-scene walk window**: Johnny walks for seconds between scenes
     with the CD idle (walk assets are SPU-resident in v0.9.3). Hide the
     backdrop/SCR reads for N+1 entirely inside the walk.
  3. SPU 72-80KB reclaimable slots for the metadata+first-window stage
     (existing machinery, modestly raised cap).
- Explicitly NOT: a new cross-boundary CACHE side region (re-runs the 522s
  BSOD shape).

### W5. Perceptual zero presentation  (after W3/W4 make setup actually short)
- Hold scene N's last displayed frame through adoption; first frame of N+1
  presents from a prebuilt buffer. Verify the displayed buffer isn't mutated
  mid-setup (cf. walk-residue fix `grForceFullRedrawNextFrame`,
  `foreground_pilot.c:1555-1562`).
- The scene-specific VSync holds (`foreground_pilot.c:1687-1759`) are
  phase-alignment pads load-bearing for `frame_mismatch=0`; touching any
  constant = matrix re-validation for that scene. Absorb, don't delete.
- Gate (from W0 detector): 0 blank frames, ≤2 VBlanks non-animating.

### Deprioritized / rejected
- Disc layout reordering: Original mode picks via rand() with filters
  (`scene_picker.c:403-444`) — only pool-level clustering possible; seeks are
  second-order vs read+parse; invalidates all 126 `pack_lba` baselines.
  Do last or never.
- Two-scene lookahead: single staging struct borrows the single stream
  window; short scenes (the motivation) have the least staging room. No-op
  as proposed.
- Sector-aligning intra-pack entries: headers already aligned; bloats packs,
  shifts LBAs, saves microseconds.

## Sequencing rule
Each workstream that touches boundary memory shape (W1, W3, W4) ships behind
its own boot token with its own ≥2h soak (and `CACHE-rewind-skip` count = 0)
before the next stacks on it. That methodology is why v0.9.3 landed where
three prior attempts burned.

## Success metrics
- Staged-hit transition: `setup_vb` ≤ 20 (~0.33s), stretch ≤ 6 with W5
  presentation (perceptual zero).
- Cold transition: `setup_vb` ≤ 90 (~1.5s) — i.e., today's *best* path
  becomes the *worst* path.
- Staged-hit rate ≥ 80% over a ≥1000-pick Original-mode soak.
- All existing correctness gates green; ≥2h soak per memory-shape change;
  one real-hardware burn-in before any release that removes CD spins.
