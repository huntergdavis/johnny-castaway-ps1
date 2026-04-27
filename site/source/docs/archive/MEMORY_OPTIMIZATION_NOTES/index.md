---
layout: page
title: "jcreborn — Memory-lite refactor"
eyebrow: Source library
subtitle: "docs/archive/MEMORY_OPTIMIZATION_NOTES.md"
description: "Generated source-library page for docs/archive/MEMORY_OPTIMIZATION_NOTES.md"
---

This is the website shelf page for [`docs/archive/MEMORY_OPTIMIZATION_NOTES.md`]({{ site.github_url }}/blob/main/docs/archive/MEMORY_OPTIMIZATION_NOTES.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Historical archive.** This is older project documentation preserved for context. It is not the current source of truth, but it explains the roads that were tried.

This branch replaces eager, whole-blob resource decompression with lazy, on-demand streaming + a tiny LRU cache, holding at most a small working set in RAM.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/archive/MEMORY_OPTIMIZATION_NOTES.md` |
| Lines | 81 |
| Approx. words | 540 |
| Code fences | 3 |

## Outline

- jcreborn — Memory-lite refactor
-   What changed (high level)
-   How it works
-   Touch points in code
-   Building
-   Runtime controls
-   Expected footprint
-   Notes / edge cases
-   File list (changed)
-   License

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/archive/MEMORY_OPTIMIZATION_NOTES.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
