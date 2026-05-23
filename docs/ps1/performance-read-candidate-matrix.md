# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/w1high-clip134-four-yellow-current-20260522/20260522-183450-3194636`
- Candidate rows: `39`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `22`
- Closed exact ranges from experiment log: `17`
- Phase-trap rows: `37`
- Deferred under-target rows: `0`
- Top next lanes: `frame-deadline-data-shape-or-render-reduction`=12, `no-decode-canonicalization-or-generated-owner`=12, `custom-terminal-data-shape-or-generated-deadline`=8, `terminal-payload-placement-or-deadline-sidecar`=7

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.
When direct standalone/guarded probes are exhausted, promote the listed
next lanes above more scalar range retries.

No open standalone or guarded direct-read probes remain in this
artifact set. The next optimization pass should start from generated
deadline ownership, custom data-shape, or pack-owned work reduction
lanes instead of another hand-authored sector range.

## Top 39 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Phase Trap | Next Lane | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|---|---|
| 1 | `walkstuf1` | `high` | 1469/1441 | 41 | `124..140` (16s) | 1 | `risky:short-visible-gap` | `no` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 2 | `building2` | `high` | 1330/1317 | 32 | `61..73` (12s) | 1 | `risky:short-visible-gap` | `no` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `high` | 1469/1441 | 41 | `372..396` (24s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 4 | `walkstuf1` | `high` | 1469/1441 | 41 | `80..96` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `high` | 1469/1441 | 41 | `253..265` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 6 | `visitor3` | `low` | 1065/1041 | 50 | `38..50` (12s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 7 | `visitor3` | `low` | 1065/1041 | 50 | `38..54` (16s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 8 | `visitor3` | `low` | 1065/1041 | 50 | `38..44` (6s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 9 | `visitor3` | `low` | 1065/1041 | 50 | `39..51` (12s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 10 | `visitor3` | `low` | 1065/1041 | 50 | `39..45` (6s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 11 | `visitor3` | `high` | 1067/1045 | 32 | `55..67` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 12 | `visitor3` | `high` | 1067/1045 | 32 | `55..71` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 13 | `visitor3` | `high` | 1067/1045 | 32 | `56..62` (6s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 14 | `building2` | `high` | 1330/1317 | 32 | `128..152` (24s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 15 | `building2` | `high` | 1330/1317 | 32 | `147..171` (24s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 16 | `building2` | `high` | 1330/1317 | 32 | `143..167` (24s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 17 | `building2` | `high` | 1330/1317 | 32 | `178..190` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 18 | `building2` | `high` | 1330/1317 | 32 | `152..164` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 19 | `building2` | `high` | 1330/1317 | 32 | `135..147` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 20 | `building2` | `high` | 1330/1317 | 32 | `182..198` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 21 | `building2` | `high` | 1330/1317 | 32 | `128..144` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 22 | `building2` | `high` | 1330/1317 | 32 | `68..84` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `frame-deadline-data-shape-or-render-reduction` | `scheduler-owned-only` |
| 23 | `walkstuf1` | `high` | 1469/1441 | 41 | `365..389` (24s) | 3 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 24 | `walkstuf1` | `high` | 1469/1441 | 41 | `74..98` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 25 | `walkstuf1` | `high` | 1469/1441 | 41 | `372..388` (16s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 26 | `walkstuf1` | `high` | 1469/1441 | 41 | `124..148` (24s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 27 | `walkstuf1` | `high` | 1469/1441 | 41 | `379..395` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 28 | `walkstuf1` | `high` | 1469/1441 | 41 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 29 | `walkstuf1` | `high` | 1469/1441 | 41 | `74..86` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 30 | `walkstuf1` | `high` | 1469/1441 | 41 | `379..391` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 31 | `visitor3` | `low` | 1065/1041 | 50 | `32..44` (12s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 32 | `visitor3` | `low` | 1065/1041 | 50 | `32..48` (16s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 33 | `visitor3` | `low` | 1065/1041 | 50 | `40..46` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 34 | `visitor3` | `high` | 1067/1045 | 32 | `40..46` (6s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 35 | `visitor3` | `high` | 1067/1045 | 32 | `42..48` (6s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 36 | `visitor3` | `high` | 1067/1045 | 32 | `55..61` (6s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 37 | `visitor3` | `high` | 1067/1045 | 32 | `55..79` (24s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 38 | `building2` | `high` | 1330/1317 | 32 | `55..79` (24s) | 3 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 39 | `building2` | `high` | 1330/1317 | 32 | `95..111` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |

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
