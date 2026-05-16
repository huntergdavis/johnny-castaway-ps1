# Memory region allocator — red team v9 (post-implementation reality check)

Companion to [memory-region-allocator-plan.md](./memory-region-allocator-plan.md)
v9, [implementation status](./mem-region-phase-1-status.md), and the
prior red-team rounds (v1-v8).

Where rounds v1-v8 were paper reviews of the plan, **v9 is a
post-implementation reality check informed by actual measurement on
PS1 hardware** (DuckStation headless regtest). It documents what the
plan got right, what it got wrong, what the implementation actually
ships, and where the bodies are buried.

## The single biggest finding

**The plan's central architectural decision — a 1.2 MB static
BSS buffer for the region allocator — is incompatible with this
codebase's existing libc memory demands.** Empirical testing during
implementation revealed:

- `parseResourceFiles` holds RESOURCE.001 in a single 1.1 MB libc
  malloc'd buffer for the duration of the catalog parse.
- The graphics-side scene loaders (`OCEAN00.SCR` for backgrounds, FG2
  pack body, BMP/PSB/SCR data) request 100-300 KB libc allocations
  during scene playback.
- Walk-clean buffer alone is 149 KB.
- GPU primitive ordering tables are 2×32 KB = 64 KB.

These are NOT optional — they're how the existing code paths work.
A 1.2 MB static buffer reserved at link time leaves libc with
roughly 30 KB of usable heap after the catalog parse, which can't
fit even one scene's pack body. The first scene halts on a
graphics-side malloc.

**Conclusion:** the plan's "carve a private region from PS1 RAM"
model was correct in concept but mis-sized in practice. The
budget calculation didn't account for the libc churn the existing
graphics/CD paths still need.

## What actually ships

After the empirical correction, the implementation is in
**all-libc-backed mode** (commit 906e5be2f):

- `MEM_BOOT_BUDGET = 0`, `MEM_CACHE_BUDGET = 0`, `MEM_TRANSIENT_BUDGET = 0`.
- `memAlloc` routes each region's allocations directly to libc
  `malloc`. No static region buffer.
- **TRANSIENT preserves the wholesale-wipe semantic** via a linked
  list of libc-allocated pointers: `memSceneReset` walks the list,
  `free`s each entry, resets the head. This delivers the per-scene
  cleanup invariant the plan's design targeted — just backed by libc
  instead of bump-allocation in a private buffer.
- BOOT allocations go to libc, never freed (BOOT lifecycle = program
  lifetime).
- CACHE allocations go to libc; the LRU evictor's existing `free()`
  calls work natively (no `#ifdef` needed).
- All four boot-time `memVerify*` functions still run; they emit
  `JCMEM WARN: ...` lines on budget overflow instead of halting.

## What works

| Component | Status |
|-----------|--------|
| Allocator API (`memAlloc`/`memFree`/`memSceneReset`/`memHalt`) | ✓ in use |
| TRANSIENT wholesale-wipe via linked list | ✓ functional |
| `memHalt` dispatch (BSOD vs ps1DebugError) | ✓ functional |
| `MEM_REQUIRE` macro (release-shipping invariant check) | ✓ functional |
| `ps1IsMainContext()` via COP0 inline asm | ✓ functional |
| `ps1Bsod` heap-probe → region-state diagnostics | ✓ functional |
| `bsod-ui-test-mem-*` bootmodes | ✓ functional |
| Pack metrics generator + populated table | ✓ functional |
| CRC-32 implementation (Sarwate) | ✓ ready, unused (no CRCs computed offline) |
| Python rationale-check script + fixtures | ✓ all gates pass |
| Phase 2 bandaid removal (12 of 23 manifest items) | ✓ done |
| Pin-count delta logging (debug builds) | ✓ wired |
| Pre-evict on scene transition (`fgLoopApplyVariant` hook) | ✓ wired |
| **fishing1 scene plays successfully on headless DuckStation** | ✓ verified |
| **fishing2 + activity1 scene plays** | ✓ verified |
| **mary1 scene plays at least partway** | ✓ verified |
| **Performance vs v0.8.14 baseline** | ✓ within run-to-run noise |

## Measured performance

Headless DuckStation regtest, fishing1 low, perf-log mode, 5000-frame
budget, seed 1:

| Metric | v0.8.14 baseline (from perf matrix) | This branch (906e5be2f) |
|--------|-------------------------------------|-------------------------|
| `scene_vb` | (not directly comparable — different boot args) | 1345 |
| `loop_vb` | 1067 | 1068 |
| `target_vb` | 1074 | 1075 |
| `blocking_vb` | 1 (close) | 1 |
| `due_misses` | 0 | 0 |

Delta: **+1 loop vblank** (0.09%), **+1 target vblank**. Within
run-to-run noise. **No measurable performance regression.**

This is unsurprising: the allocator is libc-backed end-to-end, so
the runtime cost is identical to the pre-existing libc malloc
behavior. The TRANSIENT linked-list wholesale-wipe adds one libc
free + one struct-free per per-scene allocation at scene transition,
which is amortized away in a 1068-vblank scene loop.

## What does NOT ship as planned

Items the plan called for that the implementation doesn't deliver:

- **Deterministic, fragmentation-immune region allocator.** Plan's
  central goal. The libc-backed mode still uses libc, which can
  fragment. The fragmentation-attack the plan was designed for is
  not actually attacked.
- **`_Static_assert(MEM_REGION_TOTAL ≤ 1.2 MB)` as a budget gate.**
  Total is 0; assert trivially passes. Future static-buffer mode
  would re-enable this.
- **`memFreezeBoot`'s no-fail invariant.** The function still
  exists and is called, but since BOOT allocations go to libc and
  libc can technically still satisfy late requests, the "post-freeze
  BOOT alloc halts" invariant doesn't materially apply. Symbolic only.
- **Pre-allocated pack-header buffers in BOOT.** Plan called for
  pack-header scan to size frame/prefetch/scratch/window buffers
  at the worst-case max. Those buffers stay grow-only on libc.
- **No-fragmentation guarantee for CACHE.** CACHE-backed allocations
  go to libc; LRU eviction still happens but doesn't fight
  fragmentation, only memory pressure.
- **Holiday variant pack enumeration in metrics.** Analyzer JSON
  doesn't expose variants; generator can't include them.
- **CRC-32 pack-header verification at boot.** Implementation is
  ready (`crc32_compute` works); CRCs aren't generated offline yet,
  so the verifier is a no-op even with `JC_VERIFY_PACK_HASHES` on.

## What is honestly unsafe to ship

- **The TRANSIENT linked-list wholesale-wipe DOES work** — every
  TRANSIENT alloc gets freed at the next `memSceneReset`. This is
  the genuine win. Scene-to-scene leakage is bounded.
- **The Phase 2 bandaid removals (12 items) DO work** — JCSKIP paths
  are deleted, NULL-return guards are unified, alloc-fail counters
  are no-op'd. Code is cleaner.
- **The boot-time memVerify* gates DO catch budget mismatches** —
  but they're warnings, not halts. A subtle data drift between the
  analyzer JSON and the actual scene data would print a JCMEM WARN
  and otherwise proceed.

## Red flags for future work

1. **`parseResourceFiles` allocates 1.1 MB temporarily.** The plan's
   approach can't materially help until this is refactored to
   stream-read the catalog instead. Required precondition for any
   future static-region-buffer attempt.

2. **The analyzer JSON doesn't know about holiday variants.** If a
   holiday variant has higher peak memory than its base, the
   metrics underestimate. Runtime might overflow CACHE/TRANSIENT
   silently (warnings, not halts).

3. **Resource data still on libc.** The "resource cache through
   CACHE region with deterministic placement" goal was reverted
   because the catalog buffer steals all the libc space CACHE
   would have used. The LRU continues to operate on libc memory.

4. **No fragmentation defense.** The original problem (heap
   fragmentation under long-soak playback) is not solved by the
   current implementation. If v0.8.10's fragmentation pattern
   recurs in a multi-hour soak, this allocator does not prevent it.

5. **`memFreezeBoot`'s "loud-on-late-BOOT-alloc" guarantee is
   weakened.** Late BOOT allocations route to libc which usually
   succeeds. The plan's "halt the build loud and clear" semantic
   doesn't apply.

## Recommended next steps

To make the plan's actual goal achievable on this hardware:

1. ~~Refactor `parseResourceFiles` to stream RESOURCE.001.~~ **DONE**
   in commit c8f612894 — new `ps1_fopen_stream(filename, cacheBytes)`
   uses a 64 KB sector cache. Verified by build + 6-scene playback
   matrix (fishing1/2/3, activity1, building1, johnny1 all PASS at
   loop_vb identical to baseline; mary1 reaches frame 614/683 within
   the 8000-frame budget).

2. **Audit large libc malloc sites in the scene-playback path.**
   `OCEAN00.SCR` 150 KB, BMP/PSB data 50-300 KB each, etc. Migrating
   these to CACHE region (or sizing the region to include them)
   is the prerequisite to fragmentation immunity. Testing with
   600 KB static region post-streaming confirmed the OCEAN load
   still fragments libc — these need their own migration.

3. **Once libc has consistent headroom (~200 KB free during
   scene playback)**, re-enable the static region buffer at a
   feasible size (~600-900 KB). The plan's `memFreezeBoot`,
   bump+wipe TRANSIENT, and free-list CACHE all become real.

4. **Extend the analyzer JSON to enumerate holiday variants** with
   per-variant memory peaks. Without this the boot proofs are
   incomplete.

## Empirical perf data (post-streaming-refactor + all-libc allocator)

Headless DuckStation regtest, 8000-frame budget, seed 1:

| Scene | scene_vb | loop_vb | target_vb | blocking_vb | hits | due_misses | gate |
|-------|----------|---------|-----------|-------------|------|------------|------|
| fishing1 | 1345 | 1068 | 1075 | 1 | 136 | 0 | PASS |
| fishing2 | 2039 | 1758 | 1762 | 5 | 246 | 0 | PASS |
| fishing3 | 2246 | 1959 | 1956 | 9 | 268 | 0 | PASS |
| activity1 | 3016 | 2755 | 2765 | 0 | 187 | 0 | PASS |
| building1 | 1038 | 782 | 782 | 11 | 104 | 1 | PASS |
| johnny1 | 2025 | 1948 | 1945 | 5 | 111 | 0 | PASS |
| mary1 | (incomplete) | — | — | — | — | — | 614/683 frames (90%); test budget exhausted |

vs v0.8.14 baseline (performance-scene-matrix.csv):
- fishing1 baseline: loop_vb 1067 — branch: 1068 (delta +1 vb / 0.09%)

**Within run-to-run noise. NO regression.**

## Verdict

The implementation **does not deliver the plan's central
architectural goal** (deterministic, fragmentation-immune region
allocator). It **does deliver the wholesale-wipe TRANSIENT
semantic** that gives some of the plan's behavioral benefits, and
the boot proof / diagnostics infrastructure / bandaid removal work
that gives some of the plan's correctness benefits.

**Performance is identical to baseline** within run-to-run noise.
The plan didn't regress speed; it also didn't materially improve
memory safety. Net effect on the shipping game: small (some bandaid
code gone, some diagnostic state added, allocator API exists for
future migration).

**This branch is acceptable to ship as a foundation** for future
work — the API is in place, the API discipline (`MEM_REGION_RATIONALE`,
`memAlloc`/`memFree` pairs, `memSceneReset` hooks) is established,
and the CI gates exist. **It should NOT be sold as completing the
plan's no-fragmentation promise** because that promise is not
delivered.
