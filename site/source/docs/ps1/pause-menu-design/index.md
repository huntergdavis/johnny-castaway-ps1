---
layout: page
title: "Pause Menu — Design"
eyebrow: Source library
subtitle: "docs/ps1/pause-menu-design.md"
description: "Generated source-library page for docs/ps1/pause-menu-design.md"
---

This is the website shelf page for [`docs/ps1/pause-menu-design.md`]({{ site.github_url }}/blob/main/docs/ps1/pause-menu-design.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Two compounding findings from the audit:

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/pause-menu-design.md` |
| Lines | 372 |
| Approx. words | 2512 |
| Code fences | 3 |

## Outline

- Pause Menu — Design
-   1. Why the current pause never worked
-   2. Decisions locked
-   3. Final menu structure
-   4. Debug Info sub-screen — single dense page
-     Data sources
-   5. Render pipeline (locked)
-   6. Visual approach — why POLYF4 not pixel-modify
-     Bug A: doesn't mark dirty
-     Bug B: no clean copy in rect-mode
-     Why POLYF4 wins
-   7. Source-level red-team — preflight blockers
-     Blocker 1: pausemenu.c not in CMakeLists.txt
-     Blocker 2: three undefined externs

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/pause-menu-design.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
