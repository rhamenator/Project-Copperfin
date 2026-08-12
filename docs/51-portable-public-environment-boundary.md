# Portable Public Environment Boundary

Project Copperfin's public process-environment interface is platform-neutral.
The header `include/copperfin/platform/environment.h` exposes only standard C++
types and declarations; it does not include the path implementation, select an
operating system, or embed native environment calls. The implementation belongs
to `src/platform/environment.cpp` inside `cf_platform_support`.

This is a bounded J1 follow-up to the portable public path boundary. It prevents
runtime, security, licensing, asset, Studio, and application consumers from
compiling Windows/POSIX environment mechanics and synchronization into every
translation unit that includes the interface. It does not claim that the whole
core is portable or complete the macOS or Linux ports.

## Preserved behavior

- Names remain nonempty ASCII on Windows and reject `=` or embedded NUL bytes
  everywhere.
- Windows continues to use the wide-character CRT environment APIs and the
  shared strict UTF-8/native-path conversion boundary.
- POSIX reads, writes, and clears continue to serialize through one process-wide
  mutex for callers using these helpers.
- Empty Windows assignments retain CRT removal semantics; POSIX empty
  assignments remain distinct from missing variables.
- Path values, Unicode values, scoped restoration, and concurrent access retain
  their existing contracts.

No VFP9 language behavior, xAsset format, machine-readable output, package or
debug contract, localization key, or runtime policy changed in this slice.

## Load-bearing verification

`test_platform_environment` continues to exercise value and path round trips,
Unicode, invalid names and embedded NULs, empty-value semantics, scoped
restoration, executable discovery, and concurrent process-environment access.
The new `test_platform_environment_boundary_contract` rejects platform
selection, CRT environment calls, synchronization, or the private path helper
from the public header. It also requires the private implementation and its
Windows/POSIX behavior tokens to remain registered with `cf_platform_support`.

Generated Launcher Validation builds and runs both tests on Windows, Ubuntu,
and macOS. Windows Environment and Executable Path Validation repeats them with
the adjacent environment and path consumers. Both contracts are portable,
network-free, sample-free, and use no child processes.

Local GCC Release validation builds the runtime, build, MCP, and inspection
hosts plus direct licensing, localization, security, asset, bounded-process,
and platform-model consumers. Focused, adjacent, workflow, and isolation
coverage passes `14/14`. A mutation that returned `_WIN32` selection to the
public header fails the boundary contract at the exact forbidden token, while
the unmodified source passes.

Corrected product/test head `e8f87cf42` passes all eleven protected checks.
Generated Launcher Validation run `31549944398` passes its `12/12` focused set
on Windows, Ubuntu, and macOS, including the environment behavior and boundary
contracts. Windows Environment and Executable Path Validation run
`31549944197` repeats the two contracts inside its `10/10` consumer set.
Automated review found that the original bare `setenv` contract token could be
satisfied by the substring in `unsetenv`; the corrected contract requires both
exact return statements independently. Removing only the `setenv` statement
now fails at that precise requirement.

Final evidence head `5ebe5fbeb` also passes all eleven protected checks. Its
Generated Launcher Validation run `31551072481` passes on Ubuntu and macOS;
the first Windows attempt passed both environment contracts but had an
unrelated Python-sidecar failure, and the failed-job rerun passed the complete
Windows set. Independent review at `e8f87cf42` passed the implementation,
consumer links, ASan/UBSan, and a ThreadSanitizer concurrency run. It found the
same substring-collision class in the header declaration probes: the shorter
`read_environment_variable` name could match inside
`read_environment_variable_or_empty`. The contract now requires exact
declaration prefixes for every public function, and deleting only the shorter
declaration fails at its precise requirement.

## Remaining J1 work

The environment and path interfaces are two explicit boundaries, not a blanket
portability claim. Later independently reviewed slices must inventory and
isolate the remaining Windows-only shell, printing, OLE/COM, CLR-hosting, and
other native seams while keeping portable runtime/data/parser/connector
contracts free of host-specific dependencies. J2 and J3 remain responsible for
the broader standalone IDE and core-host work on macOS and Linux.
