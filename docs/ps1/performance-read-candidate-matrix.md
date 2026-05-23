# PS1 Foreground Read Candidate Matrix

This host-side report aggregates the current `foreground-read-plan.json`
artifacts and ranks candidate retained-window read groups by scene
pressure and visible-cadence risk. It does not change the PS1 binary.

- Source artifact root: `scratch/ps1-perf-iterate/visitor3-low-phase1-seg4-38-79-under-yellow-canary-20260523/20260523-042947-2959812`
- Candidate rows: `32`
- Standalone probes: `0`
- Scheduler or guarded probes: `0`
- Scheduler-owned only: `15`
- Closed exact ranges from experiment log: `17`
- Phase-trap rows: `30`
- Deferred under-target rows: `0`
- Top next lanes: `no-decode-canonicalization-or-generated-owner`=13, `custom-terminal-data-shape-or-generated-deadline`=12, `terminal-payload-placement-or-deadline-sidecar`=7

Recent hand-authored table probes proved that nominal read-count wins can
still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and
`unsafe` rows as scheduler-owned retries, not standalone table changes.
When direct standalone/guarded probes are exhausted, promote the listed
next lanes above more scalar range retries.

No open standalone or guarded direct-read probes remain in this
artifact set. The next optimization pass should start from generated
deadline ownership, custom data-shape, or pack-owned work reduction
lanes instead of another hand-authored sector range.

## Top 32 Candidates

| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Phase Trap | Next Lane | Recommendation |
|---:|---|---|---:|---:|---|---:|---|---|---|---|
| 1 | `walkstuf1` | `high` | 1466/1445 | 35 | `159..175` (16s) | 1 | `risky:short-visible-gap` | `no` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 2 | `visitor3` | `low` | 1062/1045 | 38 | `89..105` (16s) | 1 | `risky:short-visible-gap` | `no` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 3 | `walkstuf1` | `high` | 1466/1445 | 35 | `358..382` (24s) | 4 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 4 | `walkstuf1` | `high` | 1466/1445 | 35 | `253..265` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 5 | `walkstuf1` | `high` | 1466/1445 | 35 | `246..258` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 6 | `walkstuf1` | `high` | 1466/1445 | 35 | `377..383` (6s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `no-decode-canonicalization-or-generated-owner` | `scheduler-owned-only` |
| 7 | `visitor3` | `high` | 1065/1046 | 34 | `83..89` (6s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 8 | `visitor3` | `high` | 1065/1046 | 34 | `83..95` (12s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 9 | `visitor3` | `high` | 1065/1046 | 34 | `83..99` (16s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 10 | `visitor3` | `high` | 1065/1046 | 34 | `83..107` (24s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `terminal-payload-placement-or-deadline-sidecar` | `scheduler-owned-only` |
| 11 | `visitor3` | `low` | 1062/1045 | 38 | `79..103` (24s) | 3 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 12 | `visitor3` | `low` | 1062/1045 | 38 | `89..113` (24s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 13 | `visitor3` | `low` | 1062/1045 | 38 | `82..106` (24s) | 2 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 14 | `visitor3` | `low` | 1062/1045 | 38 | `96..112` (16s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 15 | `visitor3` | `low` | 1062/1045 | 38 | `79..91` (12s) | 1 | `unsafe:tight-visible-gap` | `unsafe-visible-cost` | `custom-terminal-data-shape-or-generated-deadline` | `scheduler-owned-only` |
| 16 | `walkstuf1` | `high` | 1466/1445 | 35 | `74..98` (24s) | 3 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 17 | `walkstuf1` | `high` | 1466/1445 | 35 | `124..148` (24s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 18 | `walkstuf1` | `high` | 1466/1445 | 35 | `159..183` (24s) | 2 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 19 | `walkstuf1` | `high` | 1466/1445 | 35 | `80..92` (12s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 20 | `walkstuf1` | `high` | 1466/1445 | 35 | `365..381` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 21 | `walkstuf1` | `high` | 1466/1445 | 35 | `80..96` (16s) | 2 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 22 | `walkstuf1` | `high` | 1466/1445 | 35 | `124..140` (16s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 23 | `walkstuf1` | `high` | 1466/1445 | 35 | `74..86` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `no-decode-canonicalization-or-generated-owner` | `closed-by-experiment-log` |
| 24 | `visitor3` | `high` | 1065/1046 | 34 | `40..46` (6s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 25 | `visitor3` | `high` | 1065/1046 | 34 | `84..100` (16s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 26 | `visitor3` | `high` | 1065/1046 | 34 | `84..108` (24s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `terminal-payload-placement-or-deadline-sidecar` | `closed-by-experiment-log` |
| 27 | `visitor3` | `low` | 1062/1045 | 38 | `96..108` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 28 | `visitor3` | `low` | 1062/1045 | 38 | `9..33` (24s) | 1 | `balanced:validate-overlap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 29 | `visitor3` | `low` | 1062/1045 | 38 | `1..17` (16s) | 1 | `risky:short-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 30 | `visitor3` | `low` | 1062/1045 | 38 | `256..268` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 31 | `visitor3` | `low` | 1062/1045 | 38 | `239..251` (12s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |
| 32 | `visitor3` | `low` | 1062/1045 | 38 | `82..98` (16s) | 1 | `unsafe:tight-visible-gap` | `closed-exact-range` | `custom-terminal-data-shape-or-generated-deadline` | `closed-by-experiment-log` |

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
