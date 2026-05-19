# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/walkstuf1-high-frame92-d4-current-readroot`
- Candidate rows: `69`
- Standalone probes: `0`
- Scheduler or guarded probes: `3`
- Scheduler-owned only: `22`
- Closed exact ranges from experiment log: `44`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `building4` | `low` | 2847/2820 | 32 | `400..424` (24s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 2 | `building4` | `low` | 2847/2820 | 32 | `205..229` (24s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 3 | `building2` | `low` | 1327/1318 | 47 | `67..83` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 4 | `walkstuf1` | `high` | 1471/1440 | 57 | `345..369` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `high` | 1471/1440 | 57 | `92..108` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 6 | `building4` | `low` | 2847/2820 | 32 | `270..294` (24s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 7 | `building4` | `low` | 2847/2820 | 32 | `170..194` (24s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `building4` | `low` | 2847/2820 | 32 | `359..375` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 9 | `building4` | `low` | 2847/2820 | 32 | `178..194` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 10 | `building4` | `low` | 2847/2820 | 32 | `270..286` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 11 | `building4` | `low` | 2847/2820 | 32 | `278..294` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 12 | `visitor3` | `low` | 1065/1039 | 75 | `239..263` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `visitor3` | `low` | 1065/1039 | 75 | `256..268` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `visitor3` | `low` | 1065/1039 | 75 | `256..272` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `visitor3` | `low` | 1065/1039 | 75 | `239..251` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `visitor3` | `low` | 1065/1039 | 75 | `248..260` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `building2` | `low` | 1327/1318 | 47 | `81..105` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `building2` | `low` | 1327/1318 | 47 | `162..186` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `building2` | `low` | 1327/1318 | 47 | `158..170` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `building2` | `low` | 1327/1318 | 47 | `100..112` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `building2` | `low` | 1327/1318 | 47 | `162..178` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `building2` | `low` | 1327/1318 | 47 | `94..110` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `building2` | `low` | 1327/1318 | 47 | `153..169` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `building2` | `low` | 1327/1318 | 47 | `94..106` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `building2` | `low` | 1327/1318 | 47 | `153..165` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `building2` | `high` | 1347/1313 | 39 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 27 | `building2` | `high` | 1347/1313 | 39 | `95..119` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 28 | `building2` | `high` | 1347/1313 | 39 | `140..164` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 29 | `building2` | `high` | 1347/1313 | 39 | `158..182` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 30 | `building2` | `high` | 1347/1313 | 39 | `255..271` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 31 | `building2` | `high` | 1347/1313 | 39 | `249..265` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 32 | `building2` | `high` | 1347/1313 | 39 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 33 | `building2` | `high` | 1347/1313 | 39 | `95..111` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 34 | `building2` | `high` | 1347/1313 | 39 | `158..174` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 35 | `building2` | `high` | 1347/1313 | 39 | `249..261` (12s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 36 | `building2` | `high` | 1347/1313 | 39 | `95..107` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 37 | `building2` | `high` | 1347/1313 | 39 | `140..152` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 38 | `walkstuf1` | `high` | 1471/1440 | 57 | `352..376` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 39 | `walkstuf1` | `high` | 1471/1440 | 57 | `84..108` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 40 | `walkstuf1` | `high` | 1471/1440 | 57 | `165..189` (24s) | 3 | `risky:short-visible-gap` | `closed-by-experiment-log` |

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
