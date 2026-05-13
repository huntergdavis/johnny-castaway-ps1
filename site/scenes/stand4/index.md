---
layout: scene
title: STAND 4 — Standing at front of island, adjusts hat
ads: STAND
tag: 4
slug: stand4
status: validated
description: "STAND.ADS scene 4: Johnny stands at the front of the island and adjusts his hat. Validated 2026-05-04."
image: /assets/img/stand4-ps1-front-hat.png
image_alt: "STAND 4 on PS1 at night: Johnny stands at the front of the island under the palm tree, adjusting his hat in a short idle pose."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/stand4-ps1-front-hat.webp' | relative_url }}" />
    <img src="{{ '/assets/img/stand4-ps1-front-hat.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 4 on PS1 at night: Johnny stands at the front of the island under the palm tree, adjusting his hat in a short idle pose." />
  </picture>
  <figcaption>
    STAND 4 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Front-of-island hat-adjust idle. Compare
    <a href="{{ '/scenes/stand3/' | relative_url }}">STAND 3</a>,
    the edge-of-island variant of the same gag — small island
    placement is what differs. Fourth entry in the
    <a href="{{ '/scenes/' | relative_url }}#ads-stand">STAND family</a>'s
    14-scene catalog.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 4</code>
- Slug: <code>stand4</code>

## What this scene is

Johnny stands at the front of the island and adjusts his hat — front-position variant of the STAND family's hat-adjust pose (compare STAND 3, the edge-of-island variant). Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "taps foot" caption-mapping guess was wrong (no foot-tap in the on-PS1 pack).

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
generic normal/far-left/far-right foreground-only multi-view stitch.

Boot route:
`fgpilot stand4 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.

The scene is a tapping-foot idle loop and played cleanly.
