# Access Long-Value (Memo/OLE) Column Reading

Written for issue #5549 (parent #141): `copperfin::vfp::read_access_long_value_column()`
in `src/vfp/access_long_value.cpp`, which retrieves the actual bytes of a
Jet3/Jet4 "long value" column -- Access's mechanism for a Memo or OLE
value too large to fit inline in a row.

## Why this slice exists

#5476 (Access table/column schema inspection) and #5539 (MSysObjects
row decoding) both stopped short of reading a memo/OLE column's actual
*content* -- #5539's own row decoder only extracts `MSysObjects`'s short,
fixed/variable-length catalog columns (`Id`, `ParentId`, `Name`, `Type`).
No code anywhere in this codebase read a long-value column before this
slice.

This matters beyond memo columns on ordinary user tables: #5477
(forms/reports structural inspection), #5478 (VBA/Access Basic
extraction), and #5479 (saved-query extraction) each need to retrieve a
large, variable-length blob out of some system object's row -- Access
form/report definitions, VBA project storage, and query definitions are
all conventionally stored via this same long-value mechanism. This slice
implements that retrieval mechanism once, as a general capability, so
those three issues can each consume it rather than reinventing it.

## Evidence basis

Grounded in `mdbtools`' own `HACKING.md` (the same community
reverse-engineering documentation already established as this codebase's
evidence source for #5476/#5539 -- see `docs/68`), specifically its
"Each memo column ..." and "LVAL (Long Value) Pages" sections. That
documentation describes:

- A 12-byte in-row field descriptor: 3-byte `memo_len`, 1-byte bitmask,
  4-byte `lval_dp` "data pointer", 4 reserved/unknown bytes.
- Three bitmask cases: `0x80` (value inline, appended after the header),
  `0x40` (value in a single LVAL page, "record type 1"), `0x00` (value
  spans multiple LVAL pages, "record type 2", chained via each page's own
  data pointer).
- An LVAL page's header is "just like that of a regular data page, except
  in place of the tdef_pg is the word 'LVAL'".

This alone was not sufficient to implement from with confidence -- the
same documentation is explicit that `lval_dp`'s own row-id encoding is
underspecified, and this codebase's own established discipline (see
docs/32's rules, and the precedent from #5534/#5539's own real-fixture
detours) is to verify a byte-level format against real data before
shipping it, not merely trust a single secondary source's prose.

### Real-fixture verification (this session)

Using real, locally-available Jet3/Jet4 `.mdb` files (not committed to
the repository, per this project's established practice):

1. **In-row descriptor and the `0x40` (single-page) case.** Located a
   real `MSysObjects.LvProp` row with a non-null value
   (`memo_len=490, bitmask=0x40, lval_dp=0x00013D00`). Read the target
   LVAL page directly: its bytes at offset 4 spelled the literal ASCII
   `LVAL`, its single row (via the same row-offset-table format
   `access_msysobjects.cpp`'s row decoder already uses for regular data
   pages) was exactly 490 bytes, and its content began with `MR2\0` --
   exactly the Jet4 LvProp header `mdbtools`' own "Properties" section
   documents.
2. **`lval_dp`'s bit layout.** The naive reading (page number = low 3
   bytes, row_id = high byte -- mirroring `MSysObjects.Id`'s own
   `id & 0x00FFFFFF` masking convention already verified for #5539)
   predicted page 81152 in a 330-page file: impossible. The opposite
   reading (row_id = low byte, page number = upper 3 bytes, i.e.
   `lval_dp >> 8`) predicted page 317 -- and that page's own bytes[4:8]
   were confirmed to be the literal `LVAL` marker. This is the *opposite*
   convention from `MSysObjects.Id`'s page-number masking; the two fields
   are not interchangeable despite superficially similar 4-byte "pointer"
   shapes.
3. **The `0x80` (inline) case.** A real `Address` column value
   (`memo_len=19, bitmask=0x80`) had exactly `12 + 19 = 31` total column
   bytes, with the trailing 19 bytes being the value itself (in this
   particular case, Jet4 "compressed unicode" text -- interpreting *that*
   encoding remains this codebase's own separately-tracked gap, per
   `access_msysobjects.cpp`'s existing `CompressedUnicodeUnsupported`
   path; this slice only retrieves the raw bytes unchanged).
4. **The `0x00` (chained) case.** Located and traced a real chain
   (`memo_len=29013`) across **15 hops**: each hop's row began with its
   own 4-byte "next" data pointer (using the same low-byte-row/
   high-3-bytes-page convention as above), followed by that hop's partial
   data; the chain terminated at a `next_dp == 0`. Concatenating all 15
   hops' partial data produced exactly 29013 bytes -- reassembly
   verified byte-count-exact, not merely "didn't crash."

This is real-fixture verification of every documented code path this
slice implements, not merely trusting `mdbtools`' prose -- matching the
rigor already established for #5539's row decoder.

## What this slice implements

- `parse_access_long_value_field_descriptor()`: parses the 12-byte
  in-row header only (pure, no I/O).
- `read_access_long_value_column()`: resolves the full value for all
  three bitmask cases, given the column's own already-extracted raw
  bytes plus (for the two LVAL-page-resident cases) the container path
  and generation to read overflow pages from. Fails closed -- a
  structured error, not a guessed/partial value -- for a descriptor too
  short, an inline value whose length disagrees with its own declared
  length, an unrecognized bitmask, a page missing the `LVAL` marker, a
  malformed row directory, a row_id beyond a page's own declared row
  count, or a chain exceeding a generous bounded hop count (a guard
  against a corrupt or cyclic chain, not a realistic limit -- the largest
  chain observed during this slice's development was 15 hops).

## What this slice deliberately does not implement

- **Writing/mutation of any kind.**
- **Interpreting what is *inside* a retrieved long value.** `LvProp`'s
  own documented inner chunk structure, VBA project decompression
  (MS-OVBA), and form/report structural parsing are each their own
  separate, already-tracked issues (#5477, #5478, #5479) that will
  consume this capability's raw-bytes output.
- **Jet4 "compressed unicode" text decoding.** A memo value's raw bytes
  are still returned unchanged even when they carry the `0xFF 0xFE`
  compressed-unicode marker; only *interpreting* those bytes as text
  remains out of scope, matching `access_msysobjects.cpp`'s own existing,
  separately-tracked gap for the identical encoding on `MSysObjects.Name`.

## Follow-up work

- **#5477/#5478/#5479** each consume this capability's raw-bytes output
  for their own respective format-specific parsing.
- **Jet4 compressed-unicode text decoding**, if a later slice needs to
  interpret a long-value column's bytes as text rather than opaque
  binary (already tracked as a gap in `access_msysobjects.cpp`).
