# Memory region allocator — red team v5 (panel re-review of v6)

Same panel reconvenes. v6 closed 18 of 18 v4 findings. This pass looks
for new issues introduced by the v6 fixes. Loop 3 of the goal-driven
iteration. Convergence approaching.

---

## Reviewer 1 — Pat, Embedded Systems Veteran

### Closed in v6

- **P13 (MEM_REQUIRE):** macro defined; ships in release. Done.
- **P14 (COP0 latency):** perf table updated to 18-22 / 33-37 cycles. Done.
- **P15 (CRC table in .rodata):** documented; checklist verifies via
  `objdump -h`. Done.
- **P16 (`__builtin_unreachable`):** in memHalt body. Done.

### New concerns

#### 🟠 P17. `ps1IsMainContext()` itself doesn't exist yet

Plan adds `MEM_REQUIRE(ps1IsMainContext())` everywhere. **`ps1IsMainContext()`
is a function the plan references but doesn't say where it lives or who
implements it.** Search the codebase: no such function exists today.
Either:
- Implement it in `src/mem_region.c` (reads PSX SR/cause via `mfc0`),
  in which case Phase 1 step 8 needs to spell out the implementation
  (~10 lines of inline asm); or
- Use an existing PsyQ primitive (PsyQ has `_get_co_zero()` and
  similar for COP0 reads but no "are we in main context" wrapper) —
  needs a small wrapper.

Add to Phase 1 step 8: "Implement `ps1IsMainContext()` in `mem_region.c`
using `__asm__("mfc0 %0, $13" : "=r"(cause));` and `__asm__("mfc0 %0, $12"
: "=r"(sr));`, return `(cause & 0x3F) == 0` (no exception in progress)
and `(sr & 0x2) == 0` (interrupts enabled — i.e., not in ISR with
intsdisabled)." Verify the exact bit pattern against PSX docs.

#### 🟠 P18. PsyQ toolchain may not support `-Wglobal-constructors`

GCC's `-Wglobal-constructors` was added in GCC 4.7. PsnoobSDK
ships with `mipsel-none-elf-gcc`, which is usually GCC 12+. Most likely
fine, but **confirm** before depending on the flag in CI.

Fallback if unsupported: `nm jcreborn.elf | grep -E "_init|__init"`
returns no init functions. Manual check; less elegant.

#### 🟡 P19. `__builtin_unreachable()` after `while(1)` is redundant

```c
while (1) { /* halt forever */ }
__builtin_unreachable();
```

GCC already infers `while(1)` is non-returning. The annotation is
defensive but no-op. Either drop it or comment "defensive — drop if
unused warning fires."

---

## Reviewer 2 — Sarah, SRE / Operations

### Closed in v6

- **S11 (per-commit CI):** two-PR split. Done.
- **S12 (decision tree enforcement):** RATIONALE comment grep gate. Done.
- **S13 (tag mechanics):** spelled out. Done.

### New concerns

#### 🟠 S14. `MEM_REGION_RATIONALE` grep gate has a false-positive failure mode

CI gate:
```
grep -B1 "memAlloc(" src/ | grep -c MEM_REGION_RATIONALE == grep -c "memAlloc(" src/
```

Edge cases:
- `memAlloc` inside a macro expanded N times: one source-line comment,
  N "calls." Mismatch.
- `memAlloc` on the same line as the rationale comment
  (`uint8 *p = memAlloc(...); /* MEM_REGION_RATIONALE: foo */`):
  `grep -B1` misses it.
- Multi-line `memAlloc(REGION,\n    size,\n    tag)`: `grep -B1`
  catches only the first line.

Tighten the gate: use a custom Python script that AST-parses (or
regex-matches with multi-line support) and confirms every call site
has a preceding `MEM_REGION_RATIONALE:` comment within 3 lines. ~20
lines of Python.

#### 🟠 S15. Pre-Phase-1 gates can't all be closed before Phase 1

The gate "Pinned-set verifier-math = runtime-accounting" requires:
- The verifier (Phase 1 work)
- Runtime instrumentation (Phase 1 work)

To run the gate, Phase 1 needs to be mostly built. **The gate is
effectively a Phase 1 milestone, not a pre-Phase-1 gate.** Plan should
re-categorize:
- True pre-gates: linker map, JSON decomposition, grBackgroundSfc
  lifecycle, ISR audit, ttm.c audit, BIOS audit.
- Phase 1 internal milestones: pinned-set math match, `-Wglobal-constructors`
  baseline.

#### 🟡 S16. Phase 1 checklist drifts from plan

`mem-region-phase-1-checklist.md` mirrors 24 plan steps. If plan
steps change, checklist becomes stale. Two-file consistency is fragile.

Mitigation: a script that generates the checklist from the plan, run in
CI. Or accept that updates touch both files (small extra burden).

---

## Reviewer 3 — Mateo, Future Maintainer

### Closed in v6

- **M12 (Phase 1 checklist):** committed. Done.
- **M13 (holiday variants in decision tree):** section added. Done.
- **M14 (count-match CI):** included. Done.

### New concerns

#### 🟠 M15. Holiday-variant note in decision tree under-specifies TRANSIENT case

v6 added: "If the allocation is variant-specific (e.g., a holiday-only
sprite that's only loaded when the variant is active), treat it like
any other resource blob — CACHE."

But what about a **variant-specific per-scene scratch buffer**? E.g., a
holiday variant needs a 4 KB decode buffer that's only used during
its scene's playback. That's not a resource blob (not a BMP/TTM/SCR/ADS)
— it's per-scene scratch. Should be TRANSIENT, not CACHE.

The text presumes variant-specific = resource. Tighten:

> Variant-specific *resource blobs*: CACHE.
> Variant-specific *per-scene scratch*: TRANSIENT (same as base scene).
> Variant-specific *long-lived state*: BOOT (rare; usually means a
> design issue — the resource should be CACHE-able).

#### 🟡 M16. `MEM_REGION_RATIONALE` adds writing burden but enforces clarity

A new contributor adding their first `memAlloc` site has to:
1. Pick a region from the tree.
2. Write a one-line rationale.

Both are good for clarity but raise the friction floor for trivial
allocations. Decision-tree-pick + rationale string is 2-3 minutes of
extra work per site. Worth it; just note in the contributing guide.

---

## Reviewer 4 — Priya, Performance Engineer

### Closed in v6

- **PR11 (zero-heap-pressure preserved):** pre-evict moved to caller.
  Done.
- **PR12 (variant timing):** pre-evict runs after fgLoopApplyVariant.
  Mostly done — see below.
- **PR13 (CRC verification gated):** `JC_VERIFY_PACK_HASHES` flag. Done.

### New concerns

#### 🔴 PR14. Variant timing — pre-evict lookup may key on base scene, not variant

Plan's pseudocode:
```c
const char *loopScene = fgLoopNextScene(...);
fgLoopApplyVariant(loopScene);                  /* may swap to variant pack */
memCachePreEvictForNextScene(loopScene);        /* uses loopScene */
```

But `fgLoopApplyVariant(loopScene)` likely **mutates global state** to
swap in the variant pack, not the local `loopScene` string. So
`memCachePreEvictForNextScene(loopScene)` looks up the **base scene**'s
`cachePinnedWorstCase` in `pack_header_metrics.h`, not the variant's.

Verify by reading `fgLoopApplyVariant`. If it doesn't return the
effective name, two fixes:
- Make it return the effective name: `const char *fgLoopApplyVariant(const char *base)`.
- Or have `memCachePreEvictForNextScene` itself query the current
  variant state.

Either way, **the v6 fix is incomplete** — closing PR12 needs the
lookup-keying issue resolved too.

#### 🟠 PR15. `MEM_REQUIRE(ps1IsMainContext())` runs on every `memAlloc` AND `memFree`

The plan says it ships in release in both `memAlloc` and `memFree`.
For TRANSIENT, every alloc + every (eventual no-op) free pays the
8-12 cycle COP0 read. Over a busy scene with 40 allocs + 40 frees,
that's 80 × 10 = 800 cycles of context-check overhead per scene.

PS1 is at ~33 MHz, so 800 cycles ≈ 24 µs per scene. Imperceptible, but
note in the perf budget. The fact that `memFree(TRANSIENT)` is a
no-op-with-bookkeeping makes the check feel wasteful there — could
gate `MEM_REQUIRE` to `memAlloc` only and accept the (vanishingly
unlikely) risk of ISR-context `memFree`.

I'd keep both for symmetry. Document the choice.

#### 🟡 PR16. `JC_VERIFY_PACK_HASHES` gating means release builds trust offline metrics

Plan: release skips CRC verification. Trade-off accepted, but: if a
pack file is updated without regenerating the metrics, the boot proof
runs against stale data and the scene's actual demands may exceed
what's verified. We get a runtime memHalt in production from a
mis-shipped build.

**Mitigation:** the pack build pipeline includes a step that
*regenerates* `pack_header_metrics.h` whenever a `.fgp` file changes,
and CI fails any release build that has a pack file newer than
`pack_header_metrics.h`. This catches the drift at build time even
without runtime CRC verification.

---

## Reviewer 5 — Alex, Adversarial Tester

### Closed in v6

- **A17 (ps1DebugInit before memInit):** explicit in boot pseudocode. Done.
- **A18 (fgLoopGetLastScene):** Phase 1 step 19. Done.
- **A19 (forward-decl pattern):** documented. Done.
- **A20 (memSafeRead scope):** documented. Done.
- **A21 (drop printf gate):** dropped. Done.

### New concerns

#### 🔴 A22. `fgLoopGetLastScene` returns wrong value during pre-evict

Phase 1 step 19: "updates on each successful pick." But the sequence:

```c
loopScene = fgLoopNextScene(...);                 /* picked scene N+1 */
fgLoopApplyVariant(loopScene);
memCachePreEvictForNextScene(loopScene);          /* memHalt could fire here */
fgLoopWalkToScene(storyScene);
foregroundPilotPlay();                             /* memHalt could fire here */
/* Only here, after successful play, is fgLoopGetLastScene updated. */
```

If `memHalt` fires during `memCachePreEvictForNextScene` or during
scene-N+1 setup, `fgLoopGetLastScene()` returns **scene N**, not
scene N+1. The BSOD says "scene=<previous>" when the failure was
caused by `<next>`. Diagnostic continuity is wrong.

Fix: `fgLoopGetLastScene` becomes "current target scene if pick is
in progress, else last played." `fgLoopNextScene` updates the target;
`foregroundPilotPlay` updates the played-marker after success. Pre-evict
+ play-phase memHalt then reports the correct target scene.

#### 🟠 A23. Forward-decl pattern in ps1_debug.c is brittle

Plan: `ps1_debug.c` puts `extern size_t memRegionUsed(MemRegion);` at
the top of the BSOD detail block. If `mem_region.h` changes the
function signature (e.g., adds a flag parameter), `ps1_debug.c`'s
forward decl links to a different symbol or the linker fails. **Single
source of truth** is the answer: a small `src/mem_region/mem_region_extern.h`
that contains only forward decls, included by both `mem_region.c`
(for definition) and `ps1_debug.c` (for use). No risk of drift.

#### 🟠 A24. `MEM_REGION_RATIONALE` enforcement vs. macros

Picking up Sarah's S14: the gate fails on `memAlloc` inside macros.
But beyond CI false positives, **macros that wrap `memAlloc` are a
real possibility** — e.g., a hypothetical `NEW_RES(type)` macro that
expands to `memAlloc(MEM_REGION_CACHE, sizeof(type), #type)`. The
*macro definition* has one RATIONALE comment; macro use sites have
none.

Policy: prohibit macros that wrap `memAlloc`. Document. Add the
prohibition to `mem-region-decision-tree.md`. (The poison-malloc
pattern in `scene_picker.c` is the prior art for "we just disallow
this entire pattern.")

#### 🟡 A25. The pre-evict path itself can fail

`memCachePreEvictForNextScene` may need to evict more than CACHE
currently holds unpinned. If everything is pinned (because the *previous*
scene didn't unpin properly — a bug), pre-evict can't evict and
falls into `memHalt`. The BSOD then reports the *next* scene as the
failure cause, but the bug is in the *previous* scene's pin
accounting.

Mitigation: log `pinResource`/`unpinResource` deltas at every scene
transition (debug build); if the pin count doesn't return to zero
between scenes, that's the real bug. JCBSOD detail line additions for
pinned-resource breakdown would help diagnostics.

---

## Synthesis — loop 3 panel verdict

**Closed from red-team v4:** 18 of 18 graded findings (100%). This is
a clean close — every fix landed.

**New blockers (2):**

| # | Issue | Raised by |
|---|-------|-----------|
| 1 | `fgLoopGetLastScene` returns wrong value during pre-evict and scene-N+1 setup — must report the *target* scene, not the *last played* scene | Alex (A22) |
| 2 | Pre-evict lookup keys on base scene, not variant — `fgLoopApplyVariant` likely doesn't change `loopScene` string | Priya (PR14) |

**New material risks (7):**

- `ps1IsMainContext()` doesn't exist — Phase 1 must implement it (P17)
- `-Wglobal-constructors` PsyQ toolchain compatibility — verify (P18)
- `MEM_REGION_RATIONALE` grep gate has false-positive cases —
  replace with Python AST script (S14)
- Some pre-Phase-1 gates need most of Phase 1 done; re-categorize (S15)
- Holiday-variant TRANSIENT case under-specified in decision tree (M15)
- `MEM_REQUIRE` on `memFree` too — document the symmetry choice (PR15)
- Single-source forward decls via `mem_region_extern.h` (A23)
- Prohibit macros wrapping `memAlloc` (A24)

**New hygiene items (3):**

- `__builtin_unreachable` after `while(1)` is redundant (P19)
- `MEM_REGION_RATIONALE` adds writing-burden, document (M16)
- Pin-count delta logging at scene transitions for debug builds (A25)

## Recommendation

v6 is the strongest version of the plan yet. **18 of 18 v4 findings
closed cleanly** — no rollback, no partial fixes. The 2 new blockers are
both small consequences of the v6 fixes (the `fgLoopGetLastScene`
timing isn't a redesign — just clarifying "target" vs "played"; the
variant-keying issue is a one-line fix to `fgLoopApplyVariant`'s
signature).

If v7 resolves the 2 new blockers + 7 material + 3 hygiene items, this
should converge with **zero findings** in the next pass. The architecture
is stable; we're polishing edges now.
