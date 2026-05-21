# Johnny Castaway — PlayStation 1

A PS1 port of Sierra's classic *Johnny Castaway* screen saver. All 63
scenes are validated under the project's pixel-perfect visual + synced-SFX
bar. The runtime is a hybrid pipeline — the desktop host captures every
scene from the real Sierra engine into authored foreground packs; the PS1
replays those packs and owns only the narrow surface it must (background,
wave animation, holiday overlays, controller input, SPU audio).

> **Website ▸ [hunterdavis.com/johnny-castaway-ps1](https://hunterdavis.com/johnny-castaway-ps1/)** — full project site with the live scene ledger, performance battle card, deep-dive essays, devlog, and history. *This README is the short version.*

**Quick links:** [Download](#download-and-play) · [Status](#status) · [Quick start](#quick-start) · [Method](#method) · [Pause menu](#pause-menu) · [Captions](#closed-captions) · [Holidays](#holidays) · [Hardware](#hardware-target) · [Controls](#controls) · [Documentation](#documentation) · [Acknowledgements](#acknowledgements) · [License](#license)

<p align="center">
  <img src="docs/readme/johnny6-ps1-office.png" width="47%" alt="JOHNNY 6 on PS1: Johnny working in an office">
  <img src="docs/readme/johnny6-ps1-date-dream.png" width="47%" alt="JOHNNY 6 on PS1: Johnny dreaming about his island date">
</p>

<p align="center"><code>JOHNNY 6</code> on PS1 — office daydream · island date dream.</p>

<p align="center">
  <img src="docs/readme/fishing1-ps1-cast.png" width="31%" alt="FISHING 1 on PS1: daytime cast">
  <img src="docs/readme/fishing1-ps1-raft.png" width="31%" alt="FISHING 1 on PS1: raft-stage variant">
  <img src="docs/readme/fishing1-ps1-night.png" width="31%" alt="FISHING 1 on PS1: night variant">
</p>

<p align="center"><code>FISHING 1</code> — the project's reference scene: daytime cast · raft variant · night variant.</p>

## Download and play

Latest release → **[Releases page](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest)** · or read the **[Play page](https://hunterdavis.com/johnny-castaway-ps1/play/)** for the full quickstart with controller map.

Direct download (auto-tracks the latest tag):

- **[jcreborn.bin](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest/download/jcreborn.bin)** — PS1 CD image
- **[jcreborn.cue](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest/download/jcreborn.cue)** — cuesheet

Load `jcreborn.cue` in [DuckStation](https://www.duckstation.org/) (or any PS1 emulator). Boots straight into the screensaver loop; press **Start** for the pause menu.

## Status

| | |
|---|---|
| Current release | **`v0.8.16-ps1`** — memory-region allocator stability release |
| Reference bar | **`FISHING 1`** — pixel-perfect visuals + synced SFX across every applicable variant (night / low-tide / holiday / raft-stage) |
| Scenes validated | **63 / 63** — see the live [scene ledger](https://hunterdavis.com/johnny-castaway-ps1/scenes/) or [`docs/ps1/scene-status.md`](docs/ps1/scene-status.md) |
| Headless perf | **126 / 126** scene/tide rows are routed and timing-bearing; public-capped average is **+0.2371% over target / 99.7659% target speed** after the W1-low setup/`{113..129}` + `{355..371}` CD-pressure promotions, fresh-owner `160..176`, and entry65/entry39/entry55/entry56/entry59/entry63/entry66/entry85 payload trims, the W1-high frame56/`{178..194}` + `{423..439}` + `{404..416}` + `{395..411}` + retargeted `{411..423}` CD-pressure promotions plus prepare-first scheduler ownership, the VISITOR3-high 80 KiB clean-relief window plus setup-edge `40..47` speed and `42..49` CD-pressure retained-segment promotions, and the B2-high entries `92`/`94`/`95` trim plus `{185..197}` + `{158..174}` CD-pressure promotions and setup aliases for duplicate entries `141`/`142` and `38`. Live battle card at [/perf/](https://hunterdavis.com/johnny-castaway-ps1/perf/) · CSV at [`performance-scene-matrix.csv`](docs/ps1/performance-scene-matrix.csv) |
| Perf harness | `--require-improvement` gates now fail if the supplied baseline summary has no matching case label, preventing false-pass optimization promotions. |
| Acceptance gate | human visual + audible signoff |

The mainline shifted from "prove every scene" to **performance polish, stability, and content** at `v0.7.0-ps1`. Current mainline builds on `v0.8.16-ps1` with the new memory-region allocator promoted: BOOT allocations seal after startup, CACHE allocations use free-list/LRU reuse for long-lived resource data, and TRANSIENT scene allocations can be wiped between major scene loads instead of relying on a fragmented general heap. The allocator merge also adds pack-header metrics and CI gates requiring `MEM_REGION_RATIONALE` annotations at every `memAlloc` call site. The latest full matrix remains `126 / 126` routed and timing-bearing with 0 BSODs in the R34 allocator validation run; the latest focused allocator-era checkpoint refreshes the under-green VISITOR3, WALKSTUF1, BUILDING2, and BUILDING4 rows. Public rollup is `+0.2371%` over target / `99.7659%` target speed and raw signed rollup is about `-0.4798%` / `100.4962%`; bands are `121` green, `5` yellow, `0` orange, and `0` red. That is about `17.16` public over-target points removed and `12.67` public target-speed points added since the compact full-matrix baseline. The newest W1-low fresh-owner pocket keeps the row at `1470/1446` while lowering blocking/refill `33/5 -> 32/4`; the five-yellow canary stayed flat outside W1-low. The prior VISITOR3-high same-speed setup-edge slide moves the early retained edge from `40..47` to `42..49`, keeping V3-high at `1070/1046` while cutting loop reads/read time `5/61 -> 4/59` and hidden CD VBlanks `26 -> 24`; the five-yellow canary stayed clean. The prior VISITOR3-high setup-edge speed win merged overlapping high setup coverage into retained relative sectors `203..262` and paid early `40..47`, improving V3-high `1071/1045 -> 1070/1046`, overrun `26 -> 24`, refill `1 -> 0`, and loop reads `7 -> 5` while blocking stayed `35`. The prior B2-high duplicate-alias cleanup points entry `38` at setup-edge duplicate entry `35`, preserving the exact five-yellow timing baseline and fixed pack/executable layout; it is source/data ownership cleanup, not a VBlank speed win. The prior B2-high same-speed CD-pressure win adds `{158..174}` on top of `{185..197}` and the entries `92`/`94`/`95` trim, holding target speed at `97.476%` while reducing loop reads/read time again `43/197 -> 40/189`; the exact five-yellow canary stayed flat. The prior W1-high same-speed CD-pressure win retargets the old `{422..434}` tail row to `{411..423}` after `{395..411}`, keeping the row at `1472/1441` / `97.894%`, overrun `31`, blocking/refill `43/13`, and due `7`, while reducing loop reads/read time `42/201 -> 41/198`; the exact five-yellow canary stayed flat. The prior W1-high scheduler win lets high tide use the accepted WALKSTUF1 prepare-before-window ownership path, improving blocking `56 -> 43` and due misses `10 -> 7` while keeping overrun flat at `31` and the five-yellow canary clean. The recent W1-low payload-only trim cuts entry85/source frame148 from `4854 -> 4351` bytes after the prior entry65/source frame96 (`4630 -> 1666`), entry39/source frame49 (`7835 -> 4724`), entry55/source frame65 (`4716 -> 2`), entry56/source frame67 (`3425 -> 324`), entry59/source frame78 (`4200 -> 456`), entry63/source frame91 (`4643 -> 1244`), and entry66/source frame100 (`4661 -> 2082`) trims, reducing active payload `788773 -> 764658` across all eight trims while the five-yellow canary stays exact-flat; these are not counted as VBlank speed wins. The prior B2-high work-volume alias points duplicate entries `141`/`142` at setup-resident payloads for entries `116`/`118`, keeping the five-yellow canary exact-flat while reducing uncovered active ownership `286/519400 -> 284/518994`; it is not counted as a VBlank speed win. The prior W1-high same-speed CD-pressure win added `{404..416}` on top of `{423..439}`, the frame56/source67 trim, and `{178..194}`, holding target speed at `97.893%` while reducing loop reads/read time again `42/205 -> 41/200`; the five-yellow canary stayed flat. The prior B2-high same-speed CD-pressure win added `{185..197}` on top of the entries `92`/`94`/`95` trim, holding target speed at `97.476%` while reducing loop reads/read time `45/199 -> 43/197`; the five-yellow canary stayed flat. The prior W1-high same-speed CD-pressure win added `{423..439}`, holding target speed at `97.893%` while reducing loop reads/read time `43/207 -> 42/205`. The prior BUILDING2-high work-volume win trims entries `92`, `94`, and `95` in place (`8834 -> 6370`, `8873 -> 6939`, and `10247 -> 8827` bytes), reducing active payload `669408 -> 663590` while the five-yellow canary stays exact-flat. The prior VISITOR3-high clean-relief win widens the high-tide relief stream window from `68 KiB` to `80 KiB` while keeping the `56 KiB` tight-refill cap, improving active loop/target `1075/1044 -> 1071/1045`, overrun `31 -> 26`, blocking `45 -> 35`, refill `3 -> 1`, and due `3 -> 2`; the `96 KiB` variants were rejected because they saved a read but regressed loop/target cadence. The prior W1-high CD-pressure win trims WALKSTUF1 high entry56/source67 in place (`3425 -> 324` bytes, `2 -> 1` sectors) and pairs it with `{178..194}`, holding target speed at `97.893%` while improving blocking `57 -> 56` and loop reads/read time `45/209 -> 43/207`. The recent W1-low CD-pressure wins shift the main setup residency to `244..350`, keep a split `179..185` setup edge, add `{113..129}`, then add `{355..371}` as a same-speed work-volume row, holding target speed at `98.367%` while improving scene `1812 -> 1809`, blocking/refill `34/6 -> 33/5`, and loop reads/read time `30/159 -> 24/147`. The prior W1-high work-volume win trims entry57/source70 in place (`3409 -> 320` bytes, `3 -> 2` sectors) on top of entry136/source244 (`3762 -> 2596` bytes, `3 -> 2` sectors), with fixed pack LBA/PS-EXE bucket and exact-flat five-yellow timing. The prior speed win adds the WALKSTUF1 low `{378..390}` retained read group after splitting a broader refill-debt batch, improving `1470/1445 -> 1470/1446`, overrun `25 -> 24`, blocking/refill `35/7 -> 34/6`, loop reads/read time `31/163 -> 30/159`, and target speed `98.299% -> 98.367%`; due stays `4`.

The latest code-headroom pass caches the active foreground scene ID so hot
scheduler paths no longer repeat scene-name string compares. The five-yellow
canary stays exact-flat, the PS-EXE remains in the `233472` byte bucket, and
the tracked foreground hot symbols shrink (`foregroundPilotPlay -84`,
`fgRuntimeLoadSceneFrame -52`, `fgRuntimeFillWindowForEntry -24`, and
`fgRuntimeTryPrefetchWindow -12` bytes). This does not change the public speed
rollup, but it lowers code-size pressure for the next generated-owner and
custom data-shape swings.

The v0.8.13 checkpoint also extended BUILDING2 high preserve-offset payload trims through `building2-high-frame100-inplace-v926`: entry `172` / source frame `231` shrinks `1831 -> 851` bytes, entry `171` / source frame `228` shrinks `1980 -> 1025` bytes, entry `96` / source frame `119` shrinks `8781 -> 7944` bytes, entry `170` / source frame `226` shrinks `1683 -> 1186` bytes, entry `97` / source frame `121` shrinks `8718 -> 8258` bytes, entry `98` / source frame `123` shrinks `8876 -> 8637` bytes, entry `174` / source frame `239` shrinks `1625 -> 1460` bytes, entry `99` / source frame `126` shrinks `8843 -> 8728` bytes, entry `168` / source frame `219` shrinks `1372 -> 1266` bytes, entry `169` / source frame `223` shrinks `1820 -> 1495` bytes, entry `173` / source frame `235` shrinks `1765 -> 1134` bytes, and entry `100` / source frame `129` shrinks `8701 -> 8621` bytes. The current B2-high work-volume baseline then trims entries `92`, `94`, and `95` (`8834 -> 6370`, `8873 -> 6939`, `10247 -> 8827`) without moving pack offsets, then aliases duplicate entries `141`/`142` to setup-resident payloads and entry `38` to setup-edge duplicate entry `35`. Active payload drops `674798 -> 663590`, uncovered active ownership drops `286/519400 -> 284/518994`, and the entry38 alias keeps the five-yellow canary exact-flat; the current B2-high speed baseline layers the allocator-era setup slices with the `83..95`, guarded `271..287`, and `315..327` scheduler rows.

Recent releases:

- Current allocator-era optimization checkpoint — VISITOR3 high improves from `1232/1033` to `1070/1046` and VISITOR3 low from `1231/1040` to `1065/1039` by retaining the tiny stage1 prefetch frame buffer under clean-memory relief, keeping bounded clean-relief stream windows, trimming the high-tide terminal read before resident setup-segment data, extending and merging high setup residency through relative sectors `203..262`, relocating high frame `139` and high frames `56`/`57` raw into that paid gap with a `56 KiB` tight-refill cap, capping high clean strips at `64 KiB`, widening the high clean-relief stream window to `80 KiB`, then paying the early retained setup edge `40..47` and sliding the same-speed pressure baseline to `42..49`. Low adds the third retained setup segment `206..230` and moves frame `138` raw into the paid `206..232` gap. BUILDING4 high primes `264..288` during setup and moves into green, BUILDING2 high replaces its tail read group with `83..95`, adds guarded `271..287` plus `315..327`, trims entries `92`/`94`/`95`, adds `{185..197}` and `{158..174}` as same-speed CD-pressure work, and aliases entries `141`/`142` and `38` to setup/setup-edge duplicates, WALKSTUF1 low replaces its split `197..243` + `410..434` residency with `238..344` after a low-only 48 KiB clean-rect cap, adds `{91,107}`, pays a split TRANSIENT `344..350` setup edge, trims frame132, adds `{378,390}`, retargets setup to `244..350` plus split `179..185` with `{113,129}`, adds `{355,371}` to lower CD debt while staying `1470/1446`, adds fresh-owner `160..176` to lower blocking/refill `33/5 -> 32/4`, and banks entry65, entry39, entry55, entry56, entry59, entry63, entry66, and entry85 as exact-flat payload work, WALKSTUF1 high keeps `198..244`, retargets its second retained slice to `286..344`, adds `{149,165}`, encodes frame `92` as D4, trims entries `136` and `57` exact-flat, then trims frame56 and pairs `{178,194}`, adds `{423,439}`, `{404,416}`, `{395,411}`, and retargeted `{411,423}`, and now uses prepare-before-window ownership to reach `1472/1441` with blocking `43`, due `7`, and loop reads/read time `41/198`, BUILDING2 low now primes `112..128` and `226..262` during setup with a low-only `80 KiB` clean strip cap plus `{141,153}` to reach green at `1327/1318`, and BUILDING4 low now combines the `8`-row dirty-upload band merge gap with a `24 KiB` stream window to reach green at `2847/2820`. The current under-green queue is BUILDING2 high, VISITOR3 low, VISITOR3 high, WALKSTUF1 high, and WALKSTUF1 low.
- Current code-headroom promotion — the foreground scheduler now caches the active scene ID once per scene start and uses that ID in hot per-frame checks. Five yellow rows stayed exact-flat, the `233472` byte PS-EXE bucket and pack LBAs stayed fixed, and `foregroundPilotPlay`/window-fill/load/prefetch hot symbols shrank by `172` bytes combined.
- `v0.8.16-ps1` — memory-region allocator stability release. Introduces BOOT/CACHE/TRANSIENT allocation regions, scene-boundary transient wipes, CACHE free-list/LRU reuse, generated pack-header metrics, and CI rationale gates for allocator call sites. The R34 full matrix remains `126/126` PASS with 0 BSODs. Release battle card after the allocator refresh was `+0.5699%` over target / `99.4843%` target speed; bands were `118` green, `4` yellow, `2` orange, and `2` red.
- `v0.8.15-ps1` — WALKSTUF1 high setup-resident CD promotion. High tide primes relative sectors `242..388` during setup, improving the active loop/target from `1481/1428` to `1476/1441`, overrun `53 -> 35`, blocking/refill `88/24 -> 49/17`, reads/read time `67/287 -> 37/182`, and due `15 -> 7`. Public rollup improves to `+0.2285%` over target / `99.7746%` target speed; bands remain `119` green and `7` yellow.
- `v0.8.14-ps1` — JOHNNY1 local-LZ green promotion. Both JOHNNY1 tides compress full-frame entries `1` and `50` in-place behind a scene-local sentinel stream, preserving the `448370` byte high/low pack footprints, pack LBAs/sectors, and the `217088` byte PS-EXE bucket while improving both rows to `1948/1945`, overrun `3`, blocking/refill `5`, and target speed `99.85%`. Public rollup improves to `+0.2492%` over target / `99.7548%` target speed; bands are `119` green and `7` yellow.
- `v0.8.13-ps1` — under-99 payload-work checkpoint. WALKSTUF1 low advances to `walkstuf1-low-frame106-inplace-v910`, BUILDING2 high advances to `building2-high-frame173-inplace-v914`, BUILDING4 low advances to `building4-low-frame283-inplace-v913`, and the JOHNNY1 black-clear, BUILDING2 high early-row, and WALKSTUF1 low frame132 misses are logged as closed. Public rollup is `+0.2697%` over target / `99.7347%` target speed; bands remain `117` green and `9` yellow.
- `v0.8.12-ps1` — WALKSTUF1 low advances to `walkstuf1-low-frame130-inplace-v795`: frames `77` and `130` shrink in-place on top of the restored lazy-stream baseline without moving pack offsets, keeping `1770/1478/1431`, blocking/refill `64/20`, read time `60/272`, and due `11` exact-flat while active payload drops `879801 -> 799694`. Public rollup remains `+0.2708%` over target / `99.7337%` target speed; bands remain `117` green and `9` yellow.
- `v0.8.11-ps1` — corrective point release after `v0.8.10`: restores lazy CD/FG stream buffers after a heap-fragmentation merge pinned `256 KB` of CD sector pool plus boot-time FG stream buffers and caused W1-low to skip after a clean-rect allocation failure. The accepted rollback returns W1-low exact-flat to the v791 baseline at `1770/1478/1431`, blocking/refill `64/20`, read time `60/272`, and due `11`. Public rollup remains `+0.2708%` over target / `99.7337%` target speed; bands remain `117` green and `9` yellow.
- `v0.8.10-ps1` — WALKSTUF1 low continues the no-shift payload-reduction lane through `walkstuf1-low-frame76-inplace-v791`: frames `51`/`49`/`47`/`61`/`62`/`58`/`45`/`37`/`35`/`43`/`41`/`57`/`33`/`67`/`68`/`69`/`32`/`133`/`5`/`141`/`70`/`30`/`6`/`71`/`72`/`142`/`73`/`131`/`74`/`19`/`28`/`138`/`145`/`75`/`76` now shrink in-place without moving pack offsets. Timing stays exact-flat at `1478/1431`, blocking/refill `64/20`, read time `60/272`, and due `11`, while active payload drops `879801 -> 801103`. Public rollup remains `+0.2708%` over target / `99.7337%` target speed; bands are `117` green and `9` yellow.
- `v0.8.9-ps1` — VISITOR5 low uses the low-tide `30..46` retained-read group, improving from `1104/1092` to `1102/1097` and moving that row into green. WALKSTUF1 low gained the same-speed shared `427..443` CD-work row, then high/low late-tail and isolated low offscreen work-volume clips through v726, followed by v705 low late-tail physical compaction that cuts loop reads/read time `62/281 -> 60/274` on the clean current baseline while staying exact-flat at `1478/1431`; v747/v749/v750/v751/v753/v755/v756/v757/v759/v762/v763/v766/v767/v769/v770/v771/v772/v773/v774/v775/v776/v777/v779/v780/v781/v782/v783/v784/v785/v786/v787/v788/v789/v790/v791 now cut low frame51/frame49/frame47/frame61/frame62/frame58/frame45/frame37/frame35/frame43/frame41/frame57/frame33/frame67/frame68/frame69/frame32/frame133/frame5/frame141/frame70/frame30/frame6/frame71/frame72/frame142/frame73/frame131/frame74/frame19/frame28/frame138/frame145/frame75/frame76 active payload in-place while staying exact-flat, and the v760 mainline fast-poll follow-up restores the current row to `1478/1431`, blocking/refill `64/20`, read time `60/272`. W1-high frames `55`, `138`, `51`, `49`, `47`, and `45` now clip safe offscreen draw spans and drop the high runtime work to `17011/131649/745213` while staying exact-flat at `1476/1434`. BUILDING2 low `218..229` slack8 first improves `1349/1320 -> 1344/1318`, and v739 draw-tail trimming now improves it to `1339/1317`, overrun `22`, blocking `53`, reads/read time `37/150`, and due `12`. BUILDING2 high now has exact-flat offscreen work-volume clips for frames `89..92`, `94..104`, and `168..177`. VISITOR3 high tail repack improves `1065/1039 -> 1063/1040`, overrun `26 -> 23`, blocking `41 -> 35`, and reads/due `7/7 -> 6/6`; BUILDING4 low offscreen pack clipping improves `2856/2816 -> 2853/2816`, overrun `40 -> 37`, blocking `44 -> 40`, and refill `37 -> 34`, then v746 cuts frame291 active payload in place while staying exact-flat. Public rollup is `+0.2708%` over target / `99.7337%` target speed; raw signed rollup is `-0.4963%` / `100.5160%`; bands are `117` green and `9` yellow.
- `v0.8.8-ps1` — VISITOR5 high now uses a high-tide `30..46` retained-read group, improving to `1101/1096`, overrun `5`, blocking/refill `5`, reads/due `18/0`, and moving that row into green. Public rollup is `+0.2867%` over target / `99.7183%` target speed; raw signed rollup is `-0.4805%` / `100.5006%`; bands are `116` green and `10` yellow.
- `v0.8.7-ps1` — deterministic BOOTMODE scene selection, expected-scene gates in the headless perf harness, Suzy backdrop cleanup hardening, and heapless Scene Explorer thumbnail streaming. Public rollup remains `+0.3156%` over target / `99.6902%` target speed.
- `v0.8.6-ps1` — WALKSTUF1 low gap6-prefix + slack-guard promotion, WALKSTUF1 high window-prefetch / slack4 guard, and VISITOR3 high/low setup-segment resident copies for frames `131` / `128`. Public rollup `+0.3157%` over target / `99.6902%` target speed.
- `v0.8.5-ps1` — full 126-row headless performance matrix baseline.
- `v0.8.4-ps1` — on-PS1 captured thumbnails for every Scene Explorer slot; per-scene metadata reconciled against what the discs play. [Retrospective.](https://hunterdavis.com/johnny-castaway-ps1/lab/chapter-select-grind/)
- `v0.8.3-ps1` — WALKSTUF1 compact FGP3/v4 restore-minus-current packs.
- `v0.8.2-ps1` — VISITOR3 guarded-read group performance promotion.
- `v0.8.1-ps1` — clean-rect pressure-estimator stability fix surfaced by long randomized soak. [Retrospective.](https://hunterdavis.com/johnny-castaway-ps1/lab/v081-mary4-freeze/)
- `v0.8.0-ps1` — complete-scene performance baseline. [Retrospective.](https://hunterdavis.com/johnny-castaway-ps1/lab/from-87-to-99-5/)

Full release history at **[/releases/](https://hunterdavis.com/johnny-castaway-ps1/releases/)**. Per-release release notes live in [`docs/ps1/release-notes-*.md`](docs/ps1/).

## Quick start

**Prerequisites:** Docker (the build runs in `jc-reborn-ps1-dev:amd64` with PSn00bSDK 0.24 baked in), DuckStation, and the original Sierra data files (see [Original data files](#original-data-files) below).

```bash
./scripts/rebuild-and-let-run.sh noclean
```

Builds the PS1 EXE, packs `jcreborn.bin` / `jcreborn.cue` via `mkpsxiso`, launches DuckStation, and boots into `FISHING 1` via `BOOTMODE.TXT`. Default mode is the screensaver loop (each replay randomizes night / low-tide / raft / holiday). Add `noloop` to the boot string for a single-shot play.

A watchdog (`RUN_TIMEOUT_SECONDS`, default 300s) kills the emulator if it's left running; override with `RUN_TIMEOUT_SECONDS=<n>` or `0` to disable.

To bring up a new scene, see **[/docs/dev-workflow/](https://hunterdavis.com/johnny-castaway-ps1/docs/dev-workflow/)** ([raw](docs/ps1/development-workflow.md)).

## Original data files

The CD image build needs Sierra's original `Johnny Castaway` data files locally:

| File | Bytes | md5 |
|---|---:|---|
| `RESOURCE.MAP` | 1,461 | `8bb6c99e9129806b5089a39d24228a36` |
| `RESOURCE.001` | 1,175,645 | `374e6d05c5e0acd88fb5af748948c899` |

Drop them under `jc_resources/`. The repo tracks extracted VAGs and other derived artifacts; the master files are gitignored and must be present locally.

Optional — sound effects from [JCOS resources](https://github.com/nivs1978/Johnny-Castaway-Open-Source/tree/master/JCOS/Resources):

<details>
<summary><code>sound0..sound24.wav</code> expected hashes</summary>

| File | Bytes | md5 |
|---|---:|---|
| `sound0.wav` | 10,768 | `53695b0df262c2a8772f69b95fd89463` |
| `sound1.wav` | 11,264 | `35d08fdf2b29fc784cbec78b1fe9a7f2` |
| `sound2.wav` | 1,536 | `f93710cc6f70633393423a8a152a2c85` |
| `sound3.wav` | 7,680 | `05a08cd60579e3ebcf26d650a185df25` |
| `sound4.wav` | 5,120 | `be4dff1a2a8e0fc612993280df721e0d` |
| `sound5.wav` | 3,072 | `24deaef44c8b5bb84678978564818103` |
| `sound6.wav` | 15,872 | `eb1055b6cf3d6d7361e9a00e8b088036` |
| `sound7.wav` | 15,360 | `cab94bace3ef401238daded2e2acec34` |
| `sound8.wav` | 2,560 | `39515446ceb703084d446bd3c64bfbb0` |
| `sound9.wav` | 3,584 | `f86d5ce3a43cbe56a8af996427d5c173` |
| `sound10.wav` | 20,480 | `5b8535f625094aa491bf8e6246342c77` |
| `sound12.wav` | 5,632 | `8c173a95da644082e573a0a67ee6d6a3` |
| `sound14.wav` | 11,776 | `e064634cfb9125889ce06314ca01a1ea` |
| `sound15.wav` | 3,072 | `b3db873332dda51e925533c009352c90` |
| `sound16.wav` | 7,680 | `2eabfe83958db0cad77a3a9492d65fe7` |
| `sound17.wav` | 4,608 | `2497d51f0e1da6b000dae82090531008` |
| `sound18.wav` | 14,336 | `994a5d06f9ff416215f1874bc330e769` |
| `sound19.wav` | 3,584 | `5e9cb5a08f39cf555c9662d921a0fed7` |
| `sound20.wav` | 7,680 | `80e7eb0e0c384a51e642e982446fcf1d` |
| `sound21.wav` | 5,120 | `1a3ab0c7cec89d7d1cd620abdd161d91` |
| `sound22.wav` | 1,536 | `a0f4179f4877cf49122cd87ac7908a1e` |
| `sound23.wav` | 2,048 | `52fc04e523af3b28c4c6758cdbcafb84` |
| `sound24.wav` | 9,728 | `5a6696cda2a07969522ac62db3e66757` |

</details>

## Method

The PS1 build is deliberately hybrid, not a from-scratch engine rewrite:

1. **Desktop host** runs Sierra's TTM/ADS interpreter and captures every visible foreground draw plus every `PLAY_SAMPLE` opcode into a per-frame JSON bundle.
2. A **pack compiler** turns that capture into PS1-native FG2 / FGP3 packs — high-tide and low-tide base-diff spans plus a per-frame sound-event table.
3. On **PS1**, [`foreground_pilot.c`](src/foreground_pilot.c) loads the pack and stamps captured frames in step with a narrow runtime that handles background, wave animation, holiday overlay, and SPU playback. SFX fire on cue with a 3-frame delay so sample key-on matches the visible trigger.

Full pipeline — pack format byte layout, hardware constraints hit on the way, the SPI pad-poll fix, dirty-rect bookkeeping — at **[/about/method/](https://hunterdavis.com/johnny-castaway-ps1/about/method/)**.

## Pause menu

<p align="center">
  <img src="docs/readme/scene-explorer.png" width="72%" alt="Scene Explorer running on PS1: top band reads SCENE EXPLORER, 5/63 * validated, FISHING 5 Eaten by a shark, Family Fishing, Frames 69; the captured-on-PS1 thumbnail of FISHING 5 (shark on the right side of the island chewing Johnny) sits in the middle; bottom band reads Pack FG/FISHING5.FG2, navigation hints LEFT/RIGHT scene, L1/R1 family, X play, Triangle loop, O back">
</p>

<p align="center">Pause → <strong>Scene Explorer</strong>: jump straight to any of the 63 scenes. Each entry shows the captured-on-PS1 thumbnail, family, frame count, and pack name. New in <code>v0.8.4-ps1</code>; full reference at <a href="https://hunterdavis.com/johnny-castaway-ps1/docs/pause-menu/#scene-explorer">/docs/pause-menu/#scene-explorer</a>.</p>

Press **Start** mid-scene. Twelve sub-screens reachable from the main pause overlay:

- **Scene Set** · **Scene Explorer** — pool selector across seven categories, plus the chapter-select grid above.
- **Freeplay: ON / OFF** · **Freeplay Options** — direct-control Johnny mode plus its gag, visitor, and controls catalogs.
- **World Options** — day/night, tide, raft stage, holidays, island position.
- **Accessibility** — captions, sound, ocean ambience, Sound Test.
- **System** — save to memcard, set time/date, set RNG seed, perf log, reset scene, next scene.

Auto-generated walkthrough with screenshots of every menu screen at **[/help/menu/](https://hunterdavis.com/johnny-castaway-ps1/help/menu/)**.

## Holidays

<p align="center">
  <img src="docs/ps1/holidays-emblems/holiday-emblems-preview.png" width="72%" alt="Added holiday emblem sprite sheet">
</p>

The original Sierra game shipped four baked-in holiday decorations (Christmas, New Year, Halloween, St. Patrick's). This port extends that to **36 US holidays** via a code-generated table and a pure-algorithm date core (Meeus for Easter, Nth-weekday math for the others) — no external date library, no expiring tables, works for 100+ years. Reference at **[/docs/holidays/](https://hunterdavis.com/johnny-castaway-ps1/docs/holidays/)**.

## Closed captions

<p align="center">
  <img src="docs/readme/fishing1-captions.png" width="62%" alt="FISHING 1 with closed captions enabled, showing a dark band at the bottom of the frame with subtitle text">
</p>

Pause → **Accessibility** → **Captions: ON**. A dark band appears at the bottom of the frame for ~5 seconds at scene start with a short descriptive subtitle. Off by default; the toggle persists to memcard. Glyph atlas is shared with the pause menu (one font upload, no duplication).

The corpus was authored fresh from scene content — see [`docs/ps1/caption-audit-2026-04-26.yaml`](docs/ps1/caption-audit-2026-04-26.yaml) for confidence ratings, and **[/docs/captions/#post-validation-runtime-corrections-v084-ps1](https://hunterdavis.com/johnny-castaway-ps1/docs/captions/#post-validation-runtime-corrections-v084-ps1)** for the v0.8.4 chapter-select-grind reconciliation (several scene-to-caption mappings drifted from the on-PS1 gags and have been corrected on the website's per-scene pages — the runtime `captionSceneMap[]` is open work).

Implementation: [`src/ps1_captions.c`](src/ps1_captions.c) / [`.h`](src/ps1_captions.h) (corpus + renderer), [`src/foreground_pilot.c`](src/foreground_pilot.c) (per-scene fire), [`src/graphics_ps1.c`](src/graphics_ps1.c) (`captionsRender()` inside `grUpdateDisplay`).

## Hardware target

| | |
|---|---|
| Main RAM | 2 MB |
| VRAM | 1 MB |
| SPU RAM | 512 KB (all 23 SFX VAGs preloaded at boot) |
| Output | 640 × 480 interlaced, NTSC |

Every rendering decision is forced by this budget. A full-frame video approach was ruled out early (614 KB per 640×480 16-bit frame × 63 scenes ≈ gigabytes); the hybrid-playback model sidesteps that by keeping foreground content in authored packs and a narrow runtime for background / waves / overlays.

Reference + the gotchas hit in practice at **[/docs/hardware/](https://hunterdavis.com/johnny-castaway-ps1/docs/hardware/)**.

## Controls

Normal screensaver mode needs no input. Press **Start** to open the pause menu.

| Control | Action |
|---|---|
| Start | Open pause menu / resume |
| D-pad / left analog | Move cursor or adjust values |
| Cross | Select / apply |
| Circle | Back from any menu or submenu |

**Freeplay** (launched from the pause menu) gives direct control of Johnny:

| Control | Action |
|---|---|
| D-pad / left analog | Walk Johnny; movement cancels the current action |
| L2 / R2 held | Slow / fast walk |
| Circle | Fish from the nearest side of the island |
| Select | Clear screen, cancel transient actions, rebuild island |
| R1 + Up / Down / Left / Right | Day/night · tide · raft stage · holiday |
| Start | Open pause menu |

## Documentation

The website is the rendered, cross-linked, prose-context view; the GitHub paths under `docs/ps1/` are the raw source. Pick whichever you prefer.

**Top-level surfaces**

- **[/play/](https://hunterdavis.com/johnny-castaway-ps1/play/)** — download + DuckStation quickstart + controls.
- **[/help/](https://hunterdavis.com/johnny-castaway-ps1/help/)** — auto-generated player help with menu screenshots.
- **[/scenes/](https://hunterdavis.com/johnny-castaway-ps1/scenes/)** — live ledger of all 63 scenes + per-scene case studies.
- **[/perf/](https://hunterdavis.com/johnny-castaway-ps1/perf/)** — 126-variant headless perf battle card.
- **[/about/](https://hunterdavis.com/johnny-castaway-ps1/about/)** — project overview ([method](https://hunterdavis.com/johnny-castaway-ps1/about/method/), [status](https://hunterdavis.com/johnny-castaway-ps1/about/status/), [history](https://hunterdavis.com/johnny-castaway-ps1/about/history/), [voice guide](https://hunterdavis.com/johnny-castaway-ps1/about/voice/), [dev environment](https://hunterdavis.com/johnny-castaway-ps1/about/dev-environment/)).
- **[/docs/](https://hunterdavis.com/johnny-castaway-ps1/docs/)** — eighteen reference manuals: build, captions, holidays, pause menu, freeplay, story-loop walks, regtest, scripted input, performance, hardware, audio, infrastructure, file formats, AI sub-agents, vision-classifier, the SDL2 → PSn00bSDK API mapping, dev workflow, glossary.
- **[/lab/](https://hunterdavis.com/johnny-castaway-ps1/lab/)** — sixteen feature-length retrospectives (LLM-assisted dev, hallucination control, build farm, regression practice, the soak-loop freeze, the chapter-select grind, the post-validation perf arc).
- **[/hack/](https://hunterdavis.com/johnny-castaway-ps1/hack/)** — for curious hackers: learning C from this codebase, porting Johnny to a new platform, the printf-driven perf loop, the visual-debug catalog.
- **[/archaeology/](https://hunterdavis.com/johnny-castaway-ps1/archaeology/)** — the full 5-chapter project story.
- **[/devlog/](https://hunterdavis.com/johnny-castaway-ps1/devlog/)** — dated worklogs preserved verbatim.
- **[/credits/](https://hunterdavis.com/johnny-castaway-ps1/credits/)** · **[/legal/](https://hunterdavis.com/johnny-castaway-ps1/legal/)** · **[/faq/](https://hunterdavis.com/johnny-castaway-ps1/faq/)**.

**Raw doc source** — [`docs/ps1/`](docs/ps1/) on GitHub. Notable entry points: [`scene-status.md`](docs/ps1/scene-status.md) (per-scene ledger), [`current-status.md`](docs/ps1/current-status.md) (narrative), [`development-workflow.md`](docs/ps1/development-workflow.md) (per-scene loop), [`performance-scene-matrix.csv`](docs/ps1/performance-scene-matrix.csv) + [`performance-experiment-log.md`](docs/ps1/performance-experiment-log.md) (perf source of truth), [`release-notes-*.md`](docs/ps1/) (per-release notes), [`research/`](docs/ps1/research/) (dated design logs ↔ `/devlog/`), [`archaeology/`](docs/ps1/archaeology/) (timeline, retired tools, vision-artifacts).

## Repo lineage

This project began as a branch of [jno6809/jc_reborn](https://github.com/jno6809/jc_reborn) focused on a PlayStation 1 port. It has diverged far enough — hybrid scene-playback pipeline, per-scene captures, FG2 / FGP3 pack format, PS1 SPU playback path, scene-by-scene validation ledger — that it now lives in its own repository. Without that engine decode this port wouldn't exist.

## Acknowledgements

Short list; the full version with context per name is at **[/credits/](https://hunterdavis.com/johnny-castaway-ps1/credits/)**.

- [jno6809/jc_reborn](https://github.com/jno6809/jc_reborn) — engine decode + the original Johnny Reborn project
- Hans Milling (`nivs1978`), [JCOS](https://github.com/nivs1978/Johnny-Castaway-Open-Source)
- Alexandre Fontoura (`xesf`), [Castaway](https://github.com/xesf/castaway)
- [Sierra Chest's Johnny Castaway archive](http://sierrachest.com/index.php?a=games&id=255&title=johnny-castaway)
- Jeff Tunnell · Kevin and Liam Ryan · Jaap · Gregori · Guido
- [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK) · [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) · [DuckStation](https://github.com/stenzek/duckstation)
- BigSoundBank ("Sea: Waves" CC0) — ocean ambience source

## Transparency

Claude, Gemini, and OpenAI Codex were all used extensively across this project — for programming, debugging support, and generating prose for the website. Decisions and the merge bar are Hunter's; first drafts often were not. Full disclosure at **[/docs/agents/](https://hunterdavis.com/johnny-castaway-ps1/docs/agents/)** and on the footer of every page of the site.

## License

GPL-3.0, inherited from upstream `jc_reborn`.
