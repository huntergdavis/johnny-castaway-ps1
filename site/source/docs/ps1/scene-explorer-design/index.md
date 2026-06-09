---
layout: page
title: "Scene Explorer — design doc"
eyebrow: Source library
subtitle: "docs/ps1/scene-explorer-design.md"
description: "Generated source-library page for docs/ps1/scene-explorer-design.md"
---

This is the website shelf page for [`docs/ps1/scene-explorer-design.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-explorer-design.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

A new pause-menu sub-screen that lets the player browse all 63 scenes in the catalog with a full-screen thumbnail, title, family, frame count, and validation status. Cross plays the highlighted scene immediately (via the same scene-pin path the freeplay rotation already uses).

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/scene-explorer-design.md` |
| Lines | 379 |
| Approx. words | 2252 |
| Code fences | 5 |

## Outline

- Scene Explorer — design doc
-   UX
-   Thumbnail format
-   Asset shape — one PSB per scene, not a single atlas
-   Streaming + prefetch
-   Implementation phases
-     Phase 1 — Metadata pipeline
-     Phase 2 — Thumbnail extraction
-     Phase 3 — Asset packaging
-     Phase 4 — Menu state machine
-     Phase 5 — Scene-pin playback
-     Phase 6 — Docs + site
-   Red Team Review
-     R1. Atlas vs per-scene PSB (RESOLVED → per-scene)

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/scene-explorer-design.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
