---
layout: scene
title: STAND 12 — Looks out over the right water
ads: STAND
tag: 12
slug: stand12
status: validated
description: "STAND.ADS scene 12: Looks out over the right water. Validated on PS1."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 12</code>
- Slug: <code>stand12</code>

## What this scene is

Idle at SPOT_F S: looks out over the right water.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

## Validation notes

Visual signoff passed on the normal high-tide/night route using the
previously-committed FG2 pack (276 KB) without regenerating.

Same host export quirk as `STAND 10`/`STAND 11`: the host engine exits
`STAND.ADS:12` after only two frames, so the standard no-stitch export
collapses to an empty 92-byte pack. The committed pack already plays
cleanly on PS1 and was kept as-is.

Boot route:
`fgpilot stand12 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
