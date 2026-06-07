# Walk Subsystem — Spot Coordinates and Pre-Flight Audit

> 🌐 **Rendered version:** **[/source/docs/ps1/walk-spot-coordinates/](https://hunterdavis.com/johnny-castaway-ps1/source/docs/ps1/walk-spot-coordinates/)** — this doc rendered on the project website's source library.

Date: 2026-04-29
Status: pre-flight audit complete (R6.1, R6.2, R6.3, R6.5)
Branch: `walk-implementation-20260429`

This is the artifact the walk implementation plan
([walk-implementation-plan.md](walk-implementation-plan.md)) § R6
calls for: pre-flight audit + canonical (x, y) per spot extracted from
`src/walk/walk_data.h`.

## R6.1 — Spot graph completeness

The graph is **intentionally sparse**. `walkDataBookmarks[6][6]` (line
500 of `walk_data.h`) declares which (fromSpot, toSpot) pairs have a
DIRECT walk path; the rest go through calcPath's shortest-path
routing.

```
       A     B     C     D     E     F
   A   .    68    38     .     0    17       (no direct A→D)
   B  109    .   133     .     .     .       (B is leaf-like)
   C  163  196     .   211   224   245       (C is the hub)
   D    .    .   278     .   289   302       (no direct D→A, D→B)
   E  332    .   356   381     .   394       (no direct E→B)
   F  423    .   443   457   463     .       (no direct F→B)
```

24 of 30 possible directed edges are populated. The 6 missing edges
all involve B (entries to and from B from non-C nodes) and the A↔D
diagonal — Sierra's island geometry doesn't include those direct
paths. `calcPath()` reaches every spot from every other spot by
routing through C (the central hub) when needed.

**Verdict:** graph is fully connected for transition purposes. No
risk to scene-pair walks; calcPath handles the routing.

## R6.5 — Canonical (x, y) per spot

Extracted from `walkDataBookmarksTurns[]` (line 510), which indexes
into `walkData[]` at the **stationary turn-in-place** entry for each
spot. The first frame of each turn block is Johnny's canonical
standing pose at that spot, in absolute 640×480 island-relative
pixel coordinates.

| Spot | Index into walkData | Pose (flip, x, y, frame) |
|---|---|---|
| SPOT_A | walkData[91]  | (0, 300, 243, 10) |
| SPOT_B | walkData[145] | (0, 454, 255, 10) |
| SPOT_C | walkData[260] | (0, 522, 233, 10) |
| SPOT_D | walkData[314] | (0, 480, 219, 10) |
| SPOT_E | walkData[405] | (0, 395, 213, 10) |
| SPOT_F | walkData[471] | (0, 435, 228, 10) |

The frame index `10` consistently across all 6 spots = the
hands-in-pockets idle frame in `JOHNWALK.PSB`. `flip=0` means the
sprite is drawn unflipped; the pose itself communicates direction
via the sprite content.

These are the coordinates the freeplay direct-control mode needs
for its "find nearest spot" lookup, and the coordinates the walk
kernel uses to render Johnny standing at a given spot before/after
a walk transition.

Note: these are **island-relative** pre-offset coordinates. The
runtime adds `islandState.xPos` / `islandState.yPos` (the randomized
island position) before sending pixels to the GPU.

### Spot map (rough geometry)

```
    +------------- 640 wide --------------+
    |                                     |
y=210                  E(395, 213)
    |
y=220                                 D(480, 219)
    |                                     F(435, 228)
y=230                  C(522, 233)
    |
y=240         A(300, 243)
    |
y=250                          B(454, 255)
    |
    +-------------------------------------+
```

A is shore-left-back. C is shore-right (hub). E is centre-back-left.
D is centre-right. F is between E and D. B is centre-front. The
spots correspond to where Johnny stands during specific scenes — A
is fishing left, C is fishing right, B is mid-beach idle, etc.

## R6.2 — JOHNWALK asset audit

| File | Format | Size | Notes |
|---|---|---|---|
| `jc_resources/extracted/bmp/JOHNWALK.BMP` | Sierra .RES extract (non-standard BMP header) | 48,476 bytes | source asset |
| `jc_resources/transcoded/JOHNWALK.PSB` | PS1-ready transcoded sprite atlas | 48,924 bytes | already exists |

**Already transcoded.** `JOHNWALK.PSB` is on disk in the PS1 transcoded
asset directory. The transcoding step from earlier project work has
the asset ready; Phase 1 only needs to add it to the disc layout.
No additional `scripts/transcode-bmp-ps1.py` invocation required.

### Color depth

The transcoded PSB file format the PS1 build uses — confirmed from
the experiment log's repeated PAL4 references — is the same per-tile
palette format other PS1 sprites use. Walk frames will render
through the same PAL4-aware path as scene foregrounds. No special
handling.

### Sprite count

`JOHNWALK.PSB` contains every walking pose Johnny needs: each (spot,
heading) standing pose (turn-in-place block, ~16 frames per spot ×
6 spots = ~96 frames) plus walking-in-progress frames per spot-pair
edge (~24 edges × ~10-16 frames each = ~300-400 frames). The total
~500 sprite frames matches what `walkData[]` indexes into (489
entries).

## R6.3 — extract_walk_data.c lineage

`tools/src/extract_walk_data.c` (1,371 bytes) confirms the lineage
referenced in `walk_data.h:3`. The data was originally extracted
from Sierra's `SCRANTIC.SCR` file — the script-resource container
that holds the walk-animation sequencer data outside `RESOURCE.001`.
The extractor source is in the repo and can be re-run if the data
ever needs to be regenerated.

## R6.4 — footstep sample IDs (removed)

The footsteps feature was dropped on 2026-05-02 before any sample-ID
audit ran. The walk render kernel ships without per-step audio; the
pause-menu toggle and the `footstepsEnabled` global have been removed
from the codebase. The audio coverage on walks is now limited to
ambient sound (ocean loop on its dedicated SPU voice) plus the
existing per-scene SFX.

## Pre-flight verdict

| Item | Status | Risk |
|---|---|---|
| R6.1 graph completeness | done — sparse but connected via calcPath | none |
| R6.2 JOHNWALK.PSB ready | done — already transcoded | none |
| R6.3 extractor lineage | confirmed at `tools/src/extract_walk_data.c` | none |
| R6.5 canonical coords | done — see table above | none |
| R6.4 footstep sample IDs | removed 2026-05-02 — feature dropped | none |

**Phase 1 is cleared to start.**

## Cross-references

- [Walk implementation plan](walk-implementation-plan.md)
- [Freeplay direct-control mode](freeplay-mode-design.md)
- [Hardware constraints](hardware-specs.md)
- Upstream walk module: `src/walk/walk.c`, `src/walk/walk_data.h`, `src/walk/calcpath.c`
- Original asset extraction: `tools/src/extract_walk_data.c`
