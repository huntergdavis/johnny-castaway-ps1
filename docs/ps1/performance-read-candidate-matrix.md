# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/walkstuf1-low-owner160-176-fresh-five-yellow-current/20260520-200639-4163023`
- Candidate rows: `46`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `0`
- Closed exact ranges from experiment log: `46`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 12 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `building2` | `high` | 1347/1313 | 39 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 2 | `building2` | `high` | 1347/1313 | 39 | `95..119` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 3 | `building2` | `high` | 1347/1313 | 39 | `104..128` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 4 | `building2` | `high` | 1347/1313 | 39 | `117..141` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 5 | `building2` | `high` | 1347/1313 | 39 | `255..271` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 6 | `building2` | `high` | 1347/1313 | 39 | `249..265` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 7 | `building2` | `high` | 1347/1313 | 39 | `95..111` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 8 | `building2` | `high` | 1347/1313 | 39 | `140..156` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 9 | `building2` | `high` | 1347/1313 | 39 | `249..261` (12s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 10 | `building2` | `high` | 1347/1313 | 39 | `174..186` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 11 | `building2` | `high` | 1347/1313 | 39 | `95..107` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 12 | `building2` | `high` | 1347/1313 | 39 | `140..152` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |

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
