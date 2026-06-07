# PS1 Scene Validation Agent Handoff

Last updated: 2026-05-04

This is a portable handoff for another coding agent continuing Johnny
Castaway PS1 scene validation. It distills the local
`jc-ps1-scene-debug` skill, the active validation loop, and the recurring
defects already seen during the scene-by-scene push.

## Current State

- Repo: `/home/hunter/workspace/jc_reborn`
- Active branch at handoff: `main`
- Primary render path: FG2 scene playback through `fgpilot`
- Current source-of-truth ledger: `docs/ps1/scene-status.md`
- Website scene data: `site/_data/scenes.yml`
- Per-scene website pages: `site/scenes/<slug>/index.md`
- Static website output: `docs/`
- Scene explorer generated data: `src/pause_menu/scene_explorer_data.h`
- Local skill source: `/home/hunter/.codex/skills/jc-ps1-scene-debug/SKILL.md`

Immediate handoff note:

- `VISITOR 7` has human visual + audible signoff and is being promoted with the scene ledger at 44/63.
- The regenerated `VISITOR7.FG2`, `VIST7LOW.FG2`, and the VISITOR7 hold-redistribution branch in `scripts/export-scene-foreground-pilot.sh` belong with the `VISITOR 7` promotion.
- `VISITOR 7` is a timing case: foreground-only host frames already contain the coconut/tree strike pixels, but dedupe left the impact rows too short. Source frames 32, 62, 71, and 80 now get redistributed hold time without changing total scene duration.
- Production island placement remains variable. The normal, far-left, and far-right capture/test positions are evidence, not runtime pins.
- Next scene after promoting `VISITOR 7` is `WALKSTUF 1`.

## Non-Negotiables

- Pixel-perfect visual playback owns every tradeoff.
- Do not skip frames, drop sprites, or accept missing pixels for speed.
- Do not mark a scene validated until the user visually/audibly signs off.
- Do not assume a capture position is a production pin. Capture/test positions are evidence unless the original scene proves it needs fixed runtime placement and the user signs off.
- Prefer pack/export fixes before runtime fixes when the bad pixels are already present in host capture or decoded pack frames.
- Use console/log output for debug where possible; do not add visual debug overlays unless requested.
- Stop an in-flight visual run only when the user asks to test a newer build/fix, asks to stop, or the run must be replaced for the current validation task.
- Use cue-filtered DuckStation termination, not global `pkill`.

## Standard Scene Validation Loop

1. Check repo state.

   ```bash
   git status --short --branch
   ```

2. Read the current scene row.

   ```bash
   rg -n "<slug>|<PACK>|<LOWPACK>" docs/ps1/scene-status.md site/_data/scenes.yml scripts/export-scene-foreground-pilot.sh src/foreground_pilot/foreground_pilot.c config/ps1/cd_layout.xml
   ```

3. Regenerate the host capture and high/low FG2 packs before first visual validation unless intentionally inspecting a historical pack.

   Generic multi-view default:

   ```bash
   ./scripts/export-scene-foreground-pilot.sh host-results/<slug>-foreground-pilot <slug> '<ADS TAG>' <PACK> 0 1.0 <LOWPACK>
   ```

   STAND/no-stitch fast path:

   ```bash
   ./scripts/export-scene-foreground-pilot.sh --no-stitch host-results/<slug>-foreground-pilot <slug> '<ADS TAG>' <PACK> 0 1.0 <LOWPACK>
   ```

   Force generic stitching if needed:

   ```bash
   ./scripts/export-scene-foreground-pilot.sh --stitch host-results/<slug>-foreground-pilot <slug> '<ADS TAG>' <PACK> 0 1.0 <LOWPACK>
   ```

4. Launch the PS1 scene with an explicit route.

   Typical normal high-tide/night validation route:

   ```bash
   RUN_TIMEOUT_SECONDS=0 PS1_INITIAL_CAPTURE_WAIT=10 PS1_CAPTURE_COUNT=1 \
     ./scripts/rebuild-and-let-run.sh noclean fgpilot <slug> \
     lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1
   ```

   If checking low tide:

   ```bash
   RUN_TIMEOUT_SECONDS=0 PS1_INITIAL_CAPTURE_WAIT=10 PS1_CAPTURE_COUNT=1 \
     ./scripts/rebuild-and-let-run.sh noclean fgpilot <slug> \
     lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1
   ```

5. Ask the user for visual signoff. The user will usually answer `good`, `validated`, or describe the defect.

6. If defective, classify before editing:

   - Host capture contamination
   - Host capture clipping / wrong island capture position
   - Pack crop or bbox too tight
   - Runtime cleanup/restore bug
   - Timing/hold bug
   - Variant routing bug
   - Audio timing/settings bug
   - Memory pressure / allocation failure

7. Review host frames and pack output before changing runtime code.

   Useful directories:

   ```text
   host-results/<slug>-foreground-pilot/host-capture-high/
   host-results/<slug>-foreground-pilot/host-capture-low/
   host-results/<slug>-foreground-pilot/host-capture-high-fgonly/
   host-results/<slug>-foreground-pilot/host-capture-low-fgonly/
   host-results/<slug>-foreground-pilot/foreground-pack.json
   host-results/<slug>-foreground-pilot/foreground-pack-lowtide.json
   ```

8. Make the narrowest fix, regenerate both high and low packs, rebuild, and relaunch for user signoff.

9. After signoff, update documentation and generated site artifacts.

   Required source updates:

   - `docs/ps1/scene-status.md`
   - `site/_data/scenes.yml`
   - `site/scenes/<slug>/index.md`
   - `README.md`
   - `docs/ps1/README.md`
   - `docs/ps1/current-status.md`
   - `site/_config.yml`
   - `docs/ps1/performance-scene-matrix.csv` notes if pack timing predates refreshed validation pack

   Generated updates:

   ```bash
   python3 scripts/build-scene-explorer-data.py
   ./scripts/site-build-static-root.sh
   ```

10. Commit the scene promotion and any pack/export fix together.

   Example:

   ```bash
   git add <changed files>
   git commit -m "docs: validate stand5 scene"
   ```

## Safe DuckStation Stop

Use this instead of broad process killing:

```bash
python3 - <<'PY'
import os, signal
cue = '/home/hunter/workspace/jc_reborn/jcreborn.cue'
for pid in os.listdir('/proc'):
    if not pid.isdigit():
        continue
    try:
        raw = open(f'/proc/{pid}/cmdline', 'rb').read()
    except OSError:
        continue
    parts = [p.decode('utf-8', 'ignore') for p in raw.split(b'\0') if p]
    if parts and 'duckstation' in os.path.basename(parts[0]).lower() and cue in parts:
        os.kill(int(pid), signal.SIGTERM)
        print(f'terminated duckstation pid {pid}')
PY
```

After stopping, poll the launch-wrapper session so it restores
`config/ps1/BOOTMODE.TXT` and `config/ps1/bootmode_embedded.h`.

## Exporter Rules

### Generic Multi-View Stitch

Default new-scene bring-up uses normal, far-left, and far-right
foreground-only host views merged by scene-local coordinates. This solves
the PS1 architecture issue where a scene can contain more than one screen
width of useful island-relative content.

Use this when:

- Action extends off-screen at one island position.
- Full host capture clips a fishing line, bottle, splash, mermaid, thought bubble, shark, or boot.
- The user says a far-left or far-right validation run is missing pixels.

Do not confuse this with production pinning. The merged pack should remain
scene-relative and follow normal random island placement.

### No-Stitch Fast Path

`--no-stitch` is valid for simple STAND-style scenes where action is local
to the island and does not need far-left/far-right merged views.

The no-stitch path still needs foreground-only overlay across the frame.
The first `STAND 5` no-stitch attempt proved that pure base-diff treats
frame-0 static Johnny pixels as background and can fade/drop legs.

### Scene-Specific Export Defaults

Scene-specific export policy belongs in
`scripts/export-scene-foreground-pilot.sh` after visual evidence and user
signoff. Examples already present:

- `JOHNNY 2`: lower-band keyed overlay plus thought-bubble timing fixes.
- `JOHNNY 4` / `JOHNNY 5`: full-frame keyed foreground-only overlay for bottle-message contamination.
- `FISHING 5`: full-frame current-ledger overlay for shark/water pixels.
- `FISHING 7` / `FISHING 8`: far-left foreground-only recapture; production remains variable-position.
- `MARY 2`: multi-view stitch plus full-host bubble-shell injection.
- `MARY 3`: far-right full-frame keyed foreground-only capture plus timing hold fixes.
- `MARY 5`: `NORAFT` capture policy and full-wipe launch behavior.
- `STAND*`: no-stitch fast path with foreground-only overlay.
- `VISITOR 3`: scene-specific red-ship/splash synthesis from multi-view foreground-only and live full-host crash frames.

## Bug Patterns Already Seen

### Host Capture Contamination

Symptoms:

- Moving object trails remain in pack review or PS1 playback.
- Bottle, shark, feet, mermaid, or water pixels overpaint themselves.

Examples:

- `JOHNNY 2`: lower-left bottle/feet residue was host-side.
- `JOHNNY 4` / `JOHNNY 5`: bottle-message scenes needed full-frame foreground-only overlay.
- `FISHING 5`: shark frames were stale or outline-only until current-ledger foreground data was included.
- `MARY 2`: lower-water and mermaid/boot regions required merged foreground-only views.
- `VISITOR 3`: full-host red ship pixels were useful only during live crash frames; copying later full-host rows replayed stale red/splash residue.

Fixes:

- Use keyed foreground-only overlay for only the contaminated region when possible.
- Use full-frame foreground-only overlay when the full host surface is broadly stale.
- Regenerate both high and low tide packs.

### Host Capture Clipping

Symptoms:

- Sprite is clipped at screen edge in PS1.
- Moving the runtime island reveals missing source pixels.
- Host review shows the full action only at a different island X.

Examples:

- `FISHING 7` / `FISHING 8`: far-left capture exposed all right-reaching fishing pixels.
- `JOHNNY 5`: x=80 host capture kept the thrown-bottle splash in frame.
- `MARY 2`: line/mermaid/boot/splash needed multiple island-position captures stitched together.
- `MARY 4` and later generic scenes: multi-view stitch became the default bring-up path.
- `VISITOR 3`: normal/far-left/far-right foreground-only views are still required even though the red ship hull needs scene-specific full-host synthesis.

Fixes:

- Recapture at controlled host positions.
- Prefer generic normal/far-left/far-right scene-local stitch.
- Validate runtime placement separately.
- Do not add production pins unless the source scene itself requires it.

### Pack Crop Or Bounds Too Tight

Symptoms:

- Pole tips, bubbles, splash, shark fins, or thought bubbles cut off in a straight line.
- Host frame is correct but PS1 playback crops the edge.

Examples:

- `FISHING 6`: splash and pole cleanup/crop needed expansion.
- `JOHNNY 2`: thought bubbles had boundary issues while fixing timing.
- `MARY 2`: bubble shell could exist in host full frames but not in foreground-only pack source.

Fixes:

- Expand overlay rects or pack crop/bbox logic.
- Inspect `foreground-pack.json` union and per-entry bboxes.
- Use full-host injection for effect shells if foreground-only omits them.

### Timing And Hold Bugs

Symptoms:

- Correct frame exists but flashes too fast.
- Playback holds on blank recovery frames instead of the meaningful frame.
- A bubble, note, or speech frame appears for one frame and then lingers on the wrong exit frame.

Examples:

- `JOHNNY 2`: island/SOS thought bubbles needed hold redistribution.
- `JOHNNY 5`: SOS note needed longer hold while blank rows were shortened.
- `MARY 3`: dinner/thought beat needed hold time moved onto readable frames.
- `VISITOR 3`: source frame 158 holds the clean splash; moving ticks from frame 159 keeps the splash readable without stale full-host rescue rows.

Fixes:

- Use `--hold-adjust source_frame:delta_ticks`.
- Use `--hold-advance-window start:end:rows` when a cluster is delayed.
- Keep total duration balanced unless intentionally correcting scene timing.

### Story-Flag Policy Bugs

Symptoms:

- Generic raft appears when scene owns its own raft.
- Direct scene loader walks Johnny before a full-screen wipe scene.
- Broad boot tokens override story restrictions.

Example:

- `MARY 5`: `NORAFT` suppresses generic raft, `FIRST` skips the walk prelude.

Fixes:

- Enforce story flags in runtime direct scene playback.
- Match capture policy to runtime policy.
- Use broad validation boot tokens to prove clamps work.

### Black Backdrop Scenes

Symptoms:

- Scene should be full black but ocean/island draws behind it.
- Large clean snapshot creates memory pressure.

Examples:

- `JOHNNY 1`
- `JOHNNY 6`

Fixes:

- Route through `fgSceneUsesBlackBackdrop()`.
- Skip ocean/island setup.
- Use black cleanup.
- Still validate high/low pack parity.

### Memory Pressure

Symptoms:

- `JCBSOD`
- Allocation failure in scene loader or clean snapshot.
- Scene works once but fails after loader/freeplay/menu path.

Examples:

- `MARY 3`: large pack originally used clean-snapshot relief during validation;
  current perf keeps prefetch under clean pressure with an 8-VBlank guard.
- Scene loader path previously held frog/menu resources too long before direct playback.

Fixes:

- Check DuckStation log for `JCBSOD`, `JCMEM`, `JCRECT`.
- Free optional walk/prefetch/cache buffers before high-pressure clean snapshot.
- Verify no steady-state per-frame allocations.

## STAND Scene Notes

STAND scenes are usually simple island-local idle loops. They probably do
not need full multi-view stitching unless visual evidence says otherwise.

Recommended STAND export:

```bash
./scripts/export-scene-foreground-pilot.sh --no-stitch host-results/stand6-foreground-pilot stand6 'STAND 6' STAND6 0 1.0 STND6LOW
```

Recommended STAND run:

```bash
RUN_TIMEOUT_SECONDS=0 PS1_INITIAL_CAPTURE_WAIT=10 PS1_CAPTURE_COUNT=1 \
  ./scripts/rebuild-and-let-run.sh noclean fgpilot stand6 \
  lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1
```

If a STAND scene shows incomplete Johnny pixels, do not remove
foreground-only overlay. That was the `STAND 5` regression.

## Validation Promotion Checklist

After the user says a scene is validated:

1. Stop DuckStation with the cue-filtered stop command.
2. Poll the launch-wrapper session so boot override files restore.
3. Update the scene row in `docs/ps1/scene-status.md`.
4. Update the matching entry in `site/_data/scenes.yml`.
5. Rewrite `site/scenes/<slug>/index.md` from pending to validated prose.
6. Update validation counts in `README.md`, `docs/ps1/README.md`, `docs/ps1/current-status.md`, and `site/_config.yml`.
7. If the refreshed validation pack invalidates old perf sizes/timing, add a note in `docs/ps1/performance-scene-matrix.csv`.
8. Run:

   ```bash
   python3 scripts/build-scene-explorer-data.py
   ./scripts/site-build-static-root.sh
   ```

9. Check:

   ```bash
   git status --short --branch
   git diff --stat
   ```

10. Commit with a scene-specific message.

## What To Tell The User

During validation:

- State which scene is running and the exact defect class when known.
- If a fix is host-side, say so plainly.
- If a runtime pin is not required, say production remains variable-position.
- If a scene has no SFX events, say visual signoff is enough but note that the pack has no captured SFX.
- If a run is being replaced, say why before killing DuckStation.

Do not call a scene complete because the harness ran. The user's visual
signoff is the acceptance gate.

## Current Next Steps

1. Regenerate `VISITOR 4` with the current exporter before visual validation.
2. Launch `VISITOR 4` for user signoff.
3. If `VISITOR 4` validates, promote it through the same ledger/site/readme/static-site checklist.
