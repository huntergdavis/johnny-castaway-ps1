"""
holidays_concepts — master RENDERERS dict combining the reference
implementations + all 4 batch files authored by sub-agents.

Each batch file (`holidays_concepts_batchN.py`) declares its own
`RENDERERS_BATCHN` dict. We import them all and merge into a single
`RENDERERS` dict keyed by holiday id, value = N-tuple of callable
renderers (one per variant). The runner script
`holidays-generate-art.py` imports RENDERERS and renders each holiday
across all variants.

After merging the hand-authored v1..v4 tuples, this module also
synthesizes a v5 NIGHT variant per holiday by post-processing v1
through `holidays_art_lib.as_night`. v5 is procedural rather than
hand-drawn — that's OK because it's the "alternate" the owner can
choose if the day-time variants don't suit. To override v5 per
holiday, append a 5th element to the tuple in the batch file and
this module will leave it alone.

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


# ---------------------------------------------------------------------------
# v5 = NIGHT auto-extension. For every holiday whose tuple is shorter than
# 5, we append a procedural night variant generated from v1 by
# `holidays_art_lib.as_night`. A batch file can opt out by hand-authoring
# its own 5th element.
# ---------------------------------------------------------------------------

try:
    from holidays_art_lib import as_night

    def _make_night(v1_fn):
        def _v5(h):
            return as_night(v1_fn(h))
        _v5.__name__ = f"night_of_{getattr(v1_fn, '__name__', 'v1')}"
        return _v5

    extended = 0
    for hid, variants in list(RENDERERS.items()):
        if not isinstance(variants, tuple):
            variants = tuple(variants)
        if len(variants) >= 5:
            continue
        v1 = variants[0]
        RENDERERS[hid] = variants + (_make_night(v1),)
        extended += 1
    print(f"  + v5 NIGHT auto-generated for {extended} holidays")
except Exception as e:
    print(f"  [skip] v5 NIGHT auto-gen failed: {e}")
