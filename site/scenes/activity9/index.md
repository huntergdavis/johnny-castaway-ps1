---
layout: scene
title: ACTIVITY 9 — Rain dance, boat with couple passes, costume drops, naked
ads: ACTIVITY
tag: 9
slug: activity9
status: validated
description: "ACTIVITY.ADS scene 9: Johnny in costume rain-dances; a boat carrying a couple passes by, he drops the costume, and the couple keeps sailing. Validated 2026-05-05."
image: /assets/img/activity9-ps1-boat.png
image_alt: "ACTIVITY 9 running on PS1: Johnny rain-dances while a boat carrying a couple passes the island."
image_width: 1127
image_height: 677
---

Validated on 2026-05-05 under the current visual + audible signoff bar.

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/activity9-ps1-boat.webp' | relative_url }}" />
    <img src="{{ '/assets/img/activity9-ps1-boat.png' | relative_url }}"
       width="1127" height="677" fetchpriority="high" decoding="async" alt="ACTIVITY 9 running on PS1 at night: Johnny in costume on the island as a wide boat carrying a couple passes by the moonlit ocean." />
  </picture>
  <figcaption>ACTIVITY 9 on PS1 hardware. Costumed Johnny mid-rain-dance as the wide boat with passengers passes the night-palette island.</figcaption>
</figure>

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 9</code>
- Slug: <code>activity9</code>

## What this scene is

Johnny puts on a costume and starts a rain dance. A boat carrying a couple passes by — Johnny drops his costume and is left naked in the surf, and the embarrassed couple keeps sailing past. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "bathes with a brush" caption-mapping guess was wrong. The boat is real — that's why this scene needed the Activity9-specific wide stitch and `patch-activity9-boat-foreground.py`.

## Validation notes

`ACTIVITY 9` is the edge case that proved a source sprite can be wider than
the legacy scene clip while still being valid scene-relative content. The
high/low packs were rebuilt through an Activity9-specific wide stitch using
host capture/test island positions `x=-500,y=54`, `x=-154,y=54`, and
`x=500,y=54`; production playback remains variable-position safe.

`patch-activity9-boat-foreground.py` fills missing `BOAT.PSB` bow/stern
pixels from the decoded source sprite at the legacy clip edges, only into
keyed foreground holes. It also overlaps the clip edge narrowly to remove the
vertical seam and carries the last boat draw across metadata-held frames so the
late bow does not flicker.

See the live [ledger]({{ '/scenes/' | relative_url }}) and
[the method]({{ '/about/method/' | relative_url }}) for the full validation
workflow.

## Notable runtime history

`ACTIVITY 9` is one of the structurally important scenes in the
project. Two distinct retrospectives have it as their case study:

- **The last validated scene of the 63-scene grind.** It closed the
  visual-signoff sweep on `2026-05-05`. The end of the 63-scene
  grind is what cleared the path for the post-validation
  performance work; the
  [grind retrospective]({{ '/lab/the-63-scene-grind/' | relative_url }})
  walks through how the foreground-only multi-view stitch family
  cleared the last cluster.
- **An "optimized validated outlier" in `v0.8.0-ps1`.** The wide
  boat extends past the legacy 640px scene clip, which made the
  scene a poor fit for global [stream-window]({{ '/docs/glossary/#stream-window' | relative_url }}) settings. The
  Activity9-specific scoped low-tide
  [read group]({{ '/docs/glossary/#read-group' | relative_url }})
  + [padded residual]({{ '/docs/glossary/#padded-residual' | relative_url }})
  packs let it stay green on the matrix instead of being a known
  exception. The
  [perf retrospective]({{ '/lab/from-87-to-99-5/' | relative_url }})
  names this as the canonical example of "the technique generalized
  even though the win was specific."

Production playback stays random-position-safe — the host capture
positions in the validation notes were evidence-gathering, not
runtime pins.
