# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate`
- Candidate rows: `22164`
- Standalone probes: `102`
- Scheduler or guarded probes: `580`
- Scheduler-owned only: `7806`
- Closed exact ranges from experiment log: `5965`
- Deferred under-target rows: `1810`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `visitor3` | `high` | 1456/1010 | 363 | `163..175` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 2 | `visitor3` | `high` | 1456/1010 | 363 | `163..175` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 3 | `visitor3` | `low` | 1457/1016 | 360 | `163..175` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 4 | `walkstuf1` | `high` | 1490/1424 | 91 | `207..219` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 5 | `suzy2` | `high` | 2655/2633 | 19 | `201..213` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 6 | `suzy2` | `low` | 2655/2633 | 19 | `201..213` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 7 | `suzy2` | `high` | 2655/2633 | 19 | `116..128` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 8 | `suzy2` | `low` | 2655/2633 | 19 | `116..128` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 9 | `suzy2` | `high` | 2655/2633 | 19 | `220..232` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 10 | `suzy2` | `low` | 2655/2633 | 19 | `220..232` (12s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 11 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 12 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 13 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 14 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 15 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 16 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 17 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 18 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 19 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 20 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 21 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 22 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 23 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 24 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 25 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 26 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 27 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 28 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 29 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 30 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 31 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 32 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 33 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 34 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 35 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 36 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 37 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 38 | `activity9` | `low` | 2075/2061 | 17 | `343..359` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 39 | `activity9` | `low` | 2075/2061 | 17 | `343..359` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 40 | `activity9` | `low` | 2075/2061 | 17 | `343..359` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |

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
