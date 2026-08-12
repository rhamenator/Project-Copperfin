# Private SQLite Native API Boundary

The raw SQLite C ABI selection used by Copperfin's read-only relational
connector is an implementation detail, not a Copperfin public interface.
`src/platform/sqlite_api.h` now owns that detail beside the connector. No file
under `include/copperfin/` includes SQLite headers, publishes Windows import
syntax, or selects an operating system.

## Preserved behavior

- Windows 10 and newer still link the operating-system `winsqlite3` library.
  SDKs with `winsqlite3.h` use it; SDK layouts without that optional header use
  the same narrow public-domain ABI declarations as before.
- Linux and macOS still use the discovered system SQLite3 development package.
- `COPPERFIN_REQUIRE_SQLITE_CONNECTOR=ON` still fails configuration when the
  required native library is unavailable.
- The connector's read-only policy, resource ceilings, typed result JSON,
  physical-file checks, audit events, runtime-host CLI, and unavailable adapter
  are unchanged.
- The two integration tests receive the private source include root only so
  they can create local synthetic fixture databases through the same native
  ABI. That include root is not propagated by `cf_sqlite_connector`.

## Regression contract

`test_platform_sqlite_api_boundary_contract` scans every file in Copperfin's
public include tree and rejects OS-selection macros, native SQLite headers, and
Windows import syntax. It requires the exact private Windows-header,
Windows-fallback, and POSIX-header branches, requires exactly the three intended
source/test consumers, and checks exact private include-root wiring for the
connector and two tests.

Generated Launcher Validation runs this source contract beside the real
connector and runtime-host integration tests on Windows, Ubuntu, and macOS.
Local GCC Release builds both tests and the runtime host, and focused behavior,
workflow, boundary, and isolation coverage passes `6/6`. Deliberately restoring
the public shim fails at the public-tree boundary; changing only the private
Windows fallback selector fails at its exact required token.
Clang 21 ASan/UBSan with leak detection also builds and passes the real
connector regression plus the boundary contract `2/2`.
An alternate-suffix `.hpp` public leak and a fourth private-shim consumer are
also rejected, proving the tree-wide scan and closed consumer count are
load-bearing rather than dependent on today's filenames.

At exact signed/DCO corrected head `52b9b5c65`, all eleven protected PR
checks pass. Generated Launcher Validation `31559462483` builds and runs the
real connector, runtime-host integration, and boundary contract on Windows,
Ubuntu, and macOS. Windows DECLARE ABI Validation `31559462461`, Windows
Environment and Executable Path Validation `31559462418`, GCC/Clang Executable
Path Validation `31559462383`, DCO `31559461132`, and both socket checks pass.
GitHub reports no review comments or unresolved review threads at that head.

## Scope

This is a pure J1 ownership/boundary correction. It does not change the SQLite
provider, add another database, alter VFP9 behavior or machine-readable output,
or claim completion of the remaining shell, printing, OLE/COM, CLR-hosting,
macOS-host, or Linux-host work.
