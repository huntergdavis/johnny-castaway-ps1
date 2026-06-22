---
layout: page
title: "Johnny Castaway PS1 — v0.9.6"
eyebrow: Source library
subtitle: "docs/ps1/release-notes-0.9.6.md"
description: "Generated source-library page for docs/ps1/release-notes-0.9.6.md"
---

This is the website shelf page for [`docs/ps1/release-notes-0.9.6.md`]({{ site.github_url }}/blob/main/docs/ps1/release-notes-0.9.6.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

A deep-soak stability release. Where 0.9.5 fixed the first wave of CACHE exhaustion BSODs, 0.9.6 closes the remaining deep-soak failure classes — validated by a ~15-hour continuous soak (7,800+ scenes) with zero crashes and zero hangs, during which the recovery machinery fired ~65 times (50 memory withhold-rebuilds, 15 graceful clean-rect declines) and recovered every time.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/release-notes-0.9.6.md` |
| Lines | 42 |
| Approx. words | 284 |
| Code fences | 0 |

## Outline

- Johnny Castaway PS1 — v0.9.6
-   Fixes
-     Memory: the whole best-effort CACHE-alloc class is now graceful
-     CD-ROM: recover instead of freezing
-     Diagnostics
-   Notes

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/release-notes-0.9.6.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
