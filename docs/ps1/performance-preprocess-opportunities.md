# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `2.4134%` over target
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
| 7 | `mary1` | `high` | 162.42 | 3.69% | 78.75% | 274.07% | 89 | candidate |
| 8 | `building7` | `high` | 155.28 | 4.77% | 79.16% | 319.45% | 67 | candidate |
| 9 | `activity9` | `low` | 155.17 | 8.20% | 63.79% | 584.63% | 192 | large upload-ready payload; needs compression/selective bands |
| 10 | `building4` | `high` | 153.03 | 4.87% | 65.1% | 191.66% | 258 | candidate |
| 11 | `building5` | `high` | 141.9 | 5.04% | 73.0% | 558.04% | 77 | large upload-ready payload; needs compression/selective bands |
| 12 | `mary1` | `low` | 136.19 | 3.20% | 80.45% | 268.49% | 46 | candidate |
| 13 | `activity5` | `high` | 131.7 | 9.37% | 76.53% | 677.37% | 37 | large upload-ready payload; needs compression/selective bands |
| 14 | `building4` | `low` | 129.83 | 4.58% | 65.1% | 191.66% | 186 | candidate |
| 15 | `activity9` | `high` | 129.61 | 7.02% | 63.79% | 584.63% | 151 | large upload-ready payload; needs compression/selective bands |
| 16 | `visitor6` | `high` | 128.36 | 7.49% | 79.65% | 832.92% | 26 | large upload-ready payload; needs compression/selective bands |
| 17 | `building7` | `low` | 127.91 | 4.19% | 79.16% | 319.45% | 24 | candidate |
| 18 | `activity5` | `low` | 126.59 | 8.96% | 76.53% | 677.37% | 38 | large upload-ready payload; needs compression/selective bands |
| 19 | `building6` | `high` | 116.28 | 5.26% | 44.17% | 810.26% | 239 | large upload-ready payload; needs compression/selective bands |
| 20 | `building5` | `low` | 116.26 | 4.48% | 73.0% | 558.04% | 27 | large upload-ready payload; needs compression/selective bands |

## Read Before Acting

- High upload-byte savings are not automatically promotable; naive direct16
  expansion already regressed WALKSTUF1 low by adding too much CD pressure.
- Rows with large payload growth need selective bands, compression, or setup
  residency before runtime promotion.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
