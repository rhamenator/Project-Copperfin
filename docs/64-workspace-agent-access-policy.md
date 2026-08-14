# Workspace-Agent Access Policy

Governing product/derived requirements: `RQ-CF-AGENT-001`,
`RQ-CF-AGENT-002`, `RQ-CF-AGENT-003`, `RQ-CF-AGENT-004`,
`RQ-CF-AGENT-005`, and `RQ-CF-AGENT-006` in
`docs/32-recovered-requirements-traceability.md`. The public policy header,
descriptor implementation, strict managed client, and focused tests carry the
reverse links back to those requirements.

Copperfin's built-in coding assistant is intended to progress from advisory
help to useful workspace editing and process execution. Provider authentication
and local machine authority are separate decisions: signing in to a model
provider never grants filesystem, process, network, or privilege access.

## Access modes

The provider-independent `cf_security` policy exposes three stable modes:

| Mode | Workspace files | Local processes | Outside workspace | Network | Elevation |
| --- | --- | --- | --- | --- | --- |
| `advisory` | none | none | none | none | never |
| `workspace_sandbox` | read/write/delete | allowed inside the sandbox contract | denied | denied | never |
| `unrestricted_local` | read/write/delete | allowed as the current user | allowed as the current user | allowed | never |

The lowercase names above are the exact machine spellings. The parser rejects
aliases, case changes, surrounding whitespace, and unknown values; invalid
enum values serialize only as the non-admissible `invalid` sentinel.

The workspace sandbox is the intended default once mutable assistant tooling
ships. Unrestricted mode is deliberately available for users who accept its
risks, but it never means administrator, root, UAC, `sudo`, or another implicit
privilege elevation.

## Unrestricted warning gate

The host may admit `unrestricted_local` only when all of these facts are true:

1. the workspace assistant was explicitly enabled;
2. the effective native role grants the high-risk `ai.workspace_agent`
   permission;
3. activation originates from trusted Copperfin product UI;
4. an audit sink is available;
5. the product displayed the exact current warning contract,
   `workspace-agent.unrestricted-local.v1`; and
6. the user affirmatively accepted that warning.

Missing consent, a stale or substituted warning, direct adapter activation,
missing permission, and missing audit all fail closed. Warning identity and
diagnostic codes are invariant machine contracts; warning and decision prose
is localized through the normal Copperfin catalogs.
Every activation decision requires a content-free audit outcome, including a
request denied because the feature is disabled; disabled state removes local
authority, not security observability.
Unknown or out-of-range serialized access modes also fail closed without any
capabilities, even if every unrestricted-warning field is present.

The warning tells users that the agent can read, modify, or delete any file
their account can access, run local processes, use the network, and expose
secrets available to that account. It also states that provider sign-in does
not grant this authority and that Copperfin will not elevate privileges.

## Provider authentication boundary

Model adapters must use an officially supported delegated OAuth flow when a
provider offers one for embedded clients. Otherwise they must use a separately
configured API credential or an administrator-managed gateway. Copperfin must
not copy, borrow, or replay another editor extension's session token.
Credentials belong in the platform secret provider or operating-system vault;
they never belong in workspace files, prompts, audit content, or source
control. Authentication success is not an input to
`evaluate_workspace_agent_activation()` and therefore cannot bypass the local
policy.

## Studio-host descriptor boundary

`copperfin_studio_host --workspace-agent-policy --json` is the versioned,
read-only bridge for trusted product clients. Schema version 1 reports the
default-disabled feature state, advisory default, exact modes and capabilities,
native permission id, trusted-UI and audit requirements, invariant warning id,
localized warning prose, provider-authentication separation, and the permanent
denial of privilege elevation. It explicitly reports `descriptorOnly: true`
and `activationAvailable: false`. Capability fields are obtained from
`evaluate_workspace_agent_activation()` rather than copied into a second policy
table.

This command only describes policy. Its strict grammar accepts the descriptor
switch followed by an optional `--json`; mixed operational switches, duplicate
switches, reordered switches, and attempted activation arguments return status
2 without a partial descriptor. A command line therefore cannot assert that it
is trusted product UI or manufacture warning consent. Actual activation remains
reserved for the future trusted session/UI boundary.

The shared managed `CopperfinWorkspaceAgentPolicyClient` consumes only that
exact JSON descriptor command. Before exposing a descriptor to either the
standalone Studio shell or VSIX, it requires the complete schema shape and
field types, descriptor-only/no-activation state, advisory/default-disabled
state, provider-auth separation, native permission and UI/audit/warning gates,
the current warning identity, exactly three stable mode names with exact
capabilities, localized nonempty warning prose, and permanent denial of
elevation. Missing or unknown fields, aliases, duplicate members or modes,
stale identities, type substitutions, and any capability expansion fail
closed. This client is
read-only and does not supply an activation or execution method.

Standalone Studio exposes that validated descriptor through a localized
**Workspace Assistant Access** preview under **View**. The dialog defaults to
advisory, lets the user inspect all three exact modes and capabilities, and
uses the host's validated current warning identity to select catalog-owned
warning prose when unrestricted mode is selected. Host-provided warning text is
never rendered as trusted product guidance.
It states that activation is unavailable and contains only a Close button: it
does not enable a feature, grant permission, record consent, start a session,
authenticate a provider, or execute a tool. Invalid descriptor diagnostics are
kept behind fixed localized UI text rather than exposing raw parser, process,
or untrusted host-output details. Missing-host and timeout diagnostics select
their catalog-owned actionable guidance; invalid, unknown, start-failure, and
nonzero-host-output cases use the generic verification error. The mode
selector, capability text, status, and Close action expose localized
accessibility names. Selecting a mode only updates the displayed descriptor
information.

## Persistent lifecycle audit boundary

The provider-independent `WorkspaceAgentSessionAuditFileSink` persists only
the exact version-1 lifecycle tuples produced by the native session controller.
Each accepted start, denial, or stop is appended as
`workspace_agent.session.v1` to the existing contained immutable audit chain;
the committed entry hash becomes the controller receipt. The record contains
only lifecycle kind, generation, requested and effective mode, outcome, and
diagnostic code. It does not contain prompts, workspace or file paths,
credentials, provider tokens, warning text, user content, or the returned
receipt.

The product host must supply an existing product-owned storage root and a
relative log path. The sink canonicalizes that root but preserves the original
relative path for no-follow traversal. Existing symlink redirection makes the
configuration inert; a post-construction link swap is rejected by the common
contained writer, which also rejects reparse and hard-link substitution,
cross-device replacement, malformed or hash-invalid existing chains, and
concurrent-process races. Bounded persistence recomputes the complete existing
chain under the writer lock before admitting a new receipt. A
bounded size is checked under the writer lock before allocation or mutation;
the default is 4 MiB and admitted configuration range is 512 bytes through
64 MiB. Invalid configuration or event shape, containment failure, malformed
state, and a full log return no receipt. Start therefore grants no authority,
while stop authority has already been revoked before persistence is attempted.

The chain uses unkeyed hashes for tamper evidence; it is not a digital
signature or authenticated external ledger. An attacker controlling the
storage root can delete or replace the whole chain. Product-owned root/ACL
selection, rotation and retention, external or authenticated anchoring,
multi-host correlation, recovery UI, and integration with the eventual trusted
activation host remain explicit gaps.

## Current implementation and remaining work

The current slices implement the portable access-mode, capability, RBAC,
localized warning, fail-closed admission, read-only Studio-host descriptor,
and strict read-only managed-consumer contracts with direct regression
coverage. A localized read-only Studio preview makes that contract visible
without weakening the activation boundary.

The native `WorkspaceAgentSessionController` is the first non-executing
session-lifecycle boundary. It reevaluates the existing activation policy,
withholds all authority until a content-free start event is durably accepted
with a nonempty audit receipt, and binds the admitted mode and capabilities to
one immutable session generation. A second start cannot replace or expand an
active session. An unexpected policy-evaluation failure becomes a content-free,
audited denial and restores the controller to an idle transition; it creates no
authority and cannot permanently block a later valid start. Stop clears the
authority snapshot before invoking the audit sink, so a missing, failing,
empty-receipt, or throwing stop sink remains
visible but cannot prolong authority. Inactive snapshots retain neither
capabilities nor the activation receipt. The versioned JSON audit event carries
only event kind, generation, requested/effective mode, outcome, and stable
diagnostic code; it carries no prompt, workspace path, credential, or receipt.
Serialization uses the classic locale, so process-wide digit grouping cannot
change numeric fields or control-character escapes into invalid JSON.
This native contract has no command-line or product-UI activation surface and
does not itself access files, run processes, use the network, or authenticate a
provider.
Weakening the warning-identity comparison to admit a stale nonempty warning
causes the dedicated regression to fail at that exact assertion; restoration
returns the policy test to green.
The policy test also passes under Clang ASan/UBSan with leak detection and no
findings.
Corrected signed/DCO head `4c4014f94` passes all eleven protected checks,
including native execution on Windows, Ubuntu, and macOS. Exact run identifiers
and the corrected independent-review record are in the safety traceability
report.
The descriptor process regression requires the versioned defaults, three mode
names, risk-bearing capabilities, provider separation, RBAC/UI/audit gates,
exact warning identity, and no elevation. It also proves mixed switches and a
generic `--activate-unrestricted` attempt fail without emitting policy output;
dedicated process cases likewise reject duplicate and reordered descriptor
switches without partial output. Policy-bearing arguments are validated before
the optional product-license status handler, so enabling the archived licensing
build flag cannot consume a mixed request and bypass the exclusive grammar.
The policy/descriptor pair passes Clang ASan/UBSan with leak detection `2/2`
and no findings. The focused Release selection covering policy, the real host
process, native isolation, and safety traceability passes `4/4`; the emitted
schema also parses and satisfies its invariant fields with `jq`.
Corrected signed/DCO descriptor head `def609305` passes all eleven protected
checks, including direct native execution on Windows, Ubuntu, and macOS. The
first Windows generated-launcher attempt was blocked before Copperfin
configuration by an external R-version lookup failure; its failed job was
rerun without a source change and passed. Exact run identifiers are retained
in the safety traceability report.
The managed smoke project and standalone Studio project compile warning-free
against the net472 reference contract on Linux. Exact-head hosted Linux run
`31677215316` executes the managed smoke under Mono/Xvfb and passes; its
retained `copperfin-managed-ui-linux` artifact has digest
`sha256:c2c2478e681db16907e2894ab77134506e9be5ebe277558c321870850720a4b1`
and expires `2026-11-11T07:18:18Z`. Exact-head Windows Deep Validation
`31677215577` passes `367/367` native tests, builds the VSIX and both managed
hosts, and executes the managed VSIX, language-service, process-runner, and
Designer smoke suites. The workspace-agent smoke passes its descriptor-only
grammar and success assertions plus all twenty fail-closed cases. The retained
`copperfin-windows-deep-validation-Release-build-2-test-2` artifact has digest
`sha256:d0ea4b744ad2924ba37e82bbe8eff86c988cbadc3ea5e159b951134493026b5d`
and expires `2026-11-11T07:18:19Z`.
The read-only preview's exact product/test head `4d76b3277` passes Linux
Mono/Xvfb run `31687794634`; its retained `copperfin-managed-ui-linux`
artifact has digest
`sha256:ce816aa5755edea6bbb3750307831a56debbb1a4a1d5aa02aff5150dd028bb60`
and expires `2026-11-11T09:42:16Z`. Windows Deep Validation `31687794715`
passes `367/367` native tests and the complete managed/Studio/Designer
selection, including direct localized-preview, background-load,
catalog-owned-warning, and pseudo-localization assertions. Its retained
`copperfin-windows-deep-validation-Release-build-2-test-2` artifact has digest
`sha256:83a8f7c96ffefd173b98781f21e0f8498610534164f5acf2f0afb7a6c4114ad9`
and expires `2026-11-11T09:42:15Z`.
It does not yet ship a model adapter, OAuth client, conversation UI, mutable
tool executor, sandbox implementation, diff/undo surface, product-visible stop
control or session indicator, or the WinForms consent dialog that must render
and bind the warning during a real activation attempt.

Those surfaces must consume this policy rather than duplicate it. The trusted
host must persist the controller's content-free activation outcome events,
keep unrestricted activation session-scoped, visibly indicate the effective
mode, and route stop through immediate revocation. Until that wiring exists, the
existing read-only MCP DBF-header host remains Copperfin's only executable AI
tool surface.

Safety traceability for the warning procedure is recorded in
[`safety/traceability-report-2026-08-12-workspace-agent-access-policy.md`](safety/traceability-report-2026-08-12-workspace-agent-access-policy.md).
