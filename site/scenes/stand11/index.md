---
layout: scene
title: STAND 11 — Shades under the palm
ads: STAND
tag: 11
slug: stand11
status: validated
description: "STAND.ADS scene 11: Shades under the palm. Validated on PS1."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 11</code>
- Slug: <code>stand11</code>

## What this scene is

Idle at SPOT_E NW: shades hand over eyes and looks out under the palm.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

## Validation notes

Visual signoff passed on the normal high-tide/night route using the
previously-committed FG2 pack (95 KB) without regenerating.

Like `STAND 10`, the host engine quirks on `STAND.ADS:11`: it exits after
only two frames, so the standard no-stitch export collapses to an empty
92-byte pack. The committed pack already plays cleanly on PS1, so it was
kept as-is.

Boot route:
`fgpilot stand11 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
