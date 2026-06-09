# Memory region allocator — Phase 1 implementation checklist

Companion to [memory-region-allocator-plan.md](./memory-region-allocator-plan.md).
Mirrors the 24 steps of Phase 1 as a ticked TODO list. Each item names
its completion criteria. Use as a PR self-review against the plan.

Pre-implementation gates (must close before opening the Phase 1 PR):

- [ ] **PS1 binary rebuilt; `_end ≈ 629 KB` confirmed** in fresh map.
- [ ] **`scene_analysis_current.json` `peak_memory_bytes` decomposed**;
      confirmed to include BMP + TTM + SCR data.
- [ ] **`grBackgroundSfc` (graphics.c:1884) lifecycle audit:** one-shot,
      no realloc paths.
- [ ] **ISR-registration audit:** no path from any callback reaches the
      planned `memAlloc` sites.
- [ ] **`ttm.c` opcode handler audit:** no hidden `safe_malloc` or
      `malloc` calls in opcode bodies.
- [ ] **BIOS/PsyQ allocation audit:** padmgr + memcard footprint
      verified ~60 KB; no runtime growth.
- [ ] **Pinned-set verifier-math = runtime-accounting:** instrumented
      debug build logs runtime pinned size per scene; equals verifier
      output.

Phase 1 implementation:

- [ ] **Step 1 — New files exist:**
      `src/mem_region.{c,h}`, `src/mem_region_verify.c`,
      `src/generated/pack_header_metrics.h`,
      `scripts/generate-pack-metrics.py`,
      `docs/ps1/mem-region-decision-tree.md`.
- [ ] **Step 2 — PC + PS1 implementations:** API parity confirmed by
      compiling both targets and running unit tests in PC.
- [ ] **Step 3 — CACHE non-recursive eviction:** `lruEvictUnpinned`
      returns bytes-freed, never calls `memFree`.
- [ ] **Step 4 — 4-byte alignment:** `MEM_REGION_ALIGN = 4` constant
      in `mem_region.h`; bump implementations round size up.
- [ ] **Step 5 — CRC-32 pack hashing:** Sarwate algorithm, table in
      `.rodata`. Verified via `objdump -h` that the table is in the
      read-only section.
- [ ] **Step 6 — `memHalt` primitive:** body matches plan's pseudocode;
      `__builtin_unreachable()` after each branch.
- [ ] **Step 7 — `formatReason`:** declared in `ps1_debug.h`;
      `volatile static` buffer + depth counter; emits
      `"[concurrent fatal]"` on re-entry.
- [ ] **Step 8 — `MEM_REQUIRE` macro:** hand-rolled, no `<assert.h>`
      dependency; ships in release.
- [ ] **Step 9 — `ps1Bsod` heap-probe REPLACED:** `fgProbeLargestAlloc`,
      `prefetchBufBytes` lines removed; `memBoot/Cache/TransientUsed/Peak`
      + `sceneAllocBalance` lines added via `memSafeRead`.
      `walkClean*` / `johnwalkSlot*` lines kept.
- [ ] **Step 10 — `fatalError` upgrade:** PS1 path renders via
      `ps1DebugError`, not bare `printf` + `while(1)`.
- [ ] **Step 11 — Boot integration:** `ps1DebugInit` precedes `memInit`
      (already true at `src/jc_reborn.c:1688`; confirmed not regressed).
      All `memVerify*` functions hooked. `-Wglobal-constructors
      -Werror=global-constructors` in PS1 build config.
- [ ] **Step 12 — 39 call sites migrated:** every `safe_malloc` /
      `malloc` (except the explicit whitelist) replaced with
      `memAlloc(REGION, n, "tag")` + `INIT_*` annotation comment.
- [ ] **Step 13 — Hooks:** `memSceneReset` at `fgRuntimeReset`
      (`src/foreground_pilot/foreground_pilot.c:1470`); `memCachePreEvictForNextScene`
      at `jc_reborn.c:~1956` (after `fgLoopApplyVariant`).
- [ ] **Step 14 — Clean-rect reorder:** clean-rect allocation is the
      first TRANSIENT alloc in `foregroundPilotRuntimeStart`.
- [ ] **Step 15 — `src/malloc_poison.h`:** every `.c` in `src/`
      includes it (or transitively includes a `src/common.h` that
      does); whitelist comments justify each exception.
- [ ] **Step 16 — Bootmodes:** `bsod-ui-test-mem-{boot,cache,transient}`
      added following the existing `bsod-test` bootmode pattern.
- [ ] **Step 17 — CI checks pass:**
      `MEM_DEV_BUILD=0`, `BSOD_UI_TEST_*=0`, grep gates clean,
      pack hashes match generated metrics.
- [ ] **Step 18 — PR template updated:** `MEM_DEV_BUILD=0` re-test
      checkbox is present and ticked.
- [ ] **Step 19 — `fgLoopGetLastScene()` implemented:** returns the
      slug of the most recently played scene; updated on each
      successful pick.
- [ ] **Step 20 — `MEM_REGION_RATIONALE:` comments:** every
      `memAlloc` call site has a justification one-liner.
- [ ] **Step 21 — Count-match CI:** `MEM_REGION_*` enum count in
      `mem_region.h` == occurrences in decision tree doc.
- [ ] **Step 22 — `JC_VERIFY_PACK_HASHES` build flag:** defaults off
      in release, on in dev/QA; release boot doesn't pay CD time.
- [ ] **Step 23 — `-Werror=global-constructors`:** in PS1 build flags,
      baseline build clean.
- [ ] **Step 24 — Phase-1 checklist** (this doc) included in the PR.

End-to-end validation (from the plan, run after merge to a long-lived
branch but before merge to main):

- [ ] PC + PS1 builds compile and link.
- [ ] `_Static_assert(MEM_REGION_TOTAL <= 1.2 MB)` passes.
- [ ] All four `memVerify*` checks pass at boot for every scene + variant.
- [ ] 20-iteration soak rotation: 0 `memHalt` / `JC_BSOD` / `fatalError`.
- [ ] Heap probe constant across rotation.
- [ ] Removal grep gate: zero hits for the regex from the plan's
      manifest.
- [ ] Full-`src/` audit gate: zero `malloc`/`free`/`calloc`/`realloc`
      hits outside whitelist.
- [ ] `sceneAllocBalance` never positive at reset (debug build).
- [ ] `memFreezeBoot` not triggered post-boot.
- [ ] PC valgrind clean.
- [ ] Per-scene frame-byte diff vs. baseline matches.
- [ ] DCache miss-rate measurement recorded (informational only).
