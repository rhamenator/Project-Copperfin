# Workspace-Agent Session-Revocation Lease Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-022` move-only, non-executing session-
revocation lease prerequisite for a future controlled process launcher

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-005`,
`RQ-CF-AGENT-019`, `HZ-system-failure-01`, and `HZ-data-corruption-01`

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, launch readiness, nor suitability for a
safety-critical deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-revocation-lease-001`: only the exact active process-capable generation may acquire a move-only lease; inactive, zero, stale, incapable, unavailable, and transitioning authority shall fail closed | `DV-workspace-agent-revocation-lease-001`; `DV-workspace-agent-revocation-lease-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-revocation-lease-002`: stop shall enter its serialized transition but wait to revoke and audit until all outstanding leases are released | `DV-workspace-agent-revocation-lease-001`; `DV-workspace-agent-revocation-lease-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-revocation-lease-003`: a lease shall expose only validity and exact generation, shall be move-only, and shall not contain a plan, path, target identity, argument, environment, native handle, or process capability | `DV-workspace-agent-revocation-lease-001`; `DV-workspace-agent-revocation-lease-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-revocation-lease-004`: acquiring a lease shall not weaken the invariant denial of `RQ-CF-AGENT-019` or perform any process or filesystem side effect | `DV-workspace-agent-revocation-lease-001`; `DV-workspace-agent-revocation-lease-002`; `DV-workspace-agent-revocation-lease-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-revocation-lease-001`: focused controller regression
  proves exact-generation admission, stale/inactive/incapable denial, move-only shape,
  stop waiting, release completion, and unchanged launch-gate denial.
- `DV-workspace-agent-revocation-lease-002`: source and documentation contracts
  prove that no authority-bearing plan, target, handle, or launch is exposed.
- `DV-workspace-agent-revocation-lease-003`: warning-free Release, sanitizer,
  broader safety/isolation, protected Windows/Ubuntu/macOS, diff, and exact-head
  review evidence are required before implementation evidence is complete.

## Hazard, misuse, boundary, and rollback analysis

- Revocation race: each lease increments an explicit generation-owned count
  under a mutex; releasing it decrements and signals a condition variable.
  Stop prevents new acquisition through the controller transition, waits for
  the count to reach zero, and only then clears authority. This avoids
  recursively acquiring a non-recursive shared mutex when one executor thread
  owns more than one lease. Stop cannot report revocation during the future
  narrow launch boundary.
- Denial of service: a leaked or overlong lease delays stop. Product code must
  retain it only around a direct launch syscall, never user interaction,
  network activity, unbounded preparation, or child lifetime.
- Lifetime: the controller is the authority owner and must outlive every lease.
  Host integration must enforce destruction order.
- Authority confusion: the lease exposes no submitted plan or filesystem
  identity and cannot be used to select an executable, working directory,
  arguments, environment, endpoint, or sandbox.
- Non-claims: no target is pinned, no process is launched, no sandbox or
  endpoint/descendant policy is applied, and no outcome is audited. The
  `RQ-CF-AGENT-019` promotion gate remains invariantly denied.
- Rollback: remove the lease API and focused regression together; the launch
  gate must remain denied and no executor may consume point-in-time preflight.

Potential Severity If Misused: high

## Verification

Current local evidence:

- warning-free GCC 15 Release workspace-agent/security selection passed
  `13/13`, including one hundred additional consecutive focused session
  executions after the counted-lease correction;
- fresh Clang 21 ASan/UBSan with leak detection passed `3/3`;
- fresh GCC 15 ThreadSanitizer focused execution passed `1/1` after the
  counted-lease correction;
- repository community, native-platform workflow, GitHub Actions, isolation,
  and safety contracts passed `5/5`, including the corrected final-byte
  330.80-second safety scan;
  and
- `git diff --check` passed.

Protected Windows/Ubuntu/macOS checks, exact-head review, and qualified
high-severity sign-off remain pending. Requirement status remains `gap`; no
execution or launch readiness is claimed.
