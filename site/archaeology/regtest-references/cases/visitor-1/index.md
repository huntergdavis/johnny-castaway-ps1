---
layout: page
title: "VISITOR-1 regression reference"
eyebrow: Host regression reference
subtitle: "VISITOR.ADS tag 1"
description: "Regtest reference case for VISITOR-1."
---

This is the public shelf page for the host regression reference at
[`docs/ps1/archaeology/regtest-references/VISITOR-1`]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-1).

## What this case captures

`VISITOR-1` is the host-side reference for `VISITOR.ADS` tag `1`.
The host runner booted the scene with:

```text
window nosound story single 54 seed 1
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `VISITOR` |
| Tag | `1` |
| Scene index | `54` |
| Status | `verified` |
| Capture date | `2026-03-28T15:45:13Z` |
| Frames captured | `129` |
| Exit code | `0` |
| Timed out | `False` |
| Fatal error | `False` |

## State hash

```text
7059e67b6d65b33e9c1315d4bf94b0ebdadd161ef0167c00c6b99edb183914c9
```

## Source artifacts

- [`metadata.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-1/metadata.json)
- [`result.json`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-1/result.json)
- [`review.html`]({{ site.github_url }}/blob/main/docs/ps1/archaeology/regtest-references/VISITOR-1/review.html)

## Cross-links

- [Live scene page: `VISITOR 1`]({{ '/scenes/visitor1/' | relative_url }}) — current PS1 validation status, last-verified release tag, and per-scene case study.
- [All regtest references]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Regtest reference narrative]({{ '/archaeology/regtest-references/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }})
- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
