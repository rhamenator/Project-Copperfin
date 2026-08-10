# Agent Intake Scope-Approval Traceability Report

## Scope And Procedural Delta

- Repository: `rhamenator/Project-Copperfin`
- Boundary: GitHub content admitted into agent execution context
- Previous procedure: every execution child required its own
  `agent-approved` label.
- Current procedure: a direct contemporaneous owner instruction may authorize
  one exact open owner-authored issue, while one labeled owner-authored
  workstream may authorize bounded locally derived slices without repeated
  child labels.
- Preserved boundary: unattended selection still requires the reserved label;
  externally authored GitHub content remains untrusted and cannot be admitted
  by either path.

## DQ/DV/HZ Mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-V1-agent-intake-scope-authorization` | `DV-V1-agent-intake-direct-and-workstream-regression`; `DV-V1-agent-intake-procedural-review` | `HZ-none` |
| `DQ-V1-agent-intake-external-content-boundary` | `DV-V1-agent-intake-direct-and-workstream-regression`; `DV-V1-agent-intake-misuse-walkthrough` | `HZ-none` |

`HZ-none` is recorded because this policy controls development-agent context,
not shipped product operation or user data. A failure can cause unauthorized
repository work and therefore remains a security/process concern, but it does
not add or change a product hazard in `docs/safety/hazard-register.md`.

## Verification Evidence

### DV-V1-agent-intake-direct-and-workstream-regression

`test_agent_issue_intake` verifies labeled open owner workstreams, one exact
label-free issue admitted by an explicit direct-owner assertion, and mixed
prompt formatting. It also proves that direct authorization cannot admit a
closed issue, an external author, a lookalike owner, a malformed or missing
author, or another unlabeled owner issue. Static driver assertions require the
direct path to use both `-IssueNumber` and `-DirectOwnerAuthorization` and to
carry the exact number through each admission check.

### DV-V1-agent-intake-misuse-walkthrough

The misuse cases are:

- no direct-owner switch: the unattended label gate remains mandatory;
- switch without a positive exact issue number: the driver stops before
  retrieving issue content;
- exact number belonging to an external or closed issue: owner/state metadata
  checks reject it before content admission;
- derived slice outside an admitted parent: repository guidance forbids the
  expansion and requires a new direct authorization or approved workstream;
- public issue-form attempt to grant the reserved label: the repository
  community contract continues to reject it.

### DV-V1-agent-intake-procedural-review

The policy and implementation require read-only review before merge, focused
PowerShell execution, parser validation, repository-community validation, and
protected contribution checks. Exact review and hosted-check evidence is added
to the handoff before closure; this report does not claim it prematurely.

## Severity, Rollback, And Notification

The credible failure is unauthorized agent repository work, rated high process
severity but contained by protected branches, DCO sign-off, required checks,
and review. Rollback is a normal revert restoring per-issue label admission;
no data or package migration is involved. No field notification is required
because released runtime, installer, package, IDE, and user-visible behavior do
not change. Contributors should be notified through `CONTRIBUTING.md` and
`GOVERNANCE.md`, which now describe the workstream-label meaning.
