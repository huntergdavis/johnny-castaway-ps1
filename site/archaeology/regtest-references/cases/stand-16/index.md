---
layout: page
title: "STAND-16 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 16"
description: "Regtest reference case for STAND-16."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-16`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-16).

## What this case captures

`STAND-16` is the host-side reference for `STAND.ADS` tag `16`.
The host runner booted the scene with:

```text
window nosound story single 51 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `16` |
| Scene index | `51` |
| Status | `verified` |
| Capture date | `2026-03-28T15:42:28Z` |
| Frames captured | `90` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
a06e69d8ab04b658ac38fa14783960a2c99e0d038952a7eab7916758f9ed63de
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-16/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-16/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-16/review.html)

## Cross-links

- [Live scene page: `STAND 16`]({{ '/scenes/stand16/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
