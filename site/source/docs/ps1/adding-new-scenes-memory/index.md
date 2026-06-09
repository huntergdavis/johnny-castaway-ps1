---
layout: page
title: "Adding a new scene or holiday variant — memory checklist"
eyebrow: Source library
subtitle: "docs/ps1/adding-new-scenes-memory.md"
description: "Generated source-library page for docs/ps1/adding-new-scenes-memory.md"
---

This is the website shelf page for [`docs/ps1/adding-new-scenes-memory.md`]({{ site.github_url }}/blob/main/docs/ps1/adding-new-scenes-memory.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Companion to memory-region-allocator-plan.md. Read this before merging a PR that:

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/adding-new-scenes-memory.md` |
| Lines | 180 |
| Approx. words | 1064 |
| Code fences | 3 |

## Outline

- Adding a new scene or holiday variant — memory checklist
-   Why this matters
-   The checklist
-   Troubleshooting
-     Staticassert("MEMREGIONTOTAL too large") fails
-     fatalError("BOOT region budget exceeded") at boot
-     fatalError("TRANSIENT region budget exceeded scene=X") at boot
-     fatalError("CACHE pinned working set too big scene=X") at boot
-   Using MEMDEVBUILD for iteration
-   Adding a new region (rare, but here's the pattern)
-   When in doubt

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/adding-new-scenes-memory.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
