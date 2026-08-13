# Workspace-Agent Access Policy Traceability Report

## Scope And Procedural Delta

- Governing requirements: `RQ-CF-AGENT-001`, `RQ-CF-AGENT-002`,
  `RQ-CF-AGENT-003`, and `RQ-CF-AGENT-004` in
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
| `DQ-V1-workspace-agent-explicit-authority` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-host-descriptor`; `DV-V1-workspace-agent-managed-client`; `DV-V1-workspace-agent-policy-preview`; `DV-V1-workspace-agent-misuse-walkthrough`; `DV-V1-workspace-agent-independent-review` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-V1-workspace-agent-unrestricted-warning` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-localization`; `DV-V1-workspace-agent-managed-client`; `DV-V1-workspace-agent-policy-preview`; `DV-V1-workspace-agent-independent-review` | `HZ-doc-command-01`; `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-V1-workspace-agent-provider-separation` | `DV-V1-workspace-agent-policy-regression`; `DV-V1-workspace-agent-host-descriptor`; `DV-V1-workspace-agent-managed-client`; `DV-V1-workspace-agent-misuse-walkthrough` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `RQ-CF-AGENT-001` | `DV-V1-workspace-agent-policy-regression`; protected exact-head matrix; reverse links in the public policy header and focused test | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01` |
| `RQ-CF-AGENT-002` | `DV-V1-workspace-agent-host-descriptor`; reverse links in the descriptor implementation and process test; protected exact-head matrix | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `RQ-CF-AGENT-003` | `DV-V1-workspace-agent-managed-client`; reverse links in the strict managed client and smoke; exact-head Linux execution `31677215316`; exact-head Windows execution `31677215577` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `RQ-CF-AGENT-004` | `DV-V1-workspace-agent-policy-preview`; reverse links in standalone Studio, the dialog, managed localization, and focused smoke; local managed compile passes while exact-head Linux/Windows execution remains required | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01` |

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
false-valued security fields, wrong types, unknown fields, duplicate members
(including escaped-equivalent names), non-descriptor or activation-capable
envelopes, provider-auth authority, substituted permission or warning
identifiers, duplicate/aliased modes, capability expansion,
privilege elevation, and malformed JSON. Both the complete Designer smoke
assembly and standalone Studio shell compile warning-free against the net472
contract on Linux. Exact-head hosted Linux run `31677215316` executes the
managed workspace-policy smoke under Mono/Xvfb and passes. Its retained
`copperfin-managed-ui-linux` artifact has digest
`sha256:c2c2478e681db16907e2894ab77134506e9be5ebe277558c321870850720a4b1`
and expires `2026-11-11T07:18:18Z`. Exact-head Windows Deep Validation
`31677215577` passes `367/367` native tests, builds the VSIX and both managed
hosts, and executes the managed VSIX, language-service, process-runner, and
Designer smoke suites. The workspace-agent smoke's descriptor-only grammar and
success assertions plus all twenty fail-closed cases pass directly. The retained
`copperfin-windows-deep-validation-Release-build-2-test-2` artifact has digest
`sha256:d0ea4b744ad2924ba37e82bbe8eff86c988cbadc3ea5e159b951134493026b5d`
and expires `2026-11-11T07:18:19Z`.

### DV-V1-workspace-agent-policy-preview

`SmokeStandaloneStudioWorkspaceAgentPolicySurface` requires localized standalone
Studio menu/dialog chrome, advisory default selection, exactly three modes,
host-provided unrestricted warning prose, explicit false elevation, and only a
localized Close button. It requires localized accessibility names for the mode
selector, capability text, and Close action and exercises pseudo-localized
chrome. The preview
has no activation, consent, provider, session, or executor control; selecting a
mode changes displayed information only. Malformed descriptor details map to a
localized generic UI error while retaining the stable diagnostic code. A
synthetic nonzero-host result likewise proves that raw host output is retained
outside the user-facing message rather than treated as trusted prose. Synthetic
missing-host and timeout results prove their messages are selected from fixed
catalog text by diagnostic code rather than copied from the result. Both the full Designer smoke assembly
and standalone Studio shell compile warning-free against net472 locally;
direct Mono and Windows hosted execution remain scheduled before merge.

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

Review of the managed consumer found that `JavaScriptSerializer` could collapse
duplicate object members before shape validation. The correction validates the
JSON token stream before deserialization with a 1 MiB input ceiling and depth
limit 64, rejects duplicate members in every object, and compares decoded member
names so escaped-equivalent spellings also collide. Direct regressions cover
duplicates in the envelope, activation object, warning object, modes,
capabilities, and escaped names. A fresh automated review of rebased exact head
`91e35d3bf3e26279cfe20e498ca57c7e52753aa9` found no further major issue.

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

The managed-client rebased exact head
`91e35d3bf3e26279cfe20e498ca57c7e52753aa9` has direct retained execution on
both hosted paths:

- Linux managed-UI run `31677215316` passes the workspace-agent smoke under
  Mono/Xvfb. Its retained `copperfin-managed-ui-linux` artifact has digest
  `sha256:c2c2478e681db16907e2894ab77134506e9be5ebe277558c321870850720a4b1`
  and expires `2026-11-11T07:18:18Z`.
- Windows Deep Validation `31677215577` passes `367/367` native tests, builds
  the VSIX, passes localized-resource verification, executes the managed VSIX,
  language-service, .NET Framework process-runner, Studio, and Designer
  contracts, and passes the workspace-agent smoke. Its retained
  `copperfin-windows-deep-validation-Release-build-2-test-2` artifact has digest
  `sha256:d0ea4b744ad2924ba37e82bbe8eff86c988cbadc3ea5e159b951134493026b5d`
  and expires `2026-11-11T07:18:19Z`.

These runs verify and close the strict read-only managed-consumer requirement.
They do not implement or close mutable activation, provider authentication,
sessions, audit commits, executor, sandbox, or trusted activation UI.

## Severity, Rollback, And Notification

The credible failure is unintended file deletion, secret disclosure, process
execution, or network activity under the user's identity. Misuse severity is
high; uncontrolled data corruption can intersect `HZ-data-corruption-01`.
For the read-only preview, an additional boundary failure would be presenting
untrusted host stderr as product guidance or exposing a control whose purpose
cannot be determined by assistive technology. The UI therefore maps every
load failure to fixed localized product prose and labels its interactive and
read-only surfaces without granting them mutable authority.
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
