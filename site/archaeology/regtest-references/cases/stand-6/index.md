---
layout: page
title: "STAND-6 regression reference"
eyebrow: Host regression reference
subtitle: "STAND.ADS tag 6"
description: "Regtest reference case for STAND-6."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/STAND-6`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-6).

## What this case captures

`STAND-6` is the host-side reference for `STAND.ADS` tag `6`.
The host runner booted the scene with:

```text
window nosound story single 43 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `STAND` |
| Tag | `6` |
| Scene index | `43` |
| Status | `verified` |
| Capture date | `2026-03-28T15:41:31Z` |
| Frames captured | `39` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
827df66aa783fc338eef3a26332224550c2167285f8d0ad7460136f7e5835219
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-6/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-6/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/STAND-6/review.html)

## Cross-links

- [Live scene page: `STAND 6`]({{ '/scenes/stand6/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
