# Table-Buffer Negative RECNO Traceability

## Scope

This record covers `RQ-CF-PRG-018`, recovered only from the installed VFP9
help topic `html/8d95aef2-00d6-4a21-ba50-af1ba979eee7.htm` (How to: Append
and Delete Records in Table Buffers).

## Derived Constraints

`DQ-prg-table-buffer-recno-001`: pending local table-buffer appends in modes
4 and 5 shall have sequential public negative `RECNO()` identities, and
`GO -n` shall select the corresponding pending append.

`DQ-prg-table-buffer-recno-002`: a public negative identity shall never be
used as a DBF physical record index. Commit, locking, verified-byte admission,
rollback, and staged mutation shall continue to use Copperfin-owned positive
internal positions.

`DQ-prg-table-buffer-recno-003`: `TABLEUPDATE()` shall materialize pending
rows with ordinary positive physical identities, while `TABLEREVERT(.T.)`
shall remove pending rows without writing them.

## Hazard, Misuse, and Rollback

`HZ-data-corruption-01` applies. Confusing a negative buffered identity with a
physical DBF index could direct a mutation, rollback, or admission update at
the wrong row. The implementation translates negative values only at the
public navigation boundary and retains existing positive positions elsewhere.

Rollback for this slice is coherent only when the public `RECNO()` mapping,
negative-navigation translation, focused regression, requirement entry,
coverage note, changelog, handoff, and this record are removed together.
Existing buffered append, commit, and revert behavior then remains at its
prior documented boundary.

## Verification

`DV-prg-table-buffer-recno-001` is the portable CTest target
`test_prg_engine_runtime_surface_functions_buffering`. It covers two pending
appends, negative identity sequencing, `GO -1`/`GO -2` round trips, commit to
positive physical rows, and full revert in both pessimistic and optimistic
table-buffer modes. It uses test-owned local files only; no network,
subprocess, environment, or sample dependency is introduced.

This is development-assurance evidence, not a claim of formal DO-178C
compliance, certification, complete VFP buffering parity, or remote-cursor
support.
