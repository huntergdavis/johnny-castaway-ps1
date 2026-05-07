# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `0.6781%` over target
- Average estimated align4 x-band upload byte saving: `67.61%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `visitor3` | `low` | 299.45 | 38.42% | 56.97% | 303.8% | 309 | x-band rect cap pressure; needs selective bands; rect/frame 1.5 |
| 2 | `visitor3` | `high` | 296.24 | 37.98% | 56.97% | 303.8% | 303 | x-band rect cap pressure; needs selective bands; rect/frame 1.5 |
| 3 | `building4` | `high` | 221.36 | 7.61% | 65.1% | 191.66% | 336 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 4 | `building4` | `low` | 207.74 | 7.08% | 65.1% | 191.66% | 318 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 5 | `walkstuf1` | `low` | 184.96 | 15.53% | 46.64% | 493.1% | 335 | x-band rect cap pressure; needs selective bands; rect/frame 2.85 |
| 6 | `walkstuf1` | `high` | 171.55 | 13.68% | 46.64% | 493.1% | 328 | x-band rect cap pressure; needs selective bands; rect/frame 2.85 |
| 7 | `building2` | `low` | 153.21 | 11.12% | 67.28% | 345.12% | 228 | candidate; rect/frame 2.47 |
| 8 | `building2` | `high` | 152.86 | 10.94% | 67.28% | 345.12% | 232 | candidate; rect/frame 2.47 |
| 9 | `johnny2` | `low` | 130.77 | 2.80% | 74.53% | 443.76% | 377 | candidate; rect/frame 1.52 |
| 10 | `johnny2` | `high` | 129.52 | 2.86% | 74.53% | 443.76% | 369 | candidate; rect/frame 1.52 |
| 11 | `building6` | `low` | 68.45 | 3.20% | 44.17% | 810.26% | 136 | x-band rect cap pressure; needs selective bands; rect/frame 2.52 |
| 12 | `building6` | `high` | 65.95 | 3.19% | 44.17% | 810.26% | 126 | x-band rect cap pressure; needs selective bands; rect/frame 2.52 |
| 13 | `walkstuf3` | `high` | 60.93 | 1.89% | 81.22% | 251.74% | 104 | candidate; rect/frame 2.09 |
| 14 | `mary1` | `high` | 50.14 | 0.77% | 78.75% | 274.07% | 84 | candidate; rect/frame 2.33 |
| 15 | `activity9` | `high` | 40.8 | 2.19% | 50.66% | 560.7% | 72 | x-band rect cap pressure; needs selective bands; rect/frame 3.48 |
| 16 | `walkstuf3` | `low` | 35.23 | 1.13% | 77.8% | 581.73% | 60 | large upload-ready payload; needs compression/selective bands; rect/frame 3.21 |
| 17 | `activity9` | `low` | 32.99 | 1.80% | 50.66% | 560.7% | 57 | x-band rect cap pressure; needs selective bands; rect/frame 3.48 |
| 18 | `johnny6` | `high` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 19 | `johnny6` | `low` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 20 | `mary1` | `low` | 29.84 | 0.41% | 80.45% | 268.49% | 55 | candidate; rect/frame 1.61 |

## Read Before Acting

- High upload-byte savings are not automatically promotable; naive direct16
  expansion already regressed WALKSTUF1 low by adding too much CD pressure.
- Rows with large payload growth need selective bands, compression, or setup
  residency before runtime promotion.
- Use `scripts/analyze-fg2-preprocess-plans.py --hotspot-count N` on the
  selected pack before a runtime probe. VISITOR3 now proves why: cap-hit frames
  `134..136` save `0%` under blanket x-band, while nearby non-cap frames carry
  most of the useful byte saving.
- The current VISITOR3 detail sheet is
  `docs/ps1/performance-preprocess-visitor3-hotspots.csv`. Its default
  threshold plan selects `96 / 144` frames, excludes `3` cap-hit frames, and
  estimates `6114568` selected-subset upload bytes saved.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
