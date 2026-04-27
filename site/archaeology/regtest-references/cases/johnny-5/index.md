---
layout: page
title: "JOHNNY-5 regression reference"
eyebrow: Host regression reference
subtitle: "JOHNNY.ADS tag 5"
description: "Regtest reference case for JOHNNY-5."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/JOHNNY-5`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-5).

## What this case captures

`JOHNNY-5` is the host-side reference for `JOHNNY.ADS` tag `5`.
The host runner booted the scene with:

```text
window nosound story single 29 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `JOHNNY` |
| Tag | `5` |
| Scene index | `29` |
| Status | `verified` |
| Capture date | `2026-03-28T15:35:28Z` |
| Frames captured | `171` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
69213c167f327c8666b6eb1cdeb46b329fb0276fb198124d0659c18c2f3a3d62
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-5/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-5/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-5/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
