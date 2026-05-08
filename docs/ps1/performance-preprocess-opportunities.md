# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `120`
- Average measured timing gap: `-0.0694%` over target
- Average estimated align4 x-band upload byte saving: `67.59%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `walkstuf1` | `low` | 173.13 | 14.00% | 46.64% | 493.1% | 325 | x-band rect cap pressure; needs selective bands; rect/frame 2.85 |
| 2 | `walkstuf1` | `high` | 168.25 | 13.23% | 46.64% | 493.1% | 326 | x-band rect cap pressure; needs selective bands; rect/frame 2.85 |
| 3 | `visitor3` | `high` | 110.62 | 11.04% | 55.86% | 610.99% | 190 | x-band rect cap pressure; needs selective bands; rect/frame 1.51 |
| 4 | `visitor3` | `low` | 108.0 | 10.84% | 55.86% | 610.99% | 184 | x-band rect cap pressure; needs selective bands; rect/frame 1.51 |
| 5 | `building2` | `low` | 83.9 | 6.06% | 67.28% | 488.66% | 123 | candidate; rect/frame 2.47 |
| 6 | `building6` | `low` | 68.45 | 3.20% | 44.17% | 810.26% | 136 | x-band rect cap pressure; needs selective bands; rect/frame 2.52 |
| 7 | `building6` | `high` | 65.95 | 3.19% | 44.17% | 810.26% | 126 | x-band rect cap pressure; needs selective bands; rect/frame 2.52 |
| 8 | `walkstuf3` | `high` | 60.93 | 1.89% | 81.22% | 251.74% | 104 | candidate; rect/frame 2.09 |
| 9 | `mary1` | `high` | 50.14 | 0.77% | 78.75% | 274.07% | 84 | candidate; rect/frame 2.33 |
| 10 | `building4` | `low` | 47.04 | 1.42% | 65.1% | 481.57% | 84 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 11 | `building2` | `high` | 37.2 | 2.51% | 67.28% | 589.07% | 60 | large upload-ready payload; needs compression/selective bands; rect/frame 2.47 |
| 12 | `walkstuf3` | `low` | 35.23 | 1.13% | 77.8% | 581.73% | 60 | large upload-ready payload; needs compression/selective bands; rect/frame 3.21 |
| 13 | `building4` | `high` | 34.98 | 0.99% | 65.1% | 481.57% | 67 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 14 | `activity9` | `high` | 34.25 | 1.85% | 50.66% | 560.7% | 60 | x-band rect cap pressure; needs selective bands; rect/frame 3.48 |
| 15 | `johnny6` | `high` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 16 | `johnny6` | `low` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 17 | `mary1` | `low` | 29.84 | 0.41% | 80.45% | 268.49% | 55 | candidate; rect/frame 1.61 |
| 18 | `johnny1` | `high` | 29.73 | 1.54% | 54.11% | 339.7% | 54 | candidate; rect/frame 1.5 |
| 19 | `johnny1` | `low` | 29.73 | 1.54% | 54.11% | 339.7% | 54 | candidate; rect/frame 1.5 |
| 20 | `activity9` | `low` | 23.93 | 1.31% | 50.66% | 919.43% | 41 | x-band rect cap pressure; needs selective bands; rect/frame 3.48 |

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
- The current VISITOR3 default selective upload-ready footprint does not fit
  as a layout-neutral append: it needs `2462072` payload+rect bytes per pack
  against `814847` bytes of padded zero-tail slack. The next runtime probe
  needs the now-modeled smaller subset, compression, or an explicit
  layout-moving experiment.
- The same-footprint VISITOR3 budgeted subset now has an exact analyzer
  target: `74 / 96` default-selected frames fit in `814184` payload+rect
  bytes, leaving `663` bytes of slack and retaining `3858104` modeled
  upload bytes saved (`63.1%` of the default plan's savings).
- The raw pack-emitted upload-ready lane is blocked for VISITOR3 under
  the current FGP3 data: `0` selected x-band bytes are fully covered by
  current opaque draw spans, so the modeled win depends on restored
  background/cleanup pixels that are dynamic at runtime.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
