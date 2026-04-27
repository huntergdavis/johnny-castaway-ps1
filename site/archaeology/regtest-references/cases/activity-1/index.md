---
layout: page
title: "ACTIVITY-1 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 1"
description: "Regtest reference case for ACTIVITY-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-1).

## What this case captures

`ACTIVITY-1` is the host-side reference for `ACTIVITY.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 0 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `1` |
| Scene index | `0` |
| Status | `verified` |
| Capture date | `2026-03-28T15:20:04Z` |
| Frames captured | `356` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
a58f99082baadc2f86a250c7a16b15598ccbd4c9da432b92d49770df175a2669
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-1/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
