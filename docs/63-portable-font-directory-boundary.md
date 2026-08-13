# Portable Font-Directory Boundary

Copperfin's `AFONT()` implementation discovers installed fonts before filling
the VFP-facing array. The ordered host directories used for that discovery now
come from `copperfin::platform::font_search_directories()` in
`cf_platform_support`, rather than from operating-system branches in the PRG
interpreter.

## Preserved behavior

- Windows searches `%WINDIR%/Fonts`, with `C:\Windows\Fonts` as the existing
  fallback when `WINDIR` is unavailable.
- macOS searches `/System/Library/Fonts`, then `/Library/Fonts`, then the
  current user's `~/Library/Fonts` when `HOME` is available.
- Other POSIX hosts search `/usr/share/fonts`, then `/usr/local/share/fonts`,
  then `~/.fonts` and `~/.local/share/fonts` when `HOME` is available.
- The platform function returns the ordered candidates without testing whether
  they exist. The runtime retains existence checks and recursive enumeration.
- The runtime also retains font-extension filtering, display-name extraction,
  case-insensitive deduplication, sorting, `AFONT()` array shape and count, and
  the fixed headless fallback used when no host fonts are discovered.

The public contract exposes only standard C++ filesystem paths and a vector.
Host macros, environment-variable names, and native directory literals remain
private to the platform implementation. Environment text is converted through
the established UTF-8 path boundary before it becomes a native path.

## Verification

`test_platform_font_directories` directly checks that each host returns its
documented roots in order, that optional user roots follow environment
availability, and that no duplicate root is returned. The existing
`test_prg_engine_arrays` remains the VFP-facing `AFONT()` consumer regression.

The source boundary contract rejects host macros, environment names, and
directory literals in the portable header and PRG runtime; requires platform
ownership and the exact runtime delegation; and protects build plus hosted
scheduling. The GitHub Actions and native-workflow ledgers require the direct
behavior and ownership tests on Windows, Ubuntu, and macOS. Local GCC Release
behavior, boundary, real `AFONT()` consumer, workflow, GitHub Actions, and
isolation coverage passes `6/6`; Clang ASan/UBSan passes the same focused
selection `6/6` with no findings. Changing the first POSIX root causes the direct ordered
behavior assertion to fail, and restoration returns the selection to green.
Hosted and review evidence remains required before this slice can merge.

This bounded J1 slice does not replace the fixed no-font fallback, implement a
native font metadata provider, change `AFONT()` results, or complete the macOS
and Linux host ports.
