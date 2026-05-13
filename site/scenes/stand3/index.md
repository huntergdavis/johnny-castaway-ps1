---
layout: scene
title: STAND 3 — Standing at edge of island, adjusts hat
ads: STAND
tag: 3
slug: stand3
status: validated
description: "STAND.ADS scene 3: Johnny stands at the edge of the island and adjusts his hat. Validated 2026-05-04."
image: /assets/img/stand3-ps1-hat-adjust.png
image_alt: "STAND 3 on PS1 at night: Johnny stands at the leftmost edge of the island and lifts his hat in a short idle pose."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/stand3-ps1-hat-adjust.webp' | relative_url }}" />
    <img src="{{ '/assets/img/stand3-ps1-hat-adjust.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 3 on PS1 at night: Johnny stands at the leftmost edge of the island and lifts his hat in a short idle pose." />
  </picture>
  <figcaption>
    STAND 3 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The hat-adjust idle: Johnny is at the left edge with his hand
    raised to his cap. Third entry in the
    <a href="{{ '/scenes/' | relative_url }}#ads-stand">STAND family</a>'s
    14-scene catalog of subtle stand-pose loops.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 3</code>
- Slug: <code>stand3</code>

## What this scene is

Johnny stands at the edge of the island and adjusts his hat — one of the idle standing-pose beats in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches and sharpens the prior "lifts hat" caption-mapping with the edge-of-island position made explicit.

## Validation notes

Visual signoff passed on the normal high-tide/night route:
`fgpilot stand3 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.

The scene is a short hat-lift idle loop and played cleanly.
