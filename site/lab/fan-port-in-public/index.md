---
layout: page
title: Building a fan port in public
eyebrow: Lab . Legal and social shape
subtitle: GPL code, Sierra assets, original creators, and the Credits screen that says the quiet part plainly.
description: "A practical essay on the legal and social posture of the Johnny Castaway PS1 fan port: GPL code, original Sierra assets, release artifacts, and public disclaimers."
date: 2026-04-26
---

## The line

This is a fan port. It is not an official Sierra release. I do not own Johnny
Castaway. I do not have a license to the character. The original creator has
generously allowed fan ports. That generosity is not a property right and I do
not treat it like one.

That is the line. The site says it. The in-game Credits screen says it. The
legal page says it.

## What ships

The public repository ships GPL-3.0 code, scripts, docs, generated PS1 runtime
artifacts, and release disc images.

The development host build requires original Sierra data files supplied by the
user. The repo does not pretend those assets are GPL. The legal posture is
plain because it has to be plain. A fan project gets very little room for
cuteness here.

## Why release a disc image at all

Because the point is to play it.

The release script creates `jcreborn.bin` and `jcreborn.cue`, uploads them as
GitHub Release assets, and creates a minimal release tag whose tree contains
only those two files. That is not an accident. It means a reader can download
the release and boot the disc without learning the build system first.

Open source does not mean "you must compile it yourself to prove purity."
It means the code is there, the build is there, and the artifact is free.

## If you paid for this

The Credits screen says:

> If you paid for this, you were cheated.
> Open source and free.

That is partly a joke and partly a policy. Fan ports get scraped. Someone will
eventually put a zip on a ROM site, or a disc on an auction site, or a "limited
homebrew edition" label on a burned CD-R. The project cannot stop that. It can
make the real source and real releases obvious enough that nobody has an
excuse.

## Public work is still careful work

The fact that the project is public does not mean every idea lands. The
validated-scene count is conservative. The docs say what is done and what is
not. The release notes do not pretend a scene is perfect because it booted
once. The website is large because the context matters, not because the project
needs hype.

Public fan work survives on trust. Trust is mostly made of boring accuracy.

## Cross-links

- [Legal]({{ '/legal/' | relative_url }})
- [Credits]({{ '/credits/' | relative_url }})
- [Play]({{ '/play/' | relative_url }})
- [FAQ]({{ '/faq/' | relative_url }}) — the author-written answers
  to the practical questions a public fan port keeps fielding.
- [Releases]({{ '/releases/' | relative_url }}) — the release-
  notes discipline this article describes in practice.
- [Voice guide]({{ '/about/voice/' | relative_url }}) — the
  editorial standard the public-facing writing holds itself to.
- [Build & toolchain]({{ '/docs/build/' | relative_url }})
