---
layout: scene
title: VISITOR 3 — Yacht couple, photos
ads: VISITOR
tag: 3
slug: visitor3
status: validated
description: "VISITOR.ADS scene 3: Yacht couple, photos. Validated on PS1 after VISITOR3-specific red-ship and splash synthesis."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 3</code>
- Slug: <code>visitor3</code>
- High-tide pack: <code>VISITOR3.FG2</code>
- Low-tide pack: <code>VIST3LOW.FG2</code>

## What this scene is

A large red ship/yacht crashes through the scene while Johnny reacts in the
water. The original host renderer reveals the ship as moving slices, but the
visible scene expects those slices to accumulate into a full hull before the
cleanup frames remove it.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

## Validation notes

VISITOR 3 is not a plain base-diff scene. The validated high/low packs use the
standard normal, far-left, and far-right foreground-only capture set, then a
VISITOR3-specific synthesis helper builds the foreground source:

- Foreground-only views keep the moving sprite pixels clean.
- Full-host frames contribute the red ship hull only during live crash frames.
- FGP3 temporal cleanup clears the post-crash blank rows instead of holding stale red.
- Hold timing keeps the real source frame-158 splash visible without replaying stale right-side splash residue.

Production island placement remains variable. The capture/test island positions
were evidence-gathering positions, not runtime pins.

## Notable runtime history

`VISITOR 3` is the largest single optimization target left on the
[performance battle card]({{ '/perf/' | relative_url }}) at
`{{ site.release.tag }}`. Both `visitor3` high and `visitor3` low
sit in the **red** band — around `69.4%` target speed after the
`v0.7.2` prefetch-relief refresh. The wide multi-view stitch (the
red ship crossing the full scene width) hits the
[prefetch window]({{ '/docs/glossary/#prefetch-window' | relative_url }})
the hardest of any scene; the timing gap is concentrated in the
ship's live crash window, not the foreground replay around it.

The arc that moved the rest of the matrix from `87.1%` to `99.5%`
target speed is at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).
The named-experiment queue for `visitor3` lives in
[`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md);
recent rejected probes include several read-group, slack-gated, and
setup-prime variants that didn't beat the canary.
