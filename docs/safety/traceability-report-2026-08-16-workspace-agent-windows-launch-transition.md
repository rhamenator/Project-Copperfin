# Workspace-Agent Windows Launch-Handle Transition Traceability Report

Date: 2026-08-16

Scope: candidate v1 `RQ-CF-AGENT-027` non-executing Windows immutable-image
launch-handle transition

Allowed requirement source: explicit repository-owner product policy for a
useful Windows-first built-in assistant under H3/I2; derived from
`RQ-CF-AGENT-026`, `HZ-system-failure-01`, and `HZ-data-corruption-01`.

This report applies Copperfin's DO-178C-inspired general-purpose quality
baseline. It claims neither formal compliance nor certification, process-tool
activation, sandbox containment, nor suitability for a safety-critical use.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-windows-launch-transition-001`: after the write-capable handle closes, a volume-relative file identifier shall anchor the exact object and only a linked-path read/delete handle with equal identity may become retained authority | `DV-workspace-agent-windows-launch-transition-001`; `DV-workspace-agent-windows-launch-transition-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-launch-transition-002`: success shall retain no write access, shall deny new write/delete/rename opens, shall rebind the published pathname to the exact retained identity after the transition, and shall preserve exact bytes and cleanup ownership | `DV-workspace-agent-windows-launch-transition-001`; `DV-workspace-agent-windows-launch-transition-002`; `DV-workspace-agent-windows-launch-transition-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-launch-transition-003`: the transitioned image shall admit a direct `CreateProcessW` loader open without exposing a path, handle, identifier, or execution operation through the product boundary | `DV-workspace-agent-windows-launch-transition-001`; `DV-workspace-agent-windows-launch-transition-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-launch-transition-004`: denial shall return no authority and shall never adopt or delete an identity-mismatched object | `DV-workspace-agent-windows-launch-transition-002`; `DV-workspace-agent-windows-launch-transition-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-launch-transition-005`: a writable file mapping created during the close/reopen interval shall prevent the final open that omits write sharing, including after the mapping's writer handle closes | `DV-workspace-agent-windows-launch-transition-001`; `DV-workspace-agent-windows-launch-transition-002`; `DV-workspace-agent-windows-launch-transition-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-windows-launch-transition-001`: the Windows platform
  regression materializes the running test executable, opens a reader without
  sharing write access, denies cooperating write/delete/rename opens, proves a
  live writable mapping outlives its writer handle but still causes the final
  no-write-sharing open to fail, launches the
  fixed test-only child with `CreateProcessW`, waits for its exact success, and
  proves destruction removes the image.
- `DV-workspace-agent-windows-launch-transition-002`: the machine-readable
  contract requires a file-id identity anchor, identity-equal linked-path
  read/delete access, launch-transition
  failure state, exact-byte verification, and the RQ-027 regression markers.
- `DV-workspace-agent-windows-launch-transition-003`: focused Release behavior,
  broader workspace-agent/isolation/safety contracts, protected Windows,
  Ubuntu, and macOS execution, diff validation, and exact-head review are
  required before implementation evidence is complete.

## Hazard, misuse, boundary, and rollback analysis

- Path substitution: transition uses `OpenFileById` with the still-live
  verified parent as the volume hint to retain a read-only identity anchor,
  then opens the non-reparse leaf for read/delete access and requires the same
  volume/file/creation/size identity before that path handle becomes authority.
  The final path handle denies further delete/rename access. A renamed original or replacement leaf
  therefore fails without adopting or deleting the replacement.
- Cooperating writer: a writer remaining open conflicts with the final
  read-only sharing request. A writer that changes bytes and closes is detected
  by the complete post-transition reread. The final handle itself has no write
  access and denies later write, delete, and rename requests.
- Writable mapping: the Windows sharing contract treats a live writable file
  mapping as write access even after the creating handle closes. Because the
  final path open omits `FILE_SHARE_WRITE`, such a mapping produces
  `ERROR_SHARING_VIOLATION`. A dedicated hosted regression creates a
  `PAGE_READWRITE`/`FILE_MAP_WRITE` mapping from a read/write handle with no
  delete access, closes its writer, mutates through the view, and requires that
  denial before the mapping is released. The fixture therefore cannot obtain
  its expected failure from a retained delete/share constraint.
- Identifier reuse: volume/file/creation identity and exact size are compared
  before byte verification. An identity mismatch is closed without deletion,
  so a reused identifier cannot confer cleanup authority over another object.
- Unsupported filesystem: `OpenFileById` failure returns no image. Cleanup is
  identity-bound and best effort; hostile same-user sharing can retain a
  non-authoritative private leaf and consequently block later empty-layout
  cleanup rather than allowing unsafe adoption.
- Allocation and cleanup: native handles remain under non-allocating RAII.
  Final object allocation transfers the identity-matched pathname handle only after it
  succeeds; otherwise the exact object is marked for deletion and closed.
- Test execution: the regression launches only the copied test executable with
  one fixed `--rq027-child` argument; that child immediately exits. It is not a
  product executor or user-controlled command surface. Test isolation metadata
  classifies the process as bounded with a child-scoped environment.
- Non-claims: no product API exposes the image path, identifier, or handle; no
  controller launch operation is added; the invariant promotion gate remains
  denied. POSIX/macOS execution, working-directory entry, sandbox, endpoints,
  descendants, outcome audit, provider/OAuth, diff, and undo remain gaps.
- Rollback: revert the Windows reopen-by-id transition, its dedicated failure
  state and test-child path, the machine-contract additions, and this
  requirement/report together. The prior `RQ-CF-AGENT-026` exact materialized
  image remains the safe non-executing fallback.

Potential Severity If Misused: high

## Verification

Current direct evidence:

- warning-as-error GCC 15 Release behavior and contract execution passes `4/4`;
- focused Release platform, controller, workflow, and machine-contract
  execution passes `7/7`;
- fresh Clang 21 ASan/UBSan with leak detection passes the platform-directory,
  session, and isolated-environment targets at `3/3` without findings;
- licensing, community, isolation, supply-chain, and safety contracts pass
  `6/6` after the pathname-binding and lifecycle correction, including the 333.46-second
  repository-wide safety scan;
- Linux directly exercises the unchanged POSIX materialization behavior and
  the source contract; and
- `git diff --check` passes.

Protected Windows run `31939430334` rejected the first pathname-binding
correction because the retained `OpenFileById` handle did not preserve the
required delete/share lifecycle on that hosted filesystem. The corrected trust
model limits that handle to read-only identity anchoring and transfers
authority only to an identity-equal ordinary path handle, reusing the
previously verified Windows cleanup/share primitive. Replacement protected
evidence is required.

Direct Windows compile and execution, protected cross-platform workflows,
exact-head review, and qualified high-severity documentation sign-off remain
pending. Requirement status is therefore `gap`; no Windows launch-readiness
claim is made yet.
