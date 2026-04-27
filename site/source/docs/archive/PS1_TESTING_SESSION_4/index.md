---
layout: page
title: "PS1 Testing Session 4 - DuckStation Emulator Testing"
eyebrow: Source library
subtitle: "docs/archive/PS1_TESTING_SESSION_4.md"
description: "Generated source-library page for docs/archive/PS1_TESTING_SESSION_4.md"
---

This is the website shelf page for [`docs/archive/PS1_TESTING_SESSION_4.md`]({{ site.github_url }}/blob/main/docs/archive/PS1_TESTING_SESSION_4.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Historical archive.** This is older project documentation preserved for context. It is not the current source of truth, but it explains the roads that were tried.

First successful boot in DuckStation emulator! The executable loads from CD-ROM and runs, but console debug output is missing and graphics are not rendering yet. Root cause identified: missing heap initialization.

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/archive/PS1_TESTING_SESSION_4.md` |
| Lines | 263 |
| Approx. words | 934 |
| Code fences | 10 |

## Outline

- PS1 Testing Session 4 - DuckStation Emulator Testing
-   Executive Summary
-   Test Environment
-   Test Results
-     ✅ Working Components
-     ❌ Issues Identified
-   DuckStation Log Analysis
-     Key Log Entries
-   Code Changes Made
-     jcreborn.c - PS1 Initialization
-   Technical Insights
-     PSn00bSDK Initialization Order
-     ResetGraph Modes
-     Direct EXE vs CD Image Testing

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/archive/PS1_TESTING_SESSION_4.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
