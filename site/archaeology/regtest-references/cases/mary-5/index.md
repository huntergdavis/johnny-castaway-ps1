---
layout: page
title: "MARY-5 regression reference"
eyebrow: Host regression reference
subtitle: "MARY.ADS tag 5"
description: "Regtest reference case for MARY-5."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/MARY-5`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-5).

## What this case captures

`MARY-5` is the host-side reference for `MARY.ADS` tag `5`.
The host runner booted the scene with:

```text
window nosound story single 35 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `MARY` |
| Tag | `5` |
| Scene index | `35` |
| Status | `verified` |
| Capture date | `2026-03-28T15:39:39Z` |
| Frames captured | `409` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
629894577915c178f5e2e2e4e13a1af8719a6bfd511a2ba8f2305fac7c08b07b
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-5/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-5/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-5/review.html)

## Cross-links

- [Live scene page: `MARY 5`]({{ '/scenes/mary5/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
