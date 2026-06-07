# Memory region allocator — red team v3 (panel re-review of v4)

Companion to [memory-region-allocator-plan.md](./memory-region-allocator-plan.md)
(v4). Same five-reviewer panel from
[memory-region-allocator-red-team-v2.md](./memory-region-allocator-red-team-v2.md)
reconvenes. Each reviewer re-evaluates whether their prior concerns are
properly resolved and flags **new** issues introduced by the v3→v4
revisions (especially the JC_BSOD integration, which is fresh).

Items still graded 🔴 / 🟠 / 🟡 within each section. Where a v2 item is
addressed, it's noted as **closed**; where it's only partially addressed,
it's reopened.

---

## Reviewer 1 — Pat, Embedded Systems Veteran

### Closed in v4

- **P1 (ISR-safety):** addressed via assert + audit gate. (See below for
  reopening.)
- **P3 (CD-seek-at-boot):** pack metrics now offline-generated. Done.
- **P4 (8-byte alignment):** dropped to 4-byte. Done.
- **P5 (regions vs pools):** explicit argument in the new "Architectural
  decision" section. I disagree with the conclusion but the argument is
  written down, which is what I asked for.

### Reopened / new

#### 🔴 P1-bis. ISR enforcement is debug-only — re-classified as blocker

The plan resolved P1 via `assert(ps1IsMainContext())` in *debug builds*
plus an audit. Audit lets bitrot in. Debug-only asserts don't ship.
**This means production builds have no runtime guard against ISR-context
allocation.** A future PR that registers a callback which transitively
calls `memAlloc` ships clean, runs fine in unit tests, races on a real
console. Make the check unconditional. Cost is ~3 instructions per
`memAlloc` — well under the per-alloc budget.

#### 🔴 P8. `ps1Bsod` reads stale heap diagnostics that no longer mean anything

`src/platform/ps1/ps1_debug.c:228` queries `fgProbeLargestAlloc()` and prints
`JCBSOD heapKB=...` in every BSOD dump. After Phase 2 deletes the bandaid
heap-probe infrastructure (and after the project-level malloc poison
makes libc malloc unreachable), `fgProbeLargestAlloc`'s binary-search
malloc loop **either becomes dead code (probe returns 0, misleading) or
itself trips the poison and infinite-recurses BSODs**. The plan says
"`ps1Bsod` gets a 5-line addition" without acknowledging that the
existing 3-4 lines reading `fgProbeLargestAlloc`, `fgGetFrameBufferBytes`,
etc. need their semantics audited too.

**Required:** Phase 1 PR for `ps1Bsod` *replaces* the heap-probe block
entirely with region-state reads, doesn't just add to it. Keep
`walkClean*` and `johnwalkSlot*` lines if those externs still mean
something post-migration (probably they should, since those buffers move
to BOOT).

#### 🟠 P9. `ps1Bsod` is `noreturn` and assumes graphics is up

There's an explicit pre-`graphicsInit` window (memInit → resourceCatalogParse
→ audioInit → ... → graphicsInit → ... → memFreezeBoot). The plan splits
fatalError vs JC_BSOD by phase, but the split point is *fuzzy*. What
about:

- A debug-build assert in `memFreezeBoot` itself? Graphics is up,
  JC_BSOD is fine.
- A debug-build assert in `audioInit` that calls `memAlloc(BOOT, ...)`
  past budget? Graphics is *not yet* up — fatalError.
- A failure in `walkPilotInit` after `surfacePoolInit`? Graphics might
  be partially up.

The plan needs a single primitive — say, `memHalt(scene, reason)` — that
*internally* picks JC_BSOD vs fatalError based on a graphics-ready flag
(`extern int graphicsIsInitialized;`). Don't make every call site re-make
the choice.

#### 🟠 P10. PSX printf does not allocate (verify, document)

The plan reuses `ps1Bsod` which calls `printf` many times. On PsyQ, the
runtime's `printf` writes into a fixed-size internal buffer (~256 B); no
heap. On PsnoobSDK (which the project uses — see `src/platform/ps1/ps1_debug.c`
header) similar. **Verify** the project's specific libc's printf is
non-allocating and add a code comment to that effect, so a future
contributor doesn't swap in a fancy printf and re-create a BSOD recursion.

#### 🟡 P11. Watchdog / emulator timeout while in BSOD halt

Some emulators (mostly Linux-side) terminate processes that spin in
`while(1)`. DuckStation is friendly. PCSX-Redux is friendly. ePSXe may
not be. Not a shipping concern but worth a one-liner in the plan: the
`while(1)` after rendering is by design; emulator users who lose the
screen mid-BSOD can find it in the TTY log via the `JCBSOD-FATAL`
sentinel.

#### 🟡 P12. CRC-32 vs SHA-256

The plan still says "SHA-256 (or CRC-32, simpler on PSX)." Pick one.
CRC-32 is the right answer: fast, sufficient for "did the bytes
change," already used elsewhere in PsyQ for sector validation. SHA-256
is overkill and the implementation cost on R3000A is real (a couple
KB of code + table). **Commit to CRC-32 in the plan**.

---

## Reviewer 2 — Sarah, SRE / Operations

### Closed in v4

- **S1 (Phase 2 rollback):** per-file PR split + tagged baseline. Done.
- **S2 (PS1 has no telemetry):** routing through JC_BSOD means the
  halt screen carries the data, and the `JCBSOD-FATAL` log sentinel
  is already grep-friendly. Done.
- **S3 (project-level poison):** `src/malloc_poison.h` in v4. Done.
- **S4 (phases aren't independent):** plan now honestly labels them
  sequential. Done.
- **S5 (qualitative soak signal):** per-scene frame-byte diff. Done.
- **S6 (schedule realism):** 5 weeks not days. Done.

### Reopened / new

#### 🔴 S7. Per-file PR sequencing across Phase 2 is unsafe as written

v4 says "Phase 2 ships as 8 per-file PRs, each individually revertable."
But the JC_BSOD direction means the call sites in (say) `resource.c` —
items 10-13 — *depend on* the JC_BSOD plumbing already being in place,
which in turn depends on the heap diagnostic block in `ps1Bsod` having
been updated (P8 above). **The PRs are sequential, not parallel, and
PR P2.2 (`resource.c`) is broken until P2.1 (`foreground_pilot.c`)
plus the ps1Bsod update lands.**

Either:
- Spell out the dependency order in the plan (P2.0: ps1Bsod update,
  P2.1: foreground_pilot.c, P2.2-7: per-file in any order, P2.8:
  comment refresh), or
- Land all 8 in one bundled PR and accept the rollback risk.

I'd land all 8 in one PR with a tag for rollback. "Per-file revertable"
is a property of the *diff*, not the *PR*; a single PR with 8 atomic
commits gives you `git revert <sha>` for any one of them.

#### 🟠 S8. The `JCBSOD-FATAL` log sentinel changes meaning under this plan

Today, `JCBSOD-FATAL` is emitted only by `ps1Bsod`, which is only called
from the `bsod-test` bootmode (synthetic) and a handful of "should be
unreachable" sites. Test harnesses that grep `JCBSOD-FATAL` to detect
fatal soak-run end states currently see ~zero hits.

After this plan lands, JC_BSOD is the official "we hit a bug" mechanism,
and any soak run that hits one fires the sentinel. The semantic of the
sentinel changes from "did the bsod-test bootmode run as expected" to
"did the soak run actually pass." **Test harnesses must be updated** in
the same PR that lands the migration. The plan doesn't call this out.

#### 🟠 S9. Synthetic `bsod-test-mem-*` bootmodes need a back-door into the allocator

To synthesize a CACHE exhaustion mid-scene, you need *the allocator* to
return failure once. But the no-fail invariant says it never returns
failure. So the synthetic path is either:

- A direct `JC_BSOD(scene, "synthetic memory test")` call from the
  bootmode handler — bypasses the allocator entirely. Tests the UI but
  not the integration.
- A boot-flag-gated `MEM_SIMULATE_FAILURE_ONCE` that makes the next
  qualifying alloc return NULL once. Tests the integration but
  re-introduces a NULL-return code path even if behind a flag.

I'd pick the first (UI-only) and rename them `bsod-ui-test-mem-*` to
make clear they only verify the visual output, not the call chain.

#### 🟡 S10. Dev iteration with `MEM_DEV_BUILD` masks the no-fail invariant

A developer iterates with `MEM_DEV_BUILD=1` so the game runs through
their broken new scene. They forget to flip it off. CI catches it (the
plan promises a grep check). **But the dev's last local test was with
the flag on.** If their code accidentally relies on the printf+skip
behavior (e.g., a missing init that the skip-path would mask), pushing
the off-flag PR breaks CI in a way the dev didn't see locally.

Mitigation: require a one-line "I built locally with `MEM_DEV_BUILD=0`
and ran the 63-scene rotation cleanly" checkbox in the PR template for
any PR touching scene data.

---

## Reviewer 3 — Mateo, Future Maintainer

### Closed in v4

- **M1 (how to add a scene):** companion `adding-new-scenes-memory.md`
  exists. Helpful. Done.
- **M2 (MEM_DEV_BUILD):** dev mode + CI rejection. Done.
- **M3 (region naming):** BOOT/CACHE/TRANSIENT. Done.
- **M4 (decision tree):** committed to as a Phase 1 deliverable. Done.
- **M6 (JSON date filename):** `scene_analysis_current.json` rename.
  Done.

### Reopened / new

#### 🔴 M7. Discoverability of `JC_BSOD` is poor

I'm a new maintainer. I add an allocation site. I look at neighboring
code and copy the pattern. I see `fatalError(...)` used in lots of
places, including in the migrated allocator (legitimately, for boot).
So I write `fatalError("X exhausted...")` for my runtime failure too,
because I don't know `JC_BSOD` exists.

The plan needs a *single primitive* (per P9) that subsumes both, plus
a Phase-1 grep gate that catches the wrong choice:
`grep -rE "fatalError\(" src/` outside `mem_region.c`, `ps1_debug.c`,
and the audited boot-init files must return zero hits. Force everyone
to call the unified primitive.

#### 🔴 M8. `bsod-test-mem-*` bootmodes are added by example, not by interface

The plan says these three bootmodes are added. When I add a new region
in 18 months (say, separate VRAM-mirror region for the texture cache),
I need to add `bsod-test-mem-vram-mirror`. How? The plan doesn't show
the pattern. Looking at the existing `bsod-test` bootmode handler
(`src/jc_reborn.c:1979-1982`) gives me one example, but the new ones
need their own per-region scaffolding. **Document the pattern** in
`adding-new-scenes-memory.md`.

#### 🟠 M9. Companion doc points to a "decision tree" doc that doesn't exist yet

`adding-new-scenes-memory.md` links to `mem-region-decision-tree.md`
(TBD). The plan says "added in Phase 1 implementation." Both docs
should land in Phase 1 *as part of the same PR* as `mem_region.c`,
not as a follow-up. Future me reading the companion immediately wants
the decision tree.

#### 🟠 M10. `formatReason` is invisible

The plan references `formatReason("CACHE exhausted req=%zu pinned=%zu",
n, pinned)` in the JC_BSOD call example. What is `formatReason`? Where
does it live? Is it part of `mem_region.h` or `ps1_debug.h`? Could be
either. **Pin the location** — I'd say `ps1_debug.h` since it's a
BSOD-side helper, not memory-side.

#### 🟡 M11. The decision tree as written is too coarse

```
Is it allocated once at boot and never freed?  → BOOT
Is it a resource blob (BMP/TTM/SCR/ADS)?       → CACHE
Does it live exactly one scene?                → TRANSIENT
```

What about: a temporary scratch buffer used only during BMP→surface
conversion that's freed immediately? "Lives one scene" is too loose — it
lives one function call. Does that mean TRANSIENT? Probably. Worth a
clarifying line.

---

## Reviewer 4 — Priya, Performance Engineer

### Closed in v4

- **PR1 (memSceneReset cost):** quantified at ~5 cycles release / ~8 ms
  debug-poison. Estimates not measurements but a number is there.
  Mostly done.
- **PR2 (boot-time cost):** offline metrics + ~75 ms BSS clear.
  Quantified. Done.
- **PR4 (hot path):** claimed the 5-10× win. Done.
- **PR5 (RES eviction mid-scene):** pre-emptive eviction at scene
  transitions. Plan describes the idea.
- **PR6 (telemetry on fatal):** JC_BSOD's existing detail-line emission
  covers this unconditionally. Done.

### Reopened / new

#### 🔴 PR7. Pre-emptive eviction has a timing hole

Plan says `memSceneReset` looks at "projected next-scene CACHE demand
(from `pack_header_metrics.h`)." But:

- `memSceneReset` is called at the *top* of the next scene's setup,
  *after* the next scene has been chosen.
- So it knows the next scene at call time.
- But the pre-emptive eviction *should* happen *between* scenes — i.e.,
  *before* `memSceneReset` is called — so the eviction work doesn't
  block the next scene's first frame.

Either:
- Move pre-emptive eviction into the scene picker (knows the next scene
  even earlier), or
- Accept that eviction happens at `memSceneReset` time but quantify
  the cost (LRU scan of ~200 entries = ~3-5 ms during the already-paused
  transition).

The plan says "pre-empted at scene-reset, see below" but the "below"
just restates the idea. Pin the call-site location, or accept that
this is at-reset and quantify.

#### 🟠 PR8. TRANSIENT reset wipe cost claim is wrong-order

Plan's perf table: "memSceneReset release ~5 cycles, debug-poison ~8 ms."

The release path is 5 cycles only if we *don't* zero TRANSIENT. With
the new INIT_NONE-marked call sites depending on caller-side init,
release is fine. Good.

But the **debug-poison path's 8 ms claim is too low.** 250 KB at PSX
write speed (cached → write-buffer → RAM, ~16 MB/s sustained) is
~15.6 ms, not 8 ms. The plan should say ~15 ms or specifically claim
DMA-clear (which would be ~3 ms but requires the GPU DMA, which has
its own scheduling). Either re-measure or correct the estimate.

#### 🟠 PR9. The 5-line addition to `ps1Bsod` runs at the worst possible time

When BSOD fires, the system is broken. Reading three global counters
(memBootUsed/Peak, memCacheUsed/Peak, memTransientUsed/Peak) is fine
because they're plain word reads. But if a thread of execution corrupts
the region metadata (e.g., a bug writes past a TRANSIENT allocation
into the bump pointer or balance counter), the BSOD dump might read
garbage and crash before rendering. **The BSOD path should defensively
clamp/sanitize the values it reads** so a halt-while-halting can't
happen. ~3 lines.

#### 🟡 PR10. Per-alloc tag string store cost

The API takes `const char *tag` per `memAlloc`. The plan doesn't say
whether the tag is *stored* (per-alloc cost) or just used by the next
`memLogTelemetry` call (no per-alloc cost). For PERM/TRANSIENT bump
allocators there's no header to store it in. For CACHE's free-list
entries, the header could carry it (~4 bytes extra per entry).

**Pin this:** the tag is for telemetry only, used by the call site that
provides it; not stored. Document in the header.

---

## Reviewer 5 — Alex, Adversarial Tester

### Closed in v4

- **A1 (stale JSON):** pack-hash verification at boot. Done.
- **A2 (variant coverage):** generator enumerates `fgLoopApplyVariant`.
  Done.
- **A3 (uninit SCENE bytes):** poison on reset + per-site INIT_*
  annotation. Done.
- **A4 (pause flow):** pause menu pre-allocates in BOOT. Done.
- **A5 (CD corruption):** size sanity check before believing a header.
  Done.
- **A6 (LRU re-entrancy):** non-recursive contract. Done.
- **A7 (BIOS/PsyQ alloc):** in budget table. Done.
- **A8 (full src/ audit):** Phase 3 grep gate. Done.
- **A9 (poison per-TU):** project-level header. Done.

### Reopened / new

#### 🔴 A10. The new boot-time `fatalError` UI is regression-untested

`fatalError` has been in the codebase for years but in current code its
PS1 path is `printf` + `while(1)` — there's no on-screen feedback. With
the no-fail invariant + boot-time verify functions, **`fatalError` becomes
the way users discover budget mismatches.** A user with a slightly
different binary (modded? holiday DLC?) hits a verify and sees... a
black screen with no obvious cause.

**Required:** before Phase 1 ships, upgrade `fatalError` on PS1 to render
a minimal on-screen text panel (not the full BSOD UI — graphics isn't
up — but at least flip the frame buffer to a solid color and dump TTY)
or call `ps1DebugError` from `src/platform/ps1/ps1_debug.h:51` (which the team built
specifically for this and may already be safe pre-graphics).

#### 🔴 A11. `ps1Bsod` reads heap-probe data via legacy externs

(Pat's P8, restated from my angle.) After Phase 2 deletes the bandaid
infrastructure, `fgProbeLargestAlloc`'s binary-search-malloc loop hits
the poison macro and won't compile. Or, if it's kept compiling for
historical reasons, it returns 0 and the BSOD shows `heapKB=0` always,
which is meaningless. **The Phase 1 `ps1Bsod` update must replace this
block, not augment it.** Counts as 🔴 because shipping a BSOD with
broken diagnostics actively misleads support tickets.

#### 🟠 A12. Boot proof's pinned-set computation assumes thread independence

`memVerifyAllScenesPinnedFitCache` sums "one TTM + one BMP per concurrent
thread." But:

- **Shared resources:** if two threads pin the same TTM, they pin one
  block with refcount 2, not two blocks. The verifier overcounts.
- **Multi-resource threads:** if a thread pins a TTM *and* the
  TTM's referenced BMP *and* a sub-BMP, that's three resources per
  thread, not two.

The plan's accounting matches the scene_analyzer's assumptions
(`max_bmp_slots: 6, max_ttm_slots: 10`) loosely but not precisely.
**Confirm the verifier's math equals what the runtime actually pins**
by running both against the same scene set and asserting equality.
This is a pre-Phase-1 gate.

#### 🟠 A13. Pre-`memInit` allocations are still possible

PsyQ runtime startup, static initializers, and the BSS clear all happen
*before* `main()` runs. `main()` calls `memInit` first thing. But what
if a static C++ constructor (none in this project today, but in the
future?) or a PsyQ callback registered at link time touches memory?

The plan says "memAlloc before memInit calls fatalError." Add a
companion: `memAllocAfterFreezeBoot` for PERM also calls fatalError.
Both guards are in v4. Good. **But also enforce static-init-free in
the build** — no C++ constructors, no link-time `__attribute__((constructor))`
functions. Plain C only. State this explicitly.

#### 🟠 A14. JC_BSOD assumes `currentScene` is meaningful

JC_BSOD calls pass `currentScene` as the first arg. What's the value
of `currentScene` during:
- `audioInit` (boot, pre-graphics) — irrelevant, uses fatalError.
- `pauseMenuInit` (boot) — irrelevant, fatalError.
- A bug between scene N's end and scene N+1's start — `currentScene`
  might be the previous scene's name, the next scene's name, or NULL.

`ps1Bsod`'s implementation does handle NULL (writes "(unknown)"), but
the plan should require call sites to pass the most-recently-applicable
scene name and document the convention: "between scenes, pass
`fgLoopGetLastScene()` for diagnostic continuity."

#### 🟡 A15. `bsod-test-mem-*` bootmodes are an attack surface

Adding three new bootmodes means three new code paths. If someone ships
a build with one of them enabled by default, the game halts on first
boot. **CI gate:** assert the bootmode flags are all 0 by default in
the shipping config (similar to the `MEM_DEV_BUILD=0` check).

#### 🟡 A16. `formatReason` static buffer is global state

If two threads (interrupt-driven, despite the audit) both call
`formatReason` for two simultaneous BSODs, the buffer races. Probability
near zero given Pat's ISR-safety audit, but the buffer should at least
be `volatile` to suppress compiler reordering, and the second BSOD
(if it could happen) should print "[concurrent fatal]" instead of
racing into the same buffer.

---

## Synthesis — what the panel agrees blocks Phase 1

The panel landed on **six new 🔴-grade items** for v4 (after closing 14
items from v3):

| # | Issue | Raised by |
|---|-------|-----------|
| 1 | **ISR-safety enforcement is debug-only** — unconditional assert needed | Pat (P1-bis) |
| 2 | **`ps1Bsod`'s legacy heap-probe block must be replaced, not extended** — `fgProbeLargestAlloc` won't work post-Phase-2 | Pat (P8), Alex (A11) |
| 3 | **Per-file Phase 2 PRs are sequential, not parallel** — bundle them or document the dependency order | Sarah (S7) |
| 4 | **Single `memHalt(scene, reason)` primitive** that picks JC_BSOD vs fatalError internally — don't make every call site choose | Pat (P9), Mateo (M7) |
| 5 | **Boot-time `fatalError` UX is currently invisible on PS1** — upgrade to use `ps1DebugError` or similar pre-graphics screen | Alex (A10) |
| 6 | **Pre-emptive CACHE eviction has a timing hole** — happens at memSceneReset, which is the START of the next scene, so eviction work blocks the first frame | Priya (PR7) |

Plus **eight 🟠-grade items** worth landing in v5 of the plan:

- Confirm CRC-32 (Pat P12); document PSX printf is non-allocating (Pat P10).
- Update `JCBSOD-FATAL` test harnesses to match new semantic (Sarah S8).
- Rename `bsod-test-mem-*` → `bsod-ui-test-mem-*` to clarify scope
  (Sarah S9).
- PR template checkbox for `MEM_DEV_BUILD=0` local test (Sarah S10).
- Land decision-tree doc in Phase 1, not after (Mateo M9).
- Pin `formatReason`'s location (Mateo M10).
- Correct TRANSIENT-poison cost estimate (~15 ms not ~8 ms) (Priya PR8).
- Pin the tag-string-not-stored convention (Priya PR10).
- Confirm pinned-set verifier math equals runtime accounting (Alex A12).
- Forbid static initializers explicitly (Alex A13).
- CI gate that `bsod-test-mem-*` flags are off by default (Alex A15).

The 🟡 items can be handled inline during implementation.

## Recommendation

**v4 with the BSOD integration is structurally sound** — every prior
panel concern was addressed, the plan correctly identifies the existing
framework, and the boot-vs-runtime split is the right architectural
move.

But six items the panel found *new* in this re-review are blocking,
particularly **#2** (the existing `ps1Bsod` body has a heap-probe block
that no longer means anything post-migration) and **#4** (the boot-vs-
runtime decision should live inside one primitive, not at every call
site). Both are small fixes but they're load-bearing.

If v5 of the plan resolves those six, the panel would approve Phase 1.
