# Zoomies PCSX-ReARMed browser core

Zoomies vendors a modified PCSX-ReARMed WebAssembly core because the upstream
interpreter currently drops PlayStation hardware writes made through KSEG
aliases. PSn00bSDK uses those aliases for the SPU, leaving the browser build's
SPU control and volume registers at zero even though XA decoding succeeds.

The core also fixes two upstream Level-A XA defects. Each 8-bit stereo sector
contains 1,008 frames, but upstream reports 2,016, which inserts an equally
long silent tail and breaks playback cadence. Its Level-A path also sends
8-bit samples through the 4-bit nibble decoder and reads the wrong stereo byte
lanes. The replacement decoder consumes all four 8-bit sound-unit lanes and
produces PCM that tracks vgmstream's independent decoder at greater than
0.999999 correlation on the Zoomies soundtrack.

## Corresponding source

- PCSX-ReARMed: `https://github.com/EmulatorJS/pcsx_rearmed.git`
- PCSX-ReARMed commit: `588e1338f85a5867e46245f861e3d5958e7a4592`
- PCSX changes: `pcsx-rearmed.patch`
- RetroArch: `https://github.com/EmulatorJS/RetroArch.git`
- RetroArch commit: `d907a125ee76e02a21d89003c140fd0b72886e90`
- WebAudio bridge changes: `retroarch.patch`
- Build recipe: `build.sh`
- Emscripten used for the checked-in standard build: `3.1.69`

The RetroArch patch exposes the core's `AudioContext` and routes it through a
master `GainNode`. This lets the pinned EmulatorJS 4.2.3 page unlock audio and
keeps its mute and volume controls functional. It also defaults the web build
to RetroArch's 16-tap sinc resampler instead of the low-quality four-tap CC
resampler used by the upstream Emscripten configuration. When the audio queue
falls below half full, the patched driver temporarily runs immediate emulator
iterations until the existing 64 ms queue is replenished. This prevents a
throttled browser animation callback from starving WebAudio without increasing
the steady-state audio latency.

Run `./build.sh` from this directory with Emscripten, Git, Make, and 7-Zip on
`PATH`. It rebuilds distinct WebGL2 and legacy packages. After replacing a core,
bump `data/cores/reports/pcsx_rearmed.json` so EmulatorJS invalidates its
IndexedDB copy.

PCSX-ReARMed is distributed under GPL-2.0. The core archives include its full
license text; the exact upstream revisions, patches, and build recipe above are
provided as the corresponding source for these modified binaries.
