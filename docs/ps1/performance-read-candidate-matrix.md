# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/visitor3-high-remove-144-160-v082-canaries/20260507-101742-2985323`
- Candidate rows: `127`
- Standalone probes: `0`
- Scheduler or guarded probes: `9`
- Scheduler-owned only: `50`
- Closed exact ranges from experiment log: `12`
- Deferred under-target rows: `12`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `building4` | `high` | 2939/2786 | 240 | `398..414` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 2 | `building4` | `high` | 2939/2786 | 240 | `390..406` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 3 | `building4` | `high` | 2939/2786 | 240 | `663..675` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 4 | `activity9` | `low` | 2087/2056 | 42 | `514..530` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 5 | `activity9` | `low` | 2087/2056 | 42 | `498..514` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 6 | `activity9` | `low` | 2087/2056 | 42 | `507..523` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 7 | `fishing3` | `high` | 1962/1950 | 17 | `135..151` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 8 | `fishing3` | `high` | 1962/1950 | 17 | `202..218` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 9 | `fishing3` | `high` | 1962/1950 | 17 | `209..225` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 10 | `visitor3` | `low` | 1405/1015 | 301 | `106..122` (16s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `visitor3` | `low` | 1405/1015 | 301 | `99..111` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `visitor3` | `low` | 1405/1015 | 301 | `106..118` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `visitor3` | `low` | 1405/1015 | 301 | `108..114` (6s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `visitor3` | `low` | 1405/1015 | 301 | `105..111` (6s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `visitor3` | `low` | 1405/1015 | 301 | `106..112` (6s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `visitor3` | `low` | 1405/1015 | 301 | `110..116` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `visitor3` | `high` | 1406/1019 | 293 | `111..127` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 18 | `building4` | `high` | 2939/2786 | 240 | `531..547` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 19 | `building4` | `high` | 2939/2786 | 240 | `552..568` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `building4` | `high` | 2939/2786 | 240 | `414..426` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `building4` | `high` | 2939/2786 | 240 | `671..683` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `building4` | `high` | 2939/2786 | 240 | `525..537` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `building2` | `low` | 1429/1286 | 193 | `99..115` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `building2` | `low` | 1429/1286 | 193 | `388..404` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `building2` | `low` | 1429/1286 | 193 | `245..261` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `building2` | `low` | 1429/1286 | 193 | `216..232` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `building2` | `low` | 1429/1286 | 193 | `120..132` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `building2` | `low` | 1429/1286 | 193 | `99..111` (12s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 29 | `building2` | `low` | 1429/1286 | 193 | `382..394` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `building2` | `high` | 1430/1289 | 212 | `55..67` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `building2` | `high` | 1430/1289 | 212 | `55..71` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 32 | `building2` | `high` | 1430/1289 | 212 | `60..76` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 33 | `building2` | `high` | 1430/1289 | 212 | `11..23` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 34 | `building2` | `high` | 1430/1289 | 212 | `388..404` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 35 | `building2` | `high` | 1430/1289 | 212 | `79..95` (16s) | 2 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 36 | `building2` | `high` | 1430/1289 | 212 | `60..66` (6s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 37 | `building2` | `high` | 1430/1289 | 212 | `17..29` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 38 | `building2` | `high` | 1430/1289 | 212 | `60..72` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 39 | `building2` | `high` | 1430/1289 | 212 | `17..23` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 40 | `building2` | `high` | 1430/1289 | 212 | `62..68` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |

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
