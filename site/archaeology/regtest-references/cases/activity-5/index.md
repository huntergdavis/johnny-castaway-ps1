---
layout: page
title: "ACTIVITY-5 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 5"
description: "Regtest reference case for ACTIVITY-5."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-5`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-5).

## What this case captures

`ACTIVITY-5` is the host-side reference for `ACTIVITY.ADS` tag `5`.
The host runner booted the scene with:

```text
window nosound story single 5 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `5` |
| Scene index | `5` |
| Status | `verified` |
| Capture date | `2026-03-28T15:21:14Z` |
| Frames captured | `311` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
0476618226133c25c9236a0a3747a55968eefb2dd274b71067a1943a49fd1e19
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-5/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-5/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-5/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
