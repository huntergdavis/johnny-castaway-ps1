#!/usr/bin/env python3
"""Build the website menu guide from a scripted PS1 regtest run."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class ScreenDoc:
    label: str
    title: str
    route: str
    description: str
    notes: str


SCREENS: tuple[ScreenDoc, ...] = (
    ScreenDoc(
        "pause-main",
        "Pause Menu",
        "Start",
        "The top-level dispatch screen: resume, open Scene Set Options or Scene Explorer (both new in v0.8.4-ps1), enter or exit Freeplay, open Freeplay Options, World Options, Accessibility, or System.",
        "This screen is intentionally short. Anything that grows past a few rows belongs on a sub-screen.",
    ),
    ScreenDoc(
        "scene-set",
        "Scene Set Options",
        "Start, Down, Cross",
        "Scene-pool selector and picker policy in one place: pick a Scene Set (All Scenes, Fishing Only, Johnny Stories, Mary Visits, Visitors, Activities, or Misc & Suzy) and choose Random / Sequential / Original Sierra dispatch.",
        "Scene Set commits on Cross or Start so unsubmitted previews never linger.",
    ),
    ScreenDoc(
        "scene-explorer",
        "Scene Explorer",
        "Start, Down, Down, Cross",
        "The chapter-select grid. Each entry shows a captured-on-PS1 thumbnail, scene title, family, frame count, and pack name, with LEFT/RIGHT to step one scene and L1/R1 to step one family.",
        "Cross plays the highlighted scene once; Triangle loops it; Circle/Start backs out. New in v0.8.4-ps1.",
    ),
    ScreenDoc(
        "freeplay-options",
        "Freeplay Options",
        "Start, Down four times, Cross",
        "The Freeplay debug entry page: gag catalog, visitor catalog, controls, and the clear-screen rebuild action.",
        "Freeplay keeps the live joypad simple. Catalog-like actions live here, where they can be named and described.",
    ),
    ScreenDoc(
        "freeplay-gags",
        "Freeplay Gags",
        "Freeplay Options, Cross",
        "Selector for direct Johnny actions. Each entry shows the source bitmap, frame count, rough RAM cost, and a one-line behavior note.",
        "The screenshot shows the first entry only; the harness does not capture every gag row.",
    ),
    ScreenDoc(
        "freeplay-visitors",
        "Freeplay Visitors",
        "Freeplay Options, Down, Cross",
        "Selector for external events and visitors, with the same asset metadata as the gag catalog.",
        "Missing optional assets are meant to fail soft: the menu names the asset, the runtime skips cleanly.",
    ),
    ScreenDoc(
        "controls",
        "Controls",
        "Freeplay Options, Down, Down, Cross",
        "The on-disc reminder for Freeplay controls: walking, speed modifiers, fishing, clear screen, world toggles, and pause.",
        "Circle is Back everywhere in the menu. Cross is Select everywhere in the menu.",
    ),
    ScreenDoc(
        "world-options",
        "World Options",
        "Start, Down five times, Cross",
        "Visual state controls: day or night, tide, raft stage, holiday selector, island-position editor, and Back.",
        "In Freeplay, these settings use the same rebuild path as the live R1 shortcuts so the island changes immediately.",
    ),
    ScreenDoc(
        "holidays",
        "Holidays",
        "World Options, Down, Down, Down, Cross",
        "Holiday mode and forced holiday selection: Auto Date, None, Original 4, or Expanded calendar.",
        "This is the manual side of the same date resolver used by the soft date picker.",
    ),
    ScreenDoc(
        "island-position",
        "Set Island Position",
        "World Options, Down, Down, Down, Down, Cross",
        "Manual X/Y offset editor for the island anchor, plus an Auto/Manual mode toggle.",
        "This is mostly a development and placement tool, but it is kept in the player menu because it is useful on real hardware too.",
    ),
    ScreenDoc(
        "accessibility",
        "Accessibility",
        "Start, Down six times, Cross",
        "Captions, sound, footsteps, Sound Test, and Back.",
        "Captions share the pause-menu font atlas so they can draw before the pause menu has ever been opened.",
    ),
    ScreenDoc(
        "sound-test",
        "Sound Test",
        "Accessibility, Down, Down, Down, Cross",
        "A selector for individual SPU sound effects: choose an effect, see whether it is present, and play it on demand.",
        "This turns audio debugging into a deterministic menu operation instead of a waiting game.",
    ),
    ScreenDoc(
        "system",
        "System",
        "Start, Down seven times, Cross",
        "Save settings, set time/date, set RNG seed, cycle perf logging, reset current scene, or advance to the next scene.",
        "System keeps less frequent operations away from the high-use visual and Freeplay screens.",
    ),
    ScreenDoc(
        "set-time-date",
        "Set Time And Date",
        "System, Down, Cross",
        "Software clock editor. The date drives holiday lookup and lets testers jump directly to seasonal overlays.",
        "Confirming a date clears forced holiday selection so Auto Date can take over.",
    ),
    ScreenDoc(
        "set-rng-seed",
        "Set RNG Seed",
        "System, Down, Down, Cross",
        "Deterministic random-seed editor for repeatable visual tests and bug reports.",
        "Shoulder buttons use larger steps so a tester can move quickly without a keyboard.",
    ),
)


MARKER_RE = re.compile(r"JCPADSHOT\s+label=([A-Za-z0-9_.-]+)\s+frame=(\d+)(?:\s+tick=(\d+))?")
FRAME_RE = re.compile(r"frame[_-](\d+)\.png$", re.IGNORECASE)


def project_root() -> Path:
    return Path(__file__).resolve().parents[1]


def parse_markers(log_path: Path) -> dict[str, dict[str, int]]:
    markers: dict[str, dict[str, int]] = {}
    if not log_path.is_file():
        return markers
    for line in log_path.read_text(encoding="utf-8", errors="ignore").splitlines():
        match = MARKER_RE.search(line)
        if match:
            script_frame = int(match.group(2))
            target_frame = int(match.group(3)) if match.group(3) else script_frame
            markers[match.group(1)] = {
                "script_frame": script_frame,
                "target_frame": target_frame,
            }
    return markers


def frame_number(path: Path) -> int | None:
    match = FRAME_RE.search(path.name)
    if not match:
        return None
    return int(match.group(1))


def index_frames(frames_dir: Path) -> list[tuple[int, Path]]:
    frames: list[tuple[int, Path]] = []
    for path in frames_dir.rglob("frame_*.png"):
        number = frame_number(path)
        if number is not None:
            frames.append((number, path))
    frames.sort(key=lambda item: item[0])
    return frames


def nearest_frame(frames: list[tuple[int, Path]], target: int) -> tuple[int, Path, int] | None:
    if not frames:
        return None
    number, path = min(frames, key=lambda item: abs(item[0] - target))
    return number, path, abs(number - target)


def frame_at_or_after(frames: list[tuple[int, Path]], target: int) -> tuple[int, Path, int] | None:
    for number, path in frames:
        if number >= target:
            return number, path, number - target
    return nearest_frame(frames, target)


def rel_asset(label: str) -> str:
    return f"/assets/img/help/menu/{label}.png"


def build_markdown(captures: dict[str, dict[str, object]]) -> str:
    lines: list[str] = [
        "---",
        "layout: page",
        "title: Menu help guide",
        "eyebrow: Help",
        "subtitle: Every top-level pause-menu screen, captured by the headless scripted-input harness.",
        'description: "Johnny Castaway PS1 menu help guide with headless regtest screenshots and control descriptions."',
        "---",
        "",
        "A labor of love by Hunter Davis. This page is generated from the same PS1 build that players run: the headless DuckStation harness boots the disc, waits for the game to settle, presses controller buttons, and captures each menu screen from the emulator framebuffer.",
        "",
        "The point is not just documentation. It is a test. If Start stops opening the menu, if Circle stops backing out, if a sub-screen runs off the panel, or if a future refactor breaks controller input, this page stops regenerating cleanly.",
        "",
        "## The route",
        "",
        "The default capture script starts in normal screensaver playback, waits 30 seconds, opens the pause menu, then walks each major screen with D-pad, Cross, and Circle. It intentionally captures catalog entry pages once, not every gag or visitor row.",
        "",
        '<div class="menu-guide">',
    ]

    for screen in SCREENS:
        capture = captures.get(screen.label, {})
        image_exists = bool(capture.get("image"))
        marker = capture.get("marker_frame")
        source = capture.get("source_frame")
        distance = capture.get("distance")

        lines.extend(
            [
                f'<section class="menu-shot" id="{screen.label}">',
                f"<h2>{screen.title}</h2>",
                f"<p>{screen.description}</p>",
                f'<p class="menu-route"><strong>Capture route:</strong> {screen.route}</p>',
            ]
        )
        if image_exists:
            src = "{{ '" + rel_asset(screen.label) + "' | relative_url }}"
            alt = f"Captured PS1 screenshot of the {screen.title} screen."
            lines.extend(
                [
                    "<figure>",
                    f'  <img src="{src}" alt="{alt}" loading="lazy" />',
                    f"  <figcaption>Marker frame {marker}, captured frame {source}, delta {distance}.</figcaption>",
                    "</figure>",
                ]
            )
        else:
            lines.append('<p class="menu-missing">Screenshot missing from the last harness run.</p>')
        lines.extend([f"<p>{screen.notes}</p>", "</section>", ""])

    lines.extend(
        [
            "</div>",
            "",
            "## Regenerating",
            "",
            "```bash",
            "./scripts/ps1-menu-input-harness.sh",
            "```",
            "",
            "The runner temporarily writes `BOOTMODE.TXT` and `PADSCRIPT.TXT`, rebuilds the PS1 image, runs DuckStation regtest headlessly, copies the first captured frame at or after every delayed `JCPADSHOT` marker into `site/assets/img/help/menu/`, and rewrites this page.",
            "",
            "Related references: [Pause menu]({{ '/docs/pause-menu/' | relative_url }}), [Freeplay and debug mode]({{ '/docs/freeplay/' | relative_url }}), and [Regression testing]({{ '/docs/regtest/' | relative_url }}).",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_dir", nargs="?", type=Path, help="regtest run directory containing regtest.log and frames/")
    parser.add_argument("--site-dir", type=Path, default=project_root() / "site")
    parser.add_argument("--max-distance", type=int, default=12)
    parser.add_argument("--allow-missing", action="store_true")
    args = parser.parse_args()

    site_dir = args.site_dir.resolve()
    asset_dir = site_dir / "assets/img/help/menu"
    page_path = site_dir / "help/menu/index.md"
    manifest_path = asset_dir / "manifest.json"
    captures: dict[str, dict[str, object]] = {}
    missing: list[str] = []

    if args.run_dir is not None:
        run_dir = args.run_dir.resolve()
        markers = parse_markers(run_dir / "regtest.log")
        frames = index_frames(run_dir / "frames")

        asset_dir.mkdir(parents=True, exist_ok=True)
        for screen in SCREENS:
            marker = markers.get(screen.label)
            if marker is None:
                missing.append(screen.label)
                continue
            nearest = frame_at_or_after(frames, marker["target_frame"])
            if nearest is None:
                missing.append(screen.label)
                continue
            source_frame, source_path, distance = nearest
            if distance > args.max_distance:
                missing.append(screen.label)
                continue
            dest = asset_dir / f"{screen.label}.png"
            shutil.copy2(source_path, dest)
            captures[screen.label] = {
                "title": screen.title,
                "marker_frame": marker["target_frame"],
                "script_frame": marker["script_frame"],
                "source_frame": source_frame,
                "distance": distance,
                "source_path": str(source_path),
                "image": str(dest),
            }
    else:
        asset_dir.mkdir(parents=True, exist_ok=True)
        for screen in SCREENS:
            image_path = asset_dir / f"{screen.label}.png"
            if image_path.is_file():
                captures[screen.label] = {
                    "title": screen.title,
                    "image": str(image_path),
                    "marker_frame": "existing",
                    "source_frame": "existing",
                    "distance": 0,
                }
            else:
                missing.append(screen.label)

    page_path.parent.mkdir(parents=True, exist_ok=True)
    page_path.write_text(build_markdown(captures), encoding="utf-8")
    manifest_path.write_text(json.dumps(captures, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if missing and not args.allow_missing:
        print("Missing menu captures: " + ", ".join(missing), file=sys.stderr)
        return 1
    if missing:
        print("WARNING: missing menu captures: " + ", ".join(missing), file=sys.stderr)

    print(f"Wrote {page_path}")
    print(f"Wrote {manifest_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
