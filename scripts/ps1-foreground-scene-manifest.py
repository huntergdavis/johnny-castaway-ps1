#!/usr/bin/env python3
"""Maintain PS1 foreground scene routing/test metadata.

This is host-side tooling. It derives the 63 scene slugs and their high/low
FG2 pack names from config/ps1/regtest-scenes.txt, then can update the PS1 CD
layout, print ps1-perf-iterate cases, or write the all-scene performance sheet.
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import random
import re
import subprocess
from datetime import datetime
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SCENE_LIST = REPO_ROOT / "config" / "ps1" / "regtest-scenes.txt"
DEFAULT_PACK_DIR = REPO_ROOT / "generated" / "ps1" / "foreground"
DEFAULT_CD_LAYOUT = REPO_ROOT / "config" / "ps1" / "cd_layout.xml"
DEFAULT_SHEET = REPO_ROOT / "docs" / "ps1" / "performance-scene-matrix.csv"

ADS_ABBREV = {
    "ACTIVITY": "ACTV",
    "BUILDING": "BUIL",
    "FISHING": "FISH",
    "JOHNNY": "JOHN",
    "MARY": "MARY",
    "MISCGAG": "MISC",
    "STAND": "STND",
    "SUZY": "SUZY",
    "VISITOR": "VIST",
    "WALKSTUF": "WALK",
}

HIGH_CD_PREFIX = {
    "ACTIVITY": "ACTV",
    "BUILDING": "BUIL",
    "FISHING": "FISHING",
    "JOHNNY": "JOHNNY",
    "MARY": "MARY",
    "MISCGAG": "MISCGAG",
    "STAND": "STAND",
    "SUZY": "SUZY",
    "VISITOR": "VISITOR",
    "WALKSTUF": "WALK",
}


@dataclass(frozen=True)
class SceneRecord:
    ads: str
    tag: int
    slug: str
    high_source: str
    low_source: str
    high_cd_name: str
    low_cd_name: str


def low_pack_basename(ads: str, tag: int) -> str:
    base = f"{ADS_ABBREV.get(ads, ads[:4])}{tag}"
    long_name = f"{base}LOW"
    return f"{base}L" if len(long_name) > 8 else long_name


def high_cd_basename(ads: str, tag: int) -> str:
    return f"{HIGH_CD_PREFIX.get(ads, ads)}{tag}"


def parse_scene_list(path: Path) -> list[SceneRecord]:
    records: list[SceneRecord] = []
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.split("#", 1)[0].strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        ads = parts[0].upper()
        try:
            tag = int(parts[1])
        except ValueError:
            continue
        slug = f"{ads.lower()}{tag}"
        high_source = f"{ads}{tag}.FG2"
        low_source = f"{low_pack_basename(ads, tag)}.FG2"
        high_cd_name = f"{high_cd_basename(ads, tag)}.FG2"
        low_cd_name = low_source
        records.append(SceneRecord(
            ads=ads,
            tag=tag,
            slug=slug,
            high_source=high_source,
            low_source=low_source,
            high_cd_name=high_cd_name,
            low_cd_name=low_cd_name,
        ))
    return records


def ordered_for_cd(records: list[SceneRecord]) -> list[tuple[SceneRecord, str]]:
    priority = {("FISHING", 1): 0, ("FISHING", 2): 1, ("FISHING", 3): 2}
    pairs: list[tuple[SceneRecord, str]] = []
    for record in sorted(
        records,
        key=lambda item: (priority.get((item.ads, item.tag), 1000), item.ads, item.tag),
    ):
        pairs.append((record, "high"))
        pairs.append((record, "low"))
    return pairs


def xml_file_line(record: SceneRecord, tide: str) -> str:
    if tide == "high":
        cd_name = record.high_cd_name
        source = record.high_source
    else:
        cd_name = record.low_cd_name
        source = record.low_source
    return (
        f'                <file name="{cd_name}" type="data" '
        f'source="../../generated/ps1/foreground/{source}"/>'
    )


def update_cd_layout(path: Path, records: list[SceneRecord], pack_dir: Path) -> None:
    missing: list[str] = []
    for record in records:
        for filename in (record.high_source, record.low_source):
            if not (pack_dir / filename).is_file():
                missing.append(filename)
    if missing:
        raise SystemExit("missing foreground packs: " + ", ".join(missing))

    text = path.read_text(encoding="utf-8")
    block = "\n".join(xml_file_line(record, tide) for record, tide in ordered_for_cd(records))
    replacement = f'            <dir name="FG">\n{block}\n            </dir>'
    new_text, count = re.subn(
        r'            <dir name="FG">\n.*?\n            </dir>',
        replacement,
        text,
        count=1,
        flags=re.S,
    )
    if count != 1:
        raise SystemExit(f"could not replace FG directory block in {path}")
    path.write_text(new_text, encoding="utf-8")


def parse_boot(boot: str) -> tuple[str | None, int]:
    tokens = boot.split()
    scene: str | None = None
    lowtide = 0
    for i, token in enumerate(tokens):
        if token == "fgpilot" and i + 1 < len(tokens):
            scene = tokens[i + 1]
        if token == "lowtide" and i + 1 < len(tokens):
            try:
                lowtide = int(tokens[i + 1])
            except ValueError:
                lowtide = 0
    return scene, 1 if lowtide else 0


def repo_relative(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def run_timestamp_from_summary(path: Path, path_mtime: float | None = None) -> str:
    match = re.search(r"(\d{8})-(\d{6})", str(path))
    if match:
        date, clock = match.groups()
        return (
            f"{date[0:4]}-{date[4:6]}-{date[6:8]}T"
            f"{clock[0:2]}:{clock[2:4]}:{clock[4:6]}"
        )
    if path_mtime is None:
        return ""
    return datetime.fromtimestamp(path_mtime).isoformat(timespec="seconds")


def load_summary_cases(path: Path) -> list[dict]:
    summary = json.loads(path.read_text(encoding="utf-8"))
    cases = summary.get("cases")
    if isinstance(cases, list):
        return [case for case in cases if isinstance(case, dict)]
    if "boot" in summary and "sections" in summary:
        return [summary]
    return []


def load_summary_metrics(paths: list[Path]) -> dict[tuple[str, str], dict[str, str]]:
    metrics: dict[tuple[str, str], dict[str, str]] = {}
    metric_mtimes: dict[tuple[str, str], float] = {}
    for path in paths:
        if not path.is_file():
            continue
        path_mtime = path.stat().st_mtime
        for case in load_summary_cases(path):
            if case.get("gate", {}).get("pass") is False:
                continue
            boot = str(case.get("boot", ""))
            scene, lowtide = parse_boot(boot)
            if not scene:
                continue
            tide = "low" if lowtide else "high"
            metric_key = (scene, tide)
            if metric_key in metric_mtimes and path_mtime < metric_mtimes[metric_key]:
                continue
            sections = case.get("sections", {})
            timing = sections.get("timing", {})
            cd = sections.get("cd", {})
            prefetch = sections.get("prefetch", {})
            scene_info = sections.get("scene", {})
            loop_vb = timing.get("loop_vb")
            target_vb = timing.get("target_vb")
            over_target = ""
            over_percent = ""
            if isinstance(loop_vb, int) and isinstance(target_vb, int) and loop_vb > 0 and target_vb > 0:
                over_target = str(loop_vb - target_vb)
                over_percent = f"{((loop_vb - target_vb) * 100.0 / target_vb):.2f}"
            notes = ""
            if not isinstance(loop_vb, int) or not isinstance(target_vb, int) or loop_vb <= 0 or target_vb <= 0:
                notes = "metadata-only; no active-loop timing; excluded from speed averages"
            metrics[metric_key] = {
                "boot": boot,
                "pack_bytes": str(scene_info.get("pack_bytes", "")),
                "last_summary": repo_relative(path),
                "last_run_at": run_timestamp_from_summary(path, path_mtime),
                "loop_vb": str(loop_vb) if isinstance(loop_vb, int) else "",
                "target_vb": str(target_vb) if isinstance(target_vb, int) else "",
                "over_target_vb": over_target,
                "over_target_percent": over_percent,
                "blocking_vb": str(cd.get("blocking_vb", "")),
                "prefetch_overrun_vb": str(prefetch.get("overrun_vb", "")),
                "loop_reads": str(cd.get("loop_reads", "")),
                "loop_read_vb": str(cd.get("loop_read_vb", "")),
                "due_misses": str(prefetch.get("due_misses", "")),
                "pack_lba": str(scene_info.get("pack_lba", "")),
                "pack_sectors": str(scene_info.get("pack_sectors", "")),
                "status": "measured",
                "notes": notes,
            }
            metric_mtimes[metric_key] = path_mtime
    return metrics


def load_existing_sheet_metrics(path: Path | None) -> dict[tuple[str, str], dict[str, str]]:
    if path is None or not path.is_file():
        return {}

    metrics: dict[tuple[str, str], dict[str, str]] = {}
    preserved_fields = {
        "boot",
        "pack_bytes",
        "last_summary",
        "last_run_at",
        "stats_version",
        "loop_vb",
        "target_vb",
        "over_target_vb",
        "over_target_percent",
        "blocking_vb",
        "prefetch_overrun_vb",
        "loop_reads",
        "loop_read_vb",
        "due_misses",
        "pack_lba",
        "pack_sectors",
        "status",
        "notes",
    }
    with path.open("r", encoding="utf-8", newline="") as fh:
        for row in csv.DictReader(fh):
            status = row.get("status", "")
            if status == "pending":
                continue
            scene = row.get("scene_slug", "")
            tide = row.get("tide", "")
            if not scene or not tide:
                continue
            metrics[(scene, tide)] = {
                key: row.get(key, "")
                for key in preserved_fields
                if key in row
            }
    return metrics


def default_stats_version() -> str:
    override = os.environ.get("PS1_PERF_STATS_VERSION", "").strip()
    if override:
        return override
    try:
        commit = subprocess.check_output(
            ["git", "rev-parse", "--short=8", "HEAD"],
            cwd=REPO_ROOT,
            text=True,
        ).strip()
    except Exception:
        return ""
    return f"git:{commit}"


def boot_for(record: SceneRecord, tide: str) -> str:
    lowtide = "1" if tide == "low" else "0"
    return (
        f"fgpilot {record.slug} lowtide {lowtide} night 1 holiday 0 "
        "raft-stage 4 island-pos -154 54 perf-log noloop seed 1"
    )


def sheet_rows(
    records: list[SceneRecord],
    metrics: dict[tuple[str, str], dict[str, str]],
    pack_dir: Path,
    stats_version: str,
) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for record in records:
        for tide in ("high", "low"):
            cd_name = record.high_cd_name if tide == "high" else record.low_cd_name
            source = record.high_source if tide == "high" else record.low_source
            measured = metrics.get((record.slug, tide), {})
            row = {
                "scene_slug": record.slug,
                "ads": record.ads,
                "tag": str(record.tag),
                "tide": tide,
                "boot": boot_for(record, tide),
                "cd_pack": f"FG\\{cd_name}",
                "source_pack": f"generated/ps1/foreground/{source}",
                "pack_bytes": str((pack_dir / source).stat().st_size) if (pack_dir / source).is_file() else "",
                "last_summary": "",
                "last_run_at": "",
                "stats_version": "",
                "loop_vb": "",
                "target_vb": "",
                "over_target_vb": "",
                "over_target_percent": "",
                "blocking_vb": "",
                "prefetch_overrun_vb": "",
                "loop_reads": "",
                "loop_read_vb": "",
                "due_misses": "",
                "pack_lba": "",
                "pack_sectors": "",
                "status": "pending",
                "notes": "",
            }
            row.update(measured)
            if measured and not row.get("stats_version"):
                row["stats_version"] = stats_version
            rows.append(row)
    return rows


def write_sheet(path: Path, records: list[SceneRecord], summaries: list[Path],
                pack_dir: Path, stats_version: str,
                merge_existing_sheet: Path | None = None) -> None:
    metrics = load_existing_sheet_metrics(merge_existing_sheet)
    metrics.update(load_summary_metrics(summaries))
    rows = sheet_rows(records, metrics, pack_dir, stats_version)
    path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = list(rows[0].keys()) if rows else []
    with path.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def load_measured_keys(path: Path | None) -> set[tuple[str, str]]:
    measured: set[tuple[str, str]] = set()
    if path is None or not path.is_file():
        return measured
    with path.open("r", encoding="utf-8", newline="") as fh:
        for row in csv.DictReader(fh):
            if row.get("status") != "measured":
                continue
            scene = row.get("scene_slug", "")
            tide = row.get("tide", "")
            if scene and tide:
                measured.add((scene, tide))
    return measured


def print_cases(
    records: list[SceneRecord],
    tides: str,
    order: str,
    limit: int | None,
    seed: int,
    skip_measured_from: Path | None,
) -> None:
    case_rows: list[tuple[str, str]] = []
    measured = load_measured_keys(skip_measured_from)
    selected_tides = ("high", "low") if tides == "both" else (tides,)
    for record in records:
        for tide in selected_tides:
            if (record.slug, tide) in measured:
                continue
            label = f"{record.slug}-{tide}"
            case_rows.append((label, boot_for(record, tide)))
    if order == "random":
        rng = random.Random(seed)
        rng.shuffle(case_rows)
    if limit is not None:
        case_rows = case_rows[:limit]
    for label, boot in case_rows:
        print(f"{label}\t{boot}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene-list", type=Path, default=DEFAULT_SCENE_LIST)
    parser.add_argument("--pack-dir", type=Path, default=DEFAULT_PACK_DIR)
    parser.add_argument("--write-cd-layout", type=Path)
    parser.add_argument("--write-sheet", type=Path)
    parser.add_argument(
        "--merge-existing-sheet",
        type=Path,
        help="Preserve measured/blocked rows from this sheet unless a newer summary replaces them.",
    )
    parser.add_argument("--summary", action="append", type=Path, default=[])
    parser.add_argument("--stats-version", default=default_stats_version())
    parser.add_argument("--print-cases", action="store_true")
    parser.add_argument("--tides", choices=("high", "low", "both"), default="both")
    parser.add_argument("--order", choices=("list", "random"), default="list")
    parser.add_argument("--limit", type=int)
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--skip-measured-from", type=Path)
    args = parser.parse_args()

    records = parse_scene_list(args.scene_list)
    if args.write_cd_layout:
        update_cd_layout(args.write_cd_layout, records, args.pack_dir)
    if args.write_sheet:
        write_sheet(
            args.write_sheet,
            records,
            args.summary,
            args.pack_dir,
            args.stats_version,
            args.merge_existing_sheet,
        )
    if args.print_cases:
        print_cases(records, args.tides, args.order, args.limit, args.seed, args.skip_measured_from)
    if not (args.write_cd_layout or args.write_sheet or args.print_cases):
        print(json.dumps([record.__dict__ for record in records], indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
