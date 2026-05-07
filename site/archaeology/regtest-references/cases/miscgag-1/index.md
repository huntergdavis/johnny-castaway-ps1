---
layout: page
title: "MISCGAG-1 regression reference"
eyebrow: Host regression reference
subtitle: "MISCGAG.ADS tag 1"
description: "Regtest reference case for MISCGAG-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/MISCGAG-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MISCGAG-1).

## What this case captures

`MISCGAG-1` is the host-side reference for `MISCGAG.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 36 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `MISCGAG` |
| Tag | `1` |
| Scene index | `36` |
| Status | `verified` |
| Capture date | `2026-03-28T15:40:17Z` |
| Frames captured | `109` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
b5df7c4c045def5a9ced7220025b7ba6de4b664fe300a3c11817669f502a622d
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MISCGAG-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MISCGAG-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MISCGAG-1/review.html)

## Cross-links

- [Live scene page: `MISCGAG 1`]({{ '/scenes/miscgag1/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
