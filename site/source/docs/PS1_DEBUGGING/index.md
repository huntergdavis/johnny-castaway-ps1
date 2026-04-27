---
layout: page
title: "PS1 Port Debugging Guide"
eyebrow: Source library
subtitle: "docs/PS1_DEBUGGING.md"
description: "Generated source-library page for docs/PS1_DEBUGGING.md"
---

This is the website shelf page for [`docs/PS1_DEBUGGING.md`]({{ site.github_url }}/blob/main/docs/PS1_DEBUGGING.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Repository documentation.** This is part of the repository's documentation corpus. The generated page exists so it has a stable place in the public site index.

This document captures lessons learned while debugging the PlayStation 1 port of Johnny Reborn.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/PS1_DEBUGGING.md` |
| Lines | 278 |
| Approx. words | 1346 |
| Code fences | 10 |

## Outline

- PS1 Port Debugging Guide
-   Memory Issues
-     Sprite Heap Fragmentation / Missing Body Parts (FIXED)
-     Uninitialized nextTile Pointers (FIXED)
-     Sprite Pointer Array Initialization (FIXED)
-   Rendering Issues
-     Black Flashing / Tearing (PARTIALLY FIXED)
-     Black Bar at Bottom of Screen (FIXED)
-     Background Not Showing After adsInitIsland() (KNOWN ISSUE)
-     adsInitIsland() Hangs (UNRESOLVED)
-     Missing Sprites During Animation (UNRESOLVED)
-   A/B Evidence Log
-     2026-03-04 - Track A (in-scene multi-sprite dropout)
-     Persistent Drop Overlay (NEW)

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/PS1_DEBUGGING.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
