# Workspace-Agent Session Lifecycle Traceability Report

Date: 2026-08-14

## Scope and requirement

This slice implements the non-executing native lifecycle boundary in
`RQ-CF-AGENT-005`, derived from the repository owner's workspace-assistant
policy and `RQ-CF-AGENT-001`. It consumes the existing activation policy; it
does not infer requirements from the preexisting implementation.

Mapped architecture and code:

- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_workspace_agent_session.cpp`
- `docs/64-workspace-agent-access-policy.md`

## DQ/DV/HZ mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-workspace-agent-session-001`: describe the fail-closed audited start, immutable admitted authority, immediate revocation, and non-executing boundary without implying that provider, UI, executor, or sandbox functionality exists | `DV-workspace-agent-session-001`; `DV-workspace-agent-session-002`; `DV-workspace-agent-session-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-session-002`: describe policy-dependency failure as an audited denial that grants no authority and does not permanently wedge the transition | `DV-workspace-agent-session-001`; `DV-workspace-agent-session-002`; `DV-workspace-agent-session-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-session-003`: define the content-free audit event as locale-independent versioned JSON whose numeric fields and escapes remain machine-readable under process-global locale changes | `DV-workspace-agent-session-001`; `DV-workspace-agent-session-002`; `DV-workspace-agent-session-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-session-001`: focused Release policy, session,
  localization, and native-isolation regressions pass `4/4`.
- `DV-workspace-agent-session-002`: fresh Clang 21 ASan/UBSan policy/session
  execution with leak detection passes `2/2` with no finding.
- `DV-workspace-agent-session-003`: safety-traceability, community-health, and
  diff-validation contracts pass locally; protected exact-head execution and
  review remain pending below.

## Procedural delta map

- Before: the lifecycle text covered policy admission, audit receipts,
  capability binding, replacement denial, and revoke-before-audit behavior, but
  did not state what happens if policy evaluation itself throws.
- After: the policy and traceability text state that dependency failure becomes
  a content-free audited denial, grants no authority, restores the transition,
  and is directly exercised by a bounded regression.

Potential Severity If Misused: medium

## Hazard and misuse analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Audit bypass: start fails closed unless policy admits the request and the
  audit sink returns success with a nonempty receipt.
- Authority substitution: a second start cannot replace or expand an active
  session; the admitted generation, mode, and capability snapshot remain bound.
- Failed revocation evidence: stop clears authority before calling the audit
  sink. A missing, failing, empty-receipt, or throwing sink is reported but
  cannot extend the session.
- Sensitive audit content: the versioned event contains only lifecycle kind,
  generation, requested/effective mode, outcome, and diagnostic code. It has no
  prompt, path, credential, provider token, workspace content, or receipt.
- Locale-dependent machine output: serialization explicitly uses the classic
  locale, preventing process-wide digit grouping from corrupting JSON numbers
  or control-character escapes.
- Provider/authority confusion: provider authentication is outside this API and
  is not an activation input.
- Reentrant or concurrent transition: one mutex-guarded transition is admitted
  at a time; an active snapshot is not partially replaced.
- Policy dependency failure: an exception while resolving or evaluating policy
  becomes a content-free audited denial, grants no authority, and restores the
  transition so one failure cannot permanently deny later valid activation.

## Boundary and rollback

This controller does not execute tools, read or write files, launch processes,
use the network, authenticate a provider, persist an audit record, display a
consent dialog, or expose activation through a CLI. It therefore does not yet
make the workspace assistant operational. The mutable executor, real sandbox,
OAuth/provider adapter, persistent audit sink, diff/undo, product stop control,
session indicator, and trusted consent UI remain explicit gaps.

Rollback is code-local: remove the session header/source from `cf_security`,
remove the focused test and its isolation declaration, and restore the mapped
documentation rows. No provider, user, workspace, or persistent session state
is created by this slice.

## Verification

Local Release verification on Linux:

- `cmake --build build --target test_workspace_agent_session test_workspace_agent_policy -j2`: pass, warning-free for the changed targets.
- focused CTest selection covering session, policy, localization, and native
  test isolation: `4/4` pass.
- Clang 21 ASan/UBSan build plus leak-detected focused policy/session CTest:
  `2/2` pass with no sanitizer finding.
- `git diff --check`: pass.

The focused test directly proves admitted sandbox capabilities, committed
receipt binding, active-session replacement denial, revocation-before-callback,
inactive snapshot clearing, policy-denial auditing, stale-warning denial,
missing/failing/empty/throwing audit behavior, and stable content-free JSON.
The serialization case installs a process-global every-digit grouping locale
and proves generation `1234567` plus a control-character escape remain canonical
JSON rather than inheriting localized separators.
It also blocks one start audit on a worker thread under a five-second test
bound and proves
that an overlapping unrestricted start sees no partial authority and cannot
replace or expand the pending sandbox session.
On POSIX, a bounded deleted-working-directory fixture deterministically forces
catalog-backed policy evaluation to throw; the regression proves an audited
`workspace_agent.policy_evaluation_failed` denial, absent authority, transition
recovery, successful later start, and immediate revocation. Its isolation
declaration records the unique test-owned filesystem and scoped process
environment mutations instead of claiming no filesystem access.

Protected Windows, Ubuntu, and macOS execution and exact-head review are still
pending. Until those results exist, `RQ-CF-AGENT-005` remains `gap`, not
`defined`.

## Assurance statement

This is DO-178C-inspired development assurance adapted to a general-purpose
C++/.NET platform. It is not a claim of DO-178C compliance, certification, an
assigned software level, or suitability for a safety-critical deployment.

## Review evidence

- mode: maintainer self-review
- reviewer: rhamenator
- verification: fail-closed policy-dependency behavior, transition recovery,
  authority absence, audit observability, isolation truth, boundary wording,
  rollback, and requirements/code/test mapping
- verification result: passed
- automated evidence: focused Release `4/4`, Clang ASan/UBSan `2/2`, safety
  traceability workflow contract, repository community-health contract, and
  `git diff --check`
- automated evidence result: passed
- scope: medium-severity workspace-agent lifecycle documentation and native
  security-boundary correction
- result: approved as maintainer self-review; no independence claim
