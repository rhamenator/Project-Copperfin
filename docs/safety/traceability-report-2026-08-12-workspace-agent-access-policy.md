# Workspace-Agent Access Policy Traceability Report

## Scope And Procedural Delta

- Governing requirements: `RQ-CF-AGENT-001`, `RQ-CF-AGENT-002`, and
  `RQ-CF-AGENT-003` in
  `docs/32-recovered-requirements-traceability.md`.

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

| Product / documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-V1-workspace-agent-explicit-authority` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-host-descriptor`; `DV-V1-workspace-agent-managed-client`; `DV-V1-workspace-agent-misuse-walkthrough`; `DV-V1-workspace-agent-independent-review` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-V1-workspace-agent-unrestricted-warning` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-localization`; `DV-V1-workspace-agent-managed-client`; `DV-V1-workspace-agent-independent-review` | `HZ-doc-command-01`; `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-V1-workspace-agent-provider-separation` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-host-descriptor`; `DV-V1-workspace-agent-managed-client`; `DV-V1-workspace-agent-misuse-walkthrough` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `RQ-CF-AGENT-001` | `DV-V1-workspace-agent-policy-regression`; protected exact-head matrix; reverse links in the public policy header and focused test | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01` |
| `RQ-CF-AGENT-002` | `DV-V1-workspace-agent-host-descriptor`; reverse links in the descriptor implementation and process test; protected exact-head matrix | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `RQ-CF-AGENT-003` | `DV-V1-workspace-agent-managed-client`; reverse links in the strict managed client and smoke; exact-head hosted matrix pending | `HZ-system-failure-01`; `HZ-data-corruption-01` |

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

### DV-V1-workspace-agent-host-descriptor

`test_studio_host_workspace_agent_policy` executes the real Studio host and
requires schema version 1, advisory/default-disabled state, all three exact mode
names, explicit outside-workspace and network risk, permanent denial of
elevation, the native permission, trusted-UI/audit gates, provider separation,
descriptor-only/no-activation state, and the current warning identity. The endpoint derives capabilities by calling
the policy evaluator. The regression also requires fail-closed status 2 and no
partial stdout for mixed descriptor/operational and descriptor/license-status
requests, duplicate or reordered descriptor switches, and a generic
command-line unrestricted-activation attempt. This is a read-only contract and
supplies no activation path. A qps-ploc process case requires
localized warning prose while the warning id and mode names remain invariant.
The focused Release policy/host/isolation/safety selection passes `4/4`. The
policy and real host-process tests pass under Clang ASan/UBSan with leak
detection `2/2` and no findings; direct `jq` evaluation accepts the emitted
schema and required invariant fields. A separate Release configuration with
the archived product-licensing build flag enabled passes the real host process
and licensing-policy source contract `2/2`; the shipping default remains off.

### DV-V1-workspace-agent-managed-client

`SmokeManagedWorkspaceAgentPolicyContract` verifies that the shared managed
client invokes only `--workspace-agent-policy --json`, accepts localized prose
under the invariant schema, and fails closed for unsupported versions, omitted
false-valued security fields, wrong types, unknown fields, non-descriptor or
activation-capable envelopes, provider-auth authority, substituted permission
or warning identifiers, duplicate/aliased modes, capability expansion,
privilege elevation, and malformed JSON. Both the complete Designer smoke
assembly and standalone Studio shell compile warning-free against the net472
contract on Linux. Direct managed execution and the VSIX package build remain
scheduled for their Mono and Windows hosted matrices; they are not claimed by
the local compilation evidence.

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

The GitHub independent automated reviewer first found a real observability gap:
feature-disabled denials did not require an audit outcome. Corrected exact head
`4c4014f94` makes every decision require auditing, adds a direct regression,
and resolves that thread. A fresh review explicitly targeting that corrected
head found no further major issue. A separate coordination-channel review was
also requested, but no response was made an acknowledgment or merge deadlock.
The direct capability, RBAC, warning, fail-closed, localization, mutation, and
remaining-work evidence above therefore remains load-bearing rather than being
replaced by the automated verdict.

Review of the descriptor follow-up found that the optional product-license
status handler preceded policy validation and, when that archived build flag
was enabled, could consume a mixed policy request. Policy-bearing arguments are
now validated first. The process test covers the mixed request, the
licensing-policy source contract protects dispatch order, and a fresh
licensing-enabled build passes both contracts `2/2` without changing the
default-disabled product-licensing policy.

### Protected exact-head matrix

Corrected signed/DCO head `4c4014f94` passes all eleven protected checks:

- Generated Launcher Validation `31660946596` passes on Windows, Ubuntu, and
  macOS;
- Windows DECLARE ABI Validation `31660946655` passes Win32 and x64;
- Windows environment/path validation `31660946599` passes;
- GCC/Clang executable-path validation `31660946592` passes;
- DCO run `31660945231` and both Socket checks pass.

The generated-launcher jobs include the portable policy regression through the
native suite on all three hosts. This slice does not claim that those unrelated
workflow names constitute execution of the still-unimplemented agent UI or
executor.

The descriptor follow-up's corrected signed/DCO implementation head
`def609305` also passes all eleven protected checks:

- Generated Launcher Validation `31666059568` passes on Windows, Ubuntu, and
  macOS. The first Windows attempt failed before Copperfin configuration when
  the external R-version resolver could not resolve R 4.6.1; rerunning only
  that failed job passed setup, configuration, build, and tests without a
  product change;
- Windows DECLARE ABI Validation `31666059579` passes Win32 and x64;
- Windows environment/path validation `31666059576` passes;
- GCC/Clang executable-path validation `31666059522` passes;
- DCO run `31666057944` and both Socket checks pass.

These runs close the descriptor requirement and do not close the separately
identified mutable activation, provider, session, audit-commit, executor,
sandbox, or trusted-UI implementation gaps.

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
