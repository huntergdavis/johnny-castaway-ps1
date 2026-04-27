---
layout: page
title: "BUILDING-4 regression reference"
eyebrow: Host regression reference
subtitle: "BUILDING.ADS tag 4"
description: "Regtest reference case for BUILDING-4."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/BUILDING-4`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-4).

## What this case captures

`BUILDING-4` is the host-side reference for `BUILDING.ADS` tag `4`.
The host runner booted the scene with:

```text
window nosound story single 11 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `BUILDING` |
| Tag | `4` |
| Scene index | `11` |
| Status | `verified` |
| Capture date | `2026-03-28T15:26:32Z` |
| Frames captured | `629` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
f865337bd318b08e53db8c6c26936ac3898f975bfc5b432afc5ef16b3f2c9de4
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-4/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-4/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/BUILDING-4/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
