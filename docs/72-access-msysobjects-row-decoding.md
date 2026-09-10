# Access MSysObjects Row Decoding (Table-Name Enumeration)

Written for issue #5539 (parent #141, related #5476/#5474/#68/#5540/#5541),
the row-decoding slice `docs/71-access-table-definition-schema-inspection.md`
deferred as materially higher-risk:
`copperfin::vfp::scan_access_msysobjects_catalog()` in
`src/vfp/access_msysobjects.cpp`, which decodes rows from the `MSysObjects`
system catalog table using the general Jet3/Jet4 data-row decoding
algorithm, and `scan_access_container_schema()`'s own new use of it
(`src/vfp/access_table_definition.cpp`) to attach a real name to each
discovered table.

## Evidence basis

The byte layout comes from the mdbtools project's own `HACKING.md` (GPL,
community reverse-engineering documentation -- see docs/66/68/71 for the
full provenance discussion this slice inherits). Beyond that documented
layout, this slice's development went through three escalating rounds of
independent verification, in order:

1. **Manual byte-level derivation against real fixtures.** The row-decode
   algorithm's documented prose (reverse-order variable-length offset
   table, `eod` as an implicit upper-bound sentinel, a Jet3 jump table
   for rows >= 256 bytes) has several places where a plausible-sounding
   reading turns out wrong. Working forward from a real Jet3 fixture's
   `MSysObjects` row bytes for its own catalog entry (`Id=2`,
   `ParentId=251658241`, `Name="MSysObjects"`, `Type=1`) and a real Jet4
   fixture's row bytes for a `Customers` catalog entry, this slice
   independently reverse-derived, byte-by-byte, exactly where each field
   lives -- catching and correcting an initial misreading where two
   "unexplained" bytes in a hand-decoded Jet3 row turned out to be the
   real (non-printable) content of the `Owner` column, not evidence of a
   layout error.
2. **Cross-checking against `mdbtools` itself.** `mdbtools 1.0.1` was
   installed locally and used as an independent reader: `mdb-sql`
   (`SELECT Id, ParentId, Name, Type FROM MSysObjects WHERE Type=1`) and
   `mdb-schema` against both real fixtures. Every `Type == 1` row this
   slice's decoder produces for both fixtures -- 8 rows for the Jet3
   fixture, 15 for the Jet4 fixture, including names containing spaces
   (`"Order Details"`, `"Shipping Methods"`) -- matches `mdbtools`'
   output exactly, for both the catalog `Id`/`Name` values and the
   downstream `parse_access_table_definition_page()` column counts each
   named table's own TDEF page decodes to.
3. **A genuine real-world bug this cross-validation caught.** The first
   working version of this decoder assumed a discovered TDEF's column
   descriptors (`AccessTableDefinition::columns`) are returned in the
   same order as their logical `column_number` (declaration order) --
   true for every #5476 synthetic/real fixture exercised so far, and
   true for the Jet3 fixture used here. It is **not** true for the real
   Jet4 fixture used during this slice's development: that file's
   `MSysObjects` TDEF stores its column descriptors in **physical
   storage order alphabetized by name** (`Connect`, `Database`,
   `DateCreate`, ... `Type`), while each descriptor's own `column_number`
   field still correctly encodes the real declaration order (`Id=0`,
   `ParentId=1`, `Name=2`, `Type=3`, ...). Decoding against that file
   with the (wrong) assumption produced entirely garbled output --
   nonsensical `Type` values, mojibake names -- immediately caught by
   comparing against the `mdbtools` ground truth from step 2. The fix
   (reindex columns by their own `column_number` before using vector
   position as the null-mask bit index or a well-known column's lookup
   key, rather than assuming physical and logical order coincide) is
   covered by a dedicated regression test
   (`test_scan_access_msysobjects_catalog_handles_alphabetized_column_storage_order`,
   `tests/test_vfp_assets.cpp`) reproducing the same physical/logical
   mismatch against a synthetic fixture.

This is real-fixture verification cross-checked against an independent
reader's own output -- not merely trusting the community source the way
step 1 alone would have been, and materially stronger than #5476's own
TDEF-level verification (which had no independent reader available to
cross-check against at the time). It is still not verification against
an official Microsoft specification or a licensed Access installation
writing the tested bytes itself.

The real fixtures used for this cross-validation are **not** committed to
the repository, per this project's established practice (see docs/71's
own note); the committed test suite instead uses synthetic pages and rows
built from this verified layout (`tests/test_vfp_assets.cpp`).

## What this slice reads

For `MSysObjects`'s own data pages (found by scanning every page in the
container for a data page, `page_type == 0x01`, whose `tdef_pg` field
equals 2 -- `MSysObjects` is always rooted at TDEF page 2, a
well-corroborated constant):

- Every row's `Id`, `ParentId`, `Name`, and `Type` fields, using the
  column layout `parse_access_table_definition_page()` already decodes
  from page 2's own TDEF (reused directly, not re-parsed independently
  -- see "Design notes" below).
- A deleted row (its row-offset-table slot's `0x8000` flag set) is
  silently excluded, matching real Access/mdbtools semantics for a
  deleted row.

`scan_access_container_schema()` uses this to attach a `name` to each
table it already discovers via TDEF-page scanning: a catalog row's
candidate TDEF page number (`Id & 0x00FFFFFF`, per `mdbtools`'ows own
`mdb_read_catalog()` precedent -- cited during #5541's review discussion)
is only trusted when it matches a table this scan **independently
discovered** by finding an actual TDEF page at that page number. A
candidate page number with no matching discovered table is left
unattached rather than fabricating a table entry from the catalog row
alone -- the caution #5541 raised about `MSysObjects.Id` reliability,
which that issue's own follow-up (closed as a fixture mix-up, not a real
bug, after independent `mdbtools` cross-verification) resolved in
`Id`'s favor for the fixtures available, but this join still does not
blindly trust it beyond what is independently corroborated.

## What this slice does not read (deliberate, documented scope boundary)

- **Jet3 rows >= 256 bytes.** Jet3 variable-column offsets are 1-byte
  fields (max 256), requiring a jump table for larger rows. `docs/68`
  already flagged the general row-decode algorithm as having "no
  independently-checkable invariant"; the jump-table sub-path in
  particular could not be verified against any real fixture available
  during this slice's development (every real `MSysObjects` row observed
  in both fixtures was well under 256 bytes -- the largest real Jet4 row
  seen, for a table with a populated `LvProp` property blob, was 93
  bytes). Rather than ship an interpretation of the documented algorithm
  that has never been checked against real bytes, such a row is recorded
  in `AccessMSysObjectsScanResult::skipped` with a clear reason. A
  synthetic-fixture regression test proves this fails closed rather than
  silently misreading offsets
  (`test_scan_access_msysobjects_catalog_fails_closed_on_jet3_row_at_or_above_256_bytes`).
- **Jet4 "compressed unicode" text.** A `0xFF 0xFE`-prefixed encoding
  mdbtools' `HACKING.md` documents for Jet4 text values. Every real Jet4
  `Name` value observed during this slice's development was plain
  (uncompressed) UCS-2LE; a compressed value is detected by its leading
  marker bytes and recorded in `skipped` rather than decoded, for the
  same real-fixture-verification reason as the jump-table gap above.
- **A lookup-overflow row** (its row-offset-table slot's `0x4000` flag
  set, pointing at a Data Pointer to another page rather than storing
  the row inline) is recorded in `skipped` rather than followed. One
  real Jet4 fixture row exercised this path during development
  (page 14, row 36 of the real Jet4 fixture used) and was correctly
  identified and excluded.
- **Non-`MSysObjects` table data rows.** Decoding arbitrary user-table
  data (as opposed to `MSysObjects`'s catalog rows specifically) remains
  out of scope, per #5539's own stated non-goals -- a further, later step
  once this slice's row-decoder exists and is proven correct against the
  one well-known, well-documented system table first.
- **A row whose own `num_cols` (or variable-column count) does not match
  the current TDEF's column count** -- which would mean the table was
  `ALTER TABLE`-ed after the row was written -- fails closed
  (`Vfp.AccessMSysObjects.Error.ColumnCountMismatch`) rather than
  attempting partial reconstruction. `MSysObjects` is an Access-internal
  system table never subject to user `ALTER TABLE`, so real fixtures
  never exercise this path; failing closed here matches this codebase's
  established discipline for unverified shapes (e.g. #5476's own
  multi-page-TDEF rejection).

## Design notes

- `scan_access_msysobjects_catalog()` re-reads and re-scans the whole
  container independently of `scan_access_container_schema()`'s own
  first-pass page scan (both perform their own lightweight 8-byte-per-
  page header probe, then read only the pages each actually needs). This
  is a deliberate simplicity-over-micro-optimization tradeoff: the two
  scans stay cleanly separated (single-responsibility, independently
  testable) rather than threading a shared page cache between two
  otherwise-independent modules, at the cost of one extra sequential
  pass over the container's page headers when both are used together via
  `scan_access_container_schema()`. Given real Access files are read
  page-by-page regardless (never buffered whole, per docs/71's own
  established discipline) and this is a read-only inspection path, not a
  hot loop, this tradeoff was chosen deliberately rather than
  overlooked; a future slice could merge the two scans if profiling ever
  shows this matters in practice.
- `AccessColumnDefinition` gained a new `offset_f` field (previously read
  and discarded by `parse_access_table_definition_page()`) so this
  slice's row decoder can locate a fixed column's value without
  re-parsing the TDEF page a second time with a separate, potentially
  divergent implementation.
- A small set of genuinely shared byte-level helpers
  (`read_le_u16`/`read_le_u32`/`sanitize_as_utf8`) were factored out of
  `access_table_definition.cpp`'s anonymous namespace into a new
  project-internal header, `src/vfp/access_bytes_internal.h`, so this
  slice's new translation unit can reuse them rather than duplicating a
  nontrivial UTF-8 sanitizer.

## Follow-up work

- Jet3 jump-table decoding (rows >= 256 bytes) and Jet4 compressed-
  unicode text remain open gaps, deliberately deferred per the real-
  fixture-verification discipline above rather than guessed at. Neither
  blocks this slice's actual deliverable (table-name enumeration for the
  real fixtures available, where every row observed fits neither
  excluded case).
- Non-`MSysObjects` data-row decoding (arbitrary user-table content) is
  the natural next step once a use case needs it, building on this
  slice's now-proven row-decode core.
