# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `1.7731%` over target
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
| 14 | `activity12` | `low` | 98.42 | 9.36% | 72.29% | 537.34% | 12 | large upload-ready payload; needs compression/selective bands |
| 15 | `fishing2` | `high` | 97.04 | 7.59% | 71.3% | 407.97% | 6 | candidate |
| 16 | `activity12` | `high` | 95.58 | 9.05% | 74.67% | 448.43% | 0 | candidate |
| 17 | `building3` | `high` | 95.46 | 9.44% | 68.86% | 1309.63% | 10 | large upload-ready payload; needs compression/selective bands |
| 18 | `activity11` | `high` | 94.7 | 7.77% | 70.67% | 885.94% | 0 | large upload-ready payload; needs compression/selective bands |
| 19 | `activity11` | `low` | 94.7 | 7.77% | 70.67% | 885.94% | 0 | large upload-ready payload; needs compression/selective bands |
| 20 | `activity1` | `high` | 93.92 | 3.06% | 71.86% | 809.77% | 2 | large upload-ready payload; needs compression/selective bands |

## Read Before Acting

- High upload-byte savings are not automatically promotable; naive direct16
  expansion already regressed WALKSTUF1 low by adding too much CD pressure.
- Rows with large payload growth need selective bands, compression, or setup
  residency before runtime promotion.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
