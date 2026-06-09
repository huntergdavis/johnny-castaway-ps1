---
layout: page
title: "Memory region allocator — red team v4 (panel re-review of v5)"
eyebrow: Source library
subtitle: "docs/ps1/memory-region-allocator-red-team-v4.md"
description: "Generated source-library page for docs/ps1/memory-region-allocator-red-team-v4.md"
---

This is the website shelf page for [`docs/ps1/memory-region-allocator-red-team-v4.md`]({{ site.github_url }}/blob/main/docs/ps1/memory-region-allocator-red-team-v4.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Same five-reviewer panel reconvenes for the third pass. v5 closed 19 of v3's 22 graded findings; this review focuses on new issues introduced by the v5 fixes themselves. Loop 2 of the goal-driven iteration.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/memory-region-allocator-red-team-v4.md` |
| Lines | 404 |
| Approx. words | 2302 |
| Code fences | 2 |

## Outline

- Memory region allocator — red team v4 (panel re-review of v5)
-   Reviewer 1 — Pat, Embedded Systems Veteran
-     Closed in v5
-     New concerns
-       🔴 P13. assert(ps1IsMainContext()) is NOT unconditional
-       🟠 P14. MIPS COP0 read latency for ps1IsMainContext is understated
-       🟠 P15. CRC-32 table is 1 KB — accounted in BOOT budget?
-       🟡 P16. The builtinunreachable() after memHalt branches
-   Reviewer 2 — Sarah, SRE / Operations
-     Closed in v5
-     New concerns
-       🟠 S11. Per-commit CI may fail on intermediate states of the bundled PR
-       🟠 S12. The decision tree is documentation, not enforcement
-       🟡 S13. Tag in commit-bundle vs branch tag

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/memory-region-allocator-red-team-v4.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
