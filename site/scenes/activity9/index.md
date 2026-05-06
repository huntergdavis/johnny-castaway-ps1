---
layout: scene
title: ACTIVITY 9 — Bathes with a brush
ads: ACTIVITY
tag: 9
slug: activity9
status: validated
description: "ACTIVITY.ADS scene 9: Bathes with a brush. Validated after wide boat stitching and source-edge repair."
image: /assets/img/activity9-ps1-boat.png
image_alt: "ACTIVITY 9 running on PS1: Johnny bathes while a boat passes the island."
---

Validated on 2026-05-05 under the current visual + audible signoff bar.

<figure class="scene-hero">
  <img src="{{ '/assets/img/activity9-ps1-boat.png' | relative_url }}"
       width="1127" height="677" loading="lazy" decoding="async" alt="ACTIVITY 9 running on PS1 at night: Johnny stands on the island with the raft visible on the shore as a wide boat with a passenger passes by the moonlit ocean." />
  <figcaption>ACTIVITY 9 on PS1 hardware. Wide boat passing the night-palette island; raft visible on the shore.</figcaption>
</figure>

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 9</code>
- Slug: <code>activity9</code>

## What this scene is

Bathes in the ocean while a wide boat passes through the scene.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

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
