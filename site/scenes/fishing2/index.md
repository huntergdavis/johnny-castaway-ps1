---
layout: scene
title: FISHING 2 — Hooks a Titanic life preserver
ads: FISHING
tag: 2
slug: fishing2
status: validated
description: "FISHING.ADS scene 2: Johnny fishes and reels in a Titanic-stenciled life preserver ring. Validated under the FISHING 1 bar."
image: /assets/img/fishing2-ps1-titanic.png
image_alt: "FISHING 2 on PS1: Johnny pulls a TITANIC-stenciled life preserver ring out of the water on his fishing line."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/fishing2-ps1-titanic.webp' | relative_url }}" />
    <img src="{{ '/assets/img/fishing2-ps1-titanic.png' | relative_url }}"
       width="961" height="720" fetchpriority="high" decoding="async"
       alt="FISHING 2 running on PS1: Johnny on the island reeling a TITANIC-stenciled life preserver ring out of the water on his fishing line." />
  </picture>
  <figcaption>FISHING 2 on PS1 hardware. The line comes up with a Titanic life-preserver ring instead of a fish — the gag the on-PS1 pack actually plays, not the "He catches a boot" caption in the audit.</figcaption>
</figure>

## What happens

Johnny casts a line, and after a beat reels in a round life-preserver ring stenciled `TITANIC` instead of a fish. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail.

(The on-screen caption block below — preserved from the Sierra original — says "He catches a boot," which is a different fishing gag in the game. The caption-to-scene mapping in the original audit appears to be approximate; the on-PS1 pack for FISHING.ADS scene 2 plays the Titanic life-preserver beat.)

Second scene to clear the FISHING 1 bar. Same fishing-pose loop as scene 1, but with a different reel-in object. Confirmed pixel-stable and SFX-synced across night, low-tide, holiday, and raft-stage variants.

## Validation

Validated as of `2026-04-23`. Variants exercised: night, low-tide, holiday, raft-stage.
This scene clears the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 2</code>
- Slug: <code>fishing2</code>
- High-tide pack: <code>FG/FISHING2.FG2</code>
- Low-tide pack: <code>FG/FISH2LOW.FG2</code>

## Variants

- **night** — Dusk/night palette swap ([BOOTMODE]({{ '/docs/glossary/#bootmode' | relative_url }}) `night 1`).
- **low-tide** — Tide state variant; different shoreline geometry (BOOTMODE `lowtide 1`).
- **holiday** — Holiday overlay variants — christmas, halloween, etc. (BOOTMODE `holiday N`).
- **raft-stage** — Cumulative raft-build state; raft sprite gains parts as the player progresses (BOOTMODE `raft-stage N`).

## Notable runtime history

`FISHING 2` is one of the four canonical caption-mapping mismatches the
v0.8.4-ps1 chapter-select grind caught: the on-PS1 pack plays the
Titanic life-preserver beat above, but the on-screen caption block
below ("He catches a boot") describes a different fishing gag. The
"boot" caption actually belongs to `MARY 2` — Mary the mermaid swims
up while Johnny is fishing, he mistakes her for a fish, and the boot
is what he ends up reeling in instead. The
[chapter-select-grind retrospective]({{ '/lab/chapter-select-grind/' | relative_url }})
walks through the named mismaps and how the on-PS1 loop surfaced them.

## Caption

This scene has on-screen caption text. Confidence: HIGH in the [caption audit]({{ '/docs/captions/' | relative_url }}).

<blockquote class="scene-caption">
Johnny goes fishing.<br />
He catches a boot.<br />
He keeps the boot.
</blockquote>
