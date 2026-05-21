#!/usr/bin/env python3
"""Build a foreground read-candidate matrix from read-plan artifacts.

The matrix is host-side only. It aggregates the per-case
foreground-read-plan.json files emitted by ps1-perf-iterate so the next CD
experiments can rank candidates by current scene pressure and visible-cadence
risk instead of hand-picking sector ranges.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_ROOT = (
    REPO_ROOT
    / "scratch/ps1-perf-iterate/building4-restore-minus-current-v087-broad/"
      "20260507-183004-1637867"
)
DEFAULT_CSV = REPO_ROOT / "docs/ps1/performance-read-candidate-matrix.csv"
DEFAULT_MD = REPO_ROOT / "docs/ps1/performance-read-candidate-matrix.md"
DEFAULT_EXPERIMENT_LOG = REPO_ROOT / "docs/ps1/performance-experiment-log.md"
DEFAULT_SCENE_MATRIX = REPO_ROOT / "docs/ps1/performance-scene-matrix.csv"


CSV_FIELDS = [
    "rank",
    "case_label",
    "scene",
    "tide",
    "loop_vb",
    "target_vb",
    "overrun_vb",
    "blocking_vb",
    "prefetch_overrun_vb",
    "due_misses",
    "loop_reads",
    "candidate_size_sectors",
    "start_sector",
    "end_sector",
    "estimated_saved_reads",
    "visible_safety_score",
    "visible_risk_score",
    "visible_cd_cost_class",
    "visible_risk_hint",
    "scheduler_retry_class",
    "first_gap_s",
    "min_internal_gap_s",
    "partial_touch_count",
    "group_overread_sectors",
    "runtime_current_group_fit",
    "prior_experiment_status",
    "phase_trap",
    "phase_trap_reason",
    "next_lane",
    "recommendation",
    "artifact",
]


def rel(path: Path) -> str:
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def safe_int(value: Any, default: int = 0) -> int:
    return value if isinstance(value, int) else default


def safe_float(value: Any) -> float | None:
    if isinstance(value, (int, float)):
        return float(value)
    return None


def load_known_scenes(path: Path = DEFAULT_SCENE_MATRIX) -> list[str]:
    fallback = [
        "activity9",
        "building2",
        "building4",
        "building6",
        "fishing1",
        "fishing3",
        "visitor3",
        "walkstuf1",
    ]
    if not path.exists():
        return fallback

    scenes: set[str] = set()
    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            scene = str(row.get("scene_slug", "")).strip().lower()
            if scene:
                scenes.add(scene)
    return sorted(scenes, key=lambda item: (-len(item), item)) or fallback


def scene_from_label(label: str) -> str:
    if "-high" in label:
        return label.split("-high", 1)[0]
    if "-low" in label:
        return label.split("-low", 1)[0]
    return label.removesuffix("-canary")


def tide_from_label(label: str) -> str:
    return "low" if "-low" in label else "high"


def recommendation(
    candidate: dict[str, Any],
    *,
    overrun_vb: int,
    prior_experiment_status: str,
) -> str:
    cd_class = str(candidate.get("visible_cd_cost_class", ""))
    scheduler_class = str(candidate.get("scheduler_retry_class", ""))
    saved_reads = safe_int(candidate.get("estimated_saved_reads"))
    fits = candidate.get("runtime_current_group_fit") is True

    if prior_experiment_status:
        return "closed-by-experiment-log"
    if overrun_vb <= 0:
        return "defer-under-target"
    if saved_reads <= 0:
        return "reject-no-read-win"
    if not fits:
        return "reject-not-current-window-fit"
    if cd_class.startswith("safe:"):
        return "standalone-probe"
    if cd_class.startswith("balanced:"):
        return "scheduler-or-guarded-probe"
    if scheduler_class == "scheduler-owned-candidate":
        return "scheduler-owned-only"
    if scheduler_class.startswith("high-risk"):
        return "scheduler-owned-only"
    return "reject"


def phase_trap_reason(
    candidate: dict[str, Any],
    *,
    prior_experiment_status: str,
) -> str:
    cd_class = str(candidate.get("visible_cd_cost_class", ""))
    risk_hint = str(candidate.get("visible_risk_hint", ""))
    scheduler_class = str(candidate.get("scheduler_retry_class", ""))
    first_gap = safe_float(candidate.get("first_prev_gap_s"))
    internal_gap = safe_float(candidate.get("min_internal_gap_s"))

    if prior_experiment_status:
        return "closed-exact-range"
    if cd_class.startswith("unsafe:"):
        return "unsafe-visible-cost"
    if scheduler_class.startswith("high-risk"):
        return "high-risk-scheduler"
    if risk_hint.startswith("high-risk"):
        return "high-risk-visible-gap"
    if internal_gap is not None and internal_gap <= 0.12:
        return "tight-internal-gap"
    if first_gap is not None and first_gap <= 0.25:
        return "tight-first-gap"
    return ""


def next_lane(scene: str, tide: str, reason: str) -> str:
    if scene == "visitor3":
        if tide == "low":
            return "custom-terminal-data-shape-or-generated-deadline"
        return "terminal-payload-placement-or-deadline-sidecar"
    if scene == "building2":
        return "frame-deadline-data-shape-or-render-reduction"
    if scene == "walkstuf1":
        if tide == "high":
            return "no-decode-canonicalization-or-generated-owner"
        return "generated-deadline-or-sector-split-data-shape"
    if reason:
        return "non-scalar-data-shape-or-generated-owner"
    return "direct-read-probe"


def load_closed_ranges(path: Path | None) -> set[tuple[str, int, int]]:
    if path is None or not path.exists():
        return set()

    known_scenes = load_known_scenes()
    closed: set[tuple[str, int, int]] = set()
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.lower()
        if (
            "failed/no promotion" not in line
            and "rejected" not in line
            and "do not promote" not in line
            and "close " not in line
        ):
            continue

        for scene in known_scenes:
            if not re.search(rf"(?<![a-z0-9]){re.escape(scene)}(?![a-z0-9])", line):
                continue
            for match in re.finditer(r"(\d+)\s*\.\.\s*(\d+)", line):
                closed.add((scene, int(match.group(1)), int(match.group(2))))
            for match in re.finditer(r"\{(\d+)\s*,\s*(\d+)\}", line):
                closed.add((scene, int(match.group(1)), int(match.group(2))))
    return closed


def iter_rows(root: Path, closed_ranges: set[tuple[str, int, int]]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for path in sorted(root.rglob("foreground-read-plan.json")):
        plan = json.loads(path.read_text(encoding="utf-8"))
        scene_info = plan.get("scene", {})
        label = str(scene_info.get("label", path.parent.name))
        metrics = plan.get("current_metrics", {})
        overrun_vb = safe_int(metrics.get("overrun_vb"))
        if overrun_vb == 0:
            overrun_vb = safe_int(metrics.get("loop_vb")) - safe_int(metrics.get("target_vb"))

        candidate_sets = plan.get("visible_candidate_sets", {})
        if not isinstance(candidate_sets, dict):
            continue

        for size_text, candidates in candidate_sets.items():
            if not isinstance(candidates, list):
                continue
            for candidate in candidates:
                if not isinstance(candidate, dict):
                    continue
                if safe_int(candidate.get("estimated_saved_reads")) <= 0:
                    continue
                if candidate.get("append_start_fireable") is not True:
                    continue

                scene_name = scene_from_label(label)
                start_sector = safe_int(candidate.get("start_sector"))
                end_sector = safe_int(candidate.get("end_sector"))
                prior_experiment_status = (
                    "closed-exact-range"
                    if (scene_name, start_sector, end_sector) in closed_ranges
                    else ""
                )
                row = {
                    "case_label": label,
                    "scene": scene_name,
                    "tide": tide_from_label(label),
                    "loop_vb": safe_int(metrics.get("loop_vb")),
                    "target_vb": safe_int(metrics.get("target_vb")),
                    "overrun_vb": overrun_vb,
                    "blocking_vb": safe_int(metrics.get("blocking_vb")),
                    "prefetch_overrun_vb": safe_int(metrics.get("prefetch_overrun_vb")),
                    "due_misses": safe_int(metrics.get("due_misses")),
                    "loop_reads": safe_int(metrics.get("loop_reads")),
                    "candidate_size_sectors": int(size_text),
                    "start_sector": start_sector,
                    "end_sector": end_sector,
                    "estimated_saved_reads": safe_int(candidate.get("estimated_saved_reads")),
                    "visible_safety_score": safe_int(candidate.get("visible_safety_score")),
                    "visible_risk_score": safe_int(candidate.get("visible_risk_score")),
                    "visible_cd_cost_class": str(candidate.get("visible_cd_cost_class", "")),
                    "visible_risk_hint": str(candidate.get("visible_risk_hint", "")),
                    "scheduler_retry_class": str(candidate.get("scheduler_retry_class", "")),
                    "first_gap_s": safe_float(candidate.get("first_prev_gap_s")),
                    "min_internal_gap_s": safe_float(candidate.get("min_internal_gap_s")),
                    "partial_touch_count": safe_int(candidate.get("partial_touch_count")),
                    "group_overread_sectors": safe_int(candidate.get("group_overread_sectors")),
                    "runtime_current_group_fit": candidate.get("runtime_current_group_fit") is True,
                    "prior_experiment_status": prior_experiment_status,
                    "artifact": rel(path),
                }
                trap_reason = phase_trap_reason(
                    candidate,
                    prior_experiment_status=prior_experiment_status,
                )
                row["phase_trap"] = "yes" if trap_reason else "no"
                row["phase_trap_reason"] = trap_reason
                row["next_lane"] = next_lane(scene_name, str(row["tide"]), trap_reason)
                row["recommendation"] = recommendation(
                    candidate,
                    overrun_vb=overrun_vb,
                    prior_experiment_status=prior_experiment_status,
                )
                rows.append(row)

    def sort_key(row: dict[str, Any]) -> tuple[int, int, int, int, int, int, int]:
        rec_order = {
            "standalone-probe": 0,
            "scheduler-or-guarded-probe": 1,
            "scheduler-owned-only": 2,
            "closed-by-experiment-log": 3,
            "defer-under-target": 4,
        }.get(str(row["recommendation"]), 5)
        return (
            rec_order,
            1 if row["phase_trap"] == "yes" else 0,
            -safe_int(row["overrun_vb"]),
            -safe_int(row["blocking_vb"]),
            -safe_int(row["estimated_saved_reads"]),
            -safe_int(row["visible_safety_score"]),
            safe_int(row["visible_risk_score"]),
        )

    rows.sort(key=sort_key)
    for index, row in enumerate(rows, start=1):
        row["rank"] = index
    return rows


def write_csv(path: Path, rows: list[dict[str, Any]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=CSV_FIELDS, lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def fmt(value: Any) -> str:
    if value is None:
        return ""
    if isinstance(value, float):
        return f"{value:.4g}"
    return str(value)


def write_md(path: Path, root: Path, rows: list[dict[str, Any]], limit: int) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    rec_counts: dict[str, int] = {}
    phase_trap_count = 0
    lane_counts: dict[str, int] = {}
    for row in rows:
        rec_counts[str(row["recommendation"])] = rec_counts.get(str(row["recommendation"]), 0) + 1
        if row["phase_trap"] == "yes":
            phase_trap_count += 1
        lane = str(row["next_lane"])
        lane_counts[lane] = lane_counts.get(lane, 0) + 1
    direct_probe_count = (
        rec_counts.get("standalone-probe", 0)
        + rec_counts.get("scheduler-or-guarded-probe", 0)
    )
    top_lanes = sorted(lane_counts.items(), key=lambda pair: (-pair[1], pair[0]))[:5]
    lane_summary = ", ".join(f"`{lane}`={count}" for lane, count in top_lanes) or "`none`"

    lines = [
        "# PS1 Foreground Read Candidate Matrix",
        "",
        "This host-side report aggregates the current `foreground-read-plan.json`",
        "artifacts and ranks candidate retained-window read groups by scene",
        "pressure and visible-cadence risk. It does not change the PS1 binary.",
        "",
        f"- Source artifact root: `{rel(root)}`",
        f"- Candidate rows: `{len(rows)}`",
        f"- Standalone probes: `{rec_counts.get('standalone-probe', 0)}`",
        f"- Scheduler or guarded probes: `{rec_counts.get('scheduler-or-guarded-probe', 0)}`",
        f"- Scheduler-owned only: `{rec_counts.get('scheduler-owned-only', 0)}`",
        f"- Closed exact ranges from experiment log: `{rec_counts.get('closed-by-experiment-log', 0)}`",
        f"- Phase-trap rows: `{phase_trap_count}`",
        f"- Deferred under-target rows: `{rec_counts.get('defer-under-target', 0)}`",
        f"- Top next lanes: {lane_summary}",
        "",
        "Recent hand-authored table probes proved that nominal read-count wins can",
        "still regress `loop_vb` and visible `blocking_vb`. Treat `risky` and",
        "`unsafe` rows as scheduler-owned retries, not standalone table changes.",
        "When direct standalone/guarded probes are exhausted, promote the listed",
        "next lanes above more scalar range retries.",
        "",
    ]

    if rows and direct_probe_count == 0:
        lines.extend([
            "No open standalone or guarded direct-read probes remain in this",
            "artifact set. The next optimization pass should start from generated",
            "deadline ownership, custom data-shape, or pack-owned work reduction",
            "lanes instead of another hand-authored sector range.",
            "",
        ])

    lines.extend([
        f"## Top {min(limit, len(rows))} Candidates",
        "",
        "| Rank | Scene | Tide | Loop/Target | Blocking | Range | Saved | Cost Class | Phase Trap | Next Lane | Recommendation |",
        "|---:|---|---|---:|---:|---|---:|---|---|---|---|",
    ])

    for row in rows[:limit]:
        lines.append(
            "| {rank} | `{scene}` | `{tide}` | {loop}/{target} | {blocking} | "
            "`{start}..{end}` ({size}s) | {saved} | `{cost}` | `{trap}` | `{lane}` | `{rec}` |".format(
                rank=row["rank"],
                scene=row["scene"],
                tide=row["tide"],
                loop=row["loop_vb"],
                target=row["target_vb"],
                blocking=row["blocking_vb"],
                start=row["start_sector"],
                end=row["end_sector"],
                size=row["candidate_size_sectors"],
                saved=row["estimated_saved_reads"],
                cost=row["visible_cd_cost_class"],
                trap=row["phase_trap_reason"] or "no",
                lane=row["next_lane"],
                rec=row["recommendation"],
            )
        )

    lines.extend([
        "",
        "## CSV",
        "",
        "The full row-level matrix is in",
        "`docs/ps1/performance-read-candidate-matrix.csv`.",
        "",
        "## Columns",
        "",
        "- `recommendation=standalone-probe` means the candidate has saved reads,",
        "  fits the current group window, and has a `safe` visible cost class.",
        "- `recommendation=scheduler-or-guarded-probe` means the candidate is",
        "  balanced but still needs either a fresh paired gate or scheduler guard.",
        "- `recommendation=scheduler-owned-only` means prior misses say a raw table",
        "  is too risky; use generated metadata with explicit CD ownership.",
        "- `recommendation=closed-by-experiment-log` means the exact sector range",
        "  already appears in a failed or rejected experiment row.",
        "- `recommendation=defer-under-target` means the source scene is already",
        "  under its current active-loop target.",
        "- `phase_trap=yes` marks rows whose exact range is closed, whose visible",
        "  gap is too tight, or whose prior risk class says scheduler ownership",
        "  is required before the read can fire safely.",
        "- `next_lane` is the non-scalar lane to try before another local sector",
        "  table retry for that row.",
        "- `artifact` points back to the source read-plan JSON for full read",
        "  segments, gaps, and coverage.",
        "",
    ])
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=DEFAULT_ROOT)
    parser.add_argument("--csv", type=Path, default=DEFAULT_CSV)
    parser.add_argument("--md", type=Path, default=DEFAULT_MD)
    parser.add_argument("--experiment-log", type=Path, default=DEFAULT_EXPERIMENT_LOG)
    parser.add_argument("--top", type=int, default=40)
    args = parser.parse_args()

    root = args.root.resolve()
    if not root.exists():
        raise SystemExit(f"artifact root not found: {root}")

    closed_ranges = load_closed_ranges(args.experiment_log.resolve())
    rows = iter_rows(root, closed_ranges)
    write_csv(args.csv.resolve(), rows)
    write_md(args.md.resolve(), root, rows, args.top)
    print(f"wrote {len(rows)} rows to {rel(args.csv.resolve())}")
    print(f"wrote report to {rel(args.md.resolve())}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
