---
layout: page
title: "BUILDING-3 regression reference"
eyebrow: Host regression reference
subtitle: "BUILDING.ADS tag 3"
description: "Regtest reference case for BUILDING-3."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/BUILDING-3`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-3).

## What this case captures

`BUILDING-3` is the host-side reference for `BUILDING.ADS` tag `3`.
The host runner booted the scene with:

```text
window nosound story single 12 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `BUILDING` |
| Tag | `3` |
| Scene index | `12` |
| Status | `verified` |
| Capture date | `2026-03-28T15:26:00Z` |
| Frames captured | `229` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
d5995969f20084ab5b0fd0779b33a3ad82f80a7e8e68aa7a859af157b340304e
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-3/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-3/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-3/review.html)

## Cross-links

- [Live scene page: `BUILDING 3`]({{ '/scenes/building3/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
