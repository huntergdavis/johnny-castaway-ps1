---
layout: scene
title: MARY 5 — Packs the raft and says goodbye
ads: MARY
tag: 5
slug: mary5
status: validated
description: "MARY.ADS scene 5: Johnny finishes packing the raft and says goodbye to Mary before sailing off. Validated 2026-05-08."
---

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
