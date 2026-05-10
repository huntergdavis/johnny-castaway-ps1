---
layout: scene
title: STAND 16 — Spyglass, right side of island
ads: STAND
tag: 16
slug: stand16
status: validated
description: "STAND.ADS scene 16: Johnny stands on the right side of the island and looks around with a spyglass. Validated 2026-05-08."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 16</code>
- Slug: <code>stand16</code>

## What this scene is

Johnny stands on the right side of the island and looks around the horizon with a spyglass — the right-side variant of the spyglass-search pose (compare STAND 15). Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "spyglass, center" caption-mapping had the spyglass right but the position wrong.

## Validation notes

Visual signoff passed on the normal high-tide/night route after
regenerating high and low tide packs through the STAND no-stitch fast
path. The pack uses the full-frame single-position foreground-only
overlay, and wave animation comes from the runtime FG2 wave tick.

Boot route:
`fgpilot stand16 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
