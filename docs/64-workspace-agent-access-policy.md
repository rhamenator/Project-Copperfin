# Workspace-Agent Access Policy

Governing product/derived requirements: `RQ-CF-AGENT-001`,
`RQ-CF-AGENT-002`, `RQ-CF-AGENT-003`, `RQ-CF-AGENT-004`,
`RQ-CF-AGENT-005`, `RQ-CF-AGENT-006`, `RQ-CF-AGENT-007`,
`RQ-CF-AGENT-008`, `RQ-CF-AGENT-009`, `RQ-CF-AGENT-010`,
`RQ-CF-AGENT-011`, `RQ-CF-AGENT-012`, and `RQ-CF-AGENT-013` in
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
contained writer, which also rejects embedded-NUL roots and original path
components before canonicalization or lexical normalization, reparse and
hard-link substitution,
cross-device replacement, malformed or hash-invalid existing chains, and
concurrent-process races. Bounded persistence recomputes the complete existing
chain under the writer lock before admitting a new receipt. A
bounded size uses overflow-safe preflight, including the empty-chain `GENESIS`
field, before path mutation and repeats the
check under the writer lock before any input-sized copy, concatenation, hash
work, or persistent mutation;
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

## Product-owned tool registry and session-bound preflight

Before a mutable adapter exists, the native session controller exposes one
non-executing preflight for future tool dispatch. The version-1 request carries
only the exact active session generation and one stable tool identifier. It
carries no caller-selected capability fields, prompt, path, command,
credential, provider token, or user content. The native product registry maps
that identifier to the complete capability set; model output, provider
adapters, workspace configuration, and user content cannot add definitions or
weaken their requirements.

The version-1 product inventory is declarative:

| Tool identifier | Complete required capabilities |
| --- | --- |
| `workspace.inspect.v1` | workspace read |
| `workspace.apply_edit.v1` | workspace read and write |
| `workspace.run_process.v1` | workspace read/write and local process |
| `local.inspect.v1` | workspace read and outside-workspace access |
| `local.apply_edit.v1` | workspace read/write and outside-workspace access |
| `local.run_process.v1` | workspace read/write, local process, outside-workspace access, and network |
| `network.request.v1` | network |

Registry construction is compile-time validated for canonical unique IDs,
nonempty requirements, and the permanent absence of privilege elevation.
Lookup is exact and case-sensitive; aliases, surrounding whitespace, embedded
NULs, unknown versions, and provider-defined IDs do not resolve. The list
declares future operation classes only. It implements no handler and does not
make a path, process, or endpoint safe.

The controller resolves the ID through that registry, then evaluates its full
requirement set under the same mutex that protects session start, stop, and the
immutable capability snapshot. Unknown schemas or tool IDs, a session
transition, no active session, generation zero or mismatch, and any unavailable
capability fail closed with stable machine diagnostics. Advisory mode admits no
tool request. Workspace-sandbox mode can admit the three `workspace.*` entries
but not outside-workspace or network entries. Unrestricted-local mode can
additionally admit the `local.*` and network entries after its existing
activation warning gate. No registered entry requests privilege elevation.

An allowed result is a point-in-time preflight, not a transferable or reusable
authority token. It does not execute a tool, reserve a session, keep authority
alive after stop, or prove a later side effect was authorized. A future trusted
executor must resubmit the registered tool ID immediately beside every
controlled side effect, resolve the same product definition again, revalidate
the specific target through the applicable containment or process boundary,
and separately audit the actual tool outcome. Provider authentication remains
unrelated to this decision.

## Existing-file target containment preflight

The next non-executing boundary binds an existing regular-file target to the
registered file-tool class and exact active session generation. Product code
constructs the controller with one trusted absolute workspace root; provider,
model, prompt, and workspace content cannot select or replace that root. The
boundary canonicalizes the root once, retains its physical storage/file
identity, and checks that identity before and after each workspace inspection.
The stored root pathname itself must remain a direct directory; replacing it
with a symlink/reparse path back to the same physical directory still fails.

`workspace.inspect.v1` and `workspace.apply_edit.v1` accept only strict
relative paths without root syntax, `.`/`..`, trailing empty leaves, or
embedded NULs. Physical inspection rejects traversal outside the configured
root, symlink/reparse components, cross-device components, missing targets,
directories, and multiply linked files. `local.inspect.v1` and
`local.apply_edit.v1` require the unrestricted session capability plus a
strict absolute path and apply direct-leaf, regular-file, and single-link
identity checks. Sandbox denial occurs before local-path inspection, so it
does not reflect whether an outside target exists.

The registry assigns every tool one compile-time-validated target kind.
Process and endpoint tools cannot be routed through the file boundary. The
controller performs registered-tool/session preflight before filesystem
inspection and repeats it afterward; inactive, transitioning, stale, stopped,
unregistered, wrong-class, or insufficient-capability requests return no
canonical path. An allowed result identifies only the canonical file and its
point-in-time physical identity. It performs metadata inspection (including a
short-lived attributes handle on Windows) but does not read file content,
create, modify, delete, or reserve that target and is not an authority token. A future
executor must repeat session, registry, target, identity, operation, and
outcome-audit checks while holding an OS-backed handle beside each side
effect.

## Process and working-directory target containment preflight

The non-executing process boundary binds one explicit executable and working
directory to the registered process-tool class and exact active session
generation. It never searches `PATH`, expands a bare command, invokes a shell,
accepts arguments or environment values, or launches a process. Provider,
model, prompt, and workspace content cannot replace the product-owned trusted
workspace root.

`workspace.run_process.v1` accepts a strict relative executable and either the
exact `.` spelling for the workspace root or a strict relative working
directory. Both targets must remain physically contained under the unchanged
workspace identity. Indirect/reparse and cross-device components, missing or
wrong-kind targets, POSIX executables without execute permission, and multiply linked
executables fail closed. `local.run_process.v1` requires the warned
unrestricted session and explicit strict absolute executable and working-
directory paths; the executable and directory leaves must be direct, and the
executable must be a single-link regular launch target. Windows device-path,
alternate-data-stream, and UNC remote-share spellings are not admitted while
endpoint containment remains unimplemented. Sandbox capability
denial precedes any local target inspection.

The controller repeats registered-tool and exact-session admission after both
filesystem inspections. An allowed result carries only canonical executable
and working-directory paths plus their point-in-time physical identities. It
does not parse a command, read the executable, construct or inherit an
environment, grant network access, apply a sandbox, pin either target, or
launch. A future executor must repeat the complete check immediately beside
launch, use direct shell-free bounded process invocation with an explicit
secret-free environment, pin or revalidate platform-backed targets as the OS
allows, apply the selected sandbox, and audit the actual outcome.

## Process invocation-shape preflight

Before an executor exists, the next versioned non-executing preflight binds a
direct argument vector to the exact process target, registered tool, and active
session generation. The executable remains a separate canonical target and
becomes `argv[0]` only in the future executor. Each supplied entry is one
argument: no entry is parsed as a command, shell fragment, option bundle,
response file, glob, variable expansion, or search request. Empty arguments,
spaces, wildcard characters, and shell metacharacters therefore remain literal
argument content.

The version-1 vector is bounded to 64 entries, 4,096 UTF-8 bytes per entry, and
8,192 UTF-8 bytes in aggregate. Malformed UTF-8, embedded NULs, and any bound
violation fail closed without returning the argument vector or either target.
The controller copies the bounded vector and then repeats exact-session and
registered-tool admission. The public request has no command-string, PATH-
search, environment, root, shell, standard-stream, timeout, or network-policy
field.

An allowed result names the machine-readable
`isolated_session_v1` environment policy, whose policy query reports parent-
environment inheritance as false. That policy requires a fresh session-owned
environment constructed from product-controlled inputs:
no provider/OAuth token, credential, parent variable, prompt content, or
workspace-supplied environment entry may enter that environment. Provider
credentials must remain in the provider adapter and must never be converted to
process arguments. The preflight does not inspect or classify arbitrary
argument content, so an allow does not certify that arguments are secret-free.
The adjacent isolated-environment boundary now supplies the fixed platform
entries, approved executable-directory list, session-owned profile and
temporary locations, and deterministic locale/time-zone values described
below. The adjacent private-layout boundary supplies fail-closed creation and
verification for an absent generation; trusted-host root provisioning and
cleanup remain separate responsibilities. The executor must also repeat
target and session checks, encode each argument directly for the platform API,
enforce the platform's smaller serialized-command limit if applicable, apply
the real sandbox and endpoint policy, and audit only content-free outcome
metadata. This preflight does not construct that environment, serialize a
command line, read an executable, start a process, or claim execution safety.

## Isolated process-environment construction preflight

`RQ-CF-AGENT-012` adds a concrete logical environment to the same exact
invocation request without adding any request field for environment names or
values. Product code creates the boundary from a versioned trusted-host
configuration: one absolute session-storage root, between one and sixteen
absolute approved executable directories, and, on Windows only, one explicit
system root. Provider, model, prompt, workspace content, and tool arguments
cannot supply or extend those values. The boundary never reads the parent
environment.

For generation `N`, the trusted host must supply a verified private storage
root and use the adjacent preparation boundary to create `session-N` beneath it
with direct `home`, `temp`, `config`, `cache`, and `data` directories. The
environment boundary physically contains and identifies that fixed layout,
rejects symlink/reparse and cross-device components, and checks the configured
storage, executable, and Windows system directory identities before and after
construction. Missing, replaced,
indirect, malformed, path-delimiter-ambiguous, invalidly encoded, excessive,
or wrong-platform inputs fail without returning invocation targets, arguments,
paths, or environment entries. Session directory names use locale-independent
unsigned decimal generation values.

Directory replacement checks bind the storage root, executable directories,
system root, and session layout to stable storage/file identity. They do not
treat mutable directory size, link count, or modification time as object
identity, because normal creation and removal of generation-owned children
changes that namespace metadata. Physical inspection and directory-kind checks
still run before and after construction, so redirection or replacement fails
closed while a trusted host can create a later `session-N` layout.

The POSIX version emits only `HOME`, `LANG=C`, `LC_ALL=C`, `PATH`, `TMPDIR`,
`TZ=UTC`, and the three `XDG_*_HOME` entries. The Windows version emits only
`APPDATA`, `HOME`, `LOCALAPPDATA`, `PATH`, `SystemRoot`, `TEMP`, `TMP`,
`TZ=UTC`, `USERPROFILE`, and `WINDIR`. `PATH` contains only the configured
directories, in configured order; the Windows system root is not implicitly
added. Entries are valid UTF-8, deterministically ordered for their platform,
bounded to 4,096 bytes per `name=value` entry and 32,768 bytes in aggregate,
and contain neither NUL nor caller-defined names. Common credential variables
therefore cannot enter by inheritance. This structural exclusion does not
inspect or certify path text as nonsensitive.

The controller performs the complete invocation preflight, constructs the
environment for that exact generation and policy, and performs the invocation
preflight again before returning a point-in-time plan.

## Private generation-layout preparation boundary

Candidate `RQ-CF-AGENT-014` adds one portable trusted-host primitive before a
workspace-agent process executor is connected. The platform API creates
exactly one absent absolute directory leaf and verifies its access contract.
On POSIX the object must be a non-symbolic-link directory owned by the effective
user with exactly mode `0700`. On Windows it must be a non-reparse directory
owned by the process user with a protected DACL containing only explicit,
inheritable full-control entries for that user and LocalSystem. A missing
parent, existing object, wrong kind, indirection, foreign owner, broadened or
inherited access, unsupported security, or post-creation verification failure
fails closed; existing objects are never adopted or modified. Because a path
can be replaced between creation and verification, failed verification leaves
the path untouched for a later identity-aware trusted-host cleanup decision.
On POSIX, a process `umask` that removes owner bits can therefore make creation
fail and leave an unverified partial path; the library does not mutate the
process-global `umask` or path-chmod an object it cannot prove it created.
POSIX creation and verification traverse every existing parent with
descriptor-relative, no-follow directory opens, reject dot components and
indirect parents, and perform `mkdirat`/leaf verification against the bound
parent descriptor. A private leaf reached through a symbolic-link parent is
therefore not accepted.
Windows likewise opens and inspects every existing parent component with
`FILE_FLAG_OPEN_REPARSE_POINT` before creation and repeats that validation
during post-create verification; an existing symbolic-link or junction parent
therefore fails before its target is modified. Public Win32 creation remains a
full-path operation, so the same-authority race limitation below still applies.

The trusted-host environment boundary requires the configured storage root to
satisfy that same contract. Its explicit preparation method creates one new
`session-N` root and the five fixed children, verifies physical containment and
privacy again, and returns only the generation on success. Generation zero,
root replacement, an existing or partial layout, creation failure, and final
verification failure return content-free diagnostics. The method never adopts,
repairs, overwrites, or deletes an existing or partial layout. A child failure
may therefore leave a private partial generation that deliberately blocks
reuse until a future trusted-host cleanup boundary handles it.
Session-root creation opens the configured private root, compares that handle's
storage/file identity to the captured identity, and on POSIX creates the leaf
relative to the same bound descriptor. Root replacement therefore cannot
redirect the session-root creation side effect on POSIX. Windows repeats the
bound-parent identity check around its public full-path create and remains
within the explicit trusted-parent/same-authority limitation.
Successful bound creation returns the new session directory's storage/file
identity. Every fixed child is created through the same verified-parent
operation against that session identity, so a replacement path cannot receive
the child-creation side effects on POSIX and fails the pre-create identity gate
on Windows.

The generic POSIX leaf-creation operation also rejects an immediate parent that
is not owned by the effective user or root. If that parent permits group or
other writes, it must provide sticky rename protection. This prevents a
different unprivileged principal from replacing a just-created leaf before its
identity-bound inspection; unsafe parents fail before creation. Same-user and
privileged-host interference remain within the trusted-host boundary.

Before creating the session root, preparation derives the exact fixed
platform environment entries through the same builder used by construction.
Invalid encoding, an empty required value, a per-entry overflow, or aggregate
overflow fails without creating any generation directory.
Preparation also revalidates every configured executable-directory identity
and, on Windows, the configured system-root identity before creating the
session root and again before reporting success. A configuration already known
to be replaced therefore cannot consume a generation merely because its cached
canonical text still forms a valid environment entry.

Environment construction re-verifies both the captured storage-root identity
and its current privacy contract before inspecting the generation. Before
returning, it re-verifies identity and current privacy for the root, session,
and every fixed child. Permission or DACL broadening therefore fails even when
the affected directory's filesystem identity has not changed.

This preparation API is not yet wired to session start and is not a cleanup or
execution capability. Its full-path operations assume the private root's owner
and LocalSystem are trusted host authorities; future executor work must retain
sandbox separation from untrusted child processes and repeat identity and
admission checks next to launch. The boundary serializes no arguments, starts
no process, opens no endpoint, injects no provider credential, applies no real
sandbox, and records no tool outcome.

## Platform process-environment serialization preflight

`RQ-CF-AGENT-013` converts only that admitted fixed logical environment into a
native representation. POSIX serialization preserves the deterministic input
order and exact non-NUL value bytes as complete `name=value` storage strings;
a future launcher remains responsible for its transient null-terminated
pointer vector. Windows serialization strictly decodes UTF-8 into UTF-16,
case-insensitively sorts names, rejects case-insensitive duplicates, and emits
the required double-NUL-terminated environment block within an explicit caller
resource cap. The Win32 32,767-character environment limit applies to ANSI,
not the Unicode block used here. Both platforms require portable names matching
`[A-Za-z_][A-Za-z0-9_]*`, reject embedded NUL, duplicates, invalid target
contracts, and overflow, and return no partial representation on denial.

The controller brackets serialization with the complete invocation and logical-
environment preflight, compares the session, target identities, arguments,
policy, platform, and every fixed entry, and returns both the exact plan and
exactly one platform representation only if nothing changed. The shared
serializer also replaces duplicate environment assembly in the existing
bounded-process utility. Neither path consults or merges the parent environment.
The workspace-agent cap adds one storage terminator per fixed entry and, on
Windows, the additional final block terminator to the 32,768-byte logical
profile ceiling.

The serialization boundary still creates or deletes no directory, serializes
no arguments, starts no process, applies no sandbox or endpoint policy, injects
no provider credential, and records no tool outcome. The future executor must
invoke the separate preparation boundary and later clean the session layout,
repeat all identity/admission checks beside launch, consume the fixed
serialized representation without ambient merging,
pin/revalidate launch targets, apply containment, and audit content-free outcomes.

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
The same controller now supplies the product-registry-backed session preflight
described above. The public request cannot supply capability booleans; native
lookup supplies the complete immutable definition. The controller also
supplies the existing-file and process-target preflights described above. These
close product-root and point-in-time identity prerequisites for existing file
tools and explicit process executable/working-directory targets. The adjacent
invocation-shape preflight adds a bounded direct argument vector and mandatory
non-inheriting environment-policy selector. The adjacent trusted-host boundary
constructs the fixed-key, generation-owned logical environment without reading
ambient variables, and the next boundary serializes that exact plan for POSIX
or Windows without launching. None of these results is an executor,
authorization token, or real sandbox. Prospective
file creation, descriptor/handle-pinned reads and writes, delete/rename
semantics, launch-adjacent handle/revalidation, isolated environment
layout creation/cleanup and access-control integration, platform argument
serialization, endpoint policy, process
execution, and tool-outcome auditing remain unimplemented.
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
mode, route stop through immediate revocation, and require a fresh complete
preflight plus target-specific validation and outcome audit at each tool side
effect. Until that wiring exists, the
existing read-only MCP DBF-header host remains Copperfin's only executable AI
tool surface.

Safety traceability for the warning procedure is recorded in
[`safety/traceability-report-2026-08-12-workspace-agent-access-policy.md`](safety/traceability-report-2026-08-12-workspace-agent-access-policy.md).
