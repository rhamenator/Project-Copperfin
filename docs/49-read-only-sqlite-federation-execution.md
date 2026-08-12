# Read-Only SQLite Federation Execution

## Scope

Copperfin's first live relational connector executes the existing deterministic
Fox-SQL-to-SQLite plan against one explicitly named local SQLite database:

```text
copperfin_runtime_host --federation-backend sqlite --federation-query "SELECT id, ALLTRIM(name) FROM customer" --federation-target <database-path> --federation-execute-read-only true
```

Planning remains the default. Explicit execution requires the existing
`project.open` permission and emits a content-free `federation.sqlite_read`
success or rejection audit event.

## Trust Boundary

The connector accepts only a successful deterministic SQLite plan. It rejects
SQLite URI targets, opens the database read-only and without following the
final symbolic link when supported, and checks physical file identity before
and after execution. It denies extension loading, attached databases, PRAGMA,
mutation, multiple statements, and dangerous file/extension helper functions.

Results are schema-versioned JSON. Column names and text must be valid UTF-8;
values retain explicit `null`, `integer`, `real`, `text`, or hexadecimal `blob`
kinds. Provider details are not serialized into runtime-host output.

Default ceilings are 64 MiB per database, 1,000 rows, 256 columns, 1 MiB per
cell, 8 MiB aggregate result content, 10,000,000 SQLite virtual-machine steps,
and 1 MiB SQL text. Limits fail closed; results are never silently truncated.

## Build And Platform Contract

Windows 10 and newer use the system `winsqlite3` library. Copperfin keeps a
narrow private compatibility declaration header beside the connector when an
SDK installation omits the optional header; it does not expose that native ABI
through the Copperfin public include tree and does not package or replace the OS
library. Linux and macOS use a discoverable system SQLite3 development package.
Ordinary builds provide a stable unavailable adapter when SQLite is absent.
Validation and release builds set `COPPERFIN_REQUIRE_SQLITE_CONNECTOR=ON`,
making absence a configure failure. Hosted Windows, Linux, and macOS validation
builds and runs both connector tests plus the private-boundary contract. See
[53-private-sqlite-native-api-boundary.md](53-private-sqlite-native-api-boundary.md).

## Deliberate Limits

This does not implement PostgreSQL, SQL Server, Oracle, provider-backed VFP
connection handles, mutable queries, remote cursors, transactions, credentials,
network access, document/vector stores, or AI planning. Pre/post identity checks
detect replacement during the operation but do not claim an OS-level
pinned-handle guarantee against every privileged same-identity or ABA race.

## Regression Evidence

`test_sqlite_federation_connector` covers real translated queries, typed values,
stable JSON, forbidden SQL/functions, target validation, and resource limits.
`test_runtime_host_sqlite_federation` covers the product process, opt-in, RBAC,
planning compatibility, stable failures, and audit redaction. Runtime-host
localization coverage preserves invariant option tokens.

At exact signed/DCO implementation head `8aa8d514f`, Generated Launcher
Validation `31535000120` built and ran the connector and product-process tests
on Windows, Ubuntu, and macOS. All eleven protected checks passed. Independent
Linux review rebuilt the exact head with ASan/UBSan, exercised the connector
and runtime-host boundaries, cross-checked the Windows ABI declarations, and
found no defect. The reviewer could not compile the Windows-only declaration
path locally; the hosted Windows job supplied that direct build, link, and
execution evidence. A scratch race probe also confirmed both that permanent
target replacement is rejected and that the same-identity ABA limitation
documented above is real.
