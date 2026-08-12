# Portable Native DECLARE Invocation Boundary

Copperfin's native `DECLARE` execution remains Windows-only, but the PRG
interpreter no longer owns Windows ABI argument records, calling-convention
types, native pointer-backed storage, or `DispCallFunc` dispatch. The portable
internal contract in `src/runtime/native_declared_call.h` exchanges standard
C++ typed arguments, a return kind, an opaque integer-sized function identity,
portable failure state, and copied by-reference results.

## Preserved behavior

- Native calls retain the established eight-argument maximum and the existing
  VFP-compatible too-many-arguments diagnostic and error number.
- Win32 retains stdcall by default and the existing cdecl marker; x64 retains
  the single Microsoft calling convention.
- `INTEGER` and `LONG` remain signed 32-bit values, `SHORT` returns remain
  signed 16-bit values, and the explicit `INTEGER64`/`LONGLONG` extensions
  preserve values beyond binary64 precision.
- `SINGLE` remains a 32-bit floating-point ABI value, `DOUBLE` remains 64-bit,
  and mixed integer/floating argument slots retain their Win32/x64 behavior.
- `STRING` arguments remain narrow byte buffers. By-reference string and
  numeric arguments copy their post-call values back to the original PRG
  variable only after a successful invocation.
- Native string, integer, exact 64-bit, `SINGLE`, and `DOUBLE` returns keep the
  existing interpreter value mapping and localized invocation failure identity.

## Ownership boundary

`src/runtime/native_declared_call.cpp` privately owns the stable native backing
storage, raw pointer formation, `VARTYPE`, `VARIANTARG`, `CALLCONV`, `HRESULT`,
`VariantInit`, `DispCallFunc`, and the x64 typed-dispatch implementation. The
interpreter creates only portable scalar/string arguments and applies copied
by-reference results to PRG variables.

The header does not make Windows DLLs callable on macOS or Linux. Its
declarations compile there without Windows SDK types, while the implementation
and its `win64_native_call.cpp` helper are built only for Windows. The existing
call site remains guarded by `_WIN32`.

## Regression contract

`test_native_declared_call_boundary_contract` rejects Windows ABI types and
operations in the portable header and interpreter call path, requires their
private implementation ownership, and protects request/result use plus
by-reference application. Generated Launcher Validation schedules the contract
on Windows, Ubuntu, and macOS; Windows DECLARE ABI Validation schedules it on
Win32 and x64 beside the real native and managed fixtures.

Local GCC Release builds the affected runtime and focused regression targets.
The new boundary contract, loader contract, native-DECLARE-adjacent test,
parser coverage, and broad runtime-surface coverage pass `5/5`. A standalone
Linux translation unit including the portable header also compiles cleanly.
Direct Win32/x64 execution evidence is required before this slice can merge.

## Scope

This is a bounded J1 native-invocation seam. It does not add a POSIX dynamic
library backend, change `REGFN()`/`CALLFN()`, extend the supported VFP `DECLARE`
type set, implement OLE/COM portability, or port shell and printing behavior.
Those OLE, shell, printing, and J2/J3 surfaces remain separate work.
