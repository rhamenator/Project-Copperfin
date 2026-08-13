# Workspace-Agent Access Policy Traceability Report

## Scope And Procedural Delta

- Boundary: activation of a built-in local coding agent with mutable machine
  capabilities.
- Previous state: Copperfin had one bounded read-only MCP tool and no mutable
  workspace-agent access-mode or warning contract.
- New state: `advisory`, `workspace_sandbox`, and `unrestricted_local` are
  explicit machine modes. Unrestricted activation requires feature opt-in,
  native permission, trusted product UI, available audit, the current warning
  identity, and affirmative consent.
- Preserved state: no model provider, credential, extension token, or runtime
  payload can itself grant local authority; no mode permits privilege
  elevation.

## DQ/DV/HZ Mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-V1-workspace-agent-explicit-authority` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-misuse-walkthrough`; `DV-V1-workspace-agent-independent-review` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-V1-workspace-agent-unrestricted-warning` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-localization`; `DV-V1-workspace-agent-independent-review` | `HZ-doc-command-01`; `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-V1-workspace-agent-provider-separation` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-misuse-walkthrough` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

## Verification Evidence

### DV-V1-workspace-agent-policy-regression

`test_workspace_agent_policy` verifies all three capability sets. It requires
exact round-tripping of the three machine mode names and rejects aliases, case
changes, surrounding whitespace, and unknown values. It requires
the exact versioned warning and affirmative consent for unrestricted access,
rejects disabled features, missing native permission, untrusted callers,
missing audit, absent warnings, stale warnings, and canceled warnings, and
rejects an out-of-range serialized mode even when all unrestricted consent
fields are present. It also proves that unrestricted mode still denies
privilege elevation and that a feature-disabled denial still requires a
content-free audit event. The focused
policy, platform-model, complete native-isolation, and broad localization
selection passes locally `4/4`.
The focused policy test passes under Clang ASan/UBSan with leak detection and
no findings.
Temporarily weakening the warning-identity comparison to admit a stale
nonempty warning makes the policy test fail at the intended stale-warning
assertion; restoring the comparison returns the test to green.
Temporarily clearing the denial audit requirement makes the same test fail at
the feature-disabled audit assertion; restoration returns it to green.

### DV-V1-workspace-agent-localization

All four installed catalogs contain the warning and decision keys. The direct
test checks invariant warning identity, reviewed en-US text, es-419 selection,
and qps-ploc coverage. JSON parsing succeeds for every catalog.

### DV-V1-workspace-agent-misuse-walkthrough

The deterministic test walkthrough covers these credible mistakes:

- treating provider authentication as local authority: missing native
  permission is denied;
- calling the policy outside trusted product UI: denied;
- showing an older or substituted warning: denied;
- showing the warning without affirmative consent: denied;
- activating without an audit sink: denied;
- supplying an unknown serialized mode with otherwise valid consent: denied;
- interpreting unrestricted access as elevation: the elevation capability is
  always false.

### DV-V1-workspace-agent-independent-review

Independent read-only review is required before merge because the warning is a
high-impact security control. The review must assess mode capabilities, RBAC
defaults, warning completeness, fail-closed ordering, localization, and the
accuracy of the explicit remaining-work statement. Exact review and hosted
evidence will be recorded here before closure.

## Severity, Rollback, And Notification

The credible failure is unintended file deletion, secret disclosure, process
execution, or network activity under the user's identity. Misuse severity is
high; uncontrolled data corruption can intersect `HZ-data-corruption-01`.
Current policy controls are disabled-by-default feature state, nondefault
high-risk RBAC, trusted-UI origin, versioned warning consent, mandatory audit
availability, and no elevation. Required integration controls still to ship
are session scope, visible mode indication, and immediate stop/revocation.

Before the host/executor ships, rollback is a normal revert of the policy
slice and continued use of the read-only MCP baseline. After integration, the
field rollback is to disable workspace-agent activation centrally and revoke
active sessions; any incorrect warning or capability mapping requires a
corrected release and user notification. No current field notification is
required because v1 has not shipped and this slice does not yet expose a user
activation surface.
