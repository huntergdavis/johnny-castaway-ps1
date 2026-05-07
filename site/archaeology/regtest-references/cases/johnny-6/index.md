---
layout: page
title: "JOHNNY-6 regression reference"
eyebrow: Host regression reference
subtitle: "JOHNNY.ADS tag 6"
description: "Regtest reference case for JOHNNY-6."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/JOHNNY-6`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-6).

## What this case captures

`JOHNNY-6` is the host-side reference for `JOHNNY.ADS` tag `6`.
The host runner booted the scene with:

```text
window nosound story single 30 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `JOHNNY` |
| Tag | `6` |
| Scene index | `30` |
| Status | `verified` |
| Capture date | `2026-03-28T15:35:48Z` |
| Frames captured | `89` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
80a081b432cfc4a84b5221808d3b28a7a6e59b0a991dcafbf6ec6292f0d380d5
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-6/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-6/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-6/review.html)

## Cross-links

- [Live scene page: `JOHNNY 6`]({{ '/scenes/johnny6/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
