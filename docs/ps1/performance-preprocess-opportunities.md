# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `3.1300%` over target
- Average estimated align4 x-band upload byte saving: `67.13%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `visitor3` | `low` | 349.97 | 44.00% | 56.97% | 229.28% | 388 | candidate |
| 2 | `visitor3` | `high` | 349.54 | 44.06% | 56.97% | 229.28% | 384 | candidate |
| 3 | `activity10` | `low` | 297.98 | 14.37% | 72.92% | 592.31% | 664 | large upload-ready payload; needs compression/selective bands |
| 4 | `activity10` | `high` | 277.47 | 12.84% | 75.29% | 499.86% | 622 | candidate |
| 5 | `building2` | `high` | 217.99 | 19.75% | 67.28% | 258.68% | 183 | candidate |
| 6 | `building2` | `low` | 207.08 | 18.89% | 67.28% | 258.68% | 169 | candidate |
| 7 | `johnny3` | `high` | 206.0 | 7.89% | 71.74% | 463.91% | 560 | candidate |
| 8 | `walkstuf1` | `low` | 199.19 | 15.97% | 46.64% | 493.1% | 377 | candidate |
| 9 | `walkstuf1` | `high` | 196.88 | 16.10% | 46.64% | 493.1% | 364 | candidate |
| 10 | `activity9` | `low` | 183.44 | 11.15% | 63.79% | 584.63% | 152 | large upload-ready payload; needs compression/selective bands |
| 11 | `walkstuf3` | `high` | 180.45 | 8.08% | 81.22% | 251.74% | 124 | candidate |
| 12 | `activity9` | `high` | 169.48 | 10.36% | 63.79% | 584.63% | 137 | large upload-ready payload; needs compression/selective bands |
| 13 | `walkstuf3` | `low` | 166.81 | 7.92% | 77.8% | 581.73% | 104 | large upload-ready payload; needs compression/selective bands |
| 14 | `johnny3` | `low` | 163.18 | 5.40% | 74.89% | 463.95% | 464 | candidate |
| 15 | `mary1` | `high` | 162.42 | 3.69% | 78.75% | 274.07% | 89 | candidate |
| 16 | `building7` | `high` | 155.28 | 4.77% | 79.16% | 319.45% | 67 | candidate |
| 17 | `building4` | `high` | 153.03 | 4.87% | 65.1% | 191.66% | 258 | candidate |
| 18 | `building5` | `high` | 141.9 | 5.04% | 73.0% | 558.04% | 77 | large upload-ready payload; needs compression/selective bands |
| 19 | `mary1` | `low` | 136.19 | 3.20% | 80.45% | 268.49% | 46 | candidate |
| 20 | `activity5` | `high` | 131.7 | 9.37% | 76.53% | 677.37% | 37 | large upload-ready payload; needs compression/selective bands |

## Read Before Acting

- High upload-byte savings are not automatically promotable; naive direct16
  expansion already regressed WALKSTUF1 low by adding too much CD pressure.
- Rows with large payload growth need selective bands, compression, or setup
  residency before runtime promotion.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
