---
layout: page
title: "v0.9.7-ps1 — Final Pre-Release Red-Team Pass"
eyebrow: Source library
subtitle: "docs/ps1/release-0.9.7-redteam.md"
description: "Generated source-library page for docs/ps1/release-0.9.7-redteam.md"
---

This is the website shelf page for [`docs/ps1/release-0.9.7-redteam.md`]({{ site.github_url }}/blob/main/docs/ps1/release-0.9.7-redteam.md).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**Active PS1 reference.** This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.

The disc image, code, and stability are sound (24h/12,512-scene zero-crash soak; clean adversarial code review; fuzz gate green; no visual regressions; clean debug hygiene). But the public website's Download buttons 404 — the site still points at the old jcreborn. asset names while the release ships johnnycastawayps1.. The fix is website-only (no ISO rebuild, no re-tag).

## File facts

| Field | Value |
|---|---:|
| Source path | `docs/ps1/release-0.9.7-redteam.md` |
| Lines | 48 |
| Approx. words | 853 |
| Code fences | 0 |

## Outline

- v0.9.7-ps1 — Final Pre-Release Red-Team Pass
-   Verdict: ⛔ NOT ship-ready as-is — one P0 blocks distribution
-   Findings (confirmed, by severity)
-   Notable downgrades / refutations (adversarial verify earned its keep)
-   Clean dimensions (no findings)
-   P3 / follow-ups (non-blocking)
-   Recommended actions, in order

## Read it in context

- [Open the source file on GitHub]({{ site.github_url }}/blob/main/docs/ps1/release-0.9.7-redteam.md)
- [Back to the source library index]({{ '/source/' | relative_url }})
- [Main docs index]({{ '/docs/' | relative_url }})
