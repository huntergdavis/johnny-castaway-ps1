# Memory region allocator — Phase 1 implementation status

Companion to [memory-region-allocator-plan.md](./memory-region-allocator-plan.md).
Tracks Phase 1 + Phase 2 implementation progress.

**Branch:** `claude/memory-region-allocator`
**Last build:** clean, 0 warnings, `jcreborn.exe` ≈ 218 KB

## Phase 1: Allocator + migration

### ✅ Landed

**Foundation (commits 756f93782 + 6259dafcb + d0901916c + 137ea96b3):**
- `src/mem_region.{c,h}` + `src/mem_region_extern.h` + stub
  `src/generated/pack_header_metrics.{c,h}` (now populated).
- Allocator API: `memAlloc`, `memFree`, `memSceneReset`, `memHalt`,
  `memVerify*`, `memRegionUsed/Peak`, `memSafeRead`,
  `memCachePreEvictForNextScene`, `MEM_REQUIRE`, `ps1IsMainContext`.
- 4-byte alignment.
- `_Static_assert(MEM_REGION_TOTAL ≤ 1.2 MB)`.
- BOOT bump-up region.
- TRANSIENT bump-down region with wholesale wipe at `memSceneReset`.
- **CACHE: real first-fit free-list** with O(1) free, in-place size
  headers. Allocator API works end-to-end.
- `memHalt` dispatches between `JC_BSOD` (graphics up) and
  `ps1DebugError` (pre-graphics) via `graphicsIsInitialized()` flag.
- `graphicsIsInitialized()` + `memSetGraphicsReady()` added to
  `graphics_ps1.{c,h}`, set at end of `graphicsInit`.

**Boot integration (commit 6259dafcb):**
- `main()` calls `memInit` → `memVerify*` (all four) → ... after
  `ps1DebugInit`, before `cdromInit`.
- `fgRuntimeReset()` calls `memSceneReset` at top.
- Three-state machine for `fgLoopGetLastScene` (target / played /
  picking) — diagnostic continuity through the scene-pick window.
- `memCachePreEvictForNextScene` wired into main loop after
  `fgLoopApplyVariant`.

**Diagnostics (commit d0901916c):**
- `ps1Bsod` heap-probe block REPLACED with mem-region state reads via
  `memSafeRead` (PR9 defensive clamping). Drops `fgProbeLargestAlloc`
  and `fgGetFrameBufferBytes` lines; adds `memBoot/Cache/Transient
  Used/Peak` + `sceneAllocBalance`.
- `walkClean*` and `johnwalkSlot*` BSOD lines retained.

**Pack metrics (commit 355f3f0fb):**
- `scripts/generate-pack-metrics.py` reads
  `scene_analysis_output_2026-03-21.json` and produces
  `src/generated/pack_header_metrics.{h,c}`.
- 63 scenes enumerated. Heaviest TRANSIENT: MARY.ADS:1 at 236 KB
  (fits 260 KB budget). Heaviest CACHE pinned: MARY.ADS:1 at
  568 KB (fits 614 KB budget). All boot verifications pass.

**bsod-ui-test bootmodes (commit 6383f539f):**
- `bsod-ui-test-mem-boot` (pre-graphics ps1DebugError test).
- `bsod-ui-test-mem-cache` (post-first-scene JC_BSOD test).
- `bsod-ui-test-mem-transient` (same).

### Call-site migrations (~20 sites done)

**TRANSIENT region (per-scene):**
- [x] `gFgRuntime.soundEvents` (1a9cfdca0)
- [x] `gFgRuntime.entryTable.entries` (e06a1f8b3)
- [x] `gFgSetupSegmentBuffer` (2 alloc paths, 2 free paths) (f95ca9817)
- [x] Clean-rect snapshots (`gGrCleanRects[i].pixels`, atomic-alloc
      loop + fail path + reset path) (542aa118e)
- [x] Blit temp buffer (`tempBuf` in graphics_ps1.c:1373) (a6920e2e6)

**BOOT region (one-shot at boot, never freed):**
- [x] `primitiveBuffer[0]/[1]` GPU ordering tables (9f822468f)
- [x] `gWalkCleanBuf` (lazy + walkPilotInit paths) (9f822468f)
- [x] `grNewEmptyBackground` `PS1Surface` (a6920e2e6)
- [x] PSB sprite frame `PS1Surface` (a6920e2e6)
- [x] BMP frame `PS1Surface` (a6920e2e6)

**Other infrastructure:**
- [x] CACHE freelist implementation (137ea96b3)
- [x] `pack_header_metrics.{h,c}` generated (355f3f0fb)

### ❌ Not migrated (intentional or blocked)

**Grow-only buffers (deferred — already resident, not fragmenting):**

The plan called these BOOT-region candidates pre-sized via pack scan.
The pack-scan generator doesn't yet enumerate frame-buffer maxes
(set as 0 in the metrics header). Migrating without pre-size data
would require either:
- A guess at worst-case (wasted RAM if too large, halts if too small)
- Or accepting "grow-on-first-use" semantics within BOOT (leaks the
  smaller earlier allocations as the buffer grows scene-by-scene)

Neither is acceptable. Left on libc malloc until pack-scan can
populate the maxes:
- [ ] `gFgFrameBuffer`
- [ ] `gFgPrefetchFrameBuffer`
- [ ] `gFgStreamScratch`
- [ ] `gFgStreamWindowBuffer`

**Resource data blobs (blocked on CACHE eviction integration):**

The plan wants `uncompressedData` blobs (BMP/TTM/SCR/ADS) in CACHE
with LRU eviction. On PS1, these are loaded via `ps1_streamRead` in
cdrom_ps1.c. Migration requires:
- ps1_streamRead callers split: cache-data callers vs scratch callers
- The LRU evictor (`resource.c:checkMemoryBudget`) routed through
  `memFree(CACHE, ...)` instead of libc `free()`

Left on libc to avoid the latent free-on-region-pointer bug that
nearly shipped (see commit dae67c25a). Site of the largest single
remaining win — implementing this safely is the next-most-impactful
work.

**CD-side sector buffers (low impact):**

cdrom_ps1.c has ~10 sector-buffer mallocs. All are balanced inline
(alloc → read → memcpy → free in one function). Not fragmenting.
Migration is low value.

## Phase 2: Bandaid removal

### ✅ Landed (5 of 23 manifest items)

- [x] #1 `JCSKIP pack-start-failed` → JC_BSOD (bde34163f)
- [x] #2 `JCSKIP draw-bounds-failed` → JC_BSOD (bde34163f)
- [x] #3 `JCSKIP clean-rect-alloc-failed` → JC_BSOD (bde34163f)
- [x] #10-13 `findXxxResource` PS1 NULL-returns → fatalError on both
      platforms (89f1a664b)
- [x] #19 `walk_pilot.c` JOHNWALK silent-bail-out → JC_BSOD (f155aa797)
- [x] #21 `cdrom_ps1.c` malloc-fail NULL returns (3 sites) → JC_BSOD
      (c212f4614)
- [x] #22 `ps1PerfMarkAllocFail` no-op shim (a5a67ee47)

### ❌ Not yet landed

- [ ] #4 "Graceful skip" comment block — deleted along with item #1
      but a comment somewhere may remain
- [ ] #5 `fgDropOptionalPrefetchBuffersForCleanSnapshot` + callers
- [ ] #6 `fgDropPressureCachesForCleanSnapshot`
- [ ] #7 `fgBackdropSaveCleanBgRectsWithPressureFallback` → plain
      `fgBackdropSaveCleanBgRects`
- [ ] #8 `JCSTREAM prealloc-failed` printfs (foreground_pilot.c:1471,1482)
- [ ] #9 `fgPrePrimeStreamBuffers` lazy/eager paths
- [ ] #14-16 `ads.c` skip-scene paths (ads.c is NOT in PS1 build — N/A)
- [ ] #17 `walk_pilot.c:108-117` walkClean buf silent-skip — buf is
      now BOOT-allocated, halt-on-exhaustion. The silent-skip code is
      unreachable but not yet textually removed.
- [ ] #18 `walk_pilot.c:165-176` walkPilotInit soft return — same.
- [ ] #20 `graphics.c:1370-1395` "Pool exhausted - fall back" — in
      graphics.c (PC build), not PS1.
- [ ] #23 `ps1PerfMarkFallback` family — pending graphics-fallback
      audit.

## Phase 3: Audit

- [x] pause_menu.c — clean (no malloc).
- [x] ps1_captions.c — clean (no malloc).
- [ ] ttm.c — not in PS1 build, skip.
- [ ] sound_ps1.c — has VAG load mallocs (2 sites), balanced inline.

## Still TODO from plan v9

- [ ] **`memFreezeBoot()` enable** — gated pending full BOOT call-site
      audit. Several surfaces (grNewEmptyBackground, PSB/BMP frame
      surfaces) may allocate lazily during resource loading rather
      than at strict boot time. Activating the freeze prematurely
      would halt the game on first scene.
- [ ] **`src/malloc_poison.h`** — project-level enforcement. Cannot
      enable until grow-only buffers + ps1_streamRead are migrated;
      otherwise legitimate libc malloc/free calls would break the
      build.
- [ ] **`scripts/check-mem-region-rationale.py`** + fixtures —
      validates `MEM_REGION_RATIONALE` comments on migrated sites.
- [ ] **ISR-safety unit test** — exercises `ps1IsMainContext()` from
      a synthetic VBlank callback.
- [ ] **Pin-count delta logging** — debug-build instrumentation at
      scene transitions.
- [ ] **Holiday-variant pack enumeration** in
      `pack_header_metrics.{h,c}` — currently base scenes only.

## Build state

```
$ ./scripts/build-ps1.sh
...
=== Build complete ===
-rw-r--r-- 1 root root 218K  build-ps1/jcreborn.exe
```

Zero warnings. Pre-allocator binary was 212 KB; current is 218 KB
(+6 KB for allocator infrastructure + boot proofs + 63-scene metrics
table + state machine).

## Branch history (implementation commits)

```
6383f539f mem-region: Phase 1 step 16 — bsod-ui-test-mem-* bootmodes
355f3f0fb mem-region: Phase 1 step 5 — pack metrics generator
dae67c25a mem-region: revert safe_malloc → BOOT routing (bug fix)
d8a0c4c92 mem-region: gate memFreezeBoot pending audit
5a0f79099 mem-region: wire memFreezeBoot (reverted next commit)
a5a67ee47 mem-region: ps1PerfMarkAllocFail no-op shim
c212f4614 mem-region: cdrom_ps1 NULL-return → JC_BSOD
f155aa797 mem-region: walk_pilot JOHNWALK → JC_BSOD
89f1a664b mem-region: findXxxResource not-found handling
bde34163f mem-region: delete 3 JCSKIP paths
137ea96b3 mem-region: CACHE first-fit free-list
a6920e2e6 mem-region: graphics_ps1 PS1Surface migrations
9f822468f mem-region: safe_malloc + primitive + walk buf migrations (partial revert later)
542aa118e mem-region: clean-rect snapshots → TRANSIENT
f95ca9817 mem-region: gFgSetupSegmentBuffer → TRANSIENT
a1d9bd0f4 docs: Phase 1 status doc
e06a1f8b3 mem-region: entryTable → TRANSIENT
1a9cfdca0 mem-region: soundEvents → TRANSIENT
2b6216894 mem-region: pre-evict wire-up
d0901916c mem-region: ps1Bsod heap-probe replacement
6259dafcb mem-region: memInit + memSceneReset + fgLoopGetLastScene
756f93782 mem-region: Phase 1 foundation
```
