# Memory region allocator — implementation plan

**Status:** approved, pre-implementation. See companion red-team document at
[memory-region-allocator-red-team.md](./memory-region-allocator-red-team.md).

## Context

The PS1 build runs in 2 MB of main RAM and currently uses libc `malloc`/`free`
through a single wrapper, `safe_malloc` (`src/utils.c:123`). Across ~50 call
sites the heap takes per-scene churn: per-scene buffers (`gFgSetupSegmentBuffer`,
clean-rect snapshots, entry tables, sound-event arrays) are allocated and freed
on every scene transition, while a 600 KB LRU resource cache (`src/resource.c`)
holds variable-size BMP/TTM/SCR/ADS bytes that come and go as scenes touch them.

That churn fragments the heap. The v0.8.10-ps1 release shipped a "heap
fragmentation experiment" (256 KB CD sector pool + boot-time stream buffers)
that immediately broke large clean-rect scenes — `JCSKIP clean-rect-alloc-failed`
fired because the pinned blocks left no contiguous space for the snapshot. The
v0.8.11-ps1 release rolled it back. Current bandaids — "grow-only" frame buffers
(`src/foreground_pilot.c:1265-1287`), a resident 149 KB walk buffer
(`src/walk_pilot.c:50-67`), pressure-drop caches that free optional buffers
mid-play, and the resource LRU itself — partly hide the problem but don't fix
the underlying allocator behavior. Heaviest scene measured is MARY.ADS tag1 at
555.3 KB peak; the heap probe at `src/foreground_pilot.c:1424` already exists
to report largest-free-block, so we have telemetry to confirm any fix.

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

## Architecture: three regions, one static buffer

```
PS1 RAM (one static buffer, byte offsets below)
+----------------------------------------------+ 0x00
| PERM region (bump up)                        |
|   engine/font/audio mixer state              |
|   gFgFrameBuffer (grow-only)                 |
|   gFgPrefetchFrameBuffer (grow-only)         |
|   gFgStreamScratch, gFgStreamWindowBuffer    |
|   gWalkCleanBuf (149 KB resident)            |
|   surface pool (graphics.c:1237)             |
|   resource struct arrays (TBmpResource etc.) |
|   sentinel: pointer never moves after boot   |
+----------------------------------------------+ permTop (frozen post-boot)
| RES region (segregated free-list + LRU)      |
|   resource->uncompressedData blobs           |
|   LRU eviction inside this region only       |
|   call out to evictor when alloc fails       |
+----------------------------------------------+ sceneBottom (fixed)
|                                              |
|                                              |
| SCENE region (bump down)                     |
|   entry tables, sound events                 |
|   setupSegmentBuffer                         |
|   clean-rect snapshots                       |
|   any per-scene scratch                      |
|   reset on every fgRuntimeReset() call       |
+----------------------------------------------+ bufferEnd
```

PERM and SCENE are bump allocators (~3 instructions per alloc, no metadata).
RES owns a contiguous slab with its own segregated free-list; fragmentation
inside RES cannot starve PERM or SCENE because the slab boundary is fixed at
boot. LRU evictions stay inside RES.

### Budget sizing (PS1, 1.6 MB usable after kernel/stack)

| Region | Budget    | Rationale                                                    |
|--------|-----------|--------------------------------------------------------------|
| PERM   | ~500 KB   | Engine + grow-only buffers + walk buf (149 KB) + struct arrays |
| RES    | 600 KB    | Matches current `memoryBudget` at `src/resource.c:133`       |
| SCENE  | 700 KB    | Largest scene 555 KB + clean-rect 181 KB + headroom          |
| **Total** | **1.8 MB** | Tight but fits inside the 1.6-1.8 MB usable envelope.     |

Budgets are compile-time constants but easily tunable. The plan calls for
running each scene through the heap-probe telemetry on first boot of the new
allocator and adjusting if PERM peak < budget (give the slack to RES or SCENE).

## API design

New file: `src/mem_region.h` + `src/mem_region.c`. Single header drives both
PS1 and PC. The hybrid build:

- **PS1 (`#ifdef PS1_BUILD`):** real region allocator over one static buffer.
- **PC:** API-identical stub that forwards each region's allocs to `malloc` and
  tracks high-water marks. `memSceneReset()` calls `free()` on tracked SCENE
  pointers. PC keeps libc/valgrind/ASan visibility for development.

```c
typedef enum {
    MEM_REGION_PERM,    /* grow-only at boot, never freed */
    MEM_REGION_RES,     /* LRU cache; eviction allowed */
    MEM_REGION_SCENE,   /* bump+wipe between scenes */
} MemRegion;

void *memAlloc(MemRegion region, size_t size, const char *tag);
void  memFree (MemRegion region, void *ptr);
            /* PERM:  asserts in debug, no-op in release */
            /* RES:   real free (used by LRU evictor) */
            /* SCENE: increments outstanding count; no real free */

void memSceneReset(const char *sceneName);
            /* Wipes SCENE region, logs peak, asserts pin-count == 0 */

/* Diagnostics — zero overhead on hot path */
size_t memRegionUsed(MemRegion);
size_t memRegionPeak(MemRegion);
void   memLogTelemetry(void);    /* one JCMEM line, matches JCPERF style */

/* Boot */
void memInit(void);  /* called once at startup, before any allocation */
```

### Telemetry shape

```
JCMEM perm=512K/512K res=423K/600K scene_peak=587K (MARY.ADS) wipe=423K
```

Emitted at scene-end and on pause-menu show (alongside the existing
`fgProbeLargestAlloc` line in `src/pause_menu.c:602`).

### Pin-count proof of zero-outstanding

On PS1 debug builds (`MEM_DEBUG_SCENE_PINS`), every SCENE alloc increments a
counter, every `memFree(SCENE, ...)` decrements it. `memSceneReset` asserts
the counter is zero before wiping. Catches "I accidentally allocated this in
SCENE but treat it as long-lived" bugs during development. Compiled out in
release builds — zero runtime cost.

## Implementation phases

Three phases, each independently shippable and testable.

### Phase 1 — Infrastructure, no behavior change

1. Add `src/mem_region.{c,h}` with the API above.
2. **PC implementation:** forward to `malloc`/`free` with bookkeeping. Working
   on day one.
3. **PS1 implementation:** static buffer; PERM/SCENE bump allocators; RES uses
   a segregated free-list (size classes: 64 B, 256 B, 1 KB, 4 KB, 16 KB, 64 KB,
   variable >64 KB). When an alloc can't be satisfied, call the existing LRU
   evictor to free unpinned blocks inside RES. ~400 LOC total.
4. Hook `memSceneReset()` into `fgRuntimeReset()` at
   `src/foreground_pilot.c:1470`. Initially a no-op call site — no allocations
   are migrated yet.
5. Add telemetry line.

**Validation:** game runs identically to today (no allocations touch the
regions yet). PC + PS1 builds green. New telemetry line shows zeros.

### Phase 2 — Migrate per-scene allocations to SCENE region

Migrate the high-traffic per-scene allocations first since they're what causes
fragmentation:

| Call site                                          | Region   | Notes |
|----------------------------------------------------|----------|-------|
| `foreground_pilot.c:2428` setupSegmentBuffer       | SCENE    | already freed on scene end |
| `foreground_pilot.c:1531` soundEvents              | SCENE    | freed in `fgRuntimeReset` |
| `foreground_pilot.c:877` entryTable                | SCENE    | freed in `fgFreeEntryTable` |
| clean-rect snapshots (`grSaveCleanBgRects`)        | SCENE    | freed at `grFreeCleanBgRects` |
| `ads.c:1134` ttmLayer per-scene                    | SCENE    | freed at scene end |
| PC-side per-frame `screenBuffer` (jc_reborn.c:1264)| SCENE    | naturally frame-scoped |

Once these are in SCENE, the per-scene `free()` calls become `memFree(SCENE, ...)`
which is a no-op (release) or pin-decrement (debug). `memSceneReset()` does
the real wipe.

**Validation:** run the full 63-scene rotation. JCMEM telemetry shows SCENE
peak per scene; compare against current `pinResource` tracking from
`src/resource.c`. No regressions in JCSKIP rate. Heap probe
(`fgProbeLargestAlloc`) at scene-start should now be roughly constant across
the rotation instead of degrading.

### Phase 3 — Migrate PERM allocations and resource cache

| Call site                                            | Region | Notes |
|------------------------------------------------------|--------|-------|
| `foreground_pilot.c:1400` gFgFrameBuffer             | PERM   | grow-only, currently lazy |
| `foreground_pilot.c:1411` gFgStreamScratch           | PERM   | grow-only |
| `foreground_pilot.c:gFgPrefetchFrameBuffer/StreamWindowBuffer` | PERM | grow-only |
| `walk_pilot.c:108` gWalkCleanBuf (149 KB)            | PERM   | resident |
| `graphics.c:1237` surface pool                       | PERM   | 4-slot, resident |
| `resource.c:158-499` parseXxxResource struct mallocs | PERM   | one-shot at boot |
| `resource.c:loadOrUncompress` uncompressedData blobs | RES    | LRU-managed |
| `ads.c:` per-ADS uncompressedData                    | RES    | LRU-managed |

The `uncompressedData` blobs on resource structs become RES allocations. The
existing `checkMemoryBudget()` / LRU eviction logic at `src/resource.c:883-954`
calls `memFree(RES, ...)` instead of `free()` directly. RES's internal allocator
will also call back into the evictor if it can't satisfy a request from its own
free-list — this is the one new feedback loop and it needs careful integration
testing.

**Validation:** full rotation again. RES peak should hit the 600 KB budget on
heavy scenes (matches current behavior); eviction events still fire on the
expected boundaries. No `JCSKIP clean-rect-alloc-failed` anywhere across a
long-running session (this is the v0.8.10 regression we're guarding against).
PERM `peak` should stabilize after the first 2-3 scenes (matches the existing
"grow-only" comment block at `foreground_pilot.c:1267`).

## RES region sub-allocator details

The middle region is the only one with allocator complexity. Approach:

- **Segregated free-lists by size class** (64 B, 256 B, 1 KB, 4 KB, 16 KB,
  64 KB, plus a sorted list for >64 KB). Round-up-to-class on alloc, exact
  return to class on free. ~150 LOC.
- **Eviction integration:** if no class can satisfy a request, call into the
  existing LRU evictor with the required size; retry once.
- **Why not TLSF:** TLSF is the right answer if we expected truly arbitrary
  size distributions. The actual resource sizes cluster — TTM bytecode in low
  KB, BMP frame data 20-100 KB, SCR backgrounds ~300 KB. Six size classes
  cover the distribution without TLSF's bitmap machinery.
- **First-fit within class** keeps the per-alloc cost at ~10 cycles.

## Critical files

- **New:** `src/mem_region.h`, `src/mem_region.c`
- **Modified, hot path:**
  - `src/utils.c:123` — `safe_malloc` keeps existing signature, becomes a thin
    wrapper around `memAlloc(MEM_REGION_PERM, ...)`. Existing callers that
    haven't been migrated yet land in PERM by default (safe overestimate during
    the migration; revisit each in phase 2/3).
  - `src/foreground_pilot.c:1470` — `fgRuntimeReset()` adds
    `memSceneReset(currentSceneName)` at the top.
  - `src/foreground_pilot.c:1424` — `fgProbeLargestAlloc()` already gives the
    "is the heap healthy" signal; keep it, complement with JCMEM line.
  - `src/resource.c:883` — `checkMemoryBudget()` adjusted to call `memFree(RES, ...)`.
  - `src/pause_menu.c:602` — additionally show JCMEM line.
- **Read-only references during design:**
  - `src/scene_picker.c:62-78` — exemplar of the "poison malloc" pattern; the
    new allocator file will use the same pattern internally to prove its own
    region wrappers aren't sneaking in libc allocations.
  - `docs/ps1/release-notes-0.8.11.md` — prior fragmentation incident.
  - `docs/ps1/archaeology/memory-constraints.md` — budget context.

## Verification

End-to-end checks, in order:

1. **Build green on both platforms.** `./scripts/build-ps1.sh` and the PC build
   each compile and link with phase 1 in place.
2. **Long-run rotation:** play all 63 scenes in a loop for 20+ iterations on
   PS1 hardware. Existing JCSKIP/JCPERF telemetry should show no regressions
   in band counts (currently 117 green / 9 yellow / 0 orange/red per
   `docs/ps1/release-notes-0.8.11.md`).
3. **Heap probe stability:** the largest-free-block reading from
   `fgProbeLargestAlloc()` at scene-start should be roughly flat across the
   rotation instead of trending down. Compare phase 0 (baseline) vs phase 3
   (full migration).
4. **JCMEM peaks line up:** scene peaks reported by JCMEM should match the
   per-scene memory data already in
   `docs/ps1/research/generated/scene_analysis_output_2026-03-21.json`
   (heaviest MARY.ADS tag1 at 555.3 KB).
5. **Pin-count assert never fires** in debug builds across the full rotation —
   if it fires, an alloc was placed in the wrong region and needs to be moved.
6. **No `JCSKIP clean-rect-alloc-failed`** anywhere across the long-running
   test. This is the specific regression v0.8.10 introduced and v0.8.11
   reverted; this plan must not reintroduce it.
7. **PC build:** runs cleanly under valgrind (memcheck), no new leaks from
   the wrapper layer.
