---
layout: page
title: "SUZY-1 regression reference"
eyebrow: Host regression reference
subtitle: "SUZY.ADS tag 1"
description: "Regtest reference case for SUZY-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/SUZY-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/SUZY-1).

## What this case captures

`SUZY-1` is the host-side reference for `SUZY.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 52 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `SUZY` |
| Tag | `1` |
| Scene index | `52` |
| Status | `verified` |
| Capture date | `2026-03-28T15:42:42Z` |
| Frames captured | `184` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
b78a780fb1334d9872a02008560846c1c5a8dfb8869d8e24033ade23d3770a0a
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/SUZY-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/SUZY-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/SUZY-1/review.html)

## Cross-links

- [Live scene page: `SUZY 1`]({{ '/scenes/suzy1/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
