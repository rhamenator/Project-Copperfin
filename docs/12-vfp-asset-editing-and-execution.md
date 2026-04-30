# VFP Asset Editing And Execution

## Goal

Copperfin should be able to open, inspect, edit, preserve, and progressively execute legacy VFP application assets.

This includes both source-like artifacts and designer metadata artifacts.

## Priority Asset Families

Projects:

- `.pjx`
- `.pjt`

Forms and classes:

- `.scx`
- `.sct`
- `.vcx`
- `.vct`

Reports:

- `.frx`
- `.frt`

Labels:

- `.lbx`
- `.lbt`

Menus:

- `.mnx`
- `.mnt`

Code and headers:

- `.prg`
- `.h`

Data and metadata:

- `.dbf`
- `.fpt`
- `.cdx`
- `.dbc`

## Product Promise

Copperfin should support these modes:

### 1. Inspect

- read asset metadata safely
- show dependencies and structure
- identify unsupported features

### 2. Round-Trip Edit

- edit supported properties and code
- preserve non-understood metadata where possible
- write files back without destructive reshaping

### 3. Normalize

- import assets into an internal editable workspace model
- generate source-control-safe text representations
- keep links back to original binary assets

### 4. Execute

- run PRG/business logic in the Copperfin runtime
- bind project/forms/reports/menus into a compatibility host
- mix legacy assets with modern connectors and .NET integrations

## Current Implementation Snapshot

The current repo state is no longer only aspirational in this area:

- asset inspection is implemented for `DBF`/`FPT`, `CDX`/`DCX`/`IDX`/`NDX`/`MDX`, and first-pass `DBC` companion discovery/validation
- native DBC catalog export now supports whole-database JSON snapshots that include database identity, catalog-object rows, and table data blocks for resolvable local `TABLE` entries
- DBC `PROPERTIES` memo payloads are now decoded from raw sidecar bytes (no text-mode filtering), enabling first-pass property parity for `DATABASE`/`TABLE`/`VIEW`/`RELATION`/`CONNECTION` metadata extraction
- local-table mutation coverage exists for the shipped runtime flow, including memo-backed writes, structural mutations, compaction/truncation paths, and indexed-table guardrails
- the runtime can execute a substantial first-pass PRG surface, including work areas, local queries/mutations, SQL pass-through / remote cursor behavior, and a growing expression/runtime-helper layer
- xAsset-backed runtime bootstraps now exist for forms/classes, reports/labels, and menus, but these remain first-pass compatibility lanes rather than full VFP parity

This document stays focused on the execution/editing model. The detailed moving implementation ledger lives in `remaining-work.md` and `docs/22-vfp-language-reference-coverage.md`.

## Editing Strategy

### Binary Asset Rule

For binary VFP assets, Copperfin should not immediately rewrite everything into a new format.

Instead:

1. parse into normalized internal metadata
2. preserve raw/original records where needed
3. edit through structured models
4. write back only through a round-trip-safe serializer

### Source Control Rule

Each editable asset should have:

- original binary source
- normalized internal representation
- canonical text serialization for diff/review when possible

This mirrors the value proposition of tools like SCCText without making the binary asset the only truth humans can inspect.

## Execution Strategy

### Stage 1: Code Execution

Support:

- `.prg`
- command/eval logic
- work area and query operations
- report invocation hooks
- first-pass expression/runtime-helper compatibility used by shipped PRG flows

### Stage 2: Project And Asset Binding

Support:

- project loading
- form/class/report/menu registration
- dependency resolution
- asset identity and lookup
- index and metadata sidecar discovery where legacy assets depend on companion files

### Stage 3: Compatibility Host

Support:

- loading imported forms and class libraries into Copperfin Studio/runtime
- binding controls, data sessions, menus, and reports
- bridging unsupported areas with diagnostics rather than silent failure

### Stage 4: Mixed-Mode Runtime

Support:

- legacy VFP assets
- SQL connectors
- .NET service or library calls
- modern security policy

## Round-Trip Safety Requirements

For supported asset types, Copperfin should:

- preserve object identity and ordering where required
- preserve unsupported fields as opaque data when necessary
- avoid destructive reformatting
- emit explicit warnings when write-back fidelity cannot be guaranteed

## First Milestones

### Milestone A

- inspect PJX/SCX/VCX/FRX/LBX/MNX structure
- produce dependency and property reports

### Milestone B

- round-trip edit a limited subset of properties safely
- serialize canonical text snapshots for diffing

### Milestone C

- load project and PRG execution together
- bind forms/reports into a compatibility workspace

### Milestone D

- mixed-mode execution with .NET and SQL connectors

## How To Use: DBC JSON Export

Use the native asset-inspector API to export a database container (`.dbc`) into a single JSON snapshot.

Minimal C++ call:

```cpp
const auto export_result = copperfin::vfp::export_database_as_json("/path/to/northwind.dbc");
if (!export_result.ok) {
		// export_result.error contains the failure reason
}
// export_result.json contains the serialized snapshot
```

Example JSON shape:

```json
{
	"database": {
		"path": "/data/northwind.dbc",
		"name": "northwind"
	},
	"catalog": [
		{
			"record_index": 0,
			"object_type": "database",
			"object_name": "northwind",
			"parent_name": "",
			"properties": {
				"Caption": "Northwind"
			}
		},
		{
			"record_index": 1,
			"object_type": "table",
			"object_name": "Customers",
			"parent_name": "northwind",
			"properties": {
				"Comment": "Active customers"
			}
		}
	],
	"tables": {
		"Customers": {
			"fields": [
				{"name": "CUSTID", "type": "N", "length": 8, "decimals": 0},
				{"name": "COMPANY", "type": "C", "length": 40, "decimals": 0}
			],
			"records": [
				{"CUSTID": 1, "COMPANY": "Acme Corp"}
			]
		}
	}
}
```

Notes:

- `catalog` comes from DBC rows, including decoded binary `PROPERTIES` memo payloads.
- `tables` is populated only for local table files that can be resolved next to the DBC.
- `max_rows_per_table` can be used to cap exported row counts for large datasets.

## Key Risks

- binary asset edge cases not documented in public help
- preserving designer metadata exactly enough for round-trip confidence
- unsupported ActiveX or COM behaviors
- hidden dependencies stored in project metadata or resource records

## Recommended Validation Artifacts

- golden copies of representative PJX/SCX/VCX/FRX/MNX files
- binary diff checks after no-op round trip
- property-level round-trip tests
- execution traces for imported sample applications
- DBC JSON export fixtures that assert decoded `PROPERTIES` round-trips for representative catalog object types
