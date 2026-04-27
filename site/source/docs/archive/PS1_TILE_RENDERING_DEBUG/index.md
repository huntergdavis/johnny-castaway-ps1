---
layout: page
title: "PS1 Sprite Tiling Debug Log"
eyebrow: Source library
subtitle: "docs/archive/PS1_TILE_RENDERING_DEBUG.md"
description: "Generated source-library page for docs/archive/PS1_TILE_RENDERING_DEBUG.md"
---

This is the website shelf page for [`docs/archive/PS1_TILE_RENDERING_DEBUG.md`]({{ site.github_url }}/blob/main/docs/archive/PS1_TILE_RENDERING_DEBUG.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Historical archive.** This is older project documentation preserved for context. It is not the current source of truth, but it explains the roads that were tried.

JOHNWALK.BMP sprites are 64x78 pixels - 14 pixels taller than the PS1's 64-pixel texture limit. Johnny's feet are cut off because only the top 64 rows are loaded/rendered.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/archive/PS1_TILE_RENDERING_DEBUG.md` |
| Lines | 2114 |
| Approx. words | 10725 |
| Code fences | 37 |

## Outline

- PS1 Sprite Tiling Debug Log
-   Problem Statement
-   CURRENT STATUS (2025-12-28 - Session 7)
-     BREAKTHROUGH: Incremental Loading Works!
-     Key Fix: Static Animation Variables
-     Tested Approaches
-     Current Implementation
-   Session 4: Memory Investigation (2025-12-28)
-     Critical Discovery: Each Frame Needs UNIQUE Buffer Address
-     Test Results Table
-     The Key Insight
-     Hypotheses
-     Current Approach: Ring Buffer
-   Previous Status (2025-12-27)

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/archive/PS1_TILE_RENDERING_DEBUG.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
