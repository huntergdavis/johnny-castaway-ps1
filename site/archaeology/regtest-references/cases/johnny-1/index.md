---
layout: page
title: "JOHNNY-1 regression reference"
eyebrow: Host regression reference
subtitle: "JOHNNY.ADS tag 1"
description: "Regtest reference case for JOHNNY-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/JOHNNY-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-1).

## What this case captures

`JOHNNY-1` is the host-side reference for `JOHNNY.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 25 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `JOHNNY` |
| Tag | `1` |
| Scene index | `25` |
| Status | `verified` |
| Capture date | `2026-03-28T15:33:19Z` |
| Frames captured | `119` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
1e83c88cfc7c4fda5c14b0e631b51ee81a4c102515a5825b87b1b67d00a446b1
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-1/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
