# Index Format Notes

This phase adds a safe xBase index probe layer to the native asset inspector.

Current coverage:

- `CDX` and `DCX`
  - Minimal header probe for block size, root node offset, free node hint, key-length hint, and key-pool hint.
  - Treated as multi-tag index containers.
  - First-pass directory leaf-page parsing now surfaces stored tag names plus conservative per-tag page hints from plausible node pages, and page-local key/`FOR` expression hints now prefer the hinted tag-page neighborhood before falling back to whole-file heuristics.
  - First-pass expression-derived normalization/collation hints are now surfaced for tag expressions such as `UPPER(...)`/`LOWER(...)`, with the current implementation explicitly treated as heuristic metadata rather than true binary collation fidelity.
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
  - First-pass block-level probe with block-local tag hint enumeration from non-header metadata regions.
  - Minimal plausibility checks now require a non-empty header block plus at least one plausible non-header tag hint.
  - Treated as a production multi-tag index container pending deeper layout, expression, and write parsing.

Current inspector behavior:

- Direct inspection recognizes `CDX`, `DCX`, `IDX`, `NDX`, and `MDX`.
- DBF/DBC-family inspection now reports structured validation findings when expected structural companion indexes are missing or when present companion indexes fail to parse.
- The runtime order loader now preserves additive normalization/collation hints through `SET ORDER` and temporary `SEEK ... TAG` overrides, and emits those hints in `runtime.order` / `runtime.seek` event detail for verification.
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

1. Extend the new `CDX/DCX` directory-page parsing into deeper per-tag metadata extraction instead of relying on expression matching heuristics for anything beyond first-pass key/`FOR` hints.
2. Deepen `MDX` parsing beyond block-local tag hints into real layout and expression metadata extraction.
3. Move beyond first-pass expression-derived normalization/collation hints, current opaque single-index header markers, and the narrow `NDX` key-domain runtime slice into more format-grounded metadata where the file layouts support it.
4. Correlate DBF field metadata with index expressions for migration planning.
5. Deepen read-only validation against real VFP and dBase fixtures beyond the current smoke coverage.
