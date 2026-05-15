# Memory region allocator — implementation plan

**Status:** v3 — incorporates the "deterministic platform, no failure paths"
directive. See companion documents at
[memory-region-allocator-red-team.md](./memory-region-allocator-red-team.md)
(v1 critique) and
[memory-region-allocator-red-team-v2.md](./memory-region-allocator-red-team-v2.md)
(principal-engineer multi-reviewer critique of this v3).

## What changed in v3

v2 still treated `JCSKIP` and "graceful fallback" paths as legitimate. They
are not. The PSX is a deterministic platform: identical inputs produce
identical state, including memory state. If an allocation fails, that's a
specific bug to fix (budget too small, or call site in the wrong region), not
a runtime condition to paper over with a scene skip. v3 makes "allocations
cannot fail" the load-bearing invariant. Every site that currently silently
skips, returns NULL, drops a buffer to make room, or falls back to a degraded
path is enumerated in the removal manifest below and deleted in Phase 2.

The allocator's job changes accordingly: not "satisfy most requests and let
callers handle failures," but "satisfy every request that the game data
implies, provably at compile/boot time."

## Core invariant

> Every `memAlloc(...)` call in jc_reborn returns a valid pointer. The
> allocator never returns NULL. If a region cannot satisfy a request, the
> program calls `fatalError` immediately — that is the *only* failure path.
>
> `fatalError` triggering means we have a real bug: a misclassified call
> site, an under-sized region budget, or game data the allocator was not
> verified against. The plan eliminates all three at boot time.

This invariant is what makes "delete all the skip code" possible. Every
defensive NULL check and graceful-fallback path in the codebase exists
because `malloc` could return NULL. With deterministic regions sized to
proven-worst-case fit at boot, those paths become unreachable, and
unreachable paths get deleted.

## Context

The PS1 build runs in 2 MB of main RAM and currently uses libc `malloc`/`free`
through a single wrapper, `safe_malloc` (`src/utils.c:123`). Across 39 call
sites the heap takes per-scene churn: per-scene buffers
(`gFgSetupSegmentBuffer`, clean-rect snapshots, entry tables, sound-event
arrays) are allocated and freed on every scene transition, while a 600 KB
LRU resource cache (`src/resource.c`) holds variable-size BMP/TTM/SCR/ADS
bytes that come and go as scenes touch them.

That churn fragments the heap. The v0.8.10-ps1 release shipped a "heap
fragmentation experiment" (256 KB CD sector pool + boot-time stream buffers)
that immediately broke large clean-rect scenes — `JCSKIP clean-rect-alloc-failed`
fired because the pinned blocks left no contiguous space for the snapshot.
The v0.8.11-ps1 release rolled it back. Current bandaids — "grow-only" frame
buffers (`src/foreground_pilot.c:1267-1287`), a resident 149 KB walk buffer
(`src/walk_pilot.c:50-67`), pressure-drop helpers that free buffers mid-play,
NULL-return paths in resource lookup, silent scene skips in ads.c, and a
surface-pool fall-back to non-pooled allocation — partly hide fragmentation
but never close the hole. Multiple `JCSKIP` paths in `foreground_pilot.c`
silently abandon scenes when allocation fails.

This plan replaces the heap with a deterministic three-region allocator
backed by one static buffer. PERM grows at boot and never shrinks. RES holds
the existing LRU cache inside a bounded sub-allocator. SCENE is a bump
allocator wiped wholesale on every scene transition. Region sizes are proven
sufficient at boot via a pack-header scan against
`docs/ps1/research/generated/scene_analysis_output_2026-03-21.json`. With
allocations guaranteed to succeed, every skip/fallback/NULL-return site
becomes dead code and is removed in Phase 2.

## Goals / non-goals

**Goals.**

- Allocations cannot fail. Eliminate the entire class of "scene N+1 can't
  allocate" bugs along with every code path that handles such failures.
- Keep the existing LRU hit-rate so common resources stay resident across
  scene transitions.
- Add zero measurable per-alloc overhead on the hot path.
- Reduce code size by deleting all bandaid + skip code (~23 call sites,
  several hundred lines of fallback logic).
- Make region budgets compile-time-verifiable against actual scene data.

**Non-goals.** Not a generic malloc replacement — call sites must declare
their region. Not a defragmenting allocator — RES uses segregated free-lists,
not compaction. Not a behavior change for scene rendering or LRU eviction
policy — the cache hits and misses the same resources as today, just inside
a region instead of on the libc heap.

## Memory budget — verified from artifacts

### PSX user-RAM math (from `build-ps1/jcreborn.map`)

| Region                   | Address range            | Size      |
|--------------------------|--------------------------|-----------|
| Exe + BSS (current)      | 0x80010000 .. 0x800ad4fc | 629 KB    |
| Region buffer (proposed) | 0x800ad500 .. ~0x801daf00 | ~1.20 MB |
| Free margin              | -                        | ~150 KB   |
| Stack reserve            | top of RAM, growing down | 64 KB     |
| **Total**                | 0x80010000 .. 0x801FFFF0 | 1.92 MB   |

The region buffer goes in BSS: `static uint8 g_memRegionBuf[MEM_REGION_TOTAL]`.
A `_Static_assert(MEM_REGION_TOTAL <= 1200 * 1024)` at compile time gates
budget changes against the linker map.

### Region splits (verified against `scene_analysis_output_2026-03-21.json`)

```
PS1 region buffer (1.2 MB)
+----------------------------------------------+ 0x00
| PERM region (bump up, ~350 KB)               |
|   gFgFrameBuffer (pre-sized via boot scan)   |
|   gFgPrefetchFrameBuffer (pre-sized)         |
|   gFgStreamScratch (pre-sized)               |
|   gFgStreamWindowBuffer (pre-sized)          |
|   gWalkCleanBuf (149 KB)                     |
|   grBackgroundSfc backing (300 KB)           |
|   surface pool (graphics.c:1280)             |
|   resource struct arrays                     |
|   audio mixer state                          |
|   FROZEN after memFreezePerm()               |
+----------------------------------------------+ permTop
| RES region (segregated free-list, ~600 KB)   |
|   resource->uncompressedData blobs (BMP/TTM/ |
|   SCR/ADS), evicted by existing LRU policy   |
|   peak demand: MARY.ADS tag1 at 568 KB       |
+----------------------------------------------+ sceneBottom (fixed)
| SCENE region (bump down, ~250 KB)            |
|   alloc order, FIRST to LAST:                |
|     1. clean-rect snapshot (≤ 181 KB)        |
|     2. setupSegmentBuffer (≤ 32 KB)          |
|     3. entry table, sound events (~10 KB)    |
|     4. scratch                               |
|   reset at fgRuntimeReset()                  |
+----------------------------------------------+ bufferEnd
```

### Proof-of-fit (the part that makes "no failure" tenable)

- **PERM.** Fully enumerated at boot. The pack-header scan
  (`fgScanPackHeadersForMaxSizes()`) walks all 63 FG2 packs and records the
  max payload size for each of the four grow-only buffers. PERM allocations
  also include fixed-size assets (`gWalkCleanBuf`, `grBackgroundSfc`, font
  tables, surface pool, struct arrays). The total is deterministic at the
  end of boot. `memFreezePerm()` snapshots the used count; any later PERM
  alloc is `fatalError`.

- **SCENE.** A SCENE peak per scene is computed offline from
  `scene_analysis_output_2026-03-21.json` plus the clean-rect estimator
  (`fgBackdropCleanRectEstimateForPack`). At boot, `memVerifyScenesFitInScene()`
  re-runs the estimator against every pack and `fatalError`s if any scene's
  worst-case SCENE footprint exceeds the SCENE budget. Worst case observed
  today: 181 KB clean-rect + 32 KB setup + ~30 KB other ≈ 243 KB, fits the
  250 KB budget with 7 KB margin.

- **RES.** The hard constraint is the *pinned* working set — the resources
  that can't be evicted because an active TTM thread is using them. The
  scene_analysis JSON reports `max_concurrent_threads` (peak = 20 for
  ACTIVITY scenes). Each thread pins at most one TTM + at most one BMP at
  a time. `memVerifyResPinnedFits()` at boot computes
  `pinnedSet = sum of (largest TTM + largest BMP per concurrent thread)`
  across all 63 scenes and `fatalError`s if it exceeds the RES budget.
  Unpinned working set can always be evicted in-place by the LRU.

If any of these boot-time checks `fatalError`, that is the *only* moment
when a budget mismatch is allowed to be discovered. Once boot succeeds,
every allocation that the game can possibly request is provably backed by
sufficient region space.

## API design

New file: `src/mem_region.h` + `src/mem_region.c`. Single header drives both
PS1 and PC.

- **PS1 (`#ifdef PS1_BUILD`):** real region allocator over the static buffer.
- **PC:** API-identical implementation; same compile-time budgets enforced.
  Out-of-budget calls `fatalError` identically. PC retains libc heap for
  external dependencies like SDL.

```c
typedef enum {
    MEM_REGION_PERM,    /* bump up; freezes after boot */
    MEM_REGION_RES,     /* LRU cache; eviction allowed */
    MEM_REGION_SCENE,   /* bump down; wipes between scenes */
} MemRegion;

/* All allocations are aligned to MEM_REGION_ALIGN (8 bytes). Returns a
 * valid pointer or fatalErrors — never returns NULL. */
#define MEM_REGION_ALIGN 8
void *memAlloc(MemRegion region, size_t size, const char *tag);

/* Free is a no-op in PERM (debug build asserts post-freeze), a real
 * release in RES (used only by the LRU evictor — game code never frees
 * RES directly; it touches/unpins resources and the evictor handles
 * actual release), and a balance-decrement in SCENE. */
void memFree(MemRegion region, void *ptr);

/* Wipes SCENE region, logs peak, asserts sceneAllocBalance == 0 in debug. */
void memSceneReset(const char *sceneName);

/* Diagnostics */
size_t memRegionUsed(MemRegion);
size_t memRegionPeak(MemRegion);
void   memLogTelemetry(void);  /* gated behind FG_HEAP_PROBE_LOGS */

/* Boot. memAlloc before this is fatalError. */
void memInit(void);
void memFreezePerm(void);
```

### Boot sequence

```c
int main(...) {
    memInit();                             /* regions ready */
    fgScanPackHeadersForMaxSizes();        /* learn payload maxima */
    memVerifyScenesFitInScene();           /* per-scene SCENE proof */
    memVerifyResPinnedFits();              /* per-scene RES pin proof */
    /* PERM allocations: pre-sized buffers, fixed assets, catalog parse */
    audioInit();
    resourceCatalogParse();
    fontInit();
    surfacePoolInit();
    walkPilotInit();
    memFreezePerm();                       /* PERM is now read-only */
    runMainSceneLoop();
}
```

Any of the three verify functions calling `fatalError` at boot means the
plan needs a budget adjustment before the build ships — not a runtime
behaviour change.

### `sceneAllocBalance`

Renamed from v1's "pin-count" (avoided collision with LRU `pinCount`).
Increments on `memAlloc(SCENE, ...)`, decrements on `memFree(SCENE, ...)`,
asserted == 0 by `memSceneReset` in debug builds. Compiled out in release.

### Telemetry

```
JCMEM perm=512K/512K res=423K/600K scene_peak=234K (MARY.ADS) wipe=187K balance=0
```

Gated behind `FG_HEAP_PROBE_LOGS`. Emitted at the next `fgRuntimeReset()`
(which is at the start of scene N+1's setup, not end of N — see semantics
note below).

## Bandaid + skip-code removal manifest

Phase 2 deletes every site below. The removal is verified by a grep gate
in CI: `grep -rE "JCSKIP|Caller handles gracefully|skip scene silently|skip scene gracefully|Graceful skip|Pool exhausted - fall back|failed silently|allocation failure" src/` must return zero hits after Phase 2.

| # | File:line                                   | Pattern                                | Removal                                                    |
|---|---------------------------------------------|----------------------------------------|------------------------------------------------------------|
| 1 | `foreground_pilot.c:3787`                   | `JCSKIP pack-start-failed`             | Delete; replace cleanup-and-return with fatalError         |
| 2 | `foreground_pilot.c:3808`                   | `JCSKIP draw-bounds-failed`            | Delete; fatalError                                         |
| 3 | `foreground_pilot.c:3847`                   | `JCSKIP clean-rect-alloc-failed`       | Delete; fatalError. Clean-rect is SCENE-reserved.          |
| 4 | `foreground_pilot.c:3782-3786`              | "Graceful skip instead of BSOD" block  | Delete entirely                                            |
| 5 | `foreground_pilot.c:fgDropOptionalPrefetchBuffersForCleanSnapshot` | Drop optional buffers mid-play | Delete function and 4 callers                              |
| 6 | `foreground_pilot.c:fgDropPressureCachesForCleanSnapshot`          | Higher-level pressure drop      | Delete                                                     |
| 7 | `foreground_pilot.c:fgBackdropSaveCleanBgRectsWithPressureFallback`| Retry-after-pressure wrapper    | Replace with plain `fgBackdropSaveCleanBgRects` (no retry) |
| 8 | `foreground_pilot.c:1471,1482`              | JCSTREAM prealloc-failed printfs       | Delete; pre-allocation can't fail under the new allocator  |
| 9 | `foreground_pilot.c:fgPrePrimeStreamBuffers`| Lazy/eager prealloc paths              | Delete; replaced by deterministic boot-time PERM allocs    |
| 10| `resource.c:700-704` (`findAdsResource`)    | `return NULL` on PS1; PC fatalErrors   | Unify: fatalError on both platforms                        |
| 11| `resource.c:715-722` (`findBmpResource`)    | Same NULL-return pattern               | fatalError on both                                         |
| 12| `resource.c:732-737` (`findScrResource`)    | Same                                   | fatalError on both                                         |
| 13| `resource.c:747-752` (`findTtmResource`)    | Same                                   | fatalError on both                                         |
| 14| `ads.c:1696`                                | `if (adsResource == NULL) return;`     | Delete (findAdsResource fatalErrors now)                   |
| 15| `ads.c:1708`                                | `ps1_loadAdsData failed silent return` | fatalError on load failure                                 |
| 16| `ads.c:1753+`                               | "skip this scene gracefully" TTM block | fatalError on TTM load failure                             |
| 17| `walk_pilot.c:108-117`                      | walkClean buf malloc-fail silent skip  | Delete; buf is PERM-allocated at boot, can't fail          |
| 18| `walk_pilot.c:165-176`                      | walkPilotInit alloc-fail soft return   | fatalError                                                 |
| 19| `walk_pilot.c:255-260`                      | JOHNWALK silent-bail-out               | fatalError if `walkPilotEnsureBmp` fails                   |
| 20| `graphics.c:1370-1395`                      | "Pool exhausted - fall back"           | Pool sized correctly; fatalError if exhausted             |
| 21| `cdrom_ps1.c:644,885,2354`                  | malloc-failed NULL returns + prints    | fatalError on PS1; routes through memAlloc                 |
| 22| `ps1_perf.c:1032 (and 6 callers)`           | `ps1PerfMarkAllocFail` + counters      | Delete — no allocation failures exist to mark              |
| 23| `ps1_perf.c:1058,1064`                      | `ps1PerfMarkFallback` family           | Delete *if* the graphics fallbacks they count also vanish (Phase 3 audit confirms) |

**Not removed** — these are intentional design choices, not failure handlers:

- `story.c:493,644` — dead-scene picks and intro-skip-on-override are gameplay logic.
- `ps1_captions.c:597` — empty-caption skip is "this scene has no caption," not a failure.
- `scene_picker.c:347` "screensaver should never go dark" — *policy* choice when Original mode has no FINAL-flagged pick in the current pool. Sierra-faithful; not failure handling. Worth a comment refresh.
- `ps1_features.c:60 ps1SkipToNextScene` — user pressing the skip button.
- `pause_menu.c:1830` — Circle = "I changed my mind", user input.
- Sierra-faithful walk-aware filtering in `scene_picker.c:432,453,462`.
- `foreground_pilot.c:3830` `JCMEM black-clean ... skip-clean-rects` — *positive* code path: black backdrop with temporal residual genuinely doesn't need clean-rects. Keep behaviour; rename print so it doesn't read as a failure ("JCMEM black-backdrop-mode").

## Implementation phases

### Phase 1 — Allocator infrastructure + full migration

1. `src/mem_region.{c,h}` with the API above. ~400 LOC.
2. PC implementation: forward each region to `malloc` with same compile-time
   budgets and identical fatalError semantics.
3. PS1 implementation: bump allocators (8-byte aligned) for PERM/SCENE;
   segregated free-list for RES (size classes 64 B, 256 B, 1 KB, 4 KB,
   16 KB, 64 KB, plus a >64 KB list).
4. Boot integration: `memInit`, `fgScanPackHeadersForMaxSizes`,
   `memVerifyScenesFitInScene`, `memVerifyResPinnedFits`, `memFreezePerm`.
5. Migrate all 39 `safe_malloc`/`malloc` call sites to `memAlloc(REGION, ...)`
   with explicit region tags. No default region.
6. Hook `memSceneReset()` into `fgRuntimeReset()` at
   `src/foreground_pilot.c:1470`.
7. Reorder `foregroundPilotRuntimeStart` so clean-rect allocates first in
   SCENE (currently mid-setup).
8. Adopt the poison-malloc pattern from `src/scene_picker.c:62-78` inside
   `mem_region.c` so libc allocs can't sneak back in.

**Validation:** game renders byte-for-byte identically across the 63-scene
rotation. PC + PS1 builds green. All three boot-time `memVerify*` checks
pass.

### Phase 2 — Delete bandaid + skip code

Delete every row from the removal manifest above (23 sites). Verify with
the grep gate.

**Validation:** soak test 20+ iterations of the 63-scene rotation. Zero
`fatalError` calls (which would indicate a misclassified region or a
hidden allocation path). Visual diff against Phase 1 baseline matches.

### Phase 3 — Audit remaining allocation surfaces

Sites that weren't in the original 39 because they're indirect or in
less-traveled code:

| Call site                                     | Region | Notes                                          |
|-----------------------------------------------|--------|------------------------------------------------|
| `graphics.c:1808` `scrResource->uncompressedData` | RES | LRU-managed; lazy at first use                 |
| `graphics.c:1884` `safe_malloc(640*480)` for grBackgroundSfc | PERM | 300 KB one-shot at graphics init |
| `graphics.c:1935` `bmpResource->uncompressedData` | RES | LRU-managed                                    |
| `graphics.c:1835,1963` per-conversion BMP→surface | SCENE | Conversion scratch                             |
| `ads.c` per-ADS uncompressedData              | RES    | first-play decompress, LRU-managed             |
| `pause_menu.c` (TBD)                          | PERM   | Audit; menu state is long-lived                |
| `ps1_captions.c` (TBD)                        | PERM/SCENE | Audit                                      |

**Validation:** the grep gate stays clean. `git grep "free(" src/` shows only
calls inside `mem_region.c` and the LRU evictor (which calls `memFree(RES, ...)`).

## RES sub-allocator details

- Segregated free-lists at 64 B, 256 B, 1 KB, 4 KB, 16 KB, 64 KB, plus a
  sorted list for >64 KB. Round-up-to-class on alloc; exact-class return on
  free. ~150 LOC.
- First-fit within class.
- On alloc failure (class empty or oversize list can't satisfy):
  1. Call LRU evictor with `requiredBytes`.
  2. Retry once.
  3. If still failing, `fatalError("RES exhausted, scene=%s req=%lu")`.
  4. The boot-time `memVerifyResPinnedFits` is supposed to prove (3) is
     unreachable. If it fires anyway, that's the bug to fix.

## Critical files

- **New:** `src/mem_region.h`, `src/mem_region.c`,
  `src/mem_region_verify.c` (boot-time scene-fit checks).
- **Modified, hot path:**
  - `src/utils.c:123` — `safe_malloc` thin-wraps `memAlloc(MEM_REGION_PERM)`
    for the catalog-parse path only. Other call sites use `memAlloc` directly
    with explicit region tags.
  - `src/foreground_pilot.c:1470` — `fgRuntimeReset` calls `memSceneReset`.
  - `src/foreground_pilot.c:1267-1287` — grow-only comment block deleted;
    buffers are PERM-sized at boot.
  - `src/foreground_pilot.c` — removal manifest items 1-9, 23 (printfs and
    fallback helpers).
  - `src/foreground_pilot.c:3776` — clean-rect alloc reordered to first.
  - `src/resource.c:760-954` — LRU calls `memFree(RES, ...)`. Items 10-13
    (NULL-return guards) deleted.
  - `src/ads.c:1685+` — items 14-16 deleted.
  - `src/walk_pilot.c` — items 17-19 deleted.
  - `src/graphics.c:1370,1884` — items 20 + grBackgroundSfc migration.
  - `src/cdrom_ps1.c` — item 21.
  - `src/ps1_perf.c` — items 22-23.
  - `src/pause_menu.c:602` — JCMEM line added.
- **Read-only references during design:**
  - `src/scene_picker.c:62-78` — poison-malloc pattern.
  - `docs/ps1/release-notes-0.8.11.md` — prior fragmentation incident.
  - `docs/ps1/archaeology/memory-constraints.md` — budget context.
  - `build-ps1/jcreborn.map` — usable-RAM math.
  - `docs/ps1/research/generated/scene_analysis_output_2026-03-21.json` —
    per-scene resource peaks.

## Verification

Pre-implementation gates (must close before Phase 1 lands):

- [ ] Rebuild PS1 binary, re-read `_end` from a fresh map. Confirm exe+BSS
      is ~629 KB after any unrelated drift.
- [ ] Confirm `scene_analysis_output_2026-03-21.json` `peak_memory_bytes`
      really is the full RES footprint (BMP + TTM + SCR), not just BMP.
- [ ] Confirm `grBackgroundSfc` (graphics.c:1884) is genuinely one-shot;
      grep for re-allocation paths.

End-to-end gates (must close before merge):

1. **Build green on both platforms.** Compile, `_Static_assert` for total
   buffer size passes.
2. **Boot proofs pass.** `memVerifyScenesFitInScene` and
   `memVerifyResPinnedFits` succeed for all 63 packs on PS1 and PC.
3. **Long-run rotation:** 20+ iterations of the 63-scene rotation on PS1
   hardware. Existing band counts (117 green / 9 yellow / 0 orange / 0 red
   per `docs/ps1/release-notes-0.8.11.md`) do not regress.
4. **Heap probe stability:** `fgProbeLargestAlloc()` at scene-start is
   constant across the rotation. (With one static buffer, this is
   essentially tautological — kept as a sanity check.)
5. **Removal grep gates:** the regex from the manifest above returns zero
   hits in `src/`.
6. **No `fatalError` triggers** across the soak test. If one fires, the
   bug is in the plan, not the code.
7. **`sceneAllocBalance` never positive at reset** in debug builds.
8. **`memFreezePerm` not triggered post-boot.**
9. **PC valgrind clean** (one static buffer "still reachable" at exit
   is acceptable).

### Semantics gotcha (kept visible)

`fgRuntimeReset()` runs at the *start* of the next scene's setup, not at
the end of the current scene. So the JCMEM "wipe=NNN" reading for scene N
appears at the start of scene N+1's log. Not a bug; a grep gotcha worth
knowing about. `fgRuntimeReset()` also used to run on the JCSKIP failure
cleanup path (`foreground_pilot.c:3777`) — once JCSKIP is removed, that
cleanup path is removed with it. The next scene's `fgRuntimeReset` is the
only reset.
