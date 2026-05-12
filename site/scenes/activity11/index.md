---
layout: scene
title: ACTIVITY 11 — Bird steals Johnny's clothes
ads: ACTIVITY
tag: 11
slug: activity11
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 11: A bird swoops in and steals Johnny's clothes. Validated 2026-05-08."
image: /assets/img/activity11-ps1-bird-clothes-thief.png
image_alt: "ACTIVITY 11 on PS1 at night: a bird perches in the palm leaves holding Johnny's clothes while a naked Johnny stands in the water at the lower-left of the island."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/activity11-ps1-bird-clothes-thief.webp' | relative_url }}" />
    <img src="{{ '/assets/img/activity11-ps1-bird-clothes-thief.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="ACTIVITY 11 on PS1 at night: a bird perches in the palm leaves holding Johnny's clothes while a naked Johnny stands in the water at the lower-left of the island." />
  </picture>
  <figcaption>
    ACTIVITY 11 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Both halves of the gag in one frame: the bird is perched in
    the palm leaves with the stolen clothes, and Johnny is in the
    water at the lower-left, clothes-less. The on-PS1 loop also
    overturned the original
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>'s
    "rain dance" guess for this scene — that gag is
    <a href="{{ '/scenes/activity5/' | relative_url }}">ACTIVITY 5</a>.
  </figcaption>
</figure>

Validated 2026-05-05. Visual + audible signoff after re-exporting
high/low packs with a frame-wide keyed overlay
(`KEYED_OVERLAY_RECT="0,0,640,480"`); fixes stale Johnny / bird-outline
residue that the foreground-only diff carried against the moving rain
backdrop.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 11</code>
- Slug: <code>activity11</code>

## What this scene is

A bird swoops in and steals Johnny's clothes off the island. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "rain dance" caption-mapping guess was wrong.

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
