---
layout: scene
title: STAND 6 — Looks out at the ocean, scratches head
ads: STAND
tag: 6
slug: stand6
status: validated
description: "STAND.ADS scene 6: Johnny looks out at the ocean and scratches his head. Validated 2026-05-08."
image: /assets/img/stand6-ps1-head-scratch.png
image_alt: "STAND 6 on PS1 at night: Johnny stands at the front of the island under the palm tree looking out over the moonlit ocean and scratching his head."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/stand6-ps1-head-scratch.png' | relative_url }}"
       width="961" height="720"
       loading="lazy"
       decoding="async"
       alt="STAND 6 on PS1 at night: Johnny stands at the front of the island under the palm tree looking out over the moonlit ocean and scratching his head." />
  <figcaption>
    STAND 6 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The look-out-and-scratch-head idle: Johnny is at the front of
    the island puzzled by the ocean. Same no-stitch fast-path
    export pattern as
    <a href="{{ '/scenes/stand5/' | relative_url }}">STAND 5</a>
    (single-position foreground-only overlay so frame-0 static
    pixels survive base-diff). Sixth entry in the
    <a href="{{ '/scenes/' | relative_url }}#ads-stand">STAND family</a>.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 6</code>
- Slug: <code>stand6</code>

## What this scene is

Johnny looks out at the ocean and scratches his head — a confused-looking idle pose in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "looks at the palm" caption-mapping guess was wrong (he's looking out at the ocean, not at the palm tree).

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path with a full-frame single-position foreground-only
overlay (the same export pattern that fixed `STAND 5`).

Boot route:
`fgpilot stand6 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
