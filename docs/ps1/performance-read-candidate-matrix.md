# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/current-under99-readplans-v526-20260513`
- Candidate rows: `79`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `2`
- Closed exact ranges from experiment log: `67`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 79 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `visitor3` | `high` | 1065/1039 | 41 | `211..235` (24s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 2 | `visitor3` | `high` | 1065/1039 | 41 | `228..252` (24s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `low` | 1478/1431 | 64 | `297..321` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 4 | `walkstuf1` | `low` | 1478/1431 | 64 | `285..309` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 5 | `walkstuf1` | `low` | 1478/1431 | 64 | `291..315` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 6 | `walkstuf1` | `low` | 1478/1431 | 64 | `273..297` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 7 | `walkstuf1` | `low` | 1478/1431 | 64 | `297..313` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 8 | `walkstuf1` | `low` | 1478/1431 | 64 | `305..321` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 9 | `walkstuf1` | `low` | 1478/1431 | 64 | `291..307` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 10 | `walkstuf1` | `low` | 1478/1431 | 64 | `371..387` (16s) | 2 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 11 | `walkstuf1` | `low` | 1478/1431 | 64 | `285..297` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 12 | `walkstuf1` | `low` | 1478/1431 | 64 | `297..309` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 13 | `walkstuf1` | `low` | 1478/1431 | 64 | `190..202` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 14 | `walkstuf1` | `low` | 1478/1431 | 64 | `273..285` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 15 | `walkstuf1` | `low` | 1478/1431 | 64 | `291..297` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 16 | `walkstuf1` | `low` | 1478/1431 | 64 | `305..311` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 17 | `walkstuf1` | `low` | 1478/1431 | 64 | `303..309` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 18 | `walkstuf1` | `high` | 1476/1434 | 81 | `298..322` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 19 | `walkstuf1` | `high` | 1476/1434 | 81 | `287..311` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 20 | `walkstuf1` | `high` | 1476/1434 | 81 | `292..316` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 21 | `walkstuf1` | `high` | 1476/1434 | 81 | `274..298` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 22 | `walkstuf1` | `high` | 1476/1434 | 81 | `298..314` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 23 | `walkstuf1` | `high` | 1476/1434 | 81 | `306..322` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 24 | `walkstuf1` | `high` | 1476/1434 | 81 | `360..376` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 25 | `walkstuf1` | `high` | 1476/1434 | 81 | `178..194` (16s) | 2 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 26 | `walkstuf1` | `high` | 1476/1434 | 81 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 27 | `walkstuf1` | `high` | 1476/1434 | 81 | `287..299` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 28 | `walkstuf1` | `high` | 1476/1434 | 81 | `298..310` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 29 | `walkstuf1` | `high` | 1476/1434 | 81 | `268..280` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 30 | `walkstuf1` | `high` | 1476/1434 | 81 | `292..298` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 31 | `walkstuf1` | `high` | 1476/1434 | 81 | `306..312` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 32 | `walkstuf1` | `high` | 1476/1434 | 81 | `304..310` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 33 | `building2` | `high` | 1351/1311 | 54 | `202..226` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 34 | `building2` | `high` | 1351/1311 | 54 | `11..35` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 35 | `building2` | `high` | 1351/1311 | 54 | `17..41` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 36 | `building2` | `high` | 1351/1311 | 54 | `90..114` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 37 | `building2` | `high` | 1351/1311 | 54 | `17..33` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 38 | `building2` | `high` | 1351/1311 | 54 | `11..27` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 39 | `building2` | `high` | 1351/1311 | 54 | `206..222` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 40 | `building2` | `high` | 1351/1311 | 54 | `210..226` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 41 | `building2` | `high` | 1351/1311 | 54 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 42 | `building2` | `high` | 1351/1311 | 54 | `206..218` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 43 | `building2` | `high` | 1351/1311 | 54 | `214..226` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 44 | `building2` | `high` | 1351/1311 | 54 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 45 | `building2` | `high` | 1351/1311 | 54 | `23..29` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 46 | `building4` | `low` | 2855/2815 | 46 | `178..202` (24s) | 1 | `balanced:medium-visible-gap` | `closed-by-experiment-log` |
| 47 | `building2` | `low` | 1349/1320 | 70 | `210..234` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 48 | `building2` | `low` | 1349/1320 | 70 | `202..226` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 49 | `building2` | `low` | 1349/1320 | 70 | `214..238` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 50 | `building2` | `low` | 1349/1320 | 70 | `206..230` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 51 | `building2` | `low` | 1349/1320 | 70 | `210..226` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 52 | `building2` | `low` | 1349/1320 | 70 | `222..238` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 53 | `building2` | `low` | 1349/1320 | 70 | `218..234` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 54 | `building2` | `low` | 1349/1320 | 70 | `214..230` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 55 | `building2` | `low` | 1349/1320 | 70 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 56 | `building2` | `low` | 1349/1320 | 70 | `222..234` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 57 | `building2` | `low` | 1349/1320 | 70 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 58 | `building2` | `low` | 1349/1320 | 70 | `218..230` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 59 | `building2` | `low` | 1349/1320 | 70 | `67..73` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 60 | `johnny1` | `low` | 1974/1945 | 26 | `131..147` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 61 | `johnny1` | `low` | 1974/1945 | 26 | `145..161` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 62 | `johnny1` | `low` | 1974/1945 | 26 | `138..154` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 63 | `johnny1` | `low` | 1974/1945 | 26 | `123..139` (16s) | 1 | `risky:multi-partial-overlap` | `closed-by-experiment-log` |
| 64 | `johnny1` | `high` | 1973/1945 | 25 | `131..147` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 65 | `johnny1` | `high` | 1973/1945 | 25 | `145..161` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 66 | `johnny1` | `high` | 1973/1945 | 25 | `138..154` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 67 | `johnny1` | `high` | 1973/1945 | 25 | `123..139` (16s) | 1 | `risky:multi-partial-overlap` | `closed-by-experiment-log` |
| 68 | `visitor3` | `high` | 1065/1039 | 41 | `253..277` (24s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 69 | `visitor3` | `low` | 1062/1040 | 42 | `206..230` (24s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 70 | `building4` | `low` | 2855/2815 | 46 | `262..286` (24s) | 1 | `risky:short-visible-gap` | `reject-not-current-window-fit` |
| 71 | `building4` | `low` | 2855/2815 | 46 | `274..298` (24s) | 1 | `risky:short-visible-gap` | `reject-not-current-window-fit` |
| 72 | `johnny1` | `low` | 1974/1945 | 26 | `131..155` (24s) | 2 | `balanced:validate-overlap` | `reject-not-current-window-fit` |
| 73 | `johnny1` | `low` | 1974/1945 | 26 | `138..162` (24s) | 2 | `balanced:validate-overlap` | `reject-not-current-window-fit` |
| 74 | `johnny1` | `low` | 1974/1945 | 26 | `123..147` (24s) | 2 | `risky:multi-partial-overlap` | `reject-not-current-window-fit` |
| 75 | `johnny1` | `low` | 1974/1945 | 26 | `145..169` (24s) | 1 | `risky:overread` | `reject-not-current-window-fit` |
| 76 | `johnny1` | `high` | 1973/1945 | 25 | `131..155` (24s) | 2 | `balanced:validate-overlap` | `reject-not-current-window-fit` |
| 77 | `johnny1` | `high` | 1973/1945 | 25 | `138..162` (24s) | 2 | `balanced:validate-overlap` | `reject-not-current-window-fit` |
| 78 | `johnny1` | `high` | 1973/1945 | 25 | `123..147` (24s) | 2 | `risky:multi-partial-overlap` | `reject-not-current-window-fit` |
| 79 | `johnny1` | `high` | 1973/1945 | 25 | `145..169` (24s) | 1 | `risky:overread` | `reject-not-current-window-fit` |

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
