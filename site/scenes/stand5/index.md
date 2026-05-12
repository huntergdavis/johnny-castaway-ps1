---
layout: scene
title: STAND 5 — Standing at front of island, looking out over the ocean
ads: STAND
tag: 5
slug: stand5
status: validated
description: "STAND.ADS scene 5: Johnny stands at the front of the island and looks out over the ocean. Validated 2026-05-08."
image: /assets/img/stand5-ps1-look-out.png
image_alt: "STAND 5 on PS1 at night: Johnny stands at the front of the island under the palm tree, looking out over the moonlit ocean."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/stand5-ps1-look-out.webp' | relative_url }}" />
    <img src="{{ '/assets/img/stand5-ps1-look-out.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 5 on PS1 at night: Johnny stands at the front of the island under the palm tree, looking out over the moonlit ocean." />
  </picture>
  <figcaption>
    STAND 5 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The look-out-to-sea idle pose at the front of the island.
    Engineering footnote: STAND 5 is what surfaced the
    "<a href="{{ '/docs/glossary/#no-stitch-fast-path' | relative_url }}">no-stitch fast path</a>
    fades Johnny's legs" bug — pure base-diff
    treated frame-0 static pixels as background and dropped them.
    The exporter fast path now keeps a single-position
    foreground-only overlay while still skipping far-left /
    far-right stitch captures for simple STAND scenes. Fifth
    entry in the
    <a href="{{ '/scenes/' | relative_url }}#ads-stand">STAND family</a>.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 5</code>
- Slug: <code>stand5</code>

## What this scene is

Johnny stands at the front of the island and looks out over the ocean — the look-out-to-sea idle pose in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "looks out over the ocean" caption-mapping with the front-of-island position made explicit.

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path.

Boot route:
`fgpilot stand5 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.

The first no-stitch attempt faded Johnny's legs because pure base-diff treated
frame-0 static pixels as background. The exporter fast path now keeps a
single-position foreground-only overlay while still skipping far-left/far-right
stitch captures for simple STAND scenes.
