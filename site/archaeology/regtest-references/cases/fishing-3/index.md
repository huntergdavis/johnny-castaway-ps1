---
layout: page
title: "FISHING-3 regression reference"
eyebrow: Host regression reference
subtitle: "FISHING.ADS tag 3"
description: "Regtest reference case for FISHING-3."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/FISHING-3`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-3).

## What this case captures

`FISHING-3` is the host-side reference for `FISHING.ADS` tag `3`.
The host runner booted the scene with:

```text
window nosound story single 19 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `FISHING` |
| Tag | `3` |
| Scene index | `19` |
| Status | `verified` |
| Capture date | `2026-03-28T15:31:05Z` |
| Frames captured | `396` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
d596ba664ecaa2d34571cd961e655386ba84fee030676a11c62e6314c9fb575e
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-3/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-3/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-3/review.html)

## Cross-links

- [Live scene page: `FISHING 3`]({{ '/scenes/fishing3/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
