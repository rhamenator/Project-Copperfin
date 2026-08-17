# Workspace-Agent Bounded Workspace File Read Traceability

## Scope

This report records candidate `RQ-CF-AGENT-029`: a narrow, read-only
`workspace.inspect.v1` execution boundary. It does not authorize editing,
deletion, rename, local-file access, processes, endpoints, providers, or UI
activation.

## Requirement and design traceability

`RQ-CF-AGENT-029` is derived from the owner policy in
`docs/64-workspace-agent-access-policy.md`, existing exact-generation file
target containment (`RQ-CF-AGENT-009`), and content-free operation audit
discipline (`RQ-CF-AGENT-028`). The implementation is confined to:

- `WorkspaceAgentFileTargetBoundary::snapshot_workspace_file()`;
- `WorkspaceAgentSessionController::read_workspace_file_snapshot()`; and
- `WorkspaceAgentSessionAuditFileSink` schema-v3 validation.

The controller fixes the tool to `workspace.inspect.v1`, accepts only a
schema-version-1 generation and relative workspace target, applies the fixed
4 MiB cap, records a content-free intent before reading, uses the pre-existing
identity-bracketed snapshot primitive, repeats target/session admission after
capture, and clears bytes on any failed outcome audit. Schema-v3 records carry
only mode, generation, operation namespace, operation id, fixed outcome, and
fixed diagnostic.

## Hazards and boundaries

`HZ-system-failure-01` and `HZ-data-corruption-01` apply. The mitigations are
strict product-selected tool scope, bounded data, physical containment,
before/after identity checks, root revalidation, exact active-session checks,
and durable content-free intent/outcome events. This read-only boundary does
not make the workspace sandbox real; it does not provide a mutation operation
or recoverable diff/undo receipt.

## Local verification

Focused Debug CTest passed:

- `test_workspace_agent_target_containment` validates successful owned capture,
  schema rejection before read, invalid-path denial before intent, paired
  content-free schema-v3 correlation, and byte clearing on outcome-audit
  failure.
- `test_workspace_agent_audit_sink` validates persistent
  `workspace_agent.file_read.v3` records and rejects missing/injected
  correlation or diagnostic fields.
- `test_workspace_agent_session` remains green with the expanded audit event
  enum and serialization contract.

Signed/DCO implementation head `03b48f290` passed the protected Windows,
Ubuntu, and macOS generated-launcher matrix (`31991133616`), Windows Win32 and
x64 DECLARE validation (`31991133659`), Windows environment/path validation
(`31991133647`), GCC/Clang executable-path checks (`31991133687`), DCO
(`31991132483`), and both Socket checks. PR `#5048` merged as `9dedee8de`.
The requirement remains `candidate`: the retained evidence does not make the
workspace sandbox real or add mutation, recovery, diff, undo, provider/UI, or
trusted-activation authority. No completion or safety-critical suitability
claim is made.
