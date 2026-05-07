---
layout: page
title: "FISHING-6 regression reference"
eyebrow: Host regression reference
subtitle: "FISHING.ADS tag 6"
description: "Regtest reference case for FISHING-6."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/FISHING-6`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-6).

## What this case captures

`FISHING-6` is the host-side reference for `FISHING.ADS` tag `6`.
The host runner booted the scene with:

```text
window nosound story single 22 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `FISHING` |
| Tag | `6` |
| Scene index | `22` |
| Status | `verified` |
| Capture date | `2026-03-28T15:32:30Z` |
| Frames captured | `119` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
98dc536462408bbdc9d3a2aace50982dabe8f25f56e15f04ded274e2a4226c06
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-6/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-6/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-6/review.html)

## Cross-links

- [Live scene page: `FISHING 6`]({{ '/scenes/fishing6/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
