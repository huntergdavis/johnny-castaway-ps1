---
layout: page
title: "PS1 Port — Current Status"
eyebrow: Source library
subtitle: "docs/ps1/current-status.md"
description: "Generated source-library page for docs/ps1/current-status.md"
---

This is the website shelf page for [`docs/ps1/current-status.md`]({{ site.github_url }}/blob/main/docs/ps1/current-status.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

validated on the existing on-disc packs (no rework — high-tide nighttime route, belly-flop dive). Earlier this push: WALKSTUF 1 and WALKSTUF 2 were validated. WALKSTUF1 uses re-exported high/low packs with -500/+300 stitch positions and a range-gated Johnny-bbox hold (frames 63-165, glitch threshold 1000) so Johnny stays drawn in his rest position while the boat + mermaid scene plays out and the foreground-only diff drops him. WALKSTUF2 signed off on th...

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/current-status.md` |
| Lines | 447 |
| Approx. words | 4705 |
| Code fences | 1 |

## Outline

- PS1 Port — Current Status
-   Overall
-   Scenes: 46 / 63 fully validated
-   Primary render methodology: hybrid scene playback (fgpilot)
-     Pipeline
-     Acceptance model
-   Audio
-   Historical status numbers (not current)
-   Build size
-   Known limitations
-   See also

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/current-status.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
