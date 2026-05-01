---
layout: page
title: "Ocean Ambience — PS1 Implementation Plan"
eyebrow: Source library
subtitle: "docs/ps1/background-music-feasibility.md"
description: "Generated source-library page for docs/ps1/background-music-feasibility.md"
---

This is the website shelf page for [`docs/ps1/background-music-feasibility.md`]({{ site.github_url }}/blob/main/docs/ps1/background-music-feasibility.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

Date: 2026-05-01 Branch context: read-only research; no code changes. Author: research pass for the active perf iteration.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/background-music-feasibility.md` |
| Lines | 560 |
| Approx. words | 3535 |
| Code fences | 4 |

## Outline

- Ocean Ambience — PS1 Implementation Plan
-   Goal
-     What "zero CPU" rules out
-   Why Ocean Ambience Is a Special Case
-   Mechanism Choice: Pre-loaded ADPCM Loop in SPU RAM
-   Public-Domain Audio Sourcing
-     Verified CC0 Candidates (researched 2026-05-01)
-       Primary recommendation: amholma — "Gentle Waves - Quiet Beach"
-       Backup: INNORECORDS — "Zen Ocean Waves, Ocean Waves Ambience"
-       Notes on candidates we considered and didn't pick
-   Sizing the Loop
-   Authoring Pipeline
-   Pause-Menu Toggle: oceanAmbientEnabled
-     Global state

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/background-music-feasibility.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
