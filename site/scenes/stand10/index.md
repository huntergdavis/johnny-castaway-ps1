---
layout: scene
title: STAND 10 — Looks at his raft
ads: STAND
tag: 10
slug: stand10
status: validated
description: "STAND.ADS scene 10: Looks at his raft. Validated on PS1."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 10</code>
- Slug: <code>stand10</code>

## What this scene is

Idle at SPOT_D NE: looks at his raft.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

## Validation notes

Visual signoff passed on the normal high-tide/night route using the
previously-committed FG2 pack (96 KB) without regenerating.

The host engine quirks on `STAND.ADS:10`: it exits after only two frames,
so the standard no-stitch export collapses to an empty 92-byte pack.
Rather than chase the host-side cause, we kept the committed pack — which
already plays cleanly on PS1 — and signed off the scene as-is.

Boot route:
`fgpilot stand10 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
