---
layout: post
title: "What twenty coasters taught us about real PS1 hardware — July 5, 2026"
date: 2026-07-05
editor_note: "The 0.9.13 release marathon in one devblog: twenty burned CD-Rs chasing graphical bands, dead audio, a horrifying buzz, and freezes that no emulator would reproduce — ending in a one-byte register fix and a testing doctrine we're writing down for every future project."
source_path: docs/ps1/real-hardware-testing-lessons.md
description: "Real-hardware lessons from the Johnny Castaway PS1 port: why emulators can't validate hardware, how status registers lie, and the one-byte bug that wore five different disguises."
---

Version 0.9.13 took twenty burn tests. Not because we shipped twenty
features — because real PlayStation hardware kept telling us, politely
and then less politely, that everything our emulator had blessed was
wrong. This post is the story of that week, and the doctrine we're
carrying into every future retro project. The full engineering
reference lives in `docs/ps1/real-hardware-testing-lessons.md`; this is
the narrative version.

## The setup

The scene explorer — a pause-menu screen that streams 150 KB preview
images off the CD — worked beautifully in DuckStation and produced
"rotated strips" on a real console. Then it crashed after two previews.
Then the background ocean died. Then it came back as a buzz loud enough
to make our tester lunge for the power switch. Every build fixed the
previous theory's bug, verified clean in the emulator, and came back
from the console with a new symptom.

## The villains, in order of unmasking

**DrawSync lies.** The SDK's "GPU is done" call returns early on real
silicon. We knew this from the boot-galleon work, and it still bit us
twice more — once via the draw *queue*, where a queued-but-not-started
texture upload is invisible to every hardware status register and then
starts itself, via interrupt, in the middle of your carefully-serialized
direct GPU writes.

**Drives lie.** A worn drive that hiccups mid-read retries by
re-seeking — and silently re-delivers sectors it already sent. No
error. No status. Our tester's photo of a preview with *two moons in
it* — the bottom half repeating the top — was the entire proof. The fix
that finally stuck: read sectors raw, with their addresses attached,
and make every sector prove it is who it claims to be before a single
byte reaches the screen.

**Commands evaporate.** Every CD read ends with an asynchronous pause
command, and its completion interrupt resets the controller's parameter
FIFO. Issue a seek in that window and the seek target silently
vanishes — the next read comes from the wrong place. The race window
scales with drive fatigue, which is why the bands got *worse the longer
you browsed*. We'd guarded one seek against this since 0.9.6. There
were six more.

**And underneath everything: one byte.** The fix we shipped in
burntest6 — wait for the CD DMA channel to finish before touching its
buffer — polled address `0xBF8010B0`. The channel-busy bit lives at
`0xBF8010B8`. `B0` is the channel's *address register*, whose bit 24 is
always zero, so our "wait" returned instantly, every time, for thirteen
builds. Worse than the stale reads: with no wait, each new read
reprogrammed a still-active DMA channel, which sprays incoming sectors
across random RAM. That one byte wore five disguises — preview bands,
dead ambience, the buzz (a self-healing watchdog re-keying a voice over
corrupted sound tables), wandering debris bands, and intermittent
red-screen boots. The diagnostic counter watching that register read
zero all week. *A counter that never fires is a smell, not a comfort.*

## What actually worked

Not cleverness — instrumentation. We put seven counters on the pause
menu screen and had our tester read them off the CRT like a flight
recorder: `P0 Q0 R0 W0 c-1 D13 S1`. Each report *excluded* mechanisms.
A RAM-generated test pattern pushed through the same rendering path
split the world in one button press: pattern clean means the data path
corrupts, pattern broken means the render path does. And the tester's
own observations were precision instruments — "the band moved to the
top after tree scenes" identified an entire subsystem (per-scene
backdrop restoration painting from corrupted buffers) in one sentence.

The other thing that worked: policy. One change per burn. Bounded
waits everywhere, so failure degrades instead of freezing. I/O errors
never halt — retry, black-fill, or skip to the next scene. Reads that
are cosmetic (preview thumbnails) never feed the machinery that resets
scenes. And when a fix doesn't change the symptom, prove the fix
*executes* before inventing the next theory.

## The doctrine

Emulators validate logic. Consoles validate reality. Between them sits
a class of bug that only exists where interrupts have latency, DMA has
momentum, drives have worn sleds, and status registers were written by
optimists. You cannot test your way around it from a desk — but you can
design for it: verify instead of trust, bound instead of wait, degrade
instead of halt, and put your instruments where the tester can read
them.

Twenty coasters. One byte. Worth writing down.
