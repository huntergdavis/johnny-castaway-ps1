---
layout: scene
title: ACTIVITY 5 — Rain dance, struck by lightning
ads: ACTIVITY
tag: 5
slug: activity5
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 5: Johnny puts on a costume, performs a rain dance, and gets struck by lightning. Validated 2026-05-05."
image: /assets/img/activity5-ps1-rain-dance.png
image_alt: "ACTIVITY 5 on PS1 at night: Johnny in costume mid-rain-dance with a storm-cloud thought bubble hovering over the palm tree."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/activity5-ps1-rain-dance.webp' | relative_url }}" />
    <img src="{{ '/assets/img/activity5-ps1-rain-dance.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="ACTIVITY 5 on PS1 at night: Johnny in costume mid-rain-dance with a storm-cloud thought bubble hovering over the palm tree." />
  </picture>
  <figcaption>
    ACTIVITY 5 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Johnny is in costume mid-dance; the storm-cloud thought bubble
    (source frame 46, held +30 vblanks for readability) is the
    cue right before the lightning strike. The pack uses the
    JOHNNY 2-style upper-bubble + lower-overlay split so the
    bubble survives base-diff and the post-strike splash cleans
    up without ghost trails.
  </figcaption>
</figure>

Validated 2026-05-05. Visual + audible signoff on the rain-dance gag.
High/low packs use the JOHNNY 2-style split — upper thought-bubble
lane on full base-diff so the storm-cloud bubble + connector dots
survive, and lower third on keyed overlay so the post-strike splash
cleans up without ghost trails. Source frame 46 (the storm-cloud
bubble) gets +30 vblanks of hold so the gag is readable.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 5</code>
- Slug: <code>activity5</code>

## What this scene is

Johnny puts on a costume and performs a rain dance, then gets struck
by lightning. The storm-cloud thought bubble appears mid-dance (frame
46, held +30 vblanks for readability) and the lightning-strike splash
follows. Confirmed by direct on-PS1 playback observation while
capturing the chapter-select thumbnail; the earlier "climb / look /
dive" caption-mapping guess was wrong.

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
