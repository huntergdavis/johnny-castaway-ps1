---
layout: page
title: "ACTIVITY-7 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 7"
description: "Regtest reference case for ACTIVITY-7."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-7`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-7).

## What this case captures

`ACTIVITY-7` is the host-side reference for `ACTIVITY.ADS` tag `7`.
The host runner booted the scene with:

```text
window nosound story single 7 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `7` |
| Scene index | `7` |
| Status | `verified` |
| Capture date | `2026-03-28T15:22:06Z` |
| Frames captured | `115` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
f23f992d2bb981170615f7ae5c04c6f5aadcc8e4dbf4a033c1f8c27630bb40dc
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-7/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-7/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-7/review.html)

## Cross-links

- [Live scene page: `ACTIVITY 7`]({{ '/scenes/activity7/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
