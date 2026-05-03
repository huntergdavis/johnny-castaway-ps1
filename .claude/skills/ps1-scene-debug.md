# PS1 Scene Debug Workflow

Use this when a PS1 scene-validation pass shows residue, clipped sprites,
wrong island placement, high/low tide mismatch, missing foreground pixels,
bad SFX timing, or incorrect frame holds.

## Core Rule

Do not guess which layer owns the bug. Prove whether the defect is in host
capture, pack encoding, runtime restore/cleanup, variant routing, island
placement, or timing before editing code.

## Triage Buckets

| Bucket | Evidence | Fix Direction |
|---|---|---|
| Island position mismatch | Host pixels are correct at one island X/Y but PS1 uses another, or the host pixels are clipped by the capture viewport. | Pin capture and runtime to the same `island-pos`; recapture if the source pixels are clipped. |
| Host capture contamination | The unwanted residue is already visible in host frames or pack-frame review before PS1 runs. | Fix capture/export policy with foreground-only overlay, keyed overlay, recapture, or frame selection. |
| Pack crop/bounds too tight | Host frame is correct but PS1 clips pole, splash, bubbles, shark, or other pixels in a straight line. | Expand pack/crop/bbox metadata or overlay rect; regenerate high/low packs. |
| Runtime cleanup/restore bug | Host pack frame is correct, but PS1 leaves old pixels after frame advance or scene end. | Fix clean rect, restore span, active region, terminal cleanup, or scene-specific backdrop policy. |
| Timing/hold bug | Correct frames exist but appear too briefly, too late, or hold on the wrong frame. | Adjust pack-time holds. Metadata delay belongs to the current frame; preserve total duration unless deliberately correcting scene timing. |
| Variant routing bug | High-tide capture plays on low-tide background, wrong holiday/raft/night state, or wrong pack basename. | Fix route, CD layout, boot tokens, and regenerate both variants if needed. |
| Audio timing/settings bug | Visuals are correct but SFX starts too early/late or ignores saved mute. | Compare host `sound-events.jsonl` to the FG2 sound table; verify memcard settings apply before audio init. |

## Repeatable Loop

1. Read `docs/ps1/scene-status.md` and recent commits for the scene.
2. Identify pack names and routing with `rg` across `src/foreground_pilot.c`,
   `config/ps1/cd_layout.xml`, and `scripts/export-scene-foreground-pilot.sh`.
3. Reproduce with explicit boot tokens: `lowtide`, `night`, `holiday`,
   `raft-stage`, `island-pos`, `seed`, and `loop`/`noloop`.
4. Inspect the user screenshot or run output and name the exact bad region.
5. Review host frames and pack JSON before runtime edits. If no HTML/contact
   sheet exists, create a temporary review surface from sampled frames.
6. If the bad pixels are already in host capture, fix capture/export first.
7. If host capture is clean, inspect pack bboxes, terminal cleanup, active
   region, high/low routing, and runtime restore/compose paths.
8. Make the narrowest fix, regenerate both high/low packs when pack inputs
   change, rebuild, and ask for user visual/audible signoff.
9. Only after signoff, update scene docs, website data, and release notes.

## Patterns Proven By JOHNNY2

- Test island placement before assuming a renderer bug.
- Host-clipped pixels cannot be recovered by shifting runtime placement.
- Review host/pack frames directly; JOHNNY2 lower-left bottle and feet residue
  was capture contamination, not a PS1 memory leak.
- Use keyed overlay only where needed. JOHNNY2 kept full base-diff capture for
  thought bubbles above the lower band and used foreground-only keyed overlay
  in the moving lower band.
- Fix visual pacing at pack time with hold adjustments when the right frames
  exist but are held on the wrong source rows.

## FISHING5 Shark Checklist

1. Confirm the current blocker row: visible leftover shark sprites/frames.
2. Run high tide explicitly:
   `./scripts/rebuild-and-let-run.sh noclean fgpilot fishing5 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 seed 1`
3. Repeat low tide with `lowtide 1`.
4. Review host frames around the shark/splash interval. If residue is already
   in host frames, fix capture/export policy first.
5. If host frames are clean, inspect FGP3 bboxes and terminal cleanup; shark
   residue is likely moving-water cleanup, not general memory leakage.
6. Test island X/Y if the shark interaction depends on screen-edge placement.
7. After a fix, run the shark interval twice. Bugs that appear only on the
   second pass are cleanup/state issues, not initial capture issues.
