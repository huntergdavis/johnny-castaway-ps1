---
layout: page
title: "PS1 Triangle Rendering - Key Findings"
eyebrow: Source library
subtitle: "docs/archive/PS1_TRIANGLE_FINDINGS.md"
description: "Generated source-library page for docs/archive/PS1_TRIANGLE_FINDINGS.md"
---

This is the website shelf page for [`docs/archive/PS1_TRIANGLE_FINDINGS.md`]({{ site.github_url }}/blob/main/docs/archive/PS1_TRIANGLE_FINDINGS.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Historical archive.** This is older project documentation preserved for context. It is not the current source of truth, but it explains the roads that were tried.

1. VSync(0) - wait for vertical blank 2. ClearOTagR(ot, OTLEN) - clear ordering table 3. Reset primitive buffer pointer: nextpri = primbuff 4. Allocate and setup primitives from buffer 5. Add primitives to ordering table: addPrim(ot, prim) 6. PutDrawEnv(&draw) - FIRST (clears background if isbg=1) 7. DrawOTag(ot+OTLEN-1) - SECOND (draws primitives)

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/archive/PS1_TRIANGLE_FINDINGS.md` |
| Lines | 121 |
| Approx. words | 463 |
| Code fences | 3 |

## Outline

- PS1 Triangle Rendering - Key Findings
-   What Works ✅
-     POLYF3 Triangles
-   What Doesn't Work ❌
-     TILE Rectangles
-     POLYF4 Quads
-   Build Process Issues 🔨
-     Critical Steps for Rebuild:
-   PS1 Rendering Pipeline 🎨
-     Correct Order:
-     Background Clearing:
-   Verified Working Test 🎯
-   Next Steps 📋
-   Resources 📚

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/archive/PS1_TRIANGLE_FINDINGS.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
