# v0.4.20-ps1 Release Notes

> Release target: `v0.4.20-ps1`
> Date: 2026-04-30

This is the walking-loop release. Johnny no longer simply disappears
from one finished scene and appears at the start of the next one. The
PS1 screensaver loop now carries his spot and heading forward, runs the
original Sierra walk routes, and visibly walks him across the island
before the next FG2 scene begins.

## Highlights

- Story-loop walking is wired into the PS1 screensaver loop.
- Johnny uses the original `walk_data.h` route table and `JOHNWALK`
  sprite bank.
- The ocean keeps animating during walks.
- Active holiday overlays persist across scene -> walk -> scene
  transitions.
- Palm-tree occlusion works: the trunk and leaves are re-stamped over
  Johnny when the route passes behind the tree.
- The pause/menu/holiday/caption functionality from the previous
  milestone remains intact.

## Stability Fixes

The first walk build exposed two real PS1-runtime bugs:

- Walk poses could paint into the background when the clean erase buffer
  failed to reallocate later in the session.
- The run could BSOD at the end of a walk sequence when the next
  foreground scene tried to reserve a large setup-prime streaming
  window.

The release fix makes both budgets deterministic:

- The walk erase buffer is a persistent tight rect (`340x224`, about
  149 KB) that covers Johnny's route plus the palm cover-up sprites.
  It is captured fresh when the island state changes, but the allocation
  itself stays resident instead of fragmenting the heap with repeated
  free/malloc cycles.
- `JOHNWALK` is still released after each walk so scene playback can
  reclaim that sprite memory.
- Foreground setup-prime windows are capped to a bounded resident
  budget. They remain a performance cache, not a mandatory scene-start
  allocation.

## Soak Evidence

The release candidate was run in DuckStation through the organic
screensaver loop until roughly 599 seconds. It crossed the old
`fishing1` crash point repeatedly and continued into later walk/scene
transitions.

Observed in the saved TTY log:

- no `JCBSOD`
- no `JCWALK: walkClean buffer alloc failed`
- repeated `grSaveCleanBgRects=1` and `grSaveCleanBgRects=2` successes
  after heap settled around a 150 KB largest contiguous block

Saved local evidence:

- `scratch/walk-soak/duckstation-final-0.4.20-candidate.log`

## Known Boundaries

- This does not promote new foreground scenes past the existing human
  signoff bar. `FISHING 1` and `FISHING 2` remain the fully validated
  scenes.
- Footstep audio is still a future audit item. The walk render kernel
  has the hook; the original-engine sample mapping still needs final
  confirmation before enabling real step sounds.
- Freeplay walking remains separate future work. This release is the
  story-loop connector between authored FG2 scenes.
