---
layout: page
title: FG2 pack manifest
eyebrow: Reference
subtitle: The JSON sidecar that travels with each pack — scene indices, ADS names, resource memberships, peak memory, prefetch hints. Schema version 1, draft.
description: Reference for the FG2 pack manifest — the JSON sidecar describing one compiled scene pack. Required fields, resource sections, runtime requirements envelope, transition and prefetch hints, an example manifest, and how the manifest is consumed by the CD image build, the regtest harness, and runtime sanity checks.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

A labor of love by Hunter Davis. The pack manifest is the JSON file that
sits next to each compiled FG2 pack and describes what is inside it. The
[pack payload]({{ '/docs/formats/pack-payload/' | relative_url }}) is the
binary the PS1 reads at runtime; the manifest is what every other tool in
the pipeline reads to *reason* about that binary. CD image builders read
manifests to decide layout. The regtest harness reads them to check that
the right packs are present. Pack-planning tools read them to estimate
peak memory and pick pack granularity. Runtime sanity checks read a
trimmed subset of them to assert "the pack on disc is the pack we built."

It is intentionally a JSON manifest, not a binary on-disc format. From the
[source schema]({{ site.github_url }}/blob/main/docs/ps1/research/PACK_MANIFEST_SCHEMA.md):

> Define the minimum manifest contract for Phase 5 pack compilation work so
> that analyzer-derived planning tools emit a stable intermediate format,
> pack compiler work can start before the final binary pack layout is
> chosen, and transition and prefetch studies target the same field names.

The schema is on `schema_version: 1`, draft. It is producible from
analyzer output plus post-processing tools and consumable without
scraping text reports. Pack granularity is still open — one manifest can
cover one ADS tag, one scene cluster, or a hybrid pack with a shared
bank.

If you paid for this, you were cheated. Open source and free.

## Required top-level fields

Every manifest must carry these keys:

| Field                  | Type             | Notes |
|------------------------|------------------|-------|
| `schema_version`       | integer          | Currently `1`. |
| `manifest_kind`        | string           | `"scene_pack_manifest"`. |
| `pack_id`              | string           | Stable identifier for this pack across rebuilds. |
| `pack_strategy`        | string enum      | `"ads_cluster"`, `"scene_tag"`, or `"hybrid"`. |
| `scene_indices`        | array of integer | Story-order scene indices this pack covers. |
| `ads_names`            | array of string  | Source ADS file names. Often a single entry. |
| `resources`            | object           | See *resources* below. |
| `runtime_requirements` | object           | See *runtime requirements* below. |
| `transition_hints`     | object           | See *transition hints* below. |
| `prefetch_hints`       | object           | See *prefetch hints* below. |

`pack_strategy` is the planner's intent — *why* these scenes were grouped
into one pack. `ads_cluster` means "all scenes that share an ADS family
landed here." `scene_tag` means "this is one tag's worth of scenes." `hybrid`
means "this pack plus a separately-loaded shared bank covers the working
set."

## `resources`

The resources section describes what the pack needs available for
deterministic runtime use. Each sub-list is the set of asset names the
pack depends on — the manifest does not embed the assets, only references
them.

```json
"resources": {
  "bmps": ["GJDIVE.BMP", "MJDIVE.BMP", "JOHNWALK.BMP"],
  "scrs": ["ISLETEMP.SCR"],
  "ttms": ["GJDIVE.TTM", "MJDIVE.TTM"],
  "ads":  ["ACTIVITY.ADS"],
  "shared_candidates": ["JOHNWALK.BMP"]
}
```

| Key                  | Meaning |
|----------------------|---------|
| `bmps`               | BMP names the pack references. |
| `scrs`               | SCR (background) names. |
| `ttms`               | TTM (animation script) names. |
| `ads`                | ADS (scene controller script) names. |
| `shared_candidates`  | Resources the planner thinks should move into a shared bank later. |

`shared_candidates` is purely advisory. If a BMP appears in many packs
(e.g. `JOHNWALK.BMP`, which is on screen in nearly every walking scene),
the manifest tags it for later deduplication. Today nothing actually
deduplicates them — the source schema notes "no shared-bank deduplication
is applied across packs yet."

## `runtime_requirements`

The runtime envelope the pack must satisfy. If the pack covers multiple
scenes, these values represent the envelope for the whole pack, not a
single scene.

```json
"runtime_requirements": {
  "max_concurrent_threads": 20,
  "max_sprite_frames": 160,
  "peak_memory_bytes": 436592,
  "memory_components": {
    "bmp_indexed_bytes": 228598,
    "ttm_bytes": 7288,
    "ads_bytes": 2528,
    "scr_bytes": 112038,
    "sprite_pointer_bytes": 640,
    "ttm_slot_overhead_bytes": 49152
  }
}
```

| Field                       | Meaning |
|-----------------------------|---------|
| `max_concurrent_threads`    | Peak active TTM thread count this pack ever needs. |
| `max_sprite_frames`         | Peak frame count across all sprite slots. |
| `peak_memory_bytes`         | Top-line worst-case memory for the pack. |
| `memory_components.*`       | Breakdown by source — which subsystem owns what fraction of peak. |

The pack-planning tools use `peak_memory_bytes` to decide whether a
candidate pack fits the PS1's working budget. The runtime does not
allocate against these numbers directly — it allocates against fixed
buffers — but if a pack reports `peak_memory_bytes` larger than the
fixed buffer, that is a sign the pack will not fit and the planner
should split it.

## `transition_hints`

Pack-boundary and load-time planning data. Generated post-analysis from
the story-order graph and resource churn between scenes.

```json
"transition_hints": {
  "likely_next_pack_ids": ["ads-cluster-building"],
  "high_churn_transitions": [
    { "from_scene_index": 9, "to_scene_index": 10, "total_resource_churn": 12 }
  ],
  "transition_budget_notes": "Derived from analyzer post-processing; not yet runtime-validated."
}
```

| Field                       | Meaning |
|-----------------------------|---------|
| `likely_next_pack_ids`      | Pack IDs the runtime is likely to load next. Feeds prefetch. |
| `high_churn_transitions`    | Scene-to-scene transitions where many resources change at once. |
| `transition_budget_notes`   | Optional freeform note about heuristics in use. |

These are heuristic. The schema doc is explicit that scene ordering is the
analyzer's story order, not an instrumented runtime trace. When the
project gains validated transition telemetry the hints should be
regenerated from real data.

## `prefetch_hints`

Early streaming policy work. Drives the
[transition prefetch schema]({{ '/docs/formats/transition-prefetch/' | relative_url }}).

```json
"prefetch_hints": {
  "candidate_scene_indices": [3, 4, 5],
  "additional_bmps": ["FISHPOLE.BMP"],
  "additional_scrs": [],
  "additional_ttms": ["GJNAT3.TTM"],
  "heuristic": "ads_family_union",
  "confidence": "low"
}
```

| Field                        | Meaning |
|------------------------------|---------|
| `candidate_scene_indices`    | Scene indices likely to follow the current pack. |
| `additional_bmps` / `_scrs` / `_ttms` | Resources to consider warming. |
| `heuristic`                  | Name of the rule that produced this hint. |
| `confidence`                 | `low`, `medium`, or `high`. |

Current expectation per the schema: "all initial prefetch hints should be
treated as heuristic; confidence should stay `low` until we have
validated transition data." Today every hint is `low`. Promoting hints
to `medium` or `high` requires runtime telemetry that does not exist yet.

## Example manifest

A complete manifest for an ADS-cluster pack:

```json
{
  "schema_version": 1,
  "manifest_kind": "scene_pack_manifest",
  "pack_id": "ads-cluster-activity",
  "pack_strategy": "ads_cluster",
  "scene_indices": [0, 1, 2],
  "ads_names": ["ACTIVITY.ADS"],
  "resources": {
    "bmps": ["GJDIVE.BMP", "MJDIVE.BMP", "JOHNWALK.BMP"],
    "scrs": ["ISLETEMP.SCR"],
    "ttms": ["GJDIVE.TTM", "MJDIVE.TTM"],
    "ads": ["ACTIVITY.ADS"],
    "shared_candidates": ["JOHNWALK.BMP"]
  },
  "runtime_requirements": {
    "max_concurrent_threads": 20,
    "max_sprite_frames": 160,
    "peak_memory_bytes": 436592,
    "memory_components": {
      "bmp_indexed_bytes": 228598,
      "ttm_bytes": 7288,
      "ads_bytes": 2528,
      "scr_bytes": 112038,
      "sprite_pointer_bytes": 640,
      "ttm_slot_overhead_bytes": 49152
    }
  },
  "transition_hints": {
    "likely_next_pack_ids": ["ads-cluster-building"],
    "high_churn_transitions": [
      { "from_scene_index": 9, "to_scene_index": 10, "total_resource_churn": 12 }
    ],
    "transition_budget_notes": "Derived from analyzer post-processing; not yet runtime-validated."
  },
  "prefetch_hints": {
    "candidate_scene_indices": [3, 4, 5],
    "additional_bmps": ["FISHPOLE.BMP"],
    "additional_scrs": [],
    "additional_ttms": ["GJNAT3.TTM"],
    "heuristic": "ads_family_union",
    "confidence": "low"
  }
}
```

The pack compiler emits this alongside `pack_payload.bin`. The bytes
described in the manifest's `resources` lists end up referenced from
`pack_index.json` — see the
[pack payload reference]({{ '/docs/formats/pack-payload/' | relative_url }})
for the relationship between the binary and its index.

## How the manifest is consumed

Three readers, three jobs.

**CD image build.** `mkpsxiso` does not read the manifest itself, but the
`cd_layout.xml` generator does. It walks the manifest set, picks which
`FG\*.FG2` payloads to stamp onto the disc, and orders sectors so that
packs the manifest marks as `likely_next_pack_ids` of each other land
near each other on the spiral. This is best-effort — the layout file
ships hand-curated for the pilot scenes — but the manifest is the source
of truth when the layout gets regenerated.

**Regtest harness.** The [regtest]({{ '/docs/regtest/' | relative_url }})
tooling reads each manifest at startup to know which scene indices a
pack is supposed to cover. When a regtest run captures a screenshot for
scene N, the harness looks up the manifest that owns N and asserts the
pack actually loaded. If the pack loaded but the dirty-rect bookkeeping
disagrees with the manifest's `runtime_requirements`, the run is flagged
for review.

**Runtime sanity checks.** The PS1 build does *not* parse JSON. It
consumes a trimmed binary index (`pack_index.json` written into a
fixed-shape sidecar) that carries only the fields the runtime cares
about: pack ID, scene indices, magic + version of the corresponding FG2
payload. A mismatch between disc and index is logged to TTY and the
scene falls back to the legacy ADS path.

## Known limitations

Direct from the schema doc:

- the research sidecar index is still JSON
- no shared-bank deduplication is applied across packs yet
- no checksum validation is performed on-console yet

The runtime does not currently verify SHA256 of the pack payload against
the `sha256` field that `pack_index.json` carries per entry. That check
exists only in the regtest harness today.

## Related references

- [FG2 pack payload]({{ '/docs/formats/pack-payload/' | relative_url }}) —
  the binary the manifest describes.
- [Transition prefetch schema]({{ '/docs/formats/transition-prefetch/' | relative_url }}) —
  the post-processed planner that feeds `prefetch_hints`.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) — the
  consumer that uses manifests for cross-checks.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — where
  `cd_layout.xml` and `mkpsxiso` come together.

## Source on GitHub

- [`docs/ps1/research/PACK_MANIFEST_SCHEMA.md`]({{ site.github_url }}/blob/main/docs/ps1/research/PACK_MANIFEST_SCHEMA.md)
