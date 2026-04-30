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
- `CALL` — first-pass command-path parity now includes `CALL <target> WITH ...` argument binding and external `.prg` target resolution when local routines are not found, including BYREF `@var` write-back through existing call-reference plumbing
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
- `SET FIELDS TO/ON/OFF` — first-pass session-scoped field visibility for runtime field lookup, including list readback through `SET('FIELDS')`; the shipped visibility lane now also honors `LIKE` / `EXCEPT` filters for both direct field resolution and consumers such as `BROWSE`
- `SET ORDER TO TAG`
- `SEEK`
- `GO` / `GOTO`
- `SKIP`
- `BROWSE` — first-pass headless/runtime browse event lane over local/runtime cursors; honors selected or `IN`-targeted cursor context, reuses session `SET FILTER` / `SET FIELDS` state by default, now including `LIKE` / `EXCEPT` field-visibility filters, accepts inline `FIELDS` and `FOR` clause capture, and emits `runtime.browse` metadata for later host/UI wiring
- `EDIT`, `CHANGE`, `INPUT`, `ACCEPT` — first-pass headless interactive command lane that emits runtime events (`runtime.edit`, `runtime.change`, `runtime.input`, `runtime.accept`) with captured prompt/target/field metadata for host-driven UI integration; `EDIT` / `CHANGE` now also surface effective selected-cursor/view metadata when a local cursor is active, including visible fields and active filter state, while `INPUT` / `ACCEPT` now reuse that same selected-cursor/view metadata and assign deterministic empty-string headless results for `TO <target>` variables
- `WAIT`, `KEYBOARD`, `DISPLAY`, `LIST` — first-pass headless interaction/event lane that emits `runtime.wait`, `runtime.keyboard`, `runtime.display`, and `runtime.list` events with captured mode/clause payloads for host integration; `WAIT` now captures resolved prompt/timeout/target state plus source expressions for common `WAIT [WINDOW] ... [TIMEOUT ...] [TO <target>] [NOWAIT] [NOCLEAR]` forms and assigns deterministic empty-string headless `TO` results, `KEYBOARD` now captures resolved key payloads plus source expressions and common `PLAIN` / `CLEAR` flags, and `DISPLAY` / `LIST` now surface adjacent `STRUCTURE` / `STATUS` / `MEMORY` metadata in addition to the richer `RECORDS` lane (`STRUCTURE` emits selected-cursor schema details, `STATUS` emits current data-session/open-cursor/selected-view state including active filter and visible fields, and `MEMORY` emits visible and shadowed public/private/local/global memvar plus runtime-array summaries with scope/type/detail and first-pass object previews)
- `GETFILE`, `PUTFILE`, `GETDIR`, `INPUTBOX` — first-pass headless dialog-helper command lane; parser now captures both common clause-keyword forms and parenthesized/positional argument forms for prompt/title/default/filter/target metadata, and dispatcher emits `runtime.getfile`, `runtime.putfile`, `runtime.getdir`, and `runtime.inputbox` events while assigning deterministic empty-string first-pass results to `TO <target>` variables in headless mode
- `PUSH KEY` / `POP KEY`, `PUSH MENU` / `POP MENU`, `PUSH POPUP` / `POP POPUP` — first-pass session-scoped key/menu/popup stack semantics with deterministic push/pop depth behavior and runtime events (`runtime.push_*` / `runtime.pop_*`) including empty-pop signaling
- `SAVE TO`, `RESTORE FROM` — first-pass memvar persistence lane with `.mem` auto-extension behavior, `LIKE`/`EXCEPT` filtering, `ADDITIVE` merge support, robust escaped text serialization, and explicit empty-value markers for stable round-trip restore behavior
- `RELEASE` — first-pass public/global release semantics now preserve public-pinned scalar variables and arrays across `RELEASE ALL`, `RELEASE ALL LIKE`, and `RELEASE ALL EXCEPT`, while named and non-public release paths continue to remove ordinary bindings
- `SELECT`
- `USE`, `USE IN`, `USE AGAIN`
- `SET DATASESSION TO`
- `SET DEFAULT TO`
- `SET`
- `CLOSE ALL` / `CLOSE TABLES` / `CLOSE DATABASES` — first-pass close semantics now cover both open work-area cursors and non-DBF runtime handles for `ALL`/`DATABASES` scopes (SQL connection state, tracked OLE handles, and outstanding low-level `FOPEN` handles)
- `ON ERROR`
- `ON SHUTDOWN` — first-pass shutdown-handler semantics for `ON SHUTDOWN DO <routine> [WITH ...]` plus common inline compatibility forms including `ON SHUTDOWN CLEAR EVENTS` and close-style cleanup commands such as `ON SHUTDOWN CLOSE DATABASES ALL`; shutdown routines run before final `QUIT` completion and nested `QUIT` inside the shutdown routine is treated as the terminal quit rather than recursing indefinitely
- `PUBLIC` — first-pass public identity declaration for memory variables and arrays
- `LOCAL`
- `PRIVATE`
- `PARAMETERS` / `LPARAMETERS` — first-pass argument binding for `DO ... WITH ...`, including `@` by-reference write-back and default-value expressions when callers omit trailing arguments; default expressions now evaluate through the ordinary expression engine, so macro-expanded defaults such as `LPARAMETERS x = &cExpr` resolve consistently
- `DIMENSION` / array-form `DECLARE` — first-pass runtime arrays with one- and two-dimensional sizing
- `STORE` — evaluates the expression once and assigns through the same target semantics as ordinary assignment, including scalar variables, direct array elements, macro-expanded targets, `m.` memory variables, and OLE property targets
- `CREATE TABLE` — first-pass local DBF creation for field declarations (`C`/`N`/`F`/`B`/`L`/`D`/`I`/`Y`/`T`/`M`/`V`/`Q` plus common long-form names); nullable/default annotations are accepted by the parser, created/open cursors apply field defaults for omitted `INSERT INTO` fields, and `NOT NULL` fields are validated with append rollback on failure
- `CREATE CURSOR` — local temp-backed DBF cursor creation using the shared structural field-declaration parser and field-rule mapping; resulting cursors now participate in normal local-table mutation/schema flows (`APPEND BLANK`, `REPLACE`, `INSERT INTO`, `AFIELDS`, `FCOUNT`, `FSIZE`, `RECCOUNT`) instead of the old synthetic stub, and `DEFAULT` / `NOT NULL` metadata are enforced through the same open-cursor field-rule path used by `CREATE TABLE`
- `ALTER TABLE ... ADD/DROP/ALTER COLUMN` — first-pass local DBF schema rewrite that appends, removes, or rewrites one supported field while preserving row values and deleted flags where the resulting schema can store them
- `COPY TO` — full local DBF export via `create_dbf_table_file`; optional `FIELDS` clause filters columns, including single-field keyword-named selections such as `FIELDS TYPE` plus deeper `LIKE` / `EXCEPT` forms across the same copy-to/copy-to-array lane; optional `FOR` clause filters rows by visibility; first-pass `TYPE SDF` fixed-width text export plus VFP-style `TYPE CSV`, `TYPE TAB`, and `DELIMITED` text export, including CSV field-name headers, quoted character fields, tab-delimited rows, `WITH <enclosure>`, and `WITH CHARACTER <separator>` handling; first-pass `TYPE JSON` export emits a modern object-array JSON payload with typed numeric/logical values and string keys from field names; first-pass `TYPE XLS` export emits SpreadsheetML-style workbook XML with header-row and typed cell data; first-pass `TYPE DIF` export emits a conservative DIF-style text table with header-row emission and typed numeric/string cells; first-pass `TYPE SYLK` export emits a conservative SYLK-style text table with header-row emission and typed numeric/string cells; first-pass `COPY TO ARRAY <name>` populates a two-dimensional runtime array (rows=qualifying records, cols=fields) with optional `FIELDS` and `FOR` clauses, including macro-expanded array-name targets
- DBC metadata export now also has a first-pass whole-database JSON lane through the native asset-inspector API (`export_database_as_json`), including decoded binary `PROPERTIES` memo bags from `.dct` sidecars plus table-schema/table-row snapshots for resolvable local `TABLE` catalog entries
- `COPY STRUCTURE TO` — schema-only export (zero records); first-pass
- `APPEND FROM` — full local DBF import via `parse_dbf_table_from_file`; inserts each non-deleted source record using `append_blank_record_to_file` + `replace_record_field_value`; optional `FIELDS` clause, including single-field keyword-named selections such as `FIELDS TYPE` plus deeper `LIKE` / `EXCEPT` forms across the same append/import array lane; first-pass `TYPE SDF` fixed-width text import plus first-pass `TYPE CSV`, `TYPE TAB`, and `DELIMITED` import with quoted-field parsing, tab-delimited rows, and matching CSV header skipping; first-pass `TYPE JSON` import reads the shipped object-array JSON lane and maps object keys to destination field names through the shared blank-row plus field-replacement path; first-pass `TYPE XLS` import reads the shipped SpreadsheetML-style workbook lane, skips a matching header row, and appends rows through the shared blank-row plus field-replacement path; first-pass `TYPE DIF` import reads the shipped DIF-style text lane, skips a matching header row, and appends rows through the same shared DBF mutation path; first-pass `TYPE SYLK` import reads the shipped SYLK-style text lane, skips a matching header row, and appends rows through that same shared DBF mutation path; first-pass `APPEND FROM ARRAY <name>` appends rows of a runtime array as new records (columns map positionally to destination fields with optional `FIELDS` filtering), including macro-expanded array-name sources
- `SCATTER MEMVAR` / `SCATTER TO <array>` / `SCATTER NAME <object>` — snapshot current record fields to `m.<fieldname>` variables, runtime arrays, or object properties; `FIELDS`, `BLANK`, and `MEMO` are honored with typed values, including runtime-facing `MM/DD/YYYY` and `MM/DD/YYYY HH:MM:SS` strings for `D`/`T` fields; the parser now also disambiguates single-field filters such as `FIELDS NAME` from the command's own `NAME` clause for both memvar and object-target forms, and deeper regression coverage now exercises `LIKE` / `EXCEPT` filters on object-target flows; macro-expanded array-name targets are supported; `SCATTER NAME &cObjectName`-style macro-expanded object variable targets are also supported; nested object-property targets such as `SCATTER NAME oHolder.Row` / `GATHER NAME oHolder.Row` are supported, and scatter now creates missing nested child-object targets on demand instead of requiring them to already exist; non-`ADDITIVE` scatter on an existing object target now replaces that target object instead of mutating it in place, while `SCATTER NAME <object> ADDITIVE` reuses an existing object when present and merges field properties onto it; first-pass
- `GATHER MEMVAR` / `GATHER FROM <array>` — write `m.<fieldname>` variables or array elements back to current record via direct `replace_record_field_value` calls (type-faithful, no string-quote round-trip); `FIELDS` and `FOR` are honored, including single-field `FIELDS NAME` filters on both memvar and `GATHER NAME <object>` forms plus deeper `EXCEPT` coverage on object-target flows, date/datetime/blank runtime strings now restore consistently into DBF storage, and macro-expanded array-name sources are supported; first-pass
- `UPDATE` — first-pass local/remote cursor command forms, including `UPDATE <alias> SET ...`, `UPDATE SET ...` against the selected cursor, and `UPDATE IN <alias> SET ...`; `WHERE`/`FOR` scope clauses reuse scoped record replacement semantics, including macro-expanded `FOR &cExpr` filters
- `DELETE FROM` / `INSERT INTO` — first-pass SQL-style local/remote cursor mutation forms; `DELETE FROM <target> WHERE/FOR <expr>` tombstones matching target rows and defaults to all rows when no condition is supplied, including macro-expanded `FOR &cExpr` filters; `INSERT INTO <target> [(fields)] VALUES (...)` appends through the shared blank-row plus field-replacement path, including expression-driven cursor targets, schema-order mapping when the field list is omitted, selected-cursor preservation, local DBF persistence, and synthetic SQL result cursor parity
- `INSERT INTO` failure rollback — local and synthetic remote inserts now restore the previous record count when a field write fails after the blank-row append step, preventing a failed insert from leaving an extra blank row behind
- `PACK`, `PACK MEMO`, and `ZAP` — first-pass local DBF table-maintenance commands; `PACK` physically removes deleted records while preserving schema and kept-row order, `PACK MEMO` rewrites the memo sidecar from current memo values to compact stale memo blocks, and `ZAP` truncates the table to zero records while keeping it appendable; local cursors now honor first-pass exclusive-use guards from `SET EXCLUSIVE` / `USE ... SHARED|EXCLUSIVE`; synthetic remote/result cursors get matching in-memory behavior where applicable
- `SET REPROCESS`, `SET MULTILOCKS`, `RLOCK()`, `FLOCK()` / `LOCK()`, `ISRLOCKED()`, `ISFLOCKED()`, and `UNLOCK` — first-pass in-process lock-state tracking for open local/runtime cursors, including `SET()` readback, `UNLOCK ALL`, and record-specific `UNLOCK RECORD <n>` release; this establishes runtime-visible lock semantics but does not yet claim cross-process OS locking or transaction journaling
- `SET` runtime-state commands — first-pass normalized/evaluated handling for common state options including boolean options, `REPROCESS`, date/time formatting controls, `PATH`, `DECIMALS`, `COLLATE`, `MARK`, `POINT`, `SEPARATOR`, and `CURRENCY`; string/numeric RHS values can come from literals, variables, or macro-expanded expressions where supported, with `SET()` readback defaults for the shipped options
- `BEGIN TRANSACTION`, `END TRANSACTION`, `ROLLBACK`, and `TXNLEVEL()` — first-pass transaction nesting counters with per-data-session isolation, runtime event emission (`runtime.transaction.begin` / `runtime.transaction.end` / `runtime.transaction.rollback`), durable local DBF preimage journaling for shipped mutation paths, rollback cursor refresh/closure when replay removes a table, and startup replay of pending journals after interrupted transactions

Local DBF `APPEND BLANK` now also has a conservative opaque-field lane for otherwise readable unsupported record layouts: unknown/binary field bytes are zero-initialized instead of rejecting the whole append, which lets runtime mutation continue over mixed-layout tables while still avoiding overclaimed write-surface semantics for direct typed replacement on those field families.

Shared local DBF direct writes now also have a matching conservative opaque-field replacement lane for those same otherwise readable unsupported families: callers can write `NULL`, width-fitting raw text, or `0x`-prefixed hex payloads without inventing higher-level typed semantics for the underlying field family.

Command-level aggregate assignment now also includes first-pass `TO ARRAY` forms for `COUNT`, `SUM`, and `AVERAGE`, using one-dimensional runtime array assignment through the shared array helper path. Focused regression coverage validates scope-clause behavior (`ALL`/`REST`/`NEXT`/`RECORD`), `FOR`/`WHILE` filtering, `IN` alias targeting, and malformed `TO ARRAY` diagnostics. Aggregate expression parity also now includes omitted-expression forms: `COUNT()` returns visible-row counts, and no-expression `SUM()` / `AVG()` / `MIN()` / `MAX()` use the first numeric-compatible field (`N/F/B/I/Y`) with deterministic no-numeric-field fallbacks.

The `m.` variable namespace prefix is also now correctly handled as a memory-variable alias across assignment, lookup, expression evaluation, and declarations: `m.<name>`, `M.<name>`, and bare `<name>` share the same local/global runtime binding, and `m.<name>` no longer falls through to OLE property resolution.

Runtime arrays now have first-pass direct element reads and writes through both bracket and parenthesis syntax (`array[1]`, `array[1,2]`, `array(1)`, `array(1,2)`), including declared arrays and arrays resized through `ASIZE()`. Macro-expanded array identifiers now also work across adjacent helper and element-access paths such as `ALEN(&cArrayName)`, `ALINES(&cTargetName, ...)`, `ACOPY(&cSourceName, &cTargetName, ...)`, `&cArrayName[2,1]`, and `&cArrayName(1,2)`. The array helper surface now also covers `ACOPY()` row-major copies over one- and two-dimensional arrays, `AELEMENT()` / `ASUBSCRIPT()` index conversion, `ASCAN()` start/count scans with column, case-insensitive, exactness, row-return, and first-pass predicate-expression search support, `ASORT()` start/count windows plus descending/case-insensitive/numeric-aware ordering and two-dimensional row sorting by the start element's column, and two-dimensional `ADEL()` / `AINS()` row/column shifting.

The current xAsset runtime surface behind those commands is now green at the first-pass runtime level: `DO FORM` is backed by generated `SCX/VCX` bootstrap sources that sequence `DataEnvironment` open/close hooks plus root-object lifecycle methods where present, `REPORT FORM` and `LABEL FORM` now both support preview/event-loop and `TO FILE` execution lanes, and menu assets now have executable startup/action/cleanup bootstrap modeling through the shared `MNX` xAsset path.

The current indexed-search slice covers `SET ORDER TO TAG`, first-pass `ASCENDING` / `DESCENDING` direction control on the local `SET ORDER` / `SEEK` path, grounded `UPPER(...)` seek-key normalization on that same search path, first-pass composite tag-expression evaluation for concatenated keys such as `UPPER(LAST+FIRST)`, first-pass substring-style tag evaluation for `LEFT(...)`, `RIGHT(...)`, and `SUBSTR(...)` expressions such as `UPPER(LEFT(NAME, 3))`, `UPPER(RIGHT(NAME, 3))`, and `UPPER(SUBSTR(NAME, 2, 3))`, first-pass grounded `FOR`-filtered order behavior for extracted `DELETED() = .F./.T.` hints, command/function `SEEK`, `INDEXSEEK()`, `ORDER()`, `TAG()`, `SET EXACT`, `SET NEAR`, function-order designator suffix parsing for `SEEK()` / `INDEXSEEK()` one-off `ASCENDING` / `DESCENDING` overrides, command-level one-off `SEEK ... TAG/ORDER` probes for local DBF-backed cursors, first-pass ad hoc local order expressions such as `SET ORDER TO UPPER(NAME)` and `SEEK(..., 'People', 'UPPER(NAME)')`, and descriptive stored CDX tag names such as `FULLNAME` now bind to their hinted tag-page-local expressions even when the tag name itself does not resemble the key expression. The runtime also preserves the actual inspected index-file identity for `ORDER(alias, 1)` plus `TAG(indexFile, ...)` across supported xBase index extensions instead of assuming `.cdx`.

That local indexed-search slice now also evaluates a broader grounded tag-expression family on the shipped `SEEK` path, including `LEFT(...)`, `RIGHT(...)`, `SUBSTR(...)`, `PADL(...)`, `PADR(...)`, and a constrained first-pass `STR(<numeric>[, <width>[, <decimals>]])` path captured from loaded tag metadata, including first-pass default-space `PADL/PADR` behavior. The seek path now also preserves significant search-key whitespace so padded tag expressions can match without trimming away meaningful leading/trailing spaces.

That same local temporary-order path now also has focused command-path `IN <alias|work area>` parity coverage for ad hoc expressions such as `UPPER(NAME)`, including selected-alias preservation and descending `SET NEAR` behavior on non-selected targeted cursors.

The expression/function layer currently has first-pass support for these VFP-facing built-ins:

- `COUNT()`, `SUM()`, `AVG()/AVERAGE()`, `MIN()`, `MAX()`
- first-pass `ALEN()` for runtime arrays created by `SCATTER TO <array>`, `AERROR(<array>)`, `DIMENSION` / array-form `DECLARE`, and `ASIZE()`, including total element count and dimension queries
- deeper first-pass `AERROR(<array>)` with VFP-aligned seven-column rows for normal runtime errors: code, message, mixed-case error parameter, faulting work area, fault line, fault procedure, and failing statement text; SQL/ODBC-like `1526` rows now preserve captured handle/detail plus provider-or-operation context and command/target payload when the runtime has them, and OLE/automation-like `1429` rows now preserve captured member-path detail, source object identifiers, and failing action text instead of leaving trailing columns as placeholders when Copperfin already knows that payload
- first-pass runtime array helpers over Copperfin runtime arrays: `ACOPY()`, `ADIR()`, `AELEMENT()`, `AFIELDS()` over local and synthetic SQL cursor schemas, `ALINES()`, `ASIZE()`, `ASCAN()`, `ADEL()`, `AINS()`, `ASORT()`, `ASUBSCRIPT()`, and `AUSED()`; the focused array regression target now exercises predicate-style `ASCAN()` searches, including unquoted block-style predicate arguments, common two-dimensional `ACOPY()` row/column workflows, and declared-array/metadata helper coverage separately from the legacy catch-all PRG-engine test
- `EVAL()` and deeper first-pass bounded-indirection `&macro` substitution in expression paths (identifier-chain expansion with deterministic recursion/cycle safety)
- `SQLCONNECT()`, `SQLSTRINGCONNECT()`, `SQLEXEC()`, `SQLDISCONNECT()`, `SQLROWCOUNT()`, `SQLPREPARE()`, first-pass `SQLGETPROP()`, `SQLSETPROP()`, `SQLCANCEL()`, `SQLCOMMIT()`, `SQLROLLBACK()`, `SQLDATABASES()`, `SQLPRIMARYKEYS()`, `SQLFOREIGNKEYS()`, `SQLTABLES()`, and `SQLCOLUMNS()`
- `CREATEOBJECT()`, `GETOBJECT()`
- reflection/runtime-surface object helpers: first-pass `COMPOBJ()`, `AMEMBERS()`, `ACLASS()`, `PEMSTATUS()`, `ADDPROPERTY()`, and `REMOVEPROPERTY()` with seeded `Scripting.Dictionary` member metadata for deterministic reflection behavior
- first-pass runtime-surface XML cursor bridge helpers: `CURSORTOXML()` and `XMLTOCURSOR()` over a deterministic Copperfin XML shape, including file-or-string transfer paths, row-count returns for imports, and `runtime.cursortoxml` / `runtime.xmltocursor` event emission with warning events on invalid inputs
- utility coverage already used by shipped tests and bootstrap paths, including occurrence-aware `AT()` / `RAT()`, first-pass case-insensitive `ATC()` / `RATC()`, line-oriented `ATLINE()` / `ATCLINE()` / `RATLINE()`, `LIKE()`, `INLIST()`, `PROPER()`, first-pass `STRCONV(..., 7/8)` casing modes, first-pass phonetic helpers (`SOUNDEX()` and `DIFFERENCE()`), first-pass memo-line helpers (`MEMLINES()` and `MLINE()` with session-scoped memo width, explicit width overrides, optional tab-width expansion, and opt-in LF hard-break flags), `CHRTRAN()` / first-pass `CHRTRANC()`, `STRTRAN()` with first-pass optional start-occurrence and replacement-count arguments, `SUBSTR()`, `ALLTRIM()`, `STREXTRACT()` with flag-aware delimiter handling, `STR()` with width/decimal formatting, `TRANSFORM()` numeric/picture formatting with first-pass `SET POINT`/`SET SEPARATOR`/`SET CURRENCY` support plus `@Z` blank-zero and `@B` trimmed-left-justified behavior, `PADL()` / `PADR()` / `PADC()` padding and overflow truncation, `GETWORDCOUNT()` / `GETWORDNUM()` with delimiter-set handling, `CHR()`, type/null helpers including first-pass expression-value `ISBLANK()` and single-byte `ISLEADBYTE()` plus improved `TYPE('<expr>')` parenthesized-expression handling, cursor/schema helpers (`FIELD()` and `FSIZE()` over local DBF-backed and synthetic SQL cursors), numeric helpers (`SIN()`, `COS()`, `TAN()`, `ASIN()`, `ACOS()`, `ATAN()`, `ATN2()`, `DTOR()`, `RTOD()`, `LOG10()`, `RAND()`, `RGB()`, and expression-list `MIN()` / `MAX()`), financial helpers (`FV()`, `PV()`, `PAYMENT()`), hex conversion (`HEX()`), runtime-surface helpers (`CAST()`, `BITAND()`, `BITOR()`, `BITXOR()`, `BITNOT()`, `BITCLEAR()`, `BITSET()`, `BITTEST()`, `BITLSHIFT()`, `BITRSHIFT()`, `BINTOC()`, `CTOBIN()`, first-pass host probes `HOME()`, `OS()`, `DISKSPACE()`, `DRIVETYPE()`, first-pass `CURSORGETPROP()` / `CURSORSETPROP()`, lock-key probes such as `NUMLOCK()`, first-pass UUID generation `NEWID()`, first-pass code-page helpers `CPCURRENT()` / `CPCONVERT()` / `CPDBF()`, first-pass headless dialog stubs `GETPICT()` / `GETCOLOR()` / `GETFONT()`, first-pass read-context stub `VARREAD()`, and context/introspection helpers `PCOUNT()`, `GETENV()`, and first-pass `PUTENV(<name>, <value>)`), date/time helpers (`DOW()`, `CDOW()`, `CMONTH()`, `GOMONTH()`, `SECONDS()`, `MDY()`, constructor-argument forms of `DATE()` / `DATETIME()`, `STOD()`, `TTOS()`, `CTOT()` including compact sortable datetime input, `DTOC(..., 1)`, `TTOC(..., 1)`, `DTOT()`, `TTOD()` including compact sortable datetime input, `HOUR()`, `MINUTE()`, `SEC()`, first-pass `SET DATE`/`SET CENTURY`/`SET MARK` formatting and parsing for common MDY/DMY/YMD orders and date separators, first-pass `SET HOURS`/`SET SECONDS` display formatting for time and datetime output, first-pass `WEEK()` with optional first-day and first-week mode arguments including mode 2/3 cross-year rollover plus `SET FDOW`/`SET FWEEK` defaults, `QUARTER()`, `EOMONTH()`, `DTOJ()` / `TTOJ()` / `JTOD()` / `JTOT()` with first-pass invalid-input guards), portable string-parsed path helpers (`JUSTPATH()`, `JUSTFNAME()`, `JUSTSTEM()`, `JUSTEXT()`, `JUSTDRIVE()`, `FORCEEXT()`, `FORCEPATH()`, `DEFAULTEXT()`, and first-pass `CURDIR()`), file metadata helpers (`FILE()` and `FILESIZE()` with `SET PATH` search handling for relative probes), first-pass low-level file I/O helpers (`FOPEN`, `FCLOSE`, `FREAD`, `FWRITE`, `FGETS`, `FPUTS`, `FSEEK`, `FTELL`, `FEOF`, `FFLUSH`, `FCHSIZE`, `FILETOSTR`, `STRTOFILE` — handle table, path resolution, binary/text modes, POSIX/Win32 truncation), `SYS()` including first-pass deterministic `SYS(3)`, `SYS(7)`, `SYS(11)`, `SYS(13)`, `SYS(2003)`, `SYS(2004)`, `SYS(2020)`, and `SYS(2023)`, `MESSAGE()`, `PROGRAM()`, `LINENO()`, `ERROR()`, `VERSION()`, `ON()`, `SET()`, and `MESSAGEBOX()`, first-pass `TEXTMERGE()` with configurable `<<>>`-style delimiters and recursive evaluation support, first-pass `EXECSCRIPT()` RETURN-expression pass-through, and first-pass `LOOKUP()` seek-and-evaluate over local alias/tag indexes

That SQL slice now includes session-scoped synthetic cursor materialization that follows the current selected work-area flow for each data session when `SQLEXEC()` auto-opens a result cursor, plus first-pass connection-state helpers around that same per-session handle model: `SQLROWCOUNT()`, `SQLPREPARE()`, `SQLGETPROP()` / `SQLSETPROP()`, `SQLCANCEL()` / `SQLCOMMIT()` / `SQLROLLBACK()`, and first-pass catalog-style metadata helpers `SQLDATABASES()` / `SQLPRIMARYKEYS()` / `SQLFOREIGNKEYS()` / `SQLTABLES()` / `SQLCOLUMNS()`. The current connection-property pass now exposes deterministic `Connected`, `ConnectHandle`, `ConnectString`, `ConnectName`, `CurrentCatalog`, `LastCursorAlias`, `LastResultCount`, `Asynchronous`, `BatchMode`, `DispWarnings`, `DispLogin`, `Transactions`, `WaitTime`, and `PacketSize` behavior over the existing session-local handle state, while the metadata-cursor pass exposes deterministic synthetic database, table, primary-key, foreign-key, and column catalogs for top-level catalog enumeration plus `TABLE` / `VIEW` / `SYSTEM TABLE` probing, FoxPro-style and native-style `SQLCOLUMNS()` layouts, empty metadata cursors that still preserve schema, and strict cross-session handle rejection for the connection-state helpers. The shipped remote-cursor layer lets synthetic SQL rows participate in field lookup, targeted command-path filtering (`SET FILTER ... IN`) composed with targeted command-path navigation (`GO ... IN`, `SKIP ... IN`, `LOCATE ... IN`), command-path `SCAN ... IN` targeted iteration, aggregate built-ins, command-level `TOTAL` output aggregation, `CALCULATE`, in-memory `REPLACE` / `APPEND BLANK` / `DELETE` / `RECALL` mutation, non-selected-target `REPLACE ... IN` / `DELETE FOR ... IN` / `RECALL FOR ... IN` command-path behavior, `APPEND BLANK IN <alias|work area>` targeted append behavior, one-off `SEEK()` / `INDEXSEEK()` probes that use temporary order expressions over the in-memory result rows, first-pass normalization-aware temporary SQL order expressions such as `UPPER(NAME)` including combined `DESCENDING` suffix probes, and command-path `SET ORDER TO <expr>` / `SEEK` behavior over those same synthetic rows including non-selected-target `SET ORDER ... IN ... [ASCENDING|DESCENDING]` / `SEEK ... IN` parity, targeted backward/boundary navigation coverage (`GO BOTTOM IN`, `SKIP -1 IN`, targeted `BOF()` / `EOF()`), and post-mutation seeks against appended in-memory rows.

The current session/runtime-state slice also now keeps `SET DEFAULT TO` data-session-local, so `SET('DEFAULT')` and relative path resolution restore correctly after `SET DATASESSION` switches instead of leaking one session's default directory into another, and focused regression coverage now locks down the local-table counterpart to the shipped SQL selection-flow behavior so `SELECT 0`, `USE IN <selected-alias>`, and plain `USE` keep reusing each session's selected empty work area after session switches. Boolean runtime `SET` options used by search/filter semantics (`NEAR`, `EXACT`, `DELETED`) and runtime flags (`STRICTDATE`, `ENGINEBEHAVIOR`, `OPTIMIZE`, `TALK`, `SAFETY`, `ESCAPE`, `CENTURY`, `SECONDS`) now also normalize `SET ... TO` forms and logical token variants (`.T./.F.`, `1/0`, `true/false`, `yes/no`) to stable runtime state and `SET()` readback values across data-session switches, while `SET DATE TO`, `SET MARK TO`, first-pass `SET HOURS TO`, and first-pass `SET FDOW`/`SET FWEEK` state now feed the date/time helper parse/format path for common MDY/DMY/YMD orders, separators, time display, and week-number defaults.

`ON ERROR` now also has a richer first-pass compatibility lane: `ON ERROR DO <routine> WITH ...` can pass evaluated handler arguments, and handlers can inspect first-pass runtime fault metadata through `MESSAGE()`, `PROGRAM()`, `LINENO()`, `ERROR()`, `AERROR()`, `SYS(2018)`, and `ON('ERROR')` while execution resumes after the handler returns. Normal `AERROR()` rows now preserve the original faulting work area even if the handler changes selection before inspecting the array, and handler execution now keeps those fault metadata helpers bound to the original fault even if the handler triggers and catches a secondary runtime error. Static PRG diagnostics are also now wired through the Studio document-open path so analyzer findings surface to editor-facing consumers instead of staying runtime-only.

`ON SHUTDOWN` now has a first-pass compatibility lane as well. `QUIT` checks for an active shutdown handler and, if configured, executes `ON SHUTDOWN DO <routine> [WITH ...]` before the final quit cleanup/unwind. Common inline compatibility forms are also recognized, including `ON SHUTDOWN CLEAR EVENTS` and close-style cleanup commands such as `CLOSE ALL`, `CLOSE TABLES`, and `CLOSE DATABASE(S) [ALL]`. `ON('SHUTDOWN')` now reports the active shutdown clause for runtime introspection. Shutdown handlers can still perform explicit cleanup such as `CLEAR EVENTS`, `CLOSE DATABASES ALL`, and even a nested `QUIT`; Copperfin treats that nested quit as the final terminal quit instead of recursively dispatching shutdown again.

The current PRG-engine slice now also supports first-pass `WITH/ENDWITH` member binding for leading-dot object access and first-pass `TRY/CATCH/FINALLY/ENDTRY` control flow, plus first-pass `DO ... WITH @var` reference semantics so `LPARAMETERS` assignments can write back into caller variables where needed for FoxPro-style procedure behavior. Inline `&&` comments now stay out of nested double-quoted and braced text during logical-line parsing, so macro-heavy expression text is less likely to be truncated before execution.

## Immediate Runtime Backlog Derived From The Official Reference

The official command inventory is much larger than the current runtime. The deepest next command groups should stay focused on data/runtime compatibility first:

### Active Issue Mapping For This Runtime Backlog

- command-surface and expression/runtime-state depth: #7, #8
- OLE/COM and automation compatibility: #10, #11
- automation fault containment and diagnostics: #12, #13, #14

### Runtime Arrays And Expressions

- keep array compatibility work focused on official helper semantics and real FoxPro migration idioms; the current surface covers declared arrays, table/array transfer commands, metadata helpers, mutation helpers, sorting, scanning, and common row/column copy workflows, but it is still a first-pass runtime model rather than a complete VFP array engine
- deepen predicate and macro expression fidelity around `ASCAN()` and related helpers after the first-pass string/block-style and unquoted braced-literal predicate paths, especially where installed VFP help describes code-block or macro-expanded behavior that should not leak temporary scan metadata into user state
- continue moving new array regressions into `tests/test_prg_engine_arrays.cpp` / `test_prg_engine_arrays` instead of expanding the legacy `test_prg_engine.cpp` catch-all
- continue moving standalone expression/function regressions into `tests/test_prg_engine_functions.cpp` / `test_prg_engine_functions` instead of expanding the legacy `test_prg_engine.cpp` catch-all
- keep expression-function additions in reference-derived batches rather than one helper at a time; after shipping `FORCEEXT()`, `FORCEPATH()`, `CHRTRANC()`, adjacent date/time batches (`DOW()`, `CDOW()`, `CMONTH()`, `GOMONTH()`, `SECONDS()`, `MDY()`, `CTOT()`, `DTOT()`, `TTOD()`, `HOUR()`, `MINUTE()`, `SEC()`), the numeric trig/log helper batch (`SIN()`, `COS()`, `TAN()`, `ASIN()`, `ACOS()`, `ATAN()`, `ATN2()`, `DTOR()`, `RTOD()`, `LOG10()`), the first-pass phonetic string helpers (`SOUNDEX()`, `DIFFERENCE()`), the mixed utility batch (`MIN()` / `MAX()` expression lists, `STRCONV(..., 7/8)`, `ISLEADBYTE()`), and the path/color/random batch (`DEFAULTEXT()`, `CURDIR()`, `RGB()`, `RAND()`), continue with adjacent expression helpers from the reference inventory in similarly sized groups
- keep expression-function additions in reference-derived batches rather than one helper at a time; after shipping `FORCEEXT()`, `FORCEPATH()`, `CHRTRANC()`, adjacent date/time batches (`DOW()`, `CDOW()`, `CMONTH()`, `GOMONTH()`, `SECONDS()`, `MDY()`, `CTOT()`, `DTOT()`, `TTOD()`, `HOUR()`, `MINUTE()`, `SEC()`), the numeric trig/log helper batch (`SIN()`, `COS()`, `TAN()`, `ASIN()`, `ACOS()`, `ATAN()`, `ATN2()`, `DTOR()`, `RTOD()`, `LOG10()`), the first-pass phonetic string helpers (`SOUNDEX()`, `DIFFERENCE()`), the mixed utility batch (`MIN()` / `MAX()` expression lists, `STRCONV(..., 7/8)`, `ISLEADBYTE()`), the path/color/random batch (`DEFAULTEXT()`, `CURDIR()`, `RGB()`, `RAND()`), and the financial/misc batch (`HEX()`, `FV()`, `PV()`, `PAYMENT()`, first-pass `TEXTMERGE()` with configurable delimiters, first-pass `EXECSCRIPT()` RETURN-pattern detection, first-pass `LOOKUP()` seek-and-evaluate, first-pass `NEWID()` UUID v4, `CPCURRENT()` / `CPCONVERT()` / `CPDBF()` code-page stubs, and headless stubs `GETPICT()` / `GETCOLOR()` / `GETFONT()` / `VARREAD()`), continue with adjacent expression helpers from the reference inventory in similarly sized groups
- deepen `ISBLANK()` later when DBF field values preserve pristine blank-state metadata distinct from stored numeric zero or logical false; the current implementation covers expression-level empty/blank character values
- deepen `MLINE()` / `MEMLINES()` later with full compatibility flags and offset semantics beyond the current baseline; the current implementation now supports session-scoped `SET MEMOWIDTH` + `_MLINE`, explicit width overrides, optional tab-width expansion, CR hard breaks by default, optional LF hard-break mode via flag bit `1`, and optional start offsets

### Work Areas, Sessions, And Indexed Data

- finish the remaining alias/work-area edge cases across `SELECT`, `USE`, and nested data sessions after the first strict `USE ... IN <alias>` targeting pass, first-pass non-selected target preservation, plain `USE` current-work-area reuse, freed-work-area reuse plus side-effect-free `SELECT(0)` probing, stronger synthetic SQL cursor/session isolation, per-session SQL handle lifecycle cleanup, and the now-shipped session-local restoration coverage for `SET NEAR` and `SET DEFAULT TO`
- keep closing the remaining expression-driven work-area targeting edges after the shared designator-expression path now covers `SET FILTER ... IN`, `SELECT <expr>`, `USE ... IN <expr>`, and the shipped local `IN`-targeted data/search family (`SET ORDER`, `SEEK`, `LOCATE`, `SCAN`, `GO`, `SKIP`, `REPLACE`, `DELETE`, `RECALL`) for core local-cursor flows
- deepen order/search behavior beyond the current first-pass `SET ORDER TO TAG` / `SEEK` / `FOUND()` plus direction-control slice
- add adjacent data-navigation and data-search commands where VFP developers expect them to work together
- use the command inventory to pull the next search/index/data-session commands in families instead of one at a time

### Local Table Mutation And Query Flow

- continue beyond the new first-pass headless `BROWSE` lane toward the remaining interactive table-editing commands needed for real legacy business applications
- the shared DBF/FPT mutation path now covers shipped local-table write families used by current runtime flows: memo-backed pointer fields (`M`/`G`/`P`), fixed-width numeric/date families (`B`/`I`/`Y`/`T`), first-pass var-length families (`V`/`Q`), constrained `NULL` token mutation semantics, staged write safety for DBF/memo updates (including staged-artifact cleanup), physical `PACK` compaction, first-pass `PACK MEMO` sidecar rewrite/compaction, `ZAP` truncation, SQL-style insert rollback, first-pass `CREATE TABLE`, first-pass structural `ALTER TABLE`, and indexed-table plus unsupported-layout fail-fast guards until real structural/indexed storage fidelity exists
- extend the shipped filtering, browsing, locating, scanning, replacing, appending, SQL-style inserting/deleting, recalling, packing, zapping, first-pass structural `CREATE TABLE` / `ALTER TABLE`, aggregate-built-in, command-level `COUNT`/`SUM`/`AVERAGE` with first-pass scope/`WHILE`/`IN` plus first-pass `TO ARRAY`, first-pass `TOTAL` with `IN` targeting and local `N/F/I/Y` support, and `CALCULATE` families toward broader xBase parity
- deepen table-maintenance parity beyond the first-pass `PACK`/`ZAP`/`PACK MEMO` slice with real cross-process locking behavior, index rebuild/invalidated-order handling, and stronger diagnostics for unsafe structural layouts
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
