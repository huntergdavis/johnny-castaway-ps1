---
layout: scene
title: STAND 15 — Looks around with a spyglass
ads: STAND
tag: 15
slug: stand15
status: validated
description: "STAND.ADS scene 15: Johnny stands and looks around with a spyglass. Validated 2026-05-08."
image: /assets/img/stand15-ps1-spyglass.png
image_alt: "STAND 15 on PS1 at night: Johnny stands on the left edge of the island holding a spyglass to his eye, scanning the horizon."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/stand15-ps1-spyglass.webp' | relative_url }}" />
    <img src="{{ '/assets/img/stand15-ps1-spyglass.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 15 on PS1 at night: Johnny stands on the left edge of the island holding a spyglass to his eye, scanning the horizon." />
  </picture>
  <figcaption>
    STAND 15 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The spyglass-search idle: Johnny is on the left edge of the
    island with the spyglass to his eye — the most visually
    distinctive scene in the
    <a href="{{ '/scenes/' | relative_url }}#ads-stand">STAND family</a>'s
    14-scene catalog of subtle idles. Unlike
    <a href="{{ '/scenes/stand10/' | relative_url }}">STAND 10</a> /
    <a href="{{ '/scenes/stand11/' | relative_url }}">STAND 11</a> /
    <a href="{{ '/scenes/stand12/' | relative_url }}">STAND 12</a>'s
    92-byte-empty-pack quirk, STAND 15's host pipeline plays
    normally and the export produced a real 48 KB
    foreground-only pack. Ocean animation comes from the runtime
    FG2 wave tick that
    <a href="{{ '/scenes/stand8/' | relative_url }}">STAND 8</a>
    introduced.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 15</code>
- Slug: <code>stand15</code>

## What this scene is

Johnny stands and looks around the horizon with a spyglass — the spyglass-search idle pose in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "spyglass, left edge" caption-mapping (the look-around with the spyglass is the headline beat).

## Validation notes

Visual signoff passed after regenerating high and low tide packs through
the STAND no-stitch fast path. Unlike `STAND 10`-`STAND 12`, the host
engine plays this scene normally and the export produced a real 48 KB
foreground-only pack. Wave animation came from the runtime FG2 wave
tick `STAND 8` introduced.

Boot route:
`fgpilot stand15 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
