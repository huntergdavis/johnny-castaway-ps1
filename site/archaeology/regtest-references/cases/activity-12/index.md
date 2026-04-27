---
layout: page
title: "ACTIVITY-12 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 12"
description: "Regtest reference case for ACTIVITY-12."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-12`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-12).

## What this case captures

`ACTIVITY-12` is the host-side reference for `ACTIVITY.ADS` tag `12`.
The host runner booted the scene with:

```text
window nosound story single 1 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `12` |
| Scene index | `1` |
| Status | `verified` |
| Capture date | `2026-03-28T15:24:35Z` |
| Frames captured | `260` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
64020ed8b874008db4a29eb936164eedb09b92aba99f02bbda612063beb6fa7b
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-12/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-12/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-12/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
