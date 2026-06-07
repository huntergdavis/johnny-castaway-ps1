# Walk-Connected Story Loop — Implementation Plan

> 🌐 **Rendered version:** **[/source/docs/ps1/walk-implementation-plan/](https://hunterdavis.com/johnny-castaway-ps1/source/docs/ps1/walk-implementation-plan/)** — this doc rendered on the project website's source library. The GitHub copy here is the source.

Date: 2026-04-29
Status: implemented in `v0.4.20-ps1`; freeplay-specific phases remain future work
Owner: PS1 perf branch

> **2026-05-02 update — footsteps removed.** Phase 4 of this plan
> (footstep audio: pause-menu toggle, `footstepsEnabled` global,
> `fireFootstep` kernel parameter, R2.3 sample audit, R7.4 contradiction
> resolution) was reversed. The feature shipped in `v0.4.20-ps1` and was
> removed wholesale before any sample-ID audit ran. The walk render
> kernel no longer takes a footstep parameter, the pause-menu row is
> gone, and the memcard byte is preserved at its old position but is no
> longer read or written. The rest of this plan is preserved as the
> historical record of the original walk port; ignore the Phase 4 / R2.3
> / R7.4 sections below.

> **2026-05-04 update — picker promoted, `kProvenScenes` retired.**
> The scene-picking logic moved out of `fgLoopNextScene` into a
> dedicated `src/scene/scene_picker.c` (Random / Sequential / Original
> policies, see `docs/ps1/scene-picker-design.md`). The
> hand-curated `kProvenScenes[]` array referenced throughout this
> plan has been replaced by `kAllScenes[]`, which contains every
> scene with an FG2 pack on disc (63 today). Story Day filtering,
> walk-aware retry, and FINAL/FIRST gating now live in the picker
> module, not in `fgLoopNextScene`. The `kProvenScenes` snippets
> later in this doc describe the pre-picker baseline that motivated
> the walk port — read `docs/ps1/scene-picker-design.md` and
> `src/scene/scene_picker.c` for current behaviour.

## Executive Summary

The PS1 build used to pick each scene at random and teleport Johnny
between them. The original Sierra screensaver instead chains scenes together
visibly: Johnny ends one scene at a known spot and heading, then walks
across the island to the next scene's start. The walks are part of what
makes the screensaver feel like a place rather than a clip reel.

This plan ports the walking system to the PS1 build without touching the
FG2-pack replay path the perf branch depends on. The scope is the cleanest
hybrid available:

- Bring the existing `src/walk/walk.c` + `src/walk/walk_data.h` + `src/walk/calcpath.c`
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

## Release 0.4.20 Implementation Notes

The `v0.4.20-ps1` implementation keeps the hybrid FG2 scene replay path
intact and adds a sprite-driven connector between scenes:

- `walk_pilot.c` owns story-loop walks, loads `JOHNWALK.PSB` lazily,
  drives `walk.c` / `walk_data.h`, and suppresses prior-scene FG2
  recomposition while Johnny is walking.
- `walk_render.c` draws the current walk pose and re-stamps the palm
  trunk/leaves when Johnny is behind the tree, preserving the original
  occlusion rule.
- `foreground_pilot.c` captures a clean island baseline before scene
  playback dirties it; walk frames restore from that baseline, tick the
  waves, re-stamp active holiday emblems, and then draw Johnny.
- The walk erase buffer is now a tight, persistent 340x224 region
  (about 149 KB) instead of a larger free/realloc buffer. Repeated
  allocation churn was the source of later-session paint trails.
- Setup-prime foreground streaming windows are capped to a deterministic
  resident budget. Large prime windows are treated as caches, not as
  mandatory scene-start allocations, so a scene cannot BSOD just because
  a cache-sized contiguous block is unavailable.

Validation evidence for the release candidate:

- Visual signoff: Johnny walks correctly between scenes, including
  crossing in front of the palm tree without painting every pose into
  the background.
- Runtime soak: DuckStation reached roughly 599 seconds of organic
  screensaver-loop playback, repeatedly crossing the prior `fishing1`
  crash point.
- TTY log: no `JCBSOD` lines and no `JCWALK: walkClean buffer alloc
  failed` lines in `scratch/walk-soak/duckstation-final-0.4.20-candidate.log`.

## 1. Background — how the original engine does it

### 1.1 Scene metadata is pre-declared

`src/host/story_data.h` declares all 63 scenes with start/end position metadata:

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

`src/walk/walk.c` is 189 lines. `src/walk/walk_data.h` is 530 lines of static const
data extracted from Sierra's original `SCRANTIC.SCR` — per-frame
`(sprite_index, x, y, frame_id)` for every walk transition plus turning
poses. `src/walk/calcpath.c` does shortest-path through the 6-spot graph.

`adsPlayWalk(fromSpot, fromHdg, toSpot, toHdg)` in `src/ads/ads.c:2216`
orchestrates: load `JOHNWALK.BMP` into a TTM slot, call `walkInit()`, loop
`walkAnimate()` per frame until it returns `delay=0`. The PS1-specific
ifdefs in `walk.c` already partially anticipate the port (sufficient
for graphics_ps1 integration; the SDL surface dependency at line 80 is
the lone holdout).

### 1.3 The story loop chains scenes together

`storyLoop()` in `src/host/story.c:560-690`:

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

## 3.5 Cross-cutting architecture (shared with freeplay)

This walk subsystem is designed up front to be **shared with the
freeplay direct-control mode** described in
[`docs/ps1/freeplay-mode-design.md`](freeplay-mode-design.md). Both
modes need to render Johnny walking; they differ only in what drives
the steps. Avoiding two parallel implementations is non-negotiable —
the project's voice is plainspoken about not building things twice.

### 3.5.1 Two drivers, one render kernel

```
                    +----------------------------+
                    |       walk_render.c        |
                    |   (the shared kernel)      |
                    |                            |
                    |   walkRenderFrame(sprite,  |
                    |     x, y, heading,         |
                    |     behindTree, footstep)  |
                    +----------------------------+
                          ^                ^
                          |                |
            +-------------+                +--------------+
            |                                              |
   +------------------+                       +-------------------+
   |  walk_pilot.c    |                       |  scene_freeplay.c |
   |  story-loop      |                       |  freeplay         |
   |  walks           |                       |  direct-control   |
   |                  |                       |                   |
   | drives the kernel|                       | drives the kernel |
   | from walk_data.h |                       | from D-pad input  |
   | pre-baked path   |                       | per VBlank        |
   +------------------+                       +-------------------+
```

The shared kernel does ONE thing: takes a current `(sprite_frame_idx,
x, y, heading)` plus a couple of flags and emits one frame against
restored background tiles. It does not own walk state; it does not
track spots; it does not advance frames. Both drivers do that
themselves and then call the kernel.

### 3.5.2 Shared position type

```c
// src/walk/walk_render.h
struct TWalkPos {
    int x;          // pixel coord, island-relative
    int y;          // pixel coord, island-relative
    int heading;    // HDG_S..SE
    int spot;       // SPOT_A..F, or -1 if mid-edge / freely positioned
};
```

Both drivers track Johnny as `TWalkPos`. The story-loop driver
stamps `spot` at scene boundaries and lets it go to -1 mid-walk;
the freeplay driver leaves `spot` at -1 except when a contextual
verb needs to know which named spot Johnny is currently nearest.

### 3.5.3 Spot ↔ coord mapping (extracted from walk_data.h)

`walk_data.h`'s first/last frames per (fromSpot, toSpot) edge declare
the canonical pixel coordinates of each spot. From the head of the
file:

| Spot | Approx (x, y) | Source |
|---|---|---|
| A | (306, 238)   | first frame of "A to E" entry |
| C | (~395, 240)  | last frame of "A to C" |
| E | (390, 213)   | last frame of "A to E" |
| F | (~435, 224)  | last frame of "A to F" |
| B, D | TBD       | extract from B-rooted and D-rooted entries |

Phase 1's pre-flight checklist adds: extract canonical coordinates
for all 6 spots from `walk_data.h`. Both drivers use this mapping;
freeplay's "press Cross at fishing shore" verb uses it to ask "what
spot is Johnny nearest right now?" without inventing geometry.

### 3.5.4 Per-mode ownership

| Resource | Owner | Reason |
|---|---|---|
| `walkData[]` table | shared (read-only) | static const data |
| `walkPath`, `currentSpot`, `currentHdg` (statics in walk.c) | story-loop driver only | freeplay doesn't use calcPath |
| `JOHNWALK.BMP` slot 1 | shared (kernel reserves) | matches freeplay design slot allocation |
| Clean-rect set | per-mode | freeplay owns `gFreeplayOwnedRects[]`; story-loop uses the island's set |
| Holiday overlay | runtime, not kernel | both modes call the existing `fgBackdropStampHoliday` after the kernel returns |
| Footstep SFX | per-mode opt-in | conditional; see Phase 4 + R2.3 |

The kernel does NOT touch `walk.c`'s static globals
(`walkPath`, `currentSpot`, etc.). Only `walk_pilot.c` calls
`walkInit()` / `walkAnimate()` and reads those globals.
`scene_freeplay.c` never calls into `walk.c`; it computes
`(x, y, heading)` from input and calls `walkRenderFrame()` directly.
This isolation matters for R3.5 (state-machine reset safety): the
two modes cannot corrupt each other's walk state.

### 3.5.5 Behind-tree compositing is a kernel parameter

`walkRenderFrame(..., bool behindTree, ...)`. Both modes set it the
same way — based on a coordinate-zone test against the palm tree
sprite's bbox. Story-loop driver derives it from the
`(currentSpot, nextSpot)` pair (preserves walk.c's existing
`SPOT_3 ↔ SPOT_4` rule). Freeplay derives it from the live (x, y)
intersection with the tree z-region (per
`docs/ps1/freeplay-mode-design.md` § 14, lines 337-339).

The kernel doesn't know or care which mode called it — it just
stamps the trunk + leaf cover-up sprites after the walking sprite
when the flag is set.

### 3.5.6 Holiday overlay is NOT in the kernel

Both modes already have a per-frame compositor pipeline (the
freeplay design's locked render order at § 15, lines 357-371). The
kernel runs at step 6 of that pipeline ("Johnny — last among
foreground"). Holiday overlay (step 7) and help overlay (step 8)
run AFTER the kernel returns, in the mode's main loop. The walk
plan inherits that ordering. Phase 6 of this plan no longer adds
special holiday handling; it just confirms the existing pipeline
runs unchanged across walk frames.

### 3.5.7 Footstep audio rule

User-decided 2026-04-29: **if the original engine has footstep
samples in its walking code, play them. Add a pause-menu toggle so
users can turn them off.** The freeplay design's claim "original was
silent" is updated to match.

Implementation:
- Phase 4 audits the original engine's walk audio path. If samples
  fire, identify them and wire into `walkSoundEvents[]`. If the
  audit finds none, walks ship silent.
- The kernel's `fireFootstep` parameter remains. Both drivers
  always pass the trigger value the same way.
- Inside the kernel, `fireFootstep` is gated by a global runtime
  flag `footstepsEnabled` (default ON, settable via pause menu).
  When OFF, the trigger is a no-op; sample selection is unaffected.
- Pause-menu Options sub-screen gains a "Footsteps" toggle alongside
  Sound, DayNight, Tide, Raft, Captions, Perf. Persisted to memcard
  like other toggles.

## 4. Implementation phases

Each phase ends in a clean commit. Phases land in order; later phases
can defer if perf or schedule pressure says so.

### Phase 1 — Assets + build integration (~1 day)

**Goal:** walk module compiles into `jcreborn.bin`.

1. Add to `CMakeLists.txt` SOURCES:
    - `src/walk/walk.c`
    - `src/walk/calcpath.c`
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

### Phase 2 — Walk render kernel (~1-2 days)

**Goal:** the shared kernel from § 3.5.1 exists and renders correctly
when given a sprite frame + position + flags. No driver yet.

1. New file `src/walk/walk_render.c` + `src/walk/walk_render.h`. Public API:
   ```c
   void walkRenderInit(void);             // load JOHNWALK.BMP into slot 1
   void walkRenderRelease(void);          // free slot 1
   void walkRenderFrame(int spriteIdx,
                        int x, int y,
                        int heading,
                        int behindTree,
                        int fireFootstep);
   ```
   The kernel:
    - Restores background tiles via existing `grRestoreBgTiles()`
    - Draws the walking sprite at (x, y)
    - If `behindTree`, stamps the trunk + leaf cover-up sprites after
      the walking sprite
    - If `fireFootstep`, calls `soundPlay(footstepSampleId)` (sample
      id resolved per Phase 4; gated by config)
    - Returns. Does NOT call grBeginFrame/grEndFrame — the caller
      owns the frame envelope and will call holiday + help overlays
      AFTER the kernel returns (matches freeplay's locked render
      order, § 15 of `freeplay-mode-design.md`).

2. Add a boot-token test mode that calls the kernel directly:
   ```
   walk-render-test
   ```
   Renders Johnny standing at a fixed coord, facing each of the 8
   headings in turn, 60 VBlanks each. Lets us verify sprite framing,
   palette, and slot allocation independent of any walk path or
   driver state.

3. Coordinate offset handling: walk_data.h coords are absolute
   640×480; the runtime offsets by `islandState.xPos`/`yPos`. The
   kernel takes pre-offset (x, y) — drivers do the offset before
   calling.

**Acceptance:**
- Boot `walk-render-test` and see Johnny standing at fixed pixel,
  cycling through 8 facings, 60 VBlanks each.
- No VRAM corruption, slot 1 holds `JOHNWALK.BMP` cleanly, palette
  is correct.
- The kernel works without ever touching `walk.c`'s static globals.

### Phase 2.5 — Story-loop walk driver (~1-2 days)

**Goal:** call a walk SEQUENCE from a test boot token and see Johnny
walk visibly across the island, driven by `walk.c`'s state machine.

1. New file `src/walk_pilot.c`. Public API:
   ```c
   int fgWalkRender(int fromSpot, int fromHdg, int toSpot, int toHdg);
   ```
   Returns 0 on success, non-zero on harness-abort signal.

2. Implementation outline — drives the kernel from `walk.c`'s pre-baked
   path data:
   ```c
   walkInit(fromSpot, fromHdg, toSpot, toHdg);
   walkRenderInit();
   int delay = walkAnimate(/*params*/);   // returns sprite frame info
   while (delay > 0) {
       eventsWaitTick(delay);
       grBeginFrame();
       /* derive (sprite_idx, x, y, heading, behindTree) from walk.c
        * statics — currentSpot, walkPath, isBehindTree, etc. */
       walkRenderFrame(sprite_idx, x, y, heading, isBehindTree,
                       /*fireFootstep=*/0);
       /* holiday overlay, help overlay run here in main loop */
       grEndFrame();
       delay = walkAnimate(/*params*/);
   }
   walkRenderRelease();
   ```

3. The driver is the ONLY caller of `walk.c`. Its statics
   (`walkPath`, `currentSpot`, `currentHdg`, `nextSpot`, `nextHdg`,
   etc.) live entirely inside walk.c and are scoped per-`fgWalkRender`
   call.

4. Add a boot-token test mode:
   ```
   walk-test fromSpot fromHdg toSpot toHdg
   ```
   e.g. `walk-test E E A S` runs a single walk for visual review.

**Acceptance:**
- Boot `walk-test E E A S` and see Johnny walk from spot E heading east
  to spot A heading south, driven by walk.c.
- Compare frame-by-frame against the host build's same walk. Pixel-exact
  match would be ideal; visually-identical-at-2× is acceptable for
  promotion.
- No VRAM corruption, no crash, no audible artifacts.
- `walk.c`'s static state is fully reset on every `fgWalkRender` call.

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
      `unwantedFlags` logic at `src/host/story.c:567-575`)
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
   (`src/host/story.c:583`) is preserved; if 8 picks fail to satisfy filters,
   accept anything that matches `wantedFlags`. Original behavior.

**Acceptance:**
- Boot `fgpilot` with no explicit scene. The screensaver runs through
  multiple scenes WITH walks visibly connecting them.
- Walk from FISHING 1's end (D-east) to FISHING 2's start (D-west) —
  this is a turn-in-place at spot D. Walk to a new spot and back works
  cleanly.
- No teleportation. Position state remains consistent across iterations.

### Phase 4 — Footstep sounds (~1-1.5 days)

**Goal:** if the original engine had footstep samples, play them;
add a pause-menu toggle so users can turn them off.

User decision (2026-04-29): footsteps are ON by default if Sierra's
code has them. No "should we?" question — Phase 4 just identifies
the samples, wires them, and adds the toggle.

**Phase 4.1 — audit (½ day)**

- Capture audio from the host build during a walk on every spot
  edge. Identify the sample IDs that fire. Document in
  `docs/ps1/walk-spot-coordinates.md` (the same artifact R6 builds)
  alongside the canonical coords.
- Cross-check against Sierra's `RESOURCE.001` walking-sequence
  handling and `extract_walk_data.c` if it preserved audio cues.
- If audit finds zero samples: ship silent walks; the kernel's
  `fireFootstep` parameter and the pause-menu toggle still go in
  (the toggle is harmless when there's nothing to gate).

**Phase 4.2 — implementation (½ day)**

- New table `src/walk_sound_events.h`:
  ```c
  struct TWalkSoundEvent { int frameIdx; int sampleId; };
  static const struct TWalkSoundEvent walkStepSamples[NUM_OF_NODES][NUM_OF_NODES][/*...*/];
  ```
  keyed by current spot-pair edge.
- The kernel's `fireFootstep` parameter is the trigger point. Both
  drivers compute it the same way (frame index hits a step-foot-down
  position) and pass it in. The kernel calls
  `if (footstepsEnabled && fireFootstep) soundPlay(sampleId);`.
- `walk_pilot.c` derives `fireFootstep` from `walkAnimate()`'s frame
  cadence + the `walkStepSamples` table.
- `scene_freeplay.c` derives `fireFootstep` from its own walk cycle
  counter (`TFreeplayJohnny.walkStepCount` already in the freeplay
  design § 17, line 414) — fires on alternating step indices.
- Verify the relevant footstep VAGs are in the preload set. The PS1
  build preloads "all 23 SFX VAGs at boot" per `hardware-specs.md`;
  if footsteps are among them, no asset work. If not, add to the
  boot preload list — they're small samples, SPU budget is not
  tight.

**Phase 4.3 — pause-menu toggle (½ day)**

- Add `OPT_FOOTSTEPS` to the Options enum in `src/pause_menu/pause_menu.c`,
  alongside `OPT_SOUND`, `OPT_DAYNIGHT`, `OPT_TIDE`, `OPT_RAFT`,
  `OPT_HOLIDAY`, `OPT_CAPTIONS`, `OPT_PERF`.
- New global `int footstepsEnabled = 1;` in the same scope as
  `soundMuted` etc., persisted to memcard via the existing settings
  block in `src/platform/ps1/memcard.c`.
- Default ON. Memcard restoration on boot picks up the user's last
  setting.

**Acceptance:**
- If samples fire in the original: walks have audible footsteps
  matching leg cadence in both story-loop walks and freeplay walks.
  No SPU pop, click, or stuck note.
- Pause-menu Footsteps toggle works; flips state mid-walk takes
  effect on the next step trigger.
- Setting persists across power cycle via memcard.
- The freeplay design's "original was silent" line at § 12 line 282
  is updated to "footsteps when enabled (pause-menu toggle, default
  ON)."

### Phase 5 — Behind-tree compositing (~1-2 days, kernel-resident)

**Goal:** when Johnny walks behind the palm tree, the tree visibly
covers him. Implementation lives **in the shared kernel** so both
story-loop walks AND freeplay get it for free.

1. The original behind-tree code is `walk.c:103-104`:
   ```c
   isBehindTree = ((currentSpot == 3) && (nextSpot == 4))
                 || ((currentSpot == 4) && (nextSpot == 3));
   ```
   When set, `walkAnimate()` draws extra cover-up sprites
   (`walk.c:174`):
   ```c
   grDrawSprite(sfc, ttmBgSlot, 442, 148, 13, 0);  // trunk
   grDrawSprite(sfc, ttmBgSlot, 365, 122, 12, 0);  // leafs
   ```

2. The shared kernel's `behindTree` parameter triggers this stamp.
   The kernel, not the driver, owns the cover-up sprite indices and
   their positions. That keeps both modes honest:
    - Story-loop driver: passes `behindTree` from walk.c's
      static (`isBehindTree`) which it already maintains.
    - Freeplay driver: passes `behindTree` from a coordinate-zone
      test against the tree z-region (per
      `freeplay-mode-design.md` § 14, lines 337-339:
      "tree z-region (430..480, 220..250)").

3. PS1 z-order: confirm by experiment that submitting the walk
   sprite to the OT *before* the trunk + leaf cover-up produces the
   desired overlap. The PS1 GPU draws OT entries in submission
   order; later submissions land on top of earlier ones. If that's
   how the existing renderer is wired (it is, per the experiment
   log's repeated references to OT-submission ordering), the naive
   approach works.

4. The trunk and leaf sub-sprites must be reachable at walk-time.
   Two options:
    - Use the existing background TTM slot at the same indices the
      host build uses (sprite indices 12 and 13). Confirm those
      indices resolve to trunk + leaf in the PS1 background atlas.
    - Pre-extract trunk + leaf into a small dedicated asset
      (`JOHNTREE.BMP`?) loaded alongside `JOHNWALK.BMP`. Adds disc
      footprint but isolates the dependency.

   Pick option 1 unless the indices don't line up.

5. Validate visually for both modes:
    - Story-loop: walk-test from spot 3 to spot 4 and reverse.
    - Freeplay: D-pad-walk through the tree z-region.
   Both should show Johnny covered at the correct moments.

**Acceptance:**
- Both story-loop walks and freeplay walks render the behind-tree
  effect identically.
- No z-fighting, flickering, or one-frame visibility glitches.
- The cover-up sprite indices are kernel-internal — neither driver
  has to know about them.

### Phase 6 — Holiday overlay during walks (~½ day, mostly verification)

**Goal:** the active holiday emblem stays on screen during walks
(it belongs to the island, not to a specific scene), with the same
compositor call both modes use.

1. The freeplay design's locked render order
   (`freeplay-mode-design.md` § 15, lines 357-371) places the holiday
   overlay at step 7, AFTER Johnny is rendered. Story-loop walks
   inherit that order — walks don't draw the overlay; the main loop
   does, after the kernel returns.

2. The story-loop driver's frame loop has the same shape as the
   freeplay frame loop:
   ```
   grBeginFrame()
   grRestoreBgTiles()
   /* wave / persistents — story-loop walks: just waves */
   walkRenderFrame(...)        /* the kernel */
   if (holiday active) fgBackdropStampHoliday(...)
   grEndFrame()
   ```
   The `fgBackdropStampHoliday()` call is the SAME function freeplay
   uses. No refactor needed; just wire the call into the walk
   driver's frame loop in the right order.

3. Verify with `holiday christmas` boot token: walk transitions show
   the holiday emblem at the same coordinates as the scenes that
   surround them. Cross-verify with `walk-render-test` + holiday
   token: the emblem should be present even on the bare-test path.

**Acceptance:**
- Holiday emblem persists across scene→walk→scene transitions with
  no flicker, no position jump, no emblem disappearance during the
  walk frames.
- Works with all 36 holidays from `holidays.yml`.
- The same `fgBackdropStampHoliday()` call drives the overlay in
  story-loop walks, freeplay walks, and existing scenes.

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

6. **Freeplay-parity test.** Build a tiny harness scene that calls
   the shared kernel from a synthetic freeplay-style driver: feed a
   pre-recorded sequence of `(spriteIdx, x, y, heading, behindTree)`
   tuples and capture the rendered frames. Compare against
   story-loop walks rendered through the same kernel for an
   equivalent path. Any pixel difference is a kernel bug — both
   drivers should produce identical output for identical input.
   This guards against the kernel accidentally depending on
   walk.c's static state.

**Acceptance:**
- All currently-used walk edges have a regtest case.
- All cases pass pixel-perfect against host reference.
- Walk-to-scene-frame-0 transitions show no visible pop.
- Freeplay-parity test passes: synthetic-driver output =
  story-loop-driver output for matched inputs.

### Phase 8 — 11-day story calendar (~3-5 days, last phase)

**Goal:** the JOHNNY/MARY/SUZY narrative beats fire on the right days
(Mary visit on day 5, Suzy visit on day 3, raft progress, Johnny
intro/outro), matching the original.

1. Port `storyUpdateCurrentDay()` (and any helpers it calls) from
   `src/host/story.c` into a PS1-compatible form. This is the function that
   advances the day counter; on the host build it ticks once per
   storyLoop iteration with day-progression rules.

2. Persist `storyCurrentDay` to memcard alongside the existing
   pause-menu settings. The original Sierra game persists day in the
   Windows registry; the PS1 equivalent is the memcard save block,
   already wired via `src/platform/ps1/memcard.c`. Add a `currentDay` field.

3. Add scene-day filtering to the picker:
   ```c
   if (scene->dayNo != 0 && scene->dayNo != storyCurrentDay) skip;
   ```
   (already mirrored in `src/host/story.c:300`.)

4. Day advancement triggers — read from `src/host/story.c`:
    - Day rolls forward when certain `FINAL` scenes finish (raft progress,
      Mary's visit, etc.). Specific rules in `storyApplySceneDay()`
      (`src/host/story.c:118`).

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
- `src/walk/walk.c` — SDL surface dependency at line 80 hoisted behind
  `#ifndef PS1_BUILD`
- `src/jc_reborn.c` — `fgLoopNextScene()` replaced with story-aware
  picker; new `walk-test` boot token; storyCurrentDay state and
  memcard integration
- `src/foreground_pilot/foreground_pilot.c` (or a new caller) — invokes
  `fgWalkRender()` between scene plays
- `src/platform/ps1/memcard.c` — `currentDay` field
- `config/ps1/cd_layout.xml` — `JOHNWALK.BMP` route
- `src/pause_menu/pause_menu.c` — Set Day cycler under Set Time

**Reused:**
- `src/walk/walk_data.h`, `src/walk/walk.h` — drop in unchanged
- `src/walk/calcpath.c`, `src/walk/calcpath.h` — drop in unchanged
- `src/host/story_data.h` — included by PS1 picker (data only, no
  function-pointer dependency)
- `src/scene/island.c` / `islandState` — already PS1-tracked

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
`prevSpot=-1` in this case (`src/host/story.c:649`). The PS1 picker must
encode the same: when prev=-1, the next picked scene MUST be either
FIRST (sets fresh start) or another no-walk-needed scene. **Risk:
medium.** **Mitigation:** Phase 3's picker explicitly preserves the
prev=-1 sentinel and the "after LEFT_ISLAND, only FIRST" gate. Without
this, the picker either crashes (calcPath called with -1) or walks
from invalid coordinates.

**R1.4. Scenes with `spotStart=0` should not be walked TO.** Several
scenes have no defined start position (anywhere works). The original
engine's `storyHasValidStart()` check at `src/host/story.c:585` skips the
walk if the next scene's start is undefined. The PS1 picker must
preserve that gate. **Risk: low** (straightforward port) but easy to
miss.

**R1.5. The 8-pick retry limit.** The original picker tries up to 8
candidates that satisfy filters; if none match the spot/start gate, it
falls back. This is a soft constraint, not a hard one — the original
intentionally permits "imperfect" picks rather than infinite-loop.
**Risk: low.** **Mitigation:** copy the literal loop structure from
`src/host/story.c:583-590`. Don't tighten it.

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
**Mitigation:** the PS1 picker copies `src/host/story.c`'s exact RNG-call
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
- [ ] **Extract canonical (x, y) coordinates for all 6 spots from
      walk_data.h's first/last frames per edge** — needed by both
      drivers to bridge the spot↔coord systems. Output:
      `docs/ps1/walk-spot-coordinates.md`.

These five checks de-risk the highest-uncertainty items above.
Don't merge Phase 1 without them.

---

## R7. Shared-kernel architecture review (added 2026-04-29)

This section red-teams the cross-cutting architecture in § 3.5
specifically against the freeplay design.

**R7.1. Kernel API surface — what doesn't belong inside.**
The shared kernel must NOT call:
- `walk.c` statics (walkPath, currentSpot, etc.) — only walk_pilot.c
  reads those
- `calcPath()` — pathfinding is story-loop-only
- `fgBackdropStampHoliday()` — caller's job after kernel returns
- `eventsWaitTick()` — caller controls frame cadence
- `grBeginFrame()` / `grEndFrame()` — caller owns frame envelope
- `freeplayState.*` — kernel must not see freeplay's struct
The kernel's stateless API is what makes the freeplay-parity test
in Phase 7 step 6 meaningful. **Risk: medium** if reviewers add
"convenience" helpers that drag mode-specific state in. **Mitigation:**
the API in § 3.5.1 is the entire kernel surface; PRs that add
parameters need explicit review.

**R7.2. Spot↔coord mapping correctness.** § 3.5.3 extracts canonical
coords from `walk_data.h`'s first/last frames. If those frames
include lead-in turning poses, the coords will be slightly off. **Risk:
medium.** **Mitigation:** R6's "extract canonical coords" pre-flight
must validate against the host build's actual displayed Johnny
position when standing at each spot. If walk_data's first-frame coord
doesn't match standing position, look up the turn-in-place table
(`walkDataBookmarksTurns`) instead — those frames are explicitly
"standing at spot S facing heading H" poses.

**R7.3. Behind-tree zone test divergence.** The story-loop driver
derives `behindTree` from `(currentSpot, nextSpot)` per walk.c line
103. The freeplay driver derives it from a coord-zone test. These
two derivations could disagree at the same physical (x, y) — a
freeplay user walking on the same trajectory as a story-loop walk
might see a one-frame mismatch in tree-cover behavior. **Risk: low**
visually but real. **Mitigation:** Phase 5's acceptance test
explicitly compares both modes on the same trajectory; if they
disagree, the freeplay zone test wins (it's the more general one)
and the story-loop driver derives `behindTree` from the same
coord-zone test rather than the spot-pair rule.

**R7.4. Footstep contradiction — resolved 2026-04-29.** Original
risk: freeplay design said walks were silent; this plan was told
to add footsteps. Resolution: user ruled footsteps ON if the
original engine has them, with a pause-menu toggle for opt-out.
Phase 4 audits then implements. The pause-menu toggle keeps the
choice with each user instead of in the design doc. **Risk:
closed.**

**R7.5. JOHNWALK slot allocation.** Freeplay's design (§ 16) reserves
slot 1 for `JOHNWALK.BMP`. The walk kernel should use the same slot
identifier so the two modes don't fight over slots. **Risk: low.**
**Mitigation:** add a constant `WALK_BMP_SLOT = 1` to a shared
header (`walk_render.h`) — both modes include it.

**R7.6. Frame cadence model.** The story-loop driver waits whatever
delay `walkAnimate()` returns — usually 6 VBlanks per walk frame.
Freeplay's main loop is 1 VBlank per iteration. The kernel doesn't
care, but the per-mode drivers MUST drive the kernel at compatible
cadences. If freeplay calls `walkRenderFrame` once per VBlank with
an unchanged sprite frame index, Johnny appears to "stutter" — the
sprite refreshes but the pose doesn't change. **Risk: medium.**
**Mitigation:** freeplay driver advances its own sprite-frame index
on a 6-VBlank cadence (`TFreeplayJohnny.frameTimer` already in the
design at § 17 line 414); the kernel sees a moving sprite index just
like the story-loop driver.

**R7.7. Walk-end pose continuity for freeplay.** The story-loop
walk ends with Johnny in `walk.c`'s "hands in pockets" pose at the
target spot/heading (R1.2 covers this for scenes). Freeplay walks
end with Johnny in whatever sprite the last D-pad input dictated.
**Risk: low** for freeplay (no scene follows; D-pad release puts him
in IDLE pose), but the kernel's behavior when freeplay shifts from
WALK to a non-walk mode (FISH, BUILD, etc.) needs to be defined.
The kernel is stateless, so the mode-shift is the freeplay
driver's responsibility — Phase 7's freeplay-parity test should
exercise this transition.

**R7.8. Disagreement on whether freeplay even uses calcPath.** The
freeplay design specifies free 4-way movement, no spot graph. The
walk plan keeps calcPath alive for story-loop only. **Risk: very low**
— this is just a documented design choice, but worth noting that
a future "freeplay → walk to nearest spot for scene activation"
extension would require a coord→spot mapping (covered by R7.2's
canonical-coord table) and would NOT need calcPath either (it just
needs the closest-spot lookup).

**R7.9. Memcard schema for freeplay state during walks.** Phase 8
adds `currentDay` to memcard. Freeplay's design (§ 2 non-goals,
line 30) says state DOESN'T persist across sessions. So the memcard
schema only needs the story-loop's `currentDay`, not freeplay
state. Worth confirming the two save schemas don't cross-pollute.
**Risk: very low.** **Mitigation:** freeplay reads islandState
fresh on each entry; doesn't write back. memcard.c only persists
pause-menu settings + storyCurrentDay.

## R8. Phasing logic — does freeplay block on this plan?

Freeplay's Phase 1a (`freeplay: phase 1a — skeleton (walk + exit)`)
needs Johnny to walk on D-pad input. With the shared-kernel
architecture, freeplay's Phase 1a can land BEFORE walk plan Phase 3
(story-loop picker) but AFTER walk plan Phase 2 (kernel exists).

Recommended phase-interleaving if both projects run in parallel:

```
walk plan Phase 1   →  build integration + assets  (both depend)
walk plan Phase 2   →  kernel exists                (freeplay can start)
  freeplay Phase 0       freeplay Phase 0 (audit)   (independent)
  freeplay Phase 1a      freeplay Phase 1a (skeleton walk)
walk plan Phase 2.5 →  story-loop driver
walk plan Phase 3   →  scene picker
  freeplay Phase 1b → freeplay rest
walk plan Phase 4-8 (parallel with freeplay 2-6)
```

Both teams call the kernel; neither team owns it. Kernel changes
require sign-off from both owners. **Risk: low** assuming both teams
agree the kernel API in § 3.5.1 is the contract.
