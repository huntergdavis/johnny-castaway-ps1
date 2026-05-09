---
layout: scene
title: WALKSTUF 3 — Jogs around the island
ads: WALKSTUF
tag: 3
slug: walkstuf3
status: validated
last_verified: "2026-05-04"
description: "WALKSTUF.ADS scene 3: Johnny jogs around the island. Validated 2026-05-08."
image: /assets/img/walkstuf3-ps1-jog.png
image_alt: "WALKSTUF 3 on PS1 at night: Johnny mid-stride on the left side of the island, jogging in front of the palm tree."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/walkstuf3-ps1-jog.png' | relative_url }}"
       width="961" height="720"
       loading="lazy"
       decoding="async"
       alt="WALKSTUF 3 on PS1 at night: Johnny mid-stride on the left side of the island, jogging in front of the palm tree." />
  <figcaption>
    WALKSTUF 3 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Johnny mid-stride; the exercise-loop gag. The
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>'s
    "jogs around the island" line was originally attached to WALKSTUF 1 —
    on-PS1 playback showed WALKSTUF 1 is actually the yacht-party-and-
    pass-out beat, and the jog gag belongs here. See the
    <a href="{{ '/faq/' | relative_url }}">FAQ Q on caption-vs-scene-title divergence</a>.
  </figcaption>
</figure>

Validated 2026-05-04. Visual + audible signoff on commit
`f2519a0c2`; high-tide nighttime route, packs `WALK3.FG2` /
`WALK3LOW.FG2` already on disc, no rework needed.

## Pack identifiers

- ADS dispatch: <code>WALKSTUF.ADS scene 3</code>
- Slug: <code>walkstuf3</code>

## What this scene is

Johnny jogs around the island, doing laps — the exercise loop. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "walk transition" caption-mapping was vague (this is also where the audit's "jogs around the island" caption actually belongs — WALKSTUF 1 was misattributed to the jogging gag when it's actually the yacht-party-and-pass-out beat).

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

## Notable runtime history

`WALKSTUF 3` is one of the four canonical caption-mapping mismatches
the v0.8.4-ps1 chapter-select grind caught. The original audit
attached "jogs around the island" to `WALKSTUF 1`, but watching every
pack play on hardware showed `WALKSTUF 1` is the yacht-party-and-pass-
out beat — the jogging gag is here. The
[chapter-select-grind retrospective]({{ '/lab/chapter-select-grind/' | relative_url }})
walks through the named mismaps and how the on-PS1 loop surfaced them.
