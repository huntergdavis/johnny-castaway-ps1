---
layout: page
title: "BUILDING-6 regression reference"
eyebrow: Host regression reference
subtitle: "BUILDING.ADS tag 6"
description: "Regtest reference case for BUILDING-6."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/BUILDING-6`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-6).

## What this case captures

`BUILDING-6` is the host-side reference for `BUILDING.ADS` tag `6`.
The host runner booted the scene with:

```text
window nosound story single 16 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `BUILDING` |
| Tag | `6` |
| Scene index | `16` |
| Status | `verified` |
| Capture date | `2026-03-28T15:28:38Z` |
| Frames captured | `514` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
f891bf8d2e4b2baee64f425ab8d5a6daeedbd1a2ffa42a66e015d54892e0b85b
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-6/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-6/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-6/review.html)

## Cross-links

- [Live scene page: `BUILDING 6`]({{ '/scenes/building6/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
