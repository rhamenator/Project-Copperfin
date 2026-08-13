# Polyglot And AI Subprojects

## Goal

Copperfin should stay native-first in its trusted core while still being useful in modern teams that rely on:

- `.NET`
- Python
- R
- native extension languages
- AI-assisted workflows
- MCP-compatible tool ecosystems

## Core Rule

The trusted runtime, file engine, build pipeline, and security boundary stay native-first.

Everything else must integrate through explicit, auditable boundaries.

Current maturity:

- Generated-launcher C# remains a separate early transpilation path and is not
  executed by the runtime host. The production polyglot host instead exercises
  the explicitly admitted Native AOT leaf described below.
- A portable bounded-process primitive now provides direct executable/argument
  invocation, an explicit child environment, timeout and cancellation handling,
  descendant cleanup, and independently bounded exact-byte stdout/stderr
  capture. The artifact invocation adapter below now consumes this primitive;
  neither layer is a PRG route or language integration by itself.
- A portable route-execution coordinator now applies the existing lifecycle
  decision to synchronous caller-owned native work and the admitted-artifact
  adapter. It composes shadow parity and one permitted native fallback.
- A host-injected `CFPOLYGLOTDISPATCH()` PRG seam now validates immutable
  capability/JSON/sample input and returns bounded invariant evidence. It runs
  on the current PRG task so existing `SPAWN`/`CFTASK*` supervision applies,
  and `PolyglotRuntimeHost` now provisions that callback from an explicit
  validated route registry and per-capability admitted-artifact state.
- The first real language target is a checked-in C# leaf candidate for
  `samples.dotnet.add-v1`. It publishes as a self-contained Native AOT
  executable, is admitted by exact hash, and is exercised end to end through
  the production host and ordinary PRG. It is not an embedded runtime or a
  general assembly loader.
- A deterministic advisory route-impact boundary now evaluates already-captured
  direct-C++, C++/.NET-wrapper, and C#-service measurements against explicit
  latency, throughput, memory, startup, security, parity, failure, and sample
  gates. It produces a stable preferred route and fallback order but cannot
  mutate the route registry or promote traffic. The representative benchmark
  runner and checked-in workload results remain separate work.
- A separate portable admission boundary now binds a canonical capability ID
  and rooted external-process authorization to one exact lowercase SHA-256 and
  physical file identity. Its opaque token must be revalidated before a later
  execution boundary; admission itself does not launch the artifact.
- A portable serializer now emits the versioned invocation request in fixed,
  compact field order after strict identity and arguments-object admission; the
  matching response parser admits only the checked-in success/error shapes.
- Existing `SPAWN` tasks now have a nonblocking PRG supervision seam for
  status, cooperative cancellation, retained return values, and completed
  print output. Exact candidate head `09c1b1046` passes Linux Native
  `31307387144` at `326/326`, macOS Native `31307387146` at `326/326` plus
  four locales at `8/8`, and Windows Native `31307387195` at `325/325`. At that
  earlier task-supervision head no external adapter was connected; the later
  runtime-host and Native AOT target above now complete that route.
- PRG now has a portable bounded JSON control-plane facade:
  `CFJSONVALID()`, `CFJSONTYPE()`, and `CFJSONGET()` validate and inspect an
  immutable result document through RFC 6901 JSON Pointer selection. Strings
  are decoded; other selected values retain exact JSON bytes so large numbers
  are not silently rounded. This adds no artifact admission or execution.
- PRG now also has a bounded native safe-regex facade for immutable completion
  text. Its byte-oriented subset uses fixed input/pattern/state ceilings and a
  non-backtracking state machine; unsupported advanced constructs fail closed
  and remain available only through a future separately approved capability.
- PRG now has bounded exact-byte `CFSHA256()`, `CFHMACSHA256()`,
  `CFHMACVERIFY()`, `CFBASE64ENCODE()`, and strict `CFBASE64DECODE()` helpers
  for immutable result identity, shared-key authentication, and text transport.
  The HMAC helpers do not manage keys; none encrypt data or admit an artifact.
- FP/VFP-shaped collection utilities remain native: `Collection` and the
  seeded `Scripting.Dictionary` compatibility surface provide ordered and
  keyed storage without importing managed collection types into PRG.
- No general HTTP facade is shipped. `safe-http-helpers` is explicitly denied
  by the parity policy and receives no external-I/O scope. A separately
  reviewed host adapter admitted through the polyglot contract is the current
  fallback for an application that deliberately needs network access.
- Python and R now each have one strict, tested sidecar hook through the generic
  artifact boundary. Each interpreter is an admitted executable; every loose script is a
  separately admitted supporting artifact, hash/identity pinned beneath an
  explicit physical root, bound to an exact argument position, and revalidated
  before launch. These are not general embedded-language surfaces. A separate
  local MCP host now supplies one bounded read-only DBF-header tool; broader
  language environments and model/provider or mutable MCP adapters remain
  planning work.
- .NET, Python, R, and other polyglot features should require a user-selected modernization target before they are exposed as product capabilities.

## Migration Contract v1

Polyglot modernization uses an artifact-first contract boundary. A candidate capability
must describe its input, output, error, invocation decision, execution budget, fallback
mode, reproducibility hashes, and observability events before a runtime route is exposed.
The canonical machine-readable schema and examples are maintained at:

- `docs/contracts/polyglot-migration-contract-v1.schema.json`
- `docs/contracts/polyglot-migration-contract-v1.json`
- `docs/contracts/polyglot-invocation-request-v1.json`
- `docs/contracts/polyglot-success-envelope-v1.json`
- `docs/contracts/polyglot-error-envelope-v1.json`

The contract version uses `1.x`: changing the major version means an incompatible
contract and requires a new capability contract; additive backward-compatible fields
advance the minor version; protocol corrections that do not change compatibility use
the protocol patch component. The protocol version remains a semantic `major.minor.patch`
value and is separate from the contract version.

Success and error envelopes keep machine fields stable: envelope kind, capability ID,
correlation ID, protocol version, error code, retryability, and payload structure are
not localized. Human-readable error messages are display text and must use the normal
localization path at the host boundary. This contract slice defines validation and
examples only; it does not route .NET, Python, R, AI, or MCP execution.

## Route Registry v1

The route registry is a separate decision layer above the migration contract. Its
machine state values are invariant and are intentionally not localized:

- `off`: invoke the native path only.
- `shadow`: invoke native and candidate paths, but return native; comparison belongs to
  the shadow-parity slice.
- `canary`: select the candidate from a deterministic `0..99` sample when the sample is
  below the configured percentage; otherwise select native.
- `on`: select the candidate with native fallback permitted by policy.
- `retire-legacy`: select the candidate with native fallback disabled.

`load_polyglot_route_registry` validates decoded capability records before creating the
registry. Capability IDs use the lower-case machine identifier syntax from the migration
contract, duplicate IDs are rejected, canary percentages are limited to `0..100`, and
non-canary states cannot carry a percentage. Missing configuration creates an empty
registry, whose lookup is safely `off`; invalid configuration returns stable machine
error codes. This slice defines selection only and does not invoke either bridge.

## Bridge Invocation Semantics v1

Bridge invocation is evaluated as a deterministic policy decision before an adapter
starts or retries an external process. The policy validates a positive timeout, a
positive latency budget no greater than the timeout, a positive attempt limit, and the
invariant `propagate`/`ignore` cancellation and `fail-fast`/`fallback-native`/
`fallback-artifact` policy values.

The decision order is cancellation, timeout, latency-budget exhaustion, and the
reported candidate failure class. `propagate` cancellation returns a cancelled result
without fallback; `ignore` treats cancellation as a failure and applies the configured
fallback. Other failures map to stable machine error codes and then either fail fast,
select native, or select the fallback artifact. The policy model records the attempt
limit but does not itself start or retry a process. The separate bounded-process
primitive described below supplies a low-level execution boundary; adapter-specific
retry and the connection from policy decisions to an authorized artifact remain
outside the policy model.

## Artifact Admission v1

`admit_polyglot_artifact` validates one configured external executable before an
invocation adapter may consider it trusted. The request must contain a canonical
capability ID, an explicit nonempty allowed-root list in the existing
external-process policy, and an exact 64-character lowercase SHA-256. Admission
first applies the existing path, executable, signature, and publisher policy;
it then hashes the resolved file and revalidates its physical identity after
the read. Stable `polyglot.artifact.*` codes distinguish invalid identity,
missing root, policy denial, hashing failure, digest mismatch, and detected
change. Any denial clears the trusted digest and revokes the embedded process
authorization.

The result is an opaque, non-default-constructible token. Immediately before a
future execution boundary, `revalidate_polyglot_artifact_admission` repeats the
original external-process policy, requires the same physical file identity,
rehashes the exact bytes, and revalidates identity again after hashing. A
failure revokes the token in place and a revoked token cannot be restored by a
later revalidation. SHA-256 file reads use fixed 64 KiB chunks rather than
buffering the whole artifact in memory; Windows uses incremental BCrypt and
other native hosts use the portable incremental SHA-256 implementation.

Focused coverage includes standard SHA-256 vectors, valid rooted admission,
noncanonical capability/digest rejection, missing and sibling roots, digest
mismatch, same-file content mutation, and identical-byte physical replacement.
Exact candidate `e5f974df2` passes Linux Native `31354311198` and macOS Native
`31354312629` at `332/332`, the macOS four-locale SET POINT matrix at `8/8`,
and Windows Native `31354314025` at `331/331`; the artifact-admission
regression passes on every host. All eight candidate-head protected checks
pass.
This admission seam does not itself launch a process. The adjacent adapter
below now connects it to one bounded candidate invocation, but route selection,
fallback execution, PRG dispatch, and language-specific runtime adapters remain
separate work.

## Artifact Invocation Adapter v1

`invoke_polyglot_artifact` composes one opaque admitted-artifact token with the
v1 request serializer, bounded child-process transport, strict response
admission, bridge outcome policy, and migration telemetry. The invocation
capability must exactly equal the admitted canonical capability. A denied token,
capability mismatch, or invalid request fails before telemetry, so untrusted
identity data cannot enter the migration event stream. A valid bridge policy is
limited to exactly one attempt; this adapter never retries.

The adapter revalidates policy, physical identity, and exact artifact bytes
immediately beside its owned `run_bounded_process` call. The serialized request
is sent as exact stdin bytes using a complete explicit environment; stdout and
stderr remain separate and bounded. Cancellation, timeout, invalid process
configuration, launch failure, output overflow, nonzero exit, malformed or
identity-mismatched response, candidate error, success, and a late valid
response are mapped to stable adapter status/error values and bridge telemetry.
The result preserves the exact request document, bounded-process evidence,
admitted response, bridge decision, and telemetry. A configured fallback may be
reported by that decision but is never executed here.

Local GCC focused and adjacent contracts pass `8/8`, and the adapter regression
repeats `25/25`. Clang 21 ASan/UBSan passes, focused static analysis reports no
diagnostic, and the test is portable, parallel-safe, synthetic, filesystem
process-owned, and network-free. Exact candidate `0ac427755` passes Linux Native
`31363043514` and macOS Native `31363043490` at `333/333`, the macOS four-locale
`SET POINT` matrix at `8/8`, and Windows Native `31363043544` at `332/332`;
the focused adapter regression passes on every host and all eight candidate-head
protected checks pass.

This boundary does not choose a route, invoke native or shadow behavior,
execute a fallback, expose PRG dispatch, call mutable runtime state, or embed a
language runtime. Revalidation materially narrows the validation-to-use window,
but path-based execution is not an atomic handle-bound launch; that stronger
operating-system binding remains separate work.

## Route Execution Coordinator v1

`execute_polyglot_route` applies one validated route registry to a canonical
capability and deterministic `0..99` selection sample. Native work is a
synchronous caller-owned callback invoked on the calling thread. Candidate work
can enter only through `invoke_polyglot_artifact`, retaining its exact identity
revalidation, one-attempt process ownership, bounded transport, strict response
admission, cancellation, timeout, and invariant bridge evidence.

Propagated cancellation is terminal and never falls back. When policy
explicitly ignores cancellation, the bridge decision instead follows its
configured fail-fast, native-fallback, or unsupported second-artifact outcome;
the coordinator does not misreport ignored cancellation as propagated.

Lifecycle behavior is exact:

- `off` and a native-selected `canary` invoke native once and no candidate;
- `shadow` invokes native and candidate once each, always keeps native
  authoritative, and passes caller-normalized values to the parity comparator;
- a candidate-selected `canary` and `on` invoke candidate once as primary;
- candidate failure invokes native once only when both the route decision and
  bridge decision allow native fallback; and
- `retire-legacy` never invokes native, even when candidate policy selects it.

The coordinator never retries and never executes a second-artifact fallback.
Its result retains route decision, native and candidate results, parity,
authority, exact invocation counts, and whether native fallback actually ran.
Stable `polyglot.execution.*` identifiers classify invalid configuration and
terminal outcome. Route, adapter/bridge, parity, and final status evidence are
one ordered stream. Existing `polyglot.fallback.applied` means policy selected
a fallback; `polyglot.fallback.executed` separately proves that the coordinator
actually invoked native.

Missing or malformed in-memory registry state, noncanonical capability, sample
outside `0..99`, missing candidate admission, or a missing required native or
shadow-normalizer callback fails before either path. Native and normalization
exceptions are contained with invariant failure codes. A mismatched or changed
artifact token is still rejected by the adapter and cannot be bypassed here.

Local GCC focused and adjacent contracts pass `9/9`, and the focused regression
repeats `20/20`. Clang 21 ASan/UBSan and focused static analysis pass. The
portable isolation record is parallel-safe, synthetic, process-owned,
bounded-child, and network-free. Exact candidate `7aa16ffc4` passes Linux Native
`31415789574` and macOS Native `31415789554` at `334/334`, the macOS locale
matrix at `8/8`, and Windows Native `31415789688` at `333/333`; the focused
regression passes on every host and all eight candidate-head protected checks
pass.

This seam does not itself expose PRG dispatch, introduce a second task
lifecycle, permit inline foreign-language source, host a language runtime, or
allow a foreign worker to call mutable Copperfin runtime state. The adjacent
PRG boundary below can delegate to a host callback but does not provision this
coordinator. Retry, second-artifact fallback, discovery, dependency
installation, and atomic handle-bound launch remain separately reviewable work.

## Trusted Runtime Host Composition v1

`PolyglotRuntimeHost::create()` is the single reviewed production seam that
turns trusted route/artifact configuration into the callback accepted by
`PrgRuntimeSession::create()`. A capability binding contains the canonical
capability ID, one opaque admitted-artifact token, one existing
`PolyglotArtifactInvocationRequest` template, and the trusted native and shadow
normalization callbacks required by its route. The template explicitly owns
bridge policy, protocol version, correlation prefix, executable arguments,
working directory, environment, and byte/time budgets.

Construction reconstructs and validates the registry, requires a one-to-one
route/binding set, and rejects missing, duplicate, extra, or mismatched
capabilities; rejected admission; empty/invalid correlation or protocol
identity; mutable pre-bound arguments/cancellation callbacks; invalid bridge
policy or multiple attempts; and missing route-required callbacks. These
failures occur before a callback is made available and before external launch.
Unknown capabilities also return a zero-invocation unavailable result.

The returned callback shares ownership of the validated registry and bindings,
so it remains valid after the public host handle is released. It binds exact
immutable PRG request data and delegates once to `execute_polyglot_route()`.
Same-capability calls serialize around the admission token because immediate
revalidation updates that opaque token in place; separate capabilities remain
independent. A per-capability monotonic sequence extends the configured prefix
into a unique correlation ID, including concurrent calls. Only the existing
runtime result fields cross back into PRG: status/reason, authority, route
selection, invocation counts, fallback-executed flag, and the authoritative
JSON payload. The host adds no request, correlation, process, telemetry, or
mutable-runtime fields of its own; the authoritative candidate payload remains
artifact-controlled and therefore may itself contain values such as a
correlation ID when that capability's public contract permits them.

Portable focused coverage proves all five route states, both canary choices,
parity, fail-fast/native-fallback outcomes, propagated and ignored
cancellation, exact error/count/authority mapping, callback lifetime,
concurrent correlation uniqueness, output bounds, telemetry redaction, and
fail-closed configuration. A real spawned PRG session uses the production
callback and `CFTASKCANCEL()` to stop a bounded synthetic candidate through the
read-only probe. Local GCC Release focused and adjacent targets pass `6/6`, the
focused target repeats `20/20`, Clang 21 ASan/UBSan passes with leak detection,
ThreadSanitizer passes, and focused static analysis is clean. Exact product/test
candidate `b66e6085f` passes Linux Native `31453166584` and macOS Native
`31453166506` at `336/336`, Windows Native `31453166516` at `335/335`, the
focused regression on every host, the macOS locale matrix at `8/8`, and all
eight candidate-head protected checks without a blocking result.

Native payload bytes cross from this host into the PRG result only when the
native callback reports success. A failed native-authoritative result retains
its status, reason, authority, selection, invocation counts, and fallback
evidence while publishing an empty payload, matching the existing candidate
failure behavior. Focused coverage supplies nonempty synthetic native-failure
bytes, and a mutation check confirms the former unconditional copy fails that
regression. No current native invoker was known to populate failure payloads;
the guard is fail-closed preparation for future adapters. Local Release
focused and adjacent tests pass `5/5`, the focused target repeats `20/20`, and
Clang 21 ASan/UBSan passes with leak detection. Exact product/test head
`edbcecfeb` passes Linux Native `31465221620` and macOS Native `31465221684` at
`337/337`, Windows Native `31465221686` at `336/336`, the focused regression on
every host, the macOS locale matrix at `8/8`, and all eleven protected checks.

The host does not discover artifacts or language runtimes, authorize inline
foreign source, retry, promote routes, invoke a second artifact, introduce a
task model, or permit a foreign worker to enter mutable PRG state. Artifact
execution retains the documented path-based revalidation boundary and does not
claim atomic handle-bound launch.

## .NET Native AOT Leaf Candidate v1

`samples/polyglot-dotnet-candidate` implements the fixed
`samples.dotnet.add-v1` capability in C#. Publishing for a supported
Windows/Linux/macOS x64/arm64 RID produces one self-contained Native AOT
executable containing the candidate and its required .NET runtime. The host
does not search for or install a CLR, and no loose managed/runtime sidecar is
admitted.

The candidate accepts exactly one bounded invocation envelope on standard
input, requires exact unique identity and `left`/`right` fields, and emits one
matching success or typed error envelope. The production adapter supplies an
empty environment and one bounded attempt. The integration regression hashes
and admits the exact published executable, invokes it through
`PolyglotRuntimeHost`, and runs ordinary PRG through `CFPOLYGLOTDISPATCH()` and
`CFJSONGET()`. Rebuilding with another SDK, runtime pack, source revision, RID,
or option creates a different artifact that must be rehashed and readmitted.

This proves one real external .NET target, not general CLR hosting, inline C#,
reflection, arbitrary assembly loading, dependency resolution, or execution of
generated transpilation output. See `docs/43-dotnet-polyglot-candidate.md`.

Local Release focused and adjacent tests pass `7/7`, the focused target repeats
`20/20`, Clang 21 ASan/UBSan passes with leak detection, and managed
formatting/analyzers plus focused C++ static analysis are clean. Exact
product/test candidate `4e66813d2` passes Linux Native `31459407580` and macOS
Native `31459407582` at `337/337`, Windows Native `31459407816` at `336/336`,
the focused regression on every host, and the macOS four-locale matrix at
`8/8`. Generated Launcher Validation `31459366674` separately passes the
candidate on Windows, Ubuntu, and macOS, and all eleven protected PR checks
pass at documentation head `d0c7ef408`.

## PRG Polyglot Dispatch Boundary v1

`CFPOLYGLOTDISPATCH()` is the PRG-facing orchestration contract. It accepts one
canonical capability ID, one bounded UTF-8 JSON object, and an optional exact
`0..99` selection sample. Invalid requests fail before work. A configured host
callback receives only those immutable values and a read-only cancellation
probe; it cannot enter mutable PRG state. Its result is validated and serialized
as one deterministic JSON evidence document with invariant status, reason,
authority, route, invocation-count, fallback, and payload fields.

The callback executes synchronously on the current PRG task. Blocking it inside
`ENTER CRITICAL` is rejected by the shared concurrency policy. A caller that
wants nonblocking orchestration places the call in `SPAWN` and supervises it
with `CFTASKSTATUS()`, `CFTASKCANCEL()`, `CFTASKRESULT()`, and `AWAIT`.
Arguments and payload bytes are excluded from runtime events.

This boundary is unavailable unless a host injects the callback. The trusted
runtime-host composition above now supplies one from explicit admitted state,
but the PRG boundary itself does not discover artifacts, load a runtime,
authorize an executable, retry, or permit foreign threads to call the runtime.
See `docs/42-prg-polyglot-dispatch.md` for the full machine contract and current
evidence.

## Bounded Artifact Process Primitive

`run_bounded_process` is the first execution prerequisite for an artifact-invoked
bridge. It starts an absolute executable directly with an argument vector and explicit
absolute working directory; it never invokes a shell. Embedded NUL bytes in paths,
arguments, or environment values are rejected before launch. The request supplies the
complete child environment, and an empty environment is the secure default so
build-host or agent secrets are not inherited implicitly. Environment names are
validated and duplicates are rejected using platform semantics.

The call owns one process tree. Windows starts the root suspended, assigns it to a
kill-on-close Job Object, and only then resumes it. POSIX creates a new process group
and uses a close-on-exec status pipe to distinguish successful `execve` from pre-exec
failure. Both paths poll a finite timeout and a prompt cancellation callback, fail
closed if that callback throws, return invariant status/error identifiers, and remove
descendants after timeout, cancellation, launch failure, or normal root exit.

Standard output and error are captured independently as exact bytes. Each positive
budget defaults to 1 MiB and cannot exceed the fixed 16 MiB hard ceiling. Dedicated
readers drain both streams concurrently so filling one pipe cannot prevent the other
from making progress. An overflow retains only that stream's admitted prefix, returns
`output-limit-exceeded` with `polyglot.process.stdout_limit_exceeded` or
`polyglot.process.stderr_limit_exceeded`, and closes the complete owned tree. Timeout
and cancellation retain bytes captured before shutdown. Windows restricts inherited
handles with `PROC_THREAD_ATTRIBUTE_HANDLE_LIST` to the stdin pipe reader and two
output pipe writers; unrelated inheritable host/agent handles remain outside the child. POSIX readers drain
nonblocking and accept an explicit shutdown signal, so a retained writer cannot make
capture cleanup wait indefinitely.

Caller-supplied stdin is delivered as exact bytes under an independent positive 1 MiB
default and the same fixed 16 MiB hard ceiling. Its writer runs concurrently with both
readers, closes stdin immediately for empty input, and fails closed if the child closes
stdin before consuming the complete request. POSIX writes are nonblocking. macOS
applies descriptor-local `F_SETNOSIGPIPE`; other POSIX writers block `SIGPIPE`
only in the writer thread and consume only their own `EPIPE` signal while
preserving the prior thread state. Windows shutdown cancels outstanding synchronous
writer I/O. Timeout and cancellation keep their precedence and cannot be delayed by a
child that leaves a 1 MiB request unread. Request bytes are never staged in a temporary
file.

Focused synthetic tests cover exact exit status, shell-metacharacter arguments, a
Unicode working path and environment value, absence of ambient `PATH`, live and
throwing cancellation, timeout cleanup, cleanup after normal root exit, missing
executables, invalid budgets, and duplicate environment names. Local GCC, Clang, and
ASan/UBSan executions pass, together with the native test-isolation contract. Exact
implementation/test head `36f25b1e5` passes Linux Native `324/324`
(`31284210937`), macOS Native `324/324` plus its existing four-locale matrix
(`31284210940`), and Windows Native `323/323` (`31284210974`); each full suite
executes this regression. The earlier macOS textual-path assertion was corrected to
compare filesystem identity for equivalent `/var` and `/private/var` spellings without
weakening exact argv, Unicode environment, or ambient-`PATH` isolation coverage.

The capture extension adds exact binary/CRLF tests, 256 KiB simultaneous stream
saturation, independent overflow, retained pre-timeout/pre-cancellation bytes, and
invalid output budgets. Focused GCC passes and repeats 20 times; Clang 21 ASan/UBSan
and ThreadSanitizer pass without findings, and focused analyzer checks are clean.
Exact candidate head `93350a1a6` passes Linux Native `31334334063` and macOS
Native `31334334101` at `331/331`, the macOS four-locale SET POINT matrix at
`8/8`, and Windows Native `31334334089` at `330/330`; the bounded-process
regression passes on every platform. All eight candidate-head protected checks pass.

The input extension adds exact binary echo, 256 KiB simultaneous three-pipe
saturation, premature-close failure, unread-input timeout/cancellation, and invalid
input-budget cases. Focused GCC passes with package/isolation coverage (`3/3`) and the
bounded-process target repeats 20 times. Clang 21 ASan/UBSan and ThreadSanitizer
pass. Focused analyzer coverage reports no diagnostic in the owned files and one
pre-existing dead store in unchanged `query_translator.cpp`. Exact candidate head
`301d74bf5` passes Linux Native `31340579206` and macOS Native `31340580178` at
`331/331`, the macOS four-locale SET POINT matrix at `8/8`, and Windows Native
`31340581094` at `330/330`; the bounded-process regression passes on every
platform. All eight candidate-head protected checks pass.

This primitive does **not** itself authorize or hash an artifact. The adapter
above now composes admission, request serialization, this single-attempt
transport, response admission, bridge decisions, and migration telemetry.
Route selection, native/shadow execution, fallback execution, retries, PRG
dispatch, atomic handle-bound launch, and external-language adapters remain
separately reviewable work.

## Invocation Request Serialization v1

`serialize_polyglot_invocation_request` emits one compact UTF-8 JSON document
with the fixed field order `envelope_version`, `kind`, `capability_id`,
`correlation_id`, `protocol_version`, and `arguments`. Version `1.0` uses the
invariant kind `invocation`. Capability IDs and semantic protocol versions use
the same machine grammar as the migration contract. Correlation IDs are
required, validated UTF-8 strings and are JSON-escaped without localization.

The caller supplies an arguments object as JSON. Before serialization, the
object must pass complete grammar, UTF-8/escape, unique-key-at-every-depth,
object-shape, trailing-byte, byte-budget, and nesting-budget checks. Its bytes,
including admitted whitespace and escape choices, are embedded exactly; this
layer does not reorder its keys or coerce/localize its values. The complete
outer document uses a 1 MiB/32-level default budget and non-disableable 16
MiB/64-level ceilings. Stable `polyglot.request.*` codes classify every failure.

The checked-in request fixture and focused GCC/Clang tests prove fixed ordering,
repeat-byte determinism, escaping and Unicode, exact argument preservation,
malformed/ambiguous/non-object rejection, and byte/depth limits. Clang
ASan/UBSan and the native isolation contract also pass at implementation commit
`1cecc2c8e`; focused static analysis reports no findings.

Exact candidate head `282d3a3e5` passes Linux Native `31292774286` and macOS
Native `31292774288` at `325/325`, Windows Native `31294519761` at `324/324`,
and `test_polyglot_interop_envelope` on every platform. The macOS run also
passes both `SET POINT` targets under `C`, `en_US.UTF-8`, `pt_BR.UTF-8`, and
`de_DE.UTF-8` (`8/8`). All eight protected PR checks pass.

This serializer alone does not choose or authorize an artifact, send the
request, capture process output, or dispatch an external route from PRG. The
artifact invocation adapter above now composes it with admission and transport.
The separate task-supervision seam keeps FP/VFP source in control of existing
`SPAWN` workers through bounded status, cancellation, result, and completed
print-output retrieval; a future PRG-facing artifact route must reuse that
lifecycle, and foreign runtime threads must not enter mutable Copperfin runtime
state directly.

## Response Envelope Admission v1

`parse_polyglot_interop_envelope` is the next host-safe seam for #91/#4700. It
accepts already-captured response bytes and admits only the success/error shapes
defined by the checked-in v1 examples. The parser validates JSON grammar, raw and
escaped UTF-8, unique keys at every object depth, exact top-level and structured-error
fields, success-payload versus error exclusivity, machine error-code syntax, and the
`1.0` envelope version. Capability id, correlation id, and semantic protocol version
must exactly match the invocation expectations, preventing cross-capability,
cross-request, and cross-protocol response confusion.

The default admission budget is 1 MiB and 32 container levels. Callers can select a
smaller budget, but cannot exceed the hard 16 MiB and 64-level ceilings. A successful
payload must be an object and remains exact validated JSON bytes so this seam does not
invent type coercion or localization. Candidate error code, message, and retryability
are returned separately; the message is untrusted display data and is not rendered by
this layer. Stable `polyglot.envelope.*` machine codes classify admission failures.

Focused coverage consumes the checked-in success/error examples and rejects malformed,
ambiguous, oversized, over-nested, identity-mismatched, and wrong-shape responses under
GCC, Clang, and Clang ASan/UBSan. Hosted Linux/macOS runs `31287594685` and
`31287594735` pass `325/325` at implementation head `171a1652b`. Test-only final
head `09284457e` preserves either LF or CRLF fixture bytes, explicitly covers both
forms under fresh local GCC/Clang builds, and passes hosted Windows Native
`31289912992` at `324/324`, including this regression; all eight final-head
protected checks pass. Together, the adjacent request serializer and this response
parser still do not send requests, capture process output, authorize/hash artifacts,
connect to the runtime host or PRG dispatch, apply fallback, or execute a route.
Those remain separately reviewable work.

## Shadow Parity v1

Shadow comparison is caller-neutral: the comparison result always preserves the native
return path. A comparison can inspect native/candidate success state, invariant error
codes, typed fields, field order, and bounded mismatch samples. Exact values are the
default; integer/number fields may use an explicit non-negative tolerance, and order
comparison can be disabled explicitly. Mismatch categories and telemetry reasons are
machine identifiers, not localized text.

Every comparison emits `polyglot.parity.checked`; a mismatch additionally emits
`polyglot.parity.mismatch` with the capability ID, first reason, and aggregate count.
Samples are bounded by policy and include the capability ID and reason. This is a
deterministic comparator contract only; it does not route traffic, invoke a candidate,
or change the caller's native behavior.

## Migration Event Taxonomy v1

The adapter-neutral telemetry stream uses the same `category`/`detail` shape as the
runtime event stream, with invariant fields for capability, reason, latency, and
mismatch count:

| Category | Required meaning | Key fields |
| --- | --- | --- |
| `polyglot.route.selected` | Route registry selected native, shadow, canary, on, or retire-legacy | `capability_id`, `reason_code`, `detail` |
| `polyglot.fallback.applied` | A configured native or artifact fallback was selected | `capability_id`, `reason_code`, `detail` |
| `polyglot.fallback.executed` | The route coordinator actually invoked native fallback | `capability_id`, `reason_code`, `detail` |
| `polyglot.parity.checked` | Native/candidate comparison completed | `capability_id`, `reason_code`, `mismatch_count` |
| `polyglot.parity.mismatch` | Comparison found a parity difference | `capability_id`, `reason_code`, `mismatch_count` |
| `polyglot.latency.outcome` | Invocation latency or terminal outcome was classified | `capability_id`, `reason_code`, `latency_ms`, `detail` |
| `polyglot.execution.completed` | One route execution reached a terminal authoritative outcome | `capability_id`, `reason_code`, `detail` |

These event names, fields, reason codes, and enum values are machine contracts and are
not localized. The telemetry helpers only record evidence; the coordinator above is
the separate component that applies a route and never alters existing PRG runtime
events by itself.

## Route Impact Recommendation v1

`evaluate_polyglot_route_impact` consumes one policy and exactly one
already-captured measurement for direct C++, a C++/.NET wrapper, and a C#
service. Hard gates require runtime availability, explicit security approval,
a permitted security profile, contract compatibility, enough samples, zero
failures and parity mismatches, and compliance with p95 latency, throughput,
peak-memory, and p95-startup budgets. Eligible routes receive a deterministic
integer weighted score; the lowest score is preferred and the remaining
eligible routes form its fallback chain. Canonical route order breaks exact
ties, so input order and locale cannot alter the machine decision.

Invalid or wholly ineligible evidence returns no recommendation and retains the
direct-C++/native no-promotion default. The evaluator is advisory only: it does
not run workloads, invoke either route, mutate `PolyglotRouteRegistry`, or
advance `off`/`shadow`/`canary`/`on` state. Human review remains mandatory.
The complete contract, failure boundary, and remaining benchmark-runner gap are
recorded in `docs/44-polyglot-route-impact.md`.

## Developer Migration Playbook v1

Migrations are performed one leaf capability at a time. A leaf capability has one
stable capability ID, one contract revision, one candidate artifact, and one rollback
owner. Do not promote a group of capabilities as a single change when their contracts,
failure modes, or evidence differ.

### Prerequisites

Before changing a route state, record all of the following in the migration evidence:

- the capability ID and contract/protocol versions
- native and candidate artifact or source hashes
- the route and bridge policy revisions
- the expected parity fields, error codes, and latency budget
- the owner, date, environment, and rollback contact

The native path must pass its existing focused tests before a migration starts. The
candidate must pass contract validation and produce bounded, machine-readable events.
No state transition is automatic; a human or an approved release process must review
the evidence and select the next state.

### State transitions and gates

| Transition | Required evidence and promotion threshold | Rollback action |
| --- | --- | --- |
| `off` -> `shadow` | Contract validation passes; native baseline passes; candidate can be invoked without changing the native return path; zero contract violations or crashes | Return to `off` and preserve the failed candidate evidence |
| `shadow` -> `canary` | Shadow sample meets the declared parity policy; zero unclassified mismatches; `p95` latency is within the configured budget; no new native failures | Set canary percentage to `0` or return to `off` |
| `canary` -> higher percentage | The current percentage meets the same parity, latency, error, and fallback thresholds over the declared sample window; no unexplained `polyglot.fallback.applied` events | Lower the percentage to the last passing value or return to `off` |
| `canary` -> `on` | The `100%` canary window passes the declared thresholds and the rollback owner accepts the evidence; candidate and native error identities remain contract-compatible | Return to `canary` or `off` |
| `on` -> `retire-legacy` | The candidate has passed the complete migration window, no native fallback is required, and the owner has archived the final evidence | Return to `on`; use `off` only when the candidate itself is unsafe |

The default safe threshold is zero contract violations, zero crashes, and zero
unclassified parity mismatches. Latency must remain at or below the configured
budget at the declared percentile, and candidate error/fallback rates must not exceed
the approved baseline or a stricter capability-specific threshold. A threshold may be
made stricter for a capability; relaxing it requires a recorded compatibility exception
and reviewer approval. Known VFP9 behavior exceptions are not silently converted into
polyglot parity passes.

### Rollback procedure

1. Stop promotion and capture the latest invariant telemetry, candidate hash, route
   configuration, policy configuration, and failing request or parity sample.
2. Select `off` for crashes, contract violations, security failures, or unexplained
   behavior. Select the last passing canary percentage for an isolated rate or latency
   regression when the native path remains healthy.
3. Re-run the native baseline and the focused candidate/parity tests before attempting
   another promotion. Do not delete failed evidence.
4. Classify the failure as a candidate defect, contract incompatibility, environment
   problem, or documented VFP9 exception. Reopen the migration only after the owner
   records the correction and a fresh artifact hash.

For a `retire-legacy` rollback, restore `on` first so the native path is again an
approved reactive fallback. Use `off` when the candidate must not receive traffic.
Route state, reason codes, capability IDs, and telemetry categories remain invariant;
only operator-facing summaries use localized display text.

### Example leaf migration

For `reports.invoice.render`, begin with `off`, validate the contract and native
baseline, then select `shadow`. After the declared shadow window has zero unclassified
mismatches and stays within the report latency budget, promote through `canary` at
the approved percentages, for example `10`, `50`, and `100`. Review each window before
the next change, then select `on` and finally `retire-legacy` only after the complete
evidence package is archived. Any failed gate follows the rollback procedure rather
than silently advancing the route.

### Evidence package

Each transition records the route decision, `polyglot.latency.outcome`, parity events,
fallback events, test results, sample counts, percentile calculations, artifact hashes,
configuration revisions, and reviewer decision. Machine fields are stored exactly as
emitted; localized summaries are supplementary and must not replace the invariant
record. The package is sufficient for a later operator to reproduce the decision or
restore the previous route state.

## .NET Story

This is the primary secondary ecosystem.

Copperfin should support:

- calling managed components from Copperfin
- exposing Copperfin logic to managed callers
- producing managed wrappers around native modules
- shipping native executables that are still first-class `.NET` consumers
- generating `.NET`-consumable outputs such as wrappers or SDK packages

## Python Story

Python should be a sidecar and job model, not the product core.

Strong use cases:

- data-science workloads
- analytics
- migration helpers
- batch transformations
- experimentation

Recommended rule:

- run Python out-of-process
- make it policy-controlled and auditable
- do not let it become the trusted runtime boundary

Current concrete workflow:

- `samples/polyglot-python-sidecar/candidate.py` implements one strict
  `samples.python.add-v1` leaf using only the standard library
- the host starts an explicitly configured interpreter with `-I -S`, a
  complete child environment, bounded I/O/time/process-tree ownership, and no
  shell
- the script path cannot be substituted, moved, linked, replaced, or edited
  after admission without rejection before interpreter launch
- ordinary PRG remains the control plane through `CFPOLYGLOTDISPATCH()` and
  can inspect the immutable result with `CFJSONGET()`

See `docs/46-python-polyglot-sidecar.md` for the operational and trust contract.

## R Story

R should follow the same boundary as Python.

Strong use cases:

- statistical workloads
- reproducible research pipelines
- analyst-authored reports
- data-frame and model-serving jobs

Recommended rule:

- run R out-of-process
- keep it policy-controlled and auditable
- treat it as a sidecar/job boundary rather than the trusted runtime

Current concrete workflow:

- `samples/polyglot-r-sidecar/candidate.R` implements one strict
  `samples.r.mean-v1` leaf using base R only
- the host starts an explicitly configured `Rscript` with `--vanilla`, a
  complete environment containing only `R_DEFAULT_PACKAGES=base`, bounded
  I/O/time/process-tree ownership, and no shell
- the exact script is admitted beneath a physical root, bound to argument
  index 1, and rejected before launch if substituted or changed
- a closed 1 MiB/32-depth/4,096-value reader rejects duplicate members,
  malformed Unicode, non-finite numbers, and noncanonical envelope shapes
- ordinary PRG remains the control plane through `CFPOLYGLOTDISPATCH()` and
  can inspect the immutable mean result with `CFJSONGET()`

See `docs/47-r-polyglot-sidecar.md` for the operational and trust contract.

## Other Language Story

Acceptable extension modes:

- stable C ABI modules
- Rust native libraries
- policy-controlled external processes

Not every language needs first-class embedding.

The important thing is a stable, signed, well-audited extension model.

## MCP And AI Story

Copperfin now has a first dedicated, provider-independent MCP host surface:
`copperfin_mcp_host` exposes one local, read-only DBF-header inspection tool.
It accepts caller-supplied bytes only and has no caller-selected file, network,
model, shell, extension-loading, or mutable product-state access. The process
also enforces the existing `ai.mcp` role permission and emits content-free
tool-call audit events. See
[`48-mcp-read-only-dbf-header-host.md`](48-mcp-read-only-dbf-header-host.md).
Broader MCP tooling can eventually let developers use their preferred AI
models and assistants for:

- code generation
- migration help
- report/query design assistance
- diagnostics
- documentation lookup
- debugger assistance

Developers should be able to choose the model used for AI-assisted debugging and coding help, subject to organization policy.

Recommended rules:

- opt-in only
- provider-agnostic
- policy-controlled
- auditable
- able to use local or enterprise models
- allow user-selected models within administrator-approved provider/model policy
- keep ordinary relational query execution deterministic so AI is optional for the straightforward path
- reserve AI planning for ambiguous document/vector query synthesis, explanation, and debugging help

The shipped DBF-header tool satisfies the first opt-in, provider-independent,
deterministic execution boundary. `cf_security` now also has a portable local
workspace-agent access policy with advisory, sandboxed-workspace, and warned
unrestricted modes. Unrestricted activation requires a dedicated high-risk
permission, trusted product UI, audit availability, the exact current warning,
and affirmative consent; provider authentication is deliberately not an
authorization input. See
[`64-workspace-agent-access-policy.md`](64-workspace-agent-access-policy.md).
Model/provider adapters, OAuth clients, the mutable executor and sandbox, and
the user-facing assistant/dialog surfaces remain unimplemented.

## Why This Matters

This is how Copperfin answers two different pressures at once:

- keep a fast native product core
- avoid becoming isolated from the ecosystems developers actually use now
