---
layout: page
title: "MARY-4 regression reference"
eyebrow: Host regression reference
subtitle: "MARY.ADS tag 4"
description: "Regtest reference case for MARY-4."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/MARY-4`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-4).

## What this case captures

`MARY-4` is the host-side reference for `MARY.ADS` tag `4`.
The host runner booted the scene with:

```text
window nosound story single 34 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `MARY` |
| Tag | `4` |
| Scene index | `34` |
| Status | `verified` |
| Capture date | `2026-03-28T15:38:53Z` |
| Frames captured | `326` |
| Exit code | `139` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
396ef6d5492dac29381175949b5656f87ccdf8f003916f5648b0fa7a2c37706c
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-4/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-4/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-4/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
