---
layout: page
title: "Johnny Castaway PS1 — v0.9.7"
eyebrow: Source library
subtitle: "docs/ps1/release-notes-0.9.7.md"
description: "Generated source-library page for docs/ps1/release-notes-0.9.7.md"
---

This is the website shelf page for [`docs/ps1/release-notes-0.9.7.md`]({{ site.github_url }}/blob/main/docs/ps1/release-notes-0.9.7.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

A visual-confidence + deep-soak release. 0.9.6 closed the memory-exhaustion BSOD classes; 0.9.7 closes the last walk-slab crash class, makes the most demanding scene (visitor3) render correctly even under extreme deep-soak memory pressure, and fixes the remaining scene-set visual nits — validated by a 24-hour continuous soak (12,512 scenes, all 126 variants) with zero crashes and zero hangs, during which the visitor3 full-reset fired 26 times and recove...

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/release-notes-0.9.7.md` |
| Lines | 49 |
| Approx. words | 373 |
| Code fences | 0 |

## Outline

- Johnny Castaway PS1 — v0.9.7
-   Fixes
-     Memory: the inter-scene walk/raft load no longer halts
-     visitor3: pixel-perfect even under extreme deep-soak pressure
-     Visuals
-   Notes

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/release-notes-0.9.7.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
