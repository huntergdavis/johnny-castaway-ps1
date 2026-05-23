---
title: Performance battle card
eyebrow: Headless · 126-variant matrix
subtitle: Click a column header to sort. Target Speed cells are color-coded — ≥99% green, ≥95% yellow, ≥90% orange, <90% red.
description: The Johnny Castaway PS1 fan port headless-perf battle card — 126 scene/tide variants timed against target frame budget, color-coded, with sortable column headers.
---

{%- comment -%}
  Schema.org Dataset structured data. The 126-row scene/tide timing
  matrix on this page IS a published dataset — Google's Dataset Search
  indexes Schema.org Dataset records, and AI/agent crawlers consume
  the same. Hand-mirrored next to the matrix it describes so updates
  stay in one place.

  variableMeasured names six PropertyValue entries (loop_vb, target_vb,
  over_target_percent, blocking_vb, prefetch_overrun_vb, loop_reads) —
  the durable numeric measurements the body's column-by-column glossary
  describes. The distribution.contentUrl points at the raw CSV in the
  repo (durable source); the page itself is the landing page (url).

  site_root construction mirrors _includes/json-ld.html: site.url +
  site.canonical_baseurl + path. Absolute URLs survive the build's
  --baseurl "" override.
{%- endcomment -%}
{%- assign site_root = site.url | append: site.canonical_baseurl -%}
{%- assign perf_csv_url = site.github_url | append: '/blob/main/docs/ps1/performance-scene-matrix.csv' -%}
<script type="application/ld+json">
{
  "@context": "https://schema.org",
  "@type": "Dataset",
  "name": "Johnny Castaway PS1 — headless performance battle card",
  "description": "Headless DuckStation timing matrix for the Johnny Castaway PS1 fan port: 126 scene/tide variants timed against captured target frame budgets, with public-capped target speed, blocking-read VBlanks, prefetch overrun, and per-variant CD pack metadata. Regenerated from the regtest harness output on every tagged release.",
  "url": {{ site_root | append: '/perf/' | jsonify }},
  "inLanguage": "en",
  "keywords": ["PlayStation 1", "PSn00bSDK", "screensaver", "performance", "regression testing", "DuckStation", "fan port", "Johnny Castaway"],
  "creator": {
    "@type": "Person",
    "name": {{ site.author | jsonify }},
    "url": "https://hunterdavis.com/"
  },
  "license": "https://www.gnu.org/licenses/gpl-3.0.html",
  "isAccessibleForFree": true,
  {%- if site.release.release_date %}
  "dateModified": {{ site.release.release_date | jsonify }},
  {%- endif %}
  "version": {{ site.release.tag | jsonify }},
  "isPartOf": {
    "@type": "WebSite",
    "name": {{ site.title | jsonify }},
    "url": {{ site_root | append: '/' | jsonify }}
  },
  "distribution": [
    {
      "@type": "DataDownload",
      "name": "Performance scene matrix (CSV)",
      "encodingFormat": "text/csv",
      "contentUrl": {{ perf_csv_url | jsonify }}
    }
  ],
  "variableMeasured": [
    { "@type": "PropertyValue", "name": "loop_vb", "description": "VBlanks per scene loop, measured by the headless DuckStation harness." },
    { "@type": "PropertyValue", "name": "target_vb", "description": "VBlanks per scene loop, captured from the host reference run as the target frame budget." },
    { "@type": "PropertyValue", "name": "over_target_percent", "description": "Public display: how far loop_vb is above target, capped at 0.0% so faster-than-target rows do not create negative debt." },
    { "@type": "PropertyValue", "name": "blocking_vb", "description": "VBlanks spent in blocking CD reads — the headroom budget the prefetch window did not cover." },
    { "@type": "PropertyValue", "name": "prefetch_overrun_vb", "description": "VBlanks where the prefetch window exceeded its budget — pack data still in flight when the runtime needed it." },
    { "@type": "PropertyValue", "name": "loop_reads", "description": "Number of CD read transactions per scene loop." }
  ]
}
</script>

{% assign all_scenes      = site.data.scenes %}
{% assign validated_count = all_scenes | where: "status", "validated"  | size %}
{% assign total_count     = all_scenes | size %}

A labor of love by Hunter Davis. This page is the other ledger
that lives next to the [scene ledger]({{ '/scenes/' | relative_url }}).
The scene ledger tracks visual signoff (pixel-perfect against host
capture, signed off by human review). This page tracks
**headless-perf timing** — automated DuckStation runs that measure
`loop_vb` against `target_vb` for every scene/tide variant. They
are different bars and should not be mixed; their failure modes
are uncorrelated.

If you paid for this, you were cheated. Open source and free.

The reference manual for the perf work is at
[/docs/performance/]({{ '/docs/performance/' | relative_url }});
the retrospective on the optimization loop that moved this matrix
from the compact baseline to its current public-capped `{{ site.release.perf_target_speed_pct }}%`
target-speed average is at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).
A scene can be timed here without being visually certified.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

<noscript>
<p><em>Note:</em> the column-header click-to-sort affordance the
subtitle mentions requires JavaScript. Without it, the matrix
below is fully readable but cells stay in source order. The
durable numeric source is the
<a href="https://github.com/{{ site.repo }}/blob/main/docs/ps1/performance-scene-matrix.csv">CSV</a>
linked in the Rollup section.</p>
</noscript>

## At a glance

<p class="scene-perf-legend" aria-label="Current target speed distribution">
  Target Speed distribution in the current matrix:
  <span class="spd-key spd-green">123 (97.6%) ≥ 99%</span>
  <span class="spd-key spd-yellow">3 (2.4%) ≥ 95%</span>
  <span class="spd-key spd-orange">0 (0.0%) ≥ 90%</span>
  <span class="spd-key spd-red">0 (0.0%) &lt; 90%</span>
  out of 126 timing-bearing rows. Every row now contributes to speed averages.
</p>

The allocator refresh changed the current risk profile: VISITOR3 high/low have
now moved out of red, and the latest VISITOR3 clean-relief stream-window work
widens high to an 80 KiB knee while moving low tide further into yellow with a
16 KiB slack-5 window plus a third retained setup segment extended to
`206..232` and frame `138` raw relocated into that paid gap. The current
VISITOR3 high pass moves frames `56` and `57` raw into the retained `228..262`
gap and caps tight speculative refills at `56 KiB`, improving high to
`1075/1043` without hidden-refill debt; the high-only clean-strip retune caps
clean slices at `64 KiB`, the clean-relief window retune widens high from
`68 KiB` to `80 KiB`, then the setup-edge pass pays early `40..47` and the
latest same-speed slide moves that edge to `42..49`, keeping `1070/1046`
while reducing loop reads/read time `5/61 -> 4/59`. The recent VISITOR3-high
speed pass retargets segment3 to `48..55`, switches high-tide clean capture to
six exact rects, and adds a three-VBlank phase ballast; the current canary row
lands at `1065/1046`, overrun `19`, blocking/refill `34/1`, with fixed pack
LBA and PS-EXE bucket.
BUILDING4 high is now green after the setup-segment pass, BUILDING2 high
now layers a one-VBlank phase retime over the small scheduler wins from the
`83..95`, guarded `271..287`, and `315..327` read
groups plus a same-speed entries `92`/`94`/`95` payload trim, WALKSTUF1
low now compacts active payload `752740 -> 708288`, retargets setup to
`179..283` plus `154..160`, adds a one-VBlank low-tide phase offset, raises
the low-tide window guard by one VBlank, and moves into green at
`1461/1447` / `99.042%`. WALKSTUF1 high now
keeps `198..244`, extends its second retained slice to `286..344`, adds
`{149,165}`, encodes frame `92` as a previous-frame D4 delta, uses
prepare-first scheduler ownership, carries same-speed `{395..411}` plus
retargeted `{411..423}` CD-pressure work, and now compacts entries `183..191`
with previous-visible cleanup to drop runtime restore bytes `509592 -> 500782`
and upload bytes `17182720 -> 17171200`; the latest entry `58..61` tail trim
plus phase-3 retime improves high to `1469/1440` / `98.026%` while keeping
BUILDING2 high and VISITOR3 high/low exact-flat. The `62..66` screen
clip plus `{92..108}`/`{272..284}` read-group promotion improved high to
`1469/1441` / `98.094%`, cuts overrun `29 -> 28`, blocking/refill
`42/12 -> 41/11`, reads/due `41/7 -> 37/6`, and reduces runtime
rows/spans/pixels to `16547/120919/658340`; the prior `189..191`
direct-stage headroom pass moves W1-high to `1470/1442` / `98.095%` and
blocking/refill `40/10`. The prior `372..388` fresh-owner retarget then
moved W1-high again to `1468/1442` / `98.229%`, cut overrun `28 -> 26`,
blocking `40 -> 39`, and loop reads `40 -> 39` while BUILDING2 high and
VISITOR3 high/low remained exact-flat under canonical boot variants. The
prior late-layout/owner refinement physically compacts frames `183..199` and
narrows owner coverage to `183..187` / `372..384`; speed remains `1468/1442`,
but hidden refill improves `10 -> 9`, with B2/V3 exact-flat. The prior
W1-high speed pass adds a high-tide `383..399` transient setup slice and caps
W1-high clean capture at `64 KiB`, improving W1-high to `1467/1442` /
`98.296%`, overrun `25`, blocking/refill `37/8`, and reads/due `35/6`.
The prior direct-stage rescue extends W1-high direct staging to `185..191`,
improving W1-high to `1467/1443` / `98.364%`, overrun `24`, while holding
blocking/refill at `37/8` and due misses at `6`.
The current prep2 frame-gate promotion allows two-VBlank staged-frame prepare
only for high-tide W1 displayed frames `128..191`, improving W1-high to
`1466/1445` / `98.568%`, overrun `21`, blocking/refill `35/5`, and reads/due
`36/6`.
The prior VISITOR3-low entry `109..112` fixed-layout clip keeps the
five-yellow canary exact-flat while shrinking selected active payload
`8170 -> 6004`; the broad full-pack clip and paired `{46..58}` read row are
closed as phase-negative. The prior VISITOR3-low `88..104` read-group row stacks on `16..32` and `72..88` and
moves VISITOR3-low `1343/1070/1039 -> 1342/1069/1039`, cuts overrun `31 -> 30`,
blocking `70 -> 68`, loop reads `15 -> 14`, and read time `93 -> 91` with due
misses flat at `11`; the prior frame135 gap-D4 row kept
file size, LBA, sectors, and the PS-EXE bucket fixed while improving
VISITOR3-low `1071/1039 -> 1070/1039`, overrun `32 -> 31`, blocking
`80 -> 78`, and loop read time `106 -> 104`; the prior
previous-visible cleanup row already dropped runtime restore bytes
`467962 -> 64632`, max restore bytes `293544 -> 2962`, upload bytes
`18113920 -> 17494400`, and dirty rows `28303 -> 27335`. The latest
VISITOR3-low frame134 D4 headroom pass shrinks that terminal payload
`17001 -> 14202` bytes while keeping the four-yellow canary exact-flat; frame
`136` D4 is closed as phase-negative.
BUILDING2 low now primes relative sectors `112..128` and `226..262` during
setup with a low-only `80 KiB` clean-strip cap, slack-5 window guard, and
`{141,153}` read row, moving that row into green. BUILDING4 low now
uses a gap-8 dirty-upload band merge retune plus a `24 KiB` stream window to
cut its active row to `2847/2820` and move green.
That is the new allocator baseline rather than a visual
regression: the R34 allocator matrix still records `126/126` PASS with 0 BSODs.
The remaining performance work should target WALKSTUF1 high first, then
VISITOR3 high/low data-shape or scheduler ownership that does not spend
blocking/refill debt.

All 126 rows now carry active-loop timing. [`SUZY 1`]({{ '/scenes/suzy1/' | relative_url }})
needs a longer `12000`-frame matrix budget because its valid
[scene-end]({{ '/docs/glossary/#scene-end' | relative_url }}) lands after the
default `7200`-frame timing window.

## Rollup

Current battle-card rollup as of <time datetime="2026-05-23">2026-05-23</time>:

| Metric | Value |
|---|---:|
| Scenes visually validated | `{{ validated_count }} / {{ total_count }}` (`100%`) |
| Validated scenes | all 63 original scenes; see the [live ledger]({{ '/scenes/' | relative_url }}) for the source rows |
| Scene/tide variants routed through headless perf | `126 / 126` (`100%`) |
| Timing-bearing variants | `126 / 126` (`100%`) |
| Scenes with at least one active-loop timed variant | `63 / 63` (`100%`) |
| Scenes with both high/low variants measured | `63 / 63` (`100%`) |
| Pending variants | `0 / 126` (`0%`) |
| Blocked variants | `0 / 126` (`0%`) |
| Timing-bearing average over target | `+0.2%` (`0.2028%` exact, public-capped) |
| Timing-bearing average target speed | `99.8%` (`99.7992%` exact, public-capped) |
| Latest perf matrix run | full allocator matrix `2026-05-16T11:29:21`; W1-high `200..215` previous-visible cleanup under-yellow canary `2026-05-23T05:18:30`; prior VISITOR3-low phase1 / segment4 `38..79` under-yellow canary `2026-05-23T04:29:47`; prior W1-high prep2 frame-gate under-yellow canary `2026-05-23T02:30:44`; prior W1-high direct `185..191` under-yellow canary `2026-05-23T00:56:07`; prior W1-high `383..399` transient setup-slice canary `2026-05-23T00:19:21`; prior W1-high `183..199` late-layout / `372..384` owner canary `2026-05-22T23:24:32`; prior W1-high `372..388` fresh-owner retarget canonical four-row gate `2026-05-22T21:44:58`; prior W1-high frames `189..191` direct-stage four-row gate `2026-05-22T20:31:20`; prior W1-high focused direct-stage proof `2026-05-22T20:14:39`; prior W1-high entry134 screen-clip headroom gate `2026-05-22T18:34:50`; prior W1-high `{108..124}` same-speed CD-pressure gate `2026-05-22T18:12:37`; prior W1-high `62..66` clip plus `{92..108}`/`{272..284}` speed gate `2026-05-22T17:37:49`; prior W1-high no-`144` mid-cluster clip headroom gate `2026-05-22T17:07:57`; prior W1-high frame138 clip headroom gate `2026-05-22T16:09:20`; prior W1-high active-loop clip headroom gate `2026-05-22T15:49:45` |
| Stats version | full allocator refresh stamped `git:2b617cbc`; current under-yellow rows use `w1high-prevvis200-215-postv3`; prior VISITOR3-low used `v3low-phase1-seg4-38-79`; prior W1-high prep2 rows used `w1high-prep2-frame128-191`; prior W1-high `185..191` direct row used `w1high-direct185-191`; prior W1-high setup-slice row used `w1high-seg4-383-399-cap64`; prior W1-high owner row used `w1high-owner372-388-fresh-owner`; prior W1-high late direct-stage row used `w1high-direct-late-189-191-headroom`; prior W1-high headroom row used `w1high-clip134-headroom`; prior B2-high compact rows used `b2high-compact-current`; prior VISITOR3-low additive setup rows used `v3low-seg4-add55-79-cache-phase0`; prior VISITOR3-low frame134 D4 rows used `v3low-d4-frame134-headroom`; prior VISITOR3-low phase-retime rows used `v3low-phase1`; prior W1-low compact trim/retarget rows used `w1low-trim-main179-phase1`; prior B2-high setup-alias source/data work used `b2high-alias38`; BUILDING4 high remains stamped `git:391a265e1+building4-high-setupseg264-288`; BUILDING4 low remains stamped `git:0faf443b9b+building4-low-window24`; per-row version is in the [`Stats Version` column below](#reading-the-table). |
| FISHING 1 canary | high `1068 / 1073 VBlanks`, low `1067 / 1074 VBlanks`, both public-capped at `100.0%` target speed |

Current JOHNNY1 payload/speed track: `johnny1-local-lz-v932` compresses
full-frame entries `1` and `50` inside both fixed-footprint JOHNNY1 packs,
preserving pack LBA/sectors and the `217088` byte PS-EXE bucket while cutting
active payload `316608 -> 112093`. Both tides are now green at `1948/1945`,
overrun `3`, blocking/refill `5`, read time `37`, due `0`, and target speed
`99.85%`.

Latest WALKSTUF1-high cleanup-headroom track:
`w1high-prevvis200-215-postv3` compacts late frames `200..215` with
previous-visible cleanup while preserving pixels, pack footprint, LBA, sectors,
and timing. The under-yellow canary keeps W1-high exact-flat at
`1815/1466/1445`, overrun `21`, blocking/refill `35/5`, and reads/due `36/6`,
while active payload drops `833386 -> 829510`, cleanup spans/pixels/restore
bytes drop `1445/14728/29456 -> 224/893/1786`, and runtime dirty rows drop
`28345 -> 28303`. VISITOR3 high/low stay exact-flat in the same gate.

Prior VISITOR3-low phase/segment track:
`v3low-phase1-seg4-38-79` combines a one-VBlank low-tide phase retime with a
widened setup segment `38..79`. The under-yellow canary preserves VISITOR3-high
and W1-high while VISITOR3-low improves `1380/1063/1043 -> 1386/1062/1045`,
overrun `20 -> 17`, blocking/refill `46/1 -> 38/0`, reads `17 -> 12`, read
time `89 -> 72`, due misses `8 -> 6`, and target speed `98.119% -> 98.399%`.

Prior WALKSTUF1-high prep2 frame-gate track:
`w1high-prep2-frame128-191` allows two-VBlank staged-frame prepare only for
high-tide W1 displayed frames `128..191`. The under-yellow canary keeps
VISITOR3 high/low valid while W1-high improves `1816/1467/1443 ->
1815/1466/1445`, overrun `24 -> 21`, blocking/refill `37/8 -> 35/5`,
reads `37 -> 36`, due misses stay `6`, and target speed improves
`98.364% -> 98.568%`.

Prior WALKSTUF1-high direct-stage track:
`w1high-direct185-191` extends high-tide direct staging to frames `185..191`
after the `383..399` setup slice made that cadence safe, improving W1-high
`1816/1467/1442 -> 1816/1467/1443`, overrun `25 -> 24`, while holding
blocking/refill at `37/8` and due misses at `6`.

Prior WALKSTUF1-high setup-slice track:
`w1high-seg4-383-399-cap64` adds a high-tide `383..399` transient setup
slice and caps W1-high clean-rect capture at `64 KiB`, improving W1-high
`1807/1468/1442 -> 1816/1467/1442`, overrun `26 -> 25`, blocking `39 -> 37`,
refill `9 -> 8`, and reads `40 -> 35`.

Prior WALKSTUF1-high fresh-owner retarget track:
`w1high-owner372-388-fresh-owner` retargeted high-tide owner frames `183..188`
to the `372..388` sector window before the hot refill path committed its read
window. It improved W1-high `1809/1470/1442 -> 1807/1468/1442`, overrun
`28 -> 26`, blocking `40 -> 39`, reads `40 -> 39`, and target speed
`98.0952% -> 98.2289%`.

Prior WALKSTUF1-high screen-clip headroom track:
`w1high-clip134-headroom` clips entry `134` / source frame `242` while
preserving the pack footprint, entry sizes, offsets, LBA/sectors, and PS-EXE
bucket. W1-high stays exact-flat at `1808/1469/1441`, overrun `28`,
blocking/refill `41/11`, reads/due `36/6`, while selected payload drops
`4397 -> 1938`, runtime rows/spans/pixels improve
`16547/120919/658340 -> 16540/120651/654904`, restore bytes improve
`458142 -> 457128`, and upload bytes improve `17175040 -> 17172480`.
BUILDING2 high and VISITOR3 high/low stay exact-flat.

Prior WALKSTUF1-high CD-pressure track:
`w1high-rg108-124` adds the high-tide `{108..124}` read group on top of the
accepted `62..66` clip and `{92..108}`/`{272..284}` stack. It preserves
W1-high timing at `1808/1469/1441`, overrun `28`, blocking/refill `41/11`,
and due `6`, while improving loop reads/read time `37/192 -> 36/191`, total
reads/read VBlanks `52/422 -> 51/421`, hidden reads/VBlanks `31/151 -> 30/150`,
and upload calls/rects/bytes `214/684/17207040 -> 213/682/17175040`.
BUILDING2 high and VISITOR3 high/low stay exact-flat.

Prior WALKSTUF1-high speed track:
`w1high-clip62-66-rg92-108-rg272-284` clips `WALKSTUF1.FG2` entries `62..66`
and adds the high-tide `{92..108}` plus `{272..284}` read groups on top of the
prior no-`144` mid-cluster, entry `138`, active-loop, early offscreen, and
entry `58..61` tail/phase promotions. It preserves the `1535263` byte pack
footprint, LBA `24891`, sectors `750`, table entry sizes, and the `233472`
byte PS-EXE bucket. The latest clip drops selected logical payload
`23002 -> 19866`, removes `3290` cleanup plus `1904` draw pixels, and improves
W1-high `1808/1469/1440 -> 1808/1469/1441`, overrun `29 -> 28`,
blocking/refill `42/12 -> 41/11`, reads/due `41/7 -> 37/6`, and target speed
`98.026% -> 98.094%`. BUILDING2 high and VISITOR3 high/low stay exact-flat
under canonical boot variants.

Prior BUILDING2-high compact speed track:
`b2high-compact-current` physically compacts `BUILDING2.FG2`, pads back to
the original `1303332` byte footprint, and keeps LBA `6189`, sectors `637`,
and the `233472` byte PS-EXE bucket fixed. The compacted layout trims entries
`76`/`77`, drops active payload `527884 -> 520974`, and passes the strict
four-yellow pixel/timing gate. B2-high improves `1583/1340/1314 ->
1570/1330/1317`, overrun `26 -> 13`, blocking/refill `45/12 -> 32/12`,
reads/due `42/7 -> 33/4`, and is now green at `99.023%` target speed.

Latest VISITOR3-low setup-residency track:
`v3low-seg4-add55-79-cache-phase0` keeps the accepted low-tide retained
setup slices `281..305`, `150..177`, and `206..232`, then adds a fourth
VISITOR3-low-only setup slice for sectors `55..79` in the CACHE region. The
source phase returns to `0` so timing stays exact-flat while loop reads drop
`28 -> 18`, visible blocking drops `55 -> 50`, and due misses drop `10 -> 8`.
The focused VISITOR3-low proof and four-yellow canary keep pack LBA `23379`,
the `233472` byte PS-EXE bucket, and all four under-green rows pixel/timing
valid; phase `2` remains rejected despite a speed signal because it introduces
one hidden prefetch overrun.

Prior VISITOR3-low frame134 D4 data-shape track:
`v3low-d4-frame134-headroom` encodes frame `134` against the previous decoded
frame and marks that frame for previous-delta runtime decode. The payload
shrinks `17001 -> 14202` bytes, while file size, pack LBA `23379`, sectors,
and the `233472` byte PS-EXE bucket stay fixed. The four-yellow canary keeps
VISITOR3 low exact-flat at `1339/1065/1041`, overrun `24`, blocking/refill
`55/0`, reads/due `28/10`; VISITOR3 high, WALKSTUF1 high, and BUILDING2 high
also stay exact-flat. Frame `136` alone and combined `134+136` are rejected
because they regress VISITOR3-low timing to `1068/1041`, overrun `27`,
blocking `59`, reads/due `29/11`.

Prior VISITOR3-low phase-retime speed track:
`v3low-phase1` adds one low-tide phase VBlank after the accepted
four-VBlank slack-knee baseline while preserving pack LBA `23379`, the
`233472` byte PS-EXE bucket, and the accepted `16..32`, `72..88`, and
`88..104` retained read-group shape. The four-yellow canary improves VISITOR3
low from `1338/1065/1040` to `1339/1065/1041`, cutting overrun `25 -> 24`
and moving target speed `97.653% -> 97.746%`. VISITOR3 high, WALKSTUF1 high,
and BUILDING2 high stay exact-flat. Phase `2`, `3`, and `4` are rejected
because they regress loop, target, blocking, or hidden refill.

Prior WALKSTUF1-low compact trim/retarget phase track:
`w1low-trim-main179-phase1` compacts `WALK1LOW.FG2` while preserving file
size, LBA `25641`, sectors `750`, and the `233472` byte PS-EXE bucket. Active
payload drops `752740 -> 708288`; setup coverage moves to `179..283` plus
`154..160`; W1-low gets one low-tide phase VBlank and a one-VBlank low-tide
window-slack guard. The accepted canary improves W1-low from
`1809/1470/1446` to `1801/1461/1447`, cutting overrun `24 -> 14`,
blocking/refill `32/3 -> 31/2`, reads `24 -> 22`, and moving target speed
`98.367% -> 99.042%`. VISITOR3 high/low, WALKSTUF1 high, and BUILDING2 high
stayed exact-flat; that checkpoint moved the public distribution to
`122` green / `4` yellow.

Current perf-reporting code-headroom track:
`perf-nolegacy-headroom-four-yellow-current-20260522` makes `JCPERF2` the
default scene-end summary, compile-gates legacy `JCPERF` behind
`PS1_PERF_LEGACY_TRACE=1`, and scopes `ps1PerfEndScene()` to `-Os`. The
four-yellow canary keeps VISITOR3 low/high, WALKSTUF1 high, and BUILDING2 high
exact-flat with stable pack LBAs and the `233472` byte PS-EXE bucket, while
`ps1PerfEndScene` shrinks to `0xf4` bytes. This is code/layout headroom, not a
VBlank speed win, so the public speed rollup remains unchanged.

Prior WALKSTUF1-high phase-retime track:
`w1high-phase1-five-yellow-current` adds one W1-high-only VBlank before the
measured loop. It keeps all pack LBAs and the `233472` byte PS-EXE bucket fixed
while improving W1-high from `1808/1472/1441` to `1808/1471/1441`, cutting
overrun `31 -> 30` and moving target speed `97.894% -> 97.961%`; blocking/refill,
reads, and due stay `43/13`, `41`, and `7`. VISITOR3 high/low, BUILDING2 high,
and WALKSTUF1 low stay exact-flat. Phase 2 and low-tide phase 1 are closed as
regressions.

Prior BUILDING2-high fixed-layout trim track:
`b2high-entry89-91-trim-five-yellow-current` trims draw tails for entries
`89..91` without moving offsets, file size, LBA/sectors, or the `233472` byte
PS-EXE bucket. It keeps BUILDING2 high exact-flat at `1583/1340/1314`, overrun
`26`, blocking/refill `45/12`, and due `7`, while active payload drops
`539990 -> 527960` and loop reads/read time improve `44/192 -> 42/186`.
VISITOR3 high/low and WALKSTUF1 high/low stay exact-flat.

Prior BUILDING2-high phase-retime track:
`b2high-phase1-five-yellow-noreq-current` adds one B2-high-only VBlank before
the measured loop. It keeps `BUILDING2.FG2` at LBA `6189`, keeps the `233472`
byte PS-EXE bucket fixed, and improves BUILDING2 high from `1583/1341/1313` to
`1583/1340/1314`, cutting overrun `28 -> 26`, blocking/refill
`47/14 -> 45/12`, and read time `194 -> 192`. VISITOR3 high/low and WALKSTUF1
high/low stay exact-flat.

Prior VISITOR3-high retained-segment/exact-clean track:
`v3high-seg48-55-exact-clean-phase3-five-yellow-current` retargets the
high-tide setup segment3 from relative sectors `42..49` to `48..55`, captures
the clean background with six exact visible rects, and adds a V3-high-only
three-VBlank phase ballast before the measured loop. It keeps `VISITOR3.FG2`
at LBA `22619`, keeps the `233472` byte PS-EXE bucket fixed, and improves
VISITOR3 high from `1372/1082/1045` to `1358/1067/1045`, cutting overrun
`37 -> 22`, blocking `34 -> 32`, and scene time by `14` VBlanks. The other
four under-green canary rows stay exact-flat.

Prior W1-high fixed-layout cleanup track:
`w1high-prev-visible-183-191-five-yellow-current` compacts entries `183..191`
in `WALKSTUF1.FG2` with previous-visible cleanup while preserving file size,
offsets, LBA `24891`, sectors `750`, and the `233472` byte PS-EXE bucket. The
selected active payload drops `844162 -> 840654`, cleanup spans
`2211 -> 885`, cleanup pixels `6670 -> 2265`, runtime restore bytes
`509592 -> 500782`, and upload bytes `17182720 -> 17171200`; BUILDING2 high,
VISITOR3 high/low, and WALKSTUF1 high/low stay exact-flat. The public speed
rollup remains unchanged.

Prior VISITOR3-low fixed-layout canonicalization track:
`v3low-entry109-112-clip-five-yellow-current` clips entries `109..112`
in `VIST3LOW.FG2` while preserving file size, offsets, LBA `23379`, sectors
`760`, and the `233472` byte PS-EXE bucket. The selected active payload
shrinks `8170 -> 6004` bytes; BUILDING2 high, VISITOR3 high/low, and
WALKSTUF1 high/low stay exact-flat. The broad full-pack clip and raw
`{46..58}` pair are logged closed as phase-negative. The public speed rollup
remains unchanged.

Prior W1-high fixed-layout screen-clip track:
`w1high-entry127-131-clip-five-yellow-current` clips entries `127..131`
in `WALKSTUF1.FG2` while preserving file size, offsets, LBA `24891`, sectors
`750`, and the `233472` byte PS-EXE bucket. The selected active payload
shrinks `24097 -> 21512` bytes, removing `625` offscreen cleanup pixels and
`3511` offscreen draw pixels; BUILDING2 high, VISITOR3 high/low, and
WALKSTUF1 high/low stay exact-flat. The public speed rollup remains unchanged.

Earlier W1-high fixed-layout screen-clip track:
`w1high-entry53-clip-five-yellow-current` clips only entry `53` /
source frame `63` in `WALKSTUF1.FG2` while preserving file size, offsets,
LBA `24891`, sectors `750`, and the `233472` byte PS-EXE bucket. That entry
shrinks `3893 -> 2309` bytes, removing `8969` offscreen cleanup pixels and
`1945` offscreen draw pixels; BUILDING2 high, VISITOR3 high/low, and
WALKSTUF1 high/low stay exact-flat. The public speed rollup remains unchanged.

Prior W1-low fixed-layout canonicalization track:
`w1low-entry90-99-clip-five-yellow-current` re-encodes entries `90`, `91`,
`92`, `94`, `95`, `96`, `98`, and `99` while preserving file size, offsets,
LBA `25641`, sectors `750`, and the `233472` byte PS-EXE bucket. The active
cluster shrinks `47579 -> 44511` bytes with no cleanup/draw pixel removal, and
W1-low total active payload drops `755808 -> 752740`; BUILDING2 high, VISITOR3
high/low, and WALKSTUF1 high/low stay exact-flat. The public speed rollup
remains unchanged.

Prior VISITOR3-high cleanup-headroom track:
`v3high-entry62-clean-only-five-yellow-current` rewrites only entry `62` /
source frame `80` with previous-visible cleanup while preserving pack
footprint, offsets, entry table sizes, LBA `22619`, sectors `760`, and the
`233472` byte PS-EXE bucket. Entry restore bytes drop `2724 -> 596`, runtime
restore bytes drop `56312 -> 54184`, and upload bytes drop
`18038400 -> 18012160`; BUILDING2 high, VISITOR3 high/low, and WALKSTUF1
high/low stay exact-flat. The public speed rollup remains unchanged.

Prior VISITOR3-low `88..104` read-group track:
`v3low-rg88-104-five-yellow-current` adds a low-tide-only VISITOR3 retained
read group for relative sectors `88..104` on top of `16..32` and `72..88`, building on the frame135 gap-D4
baseline while preserving pack footprint, LBA `23379`, sectors `760`, and the
`233472` byte PS-EXE bucket. VISITOR3-low moves
`1343/1070/1039 -> 1342/1069/1039`, cuts overrun `31 -> 30`, blocking
`70 -> 68`, loop reads `15 -> 14`, and read time `93 -> 91` with due misses
flat at `11`; BUILDING2 high, VISITOR3 high, and WALKSTUF1 high/low stay
exact-flat.

Prior VISITOR3-low frame135 gap-D4 track:
`v3low-delta135-gap-five-yellow-noreq-current` encodes frame `135` as a
previous-frame D4 delta against frame `134` and moves the `14246` byte payload
into the existing early D4 gap at offset `204452`, while preserving pack
footprint, LBA, sectors, and the PS-EXE bucket. It improved VISITOR3-low
`1071/1039 -> 1070/1039`, overrun `32 -> 31`, blocking `80 -> 78`, and loop
read time `106 -> 104`.

Prior W1-low fixed-layout cleanup-slack track:
`w1low-slack-frames87-99-five-yellow-current` combines exact-entry
screen-span clipping with previous-visible cleanup in `WALK1LOW.FG2` while
preserving each changed entry's table `dataSize`, offsets, file size, pack LBA,
and the `233472` byte PS-EXE bucket. W1-low stays exact-flat at `1470/1446`,
overrun `24`, blocking `32`, and refill `3`, while selected-entry active
payload drops `755808 -> 751288`, cleanup spans `3322 -> 1605`, cleanup pixels
`12700 -> 7209`, cleanup restore bytes `25400 -> 14418`, runtime restore bytes
`439090 -> 430052`, upload bytes `17854720 -> 17838080`, dirty rows
`27898 -> 27872`, and loop read time `146 -> 145`. BUILDING2 high, VISITOR3
high/low, and WALKSTUF1 high stay flat in the canonical five-yellow canary.
This banks restore/upload/refill headroom, not a VBlank speed win.

Prior W1-high cleanup-only fixed-layout screen-clip track:
`w1high-cleanup-only-14-147-five-yellow-current` clips exact cleanup-only
entries `14`, `23`, `43`, `45`, `47`, `49`, `51`, `54`, `56`, `57`, `67`,
`137`, and `147` in `WALKSTUF1.FG2` while preserving each changed entry's
table `dataSize`, offsets, file size, pack LBA, and the `233472` byte PS-EXE
bucket. W1-high stays exact-flat at `1472/1441`, overrun `31`, blocking `43`,
and refill `13`, while the changed subset drops logical active payload
`29339 -> 27643`, removes `18160` cleanup pixels with `0` draw pixels removed,
runtime restore bytes drop `525020 -> 510842`, max restore bytes drop
`12684 -> 7712`, upload bytes drop `17185920 -> 17182720`, and dirty rows drop
`26853 -> 26848`. This banks restore/upload headroom, not a VBlank speed win.

Prior VISITOR3-low fixed-layout cleanup track:
`visitor3-low-prev-visible-preserve-five-yellow-current` applies
previous-visible cleanup compaction to `VIST3LOW.FG2` while preserving every
changed entry's table `dataSize`, offsets, file size, and LBA. VISITOR3-low
stays exact-flat at `1071/1039`, overrun `32`, blocking `80`, and refill `0`,
while logical active payload drops `425729 -> 405835`, cleanup restore bytes
drop `456786 -> 53456`, runtime restore bytes drop `467962 -> 64632`, max
restore bytes drop `293544 -> 2962`, upload bytes drop
`18113920 -> 17494400`, and dirty rows drop `28303 -> 27335`. BUILDING2 high,
VISITOR3 high, and WALKSTUF1 high/low stay flat in the canonical five-yellow
canary. This banks render/restore/upload headroom, not a VBlank speed win.

Prior source-headroom track: `hot-scene-id-five-yellow-current` caches the
active foreground scene ID once per scene start and replaces repeated hot
scheduler scene-name compares with byte ID checks. The five-yellow canary stays
exact-flat, pack LBAs and the `233472` byte PS-EXE bucket stay fixed, and hot
tracked foreground symbols shrink (`foregroundPilotPlay -84`,
`fgRuntimeLoadSceneFrame -52`, `fgRuntimeFillWindowForEntry -24`,
`fgRuntimeTryPrefetchWindow -12`). This does not change the public speed
rollup; it banks code headroom for generated-owner and custom data-shape work.

Prior BUILDING2-high no-decode trim-draw-tail track:
`building2-high-trimdraw74-78-82-five-yellow-current` follows the `67`, `69`,
`70`, `71`, and `72` speed subset with a safe work-headroom subset on entries
`74` and `78..82`. The speed subset improved B2-high `1343/1312 -> 1341/1313`,
overrun `31 -> 28`, blocking/refill `50/17 -> 47/14`, and target speed
`97.692% -> 97.912%`; the follow-up keeps those timing metrics exact-flat
while preserving file size/LBA and dropping active payload `548293 -> 539990`.
VISITOR3 high/low and WALKSTUF1 high/low stay flat in the canonical
five-yellow canary.

Prior W1-high preserve-entry-size screen-clip track:
`w1high-screen-clip-preserve-entry-204-211-current` clips cleanup-only entries
`204..211` while keeping each changed entry's table `dataSize`, offset, file
size, and LBA unchanged. W1-high stays `1472/1441`, overrun `31`, blocking
`43`, and refill `13`; logical active payload for that subset drops
`20365 -> 19645`, with `12368` cleanup pixels removed. BUILDING2 high,
VISITOR3 high/low, and WALKSTUF1 low stay flat in the canonical five-yellow
canary.

Prior VISITOR3-low preserve-entry-size screen-clip track:
`v3low-screen-clip-preserve-entry-five-yellow-current` clips
screen-invisible cleanup/draw span work while keeping each changed entry's
table `dataSize`, offset, file size, and LBA unchanged. VISITOR3-low stays
`1071/1039`, overrun `32`, and blocking `80`; logical active payload drops
`425729 -> 423647`, with `38366` cleanup pixels and `2139` draw pixels
removed. BUILDING2 high, VISITOR3 high, and WALKSTUF1 high/low stay flat in
the canonical five-yellow canary.

Prior W1-low preserve-entry-size screen-clip track:
`w1low-screen-clip-preserve-entry-five-yellow-current` clips
screen-invisible cleanup/draw span work while keeping each changed entry's
table `dataSize`, offset, file size, and LBA unchanged. W1-low stays
`1470/1446`, overrun `24`, and blocking `32`, while hidden refill improves
`4 -> 3`; logical active payload drops `755808 -> 712808`, with `73798`
cleanup pixels and `39618` draw pixels removed. BUILDING2 high, VISITOR3
high/low, and WALKSTUF1 high stay flat in the canonical five-yellow canary.

Prior BUILDING2 high preserve-entry-size screen-clip track:
`b2high-screen-clip-preserve-entry-five-yellow-current` clips screen-invisible
cleanup/draw span work while keeping each changed entry's table `dataSize`,
offset, file size, and LBA unchanged. B2-high improves `1343/1311 -> 1343/1312`,
overrun `32 -> 31`, blocking `51 -> 50`, refill overrun `18 -> 17`, and target
speed `97.617% -> 97.692%`; logical active payload drops `574094 -> 520974`,
with `1982` cleanup pixels and `40166` draw pixels removed. VISITOR3 high/low
and WALKSTUF1 high/low stay flat in the canonical five-yellow canary.

Prior VISITOR3 high screen-clip headroom track:
`visitor3-screen-clip-five-yellow-current` clips offscreen cleanup spans after
the previous-visible cleanup baseline for entries `101` and `116` in
`VISITOR3.FG2`. VISITOR3 high stays `1082/1045`, overrun `37`, and blocking
`34`; active high-pack payload drops `437785 -> 436469`, `7393` offscreen
cleanup pixels are removed, and pack footprint/LBA stay fixed. VISITOR3 low,
BUILDING2 high, and WALKSTUF1 high/low stay flat in the canonical five-yellow
canary. This is work headroom, not a VBlank speed win.

Prior VISITOR3 high cleanup-headroom track:
`visitor3-prev-visible-minus62-promote-five-yellow` clips cleanup spans to the
previous visible frame across every timing-safe `VISITOR3.FG2` high-tide entry
while excluding isolated regressing entry `62`. VISITOR3 high stays
`1082/1045`, overrun `37`, and blocking `34`; active high-pack payload drops
`461631 -> 437785`, cleanup restore bytes drop `542088 -> 36092`, runtime
restore bytes drop `471382 -> 56312`, and upload bytes drop
`18785280 -> 18038400`. VISITOR3 low, BUILDING2 high, and WALKSTUF1 high/low
stay flat in the canonical five-yellow canary.

Prior BUILDING2 high cleanup-speed track:
`building2-high-prev-visible-cleanup-promote-five-yellow` clips cleanup spans
to the previous visible frame across `BUILDING2.FG2` while preserving pack
footprint and LBA. B2-high improves `1357/1307 -> 1343/1311`, overrun
`50 -> 32`, and target speed `96.315% -> 97.617%`; active high-pack payload
drops `663590 -> 574094`, cleanup restore bytes drop `439186 -> 80522`, runtime
restore bytes drop `438988 -> 116648`, and upload bytes drop
`26753280 -> 24341120`. VISITOR3 high/low and WALKSTUF1 high/low stay
exact-flat in the canonical five-yellow canary.

Prior W1-low cleanup-headroom track:
`walkstuf1-low-prev-visible-cleanup-late-subset` teaches the compaction tool to
preserve cleanup only where the previous parseable frame drew visible
foreground, then applies it to late low-tide frames `194`, `196`, `198`, `200`,
`202`, `206`, `208`, and `210`. The pack footprint and LBA stay fixed, active
low-pack payload drops `764658 -> 755808`, late cleanup spans drop
`2611 -> 173`, cleanup pixels drop `24946 -> 287`, and restore bytes drop
`49892 -> 574`; the refreshed five-yellow canary stays exact-flat.

Current W1 allocator-era speed track: targeted setup segments replace the
old full-scene setup buffers with CACHE slices that fit the new allocator.
High now keeps relative sectors `198..244` resident, retargets the second
slice from `411..435` to `286..344`, adds `{149,165}`, and encodes frame `92`
as a previous-frame D4 delta. The allocator-era row improves
`1475/1433 -> 1471/1440`, overrun `42 -> 31`, blocking `76 -> 57`,
prefetch overrun `15 -> 13`, and due `15 -> 10`; the D4 pass trades loop
reads/read time from `44/199` to `45/209` while still cutting visible loop
time and overrun. The newest frame56/source67 trim plus `{178..194}` keeps
that same `1471/1440` speed while reducing blocking `57 -> 56` and loop
reads/read time `45/209 -> 43/207`. The `{423..439}` and `{404..416}` rows
then reduce loop reads/read time to `41/200`; the high-tide
prepare-before-window scheduler ownership improves blocking/due `56/10 -> 43/7`
while keeping overrun flat at `31`; `{395..411}` banks the prior same-speed
read cut; and the newest `{411..423}` replacement keeps
timing exact-flat at `1472/1441` while reducing loop reads/read time
`42/201 -> 41/198`.
Low now replaces its old split `197..243` plus `410..434` slices
with one retained `238..344` CACHE setup segment after low-only 48 KiB
clean-rect chunking, adds the `{91,107}` first-boundary read group, and then
adds a separate TRANSIENT `344..350` setup edge. The follow-up row trims
frame132, adds `{378..390}`, then retargets setup to `244..350` with a split
`179..185` edge plus `{113..129}`, and adds `{355..371}` as same-speed
read-work. The accepted compact trim/retarget phase follow-up then lowers
active payload `752740 -> 708288`, slides setup coverage to `179..283` plus
`154..160`, adds one low-tide phase VBlank, and raises the low-tide window
guard by one VBlank. The combined current row improves
`1479/1435 -> 1461/1447`, overrun `44 -> 14`,
blocking/refill `65/18 -> 31/2`, loop reads/read time `50/230 -> 22/117`,
and moves W1-low into green at `99.042%` target speed.
W1-high remains yellow but now sits at `98.026%` after the entry `58..61`
tail/phase-3 speed pass.

Current B2-high allocator-era speed track: targeted CACHE slices at relative
sectors `3..35` and `202..242` keep the focused allocator run measured without
the old full-scene setup buffer, and the current scheduler pass replaces the
tail read group with `83..95` and adds guarded `271..287` plus `315..327`.
The visible-speed row improves active loop `1351 -> 1347`, overrun
`38 -> 34`, blocking `50 -> 41`, read time `207 -> 203`, and due `7 -> 6`;
the newest same-loop pressure row keeps `1347/1313`, overrun `34`, and refill
`16` flat while reducing blocking `41 -> 39`, loop reads `47 -> 45`, read time
`203 -> 199`, and due `6 -> 5`. The newest same-speed payload trim cuts
entries `92`, `94`, and `95` from `8834 -> 6370`, `8873 -> 6939`, and
`10247 -> 8827` bytes, reducing active payload `669408 -> 663590` with the
five-yellow canary exact-flat. The follow-up `{185..197}` row keeps timing
flat while reducing loop reads/read time first `45/199 -> 43/197` and then
`43/197 -> 40/189` with `{158..174}`. The current setup-alias cleanup points
duplicate entry `38` at setup-edge entry `35`. The latest previous-visible
cleanup promotion then improves B2-high to `1343/1311`, overrun `32`, and
target speed `97.617%`, while reducing active payload `663590 -> 574094`,
runtime restore bytes `438988 -> 116648`, and upload bytes
`26753280 -> 24341120`; pack/executable layout stays fixed. The current
preserve-entry-size screen-clip follow-up keeps that table size and LBA fixed
while improving B2-high again to `1343/1312`, overrun `31`, target speed
`97.692%`, and blocking/refill `50/17`. The latest no-decode trim-draw-tail
subset trims entries `67`, `69`, `70`, `71`, and `72`, improving B2-high again
to `1341/1313`, overrun `28`, target speed `97.912%`, and blocking/refill
`47/14`. The phase-1 retime then improves B2-high again to
`1340/1314`, overrun `26`, target speed `98.060%`, and blocking/refill
`45/12`. The current fixed-footprint physical compaction pass moves B2-high
into green at `1330/1317`, overrun `13`, target speed `99.023%`, and
blocking/refill `32/12`.

Current B2-low allocator-era speed track: targeted setup residency now primes
relative sectors `112..128` and `226..262` during setup, caps low clean strips
at `80 KiB`, raises the low window slack to `5`, and adds `{141,153}`. It
improves active loop/target `1336/1316 -> 1327/1318`, cuts overrun
`20 -> 9`, blocking `48 -> 47`, reads `35 -> 27`, and due `10 -> 9`, moving
the row into green while setup cost stays outside the active loop.

Current W1-high payload speed track: `walkstuf1-high-frame92-d4-current` rewrites
frame `92` as a `2056` byte previous-frame D4 payload instead of the original
`6035` byte frame. The focused row improves `1811/1475/1441 -> 1807/1471/1440`,
overrun `34 -> 31`, and target speed `97.695% -> 97.893%` with
blocking/refill flat at `57/13`; VISITOR3 high/low, BUILDING2 high/low,
WALKSTUF1 low, and BUILDING4 low stayed flat in the under-green canary.
The follow-up entry136 and entry57 preserve-offset trims remove another
`4255` bytes and two payload sectors from the hot pack while keeping the
five-yellow timing canary exact-flat at `1471/1440`, overrun `31`,
blocking/refill `57/13`, reads `45`, and due `10`. The newest frame56 trim
removes another `3101` bytes and one sector; paired with `{178..194}`, it
keeps timing flat while lowering blocking to `56` and reads to `43`. The
newest `{423..439}` and `{404..416}` rows keep the same `1471/1440` timing and
lower loop reads/read time again `43/207 -> 41/200`. The prepare-first
scheduler row trades both loop and target up by one VBlank to `1472/1441` while
holding overrun `31` and cutting blocking/due to `43/7`; the newest
same-speed `{411..423}` replacement keeps timing exact-flat and lowers loop
reads/read time `42/201 -> 41/198`. The latest standalone W1-high phase retime
then improves the current row to `1808/1471/1441`, overrun `30`, target speed
`97.961%`, with blocking/refill flat at `43/13`; the current entry `58..61`
tail trim plus phase `3` promotion then improves W1-high again to
`1808/1469/1440`, overrun `29`, target speed `98.026%`, and blocking/refill
`42/12`.

Current W1-low read-pressure speed track: `walkstuf1-low-rg355-371-current`
keeps the accepted frame132 payload baseline, `{378..390}` speed row, and
`244..350`/`179..185` setup retarget plus `{113..129}`, then adds
`{355..371}` as same-speed work-volume. It keeps file size/LBA/sectors fixed
and held target speed at `98.367%` with scene/loop/target `1809/1470/1446`,
overrun `24`, blocking/refill `32/4`, due `4`, and loop reads/read time
`24/146` after the fresh-owner `160..176` follow-up. The preserve-entry-size
screen-clip follow-up kept speed flat while improving hidden refill to `3`.
The latest compact trim/retarget phase promotion builds on that baseline and
moves W1-low to `1801/1461/1447`, overrun `14`, blocking/refill `31/2`,
reads/read time `22/117`, due `6`, and target speed `99.042%`.
The prior frame132 payload trim moved
`1473/1447 -> 1470/1445`; the `{378..390}` speed row moved
`1470/1445 -> 1470/1446`, overrun `25 -> 24`, blocking/refill
`35/7 -> 34/6`, and reads/read time `31/163 -> 30/159`. The current
entry65, entry39, entry55, entry56, entry59, entry63, entry66, entry85,
and previous-visible late-cleanup trims keep the five-yellow canary exact-flat
while reducing active low-pack payload `788773 -> 755808`. The broader
retargeted non-preserve trim was rejected
because hidden refill regressed `7 -> 10`.

Current B4-low dirty-upload speed track: the renderer now merges dirty upload
bands across clean gaps up to `8` rows instead of splitting every clean gap.
BUILDING4 low improves `2853/2816 -> 2849/2816`, overrun `37 -> 33`,
blocking/refill `42/35 -> 38/31`, and read time `223 -> 222`, while loop
reads/due stay flat at `30/1`. A gap-11 probe was rejected as a one-VBlank
regression against gap `8`. The follow-up stream-window retune narrows
B4 low from `32 KiB` to `24 KiB`, improving `2849/2816 -> 2847/2820`,
overrun `33 -> 27`, blocking/refill `38/31 -> 32/27`, and target speed
`98.842% -> 99.052%`, moving the row green; the `48 KiB` opposite probe was
rejected at `2895/2813`.

Current VISITOR3 allocator-era speed track: VISITOR3 still forces
clean-memory relief because its split clean rects and bg tiles leave too little
room for full setup-prime buffers. The current promotion keeps the tiny stage1
prefetch frame buffer for both tides, allows a high-tide 80 KiB stream
window, allows low tide to keep a 16 KiB clean-relief stream window behind
a slack-5 guard, trims high terminal overread, extends high setup segment2
through relative sector `229`, and relocates high frame `139`'s raw payload
into that paid setup gap, then adds a third high retained setup segment at
relative sectors `228..262`, then moves high frames `56` and `57` raw into
that retained gap with a `56 KiB` tight-refill cap, merges terminal setup
coverage into `203..262`, pays the early retained setup edge `40..47`, then
slides that edge to `42..49` as same-speed CD-pressure work.
High improves `1232/1033 -> 1070/1046`, overrun `199 -> 24`, blocking
`478 -> 35`, reads `137 -> 4`, and due `137 -> 2`; the latest high-only
follow-up caps clean strips at `64 KiB`, widens the clean-relief window from
`68 KiB` to `80 KiB`, adds the `40..47` setup edge, slides it to `42..49`, and the latest low-tide follow-up adds a third
retained setup segment for sectors `206..230`, then extends it to `206..232`
and relocates frame `138` raw into the paid gap, improving low
`1088/1038 -> 1065/1039`, overrun `50 -> 26`, blocking `104 -> 75`,
reads `21 -> 18`, and due `17 -> 14`. Both VISITOR3 rows are now yellow and
the orange band is cleared without reintroducing the allocator clean-rect BSOD.

Current BUILDING4 high speed track: the allocator-era setup-segment pass primes
relative sectors `264..288` during setup. It improves high tide
`2847/2816 -> 2843/2816`, overrun `31 -> 27`, blocking/refill
`36/32 -> 34/30`, loop reads/read time `49/256 -> 47/251`, and moves the row
into green at `99.05%` target speed. The paired B4-low and W1 high/low
canaries stayed exact-flat; the only accepted tradeoff is setup time
`3115 -> 3121`, under the 0.25% canary allowance.

Current BUILDING4-low payload track: `building4-low-local-lz-entry270-v971`
adds entry `270` / source frame `410` on top of the entry `30` and `33`
local-LZ sentinels. The new entry compresses `8287 -> 8167` bytes, modeled
sectors `5 -> 4`, while preserving the fixed `1714154` byte pack footprint,
pack LBA/sectors, and PS-EXE bucket. The material gate improves loop/target
`2853/2815 -> 2851/2815`, overrun `38 -> 36`, and refill `36 -> 35`, with
blocking/read time/due flat at `42/223/1`; active payload drops
`807263 -> 799277`. The current gap-8 dirty-upload band retune now carries the
public row to `2849/2816`, and the current `24 KiB` stream-window retune moves
it green at `2847/2820`.
`walkstuf1-low-late-offscreen-v653` clips only late-tail frames after broader
low clipping proved phase-negative. Low is exact-flat at
`1770`, `1478/1431`, blocking/refill `64/20`, reads/due `62/11`, while
dropping `39072` draw pixels, `4263` spans, `313` draw rows, `79` dirty rows,
and `50560` upload bytes. `walkstuf1-low-mid-offscreen-v665` then clips the
isolated mid offscreen cluster `133..139`, staying exact-flat while runtime
rows/spans/pixels drop `17298/135025/785455 -> 17292/134774/780557`.
`walkstuf1-low-left-offscreen-v666` extends the isolated subset to frames
`43..57`, still exact-flat, and drops rows/spans/pixels to
`16838/130637/737371`. `walkstuf1-low-left0-offscreen-v668` adds frames
`30..41`, again exact-flat, and drops rows/spans/pixels to
`16680/127950/712324`. `walkstuf1-low-pretail-offscreen-v669` clips the
separate pre-tail cluster `194..201`, still exact-flat, and drops rows/spans/pixels
to `16678/127061/703725`. `walkstuf1-low-midright-b-offscreen-v672` clips the
upper `93..101` split after the broader/lower splits regressed blocking, and
drops rows/spans/pixels to `16678/126676/702461`. `walkstuf1-low-preleft2-offscreen-v673`
clips frames `27..28`, still exact-flat, and drops rows/spans/pixels to
`16678/126554/700793`. `walkstuf1-low-postleft-offscreen-v674` clips frames
`75..77`, still exact-flat, and drops rows/spans/pixels to `16678/125926/698914`.
`walkstuf1-low-left2b-offscreen-v675` clips `66..74`, proving the old v667
`58..74` failure was not contiguous, and drops rows/spans/pixels to
`16667/122562/683390`.
`walkstuf1-low-left2e-offscreen-v678` clips frame `65`, still exact-flat, and
drops rows/spans/pixels to `16663/122035/679951`.
`walkstuf1-low-postleft-singletons-v680` clips frames `79`, `81`, and `83`,
still exact-flat, and drops rows/spans/pixels to `16663/121495/678413`.
`walkstuf1-low-midright-ad-offscreen-v684` clips frames `87..88`, still
exact-flat, and drops rows/spans/pixels to `16663/121341/677987`.
`walkstuf1-low-midright-ae-offscreen-v685` clips frames `89..92`, still
exact-flat, and drops rows/spans/pixels to `16663/121134/677418`.
`walkstuf1-low-midright-af-offscreen-v686` clips frame `85`, still exact-flat,
and drops rows/spans/pixels to `16663/121005/677030`.
`walkstuf1-low-frame1-offscreen-v687` clips frame `1`, still exact-flat, and
drops rows/spans/pixels to `16663/120887/676761`.
`walkstuf1-low-postmid106-112-offscreen-v688` clips frames `106..112`, still
exact-flat, and drops rows/spans/pixels to `16663/120783/676543`.
`walkstuf1-low-frame3-offscreen-v689` clips frame `3`, still exact-flat, and
drops rows/spans/pixels to `16663/120738/676450`.
`walkstuf1-low-frame140-offscreen-v690` clips frame `140`, still exact-flat,
and drops rows/spans/pixels to `16663/120710/676400`.
`walkstuf1-low-frame61-offscreen-v691` clips frame `61`, still exact-flat,
and drops rows/spans/pixels to `16584/120026/671905`.
`walkstuf1-low-frame60-offscreen-v692` clips frame `60`, still exact-flat,
and drops rows/spans/pixels to `16504/119364/667474`.
`walkstuf1-low-frame62-offscreen-v693` clips frame `62`, still exact-flat,
and drops rows/spans/pixels to `16446/118719/662964`.
`walkstuf1-low-frame59-offscreen-v694` clips frame `59`, still exact-flat,
and drops rows/spans/pixels to `16374/118086/658773`.
`walkstuf1-low-frame58-offscreen-v695` clips frame `58`, still exact-flat,
and drops rows/spans/pixels to `16302/117490/654606`.
`walkstuf1-low-frame63-offscreen-v696` clips frame `63`, still exact-flat,
and drops rows/spans/pixels to `16272/116912/650623`.
`walkstuf1-low-tail194-210compact-v705` then physically compacts only the
late-tail entries `194..210`, preserving the exact-flat `1478/1431` timing
while active payload drops `916139 -> 879801` and loop reads/read time improve
to `60/274` on the clean current baseline. `walkstuf1-low-frame133-offscreen-v716` clips one more
mid singleton exact-flat, removing `2937` pixels and dropping rows/spans/pixels
to `16268/116724/647686`. `walkstuf1-low-frame132-offscreen-v717` clips the
adjacent singleton exact-flat, removing another `2131` pixels and dropping
rows/spans/pixels to `16267/116582/645555`.
`walkstuf1-low-frame5-offscreen-v718` clips the next early singleton exact-flat,
removing another `1864` pixels and dropping rows/spans/pixels to
`16267/116230/643691`. `walkstuf1-low-frame141-offscreen-v719` clips the next
mid/late singleton exact-flat, removing another `1695` pixels and dropping
rows/spans/pixels to `16267/115872/641996`. `walkstuf1-low-frame131-offscreen-v720`
clips the adjacent mid singleton exact-flat, removing another `1673` pixels and
dropping rows/spans/pixels to `16266/115769/640323`.
`walkstuf1-low-frame19-offscreen-v721` clips the next early singleton exact-flat,
removing another `1320` pixels and dropping rows/spans/pixels to
`16257/115699/639003`. `walkstuf1-low-frame6-offscreen-v722` clips the adjacent
early singleton exact-flat, removing another `1317` pixels and dropping
rows/spans/pixels to `16257/115392/637686`. `walkstuf1-low-frame142-offscreen-v723`
clips the next mid singleton exact-flat, removing another `1097` pixels and
dropping rows/spans/pixels to `16257/115098/636589`.
`walkstuf1-low-frame130-offscreen-v724` clips the adjacent mid singleton
exact-flat, removing another `1015` pixels and dropping rows/spans/pixels to
`16257/115041/635574`. `walkstuf1-low-frame145-offscreen-v725` clips the next
late-mid singleton exact-flat, removing another `916` pixels and dropping
rows/spans/pixels to `16257/114854/634658`. `walkstuf1-low-frame129-offscreen-v726`
clips the final current singleton exact-flat, removing another `782` pixels and
dropping rows/spans/pixels to `16257/114798/633876`.
`walkstuf1-low-frame51-inplace-v747`,
`walkstuf1-low-frame49-inplace-v749`, and
`walkstuf1-low-frame47-inplace-v750`, and
`walkstuf1-low-frame61-inplace-v751`, and
`walkstuf1-low-frame62-inplace-v753`, and
`walkstuf1-low-frame58-inplace-v755`, and
`walkstuf1-low-frame45-inplace-v756`, and
`walkstuf1-low-frame37-inplace-v757`, and
`walkstuf1-low-frame35-inplace-v759` then shrink frames `51`, `49`, `47`,
`61`, `62`, `58`, `45`, `37`, and `35` in-place without moving offsets,
keeping timing exact-flat while active payload drops `879801 -> 844668`.
`walkstuf1-low-frame43-inplace-v762` extends that same no-shift lane with frame
`43`, keeping timing exact-flat while active payload drops again to `841324`.
`walkstuf1-low-frame41-inplace-v763` extends it with frame `41`, keeping timing
exact-flat while active payload drops again to `838090`.
`walkstuf1-low-frame57-inplace-v766` extends it with frame `57`, keeping timing
exact-flat while active payload drops again to `835001`.
`walkstuf1-low-frame33-inplace-v767` extends it with frame `33`, keeping timing
exact-flat while active payload drops again to `832375`.
`walkstuf1-low-frame67-inplace-v769` extends it with frame `67`, keeping timing
exact-flat while active payload drops again to `829912`.
`walkstuf1-low-frame68-inplace-v770` extends it with frame `68`, keeping timing
exact-flat while active payload drops again to `827726`.
`walkstuf1-low-frame69-inplace-v771` extends it with frame `69`, keeping timing
exact-flat while active payload drops again to `825696`.
`walkstuf1-low-frame32-inplace-v772` extends it with frame `32`, keeping timing
exact-flat while active payload drops again to `823715`.
`walkstuf1-low-frame133-inplace-v773` extends it with frame `133`, keeping timing
exact-flat while active payload drops again to `821773`.
`walkstuf1-low-frame5-inplace-v774` extends it with frame `5`, keeping timing
exact-flat while active payload drops again to `820008`.
`walkstuf1-low-frame141-inplace-v775` extends it with frame `141`, keeping
timing exact-flat while active payload drops again to `818315`.
`walkstuf1-low-frame70-inplace-v776` extends it with frame `70`, keeping
timing exact-flat while active payload drops again to `816655`.
`walkstuf1-low-frame30-inplace-v777` extends it with frame `30`, keeping
timing exact-flat while active payload drops again to `815013`.
`walkstuf1-low-frame6-inplace-v779` extends it with frame `6`, keeping timing
exact-flat while active payload drops again to `813628`.
`walkstuf1-low-frame71-inplace-v780` extends it with frame `71`, keeping
timing exact-flat while active payload drops again to `812253`.
`walkstuf1-low-frame72-inplace-v781` extends it with frame `72`, keeping
timing exact-flat while active payload drops again to `810990`.
`walkstuf1-low-frame142-inplace-v782` extends it with frame `142`, keeping
timing exact-flat while active payload drops again to `809745`.
`walkstuf1-low-frame73-inplace-v783` extends it with frame `73`, keeping
timing exact-flat while active payload drops again to `808613`.
`walkstuf1-low-frame131-inplace-v784` extends it with frame `131`, keeping
timing exact-flat while active payload drops again to `807506`.
`walkstuf1-low-frame74-inplace-v785` extends it with frame `74`, keeping
timing exact-flat while active payload drops again to `806495`.
`walkstuf1-low-frame19-inplace-v786` extends it with frame `19`, keeping
timing exact-flat while active payload drops again to `805515`.
`walkstuf1-low-frame28-inplace-v787` retests the old resident-hole frame
narrowly as a no-shift tail shrink, keeping timing exact-flat while active
payload drops again to `804592`.
`walkstuf1-low-frame138-inplace-v788` extends it with frame `138`, keeping
timing exact-flat while active payload drops again to `803680`.
`walkstuf1-low-frame145-inplace-v789` extends it with frame `145`, keeping
timing exact-flat while active payload drops again to `802785`.
`walkstuf1-low-frame75-inplace-v790` extends it with frame `75`, keeping timing
exact-flat while active payload drops again to `801903`.
`walkstuf1-low-frame76-inplace-v791` extends it with frame `76`, keeping timing
exact-flat while active payload drops again to `801103`.
`walkstuf1-low-frame77-inplace-v794` extends it with frame `77`, keeping timing
exact-flat while active payload drops again to `800374`.
`walkstuf1-low-frame130-inplace-v795` extends it with frame `130`, keeping
timing exact-flat while active payload drops again to `799694`.
`walkstuf1-low-frame135-inplace-v797` extends it with frame `135`, keeping
timing exact-flat while active payload drops again to `799129`.
`walkstuf1-low-frame1-inplace-v798` extends it with frame `1`, keeping timing
exact-flat while active payload drops again to `798712`.
`walkstuf1-low-frame88-inplace-v800` extends it with frame `88`, keeping timing
exact-flat while active payload drops again to `798435`.
`walkstuf1-low-frame90-inplace-v801` extends it with frame `90`, keeping timing
exact-flat while active payload drops again to `798232`.
`walkstuf1-low-frame3-inplace-v802` extends it with frame `3`, keeping timing
exact-flat while active payload drops again to `798079`.
`walkstuf1-low-frame53-inplace-v846` extends it with frame `53` after the
v817 retained-read row, keeping timing exact-flat while active payload drops
again to `796649`.
`walkstuf1-low-frame136-inplace-v847` extends it with frame `136`, keeping
timing exact-flat while active payload drops again to `795483`.
`walkstuf1-low-frame79-inplace-v849` extends it with frame `79`, keeping timing
exact-flat while active payload drops again to `794755`.
`walkstuf1-low-frame81-inplace-v852` extends it with frame `81`, keeping timing
exact-flat while active payload drops again to `794095`.
`walkstuf1-low-frame129-inplace-v853` extends it with frame `129`, keeping timing
exact-flat while active payload drops again to `793537`.
`walkstuf1-low-frame139-inplace-v855` extends it with frame `139`, keeping timing
exact-flat while active payload drops again to `793194`.
`walkstuf1-low-frame87-inplace-v859` extends it with frame `87`, improving
target to `1432`, overrun to `45`, and active payload to `792902`.
`walkstuf1-low-frame89-inplace-v860` extends it with frame `89`, keeping timing
exact-flat while active payload drops again to `792657`.
`walkstuf1-low-frame98-inplace-v861` extends it with frame `98`, keeping timing
exact-flat while active payload drops again to `792451`.
`walkstuf1-low-frame27-inplace-v862` extends it with frame `27`, keeping timing
exact-flat while active payload drops again to `792266`.
`walkstuf1-low-frame101-inplace-v863` extends it with frame `101`, keeping timing
exact-flat while active payload drops again to `792098`.
`walkstuf1-low-frame93-inplace-v864` extends it with frame `93`, keeping timing
exact-flat while active payload drops again to `791931`.
`walkstuf1-low-frame94-inplace-v865` extends it with frame `94`, keeping timing
exact-flat while active payload drops again to `791764`.
`walkstuf1-low-frame97-inplace-v866` extends it with frame `97`, keeping timing
exact-flat while active payload drops again to `791597`.
`walkstuf1-low-frame99-inplace-v867` extends it with frame `99`, keeping timing
exact-flat while active payload drops again to `791431`.
`walkstuf1-low-frame100-inplace-v868` extends it with frame `100`, keeping timing
exact-flat while active payload drops again to `791265`.
`walkstuf1-low-frame134-inplace-v869` extends it with frame `134`, keeping timing
exact-flat while active payload drops again to `791103`.
`walkstuf1-low-frame91-inplace-v870` extends it with frame `91`, keeping timing
exact-flat while active payload drops again to `790945`.
`walkstuf1-low-frame92-inplace-v871` extends it with frame `92`, keeping timing
exact-flat while active payload drops again to `790787`.
`walkstuf1-low-frame95-inplace-v872` extends it with frame `95`, keeping timing
exact-flat while active payload drops again to `790629`.
`walkstuf1-low-frame140-inplace-v873` extends it with frame `140`, keeping timing
exact-flat while active payload drops again to `790538`.
`walkstuf1-low-frame108-inplace-v874` extends it with frame `108`, keeping timing
exact-flat while active payload drops again to `790454`.
`walkstuf1-low-frame109-inplace-v875` extends it with frame `109`, keeping timing
exact-flat while active payload drops again to `790381`.
`walkstuf1-low-frame107-inplace-v876` extends it with frame `107`, keeping timing
exact-flat while active payload drops again to `790322`.
`walkstuf1-low-frame110-inplace-v908` extends it with frame `110`, keeping timing
exact-flat while active payload drops again to `790267`.
`walkstuf1-low-frame111-inplace-v909` extends it with frame `111`, keeping timing
exact-flat while active payload drops again to `790233`.
`walkstuf1-low-frame106-inplace-v910` extends it with frame `106`, keeping timing
exact-flat while active payload drops again to `790208`.
`walkstuf1-low-cd-fastpoll-v760` restores the post-release current baseline to
`1478/1431`, blocking/refill `64/20`, loop reads/read time `60/272`, and due
`11` while preserving the CD long-soak timeout fallback.
`walkstuf1-low-rg394-410-v817` adds the low-tide `394..410` retained read
group, improving the post-release row to `1769/1477/1431`, overrun `46`,
loop reads/read time `58/266`, while blocking/refill/due stay `64/20/11`.
The v859 fixed-sector frame87 trim then moves W1-low to `1769/1477/1432`,
overrun `45`, blocking/refill/due `65/20/11`, and loop read time `259`; the
v860/v861/v862/v863 frame89/frame98/frame27/frame101 trims keep those metrics exact-flat while reducing payload.
`walkstuf1-low-rg209-225-v935` refreshes the current source/control baseline
and adds the low-tide `209..225` retained-read row, keeping scene/loop flat at
`1773/1481` while improving target `1428 -> 1431`, overrun `53 -> 50`,
blocking `78 -> 72`, reads/read time `61/279 -> 58/267`, and due `11 -> 10`.
The allocator-era targeted setup checkpoint then narrows W1 setup residency to
CACHE slices instead of one full-scene setup buffer. High keeps `198..244`,
retargets the second slice from `411..435` to `286..344`, adds `{149,165}`,
and now encodes frame `92` as D4, improving the current row
`1475/1433 -> 1471/1440`, overrun `42 -> 31`, blocking `76 -> 57`, prefetch
overrun `15 -> 13`, and due `15 -> 10`; the current prepare-first scheduler
row plus `{395..411}` and retargeted `{411..423}`, followed by entry `58..61`
tail trim plus phase `3`, is `1469/1440`, overrun `29`,
blocking/refill/due `42/12/7`, and loop reads/read time `41/202`.
Low caches `197..243` and `410..434`, improving `1507/1426 -> 1477/1434`,
blocking/refill `142/26 -> 58/16`, reads/read time `75/353 -> 51/236`, and due
`25 -> 9`. The latest low retarget replaces those two slices with one
`238..344` retained setup segment after low-only 48 KiB clean-rect chunking,
then adds `{91,107}` as the first post-boundary row and a split TRANSIENT
`344..350` setup edge, improving the current row
`1479/1435 -> 1473/1447`, overrun `44 -> 26`, blocking/read time
`65/230 -> 41/186`, loop reads `50 -> 36`, and due `10 -> 5`. The frame132
payload trim and `{378..390}` read group then move the current row to
`1470/1446`, overrun `24`, blocking/read time `34/159`, loop reads `30`, and
due `4`; the latest `244..350`/`179..185` setup retarget plus `{113..129}`
keeps `1470/1446` while reducing blocking/read time to `33/150`, loop reads
to `26`, and hidden refill to `5`; the `{355..371}` follow-up keeps timing
flat and lowers loop reads/read time again to `24/146`. Larger full-scene
setup buffers and wider B2 second-segment probes crossed allocator
clean-pressure cliffs and were rejected.

Latest rejected W1 note: `walkstuf1-low-midright-ac-offscreen-v683` isolated
frame `86` from the old `85..92` mid-right miss. It removed only `319` pixels
and `105` spans, kept scene/loop flat at `1770/1478`, improved overrun
`47 -> 46`, but regressed blocking `64 -> 65`. Close frame `86` for direct
clipping; move to `87..88` or smaller non-risk candidates.

Latest promoted BUILDING2 note: allocator-safe targeted setup slices cache
relative sectors `3..35` and `202..242` in MEM_REGION_CACHE, then the
read-group passes replace the tail row with `83..95`, add `{158..174}`, guarded
`271..287`, `315..327`, and `{185..197}`, then the entries `92`/`94`/`95`
payload trim reduces active payload `669408 -> 663590`. The previous-visible
cleanup pass reduces active payload again to `574094`, the preserve-entry-size
screen-clip follow-up improves B2-high to `1343/1312`, and the no-decode
trim-tail subsets reduce active payload to `539990` while B2-high stays
`1341/1313`, overrun `28`, blocking/refill `47/14`, reads/read time `44/194`,
and due `7`, still avoiding the full-buffer clean-rect allocation failure.
BUILDING2 low keeps the v626 slack-8 `218..229` retained-read row, v660
offscreen draw-span clip, and v739 dead draw-tail trim, then primes relative
sectors `112..128` and `226..262` during setup with a low-only `80 KiB` clean
strip cap, slack-5 window guard, and `{141,153}`. It now measures
`1327/1318`, overrun `9`, blocking/refill `47/0`, reads `27`, and due `9`
with fixed pack LBA/sectors.

The durable numeric source is
[`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv).
The experiment trail is
[`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md).
The pack-time preprocessing target sheet is
[`docs/ps1/performance-preprocess-opportunities.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.csv).
The human signoff ledger is
[`docs/ps1/scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md).

Maintenance rule: every accepted perf optimization, rejected experiment worth
preserving, or new scene/tide timing run must update the CSV, the experiment
log, [README.md]({{ site.github_url }}/blob/main/README.md),
[`docs/ps1/TESTING.md`]({{ site.github_url }}/blob/main/docs/ps1/TESTING.md),
and this page.

## Reading the table

- **Scene**: clicking the scene slug in any row updates the URL with
  `#perf-{slug}-{tide}` so you can copy a stable link to that exact
  row — useful when filing a follow-up against one variant. Same
  anchors are linked from each scene page's "Performance battle card"
  line.
- **[Over Target]({{ '/docs/glossary/#over-target' | relative_url }})**: how far `loop_vb` is above the captured target timing. Public site values are capped at `0.0%` for faster-than-target rows so fast scenes do not create negative debt. Lower is better.
- **[Target Speed]({{ '/docs/glossary/#target-speed' | relative_url }})**: public display of `target_vb / loop_vb`, capped at `100%` so no row reports faster than intended playback. The raw signed ratio remains in the CSV for optimization analysis.
- **[VBlanks]({{ '/docs/glossary/#vblank' | relative_url }})**: `loop_vb/target_vb` — see the
  [target_vb / loop_vb glossary entry]({{ '/docs/glossary/#target-vb' | relative_url }}).
- **[Blocking]({{ '/docs/glossary/#blocking-vb' | relative_url }})**: visible CD/blocking VBlanks.
- **[Prefetch]({{ '/docs/glossary/#prefetch-hits' | relative_url }})**: prefetch overrun VBlanks.
- **Due**: due-frame misses.
- **Latest Run**: ISO timestamp derived from the headless summary path
  (`scratch/ps1-perf-iterate/YYYYMMDD-HHMMSS`); `-` means no current
  matrix run has been recorded for that variant.
- **Stats Version**: performance/layout version for that row. The current
  four-yellow canary rows use `v3low-seg4-add55-79-cache-phase0`;
  prior VISITOR3-low frame134 D4 rows used `v3low-d4-frame134-headroom`;
  prior VISITOR3-low phase-retime rows used `v3low-phase1`;
  prior W1-low compact trim/retarget rows used `w1low-trim-main179-phase1`;
  prior BUILDING2 high source/data work used `b2high-alias38`;
  BUILDING4 high remains on `git:391a265e1+building4-high-setupseg264-288`.
  Older rows retain their per-row version stamps,
  including `johnny1-local-lz-v932`,
  `building2-high-frame100-inplace-v926`,
  `building4-low-frame40-inplace-v924`,
  `building2-high-frame173-inplace-v914`,
  `building4-low-frame283-inplace-v913`,
  `building4-low-frame284-inplace-v912`,
  `building4-low-frame293-inplace-v911`,
  `building4-low-frame41-inplace-v903`,
  `building4-low-frame426-inplace-v895`,
  `building4-low-frame292-inplace-v893`,
  `building2-high-frame100-inplace-v926`,
  `building2-high-frame173-inplace-v914`,
  `building2-high-frame169-inplace-v896`,
  `building2-high-frame168-inplace-v892`,
  `building2-high-frame99-inplace-v891`,
  `building2-high-frame174-inplace-v890`,
  `building2-high-frame98-inplace-v889`,
  `building2-high-frame97-inplace-v888`,
  `building2-high-frame170-inplace-v887`,
  `building2-high-frame96-inplace-v880`,
  `building2-high-frame171-inplace-v879`,
  `building2-high-frame172-inplace-v877`,
  `walkstuf1-low-frame106-inplace-v910`,
  `walkstuf1-low-frame111-inplace-v909`,
  `walkstuf1-low-frame110-inplace-v908`,
  `walkstuf1-low-frame107-inplace-v876`,
  `walkstuf1-low-frame109-inplace-v875`,
  `walkstuf1-low-frame108-inplace-v874`,
  `walkstuf1-low-frame140-inplace-v873`,
  `walkstuf1-low-frame95-inplace-v872`,
  `walkstuf1-low-frame92-inplace-v871`,
  `walkstuf1-low-frame91-inplace-v870`,
  `walkstuf1-low-frame134-inplace-v869`,
  `walkstuf1-low-frame100-inplace-v868`,
  `walkstuf1-low-frame99-inplace-v867`,
  `walkstuf1-low-frame97-inplace-v866`,
  `walkstuf1-low-frame94-inplace-v865`,
  `walkstuf1-low-frame93-inplace-v864`,
  `walkstuf1-low-frame101-inplace-v863`,
  `walkstuf1-low-frame27-inplace-v862`,
  `walkstuf1-low-frame98-inplace-v861`,
  `walkstuf1-low-frame89-inplace-v860`,
  `walkstuf1-low-frame87-inplace-v859`,
  `walkstuf1-low-frame139-inplace-v855`,
  `walkstuf1-low-frame129-inplace-v853`,
  `walkstuf1-low-frame81-inplace-v852`,
  `walkstuf1-low-frame79-inplace-v849`,
  `walkstuf1-low-frame136-inplace-v847`,
  `walkstuf1-low-frame53-inplace-v846`,
  `walkstuf1-low-rg394-410-v817`,
  `building4-low-frame40-inplace-v924`,
  `building4-low-frame283-inplace-v913`,
  `building4-low-frame284-inplace-v912`,
  `building4-low-frame293-inplace-v911`,
  `building4-low-frame426-inplace-v895`,
  `building4-low-frame292-inplace-v893`,
  `building4-low-frame286-inplace-v827`,
  `building4-low-frame39-inplace-v816`,
  `building4-low-frame287-inplace-v815`,
  `building4-low-frame288-inplace-v814`,
  `building4-low-frame37-inplace-v813`,
  `building4-low-frame36-inplace-v811`,
  `building4-low-frame35-inplace-v808`,
  `building4-low-frame32-inplace-v807`,
  `building4-low-frame290-inplace-v806`,
  `building4-low-frame31-inplace-v805`,
  `building4-low-frame34-inplace-v804`,
  `walkstuf1-low-frame3-inplace-v802`,
  `walkstuf1-low-frame90-inplace-v801`,
  `walkstuf1-low-frame88-inplace-v800`,
  `walkstuf1-low-frame1-inplace-v798`,
  `walkstuf1-low-frame135-inplace-v797`,
  `walkstuf1-low-frame130-inplace-v795`,
  `walkstuf1-low-frame77-inplace-v794`,
  `walkstuf1-low-frame76-inplace-v791`,
  `walkstuf1-low-frame75-inplace-v790`,
  `walkstuf1-low-frame145-inplace-v789`,
  `walkstuf1-low-frame138-inplace-v788`,
  `walkstuf1-low-frame28-inplace-v787`,
  `walkstuf1-low-frame19-inplace-v786`,
  `walkstuf1-low-frame74-inplace-v785`,
  `walkstuf1-low-frame131-inplace-v784`,
  `walkstuf1-low-frame73-inplace-v783`,
  `walkstuf1-low-frame142-inplace-v782`,
  `walkstuf1-low-frame72-inplace-v781`,
  `walkstuf1-low-frame71-inplace-v780`,
  `walkstuf1-low-frame6-inplace-v779`,
  `walkstuf1-low-frame30-inplace-v777`,
  `walkstuf1-low-frame70-inplace-v776`,
  `walkstuf1-low-frame141-inplace-v775`,
  `walkstuf1-low-frame5-inplace-v774`,
  `walkstuf1-low-frame133-inplace-v773`,
  `walkstuf1-low-frame32-inplace-v772`,
  `walkstuf1-low-frame69-inplace-v771`,
  `walkstuf1-low-frame68-inplace-v770`,
  `walkstuf1-low-frame67-inplace-v769`,
  `walkstuf1-low-frame33-inplace-v767`,
  `walkstuf1-low-frame57-inplace-v766`,
  `walkstuf1-low-frame41-inplace-v763`,
  `walkstuf1-low-frame43-inplace-v762`,
  `walkstuf1-low-cd-fastpoll-v760`,
  `walkstuf1-low-frame35-inplace-v759`,
  `walkstuf1-low-frame37-inplace-v757`,
  `walkstuf1-low-frame45-inplace-v756`,
  `walkstuf1-low-frame58-inplace-v755`,
  `walkstuf1-low-frame62-inplace-v753`,
  `walkstuf1-low-frame61-inplace-v751`,
  `walkstuf1-low-frame47-inplace-v750`,
  `walkstuf1-low-frame49-inplace-v749`,
  `walkstuf1-low-frame51-inplace-v747`,
  `walkstuf1-low-frame129-offscreen-v726`,
  `walkstuf1-low-frame145-offscreen-v725`,
  `walkstuf1-low-frame130-offscreen-v724`,
  `walkstuf1-low-frame142-offscreen-v723`,
  `walkstuf1-low-frame6-offscreen-v722`,
  `walkstuf1-low-frame19-offscreen-v721`,
  `walkstuf1-low-frame131-offscreen-v720`,
  `walkstuf1-low-frame141-offscreen-v719`,
  `walkstuf1-low-frame5-offscreen-v718`,
  `walkstuf1-low-frame132-offscreen-v717`,
  `walkstuf1-low-frame133-offscreen-v716`,
  `walkstuf1-low-tail194-210compact-v705`,
  `walkstuf1-low-frame63-offscreen-v696`,
  `walkstuf1-low-frame58-offscreen-v695`,
  `walkstuf1-low-frame59-offscreen-v694`,
  `walkstuf1-low-frame62-offscreen-v693`,
  `walkstuf1-low-frame60-offscreen-v692`,
  `walkstuf1-low-frame61-offscreen-v691`,
  `walkstuf1-low-frame140-offscreen-v690`,
  `walkstuf1-low-frame3-offscreen-v689`,
  `walkstuf1-low-postmid106-112-offscreen-v688`,
  `walkstuf1-low-frame1-offscreen-v687`,
  `walkstuf1-low-midright-af-offscreen-v686`,
  `walkstuf1-low-midright-ae-offscreen-v685`,
  `walkstuf1-low-midright-ad-offscreen-v684`,
  `walkstuf1-low-postleft-singletons-v680`,
  `walkstuf1-low-left2e-offscreen-v678`,
  `walkstuf1-low-left2b-offscreen-v675`,
  `walkstuf1-low-postleft-offscreen-v674`,
  `walkstuf1-low-preleft2-offscreen-v673`,
  `walkstuf1-low-midright-b-offscreen-v672`,
  `walkstuf1-low-pretail-offscreen-v669`,
  `walkstuf1-low-left0-offscreen-v668`,
  `walkstuf1-low-left-offscreen-v666`,
  `walkstuf1-low-mid-offscreen-v665`,
  `building2-high-frame100-inplace-v926`,
  `building2-high-frame173-inplace-v914`,
  `building2-high-frame169-inplace-v896`,
  `building2-high-frame168-inplace-v892`,
  `building2-high-frame99-inplace-v891`,
  `building2-high-frame174-inplace-v890`,
  `building2-high-frame98-inplace-v889`,
  `building2-high-frame97-inplace-v888`,
  `building2-high-frame170-inplace-v887`,
  `building2-high-frame96-inplace-v880`,
  `building2-high-frame171-inplace-v879`,
  `building2-high-frame172-inplace-v877`,
  `building2-high-frame89-offscreen-v703`,
  `building2-high-frame90-offscreen-v702`,
  `building2-high-frame91-offscreen-v701`,
  `building2-high-frame92-offscreen-v700`,
  `building2-high-tail94-104-offscreen-v698`,
  `building2-high-late-offscreen-v664`,
  `building2-low-trimtails-v739`,
  `building2-low-offscreen-drawclip-v660`,
  `walkstuf1-high-frame139-inplace-v927`,
  `walkstuf1-high-frame135-inplace-v884`,
  `walkstuf1-high-frame138-inplace-v882`,
  `walkstuf1-high-frame43-inplace-v844`,
  `walkstuf1-high-frame45-inplace-v843`,
  `walkstuf1-high-frame47-inplace-v842`,
  `walkstuf1-high-frame49-inplace-v841`,
  `walkstuf1-high-frame51-inplace-v839`,
  `walkstuf1-high-frame139-offscreen-v837`,
  `walkstuf1-high-frame135-offscreen-v836`,
  `walkstuf1-high-frame136-offscreen-v835`,
  `walkstuf1-high-frame57-offscreen-v834`,
  `walkstuf1-high-frame56-offscreen-v833`,
  `walkstuf1-high-frame43-offscreen-v832`,
  `walkstuf1-high-frame45-offscreen-v831`,
  `walkstuf1-high-frame47-offscreen-v830`,
  `walkstuf1-high-frame49-offscreen-v734`,
  `walkstuf1-high-frame51-offscreen-v733`,
  `walkstuf1-high-frame138-offscreen-v732`,
  `walkstuf1-high-frame55-offscreen-v731`,
  `walkstuf1-high-tailcompact-v657`,
  `walkstuf1-low-late-offscreen-v653`,
  `building4-low-offscreen-drawclip-v652`,
  `walkstuf1-low-rg78-91-v474`,
  `visitor3-high-tail-pack-v629`,
  `visitor3-low-frame137-primegap-v510`,
  `walkstuf1-high-current-v458-refresh`,
  `building2-low-rg218-229-slack8-v626`,
  `building2-low-delta-v454`,
  `building2-high-rg206-230-cap24-v441`,
  `visitor3-low-tail-pack-only-v338`,
  `visitor3-low-swap-f128-f129-v327`,
  `visitor3-low-f128-resident-seg27-v302`,
  `visitor3-high-f131-resident-alias121123-v299`,
  `visitor3-low-alias-noop114117-v292`,
  `visitor3-high-f140-segment-copy-v291`,
  `visitor3-low-noop113-v249`,
  `visitor3-low-noop114117-v248`,
  `visitor3-high-f127-f130-resident-copy-v238`,
  `johnny1-compact-fgp3-v173`,
  `walkstuf3-low-compact-fgp3-v171`,
  `activity9-high-compact-fgp3-v167`,
  `building6-window-slack4-v364`,
  `walkstuf3-high-compact-fgp3-v163`,
  `building2-low-restore-window-slack4-v160`,
  `visitor5-high-rg30-46-v496`,
  `visitor5-low-rg30-46-v526`,
  `building1-compact-fgp3-noautoprime-v157`,
  `mary3-preserve-window-slack8-v149`,
  `missing-scenes-current-v001`,
  `visitor3-tail-trim-stageguard-v127`,
  `graphics-composite-os-v111`,
  `building2-low-group365-381-v110`,
  `building2-high-group60-72-v109`,
  `building2-high-restore-minus-current-v108`,
  `visitor3-low-offscreen-exitright-v106`,
  `visitor3-high-offscreen-drawclip-v105`,
  `walkstuf1-compact-fgp3-v141`,
  `visitor3-low-readgroup-prune-v088`,
  `building4-restore-minus-current-v087`,
  `visitor3-restore-minus-current-v086`,
  `visitor3-high-readgroup-prune-v084`,
  `compact-u16-inline-v083`,
  `fgp3v4-drawcompact-all-v082`,
  `activity9-dead-readgroup-prune-v082`,
  `read-group-selector-single-assign-v082`,
  `visitor3-high-remove-144-160-v082`,
  `johnny2-prefetch-relief-v081`,
  `activity9-low-fgp3-cleanup-compact-v081`,
  `activity9-current-v081-refresh`,
  `building4-fgp3-cleanup-compact-window-v081`,
  `building2-fgp3-cleanup-compact-v081`,
  `visitor3-fgp3-cleanup-compact-v081`,
  `activity9-lowgroup-v072c`,
  `activity9-fgp3-v072c`,
  `activity9-window-v072c`,
  `activity4-fishing4-v072c-prefetch-relief`,
  `johnny6-compact-fgp3-v354`,
  `activity1-v072c-current-refresh`,
  `mary2-prefetch-relief-v081`,
  `mary2-fgp3-padded-v081`,
  `johnny2-fgp3-padded-v081`,
  `mary5-fgp3-padded-v081`,
  `activity11-fgp3-padded-v081`,
  `activity11-12-v072c-prefetch-relief`,
  `stale-next-v072c-current-refresh`,
  `mary1-v072c-prefetch-relief`,
  `stale-layout-v072c-current-refresh`,
  `activity9-v072c-prefetch-relief`,
  `stale-pressure2-v072c-current-refresh`,
  `johnny1-v072c-prefetch-relief`,
  `stale-pressure-v072c-current-refresh`,
  `activity10-johnny3-v072-prefetch-relief`,
  `stale-zero2-v072b-current-refresh`,
  `stale-zero-v072b-current-refresh`,
  `stale-top-v072b-current-refresh`,
  `building2-low-rg218-229-slack8-v626`,
  `building2-low-delta-v454`,
  `visitor3-low-frame129-delta-v452`,
  `mismatch-top-v072-current-refresh`,
  `stand-family-v072-current-refresh`,
  `visitor4-v072-current-refresh`,
  `stand1-v072-current-refresh`,
  `visitor3-v072-prefetch-relief`,
  `walkstuf1-v072-prefetch-relief`,
  `fishing5-v065-current-ledger-overlay`,
  `compact-fgp3-v66-final-frame-hold`,
  `compact-fgp3-v64-building2-group318-330`,
  `compact-fgp3-v63-building2low-prime`, and
  `indexed8-row-local-dirty-v1`; other refreshed rows
  include `compact-fgp3-v62-fishing3low-group253-265`,
  `compact-fgp3-v61-fishing3low-group163-175`,
  `compact-fgp3-v60-visitor3high-group230-242`,
  `compact-fgp3-v59-visitor3high-group72-84`,
  `indexed8-tile-local-compose-v1`,
  `compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through
  `compact-fgp3-v29-smallprime`; the complete full-matrix baseline remains
  `compact-fgp3-v2-fullmatrix` for rows not rerun since that pass.

## 126-variant battle card

<p class="scene-perf-legend" aria-label="Target speed legend">
  Target Speed colour key:
  <span class="spd-key spd-green">≥ 99% (at target)</span>
  <span class="spd-key spd-yellow">≥ 95% (close)</span>
  <span class="spd-key spd-orange">≥ 90% (slipping)</span>
  <span class="spd-key spd-red">&lt; 90% (well below)</span>
  Rows without timing data show "—" and stay uncolored.
</p>

<table class="scene-perf-table">
  <caption class="visually-hidden">
    Headless performance matrix at {{ site.release.tag }}:
    one row per scene/tide variant, with last run, stats version,
    over target percentage, target speed percentage, VBlanks taken,
    blocking VBlanks, prefetch hits, due-misses, and notes.
    Click any column header to sort ascending; click again for
    descending. Target Speed cells are color-coded: ≥99% green,
    ≥95% yellow, ≥90% orange, &lt;90% red.
  </caption>
  <thead>
    <tr>
      <th scope="col">Scene</th>
      <th scope="col">Tide</th>
      <th scope="col">Status</th>
      <th scope="col">Latest Run</th>
      <th scope="col">Stats Version</th>
      <th scope="col">Over Target</th>
      <th scope="col">Target Speed</th>
      <th scope="col">VBlanks</th>
      <th scope="col">Blocking</th>
      <th scope="col">Prefetch</th>
      <th scope="col">Due</th>
      <th scope="col">Notes</th>
    </tr>
  </thead>
  <tbody>
    <tr id="perf-activity1-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity1-high"><code>activity1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2754/2764</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity1-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity1-low"><code>activity1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2754/2764</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity4-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity4-high"><code>activity4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1065/1065</td>
      <td>5</td>
      <td>5</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity4-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity4-low"><code>activity4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1064/1069</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity5-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity5-high"><code>activity5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1736/1747</td>
      <td>4</td>
      <td>4</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity5-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity5-low"><code>activity5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1735/1749</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity6-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity6-high"><code>activity6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>912/909</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity6-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity6-low"><code>activity6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>912/909</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity7-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity7-high"><code>activity7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>595/596</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity7-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity7-low"><code>activity7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>594/596</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity8-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity8-high"><code>activity8</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>898/905</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity8-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity8-low"><code>activity8</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>898/905</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity9-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity9-high"><code>activity9</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.8%</td>
      <td class="spd-green">99.2%</td>
      <td>2079/2062</td>
      <td>22</td>
      <td>13</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-activity9-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity9-low"><code>activity9</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.8%</td>
      <td class="spd-green">99.2%</td>
      <td>2077/2061</td>
      <td>19</td>
      <td>14</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-activity10-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity10-high"><code>activity10</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1257/1260</td>
      <td>7</td>
      <td>1</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-activity10-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity10-low"><code>activity10</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.2%</td>
      <td class="spd-green">99.8%</td>
      <td>1260/1257</td>
      <td>19</td>
      <td>9</td>
      <td>2</td>
      <td></td>
    </tr>
    <tr id="perf-activity11-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity11-high"><code>activity11</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1715/1722</td>
      <td>3</td>
      <td>3</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity11-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity11-low"><code>activity11</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1717/1722</td>
      <td>4</td>
      <td>4</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity12-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity12-high"><code>activity12</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1407/1415</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-activity12-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity12-low"><code>activity12</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1406/1411</td>
      <td>7</td>
      <td>4</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building1-high">
      <td><a class="scene-perf-rowlink" href="#perf-building1-high"><code>building1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.8%</td>
      <td class="spd-green">99.2%</td>
      <td>788/782</td>
      <td>17</td>
      <td>15</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building1-low">
      <td><a class="scene-perf-rowlink" href="#perf-building1-low"><code>building1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.4%</td>
      <td class="spd-green">99.6%</td>
      <td>785/782</td>
      <td>17</td>
      <td>11</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building2-high">
      <td><a class="scene-perf-rowlink" href="#perf-building2-high"><code>building2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-22T23:24:32</td>
      <td>w1high-layout-owner372-384</td>
      <td>1.0%</td>
      <td class="spd-green">99.0%</td>
      <td>1330/1317</td>
      <td>32</td>
      <td>12</td>
      <td>4</td>
      <td></td>
    </tr>
    <tr id="perf-building2-low">
      <td><a class="scene-perf-rowlink" href="#perf-building2-low"><code>building2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-19T01:09:53</td>
      <td>git:7680edc56a+visitor3-high-clean64</td>
      <td>0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>1327/1318</td>
      <td>47</td>
      <td>0</td>
      <td>9</td>
      <td></td>
    </tr>
    <tr id="perf-building3-high">
      <td><a class="scene-perf-rowlink" href="#perf-building3-high"><code>building3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>5460/5465</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-building3-low">
      <td><a class="scene-perf-rowlink" href="#perf-building3-low"><code>building3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>5460/5464</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-building4-high">
      <td><a class="scene-perf-rowlink" href="#perf-building4-high"><code>building4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-18T02:30:50</td>
      <td>git:391a265e1+building4-high-setupseg264-288</td>
      <td>1.0%</td>
      <td class="spd-green">99.1%</td>
      <td>2843/2816</td>
      <td>34</td>
      <td>30</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building4-low">
      <td><a class="scene-perf-rowlink" href="#perf-building4-low"><code>building4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-19T09:17:25</td>
      <td>git:0faf443b9b+building4-low-window24</td>
      <td>1.0%</td>
      <td class="spd-green">99.1%</td>
      <td>2847/2820</td>
      <td>32</td>
      <td>27</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building5-high">
      <td><a class="scene-perf-rowlink" href="#perf-building5-high"><code>building5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3344/3347</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-building5-low">
      <td><a class="scene-perf-rowlink" href="#perf-building5-low"><code>building5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3343/3348</td>
      <td>5</td>
      <td>5</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-building6-high">
      <td><a class="scene-perf-rowlink" href="#perf-building6-high"><code>building6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.9%</td>
      <td class="spd-green">99.2%</td>
      <td>2475/2454</td>
      <td>26</td>
      <td>21</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building6-low">
      <td><a class="scene-perf-rowlink" href="#perf-building6-low"><code>building6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>2472/2457</td>
      <td>22</td>
      <td>17</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building7-high">
      <td><a class="scene-perf-rowlink" href="#perf-building7-high"><code>building7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3130/3133</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-building7-low">
      <td><a class="scene-perf-rowlink" href="#perf-building7-low"><code>building7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3128/3133</td>
      <td>5</td>
      <td>5</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing1-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing1-high"><code>fishing1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1068/1073</td>
      <td>4</td>
      <td>4</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing1-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing1-low"><code>fishing1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1067/1074</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing2-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing2-high"><code>fishing2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1760/1764</td>
      <td>4</td>
      <td>4</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing2-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing2-low"><code>fishing2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1758/1765</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing3-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing3-high"><code>fishing3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.9%</td>
      <td class="spd-green">99.1%</td>
      <td>1964/1947</td>
      <td>30</td>
      <td>25</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-fishing3-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing3-low"><code>fishing3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.1%</td>
      <td class="spd-green">99.8%</td>
      <td>1957/1954</td>
      <td>11</td>
      <td>11</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing4-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing4-high"><code>fishing4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>836/842</td>
      <td>3</td>
      <td>3</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing4-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing4-low"><code>fishing4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>834/843</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing5-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing5-high"><code>fishing5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>885/889</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing5-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing5-low"><code>fishing5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>885/890</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing6-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing6-high"><code>fishing6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>744/752</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing6-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing6-low"><code>fishing6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>744/752</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing7-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing7-high"><code>fishing7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>715/725</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing7-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing7-low"><code>fishing7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>715/725</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing8-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing8-high"><code>fishing8</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1243/1253</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing8-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing8-low"><code>fishing8</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1243/1253</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny1-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny1-high"><code>johnny1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1945/1946</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny1-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny1-low"><code>johnny1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1945/1946</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny2-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny2-high"><code>johnny2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1741/1750</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny2-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny2-low"><code>johnny2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1743/1751</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny3-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny3-high"><code>johnny3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1160/1163</td>
      <td>7</td>
      <td>3</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-johnny3-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny3-low"><code>johnny3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1157/1166</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny4-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny4-high"><code>johnny4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1204/1214</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny4-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny4-low"><code>johnny4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1204/1214</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny5-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny5-high"><code>johnny5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>810/820</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny5-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny5-low"><code>johnny5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>810/820</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny6-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny6-high"><code>johnny6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>1.0%</td>
      <td class="spd-green">99.0%</td>
      <td>2830/2802</td>
      <td>25</td>
      <td>25</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny6-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny6-low"><code>johnny6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>1.0%</td>
      <td class="spd-green">99.0%</td>
      <td>2830/2802</td>
      <td>25</td>
      <td>25</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-mary1-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary1-high"><code>mary1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>4863/4829</td>
      <td>45</td>
      <td>35</td>
      <td>2</td>
      <td></td>
    </tr>
    <tr id="perf-mary1-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary1-low"><code>mary1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>4855/4840</td>
      <td>24</td>
      <td>17</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-mary2-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary2-high"><code>mary2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2241/2247</td>
      <td>3</td>
      <td>3</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-mary2-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary2-low"><code>mary2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2241/2247</td>
      <td>3</td>
      <td>3</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-mary3-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary3-high"><code>mary3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>2302/2294</td>
      <td>54</td>
      <td>0</td>
      <td>13</td>
      <td></td>
    </tr>
    <tr id="perf-mary3-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary3-low"><code>mary3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.2%</td>
      <td class="spd-green">99.8%</td>
      <td>2300/2295</td>
      <td>53</td>
      <td>0</td>
      <td>13</td>
      <td></td>
    </tr>
    <tr id="perf-mary4-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary4-high"><code>mary4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>2026/2019</td>
      <td>8</td>
      <td>5</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-mary4-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary4-low"><code>mary4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>2026/2019</td>
      <td>8</td>
      <td>5</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-mary5-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary5-high"><code>mary5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.1%</td>
      <td class="spd-green">99.9%</td>
      <td>1584/1582</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-mary5-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary5-low"><code>mary5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.1%</td>
      <td class="spd-green">99.9%</td>
      <td>1585/1583</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-miscgag1-high">
      <td><a class="scene-perf-rowlink" href="#perf-miscgag1-high"><code>miscgag1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>953/961</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-miscgag1-low">
      <td><a class="scene-perf-rowlink" href="#perf-miscgag1-low"><code>miscgag1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>953/960</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-miscgag2-high">
      <td><a class="scene-perf-rowlink" href="#perf-miscgag2-high"><code>miscgag2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1351/1355</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-miscgag2-low">
      <td><a class="scene-perf-rowlink" href="#perf-miscgag2-low"><code>miscgag2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1351/1355</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand1-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand1-high"><code>stand1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>195/202</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand1-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand1-low"><code>stand1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>195/202</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand2-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand2-high"><code>stand2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>481/491</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand2-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand2-low"><code>stand2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>480/490</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand3-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand3-high"><code>stand3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>548/558</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand3-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand3-low"><code>stand3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>547/557</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand4-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand4-high"><code>stand4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1202/1220</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand4-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand4-low"><code>stand4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1203/1220</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand5-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand5-high"><code>stand5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1443/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand5-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand5-low"><code>stand5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1443/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand6-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand6-high"><code>stand6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1346/1364</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand6-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand6-low"><code>stand6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1347/1364</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand7-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand7-high"><code>stand7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand7-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand7-low"><code>stand7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>521/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand8-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand8-high"><code>stand8</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>483/499</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand8-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand8-low"><code>stand8</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>483/500</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand9-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand9-high"><code>stand9</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand9-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand9-low"><code>stand9</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand10-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand10-high"><code>stand10</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand10-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand10-low"><code>stand10</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand11-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand11-high"><code>stand11</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand11-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand11-low"><code>stand11</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand12-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand12-high"><code>stand12</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1451/1458</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand12-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand12-low"><code>stand12</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1450/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand15-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand15-high"><code>stand15</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>444/452</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand15-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand15-low"><code>stand15</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>444/452</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand16-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand16-high"><code>stand16</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.2%</td>
      <td class="spd-green">99.8%</td>
      <td>473/472</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand16-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand16-low"><code>stand16</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.2%</td>
      <td class="spd-green">99.8%</td>
      <td>473/472</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-suzy1-high">
      <td><a class="scene-perf-rowlink" href="#perf-suzy1-high"><code>suzy1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>5767/5735</td>
      <td>28</td>
      <td>28</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-suzy1-low">
      <td><a class="scene-perf-rowlink" href="#perf-suzy1-low"><code>suzy1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.5%</td>
      <td class="spd-green">99.5%</td>
      <td>5766/5735</td>
      <td>26</td>
      <td>26</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-suzy2-high">
      <td><a class="scene-perf-rowlink" href="#perf-suzy2-high"><code>suzy2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>2652/2633</td>
      <td>17</td>
      <td>17</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-suzy2-low">
      <td><a class="scene-perf-rowlink" href="#perf-suzy2-low"><code>suzy2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>2652/2633</td>
      <td>17</td>
      <td>17</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor1-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor1-high"><code>visitor1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>671/677</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor1-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor1-low"><code>visitor1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>671/677</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor3-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor3-high"><code>visitor3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-23T05:18:30</td>
      <td>w1high-prevvis200-215-postv3</td>
      <td>1.8%</td>
      <td class="spd-yellow">98.2%</td>
      <td>1065/1046</td>
      <td>34</td>
      <td>1</td>
      <td>2</td>
      <td></td>
    </tr>
    <tr id="perf-visitor3-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor3-low"><code>visitor3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-23T05:18:30</td>
      <td>w1high-prevvis200-215-postv3</td>
      <td>1.6%</td>
      <td class="spd-yellow">98.4%</td>
      <td>1062/1045</td>
      <td>38</td>
      <td>0</td>
      <td>6</td>
      <td></td>
    </tr>
    <tr id="perf-visitor4-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor4-high"><code>visitor4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>425/428</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor4-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor4-low"><code>visitor4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>424/428</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor5-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor5-high"><code>visitor5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.5%</td>
      <td class="spd-green">99.5%</td>
      <td>1101/1096</td>
      <td>5</td>
      <td>5</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor5-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor5-low"><code>visitor5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.6%</td>
      <td class="spd-green">99.5%</td>
      <td>1102/1096</td>
      <td>6</td>
      <td>6</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor6-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor6-high"><code>visitor6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2042/2047</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor6-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor6-low"><code>visitor6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2042/2047</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor7-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor7-high"><code>visitor7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1620/1625</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor7-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor7-low"><code>visitor7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1619/1625</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-walkstuf1-high">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf1-high"><code>walkstuf1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-23T05:18:30</td>
      <td>w1high-prevvis200-215-postv3</td>
      <td>1.4%</td>
      <td class="spd-yellow">98.6%</td>
      <td>1466/1445</td>
      <td>35</td>
      <td>5</td>
      <td>6</td>
      <td></td>
    </tr>
    <tr id="perf-walkstuf1-low">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf1-low"><code>walkstuf1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-22T04:53:05</td>
      <td>w1low-trim-main179-phase1</td>
      <td>1.0%</td>
      <td class="spd-green">99.0%</td>
      <td>1461/1447</td>
      <td>31</td>
      <td>2</td>
      <td>6</td>
      <td></td>
    </tr>
    <tr id="perf-walkstuf2-high">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf2-high"><code>walkstuf2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>451/461</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-walkstuf2-low">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf2-low"><code>walkstuf2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>451/461</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-walkstuf3-high">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf3-high"><code>walkstuf3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>2307/2290</td>
      <td>43</td>
      <td>18</td>
      <td>6</td>
      <td></td>
    </tr>
    <tr id="perf-walkstuf3-low">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf3-low"><code>walkstuf3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-16T11:29:21</td>
      <td>git:2b617cbc</td>
      <td>0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>2310/2294</td>
      <td>26</td>
      <td>18</td>
      <td>2</td>
      <td></td>
    </tr>
  </tbody>
</table>

## See also

- [/scenes/]({{ '/scenes/' | relative_url }}) — the visual-signoff
  ledger this card lives next to. Different bar, different cadence,
  different failure modes.
- [/docs/performance/]({{ '/docs/performance/' | relative_url }}) —
  reference manual: what each column means, how `loop_vb` and
  `target_vb` are measured, the column-by-column glossary.
- [/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — retrospective on which experiments moved the matrix from the
  compact baseline (`+17.4%` over target) to its current public-capped
  `{{ site.release.perf_target_speed_pct }}%` target speed.
- [/lab/v081-mary4-freeze/]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — the v0.8.1 stability follow-on that kept this matrix's mean
  untouched while fixing a clean-rect pressure freeze the per-commit
  matrix never reached. Soak loop story.
- [/docs/walks/]({{ '/docs/walks/' | relative_url }}) —
  reference manual for the walk subsystem; "Evolution by release"
  consolidates the v0.8.0 clean-rect retry path and v0.8.1
  wave-band/split-rect pressure accounting that the v0.8.1
  story above generalized to fourteen random-position scenes.
- [Glossary: experiment log]({{ '/docs/glossary/#experiment-log' | relative_url }})
  — the long-form decision record at `docs/ps1/performance-experiment-log.md`
  where every accepted and rejected probe got written down. Read
  before re-trying anything that looks promising.
- [`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv)
  — the durable numeric source the table on this page is rendered
  from.
