#!/usr/bin/env python3
"""Pack the boot SOUNDnn.VAG effects into one sector-aligned file.

Boot used to locate+read 23 individual VAG files — 23 directory walks
and seek cycles on a cold drive. SOUNDS.PAK holds them behind ONE
locate: a single 2048-byte header sector, then each VAG payload aligned
to a sector boundary so the aligned CD readers can stream them directly.

Header sector layout (little-endian):
  0   4   magic 'JCSP'
  4   2   version (1)
  6   2   entry count
  8   8*N entries: index u8, pad u8, offsetSectors u16, sizeBytes u32
"""
import re
import struct
import sys
from pathlib import Path

SECTOR = 2048


def main() -> int:
    snd_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(
        "jc_resources/extracted/snd")
    out_path = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(
        "generated/ps1/snd/SOUNDS.PAK")

    vags = []
    for p in sorted(snd_dir.glob("SOUND*.VAG")):
        m = re.fullmatch(r"SOUND(\d\d)\.VAG", p.name)
        if not m:
            continue
        vags.append((int(m.group(1)), p.read_bytes()))
    if not vags:
        print(f"no SOUNDnn.VAG files under {snd_dir}", file=sys.stderr)
        return 1

    header = bytearray(struct.pack("<4sHH", b"JCSP", 1, len(vags)))
    payload = bytearray()
    offset_sectors = 1  # payloads start after the header sector
    for idx, data in vags:
        header += struct.pack("<BBHI", idx, 0, offset_sectors, len(data))
        payload += data
        pad = (-len(data)) % SECTOR
        payload += b"\x00" * pad
        offset_sectors += (len(data) + pad) // SECTOR
    if len(header) > SECTOR:
        print("header overflow", file=sys.stderr)
        return 1
    header += b"\x00" * (SECTOR - len(header))

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(bytes(header) + bytes(payload))
    print(f"{out_path}: {len(vags)} VAGs, {offset_sectors} sectors "
          f"({(len(header) + len(payload)) // 1024} KB)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
