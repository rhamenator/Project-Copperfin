# Workspace-Agent Retained Process-Target Pins Traceability Report

Date: 2026-08-15

Scope: candidate v1 `RQ-CF-AGENT-023` non-executing retained authority for the
trusted workspace root, executable, and working-directory objects

Allowed requirement source: explicit repository-owner product policy for a
useful built-in assistant under H3/I2; derived from `RQ-CF-AGENT-010`,
`RQ-CF-AGENT-019`, `RQ-CF-AGENT-022`, `HZ-system-failure-01`, and
`HZ-data-corruption-01`

This report records DO-178C-inspired assurance adapted to a general-purpose
C++/.NET platform. It claims neither formal compliance nor certification,
assigned software level, launch readiness, nor suitability for a
safety-critical deployment.

## DQ/DV/HZ mapping

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-target-pins-001`: only an allowed inspection issued by the exact non-copyable process-target boundary may authorize one pin attempt; forged, edited, replayed, stale, or cross-boundary inspection state shall fail closed | `DV-workspace-agent-target-pins-001`; `DV-workspace-agent-target-pins-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-target-pins-002`: pin acquisition shall open and retain the original trusted workspace root, executable, and working-directory objects and require their platform identities to match the private inspection record | `DV-workspace-agent-target-pins-001`; `DV-workspace-agent-target-pins-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-target-pins-003`: the returned bundle shall be move-only, non-inheriting, and opaque, exposing neither a path nor native handle, plan, argument, environment, or launch operation | `DV-workspace-agent-target-pins-001`; `DV-workspace-agent-target-pins-002` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-target-pins-004`: controller acquisition shall require exact active process-tool admission before acquisition and recheck it afterward; pins shall not substitute for the revocation lease or weaken the invariant `RQ-CF-AGENT-019` denial | `DV-workspace-agent-target-pins-001`; `DV-workspace-agent-target-pins-002`; `DV-workspace-agent-target-pins-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-target-pins-001`: focused boundary/controller regression
  proves exact active admission, provenance, tamper/replay/cross-boundary
  denial, identity-change denial, move-only opacity, object retention, and the
  unchanged launch gate.
- `DV-workspace-agent-target-pins-002`: source and compile-time contracts prove
  that no native handle, path, or process operation is public.
- `DV-workspace-agent-target-pins-003`: warning-free Release, sanitizer,
  broader safety/isolation, protected Windows/Ubuntu/macOS, diff, and exact-head
  review evidence are required before implementation evidence is complete.

## Hazard, misuse, boundary, and rollback analysis

- Provenance and confused deputy: every successful inspection receives a
  private exact-value record bound to one logical non-copyable boundary. One
  same-boundary attempt consumes it. Public field construction or editing,
  copying for replay, and presentation to another boundary do not authorize
  different targets.
- Acquisition race: each object is opened first and its handle/descriptor
  identity is compared with the private inspection record. A path replacement
  cannot silently redirect the bundle to a different root, executable, or
  working directory.
- Windows behavior: handles are non-inheriting, open reparse points rather than
  following them, and omit write/delete sharing. This blocks ordinary later
  replacement while retained. It is not a proof against every pre-existing
  writable mapping or a substitute for pinned-byte authentication.
- POSIX behavior: descriptors use `O_CLOEXEC` and `O_NOFOLLOW`; executable
  acquisition is nonblocking and directory acquisitions require directories.
  Rename or unlink can change names while the descriptors continue to retain
  the original objects. A future executor must authenticate and execute the
  pinned executable object and enter the pinned working directory through
  those same descriptors or an equally race-free platform mechanism.
- Session race: the controller checks exact process-tool admission before
  opening and again afterward. Pins can outlive a later stop because they are
  resource retention only, not session or process authority. A future launch
  path must separately hold `RQ-CF-AGENT-022` through the direct launch.
- Denial of service: a trusted host can retain file descriptors or handles too
  long or request many fresh inspections. The boundary is not exposed to
  provider/model input and future executor code must bound each bundle to one
  launch attempt.
- Allocation failure: each newly opened native object remains under local RAII
  ownership until allocation and construction of the opaque bundle succeeds;
  every failure path therefore closes all partially acquired resources.
- Non-claims: this slice does not authenticate bytes through the retained
  executable, consume a serialized plan, acquire a revocation lease, apply a
  sandbox or endpoint/descendant policy, launch, or audit an outcome.
  `RQ-CF-AGENT-019` therefore remains invariantly denied.
- Rollback: remove the pin API, controller acquisition, and focused regressions
  together. The promotion gate must remain denied and no executor may fall back
  to point-in-time identities.

Potential Severity If Misused: high

## Verification

Current local evidence:

- warning-free GCC 15 Release workspace-agent/security execution passed
  `11/11`, including the focused pin regression after the one-attempt
  consumption correction;
- fresh Clang 21 ASan/UBSan with leak detection passed `3/3`;
- fresh GCC 15 ThreadSanitizer passed the session, process-target containment,
  and isolated-environment selection `3/3`;
- after pre-publication review corrected allocation-failure resource leakage,
  the corrected Release, ASan/UBSan/leak, and ThreadSanitizer focused sets each
  passed `3/3`;
- repository licensing, community, native-platform, GitHub Actions, isolation,
  supply-chain, and safety contracts passed `7/7`, including repeated
  329.84-, 325.82-, and post-correction 326.48-second safety scans;
- `git diff --check` passed; and
- protected Windows/Ubuntu/macOS, exact-head review, and qualified
  high-severity sign-off remain pending.

Requirement status remains `gap`; no execution or launch readiness is claimed.
