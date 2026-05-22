# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/b2high-phase1-five-yellow-noreq-current/20260521-233813-912248`
- Candidate rows: `54`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `9`
- Closed exact ranges from experiment log: `45`
- Phase-trap rows: `54`
- Deferred under-target rows: `0`
- Top next lanes: `frame-deadline-data-shape-or-render-reduction`=13, `custom-terminal-data-shape-or-generated-deadline`=12, `no-decode-canonicalization-or-generated-owner`=12, `generated-deadline-or-sector-split-data-shape`=10, `terminal-payload-placement-or-deadline-sidecar`=7

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
| 1 | `visitor3` | `low` | 1069/1039 | 68 | `46..70` (24s) | 2 | `risky:overread` | `high-risk-scheduler` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 2 | `visitor3` | `low` | 1069/1039 | 68 | `39..63` (24s) | 2 | `risky:overread` | `high-risk-scheduler` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 3 | `visitor3` | `low` | 1069/1039 | 68 | `1..17` (16s) | 1 | `risky:short-visible-gap` | `tight-first-gap` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 4 | `visitor3` | `low` | 1069/1039 | 68 | `240..252` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 5 | `visitor3` | `high` | 1067/1045 | 32 | `55..61` (6s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 6 | `visitor3` | `high` | 1067/1045 | 32 | `55..67` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 7 | `visitor3` | `high` | 1067/1045 | 32 | `55..71` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 8 | `visitor3` | `high` | 1067/1045 | 32 | `55..79` (24s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 9 | `visitor3` | `high` | 1067/1045 | 32 | `56..62` (6s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 10 | `walkstuf1` | `high` | 1472/1441 | 43 | `84..108` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 11 | `walkstuf1` | `high` | 1472/1441 | 43 | `365..389` (24s) | 3 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 12 | `walkstuf1` | `high` | 1472/1441 | 43 | `74..98` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 13 | `walkstuf1` | `high` | 1472/1441 | 43 | `92..116` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 14 | `walkstuf1` | `high` | 1472/1441 | 43 | `372..388` (16s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 15 | `walkstuf1` | `high` | 1472/1441 | 43 | `379..395` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 16 | `walkstuf1` | `high` | 1472/1441 | 43 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 17 | `walkstuf1` | `high` | 1472/1441 | 43 | `92..108` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 18 | `walkstuf1` | `high` | 1472/1441 | 43 | `84..100` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 19 | `walkstuf1` | `high` | 1472/1441 | 43 | `268..280` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 20 | `walkstuf1` | `high` | 1472/1441 | 43 | `74..86` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 21 | `walkstuf1` | `high` | 1472/1441 | 43 | `379..391` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 22 | `visitor3` | `low` | 1069/1039 | 68 | `32..56` (24s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 23 | `visitor3` | `low` | 1069/1039 | 68 | `52..76` (24s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 24 | `visitor3` | `low` | 1069/1039 | 68 | `239..251` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 25 | `visitor3` | `low` | 1069/1039 | 68 | `239..255` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 26 | `visitor3` | `low` | 1069/1039 | 68 | `46..58` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 27 | `visitor3` | `low` | 1069/1039 | 68 | `32..48` (16s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 28 | `visitor3` | `low` | 1069/1039 | 68 | `58..74` (16s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 29 | `visitor3` | `low` | 1069/1039 | 68 | `256..268` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 30 | `building2` | `high` | 1340/1314 | 45 | `135..159` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 31 | `building2` | `high` | 1340/1314 | 45 | `90..114` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 32 | `building2` | `high` | 1340/1314 | 45 | `122..146` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 33 | `building2` | `high` | 1340/1314 | 45 | `104..128` (24s) | 4 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 34 | `building2` | `high` | 1340/1314 | 45 | `249..265` (16s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 35 | `building2` | `high` | 1340/1314 | 45 | `318..334` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 36 | `building2` | `high` | 1340/1314 | 45 | `310..326` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 37 | `building2` | `high` | 1340/1314 | 45 | `82..98` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 38 | `building2` | `high` | 1340/1314 | 45 | `249..261` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 39 | `building2` | `high` | 1340/1314 | 45 | `90..96` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |
| 40 | `building2` | `high` | 1340/1314 | 45 | `310..322` (12s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `frame-deadline-data-shape-or-render-reduction` | `closed-by-experiment-log` |

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
