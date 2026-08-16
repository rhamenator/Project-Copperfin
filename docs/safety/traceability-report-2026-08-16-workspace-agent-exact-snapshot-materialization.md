# Workspace-Agent Exact-Snapshot Materialization Traceability Report

Date: 2026-08-16

Scope: candidate v1 RQ-CF-AGENT-026 one-attempt private native-image
materialization

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from RQ-CF-AGENT-019,
RQ-CF-AGENT-020, RQ-CF-AGENT-024, RQ-CF-AGENT-025,
HZ-system-failure-01, and HZ-data-corruption-01.

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, process-launch readiness, nor suitability for a
safety-critical deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| DQ-workspace-agent-image-001: only the issuing trusted controller shall consume one opaque prepared candidate and only once | DV-workspace-agent-image-001; DV-workspace-agent-image-003 | HZ-system-failure-01; HZ-data-corruption-01 |
| DQ-workspace-agent-image-002: materialization shall use only the candidate's immutable retained snapshot beneath the exact receipted generation temp directory | DV-workspace-agent-image-001; DV-workspace-agent-image-002; DV-workspace-agent-image-004 | HZ-system-failure-01; HZ-data-corruption-01 |
| DQ-workspace-agent-image-003: the native image shall be newly created, privately retained, byte-exact, and removed before pins and lease release | DV-workspace-agent-image-001; DV-workspace-agent-image-002; DV-workspace-agent-image-003; DV-workspace-agent-image-004 | HZ-system-failure-01; HZ-data-corruption-01 |
| DQ-workspace-agent-image-004: the opaque result shall expose no native authority and shall not execute or weaken the invariant promotion denial | DV-workspace-agent-image-001; DV-workspace-agent-image-003; DV-workspace-agent-image-004 | HZ-system-failure-01; HZ-data-corruption-01 |

- DV-workspace-agent-image-001: focused controller regressions cover default
  denial, cross-controller rejection, live-source mutation after preparation,
  exact-snapshot materialization, stop blocking, orderly cleanup,
  identity-bound layout cleanup, and unchanged launch denial.
- DV-workspace-agent-image-002: platform regressions cover exact bytes,
  absent-leaf creation, existing-leaf preservation, parent identity failure,
  POSIX unlink-before-success, Windows write-sharing denial, and destruction.
- DV-workspace-agent-image-003: compile-time and machine-readable contracts
  prove move-only opacity, controller-only access, source registration, and
  hosted Windows/Ubuntu/macOS scheduling.
- DV-workspace-agent-image-004: warning-free Release, sanitizers, repeated
  lifecycle execution, broader safety/isolation contracts, protected checks,
  diff validation, and exact-head review are required before implementation
  evidence is complete.

## Hazard, misuse, boundary, and rollback analysis

- Mutable-source substitution: the controller supplies only the immutable byte
  snapshot captured by RQ-CF-AGENT-024; it never reopens the executable source
  path for materialization.
- Candidate forgery, replay, or transfer: the candidate carries a private
  issuing-controller marker and is consumed by value. Cross-controller and
  moved-from candidates fail closed without exposing their contents.
- Parent or leaf substitution: creation is exclusive beneath the exact private
  temp-directory storage/file/creation identity. The creation identity is
  carried into and rechecked through the lower primitive's retained native
  parent object, closing replacement between receipt validation and leaf
  creation. Existing leaves are preserved. POSIX uses descriptor-relative
  no-follow creation and immediate unlink; Windows retains the parent identity
  around exclusive creation and retains the exact image handle without
  ordinary write/delete sharing.
- Partial write, allocation, or verification failure: construction is inside a
  catch-all denial boundary. Non-allocating local RAII owns each parent and
  image native resource immediately after open/create, retries POSIX leaf
  cleanup before releasing the parent descriptor, and transfers the image only
  after final object allocation succeeds. No image authority is returned until
  exact size, shape, and byte reread pass.
- Revocation and cleanup ordering: the image is destroyed before its prepared
  candidate; that candidate closes pins before releasing the generation lease.
  A live result therefore delays stop until every launch-adjacent resource is
  discarded.
- Stable directory identity: legitimate controlled image creation changes
  modification timestamps. Cleanup binds storage/file/creation identity and
  separately revalidates every direct private child. A missing creation time
  fails closed. This prevents rapid remove/recreate inode reuse from
  impersonating the original directory without treating mutable modification
  metadata as object identity.
- Windows residual: retaining a handle that denies writers protects
  immutability but is not claimed compatible with a future CreateProcessW file
  open. A verified launch-transition primitive is still required.
- POSIX/macOS residual: the image is unlinked and descriptor-retained. Portable
  direct execution of that descriptor, particularly on macOS, is not
  established by this slice.
- Non-claims: this slice does not enter the working directory, create a
  process, apply a sandbox, enforce endpoint/descendant policy, audit an
  outcome, or expose provider credentials. RQ-CF-AGENT-019 remains denied.
- Rollback: remove the portable image primitive, environment friend boundary,
  controller materialization wrapper, workflow/contract scheduling, and
  focused regressions together. Preserve the earlier immutable-snapshot,
  prepared-candidate, target-pin, lease, private-layout, and invariant-denial
  boundaries.

Potential Severity If Misused: high

## Verification

Current local evidence:

- GCC Release behavior and boundary execution passes
  test_platform_private_directory,
  test_platform_private_directory_boundary_contract,
  test_workspace_agent_process_image_materialization_contract,
  test_workspace_agent_session, and
  test_workspace_agent_isolated_environment at 5/5;
- the workflow path-filter contract passes;
- fifty repeated isolated-environment lifecycle executions pass;
- fresh Clang ASan/UBSan with leak detection passes the platform-directory,
  session, and isolated-environment targets at 3/3 without findings;
- fresh GCC ThreadSanitizer passes the session and isolated-environment targets
  at 2/2 without findings;
- the broader Release workspace-agent and materialization set passes 11/11;
- the first repository licensing/community/native-platform/Actions/isolation/
  supply-chain/safety sweep passed the 339.49-second safety scan and six other
  contracts; it exposed only missing isolation metadata for the new contract,
  which was added before the isolation and materialization contracts passed
  together at 2/2;
- focused regressions prove cross-controller rejection, source-mutation
  isolation, POSIX path absence, stop blocking, image-before-lease cleanup,
  existing-leaf preservation, and unchanged invariant launch denial;
- the first protected Ubuntu run reproduced immediate inode reuse after child
  replacement; requiring nonzero matching platform creation identity corrects
  that gap, and the corrected focused set passes 6/6 plus one hundred repeated
  isolated-environment lifecycle runs;
- a subsequent exact-head audit found and corrected a check/use gap where the
  environment verified creation identity but the lower native-parent bracket
  accepted only storage/file identity; direct creation-identity mismatch
  coverage and the materialization machine contract pass;
- a further exact-head audit found raw parent/image native resources could leak
  if final object allocation threw, and a failed first POSIX unlink could lose
  cleanup authority by closing the parent too early; immediate RAII ownership,
  retained-parent cleanup retry, and machine-contract coverage correct both
  paths; and
- git diff --check is required before publication.

Final-byte safety validation is required after this evidence update. Protected
Windows, Ubuntu, and macOS execution and exact-head review are also pending.
Requirement status therefore remains gap. No qualified high-severity human
sign-off is claimed.
