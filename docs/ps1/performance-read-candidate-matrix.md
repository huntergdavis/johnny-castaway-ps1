# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/visitor3-high-tight56-canaries/20260518-213723-865271`
- Candidate rows: `65`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `25`
- Closed exact ranges from experiment log: `40`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `walkstuf1` | `high` | 1475/1441 | 57 | `99..115` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 2 | `walkstuf1` | `high` | 1475/1441 | 57 | `357..369` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `high` | 1475/1441 | 57 | `171..183` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 4 | `building2` | `high` | 1347/1313 | 39 | `140..152` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `low` | 1473/1444 | 43 | `172..184` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 6 | `walkstuf1` | `low` | 1473/1444 | 43 | `225..237` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 7 | `walkstuf1` | `low` | 1473/1444 | 43 | `159..165` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `visitor3` | `low` | 1065/1039 | 75 | `248..272` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `visitor3` | `low` | 1065/1039 | 75 | `240..264` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `visitor3` | `low` | 1065/1039 | 75 | `239..263` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `visitor3` | `low` | 1065/1039 | 75 | `248..264` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `visitor3` | `low` | 1065/1039 | 75 | `256..268` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `visitor3` | `low` | 1065/1039 | 75 | `256..272` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `visitor3` | `low` | 1065/1039 | 75 | `239..251` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `visitor3` | `low` | 1065/1039 | 75 | `248..260` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `building2` | `low` | 1336/1316 | 48 | `245..269` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `building2` | `low` | 1336/1316 | 48 | `153..177` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `building2` | `low` | 1336/1316 | 48 | `145..169` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `building2` | `low` | 1336/1316 | 48 | `137..161` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `building2` | `low` | 1336/1316 | 48 | `252..268` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `building2` | `low` | 1336/1316 | 48 | `245..261` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `building2` | `low` | 1336/1316 | 48 | `252..264` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `building2` | `low` | 1336/1316 | 48 | `258..270` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `building2` | `low` | 1336/1316 | 48 | `133..145` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `building2` | `low` | 1336/1316 | 48 | `258..264` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `walkstuf1` | `high` | 1475/1441 | 57 | `84..108` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 27 | `walkstuf1` | `high` | 1475/1441 | 57 | `165..189` (24s) | 3 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 28 | `walkstuf1` | `high` | 1475/1441 | 57 | `74..98` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 29 | `walkstuf1` | `high` | 1475/1441 | 57 | `171..195` (24s) | 3 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 30 | `walkstuf1` | `high` | 1475/1441 | 57 | `178..194` (16s) | 2 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 31 | `walkstuf1` | `high` | 1475/1441 | 57 | `351..367` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 32 | `walkstuf1` | `high` | 1475/1441 | 57 | `360..376` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 33 | `walkstuf1` | `high` | 1475/1441 | 57 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 34 | `walkstuf1` | `high` | 1475/1441 | 57 | `268..280` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 35 | `building2` | `high` | 1347/1313 | 39 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 36 | `building2` | `high` | 1347/1313 | 39 | `95..119` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 37 | `building2` | `high` | 1347/1313 | 39 | `140..164` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 38 | `building2` | `high` | 1347/1313 | 39 | `158..182` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 39 | `building2` | `high` | 1347/1313 | 39 | `255..271` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 40 | `building2` | `high` | 1347/1313 | 39 | `249..265` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |

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
