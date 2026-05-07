# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `0.8231%` over target
- Average estimated align4 x-band upload byte saving: `67.07%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `visitor3` | `low` | 345.69 | 43.48% | 56.97% | 229.28% | 380 | x-band rect cap pressure; needs selective bands; rect/frame 1.5 |
| 2 | `visitor3` | `high` | 340.09 | 42.86% | 56.97% | 229.28% | 369 | x-band rect cap pressure; needs selective bands; rect/frame 1.5 |
| 3 | `building4` | `high` | 221.36 | 7.61% | 65.1% | 191.66% | 336 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 4 | `building2` | `high` | 216.5 | 14.86% | 67.28% | 258.68% | 352 | candidate; rect/frame 2.47 |
| 5 | `building4` | `low` | 207.74 | 7.08% | 65.1% | 191.66% | 318 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 6 | `building2` | `low` | 207.56 | 14.63% | 67.28% | 258.68% | 327 | candidate; rect/frame 2.47 |
| 7 | `walkstuf1` | `low` | 184.96 | 15.53% | 46.64% | 493.1% | 335 | x-band rect cap pressure; needs selective bands; rect/frame 2.85 |
| 8 | `walkstuf1` | `high` | 171.55 | 13.68% | 46.64% | 493.1% | 328 | x-band rect cap pressure; needs selective bands; rect/frame 2.85 |
| 9 | `building6` | `low` | 68.45 | 3.20% | 44.17% | 810.26% | 136 | x-band rect cap pressure; needs selective bands; rect/frame 2.52 |
| 10 | `building6` | `high` | 65.95 | 3.19% | 44.17% | 810.26% | 126 | x-band rect cap pressure; needs selective bands; rect/frame 2.52 |
| 11 | `walkstuf3` | `high` | 60.93 | 1.89% | 81.22% | 251.74% | 104 | candidate; rect/frame 2.09 |
| 12 | `mary1` | `high` | 50.14 | 0.77% | 78.75% | 274.07% | 84 | candidate; rect/frame 2.33 |
| 13 | `activity9` | `high` | 40.8 | 2.19% | 50.66% | 560.7% | 72 | x-band rect cap pressure; needs selective bands; rect/frame 3.48 |
| 14 | `walkstuf3` | `low` | 35.23 | 1.13% | 77.8% | 581.73% | 60 | large upload-ready payload; needs compression/selective bands; rect/frame 3.21 |
| 15 | `activity9` | `low` | 32.99 | 1.80% | 50.66% | 560.7% | 57 | x-band rect cap pressure; needs selective bands; rect/frame 3.48 |
| 16 | `johnny6` | `high` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 17 | `johnny6` | `low` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 18 | `mary1` | `low` | 29.84 | 0.41% | 80.45% | 268.49% | 55 | candidate; rect/frame 1.61 |
| 19 | `johnny1` | `high` | 29.73 | 1.54% | 54.11% | 339.7% | 54 | candidate; rect/frame 1.5 |
| 20 | `johnny1` | `low` | 29.73 | 1.54% | 54.11% | 339.7% | 54 | candidate; rect/frame 1.5 |

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
