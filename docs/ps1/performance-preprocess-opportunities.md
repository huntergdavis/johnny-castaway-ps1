# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `1.0272%` over target
- Average estimated align4 x-band upload byte saving: `67.13%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `visitor3` | `low` | 349.97 | 44.00% | 56.97% | 229.28% | 388 | candidate |
| 2 | `visitor3` | `high` | 349.54 | 44.06% | 56.97% | 229.28% | 384 | candidate |
| 3 | `building4` | `high` | 221.36 | 7.61% | 65.1% | 191.66% | 336 | candidate |
| 4 | `building2` | `high` | 216.5 | 14.86% | 67.28% | 258.68% | 352 | candidate |
| 5 | `building4` | `low` | 207.74 | 7.08% | 65.1% | 191.66% | 318 | candidate |
| 6 | `building2` | `low` | 207.56 | 14.63% | 67.28% | 258.68% | 327 | candidate |
| 7 | `walkstuf1` | `low` | 199.19 | 15.97% | 46.64% | 493.1% | 377 | candidate |
| 8 | `walkstuf1` | `high` | 196.88 | 16.10% | 46.64% | 493.1% | 364 | candidate |
| 9 | `activity9` | `low` | 155.17 | 8.20% | 63.79% | 584.63% | 192 | large upload-ready payload; needs compression/selective bands |
| 10 | `activity9` | `high` | 129.61 | 7.02% | 63.79% | 584.63% | 151 | large upload-ready payload; needs compression/selective bands |
| 11 | `activity4` | `low` | 113.41 | 2.99% | 72.37% | 485.6% | 361 | candidate |
| 12 | `fishing4` | `low` | 76.97 | 3.20% | 52.66% | 975.37% | 251 | large upload-ready payload; needs compression/selective bands |
| 13 | `building6` | `low` | 68.45 | 3.20% | 44.17% | 810.26% | 136 | large upload-ready payload; needs compression/selective bands |
| 14 | `building6` | `high` | 65.95 | 3.19% | 44.17% | 810.26% | 126 | large upload-ready payload; needs compression/selective bands |
| 15 | `walkstuf3` | `high` | 60.93 | 1.89% | 81.22% | 251.74% | 104 | candidate |
| 16 | `mary1` | `high` | 50.14 | 0.77% | 78.75% | 274.07% | 84 | candidate |
| 17 | `walkstuf3` | `low` | 35.23 | 1.13% | 77.8% | 581.73% | 60 | large upload-ready payload; needs compression/selective bands |
| 18 | `johnny6` | `high` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate |
| 19 | `johnny6` | `low` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate |
| 20 | `mary1` | `low` | 29.84 | 0.41% | 80.45% | 268.49% | 55 | candidate |

## Read Before Acting

- High upload-byte savings are not automatically promotable; naive direct16
  expansion already regressed WALKSTUF1 low by adding too much CD pressure.
- Rows with large payload growth need selective bands, compression, or setup
  residency before runtime promotion.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
