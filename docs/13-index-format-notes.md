# Index Format Notes

This phase adds a safe xBase index probe layer to the native asset inspector.

Current coverage:

- `CDX` and `DCX`
  - Minimal header probe for block size, root node offset, free node hint, key-length hint, and key-pool hint.
  - Treated as multi-tag index containers.
  - First-pass directory leaf-page parsing now surfaces stored tag names plus conservative per-tag page hints from plausible node pages, and page-local key/`FOR` expression hints now prefer the hinted tag-page neighborhood before falling back to whole-file heuristics.
  - When a stored tag name is descriptive rather than expression-shaped, single local tag-page expressions can now still bind through the grounded page hint instead of being dropped back to derived fallback names.
  - First-pass expression-derived normalization/collation hints are now surfaced for tag expressions such as `UPPER(...)`/`LOWER(...)`, with the current implementation explicitly treated as heuristic metadata rather than true binary collation fidelity.
  - Per-tag page marker hints (`flags` and entry counts) are now surfaced from grounded tag-page headers when page offsets are available.
  - Focused regression coverage now exercises direct `.cdx`/`.dcx` probing, adversarial decoy-expression cases, and `DBC` companion discovery.
- `IDX`
  - Visual FoxPro single-index header probe.
  - Extracts root, free-list, and EOF offsets plus key and `FOR` expression hints.
  - First-pass expression-derived normalization/collation hints are now surfaced alongside the extracted key expression.
  - An additive opaque header sort-marker hint is now surfaced from already-read header bytes, without mapping that marker to a named collation sequence yet.
- `NDX`
  - dBase-style single-index header probe.
  - Extracts root and EOF block hints, key length, maximum key count, group length, uniqueness flag, and key expression hint.
  - Additive opaque header sort-marker hints and a key-domain hint now surface from existing header bytes, again without mapping raw markers to named collations.
  - The runtime now uses the `NDX` key-domain hint for a narrow indexed-compare slice: numeric-domain `SEEK` ordering can behave numerically without changing broader collation behavior.
- `MDX`
  - Block-oriented header probe with plausibility checks for block sizing and tag table geometry.
  - Tag-table parsing now surfaces per-tag page offsets, key-format markers, key-type markers, and thread marker hints.
  - Tag-header page parsing now extracts first-pass key and `FOR` expressions with source offsets, plus expression-derived normalization/collation hints.
  - Treated as a production multi-tag index container for read/inspection workflows; write fidelity remains out of scope.

Current inspector behavior:

- Direct inspection recognizes `CDX`, `DCX`, `IDX`, `NDX`, and `MDX`.
- DBF/DBC-family inspection now reports structured validation findings when expected structural companion indexes are missing or when present companion indexes fail to parse.
- The runtime order loader now preserves additive normalization/collation hints through `SET ORDER` and temporary `SEEK ... TAG` overrides, and emits those hints in `runtime.order` / `runtime.seek` event detail for verification.
- The runtime locate/scan path now emits `runtime.rushmore` diagnostics for index-seek decisions while restoring the caller's active order after temporary optimization probes.
- Table inspection looks for same-base companion indexes:
  - `table.cdx`
  - `table.idx`
  - `table.ndx`
  - `table.mdx`
  - `table.dbf.cdx`
  - `table.dbf.idx`
  - `table.dbf.ndx`
  - `table.dbf.mdx`
- DBF headers now expose `has_production_index()` so the same flag can support both FoxPro structural indexes and dBase production indexes.

Local reality checks used during implementation:

- `C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Tastrade\Data\customer.cdx`
- `C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Tastrade\Data\Orders.CDX`
- `C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Tastrade\Data\tastrade.dcx`
- `C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Northwind\products.cdx`
- `C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Northwind\orderdetails.cdx`
- `C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Northwind\northwind.dcx`
- `C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Tastrade\Data\customer.dbf`
- `E:\DBASE\DBFS\CHNGREAS.NDX`

Reference docs used to keep the probe rules grounded:

- Visual FoxPro file-type reference:
  - <https://vfphelp.com/vfp9/html/71acd830-031d-40ee-bc2b-a8d9452d0efc.htm>
- dBASE table/header reference:
  - <https://www.dbase.com/Knowledgebase/INT/db7_file_fmt.htm>

Next implementation steps:

1. Move beyond first-pass expression-derived normalization/collation hints, current opaque single-index header markers, and the narrow `NDX` key-domain runtime slice into more format-grounded metadata where the file layouts support it.
2. Correlate DBF field metadata with index expressions for migration planning.
3. Deepen read-only validation against real VFP and dBase fixtures beyond the current smoke coverage.
4. Design and stage explicit index-write fidelity slices (separate from read/inspection parsing).
5. **Done (2026-09-04, partially):** `tests/fixtures/vfp9_descending_observation.prg`
   was run in a disposable real VFP9 process (`9.0.00.7423`); its TSV output
   plus the generated DBF/CDX/IDX fixtures are retained at
   `tests/fixtures/vfp9-descending-observation-output/` (see the README
   there for the full observed contract and open gaps). This recovered the
   zero-argument `DESCENDING()` function's black-box contract, including the
   previously-undocumented `ASCENDING` runtime inverse-override and (after a
   correction from PR #5488 review) the two-argument
   `DESCENDING(cCDXFileName, nTagNumber)` form's contract -- its data was
   initially recorded as uninterpretable, but was fully consistent once
   `TAG()`'s global open-index ordinal (`RQ-CF-PRG-010`) was correctly
   distinguished from `DESCENDING(cCDXFileName, ...)`'s CDX-file-scoped
   ordinal (see `RQ-CF-PRG-019`, status `gap`). It did **not** yet recover
   the CDX persisted-direction byte encoding needed to implement the
   persisted-tag case (for either argument form), nor the single-IDX
   runtime-override case -- an attempt to correlate the retained
   `descending.CDX` bytes against known tag directions was abandoned when
   `copperfin_inspect`'s existing tag-header heuristics produced garbled
   results against that small fixture. A larger, cleaner fixture plus an
   interactive VFP9 session are needed before `DESCENDING()` can actually be
   implemented.
