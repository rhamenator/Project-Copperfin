# Workspace-Agent Existing-File Target Containment Traceability Report

Date: 2026-08-14

## Scope and requirement

This candidate implements the non-executing existing-file target preflight in
`RQ-CF-AGENT-009`. The requirement derives from explicit repository-owner
policy, `RQ-CF-AGENT-001`, `RQ-CF-AGENT-007`, `RQ-CF-AGENT-008`, and the
filesystem target-confusion and data-corruption hazards. Existing Copperfin
code is not used as its requirement source.

Mapped architecture, code, and tests:

- `docs/64-workspace-agent-access-policy.md`
- `include/copperfin/security/workspace_agent_target_containment.h`
- `src/security/workspace_agent_target_containment.cpp`
- `include/copperfin/security/workspace_agent_tool_registry.h`
- `src/security/workspace_agent_tool_registry.cpp`
- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_workspace_agent_target_containment.cpp`
- `tests/test_workspace_agent_tool_registry.cpp`
- `tests/test_workspace_agent_session.cpp`

## DQ/DV/HZ mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-workspace-agent-target-001`: define the trusted product-owned workspace-root and registered target-kind boundary | `DV-workspace-agent-target-001`; `DV-workspace-agent-target-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-target-002`: define strict workspace/local path forms, physical containment, identity, and fail-closed diagnostics | `DV-workspace-agent-target-001`; `DV-workspace-agent-target-002`; `DV-workspace-agent-target-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-target-003`: distinguish point-in-time inspection from handle-pinned execution, mutation, and outcome audit | `DV-workspace-agent-target-002`; `DV-workspace-agent-target-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-target-001`: focused target regression covers valid
  workspace and unrestricted-local existing files plus invalid roots, absolute
  versus relative path confusion, dot/traversal and embedded-NUL forms,
  missing/directory targets, symlink indirection where supported, hard links
  where supported, workspace-root identity replacement, and replacement of the
  configured root pathname by a symlink back to the original identity.
- `DV-workspace-agent-target-002`: focused controller regression covers
  inactive, unconfigured, sandbox, unrestricted, stale, stopped, insufficient-
  capability, and wrong-target-class behavior without denied-path reflection.
- `DV-workspace-agent-target-003`: candidate focused Release security, policy,
  session, registry, target, and audit-sink selection passes `6/6`; fresh Clang
  ASan/UBSan session/registry/target execution passes `3/3`; and community plus
  native-isolation contracts pass `2/2`; and the safety contract passes `1/1`.
  Exact protected and review evidence are pending.

## Procedural delta map

- Before: a registered tool could pass capability preflight, but native code
  had no product-root-bound way to classify and inspect its actual file target.
- After: registered target kind and exact-session preflight select one strict
  workspace or local existing-file inspection path and return only canonical
  point-in-time identity after physical checks and a repeated session check.

Potential Severity If Misused: medium

## Hazard and misuse analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Caller-selected root: rejected by construction. Product code constructs the
  controller boundary from the trusted absolute workspace root; the target
  request contains no root field.
- Path confusion: workspace file tools accept only strict relative targets;
  local file tools accept only strict absolute targets. Empty, dot/traversal,
  rooted workspace, relative local, trailing-empty, and embedded-NUL forms
  fail without a canonical-path result.
- Physical redirection: workspace inspection rejects symlink/reparse and
  cross-device components, canonical escape, multiply linked targets, and
  replacement of the configured root identity.
- File-kind confusion: missing paths and directories fail. Registry target
  kinds prevent process and endpoint tools from entering the file boundary.
- Capability/session confusion: registered-tool admission runs before target
  inspection and again afterward. Sandbox denial therefore cannot probe a
  local file, and stale/stopped generations do not retain a path result.
- Time-of-check/time-of-use: the result is explicitly non-authoritative. It
  does not reserve the path or perform I/O. A future executor must repeat the
  checks and pin the physical target through an OS handle beside the operation.

## Boundary and rollback

This candidate performs filesystem metadata inspection and uses a short-lived
attributes handle on Windows, but does not read file content, create, write,
rename, delete, execute, or reserve a target. It does not authenticate a provider, activate a product UI,
implement a sandbox, or record a tool outcome. Prospective-file containment,
handle-pinned read/write/delete/rename, process and working-directory
containment, endpoint policy, executor, outcome audit, provider/OAuth, trusted
activation UI, diff, and undo remain explicit gaps.

Rollback is code-local: remove the target module and focused test, restore the
registry definitions without target kinds, remove the controller target
preflight, and restore the mapped documentation. The candidate creates no
provider, workspace, user, or persistent product state.

## Verification

Current local Release evidence:

- warning-free focused build after correcting candidate-only compiler warnings;
- security, policy, session, registry, target-containment, and persistent-sink
  selection: `6/6` pass;
- fresh Clang 21 ASan/UBSan session, registry, and target selection with leak
  detection: `3/3` pass with no finding;
- repository-community and native-test-isolation contracts: `2/2` pass; the
  generated isolation inventory covers `373` native tests;
- safety-traceability workflow contract: `1/1` pass;
- no product file content read or mutation by the new boundary;
- protected and review evidence pending.

## Assurance statement

This is DO-178C-inspired development assurance adapted to a general-purpose
C++/.NET platform. It is not a claim of DO-178C compliance, certification, an
assigned software level, or suitability for a safety-critical deployment.

## Review evidence

- mode: maintainer self-review candidate
- reviewer: rhamenator
- verification: trusted-root ownership, path grammar, physical identity,
  registry/session binding, non-execution boundary, rollback, and
  requirements/code/test mapping
- verification result: passed for the local candidate
- automated evidence: focused Release `6/6`, fresh Clang ASan/UBSan `3/3`,
  community/isolation `2/2`, and safety `1/1`; protected evidence pending
- automated evidence result: pending
- scope: medium-severity non-executing existing-file target containment
- result: approved as maintainer self-review; no independence claim
