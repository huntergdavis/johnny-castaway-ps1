# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/walkstuf3-low-compact-fgp3-v171-broad-norequire/20260508-171152-1045430`
- Candidate rows: `88`
- Standalone probes: `0`
- Scheduler or guarded probes: `4`
- Scheduler-owned only: `37`
- Closed exact ranges from experiment log: `5`
- Deferred under-target rows: `12`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `walkstuf3` | `low` | 2310/2295 | 26 | `53..69` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 2 | `walkstuf3` | `low` | 2310/2295 | 26 | `60..76` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 3 | `walkstuf3` | `low` | 2310/2295 | 26 | `179..195` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 4 | `walkstuf3` | `low` | 2310/2295 | 26 | `67..83` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 5 | `visitor3` | `low` | 1126/1025 | 170 | `317..333` (16s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 6 | `visitor3` | `low` | 1126/1025 | 170 | `338..350` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 7 | `visitor3` | `low` | 1126/1025 | 170 | `333..345` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `visitor3` | `low` | 1126/1025 | 170 | `338..354` (16s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `visitor3` | `high` | 1118/1028 | 150 | `307..323` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `visitor3` | `high` | 1118/1028 | 150 | `299..315` (16s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `visitor3` | `high` | 1118/1028 | 150 | `320..332` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `visitor3` | `high` | 1118/1028 | 150 | `315..327` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `visitor3` | `high` | 1118/1028 | 150 | `320..336` (16s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `building2` | `low` | 1349/1316 | 83 | `210..226` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `building2` | `low` | 1349/1316 | 83 | `234..250` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `building2` | `low` | 1349/1316 | 83 | `222..238` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `building2` | `low` | 1349/1316 | 83 | `218..234` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `building2` | `low` | 1349/1316 | 83 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `building2` | `low` | 1349/1316 | 83 | `238..250` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `building2` | `low` | 1349/1316 | 83 | `222..234` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `building2` | `low` | 1349/1316 | 83 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `building2` | `low` | 1349/1316 | 83 | `238..244` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `building2` | `high` | 1349/1316 | 48 | `210..226` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `building2` | `high` | 1349/1316 | 48 | `226..242` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `building2` | `high` | 1349/1316 | 48 | `206..222` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `building2` | `high` | 1349/1316 | 48 | `202..218` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `building2` | `high` | 1349/1316 | 48 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `building2` | `high` | 1349/1316 | 48 | `226..238` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `building2` | `high` | 1349/1316 | 48 | `206..218` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `building2` | `high` | 1349/1316 | 48 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `building4` | `high` | 2844/2816 | 37 | `264..280` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 32 | `building4` | `high` | 2844/2816 | 37 | `31..47` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 33 | `building4` | `high` | 2844/2816 | 37 | `337..353` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 34 | `building4` | `high` | 2844/2816 | 37 | `176..192` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 35 | `activity9` | `low` | 2085/2058 | 29 | `418..434` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 36 | `activity9` | `low` | 2085/2058 | 29 | `325..341` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 37 | `activity9` | `low` | 2085/2058 | 29 | `341..357` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 38 | `activity9` | `low` | 2085/2058 | 29 | `263..275` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 39 | `activity9` | `low` | 2085/2058 | 29 | `245..257` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 40 | `activity9` | `low` | 2085/2058 | 29 | `251..263` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |

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
