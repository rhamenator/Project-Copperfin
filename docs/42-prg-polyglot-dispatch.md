# PRG Polyglot Dispatch Boundary

## Purpose

`CFPOLYGLOTDISPATCH()` is Copperfin's first PRG-callable boundary for a
host-owned polyglot route. FP/VFP source remains the orchestration surface;
foreign-language source stays in a separately built artifact. The native PRG
engine validates immutable request data, invokes one explicitly configured host
callback synchronously on the current PRG task, and publishes one deterministic
JSON evidence document.

This slice establishes the runtime boundary. The trusted runtime-host
composition described below can now provision the callback from an explicit
validated route registry and admitted-artifact bindings. A host that does not
provide that state still reports `unavailable` and launches nothing.

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

This PRG boundary does not parse inline foreign source, discover or install an
artifact, load a language runtime, authorize an executable, retry, or permit a
foreign worker to call mutable PRG state. The adjacent trusted host composition
below owns explicit admitted state and delegates to the portable executor; it
remains unavailable when that trusted state is absent.

## Verification

`test_prg_engine_polyglot_dispatch` covers unavailable configuration, invalid
arity/types/capability/JSON/sample, critical-section rejection, exact request
transport, cooperative `CFTASKCANCEL()` propagation, callback exception containment,
malformed and inconsistent host results, the 1 MiB final-envelope ceiling,
exact large-number payload preservation, invariant event data, and real
`SPAWN`/`CFTASK*` supervision on another PRG task. The isolation record is
portable, parallel-safe, test-owned, network-free, and process-free.

Local GCC Release focused plus adjacent coverage passes `6/6`, and the focused
regression repeats `20/20`. Clang 21 ASan/UBSan and ThreadSanitizer pass.
Focused static analysis finds no issue in changed lines; its diagnostics are
pre-existing in unchanged runtime lines. Exact product/test candidate
`0d77c83b8` passes Linux Native `31429006833` and macOS Native `31429008763` at
`335/335`, and Windows Native `31429010944` at `334/334`; the focused regression
passes on all three hosts, the macOS four-locale matrix passes `8/8`, and all
eight candidate-head protected checks pass. A
superseded macOS run found a timing assumption only in this test's cancellation
fixture; the replacement waits for a test-owned callback-entry marker and
atomically owns a unique temporary root without changing product behavior.

## Trusted runtime-host integration

`PolyglotRuntimeHost::create()` now supplies this callback from a validated
route registry and explicit per-capability admitted-artifact bindings. It owns
that state for every copied callback, binds the exact immutable capability,
arguments, sample, and cancellation probe into the existing portable route
executor, and maps only the existing invariant result fields back to this PRG
boundary. Same-capability calls serialize admission revalidation and receive
unique correlation IDs; no runtime/session/frame/cursor/object reference is
available to the host callback.

Construction rejects invalid routes, missing/duplicate/mismatched bindings,
rejected admission, invalid protocol/correlation/bridge policy, multiple
attempts, mutable pre-bound request callbacks, and missing route-required
native/parity callbacks before enabling execution. Focused portable coverage
also proves all route states, fallback/parity/error identities, both
cancellation policies, bounds/redaction, callback lifetime, concurrency, real
session creation, and a marker-synchronized `CFTASKCANCEL()` reaching the
bounded candidate. Local GCC Release focused and adjacent tests pass `6/6`, the
focused target repeats `20/20`, Clang 21 ASan/UBSan passes with leak detection,
ThreadSanitizer passes, and focused static analysis is clean. Exact product/test
candidate `b66e6085f` passes Linux Native `31453166584` and macOS Native
`31453166506` at `336/336`, Windows Native `31453166516` at `335/335`, the
focused regression on every host, the macOS locale matrix at `8/8`, and all
eight candidate-head protected checks without a blocking result.

This integration still adds no language-specific runtime, discovery/install,
inline foreign source, retry, second artifact, new task lifecycle, mutable PRG
callback, or atomic handle-bound launch.
