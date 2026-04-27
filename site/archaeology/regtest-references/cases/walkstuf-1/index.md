---
layout: page
title: "WALKSTUF-1 regression reference"
eyebrow: Host regression reference
subtitle: "WALKSTUF.ADS tag 1"
description: "Regtest reference case for WALKSTUF-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/WALKSTUF-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/WALKSTUF-1).

## What this case captures

`WALKSTUF-1` is the host-side reference for `WALKSTUF.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 60 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `WALKSTUF` |
| Tag | `1` |
| Scene index | `60` |
| Status | `verified` |
| Capture date | `2026-03-28T15:47:57Z` |
| Frames captured | `366` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
8bd00eb70d11c1e1e7eba242a642e1f15d5b53203bbeab592ccc6173d18ebe02
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/WALKSTUF-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/WALKSTUF-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/WALKSTUF-1/review.html)

## Cross-links

- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
