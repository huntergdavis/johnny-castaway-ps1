---
layout: scene
title: BUILDING 5 — Builds a fire and sits by it
ads: BUILDING
tag: 5
slug: building5
status: validated
description: "BUILDING.ADS scene 5: Johnny builds a small fire on the beach and sits next to it. Validated 2026-05-05."
image: /assets/img/building5-ps1-fire.png
image_alt: "BUILDING 5 on PS1 at night: Johnny sits at the left shoreline next to a small fire, the moon visible above."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/building5-ps1-fire.webp' | relative_url }}" />
    <img src="{{ '/assets/img/building5-ps1-fire.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="BUILDING 5 on PS1 at night: Johnny sits at the left shoreline next to a small fire, the moon visible above." />
  </picture>
  <figcaption>
    BUILDING 5 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Johnny is at the shoreline next to the small driftwood fire he
    just lit. The on-PS1 loop also overturned the original
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>'s
    "fixes the raft (mermaid)" guess — that gag belongs to a
    different scene; this is the campfire beat.
  </figcaption>
</figure>

Validated on 2026-05-05 after regenerating high/low packs through the
generic normal/far-left/far-right foreground-only multi-view stitch. The
raft/mermaid repair scene played cleanly on the normal high-tide/night
validation route.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 5</code>
- Slug: <code>building5</code>

## What this scene is

Johnny gathers driftwood, builds a small fire on the beach, and sits next to it. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "fixes the raft (mermaid)" caption-mapping guess was wrong (the raft/mermaid repair gag is actually elsewhere).

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

This scene is now in the validated set. See
[the method]({{ '/about/method/' | relative_url }}) for the longer version.
