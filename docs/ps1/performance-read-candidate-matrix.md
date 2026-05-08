# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/building2-high-group60-72-v109-broad/20260508-013506-4063937`
- Candidate rows: `71`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `26`
- Closed exact ranges from experiment log: `8`
- Deferred under-target rows: `12`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `building2` | `low` | 1385/1303 | 121 | `365..381` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 2 | `building2` | `low` | 1385/1303 | 121 | `204..220` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 3 | `building2` | `low` | 1385/1303 | 121 | `371..387` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 4 | `building2` | `low` | 1385/1303 | 121 | `377..393` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 5 | `building2` | `low` | 1385/1303 | 121 | `284..296` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 6 | `building2` | `low` | 1385/1303 | 121 | `74..86` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 7 | `building2` | `low` | 1385/1303 | 121 | `204..216` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `building2` | `high` | 1349/1316 | 48 | `210..226` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `building2` | `high` | 1349/1316 | 48 | `226..242` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `building2` | `high` | 1349/1316 | 48 | `206..222` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `building2` | `high` | 1349/1316 | 48 | `202..218` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `building2` | `high` | 1349/1316 | 48 | `210..222` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `building2` | `high` | 1349/1316 | 48 | `226..238` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `building2` | `high` | 1349/1316 | 48 | `206..218` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `building2` | `high` | 1349/1316 | 48 | `185..197` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `building4` | `high` | 2844/2816 | 37 | `264..280` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 17 | `building4` | `high` | 2844/2816 | 37 | `31..47` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 18 | `building4` | `high` | 2844/2816 | 37 | `337..353` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 19 | `building4` | `high` | 2844/2816 | 37 | `176..192` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 20 | `activity9` | `low` | 2085/2058 | 29 | `418..434` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 21 | `activity9` | `low` | 2085/2058 | 29 | `325..341` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 22 | `activity9` | `low` | 2085/2058 | 29 | `341..357` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 23 | `activity9` | `low` | 2085/2058 | 29 | `263..275` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 24 | `activity9` | `low` | 2085/2058 | 29 | `245..257` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 25 | `activity9` | `low` | 2085/2058 | 29 | `251..263` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `activity9` | `low` | 2085/2058 | 29 | `279..291` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `visitor3` | `low` | 1138/1024 | 191 | `97..121` (24s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 28 | `visitor3` | `low` | 1138/1024 | 191 | `97..109` (12s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 29 | `visitor3` | `low` | 1138/1024 | 191 | `97..113` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 30 | `visitor3` | `high` | 1137/1024 | 190 | `97..121` (24s) | 2 | `unsafe:tight-visible-gap` | `closed-by-experiment-log` |
| 31 | `visitor3` | `high` | 1137/1024 | 190 | `97..109` (12s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 32 | `visitor3` | `high` | 1137/1024 | 190 | `97..113` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 33 | `building2` | `low` | 1385/1303 | 121 | `371..383` (12s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 34 | `activity9` | `low` | 2085/2058 | 29 | `229..245` (16s) | 1 | `balanced:medium-visible-gap` | `closed-by-experiment-log` |
| 35 | `fishing1` | `high` | 1068/1074 | 2 | `146..170` (24s) | 3 | `risky:short-visible-gap` | `defer-under-target` |
| 36 | `fishing1` | `high` | 1068/1074 | 2 | `158..182` (24s) | 3 | `risky:short-visible-gap` | `defer-under-target` |
| 37 | `fishing1` | `high` | 1068/1074 | 2 | `164..188` (24s) | 3 | `risky:short-visible-gap` | `defer-under-target` |
| 38 | `fishing1` | `high` | 1068/1074 | 2 | `125..149` (24s) | 2 | `balanced:validate-overlap` | `defer-under-target` |
| 39 | `fishing1` | `high` | 1068/1074 | 2 | `146..158` (12s) | 1 | `risky:short-visible-gap` | `defer-under-target` |
| 40 | `fishing1` | `high` | 1068/1074 | 2 | `170..182` (12s) | 1 | `risky:short-visible-gap` | `defer-under-target` |

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
