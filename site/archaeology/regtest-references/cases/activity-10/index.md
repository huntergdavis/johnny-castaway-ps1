---
layout: page
title: "ACTIVITY-10 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 10"
description: "Regtest reference case for ACTIVITY-10."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-10`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-10).

## What this case captures

`ACTIVITY-10` is the host-side reference for `ACTIVITY.ADS` tag `10`.
The host runner booted the scene with:

```text
window nosound story single 3 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `10` |
| Scene index | `3` |
| Status | `verified` |
| Capture date | `2026-03-28T15:23:26Z` |
| Frames captured | `267` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
b1bfca809d2890bf88196195e60b5603c1427a9b80177dee6b05e5ec44db5e2b
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-10/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-10/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-10/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
