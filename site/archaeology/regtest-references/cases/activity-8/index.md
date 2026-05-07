---
layout: page
title: "ACTIVITY-8 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 8"
description: "Regtest reference case for ACTIVITY-8."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-8`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-8).

## What this case captures

`ACTIVITY-8` is the host-side reference for `ACTIVITY.ADS` tag `8`.
The host runner booted the scene with:

```text
window nosound story single 8 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `8` |
| Scene index | `8` |
| Status | `verified` |
| Capture date | `2026-03-28T15:22:22Z` |
| Frames captured | `147` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
d18b5b715a9509418bd5283f66ca7af6304837bd5975f576ce4bdf7d47e34825
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-8/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-8/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-8/review.html)

## Cross-links

- [Live scene page: `ACTIVITY 8`]({{ '/scenes/activity8/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
