---
title: Releases
eyebrow: Tagged versions · what shipped, when
subtitle: A short index of every milestone and stability release. Themes, headlines, links to the full notes and the disc image.
description: Every tagged release of the Johnny Castaway PS1 fan port — themes, dated headlines, and links to the full notes and downloadable disc image for each version.
redirect_from:
  - /changelog/
---

A labor of love by Hunter Davis. The project tags milestones every ~10 newly validated scenes and cuts smaller stability releases between them when the runtime needs a fix that doesn't change the validated set. Below is the short version of what each tag carried — the full notes live alongside the source for anyone who wants the root-cause-and-mitigation depth.

The current release line is **`{{ site.release.tag }}`** with
**{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}** scenes signed off under the
[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Latest

### `v0.9.7-ps1` - visitor3 pixel-perfect under deep-soak pressure
<time datetime="2026-06-25"><em>2026-06-25</em></time>

A visual-confidence and deep-soak release. It closes the last walk-slab crash
class and makes the most memory-demanding scene render correctly even under
extreme deep-soak fragmentation.

- **visitor3 full-reset.** When the ceiling scene (~400 KB clean rect) strands
  even after the withhold-rebuild, a frog-clock-masked full-cache-reset
  defragments the pool to pristine via a *conditional* rewind (not a blind
  wipe — no orphaned pointers) and the scene reloads on the normal
  pixel-perfect path. It doubles as a periodic whole-soak defragmentation.
- **Correct decline fallback.** If even a pristine pool can't fit the scene
  (a TRANSIENT+CACHE capacity ceiling), it now re-composites the island, raft,
  and holiday every frame instead of showing ocean only — crash-free *and*
  correct-looking.
- **Walk/raft no-halt + stand-family flash.** The inter-scene 48 KB walk load
  degrades to a teleport on a fragmented CACHE instead of BSOD'ing, and STAND
  scenes no longer flash the empty island on their first frame.
- **24h soak.** 12,512 scenes, all 126 variants, 26 visitor3 full-resets all
  recovered pixel-perfect — 0 declines, 0 BSOD, 0 CACHE-FAIL.

[Full notes]({{ '/source/docs/ps1/release-notes-0.9.7/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.9.7-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.9.6-ps1` - deep-soak stability
<time datetime="2026-06-22"><em>2026-06-22</em></time>

A deep-soak stability release. Where `v0.9.5-ps1` fixed the first wave of
CACHE-exhaustion BSODs, `v0.9.6-ps1` closes the remaining deep-soak failure
classes.

- **The whole best-effort CACHE-alloc class is now graceful.** Allocations
  that NULL-checked their result but used the *halting* allocator (dead-code
  guards) now use no-halt + recover-via-withhold-rebuild: the per-scene stream
  window, clean-rect floor slabs, alignment scratch, and the reserve-stable-
  shape buffers.
- **CD-ROM recovers instead of freezing.** An explicit drive error re-inits
  the controller after a few failures; a silent read stall self-heals after
  ~15 s instead of spinning forever.
- **Self-naming BSODs.** A CACHE-exhaustion halt now names the failing
  allocation, and the memory simulator models the per-scene frame buffer.
- **~15h / 7,800-scene zero-crash soak** with ~65 graceful recoveries.

[Full notes]({{ '/source/docs/ps1/release-notes-0.9.6/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.9.6-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.9.5-ps1` - stability, rename, and visual polish
<time datetime="2026-06-20"><em>2026-06-20</em></time>

A stability and polish release that also renamed the project artifacts to
`johnnycastawayps1`.

- **Deep-soak BSOD fixes.** visitor3, black-scene, and frame-buffer
  exhaustion strands now recover or decline gracefully.
- **Rename to `johnnycastawayps1`** (binary, ISO, and the DuckStation window
  title), plus memory-card format v7 and a sequential scene picker.
- **Visual fixes** — ghost Johnny, the bubble dot, and stand-family scenes —
  backed by a new 126-variant visual-audit harness and a frame-buffer-aware
  memory simulator.

[Full notes]({{ '/source/docs/ps1/release-notes-0.9.5/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.9.5-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.9.4-ps1` - scene-945 memory campaign and release-final UI
<time datetime="2026-06-18"><em>2026-06-18</em></time>

A memory-exhaustion and UI release.

- **scene-945 campaign fixes** for the multi-hour memory-exhaustion BSOD,
  plus a freeplay→scene BSOD fix and refreshed wave visuals.
- **Release-final UI** — production boot, surf-at-start, pause-menu scene info
  with a frame counter, and SELECT to advance to the next scene.

[Full notes]({{ '/source/docs/ps1/release-0.9.4-and-next-branch-plan/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.9.4-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.9.3-ps1` - SPU async staging and Original-order soak release
<time datetime="2026-06-09"><em>2026-06-09</em></time>

A stability and loading point release after `v0.9.2-ps1`. It promotes the
async-staging branch after the Original-order soak held clean, and makes
unused SPU RAM part of the runtime's release path.

- **SPU RAM is now a cold cache.** After audio reserves its samples, the
  runtime can park prefetched scene metadata, foreground windows, walk PSBs,
  and the walk clean buffer in unused SPU RAM, then DMA them back before CPU
  or GPU use.
- **Transition staging uses non-blocking reads.** The foreground pilot starts
  aligned async CD reads for next-scene metadata and payload windows, polls
  them during slack, and drains them before any later blocking read or buffer
  mutation.
- **Original-order soak stayed clean.** The long visible run reached roughly
  293 picks with recurring SPU walk loads and no `JCBSOD`, fatal,
  invalid-read, or `JCWALK` allocation-failure markers.

[Full notes]({{ '/source/docs/ps1/release-notes-0.9.3/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.9.3-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.9.2-ps1` - long-run CD and VISITOR3-low stability release
<time datetime="2026-05-31"><em>2026-05-31</em></time>

A stability point release after `v0.9.1-ps1`. It promotes the extended-soak
fixes for the two rare failures found only after randomized scene playback had
been running for many hours.

- **CD searches are quiesced.** Runtime file lookups now drain the prior
  read/pause path before `CdSearchFile()`, preventing async `CdlPause`
  completion from racing later directory-walk `CdlSetloc` commands.
- **VISITOR3-low gets surgical memory relief.** Low tide drops the clean-relief
  stream window that competed with the low setup tail and five split clean-rect
  strips. VISITOR3-high keeps its reduced high-tide relief window.
- **Soak verification stayed clean.** The targeted VISITOR3-low run confirmed
  `no-prefetch` with no `fg-stream-window`, and the follow-up randomized soak
  was intentionally stopped for release after about 47.5 hours with no
  `JCBSOD`, `JCSKIP`, `CACHE exhausted`, or DuckStation `Incorrect parameters`
  markers.

[Full notes]({{ '/source/docs/ps1/release-notes-0.9.2/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.9.2-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.9.1-ps1` - longevity testing improvements
<time datetime="2026-05-24"><em>2026-05-24</em></time>

A stability checkpoint after `v0.9.0-ps1`. It raises CACHE headroom by 32 KB,
wraps long BSOD reasons on screen, and records a 6-hour 126-scene headless
soak with 0 BSODs.

[GitHub release]({{ site.github_url }}/releases/tag/v0.9.1-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.9.0-ps1` - all routed rows green
<time datetime="2026-05-23"><em>2026-05-23</em></time>

A performance milestone release: all 126 routed high/low scene variants are
timing-bearing and at or above 99% target speed.

[GitHub release]({{ site.github_url }}/releases/tag/v0.9.0-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.14-ps1` — JOHNNY1 local-LZ green promotion
<time datetime="2026-05-15"><em>2026-05-15</em></time>

A performance point release after `v0.8.13-ps1`. It promotes the JOHNNY1
local-LZ full-frame payload swing and moves both JOHNNY1 tides into the green
band without changing pack footprint or the PS-EXE bucket.

- **JOHNNY1 high/low advance to v932.** Entries `1` and `50` in both fixed
  `448370` byte packs now use a scene-local local-LZ sentinel stream.
- **Both tides are green.** JOHNNY1 high/low move from `1973/1945` to
  `1948/1945`, overrun `28 -> 3`, blocking/refill `25 -> 5`, read time
  `58 -> 37`, and target speed `98.56% -> 99.85%`.
- **Battle card improves.** Public rollup is `+0.2492%` over target /
  `99.7548%` target speed; raw signed rollup is `-0.5179%` / `100.5371%`;
  bands are `119` green and `7` yellow.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.14/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.14-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.13-ps1` — under-99 payload-work checkpoint
<time datetime="2026-05-15"><em>2026-05-15</em></time>

A performance point release after `v0.8.12-ps1`. It banks same-speed payload
work on WALKSTUF1 low, BUILDING2 high, and BUILDING4 low while logging several
closed misses for the next optimization path.

- **WALKSTUF1 low advances to v910.** The no-shift lane reaches frame `106`
  and cuts active payload to `790208` bytes while holding `1477/1432`.
- **BUILDING2 high and BUILDING4 low bank payload reductions.** B2 high
  reaches `building2-high-frame173-inplace-v914`; B4 low reaches
  `building4-low-frame283-inplace-v913`.
- **Battle card stays stable.** Public rollup is `+0.2697%` over target /
  `99.7347%` target speed; bands are `117` green and `9` yellow.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.13/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.13-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.12-ps1` — WALKSTUF1 low frame77/frame130 payload trims
<time datetime="2026-05-14"><em>2026-05-14</em></time>

A performance point release after `v0.8.11-ps1`. It keeps the restored lazy
stream allocation baseline and promotes two more same-speed WALKSTUF1 low
no-shift payload reductions.

- **WALKSTUF1 low advances to v795.** Frames `77` and `130` now shrink
  in-place without moving pack offsets, pack LBA/sectors, or the PS-EXE bucket.
- **Timing stays exact-flat.** W1-low remains `1770/1478/1431`,
  blocking/refill `64/20`, read time `60/272`, and due `11`, while active
  payload drops `879801 -> 799694`.
- **Battle card remains stable.** Public rollup is `+0.2708%` over target /
  `99.7337%` target speed; raw signed rollup is `-0.4963%` / `100.5160%`;
  bands are `117` green and `9` yellow.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.12/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.12-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.11-ps1` — Lazy stream-buffer release-regression fix
<time datetime="2026-05-14"><em>2026-05-14</em></time>

A corrective point release after `v0.8.10-ps1`. It keeps the W1-low v791
payload baseline, but rolls back the post-release heap-fragmentation experiment
that pinned too much memory before large clean-rect scenes could allocate their
snapshots.

- **WALKSTUF1 low renders and measures correctly again.** The bad merge pinned
  a `256 KB` CD sector pool plus boot-time FG stream buffers and caused the
  post-release W1-low check to skip after `JCSKIP clean-rect-alloc-failed`.
- **Lazy stream allocation is restored.** CD sector staging is back to per-read
  allocation, FG frame/scratch buffers stay lazy at boot, and W1-low returns
  exact-flat to v791: `1770/1478/1431`, blocking/refill `64/20`, read time
  `60/272`, due `11`.
- **Battle card remains stable.** Public rollup is `+0.2708%` over target /
  `99.7337%` target speed; raw signed rollup is `-0.4963%` / `100.5160%`;
  bands are `117` green and `9` yellow.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.11/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.11-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.10-ps1` — WALKSTUF1 low no-shift payload follow-through
<time datetime="2026-05-14"><em>2026-05-14</em></time>

A performance point release after `v0.8.9-ps1`. It promotes the current
mainline no-shift payload baseline through
`walkstuf1-low-frame76-inplace-v791`, while keeping all deterministic boot,
Scene Explorer, Suzy backdrop, and prior performance wins.

- **WALKSTUF1 low has the newest in-place payload baseline.** Frames `51`,
  `49`, `47`, `61`, `62`, `58`, `45`, `37`, `35`, `43`, `41`, `57`, `33`, `67`, `68`, `69`, `32`, `133`, `5`, `141`, `70`, `30`, `6`, `71`, `72`, `142`, `73`, `131`, `74`, `19`, `28`, `138`, `145`, `75`, and `76` shrink without moving
  offsets; timing stays exact-flat at `1478/1431`, while active payload drops
  `879801 -> 801103`.
- **Battle card remains stable.** Public rollup is now `+0.2708%` over
  target / `99.7337%` target speed; raw signed rollup is `-0.4963%` /
  `100.5160%`; bands are `117` green and `9` yellow.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.10/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.10-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.9-ps1` — WALKSTUF1 low in-place payload reductions
<time datetime="2026-05-14"><em>2026-05-14</em></time>

A performance point release after `v0.8.8-ps1`. It moved VISITOR5 low into
green, kept the BUILDING2 low and VISITOR3 high wins, and opened the W1-low
in-place payload lane that v0.8.10 continues.

- **VISITOR5 low joins high in green.** The low-tide row uses the matching
  `30..46` retained-read shape, improving `1104/1092 -> 1102/1097`.
- **BUILDING2 low improves again.** The `218..229` slack-8 row plus v739
  draw-tail trim measure `1339/1317`, overrun `22`, blocking/read time
  `53`/`150`, reads `37`, and due misses `12`.
- **Battle card improves from v0.8.8.** Public rollup is `+0.2708%` over
  target / `99.7337%` target speed; raw signed rollup is `-0.4963%` /
  `100.5160%`; bands are `117` green and `9` yellow.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.9/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.9-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.8-ps1` — VISITOR5 high retained-read promotion
<time datetime="2026-05-13"><em>2026-05-13</em></time>

A performance point release after `v0.8.7-ps1`. It keeps deterministic
BOOTMODE scene selection, Suzy backdrop cleanup, and heapless Scene Explorer
preview loading, then promotes the verified VISITOR5 high-tide `30..46`
retained-read group.

- **VISITOR5 high moves into green.** Active loop/target improves
  `1107/1090 -> 1101/1096`, overrun falls `17 -> 5`, blocking/refill falls
  `16 -> 5`, loop reads fall `19 -> 18`, and due misses remain `0`.
- **Battle card improves.** Public rollup is now `+0.2867%` over target /
  `99.7183%` target speed; raw signed rollup is `-0.4805%` / `100.5006%`.
- **Band shape is cleaner.** `116` rows are green, `10` are yellow, `0`
  orange, and `0` red.
- **The next outliers are explicit.** WALKSTUF1 low, BUILDING2 high,
  WALKSTUF1 high, VISITOR3 high/low, and BUILDING2 low remain the main
  under-99 tactical queue.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.8/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.8-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.7-ps1` — Deterministic BOOTMODE scene selection + Scene Explorer preview stability
<time datetime="2026-05-11"><em>2026-05-11</em></time>

A stability and operability point release after `v0.8.6-ps1`. It keeps the
same 126-row headless performance matrix baseline, but fixes two release-risk
surfaces: direct scene booting is now provable from the runtime log, and Scene
Explorer thumbnails no longer need a large heap allocation while paused.

- **BOOTMODE scene selection is deterministic and auditable.** The PS1 runtime
  logs a single early [`JCBOOT`]({{ '/docs/glossary/#jcboot' | relative_url }}) line with the boot source, explicit scene, seed,
  loop mode, tide/night/holiday/raft state, island position, and normalized
  boot text before title/resources/sound side effects.
- **Headless perf now fails if it measures the wrong scene.** The iterator
  derives the expected scene from `fgpilot <slug>` boot strings and rejects
  runs that report a different `JCPERF2 scene` or fall through to `JCPICK`.
- **Suzy black-backdrop scenes clean up after themselves.** Scene-specific
  backdrop state now clears clean-bg black mode and frees clean-bg rect state
  on entry/cleanup, preventing the following scenes from losing island/tree
  background layers.
- **Scene Explorer previews stream instead of malloc.** Thumbnails load in
  16-row chunks through a static buffer, avoiding the fragile 153 KB paused-menu
  allocation that could leave the explorer text-only after long runs.
- **Battle card unchanged numerically.** Public rollup remains `+0.3156%` over
  target / `99.6902%` target speed; raw signed rollup remains `-0.4529%` /
  `100.4740%`.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.7/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.7-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.6-ps1` — WALKSTUF1 / VISITOR3 setup-segment compaction follow-through
<time datetime="2026-05-10"><em>2026-05-10</em></time>

A performance point release after `v0.8.5-ps1`. It keeps the
v0.8.5 full 126-row headless matrix as the baseline and adds the
WALKSTUF1 low gap6-prefix + slack-guard promotion, the WALKSTUF1 high
window-prefetch / slack4 guard on the gap-compressed pack, and the
VISITOR3 high/low setup-segment resident copies for frames `131` / `128`.

- **126 / 126 rows remain timing-bearing.** No row regressed; band shape
  (`111` green, `15` yellow, `0` orange, `0` red) is unchanged from v0.8.5.
- **Public rollup `+0.3157%` / `99.6902%` target speed.** Down from
  v0.8.5's `+0.3215%` / `99.6847%`.
- **Raw signed rollup `-0.4529%` / `100.4740%` target speed.** Down from
  v0.8.5's `-0.4470%` / `100.4685%`.
- **Stats-version anchors that moved.** `walkstuf1-low-prefix-gap6-slackguard-v305`,
  `walkstuf1-high-gap1-windowprefetch-slack4-v288`,
  `visitor3-high-f131-resident-alias121123-v299`,
  `visitor3-low-f128-resident-seg27-v302`.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.6/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.6-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.5-ps1` — Full 126-row headless performance matrix
<time datetime="2026-05-09"><em>2026-05-09</em></time>

A performance/methodology point release after `v0.8.4-ps1`. It keeps the
custom Scene Explorer thumbnails and scene-page reconciliation from the
chapter-select grind, then promotes the current headless matrix as the
public baseline.

- **All 126 high/low rows are timing-bearing.** The matrix now has active-loop
  timing for every routed scene/tide variant: 63 scenes x 2 tide rows.
- **Public rollup is effectively at native speed.** The public-capped average
  is `+0.3184%` over target / `99.6876%` target speed; the raw signed
  optimization matrix is `-0.4501%` / `100.4714%`.
- **The methodology has removed about 17.08 over-target points.** Since the
  compact full-matrix baseline, the accepted promotions add about `12.59`
  target-speed points while keeping the visual + audible scene bar intact.
- **The missing-scene confusion is closed.** MARY1/2/3 and SUZY1/2 are
  measured and green; `suzy3` is not a standalone Johnny Castaway scene route.
- **Latest VISITOR3 low probe promoted; prior misses remain recorded.** The
  v302 frame-128 resident-segment copy joins the VISITOR3 v299 and WALKSTUF1 v288/v289
  wins, while rejected scalar prime and hand-table probes stay in the
  experiment log.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.5/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.5-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.4-ps1` — Custom chapter-select thumbnails for all 63 scenes
<time datetime="2026-05-08"><em>2026-05-08</em></time>

A content-and-UX point release. The in-game Scene Explorer now ships a
custom on-PS1-captured thumbnail for every one of the 63 scenes, and the
scene-page metadata across the website is reconciled against what the
discs actually play.

- **Every Scene Explorer slot has a custom thumbnail.** Each pack was
  booted under DuckStation, played to a representative frame, captured
  at native resolution, and encoded as a 320×240 RGB555 SCR. No scene
  falls back to the auto-generated frame.
- **Scene titles and bodies match the on-PS1 packs.** Many of the prior
  caption-mapping audit guesses were wrong once watched on hardware. Per-
  scene commits lift the "(Guess.)" hedging on every page where it
  appeared.
- **Several caption-to-scene mismaps caught.** "He catches a boot" is
  MARY 2, not FISHING 2; the octopus-steals-fish gag is FISHING 3, not
  the audit's "crab snaps his nose"; coconut-plane-hit is VISITOR 5, not
  VISITOR 4; jogs-around-island is WALKSTUF 3, not WALKSTUF 1.
- **One missing-manifest bug fixed mid-loop.** `config/ps1/cd_layout.xml`
  only listed 42 of 63 SCRs; the other 21 were on disk but never made it
  onto the CD. Manifest now lists all 63.
- **`scripts/apply-scene-correction.py` is the new in-loop tool.** Single
  command updates all five sources of truth per scene (page index.md,
  scenes.yml notes, scene-status.md Notes, thumbnail SCR, progress
  tracker) in one pass, with idempotent failure mode so a re-run can't
  silently clobber a prior fix.

No perf code or pack content changed; the 120 timing-bearing rows on
[/perf/]({{ '/perf/' | relative_url }}) average the same `+0.5576%` public
over target / `99.4669%` public target speed as v0.8.3-ps1. Scene
validation scope is unchanged: 63 / 63 remain signed off.

The retrospective on the loop is at
[/lab/chapter-select-grind/]({{ '/lab/chapter-select-grind/' | relative_url }}).

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.4/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.4-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.3-ps1` — WALKSTUF1 compact foreground performance
<time datetime="2026-05-08"><em>2026-05-08</em></time>

This point release promotes the WALKSTUF1 compact foreground pack pass and
records the latest VISITOR3 follow-up as a non-promoting threshold closure.

- **WALKSTUF1 gets compact FGP3/v4 packs.** Both tides move from PAL4/FGP2 to
  padded compact FGP3/v4 restore-minus-current packs while preserving the
  `1535263` byte pack footprints, original LBAs, and the `215040` byte PS-EXE
  bucket.
- **The outlier gap drops sharply.** WALKSTUF1 high improves `1592/1406 ->
  1491/1426`; low improves `1604/1407 -> 1489/1427`.
- **Visible CD pressure falls.** High blocking drops `275 -> 85`; low blocking
  drops `270 -> 86`; loop reads drop from `134/132` to `69/69`.
- **Battle card is now public-capped at native speed.** The 120 timing-bearing
  rows average `+0.5576%` public over target / `99.4669%` public target speed;
  the raw signed CSV remains `-0.2497%` / `100.2899%` for optimization work.
- **VISITOR3 threshold probing is closed for this baseline.** Lowering the
  fallthrough guard from `6` to `5` VBlanks stayed exact-flat, so the runtime
  keeps the accepted guard and moves future VISITOR3 work back to generated
  scheduler/data-shape lanes.

The post-validation perf retrospective at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }})
covers v0.8.0 → v0.8.3 as one continuous arc.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.3/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.3-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

## Earlier milestones

### `v0.8.2-ps1` — VISITOR3 guarded-read performance
<time datetime="2026-05-07"><em>2026-05-07</em></time>

This point release promotes the next VISITOR3 high-tide guarded generated-window
read group and ships the current upstream website/docs polish with the latest
performance battle-card numbers.

- **VISITOR3 high visible CD pressure improves.** The guarded `138..162` read
  window lowers `blocking_vb 294 -> 293`, `loop_reads 40 -> 39`, and
  `loop_read_vb 335 -> 332`.
- **Loop cadence stays fixed.** VISITOR3 high remains `1406/1019` VBlanks with
  `overrun_vb=387` and `prefetch_overrun_vb=7`.
- **Battle card remains near target.** The 120 timing-bearing rows average
  `+0.5706%` over target / `99.6769%` target speed.
- **Site/docs are current with `main`.** The release includes the latest site
  navigation, glossary, lab/feed, structured-data, and page-TOC polish.

The post-validation perf retrospective at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }})
covers this and the v0.8.3 follow-on as continuations of the
v0.8.0 baseline arc.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.2/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.2-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.1-ps1` — clean-rect pressure stability
<time datetime="2026-05-06"><em>2026-05-06</em></time>

A randomized long-run soak exposed a scene-load freeze after a large split
clean-rect save. The pressure estimator was counting only foreground pack
bounds, not the ocean wave band or the upper/lower split actually saved for
restore. `v0.8.1` fixes that accounting and keeps the complete-scene
performance baseline intact. The retrospective on what the soak found that
the matrix didn't is at
[/lab/v081-mary4-freeze/]({{ '/lab/v081-mary4-freeze/' | relative_url }}).

- **Scene-load freeze fixed.** Large clean snapshots now account for wave-band
  expansion and split rects before allocation.
- **Pressure relief is generalized.** The fix covers every random-position
  scene with the same large-clean risk, not just the failing MARY4 route.
- **Focused soak routes pass.** MARY4 and FISHING1 pressure routes complete
  with `scene-end` and `alloc_fail=0`.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.1/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.1-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.0-ps1` — complete-scene performance baseline
<time datetime="2026-05-06"><em>2026-05-06</em></time>

The first release after the post-validation polish phase to promote the headless optimization methodology as a release baseline. All 63 scenes stay green under the visual + audible bar, and all 126 high/low variants are now routed through the perf matrix. Timing-bearing rows now average **+0.9% over target / [99.5% target speed]({{ '/docs/glossary/#target-speed' | relative_url }})** — about 16.5 percentage points of over-target gap closed since the compact full-matrix baseline. The retrospective on which experiments moved that line is at [/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).

- **63 / 63 scenes still validated** after the post-validation bugfix pass.
- **126 / 126 variants routed**, 120 carrying active-loop timing.
- **Performance is near target** — `+0.9%` average over target, `99.5%` target speed.
- **ACTIVITY 9 is now an optimized validated outlier** — wide-boat stitch + padded FGP3 residual packs + scoped low-tide read group.
- **Random-run clean-rect pressure fixed** — the BUILDING4 soak regression that exposed walk-clean memory pressure now releases the stale walk clean buffer, retries the large scene clean snapshot, and recaptures the walk baseline cleanly.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.0/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.0-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.7.2-ps1` — story-loop walk backdrop guard
<time datetime="2026-05-05"><em>2026-05-05</em></time>

A randomized [story-loop walking]({{ '/docs/walks/' | relative_url }}) regression let Johnny walk over water and leave repeated walking poses when the next scene's island backdrop state differed from the framebuffer left by the previous scene. The fix was a backdrop key — the runtime remembers the tide / raft / night / holiday / island position that produced the previous frame and refuses to start a walk unless the next scene matches.

- **Walks now require a matching backdrop key.** Inter-scene walks only run when tide, raft, night, holiday, and island X/Y all match the previous rendered scene.
- **Scene-policy changes force a clean scene load.** Moving from a variable-position scene to a fixed/left-island/no-raft/tide/holiday variant inside the same sequence no longer draws Johnny over stale water.
- **Menu and freeplay resets clear the walk context.** Scene Set changes, Scene Explorer launches, and Freeplay exits all invalidate the remembered backdrop.

[Full notes]({{ '/source/docs/ps1/release-notes-0.7.2/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.7.2-ps1)

### `v0.7.1-ps1` — persisted holiday mode and first-run defaults
<time datetime="2026-05-05"><em>2026-05-05</em></time>

Memory-card schema bumped to v6 to separate the holiday *policy* from the manual override, so the pause menu can offer five distinct holiday modes. New boots default to `AUTO DATE:ORIG4` — Sierra's original four overlays, automatic by date.

- **`AUTO DATE:ORIG4` is now the fresh/no-card default.** Reflects the source material's holiday set.
- **Holiday mode is persisted separately from manual selection.** Memory card schema v6.
- **Walking holiday z-order corrected.** Story-loop walks now stamp holiday overlays before Johnny so default decorations don't paint over the walking sprite.

[Full notes]({{ '/source/docs/ps1/release-notes-0.7.1/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.7.1-ps1)

### `v0.7.0-ps1` — complete scene validation
<time datetime="2026-05-05"><em>2026-05-05</em></time>

The 63-scene grind ended here. Every routed scene the original Sierra game had now plays pixel-perfect on the PS1 with synced SFX across every applicable variant. The retrospective on what the daily loop actually looked like is at [/lab/the-63-scene-grind/]({{ '/lab/the-63-scene-grind/' | relative_url }}).

- **63 / 63 scenes validated.** The live ledger is fully green under the FISHING 1 reference bar.
- **ACTIVITY 9 completed the sweep.** The final scene needed a wide-boat repair path because `BOAT.PSB` can extend past the legacy 640px scene clip.
- **Landing page now uses the ACTIVITY 9 boat screenshot.** The hero image shows the final validated scene running in the PS1 build.

[Full notes]({{ '/source/docs/ps1/release-notes-0.7.0/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.7.0-ps1)

### `v0.6.0-ps1` — ocean ambience
<time datetime="2026-05-01"><em>2026-05-01</em></time>

A 20-second CC0 ocean-loop sample plays on a dedicated SPU voice
underneath every scene. Toggleable from the pause menu; choice
persists to the memory card. Zero per-frame CPU cost — the SPU
loops the sample in hardware while the main CPU keeps doing
playback work.

- **Looping ocean ambience** on SPU voice slot reserved at boot.
- **Pause → Accessibility → Ocean: ON / OFF**, persisted to memcard.
- **Bootmode token** for capture/test discs that need a known-state run.

[GitHub release]({{ site.github_url }}/releases/tag/v0.6.0-ps1)

### `v0.6.1-ps1` through `v0.6.13-ps1` — the validation-cluster patches
<em><time datetime="2026-05-01">2026-05-01</time> → <time datetime="2026-05-04">2026-05-04</time></em>

The stability releases between `v0.6.0-ps1` (ocean ambience) and
`v0.7.0-ps1` (complete scene validation) are scene-by-scene
bring-up tags rather than feature milestones. Each one carries a
specific scene clearing the FISHING 1 bar (FISHING 5, MARY 5,
VISITOR 4–7, and others) plus the small bugfix needed to get the
scene through. They aren't enumerated here because the per-scene
narrative belongs in the [history page]({{ '/about/history/' | relative_url }})
and the [scene ledger]({{ '/scenes/' | relative_url }}) tracks the
final state. The full list of GitHub-tagged builds in this cluster
is at [{{ site.repo }}/releases]({{ site.github_url }}/releases).

### `v0.5.0-ps1` — Freeplay and debug mode
<time datetime="2026-05-01"><em>2026-05-01</em></time>

The release that turned the project from a passive screensaver port into something a player could touch. Direct-control Johnny via D-pad / analog, gag and visitor debug catalogs in the pause menu, controllable world state, and a no-allocation steady-state freeplay loop.

- **Freeplay mode** with D-pad / analog walking, fishing, and L2/R2 speed modifiers.
- **Pause-menu debug catalogs** for gags, visitors, sound effects, controls, world options, accessibility, and system pages.
- **Frog-clock loading transitions** keep the screen coherent between scene swaps.
- **Steady-state freeplay loop does not allocate.** Important because freeplay can run indefinitely.

[Full notes]({{ '/source/docs/ps1/release-notes-0.5.0/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.5.0-ps1)

### `v0.4.20-ps1` — story-loop walking
<time datetime="2026-04-30"><em>2026-04-30</em></time>

The first build where Johnny no longer teleports between finished scenes. The PS1 screensaver loop now carries his spot and heading forward, runs Sierra's original `walk_data.h` route table, and visibly walks him across the island before the next FG2 scene begins.

- **Story-loop walking** wired into the PS1 screensaver loop using the original route table and `JOHNWALK` sprite bank.
- **The ocean keeps animating during walks**, and active holiday overlays persist across scene → walk → scene transitions.
- **Palm-tree occlusion works.** The trunk and leaves are re-stamped over Johnny when the route passes behind the tree.
- **Tight, persistent walk erase buffer** (340×224, ≈149 KB) replaces the early build's per-walk free/malloc churn that fragmented the heap.

[Full notes]({{ '/source/docs/ps1/release-notes-0.4.20/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.4.20-ps1)

## Where to go from here

- [Play]({{ '/play/' | relative_url }}) is the download page — latest `.bin` / `.cue`, DuckStation quickstart, controller map.
- The [FAQ]({{ '/faq/' | relative_url }}) covers the recurring questions: what this is, why PS1, is this legal, do I need Sierra files, real-hardware support.
- The [scene ledger]({{ '/scenes/' | relative_url }}) is the live status of every scene at the current release.
- The [history page]({{ '/about/history/' | relative_url }}) is the longer narrative — pre-port era, first PS1 attempts, the hybrid pivot, the 63-scene grind.
- The [devlog]({{ '/devlog/' | relative_url }}) is the daily worklog. Releases are the milestones; the devlog is the run-up.
- Older releases (`v0.3.x` and earlier) aren't included above because they predate the per-version release-notes habit. Their commit messages and the [history page]({{ '/about/history/' | relative_url }}) cover the same ground.

The full list of all tagged releases on GitHub is at
[{{ site.repo }}/releases]({{ site.github_url }}/releases).
