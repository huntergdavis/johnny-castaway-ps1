---
layout: page
title: "Memory region allocator — red team"
eyebrow: Source library
subtitle: "docs/ps1/memory-region-allocator-red-team.md"
description: "Generated source-library page for docs/ps1/memory-region-allocator-red-team.md"
---

This is the website shelf page for [`docs/ps1/memory-region-allocator-red-team.md`]({{ site.github_url }}/blob/main/docs/ps1/memory-region-allocator-red-team.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Companion to memory-region-allocator-plan.md.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/memory-region-allocator-red-team.md` |
| Lines | 360 |
| Approx. words | 2352 |
| Code fences | 1 |

## Outline

- Memory region allocator — red team
-   🔴 1. PERM cannot hold the grow-only frame buffers
-   🔴 2. Mid-play clean-rect allocation has no fallback under SCENE bump
-   🔴 3. The budget arithmetic doesn't close
-   🟠 4. PSX usable-RAM assumption is unverified
-   🟠 5. Default-to-PERM in migration is unsafe
-   🟠 6. PC stub has unlimited malloc — PS1-only failures won't surface on PC
-   🟠 7. grNewLayer() is already a pool — the plan mis-classified ttmLayer
-   🟠 8. safemalloc(640 480) for grBackgroundSfc is a 300 KB allocation we didn't account for
-   🟠 9. Resource-struct PERM growth during boot is more involved than "one-shot at boot"
-   🟠 10. RES eviction → re-alloc feedback loop can spin or deadlock
-   🟡 11. Alignment is not in the design
-   🟡 12. "Pin-count" terminology collides with LRU's pinCount
-   🟡 13. Telemetry log volume

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/memory-region-allocator-red-team.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
