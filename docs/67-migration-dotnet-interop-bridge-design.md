# Migration .NET Interop Bridge: Design

Design pass for issue #5480 (parent #139, related #137, #35, #36, #37), before
any implementation, matching the pattern where higher-ambiguity boundaries get
a design doc first (e.g. `docs/64-workspace-agent-access-policy.md` preceded
the workspace-agent implementation). No code changes in this document.

## Central design decision: reuse the existing polyglot boundary, don't build a new one

#139 asks for "a bounded .NET runtime bridge for executing non-VFP statements
during migration workflows." Copperfin already has a mature, production
boundary that does exactly this shape of thing for a different capability
(`samples.dotnet.add-v1`): `CFPOLYGLOTDISPATCH()` → `PolyglotRuntimeHost` →
route registry → artifact admission (exact SHA-256, rooted, revalidated) →
`invoke_polyglot_artifact` → `run_bounded_process` (one attempt, bounded I/O,
empty environment, no shell) → response envelope admission. See
`docs/19-polyglot-and-ai-subprojects.md` for the full v1 contract chain and
`docs/43-dotnet-polyglot-candidate.md` for the shipped Native AOT leaf
candidate that already exercises every layer of it end to end, cross-platform
(win-x64, win-arm64, linux-x64, linux-arm64, osx-x64, osx-arm64 RIDs).

**#139's migration-execution bridge should be a new capability registered
through this existing machinery, not a second, parallel bridge.** Building a
separate mechanism would duplicate a large amount of already-reviewed,
already-tested trust-boundary code (admission, hashing, revalidation, bounded
process ownership, response validation, telemetry) and would need its own
independent portable-core review -- the exact erosion risk #139 itself warns
about. Reusing the existing boundary means the migration bridge inherits its
guarantees for free: cross-platform by construction (the admission/invocation
path already targets all six RIDs), no CLR hosting, no ambient runtime
discovery, no shell, bounded I/O, exact-hash artifact identity, and existing
telemetry categories (`polyglot.route.selected`, `polyglot.execution.completed`,
etc.).

This also answers the "how does this interact with the portable-core boundary
(#35) and the macOS/Linux ports (#36/#37)" question directly: it doesn't need
to interact with them specially, because the *existing* boundary it reuses was
already designed and evidenced to be portable. A migration-execution
capability that stays inside `CFPOLYGLOTDISPATCH()`'s existing contract cannot
reintroduce a Windows-only or .NET-only dependency into the trusted core,
because nothing in the trusted core calls into it directly -- PRG dispatches
through the same seam every other polyglot capability uses.

## What "executing non-VFP statements during migration workflows" concretely means

This needs a narrower, concrete definition before any implementation, because
"execute non-VFP statements" is ambiguous between two very different things:

1. **Arbitrary C# source execution** (interpret/compile/run caller-supplied
   C# text). This is explicitly out of scope -- it is general code execution,
   which #139's own non-goals and `docs/43`'s "Nonclaims" section both
   prohibit ("not... inline C# or mixed-language PRG syntax," "not general
   arbitrary code execution").
2. **Invoking one pre-built, admitted .NET artifact with structured migration
   data**, the same shape as `samples.dotnet.add-v1`: fixed capability ID,
   bounded JSON arguments, bounded JSON result. This is the only shape
   consistent with the existing boundary and with #139's own scoping as an
   "optional runtime bridge," not a language feature.

**Recommendation: shape 2 only, and one capability per transform.** Each
concrete migration transform must be its own capability, bound to exactly one
admitted executable -- e.g. `migration.dotnet.<specific-transform>-v1`, not a
single generic `migration.dotnet.transform-v1` that accepts a transform
identifier as a request argument and picks among several artifacts. That
generic shape does not fit the existing machinery: `PolyglotRuntimeHost::
dispatch_callback()` selects exactly one bound candidate by `capability_id`
alone and copies the caller's `arguments_json` opaquely into the request
(`src/runtime/polyglot_runtime_host.cpp:262-284`); it has no mechanism for one
capability to resolve or load a *different* artifact based on a runtime
argument. Attempting that inside the candidate itself would mean the .NET
process resolving/loading a named artifact at runtime -- exactly the generic
discovery/loader boundary this document (and #139's non-goals) exclude. This
matches `docs/19`'s own "Developer Migration Playbook v1" principle already:
"A leaf capability has one stable capability ID, one contract revision, one
candidate artifact, and one rollback owner."

The "non-VFP statement" a migration author wants executed is therefore the
fixed logic *inside* one specific admitted .NET artifact per capability
(analogous to how `samples.dotnet.add-v1`'s "statement" is "add these two
integers") -- never caller-supplied source text, and never a caller-selected
artifact, flowing through the boundary at request time.

What crosses the boundary for a given transform capability, mirroring the
existing contract exactly:

- **In**: capability ID, correlation ID, protocol version, one bounded JSON
  arguments object containing only the specific migration-record fields that
  *this* capability's own contract declares (never a raw file path,
  connection string, artifact identifier, or arbitrary blob -- narrow, typed
  fields only, admitted the same way `samples.dotnet.add-v1`'s `left`/`right`
  integers are).
- **Out**: one bounded JSON result object or a typed, nonretryable/retryable
  error envelope, per the existing success/error envelope contract.
- **What the host boundary itself enforces, regardless of which artifact is
  admitted**: no shell invocation, one bounded attempt, bounded stdin/stdout/
  stderr, an explicit (default empty) environment, and process-tree cleanup on
  timeout/cancellation (`run_bounded_process`, `include/copperfin/platform/
  bounded_process.h`). This is a process-lifecycle and I/O boundary, not a
  network or filesystem sandbox -- `run_bounded_process` does not restrict
  what syscalls an admitted executable can make once it is running.
- **What is *not* host-enforced, and must instead come from artifact review**:
  absence of network access, reflection, or arbitrary assembly loading. The
  existing `samples.dotnet.add-v1` candidate has these properties because its
  own source code simply does not do those things (`docs/43`'s "Security
  Boundary And Nonclaims" describes *that candidate's* behavior, not a host
  guarantee) -- a compromised or carelessly written migration-transform
  artifact admitted under this same boundary *could* make network calls or
  load arbitrary assemblies, and nothing in `run_bounded_process` or the
  admission chain would stop it. Each transform capability's implementation
  slice must therefore treat artifact source review (or, for a genuinely
  untrusted artifact, an actual OS-level sandbox -- out of scope for this
  document) as a required control, and must not claim network/reflection
  isolation as inherited for free from the host.

## Guardrails: optional, never a hidden requirement

Mirroring `docs/21-database-federation-and-query-translation.md`'s AI Rule
framing exactly:

- **Deterministic migration path first.** Every migration slice under root
  #137 (database/container interchange, Access forms/reports/VBA, legacy
  xBase interpretation) must have a native, deterministic implementation path
  that does not require this bridge. The bridge is for migration-workflow
  steps where a .NET-ecosystem tool is a genuinely better fit for one bounded
  transform (for example, a well-established .NET library for a specific
  encoding or legacy format quirk) -- it is an accelerant for specific slices,
  not infrastructure the migration pipeline depends on to function.
- **Optional, policy-controlled layer second.** Every migration-bridge
  capability must be registered in the existing route registry, defaulting to
  `off`. No migration slice may hard-require a nonzero route state to produce
  correct output; at `off`, the deterministic native path (or an explicit "not
  yet available" result) must still be correct, just narrower in what it
  covers.
- **A deployment without the artifact must not include that capability's
  binding at all.** `PolyglotRuntimeHost::create()` unconditionally rejects
  construction if a capability's binding fails artifact admission
  (`!binding.artifact_admission.ok()`, `src/runtime/polyglot_runtime_host.cpp:208`)
  and separately requires an exact one-to-one match between route-registry
  entries and capability bindings (`state->capabilities.size() !=
  state->route_registry.entries.size()`, same file, ~line 249) --
  *regardless* of whether that capability's route is `off`. Documenting "at
  `off`, fall back to native" is not sufficient by itself: on a platform or
  deployment where a migration capability's artifact genuinely is not
  available, the host construction that owns *every* capability would fail
  outright unless that capability's route entry and binding are both omitted
  from the set passed to `create()` in the first place. The migration
  workflow layer (not the host) is responsible for knowing which transform
  capabilities are compiled into the current deployment's route/binding set,
  and must use the deterministic native path directly -- without attempting a
  dispatch -- for any capability that was never registered. This is a
  deployment-composition responsibility, not something `CFPOLYGLOTDISPATCH()`
  or the host needs to change to support.
- **Never a hidden dependency.** A migration slice must document, in its own
  requirement/traceability row, whether it uses a bridge capability, whether
  that capability is registered in the current deployment at all (per the
  point above), and what the deterministic fallback behavior is when the
  capability is absent, its route is `off`, or its artifact is unadmitted.
  Silent fallback that produces different output depending on route state
  without recording that fact is not acceptable evidence.

## Non-overlap with `CopperfinStudioHostBridge.cs`

`vsix/Copperfin.VisualStudio/CopperfinStudioHostBridge.cs` resolves and
launches `copperfin_studio_host`, the native process backing the Visual
Studio extension's UI. It is IDE-process plumbing for a single specific
integration (VS extension ↔ native Studio host), unrelated to PRG-level
polyglot dispatch. The migration .NET bridge designed here is a
`CFPOLYGLOTDISPATCH()` capability invoked from PRG migration workflows, with
no relationship to the VS extension's host process. A future implementation
slice must not route through, depend on, or be confused with
`CopperfinStudioHostBridge.cs` in any way; if a name collision risk exists
(e.g. both wanting to say "host bridge"), the migration capability's identity
should be namespaced under `migration.*` in its capability ID, consistent with
the existing `samples.*` prefix convention.

## Concrete bounded first implementation slice (for the follow-up issue)

Per this issue's acceptance criteria, once this doc lands, open a follow-up
issue scoped to exactly this:

1. Define one concrete migration transform capability with its own capability
   ID (e.g. `migration.dotnet.<specific-transform>-v1`) and a fixed, narrow
   JSON argument/result shape for one real, currently-blocked migration
   need -- not a generic "run any .NET transform" capability that picks an
   artifact at request time (excluded above). The specific transform should
   be chosen from an actual gap found in the #137 migration-bridge slices
   (for example, if a specific character-encoding or numeric-format edge case
   in the DBC/DBF JSON import/export work, or in Access field-type mapping,
   turns out to be better served by an existing .NET library than a new
   native implementation). If a second transform need is found later, it gets
   its own capability ID and its own admitted artifact, not a branch inside
   this one.
2. Add that capability's checked-in artifact under `samples/` or a new
   `migration/` sibling directory, following `samples/polyglot-dotnet-candidate`'s
   exact publish/admission/hash pattern.
3. Register the capability in the route registry, defaulting to `off`.
4. Add a `docs/32` traceability row for the migration slice that uses it,
   explicitly documenting the deterministic fallback both when the route is
   `off` and when the capability is not registered at all in a given
   deployment (per the Guardrails section above), and recording that the
   artifact's freedom from network access/reflection/arbitrary assembly
   loading was confirmed by source review of that specific artifact, not
   assumed from the host boundary.
5. Prove the capability end to end the same way `test_polyglot_dotnet_candidate`
   proves the existing sample: focused native/managed tests, cross-platform
   hosted evidence, and the standard protected-check gate.

This slice is intentionally narrow -- one capability, one real use case -- so
it can be evaluated on its own merits rather than as a speculative general
bridge.

## Acceptance

- This design doc exists and is reviewed against the portable-core boundary
  constraints in `docs/19`/`docs/43`.
- It defines a concrete, bounded first implementation slice (above) that a
  follow-up issue can scope directly.
- The follow-up issue is opened once this doc lands, per this issue's own
  acceptance criteria.
