---
layout: page
title: Story-loop walks
eyebrow: Reference
subtitle: How Johnny stops teleporting between scenes — the walk subsystem, the spot graph, and the runtime invariants the kernel has to preserve.
description: Reference manual for the Johnny Castaway PS1 port's story-loop walk subsystem — walk_pilot, walk_render, the six-spot route table from the original Sierra walk_data, and the runtime invariants (persistent clean buffer, ocean waves, palm-tree occlusion, holiday overlay re-stamping) that keep walks visually correct.
---

A labor of love by Hunter Davis. The original *Johnny Castaway*
screensaver did not move Johnny between scenes. Each scene started
with him already at his start spot. When the next scene picked a
different spot, the engine teleported him there. This page documents
the PS1 build's *story-loop walks* — the path that replaced that
teleport with a real walk, using Sierra's own pre-baked route table
from `walk_data.h`.

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## What this is

Story-loop walks fire **between scenes**, not inside them. The
screensaver-loop [scene picker]({{ '/docs/glossary/#scene-picker' | relative_url }}) decides what's next, asks the walk
subsystem to move Johnny from his current `(spot, heading)` to the
next scene's start `(spot, heading)`, and only then plays the next
scene's [FG2 pack]({{ '/docs/glossary/#fg2-pack' | relative_url }}).

The walk happens against the live island background — ocean still
animating, holiday overlay still on screen if applicable, the palm
tree's trunk and leaves still occluding correctly when Johnny passes
behind them. Pause works, mute works, the captions are off (no
caption fires during a walk). When the walk finishes, the next scene
loads its pack as if Johnny had always been at the new spot.

Freeplay's direct-control walking is a different surface — same
draw kernel, but the player drives `(x, y)` per [VBlank]({{ '/docs/glossary/#vblank' | relative_url }}) from the
D-pad instead of consuming a pre-baked path. See
[/docs/freeplay/]({{ '/docs/freeplay/' | relative_url }}).

The feature shipped in
[`v0.4.20-ps1`]({{ '/releases/#v0420-ps1--story-loop-walking' | relative_url }})
and gained three runtime guards in subsequent releases (see
[Evolution by release](#evolution-by-release) below).

## Modules

The walk subsystem lives in five modules under `src/`:

| File | Lines | Owner | Notes |
|---|---:|---|---|
| `walk_data.h` | 533 | Sierra (via `jc_reborn`) | Pre-baked spot graph, shortest-path bookmarks, per-segment frame tables. Unchanged from the upstream engine decode. |
| `walk.c` / `walk.h` | 195 / 25 | upstream | The original walk state machine. Consumes `walk_data.h`; produces a per-tick `(x, y, sprite_idx, heading)`. |
| `calcpath.c` / `.h` | 130 / 25 | upstream | The `calcPath(fromNode, toNode)` BFS-style path-finder over the spot graph. Walk_pilot calls it when no direct (from, to) edge exists in `walk_data.h`. |
| `walk_pilot.c` / `.h` | 354 / 84 | this port | Story-loop walk driver. Wraps `walk.c` + `calcpath.c` for the screensaver loop, owns the persistent walk-area clean buffer, and feeds `walk_render` one frame at a time. |
| `walk_render.c` / `.h` | 119 / 75 | this port | Per-frame draw kernel. Restores the island background, stamps the JOHNWALK sprite, and re-stamps the palm trunk + leaves on top when Johnny is between SPOT_3 and SPOT_4. Used by both walk_pilot and freeplay. |

The split between `walk_pilot` (state) and `walk_render` (pixel push)
is deliberate. Freeplay reuses `walk_render` directly — same kernel,
different driver — and walk_pilot's clean-buffer ownership stays out
of freeplay's allocation budget.

## The spot graph

Johnny stands at one of six spots on the island, labelled `A`
through `F` in the source. The graph is intentionally sparse —
not every (from, to) pair has a direct path. `calcPath` falls back
to shortest-path routing through the bookmarks table when there's
no direct entry.

```
       A     B     C     D     E     F
   A   .    68    38     .     0    17       (no direct A→D)
   B  109    .   133     .     .     .       (B is leaf-like)
   C  163  196     .   211   224   245       (C is the hub)
   D    .    .   278     .   289   302       (no direct D→A, D→B)
   E  332    .   356   381     .   394       (no direct E→B)
   F  423    .   443   457   463     .       (no direct F→B)
```

The numbers are byte offsets into `walkDataPath[]`, not frame
counts. Each entry points at a sequence of `(x, y, sprite_idx)`
tuples the kernel walks through one tick at a time.

The full pre-flight audit, the per-spot canonical `(x, y)`
coordinates, and the heading conventions are documented in the
upstream source doc — see
[walk-spot-coordinates]({{ '/source/docs/ps1/walk-spot-coordinates/' | relative_url }})
for the deep dive.

## Runtime invariants

The walk render kernel has to keep four things true on every walk
frame, against the live island. Breaking any of them shows up
immediately as a visual regression:

- **Waves keep moving.** The ocean wave-band animates per VBlank
  the same as during a scene; walks don't freeze the [backdrop]({{ '/docs/glossary/#backdrop' | relative_url }}).
- **Holiday overlay re-stamps.** If a holiday emblem is on screen,
  the kernel re-stamps it *behind* Johnny each walk frame so he
  visibly walks past it instead of erasing it.
- **Palm-tree occlusion holds.** When Johnny passes between
  `SPOT_3` and `SPOT_4`, `walkRenderFrame` stamps the palm trunk
  and leaves *after* the walking sprite so the tree visibly
  covers him. The `behindTree` flag in the kernel signature is
  the toggle.
- **The walk-area clean buffer is persistent.** Earlier designs
  re-allocated the buffer per walk start; that fragmented the
  heap and made later walks lose their erase baseline. Current
  design captures one tight walk-area buffer during scene setup
  and refreshes its pixels only when island state changes.
  `walkPilotCaptureCleanWalkAreaIfStale` is the entry point —
  it compares the current `(raft, lowTide, night, holidayId,
  xPos, yPos)` to the last capture and re-snapshots only on
  drift.

The state-key is the `(raft, lowTide, night, holidayId, xPos,
yPos)` tuple — six fields. Any change invalidates the cached
clean buffer and forces a re-snapshot.

## Evolution by release

The walk subsystem grew runtime guards over the v0.4.20 → v0.8.1
releases:

- **`v0.4.20-ps1`** — story-loop walks land. Johnny stops
  teleporting between scenes. Footstep audio shipped here too
  but was removed wholesale before any sample-ID audit
  (see the historical
  [walk-implementation-plan]({{ '/source/docs/ps1/walk-implementation-plan/' | relative_url }})
  for the original Phase 4 footsteps record).
- **`v0.7.2-ps1`** — backdrop-key guard. Walks now run only
  when the next scene matches the previous rendered tide,
  raft, night, holiday, and island X/Y state. A scene that
  changes any of those flags skips the walk; the next scene
  starts at its own start spot directly. Without this guard
  some walks rendered against a stale backdrop, e.g. low-tide
  Johnny walking on an in-progress high-tide ocean.
- **`v0.8.0-ps1`** — clean-rect retry path. If the per-walk
  clean-buffer allocation fails first try, the kernel waits one
  frame and retries instead of dropping the walk. Reduced the
  fail-rate-under-pressure from "occasionally noticeable" to
  "not seen since this shipped."
- **`v0.8.1-ps1`** — wave-band/split-rect pressure accounting.
  The clean-rect pressure estimator was counting only foreground
  pack bounds, not the ocean wave band or the upper/lower split
  rects actually saved for restoration. The MARY 4 random-load
  freeze
  ([retrospective]({{ '/lab/v081-mary4-freeze/' | relative_url }}))
  was the canary. Fix is centralized in a helper that mirrors
  the actual save path; covers all 14 random-position scenes.

## How to debug

The BSOD log snapshot ships three diagnostic accessors that print
the walk subsystem's state at the moment of failure:
`walkPilotCleanBufferAllocated`, `walkPilotCleanBufferBytes`, and
`walkPilotJohnwalkSlotLoaded`. Look for them in any TTY/file log
captured during a freeze — the values say whether the persistent
clean buffer was alive at the point of the bug.

For visual debugging, the
[regtest harness]({{ '/docs/regtest/' | relative_url }}) captures
frame PNGs every N frames; comparing two consecutive walk
captures usually reveals stale-backdrop bleed-through. The
[scripted-input harness]({{ '/docs/scripted-input/' | relative_url }})
can drive a deterministic walk by replaying a fixed pad-script.

For repros that the bounded matrix can't reach, run a
[soak]({{ '/docs/glossary/#soak-test' | relative_url }}) — long
randomized DuckStation pass — and watch for the kind of
clean-pressure freeze v0.8.1 fixed.

## Related pages

- [Freeplay reference]({{ '/docs/freeplay/' | relative_url }}) —
  the other consumer of `walk_render`. Same kernel, D-pad input
  instead of pre-baked path.
- [Hardware envelope]({{ '/docs/hardware/' | relative_url }}) —
  why the persistent clean buffer matters: 2 MB total RAM, every
  re-allocation is a fragmentation risk.
- [Performance battle card]({{ '/perf/' | relative_url }}) —
  per-tide timing for every scene, including the walk-adjacent
  ones.
- [Performance reference]({{ '/docs/performance/' | relative_url }})
  — what each column on the battle card means; the walk
  subsystem's persistent clean buffer is part of the same
  pressure-accounting envelope.
- [v0.8.1 retrospective]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — the soak-vs-matrix war story that drove the wave-band pressure
  fix.
- [Glossary: clean-rect]({{ '/docs/glossary/#clean-rect' | relative_url }})
  · [Glossary: BOOTMODE.TXT]({{ '/docs/glossary/#bootmode' | relative_url }})

## Source on GitHub

- [`src/walk/walk_pilot.h`]({{ site.github_url }}/blob/main/src/walk/walk_pilot.h)
  · [`src/walk_pilot.c`]({{ site.github_url }}/blob/main/src/walk_pilot.c)
- [`src/walk/walk_render.h`]({{ site.github_url }}/blob/main/src/walk/walk_render.h)
  · [`src/walk/walk_render.c`]({{ site.github_url }}/blob/main/src/walk/walk_render.c)
- [`src/walk/walk.h`]({{ site.github_url }}/blob/main/src/walk/walk.h)
  · [`src/walk/walk.c`]({{ site.github_url }}/blob/main/src/walk/walk.c)
- [`src/walk/calcpath.h`]({{ site.github_url }}/blob/main/src/walk/calcpath.h)
  · [`src/walk/calcpath.c`]({{ site.github_url }}/blob/main/src/walk/calcpath.c) — the `calcPath(fromNode, toNode)` path-finder the walking pilot calls when it needs to compose a multi-segment route between two scene endpoints (referenced in the body's "(from, to) pair has a direct path" paragraph).
- [`src/walk/walk_data.h`]({{ site.github_url }}/blob/main/src/walk/walk_data.h) — the upstream Sierra route table.
- [`docs/ps1/walk-implementation-plan.md`]({{ site.github_url }}/blob/main/docs/ps1/walk-implementation-plan.md) — original walk port plan, with the Phase 4 footsteps record kept for historical accuracy.
- [`docs/ps1/walk-spot-coordinates.md`]({{ site.github_url }}/blob/main/docs/ps1/walk-spot-coordinates.md) — pre-flight audit and per-spot coordinates.
