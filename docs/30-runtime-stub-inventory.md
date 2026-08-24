# Runtime Stub Inventory

This is the review register for runtime paths that intentionally return a
fallback, depend on an unavailable host callback, or emit placeholder output.
It prevents a no-op from being mistaken for completed VFP behavior and gives
future implementation slices an explicit ownership boundary.

## Register

| Surface | Location | Current behavior | Classification | Follow-up |
| --- | --- | --- | --- | --- |
| `AMEMBERS()` / `ACLASS()` | `src/runtime/prg_engine_runtime_surface_dispatch_object.inl` | Empty arrays and zero when object/array callbacks are unavailable | Host-capability fallback | Keep callback wiring covered; implement only in a separately owned reflection slice |
| `PEMSTATUS()` / `GETPEM()` / `SETPEM()` / `ADDPROPERTY()` / `REMOVEPROPERTY()` | `src/runtime/prg_engine_runtime_surface_dispatch_object.inl` | Deterministic false/empty results when object resolution or member routing is unavailable | Host-capability fallback | Add a child issue before changing callback contracts |
| `CURSORTOXML()` / `XMLTOCURSOR()` | `src/runtime/prg_engine_runtime_surface_dispatch_general.inl` | Empty/false result when cursor snapshot/load callbacks are unavailable | Host-capability fallback | Preserve verified-file admission, invariant/complete field numeric metadata, and machine-contract rules; no silent fallback in strict paths |
| `AFONT()` | `src/runtime/prg_engine_variables.inl`, with host roots in `src/platform/font_directories.cpp` | Searches real platform font directories and returns discovered names; only falls back to a fixed stub set when that search finds nothing. Ordered Windows/macOS/POSIX root selection is platform-owned; enumeration and font-name policy remain runtime-owned. | Host-capability fallback | Replace the fixed fallback only with a separately verified font-provider slice; the primary host scan and its OS-root seam are no longer stubs |
| Generated runtime bridge exports | `src/runtime/runtime_pipeline_library_export_manifest.cpp` | Emits placeholder return/admission plumbing for unsupported bridge signatures; supported DLL/OCX/FLL API arities and generated wrapper source-line/count literals are invariant machine metadata after #4868/#4869 | Deliberate interop boundary | Track unsupported signatures against the .NET/native interop roadmap; preserve classic-locale manifest values and C++ literals, and do not delegate piecemeal within the generator |
| `CREATEOBJECTEX()` remote COM activation | `src/runtime/prg_engine_expression.inl`, `src/runtime/prg_engine_sql.inl` | Preserves the documented class/ProgID, computer-name, and optional-IID inputs in an opaque virtual-COM reference and event, without opening a network connection, consulting the registry, or invoking platform COM. Native PRG classes are deliberately excluded. | Deliberate interop boundary | Implement real remote activation and early-bound invocation only in a separately assessed Windows/security slice; do not treat virtual provenance as a connected remote object. |
| `EVENTHANDLER()` COM-event lifecycle | `src/runtime/prg_engine_expression.inl`; `src/runtime/prg_engine.cpp`; governing recovery in `docs/32-recovered-requirements-traceability.md` | Recognizes the documented expression and returns logical `.F.` for every input because Copperfin has no admitted connected-COM event source, subscription, interface check, or release-cleanup path. Existing native `BINDEVENT()` / `RAISEEVENT()` and virtual `CREATEOBJECTEX()` provenance are not substitutes. | Deliberate interop boundary | Add successful binding only through the documented local Windows COM-event lane: portable admission/lifetime work `#5168` followed by owned-source verification `#5164`; do not add remote activation, registry discovery, or third-party server dependence. |
| Unsupported optimizer/query shapes | `include/copperfin/runtime/index_seek_optimizer.h`, `include/copperfin/runtime/rushmore_planning.h` | Records an explicit unsupported/fallback reason and uses the documented fallback | Deliberate semantic boundary | Expand only with a behavior source and regression fixture |
| SQLite federation connector when the platform dependency is absent | `src/platform/sqlite_federation_connector_unavailable.cpp` | Reports stable `federation.sqlite.connector_unavailable`; validation/release configurations fail during CMake configure instead of admitting this adapter | Build-capability fallback | Keep ordinary source builds portable; `#30` provider execution and every release lane must require the real connector |

`ON KEY [LABEL]`, `PUSH KEY`, and `POP KEY` are not stub entries: their
headless static-assignment and snapshot semantics are implemented in the PRG
runtime. Native input capture, key buffering, and form-local keyboard/UI
semantics are separate compatibility boundaries, not silent fallback behavior.

## Maintenance Rules

- A new stub, placeholder, unavailable callback, or deterministic no-op gets an
  entry here in the same slice that introduces it, with a parent/child issue.
- “Fallback” means the behavior is intentional and tested; it is not an MVP
  completion claim. User-facing diagnostics remain catalog-backed, and parser
  tokens, enum values, JSON keys, and runtime identifiers remain invariant.
- A completion slice removes or updates the row only after focused tests prove
  the replacement behavior and the relevant cross-platform validation passes.
- Delegated work must own a disjoint source/test area and one register row. Do
  not delegate adjacent edits in the same translation unit as active work.

## Discovery Check

Use this review command before selecting a runtime slice:

```sh
rg -n "Stub|stub|placeholder|not implemented|not supported|unsupported|no-op|noop" \
  src include tests docs
```

The search is a discovery aid, not an automatic completion list: “unsupported”
enum values and documented fallback reasons are not necessarily stubs.
