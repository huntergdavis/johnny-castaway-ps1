# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/walkstuf1-high-rg213-229-slack4-v316-broad-norequire/20260511-205651-3681796`
- Candidate rows: `79`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `37`
- Closed exact ranges from experiment log: `8`
- Deferred under-target rows: `12`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `walkstuf1` | `low` | 1483/1429 | 74 | `305..321` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 2 | `walkstuf1` | `low` | 1483/1429 | 74 | `291..307` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `low` | 1483/1429 | 74 | `78..94` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 4 | `walkstuf1` | `low` | 1483/1429 | 74 | `285..297` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `low` | 1483/1429 | 74 | `297..309` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 6 | `walkstuf1` | `low` | 1483/1429 | 74 | `190..202` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 7 | `walkstuf1` | `low` | 1483/1429 | 74 | `78..90` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 8 | `walkstuf1` | `low` | 1483/1429 | 74 | `291..297` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `walkstuf1` | `low` | 1483/1429 | 74 | `305..311` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `walkstuf1` | `low` | 1483/1429 | 74 | `303..309` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `walkstuf1` | `high` | 1480/1429 | 85 | `306..322` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `walkstuf1` | `high` | 1480/1429 | 85 | `344..360` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 13 | `walkstuf1` | `high` | 1480/1429 | 85 | `357..369` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `walkstuf1` | `high` | 1480/1429 | 85 | `287..299` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `walkstuf1` | `high` | 1480/1429 | 85 | `298..310` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `walkstuf1` | `high` | 1480/1429 | 85 | `292..298` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `walkstuf1` | `high` | 1480/1429 | 85 | `306..312` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `walkstuf1` | `high` | 1480/1429 | 85 | `304..310` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `building2` | `high` | 1352/1311 | 56 | `17..33` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `building2` | `high` | 1352/1311 | 56 | `226..242` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `building2` | `high` | 1352/1311 | 56 | `11..27` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `building2` | `high` | 1352/1311 | 56 | `222..238` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `building2` | `high` | 1352/1311 | 56 | `226..238` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `building2` | `high` | 1352/1311 | 56 | `222..234` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `building2` | `high` | 1352/1311 | 56 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `building2` | `high` | 1352/1311 | 56 | `206..218` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `building2` | `high` | 1352/1311 | 56 | `23..29` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `building2` | `low` | 1349/1318 | 81 | `222..238` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `building2` | `low` | 1349/1318 | 81 | `218..234` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `building2` | `low` | 1349/1318 | 81 | `214..230` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `building2` | `low` | 1349/1318 | 81 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 32 | `building2` | `low` | 1349/1318 | 81 | `238..250` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 33 | `building2` | `low` | 1349/1318 | 81 | `222..234` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 34 | `building2` | `low` | 1349/1318 | 81 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 35 | `building2` | `low` | 1349/1318 | 81 | `67..73` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 36 | `building2` | `low` | 1349/1318 | 81 | `238..244` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 37 | `johnny1` | `high` | 1973/1945 | 25 | `123..139` (16s) | 1 | `risky:multi-partial-overlap` | `scheduler-owned-only` |
| 38 | `walkstuf1` | `low` | 1483/1429 | 74 | `297..313` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 39 | `walkstuf1` | `high` | 1480/1429 | 85 | `298..314` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 40 | `walkstuf1` | `high` | 1480/1429 | 85 | `178..194` (16s) | 2 | `risky:short-visible-gap` | `closed-by-experiment-log` |

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
