---
layout: scene
title: MARY 5 — Goodbye
ads: MARY
tag: 5
slug: mary5
status: validated
description: "MARY.ADS scene 5: Goodbye. Validated after multi-view stitch and story-flag policy fixes."
---

Validated on `2026-05-03` under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 5</code>
- Slug: <code>mary5</code>

## What This Scene Is

Mary's day-8 goodbye scene. Johnny leaves on the scene's own raft art,
so the generic island raft is not applicable.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

## Validation Notes

High/low packs were rebuilt through the generic normal/far-left/far-right
foreground-only multi-view stitch, with the generic raft off during capture.

Runtime policy now respects the source story flags for direct `fgpilot`
playback too: `NORAFT` clamps the generic raft off even when broad test
routes include `raft-stage`, and `FIRST` skips the walk prelude because the
frog/full-wipe transition owns the screen.

Production island placement remains variable-position safe.
