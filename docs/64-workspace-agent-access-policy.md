# Workspace-Agent Access Policy

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

## Current implementation and remaining work

This slice implements the portable access-mode, capability, RBAC, localized
warning, and fail-closed admission contracts with direct regression coverage.
Weakening the warning-identity comparison to admit a stale nonempty warning
causes the dedicated regression to fail at that exact assertion; restoration
returns the policy test to green.
The policy test also passes under Clang ASan/UBSan with leak detection and no
findings.
Corrected signed/DCO head `4c4014f94` passes all eleven protected checks,
including native execution on Windows, Ubuntu, and macOS. Exact run identifiers
and the corrected independent-review record are in the safety traceability
report.
It does not yet ship a model adapter, OAuth client, conversation UI, mutable
tool executor, sandbox implementation, diff/undo surface, stop control,
session indicator, or the WinForms dialog that must render the warning.

Those surfaces must consume this policy rather than duplicate it. The trusted
host must record content-free activation outcome events, keep unrestricted
activation session-scoped, visibly indicate the effective mode, and revoke
capabilities immediately when the session stops. Until that wiring exists, the
existing read-only MCP DBF-header host remains Copperfin's only executable AI
tool surface.

Safety traceability for the warning procedure is recorded in
[`safety/traceability-report-2026-08-12-workspace-agent-access-policy.md`](safety/traceability-report-2026-08-12-workspace-agent-access-policy.md).
