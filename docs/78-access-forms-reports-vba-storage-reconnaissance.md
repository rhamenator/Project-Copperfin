# Access Forms/Reports/VBA-Module Storage Reconnaissance

Clean-room reconnaissance for #5477 (forms/reports structural inspection)
and #5478 (VBA/Access Basic project extraction), done before writing any
extraction code, per `docs/07-clean-room-rules.md` and mirroring how
`docs/66-access-container-format-notes.md` grounded (and corrected) the
container-signature work before #5476 was implemented.

## Central finding: this is not the same kind of problem as tables/queries/indexes, and cannot be solved with currently available evidence

Every prior Access slice this project has shipped (#5476's container/TDEF
reading, #5539's MSysObjects row decoding, #5551's saved-query
reconstruction, #5549's long-value/OLE reading) worked because the target
was a genuine **Jet/ACE database-engine construct** -- table/column/row
storage -- which, while never officially specified by Microsoft, has at
least *some* community reverse-engineering coverage (mdbtools' `HACKING.md`,
Jackcess) this project could learn from and independently verify against
real fixtures.

Forms, Reports, and VBA Modules are different in kind, not just degree:
they are **Access *application*-layer objects** (members of the Access
object model's `Forms`/`Reports`/`Modules`/`VBProject` collections),
persisted inside the same `.mdb`/`.accdb` file as a storage convenience,
but never intended to be read through the Jet/ACE data-access surface.
Every piece of real-world tooling this reconnaissance could find --
Access itself, mdbtools, Jackcess, oletools -- confirms this:

- **mdbtools' `HACKING.md` (fetched fresh this session, `apt-get source
  mdbtools`, version 1.0.1) documents zero coverage of form/report/module
  storage.** Its only mention of "module" anywhere in the source tree is
  the `MDB_MODULE` object-type label used for `--type` filtering in
  `mdb-tables`/`mdb-export` -- there is no code anywhere in the 1.0.1
  source that reads a module's, form's, or report's own design/code
  content.
- mdbtools' `HACKING.md` *does* document a real, related-but-distinct
  mechanism worth recording for a future table/column-metadata slice:
  the `LvProp` column of `MSysObjects` holds a chunked properties-bag
  format (`'KKD\0'` magic in Jet3, `'MR2\0'` in Jet4; length-prefixed
  chunks, type `0x0080` for a name table, `0x0000`/`0x0001` for values)
  covering things like column defaults, captions, and formats. This is
  **not** form/report/module storage -- it is *table/column* design
  metadata -- but it is the closest documented thing to a "properties"
  mechanism in the whole spec, and is worth returning to for a future
  #5476-family slice that wants column defaults/captions/formats.
- **oletools (installed this session as independent confirmation, see
  the #5477/#5478 investigation-comment history)** implements a real,
  Microsoft-documented spec for VBA project storage --
  [MS-OVBA] "Office VBA File Format Structure" -- and successfully
  extracts VBA from Word/Excel/PowerPoint (`.doc`/`.xls`/`.ppt`, all
  genuine OLE/CFB compound-file containers). It has an **explicit,
  unimplemented TODO specifically for Access**, because `.mdb`/`.accdb`
  is not an OLE/CFB container at all -- it's the Jet/ACE format -- so
  even a tool that fully understands MS-OVBA-formatted VBA project bytes
  still needs to know *where inside the Jet/ACE container those bytes
  live*, and that embedding mechanism is exactly the undocumented part.

This reframes #5478 in a genuinely useful way: **the VBA *payload* format
(MS-OVBA) is real, citable, Microsoft-documented public documentation**,
distinct from the *container* problem (locating and extracting that
payload's bytes from inside a Jet/ACE `.mdb`/`.accdb` file, which remains
undocumented). #5477 (forms/reports) does not have an equivalent
documented payload format -- Access's own form/report control-layout
binary format has no MS-OVBA-equivalent public specification this
reconnaissance could find.

### #5478 is further along than this alone suggests: the container half is partly located, just not fully unwrapped

A prior investigation pass on #5478 (recorded in that issue's own comment
history, done after #5549's long-value/OLE reading landed) got closer
than this session's own fresh page-type probing realized at first:

- `MSysModules2` is a real, discovered system table (columns: `Flags`,
  `Form`, `Module`, `Name`, `ReplicationVersion`, `Type`, `TypeInfo`,
  `Version`) whose `Module` column is an OLE/long-value field --
  `read_access_long_value_column()` (#5549) successfully retrieves its
  bytes from two real fixture rows (5752 and 4113 bytes).
- Those retrieved bytes are **not themselves a raw OLE Compound File
  Binary (MS-CFB) structure** -- no CFB signature (`D0 CF 11 E0 A1 B1
  1A E1`) appears anywhere in either value -- but they *do* contain
  recognizable UTF-16LE substrings `_VBA_MODULE` and `_VBA_PROJECT`,
  the well-known MS-CFB stream names MS-OVBA's own storage layout uses.
  This means the MS-OVBA-formatted project **is in there**, wrapped in
  some other, undocumented Access-internal framing (a length/offset
  header, Access's own additional compression, or both) that this
  session did not crack.

So the accurate picture for #5478 is: the column holding the payload is
known and already extractable with shipped code (`MSysModules2.Module`
via `read_access_long_value_column()`); what remains undocumented is
only the wrapper immediately around the MS-OVBA CFB blob, not the whole
container-to-payload path. This is a narrower, more tractable-looking gap
than #5477's -- worth revisiting first if a future evidence source
narrows it further (see "A third, lower-cost path" below).

## Methodology and evidence: what this session directly verified

Using the real, locally available Jet3 fixtures (`~/Downloads/Access
Databases/*.mdb`, see `project_copperfin_access_fixtures_available`
memory -- private user data, never committed, inspected for structure
only, never for record content), this session:

1. Ran `scan_access_msysobjects_catalog()` (already-shipped, #5539)
   against three real fixtures (`Employment Information.mdb`,
   `Employment Information (Access 2000).mdb`, `Order Entry1.mdb`) and
   filtered for `Type & 0x7F` in `{0 (MDB_FORM), 4 (MDB_REPORT), 7
   (MDB_MODULE)}`, confirming real Form/Report/Module rows decode
   correctly (names, parent Ids, and `AccessCatalogEntry::type` all
   plausible and consistent across all three fixtures).
2. **Disproved, via direct real-fixture inspection, that
   `AccessCatalogEntry::candidate_page_number` (`id & 0x00FFFFFF`) is a
   usable pointer for Form/Report/Module rows** -- this codebase's own
   header comment already flagged this value as "only meaningful for
   Type == 1 [table] rows... NOT independently verified... on its own"
   for other types, and this reconnaissance confirms that caveat is load-
   bearing: following `candidate_page_number` for a real Form row lands
   on pages whose own type byte does not match a table-definition page at
   all.
3. Built a throwaway page-type-mapping probe (scratchpad-only, not
   committed) and cross-referenced every page's own type byte against
   mdbtools' `HACKING.md`'s page-type table:
   ```
   0x00 Database definition page (always page 0)
   0x01 Data page
   0x02 Table definition (TDEF)
   0x03 Intermediate index page
   0x04 Leaf index page
   0x05 Page usage bitmap
   ```
   The pages `candidate_page_number` pointed to for Form/Report rows were
   type `0x03`/`0x04` -- **B-tree index pages, not form/report design
   pages at all** -- confirming the pointer is simply meaningless noise
   for these row types (an `id` value that happens to numerically collide
   with an unrelated index page somewhere in the file), not a real,
   undiscovered mechanism this session almost found.
4. Confirmed no readable text, no `LVAL` marker (the existing
   `access_long_value.cpp` OLE/memo chain marker, `0x01` page type +
   `"LVAL"` at byte offset 4), and no recognizable structure anywhere
   near those pages that would suggest an easily-recoverable adjacent
   payload.

## What this means for #5477 and #5478's own scope text

Both issues' "Blocked on" / grounding language ("grounded in the same
MS-JETDB/ACE Open Specification family as #5474/#5476", "a working Access
container reader... covering form/report object storage specifically")
assumed forms/reports/modules are reachable the same way tables are, once
a container reader exists. **This reconnaissance finds that assumption
does not hold** -- matching `docs/66`'s own precedent of correcting a
sibling issue's premise rather than silently working around it. There is
no known pointer, from any MSysObjects column this session could locate,
to a Form/Report/Module's own design or code storage, and no public or
community documentation (checked: mdbtools 1.0.1's full source and
`HACKING.md`, oletools' own explicit non-coverage) describes one.

## Two real paths forward -- both require infrastructure this session cannot provide

- **(a) Real Access automation**, matching how `rhamenator/access-to-
  foxpro` (see `project_copperfin_access_to_foxpro_reference` memory)
  solves this: install a licensed Microsoft Access on the existing
  `copperfin-vfp9-win11` VM (currently VFP9-only, no Access) and use
  Access's own `Application.SaveAsText` (forms/reports/modules as
  human-readable text) and/or the VBA Extensibility library, the same
  way this project already treats "observed product behavior from a
  real, licensed install" as legitimate clean-room evidence (`docs/07`,
  `docs/66`'s own "Allowed" list) -- exactly the precedent that made the
  CDX writer's real-VFP9 verification (docs/77) possible. This would make
  Access forms/reports/VBA inspection depend on a real Access
  installation being present wherever that inspection runs, which no
  other Copperfin import/export path currently requires -- a real
  architecture decision, not a detail, and one this reconnaissance
  deliberately does not make unilaterally.
- **(b) Independent from-scratch binary reverse engineering** of the
  Form/Report/Module storage mechanism with no reference material at
  all (unlike every prior Access slice, which had at least mdbtools/
  Jackcess to learn from) -- open-ended, unbounded effort with no way to
  verify correctness short of (a) anyway, since there is no other ground
  truth to check hypotheses against.

Neither path is something this reconnaissance pass attempts to start,
per `docs/68`'s own established caution against shipping unverified
binary-parsing logic for a consequential (data-import) path, and per
this project's clean-room discipline against guessing. This document
records what was checked and found, so a future slice -- whichever path
is chosen -- does not have to re-derive it.

### A third, lower-cost path specific to #5478's narrower remaining gap

Because #5478's remaining unknown is *only* the wrapper immediately
around an already-located MS-OVBA CFB blob (see above), there is a
cheaper option than installing Access on Copperfin's own VM: `rhamenator/
access-to-foxpro` (see `project_copperfin_access_to_foxpro_reference`
memory) already extracts real VBA source from real Access databases via
Access automation on the user's own machine, and the user has separately
had Codex run it against databases beyond the original employment
application it was developed against. Once real "wrapper bytes in ->
correct VBA source out" pairs exist for the *same* real database from
that tool, byte-diffing the raw `MSysModules2.Module` value against the
known-correct decompiled source (or a re-compressed round-trip of it)
could reveal the wrapper's header/compression scheme without requiring
Access to be installed anywhere in Copperfin's own environment -- turning
an open-ended reverse-engineering problem into a much more constrained
"solve for this one framing format" problem. This does not help #5477
(no equivalent narrowed-down payload location exists there yet).

## Sources

- mdbtools 1.0.1 source (`apt-get source mdbtools`, GPL): `HACKING.md`
  (page-type table, `LvProp` properties-bag format) and full-source grep
  for `module`/`vba` (found only the `MDB_MODULE` type-filter label, no
  content-reading code).
- oletools (installed in an isolated Python venv this session, not a
  runtime dependency): its own source confirms MS-OVBA support for
  OLE/CFB-container formats and an explicit absence of Access support.
- This session's own real-fixture probing (three real Jet3/Jet4 `.mdb`
  files, see the fixtures-available memory) via `scan_access_
  msysobjects_catalog()` and a scratchpad-only page-type-mapping tool
  (not committed).
