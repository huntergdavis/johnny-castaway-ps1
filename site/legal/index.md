---
title: Legal
eyebrow: License + disclaimer
subtitle: GPL-3.0 for the code; Sierra retains the character.
description: Legal terms for the Johnny Castaway PS1 fan port — GPL-3.0 license on the project's own code, MPL-2.0 attribution for spicyjpeg's pad-poll example, the Sierra On-Line character/asset disclaimer, trademark note, takedown procedure, and the static-no-tracking privacy stance.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## License (code)

The code in this project is licensed under the
**GNU General Public License v3.0**. The full license text is in
[LICENSE]({{ site.github_url }}/blob/main/LICENSE) at the repository
root. In short: you can use, modify, and redistribute the code,
provided downstream work is also GPL-licensed and includes the
license + source.

This project also includes code derived from
**spicyjpeg's PSn00bSDK pad-poll example**, which is licensed
under the **Mozilla Public License v2.0**. That's compatible with
GPL-3.0 in the form used here (see [`src/platform/ps1/spi.c`]({{ site.github_url }}/blob/main/src/platform/ps1/spi.c)).

## Disclaimer (assets and character)

> *Johnny Castaway*, the character, the original screensaver, and
> all original Sierra art / audio assets are © Sierra On-Line and
> are **not** licensed under GPL.

This project ships **only the code that drives the port**. End
users running the released `.bin/.cue` are playing baked
*playback packs* — small binary files that record what the
original engine drew, in a form the PS1 can render. The host
build, used during development, requires the original Sierra
data files (`RESOURCE.MAP`, `RESOURCE.001`) which the user must
supply themselves.

## Trademarks

*Johnny Castaway*, *Sierra*, and *Sierra On-Line* are trademarks
of their respective owners. Their use here is descriptive only.

## How to file a takedown

If you represent the original creator or a successor in interest
and want this work taken down, [file a GitHub issue]({{ site.github_url }}/issues/new)
on the project repo. We'll comply.

## Privacy

This site is statically hosted on GitHub Pages. It does not set
cookies, run analytics, or load tracking scripts. GitHub may log
requests; that's GitHub's policy, not ours.

## Related pages

- [Credits]({{ '/credits/' | relative_url }}) — the longer
  attribution: Sierra On-Line, the prior reverse-engineering
  ports, the toolchain authors, the AI sub-agents, and the four
  self-hosted SIL OFL fonts.
- [FAQ: Is this legal?]({{ '/faq/' | relative_url }}) — the
  short author-written version of the disclaimer above.
- [About]({{ '/about/' | relative_url }}) — what this project
  is and isn't, in plain language.
- [Lab: Building a fan port in public]({{ '/lab/fan-port-in-public/' | relative_url }})
  — the magazine treatment of this page's subject: GPL-3.0,
  Sierra-asset disclaimers, the original-creator's permission,
  "if you paid for this you were cheated" as voice, and how a
  fan port ships without lawyers in 2026.
