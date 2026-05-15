# Memory region allocator — implementation plan

**Status:** v2 — incorporates red-team feedback. See companion document at
[memory-region-allocator-red-team.md](./memory-region-allocator-red-team.md)
for the prior critique that drove these revisions, and
[memory-region-allocator-red-team-v2.md](./memory-region-allocator-red-team-v2.md)
for the principal-engineer review of this v2.

## What changed from v1

Three 🔴 showstoppers, six 🟠 risks, and seven 🟡 hygiene items from the v1
red-team are folded into the plan below. The biggest structural changes:

- **Budgets were sized backwards.** v1 had SCENE = 700 KB; the corrected number
  is ~250 KB. The 568 KB MARY peak is *resource bytes*, which live in RES.
- **Total memory ceiling is 1.2 MB, not 1.8 MB.** Verified against the actual
  linker map (`build-ps1/jcreborn.map`): `_end = 0x800ad4fc`, so exe + BSS =
  629 KB, leaving ~1.26 MB usable above `_end` and below the stack.
- **Frame buffers are pre-sized at boot** via a pack-header scan so they can
  live in PERM. The pressure-drop fallback (which v1 would have broken) is
  no longer required.
- **Allocation order inside SCENE is now explicit:** clean-rect first. The
  bump pointer can't reclaim space, so the largest variable-sized SCENE
  allocation must get first dibs.
- **No "default-to-PERM" during migration.** Every call site gets an
  explicit region tag in Phase 1.

## Context

The PS1 build runs in 2 MB of main RAM and currently uses libc `malloc`/`free`
through a single wrapper, `safe_malloc` (`src/utils.c:123`). Across 39 call
sites (34 `safe_malloc` + 5 raw `malloc`) the heap takes per-scene churn:
per-scene buffers (`gFgSetupSegmentBuffer`, clean-rect snapshots, entry
tables, sound-event arrays) are allocated and freed on every scene
transition, while a 600 KB LRU resource cache (`src/resource.c`) holds
variable-size BMP/TTM/SCR/ADS bytes that come and go as scenes touch them.

That churn fragments the heap. The v0.8.10-ps1 release shipped a "heap
fragmentation experiment" (256 KB CD sector pool + boot-time stream buffers)
that immediately broke large clean-rect scenes — `JCSKIP clean-rect-alloc-failed`
fired because the pinned blocks left no contiguous space for the snapshot. The
v0.8.11-ps1 release rolled it back. Current bandaids — "grow-only" frame buffers
(`src/foreground_pilot.c:1267-1287`), a resident 149 KB walk buffer
(`src/walk_pilot.c:50-67`), pressure-drop caches that free optional buffers
mid-play (`fgDropOptionalPrefetchBuffersForCleanSnapshot()` called from at
least four sites), and the resource LRU itself — partly hide the problem but
don't fix the underlying allocator behavior. The heap probe at
`src/foreground_pilot.c:1424` already exists to report largest-free-block.

This plan introduces a deterministic three-region allocator that owns one
static buffer for the lifetime of the process. PERM grows at boot and never
shrinks. RES holds the existing LRU cache inside its own bounded sub-allocator
so eviction stays in-place. SCENE is a bump allocator that wipes wholesale on
every scene transition. Marking SCENE allocations is free; resetting is a
single pointer write plus a development-only zero-pin assert.

## Goals / non-goals

**Goals.** Eliminate the class of "scene N+1 can't allocate because the heap
fragmented during scenes 1..N" bugs. Keep the existing LRU hit-rate so common
resources stay resident across scene transitions. Add zero measurable per-alloc
overhead on the hot path. Match the existing JCPERF/JCSKIP telemetry style.
Keep the PC build functional with the same API.

**Non-goals.** Not a generic malloc replacement — call sites must declare their
region. Not a defragmenting allocator — RES uses segregated free-lists, not
compaction. Not a behavior change for scene rendering or LRU eviction policy —
the cache hits and misses the same resources as today.

## Memory budget — verified from artifacts

### PSX user-RAM math (from `build-ps1/jcreborn.map`)

| Region                   | Address range            | Size      |
|--------------------------|--------------------------|-----------|
| Exe + BSS (current)      | 0x80010000 .. 0x800ad4fc | 629 KB    |
| Region buffer (proposed) | 0x800ad500 .. ~0x801daf00 | ~1.20 MB  |
| Free margin              | -                        | ~150 KB   |
| Stack reserve            | top of RAM, growing down | 64 KB     |
| **Total**                | 0x80010000 .. 0x801FFFF0 | 1.92 MB   |

The region buffer goes in BSS (`static uint8 g_memRegionBuf[MEM_REGION_TOTAL];`).
A `_Static_assert` at compile time gates `MEM_REGION_TOTAL <= 1.2 MB` so any
budget bump must be justified against the linker map.

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
|   grBackgroundSfc backing (300 KB if loaded) |
|   surface pool (graphics.c:1280, 4 slots)    |
|   resource struct arrays                     |
|   audio mixer state                          |
|   sentinel: pointer frozen after boot        |
+----------------------------------------------+ permTop
| RES region (segregated free-list, ~600 KB)   |
|   resource->uncompressedData blobs (BMP/TTM/ |
|   SCR/ADS), evicted by existing LRU policy   |
|   max single resource: MARY BMP-indexed      |
|   (406 KB); typical scene working set 350-   |
|   570 KB                                     |
+----------------------------------------------+ sceneBottom (fixed)
| SCENE region (bump down, ~250 KB)            |
|   alloc order, top to bottom (largest first):|
|     1. clean-rect snapshot (up to 181 KB)    |
|     2. setupSegmentBuffer (~32 KB)           |
|     3. entry table, sound events (~10 KB)    |
|     4. scratch                               |
|   reset on every fgRuntimeReset() call       |
+----------------------------------------------+ bufferEnd
```

**Why 350/600/250.** MARY.ADS tag1 peaks at 568,578 bytes of resource working
set per `scene_analysis_output_2026-03-21.json` — that fits in RES = 600 KB.
FISHING-class clean-rect is 181 KB and lives in SCENE. PERM holds everything
that's allocated once: 149 KB walk buf + pre-sized frame buffers (worst-case
~120 KB after boot scan) + grBackgroundSfc (300 KB if SCR backdrop is loaded
— only one scr loaded at a time so this is a slot, not a stack) + assorted
structs ≈ 350 KB headroom.

**Why a pack-header scan.** v1 wanted to put `gFgFrameBuffer` etc. in PERM
but they grow over the first scenes — a bump allocator can't grow PERM
mid-game. The fix is to learn the max-sized scene at boot. FG2 pack headers
are tiny (~32 bytes each, 63 packs ≈ 2 KB of header reads). Scan once at
`memInit()`, pick the max payload across all packs, allocate that in PERM.
After this, the pressure-drop fallback in
`fgDropOptionalPrefetchBuffersForCleanSnapshot()` is no longer needed and
can be removed.

## API design

New file: `src/mem_region.h` + `src/mem_region.c`. Single header drives both
PS1 and PC. The hybrid build:

- **PS1 (`#ifdef PS1_BUILD`):** real region allocator over one static buffer.
- **PC:** API-identical implementation. Same compile-time budgets enforced
  identically — `memAlloc(REGION, n)` returns NULL when the region's logical
  budget is exhausted, even though backing `malloc` would have succeeded.
  This way PC tests catch over-budget bugs that would otherwise only show
  up on PS1 hardware. Set `JC_MEM_SIMULATE_PS1_BUDGET=0` to relax during
  unrelated PC debugging.

```c
typedef enum {
    MEM_REGION_PERM,    /* grow-only at boot, never freed */
    MEM_REGION_RES,     /* LRU cache; eviction allowed */
    MEM_REGION_SCENE,   /* bump+wipe between scenes */
} MemRegion;

/* All allocations are aligned to MEM_REGION_ALIGN (8 bytes — covers
 * uint64/double/pointer alignment on MIPS R3000A even though the CPU
 * doesn't have a hardware double). Sizes are rounded up before bump. */
#define MEM_REGION_ALIGN 8

void *memAlloc(MemRegion region, size_t size, const char *tag);
            /* tag is a const-string call-site identifier (e.g.,
             *  "fgEntryTable") used in JCMEM telemetry. NULL allowed. */
void  memFree (MemRegion region, void *ptr);
            /* PERM:  fatalError in debug, no-op in release */
            /* RES:   real free (used by LRU evictor) */
            /* SCENE: decrements sceneAllocBalance; no real free */

void memSceneReset(const char *sceneName);
            /* Wipes SCENE region, logs peak, asserts
             * sceneAllocBalance == 0 in debug builds. */

/* Diagnostics — zero overhead on hot path */
size_t memRegionUsed(MemRegion);
size_t memRegionPeak(MemRegion);
void   memLogTelemetry(void);
            /* Emits one JCMEM line; gated behind FG_HEAP_PROBE_LOGS. */

/* Boot. memAlloc before this calls fatalError. */
void memInit(void);
```

### Boot order (encoded as comments + an init-stage counter)

```
main() {
    memInit();                            /* stage 1: regions ready */
    fgScanPackHeadersForMaxSizes();       /* stage 2: learn max payloads */
    /* PERM allocations: frame buffers @ max from scan, walk buf,
     * surface pool, audio mixer, resource catalog parse, font load. */
    /* After all PERM is done, the implementation calls memFreezePerm()
     * which records permTop. Any PERM alloc after this is a fatalError. */
    memFreezePerm();
    /* Now scenes can run. */
    runMainSceneLoop();
}
```

Calling `memAlloc(MEM_REGION_PERM, ...)` after `memFreezePerm()` is a
`fatalError`. Catches "I tried to allocate something permanent during a
scene" bugs at the first call site instead of by silent overflow.

### Telemetry shape (gated)

```
JCMEM perm=512K/512K res=423K/600K scene_peak=234K (MARY.ADS) wipe=187K balance=0
```

Gated behind the existing `FG_HEAP_PROBE_LOGS` build flag so log volume
stays opt-in. Emitted at scene-end (technically at the next `fgRuntimeReset`
which runs at the start of scene N+1; see "semantics" note below). Also
emitted on pause-menu show.

### `sceneAllocBalance` (renamed from "pin-count" in v1)

Renamed to avoid collision with the LRU's `pinCount` field on resources.
Same semantics: increments on `memAlloc(SCENE, ...)`, decrements on
`memFree(SCENE, ...)`. `memSceneReset` asserts the balance is 0 in debug
builds before wiping. Compiled out in release builds.

### Failure contract for RES

`memAlloc(RES, n)` proceeds as:

1. Try segregated free-list (size class match).
2. If empty, call LRU evictor with required bytes.
3. Retry once.
4. If still failing, return NULL. **No further fallbacks.**

Callers handle NULL the same way they currently handle `malloc()` returning
NULL (which is "occasionally"): some propagate up to JCSKIP, some
`fatalError`. The allocator does not silently degrade or spin.

## Implementation phases

Three phases, each independently shippable and testable.

### Phase 1 — Infrastructure, full migration of all 39 call sites

1. Add `src/mem_region.{c,h}` with the API above.
2. **PC implementation:** forward each region's allocs to `malloc` with
   bookkeeping. Same compile-time budgets enforced; over-budget returns NULL.
3. **PS1 implementation:** one static buffer; PERM/SCENE bump allocators
   (8-byte aligned); RES segregated free-list with size classes 64 B, 256 B,
   1 KB, 4 KB, 16 KB, 64 KB, and a sorted list for >64 KB. Eviction integrates
   with `src/resource.c`'s LRU. ~400 LOC.
4. Add `fgScanPackHeadersForMaxSizes()` — reads all 63 FG2 pack headers at
   boot, records the max payload sizes for frame/prefetch/scratch/window
   buffers. Allocates them in PERM at the learned max.
5. Hook `memSceneReset()` into `fgRuntimeReset()` at
   `src/foreground_pilot.c:1470`.
6. **Migrate all 39 call sites in this phase.** No default — every
   `safe_malloc` / `malloc` call gets an explicit region tag. Per-call-site
   migration map is the union of the Phase 2/Phase 3 tables below plus the
   miscellaneous sites surveyed in the codebase audit.
7. Remove `fgDropOptionalPrefetchBuffersForCleanSnapshot()` and its callers
   — no longer needed once frame buffers are pre-sized at boot max.
8. Reorder `foregroundPilotRuntimeStart` so clean-rect allocates first in
   SCENE setup (currently it allocates mid-setup, after several KB of other
   SCENE state).
9. Add telemetry line.

**Validation:** game runs identically to today, byte-for-byte rendering
match across the 63-scene rotation. PC + PS1 builds green. PC `valgrind`
shows no new leaks (region buffer is one giant block, all SCENE bytes
freed at process exit). JCMEM peaks match per-scene data from
`scene_analysis_output_2026-03-21.json`.

### Phase 2 — Remove fragmentation bandaids

With the allocator in place, the bandaid code added to fight fragmentation
becomes dead weight. Phase 2 deletes it cleanly:

- `fgDropPressureCachesForCleanSnapshot` (all callers) — clean-rect now
  allocates first in SCENE, so the drop path is unreachable.
- The "grow-only" exception in `fgReleaseStreamBuffers` — buffers are
  pre-sized in PERM, never freed.
- `fgPrePrimeStreamBuffers` and its lazy/eager logic — replaced by the
  boot-time pack-header scan.
- The retry/fallback logic in `fgBackdropSaveCleanBgRectsWithPressureFallback`
  — clean-rect either fits in SCENE or it doesn't; on NULL return, the
  existing JCSKIP path handles it.

**Validation:** the band counts from `docs/ps1/release-notes-0.8.11.md`
(117 green / 9 yellow / 0 orange / 0 red) must not regress. Long-run soak
test (20+ iterations of the 63-scene rotation) shows zero
`JCSKIP clean-rect-alloc-failed` instances.

### Phase 3 — Audit remaining allocation surfaces

| Call site                                            | Region | Notes |
|------------------------------------------------------|--------|-------|
| `graphics.c:1884` `safe_malloc(640*480)` for grBackgroundSfc | PERM | 300 KB one-shot; allocate during init |
| `graphics.c:1808` scrResource->uncompressedData       | RES    | LRU-managed |
| `graphics.c:1935` bmpResource->uncompressedData       | RES    | LRU-managed |
| `graphics.c:1835,1963` per-conversion `width*height`  | SCENE  | temporary during BMP→surface conversion |
| `pause_menu.c` allocations (TBD)                      | PERM   | audit — menu state is long-lived |
| `ads.c` per-ADS uncompressedData                      | RES    | first-play decompress, LRU-managed |

**Note:** ADS data enters RES at first-play time, not at boot. `parseAdsResource`
sets `uncompressedData = NULL` at parse (`resource.c:241`); the actual
decompression happens lazily in `ads.c`. The RES sub-allocator handles
this just fine; it just means the RES high-water mark grows over the first
few scenes' playtime rather than being known at boot.

**Note (removed from v1):** `ads.c:1134 ttmLayer` is **not** a malloc.
It's a pool handout via `grNewLayer()` (`graphics.c:1280`, see "pooled
allocation" comment). The pool slots are PERM-resident; individual
handouts are pool ops. No migration work needed here.

**Validation:** all 39 call sites accounted for. The pin-poison pattern from
`src/scene_picker.c:62-78` is added to `src/mem_region.c` to prevent direct
`malloc`/`free` from sneaking back in.

## RES region sub-allocator details

The middle region is the only one with allocator complexity. Approach:

- **Segregated free-lists by size class** (64 B, 256 B, 1 KB, 4 KB, 16 KB,
  64 KB, plus a sorted list for >64 KB). Round-up-to-class on alloc, exact
  return to class on free. ~150 LOC.
- **Why not TLSF:** TLSF is the right answer if we expected truly arbitrary
  size distributions. The actual resource sizes cluster — TTM bytecode in
  low KB, BMP frame data 20-100 KB, SCR backgrounds ~300 KB. Six size
  classes cover the distribution without TLSF's bitmap machinery.
- **First-fit within class** keeps the per-alloc cost at ~10 cycles.
- **Eviction integration:** failure contract above.

## Critical files

- **New:** `src/mem_region.h`, `src/mem_region.c`
- **Modified, hot path:**
  - `src/utils.c:123` — `safe_malloc` becomes a thin wrapper around
    `memAlloc(MEM_REGION_PERM, ...)` for the resource-catalog-parse path
    only (which is the major remaining caller). Other call sites use
    `memAlloc(...)` directly with explicit region tags.
  - `src/foreground_pilot.c:1470` — `fgRuntimeReset()` calls `memSceneReset()`.
  - `src/foreground_pilot.c:1267-1287` — grow-only comment block can be
    deleted; buffers are pre-sized.
  - `src/foreground_pilot.c:1383+` — `fgDropOptionalPrefetchBuffersForCleanSnapshot`
    deleted along with its 4 callers.
  - `src/foreground_pilot.c:3776` — clean-rect allocation reordered to first
    in SCENE setup.
  - `src/resource.c:760-954` — LRU still owns eviction *policy*; calls
    `memFree(RES, ...)` instead of `free()`.
  - `src/pause_menu.c:602` — JCMEM line added next to existing heap probe.
  - `src/graphics.c:1884` — grBackgroundSfc backing migrated to PERM.
- **Read-only references during design:**
  - `src/scene_picker.c:62-78` — exemplar of the "poison malloc" pattern;
    `src/mem_region.c` will use the same pattern internally so libc allocs
    can't sneak in.
  - `docs/ps1/release-notes-0.8.11.md` — prior fragmentation incident.
  - `docs/ps1/archaeology/memory-constraints.md` — budget context.
  - `build-ps1/jcreborn.map` — source of truth for usable-RAM math.
  - `docs/ps1/research/generated/scene_analysis_output_2026-03-21.json` —
    source of truth for per-scene resource peaks.

## Verification

Pre-implementation gates (must close before Phase 1 lands):

- [ ] Rebuild PS1 binary, re-read `_end` from the fresh map. Confirm exe+BSS
      is still ~629 KB after any unrelated drift.
- [ ] Manually verify `scene_analysis_output_2026-03-21.json` `peak_memory_bytes`
      includes both BMP and SCR data — i.e., it really is the full RES footprint
      for a scene, not just the BMP slice.
- [ ] Spot-check `graphics.c:1884` lifecycle: confirm `grBackgroundSfc` is
      allocated exactly once at init and never reallocated, otherwise it
      can't be PERM.

End-to-end checks, in order:

1. **Build green on both platforms.** `./scripts/build-ps1.sh` and the PC
   build each compile and link with phase 1 in place. `_Static_assert` for
   total buffer size compiles.
2. **Long-run rotation:** play all 63 scenes in a loop for 20+ iterations
   on PS1 hardware. Existing JCSKIP/JCPERF telemetry should show no
   regressions in band counts (currently 117 green / 9 yellow / 0 orange / 0
   red per `docs/ps1/release-notes-0.8.11.md`).
3. **Heap probe stability:** the largest-free-block reading from
   `fgProbeLargestAlloc()` at scene-start should be a constant across the
   rotation (within ~16-byte free-list granularity), not trending down.
4. **JCMEM peaks line up:** scene peaks reported by JCMEM should match
   `scene_analysis_output_2026-03-21.json` per-scene resource peaks (within
   the alignment-rounding margin).
5. **`sceneAllocBalance` never positive at reset** in debug builds across
   the full rotation. If it fires, a call site was misclassified.
6. **No `JCSKIP clean-rect-alloc-failed`** anywhere across the long-running
   test. This is the specific regression v0.8.10 introduced and v0.8.11
   reverted; this plan must not reintroduce it.
7. **PC build:** runs cleanly under valgrind (memcheck), no new leaks from
   the wrapper layer. The single static region buffer shows as one
   "still reachable" block at exit — acceptable.
8. **`memFreezePerm` not triggered post-boot:** no PERM allocations during
   scene playback. If this fires, a long-lived allocation slipped into
   what should be SCENE.

### Semantics gotcha (kept visible)

`fgRuntimeReset()` runs at the *start* of the next scene's setup, not at the
end of the current scene. So the JCMEM "wipe=NNN" reading for scene N
appears in the log at the start of scene N+1's output. Not a bug; just a
docs-and-grep gotcha worth knowing about. `fgRuntimeReset()` is also called
in the JCSKIP failure cleanup path at `foreground_pilot.c:3777`, which
means partial allocations from a failed scene-start get cleaned up by the
next reset attempt. That outcome is correct.
