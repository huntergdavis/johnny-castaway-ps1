# Scene Picker Algorithms — Research + Design (v2)

> Status: **shipped** as branch `scene-picker-policies-20260504`. PR1
> (Random + Sequential) and PR2 (Original Sierra state machine + Force
> Story Day) both landed; the runtime "validated/proven set" gate was
> retired in the same arc — see "Today's runtime" below.

## What we had before this work

`fgLoopNextScene(explicitScene, sceneSetIdx)` was a three-way fallback:

1. If a caller (Scene Explorer pin, CLI `fgpilot <slug>`) set
   `explicitScene`, replay that slug forever until cleared.
2. Otherwise, if the active Scene Set had a non-empty pool, return
   `pool[rand() % pool.count]`.
3. Otherwise, return `kProvenScenes[rand() % NUM_PROVEN_SCENES]` — a
   six-entry hand-curated walk-friendly fallback. This array is gone;
   `kAllScenes` (all 63 scenes that ship with FG2 packs) replaced it.

Wrapped around it: a story-day calendar tick, an island-position reroll
on sequence boundaries, a walk render to the next scene's start, the
FG2 pack play, then a position update for the next iteration. There is
no scene-repeat prevention, no FINAL/FIRST sequence structure, no
day-of-story enforcement (the check at `jc_reborn.c:1760` is a no-op
comment), and no walk-aware retry.

The original Sierra engine in `src/host/story.c:481-725` is the opposite
shape — a state machine that runs sequences of (6-19 intermediate scenes
+ 1 FINAL), filters by `FINAL` / `FIRST` / `LOWTIDE_OK` / `VARPOS_OK` /
`dayNo` per-pick, retries up to 8 times when a candidate has no valid
walk-in spot, and forbids same-tag-twice-in-a-row via `lastAdsName` /
`lastAdsTag` tracking. It still uses `rand() % matchingCount` at the
bottom — the difference from the PS1 picker is what the matching pool
contains, not the dice.

All 63 scenes have on-disc FG2 packs and run on the PS1 today; visual
validation is in flight and lands within days. The validated/unvalidated
distinction is therefore **not** a constraint on the picker design.

## Goal

A user-selectable scene-picker policy, exposed in the pause menu next to
Scene Set:

| Policy | Behavior |
|---|---|
| **Random** (current) | Uniform random over the active Scene Set pool. |
| **Sequential** | Cycle through the active pool in fixed order. |
| **Original** | The Sierra `story.c` algorithm: FINAL sequence boundaries, FIRST gating, walk-aware retry, repeat prevention, day-of-story filtering. |

Picker policy *composes* with Scene Set: the Set narrows the candidate
pool, the Picker decides how to draw from it.

Memcard persists the choice across boots. The pause-menu UX mirrors
Scene Set — Left/Right preview, Cross/Start commit, frog-clock
transition.

## Memory budget — the load-bearing constraint

Porting Sierra's state machine has to be **net-zero heap pressure**.
The PS1 already runs a tight steady-state allocation budget (~ 700 KB
in flight at peak scene playback) and the validation push has been
trimming, not growing, the heap. The picker can't add to that pile.

### Original algorithm static-budget analysis

`storyPickScene` already exists at `src/host/story.c:287` and is the
candidate to port. It works like this:

```c
static struct TStoryScene *storyPickScene(
        uint16 wantedFlags, uint16 unwantedFlags)
{
    int scenes[NUM_SCENES];        /* 252 bytes on stack */
    int numScenes = 0;

    for (int i = 0; i < NUM_SCENES; i++) {
        struct TStoryScene scene = storyScenes[i];
        if ((scene.flags & wantedFlags) == wantedFlags
            && !(scene.flags & unwantedFlags)
            && (scene.dayNo == 0 || scene.dayNo == storyCurrentDay)) {
            scenes[numScenes++] = i;
        }
    }
    return &storyScenes[scenes[rand() % numScenes]];
}
```

- **Heap allocations: 0**. Every byte is on the stack or in BSS.
- **Stack frame**: 252 bytes for `scenes[]` + 8 bytes for counters /
  return-pointer = ~260 bytes per call. Released on return.
- **BSS**: `storyScenes[63]` is already linked into the binary
  regardless of policy — it's the source of truth for `dayNo` /
  `flags` / `spotStart` / `spotEnd` we already query in `fgLoopWalkToScene`.
  No new BSS.
- **No string allocs**: `lastAdsName[13]` is also a stack-local in the
  caller. Per-sequence-level state (described below) needs to be
  promoted to module statics, but those add up to **~60 bytes total**.

### State to lift into module statics

The original `storyPlay` keeps sequence state on its stack frame
(because it owns the entire sequence loop). Our PS1 outer loop in
`jc_reborn.c` owns the per-iteration loop instead — so we have to lift
the sequence state into module statics that survive between calls. The
inventory:

| Variable | Size | Purpose |
|---|---|---|
| `pickerOriginalSequenceState` | 1 byte | enum: BOOT / INTERMEDIATE / FINAL_PENDING / SEQUENCE_DONE |
| `pickerOriginalIntermediatesRemaining` | 1 byte | counter, 0..19 |
| `pickerOriginalFinalScene` | 4 bytes | pointer to the final scene already chosen for this sequence |
| `pickerOriginalLastAdsName[13]` | 13 bytes | for repeat-prevention |
| `pickerOriginalLastAdsTag` | 2 bytes | uint16; 0xFFFF = unset |
| `pickerOriginalFirstSequence` | 1 byte | 0/1 — `unwantedFlags |= FIRST` while set |
| `pickerOriginalPolicyDirty` | 1 byte | 1 if user just changed picker; reset state on next call |
| **Total** | **~24 bytes** | All `BSS`, never heap. |

That's the entire memory cost of going from current Random to
full-Sierra Original. It is small enough to inline in the existing
`fgLoopNextScene` source file rather than introduce a new translation
unit; no new linker section, no new pool, no new buffer.

### Stack-frame check during scene play

The picker is called once per outer-loop iteration in `jc_reborn.c`'s
`do {} while(...)` body. The picker's 260-byte stack frame is reclaimed
before any FG2 / walk / scene-play work runs, so it does not race
peak-stack scenarios that already pile up during scene transitions
(BMP load + overlay compose + hold-state).

### Sequential mode is even cheaper

| Variable | Size | Purpose |
|---|---|---|
| `pickerSequentialCursor` | 2 bytes | 0..63 index into the active pool |

Total: 2 bytes. Cursor resets to 0 on Scene Set cycle.

### Build size

The Original-mode port reuses the already-linked `storyScenes[]` table
and adds one new picker function (~80 lines). Estimated `.text` growth
~1 KB; well within the slack the executable currently has.

## The four mechanics that have to be ported (Original)

Out of Sierra's `storyPlay` + `storyPickScene`, only four mechanics
need to come over. The rest of `storyPlay` is the ADS interpreter
glue, which the PS1 doesn't use.

### M1. Flag-based filtering with day-of-story

```c
matches = (scene.flags & wantedFlags) == wantedFlags
       && !(scene.flags & unwantedFlags)
       && (scene.dayNo == 0 || scene.dayNo == storyCurrentDay);
```

Direct port of `storyPickScene`. `storyCurrentDay` already ticks
(`fgLoopAdvanceStoryDayIfNeeded` in `jc_reborn.c:1735`), it just isn't
read by the picker today.

### M2. Sequence structure

A "sequence" is exactly one FINAL scene preceded by 6 + rand()%14
intermediates. Implementation:

```c
if (sequenceState == BOOT || sequenceState == SEQUENCE_DONE) {
    /* pick FINAL up front, store pointer, count down intermediates */
    finalScene = pickFinal(...);
    intermediatesRemaining = 6 + rand() % 14;
    sequenceState = INTERMEDIATE;
}
if (sequenceState == INTERMEDIATE) {
    if (intermediatesRemaining > 0) {
        intermediatesRemaining--;
        return pickIntermediate(...);
    }
    sequenceState = FINAL_PENDING;
}
if (sequenceState == FINAL_PENDING) {
    sequenceState = SEQUENCE_DONE;  /* next call starts a new sequence */
    return finalScene;
}
```

The `BOOT` and `SEQUENCE_DONE` states are functionally identical at
the entry point — they exist as separate enum values only so we can
distinguish "user just selected Original mode for the first time"
from "we just finished a sequence." That distinction matters for
`firstSequence` (M4 below).

### M3. Walk-aware retry

The original loops up to 8 times to reject candidates whose
`spotStart` is missing when transitioning from a defined position, or
whose `spotEnd` is missing (no walk-out for next iteration), or that
repeat the previous scene's ADS+tag.

We can lift this verbatim from `src/host/story.c:589-596` — it's a tight
loop on `storyPickScene` with no allocations.

### M4. FIRST gating

`unwantedFlags |= FIRST` on every pick except the boot-of-sequence
final scene. The PS1 already special-cases FIRST in the walk path
(`fgLoopWalkToScene` skips the walk when the next scene has FIRST).
Adding the same flag to the wantedFlags computation is a one-line
change.

## Pause-menu UX

Add **Scene Picker** to the main pause menu. Lives between
**Scene Set** and **Freeplay** since it's conceptually a Scene Set
modifier. Same Left/Right preview + Cross/Start commit pattern, same
frog-clock transition on commit:

```
> Scene Picker: < Random >
```

Cycle through Random / Sequential / Original. No sub-screen needed —
each policy is parameterless from the user's standpoint.

If the main menu is too tight (it's at 8 items already), nest under
Scene Set: pressing Right on Scene Set cycles its value, pressing
Triangle opens a "Scene Set Options" sub-screen with both Set and
Picker. Decide on visual signoff.

## Implementation surface

| File | Change |
|---|---|
| `src/jc_reborn.c` | Replace `fgLoopNextScene` with a dispatcher that calls `pickerRandom` / `pickerSequential` / `pickerOriginal`. Lift the walk-aware retry helper into a shared inline. |
| `src/scene/scene_picker.c` (new, optional) | All three picker bodies + module statics. Could just live inline in `jc_reborn.c` if it stays small. |
| `src/scene/scene_picker.h` (new, optional) | Public surface: `pickerNextScene`, `pickerSetPolicy`, `pickerOnSceneSetCycle`. |
| `src/pause_menu/pause_menu.h` | New `pauseMenuPickerPolicy` int + `pauseMenuRequestPickerCycle` flag. |
| `src/pause_menu/pause_menu.c` | New `MENU_SCENE_PICKER` row, optional sub-screen if nested. |
| `src/platform/ps1/memcard.c` | Persist picker policy. Schema bump. |
| `docs/ps1/scene-picker-design.md` | This file. |
| `docs/ps1/pause-menu-design.md` | New picker section after Scene Set. |
| `site/help/menu/index.md` | New menu-help entry. |

The Original-mode helpers (`storyHasValidStart`, `storyHasValidEnd`)
already build into the PS1 binary — `story.c` is in the link list even
though most of it is dead-code. We pull two existing functions, no
new C bodies.

## Migration path

1. Random + Sequential first — both small, both ship in one PR. Pause
   menu cell. Memcard persistence. ~1 day.
2. Original second — port `storyPickScene` + sequence state machine +
   walk-aware retry + repeat prevention. Wire `storyCurrentDay`. ~2-3
   days. Memory probe (JCMEM heap before/after) to verify zero
   pressure delta vs. Random.
3. Force Story Day debug knob third — optional, makes day-of-story
   testable in minutes rather than days.

## Red team v2

### R1. Original mode + small Scene Set pool starves walk retry

If the user selects "Misc & Suzy" (4 scenes) + Original, the 8-retry
walk-aware-pick loop can run out of distinct candidates fast — the
fourth retry has only previously-rejected scenes to choose from.

**Fix**: cap retry budget to `min(8, pool.count - 1)`. After that,
accept whatever last pick came back; the walk fallback already
handles missing start spots by no-op'ing the walk render.

### R2. Original mode within "Fishing Only" Set has no FINAL scene

Original requires picking a FINAL scene to anchor each sequence. If
the active Scene Set has *no* FINAL-flagged scenes, sequence state
deadlocks at the BOOT step.

Looking at `storyScenes[]`, the families with FINAL-flagged members:
fishing 3/4/5/6, johnny 1, johnny 2, mary 5, miscgag 1/2, several
visitor and activity scenes, every stand and walkstuf. So most Sets
have at least one FINAL.

But "Mary Visits" has only mary5 with FINAL (the others are
intermediates). If the active Set is "Mary Visits" + Original, the
sequence will play one FINAL (mary5) every iteration with zero
intermediates. That's a degenerate sequence but not a crash.

**Fix**: when fewer than 2 FINAL candidates exist in the pool, skip
the sequence structure and degrade to Random within the pool for that
Set. Surface this as a JCPICK telemetry line so it's visible in logs.

### R3. Memory budget honored — *but* `storyScenes[]` already counted

The static-budget analysis assumes `storyScenes[]` is already linked
in. That's true today (`fgLoopWalkToScene` reads its `spotStart`/
`hdgStart` fields, so the table can't be dead-code-eliminated). If a
future refactor strips it (e.g., a smaller PS1-specific scene table),
Original mode would need to relink it — adding ~63 × sizeof entry =
~3 KB BSS.

**Fix**: add a regression assertion in `scripts/release.sh` that
checks `storyScenes` symbol presence in `jcreborn.elf`. Cheap
guardrail.

### R4. `storyCurrentDay` plumbing crosses a wall

The picker reads `storyCurrentDay`. The day tick lives in
`fgLoopAdvanceStoryDayIfNeeded`. Today nothing else reads it, so the
variable is effectively private. Original mode makes it cross-module.

**Fix**: expose `int storyCurrentDay` (already a global in `story.c`)
via `story.h`. No header file gymnastics — the symbol is already
there.

### R5. Memcard schema migration

Picker policy adds one field to the save format. We've bumped before
(when Scene Set landed); same pattern: bump version byte, default
missing field to Random on load.

**Fix**: increment `MEMCARD_SCHEMA_VERSION` from N to N+1; in load
path, if read schema < N+1, default `pauseMenuPickerPolicy` to
`PICKER_RANDOM`. No data loss.

### R6. Sequential cursor across Scene Set cycle

User picks Sequential, plays through fishing 1..5, then cycles Scene
Set to "Johnny Stories". What's the cursor at? Index 5. If we don't
reset, Johnny Stories starts at index 5 = `johnny6`, not `johnny1`.

**Fix**: reset cursor to 0 on `pauseMenuRequestSceneSetCycle`. Same
hook the existing scene-set transition uses.

### R7. Pause-menu real estate is tight

Main menu is at 8 items (Resume / Scene Set / Scene Explorer /
Freeplay / Freeplay Options / World Options / Accessibility / System).
Adding Scene Picker makes 9.

**Options:**
- (a) Add a 9th row, accept the overflow.
- (b) Nest Scene Picker under Scene Set as a "Scene Set Options"
  sub-screen.
- (c) Combine Scene Set + Scene Picker into a two-row "Random Mix"
  sub-screen accessed from a single main-menu entry "Scene Mix".

**Recommendation**: (b). The two controls are conceptually paired —
Set narrows, Picker chooses how. Nesting them makes the relationship
explicit.

### R8. Repeat-prevention as a free Random win

The original's `lastAdsName` / `lastAdsTag` repeat-prevention is six
lines of code. There's no reason it should be Original-only. Apply
to Random too — picking the same scene twice in a row is always
worse than picking a different one.

**Fix**: Random and Original both consult the same `lastAdsName` /
`lastAdsTag` statics. Sequential doesn't need it (cursor advances by
1 each call).

### R9. Original mode interacts with the Scene Explorer pin

`pauseMenuRequestPlayScene` and `pauseMenuRequestLoopScene` set
`explicitScene` and short-circuit the picker entirely. That's
correct — pinning trumps policy. But the new dispatcher must keep
the `if (explicitScene) return explicitScene` line as the very first
check, before any policy dispatch. If we forget, Original mode will
override the user's explicit pick.

**Fix**: explicit unit-test-style assertion in the dispatcher that
explicitScene wins.

### R10. Force Story Day knob — when does it ship?

Original mode + day-of-story filtering means JOHNNY 2 only fires on
day 2, MARY 1 only on day 5, etc. Real-world testing of day filtering
takes days. Without an override, validating Original mode is
impractical.

**Fix**: ship the System → Force Story Day knob in the same release
as Original. Easy hook — `storyForcedCurrentDay` already exists at
`src/host/story.c:312`, just needs a pause-menu front-end.

### R11. Sequential mode's "order" is unspecified

The pool's order today is the order we authored in
`gSceneSetPools[]`. For Fishing Only that's `fishing1..fishing8`,
which feels natural. For "All Scenes" the order is whatever
`kAllScenes[]` lists (alphabetical-by-family-then-tag — activity1
through walkstuf3); it cycles all 63 in ~14 minutes of playback
before wrapping.

**Recommendation**: Sequential cycles through the pool in
authored order. Document this. If users want alphabetical, that's a
v2 sub-option.

### R12. Memory probe ground truth

The "zero heap pressure delta" claim has to be verified, not assumed.
The `JCMEM` telemetry line probes `mallinfo()` at scene transitions
already. Add an at-boot probe that records steady-state heap, then
run a 30-minute soak in each policy and confirm equivalence (within
margin).

**Fix**: bake a soak-test target into `scripts/regtest-soak-pickers.sh`
that runs each policy for N minutes and dumps `JCMEM` deltas. Land
this with the Original PR.

### R13. ADS terminology vs slug terminology

Sierra's `lastAdsName` is the .ADS file basename (`"FISHING.ADS"`),
not the slug (`"fishing1"`). The PS1 uses slugs everywhere. We need
to map slug ↔ ADS name for repeat-prevention. The mapping is in
`storyScenes[]` (the `adsName` field). Easy lookup but a real
crossing.

**Fix**: repeat-prevention compares `(scene->adsName, scene->adsTagNo)`
tuples directly — never converts to slug. The PS1 outer loop only
needs the slug for FG2 path lookup. Two namespaces, one mapping
function (`fgLoopFindStorySceneBySlug`, already exists at
`jc_reborn.c:393`).

### R14. Sequence boundaries vs scene-set cycles

The user changes Scene Set mid-sequence. What happens to the in-flight
Sierra sequence?

Option A: scene-set cycle resets sequence state to BOOT. Next pick
starts a fresh sequence in the new pool. Cleanest.

Option B: sequence continues — but the FINAL scene we picked at
sequence start may no longer be in the new pool. That's a runtime
bug.

**Recommendation**: A. `pauseMenuRequestSceneSetCycle` already resets
position state, walk cache, and explicit-scene pin; resetting picker
sequence state fits the same pattern.

### R15. `firstSequence` semantics on policy switch

User boots in Random, plays for an hour, switches to Original. Is
this the "first sequence" in the original sense (allow FIRST-flagged
scenes)? Probably not — the screensaver has been running, FIRST
shouldn't fire mid-stream.

**Fix**: `pickerOriginalFirstSequence = 0` whenever the policy is
**switched to** Original from another. Only set to 1 at boot or after
explicit "Reset Picker" (no UI for that today, fine).

### R16. JCPICK telemetry surface

Validating Original mode requires understanding which scenes the
picker chose, why, and what flags each pick used. Without telemetry,
debugging will be guess-and-check. The existing `JCPAUSE`, `JCMEM`,
`JCWALK` patterns are right model.

**Fix**: emit `JCPICK policy=N seqState=N intsLeft=N picked=<slug>
flags=0x<hex> retries=N` per pick. Log volume is one line per scene
boundary — negligible.

### R17. Hard guarantee: no malloc in any picker path

The static-budget analysis is theoretical — has to be enforced.
`scene_picker.c` should `#define malloc() PICKER_NO_MALLOC` at the
top, with `PICKER_NO_MALLOC` expanding to a compile error. Same for
`free`, `realloc`, `calloc`. Belt-and-suspenders against future
contributors adding "just one little allocation."

**Fix**: include a poison-include guard. Trivial.

## TLDR

Three picker policies — Random (current), Sequential (cycle pool in
authored order), Original (port Sierra's `storyPickScene` + sequence
state machine). Picker policy *composes* with Scene Set — Set narrows
the candidate pool, Picker decides how to draw from it. UI: pause-menu
cell, nest under Scene Set in a "Scene Set Options" sub-screen.
Memcard persists.

**Memory budget audit:** Original mode adds ~24 bytes of module
statics, ~260 bytes of stack frame (released after every pick),
**zero heap allocations**. `storyScenes[]` is already linked.
`.text` grows ~1 KB. The poison-include guard at the top of
`scene_picker.c` enforces "no malloc" structurally.

**Implementation order**: Random + Sequential in PR 1 (~1 day);
Original + Force Story Day debug knob in PR 2 (~2-3 days, including
the soak-test guard that verifies zero heap pressure delta vs.
Random); both PRs gate behind pause-menu visual signoff.

**Three things I'd do before any code lands:**

1. Confirm pause-menu nest pattern (R7): main-menu **Scene Set Options**
   sub-screen carries both Set + Picker. Avoids 9-row main menu.
2. Add `JCPICK` telemetry + the malloc-poison guard (R16, R17) **in the
   first commit**, not retroactively — they're cheap and keep the
   memory-safety claim honest from day one.
3. Ship `Force Story Day` debug knob with Original (R10) — without it,
   day-of-story validation is glacial.
