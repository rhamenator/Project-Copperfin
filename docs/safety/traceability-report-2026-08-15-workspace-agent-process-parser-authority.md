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
| `DQ-workspace-agent-process-parser-001`: trusted product-host configuration alone binds one supported Windows parser contract to exact canonical executable identity | `DV-workspace-agent-process-parser-001`; `DV-workspace-agent-process-parser-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-process-parser-002`: invalid, excessive, indirect, multiply linked, duplicate, unknown-contract, missing, wrong-identity, and changed-identity authority fails without sensitive reflection | `DV-workspace-agent-process-parser-001`; `DV-workspace-agent-process-parser-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
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
  binds that contract to the exact canonical executable identity and both
  bracketed checks remain equal.

Potential Severity If Misused: high

## Hazard, misuse, boundary, and rollback analysis

- Command-line confusion: PE structure does not prove parsing behavior. Only the
  single supported contract can be attested, and only for a configured exact
  physical identity.
- Authority injection: provider, model, prompt, workspace, and tool-request data
  cannot create or choose a binding.
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

Current candidate evidence:

- warning-free GCC Release build of the boundary and integrated controller
  targets passes;
- focused parser, source-contract, isolated-environment, argument-serialization,
  and process-invocation verification passes locally;
- native workflow and GitHub Actions source contracts pass;
- the broader security/community/isolation selection passed sixteen unaffected
  tests plus the 327-second safety gate; its sole failure was the new contract
  test's unsupported isolation-label spelling, after which the corrected
  isolation/focused selection passed `4/4`;
- fresh Clang 21 ASan/UBSan with explicit leak detection passes parser, source-
  contract, integrated environment, and argument-serialization tests `4/4`;
- `git diff --check` passes;
- exact signed/DCO head, protected Windows/Ubuntu/macOS execution, and exact-
  head review remain required before implementation evidence is complete.

The retained risk classification is `high`. No independent final safety
approval or launch readiness is claimed.
