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

### #5478 may be further along than this alone suggests -- but only for Jet3/Jet4 MDB, and only as an unvalidated hypothesis so far

A prior investigation pass on #5478 (recorded in that issue's own comment
history, done after #5549's long-value/OLE reading landed) found a real,
if not yet structurally validated, lead -- **scoped to Jet3/Jet4 `.mdb`
only**. Every fixture that pass and this reconnaissance directly
inspected was a Jet3/Jet4 `.mdb`; no `.accdb` (ACE) fixture was checked
at all, and `read_access_long_value_column()` deliberately rejects
`AccessContainerGeneration::later` for page-backed values
(`access_long_value.cpp`), so this lead does not currently extend to ACE
even in principle without new work there:

- `MSysModules2` is a real, discovered Jet3/Jet4 system table (columns:
  `Flags`, `Form`, `Module`, `Name`, `ReplicationVersion`, `Type`,
  `TypeInfo`, `Version`) whose `Module` column is an OLE/long-value
  field -- `read_access_long_value_column()` (#5549) successfully
  retrieves its bytes from two real Jet3/Jet4 fixture rows (5752 and
  4113 bytes).
- Those retrieved bytes are **not themselves a raw OLE Compound File
  Binary (MS-CFB) structure** -- no CFB signature (`D0 CF 11 E0 A1 B1
  1A E1`) appears anywhere in either value. They *do* contain
  recognizable UTF-16LE substrings `_VBA_MODULE` and `_VBA_PROJECT`, the
  well-known MS-CFB stream names MS-OVBA's own storage layout uses --
  **but that is the only evidence found, and it is not sufficient on its
  own to conclude a complete MS-OVBA project (or any CFB blob with
  recoverable boundaries) is actually present.** Those substrings could
  just as easily be fragments or metadata within some other Access-
  internal serialization that happens to reuse MS-CFB's own stream-name
  strings. This is a **hypothesis pending structural validation**, not a
  confirmed payload location, and should be treated as such until
  something can actually parse a boundary, length field, or checksum
  around those substrings and get a self-consistent result.

So the accurate picture for #5478 is narrower than it might first read:
for **Jet3/Jet4 `.mdb` only**, there is a real, extractable candidate
column (`MSysModules2.Module`, retrievable today with shipped code) that
*might* hold the MS-OVBA payload wrapped in an undocumented framing --
but this has not been structurally confirmed, and nothing is currently
known about ACE/`.accdb`'s equivalent (if any) at all. This is worth
revisiting first if a future evidence source can validate or refute the
hypothesis (see "A third, lower-cost path" below), not treated as an
already-solved sub-problem.

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

## Two real paths forward, as first assessed (see the 2026-09-11 update below for what actually happened)

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

## Update 2026-09-11: path (a) is now verified, and it resolves the storage/architecture question for both issues

**This update resolves *where the data lives and how to reach it* for
both issues -- it does not itself satisfy either issue's acceptance
criteria (representative-fixture classification, malformed/unsupported-
input handling, focused tests, `docs/32` traceability, CHANGELOG entry).
Neither #5477 nor #5478 is closed by this update.**

The user installed Microsoft Access 365 on a clone of the project's
existing Windows VM (`copperfin-access365-win11`, cloned from
`copperfin-vfp9-win11` via `virt-clone` specifically to avoid COM-
registration conflicts between an old MSI-based Access and modern
Click-to-Run 365) and authorized proceeding with path (a). Real,
directly observed product behavior against a real fixture
(`Order Entry1.mdb`, a Jet3 `.mdb`) via COM automation
(`CreateObject("Access.Application")`, `.Visible = $false`,
`OpenCurrentDatabase()`, `SaveAsText()`):

- **`SaveAsText(acForm, "Switchboard", path)`** produced a complete,
  human-readable, structurally nested `Begin <ObjectType> ... End` text
  file (2080 lines) -- version/checksum header, form-level properties,
  a `NameMap` binary blob (unrelated GUID/name-resolution bookkeeping,
  not itself needed for structural inspection), and a fully nested
  control tree (`Section` containing `Label`/`Rectangle`/`Image`/
  `CommandButton`/etc., each with its own property list, `OnCurrent`/
  `OnOpen` event bindings shown as `"[Event Procedure]"` markers). This
  is a direct, storage-level answer to #5477's acceptance criteria
  ("control hierarchy, control types, bound field references, basic
  layout/property metadata") -- an actual conforming implementation
  still needs to be built and tested against it.
- **`SaveAsText(acReport, "Invoice", path)`** produced the identical
  `Version`/`Checksum`/`Begin Report ... End` grammar (1421 lines,
  same nested `Section`/control-property structure) -- reports and forms
  share one text format, not two.
- **`SaveAsText(acModule, "Global Code", path)`** produced the module's
  **raw, plain VBA source code verbatim** (14 lines, e.g. `Option Compare
  Database`, a full `Function IsLoaded(...) ... End Function` body) --
  no wrapper, no CFB structure, no compression.
- **Crucially, a standalone module is only part of #5478's own scope.**
  The *same* form export above (`Switchboard`) also contains, after its
  structural block's closing `End`, a literal `CodeBehindForm` marker
  line followed by that form's own class-module `Attribute VB_...`
  lines and its plain VBA source verbatim (confirmed for a report too --
  the marker is `CodeBehindForm` even for a report, not renamed). **Most
  real-world Access VBA lives in form/report event handlers, not
  standalone modules** -- a #5478 implementation that only enumerates
  `acModule` objects and ignores each form's/report's own code-behind
  would silently miss most of an application's actual code. A complete
  VBA-extraction implementation must enumerate all three object kinds
  (forms, reports, standalone modules) and combine each form's/report's
  code-behind with every standalone module's own source.
- Together, these observations mean the `MSysModules2.Module`/MS-OVBA
  byte-diffing plan recorded earlier in this document is no longer the
  best next step for the Jet3/Jet4 hypothesis it targeted: `SaveAsText`
  reaches the same VBA source directly, without needing to crack that
  wrapper at all, the same way `access-to-foxpro`'s own `access_design.py`
  already does it (per its `COPPERFIN.md` handoff notes).

This means: **neither #5477 nor #5478 needs any binary Jet/ACE reverse
engineering to reach the underlying data.** The earlier sections of this
document (page-type mapping, the disproven `candidate_page_number`
heuristic, the `MSysModules2.Module`/MS-OVBA hypothesis) remain accurate
as a record of what was checked, but are no longer the load-bearing path
forward -- they describe a harder problem than the one that actually
needs solving.

### Security requirement for any automation helper (real gap found in review)

`OpenCurrentDatabase()` against a database containing an `AutoExec`
macro, a startup form, or other active content can **execute that
content under the importing user's own authority** before `SaveAsText`
ever runs -- automation does not implicitly sandbox this the way a
read-only inspection tool needs. This directly conflicts with this
project's own security model (`docs/04-security-model.md`'s Runtime
Boundary explicitly protects both "COM/interop access" and "macro/eval
execution"), and turns what should be a read-only inspection path into
arbitrary code execution for an untrusted or malicious input file.

Any automation helper this architecture eventually ships **must** set
`Application.AutomationSecurity = msoAutomationSecurityForceDisable`
(disables all VBA macro execution for automation-opened databases for
the session) *before* calling `OpenCurrentDatabase()`, and this
invariant must be preserved in the helper's implementation, not treated
as optional hardening. This session's own ad hoc verification testing
(the exports described above) did **not** set this property -- an
acceptable risk for a single trusted, self-authored sample fixture
(`Order Entry1.mdb`) inspected interactively, but not a pattern to carry
into any real, shippable automation helper. `AutomationSecurity` only
disables macro execution, not the object model itself: this was
directly re-verified by rerunning the `Switchboard` form export above
with `AutomationSecurity = 3` set beforehand -- the resulting text file
was byte-for-byte identical to the original (unsecured) export, so
forcing macros off does not change or degrade `SaveAsText`'s own output
in any way.

### Implications for implementation architecture

Copperfin's forms/reports/VBA inspection for Access sources should be
built on `Application.SaveAsText`'s text output, not on raw container
parsing:

- **The automation helper must force-disable macros before opening any
  input database** (`Application.AutomationSecurity =
  msoAutomationSecurityForceDisable`, set before `OpenCurrentDatabase()`)
  -- see the security requirement above. This is not optional hardening;
  omitting it turns a read-only inspection tool into arbitrary code
  execution for an untrusted input file, directly conflicting with
  `docs/04-security-model.md`'s Runtime Boundary.
- **A complete VBA-extraction implementation (#5478) must enumerate
  all three object kinds** -- forms, reports, and standalone modules --
  and combine each form's/report's own `CodeBehindForm`-delimited
  code-behind with every standalone module's own source. Treating
  `acModule` exports alone as "the VBA" would silently miss most of a
  typical application's actual code, since event-handler code (by far
  the most common kind) lives in form/report code-behind, not standalone
  modules.
- This is a genuine, real dependency this codebase has not had before:
  a Windows machine with a licensed Access installation reachable at the
  time of import. Every other Access slice shipped so far
  (#5476/#5539/#5549/#5551) reads raw bytes with zero external
  dependencies. This slice cannot avoid that dependency -- there is no
  other way to reach this data (see the "central finding" above) -- so
  the design should isolate it cleanly: a small, separately invoked
  automation step (a PowerShell/VBScript helper, matching this
  project's existing "shell out to an external interpreter" precedent
  already used for its polyglot Python/.NET/R sidecars) produces the
  `SaveAsText` output files, and Copperfin's own portable C++ code parses
  *those already-produced text files* -- keeping the COM-automation
  dependency confined to one narrow, replaceable step, and the actual
  parsing logic portable and independently testable without Access
  installed anywhere.
- The `SaveAsText` text grammar itself (`Begin <Type> ... End`,
  indented property assignments, nested blocks) is not an officially
  published Microsoft specification, but it is directly, repeatedly
  observable from a real licensed installation -- exactly the "observed
  product behavior" evidence category `docs/07`/`docs/66` already treat
  as legitimate, and the same standard the CDX writer's real-VFP9
  verification (`docs/77`) relied on. Parsing it is a clean-room text-
  grammar problem, not a binary reverse-engineering problem -- a much
  more tractable and independently verifiable task (more fixtures can be
  generated on demand from the real installation to check any parsing
  hypothesis, unlike the binary investigation above, which had no
  oracle to check guesses against).
- `NameMap` and other embedded binary blobs inside the text (GUID/name-
  resolution bookkeeping) can be treated as opaque/skipped for a first
  slice -- #5477's own acceptance criteria (control hierarchy, control
  types, basic properties) does not require decoding them.

### Update 2026-09-11 (continued): the external-process admission step landed

The "implications for implementation architecture" section above named
the remaining piece: a small, separately invoked automation step, given
the same external-process admission treatment
`samples/polyglot-python-sidecar/` already has (pinned digest, admitted
root, revalidation). That piece now exists --
`run_access_saveastext_export()`
(`include/copperfin/vfp/access_saveastext_export.h`,
`src/vfp/access_saveastext_export.cpp`) admits and revalidates both the
PowerShell host (by physical location, `authorize_external_process()` --
not digest-pinned, since it is a well-known OS-shipped executable that
changes with every servicing update) and the checked-in
`export_access_design.ps1` script (digest-pinned,
`admit_polyglot_supporting_artifact()`), launches it with the script's
own revalidated resolved path bound to a fixed argument position, and
reads back the `manifest.json` it writes.

This is genuinely tested end-to-end against the real, checked-in
script and a real local PowerShell process -- including a run against a
real-but-fake database file that gets far enough to create the output
directory before real Access automation itself (unavailable in this
development environment) fails, proving the admission-and-launch
mechanics work without needing a licensed Access installation to verify
them. It does **not** close either #5477 or #5478 on its own: no single
function yet combines this export step with
`parse_access_saveastext_design_from_file()` into one coherent per-
database structural-inspection or VBA-extraction result, no PRG-level
command surface exists (`#5517`'s own `IMPORT DATABASE` wizard remains
unbuilt), no VBA-classification/aggregation logic exists for #5478
specifically, and a genuine real-Access end-to-end run (this session's
own environment has no licensed Windows Access installation) remains
outstanding.

### A third, lower-cost path specific to #5478's Jet3/Jet4 hypothesis (superseded, kept for the record)

**Superseded by the 2026-09-11 update above** -- `SaveAsText` gives the
VBA source directly, so this byte-diffing plan is no longer needed.
Kept here only as a record of a path that was considered before real
Access automation became available.

Because #5478's Jet3/Jet4 lead is at least narrowed to one candidate
column (`MSysModules2.Module`, see above) rather than being completely
unlocated, there is a cheaper option than installing Access on
Copperfin's own VM to validate or refute it: `rhamenator/access-to-
foxpro` (see `project_copperfin_access_to_foxpro_reference` memory)
already extracts real VBA source from real Access databases via Access
automation on the user's own machine, and the user has separately had
Codex run it against databases beyond the original employment
application it was developed against. Once real "raw `MSysModules2.
Module` bytes in -> correct VBA source out" pairs exist for the *same*
real Jet3/Jet4 database from that tool, byte-diffing the two (or a
re-compressed round-trip of the known-correct source) could confirm
*whether* the `_VBA_MODULE`/`_VBA_PROJECT` substrings really do bound an
MS-OVBA payload at all, and if so reveal its wrapper's header/compression
scheme -- without requiring Access to be installed anywhere in
Copperfin's own environment. This turns an open-ended reverse-engineering
problem into a much more constrained "test and, if confirmed, solve for
this one framing format" problem. It applies only to Jet3/Jet4 `.mdb`
(ACE/`.accdb` has no equivalent lead yet) and does not help #5477 (no
equivalent narrowed-down payload location exists there yet, for either
generation).

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
