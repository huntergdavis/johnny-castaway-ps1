---
layout: page
title: "STAND-7 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 7"
description: "Regtest reference case for STAND-7."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-7`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-7).

## What this case captures

`STAND-7` is the host-side reference for `STAND.ADS` tag `7`.
The host runner booted the scene with:

```text
window nosound story single 44 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `7` |
| Scene index | `44` |
| Status | `verified` |
| Capture date | `2026-03-28T15:41:38Z` |
| Frames captured | `25` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
5350670b3f2639b25dd6ea96e2a8af0ca7c1a1cb7ddaf451174d4a3042b09721
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-7/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-7/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-7/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
