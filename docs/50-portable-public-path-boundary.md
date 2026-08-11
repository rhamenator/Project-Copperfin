# Portable Public Path Boundary

Project Copperfin's public path conversion interface is platform-neutral. The
header `include/copperfin/platform/path.h` exposes only standard C++ types and
declarations; it does not include the Windows SDK, select an operating system,
or embed native calls. The implementation belongs to
`src/platform/path.cpp` inside `cf_platform_support`.

This is the first explicit library-boundary increment for v1 roadmap lane J1.
It prevents the path abstraction's many runtime, security, licensing, asset,
Studio, and application consumers from inheriting Windows SDK declarations or
macros merely by including a portable interface. It does not make every core
header platform-neutral and does not complete the macOS or Linux ports.

## Preserved behavior

- Windows continues to convert between native UTF-16 paths and strict UTF-8
  with the same Win32 conversion APIs and failure behavior.
- Windows path-component comparison remains case-insensitive and uses the same
  ordinal, invariant-locale, and lowercasing fallbacks.
- POSIX hosts continue to convert filesystem UTF-8 bytes without locale
  dependence and compare components case-sensitively.
- Empty input continues to produce an empty result.

No VFP9 language behavior, xAsset format, machine-readable output, package or
debug contract, localization key, or runtime policy changed in this slice.

## Load-bearing verification

`test_platform_path` directly verifies UTF-8 round trips, empty values, and
the operating-system-specific component-comparison rule. The
`test_platform_path_boundary_contract` source check rejects Windows selection,
Windows SDK inclusion, or Win32 implementation tokens in the public header and
requires the private implementation to remain registered with
`cf_platform_support`.

Both tests are portable, parallel-safe, network-free, sample-free, and use no
child processes. Generated Launcher Validation builds and runs them on Windows,
Ubuntu, and macOS; Windows Environment and Executable Path Validation repeats
them beside the existing environment/path consumers. Local GCC Release focused,
adjacent-consumer, isolation, and workflow-contract validation passes `10/10`,
including licensing, security, VFP asset, and environment-path consumers. Clang
21 ASan/UBSan passes the two new tests with leak detection.

Exact corrected head `dac37ee59` passes all eleven protected checks. Generated
Launcher Validation run `31541415911` builds, links, and runs the two path
contracts as part of a `10/10` focused set on Windows, Ubuntu, and macOS.
Windows Environment and Executable Path Validation run `31541415790` repeats
both inside its `9/9` consumer set. Automated review identified that the
preserved `CharLowerBuffW` fallback's User32 dependency had remained implicit
through CMake's default Windows libraries; the corrected head declares that
dependency on `cf_platform_support`, and the source contract protects it.

Final evidence head `e0fbdd11c` passes all eleven protected checks. Generated
Launcher Validation run `31542720317` runs both path contracts on Windows,
Ubuntu, and macOS, and Windows Environment and Executable Path Validation run
`31542720276` repeats them inside its `9/9` focused set. Independent review
found no functional defect, but did identify that the move had dropped the
hard-won rationale for the layered Windows Unicode comparison fallbacks. The
original rationale is restored, with one inherited statement corrected to
match the `CharLowerBuffW` failure contract. Review also identified unchecked
narrowing into Win32's signed length type; unrepresentable component lengths
now fail closed instead of reaching the comparison APIs. The source contract
protects the length guard and key explanations alongside the implementation
and explicit User32 link dependency. Representable path behavior is unchanged.

## Remaining J1 work

This boundary is a seed, not a blanket portability claim. Later independently
reviewed slices must inventory and isolate the remaining Windows-only shell,
printing, OLE/COM, CLR-hosting, and other native seams, while keeping portable
runtime/data/parser/connector contracts free of host-specific dependencies.
J2 and J3 remain responsible for the broader standalone IDE and core-host work
on macOS and Linux.
