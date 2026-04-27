---
layout: page
title: "MARY-3 regression reference"
eyebrow: Host regression reference
subtitle: "MARY.ADS tag 3"
description: "Regtest reference case for MARY-3."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/MARY-3`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-3).

## What this case captures

`MARY-3` is the host-side reference for `MARY.ADS` tag `3`.
The host runner booted the scene with:

```text
window nosound story single 32 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `MARY` |
| Tag | `3` |
| Scene index | `32` |
| Status | `verified` |
| Capture date | `2026-03-28T15:38:03Z` |
| Frames captured | `448` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
df66a6924790ec81450b74c6ae2348b5e7fc8029896b2affe329195f99b19ea2
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-3/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-3/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/MARY-3/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
