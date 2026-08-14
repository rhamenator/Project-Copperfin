# Solo-Maintainer Safety Review Gate Traceability

## Scope and policy source

This slice reconciles active safety-documentation intake and validation with the
repository owner's adopted solo-maintainer assurance policy. The owner explicitly
directed that unavailable routine independent review must not impose avoidable
development delay. The existing policy continues to prohibit calling self-review
or automation independent and retains qualified human review for `high` and
`catastrophic` safety-documentation changes and for completed-project or first-
stable-release readiness.

This is DO-178C-inspired development assurance for a general-purpose platform.
It is not a claim of DO-178C compliance, certification, an assigned software
level, or safety for a particular deployment.

## Documentation Requirement IDs

- `DQ-assurance-review-001`: `none`, `low`, and `medium` safety-documentation
  changes accept explicit maintainer self-review plus applicable automated
  verification during development, without an independence claim.
- `DQ-assurance-review-002`: `high` and `catastrophic` safety-documentation
  changes require a second qualified human reviewer before closure. The
  reviewer-controlled sign-off must carry meaningful qualification and
  verification evidence and bind to the SHA-256 of the exact issue body
  reviewed; a later body edit requires renewed sign-off, and the latest
  applicable sign-off for that reviewer and digest supersedes earlier approval.
- `DQ-assurance-review-003`: machine-evaluated review closure must use unique,
  explicit structured outcome fields and exactly one rendered independent
  sign-off section per sign-off comment. Headings and fields inside fenced or
  indented code or HTML comments are not evidence, and a sign-off comment that
  contains a top-level raw-HTML construct fails closed. HTML comments outside
  fenced, indented, and inline code are masked while retaining source-line and
  column boundaries. ATX headings require whitespace after their marker
  and may use a whitespace-separated closing hash sequence. Free-form
  verification scope identifies what was checked but cannot substitute for a
  `qualified` qualification result or a `passed` verification or automation
  result.
- `DQ-assurance-review-004`: severity must come from exactly one rendered
  severity section, or from exactly one legacy severity field when no rendered
  section exists. Hidden, duplicated, or mixed severity declarations fail
  closed and cannot select a weaker review lane.
- `DQ-assurance-review-005`: structured issue-body evidence sections and
  traceability mappings must be parsed from rendered Markdown and occur exactly
  once when present. Historical rendered marker text remains valid only when
  the corresponding structured heading is absent. Current Review Evidence and
  legacy Independent Review Evidence schemas are mutually exclusive. Raw HTML
  blocks are excluded without blanking unrelated rendered evidence; Markdown
  autolinks remain ordinary rendered evidence. Schema presence is counted
  independently from successful section extraction, so duplicate current
  headings cannot fall through to a legacy approval. The raw body is retained
  only for the reviewer-bound digest.
- `DQ-assurance-baseline-001`: requirements recovery and bidirectional
  traceability are an ongoing project-wide baseline; existing implementation
  is never its own requirement source.

## Documentation Verification IDs

- `DV-assurance-review-001`: validate a closed low-severity fixture containing
  Review Evidence for maintainer self-review and focused automation.
- `DV-assurance-review-002`: reject a closed high-severity fixture that contains
  only maintainer self-review, and require approved Independent Human Review
  evidence.
- `DV-assurance-review-003`: retain acceptance of historical Independent Review
  evidence and all existing mapping, hazard, classifier, and intake-boundary
  regressions.
- `DV-assurance-review-004`: accept a closed high-severity fixture only when its
  current Review Evidence explicitly records independent-human mode and an
  approved result.
- `DV-assurance-review-005`: reject fenced/commented severity declarations in
  either risk direction, duplicate rendered severity sections, multiple values
  within one section (including multiple values on one line), hidden values
  inside a rendered section, raw/rendered value disagreements across hidden
  heading boundaries in either ordering, and mixed section/legacy declarations.
- `DV-assurance-review-006`: reject duplicate rendered Review Evidence and
  DQ/DV/HZ Mapping sections and mixed current/legacy review schemas; accept
  unrelated raw-HTML detail blocks and Markdown autolinks while preserving the
  exact raw-body digest binding. Reject duplicate current review headings even
  when a structurally valid legacy approval is also present.
- `DV-assurance-review-005`: parse a high-severity issue-form value with a
  same-line rationale and reject its legacy review section when that section
  says independent approval remains open.
- `DV-assurance-review-006`: accept the level-three Markdown headings emitted by
  a GitHub issue form as well as historical level-two headings.
- `DV-assurance-review-007`: reject a high-severity Review Evidence section whose
  reviewer is blank or an unfilled placeholder even when its mode and result
  claim independent approval.
- `DV-assurance-review-008`: reject an independent-review claim whose reviewer
  login equals the issue author's login.
- `DV-assurance-review-009`: reject low-severity self-review that lacks an
  affirmative result, verification scope, or automated-evidence field.
- `DV-assurance-review-010`: parse legacy approval from an affirmative
  `result`/`status` field so `not approved` and `not verified` cannot pass by
  substring.
- `DV-assurance-review-011`: reject an otherwise valid independent-review claim
  unless the named distinct reviewer personally posts a structured sign-off
  from a GitHub `User` account containing their login, qualification basis,
  verification, and affirmative result.
- `DV-assurance-review-012`: reject missing, non-passing, duplicated, or
  result/status-aliased qualification, verification, and automation outcomes;
  accept only the required unique structured `qualified`/`passed` fields.
- `DV-assurance-review-013`: reject failed, cancelled, timed-out, or negated
  evidence without relying on a producer-name whitelist or a bounded number of
  intervening words, while retaining affirmative failure-boundary guarantees.
- `DV-assurance-review-014`: order applicable reviewer sign-offs by their latest
  GitHub `updated_at` value, so editing an older comment to withdraw approval
  supersedes a later-created but earlier-updated approval.
- `DV-assurance-review-015`: reject a latest applicable sign-off that contains
  disallowed top-level angle syntax instead of falling back to an older
  approval; validate email autolinks against the CommonMark domain grammar and
  preserve severity boundaries across fenced and indented code examples.
- `DV-assurance-baseline-001`: inspect the charter, agent rules, README,
  assurance policy, ontology, and recovered-requirements matrix for one
  consistent permitted-source and ongoing-traceability boundary.

## Hazard Linkage IDs

- `HZ-system-failure-01`
- `HZ-doc-command-01`

## DQ/DV/HZ Mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-assurance-review-001` | `DV-assurance-review-001`; `DV-assurance-review-003`; `DV-assurance-review-006`; `DV-assurance-review-009` | `HZ-system-failure-01`; `HZ-doc-command-01` |
| `DQ-assurance-review-002` | `DV-assurance-review-002`; `DV-assurance-review-003`; `DV-assurance-review-004`; `DV-assurance-review-005`; `DV-assurance-review-007`; `DV-assurance-review-008`; `DV-assurance-review-010`; `DV-assurance-review-011`; `DV-assurance-review-014`; `DV-assurance-review-015` | `HZ-system-failure-01`; `HZ-doc-command-01` |
| `DQ-assurance-review-003` | `DV-assurance-review-012`; `DV-assurance-review-013` | `HZ-system-failure-01`; `HZ-doc-command-01` |
| `DQ-assurance-baseline-001` | `DV-assurance-baseline-001` | `HZ-system-failure-01`; `HZ-doc-command-01` |

## Procedural Delta Map

- Before: `agents.md`, the issue form, the validator, and the triage rubric
  required Independent Review for every safety-documentation change, even
  though the controlling solo-maintainer policy permitted documented self-review
  for ordinary development.
- After: the form requests Review Evidence and records its mode. The validator
  admits explicit self-review for `none`/`low`/`medium`, rejects it for
  `high`/`catastrophic`, and admits current or historical Independent Review
  records only when a structured sign-off comment is authored by the named
  reviewer account, rejects placeholder or negated qualification/verification,
  requires unique structured passing verification/automation outcomes rather
  than inferring success from producer-specific prose, and binds
  the comment to the exact current issue-body SHA-256. Stable-release review
  requirements are unchanged.
- Requirements recovery: charter and operating guidance now state that the
  traceability baseline applies to every behavior-changing slice rather than a
  later optional pass.

## Misuse Analysis

If the relaxed path ignored severity, a high-impact procedural error could close
without qualified human review. The validator's explicit high/catastrophic
negative gate prevents that. If self-review were described as independent,
downstream users could overestimate assurance; all controlling prose explicitly
forbids that claim. If severity were detected by an unrelated word elsewhere in
an issue, the wrong gate could apply; severity is now parsed from the issue-form
section (including a same-line rationale after the selected value) or its legacy
`Severity: value` representation. A legacy Independent Review section that says
approval is pending, unavailable, incomplete, or still required cannot satisfy
the high/catastrophic gate. A distinct login typed by the issue author is not
independent evidence; the validator fetches issue comments and requires the
named reviewer to author the approved sign-off and state a qualification basis.

Potential Severity If Misused: medium

## Review Evidence

- mode: maintainer self-review
- reviewer: rhamenator
- verification: policy consistency, severity parsing, legacy compatibility,
  fail-closed high/catastrophic behavior, and ongoing requirements-recovery
  wording
- verification result: passed
- automated evidence: focused safety-traceability and community-health
  contracts, YAML/JSON parsing, and diff validation
- automated evidence result: passed
- scope: policy consistency, severity parsing, legacy compatibility, fail-closed
  high/catastrophic behavior, and ongoing requirements-recovery wording
- result: approved as maintainer self-review; no independence claim

## Simulation/Walkthrough Evidence

The focused contract executes a permitted low-severity self-review fixture, a
forbidden high-severity self-review fixture, and an approved high-severity
independent-human-review fixture. It also reruns legacy independent-review
fixtures; rejects incomplete low-risk evidence, author-as-independent-reviewer,
unattested login claims, placeholder reviewers, and negated legacy results;
rejects stale issue-body digests, placeholder or negated independent-review
qualification/verification, and placeholder lower-risk self-review verification
or automation. The shared evidence predicate normalizes punctuation and rejects
negative, incomplete, missing, deferred, unavailable, skipped, and failed-state
tokens; dedicated mutations cover `not-qualified`, `unavailable at this time`,
and failed automation. Failed workflow evidence and status/outcome `failure`
forms are rejected while legitimate `failure boundaries` scope prose remains
valid. Affirmative safety guarantees such as `does not mutate user data` and
`never mutates user data` also remain valid. The live GitHub loader re-fetches the issue
after comment pagination and accepts only a stable body, comment-count, and
update snapshot;
accepts reviewer-authored current and historical sign-offs bound to their exact
fixture bodies; and reruns malformed mapping/hazard cases, classifier behavior,
and hostile issue-number probes through the production PowerShell validator.

## Rollback And Field Notification Plan

If the severity or review-mode gate misclassifies evidence, revert this bounded
slice, restore the prior validator and issue-form wording, mark affected evidence
as unverified, and notify maintainers through the repository changelog and the
affected safety issue or release-readiness record. No runtime, package, artifact,
or user data is mutated by this policy slice.

## Evidence

Focused results are recorded with the final signed commit and protected workflow
results after review. The durable requirement-to-code-to-test row is
`LLR-CF-ASSURANCE-001` in
`docs/32-recovered-requirements-traceability.md`.

A live API probe against issue `#4403` reached a stable post-comment snapshot
through the production loader and then failed closed at its expected legacy
review-evidence boundary. It did not fail in pagination, refresh, or snapshot
stabilization.
