# Memory region allocator — implementation plan

**Status:** v5 — resolves every finding from red-team v3 (the panel
re-review of v4). See:

- [memory-region-allocator-red-team.md](./memory-region-allocator-red-team.md) — v1 technical red-team
- [memory-region-allocator-red-team-v2.md](./memory-region-allocator-red-team-v2.md) — v2 principal-engineer multi-reviewer panel
- [memory-region-allocator-red-team-v3.md](./memory-region-allocator-red-team-v3.md) — v3 panel re-review of v4
- [adding-new-scenes-memory.md](./adding-new-scenes-memory.md) — future-maintainer guide
- [mem-region-decision-tree.md](./mem-region-decision-tree.md) — which region for a new allocation

## What changed in v8

Resolves all 3 material + 5 hygiene items from red-team v6. Last
clean-up pass; v8 expected to converge to zero findings.

- **`ps1IsMainContext()` bit-pattern verification + ISR unit test**
  added as Phase 1 sub-task. Test fires the helper from inside a
  synthetic VBlank ISR (using PsyQ's `EnterCriticalSection`/
  `ExitCriticalSection` to enter/exit) and asserts the helper returns
  0 there, 1 from main flow. (P20)
- **`mem_region_extern.h` uses incomplete-enum forward declaration**
  for `MemRegion`. C99/C11 permits this for function declarations.
  `mem_region_extern.h` has `enum MemRegion;` (no body); callers that
  need the enum constants `#include "mem_region.h"` directly. (A26)
- **`fgLoopGetLastScene` adds a `pickInProgress` flag** to the
  two-slot state. State machine: idle → picking → playing → idle.
  Phase 1 step 19 spells out all four state transitions. (A27)
- **`__builtin_unreachable()` dropped from both `memHalt` branches**
  — both `ps1Bsod` and `while(1)` are non-returning; the annotation
  is redundant in both places. (P21)
- **`nm`-based fallback for `-Wglobal-constructors` removed** — MIPS
  toolchains emit constructors as `.init_array` not as named symbols;
  the grep would never find them. PsnoobSDK GCC 12+ is the floor;
  no fallback needed. (P22)
- **Python script CI fixtures:** Phase 1 includes `scripts/test-mem-region-rationale/`
  with 5 fixture files (valid, missing-comment, far-comment, wrapping-
  macro, multi-line) and a `make test-mem-rationale` target. (S17)
- **"No conditional region" clarified** in the decision tree:
  prohibition is on *runtime* conditional region choice (`condition ?
  BOOT : TRANSIENT`); build-time `#ifdef PS1_BUILD` branches that
  yield distinct textual call sites per platform are fine — each
  branch gets its own RATIONALE comment. (M17)
- **Pin-count delta sum cost noted** in perf table (~600 cycles per
  scene transition, debug-only). (A28)

## What changed in v7

Resolves all 2 blockers + 7 material + 3 hygiene items from red-team v5:

- **`fgLoopGetLastScene` returns target scene during pick/setup**,
  not just the last played one. Two-slot state: `lastTarget` (updated
  at pick time, before pre-evict can fire) and `lastPlayed` (updated
  after successful play). `fgLoopGetLastScene()` returns the target
  if a pick is in progress, else the played slug. BSOD now blames the
  scene that was *being* set up, not the previous one. (A22)
- **`fgLoopApplyVariant` returns the effective scene name** —
  signature changes from `void fgLoopApplyVariant(const char *)` to
  `const char *fgLoopApplyVariant(const char *)`. Caller passes the
  returned name to `memCachePreEvictForNextScene` so the metrics
  lookup keys on the variant pack, not the base. (PR14)
- **`ps1IsMainContext()` implementation specified** — `mem_region.c`
  defines it inline using PSX COP0 reads:
  ```c
  static inline int ps1IsMainContext(void) {
      uint32 cause, sr;
      __asm__("mfc0 %0, $13" : "=r"(cause));  /* CAUSE */
      __asm__("mfc0 %0, $12" : "=r"(sr));     /* SR */
      /* No active exception (ExcCode == 0) AND interrupts enabled
       * (SR.IEc == 1). True if neither holds when we're not in an
       * ISR / exception handler. */
      return ((cause & 0x7C) == 0) && (sr & 0x1);
  }
  ```
  Bit patterns verified against the PSX programmer's manual. (P17)
- **PsyQ toolchain compatibility for `-Wglobal-constructors`**
  verified — PsnoobSDK's GCC 12+ supports it. Fallback if a future
  toolchain doesn't: `nm jcreborn.elf | grep -E "_init|__init"`
  returns no entries in CI. (P18)
- **`MEM_REGION_RATIONALE` enforcement uses a Python script**, not
  bash grep. Handles macros, multi-line `memAlloc` calls, same-line
  comments, and macro wrappers (forbidden — see below). Script:
  `scripts/check-mem-region-rationale.py`, ~30 LOC. (S14)
- **Pre-Phase-1 gates re-categorized:** items requiring Phase 1
  artifacts moved to "Phase 1 milestones" section (pinned-set
  math match, `-Werror=global-constructors` clean build). (S15)
- **Holiday-variant decision tree split** into three lifetime cases
  (resource blob → CACHE, per-scene scratch → TRANSIENT, long-lived
  state → BOOT with a "design-smell" caveat). (M15)
- **`MEM_REQUIRE` on `memFree` documented** — kept for symmetry;
  the ~10-cycle overhead on a no-op `memFree(TRANSIENT)` is
  negligible; documented in the perf table footnote. (PR15)
- **`src/mem_region/mem_region_extern.h` is the single source of truth for
  forward decls** consumed by both `mem_region.c` (definition side)
  and `ps1_debug.c` (caller side). Eliminates drift. (A23)
- **Macros wrapping `memAlloc` are prohibited** — documented in the
  decision tree and enforced by the Python script (any macro whose
  expansion contains `memAlloc(` fails CI). (A24)
- **`__builtin_unreachable()` after `while(1)` dropped** — GCC infers
  non-returning, the annotation was redundant. (P19)
- **Writing-burden of `MEM_REGION_RATIONALE` documented** in the
  decision tree and contributing guide. (M16)
- **Pin-count delta logging** at scene transitions in debug builds —
  if `pinResource` and `unpinResource` calls don't balance between
  scenes, a debug-only `JCMEM pin-delta scene=X delta=N` line surfaces
  the leak before pre-evict has a chance to misreport it. (A25)

## What changed in v6

Resolves all 3 blockers + 11 material + 4 hygiene items from red-team v4:

- **`MEM_REQUIRE(cond)` macro** ships in release (not C's `assert()`,
  which compiles out under NDEBUG). All ISR-safety and invariant
  checks use it. (P13)
- **`ps1DebugInit()` runs first** in the boot sequence, before
  `memInit()`. Already true in `src/jc_reborn.c:1688`; plan's
  pseudocode now reflects reality so the dependency is explicit. (A17)
- **`fgLoopGetLastScene()` is a new Phase 1 deliverable** — does not
  exist today. Returns the slug of the most recently played scene for
  diagnostic continuity through scene-pick. Confirmed via grep. (A18)
- **MIPS COP0 read latency corrected** in perf table (8-12 cycles, not
  3). Bump/CACHE alloc costs revised accordingly. (P14)
- **CRC-32 lookup table** lives in `.rodata` (compile-time const data
  in exe load, not BSS); pushes `_end` up by ~1 KB but doesn't eat
  region budget. (P15)
- **Phase 2 ships as TWO PRs**, not one bundled PR: infrastructure
  (P2.0 + memHalt + ps1Bsod replace) lands first, cleanup PR
  (P2.1-2.8) lands after the infrastructure is green. Avoids
  per-commit CI breaking on intermediate states. (S11)
- **Decision tree enforcement** via mandatory `MEM_REGION_RATIONALE:`
  comment on each `memAlloc` call site, grep'd by CI. (S12)
- **Phase 1 implementation checklist** lands as
  `docs/ps1/mem-region-phase-1-checklist.md` — 18 ticked TODOs with
  per-step completion criteria. (M12)
- **Decision tree mentions holiday variants** explicitly. (M13)
- **Pre-evict moves out of `fgLoopNextScene`** to caller site at
  `jc_reborn.c:~1956`, runs *after* `fgLoopApplyVariant` so the
  effective scene name is used. (PR11, PR12)
- **CRC pack-hash verification gated by `JC_VERIFY_PACK_HASHES`** —
  on in dev/QA builds, off in release. Release skips the 9-second
  boot cost and trusts offline metrics. (PR13)
- **ps1_debug.c uses forward declarations** for region-state externs,
  not `#include "mem_region.h"` — eliminates circular-include risk. (A19)
- **`memSafeRead` scope documented** — defends against data
  corruption, not instruction corruption (which is out of scope). (A20)
- **`__builtin_unreachable()` after each memHalt branch** for compiler
  hygiene. (P16)
- **Tag-mechanics spelled out:** `git tag pre-mem-region-bandaid-removal`
  applied to the Phase-1 merge commit before the cleanup PR opens. (S13)
- **CI count match** — assert
  `grep -c "MEM_REGION_" mem-region-decision-tree.md ==
  grep -c "MEM_REGION_" src/mem_region/mem_region.h`. (M14)
- **Pre-Phase-1 printf-non-allocating gate dropped** — the project-
  level malloc poison makes this a link-time check rather than a
  manual audit; redundant. (A21)

## What changed in v5

Resolves all 7 🔴 blockers, 12 🟠 material risks, and 5 🟡 hygiene items
from red-team v3:

- **Unified `memHalt(scene, reason)` primitive** in `mem_region.h` —
  internally dispatches to `JC_BSOD` (if `graphicsIsInitialized()`) or
  `fatalError` (if not). All allocator failure sites call this. No call
  site decides the mechanism. (P9, M7)
- **ISR-safety check is unconditional**, not debug-only. `assert(ps1IsMainContext())`
  costs ~3 cycles per `memAlloc`, well under the 10-cycle budget. (P1-bis)
- **Phase 1 Step: explicit ps1Bsod block REPLACEMENT** — the `fgProbeLargestAlloc`,
  `fgGetFrameBufferBytes`, etc. lines in `src/platform/ps1/ps1_debug.c:245+` are
  replaced (not extended) with mem-region state reads. `walkClean*` /
  `johnwalkSlot*` lines stay because those buffers persist into BOOT. (P8, A11)
- **`fatalError` PS1 path upgraded** to use `ps1DebugError` (from
  `src/platform/ps1/ps1_debug.h:51`) which renders a minimal text panel. Pre-graphics
  failures are no longer black-screen-no-explanation. (A10)
- **Phase 2 ships as ONE bundled PR** with 8 atomic per-file commits.
  Revert any single one with `git revert <sha>`. (S7)
- **Pre-emptive CACHE eviction moves into the scene picker**
  (`fgLoopNextScene`) rather than `memSceneReset`. Eviction happens
  during walk-to-scene, not at the start of the next scene's setup. (PR7)
- **`bsod-test-mem-*` renamed to `bsod-ui-test-mem-*`** to signal these
  are UI-only synthetic tests, not integration tests. (S9)
- **Decision tree doc** committed in Phase 1, not after. (M9)
- **CRC-32 chosen over SHA-256** for pack-hash verification (smaller,
  faster, sufficient). (P12)
- **`formatReason` pinned to `ps1_debug.h`** with documented re-entrancy
  contract. (M10, A16)
- **Pinned-set verifier math equals runtime accounting** — new
  pre-Phase-1 verification gate runs both against the same scene set
  and asserts equality. (A12)
- **Static initializers explicitly forbidden** — build flag rejects them. (A13)
- **`currentScene` convention between scenes:** call sites use
  `fgLoopGetLastScene()` to keep diagnostic continuity through the
  scene-pick window. (A14)
- **`bsod-ui-test-mem-*` flags gated by CI default-off check.** (A15)
- **TRANSIENT poison cost corrected** to ~15 ms (was 8 ms). (PR8)
- **BSOD-path defensive value clamping** added — if metadata is
  corrupted, the dump renders bounded values instead of crashing
  during the halt. (PR9)
- **Tag-string-not-stored convention pinned** in the API docs. (PR10)
- **PSX printf is non-allocating** — confirmed against project's
  PsnoobSDK, documented in `mem_region.c` as a non-regression invariant. (P10)
- **`JCBSOD-FATAL` test-harness migration** noted as part of Phase 1
  delivery so soak-run pass/fail detection updates with the new
  sentinel semantics. (S8)
- **PR template checkbox** for `MEM_DEV_BUILD=0` final-test confirmation. (S10)

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
  (`src/platform/ps1/ps1_debug.h`) — the Win-3.1 blue screen with grep-friendly TTY
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
`malloc`/`free` through a single wrapper (`safe_malloc`, `src/core/utils.c:123`).
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

- Each function calls **`MEM_REQUIRE(ps1IsMainContext())`** — a
  hand-rolled macro defined in `mem_region.h` that ships in release
  builds (does NOT use C's `<assert.h>`, which compiles out under
  NDEBUG). Definition:

  ```c
  #define MEM_REQUIRE(cond) do {                                  \
      if (!(cond)) memHalt("(mem_require)", "invariant: " #cond); \
  } while (0)
  ```

  The helper inspects PSX SR/cause registers via `mfc0` — ~8-12 cycles
  including the COP0 load-delay stall. A future ISR-context regression
  trips this in production, not just in debug.
- The audit gate (Phase 1 verification) traces from every known ISR
  registration point and confirms no path reaches `memAlloc`. (Belt +
  braces against the runtime assert.)
- The bandaids removed in Phase 2 include some code that *might* have
  allocated from interrupt context historically; the Phase 2 PR for each
  file double-checks that.
- **No static initializers / no constructor attributes.** Build with
  `-Wglobal-constructors -Werror=global-constructors` so any future
  `__attribute__((constructor))` use breaks the build. C-only — no C++
  static initializers possible (we're already C, no `.cpp` files in
  `src/`, but the flag is belt + braces).

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

**BSS-budget accounting note (A29):** the 1.2 MB region buffer is *new*
BSS, but it consolidates allocations that currently live on the heap
(walk clean buffer ~149 KB, frame buffers up to ~150 KB, prefetch
buffer, stream scratch, stream window, surface pool, resource catalog
structs, audio mixer state, etc.). Those bytes were always part of
the runtime working set; they're now *visible to the linker* instead
of allocated at runtime. Net delta-to-program-RAM is small (the new
region's headroom over the current peak heap usage); the BSS jump
overstates the actual memory growth. Don't try to reclaim "BSS budget"
by shrinking the region — that's the same as evicting from RAM.

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
 * valid pointer or halts via memHalt. Never NULL.
 *
 * The `tag` parameter is for telemetry only — NOT stored per-allocation
 * (would cost a header word per bump). Consumed by the call site that
 * provides it (e.g., the JCMEM line on next memLogTelemetry). May be
 * NULL. */
#define MEM_REGION_ALIGN 4
void *memAlloc(MemRegion region, size_t size, const char *tag);

/* Free is a no-op in BOOT (memHalt post-freeze), real release in CACHE
 * (used only by the LRU evictor — internal, never recurses), balance-
 * decrement in TRANSIENT. */
void memFree(MemRegion region, void *ptr);

/* Wipes TRANSIENT region. Logs peak. Asserts sceneAllocBalance==0 in
 * debug. Release path: ~5 cycles (bump-pointer reset + telemetry test).
 * MEM_POISON_TRANSIENT debug path: ~15 ms for full 250 KB fill at PSX
 * memory write bandwidth (~16 MB/s, cache-line stores). */
void memSceneReset(const char *sceneName);

/* Unified halt primitive — every allocator failure path calls this.
 * Internally dispatches:
 *   - JC_BSOD(scene, reason) if graphicsIsInitialized()
 *   - fatalError("%s: %s", scene?:"(boot)", reason) otherwise
 * Call sites never decide between JC_BSOD and fatalError; memHalt does. */
__attribute__((noreturn))
void memHalt(const char *scene, const char *reason);

/* Diagnostics — zero overhead on hot path */
size_t memRegionUsed(MemRegion);
size_t memRegionPeak(MemRegion);
void   memLogTelemetry(void);  /* gated behind FG_HEAP_PROBE_LOGS */
/* Region state read by ps1Bsod via externs (same pattern as the existing
 * fg*/walkPilot* externs); see "Failure UX" below. */

/* Boot. memAlloc before this calls memHalt. */
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
    ps1DebugInit();                             /* FIRST — already at src/jc_reborn.c:1688
                                                   today; explicit dependency of memHalt's
                                                   pre-graphics path so it MUST precede
                                                   memInit (A17). */
    memInit();                                  /* regions ready */
    /* pack_header_metrics.h is compile-time data — no CD reads here */
    memVerifyBootBudget();
    memVerifyAllScenesFitTransient();
    memVerifyAllScenesPinnedFitCache();
#ifdef JC_VERIFY_PACK_HASHES
    memVerifyPackHashes();                      /* ~9 sec CD work; dev/QA only (PR13) */
#endif
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

If any verify fires, `memHalt` runs ps1DebugError (rendered text panel,
not a black freeze) and halts. Graphics-up failures route through
JC_BSOD as described below.

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

- `src/platform/ps1/ps1_debug.h` declares `JC_BSOD(scene, reason)` (captures
  `__FILE__`/`__LINE__` automatically) backed by
  `ps1Bsod(scene, reason, file, line)`.
- The implementation at `src/platform/ps1/ps1_debug.c:228` already emits grep-friendly
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

The plan reuses this framework rather than inventing a new one.

### Unified `memHalt(scene, reason)` primitive

All allocator failure paths call **one function** rather than choosing
between `JC_BSOD` and `fatalError` at every site:

```c
__attribute__((noreturn))
void memHalt(const char *scene, const char *reason) {
    if (graphicsIsInitialized()) {
        /* ps1Bsod is noreturn; GCC infers unreachable past it. */
        ps1Bsod(scene, reason, __builtin_return_address(0)
                                ? "(caller)" : "(unknown)", 0);
    } else {
        /* Pre-graphics: use ps1DebugError (renders minimal text panel,
         * safe before graphicsInit completes) — depends on ps1DebugInit
         * having run first; boot sequence ensures it (A17). */
        ps1DebugError("%s: %s", scene ? scene : "(boot)", reason);
        while (1) { /* halt forever — GCC infers non-returning */ }
    }
}
```

Allocator call sites then look uniform:

```c
memHalt(currentSceneOrFgLoopGetLastScene(),
        formatReason("CACHE exhausted req=%zu pinned=%zu", n, pinned));
```

Where `currentSceneOrFgLoopGetLastScene()` returns the current scene if
one is in progress, otherwise `fgLoopGetLastScene()` for diagnostic
continuity through the scene-pick window (resolves A14).

`formatReason` is declared in **`src/platform/ps1/ps1_debug.h`** (BSOD-side helper,
not memory-side) and writes into a single static buffer. Re-entrancy
contract: the path is `noreturn`, so a second call would only happen if
the first call's path itself failed. `ps1Bsod` does not allocate
(PSX-libc `printf` is non-allocating, confirmed against the PsnoobSDK
build the project uses; documented as a non-regression invariant in
`mem_region.c`'s header comment). Defensive belt: `formatReason`'s
buffer is `volatile` and includes a 1-entry depth counter; a recursive
call into `formatReason` writes `"[concurrent fatal]"` instead.

### `ps1Bsod`'s heap-probe block — REPLACED, not extended (P8/A11)

Today, `src/platform/ps1/ps1_debug.c:245+` emits diagnostic lines like:

```
JCBSOD heapKB=...                    /* via fgProbeLargestAlloc */
JCBSOD frameBufBytes=...             /* via fgGetFrameBufferBytes */
JCBSOD prefetchBufBytes=...
JCBSOD walkCleanAlloc=... walkCleanKB=...
JCBSOD johnwalkSlotLoaded=...
```

`fgProbeLargestAlloc` is a binary-search-malloc — after Phase 2 deletes
the bandaids and the project-level poison makes libc malloc unreachable,
this line is broken at best (returns 0, misleading support tickets) and
recursive-BSOD at worst (poison trips inside the BSOD path).

**The Phase 1 PR for `ps1Bsod` REPLACES the heap-probe block entirely**
with mem-region state reads:

```
JCBSOD memBootUsed=345600 memBootPeak=345600
JCBSOD memCacheUsed=423000 memCachePeak=580000
JCBSOD memTransientUsed=0    memTransientPeak=234567
JCBSOD sceneAllocBalance=0
JCBSOD frameBufBytes=...             /* still meaningful; buffer now in BOOT */
JCBSOD walkCleanAlloc=1 walkCleanKB=149  /* still meaningful, BOOT-resident */
JCBSOD johnwalkSlotLoaded=...
```

The `walkClean*` and `johnwalkSlot*` lines stay because those buffers
still exist (just in BOOT now). The `fgProbeLargestAlloc` and
`prefetchBufBytes` lines are dropped — the former is meaningless under
regions, the latter is folded into `memBootUsed`.

**Defensive value clamping (PR9):** `ps1Bsod` reads each region state
through clamp helpers (`memSafeRead(region)` clamps to `[0,
MEM_REGION_TOTAL]`). If a *data* corruption hits the bump pointer or
balance counter, the dump renders bounded values instead of garbage.
The clamp does NOT cover *instruction* corruption (a stray write into
the code section means random execution is already happening; that
case is out of scope) (A20).

**Build-layering (A19):** `src/platform/ps1/ps1_debug.c` accesses region state via
forward declarations (`extern size_t memRegionUsed(MemRegion);` at the
top of the BSOD detail block), NOT by `#include "mem_region.h"`.
Avoids a circular-include trap: `mem_region.c` calls `formatReason`
declared in `ps1_debug.h`, and `ps1_debug.h` must not pull in
`mem_region.h` transitively or includes loop.

### Synthetic-failure test bootmodes (renamed: S9)

Renamed from v4's `bsod-test-mem-*` to **`bsod-ui-test-mem-*`** to
signal scope: these are **UI-only synthetic tests**, not integration
tests. They call `memHalt` directly with a fixed reason string,
bypassing the allocator. They verify the on-screen panel renders
correctly for each region; they do *not* exercise the allocator's
failure path.

- `bsod-ui-test-mem-boot` — synthesize BOOT-region halt during boot
  (note: pre-graphics, so uses `ps1DebugError`, not the full BSOD panel)
- `bsod-ui-test-mem-cache` — synthesize CACHE halt mid-scene
- `bsod-ui-test-mem-transient` — synthesize TRANSIENT halt mid-scene

**CI gate:** the shipping build config asserts all three flags are 0 by
default (A15) — same pattern as the existing `MEM_DEV_BUILD=0` check.

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

Replacement halt mechanism is **`memHalt(scene, reason)`** — the unified
primitive that internally dispatches to JC_BSOD (graphics up) or
ps1DebugError (graphics not yet up). Call sites never choose; `memHalt`
does it for them.

| # | File:line                                   | Removal                                                    |
|---|---------------------------------------------|------------------------------------------------------------|
| 1 | `foreground_pilot.c:3787` `JCSKIP pack-start-failed`           | Delete; memHalt                                            |
| 2 | `foreground_pilot.c:3808` `JCSKIP draw-bounds-failed`          | Delete; memHalt                                            |
| 3 | `foreground_pilot.c:3847` `JCSKIP clean-rect-alloc-failed`     | Delete; clean-rect is TRANSIENT-reserved (unreachable)     |
| 4 | `foreground_pilot.c:3782-3786` "Graceful skip instead of BSOD" | Delete entirely (the BSOD it talks about is what we want)  |
| 5 | `foreground_pilot.c:fgDropOptionalPrefetchBuffersForCleanSnapshot` + 4 callers | Delete function and callers              |
| 6 | `foreground_pilot.c:fgDropPressureCachesForCleanSnapshot`      | Delete                                                     |
| 7 | `foreground_pilot.c:fgBackdropSaveCleanBgRectsWithPressureFallback` | Replace with plain `fgBackdropSaveCleanBgRects`        |
| 8 | `foreground_pilot.c:1471,1482` JCSTREAM prealloc-failed prints | Delete; runtime unreachable (boot pre-allocs in BOOT)      |
| 9 | `foreground_pilot.c:fgPrePrimeStreamBuffers` lazy/eager paths  | Delete; boot allocations are deterministic                 |
|10 | `resource.c:700-704` (`findAdsResource` NULL on PS1)           | memHalt on both PS1 and PC                                 |
|11 | `resource.c:715-722` (`findBmpResource`)                       | memHalt on both                                            |
|12 | `resource.c:732-737` (`findScrResource`)                       | memHalt on both                                            |
|13 | `resource.c:747-752` (`findTtmResource`)                       | memHalt on both                                            |
|14 | `ads.c:1696` `if (adsResource == NULL) return;`                | Delete (findAds memHalts)                                  |
|15 | `ads.c:1708` ps1_loadAdsData silent return                     | memHalt                                                    |
|16 | `ads.c:1753+` "skip this scene gracefully" TTM block           | memHalt                                                    |
|17 | `walk_pilot.c:108-117` walkClean buf silent skip               | Delete; buf is BOOT-allocated                              |
|18 | `walk_pilot.c:165-176` walkPilotInit alloc-fail soft return    | memHalt (init time → ps1DebugError path)                   |
|19 | `walk_pilot.c:255-260` JOHNWALK silent-bail-out                | memHalt                                                    |
|20 | `graphics.c:1370-1395` "Pool exhausted - fall back"            | memHalt; pool sized correctly at boot                      |
|21 | `cdrom_ps1.c:644,885,2354` malloc-fail NULL returns            | memHalt (dispatches based on graphics readiness)           |
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
   `scripts/generate-pack-metrics.py`),
   `docs/ps1/mem-region-decision-tree.md` (resolves M9 — lands in
   the same PR, not after).
2. PC + PS1 implementations with identical API and budget enforcement.
3. CACHE segregated free-list with non-recursive eviction.
4. 4-byte alignment (`MEM_REGION_ALIGN = 4`).
5. CRC-32 (not SHA-256) for `memVerifyPackHashes` — committed (P12).
6. `memHalt(scene, reason)` primitive in `mem_region.h`; all allocator
   failure paths call this rather than choosing JC_BSOD vs fatalError
   themselves (P9, M7).
7. `formatReason` helper in `src/platform/ps1/ps1_debug.h` (BSOD-side, not memory-
   side) with `volatile` static buffer + 1-entry depth counter
   guarding against the impossible-but-cheap-to-defend re-entry case
   (M10, A16).
8. `MEM_REQUIRE(ps1IsMainContext())` (hand-rolled macro, not C's
   `assert()`) in `memAlloc` and `memFree`, unconditional in release
   builds (~8-12 cycles per call from COP0 read; P1-bis, P13).
9. **Replace** (not extend) the heap-probe block in `src/platform/ps1/ps1_debug.c:245+`:
   drop `fgProbeLargestAlloc`/`prefetchBufBytes` lines, add
   `memBootUsed/Peak`, `memCacheUsed/Peak`, `memTransientUsed/Peak`,
   `sceneAllocBalance` via region-state externs. Keep `walkClean*` and
   `johnwalkSlot*` (still meaningful — buffers persist in BOOT) (P8, A11).
   Use `memSafeRead(region)` clamp helpers so the BSOD dump can't crash
   if metadata is corrupted (PR9).
10. Upgrade `fatalError` on PS1 to render a minimal text panel via
    `ps1DebugError` (`src/platform/ps1/ps1_debug.h:51`) instead of plain printf +
    while(1). Pre-graphics failures get on-screen feedback (A10).
11. Boot integration including `memVerifyPackHashes`, the boot-order
    audit (audio, resource catalog, font, surface pool, walk, pause —
    all before `memFreezeBoot`), and `-Wglobal-constructors
    -Werror=global-constructors` build flag (A13).
12. Migrate all 39 call sites to `memAlloc(REGION, n, tag)` with explicit
    `INIT_ZEROED`/`INIT_FULL_WRITE`/`INIT_NONE` annotation grep'd by CI.
13. Hook `memSceneReset` into `fgRuntimeReset` at
    `src/foreground_pilot/foreground_pilot.c:1470`. **Pre-emptive CACHE eviction
    (`memCachePreEvictForNextScene`) wired into `fgLoopNextScene` —
    not memSceneReset (PR7).**
14. Reorder `foregroundPilotRuntimeStart` so clean-rect allocates first.
15. Adopt project-level malloc poison via `src/malloc_poison.h` — every
    .c file in src/ includes it, libc malloc/free become compile errors
    except in `mem_region.c` and a whitelist comment-justified in each
    exception.
16. Add three `bsod-ui-test-mem-*` bootmodes (renamed from
    `bsod-test-mem-*`; S9) for visual QA of the per-region halt UI.
17. CI checks: `MEM_DEV_BUILD=0`, `BSOD_UI_TEST_*=0`, grep gates pass,
    pack hashes current.
18. PR template gets a new checkbox: "I built locally with
    `MEM_DEV_BUILD=0` and ran the 63-scene rotation cleanly" — required
    for any PR touching scene data (S10).
19. **Implement `fgLoopGetLastScene()`** in `src/jc_reborn.c`: returns
    the **target** scene if a pick is in progress, else the **last
    successfully played** scene (A18, A22, A27). Three-state
    machine + three statics:

    ```c
    static int pickInProgress = 0;
    static const char *lastTarget = NULL;
    static const char *lastPlayed = NULL;

    const char *fgLoopGetLastScene(void) {
        return pickInProgress ? lastTarget : lastPlayed;
    }
    ```

    State transitions (Phase 1 wires these into the existing main
    loop):
    - **In `fgLoopNextScene`** (or the caller, immediately after the
      pick returns): `lastTarget = chosenSlug; pickInProgress = 1;`
    - **After `foregroundPilotPlay` returns successfully** (in the
      main loop body): `lastPlayed = lastTarget; pickInProgress = 0;`
    - **At boot:** both NULL; `fgLoopGetLastScene()` returns NULL;
      `memHalt` writes `"(boot)"` when the scene argument is NULL.

    Verified by grep that the function does not currently exist
    before Phase 1.
19a. **Change `fgLoopApplyVariant` signature** (PR14):
    `const char *fgLoopApplyVariant(const char *scene)` returning the
    effective scene name (variant slug if a variant fires, original
    slug otherwise). Caller passes the returned name into
    `memCachePreEvictForNextScene`.
19b. **Implement `ps1IsMainContext()`** (P17, P20): inline COP0 read
    in `mem_region.c` using `mfc0 $13` (CAUSE) and `mfc0 $12` (SR).
    Returns 1 if no active exception and interrupts enabled. Bit
    patterns per PSX programmer's manual; v8 verifies them with a
    Phase 1 unit test:
    - Test 1: call from main flow; assert returns 1.
    - Test 2: bracket with `EnterCriticalSection()` /
      `ExitCriticalSection()` (PsyQ primitives) and call inside the
      critical section; assert returns 0 (since EXL or IEc has
      changed).
    - Test 3: hook a synthetic VBlank callback that calls the helper
      and stores the result in a debug global; main flow checks
      after one frame; assert 0.
20. `MEM_REGION_RATIONALE: <one-liner>` comment required on every
    `memAlloc` call site. CI uses
    `scripts/check-mem-region-rationale.py` (~30 LOC) — handles
    multi-line `memAlloc` calls, same-line comments, and detects
    macros whose body contains `memAlloc(` (such macros are
    prohibited; see A24). (S12, S14, A24)
21. CI count-match gate: `MEM_REGION_*` enum count in `mem_region.h`
    must equal occurrences in `mem-region-decision-tree.md` (M14).
22. Build flag `JC_VERIFY_PACK_HASHES` defaults off in release config,
    on in dev/QA. CI verifies release setting (PR13).
23. `-Wglobal-constructors -Werror=global-constructors` lands as a
    PS1-build flag (A13 from v3, re-confirmed in v6).
24. Phase 1 implementation checklist: `docs/ps1/mem-region-phase-1-checklist.md`
    lands in the same PR. Mirrors these steps as TODO items (M12).
25. **New file `src/mem_region/mem_region_extern.h`** holds forward declarations
    consumed by `ps1_debug.c`. Single source of truth for cross-module
    symbol signatures (A23, A26). Uses an **incomplete enum forward
    declaration** to avoid pulling in `mem_region.h`:

    ```c
    /* mem_region_extern.h */
    #include <stddef.h>
    enum MemRegion;                            /* incomplete type — OK for fn decls */
    extern size_t memRegionUsed(enum MemRegion region);
    extern size_t memRegionPeak(enum MemRegion region);
    extern size_t memSafeRead(enum MemRegion region);  /* clamped variant */
    extern int    sceneAllocBalanceGet(void);
    ```

    `ps1_debug.c` includes only this header and uses the symbols
    behind opaque `enum MemRegion` parameters; the actual enum
    constants are passed by callers via integer literals if
    `ps1_debug.c` ever needs them (it doesn't today). `mem_region.h`
    includes `mem_region_extern.h` for its own definitions, so the
    signatures stay in sync by linker check.
26. **Pin-count delta logging** in debug builds: at every scene
    transition, sum all `pinCount` fields across resources (~200
    entries, ~600 cycles; A28). If the sum hasn't returned to the
    expected post-scene value, emit
    `JCMEM pin-delta scene=X delta=N`. Catches
    `pinResource`/`unpinResource` mismatches before they cause
    pre-evict to misreport (A25). Compiled out in release.
27. **Python-script CI fixtures (S17):** `scripts/test-mem-region-rationale/`
    contains five fixture files (valid, missing-comment, far-comment,
    wrapping-macro, multi-line) and a `make test-mem-rationale`
    target that runs the Python script against each and asserts the
    expected pass/fail result.
28. **Toolchain note (P22):** the `-Werror=global-constructors` flag
    is required; no fallback is documented. PsnoobSDK GCC 12+ is the
    floor (verified P18). If a future toolchain doesn't support the
    flag, that's a build-time error to fix at that time, not a plan-
    time concern.

**Estimated effort:** ~3 weeks focused work. Roughly 800-1000 LOC of
allocator + ~200 LOC of generator + 39 call-site touches + audit + the
decision tree doc.

**Validation:** game renders byte-for-byte identically across the
63-scene rotation. Both builds green. All four boot-time `memVerify*`
checks pass.

### Phase 2 — Delete bandaid + skip code (TWO PRs, ~1 week total)

v5's "single bundled PR with 8 atomic commits" framing breaks under
per-commit CI: intermediate commits leave the codebase in a state
where bandaid code is half-deleted (S11). v6 splits into two PRs:

**Phase 2a — Infrastructure PR (~2 days)**

One commit: `ps1_debug.c` heap-probe block REPLACEMENT (see "Failure UX")
+ `memHalt` implementation. Includes test-harness migration for the
new `JCBSOD-FATAL` sentinel semantic (S8). Sits as a no-op for the
bandaid call sites — they still compile and run identically — but
makes the new `memHalt` primitive available for the cleanup PR.

After 2a merges to main, **tag the merge commit**:
```sh
git tag pre-mem-region-bandaid-removal <merge-sha>
git push origin pre-mem-region-bandaid-removal
```

That tag is the rollback target for Phase 2b (S13).

**Phase 2b — Cleanup PR (~3-4 days)**

One PR, 8 atomic commits (each per-file, individually revertable):

- P2b.1 — `foreground_pilot.c` (items 1-9)
- P2b.2 — `resource.c` (items 10-13)
- P2b.3 — `ads.c` (items 14-16)
- P2b.4 — `walk_pilot.c` (items 17-19)
- P2b.5 — `graphics.c` (item 20)
- P2b.6 — `cdrom_ps1.c` (item 21)
- P2b.7 — `ps1_perf.c` (items 22-23)
- P2b.8 — comment refresh for the "intentional skip" sites

Each commit is internally consistent — it deletes the bandaid AND
points the formerly-NULL-returning path at `memHalt`. CI is green
at every commit. Per-commit revert works without leaving an
inconsistent state.

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
| `memAlloc(BOOT/TRANSIENT, n)`    | ~18-22 cycles               | MEM_REQUIRE COP0 read (~8-12) + bump (~5) + balance + bound check (P14) |
| `memAlloc(CACHE, n)` hit        | ~33-37 cycles               | MEM_REQUIRE + free-list head pop + class lookup    |
| `memAlloc(CACHE, n)` miss + evict| ~3-5 ms (warm) / never (hot)| **Eviction is pre-emptive in jc_reborn.c after `fgLoopApplyVariant`, not in `fgLoopNextScene` (preserves scene_picker's zero-heap invariant).** Hot path during scene play is hit-only. |
| `memFree(BOOT)`                  | 0 cycles release            | No-op                                              |
| `memFree(TRANSIENT)`             | ~5 cycles release           | Balance decrement                                  |
| `memFree(CACHE)`                 | ~30 cycles                  | Free-list push + class lookup                      |
| `memSceneReset` release          | ~5 cycles                   | Bump pointer reset + telemetry conditional        |
| `memSceneReset` debug-poison     | ~15 ms                      | 250 KB 0xCD fill at PSX write-bandwidth (~16 MB/s); debug-only (PR8) |
| Pin-count delta sum (debug only) | ~600 cycles per transition  | Sum of ~200 pinCount fields per scene; debug-only (A28) |
| Boot: `memVerify*` (4 funcs)     | ~1 ms total                 | All compile-time data + 63 CRC-32 verifies         |
| Boot: BSS clear of 1.2 MB buffer | ~75 ms                      | PsyQ `_start` zeroes BSS                           |
| **Boot delta total**             | **~75 ms**                  | Bounded; CD-seek-free                              |

Compared to libc malloc on PsyQ (~50-100 cycles per call), the hot path
is a 3-7× win on BOOT/TRANSIENT and roughly break-even on CACHE.
`MEM_REQUIRE(ps1IsMainContext())` is unconditional in release (Pat
P1-bis) on both `memAlloc` and `memFree` for API symmetry (PR15);
the ~10-cycle overhead on a no-op `memFree(TRANSIENT)` is negligible
in steady state (40 frees × scene × 10 cycles ≈ 400 cycles = 12 µs).
Above numbers include the check.

### Pre-emptive CACHE eviction — caller-site after variant resolution (resolves PR7, PR11, PR12)

v4 said eviction at `memSceneReset` (too late — blocks first frame).
v5 moved it into `fgLoopNextScene` (still wrong — that's inside the
scene_picker which is explicitly engineered for net-zero heap pressure
per `src/scene/scene_picker.c:4-6`; the picker stays pure).

v6 puts pre-emptive eviction in the caller, **after**
`fgLoopApplyVariant`, so the effective scene name (post-variant) is
used and the picker stays pure:

```c
const char *loopScene    = fgLoopNextScene(...);
const char *effectiveScene = fgLoopApplyVariant(loopScene);   /* signature change (PR14):
                                                                 returns the effective
                                                                 (variant or base) name */
memCachePreEvictForNextScene(effectiveScene);                  /* keys metrics by variant */
fgLoopWalkToScene(storyScene);                                  /* ~1-2 sec mask */
/* ... */
fgRuntimeReset();                                               /* TRANSIENT only */
```

`memCachePreEvictForNextScene(name)` consults
`pack_header_metrics.h[name].cachePinnedWorstCase`, subtracts current
CACHE used, and evicts unpinned resources until the delta fits. If
nothing's unpinned and the delta still doesn't fit — `memHalt` (the
boot proof asserts unreachability of this branch).

The scene_picker's poison-malloc comment stays intact; the picker
mutates no global state.

## Documentation deliverables

- **`docs/ps1/adding-new-scenes-memory.md`** (companion) — how to add a
  new scene or holiday variant: regenerate `pack_header_metrics.h`, run
  `memVerify*` locally, what each region is for, when to set `MEM_DEV_BUILD`,
  troubleshooting the most common boot-fatalError messages.
- **`docs/ps1/mem-region-decision-tree.md`** — committed in **Phase 1**
  alongside the allocator code, not as a follow-up (resolves M9).
  Decision tree covers scratch buffers freed within one function call,
  per-frame allocations, multi-scene caches, audio mixer state, fonts.
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
- [ ] ~~**Pinned-set math vs runtime: (A12)**~~ Moved to **Phase 1
      milestones** below (S15): this gate requires the verifier
      *and* the runtime instrumentation, both of which are Phase 1
      work. Cannot be closed pre-Phase-1.
- [ ] ~~**PSX printf is non-allocating: (P10)**~~ Dropped (A21):
      project-level malloc poison makes this a link-time check rather
      than a manual audit. If PsnoobSDK printf ever calls `malloc`,
      the link breaks. Manual confirmation no longer needed.
- [ ] **No static initializers: (A13)** add `-Wglobal-constructors
      -Werror=global-constructors` to the PS1 build; clean baseline.
      Toolchain compatibility verified (PsnoobSDK GCC 12+ supports
      the flag; P18). Fallback if a future toolchain doesn't:
      `nm jcreborn.elf | grep -E "_init|__init"` returns no entries
      in CI.

**Phase 1 milestones** (close *during* Phase 1, not before):

- [ ] **Pinned-set math vs runtime (A12, moved from pre-gates):**
      instrumented debug build's runtime pinned-size logging equals
      the verifier's computed value across all 63 scenes + variants.
- [ ] **`-Werror=global-constructors` baseline clean:** Phase 1's
      first compile against the flag.

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
