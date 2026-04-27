---
layout: page
title: "MISCGAG-2 regression reference"
eyebrow: Host regression reference
subtitle: "MISCGAG.ADS tag 2"
description: "Regtest reference case for MISCGAG-2."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/MISCGAG-2`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MISCGAG-2).

## What this case captures

`MISCGAG-2` is the host-side reference for `MISCGAG.ADS` tag `2`.
The host runner booted the scene with:

```text
window nosound story single 37 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `MISCGAG` |
| Tag | `2` |
| Scene index | `37` |
| Status | `verified` |
| Capture date | `2026-03-28T15:40:33Z` |
| Frames captured | `216` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
a908ce172c8e07a65c27b345b50152b3bd58553cf29253bd9c8c1c9f17ad9f70
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MISCGAG-2/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MISCGAG-2/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MISCGAG-2/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
