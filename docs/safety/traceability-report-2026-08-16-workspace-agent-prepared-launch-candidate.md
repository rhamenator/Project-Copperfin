# Workspace-Agent Prepared Launch-Candidate Traceability Report

Date: 2026-08-16

Scope: candidate v1 `RQ-CF-AGENT-025` non-executing exact-plan, authenticated-
pin, and revocation-lease composition

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-019`,
`RQ-CF-AGENT-022`, `RQ-CF-AGENT-023`, `RQ-CF-AGENT-024`,
`HZ-system-failure-01`, and `HZ-data-corruption-01`

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, launch readiness, nor suitability for a safety-
critical deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-prepared-launch-001`: only the trusted controller shall construct and retain the serialized invocation; callers shall not submit a purported admitted plan | `DV-workspace-agent-prepared-launch-001`; `DV-workspace-agent-prepared-launch-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-prepared-launch-002`: preparation shall acquire authenticated target pins and the exact-generation lease, repeat the complete serialization preflight, and require exact plan, retained-identity, and snapshot matches | `DV-workspace-agent-prepared-launch-001`; `DV-workspace-agent-prepared-launch-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-prepared-launch-003`: success shall return one opaque move-only candidate whose destruction closes pins before releasing its lease | `DV-workspace-agent-prepared-launch-001`; `DV-workspace-agent-prepared-launch-002`; `DV-workspace-agent-prepared-launch-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-prepared-launch-004`: composition shall remain non-executing and shall not weaken the invariant launch denial | `DV-workspace-agent-prepared-launch-001`; `DV-workspace-agent-prepared-launch-002`; `DV-workspace-agent-prepared-launch-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-prepared-launch-001`: focused controller regression proves
  inactive and stale denial, exact successful composition, moved-from opacity,
  stop blocking while live, orderly release, and unchanged launch denial.
- `DV-workspace-agent-prepared-launch-002`: compile-time contracts prove that
  the candidate is move-only and exposes no plan, arguments, environment,
  executable bytes, or native handle.
- `DV-workspace-agent-prepared-launch-003`: warning-free Release, sanitizer,
  broader safety/isolation, protected Windows/Ubuntu/macOS, diff, and exact-head
  review evidence are required before implementation evidence is complete.

## Hazard, misuse, boundary, and rollback analysis

- Caller-plan forgery or editing: the public preparation API accepts only the
  bounded invocation request. Both retained serialized plans are constructed
  inside the trusted controller and compared across every logical and platform
  representation field.
- Stop and revocation race: target pinning is followed by exact-generation
  lease acquisition and a complete final preflight. A concurrent stop makes
  preparation fail closed or waits for a successfully returned candidate.
- Target substitution: the final plan's executable and working-directory
  identities must match the still-open retained objects. The immutable byte
  snapshot is reauthenticated after that match; no mutable path is reopened as
  authority.
- Partial construction and allocation failure: pins and the lease remain under
  move-only local RAII ownership until candidate construction succeeds. Any
  exception returns a content-free unavailable result and releases both.
- Lifetime ordering: the candidate declares the lease before pins and plan, so
  reverse member destruction discards the plan, closes pins, and releases the
  lease last.
- Misuse and resource retention: a leaked or long-lived candidate can delay
  stop, and the issuing controller must outlive it. Trusted host code must
  retain it only around a future bounded direct launch operation and discard it
  on every denial or failure.
- Non-claims: this slice does not enter the working directory, materialize or
  execute the private snapshot, apply a sandbox, enforce endpoint or descendant
  policy, audit an outcome, or expose provider credentials. `RQ-CF-AGENT-019`
  remains invariantly denied.
- Rollback: remove the prepared-candidate class, result, controller method,
  private retained-identity matcher, and focused regressions together. The
  existing independent plan, pin, lease, and invariant-denial boundaries must
  remain fail closed.

Potential Severity If Misused: high

## Verification

Current local evidence:

- warning-as-error GCC Release workspace-agent policy, session, registry,
  process containment,
  parser, invocation, isolated environment, and audit-sink execution passes
  `8/8`;
- fifty repeated isolated-environment candidate-lifecycle executions pass;
- ASan/UBSan with leak detection and ThreadSanitizer each pass the session,
  process-containment, and isolated-environment targets `3/3` without findings;
- repository licensing, community, native-platform, GitHub Actions, isolation,
  supply-chain, and safety contracts pass `7/7`, including the 338.24-second
  safety scan;
- the new regression proves empty, inactive, stale, successful, moved-from,
  stop-blocking, orderly-release, and unchanged-launch-denial behavior; and
- `git diff --check` passes.

Protected Windows/Ubuntu/macOS, exact-head review, and merge evidence remain
pending. Requirement status
stays `gap`; no exact-snapshot execution, sandbox, endpoint/descendant, or
outcome-audit readiness is claimed, and no qualified high-severity human sign-
off is claimed.
