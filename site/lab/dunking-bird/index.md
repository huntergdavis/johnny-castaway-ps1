---
layout: page
title: The dunking bird
eyebrow: Lab . Advanced development techniques
subtitle: How a 50-cent novelty toy kept a Johnny Castaway performance experiment alive for nine days.
description: A fifty-cent dunking bird kept an LLM coding session warm for nine days while a Johnny Castaway PS1 performance experiment ran. The hack, the thermodynamics, and what advanced-development means in 2026.
date: 2026-04-26
---

## The problem first

Most of the LLM-augmented work on the PS1 port is short bursts. Draft a
caption corpus, audit some YAML, generate a sprite sheet, write the
first cut of a docs page. You sit down at the keyboard, run a session,
read the output, edit, ship.

The performance work is different. The performance work is *long*. A
typical performance experiment on this project goes like this. I have a
hypothesis — say, that prefetching the next FG2 pack's diff spans during
the ocean-loop quiet frames will let `FISHING 1` lock to 60 Hz instead
of dropping to 30 on the cast animation. I write a flag. I rebuild. I
run the regtest harness in headless Docker against a thirty-second
window of `FISHING 1`. I read the perf log. I tweak a constant. I
rebuild. I run again. The actual code change that closes the
hypothesis is fifty lines, but the *exploration* — the parameter
sweep, the variant comparisons, the rollback when one branch makes
another scene worse — takes days of clock time because each iteration
involves a Docker build, a regtest run, and human eyes on a frame
diff.

I sleep eight hours a night. The session times out in two. If the
session times out, I lose the model's working context — the
breadcrumbs of which experiments worked, which were rejected, which
the agent already proposed. I wake up to one experiment of progress
instead of fifty.

That's the problem. The experiment cycle takes days; my attention
cycle is measured in hours. I needed a way to keep the keyboard warm.

## The dunking bird

A drinking bird, also called a dunking bird or a sipping bird, is a
desktop novelty toy invented in the 1940s. There are three thousand
explanations of how it works on the internet and most of them are
wrong; here is the right one. It's a glass bird. It has a head and an
abdomen connected by a thin glass tube. The abdomen is partly filled
with a low-boiling-point liquid — historically dichloromethane,
methylene chloride, sometimes ether, dyed pink or red so you can see
it move. Above the liquid the head is empty, but the whole sealed
volume above the abdomen liquid is the same vapor space.

The head wears a felt hat. Strictly the felt covers the whole head.

You wet the felt. The water evaporates. Evaporation is endothermic, so
the head bulb cools. Cooling drops the saturated vapor pressure inside
the head. The vapor pressure in the warmer abdomen is now higher than
the vapor pressure in the cooler head, and the pressure differential
pushes the liquid up the tube into the head. As the liquid rises, the
center of mass of the bird rises, and at some point the bird tips
forward. When it tips, two things happen: the beak dunks into the
glass of water you have placed in front of it, re-wetting the felt,
*and* the open end of the internal tube clears the liquid surface in
the abdomen. Vapor rushes through, pressures equalize, the liquid
falls back into the abdomen, the bird stands up again, and the cycle
restarts.

It looks like perpetual motion. It is a heat engine. The energy comes
from the temperature differential between the room air and the
evaporative cooling at the felt. It runs as long as there is water in
the glass and humidity in the room is below the wet-bulb threshold.
On a dry afternoon in California it dips for hours.

I have one on my desk because I think they're nice. I have had one on
my desk for years. I am the kind of person who has a dunking bird and
a Sherline lathe and a Tek 465 next to a clean copy of *The C
Programming Language*, second edition. None of this is to brag; it's
to set the stage for what comes next.

## The hack

You position the bird so that when its beak swings down toward the
water glass, it strikes a key on the keyboard.

That's the hack.

I picked the spacebar — wide target, harmless keystroke, won't perturb
a vim session if my finger landed there. I lined the bird up so the
beak grazed the keycap on the down-stroke, taped a small square of
felt to the front edge of the keyboard so the bird had a soft stop on
the back-swing, and let it run.

It ran. The first dip was around six seconds after I wet the felt.
Then every twelve seconds. Then it settled into about eighteen-second
periods. Each dip pressed the spacebar exactly once. The OS read it as
a keystroke. The session timeout reset. The screensaver did not fire.
The Claude Code window stayed in focus. Whatever the agent was waiting
on — an experiment to finish, a build to complete, a test run to
return — it stayed alive in working memory because something somewhere
believed a human was at the keyboard.

I went to bed.

<blockquote class="pullquote">
The actual code wasn't being typed by the bird. The bird just kept the
typing surface ready.
</blockquote>

I should be clear about what the bird was and wasn't doing. The bird
was not writing code. The bird was not running experiments. The bird
was not making decisions. The bird was pressing the spacebar every
eighteen seconds, and that was enough — enough to keep the session
warm, enough to keep the agent in its working context, enough that
when I came back to the desk the next morning the agent had run
forty-two performance variants and had a ranked list waiting for my
review.

There is a digital version. A two-line shell script that fires
`xdotool key space` every fifteen seconds will accomplish the same
thing. I have used both. The digital version is more reliable on
days when the room air is humid. The physical version is more
pleasing. There is a real and slightly silly satisfaction in walking
past your desk at 2 a.m. and seeing the bird still tipping, the
glass still half-full, a felt-headed thermodynamic engine standing
in for your attention.

## Why it actually worked

The performance experiment in question was the prefetch sweep that
eventually became the `FISHING 1` 60 Hz lock. It ran continuously for
nine days. At the end of nine days the agent had explored a roughly
two-hundred-point parameter space across three orthogonal flags
(prefetch quantum, ocean-frame slack, dirty-rect overlap threshold),
and the ranked top-ten included one configuration that survived all
the regtest gates and merged cleanly into `main`. The result lives in
the [/docs/performance/]({{ '/docs/performance/' | relative_url }})
log, and the methodology that produced it lives in
[the LLM-pass essay]({{ '/lab/llm-pass/' | relative_url }}).

What the bird actually contributed:

- **It kept the LLM session warm.** The agent's working context — the
  prior experiments, the ranked list, the rejected branches — stays
  in memory only as long as the session is alive. A timeout flushes
  it. The bird kept the session alive.
- **It kept the foreground window in focus.** macOS and Linux both
  give different priority to the foreground app. A click into the
  terminal puts the session in the foreground; a stray click into a
  browser yanks it out. The bird only ever pressed the spacebar in
  the terminal because the terminal was where I'd left the focus.
- **It cleared screensaver and idle timers.** Most operating systems
  will dim the screen, sleep the disk, or kick the network if the
  user is idle. Most of those are configurable to "never," but on
  shared laptops with corporate MDM you don't always own the
  configuration. A keystroke every eighteen seconds is a hard reset
  on every idle timer in the stack.
- **It was visible.** When I walked into the office in the morning,
  one glance at the bird told me the session was still going. No
  console check, no log scrub. The bird was the indicator.

The performance experiment finished. The result merged. Nothing about
the result is bird-dependent. But the result happened in nine days
instead of nine weeks, and that compression came from the bird.

## The deeper point

Half of advanced development in 2026 is software, and half is the kind
of thing you don't see in a conference talk. Foot pedals as extra
modifier keys. Doorbells wired to build-success notifiers. A Wii
balance board under your desk so you can lean to scroll. A piezo
buzzer that goes off when the regtest harness flips a frame. A Raspberry
Pi Zero that opens a ticket in the bug tracker when the office
temperature exceeds 28 °C because the build server is in the same room
and starts throttling. None of this is in the LLM. None of this is in
the IDE. It is the layer between the human and the keyboard, and the
people who get the most out of modern tooling tend to be the people
who have built that layer carefully over years.

The dunking bird falls cleanly into that bucket. It is a piece of
desk furniture from 1945 that solves a problem from 2026. The fact
that it works is not a clever insight; the fact that it *works for
this* is the insight. A keystroke every eighteen seconds is exactly
what an LLM session needs to stay alive, and a dunking bird is exactly
the right cadence — slow enough that it doesn't crash the typing
buffer, fast enough that no idle timer fires, regular enough that the
agent never sees a long pause.

I am not embarrassed about this. I am the world's expert in computer
science *and* in Johnny Castaway, and I have a felt-headed bird
tapping my keyboard at 3 a.m. Both of those statements are true and
neither of them is in tension with the other. Craft is the willingness
to use the right tool, and the right tool here is sometimes a heat
engine.

## Disclosure

The result of the experiment lives at
[/docs/performance/]({{ '/docs/performance/' | relative_url }}). The
agent methodology is at
[/lab/llm-pass/]({{ '/lab/llm-pass/' | relative_url }}). The
hardware-vs-software discussion of "what counts as the dev
environment" continues at
[/lab/build-farm/]({{ '/lab/build-farm/' | relative_url }}).

The bird is a Magic Toy Drinking Bird, made by various Chinese
manufacturers, sold for between fifty cents and four dollars depending
on whether you buy in bulk. Mine cost ninety-nine cents at a hardware
store in Berkeley in 2018. It still runs.

## Coda: other things on the desk

A short list of physical-world hacks I've used over the years on
various Johnny ports.

- **A foot pedal as a modifier key.** USB foot pedal wired through
  `xdotool` to act as a Hyper key for window-manager bindings. This
  predates Johnny by about a decade but lives on under every port.
- **A doorbell as a build-success notifier.** Cheap wireless doorbell,
  receiver on the desk, transmitter wired (with a small relay) to a
  GPIO pin on the build server. When `make all` exits zero, the
  doorbell rings. When it fails, silence. The Pavlovian conditioning
  is real and it is good.
- **A piezo buzzer for the regtest harness.** A 5 V piezo on a USB
  relay. Different tones for different scenes. `FISHING 1` regression
  is a low buzz. `STAND` regression is a high one. After about a week
  you can tell which scene broke from the next room.
- **A second monitor permanently displaying `htop`.** Not exactly a
  hack, but worth mentioning. There is no substitute for a glance at
  the actual load average when something feels off. The monitor cost
  $40 used and it has earned that back ten times.
- **A label maker.** Every cable on the desk is labeled. Every USB
  device is labeled. The dunking bird's glass has a label on the
  bottom that says "DO NOT MOVE." This is a hack against my own
  future self and it works.

The bird is not the cleverest of the bunch. It is just the most
visible. People who walk into the office ask about it before they ask
about the fact that the entire wall behind me is a corkboard covered
in 35 holiday emblem printouts. The bird wins on charm.

## Cross-references

- [The LLM pass]({{ '/lab/llm-pass/' | relative_url }}) — the
  methodology essay this article is a sidebar to.
- [The 24/7 build farm]({{ '/lab/build-farm/' | relative_url }}) —
  what the bird was keeping warm.
- [Hallucination engineering]({{ '/lab/hallucination-engineering/' | relative_url }}) —
  what happens when a long-running agent session goes sideways.
- [/docs/agents/]({{ '/docs/agents/' | relative_url }}) — the
  reference page on AI sub-agent usage.
- [/docs/performance/]({{ '/docs/performance/' | relative_url }}) —
  the perf log the experiment produced.

The bird itself is documented exhaustively at
[en.wikipedia.org/wiki/Drinking_bird](https://en.wikipedia.org/wiki/Drinking_bird).
The Wikipedia page is correct about the thermodynamics, which is more
than I can say for most YouTube explanations. Read the citations.
