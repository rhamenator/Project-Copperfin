# Access Table Definition (TDEF) Schema Inspection

Written for issue #5476 (parent #141, related #5474/#137), the first
schema-reading slice of Access/JET container support:
`copperfin::vfp::parse_access_table_definition_page()` and
`copperfin::vfp::scan_access_container_schema()` in
`src/vfp/access_table_definition.cpp`. Builds on `#5521`'s
container-level signature/generation detector
(`src/vfp/access_container.cpp`) and is grounded in
`docs/68-access-mdb-jet-physical-page-layout-notes.md`.

## Evidence basis

The byte layout implemented here comes from the mdbtools project's own
`HACKING.md` (GPL, community reverse-engineering documentation, not an
official Microsoft specification -- see `docs/66`/`docs/68` for the full
provenance discussion). Beyond that documented layout, this slice's
development **independently cross-checked the implementation against
real Jet3 and Jet4 `.mdb` fixtures**:

- The decoded column names for each fixture's own `MSysObjects` table
  (always rooted at page 2) matched Access's well-known real
  system-catalog schema exactly, for both a Jet3 fixture (17 columns:
  `Id`, `ParentId`, `Name`, `Type`, `DateCreate`, `DateUpdate`, `Owner`,
  `Flags`, `Database`, `Connect`, `ForeignName`, `RmtInfoShort`,
  `RmtInfoLong`, `Lv`, `LvProp`, `LvModule`, `LvExtra`) and a Jet4
  fixture (the same 17 columns, alphabetically ordered in that
  particular file).
- Column type/length/fixed-length decoding was internally consistent
  with what those names' real semantics predict: `Id`/`ParentId`/
  `Flags` decoded as fixed-length `LONGINT` (4 bytes); `Name`/
  `ForeignName` as variable-length `TEXT`; `DateCreate`/`DateUpdate` as
  fixed-length `DATETIME` (8 bytes); `Database`/`Connect` as `MEMO`
  (0-length, matching mdbtools' own "0 if memo" note); `Lv`/`LvProp`/
  `LvModule`/`LvExtra`/`RmtInfoLong` as `OLE` (long binary, 0-length).
- Every *fixed-length* column's otherwise-unused `offset_F` field held a
  plausible, sequential byte offset; every *variable-length* column's
  held uninitialized-looking garbage -- exactly the behavior expected if
  the fixed-length bitmask bit means what mdbtools' notes say it means,
  and exactly matching mdbtools' own opening caveat that Jet does not
  zero pages before writing them.
- Scanning a whole real fixture for TDEF pages (not just page 2) also
  correctly decoded several other well-known real Access system/spec
  tables (`MSysACEs`, `MSysQueries`, `MSysRelationships`,
  `MSysIMEXColumns`, `MSysIMEXSpecs`) with recognizable column names,
  and one Jet4 fixture surfaced a real column using an undocumented
  type byte (`0x11`), which the implementation correctly reports as
  `AccessColumnType::unknown` rather than guessing or crashing.

This is real-fixture verification, not merely trusting the community
source -- but it is still not verification against an official
specification or a licensed Access installation writing the tested
bytes itself, and no `.accdb` (ACE) fixture was available to verify the
Jet4-derived assumptions against for that engine specifically (see
"ACE/.accdb caveat" below). Per `docs/07-clean-room-rules.md`, this
document records that distinction rather than overstating the evidence.

The real fixtures used for this cross-validation are **not** committed
to the repository and contain no content this document quotes beyond
generic, standard Access system-catalog schema (identical across
virtually every real Access database, not specific to any one file);
the committed test suite (`tests/test_vfp_assets.cpp`) instead uses
synthetic pages built from this verified layout, per this project's
practice of not committing real fixture files that may carry personal
or business data.

## What this slice reads

For a single Table Definition (TDEF) page (page type byte `0x02`):

- Whether the table is a system table (`table_type == 0x53`, `'S'`) or a
  user table (`0x4E`, `'N'`).
- Row count (`num_rows`).
- Every column's name, type (both the raw byte and a best-effort
  `AccessColumnType` classification), declared length, and whether the
  column-descriptor bitmask marks it fixed-length.

`scan_access_container_schema()` scans every page in a container for
TDEF pages that are not a continuation page of another TDEF's `next_pg`
chain, and parses each one, collecting per-page failures into `skipped`
rather than aborting the whole scan or silently dropping them.

## What this slice does not read (deliberate, documented scope boundary)

- **Table names.** A table's name is not stored in its own TDEF page --
  it lives in rows of the `MSysObjects` system catalog table (itself
  rooted at page 2's TDEF, which this slice *can* decode the columns
  of). Reading `MSysObjects`'s actual *rows* requires the general Jet
  data-row-decoding algorithm: a null bitmask, a variable-length column
  offset table stored in reverse order, and -- for Jet3 specifically --
  a jump table for rows whose size exceeds 256 bytes. `docs/68`
  explicitly flagged this as the higher-risk, easy-to-get-subtly-wrong
  piece best deferred to its own focused slice rather than attempted
  alongside the (well-corroborated, byte-offset-driven) TDEF page format
  in the same pass -- see the follow-up issue this document's closing
  section links. Every discovered table is therefore identified only by
  its TDEF page number in this slice's output, not a name.
- **Multi-page TDEFs.** A TDEF whose `next_pg` is nonzero (a table with
  enough columns/indexes that its definition spans more than one page)
  is explicitly rejected (`Vfp.AccessTableDefinition.Error
  .MultiPageTdefUnsupported`) rather than silently parsed from only its
  first page's partial data. Real fixtures used during this slice's
  development did not exercise this path (every table examined fit on
  one page), so reassembling a spanning TDEF's logical byte stream
  across pages is left to a future slice rather than guessed at.
- **Index metadata.** The TDEF page also describes the table's indexes
  (logical index list, index columns, foreign-key relationships); this
  slice's cursor skips past that section (`num_real_idx` bytes) without
  decoding it, since #5476's scope is table/column schema, not indexes
  (index *file* reading is separately tracked in the existing CDX/IDX/
  NDX/MDX/NTX probe work, `src/vfp/index_probe.cpp`, for xBase-family
  index files -- Access indexes live inside the TDEF page instead and
  are not yet covered by either).
- **Encryption.** The Database Definition page (page 0) that this
  slice's container-level detector already reads is itself "encrypted"
  with a simple RC4 key, per mdbtools' notes -- this slice never needed
  to decrypt it (only the leading signature/generation bytes, which sit
  before the encrypted region, are read), and no data page or TDEF
  content this slice reads is itself encrypted in an unencrypted
  database. An encrypted *database* (a distinct, database-wide password
  feature) is out of scope entirely and not detected or rejected
  specially -- it would most likely surface as this slice's own
  structural checks failing closed on what looks like corrupt data,
  which is a reasonable fail-closed outcome for an unsupported case
  even though it is not a purpose-built diagnostic for it.
- **Jet3 column-name code page.** Jet3 column names are stored in the
  database's legacy single-byte code page, not UTF-8. Reading the
  actual code-page byte (documented at offset `0x3C` on the Database
  Definition page) would require decrypting page 0's RC4-obscured
  region -- the same encryption boundary noted above -- which this
  slice does not implement. Correct transcoding of a genuinely
  non-ASCII Jet3 name therefore isn't possible yet; what this slice
  does instead is guarantee the returned name is always valid UTF-8 (an
  invariant every other string this codebase returns upholds) by
  replacing any byte sequence that isn't valid UTF-8 with U+FFFD,
  rather than passing legacy-code-page bytes through unchanged and
  handing invalid UTF-8 to callers. A real ASCII-only Jet3 name --
  every column name observed in this slice's real-fixture
  cross-validation -- round-trips unchanged either way.

Container scanning also never buffers a whole database into memory at
once: `scan_access_container_schema()` first reads only each page's
leading 8 bytes (page type and `next_pg`) to discover TDEF pages and
their continuation chains, then reads one full page at a time only for
the pages it actually decodes. A real Access database can be up to
~2 GB (the Jet/ACE file-size ceiling), so this keeps memory use bounded
by page count and page size, not file size.

## ACE (`.accdb`) caveat

`AccessContainerGeneration::later` (the bucket #5521's container
detector uses for any generation byte the community sources do not
agree on a precise Jet/ACE-edition mapping for) is assumed to share
Jet4's 4096-byte page size in `access_container_page_size()`. This is
an **extrapolation**, not independently verified -- no real `.accdb`
fixture was available during this slice's development to confirm ACE's
actual page size or whether its TDEF/column-descriptor byte layout is
otherwise identical to Jet4's. A future slice with real ACCDB fixtures
should verify or correct this before treating ACE support as anything
more than "probably works the same as Jet4."

## Follow-up work

- **MSysObjects row decoding / table-name enumeration**: tracked as
  #5539, given the materially higher implementation risk `docs/68`
  already identified for the general row-decoding algorithm, now
  informed by this slice's real-fixture-verified TDEF groundwork.
- **Multi-page TDEF reassembly**, **index metadata decoding**, and
  **ACE-specific verification** are each noted above as deliberate,
  documented gaps rather than silently unhandled cases.
