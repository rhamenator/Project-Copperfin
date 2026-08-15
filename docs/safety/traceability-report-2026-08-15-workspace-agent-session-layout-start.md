# Workspace-Agent Session Layout Start Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-016` configured session-start integration
for the private generation layout

Allowed requirement source: explicit repository-owner product policy under
H3/I2; derived from `RQ-CF-AGENT-005`, `RQ-CF-AGENT-012`,
`RQ-CF-AGENT-014`, and the listed hazards

Implementation and verification:

- `include/copperfin/security/workspace_agent_session.h`
- `src/security/workspace_agent_session.cpp`
- `tests/test_workspace_agent_isolated_environment.cpp`
- `tests/test_workspace_agent_session.cpp`
- `docs/64-workspace-agent-access-policy.md`
- `docs/32-recovered-requirements-traceability.md`

This report records DO-178C-inspired development assurance adapted to a
general-purpose C++/.NET platform. It is not a claim of formal DO-178C
compliance, certification, an assigned software level, or suitability for a
safety-critical deployment.

## Derived and verification requirements

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-session-layout-start-001`: prepare the exact configured process-capable candidate generation after policy admission and before audit-backed authority activation | `DV-workspace-agent-session-layout-start-001`; `DV-workspace-agent-session-layout-start-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-session-layout-start-002`: convert invalid supplied configuration, preparation denial or exception, and generation mismatch into content-free audited denial without authority | `DV-workspace-agent-session-layout-start-001`; `DV-workspace-agent-session-layout-start-002`; `DV-workspace-agent-session-layout-start-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-session-layout-start-003`: avoid layout creation for policy-denied or non-process-capable sessions and never adopt, repair, overwrite, delete, or reuse an existing generation | `DV-workspace-agent-session-layout-start-001`; `DV-workspace-agent-session-layout-start-003`; `DV-workspace-agent-session-layout-start-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-session-layout-start-004`: if audit commit fails after preparation, withhold authority, preserve the generation for identity-aware cleanup, and advance every later attempt | `DV-workspace-agent-session-layout-start-001`; `DV-workspace-agent-session-layout-start-003`; `DV-workspace-agent-session-layout-start-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-session-layout-start-005`: retain the unconfigured controller's non-executing lifecycle compatibility while leaving root provisioning, cleanup, launch, sandbox, endpoint policy, and outcome audit outside this slice | `DV-workspace-agent-session-layout-start-002`; `DV-workspace-agent-session-layout-start-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-session-layout-start-001`: focused native behavior tests
  prove that a configured process-capable start creates and verifies the exact
  generation's fixed private layout before returning active authority.
- `DV-workspace-agent-session-layout-start-002`: focused denial tests prove
  invalid supplied configuration and preexisting generation state produce a
  content-free audited denial, while a controller with no environment boundary
  retains its existing non-executing preflight denial.
- `DV-workspace-agent-session-layout-start-003`: focused lifecycle tests prove
  policy denial creates no layout, audit failure grants no authority and does
  not delete the prepared generation, and a later successful start uses the
  next generation rather than adopting the orphan.
- `DV-workspace-agent-session-layout-start-004`: warning-free Release, fresh
  sanitizer, broader security/community/isolation/safety tests, protected
  Windows/Ubuntu/macOS execution, diff validation, and exact-head review are
  required before definition.

## Requirement delta

- Before: the trusted environment boundary could prepare a private fixed
  generation, but session startup did not invoke it; fixtures or a future host
  had to create the layout separately.
- After: a controller explicitly constructed with trusted environment
  configuration prepares an admitted process-capable session's exact candidate
  generation before committing the start audit or activating authority.

Potential Severity If Misused: high

## Hazard, misuse, and boundary analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Authority ordering: policy admission precedes preparation. Preparation must
  report success for the exact candidate generation before the start event is
  offered to the audit sink, and only a committed nonempty receipt permits the
  controller to publish authority.
- Invalid trusted input: the constructor records that configuration was
  supplied independently of whether boundary construction succeeded. An
  invalid configuration therefore cannot silently fall back to the
  unconfigured lifecycle lane for a process-capable start.
- Existing-state protection: the preparation boundary remains the sole owner
  of layout creation semantics. Existing and partial generations are denied,
  never adopted, repaired, overwritten, permission-mutated, or deleted.
- Policy denial: disabled, unauthorized, or otherwise denied requests create
  no generation. Non-process-capable advisory sessions likewise have no need
  for process-environment storage and create none.
- Audit failure: preparation necessarily precedes audit if the audit event is
  to describe the final allowed or denied start outcome. If persistence then
  fails, authority remains absent but the successfully verified generation is
  left untouched because this slice has no identity-bound recursive cleanup
  contract. The generation counter advances, so a later attempt cannot adopt
  that state.
- Partial preparation: a preparation exception or denial can leave private
  partial state under the existing `RQ-CF-AGENT-014` fail-closed contract. The
  controller does not guess ownership or perform path-based cleanup.
- Concurrency: preparation runs while the controller is in its `starting`
  transition. Concurrent start, stop, snapshot authority, and tool preflight
  cannot observe or replace a partially activated session; tool requests fail
  closed until the transition returns to idle.
- Compatibility boundary: the no-configuration constructor remains useful for
  policy, audit, and file-only non-executing lifecycle composition. It grants
  no environment plan; process-environment preflight still returns the stable
  unavailable diagnostic.
- Information exposure: preparation failures reach audit and the caller only
  as stable diagnostic codes. No path, identity, ACL, prompt, argument,
  credential, provider token, or environment value is added to lifecycle
  events.
- Execution boundary: this slice does not provision the trusted storage root,
  clean a generation, validate a Windows child parser or executable format,
  pin launch targets, start a process, apply a sandbox or endpoint policy,
  authenticate a provider, expose activation UI, or audit tool outcomes.

## Rollback

Rollback removes the configured-start preparation call, its supplied-
configuration state, focused lifecycle regressions, `RQ-CF-AGENT-016`, and
mapped current guidance. Any generation created before rollback remains
subject to an explicit identity-aware trusted-host cleanup decision; rollback
must not recursively delete unresolved state.

## Verification

Current candidate evidence:

- warning-free GCC Release security/community/isolation/safety selection:
  `17/17` pass, including the full safety traceability workflow contract;
- focused `test_workspace_agent_session` and
  `test_workspace_agent_isolated_environment`: `2/2` pass after the final
  non-process-capable lifecycle regression;
- fresh Clang 21 ASan/UBSan with leak detection: session,
  isolated-environment, and private-directory verification `3/3` pass with no
  finding;
- direct private-layout, audited-denial, invalid-configuration, policy-denial,
  audit-failure, and later-generation recovery assertions pass;
- `git diff --check` passes.

Still required before `RQ-CF-AGENT-016` is defined:

- protected Windows, Ubuntu, and macOS exact-head checks;
- DCO/signature and supply-chain checks;
- exact-head thread-aware review with every actionable finding addressed.

## Review evidence

- mode: high-severity maintainer self-review; no independent final safety
  approval claimed
- reviewer: rhamenator
- verification: authority ordering, invalid-configuration fail-closed behavior,
  existing-state preservation, audit-failure recovery, generation freshness,
  content-free denial, rollback, and traceability
- result: candidate Release `17/17` and sanitizer `3/3` verification passes;
  protected and
  exact-head review evidence remain pending
