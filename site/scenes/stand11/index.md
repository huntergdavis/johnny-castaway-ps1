---
layout: scene
title: STAND 11 — Left side of island, looks around
ads: STAND
tag: 11
slug: stand11
status: validated
description: "STAND.ADS scene 11: Johnny stands on the left side of the island and looks around. Validated 2026-05-08."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 11</code>
- Slug: <code>stand11</code>

## What this scene is

Johnny stands on the left side of the island and looks around — a left-position idle pose in the STAND family (compare STAND 8, the right-side variant). Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "shades under the palm" caption-mapping guess was wrong (no shading-eyes pose in the on-PS1 pack).

## Validation notes

Visual signoff passed on the normal high-tide/night route using the
previously-committed FG2 pack (95 KB) without regenerating.

Like `STAND 10`, the host engine quirks on `STAND.ADS:11`: it exits after
only two frames, so the standard no-stitch export collapses to an empty
92-byte pack. The committed pack already plays cleanly on PS1, so it was
kept as-is.

Boot route:
`fgpilot stand11 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
