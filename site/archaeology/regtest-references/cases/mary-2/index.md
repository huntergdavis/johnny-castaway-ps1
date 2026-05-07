---
layout: page
title: "MARY-2 regression reference"
eyebrow: Host regression reference
subtitle: "MARY.ADS tag 2"
description: "Regtest reference case for MARY-2."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/MARY-2`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-2).

## What this case captures

`MARY-2` is the host-side reference for `MARY.ADS` tag `2`.
The host runner booted the scene with:

```text
window nosound story single 33 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `MARY` |
| Tag | `2` |
| Scene index | `33` |
| Status | `verified` |
| Capture date | `2026-03-28T15:37:13Z` |
| Frames captured | `436` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
0e9bc00173d04eb77916e831d8aae0462ec7dd8c31b8c3da9470c7a06c07c962
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-2/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-2/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-2/review.html)

## Cross-links

- [Live scene page: `MARY 2`]({{ '/scenes/mary2/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
