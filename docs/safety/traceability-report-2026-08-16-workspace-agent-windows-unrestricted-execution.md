# Workspace-Agent Windows Unrestricted Execution Traceability Report

Date: 2026-08-16

Scope: candidate v1 `RQ-CF-AGENT-028` Windows-only, warned-unrestricted,
exact-image bounded execution

Allowed requirement source: explicit repository-owner product policy for a
useful Windows-first built-in assistant under H3/I2; derived from
`RQ-CF-AGENT-001`, `RQ-CF-AGENT-019`, `RQ-CF-AGENT-025` through
`RQ-CF-AGENT-027`, `HZ-system-failure-01`, and `HZ-data-corruption-01`.

This report applies Copperfin's DO-178C-inspired general-purpose quality
baseline. It claims neither formal compliance nor certification, sandbox
containment, general assistant readiness, nor suitability for a safety-critical
use.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-windows-execution-001`: only one same-controller exact-generation materialized launch in explicitly warned unrestricted-local mode on a confirmed non-elevated Windows host may reach the private launcher | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-002`: caller controls shall contain only bounded transport and cancellation fields; the executable, arguments, environment, working directory, mode, flags, and native authority shall come only from the retained plan and private image | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-003`: a durable content-free intent shall precede any process attempt and a status-consistent correlated content-free outcome shall be submitted after image cleanup; failed intent audit shall start nothing | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-004`: the child shall start suspended, inherit only fixed standard handles, enter a kill-on-close Job Object before resume, obey bounded transport/time/cancellation controls, and leave no authorized descendant | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-005`: Job Object assignment shall commit launch and release the exact-generation lease so stop can revoke before a long-running child exits, while the private image remains owned until the process tree closes | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-006`: workspace-sandbox, stale or cross-controller authority, invalid controls, elevated or unknown elevation, and non-Windows hosts shall consume the attempt and fail closed without execution; the public promotion gate shall remain denied | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-002`; `DV-workspace-agent-windows-execution-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-007`: admitted images shall attest launch-wide system-only dependency behavior; the private image shall not fall back to mutable source-adjacent or working-directory DLLs | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-008`: same-controller lifecycle reentry on a synchronous audit callback's own thread shall fail closed without waiting on that stack's revocation lease; unrelated concurrent stop shall retain normal revocation semantics | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-005` | `HZ-system-failure-01` |
| `DQ-workspace-agent-windows-execution-009`: every Windows directory component from the volume root through the private image parent shall remain held without delete sharing through process completion, and a final pathname reopen under that retained chain shall match the authenticated image identity before launch | `DV-workspace-agent-windows-execution-003`; `DV-workspace-agent-windows-execution-006` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-windows-execution-001`: focused controller regressions
  exercise one-attempt consumption, sandbox and platform denial, failed-intent
  no-launch behavior, exact command/environment/working-directory use, output,
  exit, cleanup, and revocation lifecycle.
- `DV-workspace-agent-windows-execution-002`: durable audit-sink regressions
  admit only schema-v2 content-free intent/outcome records and reject zero
  operation identifiers, injected diagnostics, mode mismatches, and
  status/diagnostic mismatches. The machine contract preserves the private
  exact-image seam and invariant public boundary.
- `DV-workspace-agent-windows-execution-003`: protected Windows must execute the
  exact image and the long-running-child lease regression; Ubuntu and macOS must
  exercise fail-closed platform denial. Broader security, isolation, safety,
  exact-head review, and merge evidence are required before implementation
  evidence is complete.
- `DV-workspace-agent-windows-execution-004`: parser/dependency configuration
  contracts require the exact-digest `self_contained_launch_image_v1`
  attestation and reject absent or unknown dependency contracts.
- `DV-workspace-agent-windows-execution-005`: a synchronous intent-audit sink
  attempts same-controller `stop()`; the call is denied without deadlock and a
  later ordinary stop and cleanup succeed. A distinct slow-intent fixture proves
  an unrelated thread's stop waits and then revokes instead of being denied.
- `DV-workspace-agent-windows-execution-006`: the Windows private-image
  regression attempts to rename and replace an image ancestor while the opaque
  image remains live and requires denial; protected Windows execution then
  proves `CreateProcessW` still launches the exact retained image.

## Hazard, misuse, boundary, and rollback analysis

- Unrestricted authority: this mode is intentionally dangerous. The child has
  the current user's operating-system access and is not confined to the
  workspace. The exact versioned warning and explicit consent remain mandatory.
  The implementation never requests elevation and refuses an elevated or
  indeterminate host token.
- False sandbox claim: `workspace_sandbox` cannot execute because path
  containment is not an operating-system sandbox. Denial is explicit and
  covered rather than silently falling through to unrestricted execution.
- Command substitution: the executor accepts no executable path, arguments,
  environment entries, working directory, mode, or flags. It consumes the
  authenticated retained plan and supplies the private exact image internally
  as `lpApplicationName`; no shell, PATH search, globbing, or response-file
  interpretation is added.
- Mutable-source replacement: the launcher never reopens or executes the
  original tool source. The RQ-027 read-only identity-bound private image
  remains live throughout process completion and cleanup. No-delete-share
  handles retain the complete Windows directory chain through that lifetime,
  and a final identity-matched pathname reopen after chain acquisition prevents
  ancestor rename/replacement from redirecting the loader. RQ-018's exact-
  digest launch-wide dependency attestation rejects tools that need
  application-local or working-directory DLLs until an authenticated
  dependency-closure design exists.
- Descendant escape and resource exhaustion: the process is created suspended
  and assigned to a kill-on-close Job Object before resume. Timeout,
  cancellation, output limits, transport failure, and ordinary completion all
  close the owned tree. Transport and timeout limits are fixed and bounded.
- Revocation race: launch commitment occurs only after successful Job Object
  assignment. Releasing the plan, pins, and lease there lets stop finish while
  the child is still bounded, without destroying the private image until the
  owned tree closes.
- Audit loss or disclosure: failed intent audit starts nothing. Process audit
  records contain only schema, kind, generation, mode, operation identifier,
  stable outcome, and stable diagnostic. Output, paths, arguments, environment,
  prompts, credentials, receipts, and native errors are excluded. Outcome-audit
  failure remains visible but cannot retroactively stop a completed attempt.
  Crash-recoverable intent reconciliation is not yet implemented.
- Audit-callback reentry: same-controller lifecycle changes on the callback's
  own thread are not supported. They fail closed immediately, preventing that
  callback from waiting on its own process-intent lease. Thread-local tracking
  preserves ordinary concurrent stop, which waits and then revokes normally.
- Rollback: revert RQ-028's controller method, private bounded-process seam,
  schema-v2 process-audit validation, regressions, and this documentation
  together. RQ-025 through RQ-027 then remain safe non-executing prerequisites,
  and RQ-019 continues to deny public promotion.

Potential Severity If Misused: high

## Verification

Current direct evidence:

- local Debug focused behavior and contract execution passes `5/5` for bounded
  process, workspace-agent session, isolated environment, durable audit sink,
  and the exact-snapshot machine contract;
- fresh GCC 15 Release with `-Werror` passes the same `5/5` without warnings;
- fresh Clang 21 ASan/UBSan with leak detection passes session, isolated
  environment, and durable audit sink at `3/3` without findings;
- licensing, community, release-license, isolation, supply-chain, and safety
  contracts pass `6/6`, including the 327.99-second repository-wide safety
  scan;
- the extended exact-snapshot materialization machine contract passes and
  verifies the private runner, suspended `CreateProcessW`, Job Object assignment,
  launch-commit callback, controller-only consumption, sandbox denial, and
  revocation-lifecycle regression;
- Linux directly exercises the non-Windows denial and content-free paired audit;
  and
- `git diff --check` passes before publication.

Protected Windows execution, protected Ubuntu/macOS denial, warning-as-error and
sanitizer evidence, exact-head automated review, thread resolution, merge
identity, and qualified high-severity documentation sign-off remain pending.
Requirement status therefore remains `gap`. This slice is not a real sandbox,
does not connect provider/model output to execution, and does not make the
public promotion gate available.

Initial protected Windows environment run `31946401986` compiled the product
libraries but rejected the test-only child fixture because MSVC treats direct
`getenv` use as deprecated under that target's warning policy. The fixture now
uses Copperfin's portable environment reader, preserving the same absent-secret
assertion without weakening warnings. Focused local behavior and the machine
contract pass `2/2`; a corrected protected Windows rerun remains required.

Corrected-head generated-launcher run `31947228580` then compiled and passed the
private-image, parser, session, and contract targets, but its elevated hosted
runner could not satisfy RQ-028's deliberate non-elevated execution condition;
both direct execution assertions failed. The Windows-only test driver now
detects that validation-host condition and reexecutes itself once with a test-
owned `LUA_TOKEN | DISABLE_MAX_PRIVILEGE` restricted token, with a bounded wait
and explicit proof that the child is non-elevated. Product execution still
refuses elevated or indeterminate hosts; no production privilege or policy was
loosened. A fresh protected rerun is required.
