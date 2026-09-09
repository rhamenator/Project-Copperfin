# dBASE-Family Import Field Mapping

Written field-type mapping for issue #5523 (parent #5517, #137), the first
concrete slice of the `IMPORT DATABASE ... TYPE XBASE` wizard:
`copperfin::vfp::import_dbase_table_to_vfp_native()` in
`src/vfp/dbf_import.cpp`. This covers dBASE-family (`DbfFormatFamily::dbase`)
sources only -- FoxBASE, FoxPro, and Clipper source families are explicit
follow-up slices once this single-table dBASE path is proven, per #5517's
scoping comment.

## Supported source types

| dBASE type | Meaning | Target VFP type | Notes |
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

| dBASE type | Meaning | Why not supported yet |
| --- | --- | --- |
| `B` | Binary DBT-block pointer (dBASE's own `B`, distinct from VFP's `B` double) | The existing reader already treats this as an opaque binary payload (not reinterpreted, per `RQ-CF-LEGACY-002`). Writing it into a VFP General (`G`) field would require the same "resolve to a memo/General block, write raw bytes" path `M` uses, but through `write_memo_field_bytes()` (raw bytes) rather than `write_memo_field_text()` (encoded text) -- deferred to a follow-up slice rather than adding an unverified second memo-write path in this first pass. |
| `@` | Julian-day/millisecond timestamp | The existing reader does not yet convert this to a real calendar date/time (`parse_dbf_table_from_file()` currently exposes it only as a diagnostic `julian:N millis:M` string, not a value `write_field_bytes()`'s VFP `T`/`Y` cases can consume). There is nothing meaningful to write until that conversion exists upstream in the reader. |
| any other letter | -- | Not part of dBASE's documented type set for this family; fails closed the same as `B`/`@`. |

An unsupported source field type fails the whole import closed *before* any
destination file is created or written -- see
`test_import_dbase_table_to_vfp_native_rejects_unsupported_field_type` in
`tests/test_dbf_table.cpp`.

## Non-goals of this slice (see #5517 for the full wizard's scope)

- No FoxBASE/FoxPro/Clipper source-family support.
- No DBC container import.
- No dry-run/report mode -- this slice fails closed on the first unmappable
  field rather than producing a pre-commit report of every issue in the
  source table.
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
