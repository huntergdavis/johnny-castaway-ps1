# Pad Scripts

Reusable `PADSCRIPT.TXT` fixtures for PS1 headless integration smokes.

To run one manually:

```sh
./scripts/regtest-scene.sh --scene "FISHING 1" \
  --boot "fgpilot fishing1 lowtide 1 loading-waves perf-detail seed 1" \
  --pad-script captions-enable-next-scene \
  --frames 1800 --interval 60 --output scratch/caption-pad-smoke
```

Add `--pad-script-log` when you need parsed input events in the PS1 log.
