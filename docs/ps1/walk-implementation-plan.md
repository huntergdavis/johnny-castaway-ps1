# Walk-Connected Story Loop — Implementation Plan

> 🌐 **Rendered version:** **[/source/docs/ps1/walk-implementation-plan/](https://hunterdavis.com/johnny-castaway-ps1/source/docs/ps1/walk-implementation-plan/)** — this doc rendered on the project website's source library. The GitHub copy here is the source.

Date: 2026-04-29
Status: planned, not started
Owner: PS1 perf branch

## Executive Summary

The PS1 build currently picks each scene at random and teleports Johnny
between them. The original Sierra screensaver instead chains scenes together
visibly: Johnny ends one scene at a known spot and heading, then walks
across the island to the next scene's start. The walks are part of what
makes the screensaver feel like a place rather than a clip reel.

This plan ports the walking system to the PS1 build without touching the
FG2-pack replay path the perf branch depends on. The scope is the cleanest
hybrid available:

- Bring the existing `src/walk.c` + `src/walk_data.h` + `src/calcpath.c`
  modules into the PS1 build.
- Ship `JOHNWALK.BMP` on the disc.
- Add a tiny scene picker that maintains Johnny's current spot/heading and
  picks the next FG2 pack with a reachable start.
- Add a walk-rendering path on PS1 (sprite-driven, not pack-driven).
- Trigger footstep sounds during walks, matching the original engine's
  sample table.
- Implement the 11-day story calendar so JOHNNY/MARY/SUZY narrative beats
  fire on the right days.

The story.c / ads.c / ttm.c bytecode interpreters stay out. The host build
keeps using them for capture; the PS1 keeps using FG2 packs for replay.
Only the walk-and-pick subset crosses over.

## 1. Background — how the original engine does it

### 1.1 Scene metadata is pre-declared

`src/story_data.h` declares all 63 scenes with start/end position metadata:

```c
struct TStoryScene {
    char adsName[13];   // e.g. "FISHING.ADS"
    int  adsTagNo;      // 1..N within file
    int  spotStart;     // SPOT_A..F or 0 for "anywhere"
    int  hdgStart;      // HDG_S..SE  or 0 for "anywhere"
    int  spotEnd;       // SPOT_A..F or 0 for "left island / no defined end"
    int  hdgEnd;
    int  dayNo;         // 0 = any, 1..11 = required story day
    int  flags;         // FINAL | FIRST | ISLAND | LEFT_ISLAND | VARPOS_OK
                        // | LOWTIDE_OK | NORAFT | HOLIDAY_NOK
};
```

Six labeled spots on the island (`SPOT_A..F`) and eight headings
(`HDG_S, SW, W, NW, N, NE, E, SE`).

### 1.2 The walking system is a self-contained module

`src/walk.c` is 189 lines. `src/walk_data.h` is 530 lines of static const
data extracted from Sierra's original `SCRANTIC.SCR` — per-frame
`(sprite_index, x, y, frame_id)` for every walk transition plus turning
poses. `src/calcpath.c` does shortest-path through the 6-spot graph.

`adsPlayWalk(fromSpot, fromHdg, toSpot, toHdg)` in `src/ads.c:2216`
orchestrates: load `JOHNWALK.BMP` into a TTM slot, call `walkInit()`, loop
`walkAnimate()` per frame until it returns `delay=0`. The PS1-specific
ifdefs in `walk.c` already partially anticipate the port (sufficient
for graphics_ps1 integration; the SDL surface dependency at line 80 is
the lone holdout).

### 1.3 The story loop chains scenes together

`storyLoop()` in `src/story.c:560-690`:

```
1. Pick a "final scene"
2. If the final scene isn't FIRST, fill 6 + rand()%14 = 6-19 intermediate
   scenes that lead up to it.
3. For each intermediate scene:
    a. Pick a candidate respecting wantedFlags / unwantedFlags
       (LOWTIDE_OK if low tide, VARPOS_OK if island shifted, not FINAL,
       not FIRST after the first iteration, not the same as last)
    b. If we have a prevSpot: adsPlayWalk(prev, scene.start)
    c. Play the scene
    d. Update prevSpot/prevHdg from scene.spotEnd/hdgEnd
       (or to -1 if scene has no defined end — LEFT_ISLAND, etc.)
4. Walk to final scene start, play it.
```

That's the entire story-loop algorithm. Story-day advances inside
`storyUpdateCurrentDay()` (file-level, deterministic), and limits which
scenes can be picked via the `dayNo` field.

## 2. Current PS1 state

`CMakeLists.txt` SOURCES does NOT include `story.c`, `walk.c`, `ads.c`,
`ttm.c`, `calcpath.c`. The PS1 build is the FG2-pack replay path only.
Scene picking lives in one function:

```c
// src/jc_reborn.c:196
static const char *fgLoopNextScene(const char *explicitScene)
{
    if (explicitScene && explicitScene[0] != '\0')
        return explicitScene;
    return kProvenScenes[rand() % NUM_PROVEN_SCENES];
}
```

`kProvenScenes[]` currently holds `{"fishing1", "fishing2"}`. Random pick,
no spot tracking, no walks, no story day, no FIRST/FINAL gating.

The host capture pipeline does go through `storyLoop()` — meaning each
FG2 pack already starts with Johnny in his correct pose for that scene.
The walks BETWEEN scenes were never captured into packs; they are
literally the connective tissue. That is the gap.

## 3. Architecture choice — port walk.c only

Three options were considered:

| Option | Description | Verdict |
|---|---|---|
| A. Port the full engine | story.c + walk.c + ads.c + ttm.c + RESOURCE.001 on disc | rejected — defeats the hybrid pipeline; explodes RAM and code budget |
| B. Pre-bake every walk as an FG2 pack | one walk pack per (fromSpot, toSpot) directed edge, runtime picks | possible — but 60-100 new packs, capture harness churn, walks aren't perfectly position-independent |
| C. Port walk.c + tiny picker only | walk module is self-contained, scene picker is ~100 lines | **chosen** |

Option C keeps the scene replay path on FG2 packs and adds a separate,
sprite-driven walk render path. Walk frames are short (10-30 VBlanks
each, 1-5 walks per scene transition); the perf optimizations the perf
branch built up for FG2 don't apply, but they don't need to — walks are
not the cost surface.

## 4. Implementation phases

Each phase ends in a clean commit. Phases land in order; later phases
can defer if perf or schedule pressure says so.

### Phase 1 — Assets + build integration (~1 day)

**Goal:** walk module compiles into `jcreborn.bin`.

1. Add to `CMakeLists.txt` SOURCES:
    - `src/walk.c`
    - `src/calcpath.c`
   (`walk.h`, `walk_data.h`, `calcpath.h` are headers — pulled in
   transitively.)

2. Audit `walk.c:80` — `SDL_Surface *sfc` parameter. The PS1 build has
   no SDL_Surface type. Fix one of two ways:
    - Cleanest: hoist the surface parameter behind `#ifndef PS1_BUILD`
      so PS1 calls drop the argument; or
    - Stub: typedef `SDL_Surface` to `void` under `PS1_BUILD` and let
      the unused parameter warn-but-not-fail.
   The first is cleaner and is the path the rest of the codebase uses.

3. Audit `walk.c` graphics calls:
    - `grClearScreen(sfc)` — guarded `#ifndef PS1_BUILD` already (line 162)
    - `grDrawSpriteFlip(sfc, ...)` and `grDrawSprite(sfc, ...)` — these
      need either PS1 equivalents or to drop the `sfc` parameter under
      `PS1_BUILD`. Confirm `graphics_ps1.h` exports them and adapt.

4. Add `JOHNWALK.BMP` to `config/ps1/cd_layout.xml` so it ships on disc.
   Verify size and that `cd_layout.xml` doesn't push other assets across
   sector boundaries (CD layout is perf-sensitive on this branch — see
   the experiment log's notes about LBA shifts).

5. Verify `JOHNWALK.BMP` color depth and palette. Sierra art is 8-bit
   indexed; the project's existing PS1 sprite path expects PAL4 or
   indexed8 chunks. If `JOHNWALK.BMP` is straight 8-bit without per-tile
   palette analysis, decide whether to:
    - Pass it through `scripts/transcode-bmp-ps1.py` like other BMPs, or
    - Use a simpler one-CLUT loader in `grLoadBmp`.

**Acceptance:**
- `jcreborn.bin` builds without warnings related to walk.c
- `JOHNWALK.BMP` is on the disc image, loadable by `grLoadBmp()`
- Boot a regression scene (`fishing1`) — no behavioral change, just
  proving the new code is on the disc and not regressing anything.

### Phase 2 — Walk rendering on PS1 (~2-3 days)

**Goal:** call a walk sequence from a test boot token and see Johnny
walk visibly across the island.

1. New file `src/walk_pilot.c` (or extend `foreground_pilot.c`).
   Public API:
   ```c
   int fgWalkRender(int fromSpot, int fromHdg, int toSpot, int toHdg);
   ```
   Returns 0 on success, non-zero on harness-abort signal.

2. Implementation outline (mirrors `adsPlayWalk()` in `src/ads.c:2216`
   but without the SDL/TTM thread machinery):
   ```c
   walkInit(fromSpot, fromHdg, toSpot, toHdg);
   int delay = walkAnimate(/*ttm slot params*/);
   while (delay > 0) {
       eventsWaitTick(delay);   // wait the right number of VBlanks
       grBeginFrame();
       grRestoreBgTiles();      // restore island background
       /* draw walking sprite from JOHNWALK */
       /* ...holiday overlay from existing path... */
       grEndFrame();
       delay = walkAnimate(/*params*/);
   }
   ```

3. Coordinate offset: the walk_data.h coordinates are absolute
   640×480 pixels. The runtime offsets by `islandState.xPos` and `yPos`
   (stored in `grDx`/`grDy`) so walks track the randomized island
   position. Confirm the existing PS1 walk is wired the same way.

4. Sprite rendering on PS1: `grDrawSprite()` in `graphics_ps1.c` should
   already accept walking sprites if they share the same atlas format
   as scene foregrounds. If `JOHNWALK.BMP` decodes to multiple sub-frames,
   the slot's `numSprites` count must be correctly populated.

5. Add a boot-token test mode for development:
   ```
   walk-test fromSpot fromHdg toSpot toHdg
   ```
   e.g. `walk-test E E A S` runs a single walk for visual review.

**Acceptance:**
- Boot `walk-test E E A S` and see Johnny walk from spot E heading east
  to spot A heading south.
- Compare frame-by-frame against the host build's same walk. Pixel-exact
  match would be ideal; visually-identical-at-2× is acceptable for
  promotion.
- No VRAM corruption, no crash, no audible artifacts.

### Phase 3 — Scene picker with spot/heading tracking (~1 day)

**Goal:** the screensaver loop uses Johnny's current spot to pick the
next scene, walks to that scene's start, plays it, repeats.

1. Reuse `storyScenes[]` directly. `story_data.h` is data only —
   include it from PS1 with no other story.c dependency.

2. New static state in `src/jc_reborn.c` or `src/walk_pilot.c`:
   ```c
   static int storyCurrentSpot = SPOT_E;   // safe default; FISHING1 starts here
   static int storyCurrentHdg  = HDG_W;
   static int storyCurrentDay  = 1;        // populated by Phase 8
   ```

3. Replace `fgLoopNextScene()` with a version that:
    - Walks `storyScenes[]` for entries matching `kProvenScenes[]`
    - Filters by current variant flags (LOWTIDE_OK, VARPOS_OK,
      `dayNo == 0 || dayNo == storyCurrentDay`)
    - Excludes FINAL on intermediate picks, excludes FIRST after the
      first iteration (mirrors the original's `wantedFlags`/
      `unwantedFlags` logic at `src/story.c:567-575`)
    - Picks one at random from the candidates
    - Returns its `TStoryScene*`

4. New screensaver-loop step: between scene plays, call
   `fgWalkRender(storyCurrentSpot, storyCurrentHdg, scene.spotStart,
   scene.hdgStart)` if the previous scene had a defined end and the
   next scene has a defined start.

5. After playing scene N's FG2 pack, update
   `storyCurrentSpot/Hdg = scene.spotEnd/hdgEnd` (or set to -1 / "no
   walk possible" if the scene has no defined end — LEFT_ISLAND scenes,
   FINAL scenes that don't reset, etc.).

6. The 8-pick retry limit from the original story loop
   (`src/story.c:583`) is preserved; if 8 picks fail to satisfy filters,
   accept anything that matches `wantedFlags`. Original behavior.

**Acceptance:**
- Boot `fgpilot` with no explicit scene. The screensaver runs through
  multiple scenes WITH walks visibly connecting them.
- Walk from FISHING 1's end (D-east) to FISHING 2's start (D-west) —
  this is a turn-in-place at spot D. Walk to a new spot and back works
  cleanly.
- No teleportation. Position state remains consistent across iterations.

### Phase 4 — Footstep sounds (~1 day)

**Goal:** matching the original engine, walks emit the correct footstep
samples on the correct frames.

1. Identify the walk audio in the original `RESOURCE.001`. The
   `JOHNWALK` walking animation should fire footsteps at known frame
   intervals. Reverse-engineer from:
    - The host build's behavior — capture audio during a walk, identify
      sample IDs from the SPU state.
    - Sierra's original `SCRANTIC.SCR` — the same source `walk_data.h`
      was extracted from. Check `extract_walk_data.c` if it preserved
      audio cues.

2. Add a `walkSoundEvents[]` table next to `walk_data.h` if needed:
   ```c
   struct TWalkSoundEvent { int frameIdx; int sampleId; };
   ```
   keyed by current spot-pair edge.

3. Hook into `walkAnimate()` (or wrapped in `fgWalkRender()`'s loop) so
   that when the walk frame index hits a sound trigger, call
   `soundPlay(sampleId)` to fire the SPU sample.

4. Verify the relevant footstep VAGs are already preloaded. The PS1
   build preloads "all 23 SFX VAGs at boot" per `hardware-specs.md`; if
   footsteps are among them, no asset work needed. If not, add to the
   boot preload list.

**Acceptance:**
- Walks have audible footsteps that match the rhythm of the leg
  movement.
- No SPU pop, click, or stuck note. Verify in DuckStation and (if
  available) on hardware.
- Footstep timing matches host build within ±1 VBlank.

### Phase 5 — Behind-tree compositing (~1-2 days)

**Goal:** when Johnny walks between spots 3 and 4 (the path that goes
behind the palm tree), the tree visibly covers him at the right
points.

1. The original behind-tree code is `walk.c:103-104`:
   ```c
   isBehindTree = ((currentSpot == 3) && (nextSpot == 4))
                 || ((currentSpot == 4) && (nextSpot == 3));
   ```
   When set, `walkAnimate()` draws extra cover-up sprites (`walk.c:174`):
   ```c
   grDrawSprite(sfc, ttmBgSlot, 442, 148, 13, 0);  // trunk
   grDrawSprite(sfc, ttmBgSlot, 365, 122, 12, 0);  // leafs
   ```
   These sprites are pulled from the BACKGROUND TTM slot. Drawing them
   AFTER the walking sprite means they cover Johnny at the right pixels.

2. On PS1, the question is whether `grDrawSprite()` calls in sequence
   can produce the desired overlap. The PS1 GPU's ordered-table model
   has explicit z-priority, but if the walk render simply submits to
   the OT in order (walk sprite first, then trunk + leaf sprites), the
   trunk and leaf will draw in front. Confirm by experiment.

3. The trunk and leaf sprites must be available in a TTM slot that's
   resident during the walk. On host the background slot
   (`ttmBackgroundThread.ttmSlot`) holds the island scenery; on PS1 the
   equivalent is the loaded background asset. Need a loader path that
   extracts the trunk and leaf sub-sprites from the island background
   atlas and keeps them callable during walks.

4. Alternative if (3) is too invasive: pre-extract the trunk and leaf
   sub-sprites at pack-build time and ship as a small new asset
   (`JOHNTREE.BMP`?) loaded alongside `JOHNWALK.BMP`.

5. Validate visually with regression: walk from spot 3 to spot 4 and
   reverse. Johnny should disappear behind the palm trunk and reappear
   on the other side, matching host frame-by-frame.

**Acceptance:**
- Behind-tree walks render correctly: Johnny is hidden behind the
  trunk and leaf cluster at the right moments.
- No z-fighting, flickering, or one-frame visibility glitches.
- Same trunk position whether Johnny is or is not currently behind it.

### Phase 6 — Holiday overlay during walks (~½ day)

**Goal:** the active holiday emblem stays on screen during walks (it
belongs to the island, not to a specific scene).

1. The PS1 runtime currently renders the holiday overlay during scene
   playback via the existing graphics_ps1 path. During walks, the same
   overlay path should fire — walks just don't touch it.

2. In `fgWalkRender()`'s frame loop, after the walking sprite is
   composed, call the existing holiday overlay function. If holiday
   overlay is currently embedded in scene playback rather than the
   per-frame compositor, refactor to a callable used by both scene and
   walk paths.

3. Verify with `holiday christmas` boot token: walk transitions show
   the Christmas tree at the same coordinates as the scenes that
   surround them.

**Acceptance:**
- Holiday emblem persists across scene→walk→scene transitions with no
  flicker or position jump.
- Works with all 36 holidays from `holidays.yml`.

### Phase 7 — Validation harness (~2-3 days)

**Goal:** walks are part of the FISHING-1-bar validation set.

1. New regtest cases for each used walk edge. Boot string format:
   ```
   walk-test E E A S noloop perf-log
   ```
   where (E, E, A, S) = (fromSpot, fromHdg, toSpot, toHdg). One case
   per (fromSpot, toSpot) pair seen in connecting validated scenes.
   Initially that's a small set — fishing1 ↔ fishing2 ↔ fishing3 share
   spot D, so the only walk to test in MVP is the turn-in-place at D.

2. As `kProvenScenes[]` grows, the regression set adds the new edges
   automatically. Tooling: a small Python script
   `scripts/walk-coverage-from-proven.py` reads `kProvenScenes` plus
   `storyScenes[]`, emits the union of edges, prints the regtest cases
   needed to cover them.

3. Walk reference frames: capture the host build's same walk via
   `walk-test` mode. Pixel-diff against PS1 capture, same FISHING 1
   bar.

4. Add walk-end → scene-start continuity check: the last walk frame's
   Johnny pose must match the scene FG2 pack's frame 0 within a small
   pixel tolerance (Sierra's captures aren't bit-exact between phases
   either). If the gap is non-trivial, the walk's "hands in pockets"
   final pose may need a 1-frame settle or a different final-pose
   selection.

5. Add walks to the dirty-region template if the existing template
   format is shared between scenes and walks. Currently the template
   is FG2-pack-specific; walks may need their own dirty layout (or a
   simpler "full active region" mode).

**Acceptance:**
- All currently-used walk edges have a regtest case.
- All cases pass pixel-perfect against host reference.
- Walk-to-scene-frame-0 transitions show no visible pop.

### Phase 8 — 11-day story calendar (~3-5 days, last phase)

**Goal:** the JOHNNY/MARY/SUZY narrative beats fire on the right days
(Mary visit on day 5, Suzy visit on day 3, raft progress, Johnny
intro/outro), matching the original.

1. Port `storyUpdateCurrentDay()` (and any helpers it calls) from
   `src/story.c` into a PS1-compatible form. This is the function that
   advances the day counter; on the host build it ticks once per
   storyLoop iteration with day-progression rules.

2. Persist `storyCurrentDay` to memcard alongside the existing
   pause-menu settings. The original Sierra game persists day in the
   Windows registry; the PS1 equivalent is the memcard save block,
   already wired via `src/memcard.c`. Add a `currentDay` field.

3. Add scene-day filtering to the picker:
   ```c
   if (scene->dayNo != 0 && scene->dayNo != storyCurrentDay) skip;
   ```
   (already mirrored in `src/story.c:300`.)

4. Day advancement triggers — read from `src/story.c`:
    - Day rolls forward when certain `FINAL` scenes finish (raft progress,
      Mary's visit, etc.). Specific rules in `storyApplySceneDay()`
      (`src/story.c:118`).

5. Pause-menu integration: existing "Set Time" editor adds a
   "Set Day" cycler (1..11 → wrap). Useful for debugging story-day
   state.

6. Variant interactions:
    - Some scenes (`MARY.ADS:5`) flag `NORAFT` — pre-raft only. Need
      `islandState.raft` from the runtime which is already tracked.
    - Some scenes flag `HOLIDAY_NOK` — don't run during a holiday.
      Already known via `islandState.holiday`.

7. Add `kProvenScenes` candidates that have nonzero `dayNo` once the
   day system is live. Until then, day filtering is a no-op (all
   currently-validated scenes are `dayNo=0`).

**Acceptance:**
- Day counter advances visibly across long-loop runs.
- Force `story-day 5` boot token; only Mary-visit-day-5 scenes can
  occur (within the validated set).
- Day persists across power cycle via memcard.

## 5. Files touched

**New:**
- `src/walk_pilot.c` — PS1-side walk render driver
- `src/walk_sound_events.h` — footstep table (Phase 4)
- `scripts/walk-coverage-from-proven.py` — regtest case generator
- `docs/ps1/walk-implementation-plan.md` — this file

**Modified:**
- `CMakeLists.txt` — SOURCES gains walk.c, calcpath.c (and walk_pilot.c)
- `src/walk.c` — SDL surface dependency at line 80 hoisted behind
  `#ifndef PS1_BUILD`
- `src/jc_reborn.c` — `fgLoopNextScene()` replaced with story-aware
  picker; new `walk-test` boot token; storyCurrentDay state and
  memcard integration
- `src/foreground_pilot.c` (or a new caller) — invokes
  `fgWalkRender()` between scene plays
- `src/memcard.c` — `currentDay` field
- `config/ps1/cd_layout.xml` — `JOHNWALK.BMP` route
- `src/pause_menu.c` — Set Day cycler under Set Time

**Reused:**
- `src/walk_data.h`, `src/walk.h` — drop in unchanged
- `src/calcpath.c`, `src/calcpath.h` — drop in unchanged
- `src/story_data.h` — included by PS1 picker (data only, no
  function-pointer dependency)
- `src/island.c` / `islandState` — already PS1-tracked

## 6. Memory and code budget

- `walk_data.h`: ~530 lines × 4 fields × 2 bytes ≈ **4 KB const**
- `JOHNWALK.BMP`: estimated **64-128 KB** asset on disc, **~32-64 KB
  in VRAM** as a sprite atlas
- `walk.c` + `walk_pilot.c` + picker: estimated **3-5 KB** compiled
  MIPS code
- `storyScenes[]` from `story_data.h`: 63 entries × ~32 bytes ≈ **2 KB
  const**
- `currentDay` memcard delta: 1 byte
- Total walk-system memory delta: **~10-20 KB code + ~150 KB asset**.
  Comfortable inside the 2 MB main RAM budget.

## 7. Acceptance for the whole feature

- Screensaver loop runs through 6-19 scenes in sequence with visible
  walks between them, matching the original's pacing.
- Spot/heading tracking is consistent across pause-menu navigation,
  scene reset, and screensaver-loop iterations.
- No FG2-pack perf regression — existing `fishing1`/`fishing2` perf
  numbers stay within noise.
- Walks have correct footstep audio.
- Behind-tree compositing renders correctly on the spot 3↔4 path.
- Holiday overlay persists across scene→walk→scene transitions.
- 11-day calendar drives MARY/SUZY/JOHNNY narrative scenes when their
  respective days are active.
- Pixel-perfect walk frames against host build at the FISHING-1 bar
  for every used edge.

## 8. Out of scope

- Re-introducing the ADS / TTM bytecode interpreter on PS1
- Re-introducing the resource interpreter (`RESOURCE.MAP` / `.001`) on
  the runtime PS1 disc
- Rewriting the per-pack FG2 perf path to share code with the walk
  render path
- Adding new walk routes Sierra didn't author
- Per-scene `LEFT_ISLAND` cinematics that Sierra didn't author for the
  ones we now want to skip
- A graphical map of the island with hover-state info (purely a
  website thing if anyone wants it later)

---

# Red-team review

This section is the post-write self-review. Each subsection looks for
specific failure modes the plan above could hit.

## R1. Correctness

**R1.1. Coordinate frame match.** `walk_data.h` coordinates are absolute
640×480 pixels from Sierra's original game. The PS1 also runs at 640×480
interlaced. Coordinates should map directly. **Risk: confirmed low** as
long as the runtime applies the existing `islandState.xPos/yPos` offset
the same way the host build does. **Mitigation:** Phase 2 acceptance
includes a frame-by-frame compare against host capture; mismatches will
surface there.

**R1.2. Walk-end-pose vs scene-frame-0 continuity.** When `walkAnimate()`
returns `delay=0`, Johnny is in his "hands in pockets" final pose at the
target spot/heading. The next FG2 pack's frame 0 must start with Johnny
in that exact pose, otherwise we get a one-frame visual pop. Sierra's
host capture went through the same flow, so the pack's frame 0 *should*
match — but the pack format records absolute capture frames, so any
slight phase difference in PS1's walk render vs host walk render will
produce a visible pop. **Risk: medium.** **Mitigation:** Phase 7
acceptance includes an explicit walk-end → scene-frame-0 continuity
check. If a pop appears, the walk's final pose may need to be replaced
with a one-VBlank "settle" frame matching scene-frame-0, or the FG2
pack regenerated with a known walk-end starting pose.

**R1.3. LEFT_ISLAND scenes leave Johnny invisible.** FISHING 4, FISHING
7, FISHING 8, MARY 5, VISITOR 5 set `LEFT_ISLAND` and have no defined
`spotEnd/hdgEnd`. After they finish, Johnny is "off the island" until a
FIRST scene resets the position. The original engine sets
`prevSpot=-1` in this case (`src/story.c:649`). The PS1 picker must
encode the same: when prev=-1, the next picked scene MUST be either
FIRST (sets fresh start) or another no-walk-needed scene. **Risk:
medium.** **Mitigation:** Phase 3's picker explicitly preserves the
prev=-1 sentinel and the "after LEFT_ISLAND, only FIRST" gate. Without
this, the picker either crashes (calcPath called with -1) or walks
from invalid coordinates.

**R1.4. Scenes with `spotStart=0` should not be walked TO.** Several
scenes have no defined start position (anywhere works). The original
engine's `storyHasValidStart()` check at `src/story.c:585` skips the
walk if the next scene's start is undefined. The PS1 picker must
preserve that gate. **Risk: low** (straightforward port) but easy to
miss.

**R1.5. The 8-pick retry limit.** The original picker tries up to 8
candidates that satisfy filters; if none match the spot/start gate, it
falls back. This is a soft constraint, not a hard one — the original
intentionally permits "imperfect" picks rather than infinite-loop.
**Risk: low.** **Mitigation:** copy the literal loop structure from
`src/story.c:583-590`. Don't tighten it.

**R1.6. Random number generator state.** The PS1 picker uses `rand()`.
The host capture used the same `rand()` per scene; if the PS1 picker
seeds and steps the RNG identically, the scene sequence is reproducible.
**Risk: low.** **Mitigation:** existing seed plumbing (`fgpilot ... seed
N`) should cover this; ensure the walk picker does NOT consume more
RNG calls than the original story loop or the seed reproducibility
breaks.

## R2. Risk

**R2.1. Behind-tree compositing might require a new render path.**
Phase 5 assumes that drawing a "walk sprite then trunk + leaf sprite"
in submission order yields the correct z-order on PS1. If sprites in
the OT use shared z, last-submitted-wins, that works. If z is per-prim
fixed, we may need explicit OT depth assignment. **Risk: medium.**
**Mitigation:** Phase 5 starts with an experiment, not a write. If the
naive approach doesn't produce correct overlap, escalate to either
explicit OT z assignment or a separate "post-compose cover-up" render
pass that runs after the walk sprite.

**R2.2. JOHNWALK.BMP color-depth surprise.** The plan assumes the
walking atlas is straight 8-bit indexed and either fits PAL4
per-tile-CLUT analysis or works as 8-bit straight. If it has unusual
masking (e.g. transparency-via-color-key 0), the existing PS1 sprite
loader may need to handle it. **Risk: low-medium.** **Mitigation:**
Phase 1 inspects the BMP first and decides which loader path to use.
Don't write Phase 2 code until this is settled.

**R2.3. Footstep sample identification.** The plan assumes we can
identify the right SPU samples for footsteps from the original engine.
If they aren't separately preloaded — or if they were inline-streamed
from the resource file — Phase 4 may need to capture and add new VAGs
to the boot preload. That changes SPU RAM budgeting. **Risk: low**
(footsteps are small samples) but worth nailing down before promising
sound.

**R2.4. Scene-graph completeness.** `walk_data.h` was extracted from
Sierra's `SCRANTIC.SCR`. If the extractor missed any spot pairs,
`calcPath()` may return an unreachable-error for a transition, and
the runtime crashes or renders junk. **Risk: low.** **Mitigation:**
write a one-time validator that walks every (fromSpot, toSpot) pair in
the 6-node graph and confirms `walkDataBookmarks[from][to]` is non-zero.

**R2.5. Walk takes ~10-30 VBlanks per spot edge.** Worst-case path
through the 6-node graph is ~5 edges, so up to ~150 VBlanks (~2.5s
at 60Hz) for a single transition. That's about double the average
scene length. If the screensaver-loop pacing feels sluggish, the
relative weight of walks-vs-scenes could be off. **Risk: low**
(the original game has the same pacing, presumably acceptable to
Sierra). **Mitigation:** if it feels wrong post-Phase 3, accept that
walks are part of the screensaver's character and don't speed them
up.

**R2.6. Memcard schema bump for `currentDay`.** Phase 8 adds a field
to the memcard save. If the existing save block has no version field,
new save blocks won't load on old binaries (or vice versa). **Risk:
low** (single-author project, memcard mismatches affect only the
author). **Mitigation:** add a save-format version field in Phase 8 if
not already present.

**R2.7. Pause-menu interaction during walks.** The user can pause mid-
walk via START. The pause menu currently tracks pause state per-scene;
adding walks creates a new "in-walk" state that may confuse Reset
Current Scene, Next Scene, etc. **Risk: medium.** **Mitigation:**
explicit Phase 3 acceptance includes pause-during-walk → resume-
during-walk → finish-walk path. If broken, walks can be made
non-interruptible (pause is queued until walk completes).

## R3. Algorithmic soundness

**R3.1. `calcPath()` correctness.** The pathfinder operates on a 6-node
undirected graph. `calcPath` is small and runs once per scene
transition. **Complexity: O(V) ≤ 6 with the existing implementation.**
**Risk: very low.** Read the existing calcpath.c once to confirm it
returns a deterministic shortest path.

**R3.2. Picker termination.** The 8-pick retry guarantees the loop
terminates regardless of filter strictness. **Complexity: O(N×8)** in
the worst case where N is the validated scene count (currently 2,
asymptotically 63). **Risk: very low.**

**R3.3. RNG consumption count.** If the picker calls `rand()` in a
different pattern than the original story loop, the `seed` boot token
loses reproducibility. **Risk: medium for testing only.**
**Mitigation:** the PS1 picker copies `src/story.c`'s exact RNG-call
sequence. Don't add or remove any `rand()` calls without versioning the
seed model.

**R3.4. Story-day advancement determinism.** Phase 8's day rules must
mirror the original engine exactly, otherwise the perceived narrative
arc differs. The original advances on specific FINAL scenes ending.
**Risk: medium.** **Mitigation:** Phase 8 starts by porting
`storyApplySceneDay()` *verbatim*, not paraphrasing it.

**R3.5. State-machine reset safety.** `walk.c` uses static globals
(`walkPath`, `currentSpot`, etc.). If the screensaver loop is
interrupted (pause, hard reset, scene abort), those globals may carry
stale state into the next walk. **Risk: medium.** **Mitigation:**
`walkInit()` is called on every walk and resets everything from
parameters. Verify by reading walk.c — no walkAnimate-leak path
should be possible. (Confirmed: line 50-74 reset everything.)

## R4. Phasing logic

**R4.1. Phase ordering — Phase 5 (behind-tree) before Phase 7
(validation).** Behind-tree is a visual correctness issue that the
validation harness will catch. If we run Phase 7 before Phase 5,
spot 3↔4 walk validation fails. **Risk: low** as long as we accept
that those edges fail validation until Phase 5 lands.

**R4.2. Phase 8 (story calendar) is last but the user said "include
it." That order is correct because the calendar is independent of
walks: story-day filters scenes but doesn't change how walks are
rendered. Phase 8 can land any time after Phase 3.

**R4.3. Should Phase 4 (footsteps) block Phase 7 (validation)?** The
host build presumably has footsteps. Phase 7 pixel-diff doesn't catch
audio. So footsteps can be added in parallel with validation, or after,
without breaking the visual gate. Order is flexible.

## R5. What this plan doesn't cover

- The first time we hit a NEW spot edge (one not currently exercised
  by validated scenes), we'll discover whether `walkDataBookmarks`
  has data for it. As more scenes get validated, more edges activate;
  some may surface gaps in `walk_data.h` that R2.4's validator should
  catch up front. The plan should explicitly run R2.4's validator in
  Phase 1, not later.
- Performance budget for walks. Walks are short and infrequent
  compared to scene playback; the plan assumes they fit. If the perf
  branch's perf gates extend to include walks, we may discover the
  walk render path needs the same kind of dirty-region/restore
  optimization that scenes got. Out of scope for now; named here so
  the perf agent knows it's a downstream concern.

## R6. Pre-flight checklist before Phase 1 starts

- [ ] Run R2.4 validator: every (fromSpot, toSpot) pair in the 6-node
      graph has non-zero `walkDataBookmarks[from][to]` in `walk_data.h`
- [ ] Open `JOHNWALK.BMP`, confirm dimensions, color depth, and that
      it transcodes cleanly via `scripts/transcode-bmp-ps1.py`
- [ ] Confirm `extract_walk_data.c` source still exists in the repo
      (referenced by `walk_data.h:3`) — if not, document where the
      extraction came from
- [ ] Identify the original engine's footstep sample IDs by replaying
      a host walk under a debugger or audio-capture wrapper

These four checks de-risk the four highest-uncertainty items above.
Don't merge Phase 1 without them.
