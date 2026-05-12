---
layout: scene
title: BUILDING 3 — Rolls over and takes a nap
ads: BUILDING
tag: 3
slug: building3
status: validated
description: "BUILDING.ADS scene 3: Johnny lies down on the sand, rolls over, and takes a nap. Validated 2026-05-08."
image: /assets/img/building3-ps1-nap.png
image_alt: "BUILDING 3 on PS1 at night: Johnny lies on the sand under the palm tree, eyes closed, with a column of Z's rising up the palm's trunk."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/building3-ps1-nap.webp' | relative_url }}" />
    <img src="{{ '/assets/img/building3-ps1-nap.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="BUILDING 3 on PS1 at night: Johnny lies on the sand under the palm tree with a column of Z's rising up the palm's trunk." />
  </picture>
  <figcaption>
    BUILDING 3 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The canonical sleep frame: Johnny is on the sand under the palm
    with a column of Z's climbing the trunk. The on-PS1 loop also
    overturned the original
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>'s
    "builds a fire" guess — the long sleep/idle beat was already
    implicit in the engineering trailer, just not yet labelled.
  </figcaption>
</figure>

Validated on 2026-05-05 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}). The high/low packs were
regenerated through the generic normal/far-left/far-right foreground-only
multi-view stitch, and the long sleeping/idle beat played cleanly on the
normal low-tide/night validation route.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 3</code>
- Slug: <code>building3</code>

## What this scene is

Johnny lies down on the sand, rolls over, and takes a long nap. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "builds a fire" caption-mapping guess was wrong (the long sleeping/idle beat was already implicit in the engineering trailer).

### How this scene gets validated

The same way every scene does: under the FISHING 1 bar.

A host-side Johnny Reborn capture/export pass produces a
base-diff `.FG2` foreground pack and a JSONL of sound events. The PS1
build replays that pack at native resolution through every variant the
original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). The
[regtest harness]({{ '/docs/regtest/' | relative_url }}) checks that the
visuals come out frame-identical and the SFX cues land on the same
ticks. Once that holds across all applicable variants, the scene moves
to `validated` and a row turns green in the
[ledger]({{ '/scenes/' | relative_url }}).
