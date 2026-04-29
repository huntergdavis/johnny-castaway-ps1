#!/usr/bin/env python3
"""Build a host-side retry manifest for PS1 performance experiments.

This script is intentionally host-only. It mines the durable markdown
experiment log, classifies failed/no-promotion experiments by retry condition,
and can attach current scene read-cost profiles from existing perf summaries.
It does not run DuckStation and does not change the PS1 binary.
"""

from __future__ import annotations

import argparse
import json
import re
from collections import Counter, defaultdict
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_LOG = REPO_ROOT / "docs" / "ps1" / "performance-experiment-log.md"
DEFAULT_JSONL = REPO_ROOT / "scratch" / "ps1-perf-iterate" / "experiments.jsonl"

LOWER_BETTER_PREFIXES = (
    "loop",
    "target",
    "overrun",
    "blocking",
    "prefetch_overrun",
    "due_miss",
    "read",
    "restore",
    "upload",
    "dirty",
    "rect",
    "byte",
    "sector",
    "setup",
    "scene",
    "exe",
    "elf",
    "fallback",
)
HIGHER_BETTER_PREFIXES = (
    "hit",
    "hits",
    "prefetch_hit",
    "stage_hit",
    "window_hit",
    "group_hit",
    "hidden",
)

GROUPS: list[tuple[str, int, tuple[str, ...]]] = [
    (
        "evidence",
        1,
        (
            "manifest",
            "classifier",
            "histogram",
            "map",
            "host-side",
            "post-processing",
            "read-cost",
            "delivered-sector",
            "read-plan",
            "cdlog",
        ),
    ),
    (
        "offset-stable-metadata",
        2,
        (
            "metadata",
            "sidecar",
            "footer",
            "metadata prefix",
            "offset",
            "sector crossing",
            "generated policy",
        ),
    ),
    (
        "generated-cd-setup",
        3,
        (
            "read group",
            "read groups",
            "group",
            "setup-prime",
            "setup segment",
            "segment",
            "pack group",
            "window",
            "refill",
            "cd pressure",
            "loop_reads",
            "seek",
        ),
    ),
    (
        "scheduler-read-policy",
        4,
        (
            "scheduler",
            "prepared",
            "present",
            "slack",
            "fallthrough",
            "direct-stage",
            "stage",
            "catch-up",
            "async",
            "ownership",
        ),
    ),
    (
        "scene-transition-hiding",
        5,
        (
            "scene_vb",
            "setup_vb",
            "inter-scene",
            "transition",
            "preload",
            "setup settle",
        ),
    ),
    (
        "pack-time-graphics",
        6,
        (
            "pal4",
            "compositor",
            "upload",
            "restore",
            "dirty-row",
            "dirty row",
            "direct16",
            "moveimage",
            "gpu",
            "pack-time",
            "scratch",
            "rect",
        ),
    ),
    (
        "layout-binary-cleanup",
        7,
        (
            "lba",
            "layout",
            "phase",
            "padding",
            "ps-exe",
            "executable",
            "-os",
            "-o3",
            "compile-gate",
            "diagnostic",
            "code-size",
            "elf",
        ),
    ),
    (
        "validation-guardrails",
        8,
        (
            "correctness",
            "visual",
            "work identity",
            "identity",
            "hash",
            "fallback",
            "frame",
            "sound",
        ),
    ),
    (
        "architectural-branch",
        9,
        (
            "gpu-sprite",
            "payload reorder",
            "duplicate tiny",
            "architectural",
            "move/residual",
        ),
    ),
]

METRIC_RE = re.compile(r"`?([A-Za-z0-9_.\-/]+)`?\s+`?(-?\d+)`?\s*->\s*`?(-?\d+)`?")
RETRY_SENTENCE_RE = re.compile(r"([^.!?]*\bretry\b[^.!?]*[.!?])", re.I)


def split_markdown_row(line: str) -> list[str]:
    text = line.strip()
    if not text.startswith("|") or not text.endswith("|"):
        return []
    cells: list[str] = []
    current: list[str] = []
    in_code = False
    escaped = False
    for ch in text[1:-1]:
        if escaped:
            current.append(ch)
            escaped = False
            continue
        if ch == "\\":
            current.append(ch)
            escaped = True
            continue
        if ch == "`":
            in_code = not in_code
            current.append(ch)
            continue
        if ch == "|" and not in_code:
            cells.append("".join(current).strip())
            current = []
            continue
        current.append(ch)
    cells.append("".join(current).strip())
    return cells


def strip_code(text: str) -> str:
    return text.replace("`", "").strip()


def parse_experiment_log(path: Path) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        cells = split_markdown_row(line)
        if len(cells) < 7:
            continue
        date = strip_code(cells[0])
        if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", date):
            continue
        rows.append(
            {
                "date": date,
                "id": strip_code(cells[1]),
                "commit": strip_code(cells[2]),
                "hypothesis": strip_code(cells[3]),
                "command": strip_code(cells[4]),
                "result": strip_code(cells[5]),
                "decision": strip_code(cells[6]),
                "line": line_number,
            }
        )
    return rows


def metric_direction(metric: str) -> str:
    normalized = metric.lower().replace("-", "_")
    if normalized in ("hits", "prefetch_hits", "stage_hits", "window_hits", "group_hits"):
        return "higher"
    if normalized.startswith(HIGHER_BETTER_PREFIXES):
        return "higher"
    if normalized.startswith(LOWER_BETTER_PREFIXES) or normalized.endswith(("_vb", "_bytes", "_reads")):
        return "lower"
    return "neutral"


def metric_weight(metric: str) -> int:
    normalized = metric.lower().replace("-", "_")
    if normalized in ("loop_vb", "scene_vb"):
        return 8
    if normalized in ("overrun_vb", "blocking_vb", "prefetch_overrun_vb"):
        return 7
    if normalized in ("due_misses", "loop_reads", "blocking_reads"):
        return 6
    if normalized.endswith("_vb"):
        return 4
    if normalized.endswith(("_bytes", "_rects", "_rows", "_calls")):
        return 2
    return 1


def extract_metric_deltas(text: str) -> list[dict[str, Any]]:
    deltas: list[dict[str, Any]] = []
    for match in METRIC_RE.finditer(text):
        raw_metric, before_text, after_text = match.groups()
        for metric in raw_metric.split("/"):
            metric = metric.strip("` ,;:.").replace("-", "_")
            if not metric:
                continue
            before = int(before_text)
            after = int(after_text)
            delta = after - before
            direction = metric_direction(metric)
            if direction == "higher":
                beneficial = delta > 0
            elif direction == "lower":
                beneficial = delta < 0
            else:
                beneficial = False
            deltas.append(
                {
                    "metric": metric,
                    "before": before,
                    "after": after,
                    "delta": delta,
                    "direction": direction,
                    "beneficial": beneficial,
                    "weight": metric_weight(metric),
                }
            )
    return deltas


def classify_status(row: dict[str, Any]) -> str:
    text = f"{row['result']} {row['decision']}".lower()
    if "promote" in text and "do not promote" not in text and "no promotion" not in text:
        return "promoted"
    if "rejected" in text:
        return "rejected"
    if "failed" in text:
        return "failed"
    if "no promotion" in text or "do not promote" in text:
        return "no-promotion"
    return "unknown"


def keyword_matches(keyword: str, lowered: str) -> bool:
    if re.fullmatch(r"[a-z0-9_]+", keyword):
        return re.search(rf"(?<![a-z0-9_]){re.escape(keyword)}(?![a-z0-9_])", lowered) is not None
    return keyword in lowered


def group_for_text(text: str) -> tuple[str, int, list[str]]:
    lowered = text.lower()
    scores: list[tuple[int, int, str, list[str]]] = []
    for group, priority, keywords in GROUPS:
        hits = [keyword for keyword in keywords if keyword_matches(keyword, lowered)]
        if hits:
            scores.append((len(hits), -priority, group, hits))
    if not scores:
        return "unclassified", 99, []
    scores.sort(reverse=True)
    hit_count, neg_priority, group, hits = scores[0]
    return group, -neg_priority, hits


def retry_condition(text: str) -> str | None:
    matches = RETRY_SENTENCE_RE.findall(text)
    if not matches:
        return None
    cleaned = " ".join(match.strip() for match in matches[:2])
    return re.sub(r"\s+", " ", cleaned)


def classify_row(row: dict[str, Any]) -> dict[str, Any]:
    text = f"{row['id']} {row['hypothesis']} {row['result']} {row['decision']}"
    lowered = text.lower()
    status = classify_status(row)
    deltas = extract_metric_deltas(text)
    positive = [item for item in deltas if item["beneficial"]]
    negative = [
        item
        for item in deltas
        if not item["beneficial"] and item["direction"] in ("lower", "higher") and item["delta"] != 0
    ]
    positive_score = sum(abs(item["delta"]) * item["weight"] for item in positive)
    negative_score = sum(abs(item["delta"]) * item["weight"] for item in negative)
    group, group_priority, group_hits = group_for_text(text)

    if "do not retry" in lowered or "unsafe" in lowered:
        disposition = "dead-end"
    elif retry_condition(text):
        disposition = "conditional-retry"
    elif status in ("failed", "rejected", "no-promotion") and positive and negative:
        disposition = "near-miss"
    elif status in ("failed", "rejected", "no-promotion") and "no-op" in lowered:
        disposition = "diagnostic-only"
    elif status in ("failed", "rejected", "no-promotion"):
        disposition = "low-priority-review"
    elif status == "promoted":
        disposition = "promoted"
    else:
        disposition = "unknown"

    if status == "promoted":
        relevance = "accepted-history"
    elif disposition in ("conditional-retry", "near-miss") and group_priority <= 4:
        relevance = "high"
    elif disposition in ("conditional-retry", "near-miss"):
        relevance = "medium"
    elif disposition == "diagnostic-only":
        relevance = "low"
    else:
        relevance = "low"

    return {
        **row,
        "status": status,
        "disposition": disposition,
        "group": group,
        "group_priority": group_priority,
        "group_hits": group_hits,
        "relevance": relevance,
        "retry_condition": retry_condition(text),
        "metrics": deltas,
        "positive_metric_count": len(positive),
        "negative_metric_count": len(negative),
        "positive_score": positive_score,
        "negative_score": negative_score,
        "near_miss_score": positive_score - negative_score,
        "top_positive_metrics": sorted(
            positive,
            key=lambda item: abs(item["delta"]) * item["weight"],
            reverse=True,
        )[:5],
        "top_negative_metrics": sorted(
            negative,
            key=lambda item: abs(item["delta"]) * item["weight"],
            reverse=True,
        )[:5],
    }


def load_summary_cases(paths: list[Path]) -> list[dict[str, Any]]:
    cases: list[dict[str, Any]] = []
    for path in paths:
        payload = json.loads(path.read_text(encoding="utf-8"))
        raw_cases = payload.get("cases")
        if isinstance(raw_cases, list):
            for case in raw_cases:
                if isinstance(case, dict):
                    case = dict(case)
                    case["_summary_file"] = str(path)
                    cases.append(case)
        elif isinstance(payload, dict):
            payload = dict(payload)
            payload["_summary_file"] = str(path)
            cases.append(payload)
    return cases


def section_int(case: dict[str, Any], section: str, key: str) -> int | None:
    value = case.get("sections", {}).get(section, {}).get(key)
    return value if isinstance(value, int) else None


def build_read_cost_profiles(summary_paths: list[Path]) -> list[dict[str, Any]]:
    profiles: list[dict[str, Any]] = []
    for case in load_summary_cases(summary_paths):
        cd = case.get("sections", {}).get("cd", {})
        prefetch = case.get("sections", {}).get("prefetch", {})
        scene = case.get("sections", {}).get("scene", {})
        loop_reads = section_int(case, "cd", "loop_reads") or 0
        hidden_reads = prefetch.get("hidden_reads") if isinstance(prefetch.get("hidden_reads"), int) else 0
        blocking_reads = prefetch.get("blocking_reads") if isinstance(prefetch.get("blocking_reads"), int) else 0
        loop_read_vb = section_int(case, "cd", "loop_read_vb") or 0
        hidden_vb = section_int(case, "cd", "hidden_vb") or 0
        blocking_vb = section_int(case, "cd", "blocking_vb") or 0
        prefetch_overrun_vb = prefetch.get("overrun_vb") if isinstance(prefetch.get("overrun_vb"), int) else 0
        due_misses = prefetch.get("due_misses") if isinstance(prefetch.get("due_misses"), int) else 0
        visible_pressure = blocking_vb + prefetch_overrun_vb
        if visible_pressure == 0 and due_misses == 0:
            pressure_class = "cd-clean"
        elif due_misses > 0:
            pressure_class = "due-miss-pressure"
        else:
            pressure_class = "visible-cd-pressure"
        profiles.append(
            {
                "summary_file": case.get("_summary_file"),
                "label": case.get("label"),
                "boot": case.get("boot"),
                "scene": scene.get("scene"),
                "lowtide": scene.get("lowtide"),
                "pack": scene.get("pack"),
                "pack_lba": scene.get("pack_lba"),
                "pack_sectors": scene.get("pack_sectors"),
                "loop_vb": section_int(case, "timing", "loop_vb"),
                "target_vb": section_int(case, "timing", "target_vb"),
                "overrun_vb": section_int(case, "timing", "overrun_vb"),
                "loop_reads": loop_reads,
                "loop_read_vb": loop_read_vb,
                "blocking_reads": blocking_reads,
                "blocking_vb": blocking_vb,
                "hidden_reads": hidden_reads,
                "hidden_vb": hidden_vb,
                "prefetch_overrun_vb": prefetch_overrun_vb,
                "due_misses": due_misses,
                "avg_loop_read_vb": round(loop_read_vb / loop_reads, 3) if loop_reads else 0,
                "avg_hidden_read_vb": round(hidden_vb / hidden_reads, 3) if hidden_reads else 0,
                "avg_blocking_vb": round(blocking_vb / blocking_reads, 3) if blocking_reads else 0,
                "visible_pressure_vb": visible_pressure,
                "pressure_class": pressure_class,
                "recommended_next_group": (
                    "generated-cd-setup"
                    if visible_pressure > 0 or due_misses > 0
                    else "scheduler-or-graphics"
                ),
                "cd_metrics": {
                    "reads": cd.get("reads"),
                    "setup_reads": cd.get("setup_reads"),
                    "seek_back": cd.get("seek_back"),
                    "seek_fwd": cd.get("seek_fwd"),
                    "seq": cd.get("seq"),
                    "max_read_vb": cd.get("max_read_vb"),
                    "max_read_sectors": cd.get("max_read_sectors"),
                },
            }
        )
    return profiles


def load_jsonl_stats(path: Path | None) -> dict[str, Any]:
    if path is None or not path.exists():
        return {}
    total = 0
    by_status: Counter[str] = Counter()
    by_label: Counter[str] = Counter()
    latest_by_label: dict[str, dict[str, Any]] = {}
    for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        if not line.strip():
            continue
        try:
            item = json.loads(line)
        except json.JSONDecodeError:
            continue
        total += 1
        status = "gate_pass" if item.get("gate_pass") else item.get("attempt_status") or "unknown"
        label = item.get("label") or "unknown"
        by_status[status] += 1
        by_label[label] += 1
        latest_by_label[label] = {
            "run_id": item.get("run_id"),
            "commit": item.get("git", {}).get("commit"),
            "gate_pass": item.get("gate_pass"),
            "metrics": item.get("metrics", {}),
        }
    return {
        "path": str(path),
        "total_records": total,
        "by_status": dict(by_status),
        "by_label": dict(by_label),
        "latest_by_label": latest_by_label,
    }


def load_read_plans(paths: list[Path]) -> list[dict[str, Any]]:
    plans: list[dict[str, Any]] = []
    for path in paths:
        payload = json.loads(path.read_text(encoding="utf-8"))
        candidate_sets = payload.get("candidate_sets", {})
        top: list[dict[str, Any]] = []
        if isinstance(candidate_sets, dict):
            for size, candidates in candidate_sets.items():
                if not isinstance(candidates, list):
                    continue
                for candidate in candidates[:5]:
                    if isinstance(candidate, dict):
                        item = dict(candidate)
                        item["candidate_sectors"] = size
                        top.append(item)
        top.sort(
            key=lambda item: (
                item.get("estimated_saved_reads") or 0,
                item.get("estimated_read_vblanks") or 0,
                item.get("covered_entry_count") or 0,
            ),
            reverse=True,
        )
        plans.append(
            {
                "path": str(path),
                "scene": payload.get("scene", {}),
                "current_metrics": payload.get("current_metrics", {}),
                "setup_coverage": payload.get("setup_coverage", {}),
                "observed_reads": payload.get("observed_reads", {}),
                "top_candidates": top[:10],
            }
        )
    return plans


def build_manifest(args: argparse.Namespace) -> dict[str, Any]:
    rows = [classify_row(row) for row in parse_experiment_log(args.experiment_log)]
    if not args.include_promoted:
        rows = [row for row in rows if row["status"] != "promoted"]

    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in rows:
        grouped[row["group"]].append(row)

    for items in grouped.values():
        items.sort(
            key=lambda row: (
                row["group_priority"],
                0 if row["relevance"] == "high" else 1 if row["relevance"] == "medium" else 2,
                -row["positive_score"],
                row["negative_score"],
                row["line"],
            )
        )

    top_queue = sorted(
        [
            row
            for row in rows
            if row["disposition"] in ("conditional-retry", "near-miss")
            and row["status"] != "promoted"
            and row["group"] != "unclassified"
        ],
        key=lambda row: (
            row["group_priority"],
            0 if row["relevance"] == "high" else 1,
            -row["positive_score"],
            row["negative_score"],
            row["line"],
        ),
    )

    return {
        "schema": "ps1-perf-retry-manifest/v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "experiment_log": str(args.experiment_log),
        "summary": {
            "rows": len(rows),
            "by_status": dict(Counter(row["status"] for row in rows)),
            "by_disposition": dict(Counter(row["disposition"] for row in rows)),
            "by_group": dict(Counter(row["group"] for row in rows)),
        },
        "top_queue": top_queue[: args.top],
        "groups": {group: items[: args.top] for group, items in sorted(grouped.items())},
        "read_cost_profiles": build_read_cost_profiles(args.summary),
        "jsonl_stats": load_jsonl_stats(args.experiments_jsonl),
        "read_plans": load_read_plans(args.read_plan),
    }


def metric_brief(metrics: list[dict[str, Any]]) -> str:
    if not metrics:
        return "-"
    parts = []
    for item in metrics[:3]:
        delta = item["delta"]
        sign = "+" if delta > 0 else ""
        parts.append(f"{item['metric']} {sign}{delta}")
    return ", ".join(parts)


def write_markdown(path: Path, manifest: dict[str, Any]) -> None:
    lines: list[str] = []
    summary = manifest["summary"]
    lines.append("# PS1 Perf Retry Manifest")
    lines.append("")
    lines.append(f"Generated: `{manifest['generated_at']}`")
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append(f"- Rows analyzed: `{summary['rows']}`")
    lines.append(f"- By status: `{json.dumps(summary['by_status'], sort_keys=True)}`")
    lines.append(f"- By disposition: `{json.dumps(summary['by_disposition'], sort_keys=True)}`")
    lines.append(f"- By group: `{json.dumps(summary['by_group'], sort_keys=True)}`")
    lines.append("")
    lines.append("## Top Queue")
    lines.append("")
    lines.append("| Group | ID | Disposition | Retry condition | Positive signals | Negative signals |")
    lines.append("|---|---|---|---|---|---|")
    for row in manifest["top_queue"]:
        condition = row.get("retry_condition") or "-"
        lines.append(
            "| "
            f"{row['group']} | `{row['id']}` | {row['disposition']} | "
            f"{condition} | {metric_brief(row['top_positive_metrics'])} | "
            f"{metric_brief(row['top_negative_metrics'])} |"
        )
    lines.append("")
    if manifest["read_cost_profiles"]:
        lines.append("## Read Cost Profiles")
        lines.append("")
        lines.append("| Label | Class | Loop | Target | Visible pressure | Reads | Avg read VBlank | Due misses | Next group |")
        lines.append("|---|---|---|---|---|---|---|---|---|")
        for profile in manifest["read_cost_profiles"]:
            lines.append(
                "| "
                f"{profile.get('label')} | {profile.get('pressure_class')} | "
                f"{profile.get('loop_vb')} | {profile.get('target_vb')} | "
                f"{profile.get('visible_pressure_vb')} | {profile.get('loop_reads')} | "
                f"{profile.get('avg_loop_read_vb')} | {profile.get('due_misses')} | "
                f"{profile.get('recommended_next_group')} |"
            )
        lines.append("")
    if manifest["read_plans"]:
        lines.append("## Read Plan Candidates")
        lines.append("")
        lines.append("| Plan | Sector range | Saved reads | Estimated read VBlanks | Risk | Entries |")
        lines.append("|---|---|---|---|---|---|")
        for plan in manifest["read_plans"]:
            scene = plan.get("scene", {}).get("label") or Path(plan["path"]).stem
            for candidate in plan.get("top_candidates", [])[:5]:
                lines.append(
                    "| "
                    f"{scene} | `{candidate.get('start_sector')}:{candidate.get('end_sector')}` | "
                    f"{candidate.get('estimated_saved_reads')} | "
                    f"{candidate.get('estimated_read_vblanks')} | "
                    f"{candidate.get('phase_risk_hint')} | "
                    f"{candidate.get('covered_entry_count')} |"
                )
        lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def print_human(manifest: dict[str, Any]) -> None:
    summary = manifest["summary"]
    print(f"Rows analyzed: {summary['rows']}")
    print(f"By status: {json.dumps(summary['by_status'], sort_keys=True)}")
    print(f"By disposition: {json.dumps(summary['by_disposition'], sort_keys=True)}")
    print(f"By group: {json.dumps(summary['by_group'], sort_keys=True)}")
    print("\nTop queue:")
    for row in manifest["top_queue"][:12]:
        print(
            f"  {row['group']:24s} {row['id']:42s} "
            f"{row['disposition']:18s} +[{metric_brief(row['top_positive_metrics'])}] "
            f"-[{metric_brief(row['top_negative_metrics'])}]"
        )
    if manifest["read_cost_profiles"]:
        print("\nRead cost profiles:")
        for profile in manifest["read_cost_profiles"]:
            print(
                f"  {profile.get('label')}: {profile.get('pressure_class')} "
                f"loop={profile.get('loop_vb')} target={profile.get('target_vb')} "
                f"visible={profile.get('visible_pressure_vb')} "
                f"reads={profile.get('loop_reads')} "
                f"avg_read_vb={profile.get('avg_loop_read_vb')}"
            )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--experiment-log", type=Path, default=DEFAULT_LOG)
    parser.add_argument("--experiments-jsonl", type=Path, default=DEFAULT_JSONL)
    parser.add_argument("--summary", action="append", type=Path, default=[])
    parser.add_argument("--read-plan", action="append", type=Path, default=[])
    parser.add_argument("--top", type=int, default=40)
    parser.add_argument("--include-promoted", action="store_true")
    parser.add_argument("--output", type=Path, help="write JSON manifest")
    parser.add_argument("--markdown", type=Path, help="write compact markdown manifest")
    parser.add_argument("--json", action="store_true", help="print JSON manifest")
    args = parser.parse_args()

    manifest = build_manifest(args)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.markdown:
        write_markdown(args.markdown, manifest)
    if args.json:
        print(json.dumps(manifest, indent=2))
    else:
        print_human(manifest)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
