---
title: Glossary
eyebrow: Reference · vocabulary
subtitle: The specific technical terms the rest of the docs use without scaffolding. Written in the project's own voice.
description: Glossary for the Johnny Castaway PS1 port — pipeline, graphics, audio, hardware, build, and voice terms used across the docs and devlog.
---

This page is a vocabulary anchor for the rest of `/docs/`. The project's prose is dense by choice — *FntFlush*, *dirty-rect bookkeeping*, *SPI tx_len*, *the FISHING 1 bar* show up without scaffolding because the [voice]({{ '/about/voice/' | relative_url }}) is "write the way an expert writes." This page lets a new reader land at the start of a paragraph rather than google a phrase.

Grouped by area, not alphabetical — most readers come in via one section of the codebase, not by spelling. Cross-references inside definitions stay terse on purpose.

<nav class="scenes-jump" aria-label="Jump to glossary section">
  <span class="scenes-jump-label">Jump to:</span>
  <a href="#pipeline">Pipeline</a> · <a href="#graphics">Graphics</a> · <a href="#audio">Audio</a> · <a href="#hardware">Hardware</a> · <a href="#build">Build</a> · <a href="#performance">Performance</a> · <a href="#voice">Voice</a>
</nav>

## Pipeline {#pipeline}

<dl>
<dt id="ads">ADS</dt>
<dd>Sierra's <em>Animation Description Script</em> file — the higher-level sequencer that picks scenes, drives Johnny's mood and posture, and routes between scene-tag indices. The original game's `ACTIVITY.ADS`, `BUILDING.ADS`, etc. are still the source of truth for which 63 scenes ship; the PS1 runtime never interprets them at runtime.</dd>

<dt id="ttm">TTM</dt>
<dd>Sierra's <em>Tagged Text Machine</em> bytecode — the lower-level animation primitives an `ADS` scene tag dispatches into. Each TTM script is a sequence of opcodes (load resource, draw rect, blit, sleep N ticks, play sample, etc.). The host build runs the real TTM interpreter to capture every visible draw; the PS1 build replays the captured stream.</dd>

<dt id="fg2-pack">FG2 pack</dt>
<dd>The per-scene foreground binary the PS1 loads off the disc. One <code>fg2.high</code> and one <code>fg2.low</code> per scene (high-tide and low-tide variants). Carries its own palette, frame-timing table, base-frame full-render, and per-frame diff spans. The whole runtime trick of this port is that the PS1 replays packs instead of running TTM.</dd>

<dt id="host-build">Host build</dt>
<dd>The desktop-side Linux build, forked from <a href="https://github.com/jno6809/jc_reborn">jno6809/jc_reborn</a>. Runs the original Sierra bytecode against the original Sierra data files (<code>RESOURCE.MAP</code>, <code>RESOURCE.001</code>) under a <em>capture mode</em> that records every draw and every <code>PLAY_SAMPLE</code> into <a href="#fg2-pack">FG2 packs</a>. Required to develop new scenes; not needed to play the released disc.</dd>

<dt id="capture">Capture (mode)</dt>
<dd>Host-build environment-variable knob that turns on FG2 export for a single scene's <a href="#story">story-flag</a> permutation. Two captures per scene: high tide + low tide. The capture must produce a deterministic replay, which is why the host respects an explicit <a href="#seed">RNG seed</a> in capture mode.</dd>

<dt id="replay">Replay</dt>
<dd>What the PS1 build does. Loads an FG2 pack from the disc, plays back every frame's diffs against its own background and overlay layers, fires sound events at the recorded ticks. The PS1 never interprets a Sierra opcode at runtime.</dd>

<dt id="fishing1-bar">The FISHING 1 bar</dt>
<dd>The project's internal acceptance bar for "scene validated." Pixel-perfect visuals against the host-captured reference, plus SFX cues that land on the same engine ticks, signed off by human visual + audible review across every variant flag that applies to the scene (night palette, low-tide shoreline, holiday overlay, raft-stage progression). Named because <code>FISHING 1</code> was the first scene to clear it.</dd>

<dt id="regtest">Regtest harness</dt>
<dd>Headless DuckStation with a <code>scripted-input</code> file (controller frame log) that boots a build, walks a known route, captures screenshots at marker frames, and pixel-diffs against stored reference shots. Source of truth for "does this still build and boot." The release bar (visual signoff) is a separate, stricter pass on top.</dd>

<dt id="story">Story flags</dt>
<dd>The handful of mood / posture / tide bytes the original engine carries between scenes. A scene's exact behavior depends on them — a fishing scene plays differently if Johnny had just woken up vs. just eaten dinner. The host build replays scenes in order to establish state before capturing; the PS1 build doesn't carry runtime state because its packs already encode the right variant.</dd>

<dt id="seed">Seed</dt>
<dd>RNG seed used by the host to make a capture deterministic, so two runs of the same scene produce byte-identical packs. Override surface in the pause menu (Pause → System → Set RNG Seed) lets the runtime force the same path the host captured under, which is occasionally useful in regtest debugging.</dd>
</dl>

## Graphics {#graphics}

<dl>
<dt id="clut">CLUT-indexed sprite</dt>
<dd>The PS1 GPU's native texture format: 4-bit (16-color) or 8-bit (256-color) indices into a Color Lookup Table. The replay engine ships sprites in 4-bit indexed form (<code>indexedPixels</code>) with a per-pack palette LUT. The 4-pixel-unrolled inner blit loop and opaque-sprite fast path live in <code>graphics_ps1.c</code>.</dd>

<dt id="dirty-rect">Dirty-rect bookkeeping</dt>
<dd>Tracking which screen rectangles changed this frame so only those get redrawn from the background, instead of redrawing the whole framebuffer. The PS1 replay engine's dirty-rect path reduces per-frame data movement by 80–95%. Without it, the SPU and the GPU compete for memory bandwidth and audio glitches.</dd>

<dt id="fntflush">FntFlush</dt>
<dd>PSn00bSDK's BIOS-text print-and-flush routine. Empirically broken in this scene-runtime context — does not honor draw ordering reliably and corrupts captioned frames. The project's caption renderer therefore stamps glyphs through the regular sprite path, never through <code>FntFlush</code>. "Do not regress to FntFlush" is a load-bearing rule.</dd>

<dt id="ot">Ordering table (OT)</dt>
<dd>The PS1 GPU's display list. Primitives are queued into the OT in z-order; the GPU processes the table front-to-back per frame. The replay engine assembles its OT inline as it iterates the FG2 pack's frame diffs.</dd>

<dt id="vram">VRAM</dt>
<dd>1 MB of dedicated GPU memory on the PS1, holding both the framebuffer and all sprite textures + CLUTs. Doubly tight: 24-bit framebuffer at 320×240 alone is ~225 KB, and every loaded sprite atlas + palette eats from the rest. Why packs ship pre-mangled sprites and why the runtime preloads palettes once at boot.</dd>

<dt id="multi-view-stitch">Multi-view stitch</dt>
<dd>For wide scenes that exceed one screen of action (e.g., LILLIPUTIAN arrival sweeping in from far-left), the host capture takes three sightlines — normal / far-left / far-right — and stitches their foreground deltas into one merged pack. <code>MISCGAG 1</code>, <code>MISCGAG 2</code>, <code>STAND 1</code> all needed this before they replayed cleanly.</dd>

<dt id="residual">Residual cleanup contract</dt>
<dd>The rule that the foreground replay must restore every dirty-rect span back to the background bitmap before next frame, with no left-over pixels. When a scene's pack misses a few pixels, the project ships a small <code>patch-&lt;scene&gt;-foreground.py</code> that keys foreground-only contamination against the full-host composite.</dd>
</dl>

## Audio {#audio}

<dl>
<dt id="vag">VAG</dt>
<dd>Sony's compressed audio format on the PS1, used by the SPU. Roughly 4-bit ADPCM with predictor coefficients per 16-sample block. All 23 SFX preload into SPU RAM at boot from <code>.VAG</code> files on the disc.</dd>

<dt id="adpcm">ADPCM</dt>
<dd><em>Adaptive Differential Pulse Code Modulation</em>. The compression scheme inside VAG: each sample is stored as a 4-bit delta predicted from the previous samples. The encoder lives in <code>scripts/wav2vag.py</code> and was extensively debugged during the <code>v0.3.6-ps1</code> milestone (commit <code>355227fa</code>).</dd>

<dt id="spu">SPU</dt>
<dd>The PS1's <em>Sound Processing Unit</em>: 24 hardware voices, 512 KB of dedicated SPU RAM, hardware sample-rate conversion and looping. Per-voice volumes, pitch, ADSR envelopes, and reverb send all controlled by writing memory-mapped registers.</dd>

<dt id="play-sample">PLAY_SAMPLE</dt>
<dd>The TTM opcode (<code>0xC051</code>) that triggers a sound effect. Captured into the FG2 pack as a per-frame sound-event table; <code>foreground_pilot.c</code> fires them with a 3-frame key-on delay so SPU register writes land before the PS1 expects audio.</dd>

<dt id="voice-slot">Voice slot</dt>
<dd>One of the SPU's 24 hardware-managed voices. The runtime uses 8 voices in round-robin for scene SFX, plus one dedicated voice for the optional ocean-ambience loop (toggle: Pause → Accessibility → Ocean).</dd>
</dl>

## Hardware & toolchain {#hardware}

<dl>
<dt id="psn00bsdk">PSn00bSDK</dt>
<dd>Lameguy64's open-source PSX SDK, version 0.24 here. Replaces Sony's proprietary Psy-Q with permissively-licensed equivalents. Build pipeline: <code>cmake</code> + <code>mips-mipsel-none-elf-gcc</code> via the SDK's CMake module.</dd>

<dt id="duckstation">DuckStation</dt>
<dd>Connor McLaughlin et al.'s PS1 emulator. Every commit is regtested against headless DuckStation; visual + audible signoff happens against the GUI build. The project's runtime works around several DuckStation HLE quirks (SPU master-volume isn't honored, the BIOS pad path is unusable, etc.).</dd>

<dt id="mkpsxiso">mkpsxiso</dt>
<dd>The CD-image packager. Reads <code>config/ps1/cd_layout.xml</code> + the compiled PS-EXE + the asset bundle and produces <code>jcreborn.bin</code> / <code>jcreborn.cue</code>.</dd>

<dt id="spi">SPI driver</dt>
<dd>The controller-poll driver in <code>src/spi.c</code>, derived from spicyjpeg's MPL-licensed pad-poll example. Direct SIO0 + Timer-2 IRQ at 250 Hz; the BIOS pad path (<code>InitPAD</code>/<code>StartPAD</code>) is unusable in PSn00bSDK 0.24 + DuckStation.</dd>

<dt id="tx-len">tx_len</dt>
<dd>The number of TX bytes the SPI poll sends before reading button bytes back. Must be 5, not 4 — DuckStation only delivers the actual button state when the full five-byte sequence comes from the TX buffer. The two-day lab essay on this is at <a href="{{ '/lab/two-day-spi-bug/' | relative_url }}">/lab/two-day-spi-bug/</a>.</dd>

<dt id="tonyhax">TonyHax</dt>
<dd>The softmod path used to boot homebrew on retail PS1 hardware. The project is smoke-tested on a SCPH-7501 via TonyHax. Treat any boot success as a small miracle.</dd>
</dl>

## Build & release {#build}

<dl>
<dt id="bin-cue">jcreborn.bin / jcreborn.cue</dt>
<dd>The shipped CD image, a <code>.bin</code> binary track plus a <code>.cue</code> cuesheet that DuckStation (and real hardware) opens. Both files belong in the same directory. End users never need original Sierra files — the pre-baked FG2 packs are derived data, not Sierra source.</dd>

<dt id="resource-files">RESOURCE.MAP, RESOURCE.001</dt>
<dd>The original Sierra data files from a legal <em>Johnny Castaway</em> 1.0 install. Required only by the host build during development. Not shipped with this project. See <a href="{{ '/legal/' | relative_url }}">/legal/</a> for the licensing line.</dd>

<dt id="release-bar">Release bar</dt>
<dd>Human visual + audible signoff against the host capture across every applicable variant. The release bar gates the version-tag commits, not the regtest harness — regtest only proves "still builds and boots."</dd>

<dt id="milestone">Milestone</dt>
<dd>Tagged release line cut every ~10 newly validated scenes (<code>v0.6.x</code>, <code>v0.7.x</code>). Smaller stability releases happen between milestones for runtime fixes that don't change the validated set.</dd>
</dl>

## Performance {#performance}

<dl>
<dt id="battle-card">Battle card (headless-perf)</dt>
<dd>The headless-DuckStation timing matrix. One row per scene/tide variant; the live aggregate is <em>average over-target %</em> and <em>average target-speed %</em> across timing-bearing rows. The live page is at <a href="{{ '/perf/' | relative_url }}">/perf/</a>, separate from the visual signoff at <a href="{{ '/scenes/' | relative_url }}">/scenes/</a> on purpose — different bars, different cadences, different failure modes. The current matrix mean is on the home-page status pill.</dd>

<dt id="target-vb">target_vb / loop_vb</dt>
<dd><code>target_vb</code> is the vblanks a scene <em>should</em> take at native rate (computed from the host capture's frame count). <code>loop_vb</code> is what the run actually took. Their ratio is the row's <em>target speed %</em>; their difference is <em>over target %</em>. Anything above zero on over-target means the row missed.</dd>

<dt id="over-target">over_target</dt>
<dd>The percentage by which <code>loop_vb</code> exceeded <code>target_vb</code>. The <em>Over Target</em> column on <a href="{{ '/perf/' | relative_url }}">/perf/</a>. Lower is better; <code>0%</code> means exact target cadence; negative means the run finished <em>under</em> the budget. The matrix-wide aggregate at <code>{{ site.release.tag }}</code> is <code>+0.9%</code>.</dd>

<dt id="blocking-vb">blocking_vb</dt>
<dd>The number of vblanks where the renderer was blocked waiting for the CD prefetcher to land the next pack chunk. The <em>Blocking</em> column on <a href="{{ '/perf/' | relative_url }}">/perf/</a>. <code>0</code> is ideal; non-zero values are usually traceable to a too-small stream-window or a wide-action scene whose pack chunk crossed a read group boundary.</dd>

<dt id="prefetch-hits">prefetch_hits</dt>
<dd>The <em>Prefetch</em> column on <a href="{{ '/perf/' | relative_url }}">/perf/</a> — vblanks where the CD prefetcher hit a buffer overrun and had to wait. Distinct from <code>blocking_vb</code>: prefetch overrun means data was ready early enough but the buffer was already full; blocking means data was late. Both move with stream-window size in opposite directions.</dd>

<dt id="clean-rect">Clean-rect (memory pressure)</dt>
<dd>The runtime's per-scene allocation for "the bytes needed to restore the static background under the foreground." Wide-action scenes need a wide clean-rect; the long-soak BUILDING4 regression resolved in <code>v0.8.0</code> was a clean-rect allocation failure under post-walk memory pressure, and <code>v0.8.1</code> extends the pressure estimator to include ocean wave-band expansion plus upper/lower split rects before allocation. The "clean-memory-relief drop-prefetch" experiment in the <a href="{{ '/lab/from-87-to-99-5/' | relative_url }}">retrospective</a> is the related performance unlock — letting the prefetch window drop to free clean-rect bytes when the scene needs them.</dd>

<dt id="fgp3">FGP3 pack format</dt>
<dd>Denser successor to the original FG2 pack format. Same per-frame foreground deltas, but with a smaller header and a residual cleanup table that replaces the runtime's "did I miss a pixel" rebuild. Most scenes' high-tide and low-tide packs are FGP3 as of <code>v0.8.0-ps1</code>; the win is per-frame upload bytes, the biggest bottleneck after raw playback on a 2× CD.</dd>

<dt id="prefetch-window">Prefetch window</dt>
<dd>The streaming budget the runtime uses to read pack data ahead of the playback frame. Originally global (one number across all scenes); the scene-local relief pass made it a per-scene setting backed by measurement. Smaller per-scene buffers, fewer evictions when the next scene is loading.</dd>

<dt id="stream-window">Stream-window (stage-1)</dt>
<dd>The CD stream's first-stage read buffer. Default was 32 KB; the per-scene retune (some up, some down) reduced blocking vblanks across the matrix by about a third. The number that mattered wasn't the size — it was that the size had been one number before, and was a per-scene measurement after.</dd>

<dt id="read-group">Read group (CD layout)</dt>
<dd>How <code>cd_layout.xml</code> bundles related scene/tide assets so the streaming code can prefetch them as a unit. Naive bundling (every scene's high+low together) backfires — the screensaver loop picks tides independently, so bundling slowed the first-tide read. <em>Scoped</em> read groups (e.g., ACTIVITY 9's low-tide variant on its own) are the correct shape.</dd>

<dt id="padded-residual">Padded residual packs</dt>
<dd>FGP3 residual cleanup tables padded to cover a sprite footprint that exceeds the legacy 640px scene clip. Used for ACTIVITY 9's wide-boat scene; replaces the per-scene <code>patch-&lt;scene&gt;-foreground.py</code> postprocess that earlier filled the missing pixels at runtime.</dd>

<dt id="canary">Canary scene</dt>
<dd>FISHING 1's high-tide run. Gets re-measured on every release. Its <code>loop_vb</code> vs <code>target_vb</code> is the load-bearing reference frame for "did this matrix-wide change just regress the easiest path." If the canary moves the wrong way, the change doesn't promote.</dd>

<dt id="experiment-log">Experiment log</dt>
<dd>The decision-record table at <code>docs/ps1/performance-experiment-log.md</code>. Every accepted optimization gets a row; every <em>rejected</em> one too — <code>-O2</code>, naive read-group probes, anything that ran for two days and produced the same number. The rejected rows pay for themselves the first time someone is about to re-try a no-op.</dd>

<dt id="promotion-rule">Promotion rule</dt>
<dd>An optimization promotes when (a) the canary doesn't regress, (b) every visual signoff still holds, and (c) the matrix mean improves. All three must hold; mixing them is how regressions ship.</dd>
</dl>

## Voice {#voice}

<dl>
<dt id="drawcredits">drawCredits</dt>
<dd>The four-line in-game credits text in <code>src/pause_menu.c</code>. Hand-written, load-bearing, the irreducible voice anchor for everything else on this domain. <em>"A labor of love by Hunter Davis. Hunter does not own or have a license to the Johnny Castaway character. The original creator generously allows fan ports. If you paid for this, you were cheated. Open source and free."</em> Any prose on the site has to sit comfortably next to it. Long version: <a href="{{ '/about/voice/' | relative_url }}">/about/voice/</a>.</dd>

<dt id="63-scene-grind">The 63-scene grind</dt>
<dd>The project's name for the work between "first scene validated" and "every scene validated." Same loop, repeated 63 times: capture, pack, route, replay, sign off. Retrospective at <a href="{{ '/lab/the-63-scene-grind/' | relative_url }}">/lab/the-63-scene-grind/</a>.</dd>

<dt id="dead-end">Dead end (named out loud)</dt>
<dd>A path that did not work, kept in the public record because dead ends are content. The <a href="{{ '/devlog/' | relative_url }}">devlog</a> is verbatim — superseded plans, partial successes, week-long timing bugs preserved unedited. The blog convention is to name them with a short lampshade ("Bummer." "Lightbulb moment!") rather than rewrite history into a clean narrative.</dd>
</dl>
