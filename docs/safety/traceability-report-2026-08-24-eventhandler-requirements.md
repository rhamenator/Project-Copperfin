# EVENTHANDLER Requirements-Recovery Boundary

## Scope and evidence

This record covers the portable capability boundary. It does not implement COM
activation, invoke a server, change native `BINDEVENT()` / `RAISEEVENT()`
semantics, or claim platform COM-event dispatch exists.

The compatibility source is mounted VFP9 `dv_foxhelp.chm` (10,870,662 bytes;
SHA-256 `abaa86e7623bb00e8bd9323cf2d8e162013598e35d9492557a3ddd1c2cf13e79`),
topic `html/81422070-37ef-492f-b813-9b40bcaed762.htm`,
`EVENTHANDLER( ) Function`. It specifies the signature, valid COM source,
matching implemented interface, logical result, explicit unbind, multiple
bindings, and automatic unbind when either object is released.

## Requirement and verification map

| Documentation requirement | Verification disposition | Controlled hazards |
| --- | --- | --- |
| `DQ-CF-EVENTHANDLER-001`: recover and distinguish the VFP9 contract from native events and virtual COM provenance. | `DV-CF-EVENTHANDLER-001`: traceability review of `LLR-VFP-COM-002`, source identity, and child boundaries. | `HZ-runtime-crash-01`; `HZ-system-failure-01` |
| `DQ-CF-EVENTHANDLER-002`: require fail-closed local admission, lifetime cleanup, and no external/remote discovery. | `DV-CF-EVENTHANDLER-002`: portable coverage of admission, interface/method validation, unbind, duplicates, and release cleanup; later owned-Windows fixture coverage of dispatch and faults. | `HZ-runtime-crash-01`; `HZ-system-failure-01` |

## Hazard, misuse, boundary, and rollback analysis

| Case | Hazard / effect | Required control and verification |
| --- | --- | --- |
| Virtual `CREATEOBJECTEX()` provenance, a remote target, or an arbitrary external server is treated as connected. | `HZ-system-failure-01`: uncontrolled activation or nondeterministic external behavior. | Admit only a product-owned local Windows event-source boundary; reject virtual provenance and remote/unavailable sources before subscription. |
| Source/handler is invalid, interface-incompatible, or released. | `HZ-runtime-crash-01`: invalid callback dispatch or stale lifetime use. | Return `.F.` without partial binding and remove bindings on either release; test both release directions. |
| A handler faults or duplicate bindings are ambiguous. | `HZ-runtime-crash-01` / `HZ-system-failure-01`: unsafe re-entry, stale callback, or repeated dispatch. | Contain faults through the runtime error boundary, leave deterministic safe state, and test faults plus duplicate/multiple binding rules. |

No requirement may be inferred from existing Copperfin code. The runtime child
must map implementation and tests back to `LLR-VFP-COM-002`; behavior beyond
the VFP topic must be recorded as owner policy or derived safety constraint.

Rollback before release is a normal code/configuration revert with no server
registration, network connection, or external process. A defect must disable
local event-source admission rather than fall back to virtual provenance,
native events, or arbitrary COM activation.

## Current disposition

`DV-CF-EVENTHANDLER-001` is satisfied by this recovery record. #5178 provides
the portable fail-closed host capability boundary, deterministic bind/unbind
registry, and source/handler release cleanup; it does not produce platform COM
callbacks. Owned Windows connection-point dispatch, handler-fault containment,
and fixture evidence remain pending #5164. `LLR-VFP-COM-002` is therefore
still a `gap`, not release evidence for a shipped COM-event feature.
