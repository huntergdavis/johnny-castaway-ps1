# Memory region allocator — implementation plan

**Status:** v4 — resolves every finding (blocker, material, and hygiene) from
the multi-reviewer red-team. See:

- [memory-region-allocator-red-team.md](./memory-region-allocator-red-team.md) — v1 technical red-team
- [memory-region-allocator-red-team-v2.md](./memory-region-allocator-red-team-v2.md) — v2 principal-engineer multi-reviewer panel
- [adding-new-scenes-memory.md](./adding-new-scenes-memory.md) — future-maintainer guide (referenced from the plan)

## What changed in v4

Every concern raised in v2's panel review is folded in. Highlights:

- **Regions renamed** to encode lifetime, not contents: `BOOT` (was PERM),
  `CACHE` (was RES), `TRANSIENT` (was SCENE).
- **Concurrency model** is explicit: `memAlloc` is main-thread-only;
  ISR/callback code never allocates. Compile-time enforced.
- **Pack-header metrics generated offline** at build time and committed as
  `src/generated/pack_header_metrics.h` — boot pays zero CD seeks for the
  scan. Includes a SHA-256 hash per pack so runtime can verify pack/JSON
  agreement at boot.
- **Holiday variants enumerated** in the offline metrics generator; the boot
  proof covers every variant of every scene, not just base packs.
- **Uninitialized TRANSIENT bytes** are poisoned with `0xCD` on reset in
  debug builds and explicitly documented as uninitialized in release —
  callers either explicit-init or read from a poisoned buffer that crashes
  loudly.
- **Phase 2 ships as 8 per-file PRs**, not one big change, each individually
  revertable. The pre-Phase-2 commit is tagged for one-flag rollback.
- **`MEM_DEV_BUILD` flag** downgrades fatal halts to printf+continue during
  iteration; CI explicitly rejects builds with the flag on.
- **CACHE eviction is non-recursive**: evictor returns "freed N bytes,"
  allocator retries. No `memFree` from inside `memAlloc`.
- **Per-type fixed pools vs. three regions**: explicit architectural
  decision section. Regions chosen because the constraint axis in jc_reborn
  is *lifetime*, not type.
- **Failure UX routes through the existing `JC_BSOD` framework**
  (`src/ps1_debug.h`) — the Win-3.1 blue screen with grep-friendly TTY
  sentinels (`JCBSOD-FATAL`, `JCBSOD <k>=<v>`, `JCBSOD-HALT`) and an
  on-screen diagnostic panel. Boot-time verifies (which run before
  graphics is up) use `fatalError`; everything during scene play uses
  `JC_BSOD`. Three new `bsod-test-mem-*` bootmodes verify each region's
  BSOD path visually.
- **Performance budget**: actual numbers, not adjectives. Boot impact,
  scene-transition cost, per-alloc cost all quantified with estimates.

## Core invariant

> Every `memAlloc(...)` call in jc_reborn returns a valid pointer. The
> allocator never returns NULL. If a region cannot satisfy a request, the
> program halts immediately via `fatalError` (boot-time, pre-graphics) or
> `JC_BSOD` (runtime, scene playing) — that is the *only* failure path.
> Allocation failures are bugs to fix in the plan, not conditions to
> handle in code.

Once the build's boot proof passes (see "Boot proof" below), every
allocation that game data implies is provably backed by sufficient region
space. The job of the plan is to make the halt paths unreachable in the
shipping configuration.

## Context

The PS1 build runs in 2 MB of main RAM and currently uses libc
`malloc`/`free` through a single wrapper (`safe_malloc`, `src/utils.c:123`).
39 call sites churn the heap per-scene, fragmenting it within a 1.1-1.2 MB
working set. The bandaid code that mitigates fragmentation
(`fgDropPressureCachesForCleanSnapshot`, `fgBackdropSaveCleanBgRectsWithPressureFallback`,
NULL-return guards in resource.c, three `JCSKIP` paths, silent scene skips
in ads.c, walk_pilot silent skips, graphics pool fall-back to non-pooled
allocation) papers over fragmentation but never closes it. v0.8.10 tried a
pinned-pool fix; it broke clean-rect scenes and was rolled back in v0.8.11.

This plan replaces the heap with a deterministic three-region allocator in
one 1.2 MB static buffer. With allocations guaranteed to succeed, every
skip/fallback/NULL-return site becomes dead code and is removed in Phase 2.

## Goals / non-goals

**Goals.** Allocations cannot fail. Existing LRU hit-rate preserved. Hot
path adds no measurable per-alloc overhead (target: ≤10 cycles per alloc,
better than libc's ~50-100). Reduce code size: ~600 lines of bandaid +
skip logic deleted. Region budgets compile-time-verifiable against actual
scene data, including holiday variants.

**Non-goals.** Not a generic malloc replacement — call sites declare a
region. Not a defragmenting allocator. Not a behavior change for scene
rendering or LRU eviction policy. Not a multi-threaded allocator.

## Architectural decision: regions vs. per-type pools

Pat's review asked: why three regions instead of N fixed-size per-type
pools? Both are valid PSX patterns. The decision criteria:

- **Pools** are right when the *type* of allocation is the dominant
  variable: each pool sized to N instances of one type, with no
  inter-pool interaction.
- **Regions** are right when the *lifetime* is the dominant variable:
  multiple types share a region because they share a lifetime, and
  reset/free semantics are bulk.

In jc_reborn:

- Per-scene allocations (entry table, sound events, setup buffer,
  clean-rect snapshot) are heterogeneous in type but homogeneous in
  lifetime: all live exactly one scene, all freed at the same boundary.
  Pool-per-type would mean five pools each carrying their own
  high-watermark and per-scene-reset logic.
- Resource cache entries (BMP/TTM/SCR/ADS uncompressedData) share an
  LRU policy but have variable sizes ranging from <1 KB (small TTM
  bytecode) to ~300 KB (SCR background). Pool-per-type doesn't work for
  variable-size items.
- Permanent allocations (frame buffers, walk buf, surface pool) are
  largely heterogeneous but share "boot-allocated, never freed."

So one region per lifetime model. The CACHE region carries a sub-allocator
internally because it really does need variable-size management — but
that sub-allocator is segregated free-list, which is just "many pools by
size class" inside CACHE. Pat's pattern is preserved where it matters.

## Concurrency model

`memAlloc`, `memFree`, and `memSceneReset` are **main-thread-only**. They
must never run from:

- VBlank / GPU DMA-completion interrupts
- Root counter callbacks
- CD-ROM ISR handlers
- Any PsyQ callback registered with `EnterCriticalSection`-guarded code

Enforcement:

- Each function asserts `assert(ps1IsMainContext())` in debug builds. The
  helper inspects PSX SR/cause registers; cheap.
- The audit gate (Phase 1 verification) traces from every known ISR
  registration point and confirms no path reaches `memAlloc`.
- The bandaids removed in Phase 2 include some code that *might* have
  allocated from interrupt context historically; the Phase 2 PR for each
  file double-checks that.

If a future feature genuinely needs interrupt-context allocation, it must
either (a) pre-allocate at boot in BOOT, or (b) use a separate lock-free
ring buffer specifically for that case. The allocator is not getting
locks.

## Memory budget — verified from artifacts

### PSX user-RAM math (from `build-ps1/jcreborn.map`)

| Region                   | Address range            | Size      |
|--------------------------|--------------------------|-----------|
| Exe + BSS (current)      | 0x80010000 .. 0x800ad4fc | 629 KB    |
| Region buffer (proposed) | 0x800ad500 .. ~0x801daf00 | ~1.20 MB |
| Free margin              | -                        | ~150 KB   |
| PsyQ padmgr + memcard pool (BIOS-managed) | ~0x801e0000 .. ~0x801ef000 | ~60 KB |
| Stack reserve            | top of RAM, growing down | 64 KB     |
| **Total**                | 0x80010000 .. 0x801FFFF0 | 1.92 MB   |

PsyQ's controller (padmgr) and memcard subsystems hold ~60 KB of internal
buffers allocated by the BIOS at fixed addresses. Verified by reading the
PsyQ headers and the linker map's `__bios*` symbols. They don't grow at
runtime; the budget table accounts for them explicitly.

A `_Static_assert(MEM_REGION_TOTAL <= 1228800)` (1.2 MB) gates the BSS
buffer size against the linker map; any change to BSS that pushes `_end`
also breaks this assert.

### Region splits (verified against `scene_analysis_output_2026-03-21.json`)

```
PS1 region buffer (1.2 MB)
+----------------------------------------------+ 0x00
| BOOT region (bump up, ~350 KB)               |
|   gFgFrameBuffer (pre-sized at boot)         |
|   gFgPrefetchFrameBuffer                     |
|   gFgStreamScratch                           |
|   gFgStreamWindowBuffer                      |
|   gWalkCleanBuf (149 KB)                     |
|   grBackgroundSfc backing (300 KB)           |
|   surface pool (graphics.c:1280)             |
|   resource struct arrays                     |
|   audio mixer state                          |
|   pause-menu state (lives across pause)      |
|   FROZEN after memFreezeBoot()               |
+----------------------------------------------+ bootTop
| CACHE region (segregated free-list, ~600 KB) |
|   resource->uncompressedData blobs           |
|   LRU eviction (non-recursive contract)      |
|   peak demand: MARY.ADS tag1 at 568 KB       |
+----------------------------------------------+ transientBottom
| TRANSIENT region (bump down, ~250 KB)        |
|   alloc order, FIRST to LAST:                |
|     1. clean-rect snapshot (≤ 181 KB)        |
|     2. setupSegmentBuffer (≤ 32 KB)          |
|     3. entry table, sound events (~10 KB)    |
|     4. scratch                               |
|   reset at fgRuntimeReset()                  |
|   poisoned 0xCD on reset in debug builds     |
+----------------------------------------------+ bufferEnd
```

### Boot proof (the part that makes "no failure" tenable)

Three checks run during `memInit`:

- **`memVerifyBootBudget()`** — sums all known BOOT-tagged allocations
  (from `pack_header_metrics.h` + fixed-size constants) and asserts the
  total fits in the BOOT budget. If a future PR adds a BOOT alloc beyond
  the budget, this fires at boot in CI, not in a user's living room.

- **`memVerifyAllScenesFitTransient()`** — iterates every scene (base +
  holiday variants) in `pack_header_metrics.h`, computes the worst-case
  TRANSIENT footprint (clean-rect estimate + setup segment +
  sound-events count × event size + entry-table size). `fatalError` with
  scene name + bytes if any scene exceeds the budget.

- **`memVerifyAllScenesPinnedFitCache()`** — for every scene (base +
  variant), sums the un-evictable pinned working set (one TTM + one BMP
  per concurrent thread, peak 20 threads in ACTIVITY scenes). `fatalError`
  if any scene's pinned set exceeds CACHE. Unpinned working set is
  evictable in-place by the LRU.

If any of the three fire at boot, the plan needs a budget adjustment
*before* the build ships, not a runtime workaround.

### Pack hash verification

`pack_header_metrics.h` includes a SHA-256 (or CRC-32, simpler on PSX) of
each pack's header bytes, recorded at the time the metrics were generated.
`memVerifyPackHashes()` reads each pack header at boot and confirms the
hash matches the recorded one. Mismatch ⇒ `fatalError("PACK_HASH_MISMATCH
scene=%s expected=%08x got=%08x — regenerate metrics or check disc")`.

This catches:
- Stale `pack_header_metrics.h` against rebuilt packs
- Stale packs against regenerated metrics
- Corrupted CD reads (under-detection of physical disc damage)

## Initialization semantics for TRANSIENT

The bump pointer reset is one word. The bytes underneath are stale (last
scene's data). This is a hazard the v2 panel identified because PC's libc
malloc and PS1's bump return different "uninitialized" content.

Resolution:

- **Release builds:** TRANSIENT memory is explicitly uninitialized. Every
  call site that allocates a buffer in TRANSIENT must either:
  - `memset` it explicitly if it depends on zero-init, or
  - Write all bytes before reading any.

  The 39-site audit annotates each call site with one of `INIT_ZEROED`,
  `INIT_FULL_WRITE`, or `INIT_NONE` (caller writes-before-read). Phase 1
  PRs enforce this via grep.

- **Debug builds (`MEM_POISON_TRANSIENT`):** `memSceneReset` fills the
  TRANSIENT region with `0xCDCDCDCD`. Any code that reads uninitialized
  TRANSIENT bytes gets a loud, recognizable pattern. Mismatched-init bugs
  surface immediately.

- **PC stub:** matches PS1 — `memAlloc(TRANSIENT, ...)` returns memory
  with `0xCDCDCDCD` in debug, undefined bytes in release. PC's libc malloc
  is not used as the backing for TRANSIENT bytes the caller might read.

## API design

```c
typedef enum {
    MEM_REGION_BOOT,        /* bump up; freezes after boot */
    MEM_REGION_CACHE,       /* LRU; eviction allowed */
    MEM_REGION_TRANSIENT,   /* bump down; wipes between scenes */
} MemRegion;

/* All allocations aligned to MEM_REGION_ALIGN (4 bytes — sufficient for
 * R3000A; the largest scalar we use is uint32 / pointer). Returns a
 * valid pointer or fatalErrors. Never NULL. */
#define MEM_REGION_ALIGN 4
void *memAlloc(MemRegion region, size_t size, const char *tag);

/* Free is a no-op in BOOT (debug: fatalError post-freeze), real release
 * in CACHE (used only by the LRU evictor), balance-decrement in TRANSIENT. */
void memFree(MemRegion region, void *ptr);

/* Wipes TRANSIENT region. Logs peak. Asserts sceneAllocBalance==0 in debug.
 * Poisons TRANSIENT bytes in MEM_POISON_TRANSIENT builds. */
void memSceneReset(const char *sceneName);

/* Diagnostics — zero overhead on hot path */
size_t memRegionUsed(MemRegion);
size_t memRegionPeak(MemRegion);
void   memLogTelemetry(void);  /* gated behind FG_HEAP_PROBE_LOGS */
/* Region state read by ps1Bsod via externs (same pattern as
 * fgProbeLargestAlloc); no dedicated dump function needed. */

/* Boot. memAlloc before this calls fatalError. */
void memInit(void);
void memFreezeBoot(void);

/* Development-only relaxation. Defined out in CI/release builds; CI
 * verifies MEM_DEV_BUILD == 0. */
#ifdef MEM_DEV_BUILD
void memDevAllowFailureWithSkip(int enable);
#endif
```

### `MEM_DEV_BUILD` flag (resolves M2)

During iteration on a new scene, a developer may want the game to keep
running when TRANSIENT overruns. `MEM_DEV_BUILD` compiles in a per-call
"if alloc fails, log + skip" path that mimics the old JCSKIP behavior.
The flag is forbidden in release builds and in CI by an explicit
`#if MEM_DEV_BUILD && !defined(JC_ALLOW_DEV_BUILD)` guard plus a CI
check (`grep MEM_DEV_BUILD docs/ps1/build-config-release.txt` must show
`MEM_DEV_BUILD=0`).

Result: developers can iterate; the shipping build still has the no-fail
invariant.

### Boot sequence

```c
int main(...) {
    memInit();                                  /* regions ready */
    /* pack_header_metrics.h is compile-time data — no CD reads here */
    memVerifyBootBudget();
    memVerifyAllScenesFitTransient();
    memVerifyAllScenesPinnedFitCache();
    memVerifyPackHashes();                      /* runtime CD verify */
    /* BOOT allocations follow */
    audioInit();
    resourceCatalogParse();
    fontInit();
    surfacePoolInit();
    walkPilotInit();
    pauseMenuInit();
    memFreezeBoot();
    runMainSceneLoop();
}
```

If any verify fires, the halt happens before graphics is up, so the
`fatalError` TTY-print + `while(1)` path runs (see "Failure UX" below).

### `sceneAllocBalance`

Increments on `memAlloc(TRANSIENT, ...)`, decrements on
`memFree(TRANSIENT, ...)`. `memSceneReset` asserts == 0 in debug. Compiled
out in release.

### CACHE failure contract (non-recursive — resolves A6)

```
memAlloc(CACHE, n, tag):
    p = freelist_alloc(n)
    if p != NULL:
        return p
    freed = lruEvictUnpinned(n)        /* returns bytes freed; does NOT call memFree */
    if freed >= n:
        p = freelist_alloc(n)
        if p != NULL:
            return p
    JC_BSOD(currentScene, formatReason("CACHE exhausted req=%zu pinned=%zu", n, lruPinnedBytes()))
```

`lruEvictUnpinned` walks the LRU list, removes the chosen victim,
updates the free-list directly (not via `memFree`), accumulates freed
bytes. No re-entrancy.

`memVerifyAllScenesPinnedFitCache()` at boot guarantees the
`fatalError` branch is unreachable in the shipping configuration.

## Failure UX — routes through the existing `JC_BSOD` framework (resolves S2)

The codebase already has a Windows-3.1-flavoured blue-screen mechanism for
exactly this purpose:

- `src/ps1_debug.h` declares `JC_BSOD(scene, reason)` (captures
  `__FILE__`/`__LINE__` automatically) backed by
  `ps1Bsod(scene, reason, file, line)`.
- The implementation at `src/ps1_debug.c:228` already emits grep-friendly
  TTY sentinels (`JCBSOD-FATAL`, `JCBSOD <k>=<v>` detail lines,
  `JCBSOD-HALT`) and renders a full-screen blue panel with white BIOS-font
  text including scene name, reason, file:line, heap probe, frame-buffer
  state, walk-buffer state. Halts forever.
- The header comment is explicit: *"NEVER fires in production; while we're
  testing a deterministic build any failure here is a code or data bug we
  want surfaced immediately rather than papered over."* That is exactly
  this plan's invariant.
- A `bsod-test` bootmode flag already exists for visually verifying the UI
  without forcing a real failure (`src/jc_reborn.c:1020-1023, 1979-1982`).

The plan reuses this framework rather than inventing a new one. Two
phase-dependent code paths:

### Boot-time failures: `fatalError` (graphics not yet up)

The four `memVerify*` checks at `memInit` time run before `graphicsInit`
finishes. `ps1Bsod` is not safe to call before graphics is initialized
(`src/graphics_ps1.c:3526` notes this). Boot-time verifies use
`fatalError(...)` with the same human-readable format. On PS1 this
prints to TTY and halts via `while(1)`; on emulator/dev hardware the
TTY console captures it, on retail hardware the user sees a frozen
screen but the reason is in the build's QA logs.

```c
fatalError("TRANSIENT region budget too small: scene=%s needs=%zu have=%zu — "
           "rebuild pack_header_metrics.h or raise MEM_TRANSIENT_BUDGET",
           pack->name, scenePeak, MEM_TRANSIENT_BUDGET);
```

### Runtime failures (scene playing): `JC_BSOD`

Every allocator failure during scene play routes through `JC_BSOD`:

```c
JC_BSOD(currentScene,
        "TRANSIENT exhausted: req=234K have=187K bal=0 peak=234K");
```

`ps1Bsod` already grabs heap-probe state via externs
(`fgProbeLargestAlloc`, `fgGetFrameBufferBytes`, etc.). The plan extends
the existing detail-line block in `ps1Bsod` to also emit region state:

```
JCBSOD memBootUsed=345600 memBootPeak=345600
JCBSOD memCacheUsed=423000 memCachePeak=580000
JCBSOD memTransientUsed=0    memTransientPeak=234567
JCBSOD sceneAllocBalance=0
```

This is a 5-line addition to `src/ps1_debug.c:228+` mirroring the
existing extern pattern (no new infrastructure). The on-screen panel
inherits the new lines automatically.

### Allocator failure call sites — exact call template

```c
/* Boot-time (memVerify*, memInit): use fatalError, graphics not up */
fatalError("BOOT region exhausted at init: req=%zu budget=%zu — "
           "audit BOOT allocations or raise MEM_BOOT_BUDGET",
           required, MEM_BOOT_BUDGET);

/* Runtime (scene playing): use JC_BSOD */
JC_BSOD(currentScene, formatReason("CACHE exhausted req=%zu pinned=%zu", n, pinned));
```

`formatReason` writes into a small static buffer (`mem_region.c` owns
it); not re-entrant, but the path is no-return anyway so re-entrancy
doesn't apply.

### Synthetic-failure test bootmode

The existing `bsod-test` bootmode synthesizes one fatal after the first
scene. The plan adds three sibling bootmodes for visually verifying each
region's BSOD path:

- `bsod-test-mem-boot` — synthesize BOOT exhaustion during init
- `bsod-test-mem-cache` — synthesize CACHE exhaustion mid-scene
- `bsod-test-mem-transient` — synthesize TRANSIENT exhaustion mid-scene

Each fires a known `JC_BSOD` with a synthetic reason string so the
on-screen panel can be photographed and approved during QA.

## Pause-during-scene state (resolves A4)

The pause menu pre-allocates its state in BOOT at startup (`pauseMenuInit`,
new function). No TRANSIENT allocations during pause. When the user
pauses, the in-progress scene's TRANSIENT region is unchanged; un-pausing
resumes against the same TRANSIENT contents. `sceneAllocBalance` is
preserved across pause.

This requires a small refactor of `pause_menu.c` to allocate its working
buffers eagerly at boot rather than lazily on first pause. Tracked in the
Phase 3 audit table.

## Source of truth for scene data

Renamed from `scene_analysis_output_2026-03-21.json` (date-suffixed) to
`scene_analysis_current.json` (stable name; the dated artifact remains
as historical reference). A pre-commit hook checks that
`scene_analysis_current.json` matches a regenerate command's output for
the current packs; mismatch fails the commit.

`pack_header_metrics.h` is generated from
`scene_analysis_current.json` *and* a direct read of all FG2 pack files.
The generator enumerates every base scene and every holiday variant
(`fgLoopApplyVariant` reachability). The header contains:

```c
struct PackHeaderMetric {
    const char *packName;
    uint32 maxFrameBytes;
    uint32 maxPrefetchBytes;
    uint32 maxStreamWindowBytes;
    uint32 maxStreamScratchBytes;
    uint32 transientWorstCase;   /* clean-rect + setup + others */
    uint32 cachePinnedWorstCase; /* pinned set this scene needs */
    uint32 headerCrc;            /* of pack's header bytes */
};
extern const struct PackHeaderMetric kPackHeaderMetrics[];
extern const size_t kPackHeaderMetricsCount;
```

Result: zero CD seeks at boot for buffer sizing. Variant packs are
covered. Hash verification catches drift.

## Bandaid + skip-code removal manifest

Phase 2 deletes every site below. Verified by a CI grep gate:
`grep -rE "JCSKIP|Caller handles gracefully|skip scene silently|skip scene gracefully|Graceful skip|Pool exhausted - fall back|failed silently|allocation failure" src/`
must return zero hits after Phase 2.

Replacement halt mechanism is **`JC_BSOD(scene, reason)`** for all
runtime sites (scene is playing, graphics is up) and `fatalError(...)`
for the few sites reachable during init only.

| # | File:line                                   | Removal                                                    |
|---|---------------------------------------------|------------------------------------------------------------|
| 1 | `foreground_pilot.c:3787` `JCSKIP pack-start-failed`           | Delete; JC_BSOD                                            |
| 2 | `foreground_pilot.c:3808` `JCSKIP draw-bounds-failed`          | Delete; JC_BSOD                                            |
| 3 | `foreground_pilot.c:3847` `JCSKIP clean-rect-alloc-failed`     | Delete; clean-rect is TRANSIENT-reserved (unreachable)     |
| 4 | `foreground_pilot.c:3782-3786` "Graceful skip instead of BSOD" | Delete entirely (the BSOD it talks about is what we want)  |
| 5 | `foreground_pilot.c:fgDropOptionalPrefetchBuffersForCleanSnapshot` + 4 callers | Delete function and callers              |
| 6 | `foreground_pilot.c:fgDropPressureCachesForCleanSnapshot`      | Delete                                                     |
| 7 | `foreground_pilot.c:fgBackdropSaveCleanBgRectsWithPressureFallback` | Replace with plain `fgBackdropSaveCleanBgRects`        |
| 8 | `foreground_pilot.c:1471,1482` JCSTREAM prealloc-failed prints | Delete; runtime unreachable (boot pre-allocs in BOOT)      |
| 9 | `foreground_pilot.c:fgPrePrimeStreamBuffers` lazy/eager paths  | Delete; boot allocations are deterministic                 |
|10 | `resource.c:700-704` (`findAdsResource` NULL on PS1)           | JC_BSOD on both PS1 and PC                                 |
|11 | `resource.c:715-722` (`findBmpResource`)                       | JC_BSOD on both                                            |
|12 | `resource.c:732-737` (`findScrResource`)                       | JC_BSOD on both                                            |
|13 | `resource.c:747-752` (`findTtmResource`)                       | JC_BSOD on both                                            |
|14 | `ads.c:1696` `if (adsResource == NULL) return;`                | Delete (findAds JC_BSODs)                                  |
|15 | `ads.c:1708` ps1_loadAdsData silent return                     | JC_BSOD                                                    |
|16 | `ads.c:1753+` "skip this scene gracefully" TTM block           | JC_BSOD                                                    |
|17 | `walk_pilot.c:108-117` walkClean buf silent skip               | Delete; buf is BOOT-allocated                              |
|18 | `walk_pilot.c:165-176` walkPilotInit alloc-fail soft return    | fatalError (init time)                                     |
|19 | `walk_pilot.c:255-260` JOHNWALK silent-bail-out                | JC_BSOD                                                    |
|20 | `graphics.c:1370-1395` "Pool exhausted - fall back"            | JC_BSOD; pool sized correctly at boot                      |
|21 | `cdrom_ps1.c:644,885,2354` malloc-fail NULL returns            | JC_BSOD (runtime) / fatalError (boot, depending on caller) |
|22 | `ps1_perf.c:1032` + 6 callers `ps1PerfMarkAllocFail` + counter | Delete                                                     |
|23 | `ps1_perf.c:1058,1064` `ps1PerfMarkFallback` family            | Delete (Phase 3 audit confirms callers also vanish)        |

**Not removed** (intentional, not failure handling):

- `story.c:493,644` — dead-scene picks, intro-override.
- `ps1_captions.c:597` — empty-caption skip ("this scene has no caption").
- `scene_picker.c:347` — "screensaver should never go dark" *policy* choice;
  refresh comment to clarify.
- `ps1_features.c:60` — `ps1SkipToNextScene` user input.
- `pause_menu.c:1830` — user pressing Circle.
- Sierra-faithful walk-aware filtering in `scene_picker.c:432,453,462`.
- `foreground_pilot.c:3830` "JCMEM black-clean ... skip-clean-rects" —
  positive code path; rename print to "JCMEM black-backdrop-mode".

## Implementation phases

### Phase 1 — Allocator + migration (one ship-able milestone, ~3 weeks)

1. New files: `src/mem_region.{c,h}`, `src/mem_region_verify.c`,
   `src/generated/pack_header_metrics.h` (generator script
   `scripts/generate-pack-metrics.py`).
2. PC + PS1 implementations with identical API and budget enforcement.
3. CACHE segregated free-list with non-recursive eviction.
4. 4-byte alignment (`MEM_REGION_ALIGN = 4`).
5. Boot integration including `memVerifyPackHashes`.
6. Migrate all 39 call sites to `memAlloc(REGION, n, tag)` with explicit
   `INIT_ZEROED`/`INIT_FULL_WRITE`/`INIT_NONE` annotation grep'd by CI.
7. Hook `memSceneReset` into `fgRuntimeReset` at
   `src/foreground_pilot.c:1470`.
8. Reorder `foregroundPilotRuntimeStart` so clean-rect allocates first.
9. Adopt project-level malloc poison via `src/malloc_poison.h` — every
   .c file in src/ includes it, libc malloc/free become compile errors
   except in `mem_region.c` and a whitelist comment-justified in each
   exception.
10. Audit ISR registration points; assert no `memAlloc` reachable from
    ISR.
11. CI checks: `MEM_DEV_BUILD=0`, grep gates pass, pack hashes current.

**Estimated effort:** ~3 weeks focused work. Roughly 800-1000 LOC of
allocator + ~200 LOC of generator + 39 call-site touches + audit.

**Validation:** game renders byte-for-byte identically across the
63-scene rotation. Both builds green. All four boot-time `memVerify*`
checks pass.

### Phase 2 — Delete bandaid + skip code (8 small PRs, ~1 week)

The 23-site removal manifest is split into 8 per-file PRs, each
individually revertable:

- P2.1 — `foreground_pilot.c` (items 1-9)
- P2.2 — `resource.c` (items 10-13)
- P2.3 — `ads.c` (items 14-16)
- P2.4 — `walk_pilot.c` (items 17-19)
- P2.5 — `graphics.c` (item 20)
- P2.6 — `cdrom_ps1.c` (item 21)
- P2.7 — `ps1_perf.c` (items 22-23)
- P2.8 — comment refresh for the "intentional skip" sites

The pre-Phase-2 commit is tagged `pre-mem-region-bandaid-removal` so
rolling back the whole set is one-flag.

**Validation:** soak test 20+ iterations. Zero `fatalError` and zero
`JC_BSOD` triggers across the run. Per-scene frame-byte diff vs.
captured baseline matches.

### Phase 3 — Audit remaining allocation surfaces (~1 week)

Sites that weren't in the 39 because they're indirect or in less-traveled
code. The full-src/ audit gate is:
`grep -rE "\bmalloc\b|\bfree\b|\bcalloc\b|\brealloc\b" src/`
returns hits only in `mem_region.c`, the LRU evictor, and the explicit
whitelist (third-party-integrated code annotated in `malloc_poison.h`).

| Site                                          | Region   | Notes                                          |
|-----------------------------------------------|----------|------------------------------------------------|
| `graphics.c:1808` scrResource uncompressedData | CACHE    | Lazy at first use                              |
| `graphics.c:1884` grBackgroundSfc (640×480)    | BOOT     | 300 KB one-shot                                |
| `graphics.c:1935` bmpResource uncompressedData | CACHE    | Lazy                                           |
| `graphics.c:1835,1963` BMP→surface scratch     | TRANSIENT| Conversion scratch                             |
| `ads.c` per-ADS uncompressedData              | CACHE    | First-play decompress                          |
| `pause_menu.c` state buffers                  | BOOT     | Eager at `pauseMenuInit`                       |
| `ps1_captions.c`                              | BOOT     | If long-lived; TRANSIENT if per-scene          |
| TTM opcode handlers in `ttm.c`                | varies   | Recursive audit — any `safe_malloc` here?      |

**Validation:** full-src/ audit gate stays clean.

### Total schedule

3 weeks (Phase 1) + 1 week (Phase 2) + 1 week (Phase 3) = **~5 weeks**
focused engineering. The plan is honest with stakeholders that this is
weeks, not days. Phases 1 → 2 → 3 are sequential (the panel's S4 point —
Phase 1's verify functions need Phase 2's migration before they're
provably correct).

## Performance budget

| Operation                        | Cost (PS1)                  | Notes                                              |
|----------------------------------|-----------------------------|----------------------------------------------------|
| `memAlloc(BOOT/TRANSIENT, n)`    | ~10 cycles                  | Bump pointer + balance increment + tag store       |
| `memAlloc(CACHE, n)` hit        | ~25 cycles                  | Free-list head pop + class lookup                  |
| `memAlloc(CACHE, n)` miss + evict| ~5-10 ms                    | LRU scan over ~200 entries (frame-killer if mid-scene); pre-empted at scene-reset, see below |
| `memFree(BOOT)`                  | 0 cycles release            | No-op                                              |
| `memFree(TRANSIENT)`             | ~5 cycles release           | Balance decrement                                  |
| `memFree(CACHE)`                 | ~30 cycles                  | Free-list push + class lookup                      |
| `memSceneReset` release          | ~5 cycles                   | Bump pointer reset + telemetry conditional        |
| `memSceneReset` debug-poison     | ~8 ms                       | 250 KB 0xCD fill; debug-only                       |
| Boot: `memVerify*` (4 funcs)     | ~1 ms total                 | All compile-time data + 63 hash verifies           |
| Boot: BSS clear of 1.2 MB buffer | ~75 ms                      | PsyQ `_start` zeroes BSS                           |
| **Boot delta total**             | **~75 ms**                  | Bounded; CD-seek-free                              |

Compared to libc malloc on PsyQ (~50-100 cycles per call), the hot path
is a 5-10× win on BOOT/TRANSIENT and roughly break-even on CACHE.

Pre-emptive CACHE eviction at scene transitions (resolves PR5):
`memSceneReset` looks at projected next-scene CACHE demand (from
`pack_header_metrics.h`) and pre-evicts unpinned resources during the
already-paused scene transition. Mid-scene CACHE eviction becomes the
unhappy path and should rarely fire — when it does, it's measured.

## Documentation deliverables

- **`docs/ps1/adding-new-scenes-memory.md`** (companion) — how to add a
  new scene or holiday variant: regenerate `pack_header_metrics.h`, run
  `memVerify*` locally, what each region is for, when to set `MEM_DEV_BUILD`,
  troubleshooting the most common boot-fatalError messages.
- **`docs/ps1/mem-region-decision-tree.md`** — quick reference for "I
  need to allocate X — which region?". Decision tree:
  ```
  Is it allocated once at boot and never freed?  → BOOT
  Is it a resource blob (BMP/TTM/SCR/ADS)?       → CACHE
  Does it live exactly one scene?                → TRANSIENT
  Otherwise: ask in #jc-reborn; one of those is wrong.
  ```
- Code-level header comment in `mem_region.h` documenting every API
  with one example call.

## Verification

Pre-implementation gates (must close before Phase 1):

- [ ] Rebuild PS1 binary, re-read `_end` from fresh map. Confirm
      exe+BSS ≈ 629 KB.
- [ ] Confirm `scene_analysis_current.json` `peak_memory_bytes` is the
      full CACHE footprint per scene (BMP + TTM + SCR), not just BMP.
- [ ] Confirm `grBackgroundSfc` (graphics.c:1884) is one-shot.
- [ ] Audit ISR registration points; confirm no `memAlloc` reachable.
- [ ] Audit `ttm.c` opcode handlers for hidden allocations.
- [ ] BIOS / PsyQ allocation audit: confirm padmgr + memcard footprint
      ≈ 60 KB, no runtime growth.

End-to-end gates (before merge):

1. Build green on PC + PS1. All `_Static_assert`s pass.
2. All four `memVerify*` checks pass at boot on PC + PS1 for every
   scene including holiday variants.
3. Long-run soak: 20+ iterations of 63-scene rotation. Existing band
   counts (117 green / 9 yellow / 0 orange / 0 red) do not regress.
4. Heap-probe stability: `fgProbeLargestAlloc()` is constant
   across the rotation (essentially tautological — kept as sanity).
5. Removal grep gate: zero hits.
6. Full-src/ audit gate: zero hits outside whitelisted files.
7. Zero `fatalError` and zero `JC_BSOD` triggers across soak. Tail of
   PS1 TTY log contains no `JCBSOD-FATAL` sentinel.
8. `sceneAllocBalance` never positive at reset (debug).
9. `memFreezeBoot` not triggered post-boot.
10. PC valgrind clean (one static region buffer "still reachable" OK).
11. Per-scene frame-byte diff vs. captured baseline matches across 63
    scenes.
12. DCache miss-rate measurement before/after (Pat's P6) — informational,
    not a gate, but recorded in the merge PR.
13. CI rejects `MEM_DEV_BUILD=1` builds.

### Semantics gotcha (kept visible)

`fgRuntimeReset()` runs at the *start* of the next scene's setup. The
JCMEM "wipe=NNN" reading for scene N appears at the start of scene N+1's
log. The JCSKIP-style cleanup path at `foreground_pilot.c:3777` no
longer exists after Phase 2; the next scene's `fgRuntimeReset` is the
only reset.

## Risks accepted

These items the v2 panel raised that are noted but not actively mitigated:

- **P7 LOC estimate optimistic.** Reflected in schedule (5 weeks not days).
- **P6 DCache thrashing.** Measured but not optimized for; if regression
  appears, follow-up with allocation-order tuning.
- **M5 Third-party library integration.** Whitelist mechanism in
  `malloc_poison.h` is the answer; specific integrations evaluated
  case-by-case.
- **S5 Frame-byte diff is per-scene, not per-frame within a scene.** The
  baseline captures key frames, not every frame. Subtle frame-timing
  regressions could escape; mitigated by the existing JCPERF band
  system continuing to run alongside.
