---
layout: scene
title: STAND 12 — Looks forward, adjusts hat
ads: STAND
tag: 12
slug: stand12
status: validated
description: "STAND.ADS scene 12: Johnny looks forward and adjusts his hat. Validated 2026-05-08."
image: /assets/img/stand12-ps1-forward-hat.png
image_alt: "STAND 12 on PS1 at night: Johnny stands at the center of the island under the palm tree, looking forward and adjusting his hat."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/stand12-ps1-forward-hat.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 12 on PS1 at night: Johnny stands at the center of the island under the palm tree, looking forward and adjusting his hat." />
  <figcaption>
    STAND 12 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The forward-facing hat-adjust idle. Extends the
    STAND-family hat-pose mini-cluster from a triple to a quartet:
    <a href="{{ '/scenes/stand3/' | relative_url }}">STAND 3</a>
    edge-hat-adjust,
    <a href="{{ '/scenes/stand4/' | relative_url }}">STAND 4</a>
    front-hat-adjust,
    <a href="{{ '/scenes/stand7/' | relative_url }}">STAND 7</a>
    look-right-hat-lift, and STAND 12 forward-hat-adjust. Same
    92-byte-empty-pack export quirk as
    <a href="{{ '/scenes/stand10/' | relative_url }}">STAND 10</a>
    and
    <a href="{{ '/scenes/stand11/' | relative_url }}">STAND 11</a>
    (host pipeline exits after two frames; previously-committed
    pack kept as-is).
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 12</code>
- Slug: <code>stand12</code>

## What this scene is

Johnny looks forward and adjusts his hat — a forward-facing variant of the STAND family's hat-adjust pose. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "looks out over the right water" caption-mapping guess was wrong.

## Validation notes

Visual signoff passed on the normal high-tide/night route using the
previously-committed FG2 pack (276 KB) without regenerating.

Same host export quirk as `STAND 10`/`STAND 11`: the host engine exits
`STAND.ADS:12` after only two frames, so the standard no-stitch export
collapses to an empty 92-byte pack. The committed pack already plays
cleanly on PS1 and was kept as-is.

Boot route:
`fgpilot stand12 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
