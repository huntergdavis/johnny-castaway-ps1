#!/usr/bin/env python3
"""
Meta-tests for the red-team itself. Each test deliberately introduces a
specific kind of bug into a temporary copy of the source and verifies
that the corresponding red-team check catches it. Restores the original
state after each test.

Run:
  python3 scripts/holidays-redteam-meta.py

Pass/fail = the red-team correctly detects each injected bug.
"""
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
ART_DIR = REPO / "scratch" / "holidays-art"
REDTEAM = REPO / "scripts" / "holidays-redteam.py"


def run_redteam() -> tuple[int, str]:
    """Run the red-team and return (exit_code, stdout)."""
    r = subprocess.run(
        [sys.executable, str(REDTEAM)], cwd=str(REPO),
        capture_output=True, text=True, timeout=60)
    return r.returncode, r.stdout


def expect_fail(check_name: str, stdout: str) -> bool:
    """Check stdout contains a [FAIL] for the named check."""
    return f"[FAIL] {check_name}" in stdout


def test_dim_mismatch():
    """Corrupting a PNG to wrong dimensions should fail
    'PNG presence + dims'."""
    src = ART_DIR / "05-ELVIS_BDAY-v1.png"
    backup_bytes = src.read_bytes()
    try:
        from PIL import Image
        im = Image.new("P", (10, 10), 0)
        im.putpalette([0] * 768)
        im.save(src)
        rc, out = run_redteam()
        return expect_fail("PNG presence + dims", out)
    finally:
        src.write_bytes(backup_bytes)


def test_missing_renderer():
    """Removing a renderer for an existing yaml entry should fail
    'master loader' (or PNG presence + dims after gen)."""
    yaml_src = REPO / "holidays.yml"
    yaml_backup = yaml_src.read_text()
    try:
        # Append a fake holiday that has no renderer.
        yaml_src.write_text(
            yaml_backup + """
- id: 99
  name: "Fake Test Holiday"
  short_name: "FAKE"
  description: "Test."
  date_rule:
    kind: fixed
    month: 1
    day: 30
  sprite:
    width: 64
    height: 48
    island_x: 100
    island_y: 100
  palette: ["#ff0000", "#00ff00", "#0000ff"]
  existing_sprite: null
""")
        rc, out = run_redteam()
        return expect_fail("master loader", out)
    finally:
        yaml_src.write_text(yaml_backup)


def test_invalid_palette_index():
    """A PNG whose pixel data uses palette idx > 15 should fail
    'palette discipline'."""
    src = ART_DIR / "05-ELVIS_BDAY-v1.png"
    backup_bytes = src.read_bytes()
    try:
        from PIL import Image
        im = Image.open(src).copy()
        # Set one pixel to idx 200
        pix = list(im.getdata())
        pix[0] = 200
        im.putdata(pix)
        im.save(src, transparency=0)
        rc, out = run_redteam()
        return expect_fail("palette discipline", out)
    finally:
        src.write_bytes(backup_bytes)


def test_duplicate_art():
    """Two byte-identical PNGs should fail 'no duplicate art'."""
    src1 = ART_DIR / "05-ELVIS_BDAY-v1.png"
    src2 = ART_DIR / "05-ELVIS_BDAY-v2.png"
    backup1 = src1.read_bytes()
    backup2 = src2.read_bytes()
    try:
        # Copy v1 over v2 — now they're identical.
        # But variant_diversity will fail first; fine if either fails.
        # Need same dim though. Inspect.
        from PIL import Image
        a = Image.open(src1)
        b = Image.open(src2)
        if a.size != b.size:
            # Skip — can't trivially make them identical.
            return None
        src1.write_bytes(backup1)
        # Make src2 byte-identical to src1
        shutil.copy(src1, src2)
        rc, out = run_redteam()
        # Variant diversity catches it first; either fail is OK.
        return ("[FAIL] variant diversity" in out
                or "[FAIL] no duplicate art" in out)
    finally:
        src1.write_bytes(backup1)
        src2.write_bytes(backup2)


def test_yaml_oob_screen():
    """Yaml island_xy that pushes sprite off the 640×480 screen
    should fail 'YAML schema'."""
    yaml_src = REPO / "holidays.yml"
    yaml_backup = yaml_src.read_text()
    try:
        # Modify id 5 to have absurdly large island_x.
        new_text = yaml_backup.replace(
            "- id: 5\n  name: \"Elvis's Birthday\"",
            "- id: 5\n  name: \"Elvis's Birthday\"", 1)
        # Hacky: replace the first 'island_x: 388' with 'island_x: 5000'
        new_text = new_text.replace("island_x: 388", "island_x: 5000", 1)
        if new_text == yaml_backup:
            return None
        yaml_src.write_text(new_text)
        rc, out = run_redteam()
        return expect_fail("YAML schema", out)
    finally:
        yaml_src.write_text(yaml_backup)


def test_clean_state_passes():
    """After all the above, clean state should pass."""
    rc, out = run_redteam()
    return rc == 0 and "all checks passed" in out


def main():
    tests = [
        ("dim_mismatch detected by 'PNG presence + dims'",
         test_dim_mismatch),
        ("missing_renderer detected by 'master loader'",
         test_missing_renderer),
        ("invalid_palette_index detected by 'palette discipline'",
         test_invalid_palette_index),
        ("duplicate_art detected by variant/no-duplicate check",
         test_duplicate_art),
        ("yaml_oob_screen detected by 'YAML schema'",
         test_yaml_oob_screen),
        ("clean state still passes",
         test_clean_state_passes),
    ]

    passed = failed = skipped = 0
    print(f"red-team meta-tests ({len(tests)} tests)")
    for name, fn in tests:
        try:
            r = fn()
        except Exception as e:
            r = False
            print(f"  [ERR ] {name}: {e}")
            continue
        if r is None:
            skipped += 1
            print(f"  [SKIP] {name}")
        elif r:
            passed += 1
            print(f"  [ OK ] {name}")
        else:
            failed += 1
            print(f"  [FAIL] {name}")

    print(f"\n{passed} passed, {failed} failed, {skipped} skipped")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
