---
layout: scene
title: BUILDING 7 — Builds a fire, grills a fish, eats it
ads: BUILDING
tag: 7
slug: building7
status: validated
description: "BUILDING.ADS scene 7: Johnny builds a fire, grills a fish on it, and eats the fish. Validated 2026-05-08."
image: /assets/img/building7-ps1-grill-fish.png
image_alt: "BUILDING 7 on PS1 at night: Johnny crouches next to a small fire on the left shoreline, mid-grill of a caught fish."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/building7-ps1-grill-fish.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="BUILDING 7 on PS1 at night: Johnny crouches next to a small fire on the left shoreline, mid-grill of a caught fish." />
  <figcaption>
    BUILDING 7 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Johnny is at the shoreline by his small fire, mid-grill — the
    next beat eats the fish. (Compare
    <a href="{{ '/scenes/building5/' | relative_url }}">BUILDING 5</a>,
    where Johnny just builds the fire and sits by it without the
    grill-and-eat continuation.) The packs were re-stitched so the
    middle campfire interval comes from clean animated foreground
    rows instead of stale full-host pixels, then converted to FGP3
    with an explicit cleanup frame so the post-meal restore is
    clean. The on-PS1 loop also overturned the original
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>'s
    "builds up the raft" guess for this scene.
  </figcaption>
</figure>

Validated on 2026-05-05 after regenerating high/low packs through the generic
normal/far-left/far-right foreground-only multi-view stitch. The middle
campfire interval is reconstructed from clean animated foreground rows instead
of stale full-host pixels, then the packs are converted to FGP3 with an
explicit cleanup frame.

User visual signoff passed on the normal high-tide/night validation route. A
last-100-frame full-host review did not show a fish-skeleton draw in this
scene.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 7</code>
- Slug: <code>building7</code>

## What this scene is

Johnny builds a small fire on the beach, grills a fish on it, and eats the fish. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "builds up the raft" caption-mapping guess was wrong (the raft-building gag is elsewhere).

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

This scene is now in the validated set under the current FISHING 1 bar. See
[the method]({{ '/about/method/' | relative_url }}) for the longer version.
