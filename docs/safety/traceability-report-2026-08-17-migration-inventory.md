# Migration Inventory Foundation Traceability

## Scope

This report records `RQ-CF-MIGRATION-001`, derived from the explicit
repository-owner migration policy in `docs/03-compatibility-and-migration.md`.
It implements only the first import-pipeline step: read-only metadata inventory
for an absolute project root that is not itself a symbolic link.

## Requirement And Boundary

The inventory accepts no relative root and no root symbolic link. It enumerates
regular-file metadata only, classifies a fixed set of VFP suffixes, and returns
project-relative entries in lexical order. Symbolic links and Windows reparse
points (including directory junctions) are reported through the existing
`skippedSymlinks` field but are never followed. Its schema-version-1 JSON
serializer omits the caller's absolute root. It does not read asset bytes,
parse assets, execute code, call a network service, create an output file, or
mutate a project.

`HZ-system-failure-01` and `HZ-data-corruption-01` apply. The containment and
no-mutation constraints prevent this foundation from treating a discovered tree
as authority to interpret or change its contents. A scan that cannot complete
returns a stable incomplete diagnostic rather than silently claiming a complete
inventory.

A name that cannot be represented as UTF-8 is omitted and produces the same
incomplete result, preserving the JSON contract rather than emitting invalid
text.

## Verification And Limitations

Focused Debug build and CTest pass `1/1` through `test_project_inventory`.
The regression proves deterministic ordering, case-insensitive recognized
suffix classification, JSON escaping, absolute-root omission, rejection of a
relative root, non-UTF-8 POSIX-name denial, and file/directory symbolic-link
reporting without traversal. The same directory-link fixture exercises Windows
reparse-point rejection on protected Windows validation, where the standard
library may otherwise identify a junction as a directory. Its machine-readable
CTest isolation record is complete: portable, parallel-safe, test-owned
filesystem only, and no environment, child-process, network, or sample
dependency.

Dependency discovery, metadata graphs, compatibility risks, migration-mode
selection, database mapping, editable workspace mapping, all other specified
migration outputs, and any source mutation remain unimplemented. This report
makes no release-completion or safety-critical suitability claim.
