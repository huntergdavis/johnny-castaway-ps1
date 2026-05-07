---
layout: page
title: "BUILDING-2 regression reference"
eyebrow: Host regression reference
subtitle: "BUILDING.ADS tag 2"
description: "Regtest reference case for BUILDING-2."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/BUILDING-2`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-2).

## What this case captures

`BUILDING-2` is the host-side reference for `BUILDING.ADS` tag `2`.
The host runner booted the scene with:

```text
window nosound story single 13 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `BUILDING` |
| Tag | `2` |
| Scene index | `13` |
| Status | `verified` |
| Capture date | `2026-03-28T15:25:27Z` |
| Frames captured | `415` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
dfa1fa828f2b857929d192b73f89790418657ae5b74000d7fee89d6a076be638
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-2/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-2/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-2/review.html)

## Cross-links

- [Live scene page: `BUILDING 2`]({{ '/scenes/building2/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
