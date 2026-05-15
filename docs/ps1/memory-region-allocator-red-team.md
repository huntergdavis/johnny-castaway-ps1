# Memory region allocator — red team

Companion to [memory-region-allocator-plan.md](./memory-region-allocator-plan.md).

This document attacks the plan as currently written. Items are graded:

- **🔴 Showstopper.** The plan as written won't work; we must change it.
- **🟠 Material risk.** Plan can ship as written but a known failure mode is
  going to bite us; mitigate before code goes to PS1 hardware.
- **🟡 Caveat / smell.** Worth knowing about; not a blocker.

---

## 🔴 1. PERM cannot hold the grow-only frame buffers

The plan assigns `gFgFrameBuffer`, `gFgPrefetchFrameBuffer`,
`gFgStreamWindowBuffer`, `gFgStreamScratch` to PERM. PERM is a strict bump
allocator: once a region above it is used, you can't grow PERM downward without
moving everything.

Two facts kill this assignment:

1. **These buffers grow over the first 2-3 scenes.** Comment block at
   `foreground_pilot.c:1267-1287` documents that the buffers stabilize after a
   few scenes "as bigger packs come along." A bump allocator can't grow them
   in-place — the first scene allocates 100 KB, scene 2 needs 150 KB, but
   permTop has already been frozen by other PERM allocs above 100 KB.
2. **The pressure-drop fallback actively frees them mid-play.**
   `fgDropOptionalPrefetchBuffersForCleanSnapshot()`
   (`foreground_pilot.c:1383`) calls `free(gFgPrefetchFrameBuffer)` and
   `free(gFgStreamWindowBuffer)` when a clean-rect allocation needs space.
   This is called from at least four sites
   (`foreground_pilot.c:1183, 1908, 3242, 3826`). With these buffers in PERM,
   we lose the entire mid-play memory-relief mechanism — which is precisely
   what prevents `JCSKIP clean-rect-alloc-failed`, the regression we promised
   to never reintroduce.

**Mitigations to consider:**

- **A. Pre-size at boot.** Run a one-time scan of all FG2 pack headers at
  startup, learn the max frameBuffer size needed across the 63-scene set,
  allocate that in PERM once. Eliminates the grow path. Cost: ~50 ms boot
  hit and a max-of-max worst-case footprint (probably ~150 KB) that smaller
  scenes don't actually need.
- **B. Move them into RES.** They're not really permanent; they're
  long-lived-but-evictable. RES's free-list handles grow + pressure-drop
  cleanly. Cost: RES budget goes up by ~150-200 KB and frame buffers compete
  with resource bytes for slots.
- **C. Carve a small grow-only sub-region inside PERM with explicit "drop"
  semantics.** PERM_BUMP (one-shot) + PERM_GROW (frame buffers only, ~200 KB
  reserved). Adds complexity but stays in PERM conceptually.

**Recommendation:** A. The 200 KB worst-case footprint we'd reserve in PERM
is the same 200 KB we'd reserve in RES under B. The one-time scan is cheap
because pack headers are tiny (~32 bytes). And A eliminates the pressure-drop
*requirement* entirely — if the buffer is sized at max from boot, we never
need to drop it.

---

## 🔴 2. Mid-play clean-rect allocation has no fallback under SCENE bump

The plan puts `grSaveCleanBgRects` snapshots in SCENE. The current code path
for these is `fgBackdropSaveCleanBgRectsWithPressureFallback()`
(`foreground_pilot.c:3826`):

```
attempt alloc
if fails → fgDropPressureCachesForCleanSnapshot() drops optional buffers
attempt alloc again
if fails → JCSKIP clean-rect-alloc-failed
```

A bump allocator has no "reclaim" semantics. If SCENE has 100 KB free at the
moment the clean-rect tries to allocate 181 KB, dropping other SCENE
allocations doesn't help — the bump pointer doesn't roll back, those
allocations' addresses are still considered live by whoever holds the
pointers.

**Mitigations:**

- **A. Allocate clean-rect FIRST in SCENE setup, before anything else.** The
  current sequence allocates setup buffers, then clean-rect later. Reordering
  guarantees clean-rect always gets first dibs on SCENE. Cost: requires
  refactoring `foregroundPilotRuntimeStart`'s sequencing.
- **B. Reserve a clean-rect slot inside SCENE at fixed offset.** Sub-allocate
  the top 200 KB of SCENE exclusively for clean-rect. Other SCENE allocs
  can't touch it. Cost: 200 KB always reserved even when the scene doesn't
  need clean-rect.
- **C. Move clean-rect to RES.** Treat the snapshot as a resource. Cost:
  competes with resource bytes for RES space; LRU could evict it mid-scene
  unless explicitly pinned.

**Recommendation:** A. The current code already has the order roughly right
(`fgBackdropSaveCleanBgRects` runs during setup, before per-frame work
starts). A small reordering to allocate clean-rect immediately after `memSceneReset`
runs gives it the entire 700 KB SCENE region to allocate into. The setup
buffer comes next; the rest of SCENE follows.

---

## 🔴 3. The budget arithmetic doesn't close

Plan claims SCENE = 700 KB. Documented numbers from
`docs/ps1/archaeology/memory-constraints.md`:

- Heaviest scene (MARY.ADS tag1) peak: 555.3 KB
- FISHING 1 clean-rect: 181 KB
- BACKGRND.PSB read-path peak: 186 KB (it allocates twice)

555 + 181 = **736 KB** if those allocations are simultaneously live. The plan
doesn't say whether the 555 KB scene peak already includes clean-rect, or
includes BACKGRND.PSB's double-allocation, or neither.

**Action required before phase 1:** verify what the 555.3 KB scene-peak
number actually contains. The raw data is in
`docs/ps1/research/generated/scene_analysis_output_2026-03-21.json` — read
it and decompose. If clean-rect is additive, SCENE budget needs to be 800 KB
minimum, which leaves only 500 KB total for PERM+RES — uncomfortably tight.

---

## 🟠 4. PSX usable-RAM assumption is unverified

Plan says "1.6-1.8 MB usable" after kernel + stack + exe. This is from
folklore, not measurement. Actual breakdown:

- PSX kernel reserves `0x80000000-0x8000FFFF` (64 KB).
- Exe loads at `0x80010000`; the current jc_reborn PSX exe size is unknown
  to this plan but is probably 200-400 KB based on similar projects.
- Stack lives at the top of RAM, growing down from `0x801FFFF8`. Default
  stack size is small but the build may have tuned it.
- BSS section sits between exe end and the heap.

If we declare a 1.8 MB static buffer in BSS, and the exe is 300 KB plus
64 KB stack plus 64 KB kernel = ~430 KB overhead, we have 1.62 MB. **The
1.8 MB plan budget overflows.**

**Action required before phase 1:** read the linker map from a current
`./scripts/build-ps1.sh` build. Locate the exact BSS top, stack bottom,
exe size. Set the static buffer size to `(stack_bottom - bss_top) - safety_margin`.
This is also a great `static_assert` to put in `mem_region.c`.

---

## 🟠 5. Default-to-PERM in migration is unsafe

Plan says "Existing callers that haven't been migrated yet land in PERM by
default (safe overestimate during the migration; revisit each in phase 2/3)."

This is **the opposite of safe.** PERM never frees. Any unmigrated per-scene
allocation that defaults to PERM leaks permanently. After 10 scenes, PERM
could be many MB. Phase 1 would brick the PS1 build the moment a per-scene
alloc runs.

**Mitigation:** default unmigrated calls to **SCENE**, not PERM. If a
permanent allocation accidentally lands in SCENE, the wipe at scene
boundary kills it, and the next request crashes obviously. That's a *loud*
failure during development, which is what we want. Defaulting to PERM
gives *silent* leaks, which is the worst outcome.

The cleaner fix: don't default at all. Make `memAlloc` require an explicit
region argument and migrate every call site in phase 1 with grep + sed.
34 `safe_malloc` sites + 5 raw `malloc` sites = ~39 call sites total. Tractable.

---

## 🟠 6. PC stub has unlimited malloc — PS1-only failures won't surface on PC

The hybrid API forwards PC allocations to libc. PC has gigabytes of RAM, so
a PS1-sized budget violation never fires on PC. Result: PC tests are green
while PS1 hardware silently overruns the region and crashes.

**Mitigation:** the PC stub takes the same compile-time budget constants
as PS1. PC's `memAlloc` decrements a counter on each request and returns
NULL when the region's budget is exhausted, mirroring the PS1 failure
mode. PC keeps malloc/free for valgrind/ASan visibility on the actual
bytes, but the *budget gate* is enforced identically on both platforms.

Optional dev affordance: a `JC_MEM_SIMULATE_PS1_BUDGET=1` env var that
toggles the strict mode on PC. Off by default, on in CI.

---

## 🟠 7. `grNewLayer()` is already a pool — the plan mis-classified `ttmLayer`

Plan's Phase 2 table lists `ads.c:1134 ttmLayer per-scene` as SCENE. But the
allocation goes through `grNewLayer()` (`graphics.c:1280`), which the
comment at line 1278 describes as "Replaces grNewLayer() with pooled
allocation." It's a 4-slot pool, set up at init. **The pool itself is
PERM; individual layer handouts/returns are pool ops, not malloc/free.**

Touching `ads.c:1134` doesn't help — there's no malloc there to migrate.
Drop this row from Phase 2's table.

---

## 🟠 8. `safe_malloc(640 * 480)` for `grBackgroundSfc` is a 300 KB allocation we didn't account for

`graphics.c:1884` allocates a 307,200-byte buffer for `grBackgroundSfc` —
the full-screen indexed background surface. It looks lifecycle-PERM
(allocated once at graphics init, lives forever), which is fine for the
plan's classification system. **But the plan didn't enumerate it.** Worth
adding to Phase 3's table and confirming it really is one-shot.

If it's reallocated under some path (e.g., when night/day toggles or
screen size changes), PERM is wrong.

---

## 🟠 9. Resource-struct PERM growth during boot is more involved than "one-shot at boot"

The plan says `resource.c:158-499` struct allocations go in PERM. True, but
they don't happen all at once — they're interleaved with file reads as the
catalog parses. Several allocations per resource (struct + sub-arrays of
tags/res entries), times ~100+ resources.

That's fine for PERM in principle, but it means PERM is being actively
allocated *throughout* boot, not in one front-loaded chunk. If anything
else (audio mixer init, font load, etc.) wants PERM in the middle of
resource parsing, the order is fragile.

**Mitigation:** document the PERM allocation order explicitly in
`mem_region.c`'s header comment. Phase 3 PR notes should include the order:
(1) `memInit`, (2) resource catalog parse, (3) audio init, (4) font init,
(5) graphics surface pool, (6) walk clean buf — first scene boots after
that. Any code that wants PERM after step 6 needs review.

---

## 🟠 10. RES eviction → re-alloc feedback loop can spin or deadlock

Plan: "RES's internal allocator will call back into the evictor if it can't
satisfy a request from its own free-list — retry once." But:

- If all resources in RES are pinned (legitimate worst case — multiple
  active TTM threads can pin overlapping sets), the evictor has nothing to
  evict. Retry returns same failure.
- The current code path on alloc failure is "return NULL, caller decides."
  The callers vary: some return NULL up the chain, some `fatalError`, some
  trigger JCSKIP. Need a uniform failure contract.

**Mitigation:** define the failure contract explicitly. `memAlloc` returns
NULL on failure; the caller is responsible. No silent fallbacks inside
the allocator. The eviction retry happens at most once, and only when
the LRU can identify *some* unpinned resource to drop. If it can't,
return NULL immediately — no spin. JCSKIP triggers naturally from
existing call-site logic.

---

## 🟡 11. Alignment is not in the design

PSX MIPS R3000A requires 4-byte alignment for word loads/stores, 8-byte
for doubles. The plan doesn't mention bump alignment. Default bump-alloc
behavior often returns whatever the next byte happens to be. SIGBUS
risk for misaligned struct loads.

**Mitigation:** trivial. `memAlloc` rounds size up to 4-byte (or 8-byte)
multiple before bump. ~2 instructions of overhead, hidden behind the
existing alloc cost. Document in `memAlloc` comment.

---

## 🟡 12. "Pin-count" terminology collides with LRU's `pinCount`

The plan introduces an outstanding-SCENE-allocation counter and calls it
"pin count." The LRU cache already has `pinCount` on each resource
(`resource.c:781, 793` etc.) meaning "is this resource currently in use by
an active TTM thread." Two unrelated concepts with the same name in
overlapping files.

**Mitigation:** rename the SCENE counter to `outstandingSceneAllocations`
or `sceneAllocBalance`. Pure naming hygiene; no functional impact.

---

## 🟡 13. Telemetry log volume

JCMEM at every scene transition: 63 scenes × 20 iterations = 1260 lines
per soak test. JCPERF already emits similar volume. Probably fine but
contributes to log churn. Consider gating behind the existing
`FG_HEAP_PROBE_LOGS` flag rather than always-on.

---

## 🟡 14. `fgRuntimeReset()` runs at scene-START, not scene-END

The plan says "wipe between scenes," which is technically correct but
slightly misleading. `fgRuntimeReset()` is actually called at the *top* of
the next scene's `foregroundPilotRuntimeStart`. So the SCENE region holds
the previous scene's bytes until the next scene starts, not until the
current one ends.

This is semantically equivalent for our purposes (nothing reads SCENE
between scene-end and next-scene-start), but the telemetry timestamps
will read oddly: the "wipe=423K" line for scene N appears at the
beginning of scene N+1's log. Not a bug, just a docs gotcha.

Also: `fgRuntimeReset()` is **also** called on the JCSKIP failure path
(`foreground_pilot.c:3777`), which means partial allocations from a
failed scene-start are cleaned up by our `memSceneReset` on the next
attempt. Good outcome.

---

## 🟡 15. Pause-menu allocations

The pause menu (`pause_menu.c`) isn't surveyed in this plan. If it
allocates (likely for menu state, font glyph buffers, etc.) it needs a
region. Probably PERM (menu state is long-lived). Worth a quick audit
during Phase 3.

---

## 🟡 16. Boot ordering: anything before `memInit()`?

Plan implies `memInit()` is called once at startup. But what about static
initializers, global constructors (C++ only — we're C, so not an issue),
or BIOS/SDK callbacks that fire before `main()`? On PS1, the PsyQ runtime
does some setup before `main`. If any of that calls `safe_malloc`, it
runs before `memInit` and crashes.

**Mitigation:** `memInit()` MUST be the very first call in `main()`. Add a
guard: if `memAlloc` is called before `memInit`, fatalError immediately.
That makes a "ran before init" bug crash loud and early.

---

## 🟡 17. ADS uncompressedData lazy loading

`resource.c:241` sets `adsResource->uncompressedData = NULL` at parse time
("ADS lazy loading: Don't decompress at startup, just skip the compressed
data"). Actual decompression happens on first play in `ads.c`. So ADS data
enters RES *not* at boot but at first-scene-play. That's fine — RES
supports it — but the plan should note this. Otherwise reviewers think
the resource catalog is fully PERM at boot, which it isn't for ADS.

---

## Summary of required changes before Phase 1 implementation

1. **🔴 Frame buffers (gFg*)**: pre-size at boot via pack-header scan,
   put in PERM at known max. Eliminates grow + pressure-drop need.
2. **🔴 Clean-rect ordering**: reorder `foregroundPilotRuntimeStart` so
   clean-rect allocates first in SCENE setup.
3. **🔴 SCENE budget**: re-derive from raw scene_analysis JSON; confirm
   555 + 181 isn't simultaneous, or raise SCENE budget to 800 KB.
4. **🟠 Static-buffer total size**: measure actual usable RAM from linker
   map, encode as a `static_assert`.
5. **🟠 Migration default**: untagged calls go to SCENE, not PERM.
6. **🟠 PC stub budget enforcement**: same compile-time budget as PS1.
7. **🟠 RES failure contract**: NULL return, at-most-one eviction retry,
   no silent fallbacks.
8. **🟠 Drop ttmLayer from Phase 2 table** — it's already a pool.
9. **🟠 Add grBackgroundSfc (300 KB) to Phase 3 table**.

The 🟡 items can be handled inside the implementation work without
re-spinning the plan.
