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
| `AFONT()` | `src/runtime/prg_engine_variables.inl` | Host-aware since `#67fb1539` (2026-07-04): searches real platform font directories (`WINDIR/Fonts`, `/System/Library/Fonts` + `~/Library/Fonts`, `/usr/share/fonts` + `~/.fonts`, per OS) and returns the discovered names; only falls back to a fixed stub set when that search finds nothing | Host-capability fallback | Replace the fallback branch only with a platform-owned font-provider slice; the primary host-scan path is no longer a stub |
| Generated runtime bridge exports | `src/runtime/runtime_pipeline_library_export_manifest.cpp` | Emits placeholder return/admission plumbing for unsupported bridge signatures; supported DLL/OCX/FLL API manifest arities are invariant machine metadata after #4868 | Deliberate interop boundary | Track unsupported signatures against the .NET/native interop roadmap; preserve classic-locale arities and do not delegate piecemeal within the generator |
| Unsupported optimizer/query shapes | `include/copperfin/runtime/index_seek_optimizer.h`, `include/copperfin/runtime/rushmore_planning.h` | Records an explicit unsupported/fallback reason and uses the documented fallback | Deliberate semantic boundary | Expand only with a behavior source and regression fixture |

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
