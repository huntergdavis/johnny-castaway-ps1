---
layout: page
title: "STAND-9 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 9"
description: "Regtest reference case for STAND-9."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-9`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-9).

## What this case captures

`STAND-9` is the host-side reference for `STAND.ADS` tag `9`.
The host runner booted the scene with:

```text
window nosound story single 46 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `9` |
| Scene index | `46` |
| Status | `verified` |
| Capture date | `2026-03-28T15:41:49Z` |
| Frames captured | `25` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
a6d5f03c6e58af361b1222d6edf79fdd32062db3f1014bce0689fc33155fbacf
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-9/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-9/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-9/review.html)

## Cross-links

- [Live scene page: `STAND 9`]({{ '/scenes/stand9/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
