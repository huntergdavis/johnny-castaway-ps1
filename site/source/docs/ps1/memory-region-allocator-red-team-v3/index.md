---
layout: page
title: "Memory region allocator — red team v3 (panel re-review of v4)"
eyebrow: Source library
subtitle: "docs/ps1/memory-region-allocator-red-team-v3.md"
description: "Generated source-library page for docs/ps1/memory-region-allocator-red-team-v3.md"
---

This is the website shelf page for [`docs/ps1/memory-region-allocator-red-team-v3.md`]({{ site.github_url }}/blob/main/docs/ps1/memory-region-allocator-red-team-v3.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Companion to memory-region-allocator-plan.md (v4). Same five-reviewer panel from memory-region-allocator-red-team-v2.md reconvenes. Each reviewer re-evaluates whether their prior concerns are properly resolved and flags new issues introduced by the v3→v4 revisions (especially the JCBSOD integration, which is fresh).

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/memory-region-allocator-red-team-v3.md` |
| Lines | 483 |
| Approx. words | 3197 |
| Code fences | 1 |

## Outline

- Memory region allocator — red team v3 (panel re-review of v4)
-   Reviewer 1 — Pat, Embedded Systems Veteran
-     Closed in v4
-     Reopened / new
-       🔴 P1-bis. ISR enforcement is debug-only — re-classified as blocker
-       🔴 P8. ps1Bsod reads stale heap diagnostics that no longer mean anything
-       🟠 P9. ps1Bsod is noreturn and assumes graphics is up
-       🟠 P10. PSX printf does not allocate (verify, document)
-       🟡 P11. Watchdog / emulator timeout while in BSOD halt
-       🟡 P12. CRC-32 vs SHA-256
-   Reviewer 2 — Sarah, SRE / Operations
-     Closed in v4
-     Reopened / new
-       🔴 S7. Per-file PR sequencing across Phase 2 is unsafe as written

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/memory-region-allocator-red-team-v3.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
