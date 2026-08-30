# Workspace-Agent Process-Parser Authority Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-018` trusted-host Windows child argument-
parser authority

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-015`,
`RQ-CF-AGENT-017`, `HZ-system-failure-01`, and `HZ-data-corruption-01`

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, arbitrary executable compatibility, or suitability for
a safety-critical deployment.

## Derived and verification requirements

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-process-parser-001`: trusted product-host configuration alone binds one supported Windows parser contract to exact canonical path, a host-supplied expected complete physical identity, and an expected lowercase SHA-256 | `DV-workspace-agent-process-parser-001`; `DV-workspace-agent-process-parser-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-parser-005`: the trusted product record for the exact image digest attests that argument parsing is self-contained from mutable non-system load-time images; adjacent-DLL-dependent parser images remain unsupported until an authenticated dependency-closure and launch-isolation contract exists | `DV-workspace-agent-process-parser-001`; `DV-workspace-agent-process-parser-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-parser-002`: invalid, excessive-count, oversized-image, indirect, multiply linked, duplicate, unknown-contract, missing, wrong-identity, malformed/mismatched-digest, changed-identity, and changed-content authority fails without sensitive reflection | `DV-workspace-agent-process-parser-001`; `DV-workspace-agent-process-parser-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-parser-003`: Windows serialization authorizes before and after the complete serialized invocation preflight and requires equal contracts | `DV-workspace-agent-process-parser-002`; `DV-workspace-agent-process-parser-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-parser-004`: POSIX retains native argv semantics and the boundary launches nothing | `DV-workspace-agent-process-parser-002`; `DV-workspace-agent-process-parser-003` | `HZ-system-failure-01` |

- `DV-workspace-agent-process-parser-001`: focused portable boundary tests prove
  exact-identity admission and every listed configuration/identity denial.
- `DV-workspace-agent-process-parser-002`: integrated environment/session tests
  prove Windows fail-closed behavior and POSIX native argv preservation.
- `DV-workspace-agent-process-parser-003`: the source contract, warning-free
  Release verification, sanitizer verification, broader safety/isolation tests,
  protected Windows/Ubuntu/macOS execution, diff validation, and exact-head
  review are required before implementation evidence is complete.

## Requirement delta

- Before: Windows argument serialization used a conventional C-runtime quoting
  algorithm but could not establish that the selected child used that parser.
- After: serialization is available only when bounded trusted-host configuration
  binds that contract to the exact canonical executable identity and authenticated
  byte digest and both bracketed checks remain equal.

Potential Severity If Misused: high

## Hazard, misuse, boundary, and rollback analysis

- Command-line confusion: PE structure does not prove parsing behavior. Only the
  single supported contract can be attested, and only for a configured exact
  physical identity.
- Authority injection: provider, model, prompt, workspace, and tool-request data
  cannot create or choose a binding.
- Resource exhaustion: binding count is capped at 64 and each contained image
  snapshot is capped at 512 MiB before allocation.
- Pre-positioning or in-place overwrite: boundary construction compares a
  physically contained snapshot to the host-supplied expected identity and
  SHA-256. Authorization re-hashes another contained snapshot, so equal-sized
  replacement bytes with restored filesystem metadata still fail closed.
- Dependency substitution: the only supported dependency contract requires
  trusted product review of the exact digest to establish a self-contained
  parser image. Images whose parser can depend on mutable adjacent DLLs cannot
  be configured; authenticating such a closure remains future work.
- Aliasing and replacement: relative, indirect, multiply linked, duplicate, and
  identity-changed files fail closed. Authorization repeats physical inspection.
- Race boundary: inspection remains point-in-time. A future executor must pin or
  revalidate beside launch; this result is not a reusable token.
- Platform boundary: POSIX uses native argv semantics and does not consume a
  Windows binding.
- Information exposure: denials return stable diagnostic identities without
  paths, arguments, environment entries, file content, prompts, or credentials.
- Non-claims: no publisher validation, launch, sandbox, endpoint control,
  descendant management, provider authentication, or outcome audit is added.
- Rollback: remove the parser boundary/configuration and the three-argument
  controller constructor, restore Windows serialization to fail closed as
  unavailable, and withdraw candidate `RQ-CF-AGENT-018`. Rollback must not
  restore unbound Windows serialization as execution-ready.

## Verification

Completed implementation evidence:

- warning-free GCC Release build of the boundary and integrated controller
  targets passes;
- focused parser, source-contract, isolated-environment, argument-serialization,
  and process-invocation verification passes locally;
- native workflow and GitHub Actions source contracts pass;
- the broader security/community/isolation selection passed sixteen unaffected
  tests plus the 327-second safety gate; its sole failure was the new contract
  test's unsupported isolation-label spelling, after which the corrected
  isolation/focused selection passed `4/4`;
- corrected-head GCC Release/source/workflow verification passes `8/8`;
- fresh Clang 21 ASan/UBSan with explicit leak detection passes parser, source-
  contract, and integrated environment tests `3/3`;
- `git diff --check` passes;
- corrected-head review identified that physical metadata alone does not
  authenticate mutable executable bytes; the candidate now binds and
  revalidates trusted SHA-256 evidence, with direct regression coverage;
- Windows diagnostic evidence found undefined behavior in the focused
  excessive-binding and duplicate-binding test setup: `std::vector` mutation
  received a reference into the same vector, which a reallocation could
  invalidate. The test now copies its canonical binding before either mutation,
  preserving the intended `DQ-workspace-agent-process-parser-002` denial
  coverage without relying on allocator layout. Windows Debug reproduced the
  standard-library assertion; optimized execution had been nondeterministic,
  including one protected-matrix `0xc0000409` fail-fast. This is test-harness
  correction only; parser authority behavior and hazard controls are unchanged;
- subsequent exact-head review identified that an adjacent mutable DLL could
  still influence parsing; configuration now accepts only exact-digest product
  evidence for a self-contained parser image and rejects every other dependency
  contract pending authenticated closure support;
- exact signed/DCO head `8e2f71551` passed all eleven protected checks in runs
  `31899195856`, `31899197341`, `31899197358`, `31899197362`, and
  `31899197346`, including Windows Win32/x64 DECLARE, Windows environment,
  Windows/Ubuntu/macOS generated-launcher, GCC/Clang, DCO, and both Socket jobs;
- the final automated review found no major issue at exact head `8e2f71551`, all
  three earlier P1 threads are resolved, and PR `#5026` merged into
  `v1-development` as `51cc62123`.

The retained risk classification is `high`. Implementation evidence is
complete, but requirement status remains `gap` because no qualified independent
human final safety approval exists. No launch readiness is claimed.
