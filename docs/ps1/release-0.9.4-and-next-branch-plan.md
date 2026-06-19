# Release 0.9.4 (ps1) + next-branch plan

Branch `ps1-transition-zero-20260609` → merge to `main` → tag `v0.9.4-ps1`.
This branch's headline work: the scene-945 / memory-exhaustion campaign
(reactive clean-rect strand recovery, ocean SCR withhold-on-retry), wave
visual fixes, the freeplay→scene TRANSIENT-wipe BSOD fix, and a batch of
release-final UI features (production boot, surf-at-start, 3-row pause
scene info + frame counter + walking indicator, SELECT=next-scene,
picker-policy lookahead reset).

## Release steps (do now, in order)

1. **Pre-release cleanup** — strip this session's debug traces so the
   release binary is production-clean:
   - `JCTILE ensure` / `JCTILE create` (background_tiles.c.inc)
   - `JCDIAG loadframe-enter` (ps1ShowFreeplayLoadingFrame)
   - `JCDIAG prep-launch ENTER/AFTER` (ps1PrepareSceneExplorerLaunch) — keep the wipe logic
   - `JCFREE exit` / `post-teardown` markers (freeplay block) — keep the teardown call
   - KEEP: freeplay-exit TRANSIENT wipe (the BSOD fix), explorer-launch
     start-fresh wipe, and the one-shot pre-BSOD `TRANSIENT-FAIL tag`
     (a legitimate crash report, fires once before a halt).
2. **Build PS1** (`scripts/build-ps1.sh`) and confirm: rc=0, no `JCTILE`/`JCDIAG`/`JCFREE` strings in sources, BOOTMODE=`fgpilot`.
3. **Commit** the branch.
4. **Merge** `ps1-transition-zero-20260609` → `main`.
5. **Run `scripts/release.sh`** on main (auto-bumps 0.9.3 → 0.9.4): full
   rebuild, copy artifacts to `release/`, update VERSION + site release
   metadata, rebuild the portable website under `docs/`, commit + tag
   `v0.9.4-ps1`, push.
6. **Verify**: VERSION=0.9.4, tag pushed, `release/` artifacts present,
   website pages regenerated (site-build-static-root + site-redteam clean).

## Next branch (off `main`, AFTER the release) — the 4 deferred tasks

1. **Memcard persistence gap** — add the 6 user-settable options the
   red-team found MISSING from save/load: closed captions, tide, raft,
   island position, perf level, scene set. Bump `MC_VERSION` to 7; write
   in `memcardSaveSettings`, read+apply (proper setters + range
   validation) in `memcardLoadSettings`.
2. **Sequential picker order** — it walks pool order (kAllScenes:
   activity→building→fishing…) but the pause menu shows catalog index
   (gSceneExplorer order), so the number jumps (26→33). Align so
   Sequential advances in displayed/catalog order (1,2,3,4…).
3. **Ghost Johnny** — standing dark silhouette at the island's right edge
   (bird scene) persists: a Johnny pose drawn outside the scene's
   clean-rect/draw bounds so the per-frame restore never wipes it.
   Fix the bounds (or re-capture the scene).
4. **Orphaned menu screens** — Scene Info and Credits have draw fns +
   dispatch but no entry point (`pauseMenuSetState` is never called).
   Either wire a menu row to them or remove the dead screens + their
   dispatch/back-handling.
