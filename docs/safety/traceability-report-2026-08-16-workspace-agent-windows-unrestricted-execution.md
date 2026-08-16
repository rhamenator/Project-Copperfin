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
| `DQ-workspace-agent-windows-execution-004`: the child shall start suspended, inherit only fixed standard handles, enter a kill-on-close Job Object atomically at creation, obey bounded transport/time/cancellation controls, and leave no authorized descendant | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-005`: confirmed Job Object ownership and pre-resume image binding shall commit launch and release the exact-generation lease so stop can revoke before a long-running child exits, while the private image remains owned until the process tree closes | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-006`: workspace-sandbox, stale or cross-controller authority, invalid controls, elevated or unknown elevation, and non-Windows hosts shall consume the attempt and fail closed without execution; the public promotion gate shall remain denied | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-002`; `DV-workspace-agent-windows-execution-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-007`: admitted images shall attest launch-wide system-only dependency behavior; the private image shall not fall back to mutable source-adjacent or working-directory DLLs | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-008`: same-controller lifecycle reentry on a synchronous audit or cancellation callback's own thread shall fail closed without waiting on that stack's revocation lease; unrelated concurrent stop shall retain normal revocation semantics | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-005` | `HZ-system-failure-01` |
| `DQ-workspace-agent-windows-execution-009`: the private image's stable identity path shall be derived from its authenticated handle as a local-volume device path (volume GUID preferred, validated `GLOBALROOT\\Device\\HarddiskVolumeN` fallback); every renameable component below that root through the private image parent shall remain held without delete sharing through process completion, and final stable- and DOS-path reopens under that retained chain shall match the authenticated image identity before creation | `DV-workspace-agent-windows-execution-003`; `DV-workspace-agent-windows-execution-006` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-010`: anonymous transport pipes shall use a protected current-logon and restricted-code DACL, remain creatable by a restricted non-elevated host, and expose only the fixed child endpoints through the explicit inheritance list | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-011`: the working directory shall be converted from its authenticated handle to the same accepted stable local-volume device form, and every renameable component beneath that explicitly parsed device root shall remain held without delete sharing until `CreateProcessW` has consumed the path | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-007` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-012`: each process intent/outcome pair shall use one fresh 128-bit operating-system-random attempt namespace plus a nonzero operation identifier from a process-wide, nonwrapping counter so records from controllers, processes, restarts, and later post-fork attempts sharing a durable sink have collision-resistant correlation identities; random-source or process-identity failure shall deny execution before intent, and a process-identity change across the synchronous intent callback shall deny the forked continuation before execution or outcome submission | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-002`; `DV-workspace-agent-windows-execution-008` | `HZ-system-failure-01` |
| `DQ-workspace-agent-windows-execution-013`: any pre-start compatibility diagnostic child shall enter the same kill-on-close Job Object atomically at creation, remain suspended, never execute user code, and retain cleanup ownership even if explicit termination fails | `DV-workspace-agent-windows-execution-003`; exact-source machine contract | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-014`: after protected Windows proves the stable device-form application name unsupported, a diagnostic-only DOS application name shall be derived from the authenticated handle, identity-matched under the retained stable hierarchy, and used only by a never-resumed Job-owned probe that retains the stable working directory; it shall not become an execution fallback without post-creation image binding | `DV-workspace-agent-windows-execution-003`; `DV-workspace-agent-windows-execution-006`; exact-source machine contract | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-execution-015`: after protected Windows proves the DOS application name and stable working-directory combination compatible, production creation shall use that handle-derived DOS name only while the stable hierarchy remains retained, place the suspended child into the kill-on-close Job Object atomically, compare its kernel-reported native image name with the native device name captured from the authenticated image handle, and terminate without resume on query or binding failure | `DV-workspace-agent-windows-execution-001`; `DV-workspace-agent-windows-execution-003`; `DV-workspace-agent-windows-execution-006`; exact-source machine contract | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-windows-execution-001`: focused controller regressions
  exercise one-attempt consumption, sandbox and platform denial, failed-intent
  no-launch behavior, exact command/environment/working-directory use, output,
  exit, cleanup, and revocation lifecycle.
- `DV-workspace-agent-windows-execution-002`: durable audit-sink regressions
  admit only schema-v2 content-free intent/outcome records and reject zero
  missing, malformed, or injected process-instance identifiers, zero operation
  identifiers, injected diagnostics, mode mismatches, and
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
  later ordinary stop and cleanup succeed. A cancellation callback performs the
  same reentrant stop attempt before process creation and must receive its own
  stable denial without deadlock. A distinct slow-intent fixture proves an
  unrelated thread's stop waits and then revokes instead of being denied.
- `DV-workspace-agent-windows-execution-006`: the Windows private-image
  regression attempts to rename and replace an image ancestor while the opaque
  image remains live and requires denial, then requires rename to succeed after
  destruction proves every hierarchy handle was released; protected Windows
  execution then proves `CreateProcessW` still launches the exact retained
  image.
- `DV-workspace-agent-windows-execution-007`: Windows target-pin coverage
  attempts to rename an ancestor of the authenticated working directory while
  the opaque pins remain live and requires denial, then requires the rename to
  succeed after release. Hosted execution proves the handle-derived working
  path remains acceptable to `CreateProcessW`.
- `DV-workspace-agent-windows-execution-008`: controller execution coverage
  requires nonzero paired identifiers and proves attempts from distinct
  controller instances receive different identifiers.

The final path observer requests only read access and shares delete solely so
it can coexist with the already-authenticated retained handle's own delete
access. The retained handle still omits write and delete sharing, so the
observer does not admit a writer, deleter, or renamer.

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
- Mutable-source and path-root replacement: the launcher never reopens or executes the
  original tool source. The RQ-027 read-only identity-bound private image
  remains live throughout process completion and cleanup. No-delete-share
  handles retain the complete renameable Windows directory chain below a
  handle-derived stable local-volume device root through that lifetime,
  and a final identity-matched pathname reopen after chain acquisition prevents
  ancestor rename/replacement from redirecting the loader. RQ-018's exact-
  digest launch-wide dependency attestation rejects tools that need
  application-local or working-directory DLLs until an authenticated
  dependency-closure design exists.
- Working-directory redirection: the target pins derive a stable local-volume device path from
  the authenticated directory handle and retain every renameable component of
  that path until process creation commits. A drive-letter, `SUBST`, mapped-
  drive, leaf, or ancestor redirection therefore cannot substitute the child's
  current directory between admission and `CreateProcessW`.
  Windows network shares do not expose an accepted local-volume device path and therefore fail
  closed at this boundary; the v1 executor supports local fixed-volume targets.
- Descendant escape and resource exhaustion: the process is created suspended
  and assigned to a kill-on-close Job Object before resume. Timeout,
  cancellation, output limits, transport failure, and ordinary completion all
  close the owned tree. Transport and timeout limits are fixed and bounded.
- Transport authority: each anonymous pipe receives an explicit protected DACL
  for the current logon and Windows restricted-code identities rather than an
  ambient token default. This permits the warned non-elevated boundary to work
  under a restricted host without granting another logon session access. Only
  the fixed child endpoints are inheritable and admitted to the process handle
  list; every parent endpoint has inheritance removed before launch.
- Revocation race: launch commitment occurs only after successful Job Object
  assignment. Releasing the plan, pins, and lease there lets stop finish while
  the child is still bounded, without destroying the private image until the
  owned tree closes.
- Audit loss or disclosure: failed intent audit starts nothing. Process audit
  records contain only schema, kind, generation, mode, process-instance and
  operation identifiers,
  stable outcome, and stable diagnostic. Output, paths, arguments, environment,
  prompts, credentials, receipts, and native errors are excluded. Outcome-audit
  failure remains visible but cannot retroactively stop a completed attempt.
  Crash-recoverable intent reconciliation is not yet implemented.
  Correlation uses a fresh 128-bit namespace from the operating system's random
  source for each attempt plus one process-wide nonwrapping counter. Distinct
  controllers share the counter, while distinct processes, restarts, and later
  post-fork attempts receive collision-resistant correlation namespaces. A
  process-identity check across the synchronous intent callback prevents a
  callback-side fork from executing or submitting a duplicate outcome.
  Random-source or process-identity failure denies execution before intent.
- Callback reentry: same-controller lifecycle changes from an audit or
  cancellation callback's own thread are not supported. They fail closed
  immediately, preventing either callback from waiting on its own retained
  launch lease. Thread-local tracking preserves ordinary concurrent stop,
  which waits and then revokes normally.
- Rollback: revert RQ-028's controller method, private bounded-process seam,
  schema-v2 process-audit validation, regressions, and this documentation
  together. RQ-025 through RQ-027 then remain safe non-executing prerequisites,
  and RQ-019 continues to deny public promotion.

Potential Severity If Misused: high

## Verification

Current direct evidence:

- local Debug focused behavior and contract execution passes `7/7` for bounded
  process, workspace-agent session and containment, isolated environment,
  durable audit sink, private image, and the exact-snapshot machine contract;
- fresh GCC 15 Release with `-Werror` passes the same `7/7` without warnings;
- fresh Clang 21 ASan/UBSan with leak detection passes target containment,
  session, isolated environment, private image, bounded process, durable audit
  sink, and the machine contract at `7/7`
  without findings;
- licensing, community, release-license, isolation, supply-chain, and safety
  contracts pass `6/6`, including the corrected-head 328.63-second repository-wide safety
  scan;
- the extended exact-snapshot materialization machine contract passes and
  verifies the private runner, suspended `CreateProcessW`, atomic Job Object ownership,
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

Later protected run `31953223676` proved private-image materialization,
pathname locking, cleanup, and all parser/session contracts, then isolated the
remaining execution failure to anonymous transport-pipe creation under the LUA
test token (`ERROR_ACCESS_DENIED`). The pipe objects now use an explicit
current-logon plus restricted-code DACL instead of the restricted token's
ambient default DACL. A fresh protected execution remains required.

Corrected pipe head run `31954380866` then proved that both private children
start under the restricted token, but they timed out before reaching the test
entry point. The execution fixture had retained a deliberately fake Windows
system root used by environment-construction and identity-mutation tests. Real
PE execution now uses the host Windows directory captured through the same
trusted system-root boundary; fake-root tests remain separate. Product code and
the no-parent-environment policy are unchanged. A fresh protected execution
remains required.

Exact head `42c7d514c` then made private-image materialization, explicit stable
local-device root parsing, and the private-directory regressions pass on
Windows, but `CreateProcessW` rejected the combined stable application and
working-directory device paths with `ERROR_INVALID_PARAMETER`. A bounded
diagnostic correction retried only that pre-start failure with the same stable
application and inherited current directory, still suspended. Exact protected
head `0023f74e9` reported
`polyglot.process.executable_path_unsupported`, native error `87`, proving the
stable device-form application name is incompatible. The next bounded probe
used a handle-derived, identity-matched extended DOS application name with the
same stable working directory, still suspended. It immediately
places the diagnostic child into the kill-on-close Job Object atomically at
creation, terminates the never-resumed process, and reports whether the
unsupported parameter is the stable application path or `lpCurrentDirectory`;
it never falls back to executing with a weaker pathname. Job ownership remains
effective if explicit termination fails, so the diagnostic cannot be orphaned.
Protected run `31961744412` again reported
`polyglot.process.executable_path_unsupported` with native error `87`, which in
that exact diagnostic head means the DOS application name plus retained stable
working directory successfully created a suspended child. The final design
therefore uses the handle-derived DOS application name, makes Job ownership
atomic at creation, and compares the suspended child's kernel-reported native
image name with the retained authenticated handle's native device name before
launch commitment or resume. Query or comparison failure terminates the owned
tree and reports the fixed content-free image-binding diagnostic.
