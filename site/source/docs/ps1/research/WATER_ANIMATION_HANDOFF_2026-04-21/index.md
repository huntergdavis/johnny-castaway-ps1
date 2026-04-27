---
layout: page
title: "PS1 Fishing 1 Water Animation Handoff"
eyebrow: Source library
subtitle: "docs/ps1/research/WATER_ANIMATION_HANDOFF_2026-04-21.md"
description: "Generated source-library page for docs/ps1/research/WATER_ANIMATION_HANDOFF_2026-04-21.md"
---

This is the website shelf page for [`docs/ps1/research/WATER_ANIMATION_HANDOFF_2026-04-21.md`]({{ site.github_url }}/blob/main/docs/ps1/research/WATER_ANIMATION_HANDOFF_2026-04-21.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**PS1 research worklog.** This is one of the working notes that drove the port: plans, validation logs, classifier notes, and the pivots that eventually shaped the runtime.

Date: 2026-04-21 Repo: workspace:/jcreborn Branch: ps1

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/research/WATER_ANIMATION_HANDOFF_2026-04-21.md` |
| Lines | 326 |
| Approx. words | 1202 |
| Code fences | 0 |

## Outline

- PS1 Fishing 1 Water Animation Handoff
-   Goal
-   Known Good Baseline
-   What Has Been Proven
-     1. The wave primitive itself works on PS1
-     2. The Fishing 1 integrated failure is not a host-data absence issue
-     3. Water failure is integration/runtime-side
-   Current Technical Findings
-     grLoadBmp() is RAM-path only on PS1
-     Integrated main loop shape
-     Foreground pack path uses direct compositing
-     Some direct helper paths clear currDirty
-   What Has Been Tried And Failed
-     High-memory backdrop frame streams

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/research/WATER_ANIMATION_HANDOFF_2026-04-21.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
