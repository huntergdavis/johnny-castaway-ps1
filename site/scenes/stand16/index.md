---
layout: scene
title: STAND 16 — Spyglass, right side of island
ads: STAND
tag: 16
slug: stand16
status: validated
description: "STAND.ADS scene 16: Johnny stands on the right side of the island and looks around with a spyglass. Validated 2026-05-08."
image: /assets/img/stand16-ps1-spyglass-right.png
image_alt: "STAND 16 on PS1 at night: Johnny stands on the right side of the island holding a spyglass to his eye, scanning the horizon."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/stand16-ps1-spyglass-right.webp' | relative_url }}" />
    <img src="{{ '/assets/img/stand16-ps1-spyglass-right.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 16 on PS1 at night: Johnny stands on the right side of the island holding a spyglass to his eye, scanning the horizon." />
  </picture>
  <figcaption>
    STAND 16 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The right-side spyglass-search idle. Position-variant pair
    with
    <a href="{{ '/scenes/stand15/' | relative_url }}">STAND 15</a>'s
    left-side spyglass-search — same prop and pose, mirrored
    across the palm tree. Closes the
    <a href="{{ '/scenes/' | relative_url }}#ads-stand">STAND family</a>'s
    14 idle-pose loops.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 16</code>
- Slug: <code>stand16</code>

## What this scene is

Johnny stands on the right side of the island and looks around the horizon with a spyglass — the right-side variant of the spyglass-search pose (compare STAND 15). Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "spyglass, center" caption-mapping had the spyglass right but the position wrong.

## Validation notes

Visual signoff passed on the normal high-tide/night route after
regenerating high and low tide packs through the STAND no-stitch fast
path. The pack uses the full-frame single-position foreground-only
overlay, and wave animation comes from the runtime FG2 wave tick.

Boot route:
`fgpilot stand16 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
