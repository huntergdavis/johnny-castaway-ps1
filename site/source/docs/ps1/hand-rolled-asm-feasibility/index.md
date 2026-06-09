---
layout: page
title: "Hand-Rolled MIPS Assembly — Feasibility Research"
eyebrow: Source library
subtitle: "docs/ps1/hand-rolled-asm-feasibility.md"
description: "Generated source-library page for docs/ps1/hand-rolled-asm-feasibility.md"
---

This is the website shelf page for [`docs/ps1/hand-rolled-asm-feasibility.md`]({{ site.github_url }}/blob/main/docs/ps1/hand-rolled-asm-feasibility.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Date: 2026-04-30 Branch context: walk-implementation-20260429 (read-only research; no code changes) Author: research pass for the active perf iteration.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/hand-rolled-asm-feasibility.md` |
| Lines | 298 |
| Approx. words | 2075 |
| Code fences | 1 |

## Outline

- Hand-Rolled MIPS Assembly — Feasibility Research
-   Question
-   Hardware Refresh
-   Toolchain and Inline Asm Support
-   Hot Path Inventory
-   Where Asm Actually Helps
-     A. Inner pixel loops (real win, modest absolute size)
-     B. Per-row memcpy in clean-rect restore (real win, hot)
-     C. Palette LUT in scratchpad (compose only)
-   Where Asm Doesn't Help (or is the wrong tool)
-   Cheaper Alternatives To Try First
-   Risk and Maintenance Cost
-   Recommended Plan If We Pursue Asm
-   Concrete Targets and Estimated Wins

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/hand-rolled-asm-feasibility.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
