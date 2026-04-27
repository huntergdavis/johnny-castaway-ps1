---
layout: page
title: "JOHNNY-4 regression reference"
eyebrow: Host regression reference
subtitle: "JOHNNY.ADS tag 4"
description: "Regtest reference case for JOHNNY-4."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/JOHNNY-4`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-4).

## What this case captures

`JOHNNY-4` is the host-side reference for `JOHNNY.ADS` tag `4`.
The host runner booted the scene with:

```text
window nosound story single 28 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `JOHNNY` |
| Tag | `4` |
| Scene index | `28` |
| Status | `verified` |
| Capture date | `2026-03-28T15:35:01Z` |
| Frames captured | `199` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
ee96bf3a73653780f3a568a5e5fa348fa4fef71e10d7d4616b8b3132929ab867
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-4/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-4/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/JOHNNY-4/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
