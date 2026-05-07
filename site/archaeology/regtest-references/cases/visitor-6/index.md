---
layout: page
title: "VISITOR-6 regression reference"
eyebrow: Host regression reference
subtitle: "VISITOR.ADS tag 6"
description: "Regtest reference case for VISITOR-6."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/VISITOR-6`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-6).

## What this case captures

`VISITOR-6` is the host-side reference for `VISITOR.ADS` tag `6`.
The host runner booted the scene with:

```text
window nosound story single 57 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `VISITOR` |
| Tag | `6` |
| Scene index | `57` |
| Status | `verified` |
| Capture date | `2026-03-28T15:46:38Z` |
| Frames captured | `286` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
3fdd72346bf180a4d5d78c1d5664241890c292582833e3d41936303be75e07c1
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-6/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-6/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-6/review.html)

## Cross-links

- [Live scene page: `VISITOR 6`]({{ '/scenes/visitor6/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
