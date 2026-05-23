# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/w1high-layout-owner372-384-four-yellow-norequire-current-20260522/20260522-232432-869167`
- Candidate rows: `42`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `26`
- Closed exact ranges from experiment log: `16`
- Phase-trap rows: `41`
- Deferred under-target rows: `0`
- Top next lanes: `no-decode-canonicalization-or-generated-owner`=15, `frame-deadline-data-shape-or-render-reduction`=12, `custom-terminal-data-shape-or-generated-deadline`=8, `terminal-payload-placement-or-deadline-sidecar`=7

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.
When direct standalone/guarded probes are exhausted, promote the listed
next lanes above more scalar range retries.

No open standalone or guarded direct-read probes remain in this
artifact set. The next optimization pass should start from generated
deadline ownership, custom data-shape, or pack-owned work reduction
lanes instead of another hand-authored sector range.

## Top 30 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Phase Trap | Next Lane | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|---|---|
| 1 | `building2` | `high` | 1330/1317 | 32 | `61..73` (12s) | 1 | `risky:short-visible-gap` | `no` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 2 | `walkstuf1` | `high` | 1468/1442 | 39 | `383..407` (24s) | 5 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `high` | 1468/1442 | 39 | `383..399` (16s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 4 | `walkstuf1` | `high` | 1468/1442 | 39 | `388..404` (16s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `high` | 1468/1442 | 39 | `383..395` (12s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 6 | `walkstuf1` | `high` | 1468/1442 | 39 | `386..402` (16s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 7 | `walkstuf1` | `high` | 1468/1442 | 39 | `386..392` (6s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 8 | `walkstuf1` | `high` | 1468/1442 | 39 | `383..389` (6s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 9 | `walkstuf1` | `high` | 1468/1442 | 39 | `388..394` (6s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 10 | `visitor3` | `low` | 1065/1041 | 50 | `38..50` (12s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 11 | `visitor3` | `low` | 1065/1041 | 50 | `38..54` (16s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 12 | `visitor3` | `low` | 1065/1041 | 50 | `38..44` (6s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 13 | `visitor3` | `low` | 1065/1041 | 50 | `39..51` (12s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 14 | `visitor3` | `low` | 1065/1041 | 50 | `39..45` (6s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 15 | `visitor3` | `high` | 1067/1045 | 32 | `55..67` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 16 | `visitor3` | `high` | 1067/1045 | 32 | `55..71` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 17 | `visitor3` | `high` | 1067/1045 | 32 | `56..62` (6s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 18 | `building2` | `high` | 1330/1317 | 32 | `128..152` (24s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 19 | `building2` | `high` | 1330/1317 | 32 | `147..171` (24s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 20 | `building2` | `high` | 1330/1317 | 32 | `143..167` (24s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 21 | `building2` | `high` | 1330/1317 | 32 | `178..190` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 22 | `building2` | `high` | 1330/1317 | 32 | `152..164` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 23 | `building2` | `high` | 1330/1317 | 32 | `135..147` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 24 | `building2` | `high` | 1330/1317 | 32 | `182..198` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 25 | `building2` | `high` | 1330/1317 | 32 | `128..144` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 26 | `building2` | `high` | 1330/1317 | 32 | `68..84` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 27 | `walkstuf1` | `high` | 1468/1442 | 39 | `379..403` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 28 | `walkstuf1` | `high` | 1468/1442 | 39 | `372..396` (24s) | 5 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 29 | `walkstuf1` | `high` | 1468/1442 | 39 | `379..395` (16s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 30 | `walkstuf1` | `high` | 1468/1442 | 39 | `365..389` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |

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
