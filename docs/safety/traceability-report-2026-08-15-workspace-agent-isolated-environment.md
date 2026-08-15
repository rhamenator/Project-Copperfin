# Workspace-Agent Isolated Environment Traceability Report

Date: 2026-08-15

Scope: v1 `RQ-CF-AGENT-012` fixed-key, generation-owned logical process
environment construction

Allowed requirement source: explicit repository-owner product policy under
H3/I2; derived from `RQ-CF-AGENT-001`, `RQ-CF-AGENT-007`,
`RQ-CF-AGENT-008`, `RQ-CF-AGENT-010`, `RQ-CF-AGENT-011`, and the listed
hazards

Implementation:

- `include/copperfin/security/workspace_agent_environment.h`
- `src/security/workspace_agent_environment.cpp`
- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_workspace_agent_isolated_environment.cpp`
- `tests/CMakeLists.txt`
- `tests/CopperfinTestIsolation.cmake`

This report records DO-178C-inspired development assurance adapted to a
general-purpose C++/.NET platform. It is not a claim of formal DO-178C
compliance, certification, an assigned software level, or suitability for a
safety-critical deployment.

## Derived and verification requirements

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-environment-001`: define a parent-independent fixed-key environment whose names and arbitrary values cannot come from provider/model/prompt/workspace/tool input | `DV-workspace-agent-environment-001`; `DV-workspace-agent-environment-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-environment-002`: bind the exact generation to physically identified trusted storage, direct profile/temp/config/cache/data layout, bounded approved PATH directories, and explicit Windows system root | `DV-workspace-agent-environment-001`; `DV-workspace-agent-environment-002`; `DV-workspace-agent-environment-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-environment-003`: define platform key sets, UTF-8/NUL/count/byte/delimiter/duplicate rules, deterministic ordering, locale-independent generation naming, and no-reflection denial | `DV-workspace-agent-environment-001`; `DV-workspace-agent-environment-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-environment-004`: distinguish logical environment construction from layout creation/ACL/cleanup, platform serialization, launch, sandbox/endpoint enforcement, provider credentials, and outcome audit | `DV-workspace-agent-environment-002`; `DV-workspace-agent-environment-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-environment-001`: focused regression proves that the
  existing request has no environment field; the host profile never inherits;
  POSIX emits exactly nine fixed keys and Windows exactly ten; profile,
  temporary, XDG/Windows application-data, locale, time-zone, system-root, and
  configured-order PATH values are exact; common credential keys are absent.
- `DV-workspace-agent-environment-002`: focused boundary regression covers
  invalid schema/policy/generation, empty/duplicate/excessive/delimiter-
  ambiguous configuration, wrong-platform system-root input, missing and
  indirect layout, stale session generation, approved-directory identity
  replacement, session-storage-root replacement, Windows system-root
  replacement, content-free denial, and deterministic ordering.
- `DV-workspace-agent-environment-003`: invocation and environment regressions,
  warning-free Release build, fresh Clang ASan/UBSan, community/isolation
  contracts, safety traceability, diff validation, protected Windows/Ubuntu/
  macOS exact-head execution, and thread-aware review are required.

## Requirement delta

- Before: `RQ-CF-AGENT-011` selected a non-inheriting profile but intentionally
  did not define or construct its concrete entries.
- After: trusted product-host configuration supplies only physically identified
  roots. Native code derives and constructs the exact generation's bounded,
  deterministic, fixed-key logical environment without consulting ambient
  variables or accepting caller-selected entries.

Potential Severity If Misused: medium

## Hazard, misuse, and boundary analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Credential exposure: parent variables are never enumerated or read. Fixed
  entry names exclude provider/OAuth tokens, GitHub/cloud credentials, SSH
  agent sockets, and Copperfin product-secret paths. Provider credentials must
  remain in the provider adapter and cannot be converted to environment values
  or process arguments.
- Environment injection: request, model, prompt, and workspace data have no
  environment field. Configuration accepts paths only; names, locale, time
  zone, layout leaves, and platform key sets are compiled product policy.
- Search-path confusion: version 1 requires 1–16 explicit absolute physically
  identified directories. Duplicate and platform-delimiter-ambiguous paths
  fail. Windows SystemRoot is not implicitly placed on PATH.
- Profile/temp escape: generation naming uses locale-independent unsigned
  decimal conversion. Only direct physically contained
  `session-N/{home,temp,config,cache,data}` directories are accepted;
  symlink/reparse and cross-device layout fails.
- Replacement and stale authority: configured and generation-owned identities
  are checked around construction, and the controller repeats the complete
  invocation preflight. The result is still explicitly point-in-time and must
  be revalidated beside launch.
- Encoding, truncation, and exhaustion: path and entry values require valid
  UTF-8 without NUL. Each `name=value` entry is bounded to 4,096 bytes, the
  profile to 32,768 bytes, and configured PATH directories to sixteen.
- Secret classification: structural exclusion prevents ambient credential
  keys, but the boundary does not inspect arbitrary path text and does not
  certify that a path is nonsensitive. Environment entries and arguments must
  not be persisted in content-free lifecycle or outcome audit records.
- Platform execution: this slice does not create or delete directories,
  inspect ACLs, serialize POSIX `envp` or a Windows Unicode environment block,
  serialize arguments, validate executable format, attach streams, invoke a
  shell, launch a process, use a network, apply a sandbox/endpoint policy, or
  record a tool outcome.

## Rollback

Rollback is code-local: remove the environment module, controller overload and
preflight result, focused test and isolation declaration, `RQ-CF-AGENT-012`,
and its mapped documentation. The implementation creates no directory, file,
process, provider, credential, network, audit, user, or persistent product
state.

## Verification

Current local evidence:

- warning-free GCC Release focused build;
- security, policy, session, registry, existing-file target, process-target,
  invocation, isolated-environment, persistent-sink, community, and native-
  isolation selection: `11/11` pass;
- fresh Clang 21 ASan/UBSan session, registry, existing-file target, process-
  target, invocation, and isolated-environment selection with leak detection:
  `6/6` pass with no finding;
- the generated isolation inventory covers `376` native tests;
- safety-traceability workflow contract: `1/1` pass;
- `git diff --check` passes.

Required before implementation closure:

- exact signed/DCO head under all protected Windows, Ubuntu, and macOS checks;
- thread-aware review and resolution of every actionable finding.

## Review evidence

- mode: medium-severity maintainer self-review; no independent final review
- reviewer: rhamenator
- verification: parent independence, fixed keys, trusted-root and exact-
  generation binding, physical identity, bounds/encoding, denial clearing,
  invocation recheck, non-execution boundary, rollback, and traceability
- verification result: focused candidate review passed; broader and protected
  exact-head review pending
- scope: medium-severity non-executing isolated logical environment construction
- result: candidate for `RQ-CF-AGENT-012`; no independence claim

Candidate review corrections preserve the public platform seam by moving host
selection out of the public header and bind directories by stable storage/file
identity rather than mutable namespace metadata. A direct lifecycle regression
proves that stopping generation 1, creating `session-2`, and starting generation
2 remains admissible, while the existing storage-root replacement regression
continues to fail closed. The corrected focused Release selection passes `9/9`,
the exact public-header platform contract passes, the isolated-environment test
passes under Clang ASan/UBSan with leak detection, the safety-traceability
workflow contract passes, and `git diff --check` remains clean.
