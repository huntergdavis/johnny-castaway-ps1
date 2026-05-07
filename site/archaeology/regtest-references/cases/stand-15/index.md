---
layout: page
title: "STAND-15 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 15"
description: "Regtest reference case for STAND-15."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-15`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-15).

## What this case captures

`STAND-15` is the host-side reference for `STAND.ADS` tag `15`.
The host runner booted the scene with:

```text
window nosound story single 50 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `15` |
| Scene index | `50` |
| Status | `verified` |
| Capture date | `2026-03-28T15:42:13Z` |
| Frames captured | `94` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
74b80d571d943f54149f97c261d76fcce76b8b7143965fdd607f52a2d9a3278d
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-15/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-15/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-15/review.html)

## Cross-links

- [Live scene page: `STAND 15`]({{ '/scenes/stand15/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
