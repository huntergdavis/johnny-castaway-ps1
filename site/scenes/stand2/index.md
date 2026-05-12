---
layout: scene
title: STAND 2 — Standing, adjusting pants
ads: STAND
tag: 2
slug: stand2
status: validated
description: "STAND.ADS scene 2: Johnny stands and hitches up his pants. Validated 2026-05-08."
image: /assets/img/stand2-ps1-pants-adjust.png
image_alt: "STAND 2 on PS1 at night: Johnny stands at the leftmost edge of the island in the pants-adjust idle pose, a short subtle stand-loop scene."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/stand2-ps1-pants-adjust.webp' | relative_url }}" />
    <img src="{{ '/assets/img/stand2-ps1-pants-adjust.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 2 on PS1 at night: Johnny stands at the leftmost edge of the island in the pants-adjust idle pose, a short subtle stand-loop scene." />
  </picture>
  <figcaption>
    STAND 2 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The pants-adjust idle loop: Johnny stands at the edge of the
    island and hitches up his shorts. The STAND.ADS family is
    14 of these short stand-pose idle beats — small variations on
    Johnny just being there — and pixel-perfect matching them all
    is much of what made the
    <a href="{{ '/lab/the-63-scene-grind/' | relative_url }}">63-scene grind</a>'s
    tail so long.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 2</code>
- Slug: <code>stand2</code>

## What this scene is

Johnny stands on the island and hitches up his pants — one of the idle standing-pose beats in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "adjusts pants" caption-mapping.

## Validation notes

Visual signoff passed on the normal high-tide/night route:
`fgpilot stand2 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.

The scene is a short pants-adjust idle loop. It played cleanly with no
visible residue, and the Scene Explorer direct-launch path no longer
walks Johnny before the frog/loading transition.
