# ON ESCAPE Runtime Traceability Report

Date: 2026-08-17

Scope: `RQ-CF-PRG-003`, VFP9-compatible static headless `ON ESCAPE` behavior.

Allowed requirement sources are the installed VFP9 help topics
`html/189f7569-fdc0-416f-8728-dfc1cc9b9176.htm` (`ON ESCAPE`),
`html/50481ef8-c338-4def-ae13-4da7f4a40748.htm` (`SET ESCAPE`), and
`html/421c6247-80e9-478a-a7a3-2f2bd4d816c0.htm` (`ON()`). This report records
DO-178C-inspired assurance adapted to a general-purpose C++/.NET platform; it
does not claim formal compliance, certification, or suitability for a
safety-critical deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification | Hazard link |
| --- | --- | --- |
| `DQ-prg-on-escape-001`: the runtime retains or clears static `ON ESCAPE`, reports it through `ON('ESCAPE')`, and defaults `SET ESCAPE` to ON. | `DV-prg-on-escape-001`: focused assignment, query, and default-state regression. | `HZ-system-failure-01` |
| `DQ-prg-on-escape-002`: when enabled at an active `READ EVENTS` boundary, Escape dispatches the static command before `ON KEY LABEL ESC`; disabling Escape suppresses only the former. | `DV-prg-on-escape-002`: focused priority, suppression, and event-loop return regression. | `HZ-system-failure-01` |
| `DQ-prg-on-escape-003`: a host request is consumed only at a safe runtime boundary and never asynchronously interrupts native code. | `DV-prg-on-escape-003`: focused host-request/event-loop regression. | `HZ-system-failure-01` |

## Implementation and verification

- Implementation: `PrgRuntimeSession::dispatch_escape()` and
  `PrgRuntimeSession::request_escape()`; parser support for `ON ESCAPE`; the
  iterative dispatch and runtime-surface expression seams.
- Focused regression: `tests/test_prg_engine_on_escape.cpp`, registered as a
  portable, test-owned native test.
- Current local evidence: GCC 15.2 Release build and focused
  `test_prg_engine_on_escape` pass on Linux. Broader and hosted Windows/macOS
  validation remain release evidence rather than a claim in this report.

## Boundary, misuse, and rollback analysis

- The public seams accept no source text, file path, process, network, or UI
  authority. Static PRG command text remains inside the existing bounded
  iterative execution path.
- A host request uses a pending signal and is consumed only by the runtime;
  it does not interrupt native work, inject a thread, or create a new input
  capture path.
- `SET ESCAPE OFF` leaves an existing `ON KEY LABEL ESC` assignment intact, so
  suppression does not silently discard static key state.
- The VFP9 `RETRY` replay nuance, native key capture and buffering, form
  `KeyPress`, and `ON PAGE` report/print behavior are deliberately not claimed.
- Rollback is one atomic slice: remove the parser/dispatch/query seams, focused
  regression and registration, and this requirements/evidence update together.

Potential severity if misused: moderate. The primary residual is behavioral
compatibility divergence at UI/native interruption boundaries, not a new
authority boundary.
