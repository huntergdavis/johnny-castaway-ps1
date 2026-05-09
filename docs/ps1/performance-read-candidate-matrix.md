# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/visitor3-high-f127-f130-resident-copy-v238-broad-regression/20260509-145507-53698`
- Candidate rows: `96`
- Standalone probes: `2`
- Scheduler or guarded probes: `3`
- Scheduler-owned only: `39`
- Closed exact ranges from experiment log: `10`
- Deferred under-target rows: `12`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `activity9` | `low` | 2075/2061 | 17 | `12..28` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 2 | `activity9` | `low` | 2075/2061 | 17 | `343..359` (16s) | 1 | `safe:long-visible-gap` | `standalone-probe` |
| 3 | `activity9` | `low` | 2075/2061 | 17 | `335..351` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 4 | `activity9` | `low` | 2075/2061 | 17 | `311..327` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 5 | `activity9` | `low` | 2075/2061 | 17 | `232..244` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 6 | `walkstuf1` | `high` | 1495/1427 | 95 | `287..303` (16s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 7 | `walkstuf1` | `high` | 1495/1427 | 95 | `329..345` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `walkstuf1` | `high` | 1495/1427 | 95 | `281..297` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `walkstuf1` | `high` | 1495/1427 | 95 | `287..299` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `walkstuf1` | `high` | 1495/1427 | 95 | `290..302` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `walkstuf1` | `high` | 1495/1427 | 95 | `89..101` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `walkstuf1` | `high` | 1495/1427 | 95 | `295..301` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `walkstuf1` | `high` | 1495/1427 | 95 | `290..296` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `walkstuf1` | `high` | 1495/1427 | 95 | `307..313` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `walkstuf1` | `high` | 1495/1427 | 95 | `95..101` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `walkstuf1` | `low` | 1489/1427 | 86 | `329..345` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `walkstuf1` | `low` | 1489/1427 | 86 | `295..311` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `walkstuf1` | `low` | 1489/1427 | 86 | `289..301` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `walkstuf1` | `low` | 1489/1427 | 86 | `89..101` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `walkstuf1` | `low` | 1489/1427 | 86 | `258..270` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `walkstuf1` | `low` | 1489/1427 | 86 | `295..301` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `walkstuf1` | `low` | 1489/1427 | 86 | `258..264` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `walkstuf1` | `low` | 1489/1427 | 86 | `307..313` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `walkstuf1` | `low` | 1489/1427 | 86 | `95..101` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `visitor3` | `high` | 1075/1037 | 59 | `261..277` (16s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `building2` | `low` | 1349/1316 | 83 | `234..250` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `building2` | `low` | 1349/1316 | 83 | `222..238` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `building2` | `low` | 1349/1316 | 83 | `218..234` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `building2` | `low` | 1349/1316 | 83 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `building2` | `low` | 1349/1316 | 83 | `238..250` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `building2` | `low` | 1349/1316 | 83 | `222..234` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 32 | `building2` | `low` | 1349/1316 | 83 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 33 | `building2` | `low` | 1349/1316 | 83 | `238..244` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 34 | `building2` | `high` | 1349/1316 | 48 | `226..242` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 35 | `building2` | `high` | 1349/1316 | 48 | `206..222` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 36 | `building2` | `high` | 1349/1316 | 48 | `202..218` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 37 | `building2` | `high` | 1349/1316 | 48 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 38 | `building2` | `high` | 1349/1316 | 48 | `226..238` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 39 | `building2` | `high` | 1349/1316 | 48 | `206..218` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 40 | `building2` | `high` | 1349/1316 | 48 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |

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
