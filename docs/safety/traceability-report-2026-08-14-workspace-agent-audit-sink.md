# Workspace-Agent Persistent Audit Sink Traceability Report

Date: 2026-08-14

## Scope and requirement

This slice implements the non-activating persistent lifecycle boundary in
`RQ-CF-AGENT-006`, derived from the repository owner's workspace-assistant
policy, `RQ-CF-AGENT-001`, and the audit-receipt contract in
`RQ-CF-AGENT-005`. It reuses the existing contained immutable audit writer; it
does not infer a requirement from preexisting implementation.

Mapped architecture and code:

- `include/copperfin/security/workspace_agent_audit_sink.h`
- `src/security/workspace_agent_audit_sink.cpp`
- `include/copperfin/security/audit_stream.h`
- `src/security/audit_stream.cpp`
- `tests/test_workspace_agent_audit_sink.cpp`
- `tests/test_security_controls.cpp`
- `docs/64-workspace-agent-access-policy.md`

## DQ/DV/HZ mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-workspace-agent-audit-001`: define exact lifecycle-event admission, content exclusions, committed-hash receipt binding, and fail-closed authority behavior | `DV-workspace-agent-audit-001`; `DV-workspace-agent-audit-002`; `DV-workspace-agent-audit-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-audit-002`: define canonical-root/relative-path containment, concurrent writer behavior, malformed-chain rejection, and a size bound checked before allocation or mutation | `DV-workspace-agent-audit-001`; `DV-workspace-agent-audit-002`; `DV-workspace-agent-audit-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-audit-003`: state that an unkeyed hash chain is not a signature or external ledger and retain root ownership, rotation, anchoring, recovery, and correlation as gaps | `DV-workspace-agent-audit-001`; `DV-workspace-agent-audit-003` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-audit-001`: local focused Release isolation,
  localization, security-controls, policy, session, and persistent-sink tests
  pass `6/6`.
- `DV-workspace-agent-audit-002`: fresh Clang 21 ASan/UBSan execution with leak
  detection passes `4/4` with no finding.
- `DV-workspace-agent-audit-003`: safety-traceability, community-health, diff,
  and maintainer self-review pass locally; protected exact-head checks and
  hosted review remain pending.

## Procedural delta map

- Before: session authority depended on a caller-supplied content-free audit
  receipt, but no product-native persistent sink bound that receipt to a
  contained durable record.
- After: an exact-allowlist adapter commits controller-produced lifecycle JSON
  through the common contained hash-chain writer and returns its entry hash,
  while invalid/full/corrupt/redirected state grants no authority.

Potential Severity If Misused: medium

## Hazard and misuse analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Sensitive-content injection: the sink accepts only exact schema/version,
  kind, mode, outcome, and diagnostic tuples. Substituted or content-bearing
  strings fail before persistence.
- Audit bypass or partial authority: a failed start append returns no receipt,
  so the controller creates no session authority. Stop has already revoked
  authority before its append.
- Path substitution: the host supplies an existing canonical storage root and
  safe relative path. Embedded-NUL roots and original log-path components fail
  before canonicalization or lexical normalization; existing redirecting
  components make the sink inert. The
  bounded resolver preserves the lexical relative path rather than erasing
  indirection through canonicalization, so the contained writer also rejects a
  post-construction intermediate or final redirection, hard links, cross-device
  replacement, and unsafe file types.
- Corrupt or racing writers: the bounded common-writer path locks across full
  existing-chain validation and append, recomputes every entry hash, and
  rejects malformed or well-shaped tampered state without mutation.
- Resource exhaustion: configuration is restricted to 512 bytes through
  64 MiB, defaults to 4 MiB, and checks existing plus prospective size under
  the writer lock. Existing-file size is rejected before its buffer allocation;
  an empty-log prospective line, including its `GENESIS` field, is checked with
  overflow-safe subtraction before path traversal can create directories, then
  exact prospective size is checked under the lock before any input-sized copy,
  concatenation, hash work, or append.
- Overstated integrity: the unkeyed chain detects ordinary mutation but does
  not authenticate the complete ledger. A storage-root attacker can delete or
  replace the entire chain.

## Boundary and rollback

This slice does not authenticate a provider, activate an assistant, execute a
tool, access a workspace, display consent, or add a CLI activation path. Root
ACL selection, rotation/retention, authenticated or external anchoring,
multi-host correlation, recovery UI, trusted activation UI, provider adapters,
executor, and real sandbox remain gaps.

Rollback is code-local: remove the sink header/source and focused test, remove
the bounded audit-writer entry point while retaining the original unbounded
APIs, and restore the mapped documentation. No provider or workspace content is
created by the test-owned fixtures.

## Verification

Local Release verification on Linux currently records:

- warning-free builds of the changed sink/session/policy targets;
- focused CTest selection: `6/6` pass;
- fresh Clang 21 ASan/UBSan policy, session, security-controls, and sink CTest:
  `4/4` pass with leak detection and no finding;
- `git diff --check`: pass.

The focused sink regression proves exact persistent bytes and receipt hashes,
content exclusions, malformed-event rejection without mutation, rejection of
the controller's zero and exhausted generation sentinels, traversal and
absolute-path and embedded-NUL rejection, missing-root and invalid-size rejection, full-log and
oversized-direct-input rejection without directory or file creation, malformed-tail
fail-closed behavior, and outside-root, in-root, and
post-construction intermediate-symlink containment when the platform supports
it. It also alters detail bytes while retaining a
well-shaped stale hash, proves standalone verification rejects the chain, and
proves bounded persistence returns no receipt and preserves the tampered bytes.
Existing security-control regressions supply direct
tail verification, concurrency, containment, symlink/reparse, and hard-link
coverage for the reused writer.

Exact-head protected and hosted-review results will replace the remaining
pending entries above before `RQ-CF-AGENT-006` advances from `gap`.

## Assurance statement

This is DO-178C-inspired development assurance adapted to a general-purpose
C++/.NET platform. It is not a claim of DO-178C compliance, certification, an
assigned software level, or suitability for a safety-critical deployment.

## Review evidence

- mode: maintainer self-review
- reviewer: rhamenator
- verification: fail-closed tuple admission, containment, size arithmetic,
  resource boundary, rollback, residual-integrity wording, and
  requirements/code/test mapping
- verification result: passed
- automated evidence: local focused Release `6/6`, fresh Clang ASan/UBSan
  `4/4`, safety-traceability workflow contract, repository community-health
  contract, and `git diff --check`; protected exact-head evidence pending
- automated evidence result: passed for local evidence
- scope: medium-severity persistent workspace-agent audit boundary
- result: approved as maintainer self-review; no independence claim
