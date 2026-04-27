---
layout: page
title: Regtest reference set
eyebrow: Archaeology
subtitle: Sixty-two frozen scene captures the regtest harness compares against to detect drift.
description: The frozen reference frames the Johnny Castaway PS1 fan port's regtest harness compares each fresh capture against, plus the metadata that lets a fresh run reproduce the conditions of the original.
---

The regtest reference set is the harness's notion of *what each scene
should look like, today, when nothing has drifted*. It is a directory
per scene — sixty-two of them, indexed by ADS family and tag — and each
directory contains the frozen capture metadata, the per-frame outcome
record, and a self-contained HTML viewer. The frame BMPs themselves
(the actual pixels) live alongside this archaeology in
`regtest-references/<SCENE>/frames/`, which is gitignored to keep the
repository tractable. The files in `docs/ps1/archaeology/regtest-references/`
are the bookkeeping.

## What this is

When a host engine produces a scene capture — say, a fresh recording of
FISHING 1 against the host build at HEAD — the regtest harness compares
that capture against the reference set. If the boot string matches, the
seed matches, the frame count matches, and the per-frame state hashes
match, the scene's behaviour has not drifted. If anything fails to
match, the harness reports which frame disagrees and points at the
offending byte range.

The references were captured on 2026-03-28, against a known-good host
build, with deterministic seed forcing and the host-side audio path
silenced. They are deliberately *frozen*. New host changes that break
this set are expected to either explain themselves (and roll the
references forward, deliberately) or roll back. They are not
auto-refreshed. The point of pinning them to a date is that drift can
be detected by comparing against a fixed point.

## The naming

Each directory is named `<ADS>-<tag>` — `FISHING-1`, `BUILDING-7`,
`STAND-15`. The ADS prefix matches the original Sierra animation script
file (`FISHING.ADS`, `BUILDING.ADS`, etc.) and the tag is the scene's
position within that ADS, one-indexed. This is the same scheme the
project uses everywhere else: in the
[scene ledger]({{ '/scenes/' | relative_url }}),
in the boot strings, in the bringup status reports.

The ten ADS families are:

- **ACTIVITY** — ten scenes (ACTIVITY-1 through ACTIVITY-12, with gaps).
- **BUILDING** — seven scenes.
- **FISHING** — eight scenes.
- **JOHNNY** — six scenes.
- **MARY** — five scenes.
- **MISCGAG** — two scenes.
- **STAND** — fourteen scenes (with non-contiguous numbering — STAND-13 and STAND-14 do not exist).
- **SUZY** — two scenes.
- **VISITOR** — six scenes (VISITOR-2 is intentionally absent).
- **WALKSTUF** — three scenes.

Sixty-two directories total, each corresponding to a scene that the
host engine could capture cleanly on 2026-03-28. The full list of 63
scenes the project recognises includes one (`VISITOR-2`) that is not
captureable on the host path; it does not have a reference set. That
shortfall is documented rather than hidden.

## What's in each directory

Each of the sixty-two scene directories contains exactly three files in
`docs/ps1/archaeology/regtest-references/<SCENE>/`:

- **`metadata.json`** — the frozen capture's identity. This holds the
  ADS name, tag, scene index, status (`verified`), the boot string used
  to launch the scene (`window nosound story single 17 seed 1` for
  FISHING 1, for example), the capture mode (`scene-default`), the
  forced seed, the forced island position and tide values when those
  were used to make the capture deterministic, the capture date
  (`2026-03-28T15:30:44Z`), the frame count (82 for FISHING 1), and the
  full ordered list of BMP filenames from `frame_00000.bmp` onward.

- **`result.json`** — the run outcome. This includes the capture
  configuration (frame budget, interval, timeout, CPU mode, forced seed),
  the outcome (exit code, frames captured, the rolling state hash, the
  scene-marker booleans `launched`/`bmp_ok`/`bmp_fail`/`sprite_count_estimate`,
  the visual-batch summary), and the absolute paths to the frames
  directory and the visual.json/visual-batch.json on the originating
  workstation. The state hash is the load-bearing field for regtest
  comparison.

- **`review.html`** — a self-contained HTML viewer that renders each
  frame as an `<img>` element with a sticky header for the boot string
  and frame budget. Opening this file in a browser walks the scene
  frame by frame; FISHING 1's review is 24 KB and renders 82 frames at
  native size. The PNGs the viewer references are at
  `frames-png/frame_NNNNN.png` — sibling to the BMPs — and live
  alongside the archaeology rather than inside it.

The frame BMPs and PNGs themselves are not in the
`docs/ps1/archaeology/` tree. They are checked-out as needed at the
sibling path `regtest-references/<SCENE>/frames/` and
`regtest-references/<SCENE>/frames-png/`. Both directories are
gitignored. A clone of the repo gives you the metadata and the viewers;
running the regtest reference capture script regenerates the frame
images on demand.

## How references get refreshed

Deliberately, when the host engine itself moves under them.

There are two times the references roll forward. The first is when a
host-side change is intentional and known to alter rendering — for
example, when the dirty-rect bookkeeping was rewritten and the per-tile
clear ordering shifted. In that case, the operator runs
`scripts/capture-host-scene.sh` against each affected scene, the new
metadata.json/result.json/review.html are written, and the diff against
the previous version of the metadata is what gets reviewed in code
review. Frame state hashes change, frame counts may change, the
rationale lives in the commit message.

The second is when the boot-string contract itself changes — when a new
forced parameter is added or a default is renamed. That kind of change
touches every scene at once. The reference set rolls in a single
commit, and the commit message says why.

Outside those two cases, the references are frozen. If a fresh capture
disagrees, the assumption is the *fresh capture* is wrong, not the
reference. That is the entire point of having a reference set.

## How regtest uses these

The regtest harness reads each scene's metadata.json to know what boot
string to issue and what frame count to expect. It reads result.json to
get the state hash to compare against. The harness does not need to
look at review.html; that file is for the human auditor.

The fast path is `state_hash` comparison: if the new run's hash matches
the reference's, the scene passed. The slow path, used when the hash
disagrees, is per-frame BMP diffing, which is what produces the
side-by-side review HTML the operator looks at to figure out what
changed. That slow-path diffing is documented in
[the regtest docs]({{ '/docs/regtest/' | relative_url }}) along with the
exit codes and the harness invocation.

Each preserved reference also has a generated shelf page under
[/archaeology/regtest-references/cases/]({{ '/archaeology/regtest-references/cases/' | relative_url }}).
Those pages expose the boot string, frame count, state hash, source artifacts,
and links back to the corresponding scene ledger entry.

## The sixty-two scenes

Each row links to the scene's narrative page. The "ADS" column is the
animation script the scene comes from; the "Tag" column is its index
within that script. Status `verified` means the scene captured cleanly
on 2026-03-28 against the host build of that day.

<table>
<thead>
<tr><th>Scene</th><th>ADS</th><th>Tag</th><th>Reference dir</th></tr>
</thead>
<tbody>
<tr><td><a href="{{ '/scenes/activity1/' | relative_url }}">ACTIVITY-1</a></td><td>ACTIVITY.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-1/">ACTIVITY-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity4/' | relative_url }}">ACTIVITY-4</a></td><td>ACTIVITY.ADS</td><td>4</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-4/">ACTIVITY-4/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity5/' | relative_url }}">ACTIVITY-5</a></td><td>ACTIVITY.ADS</td><td>5</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-5/">ACTIVITY-5/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity6/' | relative_url }}">ACTIVITY-6</a></td><td>ACTIVITY.ADS</td><td>6</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-6/">ACTIVITY-6/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity7/' | relative_url }}">ACTIVITY-7</a></td><td>ACTIVITY.ADS</td><td>7</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-7/">ACTIVITY-7/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity8/' | relative_url }}">ACTIVITY-8</a></td><td>ACTIVITY.ADS</td><td>8</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-8/">ACTIVITY-8/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity9/' | relative_url }}">ACTIVITY-9</a></td><td>ACTIVITY.ADS</td><td>9</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-9/">ACTIVITY-9/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity10/' | relative_url }}">ACTIVITY-10</a></td><td>ACTIVITY.ADS</td><td>10</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-10/">ACTIVITY-10/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity11/' | relative_url }}">ACTIVITY-11</a></td><td>ACTIVITY.ADS</td><td>11</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-11/">ACTIVITY-11/</a></td></tr>
<tr><td><a href="{{ '/scenes/activity12/' | relative_url }}">ACTIVITY-12</a></td><td>ACTIVITY.ADS</td><td>12</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/ACTIVITY-12/">ACTIVITY-12/</a></td></tr>
<tr><td><a href="{{ '/scenes/building1/' | relative_url }}">BUILDING-1</a></td><td>BUILDING.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-1/">BUILDING-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/building2/' | relative_url }}">BUILDING-2</a></td><td>BUILDING.ADS</td><td>2</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-2/">BUILDING-2/</a></td></tr>
<tr><td><a href="{{ '/scenes/building3/' | relative_url }}">BUILDING-3</a></td><td>BUILDING.ADS</td><td>3</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-3/">BUILDING-3/</a></td></tr>
<tr><td><a href="{{ '/scenes/building4/' | relative_url }}">BUILDING-4</a></td><td>BUILDING.ADS</td><td>4</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-4/">BUILDING-4/</a></td></tr>
<tr><td><a href="{{ '/scenes/building5/' | relative_url }}">BUILDING-5</a></td><td>BUILDING.ADS</td><td>5</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-5/">BUILDING-5/</a></td></tr>
<tr><td><a href="{{ '/scenes/building6/' | relative_url }}">BUILDING-6</a></td><td>BUILDING.ADS</td><td>6</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-6/">BUILDING-6/</a></td></tr>
<tr><td><a href="{{ '/scenes/building7/' | relative_url }}">BUILDING-7</a></td><td>BUILDING.ADS</td><td>7</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/BUILDING-7/">BUILDING-7/</a></td></tr>
<tr><td><a href="{{ '/scenes/fishing1/' | relative_url }}">FISHING-1</a></td><td>FISHING.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-1/">FISHING-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/fishing2/' | relative_url }}">FISHING-2</a></td><td>FISHING.ADS</td><td>2</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-2/">FISHING-2/</a></td></tr>
<tr><td><a href="{{ '/scenes/fishing3/' | relative_url }}">FISHING-3</a></td><td>FISHING.ADS</td><td>3</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-3/">FISHING-3/</a></td></tr>
<tr><td><a href="{{ '/scenes/fishing4/' | relative_url }}">FISHING-4</a></td><td>FISHING.ADS</td><td>4</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-4/">FISHING-4/</a></td></tr>
<tr><td><a href="{{ '/scenes/fishing5/' | relative_url }}">FISHING-5</a></td><td>FISHING.ADS</td><td>5</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-5/">FISHING-5/</a></td></tr>
<tr><td><a href="{{ '/scenes/fishing6/' | relative_url }}">FISHING-6</a></td><td>FISHING.ADS</td><td>6</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-6/">FISHING-6/</a></td></tr>
<tr><td><a href="{{ '/scenes/fishing7/' | relative_url }}">FISHING-7</a></td><td>FISHING.ADS</td><td>7</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-7/">FISHING-7/</a></td></tr>
<tr><td><a href="{{ '/scenes/fishing8/' | relative_url }}">FISHING-8</a></td><td>FISHING.ADS</td><td>8</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/FISHING-8/">FISHING-8/</a></td></tr>
<tr><td><a href="{{ '/scenes/johnny1/' | relative_url }}">JOHNNY-1</a></td><td>JOHNNY.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-1/">JOHNNY-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/johnny2/' | relative_url }}">JOHNNY-2</a></td><td>JOHNNY.ADS</td><td>2</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-2/">JOHNNY-2/</a></td></tr>
<tr><td><a href="{{ '/scenes/johnny3/' | relative_url }}">JOHNNY-3</a></td><td>JOHNNY.ADS</td><td>3</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-3/">JOHNNY-3/</a></td></tr>
<tr><td><a href="{{ '/scenes/johnny4/' | relative_url }}">JOHNNY-4</a></td><td>JOHNNY.ADS</td><td>4</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-4/">JOHNNY-4/</a></td></tr>
<tr><td><a href="{{ '/scenes/johnny5/' | relative_url }}">JOHNNY-5</a></td><td>JOHNNY.ADS</td><td>5</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-5/">JOHNNY-5/</a></td></tr>
<tr><td><a href="{{ '/scenes/johnny6/' | relative_url }}">JOHNNY-6</a></td><td>JOHNNY.ADS</td><td>6</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/JOHNNY-6/">JOHNNY-6/</a></td></tr>
<tr><td><a href="{{ '/scenes/mary1/' | relative_url }}">MARY-1</a></td><td>MARY.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-1/">MARY-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/mary2/' | relative_url }}">MARY-2</a></td><td>MARY.ADS</td><td>2</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-2/">MARY-2/</a></td></tr>
<tr><td><a href="{{ '/scenes/mary3/' | relative_url }}">MARY-3</a></td><td>MARY.ADS</td><td>3</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-3/">MARY-3/</a></td></tr>
<tr><td><a href="{{ '/scenes/mary4/' | relative_url }}">MARY-4</a></td><td>MARY.ADS</td><td>4</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-4/">MARY-4/</a></td></tr>
<tr><td><a href="{{ '/scenes/mary5/' | relative_url }}">MARY-5</a></td><td>MARY.ADS</td><td>5</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MARY-5/">MARY-5/</a></td></tr>
<tr><td><a href="{{ '/scenes/miscgag1/' | relative_url }}">MISCGAG-1</a></td><td>MISCGAG.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MISCGAG-1/">MISCGAG-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/miscgag2/' | relative_url }}">MISCGAG-2</a></td><td>MISCGAG.ADS</td><td>2</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/MISCGAG-2/">MISCGAG-2/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand1/' | relative_url }}">STAND-1</a></td><td>STAND.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-1/">STAND-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand2/' | relative_url }}">STAND-2</a></td><td>STAND.ADS</td><td>2</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-2/">STAND-2/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand3/' | relative_url }}">STAND-3</a></td><td>STAND.ADS</td><td>3</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-3/">STAND-3/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand4/' | relative_url }}">STAND-4</a></td><td>STAND.ADS</td><td>4</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-4/">STAND-4/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand5/' | relative_url }}">STAND-5</a></td><td>STAND.ADS</td><td>5</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-5/">STAND-5/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand6/' | relative_url }}">STAND-6</a></td><td>STAND.ADS</td><td>6</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-6/">STAND-6/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand7/' | relative_url }}">STAND-7</a></td><td>STAND.ADS</td><td>7</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-7/">STAND-7/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand8/' | relative_url }}">STAND-8</a></td><td>STAND.ADS</td><td>8</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-8/">STAND-8/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand9/' | relative_url }}">STAND-9</a></td><td>STAND.ADS</td><td>9</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-9/">STAND-9/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand10/' | relative_url }}">STAND-10</a></td><td>STAND.ADS</td><td>10</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-10/">STAND-10/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand11/' | relative_url }}">STAND-11</a></td><td>STAND.ADS</td><td>11</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-11/">STAND-11/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand12/' | relative_url }}">STAND-12</a></td><td>STAND.ADS</td><td>12</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-12/">STAND-12/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand15/' | relative_url }}">STAND-15</a></td><td>STAND.ADS</td><td>15</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-15/">STAND-15/</a></td></tr>
<tr><td><a href="{{ '/scenes/stand16/' | relative_url }}">STAND-16</a></td><td>STAND.ADS</td><td>16</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/STAND-16/">STAND-16/</a></td></tr>
<tr><td><a href="{{ '/scenes/suzy1/' | relative_url }}">SUZY-1</a></td><td>SUZY.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/SUZY-1/">SUZY-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/suzy2/' | relative_url }}">SUZY-2</a></td><td>SUZY.ADS</td><td>2</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/SUZY-2/">SUZY-2/</a></td></tr>
<tr><td><a href="{{ '/scenes/visitor1/' | relative_url }}">VISITOR-1</a></td><td>VISITOR.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-1/">VISITOR-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/visitor3/' | relative_url }}">VISITOR-3</a></td><td>VISITOR.ADS</td><td>3</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-3/">VISITOR-3/</a></td></tr>
<tr><td><a href="{{ '/scenes/visitor4/' | relative_url }}">VISITOR-4</a></td><td>VISITOR.ADS</td><td>4</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-4/">VISITOR-4/</a></td></tr>
<tr><td><a href="{{ '/scenes/visitor5/' | relative_url }}">VISITOR-5</a></td><td>VISITOR.ADS</td><td>5</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-5/">VISITOR-5/</a></td></tr>
<tr><td><a href="{{ '/scenes/visitor6/' | relative_url }}">VISITOR-6</a></td><td>VISITOR.ADS</td><td>6</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-6/">VISITOR-6/</a></td></tr>
<tr><td><a href="{{ '/scenes/visitor7/' | relative_url }}">VISITOR-7</a></td><td>VISITOR.ADS</td><td>7</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/VISITOR-7/">VISITOR-7/</a></td></tr>
<tr><td><a href="{{ '/scenes/walkstuf1/' | relative_url }}">WALKSTUF-1</a></td><td>WALKSTUF.ADS</td><td>1</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/WALKSTUF-1/">WALKSTUF-1/</a></td></tr>
<tr><td><a href="{{ '/scenes/walkstuf2/' | relative_url }}">WALKSTUF-2</a></td><td>WALKSTUF.ADS</td><td>2</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/WALKSTUF-2/">WALKSTUF-2/</a></td></tr>
<tr><td><a href="{{ '/scenes/walkstuf3/' | relative_url }}">WALKSTUF-3</a></td><td>WALKSTUF.ADS</td><td>3</td><td><a href="{{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/WALKSTUF-3/">WALKSTUF-3/</a></td></tr>
</tbody>
</table>

## Cross-links

- [Regtest documentation]({{ '/docs/regtest/' | relative_url }}) — the
  harness that consumes this set.
- [Per-case reference shelf]({{ '/archaeology/regtest-references/cases/' | relative_url }}) —
  one generated page per frozen host baseline.
- [Per-scene ledger]({{ '/scenes/' | relative_url }}) — the same scenes
  with their bringup status, narrative, and screenshots.
- [Devlog]({{ '/devlog/' | relative_url }}) — entries that walk through
  reference roll-forwards when they happened.
- [Era timeline]({{ '/archaeology/timeline/' | relative_url }}) —
  context for why the references were captured on 2026-03-28
  specifically.

## Source on GitHub

[docs/ps1/archaeology/regtest-references/]({{ site.github_url }}/tree/main/docs/ps1/archaeology/regtest-references/)
