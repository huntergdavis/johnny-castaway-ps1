---
layout: page
title: "BUILDING-5 regression reference"
eyebrow: Host regression reference
subtitle: "BUILDING.ADS tag 5"
description: "Regtest reference case for BUILDING-5."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/BUILDING-5`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-5).

## What this case captures

`BUILDING-5` is the host-side reference for `BUILDING.ADS` tag `5`.
The host runner booted the scene with:

```text
window nosound story single 14 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `BUILDING` |
| Tag | `5` |
| Scene index | `14` |
| Status | `verified` |
| Capture date | `2026-03-28T15:27:35Z` |
| Frames captured | `621` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
7f0a2c7a29b60c992f8a201ee7362d6cf29cc6d0b3c56e154f4f62ba289b1c0d
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-5/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-5/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-5/review.html)

## Cross-links

- [Live scene page: `BUILDING 5`]({{ '/scenes/building5/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
