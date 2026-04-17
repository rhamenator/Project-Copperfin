# VFP Language Reference Coverage

This document anchors Copperfin's command/runtime backlog to the official Visual FoxPro reference instead of memory.

## Primary Sources

- Microsoft Learn language index:
  - <https://learn.microsoft.com/en-us/previous-versions/visualstudio/foxpro/dd5f4hyy(v=vs.71)>
- Microsoft Learn general reference:
  - <https://learn.microsoft.com/en-us/previous-versions/visualstudio/foxpro/h9f898ae(v=vs.71)>
- Microsoft Learn VFP home page:
  - <https://learn.microsoft.com/en-us/previous-versions/visualstudio/foxpro/mt490117(v=msdn.10)>
- Local fallback when Learn coverage is incomplete or too terse:
  - [`C:\vDosPlus\FPD26\FOXHELP.DBF`](C:/vDosPlus/FPD26/FOXHELP.DBF)
  - companion memo file in the same directory

## Generated Inventory

The generated command inventory lives here:

- [`docs/generated/vfp-language-reference-summary.json`](E:/Project-Copperfin/docs/generated/vfp-language-reference-summary.json)
- [`docs/generated/vfp-language-reference-commands.json`](E:/Project-Copperfin/docs/generated/vfp-language-reference-commands.json)
- [`docs/generated/vfp-language-reference-commands.txt`](E:/Project-Copperfin/docs/generated/vfp-language-reference-commands.txt)
- [`docs/generated/vfp-chm-index-summary.json`](E:/Project-Copperfin/docs/generated/vfp-chm-index-summary.json)
- [`docs/generated/vfp-chm-command-topics.json`](E:/Project-Copperfin/docs/generated/vfp-chm-command-topics.json)
- [`docs/generated/vfp-foxtools-topics.json`](E:/Project-Copperfin/docs/generated/vfp-foxtools-topics.json)
- [`docs/generated/vfp-chm-topic-manifest.json`](E:/Project-Copperfin/docs/generated/vfp-chm-topic-manifest.json)

Refresh it with:

```powershell
powershell -ExecutionPolicy Bypass -File E:\Project-Copperfin\scripts\export-vfp-reference-index.ps1
powershell -ExecutionPolicy Bypass -File E:\Project-Copperfin\scripts\export-vfp-chm-topic-index.ps1
```

Current official index snapshot from the Learn language-reference page:

- `429` command entries
- `413` function entries
- `323` property entries
- `83` method entries
- `72` system variable entries
- `69` event entries
- `22` object entries
- `4` preprocessor-directive entries
- `3` operator entries

Current local CHM snapshot from the installed VFP help set:

- `dv_foxhelp.chm` is the main product help and the command-behavior source that matters most
- `foxtools.chm` is supplemental and much smaller
- the local indexer now emits both a command-topic view and a broader topic manifest, so it can capture `foxtools` functions that are not indexed as `Command` topics
- the generated CHM outputs give local keyword-to-HTML mappings for offline behavior lookup, including `RegFn()`, `RegFn32()`, and `CallFn()`

## Current Copperfin Runtime Surface

The native runtime/parser currently has first-pass support for these command families:

- `DO`, `DO FORM`, `REPORT FORM`, `LABEL FORM`
- `ACTIVATE POPUP`, `ACTIVATE MENU`, `RELEASE POPUP`, `RELEASE MENU`
- `RETURN`
- `IF/ELSE/ENDIF`
- `DO CASE/CASE/OTHERWISE/ENDCASE`
- `FOR/ENDFOR`
- `DO WHILE/ENDDO`
- `WITH/ENDWITH`
- `TRY/CATCH/FINALLY/ENDTRY`
- `LOOP`, `CONTINUE`, `EXIT`
- `TEXT/ENDTEXT` with first-pass literal `TEXT TO ... [ADDITIVE] [NOSHOW]` capture
- `LOCATE`
- `SCAN/ENDSCAN`
- `CALCULATE`
- `COUNT`
- `SUM`
- `AVERAGE`
- `TOTAL`
- `READ EVENTS`, `CLEAR EVENTS`
- `REPLACE`
- `APPEND BLANK`
- `DELETE`, `RECALL`
- `DELETE FROM`
- `INSERT INTO`
- `PACK`, `ZAP`
- `SET FILTER TO/OFF`
- `SET ORDER TO TAG`
- `SEEK`
- `GO` / `GOTO`
- `SKIP`
- `SELECT`
- `USE`, `USE IN`, `USE AGAIN`
- `SET DATASESSION TO`
- `SET DEFAULT TO`
- `SET`
- `ON ERROR`
- `PUBLIC`
- `LOCAL`
- `PRIVATE`
- `DIMENSION` / array-form `DECLARE` — first-pass runtime arrays with one- and two-dimensional sizing
- `STORE`
- `CREATE TABLE` — first-pass local DBF creation for field declarations (`C`/`N`/`F`/`B`/`L`/`D`/`I`/`Y`/`T`/`M`/`V`/`Q` plus common long-form names); nullable/default annotations are accepted by the parser, created/open cursors apply field defaults for omitted `INSERT INTO` fields, and `NOT NULL` fields are validated with append rollback on failure
- `ALTER TABLE ... ADD/DROP/ALTER COLUMN` — first-pass local DBF schema rewrite that appends, removes, or rewrites one supported field while preserving row values and deleted flags where the resulting schema can store them
- `COPY TO` — full local DBF export via `create_dbf_table_file`; optional `FIELDS` clause filters columns; optional `FOR` clause filters rows by visibility; first-pass `TYPE SDF` fixed-width text export plus VFP-style `TYPE CSV` and `DELIMITED` text export, including CSV field-name headers, quoted character fields, `WITH <enclosure>`, and `WITH CHARACTER <separator>` handling; first-pass `COPY TO ARRAY <name>` populates a two-dimensional runtime array (rows=qualifying records, cols=fields) with optional `FIELDS` and `FOR` clauses
- `COPY STRUCTURE TO` — schema-only export (zero records); first-pass
- `APPEND FROM` — full local DBF import via `parse_dbf_table_from_file`; inserts each non-deleted source record using `append_blank_record_to_file` + `replace_record_field_value`; optional `FIELDS` clause; first-pass `TYPE SDF` fixed-width text import plus first-pass `TYPE CSV` / `DELIMITED` import with quoted-field parsing and matching CSV header skipping; first-pass `APPEND FROM ARRAY <name>` appends rows of a runtime array as new records (columns map positionally to destination fields with optional `FIELDS` filtering)
- `SCATTER MEMVAR` / `SCATTER TO <array>` — snapshot current record fields to `m.<fieldname>` variables or a first-pass one-dimensional array; `FIELDS` and `BLANK` are honored with typed values; first-pass
- `GATHER MEMVAR` / `GATHER FROM <array>` — write `m.<fieldname>` variables or array elements back to current record via direct `replace_record_field_value` calls (type-faithful, no string-quote round-trip); `FIELDS` and `FOR` are honored; first-pass
- `UPDATE` — first-pass local/remote cursor command forms, including `UPDATE <alias> SET ...`, `UPDATE SET ...` against the selected cursor, and `UPDATE IN <alias> SET ...`; `WHERE`/`FOR` scope clauses reuse scoped record replacement semantics
- `DELETE FROM` / `INSERT INTO` — first-pass SQL-style local/remote cursor mutation forms; `DELETE FROM <target> WHERE/FOR <expr>` tombstones matching target rows and defaults to all rows when no condition is supplied; `INSERT INTO <target> [(fields)] VALUES (...)` appends through the shared blank-row plus field-replacement path, including expression-driven cursor targets, schema-order mapping when the field list is omitted, selected-cursor preservation, local DBF persistence, and synthetic SQL result cursor parity
- `INSERT INTO` failure rollback — local and synthetic remote inserts now restore the previous record count when a field write fails after the blank-row append step, preventing a failed insert from leaving an extra blank row behind
- `PACK`, `PACK MEMO`, and `ZAP` — first-pass local DBF table-maintenance commands; `PACK` physically removes deleted records while preserving schema and kept-row order, `PACK MEMO` rewrites the memo sidecar from current memo values to compact stale memo blocks, and `ZAP` truncates the table to zero records while keeping it appendable; synthetic remote/result cursors get matching in-memory behavior where applicable

The `m.` variable namespace prefix is also now correctly handled as a memory-variable alias across assignment, lookup, expression evaluation, and declarations: `m.<name>`, `M.<name>`, and bare `<name>` share the same local/global runtime binding, and `m.<name>` no longer falls through to OLE property resolution.

Runtime arrays now have first-pass direct element reads and writes through both bracket and parenthesis syntax (`array[1]`, `array[1,2]`, `array(1)`, `array(1,2)`), including declared arrays and arrays resized through `ASIZE()`.

The current xAsset runtime surface behind those commands is now green at the first-pass runtime level: `DO FORM` is backed by generated `SCX/VCX` bootstrap sources that sequence `DataEnvironment` open/close hooks plus root-object lifecycle methods where present, `REPORT FORM` and `LABEL FORM` now both support preview/event-loop and `TO FILE` execution lanes, and menu assets now have executable startup/action/cleanup bootstrap modeling through the shared `MNX` xAsset path.

The current indexed-search slice covers `SET ORDER TO TAG`, first-pass `ASCENDING` / `DESCENDING` direction control on the local `SET ORDER` / `SEEK` path, grounded `UPPER(...)` seek-key normalization on that same search path, first-pass composite tag-expression evaluation for concatenated keys such as `UPPER(LAST+FIRST)`, first-pass substring-style tag evaluation for `LEFT(...)`, `RIGHT(...)`, and `SUBSTR(...)` expressions such as `UPPER(LEFT(NAME, 3))`, `UPPER(RIGHT(NAME, 3))`, and `UPPER(SUBSTR(NAME, 2, 3))`, first-pass grounded `FOR`-filtered order behavior for extracted `DELETED() = .F./.T.` hints, command/function `SEEK`, `INDEXSEEK()`, `ORDER()`, `TAG()`, `SET EXACT`, `SET NEAR`, function-order designator suffix parsing for `SEEK()` / `INDEXSEEK()` one-off `ASCENDING` / `DESCENDING` overrides, command-level one-off `SEEK ... TAG/ORDER` probes for local DBF-backed cursors, and descriptive stored CDX tag names such as `FULLNAME` now bind to their hinted tag-page-local expressions even when the tag name itself does not resemble the key expression. The runtime also preserves the actual inspected index-file identity for `ORDER(alias, 1)` plus `TAG(indexFile, ...)` across supported xBase index extensions instead of assuming `.cdx`.

That local indexed-search slice now also evaluates a broader grounded tag-expression family on the shipped `SEEK` path, including `LEFT(...)`, `RIGHT(...)`, `SUBSTR(...)`, `PADL(...)`, `PADR(...)`, and a constrained first-pass `STR(<numeric>[, <width>[, <decimals>]])` path captured from loaded tag metadata, including first-pass default-space `PADL/PADR` behavior. The seek path now also preserves significant search-key whitespace so padded tag expressions can match without trimming away meaningful leading/trailing spaces.

The expression/function layer currently has first-pass support for these VFP-facing built-ins:

- `COUNT()`, `SUM()`, `AVG()/AVERAGE()`, `MIN()`, `MAX()`
- first-pass `ALEN()` for runtime arrays created by `SCATTER TO <array>`, `AERROR(<array>)`, `DIMENSION` / array-form `DECLARE`, and `ASIZE()`, including total element count and dimension queries
- first-pass `AERROR(<array>)` with VFP-aligned seven-column rows for normal runtime errors, plus SQL/ODBC-like `1526` and OLE/automation-like `1429` row shapes carrying provider-specific detail placeholders where Copperfin does not yet have live provider payloads
- first-pass runtime array helpers over Copperfin runtime arrays: `ACOPY()`, `ADIR()`, `AELEMENT()`, `AFIELDS()`, `ALINES()`, `ASIZE()`, `ASCAN()`, `ADEL()`, `AINS()`, `ASORT()`, and `ASUBSCRIPT()`
- `EVAL()` and first-pass `&macro` substitution in expression paths
- `SQLCONNECT()`, `SQLSTRINGCONNECT()`, `SQLEXEC()`, `SQLDISCONNECT()`
- `CREATEOBJECT()`, `GETOBJECT()`
- utility coverage already used by shipped tests and bootstrap paths, including occurrence-aware `AT()` / `RAT()`, first-pass case-insensitive `ATC()` / `RATC()`, `LIKE()`, `INLIST()`, `PROPER()`, `SUBSTR()`, `ALLTRIM()`, `STR()`, `CHR()`, `FILE()`, `SYS()`, `MESSAGE()`, `PROGRAM()`, `LINENO()`, `ERROR()`, `VERSION()`, `ON()`, `SET()`, and `MESSAGEBOX()`

That SQL slice now includes session-scoped synthetic cursor materialization that follows the current selected work-area flow for each data session when `SQLEXEC()` auto-opens a result cursor, plus a first-pass remote-cursor layer where synthetic SQL rows participate in field lookup, targeted command-path filtering (`SET FILTER ... IN`) composed with targeted command-path navigation (`GO ... IN`, `SKIP ... IN`, `LOCATE ... IN`), command-path `SCAN ... IN` targeted iteration, aggregate built-ins, command-level `TOTAL` output aggregation, `CALCULATE`, in-memory `REPLACE` / `APPEND BLANK` / `DELETE` / `RECALL` mutation, non-selected-target `REPLACE ... IN` / `DELETE FOR ... IN` / `RECALL FOR ... IN` command-path behavior, `APPEND BLANK IN <alias|work area>` targeted append behavior, one-off `SEEK()` / `INDEXSEEK()` probes that use temporary order expressions over the in-memory result rows, first-pass normalization-aware temporary SQL order expressions such as `UPPER(NAME)` including combined `DESCENDING` suffix probes, and command-path `SET ORDER TO <expr>` / `SEEK` behavior over those same synthetic rows including non-selected-target `SET ORDER ... IN ... [ASCENDING|DESCENDING]` / `SEEK ... IN` parity and post-mutation seeks against appended in-memory rows.

The current session/runtime-state slice also now keeps `SET DEFAULT TO` data-session-local, so `SET('DEFAULT')` and relative path resolution restore correctly after `SET DATASESSION` switches instead of leaking one session's default directory into another, and focused regression coverage now locks down the local-table counterpart to the shipped SQL selection-flow behavior so `SELECT 0`, `USE IN <selected-alias>`, and plain `USE` keep reusing each session's selected empty work area after session switches.

`ON ERROR` now also has a richer first-pass compatibility lane: `ON ERROR DO <routine> WITH ...` can pass evaluated handler arguments, and handlers can inspect first-pass runtime fault metadata through `MESSAGE()`, `PROGRAM()`, `LINENO()`, `ERROR()`, `AERROR()`, `SYS(2018)`, and `ON('ERROR')` while execution resumes after the handler returns. Static PRG diagnostics are also now wired through the Studio document-open path so analyzer findings surface to editor-facing consumers instead of staying runtime-only.

The current PRG-engine slice now also supports first-pass `WITH/ENDWITH` member binding for leading-dot object access and first-pass `TRY/CATCH/FINALLY/ENDTRY` control flow, plus first-pass `DO ... WITH @var` reference semantics so `LPARAMETERS` assignments can write back into caller variables where needed for FoxPro-style procedure behavior.

## Immediate Runtime Backlog Derived From The Official Reference

The official command inventory is much larger than the current runtime. The deepest next command groups should stay focused on data/runtime compatibility first:

### Active Issue Mapping For This Runtime Backlog

- command-surface and expression/runtime-state depth: #7, #8
- OLE/COM and automation compatibility: #10, #11
- automation fault containment and diagnostics: #12, #13, #14

### Work Areas, Sessions, And Indexed Data

- finish the remaining alias/work-area edge cases across `SELECT`, `USE`, and nested data sessions after the first strict `USE ... IN <alias>` targeting pass, first-pass non-selected target preservation, plain `USE` current-work-area reuse, freed-work-area reuse plus side-effect-free `SELECT(0)` probing, stronger synthetic SQL cursor/session isolation, per-session SQL handle lifecycle cleanup, and the now-shipped session-local restoration coverage for `SET NEAR` and `SET DEFAULT TO`
- keep closing the remaining expression-driven work-area targeting edges after the shared designator-expression path now covers `SET FILTER ... IN`, `SELECT <expr>`, `USE ... IN <expr>`, and the shipped local `IN`-targeted data/search family (`SET ORDER`, `SEEK`, `LOCATE`, `SCAN`, `GO`, `SKIP`, `REPLACE`, `DELETE`, `RECALL`) for core local-cursor flows
- deepen order/search behavior beyond the current first-pass `SET ORDER TO TAG` / `SEEK` / `FOUND()` plus direction-control slice
- add adjacent data-navigation and data-search commands where VFP developers expect them to work together
- use the command inventory to pull the next search/index/data-session commands in families instead of one at a time

### Local Table Mutation And Query Flow

- add the table-editing and table-browse commands needed for real legacy business applications
- the shared DBF/FPT mutation path now covers shipped local-table write families used by current runtime flows: memo-backed pointer fields (`M`/`G`/`P`), fixed-width numeric/date families (`B`/`I`/`Y`/`T`), first-pass var-length families (`V`/`Q`), constrained `NULL` token mutation semantics, staged write safety for DBF/memo updates (including staged-artifact cleanup), physical `PACK` compaction, first-pass `PACK MEMO` sidecar rewrite/compaction, `ZAP` truncation, SQL-style insert rollback, first-pass `CREATE TABLE`, first-pass structural `ALTER TABLE`, and indexed-table plus unsupported-layout fail-fast guards until real structural/indexed storage fidelity exists
- extend the shipped filtering, locating, scanning, replacing, appending, SQL-style inserting/deleting, recalling, packing, zapping, first-pass structural `CREATE TABLE` / `ALTER TABLE`, aggregate-built-in, command-level `COUNT`/`SUM`/`AVERAGE` with first-pass scope/`WHILE`/`IN`, first-pass `TOTAL` with `IN` targeting and local `N/F/I/Y` support, and `CALCULATE` families toward broader xBase parity
- deepen table-maintenance parity beyond the first-pass `PACK`/`ZAP`/`PACK MEMO` slice with exclusive-use/locking behavior, index rebuild/invalidated-order handling, and stronger diagnostics for unsafe structural layouts
- keep runtime semantics ahead of shell/designer work so the same engine can power both Visual Studio and the standalone IDE

### Native Control Flow

- extend the shipped `DO CASE/CASE/OTHERWISE/ENDCASE`, `DO WHILE/ENDDO`, `WITH/ENDWITH`, `TRY/CATCH/FINALLY`, `LOOP`/`CONTINUE`/`EXIT`, first-pass richer `ON ERROR`, and first-pass literal `TEXT/ENDTEXT` slice into the rest of the FoxPro control-flow surface
- keep tightening expression semantics around stored-expression evaluation, macro substitution, and runtime-state inspection beyond the first-pass `EVAL()` / `SET()` / `&macro` slice
- add the next adjacent control families beyond the shipped engine core as coherent batches instead of one syntax form at a time

### SQL Pass-Through And Remote Cursor Behavior

- add the broader SQL pass-through surface around the existing `SQLCONNECT` / `SQLEXEC` / `SQLDISCONNECT` slice
- bring remote cursor semantics closer to local cursor expectations where VFP does so
- use the general reference and local help corpus when Learn pages list a command but do not capture enough behavioral detail

### Automation And Compatibility Utilities

- expand the OLE/COM compatibility surface around the current `CREATEOBJECT()` / `GETOBJECT()` slice
- keep host stability and debugger fault containment mandatory for automation failures

## How To Use The References

- Use the Learn language-reference index to decide what command/function families exist and to avoid missing major areas.
- Use the Learn general reference for file structures, table/index storage, and other low-level behavior.
- Use the generated local CHM command-topic index and topic manifest to jump from a command or `foxtools` function name to the installed VFP help page that describes its behavior.
- Use installed VFP 9 help and local source trees for product-era behavior and sample patterns.
- Use `FOXHELP.DBF` and its memo file as a fallback source for older semantics when Learn pages are missing, gated, or too shallow.
