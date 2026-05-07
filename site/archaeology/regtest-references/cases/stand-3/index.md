---
layout: page
title: "STAND-3 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 3"
description: "Regtest reference case for STAND-3."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-3`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-3).

## What this case captures

`STAND-3` is the host-side reference for `STAND.ADS` tag `3`.
The host runner booted the scene with:

```text
window nosound story single 40 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `3` |
| Scene index | `40` |
| Status | `verified` |
| Capture date | `2026-03-28T15:41:11Z` |
| Frames captured | `29` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
995e2844df0c46c0d06837423c7262c30754515db5351de7529193af100f1c1f
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-3/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-3/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-3/review.html)

## Cross-links

- [Live scene page: `STAND 3`]({{ '/scenes/stand3/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
