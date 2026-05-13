---
layout: scene
title: STAND 7 — Looks right, lifts hat
ads: STAND
tag: 7
slug: stand7
status: validated
description: "STAND.ADS scene 7: Johnny looks to the right and lifts his hat. Validated 2026-05-04."
image: /assets/img/stand7-ps1-look-right-hat.png
image_alt: "STAND 7 on PS1 at night: Johnny stands on the right side of the island under the palm tree, facing right and lifting his hat."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/stand7-ps1-look-right-hat.webp' | relative_url }}" />
    <img src="{{ '/assets/img/stand7-ps1-look-right-hat.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 7 on PS1 at night: Johnny stands on the right side of the island under the palm tree, facing right and lifting his hat." />
  </picture>
  <figcaption>
    STAND 7 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The look-right hat-lift idle: Johnny is on the right side of
    the island, facing right, hand at his cap. Directional
    variant in the
    <a href="{{ '/scenes/' | relative_url }}#ads-stand">STAND family</a>'s
    hat-lift beats — see
    <a href="{{ '/scenes/stand3/' | relative_url }}">STAND 3</a>
    (edge-of-island hat-adjust) and
    <a href="{{ '/scenes/stand4/' | relative_url }}">STAND 4</a>
    (front-of-island hat-adjust) for the full hat-pose triple.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 7</code>
- Slug: <code>stand7</code>

## What this scene is

Johnny looks to the right and lifts his hat — a directional variant of the standing-pose hat-lift beat in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; sharpens the prior "lifts hat, looks around" caption-mapping with the look-right direction made explicit.

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path with a full-frame single-position foreground-only
overlay (the same export pattern that fixed `STAND 5` and `STAND 6`).

Boot route:
`fgpilot stand7 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
