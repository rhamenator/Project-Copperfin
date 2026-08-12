# Portable Native DECLARE Loader Boundary

Copperfin's native `DECLARE` capability remains Windows-only, but module
discovery and ownership no longer expose Windows loader types or operations to
the PRG interpreter state machine. `src/runtime/native_declared_library.h`
defines a portable internal request/result boundary using standard C++ paths,
strings, booleans, enums, and integer-sized opaque identities.

## Preserved behavior

- Explicit absolute and relative library paths retain their existing VFP-style
  resolution. Parentless names retain Windows loader search behavior.
- Bare `WIN32API` still searches the same ordered system modules: Kernel32,
  Gdi32, User32, Mpr, and Advapi32.
- Export lookup retains exact, `A`-suffix, leading-underscore, and Win32
  decorated-stdcall fallback behavior, including the existing cdecl marker.
- Native exports retain precedence for mixed-mode assemblies. A PE with a CLR
  directory and no resolved native export continues to route to the bounded
  managed `DECLARE` host.
- Load and missing-export diagnostics retain their existing localized
  identities, paths, function names, and Windows system-error text.
- Successful redeclaration releases the prior binding; failed redeclaration
  leaves it intact. Session shutdown releases every retained native module.
- Function invocation, argument marshaling, by-reference writeback, return
  widths, and Win32/x64 calling conventions are unchanged by this slice.

## Ownership boundary

`src/runtime/native_declared_library.cpp` privately owns `HMODULE`, `FARPROC`,
`LoadLibraryW`, `GetProcAddress`, `SearchPathW`, `GetSystemDirectoryW`,
`GetModuleFileNameW`, `FormatMessageA`, `FreeLibrary`, and managed-PE
classification. The interpreter stores only `std::uintptr_t` module and
function identities and asks the boundary to release module ownership.

The portable header does not make native libraries callable on macOS or Linux.
The call site remains behind the existing Windows guard and the implementation
is built only on Windows. The header can nevertheless be compiled by the
portable core without importing Windows SDK declarations.

## Regression contract

`test_native_declared_library_boundary_contract` rejects Windows loader types
and functions in the portable header and interpreter loader path, requires
private ownership of every loader operation, and protects the opaque result and
lifetime calls. Generated Launcher Validation runs the contract on Windows,
Ubuntu, and macOS; Windows DECLARE ABI Validation repeats it on Win32 and x64
beside the existing real native and managed fixtures.

Local GCC Release builds the interpreter and its parser/runtime regressions.
Boundary, workflow, isolation, native-DECLARE-adjacent, parser, and broad
runtime-surface coverage passes `6/6`, and a standalone Linux translation unit
compiles the portable header. Deliberately leaking `HMODULE` into the header or
`LoadLibraryW` into interpreter dispatch fails the source contract at the exact
boundary.

Exact signed/DCO implementation head `eb81efa50` passes all eleven protected
checks. Generated Launcher Validation `31566888844` passes the boundary
contract and adjacent generated-launcher, portable-platform, polyglot, MCP,
and SQLite federation coverage on Windows, Ubuntu, and macOS. Windows DECLARE
ABI Validation `31566888629` builds and runs the real native and managed
fixtures plus adjacent DECLARE regressions on Win32 and x64. Windows
environment/path validation `31566888649`, GCC/Clang executable-path
validation `31566888767`, DCO `31566887243`, and both socket checks also pass.
GitHub has no comments or unresolved review threads at that head. An
independent review was requested through the coordination channel; it is not
an acknowledgment dependency, and no response is claimed by this evidence.

## Scope

This is a bounded J1 loader/lifetime seam. It does not move native argument or
return marshaling out of the interpreter, add a POSIX dynamic-library backend,
change `REGFN()`/`CALLFN()`, implement OLE/COM portability, or port shell and
printing behavior. Those native invocation, OLE, shell, printing, and J2/J3
surfaces remain separate work.
