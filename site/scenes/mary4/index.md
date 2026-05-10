---
layout: scene
title: MARY 4 — Mary's feelings hurt as Johnny works on the raft
ads: MARY
tag: 4
slug: mary4
status: validated
description: "MARY.ADS scene 4: Johnny works on his raft to get off the island; Mary sees and is heartbroken that he plans to leave. Validated 2026-05-08."
image: /assets/img/mary4-ps1-raft-heartbreak.png
image_alt: "MARY 4 on PS1: Johnny working on his raft on the island while Mary the mermaid watches from the water, heartbroken."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/mary4-ps1-raft-heartbreak.png' | relative_url }}"
       width="961" height="720" loading="lazy" decoding="async"
       alt="MARY 4 running on PS1: Johnny working on his raft on the island while Mary the mermaid watches from the water with a heartbroken expression." />
  <figcaption>MARY 4 on PS1 hardware. Mary surfaces and sees what Johnny is doing — fixing the raft so he can sail home. The clean-rect pressure during this scene's random-position load is what the v0.8.1-ps1 soak loop caught and the
  <a href="{{ '/lab/v081-mary4-freeze/' | relative_url }}">soak retrospective</a> walks through.</figcaption>
</figure>

Validated 2026-05-03 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}): pixel-perfect human
visual signoff plus synced captured SFX.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 4</code>
- Slug: <code>mary4</code>
- High-tide pack: <code>MARY4.FG2</code>
- Low-tide pack: <code>MARY4LOW.FG2</code>
- Source-table note: generic multi-view scene-relative stitch; production island placement remains variable

## What this scene is

Johnny is working on the raft, fixing it up so he can sail home. Mary the mermaid surfaces and sees what he's doing — and her feelings are hurt by the realization that he plans to leave the island. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches and sharpens the prior "heartbroken at the raft" caption-mapping with the cause (the raft work) made explicit.

## Validation notes

The single-position host capture clipped different island-relative
pixels at different runtime placements. The validated pack uses the
generic multi-view stitch: normal, far-left, and far-right
foreground-only host views are merged into one scene-relative
foreground canvas, with a magenta synthetic base so first-frame
foreground pixels are retained.

Far-right PS1 stress playback at <code>island-pos 300 54</code> passed.
That is evidence for pack completeness, not a production pin; normal
story playback can keep randomized island placement.

## Notable runtime history

`v0.8.1-ps1` shipped because of this scene. A randomized
[soak]({{ '/docs/glossary/#soak-test' | relative_url }}) on top of
`v0.8.0-ps1` froze when the screensaver loop randomly picked
`mary4` — the
[clean-rect]({{ '/docs/glossary/#clean-rect' | relative_url }})
pressure estimator under-counted the upper/lower split save plus
the ocean wave band, so optional prefetch and
[walk]({{ '/docs/walks/' | relative_url }}) memory wasn't
released early enough and the heap fragmented before scene playback
could start. The fix is centralized (it covers fourteen non-exempt
random-position scenes, not just `mary4`) and the matrix mean
stayed compatible. The full retrospective is at
[/lab/v081-mary4-freeze/]({{ '/lab/v081-mary4-freeze/' | relative_url }}).
