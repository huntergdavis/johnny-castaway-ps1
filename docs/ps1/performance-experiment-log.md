# PS1 Performance Experiment Log

Date started: 2026-04-25

Purpose: tracked, searchable ledger for PS1 scene-playback performance
experiments. Scratch JSONL logs remain useful locally, but this file records the
decision history that should survive cleanup, branch rebases, and later retry
passes.

Original comparison baseline for this experiment series:

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
| 2026-04-25 | `fg2-partial-tail-stage` | `1413e813` | When a staged frame straddles the current stream-window end, copy the resident prefix from the window and read only the missing tail into the stage buffer instead of refilling another overlapping full window. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-143847/summary.json --require-improvement` | Failed. The scene emitted valid metrics but the outer headless process exited `137`; metrics also regressed. Baseline to current: `loop_vb 1426 -> 1446`, `blocking_vb 148 -> 178`, `loop_reads 29 -> 37`, `due_misses 5 -> 9`, while `partial_hits=10` and `prefetch_overrun_vb 104 -> 98`. Artifacts: `scratch/ps1-perf-iterate/20260425-145256/fishing1/headless-regtest.log`, `scratch/ps1-perf-iterate/20260425-145256/fishing1/perf-summary.json`. | Do not promote. Smaller tail reads increase CD transaction count and due misses; avoid partial-frame reads unless grouped with a broader sequential-read design. |
| 2026-04-25 | `fg2-stage-copy-fallthrough` | `93a91a6a` | If `fgRuntimeTryStageNextFrame()` only copies the next frame from an already-resident window, use the same held iteration to prefetch farther ahead instead of waiting immediately. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-143847/summary.json --require-improvement` | Failed before `JCPERF2`; headless regtest exited `137`, gate failure `missing_jcperf2`. The log reached `JCPERF scene-start` and repeated FG2 directory lookups but emitted no scene-end metrics. Artifacts: `scratch/ps1-perf-iterate/20260425-145649/fishing1/headless-regtest.log`, `scratch/ps1-perf-iterate/20260425-145649/fishing1/perf-summary.json`. | Do not promote. The idea may be valid, but the naive same-iteration fallthrough appears to destabilize CD/window sequencing; revisit only with an explicit prefetch state machine. |
| 2026-04-25 | `x-aware-clean-restore` | `3cbbcae1` | Track dirty X extents per tile and use them only for RAM clean-background restore, leaving the existing full-row upload path unchanged. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-143847/summary.json --allow-regression 10 --require-improvement` | Promoted. Repeat run passed with correctness clean. Baseline to current: `loop_vb 1426 -> 1366`, `overrun_vb 349 -> 289`, `blocking_vb 148 -> 125`, `restore_bytes 16035840 -> 9580428`, `due_misses 5 -> 1`; `prefetch_overrun_vb` regressed `104 -> 113` but total playback improved materially. Artifacts: `scratch/ps1-perf-iterate/20260425-150236/fishing1/headless-regtest.log`, `scratch/ps1-perf-iterate/20260425-150236/fishing1/perf-summary.json`. | Promote. This is a real render-side speedup; next pass should tackle X-aware upload so `upload_bytes` stops dominating. |
| 2026-04-25 | `fg2-pal4-opaque-span-compositor` | `8d8c7095` | FG2 PAL4 spans contain only visible pixels, so the runtime can remove the per-pixel transparent-index branch and split each span by destination tile once instead of selecting tiles per pixel. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-151313/summary.json --allow-regression 2 --require-improvement` | Promoted. Two clean passes showed identical speedup; one intervening headless run hit the known pre-`JCPERF2` wrapper/container `137` failure and was rejected by the harness. Baseline to current: `loop_vb 1366 -> 1335`, `overrun_vb 289 -> 258`, `blocking_vb 125 -> 106`, `prefetch_overrun_vb 113 -> 94`. Final VRAM hash matched baseline: `330784fc30046503d5f273b28c80abe8e584d40fc7dca3ff1771e59aa57cfbb0`. Artifacts: `scratch/ps1-perf-iterate/20260425-151745/fishing1/perf-summary.json`, `scratch/ps1-perf-iterate/20260425-152021/fishing1/perf-summary.json`. | Promote. This is a compositor hot-path win with no pack format change; next candidates remain exact-X upload and smarter CD grouping. |
| 2026-04-25 | `prefetch-duplicate-probe-skip` | `7858b725` | Once the next entry is already staged, the held loop should only call window prefetch if it would issue a read; otherwise the duplicate stage/window probes are pure CPU noise. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-152021/summary.json --allow-regression 2` | Promoted as a micro-cleanup. Timing stayed flat: `loop_vb 1335 -> 1335`, `blocking_vb 106 -> 106`, `prefetch_overrun_vb 94 -> 94`; duplicate no-op probes dropped `887 -> 0`. VRAM and SPU hashes matched baseline. Artifact: `scratch/ps1-perf-iterate/20260425-152635/fishing1/perf-summary.json`. | Promote. Not a visible-speed win, but it removes redundant held-loop work and makes future prefetch metrics cleaner. |
| 2026-04-25 | `prefetch-min-slack-6vb` | `17da88d1` | Reject stream-window refills unless at least `6` held VBlanks remain, so short-slack CD reads cannot overrun into visible playback. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-152635/summary.json --allow-regression 2 --require-improvement` | Failed. It did reduce prefetch overrun (`94 -> 24`), but made due misses and blocking worse: `loop_vb 1335 -> 1341`, `overrun_vb 258 -> 264`, `blocking_vb 106 -> 139`, `due_misses 1 -> 14`, `skipped_no_slack 0 -> 79`. Artifact: `scratch/ps1-perf-iterate/20260425-153142/fishing1/perf-summary.json`. | Do not promote. The threshold is too strict; it proves that avoiding prefetch overrun is not enough if the due-frame path has to block more often. |
| 2026-04-25 | `prefetch-min-slack-3vb` | `17da88d1` | A smaller stream-window refill threshold should avoid the worst short-slack overrun without starving near-term frames. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-152635/summary.json --allow-regression 2 --require-improvement` | Promoted. Two clean passes matched exactly. Baseline to current: `loop_vb 1335 -> 1325`, `overrun_vb 258 -> 248`, `blocking_vb 106 -> 106`, `prefetch_overrun_vb 94 -> 67`, `skipped_no_slack 0 -> 11`; due misses rose `1 -> 4` but did not increase blocking time. Artifacts: `scratch/ps1-perf-iterate/20260425-153424/fishing1/perf-summary.json`, `scratch/ps1-perf-iterate/20260425-153546/fishing1/perf-summary.json`. | Promote. This is a small but repeatable CD timing win; next CD work should reduce read cost or grouping rather than raising the threshold. |
| 2026-04-25 | `prefetch-short-slack-direct-stage` | `effbe295` | If a window refill is rejected with only `1-2` held VBlanks left, directly stage the next small entry instead of skipping all prefetch work. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-153546/summary.json --allow-regression 2 --require-improvement` | Failed. One run died before `JCPERF2`; the next emitted full metrics but regressed: `loop_vb 1325 -> 1332`, `overrun_vb 248 -> 255`, `blocking_vb 106 -> 112`, `prefetch_overrun_vb 67 -> 82`, while due misses improved only `4 -> 3`. Reads rose `37 -> 42`. Artifact: `scratch/ps1-perf-iterate/20260425-154130/fishing1/perf-summary.json`. | Do not promote. More small direct reads are not free; avoid increasing CD transaction count unless pack layout or grouping also changes. |
| 2026-04-25 | `holiday-overlap-restamp` | `effbe295` | Seed holiday decoration into the clean backdrop and restamp it only when the current FG2 frame overlaps, reducing repeated holiday dirty/upload work. | `./scripts/ps1-perf-iterate.sh --scene fishing1 --frames 4200 --timeout 180 --baseline scratch/ps1-perf-iterate/20260425-153546/summary.json --allow-regression 2 --require-improvement` | Failed as a no-op. Correctness stayed clean, but all loop and gfx counters were unchanged: `loop_vb 1325 -> 1325`, `blocking_vb 106 -> 106`, `prefetch_overrun_vb 67 -> 67`, `restore_bytes 9580428 -> 9580428`, `upload_bytes 17277440 -> 17277440`. Artifact: `scratch/ps1-perf-iterate/20260425-154928/fishing1/perf-summary.json`. | Do not promote. Fishing1's active frames overlap the Christmas decoration enough that this does not reduce dirty work; revisit only for scenes/holidays where overlap is sparse. |

## Retry Queue

| Candidate | Reason to revisit |
|---|---|
| `40 KB` stream window | Failed structurally, not because metrics proved it slow. Retry only after the headless exit-137 failure mode is understood or memory pressure is reduced elsewhere. |
| `56 KB` stream window | Correct but slower than `32 KB`; retry only if later changes can hide larger refill reads instead of blocking on them. |
| `64 KB` stream window | Earlier run also exited `137` before `JCPERF2`; likely too much pressure for the current simple single-window strategy. |
| `16 KB` stream window | Produced partial metrics but regtest exited `137`; metrics were worse on `blocking_vb` and `due_misses`, so this is low priority. |
| `no-stage1` window-only path | Failed structurally before metrics; retry only after headless stability improves or after stage/window code is simplified. |
| Partial tail reads | Proved harmful in isolation because fewer bytes did not compensate for more transactions; retry only as part of pack/group layout work. |
| Same-iteration stage-copy fallthrough | Structural failure before metrics; retry only with explicit state that prevents repeated FG2 lookup/read churn. |
| `6` VBlank prefetch slack threshold | Correct but slower; retry only if later pack/layout changes reduce due-frame miss cost enough that stricter prefetch gating can win. |
| Short-slack direct staging | Correctness clean but slower; retry only if direct-read transaction overhead is reduced by grouped or physically adjacent pack reads. |
| Holiday overlap restamping | Correctness clean but no measured fishing1 benefit; retry only after per-scene/holiday overlap stats show sparse contact. |

## Harness Notes

- `regtest_exit=137` means the waited process was killed by `SIGKILL`, but it is not always a PS1/emulator failure. If `JCPERF2` is complete, the case gate is clean, and DuckStation logs `Exiting with success`, the perf harness treats it as a post-success wrapper failure and continues to aggregate the run.
- `regtest_exit=137` with missing `JCPERF2` remains a hard failure. Those runs are still recorded as structural failures because they do not prove correctness or speed.

## Promotion Rule

An experiment is promotable only when the headless perf gate passes, correctness
tripwires remain zero, and at least one key speed metric improves without a
material regression in `loop_vb`, `blocking_vb`, `prefetch_overrun_vb`, or
scene identity. Failed experiments still get recorded here so later changes can
make them eligible for retry.
