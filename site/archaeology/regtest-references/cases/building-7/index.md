---
layout: page
title: "BUILDING-7 regression reference"
eyebrow: Host regression reference
subtitle: "BUILDING.ADS tag 7"
description: "Regtest reference case for BUILDING-7."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/BUILDING-7`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-7).

## What this case captures

`BUILDING-7` is the host-side reference for `BUILDING.ADS` tag `7`.
The host runner booted the scene with:

```text
window nosound story single 15 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `BUILDING` |
| Tag | `7` |
| Scene index | `15` |
| Status | `verified` |
| Capture date | `2026-03-28T15:29:32Z` |
| Frames captured | `753` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
bb6c1d59e0ffc536c8387cf6290342fd238dc276bed36a26e6b40aa984e00f8a
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-7/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-7/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-7/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
