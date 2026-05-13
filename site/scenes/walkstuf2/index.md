---
layout: scene
title: WALKSTUF 2 — Works on the raft
ads: WALKSTUF
tag: 2
slug: walkstuf2
status: validated
last_verified: "2026-05-04"
description: "WALKSTUF.ADS scene 2: Johnny works on his raft. Validated 2026-05-04."
image: /assets/img/walkstuf2-ps1-raft-build.png
image_alt: "WALKSTUF 2 on PS1 at night: Johnny crouches over the raft frame on the right side of the island, building it up."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/walkstuf2-ps1-raft-build.webp' | relative_url }}" />
    <img src="{{ '/assets/img/walkstuf2-ps1-raft-build.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="WALKSTUF 2 on PS1 at night: Johnny crouches over the raft frame on the right side of the island, building it up." />
  </picture>
  <figcaption>
    WALKSTUF 2 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Johnny is at the raft mid-build — one of the
    <code>raft-stage</code> variant-driving beats that visibly
    grows the island raft sprite over the course of a long
    screensaver session. The end of that arc is
    <a href="{{ '/scenes/mary5/' | relative_url }}">MARY 5</a>
    (Johnny packs the raft and sails off) and
    <a href="{{ '/scenes/suzy2/' | relative_url }}">SUZY 2</a>
    (he drifts in to the home beach), which is why this scene's
    on-screen progress matters across the whole 6-scene cross-island
    arc rather than just inside this single replay.
  </figcaption>
</figure>

Validated 2026-05-04. Visual + audible signoff on the existing on-disc
`WALK2.FG2` / `WALK2LOW.FG2` packs (no rework needed) — high-tide
nighttime route.

## Pack identifiers

- ADS dispatch: <code>WALKSTUF.ADS scene 2</code>
- Slug: <code>walkstuf2</code>

## What this scene is

Johnny works on his raft, building it up — one of the visible-progress beats that drives the raft-stage variant flag. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "walk transition" caption-mapping was vague (this scene is specifically the raft-build work, not a generic walk loop).

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
