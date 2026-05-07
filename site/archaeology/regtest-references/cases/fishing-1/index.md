---
layout: page
title: "FISHING-1 regression reference"
eyebrow: Host regression reference
subtitle: "FISHING.ADS tag 1"
description: "Regtest reference case for FISHING-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/FISHING-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-1).

## What this case captures

`FISHING-1` is the host-side reference for `FISHING.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 17 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `FISHING` |
| Tag | `1` |
| Scene index | `17` |
| Status | `verified` |
| Capture date | `2026-03-28T15:30:44Z` |
| Frames captured | `82` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
ec4f7be9f7f3b5991301e830f0e7fb56a4dc0c2f55378bb095d91100a633d4fe
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-1/review.html)

## Cross-links

- [Live scene page: `FISHING 1`]({{ '/scenes/fishing1/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
