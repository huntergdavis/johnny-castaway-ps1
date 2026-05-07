---
layout: page
title: "SUZY-2 regression reference"
eyebrow: Host regression reference
subtitle: "SUZY.ADS tag 2"
description: "Regtest reference case for SUZY-2."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/SUZY-2`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/SUZY-2).

## What this case captures

`SUZY-2` is the host-side reference for `SUZY.ADS` tag `2`.
The host runner booted the scene with:

```text
window nosound story single 53 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `SUZY` |
| Tag | `2` |
| Scene index | `53` |
| Status | `verified` |
| Capture date | `2026-03-28T15:44:24Z` |
| Frames captured | `140` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
a91e47dc1120cbf00edb4ac4d6f190c901d1b0d8e629c822672d306c5de5b82b
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/SUZY-2/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/SUZY-2/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/SUZY-2/review.html)

## Cross-links

- [Live scene page: `SUZY 2`]({{ '/scenes/suzy2/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
