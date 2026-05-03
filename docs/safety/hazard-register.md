# Hazard Register

Controlled hazard linkage register for safety-relevant documentation and behavior changes.

Purpose:

- provide stable `HZ-*` identifiers
- support investigation-grade traceability
- connect documentation procedures to hazard analysis

## Usage Rules

1. Use a unique immutable `HZ-*` id for each hazard.
2. Never recycle retired ids; mark them `retired` instead.
3. Link each safety-relevant documentation issue to one or more `HZ-*` ids.
4. If no hazard applies, record `HZ-none` with explicit rationale in the issue.
5. Update this register in the same change set where new hazard ids are introduced.

## Severity Scale

- `none`: no credible safety impact
- `low`: minor operational disruption, no expected injury
- `medium`: serious operational error possible, injury unlikely but credible
- `high`: severe operational error possible, serious injury credible
- `catastrophic`: loss-of-life or loss-of-platform hazard

## Hazard Entries

| Hazard ID | Title | Scope | Trigger / Misuse Condition | Potential Effect | Severity | Mitigation / Control | Verification Evidence | Owner | Status | Last Updated |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| HZ-data-corruption-01 | Data corruption through unsafe workflow or guidance | Data integrity and persistence | Operator or integrator follows incomplete or incorrect data-handling procedure | Silent record corruption, invalid downstream decisions, potential safety event | catastrophic | Enforce explicit backup/validation/recovery procedure steps in docs and reviews | DV walkthrough plus corruption-detection regression evidence | TBD | active | 2026-05-03 |
| HZ-system-failure-01 | System-level operational failure | Runtime and host operations | Procedure omits preconditions, recovery boundaries, or safe fallback actions | Service outage or unsafe degraded mode | high | Procedure precondition checks, fail-safe boundaries, and rollback/notification controls | DV evidence with independent review and simulation | TBD | active | 2026-05-03 |
| HZ-runtime-crash-01 | Runtime crash under operational use | Runtime execution and debugging | Misapplied runtime/debug command sequence or unsupported-state operation | Abrupt process termination and possible state loss | high | Crash-safe operational guidance, incident capture, and post-crash recovery procedure | DV evidence with crash-path simulation/walkthrough | TBD | active | 2026-05-03 |
| HZ-runtime-debug-01 | Incorrect fault recovery procedure | Runtime debug operations | Operator executes incorrect continue/retry sequence after runtime fault | Runtime resumes in unexpected state; downstream unsafe decisions possible | high | Require explicit pause-state inspection and validated recovery steps in docs | Linked DV evidence in issue and walkthrough artifact | TBD | active | 2026-05-03 |
| HZ-doc-command-01 | Ambiguous command guidance | Operator command documentation | Documentation omits critical preconditions or target context for commands | User executes destructive or incorrect command sequence | high | Procedural delta review, misuse analysis, and independent review before publish | Linked DV evidence in issue | TBD | active | 2026-05-03 |

## Investigation Checklist

When investigating an incident potentially related to documentation:

1. Identify all linked `HZ-*`, `DQ-*`, and `DV-*` ids.
2. Retrieve issue evidence ledger (commits, reviewers, artifacts, walkthrough results).
3. Compare deployed documentation revision against verified procedural delta map.
4. Confirm rollback and field-notification actions were triggered when needed.
5. Record findings back into the closing issue and changelog entry when applicable.

## Release Gate Use

Run `scripts/validate-safety-traceability.ps1` before release tagging (or run the `Safety Traceability Gate` workflow) to verify:

1. `DQ-*`, `DV-*`, and `HZ-*` presence for safety-relevant documentation issues
2. hazard ids resolve in this register
3. primary hazard coverage includes at least one of `HZ-data-corruption-01`, `HZ-system-failure-01`, or `HZ-runtime-crash-01`
4. investigation-ready evidence sections are present
