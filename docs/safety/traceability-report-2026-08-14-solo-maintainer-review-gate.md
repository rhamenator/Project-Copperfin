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
  changes require a second qualified human reviewer before closure.
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
- `DV-assurance-review-005`: parse a high-severity issue-form value with a
  same-line rationale and reject its legacy review section when that section
  says independent approval remains open.
- `DV-assurance-review-006`: accept the level-three Markdown headings emitted by
  a GitHub issue form as well as historical level-two headings.
- `DV-assurance-review-007`: reject a high-severity Review Evidence section whose
  reviewer is blank or an unfilled placeholder even when its mode and result
  claim independent approval.
- `DV-assurance-baseline-001`: inspect the charter, agent rules, README,
  assurance policy, ontology, and recovered-requirements matrix for one
  consistent permitted-source and ongoing-traceability boundary.

## Hazard Linkage IDs

- `HZ-system-failure-01`
- `HZ-doc-command-01`

## DQ/DV/HZ Mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-assurance-review-001` | `DV-assurance-review-001`; `DV-assurance-review-003`; `DV-assurance-review-006` | `HZ-system-failure-01`; `HZ-doc-command-01` |
| `DQ-assurance-review-002` | `DV-assurance-review-002`; `DV-assurance-review-003`; `DV-assurance-review-004`; `DV-assurance-review-005`; `DV-assurance-review-007` | `HZ-system-failure-01`; `HZ-doc-command-01` |
| `DQ-assurance-baseline-001` | `DV-assurance-baseline-001` | `HZ-system-failure-01`; `HZ-doc-command-01` |

## Procedural Delta Map

- Before: `agents.md`, the issue form, the validator, and the triage rubric
  required Independent Review for every safety-documentation change, even
  though the controlling solo-maintainer policy permitted documented self-review
  for ordinary development.
- After: the form requests Review Evidence and records its mode. The validator
  admits explicit self-review for `none`/`low`/`medium`, rejects it for
  `high`/`catastrophic`, and continues to admit historical Independent Review
  records. Stable-release review requirements are unchanged.
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
the high/catastrophic gate.

Potential Severity If Misused: medium

## Review Evidence

- mode: maintainer self-review
- scope: policy consistency, severity parsing, legacy compatibility, fail-closed
  high/catastrophic behavior, and ongoing requirements-recovery wording
- result: no independent-review claim; focused automated evidence required

## Simulation/Walkthrough Evidence

The focused contract executes a permitted low-severity self-review fixture, a
forbidden high-severity self-review fixture, and an approved high-severity
independent-human-review fixture. It also reruns legacy independent-review
fixtures, malformed mapping/hazard cases, classifier behavior, and hostile issue-
number probes through the production PowerShell validator.

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
