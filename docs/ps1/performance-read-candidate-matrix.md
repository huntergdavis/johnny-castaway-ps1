# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/current-under-green-candidates-20260518-v3tight64`
- Candidate rows: `64`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `34`
- Closed exact ranges from experiment log: `30`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `walkstuf1` | `high` | 1475/1441 | 59 | `142..166` (24s) | 3 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 2 | `walkstuf1` | `high` | 1475/1441 | 59 | `156..172` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `high` | 1475/1441 | 59 | `351..367` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 4 | `walkstuf1` | `high` | 1475/1441 | 59 | `149..165` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `high` | 1475/1441 | 59 | `357..369` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 6 | `walkstuf1` | `high` | 1475/1441 | 59 | `171..183` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 7 | `building2` | `high` | 1347/1313 | 39 | `140..164` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `building2` | `high` | 1347/1313 | 39 | `158..182` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `building2` | `high` | 1347/1313 | 39 | `249..265` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `building2` | `high` | 1347/1313 | 39 | `158..174` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `building2` | `high` | 1347/1313 | 39 | `95..107` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `building2` | `high` | 1347/1313 | 39 | `140..152` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `walkstuf1` | `low` | 1473/1444 | 43 | `384..408` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `walkstuf1` | `low` | 1473/1444 | 43 | `148..164` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 15 | `walkstuf1` | `low` | 1473/1444 | 43 | `172..184` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 16 | `walkstuf1` | `low` | 1473/1444 | 43 | `225..237` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 17 | `walkstuf1` | `low` | 1473/1444 | 43 | `159..165` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `visitor3` | `low` | 1065/1039 | 75 | `248..272` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `visitor3` | `low` | 1065/1039 | 75 | `240..264` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `visitor3` | `low` | 1065/1039 | 75 | `239..263` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `visitor3` | `low` | 1065/1039 | 75 | `248..264` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `visitor3` | `low` | 1065/1039 | 75 | `256..268` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `visitor3` | `low` | 1065/1039 | 75 | `256..272` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `visitor3` | `low` | 1065/1039 | 75 | `239..251` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `visitor3` | `low` | 1065/1039 | 75 | `248..260` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `building2` | `low` | 1339/1316 | 53 | `153..177` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `building2` | `low` | 1339/1316 | 53 | `145..169` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `building2` | `low` | 1339/1316 | 53 | `141..165` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `building2` | `low` | 1339/1316 | 53 | `137..161` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `building2` | `low` | 1339/1316 | 53 | `137..153` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `building2` | `low` | 1339/1316 | 53 | `137..149` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 32 | `building2` | `low` | 1339/1316 | 53 | `158..170` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 33 | `building2` | `low` | 1339/1316 | 53 | `141..153` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 34 | `building2` | `low` | 1339/1316 | 53 | `89..105` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 35 | `walkstuf1` | `high` | 1475/1441 | 59 | `84..108` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 36 | `walkstuf1` | `high` | 1475/1441 | 59 | `164..188` (24s) | 3 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 37 | `walkstuf1` | `high` | 1475/1441 | 59 | `156..180` (24s) | 3 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 38 | `walkstuf1` | `high` | 1475/1441 | 59 | `178..194` (16s) | 2 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 39 | `walkstuf1` | `high` | 1475/1441 | 59 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 40 | `walkstuf1` | `high` | 1475/1441 | 59 | `268..280` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |

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
