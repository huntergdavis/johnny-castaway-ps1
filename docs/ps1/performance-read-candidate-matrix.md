# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/current-yellow-readplans-20260518-b2low-setupseg`
- Candidate rows: `161`
- Standalone probes: `0`
- Scheduler or guarded probes: `14`
- Scheduler-owned only: `118`
- Closed exact ranges from experiment log: `29`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `walkstuf1` | `low` | 1480/1442 | 55 | `195..211` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 2 | `building2` | `high` | 1351/1313 | 50 | `264..280` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 3 | `building2` | `high` | 1351/1313 | 50 | `271..287` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 4 | `building2` | `high` | 1351/1313 | 50 | `249..261` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 5 | `walkstuf1` | `high` | 1472/1438 | 60 | `423..439` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 6 | `building2` | `low` | 1339/1316 | 53 | `250..274` (24s) | 2 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 7 | `building2` | `low` | 1339/1316 | 53 | `212..236` (24s) | 2 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 8 | `building2` | `low` | 1339/1316 | 53 | `205..229` (24s) | 2 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 9 | `building2` | `low` | 1339/1316 | 53 | `257..273` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 10 | `building2` | `low` | 1339/1316 | 53 | `212..228` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 11 | `building2` | `low` | 1339/1316 | 53 | `205..221` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 12 | `building2` | `low` | 1339/1316 | 53 | `75..91` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 13 | `building2` | `low` | 1339/1316 | 53 | `184..200` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 14 | `building2` | `low` | 1339/1316 | 53 | `250..266` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 15 | `walkstuf1` | `low` | 1480/1442 | 55 | `91..115` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `walkstuf1` | `low` | 1480/1442 | 55 | `353..377` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `walkstuf1` | `low` | 1480/1442 | 55 | `179..203` (24s) | 3 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 18 | `walkstuf1` | `low` | 1480/1442 | 55 | `172..196` (24s) | 3 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 19 | `walkstuf1` | `low` | 1480/1442 | 55 | `347..371` (24s) | 3 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 20 | `walkstuf1` | `low` | 1480/1442 | 55 | `365..389` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `walkstuf1` | `low` | 1480/1442 | 55 | `98..122` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `walkstuf1` | `low` | 1480/1442 | 55 | `360..384` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `walkstuf1` | `low` | 1480/1442 | 55 | `159..183` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `walkstuf1` | `low` | 1480/1442 | 55 | `353..369` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 25 | `walkstuf1` | `low` | 1480/1442 | 55 | `98..114` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `walkstuf1` | `low` | 1480/1442 | 55 | `155..167` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `walkstuf1` | `low` | 1480/1442 | 55 | `148..164` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 28 | `walkstuf1` | `low` | 1480/1442 | 55 | `159..175` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `walkstuf1` | `low` | 1480/1442 | 55 | `172..184` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 30 | `walkstuf1` | `low` | 1480/1442 | 55 | `396..408` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `walkstuf1` | `low` | 1480/1442 | 55 | `347..363` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 32 | `walkstuf1` | `low` | 1480/1442 | 55 | `190..206` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 33 | `walkstuf1` | `low` | 1480/1442 | 55 | `184..196` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 34 | `walkstuf1` | `low` | 1480/1442 | 55 | `179..191` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 35 | `walkstuf1` | `low` | 1480/1442 | 55 | `98..110` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 36 | `walkstuf1` | `low` | 1480/1442 | 55 | `360..372` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 37 | `walkstuf1` | `low` | 1480/1442 | 55 | `159..165` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 38 | `walkstuf1` | `low` | 1480/1442 | 55 | `103..115` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 39 | `walkstuf1` | `low` | 1480/1442 | 55 | `365..377` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 40 | `walkstuf1` | `low` | 1480/1442 | 55 | `159..171` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |

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
