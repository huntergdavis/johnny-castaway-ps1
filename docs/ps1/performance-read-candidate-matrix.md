# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/building4-restore-minus-current-v087-broad/20260507-183004-1637867`
- Candidate rows: `73`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `21`
- Closed exact ranges from experiment log: `23`
- Phase-trap rows: `47`
- Deferred under-target rows: `12`
- Top next lanes: `frame-deadline-data-shape-or-render-reduction`=26, `direct-read-probe`=24, `non-scalar-data-shape-or-generated-owner`=11, `custom-terminal-data-shape-or-generated-deadline`=6, `terminal-payload-placement-or-deadline-sidecar`=6

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.
When direct standalone/guarded probes are exhausted, promote the listed
next lanes above more scalar range retries.

No open standalone or guarded direct-read probes remain in this
artifact set. The next optimization pass should start from generated
deadline ownership, custom data-shape, or pack-owned work reduction
lanes instead of another hand-authored sector range.

## Top 40 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Phase Trap | Next Lane | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|---|---|
| 1 | `building2` | `high` | 1394/1301 | 138 | `72..84` (12s) | 1 | `risky:short-visible-gap` | `no` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 2 | `building4` | `high` | 2844/2816 | 37 | `337..353` (16s) | 1 | `risky:short-visible-gap` | `no` | `direct-read-probe` | `scheduler-owned-only` |
| 3 | `building4` | `high` | 2844/2816 | 37 | `176..192` (16s) | 1 | `risky:short-visible-gap` | `no` | `direct-read-probe` | `scheduler-owned-only` |
| 4 | `activity9` | `low` | 2085/2058 | 29 | `418..434` (16s) | 1 | `risky:short-visible-gap` | `no` | `direct-read-probe` | `scheduler-owned-only` |
| 5 | `activity9` | `low` | 2085/2058 | 29 | `325..341` (16s) | 1 | `risky:short-visible-gap` | `no` | `direct-read-probe` | `scheduler-owned-only` |
| 6 | `activity9` | `low` | 2085/2058 | 29 | `341..357` (16s) | 1 | `risky:short-visible-gap` | `no` | `direct-read-probe` | `scheduler-owned-only` |
| 7 | `activity9` | `low` | 2085/2058 | 29 | `263..275` (12s) | 1 | `risky:short-visible-gap` | `no` | `direct-read-probe` | `scheduler-owned-only` |
| 8 | `activity9` | `low` | 2085/2058 | 29 | `245..257` (12s) | 1 | `risky:short-visible-gap` | `no` | `direct-read-probe` | `scheduler-owned-only` |
| 9 | `building2` | `high` | 1394/1301 | 138 | `24..36` (12s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 10 | `building2` | `high` | 1394/1301 | 138 | `18..30` (12s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 11 | `building2` | `high` | 1394/1301 | 138 | `18..34` (16s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 12 | `building2` | `high` | 1394/1301 | 138 | `24..40` (16s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 13 | `building2` | `high` | 1394/1301 | 138 | `24..30` (6s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 14 | `building2` | `high` | 1394/1301 | 138 | `25..31` (6s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 15 | `building2` | `low` | 1385/1303 | 121 | `371..387` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 16 | `building2` | `low` | 1385/1303 | 121 | `377..393` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 17 | `building2` | `low` | 1385/1303 | 121 | `284..296` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 18 | `building2` | `low` | 1385/1303 | 121 | `74..86` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 19 | `building2` | `low` | 1385/1303 | 121 | `204..216` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 20 | `activity9` | `low` | 2085/2058 | 29 | `251..263` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `non-scalar-data-shape-or-generated-owner` | `scheduler-owned-only` |
| 21 | `activity9` | `low` | 2085/2058 | 29 | `279..291` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `non-scalar-data-shape-or-generated-owner` | `scheduler-owned-only` |
| 22 | `visitor3` | `low` | 1140/1024 | 194 | `97..121` (24s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 23 | `visitor3` | `low` | 1140/1024 | 194 | `97..109` (12s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 24 | `visitor3` | `low` | 1140/1024 | 194 | `97..113` (16s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 25 | `visitor3` | `high` | 1139/1024 | 191 | `97..121` (24s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 26 | `visitor3` | `high` | 1139/1024 | 191 | `97..109` (12s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 27 | `visitor3` | `high` | 1139/1024 | 191 | `97..113` (16s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 28 | `building2` | `high` | 1394/1301 | 138 | `11..35` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 29 | `building2` | `high` | 1394/1301 | 138 | `3..27` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 30 | `building2` | `high` | 1394/1301 | 138 | `11..27` (16s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 31 | `building2` | `high` | 1394/1301 | 138 | `365..381` (16s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 32 | `building2` | `high` | 1394/1301 | 138 | `371..383` (12s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 33 | `building2` | `low` | 1385/1303 | 121 | `67..91` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 34 | `building2` | `low` | 1385/1303 | 121 | `365..381` (16s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 35 | `building2` | `low` | 1385/1303 | 121 | `204..220` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 36 | `building2` | `low` | 1385/1303 | 121 | `371..383` (12s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 37 | `building4` | `low` | 2855/2815 | 46 | `178..202` (24s) | 1 | `balanced:medium-visible-gap` | `closed-exact-range` | `non-scalar-data-shape-or-generated-owner` | `closed-by-experiment-log` |
| 38 | `building4` | `low` | 2855/2815 | 46 | `262..286` (24s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `non-scalar-data-shape-or-generated-owner` | `closed-by-experiment-log` |
| 39 | `building4` | `low` | 2855/2815 | 46 | `274..298` (24s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `non-scalar-data-shape-or-generated-owner` | `closed-by-experiment-log` |
| 40 | `building4` | `high` | 2844/2816 | 37 | `23..47` (24s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `non-scalar-data-shape-or-generated-owner` | `closed-by-experiment-log` |

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
- `phase_trap=yes` marks rows whose exact range is closed, whose visible
  gap is too tight, or whose prior risk class says scheduler ownership
  is required before the read can fire safely.
- `next_lane` is the non-scalar lane to try before another local sector
  table retry for that row.
- `artifact` points back to the source read-plan JSON for full read
  segments, gaps, and coverage.
