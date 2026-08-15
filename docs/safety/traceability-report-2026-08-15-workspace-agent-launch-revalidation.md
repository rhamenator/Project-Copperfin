# Workspace-Agent Launch Revalidation Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-019` portable non-executing launch-adjacent
serialized-plan and executable-snapshot revalidation

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-010` through
`RQ-CF-AGENT-018`, `HZ-system-failure-01`, and `HZ-data-corruption-01`

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, launch readiness, or suitability for a safety-critical
deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-launch-revalidation-001`: compare every material field of the caller-held complete plan with fresh trusted preflight before and after executable inspection | `DV-workspace-agent-launch-revalidation-001`; `DV-workspace-agent-launch-revalidation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-launch-revalidation-002`: bracket the final complete preflight with bounded physically contained exact-executable snapshots, require admitted identities and equal lowercase SHA-256 digests, and return only the final equal digest on complete success | `DV-workspace-agent-launch-revalidation-001`; `DV-workspace-agent-launch-revalidation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-launch-revalidation-003`: denied, altered, stale, replaced, changed, excessive, or unavailable input returns no plan, digest, path, argument, or environment content | `DV-workspace-agent-launch-revalidation-001`; `DV-workspace-agent-launch-revalidation-002`; `DV-workspace-agent-launch-revalidation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-launch-revalidation-004`: the API remains non-executing and expressly preserves the post-return race, handle-pinning, sandbox, endpoint, descendant, and outcome-audit gaps | `DV-workspace-agent-launch-revalidation-002`; `DV-workspace-agent-launch-revalidation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-launch-revalidation-001`: focused controller tests prove
  successful exact-plan/digest binding and rejection of caller-plan alteration.
- `DV-workspace-agent-launch-revalidation-002`: source and documentation
  contracts preserve the no-launch, no-token, no-handle-pinning boundary and
  content-free denial.
- `DV-workspace-agent-launch-revalidation-003`: warning-free Release,
  sanitizer, broader safety/isolation, protected Windows/Ubuntu/macOS, diff,
  and exact-head review evidence are required before implementation evidence is
  complete.

## Requirement delta

- Before: a serialized invocation was bracketed internally, but a later
  launcher had no product API to prove that its caller-held plan still equaled
  current trusted session, target, environment, and parser state.
- After: the caller-held plan must exactly equal fresh trusted preflight before
  and after a bounded physical executable snapshot; success returns a fresh
  plan and its snapshot digest for immediate future consumption.

Potential Severity If Misused: high

## Hazard, misuse, boundary, and rollback analysis

- Plan substitution: every nested allow flag, diagnostic, session/mode/tool,
  canonical target and identity, direct argument, logical environment,
  serialized native environment/argument representation, platform, and parser
  contract must match; the caller's mutable plan is never returned as success.
- Executable mutation: the snapshot is physically contained under the direct
  parent, bounded to 512 MiB, checked against admitted complete identity, and
  bracketed by the full workspace/local target preflight. SHA-256 identifies
  the exact captured bytes returned with the fresh plan. A second snapshot
  after the final identity-based preflight must have equal identity and digest,
  preventing restored metadata from pairing an earlier digest with later bytes.
- Session revocation and target replacement: either complete preflight fails or
  differs, and denial returns no partial plan or digest.
- Resource exhaustion: the executable snapshot has an explicit 512 MiB cap;
  existing argument, environment, registry, and parser caps remain in force.
- Information exposure: all denials contain only stable diagnostic codes. They
  carry no path, argument, environment, prompt, credential, file bytes, or
  digest.
- Race boundary: this API retains no handle and cannot close mutation after it
  returns. A future controlled executor must consume the returned plan
  synchronously, minimize and account for that interval, and use platform-
  backed pinning where supported.
- Non-claims: no process is launched; no sandbox, endpoint control, descendant
  management, publisher validation, provider authentication, or outcome audit
  is added.
- Rollback: remove the result type and controller method and withdraw candidate
  `RQ-CF-AGENT-019`. Rollback must leave the future executor disconnected rather
  than treating earlier serialized preflight as launch authority.

## Verification

Completed local evidence:

- warning-free GCC Release workspace-agent selection passes `10/10`;
- focused integrated and source-contract verification passes `2/2`;
- fresh Clang 21 ASan/UBSan with leak detection passes parser, source contract,
  and integrated revalidation `3/3`;
- repository community, generated isolation, and safety gates pass `5/5`,
  including the 331-second safety traceability scan;
- native-platform and GitHub Actions workflow contracts pass; and
- `git diff --check` passes.

Protected Windows, Ubuntu, and macOS evidence, exact-head review, and final
retained commit identifiers remain pending.

The retained risk classification is `high`. No independent final safety
approval or launch readiness is claimed; requirement status remains `gap`.
