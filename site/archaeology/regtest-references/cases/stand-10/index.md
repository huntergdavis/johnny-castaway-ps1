---
layout: page
title: "STAND-10 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 10"
description: "Regtest reference case for STAND-10."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-10`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-10).

## What this case captures

`STAND-10` is the host-side reference for `STAND.ADS` tag `10`.
The host runner booted the scene with:

```text
window nosound story single 47 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `10` |
| Scene index | `47` |
| Status | `verified` |
| Capture date | `2026-03-28T15:41:55Z` |
| Frames captured | `25` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
e44ee5a07309205c321cb9879e36f907abb671e5857a86c7905f5b0b481b4342
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-10/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-10/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-10/review.html)

## Cross-links

- [Live scene page: `STAND 10`]({{ '/scenes/stand10/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
