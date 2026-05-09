---
layout: scene
title: VISITOR 3 — Waves down what looks like a small boat, but it's huge
ads: VISITOR
tag: 3
slug: visitor3
status: validated
description: "VISITOR.ADS scene 3: Johnny waves down what he thinks is a small boat, but the perspective gag reveals it's actually a huge boat. Validated 2026-05-08."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 3</code>
- Slug: <code>visitor3</code>
- High-tide pack: <code>VISITOR3.FG2</code>
- Low-tide pack: <code>VIST3LOW.FG2</code>

## What this scene is

Johnny spots a boat in the distance and starts waving it down, thinking it's a small one nearby. The perspective gag reveals it's actually a huge boat much further out — and the size flip plays for the laugh. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "yacht couple, photos" caption-mapping guess was wrong (no couple or photo-taking in the on-PS1 pack).

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

`VISITOR 3` remains one of the high-leverage yellow-band rows on the
[performance battle card]({{ '/perf/' | relative_url }}) at
`{{ site.release.tag }}`. After the FGP3/v4 compact metadata work, the
pack-side restore-minus-current cleanup, offscreen clips, code-shape pass, and
v4 draw-tail stage guard, `visitor3` high and low now run around
[`91.9%` and `91.0%` target speed]({{ '/docs/glossary/#target-speed' | relative_url }})
instead of sitting in the red band. The wide multi-view stitch (the red ship
crossing the full scene width) hits the
[prefetch window]({{ '/docs/glossary/#prefetch-window' | relative_url }})
harder than most scenes; the remaining timing gap is concentrated in the
ship's live crash window and same-frame cleanup/restore work.

The arc that moved the rest of the matrix from `87.1%` to `99.5%`
target speed is at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).
The named-experiment queue for `visitor3` lives in
[`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md);
recent rejected probes include several read-group, slack-gated, and
setup-prime variants that didn't beat the canary. The current `97..109`
read-plan cluster is closed for local runtime changes too: both grouped-read
and setup-owned persistent-segment probes measured exact-flat, so the next
useful lane needs generated scheduler ownership or a real payload/data-shape
change. A data-only sector-alignment probe also failed: it reduced modeled
uncovered sectors but shifted CD phase and regressed both tide variants.
