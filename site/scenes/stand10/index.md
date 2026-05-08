---
layout: scene
title: STAND 10 — Looks at his raft, looks around
ads: STAND
tag: 10
slug: stand10
status: validated
description: "STAND.ADS scene 10: Johnny looks at his raft and then looks around. Validated 2026-05-08."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 10</code>
- Slug: <code>stand10</code>

## What this scene is

Johnny stands and looks at his raft, then looks around the rest of the island — an idle pose tied to the raft-build progression. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "looks at his raft" caption-mapping with the look-around tail confirmed.

## Validation notes

Visual signoff passed on the normal high-tide/night route using the
previously-committed FG2 pack (96 KB) without regenerating.

The host engine quirks on `STAND.ADS:10`: it exits after only two frames,
so the standard no-stitch export collapses to an empty 92-byte pack.
Rather than chase the host-side cause, we kept the committed pack — which
already plays cleanly on PS1 — and signed off the scene as-is.

Boot route:
`fgpilot stand10 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
