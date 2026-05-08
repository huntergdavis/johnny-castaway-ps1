---
layout: scene
title: STAND 6 — Looks out at the ocean, scratches head
ads: STAND
tag: 6
slug: stand6
status: validated
description: "STAND.ADS scene 6: Johnny looks out at the ocean and scratches his head. Validated 2026-05-08."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 6</code>
- Slug: <code>stand6</code>

## What this scene is

Johnny looks out at the ocean and scratches his head — a confused-looking idle pose in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "looks at the palm" caption-mapping guess was wrong (he's looking out at the ocean, not at the palm tree).

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path with a full-frame single-position foreground-only
overlay (the same export pattern that fixed `STAND 5`).

Boot route:
`fgpilot stand6 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
