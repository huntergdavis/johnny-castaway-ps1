---
layout: scene
title: STAND 10 — Looks at his raft, looks around
ads: STAND
tag: 10
slug: stand10
status: validated
description: "STAND.ADS scene 10: Johnny looks at his raft and then looks around. Validated 2026-05-04."
image: /assets/img/stand10-ps1-look-raft.png
image_alt: "STAND 10 on PS1 at night: Johnny stands on the right side of the island next to the palm tree, looking at his raft."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/stand10-ps1-look-raft.webp' | relative_url }}" />
    <img src="{{ '/assets/img/stand10-ps1-look-raft.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="STAND 10 on PS1 at night: Johnny stands on the right side of the island next to the palm tree, looking at his raft." />
  </picture>
  <figcaption>
    STAND 10 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The look-at-raft idle, threaded between
    <a href="{{ '/scenes/walkstuf2/' | relative_url }}">WALKSTUF 2</a>
    (raft-build progress) and
    <a href="{{ '/scenes/mary5/' | relative_url }}">MARY 5</a>
    (pack the raft and sail off) in the running raft-stage arc.
    Engineering quirk: STAND 10's host pipeline exits after only
    two frames, so the standard no-stitch export collapses to a
    92-byte empty pack. Rather than chase the host-side cause,
    the committed pack — which already plays cleanly on PS1 —
    was signed off as-is. Paid pragmatism.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 10</code>
- Slug: <code>stand10</code>

## What this scene is

Johnny stands and looks at his raft, then looks around the rest of the island — an idle pose tied to the raft-build progression. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "looks at his raft" caption-mapping with the look-around tail confirmed.

## Validation notes

Visual signoff passed on the normal high-tide/night route using the
previously-committed FG2 pack (96 KB) without regenerating.

The host engine quirks on `STAND.ADS:10`: it exits after only two frames,
so the standard no-stitch export collapses to an empty 92-byte pack.
Rather than chase the host-side cause, we kept the committed pack — which
already plays cleanly on PS1 — and signed off the scene as-is.

Boot route:
`fgpilot stand10 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
