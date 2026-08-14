# Workspace-Agent Product Tool Registry Traceability Report

Date: 2026-08-14

## Scope and requirement

This slice implements the non-executing product-owned tool registry in
`RQ-CF-AGENT-008`. The requirement derives from the repository owner's
workspace-assistant policy and from `RQ-CF-AGENT-007`, which requires complete
capability requirements to come from a product definition rather than provider
or model input. Existing code is not used as a requirement source.

Mapped architecture and code:

- `include/copperfin/security/workspace_agent_tool_registry.h`
- `src/security/workspace_agent_tool_registry.cpp`
- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_workspace_agent_tool_registry.cpp`
- `tests/test_workspace_agent_session.cpp`
- `docs/64-workspace-agent-access-policy.md`

## DQ/DV/HZ mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-workspace-agent-tool-registry-001`: define the exact stable product-owned tool inventory and complete capability mapping | `DV-workspace-agent-tool-registry-001`; `DV-workspace-agent-tool-registry-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-tool-registry-002`: prevent provider/model/workspace input from declaring weaker requirements or registering an alias | `DV-workspace-agent-tool-registry-001`; `DV-workspace-agent-tool-registry-002`; `DV-workspace-agent-tool-registry-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-tool-registry-003`: distinguish declarative preflight from execution, target containment, sandbox, and outcome audit | `DV-workspace-agent-tool-registry-002`; `DV-workspace-agent-tool-registry-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-tool-registry-001`: focused registry regression proves
  exact inventory, complete mappings, no elevation, exact lookup, and rejection
  of aliases, future/provider IDs, whitespace, case changes, and embedded NULs.
- `DV-workspace-agent-tool-registry-002`: focused session regression proves
  registered resolution across advisory, workspace-sandbox, and unrestricted
  modes plus inactive, transition, stale-generation, and post-stop denial.
- `DV-workspace-agent-tool-registry-003`: focused Release policy, registry,
  session, and persistent-sink selection passes `4/4`; community and native
  isolation contracts pass `2/2`; the safety contract passes `1/1`; fresh
  Clang ASan/UBSan registry/session execution passes `2/2`; and
  `git diff --check` passes. Protected exact-head evidence remains to be
  recorded.

## Procedural delta map

- Before: the public preflight request carried caller-selected capability
  booleans and relied on a future executor to obtain them from an unspecified
  product definition.
- After: the public request carries only schema, exact generation, and a stable
  tool ID. Native immutable lookup supplies the complete capability set, and
  unknown or modified IDs fail closed before session admission.

Potential Severity If Misused: medium

## Hazard and misuse analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Capability understatement: the request has no capability fields. The session
  controller resolves the full native definition and compares every required
  bit atomically with the active session snapshot.
- Registry substitution: definitions are fixed native data; provider config,
  model output, workspace files, and prompts cannot register or mutate them.
- Identifier confusion: exact case-sensitive lookup rejects unknown versions,
  aliases, surrounding whitespace, slash variants, and embedded-NUL suffixes;
  rejected input is not copied into the result.
- Invalid product definition: compile-time validation requires canonical unique
  IDs, a nonempty set, and no privilege elevation for every entry.
- Overbroad operation: workspace-sandbox sessions cannot admit local/outside or
  network definitions. Unrestricted-local admission still requires the existing
  warning and consent policy and never obtains elevation.
- Target confusion: tool identity does not establish that a path, process, or
  endpoint is safe. A future executor must canonicalize and contain the actual
  target beside every side effect.
- Token confusion: registered preflight remains a point-in-time decision. It is
  not a lease or execution token and cannot preserve a stopped generation.
- Missing outcome evidence: no handler runs, so no outcome is claimed. Actual
  execution must commit separately defined attempt and outcome audit evidence.

## Boundary and rollback

This registry does not access a file, apply an edit, delete data, launch a
process, use the network, authenticate a provider, activate a session, display
consent UI, or record a tool outcome. Its operation names are declarations for
future controlled handlers, not implementations or proof of target safety.
Target containment, executor, real sandbox, provider/OAuth adapter, trusted
activation UI, outcome audit, diff, and undo remain explicit gaps.

Rollback is code-local: restore the capability-bearing preflight request,
remove the registry source/header and focused registry test, and restore the
mapped documentation. This slice creates no provider, workspace, user, or
persistent state.

## Verification

Local Release verification on Linux currently records:

- warning-free build of `test_workspace_agent_policy`,
  `test_workspace_agent_tool_registry`, `test_workspace_agent_session`, and
  `test_workspace_agent_audit_sink`;
- focused CTest selection: `4/4` pass;
- repository-community and native-isolation contracts: `2/2` pass;
- safety-traceability workflow contract: `1/1` pass in `321.35` seconds;
- fresh Clang 21 ASan/UBSan registry and session execution with leak detection:
  `2/2` pass with no finding;
- `git diff --check`: pass;
- no file, process, network, provider, or product-UI action by the new registry;
- protected Windows, Ubuntu, and macOS execution pending.

The focused regressions cover exact inventory and ordering, complete capability
sets, permanent no-elevation, exact and embedded-NUL lookup, provider/future ID
rejection, mode matrices, inactive sessions, zero and stale generations,
transitions, revocation, and post-stop denial.

## Assurance statement

This is DO-178C-inspired development assurance adapted to a general-purpose
C++/.NET platform. It is not a claim of DO-178C compliance, certification, an
assigned software level, or suitability for a safety-critical deployment.

## Review evidence

- mode: maintainer self-review
- reviewer: rhamenator
- verification: immutable inventory, complete requirement mapping, identifier
  rejection, no-elevation invariant, session binding, non-execution boundary,
  rollback, and requirements/code/test mapping
- verification result: passed for the local candidate
- automated evidence: focused Release `4/4`, community/isolation `2/2`, safety
  contract `1/1`, fresh Clang ASan/UBSan `2/2`, and `git diff --check`;
  protected exact-head execution pending
- automated evidence result: passed for all listed local candidate checks;
  protected evidence is a separate admission gate
- scope: medium-severity non-executing workspace-agent product tool registry
- result: candidate approved for protected verification as maintainer
  self-review; no independence claim
