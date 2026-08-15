# Workspace-Agent Process Invocation-Shape Traceability Report

Date: 2026-08-14

## Scope and requirement

This slice defines and implements the non-executing invocation-shape preflight
in `RQ-CF-AGENT-011`. The requirement derives from explicit repository-owner
product policy, `RQ-CF-AGENT-001`, `RQ-CF-AGENT-007`, `RQ-CF-AGENT-008`,
`RQ-CF-AGENT-010`, and command/environment confusion, system-failure, and
data-corruption hazards. Existing Copperfin code is not used as its requirement
source.

Follow-on status: this report remains the exact-head evidence for
`RQ-CF-AGENT-011`. The later `RQ-CF-AGENT-012` boundary implements fixed-key
logical environment construction; its distinct evidence and remaining secure-
layout/platform-serialization gaps are recorded in
`traceability-report-2026-08-15-workspace-agent-isolated-environment.md`.

Mapped architecture, code, and tests:

- `docs/64-workspace-agent-access-policy.md`
- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_workspace_agent_process_invocation.cpp`
- `tests/test_workspace_agent_process_containment.cpp`
- `tests/CopperfinTestIsolation.cmake`

## DQ/DV/HZ mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-workspace-agent-process-invocation-001`: define direct-vector rather than command/shell semantics, exact bounds, encoding, and no-reflection denial | `DV-workspace-agent-process-invocation-001`; `DV-workspace-agent-process-invocation-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-invocation-002`: define the non-inheriting isolated-session environment policy and prohibit caller-selected environment input | `DV-workspace-agent-process-invocation-001`; `DV-workspace-agent-process-invocation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-invocation-003`: distinguish point-in-time planning from platform serialization, environment construction, sandboxing, endpoint enforcement, execution, and outcome audit | `DV-workspace-agent-process-invocation-002`; `DV-workspace-agent-process-invocation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-process-invocation-001`: compile-time and focused
  regressions prove the request has a direct argument vector but no command,
  caller environment, or PATH-search field; empty, spaced, UTF-8, wildcard,
  and shell-metacharacter arguments remain distinct literal elements.
- `DV-workspace-agent-process-invocation-002`: focused boundary regression
  covers count, per-element, aggregate, embedded-NUL, malformed-UTF-8,
  inactive, stale, sandbox/outside-target, and warned-unrestricted behavior.
  Every denial returns no argument, tool, executable, or working-directory
  content.
- `DV-workspace-agent-process-invocation-003`: focused Release security,
  policy, session, registry, file/process target, process-invocation, and audit-
  sink selection passes `8/8`; community, isolation, sanitizer, safety, and
  protected exact-head execution provide the broader evidence.

## Procedural delta map

- Before: process preflight could bind only an executable and working directory;
  argument shape and environment inheritance remained undefined.
- After: a separate exact-session preflight binds a bounded literal UTF-8
  argument vector to that existing target result and requires the versioned
  non-inheriting `isolated_session_v1` environment policy without constructing
  an environment or executing anything.

Potential Severity If Misused: medium

## Hazard and misuse analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Shell/command confusion: executable identity is separate, and each argument
  stays one literal vector element. Shell metacharacters, whitespace, globs,
  and empty arguments are not parsed or expanded.
- Resource exhaustion: version 1 admits no more than 64 arguments, 4,096 UTF-8
  bytes per element, or 8,192 bytes in total. Overflow-safe subtraction checks
  the aggregate before addition or copying into an allowed result.
- Encoding/truncation confusion: malformed UTF-8 and embedded NUL fail with
  stable diagnostics. The future executor must still enforce the platform's
  serialized-command limit and lossless encoding beside launch.
- Credential/environment confusion: the request has no environment field, the
  allowed environment-policy query disables parent-environment inheritance, and the
  selected profile requires fresh session-owned product-controlled values.
  Exact profile construction and cleanup remain unimplemented executor work;
  provider tokens and credentials must remain out of band. This preflight does
  not classify arbitrary argument content and does not claim it is secret-free.
- Target/session confusion: the existing registered-tool and physical-target
  preflight runs first. After bounded argument copying, registered-tool and
  exact-session admission run again. A denial exposes neither target nor
  argument content.
- Audit/content confusion: arguments may contain user content and must not be
  copied into lifecycle or future outcome audit records. Only stable,
  content-free outcome metadata may be persisted.
- Time-of-check/time-of-use: the result is explicitly non-authoritative. A
  future executor must repeat session, registry, target, identity, invocation,
  platform-limit, environment, sandbox, endpoint, and outcome checks beside
  the controlled launch.

## Boundary and rollback

This implementation performs existing metadata inspection plus bounded in-
memory argument validation and copying. It does not read an executable,
construct or inherit an environment, serialize a Windows command line, invoke
a shell, start a process, attach standard streams, use the network, apply a
sandbox, authenticate a provider, activate a product UI, or record a tool
outcome.

Rollback is code-local: remove the invocation request/result and controller
method, remove the focused test and isolation declaration, and restore the
mapped documentation. The implementation creates no provider, credential,
workspace, file, process, user, network, or persistent product state.

## Verification

Current local evidence:

- warning-free GCC Release focused build;
- security, policy, session, registry, existing-file target, process-target,
  process-invocation, and persistent-sink selection: `8/8` pass;
- fresh Clang 21 ASan/UBSan session, registry, file-target, process-target, and
  invocation selection with leak detection: `5/5` pass with no finding;
- repository-community and native-test-isolation contracts: `2/2` pass;
- the generated isolation inventory covers `375` native tests;
- safety-traceability workflow contract: `1/1` pass;
- `git diff --check` passes;
- exact signed/DCO implementation head `487699f3f` passes all eleven protected
  checks in runs `31863899694`, `31863899698`, `31863899695`, `31863899697`,
  and `31863899696`; both Socket checks pass;
- implementation PR `#5010` merged as `7361f76e0`; thread-aware GitHub review
  reports zero threads;
- independent read-only review at exact head `487699f3f` passed the focused
  eight-test workspace-agent selection and found no defect in bounds/overflow,
  UTF-8, denial clearing, the final session recheck, sensitive-content wording,
  or the non-execution boundary.

## Assurance statement

This is DO-178C-inspired development assurance adapted to a general-purpose
C++/.NET platform. It is not a claim of DO-178C compliance, certification, an
assigned software level, or suitability for a safety-critical deployment.

## Review evidence

- mode: medium-severity maintainer self-review; no independent final review
- reviewer: rhamenator
- verification: direct-vector grammar, bounds, UTF-8/NUL handling, target and
  session binding, non-inheriting environment selector, non-execution boundary,
  rollback, and requirements/code/test mapping
- verification result: passed for exact signed/DCO implementation head
  `487699f3f`
- automated evidence: focused Release `8/8`, fresh Clang ASan/UBSan `5/5`,
  community/isolation `2/2`, safety `1/1`, all eleven exact-head protected
  checks, both Socket checks, zero GitHub threads, and independent focused
  read-only review pass
- automated evidence result: passed
- scope: medium-severity non-executing process invocation-shape preflight
- result: accepted for `RQ-CF-AGENT-011`; independent read-only review is
  recorded without claiming an independent final safety approval
