# Access Container Format Notes

Clean-room reconnaissance for issue #5474 (parent #141, related #137), done
before any Access/JET/ACE code, mirroring how `docs/13-index-format-notes.md`
grounded the CDX/IDX/NDX/MDX probe work before that code was written. Per
`docs/07-clean-room-rules.md`, this records what is safe to build from and
what is not, rather than letting later slices guess at runtime.

## Central finding: there is no official Microsoft specification for the on-disk MDB/ACCDB format

This issue's own scope text names "MS-JETDB (Jet/ACE database file format)"
as an assumed citable Microsoft Open Specification, by analogy with
`MS-DOC`/`MS-XLS`/`MS-PPT` for the legacy Office binary document formats.
That assumption does not hold. Web research for this reconnaissance (search
queries against Microsoft Learn's Open Specifications library, the Library of
Congress's format-description pages, and general web search; see Sources
below) turned up no `MS-JETDB`, `MS-ACCDB`, or equivalently named Open
Specification document, and multiple independent sources state directly that
the physical MDB/ACCDB file format has never had an official Microsoft
specification released:

- Microsoft's Open Specifications program covers the legacy Office
  *document* binary formats (Word/Excel/PowerPoint, part of the
  interoperability commitments that produced `MS-DOC` etc.) but does not
  extend to the Jet/ACE database engine's on-disk container format.
- All existing third-party MDB/ACCDB readers -- **mdbtools** (GPL,
  C, the longest-running community implementation), **Jackcess** (Apache
  2.0, Java, credits mdbtools as a reference and independently reverse-
  engineered JET 4 index metadata layout), and **"The Unofficial MDB
  Guide"** (a community-authored format writeup) -- are explicitly built on
  reverse engineering, not an official spec. Search results state this
  directly for both `.mdb` and the newer `.accdb`/ACE format.
- What Microsoft *does* publish is the **logical** data-access surface, and
  it is split by engine, not unified -- an important distinction this
  document's first pass conflated (caught in review, corrected here):
  - **Jet (`.mdb`)**: the OLE DB Provider for Microsoft Jet, documented on
    Microsoft Learn (`Microsoft OLE DB Provider for Microsoft Jet`; see
    Sources). Covers the Jet/MDB logical surface only.
  - **ACE (`.accdb`)**: connects as the separate `Microsoft.ACE.OLEDB.12.0`
    provider (part of the Access Database Engine / "Access Database Engine
    Redistributable"), not the Jet provider. ACE adds logical constructs
    Jet does not have -- multi-valued fields, attachment fields, and
    64-bit integer columns -- so a future slice designing ACCDB support
    from the Jet OLE DB documentation alone would be working from an
    incomplete contract. This reconnaissance pass found general-audience
    confirmation that these ACE-specific types exist and their connection
    string form, but did not find a single discrete "OLE DB Provider for
    ACE" reference page analogous to the Jet one -- the ACE-specific
    logical surface (Attachment/Multi-value field data types) appears to
    live primarily in Access's own end-user/VBA/DAO documentation rather
    than a unified OLE-DB-provider reference. A future slice should verify
    that directly rather than assume it from this summary.
  - Either way, this logical surface documents what can be
    queried/manipulated through the API (tables, columns, indexes,
    relationships, SQL/DDL dialect) -- it is a schema/query contract, not
    a physical byte-layout specification. It is legitimate "public
    documentation" input per `docs/07`, but it does not by itself let a
    new implementation read or write raw MDB/ACCDB bytes, for either
    engine.

This is a real correction to the issue's premise, not a detail to route
around silently -- the `docs/07` allowed-inputs list is narrower for this
format family than it is for CDX/IDX/DBF, where Microsoft's own VFP
documentation and installed product behavior are directly citable.

## What is and is not safely usable, per `docs/07-clean-room-rules.md`

**Allowed:**

- **Public documentation** of the *logical* surface: Microsoft Learn's DAO/
  ADO/OLE-DB-for-Jet reference, and publicly documented Access SQL/DDL
  dialect references. Grounds what a round-trip needs to preserve
  (table/column/index/relationship semantics, supported types, SQL dialect)
  without grounding physical byte layout.
- **Observed product behavior**: if a real, licensed Access install becomes
  available (this repository now has a Windows VM built for VFP9 access,
  per `agent-handoff.md`'s 2026-09-04 entry -- Access is not yet installed
  there), creating known-content `.mdb`/`.accdb` files and inspecting them
  byte-for-byte is allowed evidence, exactly like the CDX/IDX fixture work
  in `docs/13`.
- **Community-maintained projects, for study, not transplantation**: reading
  mdbtools/Jackcess source and "The Unofficial MDB Guide" to *learn* page
  structure, table/column metadata layout, index structure, and the JET-vs-
  ACE encryption/compression deltas is allowed under `docs/07`'s "Safe Use
  Of Community Projects" section -- "what the community had to fix, how
  features were organized" -- but per that same section and the clean-room
  objective ("without copying proprietary implementation code"), this means
  *learning from and independently reimplementing*, not porting code even
  where license terms would technically permit it (mdbtools is GPL,
  compatible with Copperfin's GPL-3.0-only license; Jackcess is Apache 2.0).
  Record why a structural decision was made, not just that a community
  project did it that way.

**Not allowed:**

- Decompiled Access/JET/ACE engine binaries, per `docs/07`'s restricted
  inputs and this issue's explicit non-goal statement.
- Treating any community project's reverse-engineered documentation as
  Microsoft-authoritative -- it is community evidence, cite it as such, and
  prefer independently observed product behavior wherever the two might
  conflict.

## Boundary this leaves for later slices (#141, #138)

- **Page structure, table/column/index layout**: not independently verified
  by this reconnaissance pass; available only through community
  reverse-engineering documentation (mdbtools, Jackcess, "The Unofficial MDB
  Guide") until real Access-produced fixtures can be inspected directly. A
  future slice implementing `IMPORT`/`EXPORT DATABASE ... TYPE ACCESS`
  (#141) or Access forms/reports/VBA inspection (#138) must not treat that
  community documentation as ground truth without either (a) direct
  fixture-based verification against real Access-produced files, or (b)
  explicit acknowledgment in that slice's traceability row that the
  evidence source is community reverse-engineering, not an official spec or
  observed behavior -- matching this repo's evidence-provenance discipline
  for recovered requirements.
- **JET vs. ACE encryption/compression**: MDB (JET, `.mdb`) and ACCDB (ACE,
  `.accdb`) are confirmed to differ here by every source found, but the
  exact mechanism needs its own focused reconnaissance pass against
  community documentation and/or real fixtures before any slice depends on
  reading or writing encrypted/compressed containers -- out of scope for
  this notes document.
- **Implementation approach** (native reader/writer vs. shelling out to an
  existing library vs. something else): explicitly not decided here, per
  this issue's own non-goals; belongs to the slices this unblocks.

## Sources

- Web search across Microsoft Learn's Open Specifications library and
  general sources found no `MS-JETDB`/`MS-ACCDB`/equivalent Open
  Specification document for the physical MDB/ACCDB format.
- Library of Congress format-description pages for the MDB and ACCDB format
  families:
  - <https://www.loc.gov/preservation/digital/formats/fdd/fdd000462.shtml>
    (MDB)
  - <https://www.loc.gov/preservation/digital/formats/fdd/fdd000463.shtml>
    (ACCDB)
  - Referenced via search result summaries; direct fetch returned HTTP 403
    in this session and should be retried by a future session before this
    doc is treated as final on that point.
- Community reverse-engineering references (project documentation/history
  referenced via search result summaries, not independently read
  line-by-line in this session):
  - mdbtools (GPL): <https://github.com/mdbtools/mdbtools>
  - Jackcess (Apache 2.0): <https://jackcess.sourceforge.io/> and its FAQ,
    <https://jackcess.sourceforge.io/faq.html>, which states its own
    reverse-engineering provenance and credits mdbtools
  - "The Unofficial MDB Guide" -- referenced by name in search results;
    this session did not capture a stable direct URL for it, and a future
    session should locate and verify one before citing it further
  - A 2025-11-20 writeup specifically on ACCDB reverse engineering and
    encryption was found (`yingtongli.me/blog/2025/11/20/accdb.html` and
    its companion encryption post) but returned HTTP 403 on direct fetch
    in this session -- noted as a candidate source, not verified
- Microsoft Learn, Jet/MDB logical surface:
  - <https://learn.microsoft.com/en-us/sql/ado/guide/appendixes/microsoft-ole-db-provider-for-microsoft-jet>
  - <https://learn.microsoft.com/en-us/office/client-developer/access/desktop-database-reference/microsoft-ole-db-provider-for-microsoft-jet>
  - Titles and general content confirmed via search result listings; full
    content not fetched in this session.
- ACE/ACCDB logical surface (provider name and ACE-specific type
  existence only, per the correction above -- not a discrete OLE-DB-for-
  ACE reference page):
  - <https://www.microsoft.com/en-us/download/details.aspx?id=54920>
    (Microsoft Access Database Engine 2016 Redistributable download page,
    the source of the `Microsoft.ACE.OLEDB.12.0` provider)
  - General confirmation of `.accdb`-only Multi-Valued Field, Attachment,
    and BigInt support via search result summaries of secondary sources
    (ConnectionStrings.com, a general Access Database Engine encyclopedia
    entry); none of these are Microsoft-authoritative and should be
    replaced with a primary Microsoft reference before a future slice
    relies on this claim.

**This reconnaissance pass used search-result summaries and page-title/
listing confirmation, not full independent reading of every source above --
several direct fetches returned HTTP 403 or 404 in this session's sandboxed
network environment.** A future slice that begins real implementation work
should re-verify the specific claims above (especially the "no official
spec exists" finding, since that is the load-bearing conclusion of this
document) against full source text before treating them as settled, per
`docs/07`'s clean-room discipline.
