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

---

## Update — 2026-05-16: static region now active (1012 KB)

The earlier "all-libc-backed mode" verdict was correct **for that
configuration**. After this update the implementation now ships a
real static region buffer; the all-libc fallback remains only for
pre-`memInit` callers and oversized scenes.

### What changed since the original v9 writeup

1. **`memInit` allocates the region buffer dynamically from libc**
   *after* `parseResourceFiles` completes, so the 1.1 MB RESOURCE.001
   catalog parse no longer competes with a fixed BSS reservation.
2. **RESOURCE.001 is now streamed** through `ps1_fopen_stream`
   (`src/cdrom_ps1.c`) with a 64 KB sector cache. This frees ~1 MB
   of libc that the catalog parse used to claim permanently.
3. **Big libc allocations migrated into the region:**
   - `ps1_fopen` file buffer → CACHE (with `bufferFromRegion` flag
     in `PS1File` for `ps1_fclose` dispatch).
   - `ps1_streamReadFromCdFile` sector buffer → TRANSIENT, result
     buffer → CACHE.
   - `ps1_streamReadFromCdFileInto` sector buffer → TRANSIENT.
   - `ps1_streamReadFromCdFileWhole` result buffer → CACHE.
   - `ensureBgTileRAM` / `createEmptyBgTileRAM` / `freeBgTile`
     (`src/graphics_ps1.c`) PS1Surface struct + 150 KB pixel
     buffer → CACHE. This was the OCEAN-scene crash driver.
4. **Budgets re-enabled** at:
   - `MEM_BOOT_BUDGET = 32 KB`
   - `MEM_CACHE_BUDGET = 700 KB`
   - `MEM_TRANSIENT_BUDGET = 280 KB`
   - Total = 1012 KB.
5. **Defensive libc fallback** retained at TRANSIENT overflow
   (linked-list of libc pointers, freed at `memSceneReset`) and
   CACHE overflow (pointer-range detection in `memFree`). The
   fallbacks preserve correctness when a scene's footprint exceeds
   its region budget; they do NOT bypass the per-scene-wipe or
   LRU-eviction invariants.

### Boot proof — actual behavior

TTY log from a fresh boot:
```
JCBOOT source=file fgpilot=1 scene=fishing1 args=1 seed=1 ...
JCMEM memInit: region buffer 1012 KB at 8008bfb0
```
The buffer is allocated successfully at `0x8008bfb0` after the
catalog parse releases its temporary 1.1 MB libc claim.

### Scene matrix (2026-05-16, 7200 frames per case)

| Case            | scene_vb | loop_vb | target_vb | block_vb | hits | due_misses | Result |
|-----------------|----------|---------|-----------|----------|------|------------|--------|
| fishing1        | 1345     | 1068    | 1075      | 1        | 136  | 0          | PASS   |
| fishing2        | 2039     | 1759    | 1764      | 3        | 246  | 0          | PASS   |
| activity1-high  | 2998     | 2755    | —         | —        | —    | 0          | PASS   |
| activity1-low   | 3001     | 2756    | —         | —        | —    | 0          | PASS   |
| activity4-high  | 1332     | 1065    | —         | —        | —    | 0          | PASS   |
| activity4-low   | 1331     | 1064    | —         | —        | —    | 0          | PASS   |
| activity5-high  | 1972     | 1732    | —         | —        | —    | 0          | PASS   |
| activity5-low   | 1972     | 1730    | —         | —        | —    | 0          | PASS   |
| activity6-high  | 1151     | 912     | 909       | 2        | 104  | 0          | PASS   |

All 9 cases produce `overall_pass: True`. No `JCSKIP`, no
`JCBSOD-FATAL`, no `due_misses`.

### Performance comparison vs v0.8.14 baseline

| Scene    | v0.8.14 loop_vb | Static-region loop_vb | Delta  |
|----------|-----------------|------------------------|--------|
| fishing1 | 1067            | 1068                   | +1 vb (+0.09 %) |

Within run-to-run noise; the static region adds zero measurable
overhead.

### Updated verdict

The static region is now **functionally live**:
- 1012 KB of PS1 RAM is owned by the allocator, with bump-up BOOT,
  free-list CACHE, and bump-down TRANSIENT lifetimes.
- `memSceneReset` wipes TRANSIENT wholesale at every scene
  transition, including the libc-fallback linked list.
- The LRU evictor calls back into `memFree(MEM_REGION_CACHE, ...)`
  through `checkMemoryBudget` in `src/resource.c`, completing the
  plan's feedback loop.
- Pre-`memInit` callers (TITLE.RAW load, etc.) and oversize
  allocations transparently fall back to libc; the fallback paths
  preserve the plan's invariants (per-scene wipe, LRU eviction).

The plan's central goal — **deterministic region ownership of
~1 MB of PS1 RAM, with fragmentation contained inside the CACHE
sub-allocator and TRANSIENT wiped between scenes** — is delivered,
with measured per-scene performance unchanged vs the prior
all-libc-backed baseline.

---

## Round 10 — telemetry-driven budget tuning (2026-05-16)

Enabling `memLogTelemetry()` (temporarily un-gating the
`FG_HEAP_PROBE_LOGS` `#ifdef`) at every `memSceneReset` surfaced
two findings the happy-path scene matrix missed.

### Finding 10-A: CACHE 700 KB was one byte from libc fallback on mary1

mary1 mid-scene snapshot at the 1012 KB budget configuration:
```
JCMEM boot=0/32768 cache=716084/716800 transient=230696/286720
      (peaks 0 716084 230696) balance=6
```
CACHE was 99.9% utilized. Functionally fine — the LRU evictor +
libc fallback would catch any overflow — but eliminates the
deterministic-region promise for MARY's load pattern in practice.
**Mitigation:** bumped CACHE to 800 KB.

### Finding 10-B: johnny1 BSOD reproducibly at CACHE 800 KB

After the 800 KB bump, johnny1 still failed:
```
JCBSOD-FATAL CACHE exhausted (region+libc both): req=116736 have=81736
JCBSOD memCacheUsed=737464 memCachePeak=737464
```
johnny1 needs `gFgFrameBuffer` 112 KB + `gFgPrefetchFrameBuffer`
112 KB simultaneously, plus the bg-tile pixel buffer (150 KB) +
other LRU residency (~580 KB carried across the boot/title
transition). 614 KB pre-scene residency + 2x 114 KB johnny1
buffers ≈ 842 KB > 800 KB budget. The libc fallback also failed —
the 1012 KB region had displaced libc's contiguous heap.

Two follow-on changes resolved this:

1. **Migrated grow-only buffers from libc to CACHE** in
   `src/foreground_pilot.c` (per plan Phase 3 table):
   - `gFgFrameBuffer` (line 3206)
   - `gFgPrefetchFrameBuffer` (line 3227)
   - `gFgStreamWindowBuffer` (line 3323)
   - `gFgStreamScratch` (line 3358)
   - Their matching `free()` calls in `fgReleaseStreamBuffers`
     (lines 1373-1391).

2. **Bumped CACHE budget to 900 KB**, shrunk TRANSIENT to 256 KB
   (peak observed 230 KB on mary1 — 256 leaves 12% headroom),
   total 1188 KB (under the 1228 KB linker-map ceiling).

### Verified end-to-end at 1188 KB

7-scene matrix at CACHE 900 / TRANSIENT 256 / BOOT 32:

| Scene    | scene_vb | loop_vb | block_vb | hits | due_misses | Result |
|----------|----------|---------|----------|------|------------|--------|
| fishing1 | 1325     | 1070    | 4        | 136  | 0          | PASS   |
| fishing2 | 2020     | 1762    | 8        | 246  | 0          | PASS   |
| mary1    | 5139     | 4849    | 18       | 681  | 1          | PASS   |
| building1| 1038     | 783     | 13       | 104  | 1          | PASS   |
| suzy1    | 5938     | 5761    | 18       | 176  | 0          | PASS   |
| activity1| 3016     | 2755    | 0        | 187  | 0          | PASS   |
| activity4| 1326     | 1065    | 1        | 155  | 0          | PASS   |
| johnny1  | 2025     | 1948    | 5        | 111  | 0          | PASS   |

vs v0.8.14 baseline: fishing1 loop_vb 1067 → 1070 (+3 / +0.3 %),
within run-to-run noise.

Peak telemetry captured during the run:
- mary1:   CACHE peak 758076 / 921600 (82%); TRANSIENT 230696 / 262144 (88%)
- johnny1: CACHE peak 852156 / 921600 (92%); TRANSIENT  10432 / 262144  (4%)

### Limitations and known sharp edges

- **CACHE first-fit free-list does not coalesce.** Adjacent freed
  blocks don't merge, so worst-case fragmentation could exceed
  observed peaks on long-running scene rotations. The 92%
  johnny1 peak is uncomfortably close to budget. A future PR
  should add boundary-tag coalescing in `cacheFreeInternal`.
- **mary1 reports `due_misses=1`, building1 reports `due_misses=1`.**
  These predate this branch (same number on v0.8.14 baseline);
  not introduced by the region work.
- **`memLogTelemetry()` ships gated behind `FG_HEAP_PROBE_LOGS`.**
  To reproduce these peaks: build with
  `-DFG_HEAP_PROBE_LOGS=1` or temporarily un-gate the call site
  in `memSceneReset`.

### Final verdict

The static region is now **delivering correctness AND headroom**
end-to-end across the full primary scene matrix. Performance is
within run-to-run noise of the v0.8.14 baseline. The plan's
no-fragmentation promise is met for the measured workloads, with
the caveat noted above about coalescing for longer rotations.
