---
layout: page
title: "Walk-Connected Story Loop — Implementation Plan"
eyebrow: Source library
subtitle: "docs/ps1/walk-implementation-plan.md"
description: "Generated source-library page for docs/ps1/walk-implementation-plan.md"
---

This is the website shelf page for [`docs/ps1/walk-implementation-plan.md`]({{ site.github_url }}/blob/main/docs/ps1/walk-implementation-plan.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Date: 2026-04-29 Status: implemented in v0.4.20-ps1; freeplay-specific phases remain future work Owner: PS1 perf branch

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/walk-implementation-plan.md` |
| Lines | 1192 |
| Approx. words | 7819 |
| Code fences | 18 |

## Outline

- Walk-Connected Story Loop — Implementation Plan
-   Executive Summary
-   Release 0.4.20 Implementation Notes
-   1. Background — how the original engine does it
-     1.1 Scene metadata is pre-declared
-     1.2 The walking system is a self-contained module
-     1.3 The story loop chains scenes together
-   2. Current PS1 state
-   3. Architecture choice — port walk.c only
-   3.5 Cross-cutting architecture (shared with freeplay)
-     3.5.1 Two drivers, one render kernel
-     3.5.2 Shared position type
-     3.5.3 Spot ↔ coord mapping (extracted from walkdata.h)
-     3.5.4 Per-mode ownership

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/walk-implementation-plan.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
