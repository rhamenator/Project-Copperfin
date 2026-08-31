# Specification Compliance Gap Analysis

This document answers one question: **for every document in `docs/` that reads as a
specification or contract rather than narrative direction, what would it take to
make the repository actually meet it?** It is a snapshot, built by reading each
candidate document in full and extracting its concrete, testable requirements,
then comparing them against the ground-truth build graph in
[28-repository-ontology.md](28-repository-ontology.md) and the self-reported gaps in
[30-runtime-stub-inventory.md](30-runtime-stub-inventory.md).

This file does not replace the durable roadmap in [05-roadmap.md](05-roadmap.md) or
the phase/lane evidence in [23-phase-a-dependency-breakdown.md](23-phase-a-dependency-breakdown.md).
It exists to answer "what's the gap between the spec and the code" in one place,
since no other document does that comparison directly.

## Method

- A document counts as a **specification** here if it contains concrete, numbered,
  testable requirements — exact field names, contract shapes, enum lists, format
  bytes, "must"/"shall" statements — not just direction or preference.
- A document counts as **narrative** if it only expresses intent or design taste
  ("modernize the toolbox," "keep the shell thin"). Narrative docs are listed but
  not scored.
- "Gap" evidence is pulled from the specification document's own admissions where
  it makes one (several of these docs are unusually candid about their own
  incompleteness), cross-checked against `docs/28-repository-ontology.md`'s
  ground-truth build graph and `docs/30-runtime-stub-inventory.md`'s registered
  stubs.
- Percentages are not used here for the same reason `docs/05-roadmap.md` avoids
  them: several of the source documents explicitly reject percentage-based
  completion claims. Status is expressed as **met / partial / scaffold-only /
  narrative** instead.

## Specification Compliance Map

```mermaid
flowchart TB
    classDef solid fill:#2f7a52,stroke:#1e5136,color:#ffffff,stroke-width:1px;
    classDef partial fill:#a8790c,stroke:#6e4f07,color:#ffffff,stroke-width:1px;
    classDef seed fill:#c07a2e,stroke:#7a4d1c,color:#ffffff,stroke-width:1px;
    classDef scaffold fill:#8a3a3a,stroke:#5c2626,color:#ffffff,stroke-width:1px;
    classDef narrative fill:#6b7280,stroke:#484d54,color:#ffffff,stroke-width:1px;
    classDef matrix fill:#33475b,stroke:#1f2c38,color:#ffffff,stroke-width:1px;
    classDef lane fill:#f2e0cf,stroke:#a85a2a,color:#1b2024,stroke-width:1px;

    subgraph FOUND["Foundational Contracts"]
      direction TB
      CHARTER["docs/01 Product Charter<br/>Compatibility Fidelity Rule,<br/>stack-frugal requirement,<br/>Requirements Recovery goal"]
      KBX["docs/27 Known VFP9 Bug<br/>Exceptions Registry<br/>SCAFFOLD - zero entries"]
      COMPAT["docs/03 Compatibility And<br/>Migration - 5-layer model,<br/>5 migration modes, 9 tool outputs"]
    end

    subgraph LANG["Language and Data Fidelity"]
      direction TB
      LANGCOV["docs/22 VFP Language<br/>Reference Coverage<br/>1411 documented surface items"]
      IDXFMT["docs/13 Index Format Notes<br/>CDX/IDX/NDX/MDX read+inspect;<br/>no write fidelity, no named collation"]
      ASSETEDIT["docs/12 VFP Asset Editing<br/>And Execution<br/>first-pass xAsset execution lanes"]
    end

    subgraph PIPE["Build, Package, Debug, Trust"]
      direction TB
      PIPELINE["docs/20 Runtime Build And<br/>Debug Pipeline<br/>PRG-first, not full command surface"]
      TRUST["docs/29 Package Trust Contract<br/>signed envelope spec'd;<br/>unsigned fallback still default"]
      CONCURRENCY["docs/25 Engine Concurrency<br/>Policy - fail-fast lock-order/<br/>no-block-while-held invariants"]
    end

    subgraph SEC["Security and Interop"]
      direction TB
      SECMODEL["docs/04 Security Model<br/>Entra ID, Shield, hardening tiers<br/>mostly forward-looking, plus 3<br/>narrow verified present-tense sections"]
      RBAC["docs/18 Native Security And RBAC<br/>claims a current code baseline"]
      DOTNET["docs/11 .NET Bridge Spec<br/>full CLR host + marshaling +<br/>parity matrix spec'd"]
      POLYGLOT["docs/19 Polyglot And AI<br/>Subprojects - policy rules"]
      FEDERATION["docs/21 Database Federation<br/>And Query Translation<br/>translator/planner shipped;<br/>one live read-only SQLite<br/>connector (docs/49); no other<br/>backend connector execution"]
    end

    subgraph STUBS["Self-Reported Gaps"]
      direction TB
      STUBINV["docs/30 Runtime Stub Inventory<br/>6 registered stub/fallback behaviors"]
    end

    CHARTER --> KBX
    CHARTER --> COMPAT
    CHARTER --> LANGCOV
    COMPAT --> ASSETEDIT
    ASSETEDIT --> IDXFMT
    ASSETEDIT --> LANGCOV
    PIPELINE --> TRUST
    PIPELINE --> CONCURRENCY
    PIPELINE --> STUBINV
    SECMODEL --> RBAC
    RBAC --> TRUST
    DOTNET --> POLYGLOT
    POLYGLOT --> FEDERATION
    CHARTER --> DOTNET

    class CHARTER matrix;
    class KBX scaffold;
    class COMPAT seed;
    class LANGCOV matrix;
    class IDXFMT partial;
    class ASSETEDIT partial;
    class PIPELINE partial;
    class TRUST partial;
    class CONCURRENCY partial;
    class SECMODEL seed;
    class RBAC seed;
    class DOTNET seed;
    class POLYGLOT seed;
    class FEDERATION partial;
    class STUBINV matrix;
    class FOUND,LANG,PIPE,SEC,STUBS lane;
```

Legend: dark slate = the specification itself is primarily a measurement/inventory
(coverage matrix, stub registry) rather than a pass/fail contract. Amber = the
contract is substantially implemented with named, bounded gaps. Burnt orange =
a real seed exists but the bulk of the spec's scope is still ahead. Red = scaffold
only — the mechanism exists but is empty or entirely unclaimed.

## Foundational Contracts

### v1 lane J1 — Portable core boundary

**Status: real seed, broad inventory still open.** The common public path,
process-environment, and executable-search interfaces now contain only standard
C++ declarations. The SQLite connector's OS-selected raw C ABI shim is private
to its implementation and no public Copperfin header selects a host platform or
includes a native SQLite header. The Windows-only managed `DECLARE` host now
accepts and returns portable scalar records; `HRESULT`, `VARIANT`, `SAFEARRAY`,
CLR interfaces, and `mscorlib` remain private to its Windows implementation.
Native `DECLARE` module search, export decoration/fallback, PE classification,
system-error formatting, and module release now return portable opaque integer
identities and result state; Windows loader handles and APIs remain private to
one Windows implementation. Native invocation now also exchanges portable
typed arguments, result values, and copied by-reference updates; Windows ABI
storage, pointers, calling conventions, Automation dispatch, and x64 typed
calls remain private to Windows implementation files.
Windows SDK/CRT selection, UTF conversion, host path-component comparison,
the shared normalized case-insensitive runtime path-identity helper, POSIX
environment calls, and process-wide synchronization are private implementation
in `cf_platform_support`. Strict verified-byte snapshot exclusive creation and
durable content writes also cross that boundary while admission, naming,
sidecar, cleanup, and diagnostics remain runtime policy. PRG standard-stream
open and underlying-file resize operations also cross the boundary while VFP
path/mode, handle, error, flush, verified-byte, and close policy remain in the
runtime. `AFONT()` host-root selection also crosses the boundary while
enumeration, font interpretation, array semantics, and headless fallback remain
runtime policy. Direct portable
regressions and source-level contracts run in the Windows, Linux, and macOS
validation workflow, so the boundaries are load-bearing rather than
documentary. See
`docs/50-portable-public-path-boundary.md`,
`docs/51-portable-public-environment-boundary.md`,
`docs/52-portable-executable-search-default.md`,
`docs/61-portable-exclusive-file-boundary.md`, and
`docs/62-portable-file-stream-boundary.md`, plus
`docs/63-portable-font-directory-boundary.md`, plus
`docs/53-private-sqlite-native-api-boundary.md` and
`docs/54-portable-clr-host-boundary.md`, plus
`docs/55-native-declare-loader-boundary.md` and
`docs/56-native-declare-invocation-boundary.md`. APRINTERS host discovery now
crosses the portable contract documented in
`docs/57-portable-printer-shell-boundary.md`: Windows owns Unicode spooler
enumeration privately, while POSIX invokes `lpstat` directly under bounded
process limits without a command shell. `AGETFILEVERSION()` now crosses the
portable metadata record documented in
`docs/58-portable-file-version-boundary.md`; native Windows resource APIs and
the POSIX PE-resource fallback no longer live in the interpreter.

**What it will take:** inventory the rest of the public core and isolate the
remaining report printing, print-dialog, OLE/COM, and other host-specific seams;
the CLR host implementation itself remains Windows-only by design.
This slice does not claim the standalone IDE or full core host has been ported;
those remain J2/J3 work.

### docs/01 — Product Charter

The charter's **Compatibility Fidelity Rule** is the load-bearing requirement for
the whole project: Copperfin must "exactly duplicate VFP9 behavior, including
undocumented edge-case behavior," with exactly two carve-outs — cataloged known-bug
exceptions and cataloged crash-input exceptions — both of which must be evidenced
against real, installed VFP9 or shipped documentation, never decompiled binaries.

**What it will take:** the rule is only as good as its exception registry. See
`docs/27` immediately below — until that registry has real entries, the charter's
rule is *unfalsifiable* in the sense that nothing currently distinguishes an
intentional exception from an unfiled parity bug. The charter's second concrete
requirement — the runtime must "remain stack-frugal rather than reproducing the
native-stack failure profile of `VFP.exe`" — is evidenced as implemented in
`docs/20-runtime-build-and-debug-pipeline.md` ("a heap-backed frame stack inside
the native runtime session... with a tested `MAXCALLDEPTH` guardrail"), so that
half of the charter is met. The charter's Requirements Recovery goal (a
DO-178-style traceability matrix) is explicitly a standing goal per
`docs/28-repository-ontology.md` §7. The durable matrix now begins in
`docs/32-recovered-requirements-traceability.md`; its first shipped-help-backed
row recovers the VFP period-decimal calculation contract. Broad subsystem
coverage remains unfinished.

### docs/27 — Known VFP9 Bug Exceptions Registry

**Status: scaffold only.** The document defines a precise 8-column entry schema
(`KBX-NNN`, classification, VFP9 behavior, evidence, Copperfin behavior, rationale,
status, linked issue) and a 7-step process for adding an entry — but the registry
itself states plainly: *"No entries yet. This is a scaffold... populate it as edge
cases are discovered and validated against real VFP9, not retroactively justified
after Copperfin's behavior is written."*

**What it will take:** a dedicated audit pass that (1) inventories every place the
runtime, designer, or asset layer is already known to diverge from real VFP9
behavior, (2) classifies each as `known-bug` or `crash`, (3) attaches evidence from
shipped VFP9 help/Language Reference/KB content or an installed VFP9 instance —
never decompiled binaries, per `docs/07-clean-room-rules.md` — and (4) files each
as a `KBX-NNN` row before any code comment or test can cite it as an intentional
exception. Until this exists, every undocumented Copperfin/VFP9 behavioral
difference is, by the charter's own rule, a parity defect rather than a sanctioned
exception.

### docs/03 — Compatibility And Migration

Defines a five-layer compatibility model (Data / Metadata / Runtime / UX-Designer /
Execution), five migration modes (Preserve, Wrap, Refactor, Replace, Retarget), a
9-step import pipeline, and a **9-file migration-tool output contract**
(`inventory.json`, `compatibility-report.md`, `unsupported-features.csv`,
`import-manifest.json`, `security-findings.md`, `database-target-plan.md`,
`type-mapping-report.csv`, `asset-roundtrip-report.md`,
`execution-compatibility-report.md`).

**What it will take:** `cf_migration` now provides only the first, read-only
inventory foundation for `inventory.json`: deterministic project-relative asset
classification beneath an absolute non-symbolic-link root, with symbolic links reported but
never followed. The other eight named outputs, dependency discovery, metadata
graph, compatibility risks, migration-mode suggestions, database planning, and
editable workspace mappings remain absent. The Data/Metadata/Runtime layers are
substantially seeded by `cf_vfp_assets`/`cf_design_model`/`cf_xbase_runtime`, but
the full migration pipeline remains one of the widest gaps in the review.

## Language & Data Fidelity

### docs/22 — VFP Language Reference Coverage

This is a **coverage matrix**, not a pass/fail contract: it documents the full
official VFP9 language surface — **429 commands, 413 functions, 323 properties,
83 methods, 72 system variables, 69 events, 22 objects, 4 preprocessor
directives, 3 operators — 1,418 documented items
total** — and tracks per-symbol Copperfin coverage against it, hundreds of entries
deep, each phrased as "documented VFP9 behavior → current partial coverage claim
→ explicitly excluded remaining behavior."

**What it will take:** the document's own backlog section states plainly "the
official command inventory is much larger than the current runtime," and maps the
remaining work to open issues `#7`, `#8`, `#10`, `#11`, `#13`, `#14`, `#22`, `#24`,
`#30`, `#31`. Closing this gap is symbol-by-symbol, evidenced work — there is no
shortcut, by design (per `docs/07-clean-room-rules.md`, coverage can only expand
against real VFP9 behavior or shipped documentation).

### docs/13 — Index Format Notes

**Status: partial.** `CDX`/`DCX`/`IDX`/`NDX`/`MDX` header probing and read/inspect
behavior are implemented, including a shared `has_production_index()` flag and
`runtime.order`/`runtime.seek`/`runtime.rushmore` diagnostic events. But the
document is explicit about two named gaps: normalization/collation hints are
"heuristic metadata rather than true binary collation fidelity" and are not yet
mapped to named collation sequences, and `MDX`/general index **write fidelity is
explicitly out of scope**.

**What it will take:** two bounded, separable slices — (1) a named-collation
mapping table validated against format documentation, and (2) a dedicated
index-write fidelity slice kept deliberately separate from the existing
read/inspection parsing path, per the document's own "Next Steps."

### docs/12 — VFP Asset Editing And Execution

Defines four required modes (Inspect, Round-Trip Edit, Normalize, Execute) across
eight priority asset families. Its own "Current Implementation Snapshot" is candid:
inspection is solid for `DBF`/`FPT`/index families and first-pass `DBC`; local-table
mutation coverage is real; the runtime "can execute a substantial first-pass PRG
surface" including work areas, local queries, and SQL pass-through — but xAsset
bootstraps for forms/classes, reports/labels, and menus are explicitly qualified as
**"first-pass compatibility lanes rather than full VFP parity."**

**What it will take:** the milestones (A–D) in this document give the shape of the
remaining work; the concrete blocker is the same one `docs/22` names — expanding
xAsset execution fidelity is bounded by how much of the underlying language surface
is covered, so this gap and the `docs/22` gap close together, not independently.

## Build, Package, Debug, Trust

### docs/20 — Runtime Build And Debug Pipeline

**Status: partial, and unusually well-instrumented about its own limits.** Concrete,
implemented contracts exist: the `app.cfmanifest`/`app.cfdebug` package layout, the
`key=value` manifest format, the MVP recovery walkthrough's exact CTest targets, and
a stack-safety implementation (heap-backed frame stack, tested `MAXCALLDEPTH`). The
document's own "Current Limitations" section states the execution engine is
**"PRG-first, not yet the full FoxPro/VFP command/runtime surface,"** that
`SCX`/`VCX` embedded xBase code is "partially executable," that `MNX` menu
navigation and `FRX`/`LBX` report execution both "still need work," and that
package manifests are "not the finished long-term runtime format."

**What it will take:** this document is honest that its own MVP walkthrough "does
not replace the full platform validation matrix" and does not by itself satisfy
the release-readiness DV/DQ ledger IDs it cites. Closing this gap is the same
symbol-by-symbol runtime-surface work as `docs/22`, plus a follow-on pass to
formalize the manifest format beyond the current line-based `key=value` contract.

### docs/29 — Package Trust Contract

**Status: partial — spec is exact, default posture is not enforced.** The signed
envelope (`app.cftrust` + `app.cftrust.sig`) has a byte-exact canonical format,
a native verifier (`copperfin::package_trust`, explicitly distinct from
`copperfin::licensing`), and a CI workflow for validation. But the document states
plainly: **"ordinary development packages intentionally use the unsigned fallback
and must not be presented as meeting the Windows release trust boundary"** until an
approved signer and key registry are provisioned, and **"POSIX and macOS do not
claim this Windows trust boundary yet."**

**What it will take:** two concrete, named blockers — (1) provisioning a real
signing key and an out-of-tree approved public-key registry, then setting
`COPPERFIN_ENFORCE_LAUNCHER_TRUST=ON` for release builds, and (2) a separate,
not-yet-scoped decision about whether/how POSIX and macOS get an equivalent trust
boundary at all, since the current design is explicitly Windows-first here.

### docs/25 — Engine Concurrency Policy

**Status: implementation verified by source audit and focused native tests.** Four
hard invariants (named-mutex critical sections, ascending lock order across
sections, strict LIFO exit, no blocking operations while any section is held) are
specified with exact event-category names
(`runtime.critical.order_violation`, `runtime.critical.blocking_violation`) and a
4-step implementation-obligation checklist for any new blocking-capable path.

The shared `ensure_non_blocking_critical_section_policy()` helper is used by
`AWAIT`, positive-duration `SLEEP`, and lock-retry waits reached from record/file
mutation paths. The focused `test_prg_engine_control_flow` and
`test_prg_engine_table_mutation` suites cover ordering, strict-LIFO exits,
reentrancy, fast-fail blocking violations, cooperative `YIELD`, and cleanup after
task faults. This is source/test evidence only; hosted cross-platform validation
remains a separate release gate, and any future blocking path must route through
the same helper.

## Security & Interop

### docs/04 vs docs/18 — Security Model vs Native Security And RBAC

These two documents need to be read together. **Correction (2026-08-31):**
`docs/04-security-model.md` is no longer entirely future-conditional — it now
also contains narrow, verified present-tense sections ("Current MCP
baseline," "Current workspace-agent policy baseline," "Current federation
baseline," `docs/04-security-model.md:59,65,81`) describing real, checkable
behavior. Each was independently verified against source during this
correction: the MCP host's fail-closed `ai.mcp` permission check
(`apps/copperfin_mcp_host/main.cpp`), the workspace-agent access-mode
distinctions (`include/copperfin/security/workspace_agent_session.h`), and
the SQLite federation connector's read-only enforcement
(`src/platform/sqlite_federation_connector.cpp`) all match what docs/04
claims. The bulk of docs/04 — enterprise identity federation, the
9-capability "Copperfin Shield" surface — remains genuinely future-looking;
only these three narrow baseline sections are present-tense and true. Any
internal doc still summarizing docs/04 as "entirely future-conditional" or
"pure scaffold" is itself now stale and should cite this correction instead.
`docs/18-native-security-and-rbac.md` claims a **present-tense baseline**: "a native security profile with explicit
permissions, roles, providers, features, audit events, and hardening profiles,"
surfaced in the Studio host JSON and project workspace summary, built around 5
named roles (`developer`, `build-engineer`, `security-admin`, `auditor`,
`runtime-operator`) and 3 hardening tiers (bronze/silver/gold).

**What it will take:** `docs/18`'s baseline claim maps onto real code —
`cf_security`'s `authorization`, `audit_stream`, `external_process_policy`,
`process_hardening`, and `secret_provider` modules per
`docs/28-repository-ontology.md` §3 — so the RBAC *mechanism* is real. What is
still missing is the wider vision in `docs/04`: enterprise identity federation
(Entra ID/OIDC/SAML), the 9-capability "Copperfin Shield" surface as a distinct
product component (today it's folded into `cf_security`, not a separate
`copperfin-shield` deliverable per the aspirational module list), and independent
verification that the 11-item audit-event enum `docs/04` specifies actually
matches the events `cf_security`'s `audit_stream` emits today.

### docs/11 — Engineering Spec: Copperfin .NET Bridge

**Status: spec'd in full, with a portable policy gateway, artifact-first
invocation adapter, route executor, one PRG-facing Native AOT parity-call leaf,
and an advisory measured-route decision contract; general CLR hosting remains
absent.** The
document specs four native modules (`cf_dotnet_host`, `cf_dotnet_marshaler`,
`cf_dotnet_policy`,
`cf_dotnet_codegen`), three managed surfaces, a typed marshaling contract for 11
value kinds, a policy-driven call gateway with three outcomes
(`allow`/`fallback_native`/`reject`), and a three-tier parity matrix
(`exact`/`adapted`/`intentionally_not_supported`).

The portable profile now enforces verified actor, exact capability scope,
required audit readiness, and separate reflection/assembly/I/O/secret scopes;
it emits stable diagnostics and a structured audit record. An audited allow is
non-executable until a trusted sink commits the event and returns a non-empty
receipt; the focused adapter exercises the contained hash-chained audit stream.
The threat model in `docs/37-dotnet-interop-threat-model.md` fixes the
PRG-supervised job and foreign-thread boundary. The runtime now implements the
nonblocking supervision half for existing `SPAWN` tasks, including status,
cooperative cancellation, retained return values, and completed print output.
Per-task completion publication is synchronized and directly exercised by two
sibling supervisors polling the same handle under ThreadSanitizer; unrelated
tasks remain independently scheduled.
The portable external-process prerequisite now delivers exact bounded stdin and
captures exact stdout/stderr bytes while preserving timeout, cancellation, and
complete descendant cleanup. Three concurrent workers prevent pipe deadlock,
and Windows restricts inherited handles to the three explicit standard handles.
Exact candidate `301d74bf5` passes the full Linux and macOS native suites at
`331/331`, the macOS locale matrix at `8/8`, and the Windows native suite at
`330/330`; the bounded-process regression and all eight protected checks pass.
The portable artifact-admission prerequisite separately binds a canonical
capability, explicit rooted external-process policy, exact SHA-256, and
physical file identity in an opaque, revocable token. It repeats policy,
identity, and exact-byte hashing before execution. The portable artifact
invocation adapter connects that token to deterministic request
serialization, one bounded child-process attempt, strict response admission,
bridge outcome/fallback reporting, and migration telemetry. It revalidates
immediately beside its owned path-based launch, but this is not an atomic
handle-bound execution primitive. The portable route executor now applies the
existing lifecycle decisions, invokes synchronous caller-owned native work,
uses only that adapter for candidate work, preserves native authority during
shadow comparison, and performs one native fallback only when route and bridge
policy both allow it. It does not retry, execute a second artifact, or call
mutable runtime state from a foreign worker. The trusted runtime-host layer now
exposes this route through bounded PRG dispatch.
It also exposes bounded native JSON validation, type inspection, and JSON
Pointer selection so PRG can examine immutable structured completion payloads
without embedding foreign source or losing exact number spelling.
The native safe-regex facade additionally provides bounded byte-oriented text
validation/extraction without a backtracking engine; advanced regex features
remain unsupported unless a future separately approved route supplies them.
The native payload facade also exposes bounded exact-byte SHA-256,
HMAC-SHA256 generation/verification, and strict canonical Base64 encode/decode
for PRG-controlled immutable results,
without claiming sender authentication, encryption, or executable trust.
PRG now has a host-injected bounded dispatch contract plus trusted portable
runtime-host composition from explicit route/admitted-artifact state. A
checked-in C# sample is the first real external language target: it publishes
as a self-contained Native AOT executable and is hash-admitted and invoked end
to end from ordinary PRG. General CLR hosting, arbitrary managed assembly
loading, and broader language adapters remain absent.

The route-impact boundary can deterministically reject or rank already-captured
direct-C++, C++/.NET-wrapper, and C#-service evidence and emit a measured
fallback order. It is deliberately unable to execute benchmarks or promote a
route. Exact signed product/test head `2f21378eb` passes the complete native
matrices on Linux (`338/338`), macOS (`338/338` plus `8/8` locale checks), and
Windows (`337/337`), including the focused route-impact and isolation contracts
on every host. A representative workload runner and checked-in benchmark
results remain open before the routing-strategy criterion can close.

**What it will take:** none of the four named native execution/marshaling
modules exist yet. What exists today, per `docs/28-repository-ontology.md` §3, is
`cf_runtime_pipeline`'s `_csharp_and_launcher` plus the separate Native AOT C#
leaf sample. Neither is a fully specified CLR-hosting-and-marshaling subsystem:
the launcher forwards to the native runtime host, while the sample is one
strict external capability invoked through the generic bridge. Generated
transpilation output remains emitted rather than executed; the runnable
launcher, admitted sample, and transpilation artifact are distinct outputs.

### docs/19 — Polyglot And AI Subprojects

Restates the same .NET gap. Python and R now each have one bounded external leaf
workflow: an admitted interpreter plus a separately admitted,
exact-position-bound script, exercised end to end from ordinary PRG. These are
not general embedded language runtimes or package environments.
Specifies 7 hard rules for MCP/AI tooling (opt-in only, provider-agnostic,
policy-controlled, auditable, local-or-enterprise model choice, user-selectable
models within admin policy, and — critically — ordinary relational queries must
stay deterministic and not require AI).

**Current implementation:** `copperfin_mcp_host` is a real portable local-stdio
surface with one deterministic read-only DBF-header tool. A separate
`cf_security` policy now defines advisory, workspace-sandbox, and warned
unrestricted local-agent modes, exact capabilities, nondefault high-risk RBAC,
localized warning identity, and fail-closed activation decisions. Provider
authentication cannot grant local authority. A non-executing native session
controller now binds one admitted capability snapshot to a committed,
content-free audit receipt and revokes authority before stop auditing. Native
preflights now bind registered existing-file targets and explicit process
executable/working-directory pairs to exact active sessions without file I/O,
PATH lookup, shell interpretation, or launch. A further non-executing preflight
binds a bounded direct UTF-8 argument vector and non-inheriting isolated-session
environment-policy selector to that exact target/session result. The trusted-
host boundary constructs its fixed generation-owned logical environment, a
portable non-executing boundary serializes it for POSIX or Windows without
ambient inheritance, and a separate candidate platform boundary creates and
verifies an absent generation's private fixed layout. Candidate
`RQ-CF-AGENT-017` also rejects malformed, non-launchable, or native-host-
incompatible Windows PE process images while preserving POSIX behavior; it is
structural point-in-time evidence rather than a loader proof or launch token.
Candidate `RQ-CF-AGENT-018` binds Windows C-runtime command-line serialization
to an exact canonical executable identity and SHA-256-attested byte image from
bounded trusted-host configuration, requires exact-digest product evidence that
the parser is self-contained from mutable non-system load-time images, and
repeats that authority check after
serialization; POSIX
continues to use native argv semantics. This is still non-executing point-in-
time evidence rather than launch authority. Candidate `RQ-CF-AGENT-019` now
provides an explicit fail-closed promotion gate: every submitted point-in-time
plan is denied without inspection or reflection, and no plan, digest, or
authority is returned. Review showed that sequential rechecks cannot coherently
bind the trusted root, executable, working directory, and revocation state.
Candidate `RQ-CF-AGENT-022` supplies the separate exact-generation revocation
lease. Candidate `RQ-CF-AGENT-023` now consumes private one-attempt inspection
provenance to retain the original trusted root, executable, and working-
directory objects behind opaque platform handles/descriptors. Candidate
`RQ-CF-AGENT-024` streams the executable through identity-bracketed native
handles/descriptors under a fixed cap, binds the private digest across pin
acquisition, retains the acquisition stream's exact bytes as an immutable
private snapshot, and can reverify only that snapshot without exposing its
bytes, digest, or handle. Candidate `RQ-CF-AGENT-025` now constructs the exact
serialized plan inside the trusted controller and composes it with authenticated
pins and the exact-generation lease behind one opaque move-only candidate after
a complete final revalidation. Candidate `RQ-CF-AGENT-026` consumes that
candidate once, privately materializes only its retained snapshot beneath the
exact receipted generation temp directory, and keeps the plan, pins, and lease
live until the image is destroyed. POSIX retains an unlinked descriptor;
Windows candidate `RQ-CF-AGENT-027` replaces the write-capable creation handle
with an exact file-id read-only identity anchor, retains a linked-path
read/delete handle only after identity equality, and directly
tests the `CreateProcessW` loader open. Both protected Windows workflows now
pass the exact-head pathname, mapping, sharing, loader, and cleanup regressions;
the POSIX/macOS descriptor representation is not yet claimed directly
launchable. An allow path remains unavailable until an executor consumes
exactly that private image and plan through the remaining platform transition
with the sandbox,
endpoint/descendant, and outcome-audit controls.
Candidate `RQ-CF-AGENT-020` now returns exact session/child identities from
successful preparation and exposes a separate trusted-host primitive that
removes only that exact empty layout. It is intentionally disconnected from
session lifecycle; durable receipt recovery, intent/outcome audit, partial-
cleanup retry, and owned nonempty-content policy remain open. The POSIX
descriptor verification followed by name-based `unlinkat` also retains a
documented same-authority leaf-name race.
Broader model/
provider policy, OAuth and credential adapters, trusted-host audit-backed
lifecycle cleanup, exact-snapshot execution and platform launch transition, endpoint
policy, a mutable
executor and real sandbox, outcome audit, and user-facing assistant/dialog
surfaces remain open.

### docs/21 — Database Federation And Query Translation

**Status: partial, with one live connector.** The deterministic Fox SQL
translator and execution planner now feed a bounded, explicitly enabled,
read-only local SQLite connector in `copperfin_runtime_host`. It has direct and
process-level regression coverage and a required Windows/Linux/macOS hosted
build lane. Other relational providers, provider sessions, remote cursors,
mutation/transactions, and non-relational connectors remain absent.

**What it will take:** the document names its own near-term completion bar
explicitly — broader live relational connector execution behind the existing
translator/planner, clearer provider-session/cursor capability contracts, and
deeper local-to-relational mapping documentation. The first SQLite increment is
documented in `docs/49-read-only-sqlite-federation-execution.md`; it advances but
does not close this criterion.

## Self-Reported Gaps

### docs/30 — Runtime Stub Inventory

This document is gap-evidence by construction — a living register of intentional
stubs and fallbacks, each with a file path, a reason, and a follow-up note. Six
entries exist today: `AMEMBERS()`/`ACLASS()` and `PEMSTATUS()`/`GETPEM()`/
`SETPEM()`/`ADDPROPERTY()`/`REMOVEPROPERTY()` (both in
`prg_engine_runtime_surface_dispatch_object.inl`, host-capability fallbacks),
`CURSORTOXML()`/`XMLTOCURSOR()` (same file), `AFONT()` (`prg_engine_variables.inl`,
platform-owned host root selection followed by runtime-owned enumeration and a
fixed headless fallback when no host fonts are found), generated
runtime bridge exports (`runtime_pipeline_library_export_manifest.cpp`, a
deliberate interop boundary), and unsupported optimizer/query shapes
(`index_seek_optimizer.h`/`rushmore_planning.h`, a deliberate semantic boundary).

**What it will take:** the document's own maintenance rule is the answer — "a
completion slice removes or updates the row only after focused tests prove the
replacement behavior and the relevant cross-platform validation passes." Each row
is independently closeable; none of the six are blocked on each other. The
`AMEMBERS()`/`GETPEM()` family and the runtime bridge exports both explicitly
depend on the eventual `.NET`/native interop roadmap (see `docs/11` above) before
they can move past fallback.

## Detailed Gap Diagrams

Both moved to their own files because GitHub's Mermaid renderer only
reliably renders the first diagram on a page with multiple diagrams; this
page keeps only the Specification Compliance Map inline.

### Interop, Federation, Trust, and Security

See
[diagrams/gap-analysis-interop-federation-trust-security.md](diagrams/gap-analysis-interop-federation-trust-security.md).

### Language & Data Fidelity

See
[diagrams/gap-analysis-language-data-fidelity.md](diagrams/gap-analysis-language-data-fidelity.md).

## Narrative Documents (Not Scored)

These read as direction/preference rather than testable contracts, and are
excluded from the compliance map above:

- `docs/09-adr-cpp-first.md` — architecture decision record; accepted, narrative by design.
- `docs/14-hybrid-studio-and-visual-studio-host.md` — host-model decision and sequencing; forward-looking recommendation, no current-state claims.
- `docs/16-vfp9-equivalent-subsystems.md` — a VFP9-module-to-Copperfin-target naming map, not a behavioral contract.
- `docs/17-modern-designer-direction.md` — UX/design direction only.

## Summary Table

| Doc | Subject | Status | Single biggest blocker |
| --- | --- | --- | --- |
| 01 | Product Charter | Partial | Empty exception registry makes the Compatibility Fidelity Rule unfalsifiable |
| 03 | Compatibility And Migration | Partial seed (migrator) | Only read-only `inventory.json` foundation exists; eight outputs and import analysis remain |
| 04 | Security Model | Mostly aspirational, with 3 narrow verified present-tense baseline sections | No enterprise identity/Shield product surface distinct from `cf_security` |
| 11 | .NET Bridge Spec | Partial (Windows DECLARE plus portable policy/adapter/route executor) | None of the four named native `cf_dotnet_*` modules exist |
| 12 | VFP Asset Editing And Execution | Partial | xAsset execution is first-pass, bounded by language-surface coverage |
| 13 | Index Format Notes | Partial | No index write fidelity; collation hints are heuristic, not named |
| 18 | Native Security And RBAC | Partial (real baseline) | Not yet verified against docs/04's fuller vision |
| 19 | Polyglot And AI Subprojects | Partial (portable artifact boundary, route executor, trusted host composition, PRG seam, Native AOT C# leaf, admitted Python/R sidecar leaves, advisory measured-route strategy, versioned benchmark evidence, bounded read-only MCP DBF-header host, and non-executing agent file/process target preflights) | No general CLR/Python/R runtime surface or broader model/provider and mutable MCP tooling |
| 20 | Runtime Build And Debug Pipeline | Partial | Engine is PRG-first, not the full command surface |
| 21 | Database Federation And Query Translation | Partial (real seed) | No live connector execution behind the translator/planner |
| 22 | VFP Language Reference Coverage | Partial, measured | 1,418 documented items; official surface exceeds current runtime |
| 25 | Engine Concurrency Policy | Met for current implementation | Shared blocking-policy helper and focused native coverage verify the four invariants; hosted cross-platform evidence remains a release gate |
| 27 | Known VFP9 Bug Exceptions Registry | Scaffold-only | Zero entries |
| 29 | Package Trust Contract | Partial | Unsigned fallback is still the default; POSIX/macOS unclaimed |
| 30 | Runtime Stub Inventory | Self-reporting | 6 registered stubs, each independently closeable |

## Documentation Ownership

- This file owns the specification-vs-implementation gap analysis. Its two
  detailed gap diagrams live in `docs/diagrams/gap-analysis-*.md`, split out
  because GitHub's Mermaid renderer only reliably renders the first diagram
  on a page with several.
- It does not restate `docs/05-roadmap.md`'s completion model or phase/topic map,
  and does not claim overall project completion percentages.
- Refresh it when a cited document's own status language changes, or when a
  gap closes and should move to the historical record instead.
- 2026-08-31 correction: the Database Federation node (both the inline map
  and the detailed diagram) still read "no live connector execution" and
  classified `FEDERATION` as `seed`, contradicting this file's own prose two
  paragraphs later, which already documented a real, tested, read-only local
  SQLite connector (`docs/49`) with a Windows/Linux/macOS hosted build lane.
  Verified against `src/platform/sqlite_federation_connector.cpp` and its
  test files directly (they exist and are wired into the build), not just
  against the docs. Reclassified to `partial` and narrowed the stated gap to
  the remaining non-SQLite backends. Separately, `docs/22`'s official VFP9
  surface count grew from 1,411 to 1,418 documented items (it now also lists
  4 preprocessor-directive and 3 operator entries); this file's two citations
  of the stale 1,411 total (prose and summary table) are corrected to match.
  Neither correction changes any other document's status language, so no
  further rows moved.
- 2026-08-31, second correction (same day): a systematic audit comparing
  every scored document here against actual source (not other docs) found
  that `docs/04-security-model.md` had grown 3 narrow, verified,
  present-tense "Current ... baseline" sections (MCP, workspace-agent
  policy, SQLite federation) since this file last characterized it as
  "written entirely in the future conditional." Each was independently
  re-verified against source during this correction (see the docs/04 vs
  docs/18 section above for file citations). Reclassified `SECMODEL` from
  `scaffold` to `seed` in the map and updated the summary-table row; the
  bulk of docs/04 (Entra ID/OIDC/SAML, the 9-capability Shield surface)
  remains genuinely aspirational, so this is a narrowing of the claim, not
  a reversal. The same audit also found and fixed unrelated staleness in
  three other files (not scored by this document, so not reflected in its
  map): `docs/22-vfp-language-reference-coverage.md`'s "Active Issue
  Mapping" section still listed 8 closed issues as active backlog;
  `docs/24-system-uml.md` said "five native executables" while its own
  diagram already showed six; `docs/28-repository-ontology.md` was missing
  `cf_migration` from its library table/dependency graph and had a stale
  "~300 files" test count (actual: 820 `.cpp` files, 227 registered CTest
  executables). All four fixes were verified directly against source/CMake
  output before being applied, not inferred from the docs alone.
