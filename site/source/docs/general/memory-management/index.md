---
layout: page
title: "Johnny Reborn - Memory Management"
eyebrow: Source library
subtitle: "docs/general/memory-management.md"
description: "Generated source-library page for docs/general/memory-management.md"
---

This is the website shelf page for [`docs/general/memory-management.md`]({{ site.github_url }}/blob/main/docs/general/memory-management.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**General project reference.** This is shared documentation: architecture, build setup, emulator use, memory management, and testing guidance that apply beyond one PS1 subsystem.

This document explains the memory management strategy used in Johnny Reborn, particularly for embedded systems with limited RAM.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/general/memory-management.md` |
| Lines | 228 |
| Approx. words | 704 |
| Code fences | 11 |

## Outline

- Johnny Reborn - Memory Management
-   Overview
-   Memory Budget
-   Lazy Loading Strategy
-   LRU Cache with Pinning
-   Memory Profiling Insights
-     Fixed Overhead
-     Typical Scene (400-600KB)
-     Peak Usage
-     Main Consumers
-   Optimization Techniques
-     1. Release After Conversion
-     2. Static vs Dynamic Allocation
-     3. Structure Packing

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/general/memory-management.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
