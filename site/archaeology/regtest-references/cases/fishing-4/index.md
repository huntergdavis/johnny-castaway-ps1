---
layout: page
title: "FISHING-4 regression reference"
eyebrow: Host regression reference
subtitle: "FISHING.ADS tag 4"
description: "Regtest reference case for FISHING-4."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/FISHING-4`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-4).

## What this case captures

`FISHING-4` is the host-side reference for `FISHING.ADS` tag `4`.
The host runner booted the scene with:

```text
window nosound story single 20 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `FISHING` |
| Tag | `4` |
| Scene index | `20` |
| Status | `verified` |
| Capture date | `2026-03-28T15:31:50Z` |
| Frames captured | `141` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
ff0a05d08673d1a616d218ef1ed19b82118ec113e6a7572a4f94a0790f740b2a
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-4/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-4/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-4/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
