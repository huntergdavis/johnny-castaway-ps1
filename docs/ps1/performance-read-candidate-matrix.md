# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/w1low-slack-frames87-99-five-yellow-current/20260521-085703-140832`
- Candidate rows: `47`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `0`
- Closed exact ranges from experiment log: `47`
- Phase-trap rows: `47`
- Deferred under-target rows: `0`
- Top next lanes: `frame-deadline-data-shape-or-render-reduction`=13, `custom-terminal-data-shape-or-generated-deadline`=12, `no-decode-canonicalization-or-generated-owner`=12, `generated-deadline-or-sector-split-data-shape`=10

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.
When direct standalone/guarded probes are exhausted, promote the listed
next lanes above more scalar range retries.

No open standalone or guarded direct-read probes remain in this
artifact set. The next optimization pass should start from generated
deadline ownership, custom data-shape, or pack-owned work reduction
lanes instead of another hand-authored sector range.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Phase Trap | Next Lane | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|---|---|
| 1 | `visitor3` | `low` | 1071/1039 | 80 | `248..272` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 2 | `visitor3` | `low` | 1071/1039 | 80 | `240..264` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 3 | `visitor3` | `low` | 1071/1039 | 80 | `239..263` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 4 | `visitor3` | `low` | 1071/1039 | 80 | `248..264` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 5 | `visitor3` | `low` | 1071/1039 | 80 | `9..33` (24s) | 2 | `balanced:validate-overlap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 6 | `visitor3` | `low` | 1071/1039 | 80 | `256..268` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 7 | `visitor3` | `low` | 1071/1039 | 80 | `256..272` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 8 | `visitor3` | `low` | 1071/1039 | 80 | `239..251` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 9 | `visitor3` | `low` | 1071/1039 | 80 | `248..260` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 10 | `visitor3` | `low` | 1071/1039 | 80 | `92..108` (16s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 11 | `visitor3` | `low` | 1071/1039 | 80 | `86..98` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 12 | `visitor3` | `low` | 1071/1039 | 80 | `9..25` (16s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 13 | `walkstuf1` | `high` | 1472/1441 | 43 | `84..108` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 14 | `walkstuf1` | `high` | 1472/1441 | 43 | `365..389` (24s) | 3 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 15 | `walkstuf1` | `high` | 1472/1441 | 43 | `74..98` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 16 | `walkstuf1` | `high` | 1472/1441 | 43 | `92..116` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 17 | `walkstuf1` | `high` | 1472/1441 | 43 | `372..388` (16s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 18 | `walkstuf1` | `high` | 1472/1441 | 43 | `379..395` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 19 | `walkstuf1` | `high` | 1472/1441 | 43 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 20 | `walkstuf1` | `high` | 1472/1441 | 43 | `92..108` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 21 | `walkstuf1` | `high` | 1472/1441 | 43 | `84..100` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 22 | `walkstuf1` | `high` | 1472/1441 | 43 | `268..280` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 23 | `walkstuf1` | `high` | 1472/1441 | 43 | `74..86` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 24 | `walkstuf1` | `high` | 1472/1441 | 43 | `379..391` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 25 | `building2` | `high` | 1341/1313 | 47 | `135..159` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 26 | `building2` | `high` | 1341/1313 | 47 | `90..114` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 27 | `building2` | `high` | 1341/1313 | 47 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 28 | `building2` | `high` | 1341/1313 | 47 | `104..128` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 29 | `building2` | `high` | 1341/1313 | 47 | `249..265` (16s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 30 | `building2` | `high` | 1341/1313 | 47 | `318..334` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 31 | `building2` | `high` | 1341/1313 | 47 | `310..326` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 32 | `building2` | `high` | 1341/1313 | 47 | `82..98` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 33 | `building2` | `high` | 1341/1313 | 47 | `249..261` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 34 | `building2` | `high` | 1341/1313 | 47 | `90..96` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 35 | `building2` | `high` | 1341/1313 | 47 | `310..322` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 36 | `building2` | `high` | 1341/1313 | 47 | `318..330` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 37 | `building2` | `high` | 1341/1313 | 47 | `90..102` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 38 | `walkstuf1` | `low` | 1470/1446 | 32 | `129..153` (24s) | 3 | `risky:short-visible-gap` | `closed-exact-range` | `generated-deadline-or-sector-split-data-shape` | `closed-by-experiment-log` |
| 39 | `walkstuf1` | `low` | 1470/1446 | 32 | `136..160` (24s) | 3 | `risky:short-visible-gap` | `closed-exact-range` | `generated-deadline-or-sector-split-data-shape` | `closed-by-experiment-log` |
| 40 | `walkstuf1` | `low` | 1470/1446 | 32 | `153..177` (24s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `generated-deadline-or-sector-split-data-shape` | `closed-by-experiment-log` |

## CSV

The full row-level matrix is in
`docs/ps1/performance-read-candidate-matrix.csv`.

## Columns

- `recommendation=standalone-probe` means the candidate has saved reads,
  fits the current group window, and has a `safe` visible cost class.
- `recommendation=scheduler-or-guarded-probe` means the candidate is
  balanced but still needs either a fresh paired gate or scheduler guard.
- `recommendation=scheduler-owned-only` means prior misses say a raw table
  is too risky; use generated metadata with explicit CD ownership.
- `recommendation=closed-by-experiment-log` means the exact sector range
  already appears in a failed or rejected experiment row.
- `recommendation=defer-under-target` means the source scene is already
  under its current active-loop target.
- `phase_trap=yes` marks rows whose exact range is closed, whose visible
  gap is too tight, or whose prior risk class says scheduler ownership
  is required before the read can fire safely.
- `next_lane` is the non-scalar lane to try before another local sector
  table retry for that row.
- `artifact` points back to the source read-plan JSON for full read
  segments, gaps, and coverage.
