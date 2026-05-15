# Memory region allocator — Phase 1 implementation status

Companion to [memory-region-allocator-plan.md](./memory-region-allocator-plan.md).
Tracks Phase 1 implementation progress against the 28-step plan. Each
item links to its commit when landed.

**Branch:** `claude/memory-region-allocator`
**Last build:** clean, 0 warnings, jcreborn.exe = 214 KB

## Landed in this branch

### Foundation (commit 756f93782)

- [x] **Step 1** — New files: `src/mem_region.{c,h}`,
      `src/mem_region_extern.h`, `src/generated/pack_header_metrics.{c,h}`
      (stub). Allocator core compiles cleanly under PS1 build.
- [x] **Step 2** — PS1 implementation present. PC test build not
      yet wired (test target only links specific files via tests/Makefile).
- [x] **Step 3** — CACHE non-recursive eviction signature in place
      (TODO: real free-list — see "Stubs / TODO" below).
- [x] **Step 4** — 4-byte alignment (`MEM_REGION_ALIGN = 4`).
- [x] **Step 6** — `memHalt` primitive implemented; internally
      dispatches between `ps1Bsod` (graphics up) and `ps1DebugError`
      (pre-graphics) via `graphicsIsInitialized()`.
- [x] **Step 7** — `formatHaltReason` helper in `mem_region.c` with
      `volatile` static buffer + depth-counter re-entry guard.
- [x] **Step 8** — `MEM_REQUIRE` macro defined in `mem_region.h`;
      hand-rolled (does NOT use `<assert.h>`); ships in release.
      `ps1IsMainContext()` implemented in `mem_region.c` via `mfc0`
      inline asm.

### Boot integration (commit 6259dafcb)

- [x] **Step 11** — Boot sequence wired in `main()`:
      `ps1DebugInit() → memInit() → memVerify* → cdromInit → ...`.
      All four `memVerify*` checks run at boot; trivially pass against
      the stub `pack_header_metrics`.
- [x] **Step 13 (partial)** — `memSceneReset()` hooked into
      `fgRuntimeReset()` at top, before existing per-scene state
      clearing. `memCachePreEvictForNextScene()` wired after
      `fgLoopApplyVariant` in the main scene loop.
- [x] **Step 19** — `fgLoopGetLastScene()` implemented with three-
      state machine (`pickInProgress` flag + `lastTarget` + `lastPlayed`).
      Updated by `fgLoopNextScene` (target) and `fgLoopMarkScenePlayed`
      (after successful play).
- [x] **Step 19b** — `ps1IsMainContext()` implementation via
      `mfc0 $13` (CAUSE) and `mfc0 $12` (SR); bit-pattern check per
      PSX programmer's manual.

### ps1Bsod replacement (commit d0901916c)

- [x] **Step 9** — `ps1Bsod` heap-probe block REPLACED (not extended).
      Drops `fgProbeLargestAlloc`, `fgGetFrameBufferBytes`,
      `fgGetPrefetchFrameBufferBytes` lines. Adds `memBootUsed/Peak`,
      `memCacheUsed/Peak`, `memTransientUsed/Peak`, `sceneAllocBalance`
      via `memSafeRead` (PR9 defensive clamping). Keeps
      `walkCleanAlloc/KB` and `johnwalkSlotLoaded` lines (still
      meaningful — walk buffer persists in BOOT).

### Graphics readiness flag (commit 756f93782 / part of foundation)

- [x] `graphicsIsInitialized()` predicate + `gGraphicsReady` flag set
      at end of `graphicsInit()`. Drives `memHalt`'s graphics-vs-
      pre-graphics dispatch.

## Call-site migration progress (Step 12)

**2 of 39 sites migrated as proof of pattern. Build clean at each step.**

### Migrated

- [x] `gFgRuntime.soundEvents` (foreground_pilot.c:1609) → TRANSIENT
      (commit 1a9cfdca0). Per-scene sound-event table. NULL-return
      failure path deleted (memAlloc halts on exhaustion).
- [x] `gFgRuntime.entryTable.entries` (foreground_pilot.c:947) →
      TRANSIENT (commit e06a1f8b3). Per-scene frame-metadata table.
      Same pattern as soundEvents.

### NOT YET landed in this branch

**Per-scene TRANSIENT, simple alloc/free pattern** (low risk, follow the
soundEvents/entryTable template):

- [ ] `expanded` scratch in fgLoadMetadataPrefix (foreground_pilot.c:1034)
- [ ] CD sector buffer (foreground_pilot.c:1245)

**Per-scene TRANSIENT with reuse-on-capacity logic** (medium risk — the
existing "buffer already big enough, skip realloc" pattern doesn't work
under bump-only semantics; needs careful refactor + dangling-pointer
clearing in fgRuntimeReset):

- [ ] `gFgSetupSegmentBuffer` (foreground_pilot.c:1332)
- [ ] Clean-rect snapshots (`gGrCleanRects[i].pixels`, graphics_ps1.c:3645;
      ~6 slots, atomic-allocation semantics, up to ~181 KB)

**BOOT region** (pre-sized at boot via pack-header scan):

- [ ] `gFgFrameBuffer` (foreground_pilot.c:1470)
- [ ] `gFgPrefetchFrameBuffer`
- [ ] `gFgStreamScratch` (foreground_pilot.c:1481)
- [ ] `gFgStreamWindowBuffer`
- [ ] `gWalkCleanBuf` (walk_pilot.c:108)
- [ ] `grBackgroundSfc` backing (graphics_ps1.c side)
- [ ] Resource catalog struct arrays in resource.c parse functions

**CACHE region** (LRU-managed resource cache; depends on real CACHE
free-list landing first):

- [ ] Resource `uncompressedData` blobs in resource.c parse paths
- [ ] Per-ADS uncompressedData (ads.c lazy decompress)

**Other**:

- [ ] `MEM_REGION_RATIONALE: ...` comment above every NEW migrated call site
- [ ] `INIT_*` annotation per call site
- [ ] Audit other src/ files not yet inspected (pause_menu.c, ps1_captions.c,
      memcard.c, walk_pilot.c, scene_freeplay.c)

### Phase 1 sub-steps still to do

- [ ] **Step 5** — `scripts/generate-pack-metrics.py` + filled-in
      `src/generated/pack_header_metrics.{c,h}` (current files are
      stubs with zero scenes).
- [ ] **Step 10** — `fatalError` upgraded to use `ps1DebugError`
      directly instead of plain printf+while(1).
- [ ] **Step 13 (rest)** — `memFreezeBoot()` call placement in `main()`
      after all boot-time BOOT allocations (audio init, font, etc.).
      Cannot call yet — no BOOT allocations migrated, so all current
      libc-malloc paths would be unaffected. After migration, BOOT
      freeze halts on any subsequent BOOT alloc attempt.
- [ ] **Step 14** — Reorder `foregroundPilotRuntimeStart` so clean-rect
      allocates first in TRANSIENT order.
- [ ] **Step 15** — Project-level `src/malloc_poison.h`. Every `.c`
      in `src/` includes it; libc malloc/free become compile errors.
- [ ] **Step 16** — Three `bsod-ui-test-mem-*` bootmodes for visual QA.
- [ ] **Step 17** — CI checks (`MEM_DEV_BUILD=0`, grep gates,
      pack-hash currency).
- [ ] **Step 18** — PR template `MEM_DEV_BUILD=0` checkbox.
- [ ] **Step 19a** — `fgLoopApplyVariant` signature change to return
      effective scene name; pre-evict uses returned name not input.
- [ ] **Step 20** — `scripts/check-mem-region-rationale.py` Python
      script enforcing RATIONALE comments.
- [ ] **Step 21** — CI count-match gate
      (`MEM_REGION_*` count in `mem_region.h` == decision tree).
- [ ] **Step 22** — `JC_VERIFY_PACK_HASHES` build-flag toggling
      (currently defined-out by default).
- [ ] **Step 23** — `-Wglobal-constructors -Werror=global-constructors`
      in CMakeLists.txt.
- [ ] **Step 25** — `mem_region_extern.h` exists but Phase 1 should
      audit which symbols actually need it vs which can come from
      `mem_region.h` directly. Currently includes all four diagnostic
      reads.
- [ ] **Step 26** — Pin-count delta logging at scene transitions
      (debug builds).
- [ ] **Step 27** — Python script CI fixtures (5 cases).
- [ ] **Step 28** — Toolchain compat notes; the `-Werror` flag is
      assumed but not yet enabled.

### Stubs / TODO inside landed code

- **CACHE sub-allocator is a bump stub.** Real implementation
  (segregated free-list with non-recursive LRU eviction) is in
  `cacheAllocInternal`/`cacheFreeInternal` as a `TODO(phase-1)` block.
  Game code using CACHE today would bump until exhaustion; no resource
  cache wiring exists yet.
- **`memCachePreEvictForNextScene` is a no-op** pending the
  CACHE free-list + LRU integration.
- **`memVerifyPackHashes` is a stub** pending CRC-32 implementation +
  real pack-header data.

## Pre-Phase-1 gates

- [x] **Linker map `_end ≈ 629 KB`** — verified at start of this
      branch; baked into budget table.
- [x] **`scene_analysis_current.json`** — referenced as source of
      truth; per-scene peak data examined for budget sizing.
- [ ] **`grBackgroundSfc` one-shot** — needs verification in Phase 3
      audit pass.
- [x] **ISR-safety audit** — `ps1IsMainContext()` runtime check
      lands the enforcement; full callback-graph audit still
      recommended pre-merge.
- [ ] **`ttm.c` opcode handler audit** — needed before declaring all
      allocation sites known.
- [x] **BIOS/PsyQ allocation audit** — covered in plan v9 budget
      table (~60 KB for padmgr + memcard).
- [ ] **Pinned-set math vs runtime** — moved to Phase 1 milestone;
      needs both verifier + runtime instrumentation to compare.

## Phase 2 (deferred)

The 23-site bandaid + skip-code removal manifest in the plan does
NOT begin until Phase 1's call-site migration is complete. Phase 2's
first PR (P2.0 — `ps1_debug.c` heap-probe replacement + `memHalt`
implementation) is partly done in commit d0901916c, but the manifest's
remaining 22 sites (JCSKIP paths, NULL-return guards in resource.c,
silent skips in ads.c/walk_pilot.c, surface-pool fallback,
`ps1PerfMarkAllocFail`, etc.) await Phase 1's migration to provide
the `memAlloc` call sites those bandaids were guarding.

## Build state

```
$ ./scripts/build-ps1.sh
...
=== Build complete ===
-rw-r--r-- 1 root root 214K  build-ps1/jcreborn.exe
```

0 warnings. Pre-allocator binary was 212 KB; +2 KB for allocator
infrastructure + state machine + boot integration.
