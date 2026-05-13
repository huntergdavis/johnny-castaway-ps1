# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/current-under99-readplans-v570-20260513`
- Candidate rows: `183`
- Standalone probes: `0`
- Scheduler or guarded probes: `4`
- Scheduler-owned only: `98`
- Closed exact ranges from experiment log: `81`
- Deferred under-target rows: `0`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 120 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `johnny1` | `low` | 1974/1945 | 26 | `131..155` (24s) | 2 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 2 | `johnny1` | `low` | 1974/1945 | 26 | `138..162` (24s) | 2 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 3 | `johnny1` | `high` | 1973/1945 | 25 | `131..155` (24s) | 2 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 4 | `johnny1` | `high` | 1973/1945 | 25 | `138..162` (24s) | 2 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 5 | `walkstuf1` | `low` | 1478/1431 | 64 | `237..261` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 6 | `walkstuf1` | `low` | 1478/1431 | 64 | `242..266` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 7 | `walkstuf1` | `low` | 1478/1431 | 64 | `308..332` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `walkstuf1` | `low` | 1478/1431 | 64 | `305..329` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `walkstuf1` | `low` | 1478/1431 | 64 | `141..165` (24s) | 3 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 10 | `walkstuf1` | `low` | 1478/1431 | 64 | `148..172` (24s) | 3 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 11 | `walkstuf1` | `low` | 1478/1431 | 64 | `364..388` (24s) | 3 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 12 | `walkstuf1` | `low` | 1478/1431 | 64 | `303..319` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `walkstuf1` | `low` | 1478/1431 | 64 | `155..171` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 14 | `walkstuf1` | `low` | 1478/1431 | 64 | `91..107` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `walkstuf1` | `low` | 1478/1431 | 64 | `405..421` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `walkstuf1` | `low` | 1478/1431 | 64 | `427..443` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `walkstuf1` | `low` | 1478/1431 | 64 | `378..394` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `walkstuf1` | `low` | 1478/1431 | 64 | `421..437` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `walkstuf1` | `low` | 1478/1431 | 64 | `394..410` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `walkstuf1` | `low` | 1478/1431 | 64 | `303..315` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `walkstuf1` | `low` | 1478/1431 | 64 | `345..357` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 22 | `walkstuf1` | `low` | 1478/1431 | 64 | `421..433` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 23 | `walkstuf1` | `low` | 1478/1431 | 64 | `155..167` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 24 | `walkstuf1` | `low` | 1478/1431 | 64 | `387..399` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `walkstuf1` | `low` | 1478/1431 | 64 | `184..196` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 26 | `walkstuf1` | `low` | 1478/1431 | 64 | `416..428` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 27 | `walkstuf1` | `low` | 1478/1431 | 64 | `378..390` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 28 | `walkstuf1` | `high` | 1476/1434 | 81 | `295..319` (24s) | 5 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `walkstuf1` | `high` | 1476/1434 | 81 | `84..108` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `walkstuf1` | `high` | 1476/1434 | 81 | `238..262` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `walkstuf1` | `high` | 1476/1434 | 81 | `306..330` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 32 | `walkstuf1` | `high` | 1476/1434 | 81 | `164..188` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 33 | `walkstuf1` | `high` | 1476/1434 | 81 | `360..384` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 34 | `walkstuf1` | `high` | 1476/1434 | 81 | `156..180` (24s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 35 | `walkstuf1` | `high` | 1476/1434 | 81 | `292..308` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 36 | `walkstuf1` | `high` | 1476/1434 | 81 | `304..320` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 37 | `walkstuf1` | `high` | 1476/1434 | 81 | `295..311` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 38 | `walkstuf1` | `high` | 1476/1434 | 81 | `156..172` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 39 | `walkstuf1` | `high` | 1476/1434 | 81 | `99..115` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 40 | `walkstuf1` | `high` | 1476/1434 | 81 | `92..108` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 41 | `walkstuf1` | `high` | 1476/1434 | 81 | `280..296` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 42 | `walkstuf1` | `high` | 1476/1434 | 81 | `149..165` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 43 | `walkstuf1` | `high` | 1476/1434 | 81 | `295..307` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 44 | `walkstuf1` | `high` | 1476/1434 | 81 | `304..316` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 45 | `walkstuf1` | `high` | 1476/1434 | 81 | `360..372` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 46 | `walkstuf1` | `high` | 1476/1434 | 81 | `74..86` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 47 | `walkstuf1` | `high` | 1476/1434 | 81 | `171..183` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 48 | `walkstuf1` | `high` | 1476/1434 | 81 | `183..195` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 49 | `walkstuf1` | `high` | 1476/1434 | 81 | `99..111` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 50 | `building2` | `high` | 1351/1311 | 54 | `140..164` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 51 | `building2` | `high` | 1351/1311 | 54 | `158..182` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 52 | `building2` | `high` | 1351/1311 | 54 | `167..191` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 53 | `building2` | `high` | 1351/1311 | 54 | `23..47` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 54 | `building2` | `high` | 1351/1311 | 54 | `193..217` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 55 | `building2` | `high` | 1351/1311 | 54 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 56 | `building2` | `high` | 1351/1311 | 54 | `104..128` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 57 | `building2` | `high` | 1351/1311 | 54 | `202..218` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 58 | `building2` | `high` | 1351/1311 | 54 | `202..214` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 59 | `building2` | `high` | 1351/1311 | 54 | `17..29` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 60 | `building2` | `high` | 1351/1311 | 54 | `90..106` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 61 | `building2` | `high` | 1351/1311 | 54 | `158..174` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 62 | `building2` | `high` | 1351/1311 | 54 | `140..156` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 63 | `building2` | `high` | 1351/1311 | 54 | `193..209` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 64 | `building2` | `high` | 1351/1311 | 54 | `167..183` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 65 | `building2` | `high` | 1351/1311 | 54 | `23..35` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 66 | `building2` | `high` | 1351/1311 | 54 | `24..40` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 67 | `building2` | `high` | 1351/1311 | 54 | `23..39` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 68 | `building2` | `high` | 1351/1311 | 54 | `72..84` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 69 | `building2` | `high` | 1351/1311 | 54 | `90..102` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 70 | `building2` | `high` | 1351/1311 | 54 | `140..152` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 71 | `building2` | `high` | 1351/1311 | 54 | `193..205` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 72 | `building4` | `low` | 2855/2815 | 46 | `262..286` (24s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 73 | `building4` | `low` | 2855/2815 | 46 | `274..298` (24s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 74 | `building2` | `low` | 1349/1320 | 70 | `140..164` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 75 | `building2` | `low` | 1349/1320 | 70 | `176..200` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 76 | `building2` | `low` | 1349/1320 | 70 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 77 | `building2` | `low` | 1349/1320 | 70 | `149..173` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 78 | `building2` | `low` | 1349/1320 | 70 | `104..128` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 79 | `building2` | `low` | 1349/1320 | 70 | `153..177` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 80 | `building2` | `low` | 1349/1320 | 70 | `162..186` (24s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 81 | `building2` | `low` | 1349/1320 | 70 | `202..218` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 82 | `building2` | `low` | 1349/1320 | 70 | `67..83` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 83 | `building2` | `low` | 1349/1320 | 70 | `250..266` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 84 | `building2` | `low` | 1349/1320 | 70 | `256..272` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 85 | `building2` | `low` | 1349/1320 | 70 | `202..214` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 86 | `building2` | `low` | 1349/1320 | 70 | `80..96` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 87 | `building2` | `low` | 1349/1320 | 70 | `140..156` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 88 | `building2` | `low` | 1349/1320 | 70 | `176..192` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 89 | `building2` | `low` | 1349/1320 | 70 | `140..152` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 90 | `building2` | `low` | 1349/1320 | 70 | `176..188` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 91 | `johnny1` | `low` | 1974/1945 | 26 | `123..147` (24s) | 2 | `risky:multi-partial-overlap` | `scheduler-owned-only` |
| 92 | `johnny1` | `low` | 1974/1945 | 26 | `145..169` (24s) | 1 | `risky:overread` | `scheduler-owned-only` |
| 93 | `johnny1` | `low` | 1974/1945 | 26 | `56..72` (16s) | 1 | `risky:multi-partial-overlap` | `scheduler-owned-only` |
| 94 | `johnny1` | `low` | 1974/1945 | 26 | `56..80` (24s) | 1 | `risky:multi-partial-overlap` | `scheduler-owned-only` |
| 95 | `johnny1` | `high` | 1973/1945 | 25 | `123..147` (24s) | 2 | `risky:multi-partial-overlap` | `scheduler-owned-only` |
| 96 | `johnny1` | `high` | 1973/1945 | 25 | `145..169` (24s) | 1 | `risky:overread` | `scheduler-owned-only` |
| 97 | `johnny1` | `high` | 1973/1945 | 25 | `56..72` (16s) | 1 | `risky:multi-partial-overlap` | `scheduler-owned-only` |
| 98 | `johnny1` | `high` | 1973/1945 | 25 | `56..80` (24s) | 1 | `risky:multi-partial-overlap` | `scheduler-owned-only` |
| 99 | `visitor3` | `high` | 1065/1039 | 41 | `220..244` (24s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 100 | `visitor3` | `low` | 1062/1040 | 42 | `231..255` (24s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 101 | `visitor3` | `low` | 1062/1040 | 42 | `248..272` (24s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 102 | `visitor3` | `low` | 1062/1040 | 42 | `240..264` (24s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 103 | `walkstuf1` | `low` | 1478/1431 | 64 | `297..321` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 104 | `walkstuf1` | `low` | 1478/1431 | 64 | `285..309` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 105 | `walkstuf1` | `low` | 1478/1431 | 64 | `291..315` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 106 | `walkstuf1` | `low` | 1478/1431 | 64 | `273..297` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 107 | `walkstuf1` | `low` | 1478/1431 | 64 | `155..179` (24s) | 3 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 108 | `walkstuf1` | `low` | 1478/1431 | 64 | `297..313` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 109 | `walkstuf1` | `low` | 1478/1431 | 64 | `305..321` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 110 | `walkstuf1` | `low` | 1478/1431 | 64 | `291..307` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 111 | `walkstuf1` | `low` | 1478/1431 | 64 | `371..387` (16s) | 2 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 112 | `walkstuf1` | `low` | 1478/1431 | 64 | `285..297` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 113 | `walkstuf1` | `low` | 1478/1431 | 64 | `297..309` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 114 | `walkstuf1` | `low` | 1478/1431 | 64 | `190..202` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 115 | `walkstuf1` | `low` | 1478/1431 | 64 | `273..285` (12s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 116 | `walkstuf1` | `low` | 1478/1431 | 64 | `291..297` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 117 | `walkstuf1` | `low` | 1478/1431 | 64 | `305..311` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 118 | `walkstuf1` | `low` | 1478/1431 | 64 | `303..309` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 119 | `walkstuf1` | `high` | 1476/1434 | 81 | `298..322` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 120 | `walkstuf1` | `high` | 1476/1434 | 81 | `287..311` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |

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
