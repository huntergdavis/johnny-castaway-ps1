---
layout: page
title: "STAND-1 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 1"
description: "Regtest reference case for STAND-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-1).

## What this case captures

`STAND-1` is the host-side reference for `STAND.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 38 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `1` |
| Scene index | `38` |
| Status | `verified` |
| Capture date | `2026-03-28T15:41:03Z` |
| Frames captured | `8` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
4e36a30a738850dddefd83c063ce045b9b7962f330c7f69943c1e8101df5295b
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-1/review.html)

## Cross-links

- [Live scene page: `STAND 1`]({{ '/scenes/stand1/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
