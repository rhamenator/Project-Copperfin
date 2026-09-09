# dBASE/FoxBASE/FoxPro Import Field Mapping

Written field-type mapping for issues #5523 and #5525 (parent #5517, #137),
the first concrete slices of the `IMPORT DATABASE ... TYPE XBASE` wizard:
`copperfin::vfp::import_xbase_table_to_vfp_native()` in
`src/vfp/dbf_import.cpp`. This covers dBASE-family (`DbfFormatFamily::dbase`)
and, as of #5525, FoxBASE (`DbfFormatFamily::foxbase`, restricted to the
verified `0x02` version byte -- see below) and FoxPro
(`DbfFormatFamily::foxpro`) sources. FoxBASE and FoxPro cannot produce
dBASE Level 7's `I`/`+`/`O` types (those older formats predate Level 7), so
the same base mapping table covers all four families for the shared
`C`/`N`/`F`/`L`/`D`/`M` types; FoxPro additionally has its own `G`/`P`
General/Picture types (not shared with dBASE III/IV), which this slice
does not yet support -- see the unsupported-types table below. Clipper
source support is an explicit follow-up once #5484's NTX/DBT compatibility
work is trusted for it.

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

### FoxBASE version restriction

Only FoxBASE's `0x02` version byte is accepted, not `0xFB`. Both classify
as `DbfFormatFamily::foxbase`, but `RQ-CF-LEGACY-003` records that `0xFB`'s
genuine physical layout was never verified against a real fixture --
`dbf_read_layout()` deliberately does not specialize it the way `0x02` is
specialized, so it falls back to the generic default layout. Importing an
`0xFB` file could therefore silently serialize fields read from the wrong
byte offsets while still reporting success. This restriction is enforced
in `import_xbase_table_to_vfp_native()` directly (checking
`header.version == 0x02U`, not just `format_family() == foxbase`), not
just documented.

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

- No Clipper source-family support yet (follow-up once #5484's NTX/DBT
  compatibility work is trusted for it).
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
