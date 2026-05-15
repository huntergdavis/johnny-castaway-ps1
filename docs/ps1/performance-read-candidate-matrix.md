# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/current-yellow-readplans-20260515-1120`
- Candidate rows: `63`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `20`
- Closed exact ranges from experiment log: `43`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 30 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `walkstuf1` | `low` | 1481/1431 | 72 | `289..313` (24s) | 6 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 2 | `walkstuf1` | `low` | 1481/1431 | 72 | `238..262` (24s) | 5 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `low` | 1481/1431 | 72 | `283..307` (24s) | 5 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 4 | `walkstuf1` | `low` | 1481/1431 | 72 | `283..299` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `low` | 1481/1431 | 72 | `250..262` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 6 | `walkstuf1` | `low` | 1481/1431 | 72 | `238..250` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 7 | `walkstuf1` | `low` | 1481/1431 | 72 | `256..262` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `walkstuf1` | `low` | 1481/1431 | 72 | `289..295` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `building4` | `low` | 2853/2816 | 40 | `262..286` (24s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 10 | `building4` | `low` | 2853/2816 | 40 | `274..298` (24s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 11 | `building2` | `low` | 1339/1317 | 53 | `104..128` (24s) | 5 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `building2` | `low` | 1339/1317 | 53 | `100..124` (24s) | 5 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `building2` | `low` | 1339/1317 | 53 | `89..113` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `building2` | `low` | 1339/1317 | 53 | `104..120` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `building2` | `low` | 1339/1317 | 53 | `137..153` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `building2` | `low` | 1339/1317 | 53 | `112..128` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `building2` | `low` | 1339/1317 | 53 | `96..108` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `building2` | `low` | 1339/1317 | 53 | `104..116` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `building2` | `low` | 1339/1317 | 53 | `137..149` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `building2` | `low` | 1339/1317 | 53 | `158..170` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `walkstuf1` | `low` | 1481/1431 | 72 | `297..321` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 22 | `walkstuf1` | `low` | 1481/1431 | 72 | `297..313` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 23 | `walkstuf1` | `low` | 1481/1431 | 72 | `305..321` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 24 | `walkstuf1` | `low` | 1481/1431 | 72 | `225..241` (16s) | 2 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 25 | `walkstuf1` | `low` | 1481/1431 | 72 | `283..295` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 26 | `walkstuf1` | `low` | 1481/1431 | 72 | `410..422` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 27 | `walkstuf1` | `low` | 1481/1431 | 72 | `291..297` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 28 | `walkstuf1` | `low` | 1481/1431 | 72 | `305..311` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 29 | `walkstuf1` | `high` | 1476/1434 | 81 | `298..322` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 30 | `walkstuf1` | `high` | 1476/1434 | 81 | `287..311` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |

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
- `artifact` points back to the source read-plan JSON for full read
  segments, gaps, and coverage.
