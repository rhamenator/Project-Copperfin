# Phase A Dependency Breakdown

This document expands Phase A of [remaining-work.md](/home/rich/dev/Project-Copperfin/remaining-work.md:306) into a deeper dependency model so the next implementation slices can be chosen in the right order.

It is intentionally narrower than the top-level roadmap:

- scope: Phase A only
- granularity: command/function groups and runtime engine seams
- purpose: decide what to finish first, what can run in parallel, and what is actually on the critical path

## Reading Notes

- Top-level percentages for Phase A areas come directly from [remaining-work.md](/home/rich/dev/Project-Copperfin/remaining-work.md:291).
- Group-level percentages below are inferred planning estimates based on the current backlog text, recent progress notes, and shipped test coverage. They are not yet first-class roadmap metrics.
- The dependency edges are pragmatic engineering dependencies, not strict architectural laws. They answer "what should be finished first if the goal is fastest safe progress toward VFP parity?"
- The CPM section uses longest-path logic over a directed acyclic prerequisite graph. Dijkstra's algorithm is not needed here because this is a precedence-scheduling problem rather than a shortest-path routing problem.

## Phase A Areas

| Phase A Area | Top-Level Progress | Main Risk Still Open |
| --- | --- | --- |
| A1. File and index fidelity | 92-95% | repair breadth, edge-case metadata, and runtime consumption gaps |
| A2. Work areas, sessions, and cursor semantics | 90-96% | richer order/collation/search behavior and remote/result-cursor parity |
| A3. Command and expression surface | 82% | remaining issue `#7` / `#8` command-surface and macro/eval fidelity |
| A4. Automation and interop semantics | ~41% | OLE/COM parity depth and host-failure containment under `#10`, `#11`, and `#12` |

## Command/Function Group Dependency Table

| ID | Group | Linked Issues | Est. Progress | Representative Commands / Functions | Depends On | Primarily Unblocks | Why It Matters Now |
| --- | --- | --- | --- | --- | --- | --- | --- |
| G1 | DBF/FPT parse, validation, and repair | supports `#7` | 90% | DBF/FPT readers, memo decoding, structured asset validation | none | G2, G3, G10, G11 | This is the storage truth layer; if it is wrong, everything above it is noisy. |
| G2 | Index probe fidelity and runtime metadata | supports `#7` | 91% | `CDX`, `DCX`, `IDX`, `NDX`, `MDX`, tag key/`FOR` extraction, normalization hints | G1 | G6, G7, G12, G13 | Search/order parity depends on accurate key expressions, direction, and collation hints. |
| G3 | DBC/DCT/DCX container fidelity | supports `#7` | 88% | DBC object extraction, companion discovery, catalog previews | G1, G2 | later project/runtime surfaces, richer metadata inputs | Near completion; cheap to finish and reduces later heuristics. |
| G4 | Work-area selection and alias targeting | `#7` | 96% | `SELECT`, `USE`, `USE AGAIN`, `USE IN`, `SELECT(0)`, expression-driven `IN` targets | G1 | G5, G6, G9, G10, G11, G12, G13, G14 | Almost every runtime command family assumes this is stable. |
| G5 | Data-session isolation and session-local state | `#7`, `#8` | 95% | `SET DATASESSION`, `SET DEFAULT TO`, session-local `SET()` state, session-local SQL handles | G4 | G8, G13, G14, G15 | Host/runtime parity collapses quickly if session boundaries leak. |
| G6 | Order, seek, and collation semantics | `#7` | 90% | `SET ORDER`, `SEEK`, `SEEK()`, `INDEXSEEK()`, `FOUND()`, `SET NEAR`, tag-expression evaluation | G2, G4 | G10, G11, G12, G13 | This is the main seam between index metadata and FoxPro-visible cursor behavior. |
| G7 | SQL pass-through handle and metadata API | `#7` | 90% | `SQLCONNECT`, `SQLSTRINGCONNECT`, `SQLEXEC`, `SQLPREPARE`, `SQLGETPROP`, `SQLSETPROP`, `SQLTABLES`, `SQLCOLUMNS`, `SQLDATABASES`, `SQLPRIMARYKEYS`, `SQLFOREIGNKEYS`, `SQLCOMMIT`, `SQLROLLBACK`, `SQLCANCEL` | G4, G5 | G8 | The connection/session layer is already strong enough that the remaining work should stay incremental, not disruptive. |
| G8 | Remote and result-cursor semantics | `#7` | 88% | remote cursor navigation, filtering, ordering, `APPEND BLANK`, `REPLACE`, `DELETE`, `RECALL`, targeted `IN` behavior on SQL result cursors | G4, G5, G6, G7 | Phase A closure for A2, later federation/runtime parity | This is where local cursor behavior and SQL pass-through meet. |
| G9 | Macro/eval core and expression compatibility | `#8` | 82% | `EVAL()`, `SET()`, `&macro`, `TEXTMERGE()`, `EXECSCRIPT()`, `TYPE()`, `TRANSFORM()`, macro indirection, macro-expanded identifiers | G4, G5 | G10, G11, G12, G13, G14, G15 | This remains one of the highest leverage remaining lanes for issue `#8`. |
| G10 | Local record navigation and mutation | `#7`, `#8` | 91% | `GO`, `SKIP`, `LOCATE`, `SCAN`, `REPLACE`, `APPEND BLANK`, `DELETE`, `RECALL`, `UNLOCK RECORD` | G1, G4, G6, G9 | G11, G12, G13 | Strong already, but still a shared prerequisite for deeper parity slices above it. |
| G11 | Structural table and import/export operations | `#7`, `#8` | 90% | `CREATE TABLE`, `ALTER TABLE`, `PACK`, `PACK MEMO`, `ZAP`, `APPEND FROM`, `COPY TO`, journaling-backed mutation paths | G1, G4, G9, G10 | corpus confidence, storage parity, data-migration flows | This is the last broad correctness seam in local DBF mutation workflows. |
| G12 | Field projection and data-transfer surface | `#7`, `#8` | 89% | `SCATTER`, `GATHER`, `SET FIELDS`, `BROWSE`, `EDIT`, `CHANGE`, `COPY TO ARRAY`, `APPEND FROM ARRAY`, `FIELDS LIKE/EXCEPT` | G4, G6, G9, G10 | G13, issue `#7` closure | Recently deepened; remaining work here should be narrow and correctness-driven. |
| G13 | Aggregate, lookup, and record-view helpers | `#7` | 90% | `COUNT`, `SUM`, `AVG`, `MIN`, `MAX`, `CALCULATE`, `TOTAL`, `LOOKUP`, `DISPLAY`, `LIST` | G4, G6, G10, G12 | host/report/runtime parity, diagnostics, issue `#7` closure | Mostly close; good candidate for quick closure after upstream blockers are done. |
| G14 | Headless interaction and dialog command surface | `#7`, `#8` | 84% | `WAIT`, `KEYBOARD`, `INPUT`, `ACCEPT`, `GETFILE`, `PUTFILE`, `GETDIR`, `INPUTBOX`, runtime event payloads | G4, G5, G9, G12 | host integration parity, remaining command-surface work | Already much better; remaining work should focus on edge-case clauses and macro fidelity. |
| G15 | Memory-variable and assignment semantics | `#8`, `#11` | 86% | `PUBLIC`, `PRIVATE`, `RELEASE`, `STORE`, array macro names, `DISPLAY/LIST MEMORY` | G4, G5, G9 | G12, G13, G14, A4 work | This is one of the hidden foundations under command-surface parity. |
| G16 | Compatibility corpus and regression harness | supports `#7`, `#8`, `#10`, `#11`, `#12` | 55% | VFP-tree corpus, legacy samples, regression fixtures, focused runtime test expansion | G1, G4 | all groups | This is not a runtime feature, but it is one of the best multipliers for finishing the remaining 10-20% safely. |
| G17 | Automation object activation parity | `#10`, `#11` | 35% | `CREATEOBJECT()`, `GETOBJECT()` | G5, G9, G15 | G18, later interop/runtime parity | This is the visible front door for A4 and still shallow. |
| G18 | Automation property/method behavior and containment | `#10`, `#12` | 28% | OLE/COM property access, method invocation, automation-failure isolation | G17 | Phase A closure, later host safety work | This is the deepest unfinished Phase A lane and should be sequenced after command/macro foundations are steadier. |

## Dependency Table By Recommended Work Package

This is the actionable version of the graph. It groups the above command/function clusters into deliverable slices.

| WP | Work Package | Linked Issues | Duration (Weeks) | Primary Groups | Prerequisites | Expected Output |
| --- | --- | --- | --- | --- | --- | --- |
| WP0 | Corpus and regression harness expansion | `#7`, `#8`, `#10`, `#11`, `#12` | 1 | G16 | none | broader fixture map, missing-edge inventory, faster slice validation |
| WP1 | DBF/FPT validation and repair completion | mostly `#7` | 2 | G1 | WP0 | finish repair/validation gaps in core storage path |
| WP2 | Index fidelity completion | mostly `#7` | 2 | G2 | WP1 | close remaining tag-expression, collation, and index-metadata/runtime seams |
| WP3 | DBC container completion | mostly `#7` | 1 | G3 | WP1, WP2 | close container/object-extraction and companion-resolution gaps |
| WP4 | Work-area and session residual cleanup | `#7`, `#8` | 1 | G4, G5 | WP0 | remove remaining targeting/session edge cases before higher-surface work |
| WP5 | Order/collation/search residuals | `#7` | 2 | G6 | WP2, WP4 | finish the remaining seek/order/collation behaviors that many commands inherit |
| WP6 | SQL handle/API residuals | `#7` | 2 | G7 | WP4 | finish stable connection/property/catalog semantics without disturbing cursor behavior |
| WP7 | Remote/result-cursor semantic closure | `#7` | 2 | G8 | WP5, WP6 | align SQL result-cursor behavior more closely with local cursor semantics |
| WP8 | Macro/eval/runtime-state closure | `#8` | 2 | G9, G15 | WP4 | drive issue `#8` down before more command-surface polishing |
| WP9 | Field-projection and data-transfer closure | `#7`, `#8` | 1 | G12 | WP5, WP8, WP10 | finish narrow `SCATTER`/`GATHER`/`FIELDS`/array transfer seams |
| WP10 | Local structural table-operation closure | `#7`, `#8` | 2 | G10, G11 | WP1, WP4, WP8 | finish the remaining local mutation/import/export correctness seams |
| WP11 | Query/aggregate and helper closure | `#7` | 2 | G10, G13 | WP5, WP10 | finish aggregate/view/helper behavior on top of stable cursor semantics |
| WP12 | Headless interaction/display closure | `#7`, `#8` | 1 | G13, G14 | WP8, WP9 | finish host-visible command/event fidelity after macro and field semantics settle |
| WP13 | Automation activation parity | `#10`, `#11` | 2 | G17 | WP8, WP12 | deepen `CREATEOBJECT()` / `GETOBJECT()` behavior on a steadier runtime base |
| WP14 | Automation containment and fault behavior | `#10`, `#12` | 1 | G18 | WP13 | finish A4 host-safety semantics around automation failure paths |

## Dependency Graph

```mermaid
flowchart LR
    classDef green fill:#d1fae5,stroke:#065f46,stroke-width:2px,color:#064e3b;
    classDef amber fill:#fef3c7,stroke:#92400e,stroke-width:2px,color:#78350f;
    classDef red fill:#fee2e2,stroke:#991b1b,stroke-width:2px,color:#7f1d1d;
    classDef critical fill:#fee2e2,stroke:#b91c1c,stroke-width:3px,color:#7f1d1d;
    classDef lane fill:#eef2ff,stroke:#4338ca,stroke-width:1px,color:#1e1b4b;

    subgraph A1[Phase A1 - File And Index Fidelity]
        direction TB
        G1["G1 DBF/FPT Parse + Repair<br/>90%<br/>supports #7"]
        G2["G2 Index Probe + Metadata<br/>91%<br/>supports #7"]
        G3["G3 DBC/DCT/DCX Fidelity<br/>88%<br/>supports #7 via #96"]
    end

    subgraph A2[Phase A2 - Work Areas / Sessions / Cursor Semantics]
        direction TB
        G4["G4 Work-Area Targeting<br/>96%<br/>#7"]
        G5["G5 Data Sessions + SET State<br/>95%<br/>#7 / #8"]
        G6["G6 Order / Seek / Collation<br/>90%<br/>#92"]
        G7["G7 SQL Handle/API Surface<br/>90%<br/>#7"]
        G8["G8 Remote Cursor Semantics<br/>88%<br/>#93"]
    end

    subgraph A3[Phase A3 - Command / Expression Surface]
        direction TB
        G9["G9 Macro / Eval Core<br/>82%<br/>#97"]
        G10["G10 Local Nav + Mutation<br/>91%<br/>#7 / #8"]
        G11["G11 Structural Table Ops<br/>90%<br/>#94"]
        G12["G12 Field Projection / Transfer<br/>89%<br/>#100"]
        G13["G13 Aggregate / View Helpers<br/>90%<br/>#95"]
        G14["G14 Headless Interaction / Dialogs<br/>84%<br/>#101"]
        G15["G15 Memory / Assignment Semantics<br/>86%<br/>#99"]
        G16["G16 Corpus + Regression Harness<br/>55%<br/>supports #7-#12"]
    end

    subgraph A4[Phase A4 - Automation And Interop Semantics]
        direction TB
        G17["G17 CREATEOBJECT / GETOBJECT<br/>35%<br/>#10 / #11"]
        G18["G18 OLE/COM Invoke + Containment<br/>28%<br/>#10 / #12"]
    end

    G16 --> G1
    G16 --> G4

    G1 --> G2 --> G3
    G1 --> G10
    G1 --> G11

    G4 --> G5
    G4 --> G6
    G4 --> G7
    G4 --> G9
    G4 --> G10
    G4 --> G12
    G4 --> G13
    G4 --> G14
    G4 --> G15

    G2 --> G6
    G5 --> G7
    G5 --> G8
    G5 --> G14
    G5 --> G17

    G6 --> G8
    G6 --> G10
    G6 --> G12
    G6 --> G13

    G7 --> G8

    G9 --> G10
    G9 --> G11
    G9 --> G12
    G9 --> G14
    G9 --> G15
    G9 --> G17

    G10 --> G11
    G10 --> G12
    G10 --> G13

    G11 --> G13
    G12 --> G13
    G12 --> G14

    G15 --> G17
    G17 --> G18

    class G1,G2,G3,G4,G5,G6,G7,G8,G10,G11,G12,G13 green;
    class G9,G14,G15,G16 amber;
    class G17,G18 red;
    class A1,A2,A3,A4 lane;
```

## Recommended Order

The recommended order is not simply "lowest percentage first." It is:

1. finish the storage and search seams that everything else inherits
2. close the remaining macro/eval/runtime-state seams before polishing more commands
3. finish command groups that sit on those two foundations
4. leave OLE/COM deepening until the runtime/memory/macro base is less noisy

That produces this practical order:

1. `WP0` corpus and regression harness expansion
2. `WP1` DBF/FPT validation and repair completion
3. `WP2` index fidelity completion
4. `WP4` work-area and session residual cleanup
5. `WP5` order/collation/search residuals
6. `WP8` macro/eval/runtime-state closure
7. `WP10` local structural table-operation closure
8. `WP9` field-projection and data-transfer closure
9. `WP12` headless interaction/display closure
10. `WP6` SQL handle/API residuals
11. `WP7` remote/result-cursor semantic closure
12. `WP11` query/aggregate and helper closure
13. `WP3` DBC container completion
14. `WP13` automation activation parity
15. `WP14` automation containment and fault behavior

That sequence is slightly different from the raw dependency graph because it prioritizes:

- issue `#7` / `#8` closure before automation depth
- highest fan-out prerequisites before local polish
- near-complete lanes that can be closed cheaply once their upstream seams are stable

## Gantt Chart

This is a planning schedule, not a delivery promise. It assumes focused sequential attention on the critical path with parallel work only where dependencies are already clean.

```mermaid
gantt
    title Phase A Recommended Work Order
    dateFormat  YYYY-MM-DD
    axisFormat  %m-%d

    section Foundation
    WP0 Corpus / Regression Harness (#7,#8,#10,#11,#12) :done, wp0, 2026-05-04, 1w
    WP1 DBF/FPT Validation + Repair (supports #7)       :wp1, after wp0, 2w
    WP2 Index Fidelity Completion (supports #7 / #92)   :wp2, after wp1, 2w
    WP4 Work-Area / Session Cleanup (#7,#8)             :wp4, after wp0, 1w

    section Runtime Semantics
    WP5 Order / Collation / Search Residuals (#92)      :crit, wp5, after wp2, 2w
    WP8 Macro / Eval / Runtime-State Closure (#97/#98/#99) :wp8, after wp4, 2w
    WP10 Structural Table Ops Closure (#94)             :wp10, after wp8, 2w
    WP9 Field Projection / Transfer Closure (#100)      :crit, wp9, after wp5, 1w
    WP12 Headless Interaction Closure (#101)            :crit, wp12, after wp9, 1w

    section SQL And Cursor Parity
    WP6 SQL Handle/API Residuals (#7)                   :wp6, after wp4, 2w
    WP7 Remote Cursor Semantic Closure (#93)           :wp7, after wp5, 2w
    WP11 Query / Aggregate Helper Closure (#95)        :wp11, after wp5, 2w
    WP3 DBC Container Completion (#96)                 :wp3, after wp2, 1w

    section Automation
    WP13 Automation Activation Parity (#10/#11)        :crit, wp13, after wp12, 2w
    WP14 Automation Containment (#10/#12)              :crit, wp14, after wp13, 1w
```

## CPM Chart

The critical path below is the longest prerequisite chain for Phase A completion under the above work-package model.

Critical path:

- `WP0 -> WP1 -> WP2 -> WP5 -> WP9 -> WP12 -> WP13 -> WP14`
- issue path: `(#7/#8/#10/#11/#12 support) -> (#92) -> (#100) -> (#101) -> (#10/#11) -> (#12)`

Total critical-path duration:

- `12 weeks`

```mermaid
flowchart LR
    classDef normal fill:#eef2ff,stroke:#4338ca,stroke-width:1px,color:#1e1b4b;
    classDef critical fill:#fee2e2,stroke:#b91c1c,stroke-width:3px,color:#7f1d1d;

    WP0["WP0<br/>#7,#8,#10,#11,#12<br/>1w<br/>ES 0 EF 1"]
    WP1["WP1<br/>supports #7<br/>2w<br/>ES 1 EF 3"]
    WP2["WP2<br/>#92 support<br/>2w<br/>ES 3 EF 5"]
    WP3["WP3<br/>#96<br/>1w<br/>ES 5 EF 6"]
    WP4["WP4<br/>#7/#8<br/>1w<br/>ES 1 EF 2"]
    WP5["WP5<br/>#92<br/>2w<br/>ES 5 EF 7"]
    WP6["WP6<br/>#7<br/>2w<br/>ES 2 EF 4"]
    WP7["WP7<br/>#93<br/>2w<br/>ES 7 EF 9"]
    WP8["WP8<br/>#97/#98/#99<br/>2w<br/>ES 2 EF 4"]
    WP9["WP9<br/>#100<br/>1w<br/>ES 7 EF 8"]
    WP10["WP10<br/>#94<br/>2w<br/>ES 4 EF 6"]
    WP11["WP11<br/>#95<br/>2w<br/>ES 7 EF 9"]
    WP12["WP12<br/>#101<br/>1w<br/>ES 8 EF 9"]
    WP13["WP13<br/>#10/#11<br/>2w<br/>ES 9 EF 11"]
    WP14["WP14<br/>#10/#12<br/>1w<br/>ES 11 EF 12"]

    WP0 --> WP1 --> WP2 --> WP3
    WP0 --> WP4
    WP2 --> WP5
    WP4 --> WP5
    WP4 --> WP6 --> WP7
    WP4 --> WP8
    WP8 --> WP10 --> WP11
    WP5 --> WP9 --> WP12 --> WP13 --> WP14
    WP8 --> WP9
    WP8 --> WP12
    WP1 --> WP10
    WP5 --> WP7
    WP10 --> WP9
    WP5 --> WP11

    class WP3,WP4,WP6,WP7,WP8,WP10,WP11 normal;
    class WP0,WP1,WP2,WP5,WP9,WP12,WP13,WP14 critical;
```

## First Things To Address

If the goal is "what should be handled first, concretely?", the answer is:

| Priority | First Slice | Why First |
| --- | --- | --- |
| 1 | corpus and fixture expansion around issue `#7` / `#8` leftovers | It lowers the cost of every later parity slice and reduces regression risk. |
| 2 | DBF/FPT validation and repair gaps | Storage correctness is still the deepest common dependency in Phase A. |
| 3 | remaining index-expression/collation/runtime-consumption gaps | Many command families still inherit their hardest parity bugs from this seam. |
| 4 | macro/eval/runtime-state closure | This is the highest-leverage remaining issue `#8` surface and still fans out into many commands. |
| 5 | local structural table-operation residuals | These still touch correctness, rollback, persistence, and import/export behavior. |
| 6 | narrow field-projection and headless command residuals | These are close to done, but should be finished after search and macro foundations are steadier. |
| 7 | automation activation and containment | Important, but lower ROI until the runtime base underneath it is quieter. |

## Issue Hierarchy

GitHub issue hierarchy is now in use for the remaining Phase A work. Repo-wide top-level umbrella issues are now `#108`-`#114`, and within the active runtime tree `#7` and `#8` remain the A3 umbrella issues whose durable lane issues exist as real sub-issues.

Current structure under `#7`:

| Parent | Sub-Issue | Maps To |
| --- | --- | --- |
| `#7` | `#92` Finish residual order/collation/search parity | G6 / WP5 |
| `#7` | `#93` Finish remote/result-cursor behavior parity | G8 / WP7 |
| `#7` | `#94` Finish structural table-operation parity | G10-G11 / WP10 |
| `#7` | `#95` Finish aggregate/view/helper command parity | G13 / WP11 |
| `#7` | `#96` Finish DBC/container and catalog fidelity | G3 / WP3 |

Current structure under `#8`:

| Parent | Sub-Issue | Maps To |
| --- | --- | --- |
| `#8` | `#97` Finish macro suffix/terminator and nested expansion semantics | G9 / WP8 |
| `#8` | `#98` Finish runtime-state normalization and `SET()` compatibility residuals | G5-G9 / WP8 |
| `#8` | `#99` Finish memory-variable, `PUBLIC`/`PRIVATE`/`RELEASE`, and assignment semantics | G15 / WP8 |
| `#8` | `#100` Finish field-transfer and macro-target data movement parity | G12 / WP9 |
| `#8` | `#101` Finish headless interaction macro/eval fidelity | G14 / WP12 |

Current active prompt-sized slice queue:

| Parent | Slice Issue | Intended Prompt Slice |
| --- | --- | --- |
| `#95` | `#119` | close one more aggregate/view/helper parity residual |
| `#95` | `#120` | close one more aggregate/view/helper parity residual |
| `#96` | `#121` | close one more DBC/container fidelity residual |
| `#96` | `#122` | close one more DBC/container fidelity residual |

Additional prompt-sized native slice queues now also exist under the adjacent active A3/A4 lanes:

- `#95`: `#119`, `#120`
- `#96`: `#121`, `#122`
- `#10`: `#131`, `#132`
- `#11`: `#133`, `#134`
- `#12`: `#135`, `#136`

Current dependency links:

- `#93` is blocked by `#92`
- `#95` is blocked by `#94`
- `#100` is blocked by `#97`
- `#10` is blocked by `#99`
- `#11` is blocked by `#99`

## Slice-Issue Policy

For the remaining Phase A runtime work, the implementation unit should be a prompt-sized issue rather than a broad umbrella issue.

- Keep `#7`, `#8`, and their lane issues (`#92`-`#101`) as planning and closure umbrellas.
- Keep the repo-root umbrella issues `#108`-`#114` as navigation/grouping roots rather than execution units.
- Keep milestones aligned to the same tree: `Root/#...` for repo umbrellas, `A3/#...` / `A4/#...` for active runtime lanes, and slice issues inheriting the milestone of their parent lane.
- Before starting code work, pick one open slice issue under the active lane, or create a new slice issue if the intended change does not fit an existing one.
- One implementation prompt should normally map to one slice issue, one focused validation loop, and one doc/handoff update.
- Close or retarget the slice issue when the prompt-sized implementation lands; do not hide shipped work only inside the broader lane issue body.

## Suggested Use

Use this document together with:

- [remaining-work.md](/home/rich/dev/Project-Copperfin/remaining-work.md:306)
- [docs/22-vfp-language-reference-coverage.md](/home/rich/dev/Project-Copperfin/docs/22-vfp-language-reference-coverage.md:1)

Operationally:

- pick work from the current critical path unless there is a very cheap near-complete lane to close
- avoid starting new A4 automation depth before the next A3 macro/eval slice is tighter
- treat G16 corpus expansion as a repeated enabling activity, not a one-time task
