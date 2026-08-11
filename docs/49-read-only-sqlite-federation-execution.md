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

Windows uses `winsqlite3`; Linux and macOS use a discoverable system SQLite3
development package. Ordinary builds provide a stable unavailable adapter when
SQLite is absent. Validation and release builds set
`COPPERFIN_REQUIRE_SQLITE_CONNECTOR=ON`, making absence a configure failure.
Hosted Windows, Linux, and macOS validation builds and runs both new tests.

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
