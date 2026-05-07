# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/walkstuf1-low-primecap160-v081-canaries/20260507-064747-1797388`
- Candidate rows: `128`
- Standalone probes: `0`
- Scheduler or guarded probes: `8`
- Scheduler-owned only: `58`
- Closed exact ranges from experiment log: `6`
- Deferred under-target rows: `12`

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|
| 1 | `visitor3` | `high` | 1406/1019 | 296 | `144..160` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 2 | `visitor3` | `high` | 1406/1019 | 296 | `138..154` (16s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 3 | `building4` | `high` | 2939/2786 | 240 | `398..414` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 4 | `building4` | `high` | 2939/2786 | 240 | `390..406` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 5 | `building4` | `high` | 2939/2786 | 240 | `663..675` (12s) | 1 | `balanced:validate-overlap` | `scheduler-or-guarded-probe` |
| 6 | `activity9` | `low` | 2087/2056 | 42 | `514..530` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 7 | `activity9` | `low` | 2087/2056 | 42 | `498..514` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 8 | `activity9` | `low` | 2087/2056 | 42 | `507..523` (16s) | 1 | `balanced:medium-visible-gap` | `scheduler-or-guarded-probe` |
| 9 | `visitor3` | `low` | 1405/1015 | 301 | `105..121` (16s) | 5 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 10 | `visitor3` | `low` | 1405/1015 | 301 | `99..115` (16s) | 5 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 11 | `visitor3` | `low` | 1405/1015 | 301 | `106..122` (16s) | 4 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 12 | `visitor3` | `low` | 1405/1015 | 301 | `99..111` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 13 | `visitor3` | `low` | 1405/1015 | 301 | `106..118` (12s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 14 | `visitor3` | `low` | 1405/1015 | 301 | `108..114` (6s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 15 | `visitor3` | `low` | 1405/1015 | 301 | `105..111` (6s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 16 | `visitor3` | `low` | 1405/1015 | 301 | `106..112` (6s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 17 | `visitor3` | `low` | 1405/1015 | 301 | `110..116` (6s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 18 | `visitor3` | `high` | 1406/1019 | 296 | `104..120` (16s) | 1 | `risky:short-visible-gap` | `scheduler-owned-only` |
| 19 | `walkstuf1` | `low` | 1604/1407 | 270 | `297..313` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 20 | `walkstuf1` | `low` | 1604/1407 | 270 | `702..718` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 21 | `walkstuf1` | `low` | 1604/1407 | 270 | `734..750` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 22 | `walkstuf1` | `low` | 1604/1407 | 270 | `281..297` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 23 | `walkstuf1` | `low` | 1604/1407 | 270 | `297..309` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 24 | `walkstuf1` | `low` | 1604/1407 | 270 | `702..714` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 25 | `walkstuf1` | `low` | 1604/1407 | 270 | `281..293` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 26 | `walkstuf1` | `low` | 1604/1407 | 270 | `336..348` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 27 | `walkstuf1` | `high` | 1595/1403 | 278 | `72..88` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 28 | `walkstuf1` | `high` | 1595/1403 | 278 | `734..750` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 29 | `walkstuf1` | `high` | 1595/1403 | 278 | `281..297` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 30 | `walkstuf1` | `high` | 1595/1403 | 278 | `344..360` (16s) | 3 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 31 | `walkstuf1` | `high` | 1595/1403 | 278 | `281..293` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 32 | `walkstuf1` | `high` | 1595/1403 | 278 | `297..309` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 33 | `walkstuf1` | `high` | 1595/1403 | 278 | `344..356` (12s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 34 | `building4` | `high` | 2939/2786 | 240 | `531..547` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 35 | `building4` | `high` | 2939/2786 | 240 | `552..568` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 36 | `building4` | `high` | 2939/2786 | 240 | `414..426` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 37 | `building4` | `high` | 2939/2786 | 240 | `671..683` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 38 | `building4` | `high` | 2939/2786 | 240 | `525..537` (12s) | 1 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 39 | `building2` | `low` | 1429/1286 | 193 | `99..115` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |
| 40 | `building2` | `low` | 1429/1286 | 193 | `388..404` (16s) | 2 | `unsafe:tight-visible-gap` | `scheduler-owned-only` |

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
