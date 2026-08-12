# Portable Executable-Search Default

Project Copperfin's public executable-path interface no longer selects an
operating system. `include/copperfin/platform/executable_path.h` exposes one
portable optional query, `default_executable_search_path()`, beside the existing
invocation and running-executable resolvers. Native selection remains private
to `src/platform/executable_path.cpp` in `cf_platform_support`.

## Behavior contract

- POSIX hosts continue to obtain the implementation-defined `_CS_PATH` value
  through `confstr` when it is available, with `/bin:/usr/bin` as the existing
  compile-time fallback.
- Windows returns `std::nullopt`: Copperfin does not invent a POSIX-style
  fallback when the Windows `PATH` value is absent.
- Executable invocation resolution keeps its existing host behavior. An unset
  POSIX `PATH` uses the platform default, an explicitly empty component searches
  the current directory, and Windows retains its native path and suffix rules.
- The build host and external-process policy retain their existing POSIX-only
  call sites; only the public query name and cross-platform availability change.

No VFP9 language behavior, xAsset format, localization key, machine-readable
output, package/debug contract, or security policy changes in this slice.

## Verification

`test_platform_environment` calls the portable query on every host, requiring
no default on Windows and a system executable-search default on POSIX. Its
existing unset/empty `PATH`, Unicode, environment, and running-executable tests
remain intact. `test_security_controls` covers the external-process caller, and
the real build host links through the runtime pipeline.

`test_platform_executable_path_boundary_contract` rejects OS-selection and
native executable-discovery tokens in the public header, requires exact public
declaration prefixes, and checks that Windows, macOS, Linux, and POSIX-default
mechanics remain private and registered. Generated Launcher Validation runs the
runtime and source contracts on Windows, Ubuntu, and macOS; the focused Windows
environment/path workflow repeats them. A deliberate `_WIN32` mutation in the
public header fails at the exact forbidden token.

Local GCC Release builds the platform regression, security regression, runtime
host, and build host. Focused behavior, adjacent boundary, workflow, and
isolation coverage passes `8/8`.

At exact signed/DCO implementation head `fee6c1be7`, all eleven protected PR
checks pass. Generated Launcher Validation `31554724495` runs the boundary and
behavior coverage on Windows, Ubuntu, and macOS. Windows Environment and
Executable Path Validation `31554724462` repeats the focused contract and
regression; Windows DECLARE ABI Validation `31554724438`, GCC/Clang Executable
Path Validation `31554724484`, DCO `31554724442`, and both socket checks pass.
GitHub reports no review comments or unresolved review threads at that head.

Independent review at evidence head `12f47816c` found no defect. It confirmed
the implementation is byte-identical to `fee6c1be7`, mutation-proved that one
public declaration cannot satisfy another declaration's contract token, traced
all three current callers through their existing platform guards, and matched
the Linux query result to `getconf PATH`. A fresh ASan/UBSan build and run of
the platform regression plus build and runtime hosts was clean. The reviewer
also reran both touched workflow-text contracts successfully.

## Remaining J1 work

The path, process-environment, and executable-search public interfaces now have
explicit portable contracts. J1 still requires inventory and isolation of the
remaining shell, printing, OLE/COM, CLR-hosting, and other native seams. The
standalone macOS and Linux host ports remain J2 and J3 work.
