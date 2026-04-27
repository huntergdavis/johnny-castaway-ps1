---
layout: page
title: "FISHING-8 regression reference"
eyebrow: Host regression reference
subtitle: "FISHING.ADS tag 8"
description: "Regtest reference case for FISHING-8."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/FISHING-8`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-8).

## What this case captures

`FISHING-8` is the host-side reference for `FISHING.ADS` tag `8`.
The host runner booted the scene with:

```text
window nosound story single 24 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `FISHING` |
| Tag | `8` |
| Scene index | `24` |
| Status | `verified` |
| Capture date | `2026-03-28T15:32:58Z` |
| Frames captured | `155` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
9c10b80834f293e01a5b3b52cd616249fb65391d2df863e2144248e341e4c530
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-8/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-8/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-8/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
