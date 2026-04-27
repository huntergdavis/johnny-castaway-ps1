---
layout: page
title: "JOHNNY-3 regression reference"
eyebrow: Host regression reference
subtitle: "JOHNNY.ADS tag 3"
description: "Regtest reference case for JOHNNY-3."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/JOHNNY-3`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-3).

## What this case captures

`JOHNNY-3` is the host-side reference for `JOHNNY.ADS` tag `3`.
The host runner booted the scene with:

```text
window nosound story single 27 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `JOHNNY` |
| Tag | `3` |
| Scene index | `27` |
| Status | `verified` |
| Capture date | `2026-03-28T15:34:34Z` |
| Frames captured | `222` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
9ad373e8d9915b71c4f0730ec86aebee8879c365d4a682ed017e519ff0edb407
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-3/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-3/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-3/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
