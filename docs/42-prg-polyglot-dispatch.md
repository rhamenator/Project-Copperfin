# PRG Polyglot Dispatch Boundary

## Purpose

`CFPOLYGLOTDISPATCH()` is Copperfin's first PRG-callable boundary for a
host-owned polyglot route. FP/VFP source remains the orchestration surface;
foreign-language source stays in a separately built artifact. The native PRG
engine validates immutable request data, invokes one explicitly configured host
callback synchronously on the current PRG task, and publishes one deterministic
JSON evidence document.

This slice establishes the runtime boundary. The ordinary runtime host does not
yet provision the callback from a route registry, artifact admission record, or
release configuration, so an unconfigured call reports `unavailable` and does
not launch anything.

## PRG Function

```foxpro
cEvidence = CFPOLYGLOTDISPATCH(cCapabilityId, cArgumentsJson [, nSelectionSample])
```

- `cCapabilityId` uses the invariant polyglot capability grammar: a lower-case
  ASCII letter followed only by lower-case ASCII letters, digits, `.`, `_`, or
  `-`.
- `cArgumentsJson` must be one complete UTF-8 JSON object within the native
  parser's default 1 MiB, 64-level, and 65,536-value limits.
- `nSelectionSample` defaults to `0` and, when supplied, must be an exact numeric
  integer from `0` through `99`. Character coercion is not accepted.

Invalid input fails before the host callback. The function always returns one
JSON document rather than localized prose. PRG can inspect it through
`CFJSONVALID()`, `CFJSONTYPE()`, and `CFJSONGET()`.

## Evidence Contract

The version-1 document has this fixed field order and shape:

```json
{
  "schema_version": 1,
  "status": "success",
  "error_code": "polyglot.execution.candidate_success",
  "capability_id": "interop.invoice-v1",
  "authority": "candidate",
  "route_selection": "candidate",
  "native_invocation_count": 0,
  "candidate_invocation_count": 1,
  "native_fallback_executed": false,
  "payload": {"invoice_id": 42}
}
```

Machine values are invariant and are not localized. `status` is one of
`success`, `invalid-request`, `native-failed`, `candidate-failed`, `cancelled`,
`parity-failed`, `unavailable`, or `host-failed`. Authority is `none`, `native`,
or `candidate`; route selection is `none`, `native`, `shadow`, or `candidate`.
The two invocation counts are limited to zero or one. The runtime rejects
internally inconsistent combinations, noncanonical reason codes, malformed or
oversized payload JSON, invalid enum values, and evidence documents exceeding
1 MiB. Rejected host output is replaced with a bounded fail-closed document;
partial payload bytes are not published.

## Host And Concurrency Contract

`RuntimeSessionOptions::polyglot_dispatch_callback` is the only entry point.
The callback receives capability ID, exact argument JSON, sample, and a
read-only cancellation probe. It receives no runtime session, frame, variable,
cursor, object, data-session, or event-stream reference. Exceptions are
contained as `polyglot.prg.dispatch_exception`.

The callback is synchronous on the current PRG task. A direct call may block,
so `CFPOLYGLOTDISPATCH()` is rejected through the centralized concurrency
policy while any `ENTER CRITICAL` section is held. PRG code that wants
nonblocking supervision places the call in an ordinary PRG worker and uses the
existing task lifecycle:

```foxpro
PROCEDURE RunCandidate
    RETURN CFPOLYGLOTDISPATCH('interop.invoice-v1', '{"invoice_id":42}', 17)
ENDPROC

SPAWN RunCandidate TO nTask
DO WHILE CFTASKSTATUS(nTask) == 'running'
    * Continue caller-owned work.
    YIELD
ENDDO
cEvidence = CFTASKRESULT(nTask)
AWAIT nTask TO lJoined
```

The spawned child owns its PRG runtime state. The host callback can observe the
child's atomic cancellation request but cannot enter the parent or child
runtime. A cooperative callback may return invariant `cancelled` evidence and
complete normally; otherwise the established cancelled-task lifecycle applies.
`CFTASKRESULT()` publishes only an immutable completed value.

## Failure Codes

The PRG boundary owns these invariant reasons:

- `polyglot.prg.invalid_arguments`
- `polyglot.prg.invalid_capability_id`
- `polyglot.prg.invalid_arguments_json`
- `polyglot.prg.invalid_selection_sample`
- `polyglot.prg.blocked_in_critical_section`
- `polyglot.prg.dispatch_unavailable`
- `polyglot.prg.dispatch_exception`
- `polyglot.prg.invalid_host_result`
- `polyglot.prg.result_too_large`

Valid configured results retain the host's canonical `polyglot.*` execution
reason. Runtime events record capability, status, and reason but never argument
or payload bytes.

## Nonclaims And Next Wiring

This boundary does not parse inline foreign source, discover or install an
artifact, load a language runtime, authorize an executable, retry, or permit a
foreign worker to call mutable PRG state. It also does not itself provision the
ordinary runtime host. The next host-wiring slice must build the callback only
from trusted route configuration and admitted artifact identity, call the
portable route executor, propagate its cancellation and exact evidence, and
remain unavailable when that trusted state is absent.

## Verification

`test_prg_engine_polyglot_dispatch` covers unavailable configuration, invalid
arity/types/capability/JSON/sample, critical-section rejection, exact request
transport, cooperative `CFTASKCANCEL()` propagation, callback exception containment,
malformed and inconsistent host results, the 1 MiB final-envelope ceiling,
exact large-number payload preservation, invariant event data, and real
`SPAWN`/`CFTASK*` supervision on another PRG task. The isolation record is
portable, parallel-safe, test-owned, network-free, and process-free.

Local GCC Release focused plus adjacent coverage passes `6/6`, and the focused
regression repeats `20/20`. Clang 21 ASan/UBSan passes. Focused static analysis
finds no issue in changed lines; its diagnostics are pre-existing in unchanged
runtime lines. Hosted evidence is recorded in the roadmap and handoff when
available.
