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
coverage passes `5/5`. Corrected exact head `9485852c1` passes all eleven
protected checks. Generated Launcher Validation `31563614971` passes the
boundary contract and adjacent generated-launcher, portable-platform,
polyglot, MCP, and SQLite federation coverage on Windows, Ubuntu, and macOS.
Windows DECLARE ABI Validation `31563615036` passes the real managed fixture
and adjacent DECLARE tests on Win32 and x64. Windows environment/path
validation `31563615085`, GCC/Clang executable-path validation `31563615003`,
DCO `31563612877`, and both socket checks also pass.

Independent review found no defect. It read-verified every COM owner and all
15 managed-failure returns for single cleanup, compared the moved error mapping
to its prior implementation, compiled the header from Linux, mutation-proved
both native-type exclusion and portable error consumption, and passed the
runtime host and broad control-flow regression under ASan/UBSan. That review
could not execute Windows CLR hosting; actual CLR invocation equivalence is
therefore established by the hosted Win32/x64 fixture, not by the Linux review.
The portable contract is the declaration and interpreter boundary; the
Windows-only function intentionally has no non-Windows implementation.

## Scope

This is a J1 boundary correction, not a new interop feature. It does not enable
CLR hosting on macOS/Linux, add inline foreign-language syntax, change the
out-of-process polyglot route, add managed objects/events/callbacks, or isolate
the remaining native DLL and OLE/COM paths. Those broader seams and J2/J3 host
ports remain open.
