---
layout: page
title: "STAND-8 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 8"
description: "Regtest reference case for STAND-8."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-8`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-8).

## What this case captures

`STAND-8` is the host-side reference for `STAND.ADS` tag `8`.
The host runner booted the scene with:

```text
window nosound story single 45 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `8` |
| Scene index | `45` |
| Status | `verified` |
| Capture date | `2026-03-28T15:41:44Z` |
| Frames captured | `23` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
1e7f1f4749d3b2c025ffd67eae37c83cf3d5a0a46e90b7280f73b0c523b908fd
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-8/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-8/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-8/review.html)

## Cross-links

- [Live scene page: `STAND 8`]({{ '/scenes/stand8/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
