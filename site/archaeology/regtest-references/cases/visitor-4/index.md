---
layout: page
title: "VISITOR-4 regression reference"
eyebrow: Host regression reference
subtitle: "VISITOR.ADS tag 4"
description: "Regtest reference case for VISITOR-4."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/VISITOR-4`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-4).

## What this case captures

`VISITOR-4` is the host-side reference for `VISITOR.ADS` tag `4`.
The host runner booted the scene with:

```text
window nosound story single 56 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `VISITOR` |
| Tag | `4` |
| Scene index | `56` |
| Status | `verified` |
| Capture date | `2026-03-28T15:45:58Z` |
| Frames captured | `72` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
b5cd11267392c4e334ed52873d0eaabd7ef364dba84caf1915134f974cea5d06
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-4/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-4/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-4/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
