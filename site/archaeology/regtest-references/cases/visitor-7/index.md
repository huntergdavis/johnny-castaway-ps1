---
layout: page
title: "VISITOR-7 regression reference"
eyebrow: Host regression reference
subtitle: "VISITOR.ADS tag 7"
description: "Regtest reference case for VISITOR-7."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/VISITOR-7`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-7).

## What this case captures

`VISITOR-7` is the host-side reference for `VISITOR.ADS` tag `7`.
The host runner booted the scene with:

```text
window nosound story single 58 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `VISITOR` |
| Tag | `7` |
| Scene index | `58` |
| Status | `verified` |
| Capture date | `2026-03-28T15:47:22Z` |
| Frames captured | `227` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
da0b9bd2015f2c8ee82810f1718ea7b246ee9201b508b933fc060fa0ef3a3f85
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-7/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-7/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-7/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
