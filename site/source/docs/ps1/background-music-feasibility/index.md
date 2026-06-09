---
layout: page
title: "Ocean Ambience — PS1 Implementation Plan"
eyebrow: Source library
subtitle: "docs/ps1/background-music-feasibility.md"
description: "Generated source-library page for docs/ps1/background-music-feasibility.md"
---

This is the website shelf page for [`docs/ps1/background-music-feasibility.md`]({{ site.github_url }}/blob/main/docs/ps1/background-music-feasibility.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Date: 2026-05-01 Branch context: ocean-ambience-20260501, shipped as v0.6.0-ps1. Status: shipped. Asset, boot loader, pause-menu toggle, memcard persistence, and CD-image embedding are all live.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/background-music-feasibility.md` |
| Lines | 491 |
| Approx. words | 2923 |
| Code fences | 8 |

## Outline

- Ocean Ambience — PS1 Implementation Plan
-   Shipped state (v0.6.0-ps1)
-   Goal
-     What "zero CPU" rules out
-   Mechanism Choice: Pre-loaded ADPCM Loop in SPU RAM
-   Source: BigSoundBank "Sea: Waves" (CC0)
-   Loop Construction: 20-Second Crossfade-Replace Seam
-     How the crossfade-replace works
-     Loop seam audibility
-     Final VAG specs
-   Encoding Pipeline (Reproducible)
-   SPU RAM Map
-   Pause-Menu Toggle: oceanAmbientEnabled
-     Global state

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/background-music-feasibility.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
