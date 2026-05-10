---
layout: scene
title: STAND 9 — By the palm, looks around, adjusts pants
ads: STAND
tag: 9
slug: stand9
status: validated
description: "STAND.ADS scene 9: Johnny stands by the palm tree, looks around, and adjusts his pants. Validated 2026-05-08."
image: /assets/img/stand9-ps1-tree-pants.png
image_alt: "STAND 9 on PS1 at night: Johnny stands right next to the palm tree, looking around in the pants-adjust idle pose."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/stand9-ps1-tree-pants.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 9 on PS1 at night: Johnny stands right next to the palm tree, looking around in the pants-adjust idle pose." />
  <figcaption>
    STAND 9 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The tree-side pants-adjust idle. Position-variant pair with
    <a href="{{ '/scenes/stand2/' | relative_url }}">STAND 2</a>'s
    left-edge pants-adjust. Ninth entry in the
    <a href="{{ '/scenes/' | relative_url }}#ads-stand">STAND family</a>.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 9</code>
- Slug: <code>stand9</code>

## What this scene is

Johnny stands by the palm tree, looks around, and adjusts his pants — a tree-side variant of the standing-pose pants-adjust beat (compare STAND 2). Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "looks back inland" caption-mapping guess was wrong (the look-around pose is here but the headline beat is the pants-adjust).

## Validation notes

Visual signoff passed on the normal high-tide/night route through the same
STAND no-stitch fast-path export and per-frame FG2 wave tick that
`STAND 8` introduced.

Boot route:
`fgpilot stand9 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
