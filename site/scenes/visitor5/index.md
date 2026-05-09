---
layout: scene
title: VISITOR 5 — Throws a coconut at a plane; it crashes
ads: VISITOR
tag: 5
slug: visitor5
status: validated
description: "VISITOR.ADS scene 5: Johnny sees a plane overhead, throws a coconut at it, and the plane crashes. Validated 2026-05-08."
image: /assets/img/visitor5-ps1-plane.png
image_alt: "VISITOR 5 on PS1 at night: a small biplane approaches from the upper-right while Johnny stands under the palm tree on his raft, about to throw a coconut at it."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/visitor5-ps1-plane.png' | relative_url }}"
       width="961" height="720"
       loading="lazy"
       decoding="async"
       alt="VISITOR 5 on PS1 at night: a small biplane approaches from the upper-right while Johnny stands under the palm tree on his raft." />
  <figcaption>
    VISITOR 5 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The biplane has come into frame from the upper right; Johnny is
    about to grab a coconut and bring it down. The
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>'s
    "coconut plane hit" line was originally attached to VISITOR 4 — the
    on-PS1 loop showed VISITOR 4 is just a coconut rolling off the island
    into the ocean, and the throw-at-plane gag belongs here. See the
    <a href="{{ '/faq/' | relative_url }}">FAQ Q on caption-vs-scene-title divergence</a>.
  </figcaption>
</figure>

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 5</code>
- Slug: <code>visitor5</code>

## What this scene is

A plane flies overhead. Johnny grabs a coconut and throws it at the plane — and the coconut hits hard enough that the plane crashes. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "visitor takes Johnny" caption-mapping guess was wrong (this scene is also where the audit's "coconut plane hit" caption actually belongs — VISITOR 4 was misattributed).

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

VISITOR 5 uses the generic normal/far-left/far-right foreground-only
multi-view stitch so island-relative action is complete across production
placements. Its high/low packs also use scene-specific hold redistribution
so the coconut impact and downed-plane motion stay readable without
changing total scene duration.

See [the method]({{ '/about/method/' | relative_url }}) for the longer version.

## Notable runtime history

`VISITOR 5` is one of the four canonical caption-mapping mismatches
the v0.8.4-ps1 chapter-select grind caught. The original audit had the
"coconut plane hit" caption attached to `VISITOR 4`, but watching every
pack play on hardware showed `VISITOR 4` is just a coconut rolling off
the island into the ocean — the throw-coconut-at-plane gag that
brings down the plane lives here. The
[chapter-select-grind retrospective]({{ '/lab/chapter-select-grind/' | relative_url }})
walks through the named mismaps and how the on-PS1 loop surfaced them.
