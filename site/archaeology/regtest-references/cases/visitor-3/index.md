---
layout: page
title: "VISITOR-3 regression reference"
eyebrow: Host regression reference
subtitle: "VISITOR.ADS tag 3"
description: "Regtest reference case for VISITOR-3."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/VISITOR-3`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-3).

## What this case captures

`VISITOR-3` is the host-side reference for `VISITOR.ADS` tag `3`.
The host runner booted the scene with:

```text
window nosound story single 55 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `VISITOR` |
| Tag | `3` |
| Scene index | `55` |
| Status | `verified` |
| Capture date | `2026-03-28T15:45:30Z` |
| Frames captured | `204` |
| Exit code | `134` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
6b08d1113b45e636bebf1ef888b5f48277855a5636754a4dc6ead3e76448257c
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-3/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-3/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-3/review.html)

## Cross-links

- [Live scene page: `VISITOR 3`]({{ '/scenes/visitor3/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
