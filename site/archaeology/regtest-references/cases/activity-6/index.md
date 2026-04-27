---
layout: page
title: "ACTIVITY-6 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 6"
description: "Regtest reference case for ACTIVITY-6."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-6`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-6).

## What this case captures

`ACTIVITY-6` is the host-side reference for `ACTIVITY.ADS` tag `6`.
The host runner booted the scene with:

```text
window nosound story single 6 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `6` |
| Scene index | `6` |
| Status | `verified` |
| Capture date | `2026-03-28T15:21:52Z` |
| Frames captured | `103` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
6270b61759dd99b7014c94714fa08e2289a47dac74ab73336dad360c2bef4b74
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-6/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-6/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-6/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
