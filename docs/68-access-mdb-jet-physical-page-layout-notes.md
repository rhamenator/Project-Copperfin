# Access MDB/ACCDB Physical Page Layout Notes

Clean-room reconnaissance for a prerequisite slice of issue #5476 (parent
#141, related #137, #5474), done before any physical-layout Access/JET/ACE
code, mirroring how `docs/13-index-format-notes.md` and
`docs/66-access-container-format-notes.md` ground earlier probe work before
that work's code was written. Per `docs/07-clean-room-rules.md`, this
records what is safe to build from and what is not.

## Relationship to `docs/66`

`docs/66-access-container-format-notes.md` (issue #5474) established that
**no official Microsoft specification exists for the physical MDB/ACCDB
byte layout** -- only the *logical* (DAO/OLE-DB/SQL) surface is officially
documented. It explicitly left physical page/table/column layout as future
work, to be sourced from community reverse-engineering documentation (with
honest attribution) or real fixture-based verification. This document is
that follow-up reconnaissance pass, scoped to what is needed for
container-level format/generation detection.

## Primary source

The **mdbtools** project's own `HACKING.md`
(<https://github.com/mdbtools/mdbtools/blob/dev/HACKING.md>, GPL) is the
most authoritative available community source: it is the design/format
notes written by the longest-running open-source MDB reader's own
maintainers, not a third-party summary. Corroborating secondary sources
used only to cross-check specific claims (not treated as independently
authoritative):

- <https://docs.fileformat.com/database/mdb/>
- <https://docs.fileformat.com/database/accdb/>
- <http://justsolve.archiveteam.org/wiki/Access>

All of these are community-authored, reverse-engineering-derived
documentation, not a Microsoft specification. Per `docs/66`'s boundary
note, any implementation built from this document must say so explicitly
in its requirements-traceability row rather than presenting this as
verified-against-an-official-spec or verified-against-real-Access-output.

## Container-level signature and generation byte (well-corroborated; safe to implement from)

Two or more independent sources agree on the following, so this is the
scope this document treats as safe to implement against without a real
fixture:

- The first four bytes of both `.mdb` and `.accdb` files are
  `0x00 0x01 0x00 0x00`.
- At byte offset 4, an ASCII string identifies the engine family:
  - `Standard Jet DB` for Jet-engine files (conventionally `.mdb`).
  - `Standard ACE DB` for ACE-engine files (conventionally `.accdb`).
  - This is a content-driven signature, independent of file extension --
    consistent with how Copperfin already classifies DBF families from
    header bytes rather than trusting the extension.
- At byte offset `0x14` (20 decimal), a single byte identifies the engine
  generation. The low values are well-corroborated:
  - `0x00`: Jet 3 (Access 95/97-era `.mdb`).
  - `0x01`: Jet 4 (Access 2000/2002/2003-era `.mdb`).
  - `0x02` and above: later generations. Sources **disagree on the precise
    mapping** here -- one source lists `0x02`=Jet 5, `0x03`=2010,
    `0x04`=2013, `0x05`=2016, `0x06`=2019; another reports `0x02`=ACE 12
    (Access 2007) and `0x03`=ACE 14 (Access 2010), while also noting an
    observed inconsistency where minimally-created 2010 and 2016 databases
    both showed `0x02`, and a 2016 database using the newer "large
    integer" column type showed `0x05`. This document does **not** assert
    a specific year/edition mapping for values `0x02` and above -- only
    that they represent "a later engine generation than Jet 3/Jet 4,"
    consistent with the ACE-vs-Jet family distinguished by the offset-4
    signature string. A future slice with real fixtures across Access
    versions should replace this coarse bucket with a verified mapping.

## What this document does not cover (left for the actual #5476 slice)

The deeper physical structure needed for **table/column enumeration** --
this document's reconnaissance stopped short of treating it as safe to
implement blind, for reasons recorded here rather than silently deferred:

- **Page layout** (page type byte, page size by generation, TDEF/data/
  index/usage-map page structures) and **column descriptor layout** are
  documented by mdbtools' `HACKING.md` in reasonable byte-level detail.
  However:
- **Table names are not stored in a table's own TDEF page.** They live in
  rows of the `MSysObjects` system catalog table (always rooted at page 2),
  which maps object name/type/flags to the page number of the object's own
  TDEF page. Reading `MSysObjects` requires the *general row-decoding
  algorithm* (null bitmask, variable-length column offset table stored in
  reverse order, and -- for Jet 3 -- a jump table for column offsets
  `>= 256`), applied against a **bootstrapped, hardcoded column layout**
  for `MSysObjects` itself, since a table's own TDEF is what a normal read
  needs to interpret its rows in the first place (mdbtools' own docs
  describe this as a "phony tdef" bootstrap).
- That row-decoding algorithm has several interacting, easy-to-get-subtly-
  wrong pieces (reverse-order offset tables, an offset-dependent jump
  table, null-bitmask bit ordering, UCS-2 compression toggling for Jet 4
  text). Unlike this document's container-signature scope -- which rests
  on independently checkable, corroborated constants (fixed signature
  bytes, a small well-agreed version-byte range) -- there is no equivalent
  independently-checkable invariant for the row-decode path: correctness
  there is much closer to "the byte offsets either match real Access
  output or they do not," and a hand-built synthetic fixture written from
  the same notes as the implementation would not actually prove that match
  (the fixture and the parser could share the same misunderstanding and
  silently agree with each other).
- No real Access-produced `.mdb`/`.accdb` file is available in this
  environment to cross-check against (the project's Windows VM was built
  for VFP9 access; Access is not installed there per
  `docs/66-access-container-format-notes.md`).

Per `docs/07`'s clean-room discipline and this project's established
practice of not shipping unverified binary-parsing logic for
consequential (data-import) paths, table/column/row enumeration is left
for a slice that either obtains real Access-produced fixtures (installing
Access on the existing Windows VM is the most direct path) or is executed
with materially more careful, human-reviewed synthetic-fixture validation
than a single autonomous pass affords.

## What this unblocks

A container-level MDB/ACCDB signature and generation detector -- see the
prerequisite slice tracked as a new issue linked from #5476 -- can be
built safely from this document's corroborated section alone.
