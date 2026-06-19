---
layout: page
title: "PS1 CACHE memory fuzzer & simulator"
eyebrow: Source library
subtitle: "docs/ps1/mem-fuzz-sim.md"
description: "Generated source-library page for docs/ps1/mem-fuzz-sim.md"
---

This is the website shelf page for [`docs/ps1/mem-fuzz-sim.md`]({{ site.github_url }}/blob/main/docs/ps1/mem-fuzz-sim.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

A host-side tool that links the real PS1 CACHE allocator (src/memregion.c) and drives it with the PS1's actual memory model so we can reproduce and prove out memory-exhaustion BSODs in seconds instead of multi-hour on-hardware/emulator soaks.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/mem-fuzz-sim.md` |
| Lines | 199 |
| Approx. words | 1507 |
| Code fences | 1 |

## Outline

- PS1 CACHE memory fuzzer & simulator
-   Why it works
-   Components
-   Usage
-   Findings to date (2026-06-14)
-   Validation against real soak telemetry (2026-06-14)
-   Scene-sequence simulator (tests/memsim.c)
-   Per-scene demand from real FG2 packs (2026-06-14)
-   Red-team: would this have caught every failure we saw? (2026-06-14)
-     What the first red-team pass missed (and the fix)
-   Fidelity status & roadmap

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/mem-fuzz-sim.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
