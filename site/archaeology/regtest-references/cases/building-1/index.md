---
layout: page
title: "BUILDING-1 regression reference"
eyebrow: Host regression reference
subtitle: "BUILDING.ADS tag 1"
description: "Regtest reference case for BUILDING-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/BUILDING-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-1).

## What this case captures

`BUILDING-1` is the host-side reference for `BUILDING.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 10 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `BUILDING` |
| Tag | `1` |
| Scene index | `10` |
| Status | `verified` |
| Capture date | `2026-03-28T15:25:07Z` |
| Frames captured | `157` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
bf98fdd0d69cc957f46e785c6ba97cfd1c522e66d8bd613f1b19af885cfdb48c
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-1/review.html)

## Cross-links

- [Live scene page: `BUILDING 1`]({{ '/scenes/building1/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
