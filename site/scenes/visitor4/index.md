---
layout: scene
title: VISITOR 4 — Shakes the palm; coconut rolls into the ocean
ads: VISITOR
tag: 4
slug: visitor4
status: validated
description: "VISITOR.ADS scene 4: Johnny shakes the palm tree; a coconut falls and rolls into the ocean. Validated 2026-05-04."
image: /assets/img/visitor4-ps1-coconut-rolls.png
image_alt: "VISITOR 4 on PS1 at night: Johnny stands beside the palm tree, a freshly fallen coconut on the sand at his feet, about to roll right off the island into the ocean."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/visitor4-ps1-coconut-rolls.webp' | relative_url }}" />
    <img src="{{ '/assets/img/visitor4-ps1-coconut-rolls.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="VISITOR 4 on PS1 at night: Johnny stands beside the palm tree, a freshly fallen coconut on the sand at his feet, about to roll right off the island into the ocean." />
  </picture>
  <figcaption>
    VISITOR 4 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The frozen-just-before-the-punchline beat: Johnny has shaken
    the palm, the coconut has dropped to the sand, the next frame
    rolls it off the island into the ocean — straight down the
    drain. VISITOR 4 was the canonical source of the v0.8.4
    <a href="{{ '/docs/glossary/#caption-mismap' | relative_url }}">caption-mismap</a> chain — the original audit attached "coconut
    plane hit" here, but on-PS1 playback showed there's no plane
    in this scene; the throw-coconut-at-plane gag lives at
    <a href="{{ '/scenes/visitor5/' | relative_url }}">VISITOR 5</a>.
    See the
    <a href="{{ '/faq/' | relative_url }}">FAQ Q on caption-vs-scene-title divergence</a>.
  </figcaption>
</figure>

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 4</code>
- Slug: <code>visitor4</code>

## What this scene is

Johnny grabs the palm tree and shakes it. A coconut drops loose, hits the ground, and rolls right off the edge of the island into the ocean — straight down the drain. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "coconut plane hit" caption-mapping guess was wrong (no plane in the on-PS1 pack).

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

VISITOR 4 uses the generic normal/far-left/far-right foreground-only
multi-view stitch so island-relative action is complete across production
placements.

See [the method]({{ '/about/method/' | relative_url }}) for the longer version.
