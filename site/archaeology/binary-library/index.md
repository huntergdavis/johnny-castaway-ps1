---
layout: page
title: The Binary Library
eyebrow: Archaeology · Retired validation era
subtitle: The heavy regression corpus was useful, expensive, and ultimately replaced by scene-by-scene pixel validation.
description: "A short archaeology note for the Johnny Castaway PS1 binary-library era: what it was, why it existed, and why the project no longer ships the full corpus."
---

The binary library was the branch's brute-force regression archive:
historical PS1 builds, CD images, manifests, and index files used to
bisect visual behavior while the rendering method was still unsettled.
It made sense when the project needed to compare whole historical
states and did not yet have a stable foreground playback path.

That era is over. The live workflow is scene-by-scene validation
against the FG2/FGP3 foreground methodology, capped at
[`{{ site.release.tag }}`]({{ '/releases/' | relative_url }})
with all
[`{{ site.release.scenes_validated }} of {{ site.release.scenes_total }}` scenes]({{ '/scenes/' | relative_url }})
signed off and a separate
[performance ledger]({{ '/perf/' | relative_url }}) tracking
target-speed convergence. The full binary payload is not intended
to live in the repo; the searchable archaeology records are enough
to recover the story.

## Preserved Indexes

- [binary-library index CSV]({{ site.github_url }}/blob/main/docs/ps1/archaeology/binary-library-index.csv)
- [binary-library index JSON]({{ site.github_url }}/blob/main/docs/ps1/archaeology/binary-library-index.json)
- [binary-library manifest CSV]({{ site.github_url }}/blob/main/docs/ps1/archaeology/binary-library-manifest.csv)
- [binary-library manifest JSON]({{ site.github_url }}/blob/main/docs/ps1/archaeology/binary-library-manifest.json)
- [binary-library summary]({{ site.github_url }}/blob/main/docs/ps1/archaeology/binary-library-SUMMARY.txt)

## Related Pages

- [Primary sources]({{ '/archaeology/data/' | relative_url }})
- [Retired scripts]({{ '/archaeology/retired-scripts/' | relative_url }})
- [Timeline]({{ '/archaeology/timeline/' | relative_url }})
