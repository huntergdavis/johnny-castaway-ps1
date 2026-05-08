# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `126`
- Average measured timing gap: `-0.3228%` over target
- Average estimated align4 x-band upload byte saving: `66.75%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `visitor3` | `low` | 98.92 | 9.85% | 55.86% | 650.97% | 170 | x-band rect cap pressure; needs selective bands; rect/frame 1.51 |
| 2 | `visitor3` | `high` | 87.77 | 8.75% | 55.86% | 692.86% | 150 | x-band rect cap pressure; needs selective bands; rect/frame 1.51 |
| 3 | `walkstuf1` | `high` | 58.95 | 4.56% | 45.7% | 668.56% | 117 | x-band rect cap pressure; needs selective bands; rect/frame 2.41 |
| 4 | `walkstuf1` | `low` | 56.58 | 4.34% | 45.7% | 668.56% | 113 | x-band rect cap pressure; needs selective bands; rect/frame 2.41 |
| 5 | `mary1` | `high` | 50.14 | 0.77% | 78.75% | 274.07% | 84 | candidate; rect/frame 2.33 |
| 6 | `building4` | `low` | 47.04 | 1.42% | 65.1% | 481.57% | 84 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 7 | `building2` | `low` | 43.2 | 2.51% | 67.28% | 589.07% | 84 | large upload-ready payload; needs compression/selective bands; rect/frame 2.47 |
| 8 | `walkstuf3` | `low` | 37.28 | 1.22% | 77.8% | 581.73% | 62 | large upload-ready payload; needs compression/selective bands; rect/frame 3.21 |
| 9 | `building2` | `high` | 37.2 | 2.51% | 67.28% | 589.07% | 60 | large upload-ready payload; needs compression/selective bands; rect/frame 2.47 |
| 10 | `building4` | `high` | 34.98 | 0.99% | 65.1% | 481.57% | 67 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 11 | `walkstuf3` | `high` | 32.49 | 0.87% | 81.22% | 565.34% | 65 | large upload-ready payload; needs compression/selective bands; rect/frame 2.09 |
| 12 | `building6` | `low` | 31.77 | 1.18% | 60.41% | 482.98% | 57 | x-band rect cap pressure; needs selective bands; rect/frame 1.63 |
| 13 | `johnny6` | `high` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 14 | `johnny6` | `low` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 15 | `mary1` | `low` | 29.84 | 0.41% | 80.45% | 268.49% | 55 | candidate; rect/frame 1.61 |
| 16 | `johnny1` | `high` | 29.73 | 1.54% | 54.11% | 339.7% | 54 | candidate; rect/frame 1.5 |
| 17 | `johnny1` | `low` | 29.73 | 1.54% | 54.11% | 339.7% | 54 | candidate; rect/frame 1.5 |
| 18 | `building6` | `high` | 28.1 | 1.02% | 60.41% | 482.98% | 52 | x-band rect cap pressure; needs selective bands; rect/frame 1.63 |
| 19 | `activity9` | `low` | 23.93 | 1.31% | 50.66% | 919.43% | 41 | x-band rect cap pressure; needs selective bands; rect/frame 3.48 |
| 20 | `visitor5` | `low` | 22.57 | 2.02% | 75.3% | 584.49% | 24 | large upload-ready payload; needs compression/selective bands; rect/frame 1.76 |

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
