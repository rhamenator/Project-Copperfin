# VFP DBC Stored Procedures and View SQL Storage

Written for issue #5538 (parent #137, related #5471/#5537/#113):
`copperfin::vfp::extract_dbc_stored_procedures_source()` in
`src/vfp/asset_inspector.cpp`, which extracts a database container's
Stored Procedures source code -- a genuine Visual FoxPro compatibility
requirement, not a modernization extra (#5537's own non-goal points back
to this issue as its prerequisite).

## Scope of this slice

This is a **partial** first slice, per the issue's own explicit framing
("Scope ideas: ... confirm before assuming it is 'just another PROPERTIES
entry'"):

- **Implemented**: Stored Procedures source-code extraction.
- **Deliberately deferred**: SQL view definition text extraction. See
  "What this slice does not cover" below for why.

## Evidence basis: Stored Procedures

Unlike the Access/Jet physical byte format (docs/66/68), a `.dbc`'s own
catalog table is not a proprietary undocumented binary layout -- it is
itself an ordinary Visual FoxPro table, and Microsoft's own archived
product documentation and Knowledge Base directly describe how Stored
Procedures are stored in it:

- Microsoft's archived Visual FoxPro Knowledge Base article **Q180028**
  ("PRB: DBC With Stored Procedure Produces Error When Opened",
  <https://jeffpar.github.io/kbarchive/kb/180/Q180028/>) demonstrates
  opening a real `.dbc` directly as a table (`USE 'test.dbc'`) and
  locating/reading/replacing a specific catalog row's `CODE` memo field
  (`APPEND MEMO Code FROM [Test.prg]`, `REPLACE Code WITH ...`) to work
  around a corrupted stored procedure. This is Microsoft's own
  documentation demonstrating the exact mechanism, not community
  reverse-engineering speculation.
- That same source, cross-checked against secondary community
  documentation, identifies the specific catalog row names used: a row
  named **`StoredProceduresSource`** holds the actual PRG source text (in
  its `CODE` memo field); a sibling row named `StoredProceduresObject`
  holds the compiled p-code for that source. This codebase only reads the
  former -- it does not attempt to interpret or decode the latter as
  text, since compiled p-code is binary, not source.
- General DBC catalog structure (`ObjectType`/`ObjectName`/`ParentID`/a
  `Properties` memo field) was already established and implemented by
  this codebase's existing catalog reader
  (`load_database_catalog_snapshot()`,
  `decode_dbc_properties_blob()`) before this slice; this slice only adds
  a `CODE` field lookup alongside the existing `PROPERTIES` one, reusing
  the same catalog-row-scanning infrastructure and the same
  field-name-tolerance convention (`"OBJECTNAME"`/`"OBJNAME"`/`"NAME"`/
  `"OBJECT"`, etc.) that infrastructure already has.

No real VFP-produced `.dbc`/`.dct` fixture with an actual Stored
Procedures page was available during this slice's development to
byte-for-byte cross-check against (the project's Windows VM, built
specifically for direct VFP9 access, was unreachable over the network
during this slice's development -- see the follow-up note below). The
committed test suite therefore uses a synthetic fixture built from the
verified field/row-name layout above (`tests/test_vfp_assets.cpp`,
`test_extract_dbc_stored_procedures_source_reads_code_memo` and its
siblings), matching this project's established precedent (docs/71/72) of
not committing real fixture files and of being explicit that a synthetic
fixture proves internal consistency with the documented layout, not
independent verification against real VFP-produced bytes the way the
Access work's real-fixture cross-validation achieved.

## What this slice does not cover: SQL view definition text

A SQL view's defining `SELECT` text is **not** covered by this slice.
Multiple searches for authoritative documentation of exactly where/how a
view's SQL text is physically stored (inside the `Properties` memo blob
under a specific key, in a separate field, or elsewhere) did not turn up
a source with the same directness as Q180028's Stored-Procedures
demonstration -- general VFP documentation (the View Designer's SQL
window, `CREATE SQL VIEW`) describes the *feature* thoroughly but not the
DBC-internal storage mechanism. Guessing a specific `Properties` key name
without verification would risk exactly the kind of unverified,
un-cross-checked implementation this project's evidence-provenance
discipline exists to avoid (see docs/68's original caution, echoed
throughout the Access-format work: "the byte offsets either match real
[VFP] output or they do not").

This codebase's existing generic `Properties` blob decoder
(`decode_dbc_properties_blob()`) already surfaces every key/value pair it
finds in a view's row, regardless of key name -- so raw access to
whatever a view's `Properties` blob contains is not itself blocked by
this gap. What is missing is confidently labeling one of those decoded
properties as "this is the view's defining SQL" as first-class,
structured output the way `extract_dbc_stored_procedures_source()` now
does for Stored Procedures.

## Follow-up work

- **View SQL text extraction** remains open, tracked under #5538 (this
  issue stays open, not closed, by this slice's PR). The most direct path
  to resolving it is creating a real VFP-produced `.dbc` containing an
  actual `CREATE SQL VIEW`-defined view on the project's Windows VM
  (`copperfin-vfp9-win11`, built for exactly this kind of direct-VFP9-
  fixture-creation need) and inspecting its real `Properties` blob bytes
  directly, the same real-fixture-verification method that resolved the
  Access MSysObjects work's own uncertainty (docs/72). The VM was
  unreachable over the network during this slice's development (appeared
  to be at its Windows lock screen but not responding to SSH/ICMP on its
  expected address) -- a transient/environmental issue to revisit, not a
  structural blocker.
- **#5537** (vendor-dialect SQL export including stored procedures and
  views) depends on this issue for its own stored-procedure/query export
  scope, per that issue's own non-goal note.
