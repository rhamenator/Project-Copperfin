# Workspace-Agent Tool Preflight Traceability Report

Date: 2026-08-14

## Scope and requirement

This slice implements the non-executing session-bound capability preflight in
`RQ-CF-AGENT-007`. The requirement derives from the repository owner's
workspace-assistant policy, `RQ-CF-AGENT-001`, and the audited lifecycle and
revocation boundary in `RQ-CF-AGENT-005`; existing code is not used as a
requirement source.

Mapped architecture and code:

- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_workspace_agent_session.cpp`
- `docs/64-workspace-agent-access-policy.md`

## DQ/DV/HZ mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-workspace-agent-tool-preflight-001`: distinguish a point-in-time, non-executing capability decision from a reusable authority or execution token | `DV-workspace-agent-tool-preflight-001`; `DV-workspace-agent-tool-preflight-002`; `DV-workspace-agent-tool-preflight-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-tool-preflight-002`: define exact-generation, complete-capability, transition, inactive-session, schema, and empty-request rejection | `DV-workspace-agent-tool-preflight-001`; `DV-workspace-agent-tool-preflight-002`; `DV-workspace-agent-tool-preflight-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-tool-preflight-003`: retain no-elevation, fresh side-effect-adjacent recheck, target validation, and actual-outcome audit as mandatory executor boundaries | `DV-workspace-agent-tool-preflight-001`; `DV-workspace-agent-tool-preflight-002`; `DV-workspace-agent-tool-preflight-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-tool-preflight-001`: focused Release policy, session,
  and persistent-sink regressions pass `3/3`.
- `DV-workspace-agent-tool-preflight-002`: fresh Clang ASan/UBSan session
  execution with leak detection passes `1/1` with no finding.
- `DV-workspace-agent-tool-preflight-003`: safety-traceability,
  community-health, diff, and maintainer self-review pass locally; protected
  exact-head checks are pending.

## Procedural delta map

- Before: the policy and lifecycle documentation bound capabilities to one
  audited session but defined no common way for a future tool adapter to ask
  whether its complete requirements match that live session.
- After: the documentation defines one mutex-serialized, exact-generation
  capability preflight while explicitly withholding execution-token status,
  target containment, side effects, and outcome-audit claims.

Potential Severity If Misused: medium

## Hazard and misuse analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Stale authority: generation zero and mismatched generations fail closed and
  do not disclose the current session identity.
- Partial capability checks: one request declares the complete capability set;
  one unavailable bit denies the whole request. A future trusted executor must
  derive that set from a product-owned tool definition rather than accepting a
  provider- or model-selected set.
- Transition race: preflight uses the controller mutex and denies while start
  or stop is incomplete. Stop clears authority before its audit callback.
- Privilege expansion: no current mode carries elevation authority, so every
  elevation request is denied, including unrestricted-local mode.
- Empty or future grammar: an empty request and every schema other than version
  1 fail closed with stable diagnostics.
- Token confusion: the result is documented and typed as preflight evidence,
  not durable authority. A future executor must recheck beside each side effect
  and cannot use this result to keep a stopped session alive.
- Content leakage: the request and result carry capability booleans, generation,
  effective mode, and a stable diagnostic only; they carry no prompt, path,
  command, credential, provider token, or user content.
- Missing target and outcome controls: this slice performs no path admission,
  launch, network operation, or tool audit. Those are explicit gates for the
  future executor rather than inferred from an allowed preflight.

## Boundary and rollback

This preflight does not execute a tool, access a workspace, launch a process,
use the network, authenticate a provider, display consent UI, retain a lease,
or record a tool outcome. The mutable executor, real sandbox, target-specific
containment, tool-outcome audit, provider/OAuth adapter, trusted activation UI,
product-owned tool registry, product stop/session controls, diff, and undo
remain gaps.

Rollback is code-local: remove the preflight request/result types and controller
method, remove the focused assertions, and restore the mapped documentation.
No provider, workspace, user, or persistent state is created by this slice.

## Verification

Local Release verification on Linux currently records:

- warning-free build of `test_workspace_agent_policy`,
  `test_workspace_agent_session`, and `test_workspace_agent_audit_sink`;
- focused CTest selection: `3/3` pass;
- fresh Clang 21 ASan/UBSan session execution with leak detection: `1/1` pass
  with no finding;
- safety-traceability workflow contract: pass;
- repository community-health contract: pass;
- `git diff --check`: pass.

The focused regression proves unknown-schema, empty-request, inactive-session,
zero-generation, mismatched-generation, capability-subset, mixed-capability,
no-elevation, post-stop, start-transition, and stop-transition behavior. It
also proves the sandbox and unrestricted mode matrices without executing any
tool. Exact-head protected evidence remains required before
`RQ-CF-AGENT-007` advances from `gap`.

## Assurance statement

This is DO-178C-inspired development assurance adapted to a general-purpose
C++/.NET platform. It is not a claim of DO-178C compliance, certification, an
assigned software level, or suitability for a safety-critical deployment.

## Review evidence

- mode: maintainer self-review
- reviewer: rhamenator
- verification: exact-generation behavior, complete capability comparison,
  transition and revocation handling, no-elevation invariant, non-token
  boundary, rollback, and requirements/code/test mapping
- verification result: passed for the local candidate
- automated evidence: focused Release `3/3`, Clang ASan/UBSan `1/1`, safety
  traceability workflow contract, repository community-health contract, and
  `git diff --check`; protected exact-head checks pending
- automated evidence result: passed for local evidence
- scope: medium-severity non-executing workspace-agent tool preflight
- result: approved as maintainer self-review for the local candidate; no
  independence claim
