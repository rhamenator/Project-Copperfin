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

- .NET integration currently exists as an early generated-launcher path: the native runtime pipeline can spawn a generated C# stub as a child process, but generated C# transpilation output is not executed by the runtime host.
- A portable bounded-process primitive now provides direct executable/argument
  invocation, an explicit child environment, timeout and cancellation handling,
  descendant cleanup, and independently bounded exact-byte stdout/stderr
  capture. It is infrastructure for a future artifact adapter, not a runtime
  route or language integration by itself.
- A portable serializer now emits the versioned invocation request in fixed,
  compact field order after strict identity and arguments-object admission; the
  matching response parser admits only the checked-in success/error shapes.
- Existing `SPAWN` tasks now have a nonblocking PRG supervision seam for
  status, cooperative cancellation, retained return values, and completed
  print output. Exact candidate head `09c1b1046` passes Linux Native
  `31307387144` at `326/326`, macOS Native `31307387146` at `326/326` plus
  four locales at `8/8`, and Windows Native `31307387195` at `325/325`; no
  external artifact adapter is connected to it yet.
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
- Python and broader polyglot support are planning/scaffolding surfaces only; there is no Python runtime hook today.
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

This primitive does **not** authorize or hash an artifact, validate migration
contracts or typed envelopes, automatically connect the request serializer or response
parser, implement retries or fallback, choose a route, emit migration telemetry, or
connect any external language to the PRG runtime. Those remain separately reviewable
adapter and dispatch work.

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

This is serialization only. It does not choose or authorize an artifact, write
or send the request, capture process output, or dispatch an external route from
PRG. The separate task-supervision seam now keeps FP/VFP source in control of
existing `SPAWN` workers through bounded status, cancellation, result, and
completed print-output retrieval; a later artifact adapter must reuse it, and
foreign runtime threads must not enter mutable Copperfin runtime state directly.

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
| `polyglot.parity.checked` | Native/candidate comparison completed | `capability_id`, `reason_code`, `mismatch_count` |
| `polyglot.parity.mismatch` | Comparison found a parity difference | `capability_id`, `reason_code`, `mismatch_count` |
| `polyglot.latency.outcome` | Invocation latency or terminal outcome was classified | `capability_id`, `reason_code`, `latency_ms`, `detail` |

These event names, fields, reason codes, and enum values are machine contracts and are
not localized. The telemetry model only records decisions; it does not make a route,
start a bridge, or alter existing PRG runtime events by itself.

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

## Other Language Story

Acceptable extension modes:

- stable C ABI modules
- Rust native libraries
- policy-controlled external processes

Not every language needs first-class embedding.

The important thing is a stable, signed, well-audited extension model.

## MCP And AI Story

Copperfin should eventually have a dedicated MCP host surface so developers can use their preferred AI models and assistants for:

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

## Why This Matters

This is how Copperfin answers two different pressures at once:

- keep a fast native product core
- avoid becoming isolated from the ecosystems developers actually use now
