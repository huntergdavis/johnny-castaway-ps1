---
layout: scene
title: BUILDING 4 — Lilliputians tie Johnny down while he sleeps
ads: BUILDING
tag: 4
slug: building4
status: validated
description: "BUILDING.ADS scene 4: Johnny sleeps on the sand and lilliputians tie him down with ropes (Gulliver-style). Validated 2026-05-08."
image: /assets/img/building4-ps1-lilliputians.png
image_alt: "BUILDING 4 on PS1 at night: Johnny lies under the palm tree while a crowd of lilliputian figures swarms onto the island carrying their gear, mid-Gulliver-tie-down."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/building4-ps1-lilliputians.webp' | relative_url }}" />
    <img src="{{ '/assets/img/building4-ps1-lilliputians.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="BUILDING 4 on PS1 at night: Johnny lies under the palm tree while a crowd of lilliputian figures swarms onto the island carrying their gear." />
  </picture>
  <figcaption>
    BUILDING 4 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The lilliputians are mid-arrival; Johnny is asleep under the palm
    tree about to be tied down Gulliver-style. Outside the gag, this is
    the canonical clean-rect-heavy variant on the
    <a href="{{ '/lab/from-87-to-99-5/' | relative_url }}">post-validation perf retrospective</a>'s
    <a href="{{ '/docs/glossary/#drop-prefetch' | relative_url }}">drop-prefetch</a> exception list.
  </figcaption>
</figure>

Validated on 2026-05-05 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}). The high/low packs were
regenerated through the generic normal/far-left/far-right foreground-only
multi-view stitch, then converted to FGP3 with an explicit terminal cleanup
frame so the final Johnny/bird foreground row restores cleanly.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 4</code>
- Slug: <code>building4</code>

## What this scene is

Johnny sleeps on the sand and tiny lilliputians swarm in and tie him down with ropes — straight Gulliver. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "sand castle vs. lilliputians" caption-mapping guess had the lilliputians right but the action wrong.

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

`BUILDING 4` is the scene the post-v0.7.0 random-run soak named when
the
[clean-rect]({{ '/docs/glossary/#clean-rect' | relative_url }})
allocation pipeline first regressed under
[walk-clean]({{ '/docs/walks/' | relative_url }}) memory
pressure. The `v0.8.0-ps1` fix retries the large scene clean
snapshot after releasing the stale walk clean buffer and recaptures
the walk baseline so the scene loads cleanly even after a long
random-position approach. `BUILDING 4` also appears on the v0.8.0
[clean-memory-relief drop-prefetch]({{ '/lab/from-87-to-99-5/' | relative_url }})
exception list — the per-scene opt-in that drops the prefetch
window during large clean snapshots. The follow-on `v0.8.1-ps1`
[stability fix]({{ '/lab/v081-mary4-freeze/' | relative_url }})
generalized the same pressure-estimator path for every random-
position scene.
