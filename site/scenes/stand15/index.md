---
layout: scene
title: STAND 15 — Looks around with a spyglass
ads: STAND
tag: 15
slug: stand15
status: validated
description: "STAND.ADS scene 15: Johnny stands and looks around with a spyglass. Validated 2026-05-08."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 15</code>
- Slug: <code>stand15</code>

## What this scene is

Johnny stands and looks around the horizon with a spyglass — the spyglass-search idle pose in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "spyglass, left edge" caption-mapping (the look-around with the spyglass is the headline beat).

## Validation notes

Visual signoff passed after regenerating high and low tide packs through
the STAND no-stitch fast path. Unlike `STAND 10`-`STAND 12`, the host
engine plays this scene normally and the export produced a real 48 KB
foreground-only pack. Wave animation came from the runtime FG2 wave
tick `STAND 8` introduced.

Boot route:
`fgpilot stand15 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
