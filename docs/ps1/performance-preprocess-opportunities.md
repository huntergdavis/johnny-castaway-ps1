# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `13.8570%` over target
- Average estimated align4 x-band upload byte saving: `66.69%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `walkstuf1` | `low` | 389.38 | 30.09% | 57.34% | 266.05% | 592 | candidate |
| 2 | `walkstuf1` | `high` | 380.86 | 29.54% | 57.5% | 248.48% | 562 | candidate |
| 3 | `visitor3` | `high` | 350.11 | 40.48% | 58.91% | 256.32% | 439 | candidate |
| 4 | `visitor3` | `low` | 322.05 | 37.24% | 58.94% | 253.43% | 397 | candidate |
| 5 | `building2` | `high` | 223.19 | 19.75% | 69.31% | 353.21% | 183 | candidate |
| 6 | `building2` | `low` | 212.06 | 18.89% | 69.31% | 353.21% | 169 | candidate |
| 7 | `walkstuf3` | `high` | 180.45 | 8.08% | 81.22% | 251.74% | 124 | candidate |
| 8 | `visitor5` | `high` | 167.1 | 17.64% | 72.69% | 235.82% | 113 | candidate |
| 9 | `walkstuf3` | `low` | 166.81 | 7.92% | 77.8% | 581.73% | 104 | large upload-ready payload; needs compression/selective bands |
| 10 | `mary1` | `high` | 162.42 | 3.69% | 78.75% | 274.07% | 89 | candidate |
| 11 | `activity9` | `low` | 161.33 | 11.15% | 54.09% | 534.13% | 152 | large upload-ready payload; needs compression/selective bands |
| 12 | `building4` | `high` | 158.53 | 4.87% | 69.14% | 230.27% | 258 | candidate |
| 13 | `building7` | `high` | 158.43 | 4.77% | 80.96% | 254.64% | 67 | candidate |
| 14 | `building5` | `high` | 156.73 | 5.04% | 81.84% | 245.32% | 77 | candidate |
| 15 | `building6` | `high` | 142.88 | 5.26% | 64.94% | 229.02% | 239 | candidate |
| 16 | `building6` | `low` | 141.38 | 5.25% | 64.94% | 229.02% | 233 | candidate |
| 17 | `activity9` | `high` | 138.72 | 10.36% | 49.28% | 630.14% | 137 | large upload-ready payload; needs compression/selective bands |
| 18 | `mary1` | `low` | 136.19 | 3.20% | 80.45% | 268.49% | 46 | candidate |
| 19 | `building7` | `low` | 135.41 | 4.19% | 84.03% | 226.85% | 24 | candidate |
| 20 | `building4` | `low` | 135.0 | 4.58% | 69.14% | 230.27% | 186 | candidate |

## Read Before Acting

- High upload-byte savings are not automatically promotable; naive direct16
  expansion already regressed WALKSTUF1 low by adding too much CD pressure.
- Rows with large payload growth need selective bands, compression, or setup
  residency before runtime promotion.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
