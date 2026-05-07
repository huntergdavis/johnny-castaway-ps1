# v0.8.2-ps1 Release Notes

**Date:** 2026-05-07
**Tag:** `v0.8.2-ps1`
**Theme:** VISITOR3 guarded-read performance and site/docs sync

`v0.8.2-ps1` is a performance point release after `v0.8.1-ps1`. All 63
scenes remain validated, all 126 high/low scene variants remain routed through
the headless perf matrix, and the current 120 timing-bearing rows average
`+0.5706%` over target / `99.6769%` target speed.

## Headline

- **VISITOR3 high visible CD pressure reduced again.** A guarded generated
  read window over sectors `138..162` now runs only when at least four VBlanks
  of scheduler slack are available.
- **Loop cadence stayed flat.** VISITOR3 high remains `1406/1019` VBlanks with
  `overrun_vb=387` and `prefetch_overrun_vb=7`.
- **Active CD work dropped.** VISITOR3 high lowers `blocking_vb 294 -> 293`,
  `loop_reads 40 -> 39`, and `loop_read_vb 335 -> 332` without moving the
  PS-EXE sector bucket or foreground pack LBA.
- **Website/docs were merged from current `main`.** The release includes the
  newer site navigation, glossary, lab, feed, structured-data, and page-TOC
  polish from upstream `main`, regenerated with the current perf numbers.

## Fix

`foreground_pilot.c` adds a VISITOR3 high-tide grouped read entry:

```c
{138, 162, 4}
```

The final field is the minimum slack guard. Earlier raw or adjacent ranges
showed that saved reads can still steal visible cadence; this promotion keeps
the read-group path scheduler-owned instead of treating every apparent
read-plan cluster as safe.

## Verification

- `./scripts/build-ps1.sh` passed with `build-ps1/jcreborn.exe` still in the
  `215040` byte sector bucket.
- Focused VISITOR3 high/low probe passed for the high-tide improvement and
  low-tide exact-flat canary:
  `scratch/ps1-perf-iterate/visitor3-high-group138-162-slack4-v081-probe/20260507-085014-2493254/summary.json`.
- Broad canary gate passed across FISHING1 high, FISHING3 high/low, VISITOR3
  high/low, BUILDING2 high/low, BUILDING4 high/low, BUILDING6 high/low, and
  ACTIVITY9 high/low:
  `scratch/ps1-perf-iterate/visitor3-high-group138-162-slack4-v081-canaries/20260507-085249-2508519/summary.json`.
- A visible DuckStation `fgpilot` screensaver-loop run from the committed build
  was manually checked after the VISITOR3 promotion and looked good.
- `./scripts/site-build-static-root.sh` rebuilt the website, and
  `python3 scripts/site-redteam.py docs --baseurl /johnny-castaway-ps1 --require-relative --exclude 'ps1/*' --exclude 'archive/*' --exclude 'general/*' --exclude 'readme/*'`
  passed after fixing an upstream 404-demo link that the stricter red-team
  check caught.

The release remains a performance/stability point release. No scene validation
scope changed: 63 / 63 scenes remain signed off.
