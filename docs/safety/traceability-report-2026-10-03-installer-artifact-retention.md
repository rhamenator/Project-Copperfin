# Installer Artifact Retention Traceability Report

## Scope And Assurance Boundary

This report covers `RQ-CF-REL-009` and the repository-owner-directed artifact
policy in issue #6905. It governs GitHub Actions retention and diagnostic
availability; it does not make an ordinary workflow artifact a release,
extend GitHub's service-level retention guarantees, or replace exact-candidate
digest verification.

## Requirement And Verification Map

| Requirement | Verification | Controlled hazards |
| --- | --- | --- |
| `RQ-CF-REL-009`; `DQ-installer-artifact-retention-policy` — retain short-lived reproducibility packages and diagnostics without presenting pull-request outputs as release candidates | `DV-installer-artifact-retention-policy-contract`; workflow YAML parse; exact-head hosted Windows, macOS, and Linux installer uploads | `HZ-system-failure-01`; `HZ-doc-command-01` |

Reverse traceability is carried by `.github/workflows/build-installers.yml`,
`docs/35-rc1-evaluation-guide.md`, the durable requirements matrix, and
`tests/run_installer_artifact_policy_contract_check.cmake`.

## Procedural Delta Map

| Situation | Previous procedure | New procedure |
| --- | --- | --- |
| Successful pull request | Repository-default retention; lifecycle logs bundled with packages | Exact packages, lifecycle JSON, and source retained 7 days; routine success logs omitted |
| Successful non-pull-request run | Repository-default retention | Exact packages, lifecycle JSON, and source retained 30 days |
| Failed platform job | Windows retained UI diagnostics; macOS/Linux attempted the success bundle and could add a missing-file failure | Every platform retains any produced package plus available diagnostics for 14 days; missing early-build outputs warn without masking the original failure |
| Immutable RC evaluation | Separate bundle retained 90 days | Unchanged; ordinary installer artifacts remain non-RC evidence |

## Hazard, Misuse, And Boundary Analysis

Misuse severity is **low**. Expired ordinary artifacts may delay reproduction,
and an operator might otherwise mistake a convenient pull-request package for
an immutable candidate. The evaluation guide now states the retention tiers
and the non-RC boundary. Failure artifacts are explicitly diagnostic: their
upload cannot change job or lifecycle status, and `if-no-files-found: warn`
prevents an absent package from hiding the causal build failure. When a
lifecycle fails after packaging, retaining that exact package supports local
or VM reproduction without rebuilding a potentially different revision.

## Verification And Review Evidence

Maintainer self-review verifies the three success-only uploads, three
failure-only uploads, 7/30-day successful tiers, 14-day diagnostics, exact
platform package patterns, and fail-open diagnostic versus fail-closed success
upload behavior. The focused contract enforces those properties, YAML parsing
checks syntax, and exact-head hosted platform jobs verify the action accepts
and applies the policy. This is maintainer self-review plus automated evidence,
not independent verification.

## Rollback And Field Notification

Rollback is one configuration-and-documentation revert restoring the previous
artifact upload steps. Before an RC, no field notification is needed beyond the
PR/changelog record. If a published evaluation guide or candidate references
an unavailable artifact, preserve the tag, disclose the affected workflow run
and digest, regenerate only through the next sequential immutable candidate,
and correct the retention guidance rather than rewriting prior evidence.
