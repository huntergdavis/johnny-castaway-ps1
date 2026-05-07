---
layout: page
title: "FISHING-5 regression reference"
eyebrow: Host regression reference
subtitle: "FISHING.ADS tag 5"
description: "Regtest reference case for FISHING-5."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/FISHING-5`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-5).

## What this case captures

`FISHING-5` is the host-side reference for `FISHING.ADS` tag `5`.
The host runner booted the scene with:

```text
window nosound story single 21 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `FISHING` |
| Tag | `5` |
| Scene index | `21` |
| Status | `verified` |
| Capture date | `2026-03-28T15:32:10Z` |
| Frames captured | `111` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
a62ddb2c5aaa5c9a79f8edf583b3bcb3365d2c5c1a5fe4fcb52b6936bc89170e
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-5/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-5/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/FISHING-5/review.html)

## Cross-links

- [Live scene page: `FISHING 5`]({{ '/scenes/fishing5/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
