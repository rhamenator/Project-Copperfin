# xBase Index-Derived Relation Inference

Written for issue #5534 (parent #5517's `IMPORT DATABASE ... TYPE XBASE`
wizard): `copperfin::vfp::infer_xbase_index_relations()` in
`src/vfp/xbase_relation_inference.cpp`, which scans a set of already-
identified xBase-family tables' companion index files and suggests
candidate relations between tables that both index a same-named column.

## Scope of this slice

#5534 itself frames the full idea as having two distinct, materially
different-sized pieces:

1. **Index import/rebuild**: translate a source table's recovered index
   key expressions into freshly-built VFP-native CDX tags on the
   destination table.
2. **Relation inference and presentation**: use those recovered key
   expressions to suggest candidate relationships between tables.

This slice implements **only the second piece**, read-only. The first
piece -- a real CDX B-tree *writer* -- has no existing precedent anywhere
in this codebase: every reader in `src/vfp/index_probe.cpp` (CDX, IDX,
NDX, MDX, NTX) is explicitly "header-probe-only, no B-tree
materialization" per `docs/13-index-format-notes.md`. Writing a correct,
real-VFP-compatible compound index file is a substantially larger,
higher-risk undertaking than this slice's scope -- #5534's own text
already anticipates this ("this is meaningfully bigger than the existing
table-only import; it needs (1) a real index-rebuild path ... and (2)
relation-inference heuristics"), and it remains open, explicitly tracked
as follow-up work, not attempted here.

Like `import_xbase_table_to_vfp_native()`/`preview_xbase_table_import()`
(`include/copperfin/vfp/dbf_import.h`) themselves, this is a standalone
library function, not yet wired to any PRG-level `IMPORT DATABASE`
command surface -- #5517's own wizard command syntax does not exist yet
either (no dispatch code in `src/runtime/` references the single-table
import functions). This slice ships the underlying capability for a
later orchestration layer to call, matching this epic's own established
incremental pattern of library-capability-first, PRG-wiring-later.

## The heuristic

For every pair of *different* tables that each have an index whose key
expression is a plain column reference sharing the same name (case-
insensitively), report one candidate relation. This is deliberately:

- **Non-directional.** Neither side is asserted to be the "parent"/
  primary-key table. `index_probe.cpp`'s `IndexTagProbe`/`IndexProbe`
  expose no per-tag uniqueness or primary-key marker, so there is no
  reliable signal available to this codebase for determining direction --
  asserting one would be a guess, which this project's own discipline
  avoids making. (Real xBase index files carry key *expressions*, not
  declared foreign-key metadata, so this limitation is inherent to the
  source data, not just this implementation.)
- **Conservative on the "plain column" question.** A composite/expression
  key (a concatenation, a function call like `UPPER(...)`) is not
  translatable to a single-column relation and is silently excluded from
  consideration -- not reported as an error, since this is a best-effort
  suggestion pass, not a validation pass.
- **Never silently applied.** #5534's own text is explicit that inferred
  relations "should be presented as suggestions, not silently applied" --
  this function only produces a list of candidates; nothing in this slice
  writes a persistent relation to any destination database container.

## Evidence basis for the underlying index readers

The CDX/IDX/NDX/MDX/NTX header probes this function calls
(`index_probe.cpp`, `cdx_header.cpp`) already existed before this slice
and are unchanged by it -- see those modules' own documented evidence
trail (`docs/13-index-format-notes.md`) for their provenance. One
discovery made *while writing this slice's tests* is worth recording
here: `cdx_header.cpp`'s real-fixture-tuned tag/expression-discovery
heuristics (`looks_like_tag_name_candidate()`,
`looks_like_expression_candidate()`) require a tag name to be 4-10
uppercase characters and a key expression to be at least 4 characters
(and, if it is a plain lowercase identifier with no parentheses or
comparison operator, to also contain an underscore) before recognizing
it as a candidate at all -- a short, bare identifier like `"id"` is
silently discarded by the same heuristic that correctly recognizes
`"cust_id"`. This was found by hand-tracing an initial, shorter synthetic
CDX fixture through `parse_index_probe_from_file()` directly and finding
it reported zero tags; the committed test suite's synthetic fixtures
(`tests/test_dbf_table.cpp`) use column/expression names long enough to
avoid this trap, matching the same pattern `export_database_as_postgresql_sql()`'s
own CDX-consuming tests (#5537) already had to work around for the same
reason.

## Follow-up work

- **Index rebuild** (translating recovered key expressions into a real,
  freshly-built VFP-native CDX file on the destination table) remains the
  larger, unimplemented half of #5534 -- a CDX B-tree writer with no
  existing precedent in this codebase.
- **PRG-level `IMPORT DATABASE ... TYPE XBASE` wizard wiring** (#5517)
  does not exist yet at all; this function (and the sibling single-table
  import functions it complements) are library capabilities awaiting that
  orchestration layer.
- **Presentation** of inferred relations to the user (a report, or
  suggested persistent relations in a destination `.dbc`) is left to
  whatever consumes this function's output -- out of scope for a pure
  data-structure-returning library function.
