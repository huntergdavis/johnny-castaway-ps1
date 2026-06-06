# Binary-Identical Cleanup Plan

## Question

Can we reorder functions, split files, clean comments, and improve source layout while using the PS1 executable bytes as the safety gate?

## Short Answer

Yes, but only after making the build layout deterministic enough for source-order-only edits.

Moving a live function can change the final PS1 executable even when the function body is unchanged. The cause is not C semantics; it is link layout. With `-ffunction-sections`, each function is emitted into its own input section. Without explicit sorting, GNU `ld` places matching input sections in the order it sees them. Source order and object-file order can therefore become executable address order.

The practical hardening step is to keep per-function sections and ask the linker to sort sections by name:

```cmake
target_compile_options(jcreborn PRIVATE
    -ffunction-sections
    -fdata-sections
)

target_link_options(jcreborn PRIVATE
    -Wl,--gc-sections
    -Wl,--sort-section=name
)
```

`--sort-section=name` applies `SORT_BY_NAME` to wildcard section patterns in the linker script. With sections named from symbols, this makes many source-order moves independent of physical source position.

## Local Experiment

The current PS1 build already uses:

- `-ffunction-sections`
- `-fdata-sections`
- `-Wl,--gc-sections`

Two throwaway builds were tested:

1. Move a live `atoi()` function earlier inside `src/ps1_stubs.c`.
2. Move the same `atoi()` function into a new source file while preserving compile options.

Without section sorting, moving live `atoi()` changed `build-ps1/jcreborn.exe`.

With `-Wl,--sort-section=name`, both the reordered-function build and the split-file build produced byte-identical `jcreborn.exe` output compared with the sorted-layout baseline.

## Proposed Workflow

1. Add `-Wl,--sort-section=name` to the PS1 link flags.
2. Rebuild from clean and record the one-time binary delta caused by the new deterministic layout.
3. Validate that sorted-layout baseline using the normal PS1 smoke/regtest path.
4. Treat that validated sorted-layout executable as the cleanup baseline.
5. Run cleanup as a strict one-change loop:
   - move one function or one small group at a time
   - preserve function body text, symbol name, linkage, attributes, and compile flags
   - build from clean
   - compare the final PS1 executable bytes, not just source or object files
   - accept the cleanup only if `jcreborn.exe` is byte-identical
6. If the executable differs, the change leaves the pure-cleanup lane. Split the move smaller, inspect the map/disassembly, or handle it as a normal behavioral-risk change with tests.

## Suggested Gate

Use the final PS-X executable as the primary gate:

```bash
./scripts/build-ps1.sh clean
sha256sum build-ps1/jcreborn.exe > before.sha256

# apply one cleanup move

./scripts/build-ps1.sh clean
sha256sum build-ps1/jcreborn.exe > after.sha256
cmp -s before.sha256 after.sha256
```

For a stricter local check, compare the executable directly:

```bash
cmp -s /path/to/baseline/jcreborn.exe build-ps1/jcreborn.exe
```

Do not use the ELF file as the main gate unless debug metadata has been normalized or stripped. The loaded PS1 executable is the meaningful artifact for runtime identity.

## Red-Team Pass

### Issue: Comments and whitespace may still change the binary

Comments and whitespace are usually ignored by the compiler, but line-oriented macros are not. Changes near `__LINE__`, `__FILE__`, assertions, debug macros, or generated probes can alter embedded strings or constants.

Rule: do not treat edits around file/line-sensitive code as pure cleanup unless the executable remains byte-identical.

### Issue: `__DATE__` and `__TIME__` make reproducibility fragile

The codebase currently uses `__DATE__` in the pause menu. That can make rebuilds differ by date even without source changes.

Rule: set a fixed `SOURCE_DATE_EPOCH` for deterministic builds or remove build-time date strings from release artifacts before relying on byte comparison as a cleanup gate.

### Issue: Moving functions into new files can change optimization behavior

This build has per-source `-Os` overrides. A moved function can silently get a different optimization level if the new file is not listed in the same source-property block.

Rule: every new source file created during cleanup must inherit the original file's compile options before binary comparison is considered meaningful.

### Issue: `static` functions are not stable across file splits

A `static` function moved to another translation unit changes visibility constraints, possible section name collisions, and compiler optimization context. A helper that was callable only inside one file may need a new non-static declaration, which is a semantic surface change even if the body is identical.

Rule: prefer reordering within a file first. Split file-scope helpers only when the byte gate passes and the header/API change is deliberate.

### Issue: Inlining and local context can change code generation

Moving an inline helper, macro-heavy block, or file-scope constant can affect inlining, constant propagation, and local optimization decisions.

Rule: do not assume source moves are safe. The byte gate is the authority.

### Issue: Section sorting changes code locality

Sorting by section name deliberately changes layout from source/object order to symbol-name order. That one-time shift can affect instruction cache locality and timing on real hardware or emulators.

Rule: validate the sorted-layout baseline before cleanup starts. After that, byte-identical cleanup moves preserve the validated layout.

### Issue: Duplicate names and local symbols can collide in ordering

Multiple files can contain `static` functions with the same name, creating same-named sections. Linker tie-breaking may fall back to object order.

Rule: if moving duplicate-named local functions, verify with the map file and executable comparison. Consider unique static names if necessary, but that itself will change section names and may require a new baseline.

### Issue: Data layout can move too

`-fdata-sections` plus sorted sections helps, but file-scope data, string constants, rodata, small-data placement, and GP-relative data can still change when declarations move.

Rule: include data moves in the same one-change byte gate. Do not batch data and function moves together until this process has proven stable.

## Final Plan

The cleanup loop is viable if it is treated as a constrained, binary-preserving refactor lane:

1. First commit: add deterministic section sorting.
2. Validate and bless the new sorted-layout baseline.
3. For each cleanup commit, require final `jcreborn.exe` byte identity against that baseline.
4. Keep each commit narrow enough that a binary diff can be explained immediately.
5. Any non-identical result is either reverted, split smaller, or promoted to a normal tested code change.

This gives a high-confidence safety gate: if the final PS1 executable bytes are identical, the runtime-loaded program image is identical. If they differ, the change may still be correct, but it is no longer a mechanically proven cleanup.

## References

- GNU `ld`: `--sort-section=name` applies `SORT_BY_NAME` to wildcard section patterns.
- GCC: `-ffunction-sections` and `-fdata-sections` place functions/data items into individual sections named from the symbol.
- GCC: `SOURCE_DATE_EPOCH` makes `__DATE__` and `__TIME__` reproducible.
- GCC: `-frandom-seed` can help produce reproducibly identical object files when GCC-generated unique names are involved.
