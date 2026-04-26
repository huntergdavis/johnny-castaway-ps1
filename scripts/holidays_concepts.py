"""
holidays_concepts — master RENDERERS dict combining the reference
implementations + all 4 batch files authored by sub-agents.

Each batch file (`holidays_concepts_batchN.py`) declares its own
`RENDERERS_BATCHN` dict. We import them all and merge into a single
`RENDERERS` dict keyed by holiday id, value = (v1, v2, v3) tuple of
callables. The runner script `holidays-generate-art.py` imports
RENDERERS and renders each holiday × 3 variants.

Missing batches are tolerated — if a batch file fails to import or
returns no entries, those holiday IDs simply won't render.
"""

RENDERERS: dict = {}


def _try_merge(module_name: str, dict_name: str) -> int:
    """Import module_name and merge module.dict_name into RENDERERS.
    Returns the number of entries merged. Errors are logged but
    non-fatal so a single broken batch doesn't kill the others."""
    try:
        mod = __import__(module_name)
    except Exception as e:
        print(f"  [skip] {module_name}: {e}")
        return 0
    d = getattr(mod, dict_name, None)
    if not isinstance(d, dict):
        print(f"  [skip] {module_name}: no {dict_name} dict")
        return 0
    n = 0
    for k, v in d.items():
        if k in RENDERERS:
            print(f"  [warn] {module_name}: id {k} already registered, overwriting")
        RENDERERS[k] = v
        n += 1
    return n


print("Loading holiday renderers:")
for mod_name, dict_name in [
    ("holidays_concepts_reference", "RENDERERS_REFERENCE"),
    ("holidays_concepts_batch1",    "RENDERERS_BATCH1"),
    ("holidays_concepts_batch2",    "RENDERERS_BATCH2"),
    ("holidays_concepts_batch3",    "RENDERERS_BATCH3"),
    ("holidays_concepts_batch4",    "RENDERERS_BATCH4"),
]:
    n = _try_merge(mod_name, dict_name)
    print(f"  + {mod_name:35s} {n:3d} holidays")
print(f"  TOTAL: {len(RENDERERS)} holidays registered")
