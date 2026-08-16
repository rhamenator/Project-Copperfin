# Workspace-Agent Pinned Executable Authentication Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-024` bounded byte authentication and private
immutable executable-snapshot retention

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-019`,
`RQ-CF-AGENT-022`, `RQ-CF-AGENT-023`, `HZ-system-failure-01`, and
`HZ-data-corruption-01`

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, launch readiness, nor suitability for a
safety-critical deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-pinned-auth-001`: only a pin-authorizing inspection shall stream and privately bind the exact executable digest; ordinary point-in-time preflight shall carry no pin authority and shall avoid the streaming cost | `DV-workspace-agent-pinned-auth-001`; `DV-workspace-agent-pinned-auth-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-pinned-auth-002`: inspection and acquisition shall stream through an already-open object within 256 MiB and bracket every filesystem stream with the exact physical identity; successful acquisition shall privately retain the exact bytes hashed by that same stream | `DV-workspace-agent-pinned-auth-001`; `DV-workspace-agent-pinned-auth-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-pinned-auth-003`: acquisition shall require the private inspection digest, while later reverification shall hash only the immutable private snapshot and expose neither bytes, digest, path, nor native handle | `DV-workspace-agent-pinned-auth-001`; `DV-workspace-agent-pinned-auth-002`; `DV-workspace-agent-pinned-auth-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-pinned-auth-004`: authentication shall remain resource verification only and shall not substitute for the revocation lease, exact-snapshot execution, sandbox, endpoint/descendant controls, outcome audit, or the invariant launch denial | `DV-workspace-agent-pinned-auth-001`; `DV-workspace-agent-pinned-auth-002`; `DV-workspace-agent-pinned-auth-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-pinned-auth-001`: focused boundary/controller regression
  proves restored-metadata byte-change denial, ordinary-preflight separation,
  cap denial, private-snapshot verification after POSIX rename and later source
  mutation, post-stop resource-only behavior, and the unchanged launch gate.
- `DV-workspace-agent-pinned-auth-002`: compile-time and source contracts prove
  move-only opacity and the absence of a public digest, path, native handle, or
  launch operation.
- `DV-workspace-agent-pinned-auth-003`: warning-free Release, sanitizer,
  broader safety/isolation, protected Windows/Ubuntu/macOS, diff, and exact-head
  review evidence are required before implementation evidence is complete.

## Hazard, misuse, boundary, and rollback analysis

- Path and metadata substitution: a pin-authorizing inspection opens and hashes
  the exact identity-checked executable. Pin acquisition opens the retained
  object, hashes it again, requires the private digest, and privately retains
  the exact bytes from that same stream. A same-object byte edit before
  acquisition is rejected even when public size, file identity, link count,
  and modification timestamp are restored.
- Streaming race: identity is read from the same native handle or descriptor
  before and after each complete stream. A size, timestamp, link, type, or
  object-identity change fails closed. Digest mismatch covers content changes
  that preserve the public identity tuple.
- Resource exhaustion: the fixed 256 MiB cap is checked from the opened-object
  identity before reading and enforced again by the streaming reader. Hashing
  uses one fixed 64 KiB stack buffer plus the existing bounded hash context;
  pin acquisition additionally retains at most 256 MiB on the heap as the
  immutable executable snapshot and fails closed on allocation failure.
- Windows behavior: pin-authorizing inspection and retained acquisition use
  non-inheriting handles that omit ordinary write/delete sharing. Authentication
  reads those handles and resets their file position. The later retained handle
  continues to exclude ordinary replacement while alive.
- POSIX mutable-object race: an identity-bracketed multi-chunk `pread` cannot
  exclude a cooperating writer. Acquisition therefore retains the exact bytes
  produced by the accepted stream, and later reverification hashes only that
  immutable private snapshot. A future executor must consume the snapshot,
  never reopen or execute the mutable source path.
- Session and authority: a pin bundle may still outlive stop because digest
  matching does not retain session authority. A future executor must acquire
  the exact-generation revocation lease and consume exactly the authenticated
  snapshot during launch.
- Non-claims: this slice does not consume serialized arguments or environment,
  enter the retained working directory, launch, apply a sandbox, enforce an
  endpoint/descendant policy, or audit a tool outcome. `RQ-CF-AGENT-019`
  remains invariantly denied.
- Rollback: remove native-handle hashing, the pin/preflight distinction,
  authentication state and API, and focused regressions together. Retained
  object pinning must remain fail closed and the promotion gate must remain
  denied; no caller may fall back to path-based hashing as launch authority.

Potential Severity If Misused: high

## Verification

Current local evidence:

- warning-free GCC 15 Release workspace-agent/security passes `11/11`;
- the focused regression passes restored-metadata byte-change denial, private
  digest matching, no-authority ordinary preflight, the sparse over-cap case,
  POSIX rename and post-acquisition source-mutation isolation, post-stop authentication,
  move-only opacity, and invariant launch denial;
- fresh Clang 21 ASan/UBSan with leak detection passes `3/3` without findings;
- fresh GCC 15 ThreadSanitizer passes the process-target and session regressions
  `2/2` without findings;
- repository licensing, community, native-platform, GitHub Actions, isolation,
  supply-chain, and safety contracts pass `7/7`, including the corrected
  final-byte 326.93-second safety scan; and
- `git diff --check` passes.

Corrected exact signed/DCO head `9e9d6bf18` passed all eleven protected checks
in runs `31925258049`, `31925258995`, `31925258996`, `31925258994`, and
`31925258979`, including Windows x64 and Win32 ABI validation, Windows
environment/process-path coverage, and generated-launcher coverage on Windows,
Ubuntu, and macOS. Exact-head automated review found no major issue after its
earlier P1 mutable-object race finding drove the immutable-snapshot correction;
the sole review thread is answered, resolved, and outdated. PR `#5038` merged
into `v1-development` as `c9a64c0e9`.

The evidence-only update passes the licensing, community, isolation,
supply-chain, and safety contracts `5/5`, including the 344.06-second safety
scan.

Implementation evidence is complete. Requirement status stays `gap` only for
the current qualified high-severity documentation sign-off rule; no exact-
snapshot execution or launch readiness is claimed, and no qualified human
sign-off is claimed.
