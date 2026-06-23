# Known VFP9 Bug Exceptions Registry

## Purpose

`docs/01-product-charter.md` commits Copperfin to exact duplication of VFP9 edge-case behavior, with exactly two carved-out exception categories: VFP9's catalogued bugs, and inputs where real VFP9 crashes. This document is the registry of those exceptions.

A behavioral difference between Copperfin and real VFP9 is only a legitimate exception if it has an entry here. Anything not listed here is presumed to be a parity gap (a Copperfin defect to fix), not an intentional divergence.

## Evidence Rules

Per `docs/07-clean-room-rules.md`, an entry's evidence must come from an allowed input:

- shipped VFP9 help content, the official Language Reference, service-pack/KB release notes, or
- observed behavior of a real, installed VFP9 instance (run a minimal `.prg` oracle script in VFP9 and record the output)

Decompiling or disassembling the VFP9 binary is a restricted input and must never be used to populate this registry, even though it might be technically informative.

Each entry should reference the oracle script used (kept under a `tests/compat/` or similar directory once one exists) so the observation is reproducible.

## Entry Schema

| Field | Meaning |
| --- | --- |
| ID | Stable identifier, `KBX-NNN` (Known-Bug eXception), never recycled |
| Classification | `known-bug` (VFP9 behaves incorrectly relative to its own documented intent) or `crash` (VFP9 terminates or hangs on this input) |
| VFP9 behavior | What real VFP9 actually does, as observed |
| Evidence | Oracle script path / doc citation / KB or service-pack reference, plus VFP9 build/SP version tested |
| Copperfin behavior | What Copperfin intentionally does instead |
| Rationale | Why this is excluded from parity rather than treated as a gap |
| Status | `proposed` (observed, not yet implemented), `applied` (Copperfin's divergent behavior is implemented and tested), `retired` (no longer applicable) |
| Linked issue | GitHub issue tracking the implementation, if any |

## Crash-Case Default

Unless a specific entry says otherwise, the default Copperfin behavior for any input classified `crash` is to raise a catchable runtime error through the existing `TRY`/`CATCH`/`ON ERROR` machinery (see `src/runtime/prg_engine.cpp` error/fault handling) rather than terminating the process. An entry only needs to specify a different fallback if the default catchable-error behavior is wrong for that specific case.

## Registry

_No entries yet. This is a scaffold — populate it as edge cases are discovered and validated against real VFP9, not retroactively justified after Copperfin's behavior is written._

| ID | Classification | VFP9 Behavior | Evidence | Copperfin Behavior | Rationale | Status | Linked Issue |
| --- | --- | --- | --- | --- | --- | --- | --- |
| | | | | | | | |

## Process For Adding An Entry

1. Identify the candidate divergence (from requirements-recovery work, manual testing, or a bug report).
2. Write a minimal `.prg` oracle script that isolates the behavior.
3. Run it against real, installed VFP9 (with SP/build version recorded) and capture the actual output.
4. Classify: is this a documented VFP9 bug (cross-check Microsoft KB articles, service pack release notes, or well-known community-documented defects), or does VFP9 crash/hang?
5. If neither — it is not an exception. It is a parity requirement Copperfin must meet exactly.
6. Add the row to the Registry table above with `status: proposed`.
7. Implement Copperfin's intentional alternate behavior, add regression test coverage, update `status: applied`, and link the closing issue.
