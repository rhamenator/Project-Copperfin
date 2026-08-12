# Portable CLR-Host Boundary

Copperfin's supported in-process .NET Framework `DECLARE` path remains a
Windows-only capability, but its contract with the PRG interpreter is now
portable C++ data. `src/runtime/managed_declared_call.h` exposes only fixed-width
integers, strings, vectors, enums, and value/result records. Windows SDK, COM,
Automation, CLR-hosting, and `mscorlib` types remain private to
`src/runtime/managed_declared_call.cpp`.

## Preserved behavior

- Windows Win32 and x64 builds still load the exact resolved .NET Framework v4
  assembly through `Assembly.LoadFrom`, locate a public static method through
  the CLR binder, and invoke it in process.
- VFP `DECLARE` values retain their existing scalar mappings: strings,
  32-bit and exact 64-bit signed integers, exact unsigned 64-bit results,
  `SINGLE`, `DOUBLE`, and the existing Automation logical result conversion.
- Assembly-load and method-invocation failures retain the VFP-compatible
  `DISP_E_EXCEPTION` numeric identity. Other CLR-host failures retain their
  native status number and existing localized stage-specific message.
- Exact source/fault context, repeat-call cleanup, mixed-mode native-export
  precedence, path resolution, and the unsupported-by-reference/object/callback
  limitations are unchanged.
- macOS and Linux still do not compile or link the Windows CLR host. Their
  interpreter translation unit nevertheless compiles against the same portable
  boundary declarations, preventing native SDK types from becoming a core
  parsing/runtime dependency.

## Regression contract

`test_portable_clr_host_boundary_contract` rejects Windows, COM, Automation,
and CLR SDK tokens in the portable boundary header; requires the portable
argument/value/error-stage records; verifies private ownership of CLR and
`VARIANT`/`SAFEARRAY` marshaling; and prevents CLR headers or linker directives
from returning to the interpreter translation unit. Generated Launcher
Validation runs this contract on Windows, Ubuntu, and macOS. Windows DECLARE
ABI Validation repeats it on Win32 and x64 beside the existing real managed
fixture.

The managed fixture directly exercises integer, exact signed/unsigned 64-bit,
`SINGLE`, `DOUBLE`, and string result paths, repeated success and
failure, localization, path loading, and mixed-mode precedence. The portable
contract is also run by the native workflow self-check so removal from either
hosted lane fails closed.

Local GCC Release builds the affected runtime, parser regression, and broad
runtime-surface regression. Boundary, workflow, isolation, parser, and runtime
coverage passes `5/5`. This proves the portable interpreter side on Linux;
Win32/x64 behavior remains gated by the hosted managed fixture before merge.

## Scope

This is a J1 boundary correction, not a new interop feature. It does not enable
CLR hosting on macOS/Linux, add inline foreign-language syntax, change the
out-of-process polyglot route, add managed objects/events/callbacks, or isolate
the remaining native DLL and OLE/COM paths. Those broader seams and J2/J3 host
ports remain open.
