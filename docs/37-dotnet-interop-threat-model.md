# .NET Interop Threat Model And Supervision Contract

This document defines Copperfin's v1 security boundary for optional .NET
parity calls. It is an enforcement contract for issue `#279`, not a claim that
the CLR host, a PRG job API, or arbitrary assembly execution is implemented.

## Control And Trust Boundaries

FP/VFP source remains the control plane. Foreign-language source does not need
to appear inside `.prg` files. A future PRG-callable facade may start a
supervised job and expose bounded `status`, `wait`, `cancel`, `result`, and
captured-output operations. Out-of-process execution is the default boundary.
Any later in-process Windows CLR bridge must implement the same authorization,
audit, lifetime, and result contracts rather than creating a second bypass.

The PRG/runtime thread owns mutable VFP-compatible state. Managed callbacks and
external worker threads may publish immutable completion records to a bounded
queue, but they must not enter the evaluator, select/work-area state, data
sessions, UI objects, or other mutable runtime state directly. The PRG/runtime
thread observes and applies queued results at an explicit safe point. Job
handles must be generation-safe so stale completion or cancellation cannot act
on a reused handle.

The host, not PRG payload text or a managed assembly, constructs the verified
security context supplied to `evaluate_dotnet_interop_call`. Setting a Boolean
in untrusted input is not proof. The host must bind the authenticated actor,
granted capability set, policy version, and audit-sink readiness before calling
the gateway.

## Protected Assets And Threats

| Asset or boundary | Representative threat | Required gate |
| --- | --- | --- |
| Runtime and data-session state | foreign thread re-entry, races, stale callback | runtime-thread-only mutation; immutable queued completion |
| Managed host and assemblies | arbitrary reflection or assembly loading | capability allowlist; reflection and assembly-loading scopes default empty |
| Files, network, processes | ambient external I/O through an otherwise safe helper | explicit per-capability external-I/O scope |
| Credentials and signing material | ambient environment or secret discovery | explicit per-capability secret scope; no ambient secret inheritance |
| Authorization context | caller asserts another actor or capability | host-verified actor and exact granted-capability membership |
| Audit trail | allowed work executes without durable decision evidence | available sink required; audit commit precedes an allowed execution |
| Diagnostic/log channel | newline or JSON injection obscures an event | canonical single-record JSON serialization with escaped strings |
| Availability | long/blocking in-process call stalls the runtime thread | finite in-process latency budget; supervised process for coarse/long work |

## Fail-Closed Gateway

The default profile permits only named parity capabilities. Reflection,
assembly loading, and secret access have empty capability scopes. External I/O
is initially scoped only to `safe-http-helpers`. A request is rejected before a
.NET path can be selected when any of these proofs is absent:

- non-empty authenticated actor identity;
- a host-verified policy context;
- an available audit sink when policy auditing is required;
- exact membership of the requested capability in the actor's granted scope;
- policy permission for reflection, assembly loading, external I/O, or secret
  access requested by the call.

Stable machine diagnostics use the `dotnet.interop.*` namespace. Human-facing
reason text remains localizable and must not be parsed as a machine contract.
An allow decision with `audit_commit_required=true` is conditional and carries
the non-executable `pending_audit` path. The caller must use
`evaluate_and_commit_dotnet_interop_call` with a trusted audit sink before an
executable `dotnet` path can be returned. The sink contract is a function
pointer plus opaque context, avoiding an owning callback wrapper. Missing,
failed, empty-receipt, or throwing sinks return
`dotnet.interop.audit_commit_failed` and `reject`; no managed route is exposed.
Rejection and native-fallback events are committed through the same boundary
when auditing is required.

`serialize_dotnet_interop_audit_event` emits one compact JSON object with fixed
field order:

```json
{"schema_version":1,"actor":"prg:main","capability":"task-primitives","decision":"allow","outcome":"allow","diagnostic_code":"dotnet.interop.allowed"}
```

The record always carries actor, capability, decision, outcome, and stable
diagnostic code. The serializer escapes control characters and quotes so one
untrusted value cannot forge an additional record. It emits the audit stream's
field delimiter as JSON `\\u007c`, preserving identity data instead of allowing
the line format to rewrite it.

The focused durable-adapter regression binds that sink to
`append_immutable_audit_event_to_contained_file`. A successful receipt is the
entry's SHA-256 chain hash after the audit implementation has flushed the
replacement file and directory/write-through state. The gateway treats an
empty receipt as failure even when a sink incorrectly reports success.

## Verification Evidence

At exact implementation head `5c6ccfa79`, focused GCC policy, runtime-pipeline,
and locale-install contracts pass `3/3`; Clang 21 ASan/UBSan passes the policy
model `1/1`; and focused Clang static analysis reports no findings. Hosted
Linux Native run `31297811840` and macOS Native run `31297811846` pass
`325/325`; Windows Native run `31297811822` passes `324/324`.
`test_platform_models` passes in every matrix, the macOS four-locale SET POINT
matrix passes `8/8`, and all eight protected PR checks pass at that head.

At exact durable-audit implementation head `af99a3af5`, Linux Native run
`31301544541` and macOS Native run `31301544576` pass `326/326`, and Windows
Native run `31302777143` passes `325/325`. `test_platform_models` and
`test_dotnet_interop_audit` pass on all three platforms; macOS also passes both
SET POINT targets under `C`, `en_US.UTF-8`, `pt_BR.UTF-8`, and `de_DE.UTF-8`
(`8/8`). All eight protected checks pass at that implementation head. Local
GCC selected contracts pass `5/5`, focused post-delimiter-hardening contracts
pass `2/2`, Clang 21 ASan/UBSan passes `2/2`, and focused static analysis reports
no finding.

## Nonclaims And Later Work

This slice does not load an assembly, launch a process, provide a PRG job
facade, capture worker output, transport an invocation envelope, select or hash
an artifact, or wire a runtime-host/PRG call. The policy API can now require and
prove an audit commit, and its regression uses the existing contained durable
audit stream; a later runtime adapter must derive the verified context from
trusted host state, bind that sink, enforce artifact identity at execution
time, and expose supervision to PRG without allowing foreign-thread runtime
re-entry.
