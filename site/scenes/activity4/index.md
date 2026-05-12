---
layout: scene
title: ACTIVITY 4 — Climbs the palm and dives in
ads: ACTIVITY
tag: 4
slug: activity4
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 4: Johnny climbs the palm and dives in cleanly. Validated 2026-05-05."
image: /assets/img/activity4-ps1-clean-dive.png
image_alt: "ACTIVITY 4 on PS1 at night: Johnny in the water at the lower-left of the island, post-clean-dive, with a small splash and a seagull above."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/activity4-ps1-clean-dive.webp' | relative_url }}" />
    <img src="{{ '/assets/img/activity4-ps1-clean-dive.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="ACTIVITY 4 on PS1 at night: Johnny in the water at the lower-left of the island, post-clean-dive, with a small splash and a seagull above." />
  </picture>
  <figcaption>
    ACTIVITY 4 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Johnny has just hit the water cleanly off the palm — the
    clean-dive variant of the same climb-and-water arc as
    <a href="{{ '/scenes/activity1/' | relative_url }}">ACTIVITY 1</a>'s
    belly-flop. The on-PS1 loop also overturned the original
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>'s
    "reads, seagull on head" guess for this scene; the seagull is
    overhead but the gag is the dive, not the read.
  </figcaption>
</figure>

Validated 2026-05-05. Visual + audible signoff on the existing on-disc
`ACTIVITY4.FG2` / `ACTV4LOW.FG2` packs — no rework needed. High-tide
nighttime route, palm-climb-then-clean-dive.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 4</code>
- Slug: <code>activity4</code>

## What this scene is

Johnny climbs the palm tree and dives neatly into the water — the
clean-dive variant. (Compare ACTIVITY 1, which is the belly-flop
variant of the same climb-and-dive arc.) Confirmed by direct on-PS1
playback observation; the original "reads, seagull on head" caption-
mapping guess didn't match the pack on disc.

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
