---
layout: page
title: "ACTIVITY-4 regression reference"
eyebrow: Host regression reference
subtitle: "ACTIVITY.ADS tag 4"
description: "Regtest reference case for ACTIVITY-4."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/ACTIVITY-4`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-4).

## What this case captures

`ACTIVITY-4` is the host-side reference for `ACTIVITY.ADS` tag `4`.
The host runner booted the scene with:

```text
window nosound story single 4 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `ACTIVITY` |
| Tag | `4` |
| Scene index | `4` |
| Status | `verified` |
| Capture date | `2026-03-28T15:20:49Z` |
| Frames captured | `191` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
2eb84a49f3e368b4126d656b4a2bc3c4235756dd15bed9e80c4299416cd0c991
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-4/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-4/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/ACTIVITY-4/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
