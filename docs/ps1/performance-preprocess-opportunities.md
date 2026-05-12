# PS1 Foreground Preprocess Opportunity Matrix

Status: generated host-side planning sheet

This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics
preprocessing experiments. It does not change runtime behavior or the
accepted performance baseline.

- Source CSV: `docs/ps1/performance-preprocess-opportunities.csv`
- Measured timing rows included: `126`
- Average measured timing gap: `-0.4344%` over target
- Average estimated align4 x-band upload byte saving: `66.52%`

## Top Upload-Ready Candidates

| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | `walkstuf1` | `high` | 51.05 | 3.57% | 45.7% | 669.5% | 111 | x-band rect cap pressure; needs selective bands; rect/frame 2.41 |
| 2 | `mary1` | `high` | 50.14 | 0.77% | 78.75% | 274.07% | 84 | candidate; rect/frame 2.33 |
| 3 | `walkstuf1` | `low` | 49.18 | 3.78% | 45.7% | 671.35% | 98 | x-band rect cap pressure; needs selective bands; rect/frame 2.41 |
| 4 | `building4` | `low` | 47.04 | 1.42% | 65.1% | 481.57% | 84 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 5 | `building2` | `high` | 46.58 | 3.13% | 67.28% | 589.07% | 76 | large upload-ready payload; needs compression/selective bands; rect/frame 2.47 |
| 6 | `building2` | `low` | 41.36 | 2.35% | 67.28% | 589.07% | 82 | large upload-ready payload; needs compression/selective bands; rect/frame 2.47 |
| 7 | `visitor3` | `low` | 38.26 | 3.75% | 55.14% | 960.32% | 67 | x-band rect cap pressure; needs selective bands; rect/frame 1.6 |
| 8 | `building4` | `high` | 34.98 | 0.99% | 65.1% | 481.57% | 67 | x-band rect cap pressure; needs selective bands; rect/frame 1.7 |
| 9 | `walkstuf3` | `high` | 32.49 | 0.87% | 81.22% | 565.34% | 65 | large upload-ready payload; needs compression/selective bands; rect/frame 2.09 |
| 10 | `building6` | `low` | 31.77 | 1.18% | 60.41% | 482.98% | 57 | x-band rect cap pressure; needs selective bands; rect/frame 1.63 |
| 11 | `johnny6` | `high` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 12 | `johnny6` | `low` | 31.56 | 1.14% | 54.86% | 310.08% | 56 | candidate; rect/frame 1.52 |
| 13 | `mary1` | `low` | 29.84 | 0.41% | 80.45% | 268.49% | 55 | candidate; rect/frame 1.61 |
| 14 | `visitor3` | `high` | 29.83 | 2.98% | 54.3% | 967.84% | 52 | x-band rect cap pressure; needs selective bands; rect/frame 1.64 |
| 15 | `johnny1` | `low` | 28.69 | 1.49% | 54.11% | 519.48% | 52 | large upload-ready payload; needs compression/selective bands; rect/frame 1.5 |
| 16 | `building6` | `high` | 28.1 | 1.02% | 60.41% | 482.98% | 52 | x-band rect cap pressure; needs selective bands; rect/frame 1.63 |
| 17 | `johnny1` | `high` | 27.65 | 1.44% | 54.11% | 519.48% | 50 | large upload-ready payload; needs compression/selective bands; rect/frame 1.5 |
| 18 | `walkstuf3` | `low` | 23.2 | 0.65% | 82.98% | 540.22% | 43 | large upload-ready payload; needs compression/selective bands; rect/frame 1.44 |
| 19 | `visitor5` | `low` | 22.57 | 2.02% | 75.3% | 584.49% | 24 | large upload-ready payload; needs compression/selective bands; rect/frame 1.76 |
| 20 | `suzy2` | `high` | 20.55 | 0.84% | 50.21% | 753.65% | 38 | large upload-ready payload; needs compression/selective bands; rect/frame 1.9 |

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
