# Memory region allocator — red team v4 (panel re-review of v5)

Same five-reviewer panel reconvenes for the third pass. v5 closed 19 of
v3's 22 graded findings; this review focuses on **new issues** introduced
by the v5 fixes themselves. Loop 2 of the goal-driven iteration.

Items still graded 🔴 / 🟠 / 🟡.

---

## Reviewer 1 — Pat, Embedded Systems Veteran

### Closed in v5

- **P1-bis (ISR unconditional):** plan now says unconditional. Done in
  spirit — see new concern below about the actual mechanism.
- **P8 (ps1Bsod heap-probe replace):** Phase 1 step 9 is explicit. Done.
- **P9 (unified memHalt):** API spec includes it. Done.
- **P10 (PSX printf non-alloc):** pre-Phase-1 gate. Done.
- **P11 (watchdog/emulator):** not addressed in plan but it's a 🟡 and
  ultimately a comment; accept as-is.
- **P12 (CRC-32):** committed. Done.

### New concerns

#### 🔴 P13. `assert(ps1IsMainContext())` is NOT unconditional

v5 says: "ISR-safety check is unconditional (release builds too)." But
the plan still writes `assert(ps1IsMainContext())`. C's `assert()`
compiles to nothing under `NDEBUG`, which is the standard release
configuration. **The "unconditional" claim is false as written.**

Required fix: a hand-rolled macro that doesn't depend on `<assert.h>`:

```c
#define MEM_REQUIRE(cond) do { \
    if (!(cond)) memHalt("(mem_require)", "invariant: " #cond); \
} while (0)
```

Use `MEM_REQUIRE(ps1IsMainContext())` everywhere the plan currently
says `assert(...)`. **Loud, mandatory, ships in release.**

#### 🟠 P14. MIPS COP0 read latency for ps1IsMainContext is understated

Reading SR/cause via `mfc0` on R3000A has a load-delay slot and may
stall the pipeline depending on what surrounds it. Real cost is closer
to **8-12 cycles**, not 3. Update the perf table:

- `memAlloc(BOOT/TRANSIENT, n)`: ~18-22 cycles, not 13.
- `memAlloc(CACHE, n)` hit: ~33-37 cycles, not 28.

Still a win vs ~50-100 cycle libc malloc, but the margin is tighter.
Don't oversell.

#### 🟠 P15. CRC-32 table is 1 KB — accounted in BOOT budget?

CRC-32 via Sarwate's algorithm requires a 1 KB lookup table. Likely
read-only data in the binary's `.rodata`, which goes into the exe load
(not BSS), so it pushes `_end` up by 1 KB but doesn't eat the region
buffer. **Confirm and document** which section the table lands in. If
it's in `.data` or `.bss`, it would eat budget.

#### 🟡 P16. The `__builtin_unreachable()` after memHalt branches

memHalt's body has two no-return branches (ps1Bsod, ps1DebugError +
while(1)). Compiler should recognize both as `noreturn`, but if it
doesn't, it may emit a "control reaches end of non-void function"
warning. Add `__builtin_unreachable()` after each branch defensively.

---

## Reviewer 2 — Sarah, SRE / Operations

### Closed in v5

- **S7 (PR sequencing):** bundled with atomic commits. Done.
- **S8 (test-harness migration):** in commit P2.8. Done.
- **S9 (rename `bsod-ui-test-mem-*`):** done.
- **S10 (PR template checkbox):** Phase 1 step 18. Done.

### New concerns

#### 🟠 S11. Per-commit CI may fail on intermediate states of the bundled PR

Many CI systems run on every commit pushed to a PR branch, not just
the tip. The 8-atomic-commit bundle has intermediate states where:

- After P2.0 (ps1Bsod replaced) but before P2.1: `foreground_pilot.c`
  still has `JCSKIP` code that references the heap-probe externs no
  longer in ps1Bsod's output. Code compiles, but soak-test signal
  changes mid-bundle.
- After P2.2 (resource.c) but before P2.3 (ads.c): `findAdsResource`
  now memHalts but ads.c still has its silent-skip. The plan says
  these will be deleted, but during the bundle's intermediate state,
  ads.c's defensive NULL check is unreachable dead code.

**Choices:**
1. Disable per-commit CI for this PR (use CI-only-on-PR-tip; many
   systems support this).
2. Force-push the PR to a single merge commit when ready (kills the
   atomic-commit revert story).
3. Split into 2 PRs: infrastructure (P2.0 + memHalt + ps1Bsod) and
   cleanup (P2.1-2.8).

Option 1 is cleanest but team-dependent. Option 3 is most robust if
the team has any uncertainty about CI behavior.

#### 🟠 S12. The decision tree is documentation, not enforcement

Mateo's M9 was closed by writing the decision tree doc. But a developer
who picks the wrong region still gets a passing build (until the boot
proof catches it days later, or the sceneAllocBalance assert fires,
or a peer review catches it). **There's no automated enforcement.**

Options:
- A `// MEM_REGION_RATIONALE: <one-liner>` comment required on every
  `memAlloc` call site, grep'd by CI. Forces the dev to articulate
  the choice.
- A `make decision-tree-lint` script that grep's `memAlloc(.*BOOT` and
  flags any whose surrounding function name doesn't match boot-time
  patterns.

Neither is strictly necessary but both reduce the "wrong-region
silent landing" failure mode.

#### 🟡 S13. Tag in commit-bundle vs branch tag

Plan says "`pre-mem-region-bandaid-removal` is the rollback target."
But that's the **pre-Phase-2** commit, which is itself the tip of
Phase 1. If Phase 1 lands as one PR (which is implied), the tag points
to the merge commit. Confirm tag mechanics:
`git tag pre-mem-region-bandaid-removal <phase-1-merge-sha>` happens
before Phase 2's first commit. State this in the plan.

---

## Reviewer 3 — Mateo, Future Maintainer

### Closed in v5

- **M7 (unified primitive):** memHalt. Done.
- **M8 (new-region pattern):** companion doc updated. Done.
- **M9 (decision tree in Phase 1):** mem-region-decision-tree.md
  exists. Done.
- **M10 (formatReason location):** ps1_debug.h. Done.
- **M11 (decision tree too coarse):** decision tree now covers scratch,
  per-frame, multi-scene cases. Done.

### New concerns

#### 🟠 M12. Phase 1 grew to 18 steps; needs an implementation checklist

v3's Phase 1 was 11 steps. v5's is 18. They're densely cross-referenced
(step 9 references step 6, step 13 references step 5, etc.). For the
engineer who picks this up cold, that's a lot of context to hold.

Mitigation: a `docs/ps1/mem-region-phase-1-checklist.md` companion that
mirrors the 18 steps as ticked TODO items, with one paragraph each
on what "done" means for that step. Forces the implementer to
articulate completion per step and gives the reviewer something to
check off.

#### 🟠 M13. holiday-variant case isn't in the decision tree

If I'm adding a holiday variant of an existing scene, do I follow the
same workflow as a new scene? The companion doc (adding-new-scenes-
memory.md) addresses this in passing ("the generator walks every
reachable fgLoopApplyVariant branch automatically"), but the **decision
tree** doesn't mention variants. A maintainer adding a holiday only
reads the decision tree, sees no variant note, and assumes nothing
special is required.

Add a line in the decision tree: "If your allocation lifetime depends
on a holiday flag — same region as the base scene. If you're allocating
a per-variant resource (e.g., holiday-only sprite), that's CACHE."

#### 🟡 M14. New-region step 5 (decision tree update) is documentation, not gating

The new-region pattern in adding-new-scenes-memory.md says to update
the decision tree as step 5. But nothing enforces this — a future
contributor adds MEM_REGION_VRAM_MIRROR and forgets to update the
tree. A pre-commit hook or CI grep checking
`grep -c "MEM_REGION_" docs/ps1/mem-region-decision-tree.md` against
`grep -c "MEM_REGION_" src/mem_region/mem_region.h` would catch it.

---

## Reviewer 4 — Priya, Performance Engineer

### Closed in v5

- **PR7 (pre-emptive eviction timing):** moved to fgLoopNextScene.
  Mostly done — see new concern.
- **PR8 (TRANSIENT poison cost):** corrected to ~15 ms. Done.
- **PR9 (BSOD defensive clamping):** memSafeRead. Done.
- **PR10 (tag string not stored):** documented. Done.

### New concerns

#### 🟠 PR11. Pre-emptive eviction violates scene_picker's "net-zero heap pressure"

`src/scene/scene_picker.c:4-6` is explicit: "Designed for net-zero heap
pressure: malloc/free/realloc/calloc are poison-included below."
Adding `memCachePreEvictForNextScene` to `fgLoopNextScene` (which is in
or adjacent to the picker) introduces a CACHE state mutation in code
that was previously zero-impact.

The mutation isn't *malloc/free* per se — it's eviction inside CACHE's
existing slab — but it's a state change that breaks the picker's
documented invariant. Either:

- Move `memCachePreEvictForNextScene` out of the picker and into
  the caller (`jc_reborn.c:~1956` between `fgLoopNextScene` return and
  `fgLoopWalkToScene` call) so the picker stays pure.
- Update the picker's invariant comment to explicitly carve out this
  one mutation.

Option 1 is cleaner — the picker stays a pure function.

#### 🟠 PR12. Variant re-eviction after fgLoopApplyVariant

Pre-evict is keyed on the scene name returned by `fgLoopNextScene`.
But `fgLoopApplyVariant(loopScene)` (jc_reborn.c:1911) can swap in a
holiday variant **after** the pre-evict ran. The variant has different
`cachePinnedWorstCase` in `pack_header_metrics.h`. The pre-evict
freed too much (waste) or too little (mid-scene eviction fires anyway,
defeating the optimization).

Mitigation: pre-evict runs **after** `fgLoopApplyVariant`, not after
`fgLoopNextScene`. Then it has the correct effective scene name.
Update the call-site location.

#### 🟡 PR13. CRC-32 verification time scales with pack count

`memVerifyPackHashes` reads every pack's header at boot to CRC-32 it,
then compares against the baked-in hash. 63 packs × (CD-seek + header
bytes + CRC) ≈ 9 seconds (per Pat's P3 from v2). I assumed pack-header
metrics were offline-generated and there'd be no CD work at boot.

Re-reading the plan: `pack_header_metrics.h` is offline-generated.
But the CRC verification still requires reading actual pack bytes at
boot. **Either drop CRC verification** (trust the offline metrics, no
CD work) **or accept the 9 seconds.**

Trade-off: dropping verification means a stale-data drift between
packs and metrics goes undetected at boot, surfacing as a runtime
memHalt mid-soak. Accepting verification means slow boot.

Probably the right answer is: CRC verification only when a build flag
is set (release builds skip, dev/QA builds verify). Document explicitly.

---

## Reviewer 5 — Alex, Adversarial Tester

### Closed in v5

- **A11 (ps1Bsod block replace):** explicit Phase 1 step. Done.
- **A12 (pinned-set math):** pre-Phase-1 gate. Done.
- **A13 (no static initializers):** -Werror flag. Done.
- **A14 (currentScene between scenes):** fgLoopGetLastScene fallback. Done.
- **A15 (bsod-ui-test default-off CI gate):** Phase 1 step 17. Done.
- **A16 (formatReason volatile):** in step 7. Done.

### New concerns

#### 🔴 A17. `ps1DebugInit` must run before `memHalt` pre-graphics path

`memHalt`'s pre-graphics branch calls `ps1DebugError`, which depends
on `ps1DebugInit` having set up the on-screen text rendering. Looking
at v5's boot sequence:

```c
memInit();                     /* regions ready */
memVerify*();                  /* can fire memHalt here */
audioInit();
resourceCatalogParse();        /* can also memHalt */
fontInit();
surfacePoolInit();
walkPilotInit();
pauseMenuInit();
memFreezeBoot();
```

**`ps1DebugInit` is not in this sequence.** If `memVerifyBootBudget`
fires `memHalt`, `memHalt` calls `ps1DebugError`, which calls
`ps1DebugPrint` without having been initialized. Likely renders to a
random framebuffer location or no-ops silently → user sees a
black-frozen screen, exactly the regression A10 was supposed to fix.

**Required:** boot sequence must call `ps1DebugInit()` first thing,
before `memInit()`. Or `memHalt`'s pre-graphics branch must internally
call `ps1DebugInit` defensively if `ps1DebugIsInit()` returns false.

This is the same class of bug A10 caught in v4. The fix went in the
wrong place: we made `fatalError` smarter but didn't ensure its
dependencies were ready.

#### 🔴 A18. `fgLoopGetLastScene()` may not exist

v5 says call sites use `fgLoopGetLastScene()` to keep diagnostic
continuity between scenes. **Does this function actually exist?**
Grep finds `fgLoopFindStorySceneBySlug`, `fgLoopNextScene`,
`fgLoopApplyVariant`, `fgLoopWalkToScene`, etc., but not
`fgLoopGetLastScene`. The plan asserts a function that may need to
be created.

Either:
- Verify the function exists (I couldn't, but you might find it
  under a slightly different name).
- Add "implement `fgLoopGetLastScene()` returning the slug of the
  most recently played scene" as a Phase 1 step.
- Use the existing tracking — `loopScene` is a local in
  `jc_reborn.c:1909` that's the current target; the *previous* one
  could be stored on each scene-pick and retrieved.

#### 🟠 A19. `ps1Bsod` now depends on `mem_region.h`

ps1_debug.c reads memBootUsed/Peak etc. via externs. That's a new
build-layer dependency: ps1_debug.c → mem_region.h. The reverse
(mem_region.c → ps1_debug.h for formatReason) creates a circular
include risk if either header pulls in the other transitively.

Mitigation: explicit forward declarations in ps1_debug.c rather than
including mem_region.h. Document the dependency direction
(memory → debug for formatReason; debug → memory only via externs,
no include).

#### 🟠 A20. `memSafeRead` defensive clamp can't help if state pointers are corrupt

PR9 was closed by saying `memSafeRead(MEM_REGION_BOOT)` clamps the
return. But if the *caller* (ps1Bsod) is dereferencing a corrupt
function pointer to reach `memRegionUsed`, the clamp never runs.

Practical: under what corruption scenarios does `memRegionUsed` itself
fail? If a stray write hits the bump pointer global, `memRegionUsed`
reads garbage but doesn't crash (it's just a global word read). If a
stray write hits the *code section* containing `memRegionUsed`, you're
already executing random instructions. The clamp helps the first case;
nothing helps the second.

Accept this. Update the plan to say "defensive clamp covers data
corruption; instruction corruption is out of scope."

#### 🟡 A21. Pre-Phase-1 gate "verify printf non-allocating" is redundant under poison

Plan says: read PsnoobSDK's printf, confirm fixed buffer. But Phase 1
adopts a project-level malloc poison — *if* printf called malloc,
**the build would fail at link time** because malloc is poisoned and
PsnoobSDK headers wouldn't compile. So the poison is the runtime check,
not a static one. Drop the gate; the poison covers it.

---

## Synthesis — loop 2 panel verdict

**Closed from red-team v3:** 19 of 22 graded findings, including all 7
🔴 blockers.

**New blockers (3):**

| # | Issue | Raised by |
|---|-------|-----------|
| 1 | `assert(ps1IsMainContext())` compiles out under NDEBUG — need a hand-rolled `MEM_REQUIRE` macro that ships in release | Pat (P13) |
| 2 | `ps1DebugInit` must run before `memHalt` pre-graphics path or A10's fix renders to nothing | Alex (A17) |
| 3 | `fgLoopGetLastScene()` may not exist — verify or add to Phase 1 | Alex (A18) |

**New material risks (6):**

- COP0 read latency understated; perf table needs correction (P14)
- CRC-32 table location must be verified `.rodata` not `.bss` (P15)
- Per-commit CI on the bundled PR (S11)
- Decision tree isn't enforced (S12)
- Phase 1 grew to 18 steps; needs implementation checklist (M12)
- Decision tree should mention holiday variants (M13)
- Pre-evict mutation breaks scene_picker's "zero heap pressure" — move to caller (PR11)
- Variant re-evict timing — move pre-evict after `fgLoopApplyVariant` (PR12)
- CRC verification at boot adds 9 seconds of CD work; gate or drop (PR13)
- ps1Bsod's dependency on mem_region.h needs forward-decl pattern (A19)
- `memSafeRead` clamp limits should be documented (A20)

**New hygiene items (4):**

- `__builtin_unreachable()` after memHalt branches (P16)
- Tag-mechanics for bundled PR (S13)
- Decision-tree-vs-mem_region.h CI count match (M14)
- Pre-Phase-1 "printf non-allocating" gate is redundant under poison (A21)

## Recommendation

v5 closes substantially more than it opens. The three new 🔴s are all
"the fix in v5 has a small implementation hole" issues, not architectural
problems. Specifically:

- **A17 is the most important** — it undoes A10's whole win if not
  fixed.
- **P13 is a one-line code fix** but ships under NDEBUG, so missing
  it costs production safety.
- **A18 is a verification step**, not a redesign.

If v6 resolves the 3 blockers + 11 oranges + 4 yellows, this should
converge. Plan looks healthier than at any prior iteration.
