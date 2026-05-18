# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/current-yellow-readplans-20260518-b2high-rg271-287`
- Candidate rows: `64`
- Standalone probes: `0`
- Scheduler or guarded probes: `1`
- Scheduler-owned only: `45`
- Closed exact ranges from experiment log: `18`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `building2` | `high` | 1347/1313 | 41 | `315..327` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 2 | `walkstuf1` | `low` | 1480/1442 | 55 | `91..115` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `low` | 1480/1442 | 55 | `353..369` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 4 | `walkstuf1` | `low` | 1480/1442 | 55 | `98..114` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `low` | 1480/1442 | 55 | `172..184` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 6 | `walkstuf1` | `low` | 1480/1442 | 55 | `396..408` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 7 | `walkstuf1` | `low` | 1480/1442 | 55 | `159..165` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `building4` | `low` | 2853/2816 | 42 | `274..298` (24s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 9 | `visitor3` | `low` | 1074/1039 | 85 | `248..272` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `visitor3` | `low` | 1074/1039 | 85 | `240..264` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `visitor3` | `low` | 1074/1039 | 85 | `239..263` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `visitor3` | `low` | 1074/1039 | 85 | `231..255` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `visitor3` | `low` | 1074/1039 | 85 | `248..264` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `visitor3` | `low` | 1074/1039 | 85 | `256..268` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `visitor3` | `low` | 1074/1039 | 85 | `256..272` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `visitor3` | `low` | 1074/1039 | 85 | `240..256` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `visitor3` | `low` | 1074/1039 | 85 | `239..251` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `visitor3` | `low` | 1074/1039 | 85 | `248..260` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `visitor3` | `low` | 1074/1039 | 85 | `239..255` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `visitor3` | `low` | 1074/1039 | 85 | `231..243` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `walkstuf1` | `high` | 1472/1438 | 60 | `84..108` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `walkstuf1` | `high` | 1472/1438 | 60 | `345..369` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `walkstuf1` | `high` | 1472/1438 | 60 | `164..188` (24s) | 3 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 24 | `walkstuf1` | `high` | 1472/1438 | 60 | `156..172` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 25 | `walkstuf1` | `high` | 1472/1438 | 60 | `149..165` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 26 | `walkstuf1` | `high` | 1472/1438 | 60 | `99..115` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `walkstuf1` | `high` | 1472/1438 | 60 | `357..369` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `walkstuf1` | `high` | 1472/1438 | 60 | `171..183` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 29 | `building2` | `high` | 1347/1313 | 41 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `building2` | `high` | 1347/1313 | 41 | `95..119` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `building2` | `high` | 1347/1313 | 41 | `140..164` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 32 | `building2` | `high` | 1347/1313 | 41 | `158..182` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 33 | `building2` | `high` | 1347/1313 | 41 | `308..324` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 34 | `building2` | `high` | 1347/1313 | 41 | `255..271` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 35 | `building2` | `high` | 1347/1313 | 41 | `249..265` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 36 | `building2` | `high` | 1347/1313 | 41 | `321..337` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 37 | `building2` | `high` | 1347/1313 | 41 | `95..107` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 38 | `building2` | `low` | 1339/1316 | 53 | `153..177` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 39 | `building2` | `low` | 1339/1316 | 53 | `145..169` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 40 | `building2` | `low` | 1339/1316 | 53 | `141..165` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |

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
