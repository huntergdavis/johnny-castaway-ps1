---
layout: page
title: Menu help guide
eyebrow: Help
subtitle: Every top-level pause-menu screen, captured by the headless scripted-input harness.
description: "Johnny Castaway PS1 menu help guide with headless regtest screenshots and control descriptions."
---

A labor of love by Hunter Davis. This page is generated from the same PS1 build that players run: the headless DuckStation harness boots the disc, waits for the game to settle, presses controller buttons, and captures each menu screen from the emulator framebuffer.

The point is not just documentation. It is a test. If Start stops opening the menu, if Circle stops backing out, if a sub-screen runs off the panel, or if a future refactor breaks controller input, this page stops regenerating cleanly.

## The route

The default capture script starts in normal screensaver playback, waits 30 seconds, opens the pause menu, then walks each major screen with D-pad, Cross, and Circle. It intentionally captures catalog entry pages once, not every gag or visitor row.

<div class="menu-guide">
<section class="menu-shot" id="pause-main">
<h2>Pause Menu</h2>
<p>The top-level dispatch screen: resume, open Scene Set Options or Scene Explorer (both new in v0.8.4-ps1), enter or exit Freeplay, open Freeplay Options, World Options, Accessibility, or System.</p>
<p class="menu-route"><strong>Capture route:</strong> Start</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/pause-main.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/pause-main.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Pause Menu screen. The top-level dispatch screen: resume, open Scene Set Options or Scene Explorer (both new in v0.8.4-ps1), enter or exit Freeplay, open Freeplay Options, World Options, Accessibility, or System." fetchpriority="high" />
  </picture>
  <figcaption>Marker frame 3036, captured frame 3040, delta 4.</figcaption>
</figure>
<p>This screen is intentionally short. Anything that grows past a few rows belongs on a sub-screen.</p>
</section>

<section class="menu-shot" id="scene-set">
<h2>Scene Set Options</h2>
<p>Scene-pool selector and picker policy in one place: pick a Scene Set (All Scenes, Fishing Only, Johnny Stories, Mary Visits, Visitors, Activities, or Misc & Suzy) and choose Random / Sequential / Original Sierra dispatch.</p>
<p class="menu-route"><strong>Capture route:</strong> Start, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/scene-set.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/scene-set.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Scene Set Options screen. Scene-pool selector and picker policy in one place: pick a Scene Set (All Scenes, Fishing Only, Johnny Stories, Mary Visits, Visitors, Activities, or Misc & Suzy) and choose Random / Sequential / Original Sierra dispatch." loading="lazy" />
  </picture>
  <figcaption>Marker frame 3564, captured frame 3565, delta 1.</figcaption>
</figure>
<p>Scene Set commits on Cross or Start so unsubmitted previews never linger.</p>
</section>

<section class="menu-shot" id="scene-explorer">
<h2>Scene Explorer</h2>
<p>The chapter-select grid. Each entry shows a captured-on-PS1 thumbnail, scene title, family, frame count, and pack name, with LEFT/RIGHT to step one scene and L1/R1 to step one family.</p>
<p class="menu-route"><strong>Capture route:</strong> Start, Down, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/scene-explorer.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/scene-explorer.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Scene Explorer screen. The chapter-select grid. Each entry shows a captured-on-PS1 thumbnail, scene title, family, frame count, and pack name, with LEFT/RIGHT to step one scene and L1/R1 to step one family." loading="lazy" />
  </picture>
  <figcaption>Marker frame 4142, captured frame 4145, delta 3.</figcaption>
</figure>
<p>Cross plays the highlighted scene once; Triangle loops it; Circle/Start backs out. New in v0.8.4-ps1.</p>
</section>

<section class="menu-shot" id="freeplay-options">
<h2>Freeplay Options</h2>
<p>The Freeplay debug entry page: gag catalog, visitor catalog, controls, and the clear-screen rebuild action.</p>
<p class="menu-route"><strong>Capture route:</strong> Start, Down four times, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/freeplay-options.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/freeplay-options.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Freeplay Options screen. The Freeplay debug entry page: gag catalog, visitor catalog, controls, and the clear-screen rebuild action." loading="lazy" />
  </picture>
  <figcaption>Marker frame 4740, captured frame 4740, delta 0.</figcaption>
</figure>
<p>Freeplay keeps the live joypad simple. Catalog-like actions live here, where they can be named and described.</p>
</section>

<section class="menu-shot" id="freeplay-gags">
<h2>Freeplay Gags</h2>
<p>Selector for direct Johnny actions. Each entry shows the source bitmap, frame count, rough RAM cost, and a one-line behavior note.</p>
<p class="menu-route"><strong>Capture route:</strong> Freeplay Options, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/freeplay-gags.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/freeplay-gags.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Freeplay Gags screen. Selector for direct Johnny actions. Each entry shows the source bitmap, frame count, rough RAM cost, and a one-line behavior note." loading="lazy" />
  </picture>
  <figcaption>Marker frame 5248, captured frame 5250, delta 2.</figcaption>
</figure>
<p>The screenshot shows the first entry only; the harness does not capture every gag row.</p>
</section>

<section class="menu-shot" id="freeplay-visitors">
<h2>Freeplay Visitors</h2>
<p>Selector for external events and visitors, with the same asset metadata as the gag catalog.</p>
<p class="menu-route"><strong>Capture route:</strong> Freeplay Options, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/freeplay-visitors.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/freeplay-visitors.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Freeplay Visitors screen. Selector for external events and visitors, with the same asset metadata as the gag catalog." loading="lazy" />
  </picture>
  <figcaption>Marker frame 5826, captured frame 5830, delta 4.</figcaption>
</figure>
<p>Missing optional assets are meant to fail soft: the menu names the asset, the runtime skips cleanly.</p>
</section>

<section class="menu-shot" id="controls">
<h2>Controls</h2>
<p>The on-disc reminder for Freeplay controls: walking, speed modifiers, fishing, clear screen, world toggles, and pause.</p>
<p class="menu-route"><strong>Capture route:</strong> Freeplay Options, Down, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/controls.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/controls.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Controls screen. The on-disc reminder for Freeplay controls: walking, speed modifiers, fishing, clear screen, world toggles, and pause." loading="lazy" />
  </picture>
  <figcaption>Marker frame 6404, captured frame 6405, delta 1.</figcaption>
</figure>
<p>Circle is Back everywhere in the menu. Cross is Select everywhere in the menu.</p>
</section>

<section class="menu-shot" id="world-options">
<h2>World Options</h2>
<p>Visual state controls: day or night, tide, raft stage, holiday selector, island-position editor, and Back.</p>
<p class="menu-route"><strong>Capture route:</strong> Start, Down five times, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/world-options.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/world-options.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the World Options screen. Visual state controls: day or night, tide, raft stage, holiday selector, island-position editor, and Back." loading="lazy" />
  </picture>
  <figcaption>Marker frame 7032, captured frame 7035, delta 3.</figcaption>
</figure>
<p>In Freeplay, these settings use the same rebuild path as the live R1 shortcuts so the island changes immediately.</p>
</section>

<section class="menu-shot" id="holidays">
<h2>Holidays</h2>
<p>Holiday mode and forced holiday selection: Auto Date, None, Original 4, or Expanded calendar.</p>
<p class="menu-route"><strong>Capture route:</strong> World Options, Down, Down, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/holidays.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/holidays.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Holidays screen. Holiday mode and forced holiday selection: Auto Date, None, Original 4, or Expanded calendar." loading="lazy" />
  </picture>
  <figcaption>Marker frame 7600, captured frame 7600, delta 0.</figcaption>
</figure>
<p>This is the manual side of the same date resolver used by the soft date picker.</p>
</section>

<section class="menu-shot" id="island-position">
<h2>Set Island Position</h2>
<p>Manual X/Y offset editor for the island anchor, plus an Auto/Manual mode toggle.</p>
<p class="menu-route"><strong>Capture route:</strong> World Options, Down, Down, Down, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/island-position.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/island-position.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Set Island Position screen. Manual X/Y offset editor for the island anchor, plus an Auto/Manual mode toggle." loading="lazy" />
  </picture>
  <figcaption>Marker frame 8178, captured frame 8180, delta 2.</figcaption>
</figure>
<p>This is mostly a development and placement tool, but it is kept in the player menu because it is useful on real hardware too.</p>
</section>

<section class="menu-shot" id="accessibility">
<h2>Accessibility</h2>
<p>Captions, sound, footsteps, Sound Test, and Back.</p>
<p class="menu-route"><strong>Capture route:</strong> Start, Down six times, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/accessibility.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/accessibility.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Accessibility screen. Captions, sound, footsteps, Sound Test, and Back." loading="lazy" />
  </picture>
  <figcaption>Marker frame 8806, captured frame 8810, delta 4.</figcaption>
</figure>
<p>Captions share the pause-menu font atlas so they can draw before the pause menu has ever been opened.</p>
</section>

<section class="menu-shot" id="sound-test">
<h2>Sound Test</h2>
<p>A selector for individual SPU sound effects: choose an effect, see whether it is present, and play it on demand.</p>
<p class="menu-route"><strong>Capture route:</strong> Accessibility, Down, Down, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/sound-test.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/sound-test.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Sound Test screen. A selector for individual SPU sound effects: choose an effect, see whether it is present, and play it on demand." loading="lazy" />
  </picture>
  <figcaption>Marker frame 9374, captured frame 9375, delta 1.</figcaption>
</figure>
<p>This turns audio debugging into a deterministic menu operation instead of a waiting game.</p>
</section>

<section class="menu-shot" id="system">
<h2>System</h2>
<p>Save settings, set time/date, set RNG seed, cycle perf logging, reset current scene, or advance to the next scene.</p>
<p class="menu-route"><strong>Capture route:</strong> Start, Down seven times, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/system.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/system.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the System screen. Save settings, set time/date, set RNG seed, cycle perf logging, reset current scene, or advance to the next scene." loading="lazy" />
  </picture>
  <figcaption>Marker frame 10002, captured frame 10005, delta 3.</figcaption>
</figure>
<p>System keeps less frequent operations away from the high-use visual and Freeplay screens.</p>
</section>

<section class="menu-shot" id="set-time-date">
<h2>Set Time And Date</h2>
<p>Software clock editor. The date drives holiday lookup and lets testers jump directly to seasonal overlays.</p>
<p class="menu-route"><strong>Capture route:</strong> System, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/set-time-date.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/set-time-date.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Set Time And Date screen. Software clock editor. The date drives holiday lookup and lets testers jump directly to seasonal overlays." loading="lazy" />
  </picture>
  <figcaption>Marker frame 10530, captured frame 10530, delta 0.</figcaption>
</figure>
<p>Confirming a date clears forced holiday selection so Auto Date can take over.</p>
</section>

<section class="menu-shot" id="set-rng-seed">
<h2>Set RNG Seed</h2>
<p>Deterministic random-seed editor for repeatable visual tests and bug reports.</p>
<p class="menu-route"><strong>Capture route:</strong> System, Down, Down, Cross</p>
<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/help/menu/set-rng-seed.webp' | relative_url }}" />
    <img src="{{ '/assets/img/help/menu/set-rng-seed.png' | relative_url }}" width="640" height="448" alt="Captured PS1 screenshot of the Set RNG Seed screen. Deterministic random-seed editor for repeatable visual tests and bug reports." loading="lazy" />
  </picture>
  <figcaption>Marker frame 11108, captured frame 11110, delta 2.</figcaption>
</figure>
<p>Shoulder buttons use larger steps so a tester can move quickly without a keyboard.</p>
</section>

</div>

## Regenerating

```bash
./scripts/ps1-menu-input-harness.sh
```

The runner temporarily writes `BOOTMODE.TXT` and `PADSCRIPT.TXT`, rebuilds the PS1 image, runs DuckStation regtest headlessly, copies the first captured frame at or after every delayed `JCPADSHOT` marker into `site/assets/img/help/menu/`, and rewrites this page.

Related references: [Pause menu]({{ '/docs/pause-menu/' | relative_url }}), [Freeplay and debug mode]({{ '/docs/freeplay/' | relative_url }}), and [Regression testing]({{ '/docs/regtest/' | relative_url }}).
