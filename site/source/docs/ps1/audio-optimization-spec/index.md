---
layout: page
title: "PS1 Audio Implementation Spec"
eyebrow: Source library
subtitle: "docs/ps1/audio-optimization-spec.md"
description: "Generated source-library page for docs/ps1/audio-optimization-spec.md"
---

This is the website shelf page for [`docs/ps1/audio-optimization-spec.md`]({{ site.github_url }}/blob/main/docs/ps1/audio-optimization-spec.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

All 23 original WAV files share identical format:

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/audio-optimization-spec.md` |
| Lines | 505 |
| Approx. words | 2925 |
| Code fences | 13 |

## Outline

- PS1 Audio Implementation Spec
-   1. Sound Inventory
-     1.1 Source Format
-     1.2 Complete Sound Table
-     1.3 Key Observations
-   2. Current PS1 Implementation Status
-   3. SPU RAM Budget
-     3.1 SPU RAM Layout
-     3.2 Current ADPCM Sizes (from existing VAG files)
-     3.3 After Optimizations
-     3.4 Verdict: Sample Rate Reduction Not Needed
-   4. Loading Strategy: Full Preload at Startup
-     4.1 Recommendation: Preload All Sounds
-     4.2 Main RAM Impact During Loading

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/audio-optimization-spec.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
