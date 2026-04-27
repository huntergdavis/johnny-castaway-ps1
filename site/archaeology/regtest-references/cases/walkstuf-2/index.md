---
layout: page
title: "WALKSTUF-2 regression reference"
eyebrow: Host regression reference
subtitle: "WALKSTUF.ADS tag 2"
description: "Regtest reference case for WALKSTUF-2."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/WALKSTUF-2`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/WALKSTUF-2).

## What this case captures

`WALKSTUF-2` is the host-side reference for `WALKSTUF.ADS` tag `2`.
The host runner booted the scene with:

```text
window nosound story single 61 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `WALKSTUF` |
| Tag | `2` |
| Scene index | `61` |
| Status | `verified` |
| Capture date | `2026-03-28T15:48:32Z` |
| Frames captured | `81` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
25b93053b10bc562c06e18e8246e132eefbbb66d2709522f994b8be664814c64
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/WALKSTUF-2/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/WALKSTUF-2/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/WALKSTUF-2/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
