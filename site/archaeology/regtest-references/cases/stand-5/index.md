---
layout: page
title: "STAND-5 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 5"
description: "Regtest reference case for STAND-5."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-5`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-5).

## What this case captures

`STAND-5` is the host-side reference for `STAND.ADS` tag `5`.
The host runner booted the scene with:

```text
window nosound story single 42 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `5` |
| Scene index | `42` |
| Status | `verified` |
| Capture date | `2026-03-28T15:41:24Z` |
| Frames captured | `42` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
ba780ca42cbd2af3e3a81324d04db1f3fbf5c5cdd147afc46fabd41f5a1fabda
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-5/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-5/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-5/review.html)

## Cross-links

- [Live scene page: `STAND 5`]({{ '/scenes/stand5/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
