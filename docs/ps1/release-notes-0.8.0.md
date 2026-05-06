# v0.8.0-ps1 Release Notes

**Date:** 2026-05-06
**Tag:** `v0.8.0-ps1`
**Theme:** complete-scene performance baseline

`v0.8.0-ps1` is the first release after complete scene validation that also
promotes the headless optimization methodology as a release baseline. All 63
original Johnny Castaway scenes remain signed off under the visual + audible
bar, and all 126 high/low scene variants are routed through the headless
performance matrix.

## Headline

- **63 / 63 scenes validated.** The scene ledger remains fully green after
  the post-validation bugfix pass.
- **126 / 126 variants routed.** The performance matrix covers every high
  and low tide scene route; 120 rows carry active-loop timing.
- **Performance is near target.** Timing-bearing rows now average **+0.9%**
  over target / **99.5%** target speed. Against the compact full-matrix
  baseline of about **+17.4%** over target / **87.1%** target speed, the
  headless optimization loop has removed about **16.5 percentage points** of
  over-target gap and added about **12.4 target-speed points**.
- **ACTIVITY 9 is now an optimized validated outlier.** The final validated
  scene uses the Activity9 wide-boat stitch plus padded FGP3 residual packs
  and a scoped low-tide read group.
- **Random-run clean-rect pressure is fixed.** A BUILDING4 soak regression
  exposed a clean-rect allocation failure under walk-clean memory pressure;
  the runtime now releases the stale walk clean buffer, retries the large
  scene clean snapshot, and recaptures the walk baseline after oversized
  scene rects are restored and freed.

## Performance Baseline

The current battle card is:

- `126 / 126` scene/tide variants routed.
- `120 / 126` timing-bearing variants.
- `63 / 63` scenes with both high/low variants measured.
- `0 / 126` blocked variants.
- Timing-bearing average: `+0.8692%` over target / `99.4529%` target speed.
- Latest matrix row: `2026-05-06T07:45:20`.
- FISHING1 canary: `1068 / 1074` VBlanks, `-0.6%` over target, `100.6%`
  target speed, `blocking_vb=2`.

The accepted optimization path since the compact full-matrix baseline includes
FGP3 conversions, scene-local prefetch relief, stream-window retuning, padded
residual packs for ACTIVITY9, and scoped read grouping. Rejected `-O2` and
read-group probes are preserved in the experiment logs so the next pass starts
from evidence instead of repeating no-op tests.

## Verification

Release candidate checks:

- `./scripts/build-ps1.sh` completed warning-free after the clean-rect retry
  fix.
- A standard random DuckStation run hit the previous pressure sequence
  (`fishing1`, `fishing6`, `fishing4`, `building4`) and recovered through the
  clean-rect retry path without `JCBSOD` or `JCWALK` allocation failures.
- The random run continued through later scenes including `visitor5`,
  `walkstuf3`, `stand15`, and `mary3`.
- User watched the rebuilt random-run candidate in DuckStation and confirmed
  normal playback before release.

The primary acceptance gate remains human visual and audible signoff for
scenes, with the headless performance matrix as the numeric speed baseline.
