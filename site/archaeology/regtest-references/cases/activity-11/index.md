---
layout: page
title: "ACTIVITY-11 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 11"
description: "Regtest reference case for ACTIVITY-11."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-11`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-11).

## What this case captures

`ACTIVITY-11` is the host-side reference for `ACTIVITY.ADS` tag `11`.
The host runner booted the scene with:

```text
window nosound story single 2 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `11` |
| Scene index | `2` |
| Status | `verified` |
| Capture date | `2026-03-28T15:23:56Z` |
| Frames captured | `384` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
a47bf8929ceeae747042fe87e6623760883f15f6d5bc6aabf3af7f08cbda29a3
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-11/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-11/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-11/review.html)

## Cross-links

- [Live scene page: `ACTIVITY 11`]({{ '/scenes/activity11/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
