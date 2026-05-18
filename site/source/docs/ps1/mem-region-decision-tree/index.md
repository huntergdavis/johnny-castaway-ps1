---
layout: page
title: "Memory region decision tree"
eyebrow: Source library
subtitle: "docs/ps1/mem-region-decision-tree.md"
description: "Generated source-library page for docs/ps1/mem-region-decision-tree.md"
---

This is the website shelf page for [`docs/ps1/mem-region-decision-tree.md`]({{ site.github_url }}/blob/main/docs/ps1/mem-region-decision-tree.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Companion to memory-region-allocator-plan.md.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/mem-region-decision-tree.md` |
| Lines | 162 |
| Approx. words | 867 |
| Code fences | 4 |

## Outline

- Memory region decision tree
-   Common cases worked through
-     "I need a 4 KB decompression scratch in my new scene loader"
-     "My new audio effect needs a 16 KB ring buffer for its lifetime"
-     "I need to cache a procedurally-generated palette across scenes"
-     "My new TTM opcode needs a 256 B temp during opcode dispatch"
-   Holiday variants
-   Prohibitions
-   Process notes (M16)
-   When none of these fit
-   Sanity check before merging

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/mem-region-decision-tree.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
