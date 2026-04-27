---
layout: page
title: "FISHING-7 regression reference"
eyebrow: Host regression reference
subtitle: "FISHING.ADS tag 7"
description: "Regtest reference case for FISHING-7."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/FISHING-7`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-7).

## What this case captures

`FISHING-7` is the host-side reference for `FISHING.ADS` tag `7`.
The host runner booted the scene with:

```text
window nosound story single 23 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `FISHING` |
| Tag | `7` |
| Scene index | `23` |
| Status | `verified` |
| Capture date | `2026-03-28T15:32:48Z` |
| Frames captured | `64` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
b1eb200805be80e9220c948ae251497545d73ecd9128a72a2545def669d41659
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-7/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-7/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-7/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
