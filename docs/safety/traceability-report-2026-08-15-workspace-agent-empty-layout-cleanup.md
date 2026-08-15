# Workspace-Agent Empty-Layout Cleanup Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-020` portable, explicit, identity-receipted
cleanup of an empty private workspace-agent generation layout

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-014`,
`RQ-CF-AGENT-016`, `HZ-system-failure-01`, and `HZ-data-corruption-01`

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, complete lifecycle cleanup, nor suitability for a
safety-critical deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-layout-cleanup-001`: successful preparation shall return the complete physical identity of the exact new session directory and all five fixed children; failed or incomplete preparation shall return no cleanup receipt | `DV-workspace-agent-layout-cleanup-001`; `DV-workspace-agent-layout-cleanup-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-layout-cleanup-002`: cleanup shall require a successful receipt, the unchanged trusted storage root, and exact full identity matches for the session directory and every child before the first mutation | `DV-workspace-agent-layout-cleanup-001`; `DV-workspace-agent-layout-cleanup-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-layout-cleanup-003`: cleanup shall remove only the five exact empty private child directories in reverse order and then their exact empty session root; it shall never recurse, remove content, adopt an existing generation, or derive authority from a generation number alone | `DV-workspace-agent-layout-cleanup-001`; `DV-workspace-agent-layout-cleanup-002` | `HZ-data-corruption-01` |
| `DQ-workspace-agent-layout-cleanup-004`: denial and outcome diagnostics shall be stable and content-free, and the primitive shall not be invoked automatically by session start or stop | `DV-workspace-agent-layout-cleanup-002`; `DV-workspace-agent-layout-cleanup-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-layout-cleanup-005`: lifecycle integration shall remain unavailable until the trusted host durably retains the receipt, records cleanup intent and outcome, supports retry after partial cleanup, and defines retention for owned nonempty content | `DV-workspace-agent-layout-cleanup-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-layout-cleanup-001`: platform tests prove exact empty
  removal and preservation on parent mismatch, target mismatch, ambiguity, and
  nonempty content.
- `DV-workspace-agent-layout-cleanup-002`: environment tests compare the
  returned receipt with independently observed full identities, prove complete
  cleanup, and preserve occupied or replaced layouts.
- `DV-workspace-agent-layout-cleanup-003`: source contracts, broader
  safety/isolation tests, sanitizer execution, protected Windows/Ubuntu/macOS
  matrices, exact-head review, and retained results are required before
  implementation evidence is complete.

## Requirement delta

- Before: preparation returned only a generation number, so later trusted-host
  cleanup had no exact object receipt and remained wholly unimplemented.
- After: successful preparation returns the exact session and child identities,
  and a separate explicit primitive can remove only that same empty layout.
  The session controller does not call it.

Potential Severity If Misused: high

## Hazard, misuse, boundary, and rollback analysis

- Data preservation: an added file changes the containing directory identity
  and fails preflight. Even if content appears after preflight, the platform
  empty-directory operation fails without traversing or deleting content.
- Replacement: the trusted storage root, session, and every child must match
  the receipt before mutation. A generation number or current path alone is
  never cleanup authority.
- Windows boundary: the parent is held without delete sharing, the target is
  opened with delete authority and reparse-point semantics, and deletion is
  requested against the verified target handle.
- POSIX boundary: parent and target are opened descriptor-relatively with
  no-follow semantics, and `unlinkat(..., AT_REMOVEDIR)` removes only an empty
  direct child. POSIX does not provide the handle-delete primitive used here on
  Windows, so a same-authority leaf-name replacement between descriptor
  verification and `unlinkat` remains a documented race. The operation never
  traverses content, but automatic sandbox lifecycle use remains prohibited.
- Atomicity: five child removals plus the session-root removal are not one
  transaction. A late mismatch or error can leave a partially cleaned empty
  layout. The primitive stops immediately and never rolls back by recreating
  paths, because recreation could adopt changed authority.
- Audit and retry: the receipt is currently in-memory and the primitive emits
  only a result. Durable receipt retention, intent/outcome audit, idempotent
  retry policy, and owned nonempty-content disposition remain explicit gaps.
- Information exposure: diagnostics contain no path, identity, file name,
  content, prompt, credential, or provider data.
- Rollback: removing the explicit cleanup method leaves the existing
  fail-closed preparation/start behavior intact; no lifecycle path depends on
  cleanup success.

## Verification

Current local focused evidence:

- warning-free GCC Release platform, environment, and session targets build;
- platform cleanup, platform source-contract, isolated-environment, and session
  tests pass `4/4`; and
- the broader Release workspace-agent, Studio-host, community, isolation, and
  workflow-contract selection passes `17/17`;
- fresh Clang 21 ASan/UBSan with leak detection passes the platform,
  environment, and session selection `3/3`; and
- the repository-wide safety-traceability workflow contract passes `1/1` in
  322.33 seconds; and
- `git diff --check` passes.

Protected cross-platform execution, exact-head review, and merge evidence
remain pending. Requirement status is therefore `gap`; no lifecycle cleanup or
launch readiness is claimed.
