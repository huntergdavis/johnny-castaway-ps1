# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/fgp3v4-drawcompact-all-v082-broad/20260507-143445-284827`
- Candidate rows: `66`
- Standalone probes: `0`
- Scheduler or guarded probes: `5`
- Scheduler-owned only: `23`
- Closed exact ranges from experiment log: `6`
- Deferred under-target rows: `12`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `visitor3` | `low` | 1376/1023 | 253 | `111..123` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 2 | `visitor3` | `high` | 1369/1023 | 244 | `111..123` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 3 | `building2` | `high` | 1405/1298 | 176 | `371..383` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 4 | `building2` | `low` | 1395/1294 | 144 | `371..383` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 5 | `activity9` | `low` | 2085/2058 | 28 | `229..245` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 6 | `building2` | `high` | 1405/1298 | 176 | `11..27` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 7 | `building2` | `high` | 1405/1298 | 176 | `24..36` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 8 | `building2` | `high` | 1405/1298 | 176 | `18..30` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 9 | `building2` | `high` | 1405/1298 | 176 | `18..34` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `building2` | `high` | 1405/1298 | 176 | `24..40` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `building2` | `high` | 1405/1298 | 176 | `365..381` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 12 | `building2` | `high` | 1405/1298 | 176 | `24..30` (6s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `building2` | `high` | 1405/1298 | 176 | `72..84` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 14 | `building2` | `high` | 1405/1298 | 176 | `25..31` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `building2` | `low` | 1395/1294 | 144 | `365..381` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 16 | `building2` | `low` | 1395/1294 | 144 | `204..220` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `building2` | `low` | 1395/1294 | 144 | `95..111` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `building2` | `low` | 1395/1294 | 144 | `165..181` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `building2` | `low` | 1395/1294 | 144 | `284..296` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `building2` | `low` | 1395/1294 | 144 | `95..107` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 21 | `building2` | `low` | 1395/1294 | 144 | `159..171` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 22 | `activity9` | `low` | 2085/2058 | 28 | `325..341` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 23 | `activity9` | `low` | 2085/2058 | 28 | `136..152` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 24 | `activity9` | `low` | 2085/2058 | 28 | `341..357` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 25 | `activity9` | `low` | 2085/2058 | 28 | `263..275` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 26 | `activity9` | `low` | 2085/2058 | 28 | `245..257` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 27 | `activity9` | `low` | 2085/2058 | 28 | `251..263` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `activity9` | `low` | 2085/2058 | 28 | `279..291` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `visitor3` | `low` | 1376/1023 | 253 | `97..113` (16s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 30 | `visitor3` | `low` | 1376/1023 | 253 | `111..127` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 31 | `visitor3` | `low` | 1376/1023 | 253 | `104..120` (16s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 32 | `visitor3` | `high` | 1369/1023 | 244 | `97..113` (16s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
| 33 | `visitor3` | `high` | 1369/1023 | 244 | `111..127` (16s) | 1 | `balanced:validate-overlap` | `closed-by-experiment-log` |
| 34 | `visitor3` | `high` | 1369/1023 | 244 | `104..120` (16s) | 1 | `risky:short-visible-gap` | `closed-by-experiment-log` |
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
