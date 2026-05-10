# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/visitor3-high-f131-resident-alias121123-v299-broad-norequire/20260510-033846-2573991`
- Candidate rows: `116`
- Standalone probes: `2`
- Scheduler or guarded probes: `7`
- Scheduler-owned only: `53`
- Closed exact ranges from experiment log: `8`
- Deferred under-target rows: `12`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `activity9` | `low` | 2072/2060 | 14 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 2 | `activity9` | `low` | 2072/2060 | 14 | `343..359` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 3 | `activity9` | `low` | 2072/2060 | 14 | `335..351` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 4 | `activity9` | `low` | 2072/2060 | 14 | `311..327` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 5 | `activity9` | `low` | 2072/2060 | 14 | `232..244` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 6 | `mary3` | `low` | 2300/2293 | 55 | `10..26` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 7 | `mary3` | `low` | 2300/2293 | 55 | `268..280` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 8 | `mary3` | `high` | 2297/2293 | 52 | `10..26` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 9 | `mary3` | `high` | 2297/2293 | 52 | `268..280` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 10 | `walkstuf1` | `low` | 1478/1428 | 75 | `284..300` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `walkstuf1` | `low` | 1478/1428 | 75 | `298..314` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `walkstuf1` | `low` | 1478/1428 | 75 | `306..322` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `walkstuf1` | `low` | 1478/1428 | 75 | `284..296` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `walkstuf1` | `low` | 1478/1428 | 75 | `333..345` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `walkstuf1` | `low` | 1478/1428 | 75 | `298..310` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `walkstuf1` | `low` | 1478/1428 | 75 | `214..220` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `walkstuf1` | `low` | 1478/1428 | 75 | `292..298` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `walkstuf1` | `low` | 1478/1428 | 75 | `306..312` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `walkstuf1` | `low` | 1478/1428 | 75 | `290..296` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `walkstuf1` | `high` | 1477/1431 | 90 | `298..314` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `walkstuf1` | `high` | 1477/1431 | 90 | `306..322` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `walkstuf1` | `high` | 1477/1431 | 90 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `walkstuf1` | `high` | 1477/1431 | 90 | `287..299` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `walkstuf1` | `high` | 1477/1431 | 90 | `378..390` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `walkstuf1` | `high` | 1477/1431 | 90 | `333..345` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `walkstuf1` | `high` | 1477/1431 | 90 | `292..298` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `walkstuf1` | `high` | 1477/1431 | 90 | `306..312` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `walkstuf1` | `high` | 1477/1431 | 90 | `304..310` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `building2` | `high` | 1352/1314 | 48 | `17..33` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `building2` | `high` | 1352/1314 | 48 | `11..27` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `building2` | `high` | 1352/1314 | 48 | `226..242` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 32 | `building2` | `high` | 1352/1314 | 48 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 33 | `building2` | `high` | 1352/1314 | 48 | `226..238` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 34 | `building2` | `high` | 1352/1314 | 48 | `206..218` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 35 | `building2` | `high` | 1352/1314 | 48 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 36 | `building2` | `high` | 1352/1314 | 48 | `23..29` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 37 | `building2` | `low` | 1354/1317 | 100 | `206..222` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 38 | `building2` | `low` | 1354/1317 | 100 | `234..250` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 39 | `building2` | `low` | 1354/1317 | 100 | `202..218` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 40 | `building2` | `low` | 1354/1317 | 100 | `222..238` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |

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
