---
layout: page
title: "MARY-1 regression reference"
eyebrow: Host regression reference
subtitle: "MARY.ADS tag 1"
description: "Regtest reference case for MARY-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/MARY-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-1).

## What this case captures

`MARY-1` is the host-side reference for `MARY.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 31 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `MARY` |
| Tag | `1` |
| Scene index | `31` |
| Status | `verified` |
| Capture date | `2026-03-28T15:36:39Z` |
| Frames captured | `251` |
| Exit code | `139` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
1b9920a80e0ca7ce41ef46b05c294e9f323b66333442c8dcebfdec65ad797ef5
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-1/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
