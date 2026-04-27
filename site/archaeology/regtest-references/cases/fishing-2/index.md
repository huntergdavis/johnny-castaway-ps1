---
layout: page
title: "FISHING-2 regression reference"
eyebrow: Host regression reference
subtitle: "FISHING.ADS tag 2"
description: "Regtest reference case for FISHING-2."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/FISHING-2`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-2).

## What this case captures

`FISHING-2` is the host-side reference for `FISHING.ADS` tag `2`.
The host runner booted the scene with:

```text
window nosound story single 18 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `FISHING` |
| Tag | `2` |
| Scene index | `18` |
| Status | `verified` |
| Capture date | `2026-03-28T15:30:55Z` |
| Frames captured | `67` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
67dddd67373070635c63b3167cab157db032b2098341fe89abdf53c93b246df4
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-2/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-2/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-2/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
