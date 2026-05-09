---
layout: scene
title: SUZY 2 — Johnny drifts in on his raft and meets Suzy
ads: SUZY
tag: 2
slug: suzy2
status: validated
description: "SUZY.ADS scene 2: Johnny drifts in on his raft and is reunited with Suzy. Validated 2026-05-08."
---

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

SUZY 2 is a scene-specific backdrop case: the runtime loads
`SUZBEACH.SCR` instead of painting the normal island/ocean background.
The foreground pack uses a full-frame foreground-only overlay with static
scene-local art included so `MRAFT.BMP` stays in the pack; without that,
Johnny floated in without the raft body. SFX playback also leaves mixer
headroom for overlapping samples so the scene does not clip during the
late audio beats.

## Pack identifiers

- ADS dispatch: <code>SUZY.ADS scene 2</code>
- Slug: <code>suzy2</code>

## What this scene is

Johnny drifts in on his raft and is reunited with Suzy at the shore — the rendezvous payoff for the raft-build and SOS-bottle arcs. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; sharpens the prior "raft drifts to her" caption-mapping with the meet-up made explicit.

### Validation Notes

High and low tide packs were regenerated together and signed off by
human visual + audible review. See [the method]({{ '/about/method/' | relative_url }})
for the longer version.

## Notable runtime history

`SUZY 2` high and low both appear on the
[performance battle card]({{ '/perf/' | relative_url }}) as
**metadata-only** rows — they don't reach a deterministic
scene-end the way story scenes do, so they're excluded from the
matrix's `target_speed` averages on purpose. The same applies to
`SUZY 1`. Visual signoff (the FISHING 1 bar) holds; only the
*timing* gate doesn't.
