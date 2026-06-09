---
layout: page
title: "Memory region allocator — implementation plan"
eyebrow: Source library
subtitle: "docs/ps1/memory-region-allocator-plan.md"
description: "Generated source-library page for docs/ps1/memory-region-allocator-plan.md"
---

This is the website shelf page for [`docs/ps1/memory-region-allocator-plan.md`]({{ site.github_url }}/blob/main/docs/ps1/memory-region-allocator-plan.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

re-review of v4). See:

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/memory-region-allocator-plan.md` |
| Lines | 1239 |
| Approx. words | 7739 |
| Code fences | 15 |

## Outline

- Memory region allocator — implementation plan
-   What changed in v8
-   What changed in v7
-   What changed in v6
-   What changed in v5
-   What changed in v4
-   Core invariant
-   Context
-   Goals / non-goals
-   Architectural decision: regions vs. per-type pools
-   Concurrency model
-   Memory budget — verified from artifacts
-     PSX user-RAM math (from build-ps1/jcreborn.map)
-     Region splits (verified against sceneanalysisoutput2026-03-21.json)

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/memory-region-allocator-plan.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
