# Final Under-99 Speed Handoff

Date: 2026-05-23

This is the handoff for finishing the last three yellow PS1 foreground
performance rows. The current job is not feature work. It is a constrained
pixel-perfect and timing-perfect optimization loop.

## Current Battle Card

- Branch at handoff: `perf/allocator-era-under-green-20260517`.
- Last committed optimization baseline: `5a4c8f3de0 ps1: retime w1 high first frame`.
- Last committed log-only docs baseline: `4ebfa4535e docs: log w1 high early visual miss`.
- Public rollup: `+0.2006%` over target / `99.8013%` target speed.
- Raw signed rollup: `-0.5163%` over target / `100.5317%` speed.
- Methodology total: about `17.20` public over-target points removed and `+12.70` public target-speed points gained since the compact full-matrix baseline.
- Bands: `123` green, `3` yellow, `0` orange, `0` red.

The remaining yellow rows are:

| Priority | Row | Speed | Timing | Gap | CD / prefetch |
| --- | --- | ---: | --- | ---: | --- |
| 1 | `walkstuf1 high` | `98.837%` | `1815/1462/1445` | `17` | blocking `35`, reads `36`, due `6`, refill `5` |
| 2 | `visitor3 low` | `98.399%` | `1386/1062/1045` | `17` | blocking `38`, reads `12`, due `6`, refill `0` |
| 3 | `visitor3 high` | `98.216%` | `1386/1065/1046` | `19` | blocking `34`, reads `8`, due `2`, refill `1` |

Start with `walkstuf1 high` because it is closest to green. Then work
`visitor3 low` and `visitor3 high`; visitor3 high/low can be tested together
when a shared pack/layout/runtime change is plausible.

## Non-Negotiable Rules

- Do not skip frames, drop art, shorten visible holds incorrectly, or fake the target counter to make a row green.
- Pixel-perfect scene playback and SFX timing beat speed. Any scene-specific visual or timing doubt returns to the debugging loop before promotion.
- A speed promotion must pass `--require-improvement` against the current baseline and preserve scene identity unless the change is explicitly approved as a layout-changing release.
- Same-speed work-volume/code-headroom improvements can be promoted only when key timing metrics stay flat and the work reduction is material. Label these as headroom, not VBlank speed wins.
- Every failed experiment that was seriously tested goes into `docs/ps1/performance-experiment-log.md`; do not leave future workers guessing whether a path was tried.
- After every accepted improvement, that proof becomes the new baseline for the next run.
- Work in a branch off `main`. Merge to `main` only after the branch is clean, built, gated, documented, and ready to publish.

## Debugging Loop

Use this loop when a candidate breaks pixels, scene identity, SFX, pack routing,
or timing semantics.

1. Start clean:

   ```bash
   git checkout main
   git pull --ff-only origin main
   git switch -c perf/<short-target>-<date>
   git status --short --branch
   ```

2. Read the current scene notes before changing anything:

   ```bash
   rg -n "walkstuf1|visitor3" docs/ps1/scene-status.md docs/ps1/current-status.md docs/ps1/performance-experiment-log.md
   git log --oneline -n 20 -- src/foreground_pilot.c generated/ps1/foreground scripts docs/ps1
   ```

3. Classify the failure before editing:

   | Bucket | Evidence | Fix lane |
   | --- | --- | --- |
   | Host capture contamination | Bad pixel exists in host/pack review before PS1. | Capture/export or pack-generation fix. |
   | Pack crop/bounds too tight | Host frame is correct, PS1 clips a straight edge. | Pack metadata/crop/regeneration fix. |
   | Runtime cleanup/restore bug | Host/pack frame is correct, PS1 leaves residue. | Clean rect, restore, compose, or backdrop fix. |
   | Timing/hold bug | Correct frame exists but appears too early, too late, or too briefly. | Hold metadata or runtime advance semantics, preserving total duration. |
   | Variant routing bug | Wrong tide/night/raft/holiday or wrong pack. | Route/CD layout/manifest fix. |
   | Audio bug | Visuals correct, SFX late/missing/wrong. | Sound table/event timing/memcard settings fix. |

4. Inspect source evidence before runtime fixes. For pack changes, inspect the
   generated pack JSON/contact sheets and the relevant entries. For runtime
   changes, capture the smallest deterministic PS1 boot that reproduces the
   issue.

5. Validate both tides when the scene has both packs. A high-only perf fix is
   not safe if the same runtime branch is shared with low tide.

6. Do not promote a visual fix based only on headless perf. Use visual or
   screenshot review when pixels changed, then run perf after correctness is
   established.

## Performance Loop

Refresh the current status:

```bash
python3 /home/hunter/.codex/skills/jc-ps1-battlecard/scripts/ps1_battlecard.py
git status --short --branch
```

Build and capture or reuse a baseline:

```bash
./scripts/build-ps1.sh
./scripts/ps1-perf-iterate.sh \
  --case-local-cd \
  --timeout 300 \
  --output scratch/ps1-perf-iterate/w1high-baseline-$(date +%Y%m%d-%H%M%S) \
  --case "walkstuf1-high::fgpilot walkstuf1 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed 1"
```

Run the tracked parallel W1-high source/scheduler tournament:

```bash
./scripts/ps1-w1high-parallel-swing.py --parallel 32
```

Useful variants:

```bash
./scripts/ps1-w1high-parallel-swing.py --list
./scripts/ps1-w1high-parallel-swing.py --parallel 48 --only prep2_hot35_61,direct183_s2_rg80_92
./scripts/ps1-w1high-parallel-swing.py --parallel 32 --candidates scratch/my-candidates.json --out scratch/w1high-custom-swing
```

The runner builds each candidate in a linked clone and writes
`results.json` plus per-candidate build/perf logs under the output directory.
If the default baseline file is not present locally, the runner captures a
fresh current W1-high baseline first.

Promotion gate for a candidate:

1. Apply the winning source or pack change to the real branch with `apply_patch`
   or the relevant pack-transform script.
2. Build:

   ```bash
   ./scripts/build-ps1.sh
   ```

3. Run the focused proof with `--baseline` and `--require-improvement`.
4. Run the current under-yellow canary:

   ```bash
   ./scripts/ps1-perf-iterate.sh \
     --case-local-cd \
     --timeout 300 \
     --baseline scratch/ps1-perf-iterate/w1high-frame0-consume-phase4-under-yellow-canary-20260523/20260523-065754-4032041/summary.json \
     --require-improvement \
     --output scratch/ps1-perf-iterate/<promotion-name>-under-yellow-canary-$(date +%Y%m%d) \
     --case "walkstuf1-high::fgpilot walkstuf1 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed 1" \
     --case "visitor3-high::fgpilot visitor3 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed 1" \
     --case "visitor3-low::fgpilot visitor3 lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed 1"
   ```

5. If the canary only improves one row and the other rows are exact-flat, update
   `docs/ps1/performance-scene-matrix.csv`, `docs/ps1/performance-experiment-log.md`,
   `README.md`, and the generated website/perf pages.
6. Rebuild the website:

   ```bash
   ./scripts/site-build-static-root.sh
   ```

7. Commit the promotion. Include the before/after scene/loop/target, overrun,
   blocking/refill, reads/due, pack LBA, PS-EXE bucket, and methodology total
   in the commit message or adjacent docs.

Failed candidate loop:

1. Restore tracked source/pack edits.
2. Rebuild if the failure touched build outputs:

   ```bash
   ./scripts/build-ps1.sh
   ```

3. Add a concise row to `docs/ps1/performance-experiment-log.md`.
4. Rebuild the site if the log changed.
5. Commit as `docs: log <scene> <path> miss`.

## Current W1-High Evidence

The latest accepted W1-high speed promotion presents and consumes frame 0 before
the measured loop, then uses a four-VBlank W1-high phase wait. It improved:

- `1815/1466/1445 -> 1815/1462/1445`
- overrun `21 -> 17`
- speed `98.5675% -> 98.8372%`
- blocking/refill stayed `35/5`
- reads/due stayed `36/6`
- pack LBA stayed `24891`
- PS-EXE bucket stayed `233472`

The latest detail probe after that promotion stayed exact-flat at
`1815/1462/1445`, blocking/refill `35/5`, reads/due `36/6`. Its read-planner
hints still point at tight scheduler/read clusters:

- `377..383` six-sector candidate: saves one read, high tight cluster.
- `80..92` twelve-sector candidate: saves two reads, visible-cadence risk.
- `365..381` sixteen-sector candidate: saves two reads, visible-cadence risk.
- `358..382` twenty-four-sector candidate: saves four reads, very high risk.
- `74..98` twenty-four-sector candidate: saves three reads, high risk.

The important lesson: raw read grouping is real but repeatedly phase-negative.
Treat these as inputs to generated deadline/cost scheduling, not as proof that
a static row is promotable.

## Already Closed W1-High Paths

Do not spend another day retrying these without a materially different design:

- Static read groups and owner rows around `159..175`, `365..383`, `377..383`, `74..98`, `80..92`, and late `395..456`.
- Extra setup-resident slices and widened setup windows on the allocator-era baseline.
- Global catch-up threshold changes as standalone promotions.
- Broad prepare/direct-stage window expansion without per-frame cost awareness.
- Local-LZ on active-loop W1-high frames.
- Advertised-size tail trimming that crosses sector/window boundaries.
- Pre-loop static/first-frame presentation that only moves accounting.
- Early visual-cost clipping/trimming clusters that lower work but tighten target/refill cadence.

## Next Big Swings

Use the tracked runner for source/scheduler probes, but the most likely final
wins need generated metadata or pack-shape changes that reduce cost before the
scheduler decides what to do.

1. W1-high per-frame cost/deadline table. Generate a small table for hot frame
   windows and let the scheduler prepare visual work before speculative CD only
   when the next frame is known expensive and the following read is not urgent.
2. W1-high generated read ownership. Instead of static read groups, generate a
   per-entry owner/deadline map from the read planner so `80..92`, `365..381`,
   and `377..383` fire only in slack windows with enough downstream target room.
3. W1-high no-decode/no-copy relocation. Move or alias small late payloads into
   already-resident setup/window ranges without adding decompression debt.
4. W1-high dirty-row no-op elimination. Compare current and previous visible
   rows and remove restore/upload rows that are provably redundant after the
   accepted `200..215` previous-visible cleanup pass.
5. W1-high narrow frame-cost split. Split only the frame group around max
   elapsed frame `47` and see whether the first max-overrun cluster can be made
   cheaper without changing target cadence.
6. Visitor3-low terminal data-shape redesign. Avoid D4 for frames `134` and
   `136`; try a cheaper row-reference or setup-dictionary representation that
   preserves the current `12/6` read/due cadence.
7. Visitor3-low generated deadline ownership for `46..58` and `32..56`.
   Static rows had blocking signals but overrun regressions; generated
   ownership needs to fire only when the target room exists.
8. Visitor3-high/low shared segment packing. Look for fixed-layout compaction
   that keeps LBA and PS-EXE bucket fixed while reducing high and low terminal
   active-loop CD pressure.
9. Visitor3-high tiny-frame direct staging. Re-check frame `132` style direct
   staging only with strict layout identity and high/low canary parity.
10. Cross-row code headroom. Promote code-size reductions when exact-flat;
    smaller hot functions have repeatedly changed phase enough to make later
    data-shape wins promotable.

If this list runs low, add another 20-30 ideas to this file before continuing.
Prefer ideas that change cost structure or scheduling information, not another
static read row with the same already-logged failure shape.

## Publishing Checklist

For a successful promotion:

```bash
git status --short --branch
./scripts/build-ps1.sh
python3 /home/hunter/.codex/skills/jc-ps1-battlecard/scripts/ps1_battlecard.py
./scripts/site-build-static-root.sh
git add README.md docs/ps1/performance-scene-matrix.csv docs/ps1/performance-experiment-log.md docs/ps1/final-under-99-speed-handoff.md docs site generated src scripts
git commit -m "ps1: <short promotion>"
git checkout main
git pull --ff-only origin main
git merge --ff-only <branch>
git push origin main
```

If the merge is not fast-forward, stop and rebase or merge deliberately. Do not
overwrite unrelated work.
