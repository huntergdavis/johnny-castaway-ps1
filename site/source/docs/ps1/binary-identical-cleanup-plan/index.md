---
layout: page
title: "Binary-Identical Cleanup Plan"
eyebrow: Source library
subtitle: "docs/ps1/binary-identical-cleanup-plan.md"
description: "Generated source-library page for docs/ps1/binary-identical-cleanup-plan.md"
---

This is the website shelf page for [`docs/ps1/binary-identical-cleanup-plan.md`]({{ site.github_url }}/blob/main/docs/ps1/binary-identical-cleanup-plan.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Can we reorder functions, split files, clean comments, and improve source layout while using the PS1 executable bytes as the safety gate?

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/binary-identical-cleanup-plan.md` |
| Lines | 167 |
| Approx. words | 1321 |
| Code fences | 4 |

## Outline

- Binary-Identical Cleanup Plan
-   Question
-   Short Answer
-   Local Experiment
-   Proposed Workflow
-   Suggested Gate
-   Red-Team Pass
-     Issue: Comments and whitespace may still change the binary
-     Issue: DATE and TIME make reproducibility fragile
-     Issue: Moving functions into new files can change optimization behavior
-     Issue: static functions are not stable across file splits
-     Issue: Inlining and local context can change code generation
-     Issue: Section sorting changes code locality
-     Issue: Duplicate names and local symbols can collide in ordering

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/binary-identical-cleanup-plan.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
