# Workspace-Agent Process Target Containment Traceability Report

Date: 2026-08-14

## Scope and requirement

This implementation defines the non-executing process and working-directory
target preflight in `RQ-CF-AGENT-010`. The requirement derives from explicit
repository-owner product policy, `RQ-CF-AGENT-001`, `RQ-CF-AGENT-007`,
`RQ-CF-AGENT-008`, and the process-target confusion, system-failure, and data-
corruption hazards. Existing Copperfin code is not used as its requirement
source.

Mapped architecture, code, and tests:

- `docs/64-workspace-agent-access-policy.md`
- `include/copperfin/security/workspace_agent_process_containment.h`
- `src/security/workspace_agent_process_containment.cpp`
- `include/copperfin/security/workspace_agent_tool_registry.h`
- `src/security/workspace_agent_tool_registry.cpp`
- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_workspace_agent_process_containment.cpp`
- `tests/test_workspace_agent_target_containment.cpp`
- `tests/test_workspace_agent_tool_registry.cpp`
- `tests/test_workspace_agent_session.cpp`

## DQ/DV/HZ mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-workspace-agent-process-target-001`: define exact-session, registered process-class, executable, working-directory, and product-root boundaries | `DV-workspace-agent-process-target-001`; `DV-workspace-agent-process-target-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-target-002`: define strict workspace/local spellings, physical identity, POSIX execute eligibility, and fail-closed diagnostics | `DV-workspace-agent-process-target-001`; `DV-workspace-agent-process-target-002`; `DV-workspace-agent-process-target-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-target-003`: distinguish target inspection from PATH search, command parsing, arguments, environment, sandboxing, launch, and outcome audit | `DV-workspace-agent-process-target-002`; `DV-workspace-agent-process-target-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-process-target-001`: focused boundary regression covers
  valid workspace-root/nested and unrestricted-local targets; relative versus
  absolute confusion; dot/traversal and embedded-NUL forms; missing and wrong-
  kind targets; POSIX execute permission; symlink indirection; hard links;
  local indirect working-directory leaves; and both same-identity symlink-back
  and different-identity workspace-root replacement where supported.
- `DV-workspace-agent-process-target-002`: focused controller regression covers
  inactive, unconfigured, sandbox, unrestricted, stale, stopped, invalid-
  schema, unknown-tool, insufficient-capability, and wrong-target-class
  behavior without denied-target reflection. Compile-time assertions prevent
  the target request from acquiring command, argument, or environment fields.
- `DV-workspace-agent-process-target-003`: focused Release security selection
  passes `7/7`; fresh Clang ASan/UBSan session/registry/file/process selection
  passes `4/4`; community and native-isolation contracts pass `2/2`; and the
  safety contract passes `1/1`. Exact-head protected execution passes all
  eleven required checks, and thread-aware review reports zero threads.

## Procedural delta map

- Before: a registered process tool could pass capability preflight, but native
  code had no product-root-bound way to classify its executable and working
  directory.
- After: one exact-session, registered process-class preflight inspects only
  explicit paths and returns point-in-time identities without searching,
  parsing, constructing, sandboxing, or launching anything.

Potential Severity If Misused: medium

## Hazard and misuse analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Caller-selected workspace root: rejected by construction. Product code
  creates the controller boundary; the request contains no root field.
- PATH or shell confusion: no bare-name search, shell command, argument vector,
  or environment exists in the request. A bare name denotes only an explicit
  file at the workspace root and otherwise fails.
- Path confusion: workspace targets are strict relative paths, with exact `.`
  reserved only for the workspace-root working directory. Local targets are
  strict absolute paths; Windows device, alternate-stream, and UNC remote-share
  syntax fails while endpoint containment remains unimplemented.
- Physical redirection: workspace inspection rejects symlink/reparse and
  cross-device components, canonical escape, multiply linked executables, and
  replacement or redirection of the configured root identity. Local target
  leaves must be direct and executable identity must be single-link.
- Kind and launch confusion: executables must be regular targets;
  working directories must be directories. POSIX candidates require execute
  permission. Windows launch-format compatibility remains an executor result,
  not a claim made by metadata preflight.
- Capability/session confusion: registered-tool admission runs before target
  inspection and again after both inspections. Sandbox denial therefore cannot
  probe local targets, and stale/stopped generations return no paths.
- Time-of-check/time-of-use: results are explicitly non-authoritative. A future
  executor must repeat checks beside launch, pin or revalidate targets, build a
  fixed explicit secret-free environment, apply the sandbox, and audit outcome.

## Boundary and rollback

This implementation performs filesystem metadata inspection only. It does not read
an executable, inspect or inherit process environment, accept arguments, search
`PATH`, invoke a shell, start a process, use the network, activate a product UI,
authenticate a provider, apply a sandbox, or record a tool outcome. Launch-
adjacent handle pinning/revalidation, argument and environment policy, process
execution, endpoint policy, outcome audit, provider/OAuth, trusted activation
UI, diff, and undo remain explicit gaps.

Rollback is code-local: remove the process-containment module and focused test,
remove the controller process preflight, and restore the mapped documentation.
The implementation creates no provider, workspace, user, process, or persistent
product state.

## Verification

Current local evidence:

- warning-free focused GCC Release build;
- security, policy, session, registry, file-target, process-target, and
  persistent-sink selection: `7/7` pass;
- fresh Clang 21 ASan/UBSan session, registry, file-target, and process-target
  selection with leak detection: `4/4` pass with no finding;
- repository-community and native-test-isolation contracts: `2/2` pass; the
  generated isolation inventory covers `374` native tests;
- safety-traceability workflow contract: `1/1` pass;
- exact signed/DCO implementation head `4020d70a4` passes all eleven protected
  checks in runs `31858578884`, `31858578779`, `31858578771`, `31858578807`,
  and `31858578790`; both Socket checks pass;
- implementation PR `#5006` merged as `7cb1499ce`;
- follow-up review identified that Windows accepts forward-slash and mixed-
  separator UNC roots in addition to backslash UNC roots. Exact signed/DCO
  correction head `a6f9044f3` structurally rejects all three forms and directly
  regresses executable and working-directory variants. It passes all eleven
  protected checks in runs `31860551945`, `31860553045`, `31860553024`,
  `31860553090`, and `31860553147`; both Socket checks pass;
- correction PR `#5008` merged as `53c51c433`; its sole review thread is
  resolved and outdated. No independent final review is claimed.

## Assurance statement

This is DO-178C-inspired development assurance adapted to a general-purpose
C++/.NET platform. It is not a claim of DO-178C compliance, certification, an
assigned software level, or suitability for a safety-critical deployment.

## Review evidence

- mode: medium-severity maintainer self-review; no independent final review
- reviewer: rhamenator
- verification: product-root ownership, path grammar, physical identity,
  registry/session binding, non-execution boundary, rollback, and
  requirements/code/test mapping
- verification result: passed for exact corrected implementation head
  `a6f9044f3`
- automated evidence: focused Release `7/7`, fresh Clang ASan/UBSan `4/4`,
  community/isolation `2/2`, safety `1/1`, and all eleven exact-head protected
  checks; both Socket checks pass; the forward-slash/mixed-separator UNC review
  finding is directly regressed and its sole thread is resolved
- automated evidence result: passed
- scope: medium-severity non-executing process target containment
- result: accepted for `RQ-CF-AGENT-010`; no independence claim
