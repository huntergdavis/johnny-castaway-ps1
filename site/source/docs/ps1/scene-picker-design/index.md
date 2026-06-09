---
layout: page
title: "Scene Picker Algorithms — Research + Design (v2)"
eyebrow: Source library
subtitle: "docs/ps1/scene-picker-design.md"
description: "Generated source-library page for docs/ps1/scene-picker-design.md"
---

This is the website shelf page for [`docs/ps1/scene-picker-design.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-picker-design.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

fgLoopNextScene(explicitScene, sceneSetIdx) was a three-way fallback:

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/scene-picker-design.md` |
| Lines | 503 |
| Approx. words | 2936 |
| Code fences | 4 |

## Outline

- Scene Picker Algorithms — Research + Design (v2)
-   What we had before this work
-   Goal
-   Memory budget — the load-bearing constraint
-     Original algorithm static-budget analysis
-     State to lift into module statics
-     Stack-frame check during scene play
-     Sequential mode is even cheaper
-     Build size
-   The four mechanics that have to be ported (Original)
-     M1. Flag-based filtering with day-of-story
-     M2. Sequence structure
-     M3. Walk-aware retry
-     M4. FIRST gating

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/scene-picker-design.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
