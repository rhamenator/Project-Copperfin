# Workspace-Agent Audited Pending-Layout Cleanup Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-021` controller retention and explicit,
audit-bracketed cleanup of pending private workspace-agent generation layouts

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-005`,
`RQ-CF-AGENT-006`, `RQ-CF-AGENT-016`, `RQ-CF-AGENT-020`,
`HZ-system-failure-01`, and `HZ-data-corruption-01`

This report applies DO-178C-inspired assurance to a general-purpose C++/.NET
platform. It claims neither certification nor complete automatic lifecycle
cleanup.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-audited-cleanup-001`: after successful configured layout preparation, the controller shall retain the opaque same-boundary receipt in generation order whether or not start-audit commit succeeds | `DV-workspace-agent-audited-cleanup-001`; `DV-workspace-agent-audited-cleanup-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-audited-cleanup-002`: explicit cleanup shall be unavailable while session authority is active and shall serialize against start, stop, and other cleanup transitions | `DV-workspace-agent-audited-cleanup-001` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-audited-cleanup-003`: before invoking the identity-receipted cleanup primitive, the controller shall durably commit a content-free intent event for the oldest pending generation; missing or failed intent audit shall preserve both layout and receipt without attempting mutation | `DV-workspace-agent-audited-cleanup-001`; `DV-workspace-agent-audited-cleanup-002` | `HZ-data-corruption-01` |
| `DQ-workspace-agent-audited-cleanup-004`: after an attempted cleanup, the controller shall submit a content-free outcome event distinguishing `cleaned` from `retained`; failed or denied cleanup shall preserve the receipt for explicit retry, while successful cleanup shall consume it even if outcome audit fails | `DV-workspace-agent-audited-cleanup-001`; `DV-workspace-agent-audited-cleanup-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-audited-cleanup-005`: later session generations shall remain permitted without overwriting older pending receipts up to a fixed sixty-four-receipt cap; at the cap, another process-capable start shall be audited and denied before layout creation; cleanup shall remain explicit rather than automatic during stop or destruction | `DV-workspace-agent-audited-cleanup-001`; `DV-workspace-agent-audited-cleanup-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-audited-cleanup-001`: focused controller/environment
  tests cover active-session denial, successful stop-then-cleanup, consumed
  receipts, start-audit failure retention, failed intent audit, later retry,
  occupied-content preservation, audited retained outcomes, and denial before
  creation at the fixed pending-receipt cap.
- `DV-workspace-agent-audited-cleanup-002`: the durable audit-sink contract
  accepts only exact cleanup intent/outcome event forms and rejects malformed
  substitutions without mutation.
- `DV-workspace-agent-audited-cleanup-003`: broader workspace-agent,
  concurrency, isolation, safety, sanitizer, and protected platform evidence is
  required before implementation evidence is complete.

## Requirement delta

- Before: the controller discarded every successful preparation receipt, so
  neither an activated session nor a start-audit failure retained authority for
  later safe cleanup.
- After: the controller retains successful receipts FIFO and exposes one
  explicit cleanup operation. The operation requires inactive session
  authority and a committed intent audit before it invokes the existing
  identity-receipted empty-directory primitive, then submits a distinct outcome
  audit.

Potential Severity If Misused: high

## Hazard, misuse, boundary, and rollback analysis

- Authority ordering: cleanup runs only in the controller's serialized
  `cleaning` transition and is denied while a session is active. Start and stop
  cannot overlap it. The controller borrows the FIFO front only after entering
  that transition; the non-throwing pointer assignment avoids copying
  heap-backed receipt fields and cannot strand the transition on allocation
  failure.
- Receipt preservation: start-audit failure does not discard the opaque receipt.
  Multiple later generations append rather than replace older receipts, and
  explicit cleanup consumes only the oldest successful receipt. The queue is
  capped at sixty-four; capacity is reserved before layout mutation, and a
  further process-capable start is audited and denied before creation.
- Audit-before-mutation: absent, throwing, failed, or empty-receipt intent
  audit stops before filesystem mutation. A failed cleanup retains its receipt;
  a later explicit call may retry.
- Content preservation: the underlying `RQ-CF-AGENT-020` primitive remains
  non-recursive and preserves occupied or identity-changed paths. The outcome
  event contains only generation, fixed state, and a stable diagnostic.
- Outcome-audit failure: if filesystem cleanup succeeded, the receipt is
  consumed because retrying a destructive capability against an absent path is
  misleading. The already-committed intent record and returned result expose
  the incomplete audit outcome.
- Atomicity: retry after a failure before mutation and after a nonempty denial
  is supported. A late failure after some child directories were removed still
  cannot be retried successfully by the current all-objects preflight; the
  partial empty layout is preserved for operator recovery.
- Restart boundary: receipts are process-memory capabilities and are not
  serialized. Crash/restart recovery of abandoned layouts remains unavailable.
- Rollback: removing the controller method and retained-receipt member restores
  the prior fail-closed behavior. No start, stop, launch, or executor path
  depends on cleanup success.

## Verification

Current local focused evidence:

- warning-free GCC Release build of the session, isolated-environment, and
  durable audit-sink targets; and
- focused execution passes `3/3`; and
- the combined broader Release workspace-agent/platform and
  community/isolation/source-contract selection passes `16/16`; and
- fresh Clang 21 ASan/UBSan with leak detection passes `3/3`; and
- the repository-wide safety-traceability contract passes `1/1` in 331.54
  seconds; and
- `git diff --check` passes.

An attempted all-target GCC 15 warning-as-error build stopped in unchanged CDX
code on existing missing-field-initializer and unused-function warnings before
the unrelated Studio policy target could link. The normal Release selection
above isolates this slice; protected matrices remain authoritative for the
repository's supported compilers.

The initial signed/DCO head `52250dea9` received an exact-head automated review
that found one real P2 exception-safety gap: copying the cleanup receipt after
entering `cleaning` could throw and leave the controller wedged in that state.
Corrected signed/DCO head `4d3aaa558` borrows the stable FIFO front under the
transition invariant, passed all eleven protected checks in runs
`31913152575`, `31913153471`, `31913153473`, `31913153475`, and `31913153482`,
and merged through PR `#5032` as `2504b7b3b` after the review thread was
answered and resolved. The unrelated Python-sidecar timeout seen on the
superseded head did not recur; the corrected Windows generated-launcher job
passed in 18m34s. Implementation evidence is complete.

This evidence-only update passes the product-licensing, repository-community,
native-isolation, supply-chain-workflow, and safety-traceability contracts
`5/5`; the repository-wide safety scan completed in 323.97 seconds.

The requirement remains a `gap` solely for structured sign-off by a second
qualified human reviewer because this documentation's potential severity is
high. No automatic stop/destructor cleanup, crash-recoverable receipt,
partial-removal retry, nonempty-content disposition, process launch, sandbox,
endpoint enforcement, or executor readiness is claimed.
