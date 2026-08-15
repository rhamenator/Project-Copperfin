# Workspace-Agent Launch Promotion Gate Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-019` portable, non-executing, fail-closed
promotion gate for serialized workspace-agent process plans

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-010` through
`RQ-CF-AGENT-018`, `HZ-system-failure-01`, and `HZ-data-corruption-01`

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, launch readiness, nor suitability for a
safety-critical deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-launch-revalidation-001`: every request to promote a point-in-time serialized plan shall be denied with one stable content-free diagnostic while race-free launch authority is unavailable | `DV-workspace-agent-launch-revalidation-001`; `DV-workspace-agent-launch-revalidation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-launch-revalidation-002`: the denial boundary shall not inspect or reflect untrusted request/plan content and shall return no plan, digest, path, argument, environment entry, target identity, or reusable authority | `DV-workspace-agent-launch-revalidation-001`; `DV-workspace-agent-launch-revalidation-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-launch-revalidation-003`: an allow path shall remain absent until an executor retains a trusted containment root, platform-backed executable and working-directory pins (or equivalent race-free target authority), and a revocation lease through launch, then enforces sandbox, endpoint, descendant, and outcome-audit policy | `DV-workspace-agent-launch-revalidation-002`; `DV-workspace-agent-launch-revalidation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-launch-revalidation-004`: the API shall remain non-executing | `DV-workspace-agent-launch-revalidation-002`; `DV-workspace-agent-launch-revalidation-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-launch-revalidation-001`: focused controller tests prove
  that valid, empty, and altered inputs all receive the same denial and no
  authority-bearing output.
- `DV-workspace-agent-launch-revalidation-002`: source and documentation
  contracts preserve invariant denial, no input reflection, and no launch.
- `DV-workspace-agent-launch-revalidation-003`: warning-free Release,
  sanitizer, broader safety/isolation, protected Windows/Ubuntu/macOS, diff,
  and exact-head review evidence are required before implementation evidence is
  complete.

## Requirement delta

- Before: no explicit product boundary prevented a point-in-time serialized
  plan from being promoted toward a future launcher despite unresolved
  multi-object races.
- After: the public promotion gate invariantly denies every request with
  `workspace_agent.process_launch_revalidation_pinning_unavailable` and returns
  no plan, digest, or authority.

Potential Severity If Misused: high

## Hazard, misuse, boundary, and rollback analysis

- Executable mutation: review demonstrated that bytes can change after a hash
  while metadata is restored. Repeating snapshots only moves the final
  unprotected interval; it does not pin the executable used by launch.
- Revocation: review demonstrated that a session can be stopped during a
  potentially long hash. Rechecking afterward still leaves a final interval
  unless authority is held under a revocation lease through launch.
- Containment-root substitution: review demonstrated that an executable parent
  can be renamed outside the workspace and replaced by a symlink. A fresh
  snapshot rooted at that mutable parent is not the original trusted root.
- Working-directory replacement: review demonstrated that the working
  directory can change after its last complete preflight while an executable
  snapshot is read. Reordering checks cannot provide one coherent target state.
- Architectural conclusion: sequential point-in-time checks cannot bind the
  trusted root, executable, working directory, and session as one launch
  authority. The earlier allow design was withdrawn rather than rationalized.
- Information exposure: every input receives the same stable diagnostic, with
  no path, argument, environment, prompt, credential, file bytes, or digest.
- Non-claims: no process is launched; no target is pinned; no sandbox, endpoint
  control, descendant management, publisher validation, provider
  authentication, or outcome audit is added.
- Rollback: removing the gate must leave the future executor disconnected; it
  must never make an earlier serialized preflight sufficient for launch.

## Verification

Completed local evidence for the fail-closed pivot:

- warning-free GCC Release workspace-agent selection passed `10/10`;
- focused integrated and source-contract verification passed `2/2`;
- fresh Clang 21 ASan/UBSan with leak detection passed `3/3`;
- repository community, generated isolation, and safety gates passed `5/5`,
  including the fresh 324-second safety traceability scan;
- native-platform and GitHub Actions workflow contracts passed; and
- `git diff --check` passed.

Exact signed/DCO head `e3e949414` passed all eleven protected checks in runs
`31903337717`, `31903339126`, `31903339150`, `31903339171`, and `31903339187`,
covering Windows, Ubuntu, macOS, GCC, Clang, DCO, DECLARE, generated-launcher,
and supply-chain gates. Exact-head automated review found no major issue after
the four actionable P1 threads drove withdrawal of the allow path; all four
threads are resolved. PR `#5028` merged into `v1-development` as `0e9fac5c7`.

The retained risk classification is `high`. No independent final safety
approval or launch readiness is claimed; requirement status remains `gap`.
