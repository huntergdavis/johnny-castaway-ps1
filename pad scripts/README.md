# Pad Scripts

Reusable `PADSCRIPT.TXT` fixtures for PS1 headless integration smokes.

To run one manually:

```sh
cp "pad scripts/captions-enable-next-scene.txt" config/ps1/PADSCRIPT.TXT
./scripts/regtest-scene.sh --scene "FISHING 1" \
  --boot "fgpilot fishing1 lowtide 1 loading-waves perf-detail seed 1 pad-script" \
  --frames 1800 --interval 60 --output scratch/caption-pad-smoke
```

Restore the normal empty `config/ps1/PADSCRIPT.TXT` before final builds.
