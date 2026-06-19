---
title: Performance battle card
eyebrow: Headless · 126-variant matrix
subtitle: Click a column header to sort. Target Speed cells are color-coded — ≥99% green, ≥95% yellow, ≥90% orange, <90% red.
description: The Johnny Castaway PS1 fan port headless-perf battle card — 126 scene/tide variants timed against target frame budget, color-coded, with sortable column headers.
redirect_from:
  - /performance/
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

A labor of love by Hunter Davis. This is the second ledger: 126 scene/tide
variants, timed headlessly in DuckStation against the captured target budget.
The [scene ledger]({{ '/scenes/' | relative_url }}) tracks visual signoff.
This page tracks timing. Different bars, different failure modes.

If you paid for this, you were cheated. Open source and free.

<noscript>
<p><em>Note:</em> the column-header click-to-sort affordance the
subtitle mentions requires JavaScript. Without it, the matrix
below is fully readable but cells stay in source order. The
durable numeric source is the
<a href="https://github.com/{{ site.repo }}/blob/main/docs/ps1/performance-scene-matrix.csv">CSV</a>
linked below.</p>
</noscript>

## At a glance

<p class="scene-perf-legend" aria-label="Current target speed distribution">
  Target Speed distribution in the current matrix:
  <span class="spd-key spd-green">126 (100.0%) ≥ 99%</span>
  <span class="spd-key spd-yellow">0 (0.0%) ≥ 95%</span>
  <span class="spd-key spd-orange">0 (0.0%) ≥ 90%</span>
  <span class="spd-key spd-red">0 (0.0%) &lt; 90%</span>
  out of 126 timing-bearing rows.
</p>

Current battle-card rollup as of <time datetime="2026-05-23">2026-05-23</time>:

| Metric | Value |
|---|---:|
| Scenes visually validated | `{{ validated_count }} / {{ total_count }}` (`100%`) |
| Scene/tide variants routed through headless perf | `126 / 126` (`100%`) |
| Timing-bearing variants | `126 / 126` (`100%`) |
| Pending variants | `0 / 126` (`0%`) |
| Blocked variants | `0 / 126` (`0%`) |
| Timing-bearing average over target | `+0.2%` (`0.1571%` exact, public-capped) |
| Timing-bearing average target speed | `99.8%` (`99.8440%` exact, public-capped) |
| FISHING 1 canary | high `1068 / 1073 VBlanks`, low `1067 / 1074 VBlanks`, both public-capped at `100.0%` target speed |

<ul class="doc-grid">
  <li>
    <a href="{{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv">Performance matrix CSV</a>
    <p>The durable numeric source for the table below.</p>
  </li>
  <li>
    <a href="{{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md">Experiment log</a>
    <p>Accepted and rejected perf probes. Read this before retrying a tempting idea.</p>
  </li>
  <li>
    <a href="{{ '/docs/performance/' | relative_url }}">Performance reference</a>
    <p>Counter definitions, harness behavior, bottlenecks, and maintenance rules.</p>
  </li>
  <li>
    <a href="{{ '/lab/from-87-to-99-5/' | relative_url }}">From 87 to 99.7</a>
    <p>The retrospective on how the post-validation matrix got here.</p>
  </li>
  <li>
    <a href="{{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.csv">Preprocess targets</a>
    <p>Pack-time graphics candidates for future timing work.</p>
  </li>
  <li>
    <a href="{{ site.github_url }}/blob/main/docs/ps1/performance-read-candidate-matrix.csv">Read candidates</a>
    <p>Current read-plan candidates and closed lanes.</p>
  </li>
</ul>

## Reading the table

- **Scene**: clicking the scene slug in any row updates the URL with
  `#perf-{slug}-{tide}` so you can copy a stable link to that exact
  row. Same anchors are linked from each scene page's "Performance
  battle card" line.
- **[Over Target]({{ '/docs/glossary/#over-target' | relative_url }})**:
  how far `loop_vb` is above the captured target timing. Public site
  values are capped at `0.0%` for faster-than-target rows so fast
  scenes do not create negative debt. Lower is better.
- **[Target Speed]({{ '/docs/glossary/#target-speed' | relative_url }})**:
  public display of `target_vb / loop_vb`, capped at `100%` so no row
  reports faster than intended playback. The raw signed ratio remains
  in the CSV for optimization analysis.
- **[VBlanks]({{ '/docs/glossary/#vblank' | relative_url }})**:
  `loop_vb/target_vb` for the measured active loop.
- **[Blocking]({{ '/docs/glossary/#blocking-vb' | relative_url }})**:
  visible CD/blocking VBlanks.
- **[Prefetch]({{ '/docs/glossary/#prefetch-hits' | relative_url }})**:
  prefetch overrun VBlanks.
- **Due**: due-frame misses.
- **Latest Run**: ISO timestamp derived from the headless summary path
  (`scratch/ps1-perf-iterate/YYYYMMDD-HHMMSS`); `-` means no current
  matrix run has been recorded for that variant.
- **Stats Version**: performance/layout version for that row. Older rows
  retain their per-row version stamps; the complete baseline remains in
  the CSV for anyone chasing archaeology.

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
      <td>2026-05-23T14:38:29</td>
      <td>floor-lift-multiscene</td>
      <td>0.2%</td>
      <td class="spd-green">99.8%</td>
      <td>1319/1316</td>
      <td>34</td>
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
      <td>2026-05-23T14:38:29</td>
      <td>floor-lift-multiscene</td>
      <td>0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>2832/2816</td>
      <td>34</td>
      <td>27</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building4-low">
      <td><a class="scene-perf-rowlink" href="#perf-building4-low"><code>building4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-23T14:38:29</td>
      <td>floor-lift-multiscene</td>
      <td>0.4%</td>
      <td class="spd-green">99.6%</td>
      <td>2833/2822</td>
      <td>29</td>
      <td>23</td>
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
      <td>2026-05-23T14:26:57</td>
      <td>johnny6-double-preload</td>
      <td>0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>2817/2801</td>
      <td>25</td>
      <td>25</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny6-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny6-low"><code>johnny6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-23T14:26:57</td>
      <td>johnny6-double-preload</td>
      <td>0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>2818/2802</td>
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
      <td>2026-05-23T13:25:33</td>
      <td>v3high-double-preload-phase2</td>
      <td>0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>1053/1046</td>
      <td>33</td>
      <td>0</td>
      <td>2</td>
      <td></td>
    </tr>
    <tr id="perf-visitor3-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor3-low"><code>visitor3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-23T13:11:54</td>
      <td>v3low-dp-v3h-phase4-layout-shift</td>
      <td>0.8%</td>
      <td class="spd-green">99.2%</td>
      <td>1052/1044</td>
      <td>40</td>
      <td>1</td>
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
      <td>2026-05-23T13:11:54</td>
      <td>v3low-dp-v3h-phase4-layout-shift</td>
      <td>1.0%</td>
      <td class="spd-green">99.0%</td>
      <td>1458/1444</td>
      <td>36</td>
      <td>6</td>
      <td>6</td>
      <td></td>
    </tr>
    <tr id="perf-walkstuf1-low">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf1-low"><code>walkstuf1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-23T14:38:29</td>
      <td>floor-lift-multiscene</td>
      <td>0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>1452/1448</td>
      <td>33</td>
      <td>3</td>
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
