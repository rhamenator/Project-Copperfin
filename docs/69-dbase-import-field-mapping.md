# dBASE/FoxBASE/FoxPro Import Field Mapping

Written field-type mapping for issues #5523, #5525, #5528, #5530, and
#5532 (parent #5517, #137), the first concrete slices of the
`IMPORT DATABASE ... TYPE XBASE` wizard:
`copperfin::vfp::import_xbase_table_to_vfp_native()` and
`copperfin::vfp::preview_xbase_table_import()` in
`src/vfp/dbf_import.cpp`. This covers dBASE-family (`DbfFormatFamily::dbase`)
and, as of #5525/#5528, the whole FoxBASE family (`DbfFormatFamily::foxbase`,
both `0x02` and `0xFB` -- see below) and FoxPro (`DbfFormatFamily::foxpro`)
sources. FoxBASE and FoxPro cannot produce dBASE Level 7's `I`/`+`/`O` types
(those older formats predate Level 7), so the same base mapping table
covers all four families for the shared `C`/`N`/`F`/`L`/`D`/`M` types;
FoxPro additionally has its own `G`/`P` General/Picture types (not shared
with dBASE III/IV), which this slice does not yet support -- see the
unsupported-types table below. Clipper source support (#5530) required no
new code: Clipper's default (non-DBFCDX) RDD writes DBF/DBT tables
byte-compatible with dBASE III+ (`0x83`; has-memo) layout (confirmed in
#5484), so `DbfFormatFamily` has no distinct `clipper` value -- Clipper-
produced tables already classify as `dbase` and were already accepted.

## Supported source types

`I`/`+`/`O` are dBASE Level 7-specific and cannot occur for FoxBASE/FoxPro
sources.

| dBASE-family type | Meaning | Target VFP type | Notes |
| --- | --- | --- | --- |
| `C` | Character | `C` | Direct; identical on-disk text encoding on both sides. |
| `N` | Numeric | `N` | Direct; same decimal-count semantics. |
| `F` | Float | `N` | dBASE `F` and `N` already decode/encode identically in Copperfin's reader/writer; both map to VFP `N`. |
| `L` | Logical | `L` | Direct. |
| `D` | Date | `D` | Direct; both sides use the same `YYYYMMDD` on-disk text form. |
| `M` | Memo (text) | `M` | Real memo content is copied via a second pass using the existing, already-tested `replace_record_field_value()` REPLACE path (see below) -- the base table-creation API only accepts a blank memo pointer at creation time. |
| `I` | Long (dBASE Level 7) | `I` | Direct value copy. The dBASE Level 7 big-endian sign-magnitude on-disk encoding and VFP's little-endian two's-complement encoding are already reconciled by the existing reader/writer, both exposed through the same plain signed-decimal string. |
| `+` | Autoincrement (dBASE Level 7) | `I` | Value copied like `I`. **Lossy**: the *autoincrement behavior* is not preserved -- the target field is a plain, static Integer. Only the already-assigned values carry over. |
| `O` | Double (dBASE Level 7) | `B` | Direct value copy. Both sides already decode/encode the same little-endian 8-byte IEEE-754 layout through a plain decimal string. |

## Explicitly unsupported source types (this slice)

| dBASE-family type | Meaning | Why not supported yet |
| --- | --- | --- |
| `B` | Binary DBT-block pointer (dBASE's own `B`, distinct from VFP's `B` double) | The existing reader already treats this as an opaque binary payload (not reinterpreted, per `RQ-CF-LEGACY-002`). Writing it into a VFP General (`G`) field would require the same "resolve to a memo/General block, write raw bytes" path `M` uses, but through `write_memo_field_bytes()` (raw bytes) rather than `write_memo_field_text()` (encoded text) -- deferred to a follow-up slice rather than adding an unverified second memo-write path in this first pass. |
| `@` | Julian-day/millisecond timestamp | The existing reader does not yet convert this to a real calendar date/time (`parse_dbf_table_from_file()` currently exposes it only as a diagnostic `julian:N millis:M` string, not a value `write_field_bytes()`'s VFP `T`/`Y` cases can consume). There is nothing meaningful to write until that conversion exists upstream in the reader. |
| `G` / `P` | FoxPro General / Picture (binary payload, FoxPro-only) | Same category of work as `B` above -- the reader already decodes these as opaque binary payloads (`RQ-CF-LEGACY-003`), and writing them would need `write_memo_field_bytes()`'s raw-bytes path rather than `M`'s encoded-text path. Deferred to a follow-up slice; explicitly listed in the mapping switch (not left to the generic default case) so the exclusion is a deliberate scope decision. |
| any other letter | -- | Not part of dBASE-family's documented type set; fails closed the same as `B`/`@`/`G`/`P`. |

### FoxBASE `0xFB` support (#5528)

Both `0x02` and `0xFB` classify as `DbfFormatFamily::foxbase` and are
accepted. `0x02`'s physical layout (8-byte header, 16-byte descriptors)
was independently verified against a real fixture during #5483. `0xFB`'s
was not -- #5528's research
(`docs/70-foxbase-0xfb-investigation.md`) traced the widely-repeated
"`0xFB` = FoxBASE" claim back to a community reference table whose
original compiler marked it an acknowledged unknown ("FoxPro ???"), found
no real `0xFB`-signed file ever documented in roughly two decades of
active xBase community cataloging, and confirmed a separately-checked AI
assistant's citation claiming a source-cited specification documents it
was fabricated (that specification, checked directly, contains no `0xFB`
entry). Every source that ventured an actual guess at `0xFB`'s meaning
converged on the same lineage as the already-verified `0x03` byte, so
`dbf_read_layout()` now routes `0xFB` through that same sequential-
packing layout rather than the raw VFP-native default it previously fell
through to, and `import_xbase_table_to_vfp_native()` accepts the whole
`foxbase` family on the same footing as `dbase`/`foxpro`. This is a
documented best-effort choice, not a verified fact -- see the
investigation doc for the full reasoning and evidence trail.

### Clipper source support (#5530)

Clipper source tables are already supported, with no source-family
allowlist change needed. Clipper's default (non-DBFCDX) RDD writes
DBF/DBT tables byte-compatible with dBASE III+ (`0x83`; has-memo)
layout -- confirmed in #5484 -- so `DbfFormatFamily` has no
distinct `clipper` value: Clipper-produced tables already classify as
`dbase`, which this importer already accepted from #5523 onward.
`test_import_xbase_table_to_vfp_native_round_trips_clipper_compatible_source`
proves this with the same real dBASE III + memo fixture #5484 used for
the equivalent read-side claim, rather than leaving it undemonstrated.

### Dry-run/report mode (#5532)

`preview_xbase_table_import(source_path, source_memo_sidecar_path = {})`
performs the same source-family, code-page, and per-field mapping checks
as the committing import, without ever writing anything -- it takes no
`destination_path` at all, since nothing is created. It shares its
validation logic with the committing path (`plan_field_mapping()`,
`src/vfp/dbf_import.cpp`) so the two can never drift out of sync on what
counts as importable.

The one deliberate behavioral difference: the committing path fails
closed on the *first* unmappable field it finds (a safety property --
never write a partial or silently-lossy table), while the preview
collects *every* unmappable field into `DbfImportPreviewResult::field_issues`,
so a caller can see the complete picture before deciding whether to
import at all.

The preview does not check for unresolved memo payloads (the reader's
`"<memo block N>"` diagnostic placeholder) or a destination-side
memo-sidecar conflict -- both are properties of a specific
destination/write attempt, not of the source table's importability in
the abstract, so they remain checks only the committing path performs.

An unsupported source field type fails the whole import closed *before* any
destination file is created or written -- see
`test_import_dbase_table_to_vfp_native_rejects_unsupported_field_type` in
`tests/test_dbf_table.cpp`.

## Additional safety checks (added after review)

- **Code page**: only `code_page_mark == 0` source tables are accepted, but
  `code_page_mark == 0` is *not* itself a sufficient safety guarantee --
  see the caveat below. A real single-byte code page (e.g. CP1252) decodes
  to (possibly wider) UTF-8 on read, but this slice copies the source
  field's declared byte width unchanged and always creates the destination
  as code-page-0 -- a non-ASCII character in a tightly-sized field could
  otherwise silently fail to fit. Rejected with
  `Vfp.DbfImport.Error.UnsupportedCodePage` before any destination file
  is created.
  - **#5525 finding**: `code_page_mark == 0` is Copperfin's own
    reinterpretation as "UTF-8" (see `dbf_text_encoding.cpp`), not a
    guarantee the *original* file's bytes actually are valid UTF-8 --
    older FoxPro/FoxBASE data marked code-page-0 often predates that
    convention and can contain legacy single-byte-encoded text instead.
    The real `dbase_f5.dbf` fixture (#5483's FoxPro test fixture)
    demonstrates this: its `COMN` field contains a byte that is not valid
    UTF-8 despite a code-page-0 header. This is not a gap in this slice's
    safety, though -- `create_dbf_table_file()`'s own width check still
    catches the resulting mismatch and rejects the write before any
    corruption, exactly as it would for any other oversized value (see
    `test_import_xbase_table_to_vfp_native_rejects_foxpro_field_with_non_utf8_bytes`).
    It does mean some genuine, otherwise-clean-looking code-page-0 legacy
    tables will fail to import until a future slice can either detect and
    re-decode such fields from their real original encoding, or widen
    affected target fields to accommodate the UTF-8 expansion.
- **Unresolved memo payloads**: the dBASE reader exposes a missing/
  truncated/unreadable memo block as a diagnostic string
  (`"<memo block N>"`) rather than failing the whole table parse. This
  importer detects that pattern and rejects the whole import
  (`Vfp.DbfImport.Error.UnresolvedMemoPayload`) *before* creating any
  destination file, rather than silently writing the diagnostic text as
  if it were real memo content.
- **Existing memo-sidecar conflict**: when the target schema has any
  memo field, the destination's memo-sidecar path (resolved the same
  case-insensitive way `create_dbf_table_file()` itself resolves it) is
  checked for an existing file even when the primary destination `.dbf`
  path is free -- otherwise a stray same-base `.fpt` left over from
  something else could be silently overwritten.
- **Rollback on partial failure**: if the second-pass memo-fill loop
  fails partway through (e.g. a filesystem error), the destination
  `.dbf`/`.fpt` created by the first pass are removed before returning
  the error, so a retry is not immediately blocked by the
  destination-exists check.
- **`O` (double) precision**: `dbf_table.cpp`'s reader originally
  formatted doubles with a fixed 15-digit precision, which is not always
  enough to round-trip an arbitrary IEEE-754 double exactly. Widened to
  `std::numeric_limits<double>::max_digits10` (17) so `O`-mapped values
  actually preserve their exact bit pattern through the decimal-string
  round trip, matching this document's original claim for that row.

## Non-goals of these slices (see #5517 for the full wizard's scope)

- No NTX index import -- the importer doesn't handle indexes for any
  source family yet; this is broader scope than Clipper specifically.
- No DBC container import.
- No raw-bytes memo write path -- this is why `B`/`G`/`P` remain
  unsupported (see the table above); adding it is a separate, larger
  slice touching core `dbf_table.cpp` write internals used by many other
  callers, not just this importer.
- No deleted-record-flag preservation: every source record (deleted or not)
  is imported as an active record in the destination table. A future slice
  may choose to either preserve the flag or offer a `PACK`-equivalent
  filter; this slice does neither, since the base VFP-native table-creation
  API used here (`create_dbf_table_file()`) does not expose per-record
  deleted-flag control.
- Destination-existence check is a non-atomic `std::filesystem::exists()`
  probe before writing, matching this table-creation family's existing
  behavior elsewhere in the codebase (unlike #5485's `create_dbase_iii_table_file()`,
  which was given an atomic-exclusive-create guarantee specifically because
  it is the newest writer in the codebase).
