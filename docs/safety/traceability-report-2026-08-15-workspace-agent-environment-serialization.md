# Workspace-Agent Environment Serialization Traceability Report

Date: 2026-08-15

Scope: defined v1 `RQ-CF-AGENT-013` native POSIX and Windows process-
environment serialization

Allowed requirement source: explicit repository-owner product policy under
H3/I2; derived from `RQ-CF-AGENT-010`, `RQ-CF-AGENT-011`,
`RQ-CF-AGENT-012`, and the listed hazards

Implementation and verification:

- `include/copperfin/platform/process_environment.h`
- `src/platform/process_environment.cpp`
- `include/copperfin/platform/bounded_process.h`
- `src/platform/bounded_process.cpp`
- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_process_environment_serialization.cpp`
- `tests/test_workspace_agent_isolated_environment.cpp`
- `tests/test_bounded_process.cpp`
- `tests/CMakeLists.txt`
- `tests/CopperfinTestIsolation.cmake`

This report records DO-178C-inspired development assurance adapted to a
general-purpose C++/.NET platform. It is not a claim of formal DO-178C
compliance, certification, an assigned software level, or suitability for a
safety-critical deployment.

## Derived and verification requirements

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-environment-serialization-001`: define one parent-independent portable serializer with explicit POSIX and Windows targets | `DV-workspace-agent-environment-serialization-001`; `DV-workspace-agent-environment-serialization-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-environment-serialization-002`: reject ambiguous names, target-semantic duplicates, embedded NUL, malformed Windows UTF-8, overflow, and native/caller size violations without partial output | `DV-workspace-agent-environment-serialization-001`; `DV-workspace-agent-environment-serialization-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-environment-serialization-003`: bind controller output to the exact bracketed session, target identities, arguments, policy, platform, and fixed entry set | `DV-workspace-agent-environment-serialization-002`; `DV-workspace-agent-environment-serialization-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-environment-serialization-004`: preserve the non-execution boundary and distinguish environment serialization from layout lifecycle, argument serialization, launch, sandbox, endpoint policy, and outcome audit | `DV-workspace-agent-environment-serialization-002`; `DV-workspace-agent-environment-serialization-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-environment-serialization-001`: focused portable tests
  prove POSIX input order and exact bytes, Windows case-insensitive ordering and
  strict UTF-8-to-UTF-16 conversion including a surrogate pair, exact NUL
  termination, case-sensitive/case-insensitive duplicate rules, and exact size
  boundaries.
- `DV-workspace-agent-environment-serialization-002`: controller regression
  proves that the serialized representation retains the complete admitted
  logical plan, populates only the host representation, and clears the plan,
  paths, arguments, entries, and native output on denial.
- `DV-workspace-agent-environment-serialization-003`: warning-free Release,
  fresh Clang ASan/UBSan, broader security/community/isolation/safety tests,
  protected Windows/Ubuntu/macOS exact-head execution, diff validation, and
  thread-aware review are required before definition.

## Requirement delta

- Before: `RQ-CF-AGENT-012` produced a bounded fixed logical environment but
  deliberately left POSIX `envp` and Windows Unicode-block serialization open.
- After: a shared portable boundary emits the exact native environment storage,
  and the controller binds it to a repeated complete logical preflight. The
  existing bounded-process utility consumes the same serializer rather than a
  second environment-assembly implementation.

Potential Severity If Misused: medium

## Hazard, misuse, and boundary analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Ambient credential exposure: serialization receives a complete explicit
  vector and never enumerates, reads, or merges the parent environment.
- Environment confusion: portable names reject empty, digit-leading,
  punctuation-bearing, assignment-bearing, and embedded-NUL forms. POSIX
  duplicates are case-sensitive; Windows duplicates are case-insensitive.
- Encoding confusion: POSIX preserves exact non-NUL value bytes. Windows uses
  a strict UTF-8 decoder that rejects overlong encodings, truncation, surrogate
  scalars, invalid continuation bytes, and values above Unicode maximum before
  emitting UTF-16.
- Truncation and exhaustion: subtraction-safe accounting includes every entry
  terminator and the Windows final block terminator. Windows enforces the
  explicit caller resource cap without misapplying the ANSI-only 32,767-
  character limit to Unicode blocks. Denial returns no partial POSIX or Windows
  representation.
- Stale authority: the controller repeats the complete logical environment
  preflight after serialization and compares all invocation, identity, policy,
  platform, and entry fields. Its output remains point-in-time evidence, not a
  launch token.
- Launch boundary: this slice creates or cleans no directory, changes no ACL,
  serializes no argument, validates no executable format, pins no handle,
  starts no process, uses no shell or network, applies no sandbox or endpoint
  policy, and records no tool outcome.

## Rollback

Rollback is code-local: restore the bounded-process private environment
assembly and remove the shared serializer, controller serialization result and
method, focused tests/isolation declaration, `RQ-CF-AGENT-013`, and mapped
documentation. The candidate creates no persistent file, directory, process,
credential, provider, network, audit, or user state.

## Verification

Current local evidence:

- warning-free GCC Release build of the serializer, controller, and bounded-
  process consumers;
- security, policy, session, registry, existing-file target, process-target,
  invocation, isolated-environment, serialization, persistent-sink, bounded-
  process, community, native-isolation, and safety selection: `14/14` pass;
- fresh Clang 21 ASan/UBSan with leak detection: serialization, isolated-
  environment, and bounded-process selection `3/3` pass with no finding;
- generated native-test isolation inventory covers `377` tests;
- safety traceability workflow contract: `1/1` pass;
- `git diff --check` passes;
- signed/DCO exact implementation head `b5129e08a` passed all eleven protected
  checks: contributor sign-off `31870916071`, Win32/x64 DECLARE
  `31870917594`, Windows/Ubuntu/macOS generated launcher `31870917602`,
  Clang/GCC executable paths `31870917599`, Windows environment/executable
  paths `31870917583`, and both Socket checks. PR `#5015` merged into
  `v1-development` as `94c705e6a`.

## Review evidence

- mode: medium-severity maintainer self-review; no independent final safety
  approval claimed
- reviewer: rhamenator
- verification: parent independence, target semantics, UTF conversion,
  termination, bounds/overflow, denial clearing, exact-plan rebinding,
  non-execution boundary, rollback, and traceability
- result: local focused verification, all eleven protected exact-head checks,
  and exact-head automated review passed; both actionable review threads were
  corrected, answered, and resolved before merge

Automated exact-head review found that the initial candidate incorrectly
treated the ANSI `CreateProcess` 32,767-character environment limit as a
Unicode limit. The corrected serializer honors the caller's explicit resource
cap; the workspace-agent caller remains bounded by its fixed logical-profile
policy, while the existing Unicode bounded-process path preserves its prior
large-environment compatibility. A direct 40,000-code-unit boundary regression
and both focused Release and sanitizer selections pass after the correction.
The corrected caller-cap calculation also includes the distinct final Windows
block terminator after every per-entry terminator; a direct policy-cap
regression fixes the Windows and POSIX formulas and rejects an invalid platform.
Automated review of final exact head `b5129e08a` reported no further issue.
This is maintainer/self-review evidence and does not claim independent safety
approval or formal certification.

Subsequent independent read-only review at final implementation head
`b5129e08a` verified the Windows `maximum_total_bytes + entry_count + 1`
formula, POSIX omission of the final block terminator, subtraction-safe overflow
guard, and fail-closed invalid-platform path. Its focused workspace-agent,
process-environment, and bounded-process selection passed `11/11`, with no
remaining finding. This independent engineering review is not a formal safety
approval or certification claim.
