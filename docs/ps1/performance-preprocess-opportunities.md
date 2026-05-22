# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `126`
- Average measured timing gap: `-0.4935%` over target
- Average estimated align4 x-band upload byte saving: `66.56%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `mary1` | `high` | 46.77 | 0.70% | 78.75% | 274.07% | 80 | candidate; rect/frame 2.33 |
| 2 | `building4` | `high` | 33.58 | 0.96% | 65.1% | 481.57% | 64 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 3 | `building4` | `low` | 32.15 | 0.96% | 64.45% | 541.09% | 59 | x-band rect cap pressure; needs selective bands; rect/frame 1.71 |
| 4 | `building2` | `high` | 32.14 | 1.98% | 68.82% | 618.61% | 57 | large upload-ready payload; needs compression/selective bands; rect/frame 2.33 |
| 5 | `walkstuf3` | `high` | 29.06 | 0.74% | 81.22% | 565.34% | 61 | large upload-ready payload; needs compression/selective bands; rect/frame 2.09 |
| 6 | `johnny6` | `high` | 27.86 | 1.00% | 54.86% | 436.4% | 50 | candidate; rect/frame 1.52 |
| 7 | `johnny6` | `low` | 27.86 | 1.00% | 54.86% | 436.4% | 50 | candidate; rect/frame 1.52 |
| 8 | `walkstuf1` | `high` | 27.65 | 2.08% | 45.52% | 748.41% | 56 | x-band rect cap pressure; needs selective bands; rect/frame 2.43 |
| 9 | `visitor3` | `low` | 27.22 | 2.31% | 56.11% | 942.71% | 55 | x-band rect cap pressure; needs selective bands; rect/frame 1.85 |
| 10 | `suzy1` | `high` | 26.12 | 0.56% | 37.86% | 893.33% | 56 | large upload-ready payload; needs compression/selective bands; rect/frame 1.93 |
| 11 | `suzy1` | `low` | 24.74 | 0.54% | 37.86% | 893.33% | 52 | large upload-ready payload; needs compression/selective bands; rect/frame 1.93 |
| 12 | `fishing3` | `high` | 24.53 | 0.87% | 63.39% | 486.85% | 55 | candidate; rect/frame 1.97 |
| 13 | `building6` | `high` | 24.44 | 0.86% | 60.41% | 482.98% | 47 | x-band rect cap pressure; needs selective bands; rect/frame 1.63 |
| 14 | `walkstuf3` | `low` | 24.28 | 0.70% | 82.98% | 540.22% | 44 | large upload-ready payload; needs compression/selective bands; rect/frame 1.44 |
| 15 | `mary1` | `low` | 22.32 | 0.31% | 80.45% | 268.49% | 41 | candidate; rect/frame 1.61 |
| 16 | `visitor3` | `high` | 20.45 | 2.11% | 56.61% | 947.36% | 32 | x-band rect cap pressure; needs selective bands; rect/frame 1.88 |
| 17 | `building6` | `low` | 18.81 | 0.61% | 60.41% | 482.98% | 39 | x-band rect cap pressure; needs selective bands; rect/frame 1.63 |
| 18 | `suzy2` | `high` | 18.04 | 0.72% | 50.21% | 753.65% | 34 | large upload-ready payload; needs compression/selective bands; rect/frame 1.9 |
| 19 | `suzy2` | `low` | 18.04 | 0.72% | 50.21% | 753.65% | 34 | large upload-ready payload; needs compression/selective bands; rect/frame 1.9 |
| 20 | `building2` | `low` | 17.8 | 0.68% | 67.28% | 763.64% | 47 | large upload-ready payload; needs compression/selective bands; rect/frame 2.48 |

## Read Before Acting

- High upload-byte savings are not automatically promotable; naive direct16
  expansion already regressed WALKSTUF1 low by adding too much CD pressure.
- Rows with large payload growth need selective bands, compression, or setup
  residency before runtime promotion.
- Use `scripts/analyze-fg2-preprocess-plans.py --hotspot-count N` on the
  selected pack before a runtime probe. VISITOR3 now proves why: cap-hit frames
  `114`, `134..136`, and `141..142` save `0%` under blanket x-band, while nearby non-cap frames carry
  most of the useful byte saving.
- The current VISITOR3 detail sheet is
  `docs/ps1/performance-preprocess-visitor3-hotspots.csv`. Its default
  threshold plan selects `92 / 144` frames, excludes `6` cap-hit frames, and
  estimates `5730024` selected-subset upload bytes saved.
- The current VISITOR3 default selective upload-ready footprint does not fit
  as a layout-neutral append: it needs `2111224` payload+rect bytes for high tide
  against `970076` bytes of padded zero-tail slack. The next runtime probe
  needs the now-modeled smaller subset, compression, or an explicit
  layout-moving experiment.
- The same-footprint VISITOR3 budgeted subset now has an exact analyzer
  target: `78 / 92` default-selected high-tide frames fit in `968904` payload+rect
  bytes, leaving `1172` bytes of slack and retaining `4232112` modeled
  upload bytes saved (`73.9%` of the default plan's savings).
- The raw pack-emitted upload-ready lane is blocked for VISITOR3 under
  the current FGP3 data: `0` selected x-band bytes are fully covered by
  current opaque draw spans, so the modeled win depends on restored
  background/cleanup pixels that are dynamic at runtime.
- This matrix should guide the next generated pack-format experiment, not
  more hand-authored scene branches.
