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

completed the current visual + audible validation sweep. The high/low packs were rebuilt through an Activity9-specific wide stitch (x=-500,y=54, x=-154,y=54, x=500,y=54), then patch-activity9-boat-foreground.py filled clipped BOAT.PSB bow/stern pixels from source at the legacy clip edges, added a narrow overlap band to remove the stitch seam, and carried the last boat draw across metadata-held frames so the late bow no longer flickers).

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/current-status.md` |
| Lines | 483 |
| Approx. words | 5291 |
| Code fences | 1 |

## Outline

- PS1 Port — Current Status
-   Overall
-   Scenes: 63 / 63 fully validated
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
