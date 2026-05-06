# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `1.1131%` over target
- Average estimated align4 x-band upload byte saving: `67.13%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `visitor3` | `low` | 349.97 | 44.00% | 56.97% | 229.28% | 388 | candidate |
| 2 | `visitor3` | `high` | 349.54 | 44.06% | 56.97% | 229.28% | 384 | candidate |
| 3 | `building2` | `high` | 216.5 | 14.86% | 67.28% | 258.68% | 352 | candidate |
| 4 | `building2` | `low` | 207.56 | 14.63% | 67.28% | 258.68% | 327 | candidate |
| 5 | `walkstuf1` | `low` | 199.19 | 15.97% | 46.64% | 493.1% | 377 | candidate |
| 6 | `walkstuf1` | `high` | 196.88 | 16.10% | 46.64% | 493.1% | 364 | candidate |
| 7 | `activity9` | `low` | 155.17 | 8.20% | 63.79% | 584.63% | 192 | large upload-ready payload; needs compression/selective bands |
| 8 | `building4` | `high` | 153.03 | 4.87% | 65.1% | 191.66% | 258 | candidate |
| 9 | `building4` | `low` | 129.83 | 4.58% | 65.1% | 191.66% | 186 | candidate |
| 10 | `activity9` | `high` | 129.61 | 7.02% | 63.79% | 584.63% | 151 | large upload-ready payload; needs compression/selective bands |
| 11 | `building6` | `high` | 116.28 | 5.26% | 44.17% | 810.26% | 239 | large upload-ready payload; needs compression/selective bands |
| 12 | `building6` | `low` | 114.78 | 5.25% | 44.17% | 810.26% | 233 | large upload-ready payload; needs compression/selective bands |
| 13 | `activity4` | `low` | 113.41 | 2.99% | 72.37% | 485.6% | 361 | candidate |
| 14 | `activity1` | `high` | 93.92 | 3.06% | 71.86% | 809.77% | 2 | large upload-ready payload; needs compression/selective bands |
| 15 | `activity1` | `low` | 93.92 | 3.06% | 71.86% | 809.77% | 2 | large upload-ready payload; needs compression/selective bands |
| 16 | `fishing4` | `low` | 76.97 | 3.20% | 52.66% | 975.37% | 251 | large upload-ready payload; needs compression/selective bands |
| 17 | `johnny6` | `low` | 66.17 | 3.43% | 54.86% | 310.08% | 54 | candidate |
| 18 | `johnny6` | `high` | 65.62 | 3.39% | 54.86% | 310.08% | 54 | candidate |
| 19 | `walkstuf3` | `high` | 60.93 | 1.89% | 81.22% | 251.74% | 104 | candidate |
| 20 | `mary1` | `high` | 50.14 | 0.77% | 78.75% | 274.07% | 84 | candidate |

## Read Before Acting

- High upload-byte savings are not automatically promotable; naive direct16
  expansion already regressed WALKSTUF1 low by adding too much CD pressure.
- Rows with large payload growth need selective bands, compression, or setup
  residency before runtime promotion.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
