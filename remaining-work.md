# Remaining Work

This file is the working guide for the remaining Copperfin implementation effort.

It is intentionally ordered by dependency depth:

1. deepest shared engine layers first
2. runtime semantics next
3. designer/runtime integration after that
4. IDE shells and workflow surfaces on top
5. portability after the Windows-first product is genuinely solid

The goal is not merely feature count. The goal is to finish the stack in an order where each completed layer reduces risk for the layers above it.

## North Star

Copperfin should eventually provide:

- full behavioral compatibility with the parts of `vfp9.exe` that FoxPro developers expect in real projects
- a full Visual Studio 2022+ experience for FoxPro/VFP assets and code
- a full standalone Copperfin IDE experience
- native security and policy controls
- first-class .NET interoperability
- modern database federation
- modern AI/MCP/polyglot workflows where they add value
- a future path to macOS and Linux without poisoning the Windows-first core

## Working Rules

- Keep the trusted runtime, data engine, and compatibility core native-first.
- Do not let designer or shell shortcuts dictate runtime semantics.
- Treat external xBase-family tooling as an accelerator for language-service, project-system, and modern tooling ideas.
- Treat community-maintained FoxPro tooling as an accelerator for FoxPro-specific workflow, tooling, and parity expectations.
- Reuse ideas and reference behavior aggressively, but keep Copperfin implementation clean-room.
- Do not start real cross-platform host work until the Windows runtime and standalone IDE core are stable enough to port.
- Treat this file and `docs/22-vfp-language-reference-coverage.md` as the backlog source of truth; `codex-resume-prompt.txt` is the canonical continuation prompt for automation, and ad hoc prompt files should be merged back here or deleted once stale.

## Priority Ladder

The implementation order should be:

1. Data fidelity and execution engine
2. Compatibility semantics and runtime safety
3. Report/form/menu/class runtime parity
4. Compiler/build/debug pipeline completion
5. Shared design model and visual-designer fidelity
6. Visual Studio and standalone IDE parity
7. Modernization layers: database federation, AI/MCP, polyglot, security polish
8. Portability work for standalone IDE and core runtime

## Open Issue Groups (Current)

This grouped list mirrors the active GitHub issue set so this backlog stays aligned with real unfinished work.

### Runtime Compatibility And Command Surface

- A3 runtime semantics and command depth: #7, #8
- A4 automation and host containment: #10, #11, #12
- Runtime safety and diagnostics: #13, #14

### Runtime Parity Surfaces

- Forms/classes runtime fidelity: #15
- Reports/labels runtime fidelity: #16
- Menus runtime fidelity: #17
- Project startup/build behavior: #18

### Build, Compiler, And Debug Pipeline

- Compiler/runtime contract and package model: #19
- Debugger completion: #20
- Build/run/deploy workflow tightening: #21
- AST/IR and transpilation outputs: #42, #43
- Build artifact breadth and round-trip safety: #38, #39, #40, #41

### Designers And IDE Parity

- Shared design model and memo-heavy round-trip: #22
- Designer interaction/builder/editor completion: #23, #24
- Visual Studio extension parity: #25
- Standalone IDE parity: #26

### Language Service

- Editor semantics and completions: #27
- Navigation/refactoring depth: #28
- IntelliSense metadata inputs: #29

### Federation, Interop, And Modern Platform

- Relational federation and connector behavior: #30
- Document/vector translation and AI planning policy: #31
- .NET outputs and integration hooks: #32
- Interop/compiler LINQ and runtime bridge contracts: #57, #91

### Security And Policy

- Runtime/project security depth: #33
- Extension/host/AI policy hardening: #34

### Portability

- Portable core boundary: #35
- macOS standalone/core port: #36
- Linux standalone/core port: #37

## Priority Architecture Diagram

```mermaid
flowchart TD
    classDef done fill:#d1fae5,stroke:#065f46,stroke-width:2px,color:#064e3b;
    classDef partial fill:#fef3c7,stroke:#92400e,stroke-width:2px,color:#78350f;
    classDef missing fill:#fee2e2,stroke:#991b1b,stroke-width:2px,color:#7f1d1d;
    classDef lane fill:#eef2ff,stroke:#4338ca,stroke-width:1px,color:#1e1b4b;

    subgraph P1[Phase A - Data Fidelity And Execution Engine]
        direction TB
        A1[DBF/FPT Basic Read Fidelity]
        A2[DBF Local-Table Mutation Core]
        A3[CDX/DCX Inspection]
        A4[MDX Inspection]
        A5[DBC Container Fidelity]
        B1[Work Areas And Data Sessions]
        B2[Local Query And Mutation Commands]
        B3[SQL Pass-Through And Remote Cursor Behavior]
        B4[PRG Execution Engine]
    end

    subgraph P2[Phase B - Runtime Parity Surfaces]
        direction TB
        C1[Forms And Classes Runtime Parity]
        C2[Reports And Labels Runtime Parity]
        C3[Menus Runtime Parity]
    end

    subgraph P3[Phase C - Build, Designers, IDE]
        direction TB
        D1[Build Package Debug Pipeline]
        D2[Shared Designers]
        D3[Visual Studio Integration]
        D4[Standalone Copperfin IDE]
        D5[Language Service]
    end

    subgraph P4[Phase D - Platform And Enterprise]
        direction TB
        E1[.NET Interoperability]
        E2[Database Federation]
        E3[Security Policy Controls]
    end

    A1 --> A2 --> A3 --> A4 --> A5
    A2 --> B2
    A3 --> B1
    B1 --> B2 --> B3 --> B4

    B4 --> C1 --> C2 --> C3
    B4 --> D1 --> D2 --> D3 --> D4
    D3 --> D5
    B4 --> E1 --> E2 --> E3

    class A1,A2,A3,A4,A5 done;

    class B1,B2,B3 done;

    class B4 partial;

    class C1,C2,C3 partial;

    class D1,D2,D3,D4,D5 partial;

    class E1,E2,E3 partial;

    class P1,P2,P3,P4 lane;
```

Status legend: green = implemented, amber = partial, red = missing.

## Gap Matrix

Current repo status against the Windows-first product goal:

| Area | Status | Notes |
| --- | --- | --- |
| DBF/FPT basic read fidelity | Implemented | Real DBF parsing, memo decoding, inspector/design-surface consumption, and first-pass structured asset-validation findings for storage/sidecar inconsistencies are in place for representative assets, though broader repair logic is still incomplete. |
| DBF local-table mutation core | Implemented | Native DBF append/field-update/delete-flag writes now exist for local table-backed runtime flows, including shared memo-backed `M`/`G`/`P` pointer-field persistence via FoxPro-compatible sidecars, fixed-width `B`/`I`/`Y`/`T` field create/replace/append support, first-pass `V`/`Q` var-length field create/replace/append support, constrained `NULL` token handling across supported write paths, staged/backup write semantics for DBF/memo updates (with staged-artifact cleanup checks), sidecar-path anomaly recovery for memo writes, and indexed-table mutation parity for production-flag/same-base-companion tables while preserving readable round-trip behavior. |
| CDX/DCX inspection | Implemented | Shared CDX/DCX probing now preserves per-tag page hints plus per-tag page header markers, binds key/`FOR` expressions from tag-page-local neighborhoods before whole-file fallback, surfaces expression-derived normalization/collation hints, and has focused synthetic plus optional real-sample coverage for direct probing and DBC companion discovery. |
| MDX inspection | Implemented | MDX probing now validates block-oriented headers, parses the tag table with per-tag page/format/type/thread markers, extracts first-pass key and `FOR` expressions from tag-header pages with offset hints, surfaces expression-derived normalization/collation metadata, and is covered through direct and companion-inspection tests. |
| DBC/database container fidelity | Implemented | DBC inspection now includes first-pass catalog-object extraction over container records (`OBJECTTYPE`/`OBJECTNAME`/`PARENT*` heuristics), per-type object counts and preview rows, structured warnings when catalog extraction fails or yields no object metadata, and companion `DCT/DCX` validation + parse flow through the shared asset inspector path. |
| Work areas and data sessions | Implemented | The runtime now provides robust session-scoped work-area semantics across `SELECT`, plain `USE`, `USE AGAIN`, `SET DATASESSION`, cursor identity and alias lookup functions, expression-driven `SELECT`/`USE ... IN` designators, `SELECT(0)` next-free probing with freed-area reuse, session-local SQL cursor/handle allocation, session-local `SET DEFAULT TO` and `SET` state restoration, and strict cross-session alias/work-area isolation (including same-work-area-number independence and close semantics) with full runtime regression coverage. |
| Local query/mutation commands | Implemented | Local runtime command parity now includes `GO`, `SKIP`, `SEEK`, `LOCATE`, `SCAN`, `REPLACE`, `APPEND BLANK`, `DELETE`, `RECALL`, SQL-style `DELETE FROM` / `INSERT INTO`, first-pass `CREATE TABLE`, first-pass structural `ALTER TABLE ... ADD/DROP/ALTER COLUMN`, `PACK`, `PACK MEMO`, and `ZAP` with command-level `FOR`/`WHILE` and expression-driven `IN`/cursor-target support where applicable, plus `SET FILTER TO/OFF`, aggregate built-ins, `CALCULATE`, command-level `COUNT`/`SUM`/`AVERAGE` with scope/`WHILE`/`IN` and `TO ARRAY`, and `TOTAL` with `IN` targeting plus local `I`/`Y` field support. Focused regressions now lock down cross-cursor targeting, boundary behavior, SQL-style local DBF insert/delete persistence and rollback, physical DBF creation/schema rewrite/compaction/memo-sidecar rewrite/truncation, and mutation persistence across local DBF workflows. |
| SQL pass-through/remote cursor behavior | Implemented | SQL pass-through now includes connection/session plumbing (`SQLCONNECT`/`SQLSTRINGCONNECT`/`SQLDISCONNECT`), command execution (`SQLEXEC`) with DML rows-affected tracking (`SQLROWCOUNT`), prepared-command execution (`SQLPREPARE` + `SQLEXEC(handle)`), and connection property metadata (`SQLGETPROP`/`SQLSETPROP`) with provider-hint classification. Remote cursor behavior covers local-style field lookup/navigation/filter-aware visibility, index-aware seek/order flows, aggregate semantics, in-memory mutation commands (`APPEND BLANK`, `REPLACE`, `DELETE`/`RECALL` including `FOR`/`WHILE` targeting), and SQL-style `INSERT INTO` / `DELETE FROM` over `SQLEXEC()` result cursors with focused runtime regression coverage. |
| PRG execution engine | Partial | Core execution semantics are in place, but command-surface depth and runtime-state/macro parity still have active backlog in #7 and #8. |
| Forms/classes runtime parity | Partial | First-pass runtime bootstrap is shipped, but event/lifecycle and behavior-fidelity depth remains open in #15. |
| Reports/labels runtime parity | Partial | Preview and first output paths are shipped, but report/label runtime and pipeline completion remains open in #16. |
| Menus runtime parity | Partial | First-pass bootstrap and dispatch are shipped, but menu routing/state semantics remain open in #17. |
| Build/package/debug pipeline | Partial | Baseline orchestration is shipped, but compiler/runtime contract, debugger completion, and workflow tightening remain open in #19, #20, and #21. |
| Shared designers | Partial | Shared design surfaces exist, but high-fidelity model completion and round-trip robustness remain open in #22. |
| Visual Studio integration | Partial | Extension baseline is shipped, but full parity and utility-pane completion remain open in #25. |
| Standalone Copperfin IDE | Partial | Standalone shell is shipped, but full daily-driver IDE parity remains open in #26. |
| Language service | Partial | Core language-service features are shipped, but semantic resolution/navigation/refactoring depth remains open in #27, #28, and #29. |
| .NET interoperability | Partial | Build-time launcher generation and documented architecture exist, but broad first-class runtime interop is still incomplete. |
| Database federation | Partial | Platform models now include a first-pass deterministic Fox SQL translator lane for relational backends (`sqlite`, `postgresql`, `sqlserver`, `oracle`) with focused unit coverage, and runtime host now has a deterministic federated query execution-planning lane for backend/target/sql plan materialization, but live connector execution integration is still incomplete. |
| Security/policy controls | Partial | Baseline security controls are shipped (integrity verification, policy-gated process launch, RBAC checks, secret-provider enforcement, immutable audit stream, SBOM/CVE gate), but deeper runtime/project and extension/AI policy hardening remains open in #33 and #34. |

## Phase A: Core Data And Compatibility Engine

This is the deepest layer and should continue to absorb the most effort until it is boringly reliable.

### Progress Notes

- 2026-04-21: Runtime source-size cleanup continued by extracting expression date/time helper dispatch from `prg_engine_expression.inl` into dedicated `prg_engine_date_time_functions` sources. The move keeps the existing focused `test_prg_engine_functions` date/time coverage as the regression guard while reducing the expression parser include by roughly 545 lines.

- 2026-04-21: Focused expression-function test splitting continued by moving date/time expression regressions into a dedicated `test_prg_engine_date_time_functions` target. The original `test_prg_engine_functions` target now keeps the remaining path/string/math/type/null/aggregate/runtime-surface batches smaller, while the date/time target owns constructor, conversion, week, Julian, and invalid-input coverage.

- 2026-04-21: Focused expression-function test splitting continued again by moving portable path helper coverage into a dedicated `test_prg_engine_path_functions` target. The split keeps Windows-style and POSIX-style `JUST*` / `FORCE*` regressions isolated from the remaining mixed expression-function batches.

- 2026-04-21: Date/time conversion invalid-input semantics were tightened with focused TDD coverage in `test_prg_engine_functions`: `JTOD()` / `JTOT()` now reject out-of-runtime-range Julian values instead of materializing impossible negative-year dates, `DTOJ()` / `CTOD()` reject trailing garbage after slash or compact date input, and `TTOJ()` now preserves datetime input support through explicit datetime parsing.

- 2026-04-21: The first-pass `WEEK()` expression helper now accepts a third argument for first-week-of-year mode in addition to the existing first-day argument: mode `1` keeps Jan-1-containing week behavior, mode `2` uses first full week semantics, and mode `3` uses first-4-day-week semantics. Focused `test_prg_engine_functions` coverage now locks down deterministic year-boundary and cross-year rollover behavior for these modes, including January rollback into the prior week-year and late-December mode-3 rollover into week 1.

- 2026-04-21: Expression-function date/time conversion coverage gained first-pass `STOD()` and `TTOS()` in `test_prg_engine_functions`. `STOD()` now accepts compact `YYYYMMDD` inputs and returns normalized `MM/DD/YYYY` dates with empty-string output for malformed inputs; `TTOS()` now emits deterministic sortable `YYYYMMDDHHMMSS` strings from datetime or date inputs (date-only values normalize to midnight).

- 2026-04-21: Expression-function date/time coverage gained constructor-argument support for `DATE()` and `DATETIME()` in `test_prg_engine_functions`. The runtime now accepts `DATE(year, month, day)` and `DATETIME(year, month, day[, hour[, minute[, second]]])` with first-pass range validation and deterministic `MM/DD/YYYY` and `MM/DD/YYYY HH:MM:SS` output formatting, while preserving existing no-argument current-date/current-datetime behavior.

- 2026-04-21: Expression-function date/time coverage gained another adjacent helper batch in `test_prg_engine_functions`: first-pass `WEEK()`, `QUARTER()`, and `EOMONTH()`. The runtime now computes week-of-year values with optional first-day offset (`WEEK(date, firstDay)`), quarter-of-year extraction from parsed date inputs, and end-of-month date materialization with optional month deltas (`EOMONTH(date, nMonths)`) while preserving existing `MM/DD/YYYY` string-date semantics.

- 2026-04-18: command-level aggregate follow-through gained first-pass `TO ARRAY` parity for `COUNT`, `SUM`, and `AVERAGE` using the shared runtime array assignment path. Focused `test_prg_engine_functions` coverage now validates one-element array assignment, scope-clause behavior (`ALL`/`REST`/`NEXT`/`RECORD`), `FOR` filters, `IN` alias targeting, and malformed `TO ARRAY` diagnostics.

- 2026-04-18: Expression-function date/time coverage gained the next adjacent conversion batch in `test_prg_engine_functions`: `CTOT()`, `DTOT()`, `TTOD()`, `HOUR()`, `MINUTE()`, and `SEC()`. The runtime now parses first-pass `MM/DD/YYYY HH:MM:SS` datetime strings for conversion and extraction helpers, emits normalized `MM/DD/YYYY HH:MM:SS` output for `CTOT()` / `DTOT()`, and supports component extraction from both time-only and datetime string inputs for deterministic regression assertions.

- 2026-04-18: Expression-function date/time helper coverage gained the next small adjacent batch in `test_prg_engine_functions`: `DOW()`, `CDOW()`, `CMONTH()`, `GOMONTH()`, `SECONDS()`, and `MDY()`. The runtime now parses `MM/DD/YYYY` (plus compact `YYYYMMDD`) for these helpers, supports first-pass `DOW(..., <firstDayOfWeek>)` shifting, clamps `GOMONTH()` to month-end on overflow days (for example, `01/31 + 1` month), and keeps `SECONDS()` as second-of-day output for deterministic numeric assertions.

- 2026-04-18: `ASCAN()` predicate search now preserves unquoted braced block-style arguments through expression parsing, so forms like `ASCAN(aValues, {|x| x > 8}, -1, -1, -1, 16)` work in addition to stringified predicates. Array declaration/element and array metadata/text helper regressions moved from the legacy PRG catch-all into `test_prg_engine_arrays`. A new `test_prg_engine_functions` target now starts the standalone expression-function split and covers portable string parsing for `JUSTPATH()`, `JUSTFNAME()`, `JUSTSTEM()`, `JUSTEXT()`, and new `JUSTDRIVE()` across Windows-style and POSIX-style paths.

- 2026-04-18: Expression-function test splitting continued: string/math and type/null function coverage moved from `test_prg_engine` into `test_prg_engine_functions`. The same focused target now covers the next small manifest-derived utility batch: `FORCEEXT()`, `FORCEPATH()`, and first-pass case-insensitive `CHRTRANC()` behavior. Local focused validation passed for both `test_prg_engine_functions` and the trimmed legacy `test_prg_engine`.

- 2026-04-18: Array parity now has a dedicated `test_prg_engine_arrays` regression target. `ASCAN()` gained first-pass predicate-expression search under flag `16`, including stringified `{|x| ...}` block-style predicates and temporary `_ASCANVALUE` / `_ASCANINDEX` / `_ASCANROW` / `_ASCANCOLUMN` metadata restored after the scan. Focused coverage also locks down common two-dimensional `ACOPY()` workflows: whole-row copies via `AELEMENT()` + `ALEN(..., 2)` and column-helper copies through one-element `ACOPY()` calls.

- 2026-04-18: Array parity now covers numeric-aware `ASORT()` ordering and VFP-style `ADEL()`/`AINS()` row and column behavior for two-dimensional arrays, including false-filled trailing or inserted slots. Focused PRG regression coverage exercises numeric sorting, bounded/windowed sorting, two-dimensional row sorts, row deletion/insertion, and column deletion/insertion.

- 2026-04-18: Runtime array parity advanced again: `ASCAN()` now understands search columns plus case-insensitive, exactness override, and row-return flags; `ASORT()` now supports start/count windows, descending order, case-insensitive keys, and two-dimensional row sorting by the start element's column. The Linux validation warning noise was reduced by replacing several partial designated initializers in runtime code, moving Windows-only DLL parameter helpers behind the Windows guard, formatting date/time strings without fixed buffers, and suppressing intentional test harness aggregate-default warnings locally in `test_prg_engine.cpp`.

- 2026-04-17: GitHub-hosted validation now has a dedicated native CMake workflow across Linux, macOS, and Windows, current Node 24-compatible action versions, manual-only Windows deep validation for native/VSIX/Studio/smoke-test build coverage, and fixed installer artifact uploads for CPack outputs. Runtime array parity also gained start/count-bounded `ASCAN()` scans over row-major array storage with focused regression coverage.

- 2026-04-17: Linux-side validation is now available from a repo-local `.codex-venv` CMake install, with local generated artifacts ignored and a `scripts/validate-posix.sh` wrapper that runs under `zsh`, `bash`, or `sh`. The first Linux runs exposed portability issues in Windows-only process includes, PowerShell test launching, path-separator-specific exporter assertions, DBF memo-sidecar directory reads, project-workspace Windows-path fallback handling, runtime-host canonical-name validation, and non-Windows SHA-256 hashing; those were fixed without changing the Windows-first product direction. The full native CTest suite now builds and passes under the local Linux CMake/Ninja toolchain.

- 2026-04-17: The PRG expression-function surface gained the next small official string/comparison utility batch: occurrence-aware `AT()` / `RAT()`, first-pass case-insensitive `ATC()` / `RATC()`, line-oriented `ATLINE()` / `ATCLINE()` / `RATLINE()`, wildcard `LIKE()`, `INLIST()`, and `PROPER()`. Focused coverage was added to the existing string/math expression regression cluster.

- 2026-04-17: Structural local-table coverage now includes first-pass `ALTER TABLE ... ADD COLUMN`, implemented as a conservative DBF schema rewrite that appends one supported field while preserving existing values and deleted flags. `CREATE TABLE` field parsing also accepts the broader DBF write-surface type family (`B`/`DOUBLE`, `V`/`VARCHAR`, `Q`/`VARBINARY`) and nullable/default annotations as declaration syntax. Synthetic SQL result cursor parity now covers direct SQL-style `INSERT INTO <sqlcursor>` and `DELETE FROM <sqlcursor> WHERE ...` while preserving the selected cursor and target row state. Focused `test_prg_engine` coverage passes, and `test_dbf_table` remains green.

- 2026-04-17: The structural table slice now also covers `ALTER TABLE ... DROP COLUMN`, `ALTER TABLE ... ALTER COLUMN`, runtime-enforced `DEFAULT`/`NOT NULL` metadata for created or altered open cursors, and first-pass `PACK MEMO` memo-sidecar compaction. Structural field parsing moved out of the PRG engine monolith into `prg_engine_table_structure_helpers`, and table-structure runtime regressions moved into the dedicated `test_prg_engine_table_structure` executable so new coverage no longer grows the already-large main PRG test file. `test_dbf_table`, `test_prg_engine`, and `test_prg_engine_table_structure` pass.

- 2026-04-17: The large PRG-engine regression file has started its incremental split. Shared fixture helpers now live in `tests/prg_engine_test_support.{h,cpp}`. SQL result cursor coverage moved to `test_prg_engine_sql_cursors`, and local table mutation/update coverage moved to `test_prg_engine_table_mutation`, cutting `tests/test_prg_engine.cpp` from roughly 10,240 lines to roughly 7,460 lines while keeping a legacy catch-all for the remaining clusters. `test_prg_engine`, `test_prg_engine_sql_cursors`, `test_prg_engine_table_mutation`, and `test_prg_engine_table_structure` pass.

- 2026-04-17: Local table-maintenance commands now include first-pass `PACK` and `ZAP` support in the PRG runtime. `PACK` physically compacts local DBF-backed cursors by removing deleted records while preserving schema and row order for kept records; `ZAP` truncates local DBF-backed cursors to zero records while keeping the table appendable through the existing `APPEND BLANK` / `REPLACE` mutation path. Synthetic remote/result cursors get matching in-memory first-pass behavior. New focused `test_prg_engine` coverage verifies updated `RECCOUNT()`, readable DBF output, file-size shrinkage, event emission, and append-after-`ZAP` persistence; `test_dbf_table` and `test_prg_engine` pass.

- 2026-04-17: SQL-style local table mutation now includes first-pass `DELETE FROM <target> WHERE/FOR <expr>` and `INSERT INTO <target> [(fields)] VALUES (...)`. `DELETE FROM` uses a distinct statement kind so conditionless SQL-style deletes tombstone all target-visible rows instead of inheriting xBase bare-`DELETE` current-record semantics. `INSERT INTO` appends through the existing blank-row plus field-replacement path, maps omitted field lists by target schema order, and supports expression-driven cursor targets. The shared command CSV splitter now keeps commas inside both single- and double-quoted tokens together, while expression-level double-quoted string semantics remain a separate language-compatibility slice. Focused `test_prg_engine` coverage verifies expression target resolution, reversed field-list mapping, schema-order insertion, `DELETE FROM WHERE` tombstoning, event emission, selected-cursor preservation, and persisted DBF readability.

- 2026-04-17: `INSERT INTO` failure handling now rolls back the appended blank row when a later field write fails. Local DBF-backed inserts truncate the table back to the pre-insert record count; synthetic remote/result cursors resize the in-memory row set back to its original count. Focused `test_prg_engine` coverage verifies oversized character inserts pause on the field-write error, then continuing execution sees the original `RECCOUNT()`, original DBF file size, readable DBF output, and original row values.

- 2026-04-17: `CREATE TABLE` now has first-pass local DBF support for simple field declarations (`C`/`CHAR`, `N`/`NUMERIC`, `F`/`FLOAT`, `L`/`LOGICAL`, `D`/`DATE`, `I`/`INTEGER`, `Y`/`CURRENCY`, `T`/`DATETIME`, and `M`/`MEMO`). The command creates a DBF through `create_dbf_table_file`, opens the created table as a local cursor, and immediately participates in `INSERT INTO`, field lookup, and `RECCOUNT()`. Focused `test_prg_engine` coverage verifies created schema metadata, insert/read behavior, event emission, and persisted DBF readability.

- 2026-04-16: The next combined runtime/data batch added first-pass `ALINES()`, `ADIR()`, and `AFIELDS()` over the runtime array substrate; broadened diagnostics so SQL/ODBC-like failures populate a VFP-style `1526` `AERROR()` row and OLE/automation-like failures populate a `1429` row; and expanded text import/export beyond SDF with VFP-style `COPY TO ... TYPE CSV`, `COPY TO ... DELIMITED`, `APPEND FROM ... TYPE CSV`, and `APPEND FROM ... DELIMITED`. CSV export now writes the field-name header row, quoted character fields, and unquoted numeric/logical fields; CSV import skips a matching header row. `DELIMITED` honors both field enclosure (`WITH '_'`) and separator (`WITH CHARACTER ';'`) options. Focused `test_prg_engine` coverage locks down array metadata functions, SQL/OLE `AERROR()` row shapes, and CSV/delimited round-trips.

- 2026-04-16: Runtime arrays now include the next adjacent helper family: `ACOPY()`, `AELEMENT()`, and `ASUBSCRIPT()` in addition to the existing mutator/search helpers. `ACOPY()` copies row-major windows between runtime arrays and can grow the target array when needed; `AELEMENT()` / `ASUBSCRIPT()` translate between 2D subscripts and VFP-style one-based element numbers. The diagnostics slice was corrected against the VFP reference shape: `AERROR(<array>)` now emits a one-row/seven-column array for normal runtime errors, with code, message, mixed-case error parameter, selected work area, and empty trigger/reserved columns; `SYS(2018)` exposes the uppercase error parameter. Focused `test_prg_engine` coverage locks down the array helper behavior and corrected `AERROR()`/`SYS(2018)` metadata.

- 2026-04-16: Runtime arrays now also support first-pass `DIMENSION` plus array-form `DECLARE`, direct element assignment through bracket and parenthesis syntax (`array[1] = ...`, `array[1,2] = ...`, `array(1) = ...`, `array(1,2) = ...`), and two-dimensional reads that preserve values across `ASIZE()` growth. The shared command CSV splitter was tightened so commas inside `[]` dimensions do not split declarations or argument lists prematurely. Focused `test_prg_engine` coverage locks down declared 1D/2D arrays, mixed bracket/paren reads and writes, and 2D resize preservation.

- 2026-04-16: Runtime arrays now support first-pass mutator/search helpers: `ASIZE()`, `ASCAN()`, `ADEL()`, `AINS()`, and `ASORT()` over arrays created by runtime paths such as `SCATTER TO` and `AERROR()`. `UPDATE` parsing also now covers `UPDATE SET ...` against the selected cursor and `UPDATE IN <alias> SET ...` in addition to the existing `UPDATE <alias> SET ...` form. Focused `test_prg_engine` coverage locks down the new array mutators and expanded update grammar. A later diagnostics correction restored `AERROR()` to the VFP-aligned seven-column shape.

- 2026-04-16: `AERROR(<array>)` now populates the first real structured runtime error array on top of the new runtime array substrate. The corrected first-pass shape is one row by seven columns for normal runtime errors: error code, message, mixed-case error parameter, selected work area, and empty trigger/reserved columns. This complements `MESSAGE()`, `PROGRAM()`, `LINENO()`, `ERROR()`, `SYS(2018)`, and `ON('ERROR')` and gives diagnostics/debugger work richer state to inspect without overclaiming a non-VFP nine-column shape. `UPDATE <alias> SET field = expr [, ...] WHERE/FOR <expr>` also shipped as a first-pass scoped mutation command over local and remote cursors, reusing the existing replacement engine while keeping legacy `REPLACE` current-record semantics intact. Focused `test_prg_engine` coverage locks down both `AERROR()` array access and scoped `UPDATE` mutation.

- 2026-04-16: `SCATTER` / `GATHER` parity moved beyond memvar-only basics. `SCATTER FIELDS ... MEMVAR`, `SCATTER ... MEMVAR BLANK`, and `GATHER MEMVAR FIELDS ... FOR ...` now honor field filtering, typed value conversion, typed blank values, and `FOR`-gated replacement. The runtime also has first-pass internal array storage for `SCATTER ... TO <array>` and `GATHER FROM <array>`, including one-dimensional `ALEN(<array>)`, `ALEN(<array>, 1/2)`, bracket reads (`array[1]`), and paren reads (`array(1)`). Focused `test_prg_engine` coverage locks down memvar and array round-trips, giving the upcoming `AERROR()` slice a real array substrate instead of the old `ALEN()` stub.

- 2026-04-16: `COPY TO ... TYPE SDF` and `APPEND FROM ... TYPE SDF` now have first-pass fixed-width text support. SDF export writes one CRLF-terminated row per visible local DBF record using selected field descriptor widths, with `FIELDS` and `FOR` clauses flowing through the existing DBF export selection path. SDF import reads fixed-width rows back into the current local DBF cursor by slicing against the current table schema and optional `FIELDS` selection, appending each row through the shared `append_blank_record_to_file` + `replace_record_field_value` mutation path. Focused `test_prg_engine` coverage locks down export and import round-trips.

- 2026-04-16: `m.` memory-variable namespace parity was tightened across assignment, lookup, expression evaluation, and declarations. `m.<name>`, `M.<name>`, and bare `<name>` now share the same local/global runtime binding instead of creating separate prefixed variables or falling through to OLE property resolution. `PUBLIC`/`LOCAL`/`PRIVATE`/`LPARAMETERS` declarations also canonicalize the prefix, and focused `test_prg_engine` coverage locks down prefixed-to-bare and bare-to-prefixed reads/writes.

- 2026-04-16: `COPY TO`, `COPY STRUCTURE TO`, and `APPEND FROM` moved from event-only stubs to full local-DBF implementations. `COPY TO` resolves and creates the destination file via `create_dbf_table_file`, respecting optional `FIELDS` and `FOR` clauses. `COPY STRUCTURE TO` follows the same path but skips record population (zero rows). `APPEND FROM` parses the source via `parse_dbf_table_from_file` then inserts each non-deleted source record using `append_blank_record_to_file` + `replace_record_field_value`. `GATHER MEMVAR` correctness was fixed: the old path built `"value"` string expressions and re-evaluated them (lossy for numeric/logical/date); the new path calls `replace_record_field_value` directly. A related assignment-handler bug was fixed: dotted identifiers with `m.` prefix were being incorrectly dispatched through the OLE property path; `m.` is now recognized as the VFP memory-variable namespace and stored as a plain variable. All four changes have focused regression tests in `test_prg_engine`; all tests pass.

- 2026-04-13: status classification was re-aligned to the active issue backlog. Multiple areas previously marked green are now tracked as partial until the corresponding open issue groups are closed and validated.

- 2026-04-13: the old gray dependency callouts under Phase C were retired from the architecture diagram. Shared designers, Visual Studio integration, the standalone Studio shell, and the shipped language-service baseline are all now green, so those relationships are no longer tracked as unresolved blockers.
- 2026-04-13: Phase C reached green status. The repo now treats the build/package/debug pipeline, shared designers, Visual Studio integration, standalone Studio shell, and FoxPro language-service layer as implemented, with the subsystem registry updated to match the shipped Phase C surfaces.
- 2026-04-13: Phase B runtime parity surfaces reached green status. The shared xAsset runtime model now covers first-pass form/class startup and shutdown sequencing (`DataEnvironment` open/close hooks plus root-object lifecycle methods where present), report/label preview and `TO FILE` runtime lanes are both regression-covered, and menu bootstrap generation now includes setup, action dispatch, submenu activation wrapping, and cleanup with real-sample model validation.
- 2026-04-13: PRG execution-engine parity reached green status. The runtime now supports first-pass `WITH/ENDWITH` execution semantics for leading-dot member access, first-pass `TRY/CATCH/FINALLY/ENDTRY` control flow with handled-error continuation and `FINALLY` execution, and first-pass `DO ... WITH @var` reference semantics that write callee updates back to the caller.
- 2026-04-13: first-pass host/process hardening landed across build/runtime/studio/inspect executables. Windows process startup now applies DLL search-path hardening, security-enabled package materialization now rejects non-canonical runtime-host binary names, and generated launcher publish now avoids shell-based command execution.
- 2026-04-13: enterprise security controls now have an end-to-end baseline. Runtime packages now emit SHA-256 digests for the packaged runtime host and extension payload entries, runtime host verifies those digests before execution when security is enabled, external process launches are policy-gated through trusted signature/publisher allow-list checks, security-enabled release packaging enforces provider-backed signing-key references (`env:<NAME>`), immutable hash-chained audit events are emitted for build/runtime security actions, runtime/build hosts enforce role permissions for privileged operations, and CI now includes SBOM output plus a HIGH/CRITICAL CVE gate.
- 2026-04-13: database federation now has a first-pass deterministic translator slice in native code. Copperfin can now translate constrained Fox SQL `SELECT ... FROM ...` inputs into backend-targeted relational SQL variants for `sqlite`, `postgresql`, `sqlserver`, and `oracle`, with focused regression coverage in `test_query_translator` while deeper connector/runtime execution integration remains open.
- 2026-04-13: runtime federation integration now has a first-pass deterministic connector execution-planning lane. `copperfin_runtime_host` can now materialize federated query plans (`--federation-backend` + `--federation-query`) that consume the shared translator, emit backend/connector/target metadata, and return deterministic execution-command scaffolding with focused coverage in `test_federation_execution`, while live backend connector execution still remains open.
- 2026-04-12: PRG structured-flow semantics now include first-pass `ELSEIF` branch support and `DO ... WITH` argument binding into `PARAMETERS`/`LPARAMETERS` locals for called routines, with focused regression coverage.
- 2026-04-12: the PRG engine split progressed further: parser loading now lives in `prg_engine_parser.cpp`, static diagnostics live in a shared `cf_prg_analysis` library, and the Studio document path now carries analyzer diagnostics for `.prg` files without introducing a runtime/design-model link cycle.
- 2026-04-12: `ON ERROR` compatibility now has a richer first-pass handler lane. `ON ERROR DO <routine> WITH ...` can pass evaluated handler arguments, and handler-visible `MESSAGE()`, `PROGRAM()`, `LINENO()`, and `ERROR()` now expose the failing statement context with focused runtime regression coverage.
- 2026-04-12: PRG runtime safety now includes configurable guardrails (`max_call_depth`, `max_executed_statements`, `max_loop_iterations`), cooperative scheduler yielding, and first-pass `CONFIG.FPW`/`CONFIG.FP` loading for guardrail and temp-directory policy. Runtime temp defaults now prefer OS-local temp rather than application/share paths.
- 2026-04-12: PRG runtime internals were split into dedicated modules for maintainability (`prg_engine_runtime_config.cpp` and `prg_engine_static_analysis.cpp`) to reduce monolith pressure in `prg_engine.cpp` while preserving full regression coverage.
- 2026-04-12: local mutation command parity now supports first-pass `WHILE` clause semantics across `REPLACE`, `DELETE`, and `RECALL` in addition to `FOR`/`IN` targeting. Runtime mutation walks now stop at the first failing `WHILE` boundary and preserve non-targeted records, with focused regression coverage on targeted-cursor write behavior.
- 2026-04-12: SQL pass-through now includes first-pass prepared execution and connection-property control surfaces. `SQLPREPARE`, `SQLEXEC(<handle>)`, `SQLGETPROP`, and `SQLSETPROP` are now wired through runtime connection state with provider/timeout/prepared-command metadata and event emission coverage.
- 2026-04-12: local query command parity now supports first-pass `WHILE` clause semantics on `LOCATE` and `SCAN`, including expression-driven `IN` targeting. Runtime cursor walks now stop at the first failing `WHILE` boundary, and focused regression coverage locks down targeted-cursor positioning and scan-iteration counts.
- 2026-04-12: SQL pass-through now has first-pass DML metadata coverage beyond cursor-only SELECT flows. Runtime `SQLEXEC()` now tracks rows-affected semantics for insert/update/delete-style commands and exposes the latest result through `SQLROWCOUNT(<handle>)`, while SQL connection state now infers provider hints (for example ODBC/OLE DB-style connect strings) for runtime inspection.
- 2026-04-12: command-level local mutation parity now includes first-pass `REPLACE ... FOR ... IN ...` coverage. Runtime execution now applies replacements across all matching visible records for the targeted cursor instead of only the current record, with focused regression coverage on matching/non-matching row behavior.
- 2026-04-12: reports/labels now support a first-pass non-preview render lane through `REPORT/LABEL ... TO FILE`. Runtime execution now emits render artifacts and continues script execution without entering event-loop preview mode, with focused runtime coverage for the TO FILE path.
- 2026-04-12: indexed-table local mutation guardrails were removed for production-flag and same-base companion-index tables. Shared DBF mutation APIs (`APPEND BLANK`, `REPLACE`, `DELETE`/`RECALL`) now execute on indexed tables, with focused DBF/runtime regressions locking down persisted writes and readable round-trip behavior.
- 2026-04-12: synthetic SQL remote-cursor parity now has focused mutation-command coverage. Runtime regressions now lock down `APPEND BLANK`, `REPLACE`, `DELETE FOR`, and `RECALL FOR` over `SQLEXEC()` result cursors, including appended-row value persistence and runtime event emission.
- 2026-04-12: PRG `TEXT` blocks now support first-pass `TEXTMERGE` interpolation. Runtime execution now resolves `<<expression>>` segments inside `TEXT ... ENDTEXT` blocks using the existing expression evaluator, and focused regression coverage locks down merged output values and runtime event emission.
- 2026-04-12: work-area/data-session runtime parity now has explicit cross-session isolation coverage. Regression checks now lock down that alias lookup is session-local, the same table can be opened in different sessions without collision, work-area `1` is independent per session, and `USE IN 1` only closes the current session's area 1 without bleeding into other sessions.
- 2026-04-12: DBC container inspection now surfaces first-pass catalog metadata instead of only header/companion checks. The inspector now loads container records, extracts normalized object-type/name/parent hints, emits per-type object counts plus bounded object previews, and reports structured warnings when catalog extraction fails or returns no catalog-object metadata.
- 2026-04-12: index inspection now captures richer per-tag metadata for both CDX/DCX and MDX. CDX/DCX probes now expose tag-page header marker hints (`flags`/entry counts) in addition to grounded tag-page key/`FOR` binding, while MDX probes now parse tag-table marker bytes (format/type/thread), tag-header page offsets, and first-pass key/`FOR` expressions from tag-header pages instead of name-only block hints.
- 2026-04-12: shared DBF mutation now supports first-pass `V`/`Q` var-length field create/replace/append flows using trailing-length storage semantics, with focused round-trip coverage for value updates plus blank append initialization while keeping unsupported-layout guards pinned to genuinely unsupported field types.
- 2026-04-12: shared DBF mutation now accepts a constrained `NULL` token across supported direct-write field families (`C`/`N`/`F`/`L`/`D`/`B`/`I`/`Y`/`T`) using first-pass clear/zero semantics, and date display decoding now renders all-space stored dates as empty values instead of synthetic `----` placeholders.
- 2026-04-12: staged DBF/memo writes now have focused cleanup coverage that locks down removal of temporary (`.cptmp`) and backup (`.cpbak`) artifacts after successful mutation writes.
- 2026-04-12: shared DBF mutation writes now use staged temporary-file swaps with backup/restore behavior to reduce partial-write risk during local-table mutation. Memo-backed replacements now also recover sidecar-path anomalies (for example, unexpected directory collisions at the memo sidecar path) while keeping DBF data readable and updated.
- 2026-04-12: shared DBF mutation now supports FoxPro `General (G)` and `Picture (P)` pointer fields using the same memo-sidecar write path as `M`. Create/replace/append flows now persist and reload G/P memo payloads, with focused round-trip coverage locking down pointer updates and blank-append pointer initialization.
- 2026-04-12: shared DBF mutation now supports FoxPro `Double (B)` field create/replace/append flows. The layer now enforces fixed-width 8-byte `B` storage, persists binary double values on write paths, decodes `B` values back through parser display output, and has focused round-trip coverage locking down positive/negative replacement plus blank-append zero initialization.
- 2026-04-12: local indexed-search now evaluates a constrained first-pass `STR(<numeric>[, <width>[, <decimals>]])` tag-expression path on shipped command/function `SEEK` flows. Loaded tags such as `UPPER(STR(AGE, 3))`, `UPPER(STR(AGE))`, and `UPPER(STR(AGE, 5, 1))` now produce fixed-width right-aligned keys with focused regression coverage for default-width and decimal seek behavior.
- 2026-04-12: local indexed-search now locks down first-pass default-space `PADL(...)` / `PADR(...)` tag-expression behavior in addition to explicit pad-character forms. This slice also preserves significant seek-key whitespace on command/function `SEEK`, which is required for padded-key tag matches.
- 2026-04-12: local indexed-search now evaluates first-pass padded tag expressions with `PADL(...)` / `PADR(...)` on the shipped `SEEK` path. Loaded tags such as `UPPER(PADL(NAME, 8, '0'))` and `UPPER(PADR(NAME, 8, '0'))` now produce derived keys for command/function `SEEK`, with focused regression coverage locking down padded-key matches.
- 2026-04-12: local selected-work-area reuse now has focused data-session round-trip coverage after closing the selected alias. Regression checks now lock down `USE IN <selected-alias>`, `SELECT(0)`, and restored plain `USE` behavior so the original session keeps reusing the emptied selected work area after switching away and back.
- 2026-04-12: local indexed-search now evaluates first-pass substring-style tag expressions with `RIGHT(...)` and `SUBSTR(...)` on the shipped `SEEK` path in addition to the earlier `LEFT(...)` slice. Loaded tags such as `UPPER(RIGHT(NAME, 3))` and `UPPER(SUBSTR(NAME, 2, 3))` now produce derived keys for command/function `SEEK`, and the shared CDX parser now also binds descriptive stored tag names such as `FULLNAME` to their hinted tag-page-local expressions even when the tag name does not resemble the key expression.
- 2026-04-12: local indexed-search now evaluates first-pass substring-style tag expressions with `LEFT(...)` on the shipped `SEEK` path. Loaded tags such as `UPPER(LEFT(NAME, 3))` now produce truncated normalized keys for command/function `SEEK`, and the synthetic CDX fixture coverage now writes realistic tag-page pointers so composite/tag-expression regressions keep exercising the parser/runtime seam the way FoxPro indexes expect.
- 2026-04-12: local indexed-search now evaluates first-pass composite tag expressions on the shipped `SEEK` path instead of only simple field/unary expressions. Concatenated expressions such as `UPPER(LAST+FIRST)` now produce real composite keys for command/function `SEEK`, with focused regression coverage locking down case-folded concatenated matches.
- 2026-04-12: first-pass local `TOTAL` parity now accepts FoxPro `Integer (I)` and `Currency (Y)` fields in addition to `N/F`. Output DBFs preserve fixed-width `I`/`Y` field layouts instead of widening them into invalid descriptors, and focused regression coverage now locks down grouped currency-plus-integer totals while preserving the current record position.
- 2026-04-12: local-table selection-flow parity now has focused data-session round-trip coverage alongside the existing SQL variant. Regression checks now lock down `SELECT 0`, plain `USE`, and restored selected-work-area/alias behavior across `SET DATASESSION` switches so each session keeps reusing its own selected empty local work area.
- 2026-04-12: synthetic SQL temporary-order probe coverage now also locks down the combined direction-suffix path. `SEEK()` / `INDEXSEEK(.T.)` with one-off designators such as `UPPER(NAME) DESCENDING` now have focused regression coverage proving case-folded descending near-positioning while still preserving the controlling order.
- 2026-04-12: synthetic SQL temporary-order expressions now derive and consume first-pass normalization metadata instead of treating every ad hoc order as raw string compare only. `SEEK()` with one-off order expressions such as `UPPER(NAME)` plus command-path `SET ORDER TO UPPER(NAME)` / `SEEK` now case-fold search keys against in-memory SQL rows, with focused regression coverage and runtime event metadata.
- 2026-04-12: synthetic SQL order-direction targeting now has focused non-selected-target `IN` coverage for `SET ORDER ... DESCENDING` composed with command-path `SEEK` plus `SET NEAR`. Regression checks now lock down targeted descending miss positioning while preserving the selected SQL alias and pointer.
- 2026-04-12: synthetic SQL targeted-filter parity now has focused `SET FILTER ... IN` coverage composed with `GO ... IN` / `SKIP ... IN`. Regression checks now lock down filtered targeted-cursor visibility and EOF-edge behavior while preserving the currently selected SQL alias and record pointer.
- 2026-04-12: synthetic SQL navigation parity now has focused non-selected-target `IN` coverage for `GO`, `SKIP`, and composing `LOCATE`. Regression checks now lock down targeted SQL pointer movement (including EOF-edge positioning) while preserving the currently selected SQL alias and record pointer.
- 2026-04-12: synthetic SQL mutation command-family parity now has focused non-selected-target `IN` coverage for `REPLACE`, `DELETE FOR`, and `RECALL`. Regression checks now lock down selected-cursor preservation (alias/pointer unchanged) while targeted-cursor row updates, tombstone transitions, and pointer restoration behave as expected.
- 2026-04-12: synthetic SQL scan-flow parity now has focused `SCAN ... IN <alias|work area>` coverage. Regression checks now lock down non-selected-target scan behavior so the targeted SQL cursor iterates and lands just past end-of-file while the currently selected SQL cursor alias and pointer remain unchanged.
- 2026-04-12: command-path `SET ORDER` / `SEEK` now has focused `IN <alias|work area>` parity coverage for synthetic SQL cursors. Regression checks now lock down non-selected-target behavior so targeted SQL cursor ordering/seeking updates the requested cursor while preserving the currently selected cursor alias and pointer.
- 2026-04-12: `APPEND BLANK` now has first-pass `IN <alias|work area>` command-path targeting support. The runtime can append into a non-selected synthetic SQL result cursor without switching the current selection, while still advancing the targeted cursor pointer, and focused regression coverage now locks down that behavior alongside follow-on `SET ORDER`/`SEEK` checks.
- 2026-04-12: synthetic SQL mutation parity now has focused indexed-search follow-through coverage. After `APPEND BLANK` + `REPLACE`, command-path `SET ORDER TO <expr>` / `SEEK` now stays pinned to the in-memory SQL row set and can find the appended row values, with regression coverage locking down the post-mutation `RECNO()`/field lookup behavior.
- 2026-04-12: synthetic SQL result cursors now have a writable mutation slice in the native runtime. `REPLACE`, `APPEND BLANK`, `DELETE`, and `RECALL` can mutate the in-memory result rows opened by `SQLEXEC()`, including command-level targeting coverage (`FOR`/`WHILE`), with focused regression coverage across cursor-state and row-value behavior.
- 2026-04-12: `TOTAL` now has remote-cursor compatibility coverage. `TOTAL ... IN <sql-alias>` can aggregate grouped output DBFs from synthetic SQL result rows using the same visibility/scope machinery as local cursors, with focused regression coverage.
- 2026-04-12: selected-alias replacement now has focused parity coverage. Replacing the currently selected alias with `USE ... IN <selected-alias>` is now locked down so the old alias lookup clears, the new alias stays in the same work area, the replacement cursor resets to its first record, and stale active-order state does not leak through that in-place swap.
- 2026-04-12: local indexed-search now consumes a first grounded `FOR`-filter hint on loaded orders. `SEEK` now skips records filtered out by an order's extracted `FOR` expression for the shipped `DELETED() = .F./.T.` cases, with focused regression coverage proving filtered-out keys no longer match and `SET NEAR` positions to the next visible indexed row.
- 2026-04-12: synthetic SQL result cursors now have command-path indexed-search parity in addition to helper-function probes. `SET ORDER TO <expr>` can establish a temporary remote order expression such as `NAME`, and command-level `SEEK` honors that order plus `SET NEAR` miss positioning with focused regression coverage.
- 2026-04-12: synthetic SQL result cursors now have an indexed-search bridge for one-off probes. `SEEK()` and `INDEXSEEK()` can evaluate temporary order expressions such as `NAME` against in-memory SQL result rows, including `INDEXSEEK(.F./.T.)` pointer semantics plus `SET NEAR` miss positioning, with focused regression coverage.
- 2026-04-12: function-path indexed-search parity now accepts one-off order-direction suffixes in order designators. `SEEK()` and `INDEXSEEK()` now parse trailing `ASCENDING` / `DESCENDING` in the order-designator argument and route that override through the same temporary-order seek path used by command-level probes, with focused runtime coverage locking down descending miss positioning via `SEEK(..., ..., 'TAG DESCENDING')`-style calls while preserving the controlling order.
- 2026-04-12: the local indexed-search runtime now consumes grounded normalization hints on the seek path instead of only surfacing them as metadata. `SEEK` and `SEEK()` now case-fold search keys for orders backed by `UPPER(...)` metadata, including descending orders and one-off tag/order probes, with focused regression coverage locking down normalized matches across command and function entry points.
- 2026-04-12: indexed-search parity now includes first-pass order-direction control on the shipped local `SET ORDER`/`SEEK` path. `SET ORDER TO ... [ASCENDING|DESCENDING]` now preserves descending state on the active order, one-off `SEEK ... TAG/ORDER ... [ASCENDING|DESCENDING]` probes can temporarily override direction without changing the controlling order, and focused regression coverage now locks down descending exact-match plus `SET NEAR` miss positioning.
- 2026-04-12: expression-driven cursor targeting now reaches beyond `SELECT` and `USE`. Variable-driven alias/work-area designators now flow through the shipped local `IN`-targeted data/search command family, including `SET ORDER`, `SEEK`, `LOCATE`, `SCAN`, `GO`, `SKIP`, `REPLACE`, `DELETE`, and `RECALL`, with focused regression coverage keeping those commands pinned to the targeted non-selected cursor while preserving the current selection.
- 2026-04-12: `SET DEFAULT TO` now behaves as a data-session-local runtime setting instead of one global value. A fresh `SET DATASESSION` now starts from the startup working directory unless it changes its own default path, restoring the original session also restores its changed default directory, and focused runtime coverage now locks that down with `SET('DEFAULT')` and relative `FILE()` checks across session switches.
- 2026-04-12: continuation guidance is now consolidated around the tracked backlog docs plus the canonical `codex-resume-prompt.txt` used by `scripts/drive-codex.ps1`. Stale throwaway prompt files should no longer be treated as backlog sources once their still-useful notes have been folded back into the repo docs.
- 2026-04-12: the earlier runtime-safety refactor that extracted `prg_engine_helpers.{h,cpp}` and `prg_engine_command_helpers.{h,cpp}` is now part of the shipped baseline, so follow-on runtime work should keep using those seams instead of rebuilding equivalent helper logic inside the `prg_engine.cpp` monolith.
- 2026-04-12: expression-based work-area targeting is now more consistent across adjacent runtime commands. `SELECT <expr>` and `USE ... IN <expr>` now route through the shared cursor-designator expression helper already used by `SET FILTER ... IN`, and focused runtime coverage now locks down numeric-variable and alias-variable targeting for both selection and replacement flows.
- 2026-04-12: `PRIVATE` variable scoping and `STORE` command are now implemented. `PRIVATE` saves the caller's global value before creating a fresh slot, makes that slot visible to callees (since it lives in globals), and restores the saved value when the declaring frame pops via the new `pop_frame()` helper that replaces all `stack.pop_back()` call sites except the one inside `pop_frame()` itself. `STORE <expr> TO var1, var2, ...` evaluates the expression once and assigns to all listed targets. Three focused regression tests lock down PRIVATE masking, PRIVATE callée visibility, and STORE multi-target assignment.
- 2026-04-11: `NDX` key-domain metadata now drives a first narrow runtime compare slice. The runtime preserves `NDX` key-domain hints on loaded orders and now uses numeric-domain ordering for `SEEK`/`SET NEAR` behavior when that grounded header metadata is available, while leaving broader `CDX/DCX/IDX` collation semantics untouched.
- 2026-04-11: single-index probe metadata now goes one step beyond expression-derived hints. `IDX` and `NDX` probes surface opaque header sort-marker hints from already-read header bytes, and `NDX` probes now expose a key-domain hint derived from the numeric/date header flag, while intentionally avoiding invented named-collation mappings or runtime compare changes.
- 2026-04-11: indexed metadata now carries first-pass normalization/collation hints from the shared probe layer into runtime order state. Expression-derived hints such as `UPPER(...)` now flow through `SET ORDER`, cursor snapshot/restore, and temporary `SEEK ... TAG` overrides, and focused runtime coverage now verifies those hints through `runtime.order` and `runtime.seek` event detail without changing indexed compare behavior yet.
- 2026-04-11: read-only real-fixture validation now reaches beyond the original `customer.cdx` smoke test. Optional coverage now exercises additional installed VFP `CDX` and `DCX` samples plus the local `CHNGREAS.NDX` fixture so the shared index probe has broader real-world sanity checks without requiring checked-in binaries.
- 2026-04-11: `CDX/DCX` inspection now preserves conservative per-tag page hints from directory leaf entries and prefers those hinted page neighborhoods when binding first-pass key/`FOR` expressions. Focused adversarial coverage now proves earlier stray printable expressions no longer steal direct-probe tag metadata.
- 2026-04-11: `MDX` inspection is now less heuristic-heavy. The read-only probe now enumerates first-pass tag hints from non-header 512-byte metadata regions instead of whole-file scavenging, rejects obviously implausible all-zero headers or hint-free files, and exercises that tighter behavior through both direct probe and companion inspection coverage.
- 2026-04-11: structured asset validation now extends into DBF field descriptors and record layout. Readable DBF-family assets can now report missing descriptor terminators, malformed descriptor spans, overlapping or overflowing field layouts, record-length mismatches, and duplicate or invalid field names without failing inspection or blocking Studio document open.
- 2026-04-11: structured asset validation now extends into memo sidecars. Readable DBF-family assets can now report malformed memo-sidecar headers, invalid block-size metadata, out-of-range memo block pointers, and truncated referenced memo payloads without failing inspection or blocking Studio document open.
- 2026-04-11: the native asset inspector now carries structured validation findings without failing otherwise-readable DBF-family assets. `inspect_asset()` can now report header/file-size inconsistencies, truncated record storage, missing memo or structural companion files, and malformed companion index files through additive validation metadata that also flows into the Studio document model.
- 2026-04-10: the native PRG expression/runtime slice now covers first-pass `EVAL()`, `SET()`, and `&macro` substitution plus expression-driven `USE IN` close targeting. Stored expression strings can now run through the evaluator against the current record/runtime context, macro-expanded field/alias names work in expression paths, `SET()` can inspect current session/default-directory state, and focused runtime coverage now locks down restored `SET` state across data sessions.
- 2026-04-10: native PRG command coverage now includes a first-pass literal `TEXT/ENDTEXT` slice. The parser/runtime can capture multiline text blocks into variables with `TEXT TO ... [ADDITIVE] [NOSHOW]`, preserves literal block lines without stripping FoxPro comment markers inside the block, and locks that down with focused runtime regression coverage.
- 2026-04-09: `CDX/DCX` inspection now uses first-pass real directory leaf-page parsing for tag names instead of whole-file tag-name scavenging. The asset parser now lifts stored tag names from plausible compound-index directory pages, attaches first-pass page-local key and `FOR` expression hints to those tags, and validates the shape against both synthetic fixtures and the real VFP `customer.cdx` sample.
- 2026-04-09: the shared `CDX/DCX` parser path now has focused `.dcx` regression coverage. Synthetic direct-probe coverage now locks down `DCX` tag, key-expression, and `FOR`-expression extraction, and `inspect_asset()` now has a companion-collection regression for `DBC` plus same-base `.dcx`.
- 2026-04-09: `MDX` probing now moves past file-level structural recognition. The read-only probe now surfaces first-pass tag-name hints from printable metadata runs, and same-base companion inspection exercises those hints through `inspect_asset()` while deeper layout parsing remains open.
- 2026-04-09: `APPEND BLANK` now fails fast on unsupported field layouts instead of guessing blank bytes for binary storage it does not understand. The shared DBF layer now rejects tables carrying unsupported field types during blank-record initialization, focused DBF/runtime coverage locks down the unchanged-file error path, and that closes another silent-corruption seam while broader field-type fidelity remains open.
- 2026-04-09: the shared DBF mutation layer now covers first-pass FoxPro `Currency (Y)` and `DateTime (T)` field writes. `create_dbf_table_file()`, `replace_record_field_value()`, and `append_blank_record_to_file()` now persist `Y` values as signed 4-decimal scaled integers, persist `T` values through the existing `julian:<day> millis:<milliseconds>` storage contract, initialize blank appended fixed-width values to zero bytes, and round-trip that behavior through focused DBF coverage.
- 2026-04-09: the shared DBF mutation layer now covers first-pass FoxPro `Integer (I)` field writes. `create_dbf_table_file()`, `replace_record_field_value()`, and `append_blank_record_to_file()` can now persist `I` fields as 4-byte little-endian signed values, initialize blank appended integers to zero, and round-trip that behavior through focused DBF coverage.
- 2026-04-09: the shared DBF mutation layer now fails fast on indexed tables instead of silently diverging from companion structural indexes. `append_blank_record_to_file()`, `replace_record_field_value()`, and `set_record_deleted_flag()` now reject DBFs marked with production indexes or carrying same-base `.cdx` companions, and focused DBF/runtime coverage locks down the error path while true `CDX` write/rebuild support remains open.
- 2026-04-09: the shared DBF layer now has first-pass memo write fidelity for `M` fields. `create_dbf_table_file()` and `replace_record_field_value()` can now persist FoxPro-compatible memo sidecars, blank memo fields append with zero pointers instead of corrupted placeholders, and focused DBF round-trip coverage now exercises memo-backed create/update/append flows.
- 2026-04-09: `SET NEAR` now has focused data-session restoration coverage. Missed-`SEEK` behavior is locked down across `SET DATASESSION` switches so session-local nearest-record behavior stays independent and restores correctly after returning to the original session.
- 2026-04-10: synthetic SQL result cursor auto-allocation now follows each data session's current selected work-area flow instead of always forcing another `IN 0` allocation. Focused runtime coverage now locks down `SELECT 0` plus `SQLEXEC()` behavior across two data sessions so each session reuses its own selected empty work area and restores that placement after switching back.
- 2026-04-10: work-area allocation now reuses freed areas instead of monotonically climbing forever, and `SELECT(0)` now reports the next available work area without consuming it. Focused runtime coverage locks down `USE IN`, `SELECT(0)`, and subsequent `USE ... IN 0` reopening so alias/work-area selection stays closer to practical VFP behavior after closing a cursor.
- 2026-04-10: synthetic SQL result cursors now participate in a first-pass read-only local-cursor flow. The runtime can evaluate synthetic SQL fields during `SET FILTER`, `GO TOP`, `LOCATE`, aggregate built-ins, and `CALCULATE`, and focused regression coverage now locks down that filter/locate/aggregate behavior against `SQLEXEC()` result cursors.
- 2026-04-09: `SQLCONNECT()` handle numbering now restarts per data session instead of sharing one global handle sequence. Combined with the prior session-scoped SQL handle storage, this keeps synthetic SQL connection lifecycles closer to the local cursor/session model and adds focused regression coverage for cross-session handle lookup and restored SQL cursor visibility.
- 2026-04-09: synthetic SQL result cursors now have stronger data-session isolation. SQL connection handles are now scoped per data session instead of globally, and focused runtime coverage now locks down cross-session `SQLEXEC`/`SQLDISCONNECT` isolation plus restored SQL cursor lookup after `SET DATASESSION` switches.
- 2026-04-08: plain `USE <table>` now reuses the current selected work area instead of behaving like `IN 0`, so opening into an empty selected area and replacing the selected cursor now happen in place with focused runtime regression coverage.
- 2026-04-08: `USE ... IN <alias|work area>` now preserves the current selected work area when replacing a different non-selected local cursor, and focused runtime coverage now locks down both replacement and close flows for that alias/work-area edge case.
- 2026-04-08: aggregate command follow-through now includes first-pass `IN <alias|work area>` targeting for command-level `COUNT`, `SUM`, and `AVERAGE`, and `TOTAL` can now target a non-selected local DBF cursor the same way. The runtime preserves both the selected cursor position and the targeted cursor position with focused regression coverage.
- 2026-04-08: first-pass `TOTAL` now works for local DBF-backed cursors. The native PRG runtime can write grouped DBF totals with `TOTAL TO ... ON ... FIELDS ...` plus basic visibility-aware `REST`/`FOR` semantics, and the DBF layer now has focused coverage for creating output tables without corrupting FoxPro-compatible files.
- 2026-04-08: aggregate command follow-through now covers first-pass scope-clause and `WHILE` semantics for command-level `COUNT`, `SUM`, and `AVERAGE`. `ALL`, `REST`, `NEXT n`, and `RECORD n` now run through the native visibility-aware cursor engine, preserve the selected record position, and have focused regression coverage.
- 2026-04-08: command-level aggregate follow-through now exists in the native PRG runtime. First-pass `COUNT`, `SUM`, and `AVERAGE` commands can assign visibility-aware results into variables with `FOR`/`TO`, the expression parser now handles multiplicative arithmetic needed by multi-expression aggregates, and quoted `IN` alias targets now resolve cleanly across adjacent aggregate/search commands such as `USE`, `CALCULATE`, `LOCATE`, and `SCAN`.
- 2026-04-08: first-pass command-level aggregates now exist in the native PRG runtime. `CALCULATE` can assign `COUNT()`, `SUM()`, `AVG()`, `MIN()`, and `MAX()` results into variables, supports `TO`/`INTO`, optional `FOR`, optional `IN` alias targeting, and reuses the shipped visibility-aware aggregate semantics with focused regression coverage.
- 2026-04-08: first-pass aggregate built-ins now exist in the native PRG runtime for local DBF-backed cursors. `COUNT()`, `SUM()`, `AVG()/AVERAGE()`, `MIN()`, and `MAX()` now evaluate across visible rows, respect `SET FILTER TO/OFF`, honor `SET DELETED ON`, and can target a named alias with focused regression coverage.
- 2026-04-08: native PRG branching now covers first-pass `DO CASE/CASE/OTHERWISE/ENDCASE`, including nested case blocks and case-driven execution inside `SCAN`, with focused runtime regression coverage.
- 2026-04-08: native PRG control flow now covers first-pass `DO WHILE/ENDDO` plus shared `LOOP`/`CONTINUE`/`EXIT` behavior across `DO WHILE`, `FOR`, and `SCAN`, with focused regression coverage for nested loops and cursor-backed iteration.
- 2026-04-08: persistent cursor filtering now has a first native runtime slice. `SET FILTER TO/OFF` is now cursor-scoped for local DBF-backed work areas, and `GO TOP/BOTTOM`, `SKIP`, `LOCATE`, `SCAN`, and `DELETE/RECALL FOR` now honor active filter visibility with regression coverage.
- 2026-04-08: local-table mutation/query flow moved forward materially in the native PRG engine. `LOCATE`, `SCAN ... ENDSCAN`, `REPLACE`, `APPEND BLANK`, `DELETE`, and `RECALL` now work against local DBF-backed cursors with persisted table writes, current-record field resolution, and regression coverage in both the DBF layer and runtime layer.
- 2026-04-08: `CDX/DCX` inspection moved beyond header-only probing. Copperfin now enumerates first-pass tag/expression hints from real file contents and surfaces them through the asset inspector and CLI.
- 2026-04-08: `GO`, `GOTO`, and `SKIP` cursor navigation now have runtime coverage in the native `PRG` engine tests, which closes one more piece of work-area behavior before broader table/session semantics.
- 2026-04-08: `USED()`, `DBF()`, and `FCOUNT()` now participate in the native cursor/session model for both local tables and synthetic SQL result cursors, with explicit regression coverage for data-session isolation and `USE IN`.
- 2026-04-08: `SELECT` now fails cleanly on missing aliases instead of inventing new work areas, and `USE AGAIN` / duplicate-open rules now participate in the native runtime with regression coverage.
- 2026-04-08: `SET ORDER TO TAG`, `SEEK`, and `FOUND()` now work as a first indexed-lookup slice for local DBF-backed cursors, using companion index metadata and explicit runtime regression coverage.
- 2026-04-10: command-level `SEEK` now accepts one-off `TAG` / `ORDER` overrides for local DBF-backed cursors without requiring a prior `SET ORDER`, which closes a focused indexed-search parity gap and keeps the active controlling order unchanged after the probe.
- 2026-04-10: indexed runtime metadata now preserves the actual inspected index file identity for loaded local-table orders. `ORDER(alias, 1)` now reports the real companion index path instead of assuming `.cdx`, `TAG(indexFile, ...)` now accepts supported xBase index extensions beyond `.cdx`, and focused runtime coverage locks that down with a synthetic `.idx` companion.
- 2026-04-08: the official Learn language-reference index is now part of the working parity process, with a generated command inventory plus `FOXHELP.DBF` as a fallback source for older or missing command-behavior details.
- 2026-04-08: the installed VFP CHM files now have a generated local keyword-to-topic index, so command behavior can be mined offline from `dv_foxhelp.chm` and `foxtools.chm` without treating the CHMs as opaque binaries.
- 2026-04-08: `SET NEAR ON/OFF` now participates in indexed `SEEK` behavior for local DBF-backed cursors, which closes another documented VFP search-behavior slice and gives the runtime a clearer path beyond exact-match-only index probes.
- 2026-04-08: `SEEK()`, `INDEXSEEK()`, `ORDER()`, and `TAG()` now have first-pass runtime coverage grounded in the installed VFP help topics, which expands the indexed-search surface from command-only behavior into documented helper functions.
- 2026-04-08: `SET LIBRARY TO`, `FoxToolVer()`, `MainHwnd()`, `RegFn32()`, and `CallFn()` now have a first-pass FoxTools compatibility bridge, giving Copperfin an initial modeled path for old DLL-call workflows without destabilizing the host runtime.
- 2026-04-08: `SET EXACT` now participates in both string comparison semantics and indexed `SEEK` behavior, and that setting is now scoped per data session instead of being treated as one global runtime toggle.

### A1. File And Index Fidelity

- Finish first-class read/write fidelity for `DBF`, `FPT`, `CDX`, `DCX`, `IDX`, `NDX`, `MDX`, and `DBC`.
- Add broader validation and repair logic for damaged or inconsistent xBase assets.
- Strengthen round-trip confidence on real-world FoxPro and dBase datasets.
- Expand metadata coverage for edge-case field types, memo storage, and index expressions.

### A2. Work Areas, Sessions, And Cursor Semantics

- Finish the remaining alias scoping and work-area selection behavior to match VFP expectations.
- Deepen data-session isolation, switching, restoration, and nested behavior.
- Extend indexed lookup beyond the first-pass `SET ORDER TO TAG` / `SEEK` / `FOUND()` slice into richer order, collation, and search behavior.
- Support remote cursors and result cursors with behavior closer to SQL pass-through in VFP.

### A3. Command And Expression Surface

- Expand the native execution engine from `PRG-first` into a broader FoxPro/VFP command surface.
- Keep closing gaps in command semantics, expression evaluation, macro/eval behavior, and runtime state changes.
- Build a compatibility corpus from the installed VFP tree, `E:\VFPSource`, your legacy projects, and regression samples.

### A4. Automation And Interop Semantics

- Deepen OLE/COM behavior beyond current object/property/method handling.
- Improve `CREATEOBJECT()` and `GETOBJECT()` parity.
- Preserve host stability when automation calls fail or behave unexpectedly.

## Phase B: Runtime Safety, Fault Tolerance, And Crash Containment

This layer protects real users working with broken legacy code.

### B1. Fault Isolation

- Keep runtime faults non-fatal to the host wherever possible.
- Pause execution at the offending statement with useful debugger context.
- Preserve runtime/session state well enough for diagnosis after failure.

### B2. Debug Metadata And Diagnostics

- Improve stack traces, statement context, variable inspection, and event traces.
- Capture richer runtime diagnostics for xAsset execution, report preview, menu flow, and data operations.
- Make error messages and pause reasons feel like a serious developer tool, not a prototype host.

## Phase C: Asset Runtime Parity

This is where Copperfin stops being only an engine and starts being a FoxPro application platform.

### C1. Forms And Classes

- First-pass runtime parity is now in place through executable xAsset bootstrap generation, extracted method dispatch, and root/data-environment lifecycle sequencing.
- Follow-on work should deepen container/object semantics and visual/runtime fidelity instead of reopening the now-shipped bootstrap surface.
- Designer and IDE work can build on this runtime path rather than carrying separate form/class execution heuristics.

### C2. Reports And Labels

- First-pass runtime parity is now in place for `FRX/FRT` and `LBX/LBT` preview plus `TO FILE` execution.
- Follow-on work should deepen layout fidelity, expression evaluation, export breadth, and report-object behavior on top of the shipped runtime lane.
- Treat the render/export pipeline as an area for refinement, not as a blocker for basic runtime coverage anymore.

### C3. Menus

- First-pass `MNX/MNT` runtime parity is now in place for startup, activation, action dispatch, submenu activation wrapping, and cleanup bootstrap flow.
- Follow-on work should deepen popup/menu navigation, routing, state handling, and event semantics without regressing the shipped executable bootstrap surface.

### C4. Projects

- Strengthen `PJX/PJT` project interpretation and startup/build behavior.
- Reduce heuristic-driven planning and move toward a fuller project execution model.

## Phase D: Build, Compiler, And Debug Pipeline Completion

This phase should build directly on the engine/runtime work, not the other way around.

### D1. Compiler And Package Model

- The native manifest/package/debug model is now shipped and should be treated as the baseline contract.
- Follow-on work should deepen compiler fidelity, executable generation, and `.NET`-friendly outputs without regressing the working package/run/debug lane.
- Replace remaining heuristics only where a stronger compiler/runtime contract clearly reduces risk.

### D2. Debugger Completion

- First-pass debugger coverage is now in place through the runtime host, stepping model, breakpoint support, pause-state reporting, and xAsset action dispatch.
- Follow-on work should add richer watch tooling, coverage surfaces, and tighter shell integration.
- Improve linkage between debugger state and source/design surfaces as a refinement pass rather than a blocker.

### D3. Build/Run/Deploy Workflow

- Build/run/debug orchestration is now shipped through the native hosts, generated launcher flow, and runtime package materialization.
- Follow-on work should improve deployment/runtime redistribution and migration-aware reporting instead of reopening the now-working workflow baseline.
- Keep this lane stable so later compiler/interoperability work can build on it.

## Phase E: Shared Design Model And Visual Designer Fidelity

The design model should be completed before polishing host-specific shells.

### E1. Shared Design Model

- Finish high-fidelity normalized models for forms, classes, reports, labels, menus, and projects.
- Improve memo-heavy asset decoding and round-trip preservation.
- Make the shared design model strong enough that both Visual Studio and the standalone IDE can rely on the same core.

### E2. Designer Interaction

- Add toolbox-driven creation and editing of controls and designer objects.
- Improve drag/drop, resize, alignment, grouping, container editing, and property-grid fidelity.
- Add builders, wizards, and context-aware editors where FoxPro developers expect them.

### E3. Report And Label Designer Completion

- Finish section-aware editing so it behaves like a real modern report designer.
- Keep the visual direction modern, but preserve VFP workflow expectations.

## Phase F: IDE Parity On Top Of The Shared Core

This phase should consume the completed runtime and design model rather than inventing substitutes.

### F1. Visual Studio Extension

- The Visual Studio extension is now a shipped first-pass surface with asset editors, host bridges, project insights, and FoxPro IntelliSense/navigation.
- Follow-on work should deepen designer fidelity and utility-pane richness instead of treating the extension as missing.
- Tighten build/run/debug ergonomics as refinement work, not as a blocker for baseline IDE parity.

### F2. Standalone Copperfin IDE

- The standalone Studio shell is now a shipped first-pass IDE surface with tabbed document hosting and shared native-host integration.
- Follow-on work should deepen workflow coverage and polish until it fully matches the strongest daily-driver scenarios.
- Keep it converged with the shared core rather than forking behavior away from the Visual Studio path.

## Phase G: Language Service Deepening

The baseline language-service surface is now shipped. What remains here is depth, breadth, and stronger semantics on top of that baseline.

### G1. Editor Semantics

- The shipped language-service baseline now covers completion, quick info, signature help, and project-symbol definition resolution.
- Follow-on work should improve symbol resolution across includes, defines, dotted/member contexts, and project boundaries.
- Expand completions using project symbols, open cursors, DBC metadata, and object members without regressing the current baseline.

### G2. Navigation And Refactoring

- The shipped baseline includes first-pass definition resolution and project browsing.
- Follow-on work should add richer peek/reference workflows and safe rename/refactor behavior.
- Build more semantic editor behavior inspired by external xBase tooling patterns without forcing Copperfin into another runtime model.

### G3. IntelliSense Inputs

- The current IntelliSense catalog is already shipping and should now be treated as an extensible baseline.
- Follow-on work should investigate FOXCODE-style or metadata-driven enrichment for richer completion and hints.
- Incorporate relevant ideas from `FoxcodePlus`, `GoToDefinition`, `foxref`, and `GoFish` where they strengthen the existing baseline.

## Phase H: Database Federation And Modern App Platform

This sits above the core engine. It should not destabilize parity work underneath.

### H1. Relational Backends

- Finish deterministic translation and provider behavior for SQLite, PostgreSQL, SQL Server, and Oracle.
- Make FoxPro-style data access work naturally against those backends where possible.

### H2. Document And Vector Backends

- Build the backend translation layer for JSON/document/vector stores.
- Define where deterministic translation is sufficient and where AI-assisted planning is optional.
- Keep AI assistance policy-controlled and user-selectable rather than mandatory.

### H3. Modern Integration Surface

- Improve `.NET` output and consumption paths.
- Add real MCP/AI integration hooks in the product, not only in docs and shells.
- Add practical Python/R sidecar workflows for data science scenarios without making them part of the trusted core.

## Phase I: Security Completion

Security should keep growing throughout the project, but these remaining items deserve their own completion pass.

### I1. Runtime And Project Security

- Deepen native RBAC, policy enforcement, secrets handling, and audit behavior across the runtime and project model.
- Ensure generated applications can opt into native security controls cleanly.

### I2. Extension And Host Security

- Lock down managed-extension loading, interop boundaries, and external process policy.
- Make AI/MCP integration policy-aware and enterprise-friendly.

## Phase J: Portability

Do not start broad host-porting work until the Windows-first stack is solid.

### J1. Portable Core

- Keep core runtime, data engine, parser, and connector abstractions portable by boundary.
- Isolate Windows-only functionality behind shell, printing, OLE, COM, and CLR-host seams.

### J2. macOS

- Port the standalone IDE and portable core.
- Do not assume Visual Studio extension portability because VS 2022+ is Windows-only.
- If a future macOS code editor integration is needed, treat it as a separate host strategy.

### J3. Linux

- Port the standalone IDE and portable core for Red Hat/Fedora and Debian-family targets first.
- Do not couple Linux progress to Visual Studio-specific assumptions.

## External Tooling Incorporation Plan (Language Service And IDE)

External xBase-family tooling may save substantial tooling time, but it should be used selectively.

### Use External Tooling As A Model For

- language-service structure
- project-system layering
- IntelliSense architecture
- signature help and navigation patterns
- debugger/tool-window organization
- modern xBase-family tooling strategy inside Visual Studio

### Do Not Blindly Import From External Tooling

- runtime semantics
- compiler behavior
- language differences where external tooling diverges from VFP
- assumptions that would distort FoxPro/VFP compatibility

### Immediate Language-Service Follow-Up

- audit mature external language-service and project-system layers for concepts we still lack
- identify which pieces can be mirrored clean-room in Copperfin
- prefer architecture and workflow ideas over direct feature cloning

## Community Workflow Tooling Incorporation Plan

Community-maintained FoxPro tooling should continue to inform parity and workflows.

### Highest-Value Community Inputs

- `GoToDefinition`
- `FoxcodePlus`
- `Project Explorer`
- `DataExplorer`
- `Toolbox`
- `Code References`
- `GoFish`
- `FoxBin2Prg`
- `PEM Editor`
- `Thor`
- `Automated Test Harness`
- `FoxUnit`
- `DeployFox`

### Local Source Trees To Continue Mining

- `E:\VFPSource\foxref`
- `E:\VFPSource\DataExplorer`
- `E:\VFPSource\toolbox`
- `E:\VFPSource\tasklist`
- `E:\VFPSource\taskpane`
- `E:\VFPSource\coverage`
- `E:\VFPSource\obrowser`
- `E:\VFPSource\builders`
- `E:\VFPSource\ReportBuilder`

## Agent Guidance

When spawning agents, assign them work that respects the dependency order above.

Good agent tasks:

- deepen one runtime subsystem without touching host UX
- improve one data/index format implementation
- improve one designer family on top of an already-stable model
- strengthen one language-service feature against the shared project index
- extract lessons from one external/community subsystem into a clean-room implementation note

Bad agent tasks:

- polishing shell UX before the shared model is ready
- building cross-platform host layers before Windows parity is credible
- inventing ad hoc designer logic outside the shared design model
- adding AI workflows into trusted runtime code paths

## Definition Of Done For The Windows-First Product

The Windows-first product is not done until:

- representative VFP applications can be opened, edited, built, run, and debugged
- runtime crashes are contained and diagnosable
- designers are truly useful, not just inspectable
- Visual Studio and standalone Copperfin Studio are both credible daily-driver environments
- native security is optional but real
- .NET interoperability is practical and documented
- modern database backends work without sacrificing core FoxPro compatibility

At that point, portability becomes a product-expansion effort instead of a moving target.
