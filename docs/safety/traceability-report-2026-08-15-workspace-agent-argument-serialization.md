# Workspace-Agent Argument Serialization Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-015` native POSIX and Windows
process-argument serialization

Allowed requirement source: explicit repository-owner product policy under
H3/I2; derived from `RQ-CF-AGENT-010`, `RQ-CF-AGENT-011`,
`RQ-CF-AGENT-012`, `RQ-CF-AGENT-013`, and the listed hazards

Implementation and verification:

- `include/copperfin/platform/process_arguments.h`
- `src/platform/process_arguments.cpp`
- `include/copperfin/platform/bounded_process.h`
- `src/platform/bounded_process.cpp`
- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_process_argument_serialization.cpp`
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
| `DQ-workspace-agent-argument-serialization-001`: define one portable direct-element serializer with explicit POSIX and Windows targets | `DV-workspace-agent-argument-serialization-001`; `DV-workspace-agent-argument-serialization-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-argument-serialization-002`: reject an empty executable, embedded NUL, invalid target/cap, malformed Windows UTF-8, overflow, and native size violations without partial output | `DV-workspace-agent-argument-serialization-001`; `DV-workspace-agent-argument-serialization-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-argument-serialization-003`: bind controller output to the exact bracketed canonical target, direct arguments, and fixed serialized environment | `DV-workspace-agent-argument-serialization-002`; `DV-workspace-agent-argument-serialization-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-argument-serialization-004`: preserve direct-launch compatibility in the existing bounded-process consumer and distinguish serialization from child-parser compatibility, launch authority, sandboxing, endpoint policy, and outcome audit | `DV-workspace-agent-argument-serialization-001`; `DV-workspace-agent-argument-serialization-002`; `DV-workspace-agent-argument-serialization-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-argument-serialization-001`: focused portable tests
  prove exact POSIX elements including `argv[0]`, empty and non-UTF-8
  elements, Windows spaces/quotes/trailing-backslash/Unicode behavior, and
  exact terminator-inclusive caps.
- `DV-workspace-agent-argument-serialization-002`: controller regression
  proves that exactly one host representation remains attached to an unchanged
  complete serialized-environment plan and that denial clears all content.
- `DV-workspace-agent-argument-serialization-003`: warning-free Release,
  fresh Clang ASan/UBSan, broader security/community/isolation/safety tests,
  protected Windows/Ubuntu/macOS exact-head execution, diff validation, and
  thread-aware review are required before definition.

## Requirement delta

- Before: the invocation preflight admitted only bounded direct elements and
  explicitly left platform serialization open.
- After: a shared seam emits complete POSIX argument storage or a bounded
  Windows command line, the controller brackets it with the complete fixed
  environment plan, and the existing bounded-process utility consumes the same
  direct-launch representation.

Potential Severity If Misused: medium

## Hazard, misuse, and boundary analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Command confusion: the API accepts distinct elements and performs no shell,
  PATH, glob, variable, response-file, or option interpretation. Shell-looking
  bytes remain literal.
- Encoding and element confusion: POSIX preserves exact non-NUL bytes and empty
  elements. Windows rejects malformed, overlong, truncated, surrogate, and
  out-of-range UTF-8 before emitting UTF-16.
- Windows quoting mismatch: every element, including `argv[0]`, is quoted
  using the conventional C-runtime backslash/quote algorithm, with direct
  coverage for embedded quotes and trailing backslashes. Compatibility with a
  particular child's parser remains an explicit executor admission gap.
- Truncation and exhaustion: subtraction-safe accounting includes each POSIX
  element terminator or the Windows terminating NUL. The controller and
  bounded-process Windows consumer enforce 32,767 total UTF-16 code units.
  Denial returns no partial platform representation.
- Stale authority: the controller repeats the complete serialized-environment
  preflight after argument serialization and compares every logical, identity,
  environment, and native representation field. Its output is evidence, not a
  reusable launch token.
- Launch boundary: the workspace-agent path starts no process, reads no
  executable, creates or cleans no directory, changes no ACL, opens no endpoint,
  injects no provider credential, applies no sandbox, and records no outcome.

## Rollback

Rollback is code-local: restore the bounded-process private argument assembly
and remove the shared serializer, controller result/method, focused
tests/isolation declaration, candidate `RQ-CF-AGENT-015`, and mapped
documentation. The candidate creates no persistent file, directory, process,
credential, provider, network, audit, or user state.

## Verification

Current local evidence:

- warning-free GCC Release build of the serializer, controller, and existing
  bounded-process consumer;
- security, policy, session, registry, target-containment, invocation,
  environment, platform-private-directory, serializer, bounded-process,
  community, native-isolation, and safety selection: `17/17` pass;
- focused serializer, isolated-environment, and bounded-process selection:
  Release `3/3` and fresh Clang 21 ASan/UBSan with leak detection `3/3`
  pass with no finding;
- generated native-test isolation inventory covers `380` tests;
- `git diff --check` passes.

Protected cross-platform exact-head and thread-aware review evidence remain
pending before the requirement may move from `gap` to `defined`.

## Review evidence

- mode: medium-severity maintainer self-review; no independent final safety
  approval claimed
- reviewer: rhamenator
- verification: element preservation, quoting, encoding, cap/overflow,
  denial clearing, exact-plan rebinding, direct-launch compatibility,
  non-execution boundary, rollback, and traceability
- result: broader Release and fresh sanitizer verification pass; exact-head protected and
  thread-aware review evidence remains pending
