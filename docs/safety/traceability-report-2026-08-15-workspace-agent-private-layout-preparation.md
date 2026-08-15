# Workspace-Agent Private Layout Preparation Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-014` private storage-root verification and
generation-owned session-layout preparation

Allowed requirement source: explicit repository-owner product policy under
H3/I2; derived from `RQ-CF-AGENT-012`, `RQ-CF-AGENT-013`, and the listed
hazards

Implementation and verification:

- `include/copperfin/platform/private_directory.h`
- `src/platform/private_directory.cpp`
- `include/copperfin/security/workspace_agent_environment.h`
- `src/security/workspace_agent_environment.cpp`
- `tests/test_platform_private_directory.cpp`
- `tests/run_platform_private_directory_boundary_contract_check.cmake`
- `tests/test_workspace_agent_isolated_environment.cpp`
- `tests/CMakeLists.txt`
- `tests/CopperfinTestIsolation.cmake`
- `.github/workflows/generated-launcher-validation.yml`
- `.github/workflows/windows-environment-validation.yml`

This report records DO-178C-inspired development assurance adapted to a
general-purpose C++/.NET platform. It is not a claim of formal DO-178C
compliance, certification, an assigned software level, or suitability for a
safety-critical deployment.

## Derived and verification requirements

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-private-layout-001`: define one portable leaf-only creation and verification seam with explicit POSIX and Windows privacy contracts | `DV-workspace-agent-private-layout-001`; `DV-workspace-agent-private-layout-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-private-layout-002`: reject existing objects, indirection, wrong kinds, ownership/access broadening, unsupported security, missing parents, and failed final verification without adopting or modifying existing state | `DV-workspace-agent-private-layout-001`; `DV-workspace-agent-private-layout-002`; `DV-workspace-agent-private-layout-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-private-layout-003`: require a private stable storage root and prepare exactly one fixed generation layout with content-free denial | `DV-workspace-agent-private-layout-002`; `DV-workspace-agent-private-layout-003`; `DV-workspace-agent-private-layout-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-private-layout-004`: preserve the non-execution boundary and distinguish preparation from root provisioning, cleanup, session-start integration, argument serialization, launch, sandbox, endpoint policy, and outcome audit | `DV-workspace-agent-private-layout-003`; `DV-workspace-agent-private-layout-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-private-layout-001`: portable behavior tests prove exact
  private creation and verification, leaf-only behavior, repeat/existing-object
  denial, invalid/relative/missing-parent denial, wrong-kind denial, and
  symlink/reparse denial where the host permits the fixture.
- `DV-workspace-agent-private-layout-002`: environment regression proves a
  complete private fixed layout can be consumed, while an ordinary inherited
  or broadened child, existing layout, and preexisting partial layout fail
  without repair, overwrite, deletion, or returned path content.
- `DV-workspace-agent-private-layout-003`: source-contract verification proves
  the portable API owns no native declarations, product code delegates through
  the seam, isolation metadata is complete, and hosted Windows, Ubuntu, and
  macOS workflows execute the behavior and boundary tests.
- `DV-workspace-agent-private-layout-004`: warning-free Release, fresh Clang
  ASan/UBSan, broader security/community/isolation/safety tests, protected
  exact-head Windows/Ubuntu/macOS execution, diff validation, and thread-aware
  review are required before definition.

## Requirement delta

- Before: `RQ-CF-AGENT-012` consumed a fixed generation layout but deliberately
  left secure creation, access control, and cleanup to an unspecified trusted
  host.
- After: a shared platform seam can create and verify one private directory
  leaf, and the environment boundary can prepare the exact fixed layout for an
  absent generation. Cleanup and session-start integration remain explicit
  future boundaries.

Potential Severity If Misused: high

## Hazard, misuse, and boundary analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Ambient-user disclosure: POSIX requires the effective user as owner and exact
  mode `0700`, including rejection of setuid, setgid, and sticky bits. Windows
  requires the process user as owner and a protected DACL
  containing no principal other than that user and LocalSystem, with no
  inherited or inherit-only ACE and no access-mask broadening.
- Redirection and type confusion: verification uses `lstat` on POSIX and a
  reparse-point-aware directory handle on Windows. Symbolic links, reparse
  points, regular files, missing paths, and wrong kinds fail closed.
- Existing-state damage: creation is leaf-only. An existing complete or partial
  generation is not adopted, repaired, overwritten, permission-mutated, or
  deleted. Post-creation verification never performs path-based cleanup because
  a racing authority could have replaced the created object; the unverified
  path is left for an identity-aware trusted-host cleanup decision. If creation
  of a new child fails, a private partial generation can remain and deliberately
  blocks reuse.
- Process-state isolation: POSIX `umask` is process-global and `mkdir` returns no
  identity-bound descriptor. A host `umask` that removes owner bits therefore
  fails exact-mode verification and leaves the path untouched; the library does
  not change global `umask` state or `fchmod` a path that could have been
  replaced. A direct restrictive-umask regression preserves this fail-closed
  tradeoff until identity-aware cleanup exists.
- Root replacement: the environment boundary rechecks the captured private
  storage-root identity and current privacy contract before creation,
  after preparation, during construction, and before final return, then
  physically contains and re-verifies identity and privacy for every fixed
  session directory before returning a constructed environment.
  Session-root creation additionally verifies the root's captured storage/file
  identity on an open handle; POSIX performs the creation relative to that same
  descriptor, preventing a writable outer parent from redirecting the side
  effect. Windows brackets its public full-path create with the handle check and
  retains the documented trusted-parent residual.
  The returned session identity is required as the verified parent for all five
  child creations, preventing a replacement root/session path from receiving
  those later side effects.
- Parent indirection: POSIX creation and verification walk every existing
  parent through no-follow directory descriptors, then create and inspect the
  leaf relative to the bound parent. Direct regressions prove a symlink parent
  neither redirects creation nor makes an indirectly reached private leaf pass.
  Windows opens and rejects reparse-point parent components before creation and
  repeats the check during verification; its public creation API remains
  full-path and within the recorded same-authority residual boundary.
- Configured-path replacement: preparation rechecks every approved executable
  directory and the Windows system root, when present, before the first layout
  creation and before success. Direct replacement regressions prove a known
  unusable configuration cannot consume a generation from cached path text.
- Same-authority boundary: creation currently uses full paths. It assumes the
  storage-root owner and LocalSystem are trusted host authorities; it is not a
  defense against a malicious process already operating with either authority.
  A future sandbox must prevent an untrusted child from reaching this root, and
  launch work must repeat identity and admission checks beside the operation.
- Information exposure: success returns only the generation. Denials return a
  stable diagnostic and zero generation, never a path, identity, ACL, prompt,
  credential, argument, or provider value.
- Unusable derived state: preparation builds and validates the exact fixed
  platform environment entries before creating the session root. Encoding,
  required-value, per-entry, and aggregate-limit failures therefore leave no
  generation layout behind; construction consumes the same builder.
- Execution boundary: this slice provisions no root, cleans no generation,
  binds no start lifecycle, serializes no argument, validates no executable
  format, starts no process, opens no endpoint, applies no sandbox, injects no
  credential, and records no tool outcome.

## Rollback

Rollback is bounded but state-aware: remove the preparation method, portable
private-directory seam, focused tests/isolation declaration, workflow targets,
`RQ-CF-AGENT-014`, and mapped documentation. Any private candidate test or host
layout created before rollback must be removed only by an explicit trusted-host
cleanup decision; rollback must not recursively delete an unresolved path.

## Verification

Current local candidate evidence:

- warning-free GCC Release build of the portable seam and environment consumer;
- security, environment, invocation, target, registry, session, audit-sink,
  community, isolation, and safety selection: `15/15` pass;
- focused portable behavior and source/workflow-boundary selection: `5/5`
  pass, including the final root-replacement and broadened-root regressions;
- fresh Clang 21 ASan/UBSan with leak detection: private-directory,
  isolated-environment, and environment-serializer selection `3/3` pass;
- generated native-test isolation inventory covers `379` tests;
- native-platform workflow, GitHub Actions, and private-directory boundary
  contracts: `3/3` pass;
- safety traceability workflow contract: `1/1` pass;
- `git diff --check` passes.

Required before definition:

- exact signed/DCO head on protected Windows, Ubuntu, and macOS workflows;
- both supply-chain checks and thread-aware exact-head review.

## Review evidence

- mode: high-severity maintainer self-review; no independent final safety
  approval claimed
- reviewer: rhamenator
- verification: ownership and ACL/mode policy, indirection and type rejection,
  existing/partial-state preservation, root identity, denial clearing,
  non-execution boundary, rollback, and traceability
- result: candidate local evidence only; protected exact-head and review
  evidence remain pending
