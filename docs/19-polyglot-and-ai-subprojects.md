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
- Python and broader polyglot support are planning/scaffolding surfaces only; there is no Python runtime hook today.
- .NET, Python, R, and other polyglot features should require a user-selected modernization target before they are exposed as product capabilities.

## Migration Contract v1

Polyglot modernization uses an artifact-first contract boundary. A candidate capability
must describe its input, output, error, invocation decision, execution budget, fallback
mode, reproducibility hashes, and observability events before a runtime route is exposed.
The canonical machine-readable schema and examples are maintained at:

- `docs/contracts/polyglot-migration-contract-v1.schema.json`
- `docs/contracts/polyglot-migration-contract-v1.json`
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
select native, or select the fallback artifact. The implementation records the policy's
attempt limit but does not claim that a process was started or retried; process
execution, cancellation tokens, and adapter-specific retry mechanics remain outside
this contract slice.

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
