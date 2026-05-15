# Memory region allocator — red team v2 (multi-reviewer)

Companion to [memory-region-allocator-plan.md](./memory-region-allocator-plan.md)
(v3). Where the v1 red team was a single technical pass focused on bugs in
the architecture, v2 is a panel review with five distinct perspectives. Each
reviewer reads the v3 plan independently and flags concerns from their own
lens. The synthesis at the end pulls the top items the team agrees are
blocking.

Items still graded 🔴 / 🟠 / 🟡 within each reviewer's section.

---

## Reviewer 1 — Pat, Embedded Systems Veteran

> *30 years shipping to fixed-RAM platforms. Built malloc replacements for
> three games on PSX, two on Saturn, one on N64. Skeptical of abstractions.*

### 🔴 P1. Interrupt-context allocation is not addressed

The PSX has GPU DMA-completion interrupts and root-counter callbacks that
fire asynchronously to `main()`. If any code reachable from those handlers
calls `memAlloc(SCENE, ...)`, your bump-pointer increment races against
`main`'s. There is no mention of this in the plan. I want either:

- An audit showing no `memAlloc` is reachable from any ISR/callback, or
- An ISR-safe path (disable interrupts around the bump, or a per-region
  spinlock).

The current code uses libc malloc, which on PsyQ is *also* not ISR-safe,
but the existing skip-on-failure paths mask that. After Phase 2 removes
those paths, a race becomes a hard crash. **This needs answering before
Phase 1 ships.**

### 🟠 P2. BSS clear at boot is now a 1.2 MB write pass

PsyQ's `_start` zeroes BSS on startup. A 1.2 MB clear takes roughly 40 ms
at PSX memory bandwidth (16 MB/s sustained). The current binary's BSS is
~500 KB; we're adding 700 KB. User-visible boot delay: ~25 ms additional
black-screen time before the engine takes over. Not a showstopper, worth
quoting in the plan so the perf engineer doesn't get surprised.

### 🟠 P3. The pack-header scan touches the CD at boot

63 pack-header reads = 63 CD seeks. Average PSX CD seek + read = ~150 ms.
That's ~9 seconds of additional boot. The plan calls this "tiny" without
measurement. **Cache the scan result** in a checked-in header (regenerated
when packs change), so boot doesn't pay this every time.

### 🟠 P4. 8-byte alignment is overkill on MIPS R3000A

R3000A has no hardware doubles. The largest scalar that needs alignment is
the 32-bit pointer or `uint32`, both 4-byte. Rounding sizes to 8 bytes
wastes ~4 bytes per alloc × ~40 SCENE allocs per scene × 63 scenes × 20
rotations ≈ ~200 KB of wasted alignment over a soak test. Tiny in steady
state but it does pad SCENE peak by a few percent. **Drop to 4-byte
alignment unless someone can name a specific 8-byte-aligned type we use.**

### 🟠 P5. "Why three regions, not 39 fixed pools?"

Every PSX game I shipped used per-type fixed pools, not regions. The plan
doesn't say why "three regions" beats "one fixed-size buffer per allocation
type." Pools have the same proof-of-fit story, simpler code (no
bump pointer logic), and zero risk of cross-purpose interference. The
plan should either argue against pools explicitly or adopt them.

### 🟡 P6. DCache and write-buffer behavior

PSX has a 1 KB direct-mapped DCache. A 1.2 MB region buffer in BSS thrashes
the cache on any region transition. The current heap is fragmented but
small allocations cluster in the same cache lines. Not a blocker; worth
benchmarking.

### 🟡 P7. "Allocator" reads like 400 LOC; my budget says 800

Real-world segregated free-lists with eviction integration, debug-mode
balance tracking, alignment math, telemetry, and the boot verifier sit
closer to 800-1000 LOC after corner cases. The plan's estimate is optimistic.
Not blocking, but the schedule should plan for 2-3 weeks of focused work,
not days.

---

## Reviewer 2 — Sarah, Site Reliability / Operations

> *Used to running cloud services with on-call rotations. Ports that
> mindset to whatever platform is in front of her.*

### 🔴 S1. There is no rollback story for Phase 2

Phase 2 deletes 23 call sites and ~hundreds of lines of fallback logic.
If Phase 2 ships and a class of regression appears two weeks later — a
specific scene only halts on a specific hardware revision, for example —
revert is a single multi-hundred-line patch that conflicts with anything
else merged in the meantime. The v0.8.10 → v0.8.11 rollback was tiny by
comparison and still required a dedicated point release.

**Mitigation I want before sign-off:**
- Ship Phase 2 in small, individually-revertable PRs grouped by file
  (one PR each for foreground_pilot.c, resource.c, ads.c, etc.).
- Tag the pre-Phase-2 commit explicitly so it's a one-flag revert target.
- Carry a separate "compat shim" branch where the deleted paths still
  exist behind `#ifdef KEEP_FALLBACKS`, so they can be flipped back
  without a full revert.

### 🔴 S2. PS1 has no telemetry channel from the field

`fatalError` on retail PS1 hardware shows a halt screen. There is no
phone-home. If the new allocator's boot proof fires on a user's hardware
because their CD reads pack headers differently than the test rig, we
hear about it via "the game won't start" complaints, not a stack trace.

**Mitigation:** the boot-proof `fatalError` messages must encode enough
context for an end-user to read off the screen and report: scene name,
region, bytes required, bytes available. Treat the halt screen as a
support ticket form.

### 🟠 S3. The grep gate is reactive, not preventive

Phase 2's CI guard is `grep -rE "JCSKIP|..."`. That catches a regressing
PR, but only after a contributor has already written the code. Better:
a `#define malloc PICKER_NO_MALLOC_...` pattern (from scene_picker.c)
applied at the project level — make `safe_malloc`/`malloc`/`free` outright
unavailable in src/ except where explicitly poison-bypassed. Catches the
regression at compile time, not in CI logs.

### 🟠 S4. "Independently shippable phases" is asserted, not demonstrated

The plan says Phase 1, 2, 3 are independent. But:
- Phase 1's `memVerifyScenesFitInScene` will `fatalError` at boot if call
  sites still go through libc malloc — which they do until Phase 2.
- The grep gate at Phase 2 verification depends on Phase 1 having
  migrated everything.
- Phase 3 audits "remaining surfaces" — which assumes Phase 1 missed
  something. If true, Phase 1 isn't actually complete.

The phasing is closer to a single 3-step migration than 3 ship-able units.
Be honest about that in the plan; don't promise "Phase 1 is shippable" if
it isn't.

### 🟠 S5. Soak-test signal is qualitative

"20+ iterations, band counts don't regress" is the gate. But the existing
JCPERF band system is a coarse signal — it'll catch dramatic regressions,
not subtle ones (a single scene 10% slower, a flicker, an audio glitch).
The no-fail invariant has higher stakes than the current code; the
validation bar should rise to match.

**Mitigation:** add a frame-level diff against a captured baseline for
each of the 63 scenes. If the bytes differ at any frame, fail the test.
The site already captures reference frames for some scenes; extend that.

### 🟡 S6. Schedule risk

The plan's three phases plus boot scan plus removal manifest plus
verification gates is — realistically — 4-6 weeks of focused work for a
single experienced engineer, not the "~400 LOC" estimate's implied days.
Communicate that to whoever's sponsoring this work.

---

## Reviewer 3 — Mateo, Future Maintainer (joins in 2 years)

> *Hired to add a Christmas-scene variant. Has never seen this code before.
> Reads the docs first.*

### 🔴 M1. The plan assumes I have the JSON; how do I regenerate it?

`docs/ps1/research/generated/scene_analysis_output_2026-03-21.json` is
load-bearing for `memVerifyScenesFitInScene`. The plan cites it but
doesn't say:

- What script generates it.
- When to regenerate it (after adding a scene? changing a TTM? changing
  a pack format?).
- What happens if I forget to regenerate it before merging a new scene.

If I add a scene and ship without regenerating the JSON, the boot proof
runs against stale data, passes, and the new scene either runs out of RES
or SCENE at runtime — but the plan says runtime can't fail. So either it
silently overruns (worst case) or `fatalError`s in the user's living room
(also bad).

**Required:** a "how to add a scene" section in the plan or in a
companion `CONTRIBUTING_SCENES.md`. Including the exact command to
regenerate the JSON and a CI check that the JSON is up-to-date for the
shipping packs.

### 🔴 M2. There is no "this is in development, don't fatalError" mode

If I'm prototyping a new scene that occasionally overruns SCENE, the
plan says the game halts. I can't iterate on visuals if I have to
shrink-fit my memory before I can see a frame. The plan needs a
`MEM_DEV_BUILD` flag that downgrades fatalError to warn-and-skip during
development, but keeps the strict invariant for release builds.

(This conflicts directly with the directive "no skip paths." Resolve by
making `MEM_DEV_BUILD` a build-time flag that is forbidden in CI/release.)

### 🟠 M3. The region naming is confusing

`MEM_REGION_PERM` / `MEM_REGION_RES` / `MEM_REGION_SCENE`. After 30 seconds
I know which is which, but the names don't tell me much:

- "PERM" — permanent... allocations? Permanent to what scope?
- "RES" — resources. Implies the LRU. Not clear it's a region with sub-allocation.
- "SCENE" — clearest of the three.

Better: `MEM_REGION_BOOT` (allocated at boot, never freed),
`MEM_REGION_CACHE` (LRU resource cache), `MEM_REGION_FRAME` or
`MEM_REGION_TRANSIENT` (per-scene scratch). The names should encode the
*lifetime contract*, not the contents.

### 🟠 M4. No worked example of adding an allocation

If I need to allocate a 4 KB buffer for caption pixels — which region?
Why? How do I decide? The plan describes the three regions but doesn't
walk through a decision tree for a new allocation site. Future maintainers
will end up guessing.

### 🟠 M5. The poison-malloc pattern fights me

Phase 1 adopts the poison pattern from `scene_picker.c` and applies it
project-wide. If I need to integrate a third-party library (compression,
text rendering, etc.) that calls libc malloc internally, I have to either
(a) port it to use `memAlloc`, or (b) carve out an exception. The plan
doesn't address how exceptions are granted.

### 🟡 M6. The 2026-03-21 in the JSON filename is a smell

A date in a generated artifact's filename implies the artifact is a
snapshot, not an evergreen. The plan treats it as evergreen ("verified
against"). If someone regenerates and writes
`scene_analysis_output_2026-09-14.json`, do the verify functions look up
the latest? Or are they hardcoded to that filename? Pin the convention
or drop the date.

---

## Reviewer 4 — Priya, Performance Engineer

> *Owns JCPERF/JCSKIP telemetry. Will not approve a change that adds frame
> time, even imperceptibly. Reads commit messages backward.*

### 🔴 PR1. `memSceneReset` adds 250 KB of write work at every scene transition

In debug builds with `MEM_DEBUG_SCENE_PINS`, you zero SCENE on reset for
hygiene. Even without that, the bump pointer reset is fast but the next
scene's first SCENE alloc has cold DCache. On PS1, a 250 KB zero pass is
~8 ms, which the soak test will eat at scene-transition time. Currently
that transition is one of the user-visible "pause" moments, so it may be
ok — but the plan says "small amount of time to clear memory between
scenes is acceptable, very small." 8 ms is at the edge of "very small."
**Quantify it before claiming "zero perf hit."**

### 🟠 PR2. Boot scan adds ~9 seconds, BSS clear adds ~40 ms

Per Pat's P3 and P2. The plan needs to land a worked CD-side measurement:
"boot takes N seconds today, N+M after this change." Without it the perf
engineer's review is "I don't believe you."

**Mitigation candidate:** the pack-header scan results are deterministic.
Generate `pack_header_metrics.h` offline (from the same tool that produces
the JSON), check it in, skip the boot scan entirely. Re-runs only when
packs change.

### 🟠 PR3. Pre-sized PERM frame buffers waste cache for non-worst-case scenes

If MARY's frame payload is the worst case at, say, 150 KB, and the median
scene is 60 KB, then 90 KB of PERM frame buffer is unused on the median
scene — but the DCache still maps it as "in PERM." Cache-eviction patterns
change. Possibly net-positive (one contiguous PERM beats fragmented
heap), but un-measured.

### 🟠 PR4. The hot path adds: alignment round-up, tag string store, balance bump

Per-alloc work in v3:
- Round size up to 8 (or 4 with my P4 fix): ~3 instructions.
- Store the const-char* tag somewhere (where?): the plan doesn't say if
  it's stored or just for telemetry. If stored, add a write per alloc.
- Balance counter increment for SCENE: 1 word write.
- Bump-pointer add + bounds check + fatalError-on-overflow: ~5 instructions.

Sum: ~10-12 instructions per alloc vs. ~50-100 for libc malloc on PsyQ.
That's a *win*, not a loss. Plan should claim the win explicitly.

### 🟠 PR5. RES eviction during a scene is a frame killer

If a scene's first new resource alloc triggers LRU eviction (free a TTM,
free a BMP), that eviction does a free-list update + memset + LRU scan.
On PS1, scanning all ~200 cached resources for the LRU candidate is
~3-5 ms. If it happens mid-frame, you drop a vblank.

**Mitigation:** stagger eviction. Have the LRU shed pre-emptively at
`memSceneReset` time when current RES usage is above a watermark. Pay
the eviction cost during the already-paused transition.

### 🟡 PR6. JCMEM behind FG_HEAP_PROBE_LOGS won't help production debugging

Per the v1 reviewer's note. Add a minimal JCMEM-on-fatalError print
that fires unconditionally so support tickets have data.

---

## Reviewer 5 — Alex, Adversarial Tester

> *Tries to break designs by finding the input the author didn't think of.*

### 🔴 A1. Stale scene_analysis JSON vs. shipped packs is a silent killer

The boot proof verifies against the JSON. If the JSON and the shipped
packs disagree (someone updated packs but not the JSON, or vice versa),
the proof passes on stale data and the runtime overruns silently. With
JCSKIP gone, "silently overruns" means heap corruption, not a clean halt.

**Required:** at boot, after reading pack headers, compute a hash of each
pack and verify against a hash in the JSON. If mismatch, fatalError with
"packs and analysis are out of sync — regenerate the JSON."

### 🔴 A2. Holiday/variant logic bypasses static analysis

`fgLoopApplyVariant` (jc_reborn.c:1911) mutates scene selection at runtime
based on holiday state. If a holiday variant pack has different memory
demands than the base pack — and the JSON only captures the base — the
boot proof is incomplete.

**Required:** the boot scan must include every variant of every scene,
not just the base set. The current JSON's schema doesn't appear to know
about variants (none of the JSON fields I sampled mention holiday). This
is a data gap that needs closing before Phase 1.

### 🔴 A3. Uninitialized SCENE bytes carry previous scene's data on PS1, zeros on PC

`memSceneReset` advances the bump pointer; it doesn't zero the bytes.
On PS1 the new scene's first alloc returns memory containing the previous
scene's pixels/strings/garbage. On PC, libc malloc behavior depends on the
allocator (glibc returns zero pages for fresh mmap, but reused chunks have
stale data); the PC stub forwards to malloc, so behavior diverges.

Code that accidentally reads uninitialized SCENE bytes:
- Passes on PC because malloc-ed bytes happen to be zero
- Fails on PS1 because they contain MARY's BMP fragments

**Required:** either zero SCENE on reset (Priya hates this — 8 ms) OR
zero each alloc inline (worse — 39 sites of memset). Better: poison the
SCENE region on reset with a known marker (0xCD bytes) in debug builds,
so the test catches "you forgot to initialize this buffer."

### 🟠 A4. Pause-during-scene flow is not analyzed

Pause menu interrupts scene playback. If pause-menu allocations land in
SCENE (the v3 Phase 3 audit hasn't decided yet), and the user pauses for
10 minutes, then un-pauses, the scene-in-progress still has its SCENE
allocations — but the menu also had SCENE allocs while paused. What does
`sceneAllocBalance` show? Does un-pausing trigger a partial reset?

The plan doesn't address pause as a state machine, only the
scene-N → scene-N+1 boundary.

### 🟠 A5. Corrupted CD reads at boot

PSX CDs vary in physical condition. A scratched disc could return garbage
pack headers. The boot scan computes huge buffer sizes from garbage,
`_Static_assert` is at compile time so it passes, runtime allocation
overflows the BSS buffer (since `MEM_REGION_TOTAL` is fixed but the scan
asked for more than that).

**Required:** pack-header scan must sanity-check each value against a
known max (`bufSize > MEM_REGION_TOTAL` ⇒ fatalError with "pack
corruption suspected"). User can re-seat the disc.

### 🟠 A6. RES LRU + memFree re-entrancy

`memAlloc(RES, n)` → no class match → calls LRU evictor → LRU calls
`memFree(RES, victim)` → `memFree(RES)` updates free-list → `memAlloc`
sees free-list change mid-iteration.

**Required:** the failure-contract pseudocode in the plan needs to explicitly
spell out the re-entrancy story. Either the evictor returns "I freed
N bytes, retry now" without itself calling memFree (cleaner), or memFree
is re-entrant-safe.

### 🟠 A7. BIOS / PsyQ internal allocations

PSX BIOS has its own memcard pool. PsyQ runtime has padmgr buffers.
If any of those grow during gameplay (controller plug events, memcard
swaps), they consume RAM outside our region buffer. The 1.2 MB usable
calculation assumed they're static — verify.

### 🟠 A8. TTM scripts could indirectly allocate

TTM is a scripted format. Some opcodes load resources. If an opcode
implementation in `ttm.c` does a `safe_malloc` outside the surveyed 39
sites, Phase 1's migration misses it. The plan should mandate a recursive
audit including all `*.c` files, not just the ones currently surveyed.

### 🟡 A9. The poison-malloc pattern is per-translation-unit

Defining `malloc` as a poison macro only affects the TU that included
it. Headers, third-party libs, and code that includes `<stdlib.h>` after
the poison can still call `malloc`. The plan should describe what the
poison actually catches vs. what it doesn't.

---

## Synthesis — the panel's consensus blockers

The five reviewers independently flagged 12 🔴-grade issues, several
overlapping. The five that the panel unanimously considers blocking
before Phase 1 implementation begins:

1. **🔴 P1 + A6 — ISR-safety + re-entrancy** must be explicitly answered.
   No `memAlloc` reachable from any interrupt; LRU eviction's recursive
   `memFree` either documented as safe or restructured to avoid the
   recursion.
2. **🔴 A1 + A2 — Pack hash + variant coverage** in the boot proof.
   The JSON must be hashed against shipped pack data; the data set must
   include every holiday variant. Without this, the no-fail invariant
   rests on stale data.
3. **🔴 A3 — Uninitialized SCENE bytes diverge between PC and PS1.**
   Zero or poison on reset (debug); document the contract for callers.
4. **🔴 S1 — Phase 2 rollback strategy.** Multi-PR split, tagged baseline
   commit, optional shim branch. Phase 2 is the riskiest patch in the
   project's history; it cannot land as a single change.
5. **🔴 M1 + M2 — Documentation for the future engineer.** "How to add a
   scene" doc + `MEM_DEV_BUILD` mode (degrades fatalError to warn in
   non-release builds) so the no-fail invariant doesn't block iteration.

The 🟠-grade items don't block, but the plan should address them in
text before Phase 1 work begins:

- **P3 / PR2:** check in pack-header metrics so boot doesn't pay 9 seconds
  of CD seeks.
- **P5:** explicit argument against per-type pools (or adopt them).
- **S3:** project-level malloc poison, not just CI grep.
- **PR1 / PR5:** measure scene-reset time; consider staggered LRU
  eviction.
- **A4:** pause-state machine analysis.
- **A5 / A7:** pack-corruption sanity checks; BIOS/PsyQ allocation
  audit.
- **A8:** the 39-site survey needs to be expanded to a full src/ audit,
  not relying on the existing grep.

The 🟡-grade items are hygiene; track in the implementation PR
descriptions.

## Recommendation

**Do not begin Phase 1 implementation until the five blockers above are
resolved in the plan.** The current v3 plan is the strongest version of
the design seen so far, but it inherits "we'll work it out during
implementation" risk on five points that the panel believes are
architecturally load-bearing.

If those five are answered (and the answers updated into v4 of the plan),
the panel would approve Phase 1 for implementation.
