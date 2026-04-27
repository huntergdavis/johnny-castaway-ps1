---
layout: page
title: "PS1 Fishing 1 Water Animation — Session Worklog 2026-04-21"
eyebrow: Source library
subtitle: "docs/ps1/research/WATER_ANIMATION_WORKLOG_2026-04-21.md"
description: "Generated source-library page for docs/ps1/research/WATER_ANIMATION_WORKLOG_2026-04-21.md"
---

This is the website shelf page for [`docs/ps1/research/WATER_ANIMATION_WORKLOG_2026-04-21.md`]({{ site.github_url }}/blob/main/docs/ps1/research/WATER_ANIMATION_WORKLOG_2026-04-21.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**PS1 research worklog.** This is one of the working notes that drove the port: plans, validation logs, classifier notes, and the pivots that eventually shaped the runtime.

after adsInitIsland() returns. Every downstream wave draw bails at the numSprites==0 check in grDrawSprite.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/research/WATER_ANIMATION_WORKLOG_2026-04-21.md` |
| Lines | 269 |
| Approx. words | 1883 |
| Code fences | 2 |

## Outline

- PS1 Fishing 1 Water Animation — Session Worklog 2026-04-21
-   Outcome so far
-   Evidence gathered
-   What the probe grid currently tests
-   What's been tried that did NOT work (this session)
-   Recommended next steps
-   Fix candidates that are still on the table
-   Probe infrastructure already in place (keep)
-   Boot-hang root cause (solved)
-   Debugging lessons learned
-   Resolution (landed this session)
-     1. Pre-load BACKGRND.BMP before bg-tile allocation
-     2. Wave-tick parity with the normal ads path
-     3. Rect-based clean backup (option B)

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/research/WATER_ANIMATION_WORKLOG_2026-04-21.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
