---
layout: page
title: FG2 pack payload
eyebrow: Reference
subtitle: The on-disc binary form of one captured ADS scene. 40-byte header, palette, entry table, base-plus-diff frame stream, padded to 2048-byte CD sectors.
description: Byte-level reference for the FG2 (Foreground Pilot 2) pack payload — the on-disc binary the PS1 build reads at runtime to replay one captured ADS scene. Header layout, palette, entry table, base/diff frame format, sound-event table, sector alignment, and a worked example walking the first 64 bytes.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

A labor of love by Hunter Davis. An *FG2 pack* is the on-disc binary form
of one captured ADS scene — the pre-baked replay of what the original Sierra
script does when it runs through to a stop. The PS1 does not interpret ADS
bytecode at runtime; that work happens on a host build with full SDL2, and
the result of every frame gets serialized into a flat binary file. At
runtime the console reads the binary, walks an entry table, and presents one
frame per [VBlank]({{ '/docs/glossary/#vblank' | relative_url }}) window. There is no script, no thread scheduler, no
TTM/SCR composition step. Just `read sector → upload pixels → wait`.

The format is called *FG2* because it is the second iteration of the
foreground-pilot binary. The first iteration was a quick proof-of-concept
that did not align to CD sectors and made `CdRead` more painful than it had
to be. FG2 keeps the same conceptual shape — header, palette, entry table,
frames — but commits to 2048-byte sector alignment, a stable 40-byte
header, and a single magic value (`FGP2`) that the runtime checks before
trusting anything else. It is also still not the long-term binary form. The
[source schema doc]({{ site.github_url }}/blob/main/docs/ps1/research/PACK_PAYLOAD_LAYOUT.md)
is explicit: "the live on-disc pilot format used by the runtime, even
though it is still not the final long-term binary format."

The pack payload sits in the disc image as `FG\<SCENENAME>.FG2`. At runtime
`foreground_pilot.c` resolves the file with `CdSearchFile`, reads the header
from sector 0, validates the magic, and uses `tableOffset` and `dataOffset`
to find the rest. Sound events live at `soundEventsOffset` if the scene has
any.

If you paid for this, you were cheated. Open source and free.

## On-disc layout

A pack file is a single contiguous binary. Top-level shape, in order:

| Region            | Size                          | Notes |
|-------------------|-------------------------------|-------|
| Header            | 40 bytes                      | Fixed-width. Magic `FGP2` first. |
| Palette           | 256 × 2 bytes (512 bytes)     | 16-bit BGR555, one entry per indexed color. |
| Entry table       | `frameCount` × 20 bytes       | Located at `tableOffset`. One row per presented frame. |
| Sound-event table | `soundEventCount` × 4 bytes   | Located at `soundEventsOffset`. Optional. |
| Frame data stream | variable                      | Located at `dataOffset`. Concatenated base + diff frame payloads. |
| Trailing padding  | up to 2047 bytes              | Pads the file up to a 2048-byte sector boundary. |

The header is read into a fixed `struct TFgPilotHeader` at runtime (40 bytes
on the wire, packed). The palette is uploaded once to the CLUT region of
VRAM at scene activation. The entry table is read once and cached. The
sound-event table is small and read once. Only the frame data stream is
streamed sector by sector during playback.

## Header fields

Header layout, exactly as `foreground_pilot.c:42` reads it:

```c
struct TFgPilotHeader {
    char     magic[4];           /* "FGP2" */
    uint16   version;
    uint16   frameCount;
    uint16   displayVBlanks;
    uint16   reserved0;
    uint16   screenWidth;
    uint16   screenHeight;
    uint16   unionX;
    uint16   unionY;
    uint16   unionWidth;
    uint16   unionHeight;
    uint32   tableOffset;
    uint32   dataOffset;
    uint32   soundEventsOffset;
    uint16   soundEventCount;
    uint16   reserved1;
};
```

Total: 40 bytes (`FG_PACK_HEADER_SIZE` in the source). All multi-byte
fields are little-endian (PS1 native).

| Offset | Size | Field               | Meaning |
|-------:|-----:|---------------------|---------|
| 0x00   | 4    | `magic`             | ASCII `"FGP2"`. Runtime rejects anything else. |
| 0x04   | 2    | `version`           | Header format version. Currently 1. |
| 0x06   | 2    | `frameCount`        | Number of rows in the entry table. |
| 0x08   | 2    | `displayVBlanks`    | Default VBlanks-per-frame when an entry does not override. |
| 0x0A   | 2    | `reserved0`         | Reserved. Currently flag bits: `0x0002` host-ticks, `0x0004` host-deadlines, `0x0008` scene-relative coords, `0x0010` base-diff payload required. |
| 0x0C   | 2    | `screenWidth`       | Capture screen width (typically 320). |
| 0x0E   | 2    | `screenHeight`      | Capture screen height (typically 200). |
| 0x10   | 2    | `unionX`            | Union dirty-rect X across all frames in this pack. |
| 0x12   | 2    | `unionY`            | Union dirty-rect Y. |
| 0x14   | 2    | `unionWidth`        | Union dirty-rect width. |
| 0x16   | 2    | `unionHeight`       | Union dirty-rect height. |
| 0x18   | 4    | `tableOffset`       | Byte offset of the entry table from the start of the file. |
| 0x1C   | 4    | `dataOffset`        | Byte offset of the frame-data stream. |
| 0x20   | 4    | `soundEventsOffset` | Byte offset of the sound-event table, or 0 if none. |
| 0x24   | 2    | `soundEventCount`   | Row count for sound events. May be 0. |
| 0x26   | 2    | `reserved1`         | Reserved, currently zero. |

The flag bits in `reserved0` are an in-place revision channel — when the
runtime needs a new behavior gated on a header property it stamps a flag
there rather than bumping `version`. Of those, `0x0010` (base-diff payload
required) is now mandatory: see commit `9122c0e` ("ps1: require base-diff
foreground packs"). Packs without it are rejected.

The `union*` fields exist so the runtime can pre-size its frame buffer and
seed dirty-rect bookkeeping without reading the entry table first. They are
the bounding rectangle that contains every per-frame dirty rect in the
pack.

## Palette

Immediately after the header, at byte 40 (0x28), the file stores 256 × 2
bytes of palette data. Each entry is a 16-bit value in PS1 BGR555 format:

```text
bit 15        bit 0
| | | | | | | | | | | | | | | | |
 T  B B B B B  G G G G G  R R R R R
```

`T` is the PS1 transparency bit (set means "transparent when drawn over a
background"). The host capture tool fills this in from SDL2's surface
palette and re-encodes for PS1 GPU consumption. The runtime uploads this
block as one DMA to the CLUT region.

## Entry table

At `tableOffset` (always 32-bit aligned), the file stores `frameCount`
rows of 20 bytes each. The structure as `foreground_pilot.c` reads it:

```c
struct TFgPilotEntry {
    uint16   sourceFrame;   /* Original frame index in the captured scene */
    sint16   x;             /* Dirty-rect top-left X for this frame */
    sint16   y;             /* Dirty-rect top-left Y */
    uint16   width;         /* Dirty-rect width */
    uint16   height;        /* Dirty-rect height */
    uint16   reserved0;
    uint32   dataOffset;    /* Offset into the frame-data stream */
    uint32   dataSize;      /* Payload byte count for this frame */
};
```

Per-row layout:

| Offset | Size | Field           |
|-------:|-----:|-----------------|
| 0x00   | 2    | `sourceFrame`   |
| 0x02   | 2    | `x` (signed)    |
| 0x04   | 2    | `y` (signed)    |
| 0x06   | 2    | `width`         |
| 0x08   | 2    | `height`        |
| 0x0A   | 2    | `reserved0`     |
| 0x0C   | 4    | `dataOffset`    |
| 0x10   | 4    | `dataSize`      |

Total per row: 20 bytes (`FG_PACK_ENTRY_SIZE`).

`dataOffset` is from the start of the file, not from `dataOffset` in the
header. The runtime can `CdRead` directly using this offset without further
arithmetic. `dataSize` is the *unpadded* payload byte count for the frame;
the actual on-disc layout may bump it up to a sector boundary, but only
`dataSize` bytes are decompressed.

## Base + diff frames

The first frame in the entry table is the *base*: a full indexed-color
image of the dirty rectangle. Every subsequent frame is a *diff* — only
the spans of pixels that changed since the previous frame, encoded as
`(span_offset, span_len, pixels...)` runs.

The runtime keeps a single `frameBuffer` of `union_width × union_height`
indexed pixels and applies each diff in place. To draw a frame to the GPU
it uploads the dirty rectangle to VRAM via `LoadImage`. The encoding
choice is signalled by `packFormat`:

| Value | Constant                          | Meaning |
|-------|-----------------------------------|---------|
| 2     | `kFgPilotPackFormatPal4Spans`     | 4-bit indexed, run-length spans. |
| 3     | `kFgPilotPackFormatIndexed8Spans` | 8-bit indexed, run-length spans. |

Format 3 is what current packs ship as. Format 2 was an experiment for the
smaller-palette TTMs and is not currently emitted. The header flag
`0x0010` ("base-diff payload required") asserts that the pack is in this
shape — packs older than that flag are rejected at load time.

## Sound-event table

If `soundEventsOffset` is non-zero, the file holds `soundEventCount` rows
of 4 bytes:

```c
struct TFgPilotSoundEvent {
    uint16  sourceFrame;
    uint16  sampleId;
};
```

| Offset | Size | Field         |
|-------:|-----:|---------------|
| 0x00   | 2    | `sourceFrame` |
| 0x02   | 2    | `sampleId`    |

`sourceFrame` matches the `sourceFrame` field on entry rows. `sampleId`
indexes into the project's sound bank. The runtime maintains a
`soundEventCursor` and fires events as the playback head crosses their
source frames.

## Padding & sector alignment

CD-ROM Mode 2 Form 1 sectors are 2048 bytes of user data. Every FG2 pack
file is padded with zero bytes so its total length is a multiple of 2048.
Inside the file, the source schema also aligns each compiled-pack resource
to a 2048-byte boundary so streaming code can always issue
sector-aligned reads. From the source doc:

> Each resource starts on a `2048`-byte boundary so the payload can already
> be reasoned about in CD-sector terms. The first resource currently begins
> at sector 1, after the header/TOC block is padded up to `2048` bytes.

The runtime reuses `streamScratch` (sector-sized, sector-aligned) for
`CdRead` calls and copies the relevant slice into `frameBuffer` or
`streamWindowBuffer`. `FG_PREFETCH_DEFAULT_WINDOW_BYTES` is 16 KB, which
covers eight sectors worth of frame data — enough lookahead that the
2x drive's ~150 ms cold seek does not stall presentation. See
[transition prefetch]({{ '/docs/formats/transition-prefetch/' | relative_url }}).

The first 8192 bytes of the file are reserved as a metadata-prefix region
(`FG_PACK_METADATA_PREFIX_BYTES`). The runtime treats this as the safe
chunk to read in one shot at scene activation: header, palette, full entry
table, sound-event table, and the start of the frame stream all fit
within it for typical packs.

## Reading a pack with a hex dump

Using `FG\FISHING1.FG2` as the canonical reference. The file lives on disc
at the sector `mkpsxiso` chose at build time; `CdSearchFile` resolves it
to a `CdlFILE` handle and the runtime reads from there.

The first 40 bytes of the on-disc form should look like:

```text
Offset  Bytes (hex)                                          ASCII
------  ---------------------------------------------------  --------
0x0000  46 47 50 32 01 00 NN NN  XX XX 00 00 NN NN NN NN     FGP2....
0x0008  40 01 C8 00 RR RR RR RR  RR RR RR RR 28 02 00 00     @...........(...
0x0018  TT TT TT TT 00 20 00 00  SS SS SS SS NN NN 00 00     ............
```

Field-by-field:

- `0x00..0x03`: `46 47 50 32` — ASCII `FGP2`.
- `0x04..0x05`: `01 00` — version 1.
- `0x06..0x07`: `NN NN` — frame count, little-endian. For FISHING1 this is
  in the low hundreds; the exact value depends on the captured run length.
- `0x08..0x09`: `XX XX` — `displayVBlanks`, typically 4 or 6 (60 Hz / 4 ≈
  15 Hz, the original screensaver's perceived rate).
- `0x0A..0x0B`: `reserved0`, header flag bits. With base-diff required this
  is at minimum `0x10 0x00` (little-endian `0x0010`).
- `0x0C..0x0D`: `40 01` — `screenWidth = 320` (`0x0140`).
- `0x0E..0x0F`: `C8 00` — `screenHeight = 200` (`0x00C8`).
- `0x10..0x17`: union dirty-rect (X, Y, W, H). FISHING1's union covers the
  pier area, roughly `unionX ≈ 0xA0`, `unionY ≈ 0x60`, `unionWidth ≈
  0x80`, `unionHeight ≈ 0x40` — scene-dependent.
- `0x18..0x1B`: `tableOffset = 0x00000228` (552). After the 40-byte header
  + 512-byte palette = 552, where the entry table starts.
- `0x1C..0x1F`: `dataOffset` — the offset where the frame-data stream
  starts. With `frameCount × 20` entry-table bytes packed in, this lands
  somewhere between 1 KB and 4 KB depending on frame count, and is then
  rounded up to the next 32-bit boundary.
- `0x20..0x23`: `soundEventsOffset` — non-zero if the scene has any
  sound events; zero for silent packs.
- `0x24..0x25`: `soundEventCount` — row count for the sound-event table.

Bytes `0x28..0x227` are the 256-entry palette (512 bytes of BGR555). The
entry table starts at `tableOffset`. From there, every 20 bytes is one
frame entry; `dataOffset` and `dataSize` on each row tell you where to
read the frame payload from.

If the magic at `0x00..0x03` is anything other than `FGP2`, the runtime
will print a `JCFG2: bad magic` line to TTY and refuse to activate the
pack. If `version` is not 1, same thing.

## Where this is used

- **Runtime consumer** —
  [`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c)
  is the one and only PS1-side reader. It opens the file, validates the
  magic, walks the entry table, and presents one diff frame per
  `displayVBlanks` window. The base + diff format means it never has to
  decode a full image after the first frame.
- **Host capture writer** — the host build (under `jc_reborn-host/`)
  runs each ADS scene through SDL2 and emits the FG2 binary plus the
  manifest sidecar. That writer is the source of truth for the byte
  layout — the doc you are reading is a re-narration.
- **Disc image** — `cd_layout.xml` lists every `FG\*.FG2` file
  `mkpsxiso` should stamp onto `jcreborn.bin`. See
  [Build & toolchain]({{ '/docs/build/' | relative_url }}).
- **Manifest** — every pack ships with a JSON sidecar. See
  [FG2 pack manifest]({{ '/docs/formats/pack-manifest/' | relative_url }}).

## Related references

- [FG2 pack manifest]({{ '/docs/formats/pack-manifest/' | relative_url }}) —
  the JSON sidecar that travels with each pack.
- [Dirty region template]({{ '/docs/formats/dirty-region/' | relative_url }}) —
  offline scene-state file consumed alongside a pack.
- [Transition prefetch]({{ '/docs/formats/transition-prefetch/' | relative_url }}) —
  how the runtime decides which pack to start reading next.

## Source on GitHub

- [`docs/ps1/research/PACK_PAYLOAD_LAYOUT.md`]({{ site.github_url }}/blob/main/docs/ps1/research/PACK_PAYLOAD_LAYOUT.md)
- [`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c)
