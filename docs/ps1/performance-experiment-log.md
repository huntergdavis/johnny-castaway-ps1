# PS1 Performance Experiment Log

Date started: 2026-04-25

Purpose: tracked, searchable ledger for PS1 scene-playback performance
experiments. Scratch JSONL logs remain useful locally, but this file records the
decision history that should survive cleanup, branch rebases, and later retry
passes.

Current accepted baseline for this log:

| Field | Value |
|---|---|
| Branch | `ps1-perf-prefetch-experiments` |
| Commit | `4ddc89f4 ps1: clean up headless perf containers` |
| Run ID | `20260425-143847` |
| Scene | `fishing1` |
| Boot | `fgpilot fishing1 perf-log noloop seed 1` |
| Policy | `stage1_window` |
| Window | `32 KB default` |
| `loop_vb` | `1426` |
| `target_vb` | `1077` |
| `overrun_vb` | `349` |
| `blocking_vb` | `148` |
| `loop_reads` | `29` |
| `prefetch_hits` | `150` |
| `prefetch_due_misses` | `5` |
| `prefetch_overrun_vb` | `104` |
| Correctness | `trip=0 fallback=0 frame_mismatch=0 sound_late=0 cd_fail=0 full_fallbacks=0` |

## Experiments

| Date | ID | Commit | Hypothesis | Command | Result | Decision |
|---|---|---|---|---|---|---|
| 2026-04-25 | `fg2-window-40kb` | `4ddc89f4` | A `40 KB` FG2 stream window might reduce due misses relative to the accepted `32 KB` default without reintroducing refill overrun. | `./scripts/ps1-perf-iterate.sh --case "fishing1::fgpilot fishing1 prefetch-window 40960" --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-143847/summary.json --require-improvement` | Failed before `JCPERF2`; headless regtest exited `137`, gate failure `missing_jcperf2`. No usable scene metrics. Artifacts: `scratch/ps1-perf-iterate/20260425-144406/fishing1/headless-regtest.log`, `scratch/ps1-perf-iterate/20260425-144406/fishing1/perf-summary.json`. | Do not promote. Keep as a retry candidate after headless stability work or a lower-memory/dual-window design. |
| 2026-04-25 | `fg2-window-56kb` | `1fe1ceda` | A `56 KB` FG2 stream window might reduce reads and due misses enough to beat the `32 KB` default while staying below the failed `64 KB` pressure point. | `./scripts/ps1-perf-iterate.sh --case "fishing1::fgpilot fishing1 prefetch-window 57344" --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-143847/summary.json --require-improvement` | Completed with correct pixels/sound counters but failed the improvement gate. Baseline to current: `loop_vb 1426 -> 1442`, `blocking_vb 148 -> 170`, `prefetch_overrun_vb 104 -> 140`, `loop_reads 29 -> 16`, `due_misses 5 -> 2`. Artifacts: `scratch/ps1-perf-iterate/20260425-144631/fishing1/headless-regtest.log`, `scratch/ps1-perf-iterate/20260425-144631/fishing1/perf-summary.json`. | Do not promote. Larger windows reduce transaction count but overrun held slack more often; prefer smarter refill timing/layout over simply increasing the buffer. |
| 2026-04-25 | `fg2-no-stage1-isolation` | `816b7c28` | Disabling the one-entry stage buffer might show whether stage-copy overhead or stage/window interaction is hurting the current default. | `./scripts/ps1-perf-iterate.sh --case "fishing1::fgpilot fishing1 no-stage1" --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-143847/summary.json --require-improvement` | Failed before `JCPERF2`; headless regtest exited `137`, gate failure `missing_jcperf2`. No usable scene metrics. Artifacts: `scratch/ps1-perf-iterate/20260425-144914/fishing1/headless-regtest.log`, `scratch/ps1-perf-iterate/20260425-144914/fishing1/perf-summary.json`. | Do not promote and do not infer that staging is slow. Keep staging enabled until a clean isolation run says otherwise. |

## Retry Queue

| Candidate | Reason to revisit |
|---|---|
| `40 KB` stream window | Failed structurally, not because metrics proved it slow. Retry only after the headless exit-137 failure mode is understood or memory pressure is reduced elsewhere. |
| `56 KB` stream window | Correct but slower than `32 KB`; retry only if later changes can hide larger refill reads instead of blocking on them. |
| `64 KB` stream window | Earlier run also exited `137` before `JCPERF2`; likely too much pressure for the current simple single-window strategy. |
| `16 KB` stream window | Produced partial metrics but regtest exited `137`; metrics were worse on `blocking_vb` and `due_misses`, so this is low priority. |
| `no-stage1` window-only path | Failed structurally before metrics; retry only after headless stability improves or after stage/window code is simplified. |

## Promotion Rule

An experiment is promotable only when the headless perf gate passes, correctness
tripwires remain zero, and at least one key speed metric improves without a
material regression in `loop_vb`, `blocking_vb`, `prefetch_overrun_vb`, or
scene identity. Failed experiments still get recorded here so later changes can
make them eligible for retry.
