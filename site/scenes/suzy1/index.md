---
layout: scene
title: SUZY 1 — Suzy finds a letter from Johnny
ads: SUZY
tag: 1
slug: suzy1
status: validated
description: "SUZY.ADS scene 1: Suzy back home finds a letter that Johnny sent her from the island. Validated 2026-05-08."
---

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

SUZY 1 is not an island/ocean scene. The PS1 runtime now loads the
source `SUZBEACH.SCR` screen as the clean backdrop for SUZY scenes,
then replays the foreground pack over that beach background.

## Pack identifiers

- ADS dispatch: <code>SUZY.ADS scene 1</code>
- Slug: <code>suzy1</code>

## What this scene is

Suzy, back home, finds a letter that Johnny sent her from the island — the inbound side of the JOHNNY 3 letter-to-Suzy gag. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "sunbathing daydream" caption-mapping guess was wrong.

### Validation notes

High and low packs were regenerated with the current foreground exporter.
The blocking defect was backdrop classification: the scene was correctly
captured, but runtime playback still painted the normal island and water
behind it. `SUZBEACH.SCR` is now included on disc and selected by the
FG2 runtime for SUZY scenes.

## Notable runtime history

`SUZY 1` high and low both appear on the
[performance battle card]({{ '/perf/' | relative_url }}) as
**metadata-only** rows — they don't reach a deterministic
scene-end the way story scenes do, so they're excluded from the
matrix's `target_speed` averages on purpose. The same applies to
`SUZY 2`. Visual signoff (the FISHING 1 bar) holds; only the
*timing* gate doesn't.
