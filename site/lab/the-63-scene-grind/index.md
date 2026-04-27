---
layout: page
title: The 63-scene grind
eyebrow: Lab . Retrospective
subtitle: Two scenes signed off, sixty-one to go. That is not failure. That is a ledger.
description: A retrospective on the finite scene-by-scene validation grind behind Johnny Castaway PS1.
date: 2026-04-26
---

## The finite list

The port has 63 routed scenes in the current ledger.

That number is a gift. It means the project is finite. Not easy. Not short.
Finite.

Each scene has an ADS family and tag. Each scene has a foreground pack. Some
have tide variants. Some have raft variants. Some need holiday overlays to
avoid stomping pixels. Some are mostly standing animation and some are little
movies. They all eventually need the same thing: watch the PS1 output, compare
against reference, fix what is wrong, mark the row.

## Why only two validated

At this release, two scenes are validated: `FISHING 1` and `FISHING 2`.

That can sound small if you treat the scene count like a progress bar. It is
not the right read. The first validated scene built the pipeline. The second
proved the pipeline was not a one-off accident. The remaining sixty-one are
work, but they are work inside a known frame.

The difference between "nothing is known" and "the loop is known" is enormous.

## The daily loop

The loop is:

1. Pick a scene.
2. Capture or refresh the host reference.
3. Generate or inspect the FG2 pack.
4. Wire the runtime if this scene needs a new path.
5. Build the disc.
6. Boot DuckStation.
7. Watch the scene.
8. Compare screenshots, logs, and timing.
9. Fix the obvious thing.
10. Repeat until the human review feels boring.

"Feels boring" is important. A validated scene should not feel lucky. It
should feel like you have watched it enough times that the next playthrough is
uneventful.

## The grind is the project

There is no grand final algorithm that validates the other sixty-one. There
will be tricks. There will be pack improvements. There will be more codegen.
But the work is still one scene at a time.

That is not disappointing. It is the nature of preservation work. The only way
to know a gag landed is to watch the gag.

## Why the website mirrors it

The site has 63 scene pages for the same reason the code has 63 ledger rows.
The shape of the project is the shape of the source material. If a scene has a
status, it deserves a page. If a regtest reference exists, it deserves a page.
If a holiday has an emblem, it deserves a page.

The grind becomes navigable when every unit has a shelf.

## Cross-links

- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regtest reference cases]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
- [Method]({{ '/about/method/' | relative_url }})
