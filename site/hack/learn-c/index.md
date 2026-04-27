---
layout: page
title: Learn C From Johnny
eyebrow: Curious hacker path
subtitle: Plain C, small machines, visible consequences.
description: A guided reading path through the C code in the Johnny Castaway PS1 port.
---

This is a good C codebase to learn from because mistakes become visible. A
bad pointer can become a corrupted sprite. A missing bounds check can become
a one-frame tear. A sloppy state transition can leave a pause menu ghosting
over the island. That is annoying when you are shipping; it is useful when
you are learning.

## Start With Boundaries

The important thing is not syntax. The important thing is ownership. Which
module owns memory? Which module owns time? Which module owns a draw call?
Which module is allowed to know about PS1 hardware?

Use the [API mapping]({{ '/docs/api/' | relative_url }}) as the first map. It
shows the seam between the original SDL2-oriented code and PSn00bSDK. Then
read the source-facing wrappers in the [source library]({{ '/source/' | relative_url }}),
especially:

- [API mapping reference]({{ '/source/docs/ps1/api-mapping/' | relative_url }})
- [Build system]({{ '/source/docs/ps1/build-system/' | relative_url }})
- [Hardware specs]({{ '/source/docs/ps1/hardware-specs/' | relative_url }})
- [Memory management]({{ '/source/docs/general/memory-management/' | relative_url }})

The lesson is that C on a console is not "C but older." It is C where the
machine's memory map is part of the program's public API.

## The Reading Order

Read state machines before renderers. The pause menu, holiday selection, and
caption toggle are easier to reason about than foreground pack replay because
they have fewer moving pieces and clear input-output boundaries. After that,
read the pack replay path and the audio cue path together; the whole point of
the hybrid pipeline is that pixels and sounds advance on the same frame clock.

Then read the resource code. Resource loading is where a desktop assumption
usually dies first on a console. Files are not free. Seeks are not free.
Alignment is not a courtesy.

## The C Habits This Project Rewards

- Prefer small structs with explicit ownership over clever pointer webs.
- Keep file formats boring, versioned, and easy to dump.
- Make state visible: telemetry overlays, hashes, frame counters, and build logs.
- Treat `printf` as a tool with a blast radius, not as a harmless reflex.
- Preserve old failed approaches in docs, because failed approaches explain why the current code exists.

For the long-form version of the tool-and-habit side, read
[Regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
and [Hallucination engineering]({{ '/lab/hallucination-engineering/' | relative_url }}).
