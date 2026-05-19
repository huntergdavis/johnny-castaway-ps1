# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/visitor3-high-clean64-canaries/20260519-010953-2088970`
- Candidate rows: `64`
- Standalone probes: `0`
- Scheduler or guarded probes: `1`
- Scheduler-owned only: `14`
- Closed exact ranges from experiment log: `49`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `building2` | `low` | 1327/1318 | 47 | `67..83` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 2 | `visitor3` | `low` | 1065/1039 | 75 | `239..263` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 3 | `visitor3` | `low` | 1065/1039 | 75 | `256..268` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 4 | `visitor3` | `low` | 1065/1039 | 75 | `256..272` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 5 | `visitor3` | `low` | 1065/1039 | 75 | `239..251` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 6 | `visitor3` | `low` | 1065/1039 | 75 | `248..260` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 7 | `building2` | `low` | 1327/1318 | 47 | `81..105` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `building2` | `low` | 1327/1318 | 47 | `162..186` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `building2` | `low` | 1327/1318 | 47 | `158..170` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `building2` | `low` | 1327/1318 | 47 | `100..112` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `building2` | `low` | 1327/1318 | 47 | `162..178` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `building2` | `low` | 1327/1318 | 47 | `94..110` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `building2` | `low` | 1327/1318 | 47 | `153..169` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `building2` | `low` | 1327/1318 | 47 | `94..106` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `building2` | `low` | 1327/1318 | 47 | `153..165` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `walkstuf1` | `high` | 1475/1441 | 57 | `84..108` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 17 | `walkstuf1` | `high` | 1475/1441 | 57 | `165..189` (24s) | 3 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 18 | `walkstuf1` | `high` | 1475/1441 | 57 | `74..98` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 19 | `walkstuf1` | `high` | 1475/1441 | 57 | `171..195` (24s) | 3 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 20 | `walkstuf1` | `high` | 1475/1441 | 57 | `178..194` (16s) | 2 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 21 | `walkstuf1` | `high` | 1475/1441 | 57 | `351..367` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 22 | `walkstuf1` | `high` | 1475/1441 | 57 | `99..115` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 23 | `walkstuf1` | `high` | 1475/1441 | 57 | `360..376` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 24 | `walkstuf1` | `high` | 1475/1441 | 57 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 25 | `walkstuf1` | `high` | 1475/1441 | 57 | `357..369` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 26 | `walkstuf1` | `high` | 1475/1441 | 57 | `268..280` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 27 | `walkstuf1` | `high` | 1475/1441 | 57 | `171..183` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 28 | `building2` | `high` | 1347/1313 | 39 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 29 | `building2` | `high` | 1347/1313 | 39 | `95..119` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 30 | `building2` | `high` | 1347/1313 | 39 | `140..164` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 31 | `building2` | `high` | 1347/1313 | 39 | `158..182` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 32 | `building2` | `high` | 1347/1313 | 39 | `255..271` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 33 | `building2` | `high` | 1347/1313 | 39 | `249..265` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 34 | `building2` | `high` | 1347/1313 | 39 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 35 | `building2` | `high` | 1347/1313 | 39 | `95..111` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 36 | `building2` | `high` | 1347/1313 | 39 | `158..174` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 37 | `building2` | `high` | 1347/1313 | 39 | `249..261` (12s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 38 | `building2` | `high` | 1347/1313 | 39 | `95..107` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 39 | `building2` | `high` | 1347/1313 | 39 | `140..152` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 40 | `building4` | `low` | 2849/2816 | 38 | `178..202` (24s) | 1 | `balanced:medium-visible-gap` | `closed-by-experiment-log` |

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
