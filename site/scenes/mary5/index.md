---
layout: scene
title: MARY 5 — Packs the raft and says goodbye
ads: MARY
tag: 5
slug: mary5
status: validated
description: "MARY.ADS scene 5: Johnny finishes packing the raft and says goodbye to Mary before sailing off. Validated 2026-05-08."
image: /assets/img/mary5-ps1-goodbye.png
image_alt: "MARY 5 on PS1 at night: Johnny stands on the island holding Mary the mermaid in a goodbye embrace, the scene's packed raft sitting ready on the right side of the island, the moon overhead."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/mary5-ps1-goodbye.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="MARY 5 on PS1 at night: Johnny stands on the island holding Mary the mermaid in a goodbye embrace, the scene's packed raft sitting ready on the right side of the island, the moon overhead." />
  <figcaption>
    MARY 5 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Mary's day-eight goodbye: Johnny and Mary embrace on the
    island while the scene's own packed raft sits ready on the
    right (the generic island raft is suppressed via the
    <code>NORAFT</code> story flag this scene introduced to direct
    <code>fgpilot</code> playback). The
    <code>FIRST</code> flag also routes through here, skipping the
    walk prelude because the frog/full-wipe transition owns the
    screen at scene start.
  </figcaption>
</figure>

Validated on `2026-05-03` under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 5</code>
- Slug: <code>mary5</code>

## What this scene is

Mary's day-8 goodbye scene. Johnny finishes packing the raft and says goodbye to Mary before sailing off the island; the scene uses its own raft art, so the generic island raft is not applicable. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior HIGH-confidence caption-mapping.

## Validation Notes

High/low packs were rebuilt through the generic normal/far-left/far-right
foreground-only multi-view stitch, with the generic raft off during capture.

Runtime policy now respects the source story flags for direct `fgpilot`
playback too: `NORAFT` clamps the generic raft off even when broad test
routes include `raft-stage`, and `FIRST` skips the walk prelude because the
frog/full-wipe transition owns the screen.

Production island placement remains variable-position safe.
