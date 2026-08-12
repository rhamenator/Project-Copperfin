# Portable Code-Page Boundary

The VFP-facing locale and code-page surface now delegates host discovery and
byte conversion through `copperfin::platform`. The public contract uses only
standard C++ strings, optionals, arrays, and integer code-page identifiers.
Windows SDK types and conversion calls, plus POSIX locale and `iconv` APIs,
remain private to `cf_platform_support`.

## Preserved behavior

- `CPCURRENT()`, `CPCONVERT()`, `CPDBF()`, `SYS(2006)`, `SYS(2029)`, and the
  interpreter's code-page configuration retain their established VFP-facing
  results and diagnostics.
- Windows host and OEM discovery still use the active ANSI and OEM code
  pages, with the same deterministic fallback when Windows reports zero.
- Linux and macOS still prefer the active locale codeset, then `LC_ALL`,
  `LC_CTYPE`, and `LANG`, with code page 1252 as the final fallback.
- The supported VFP code-page set and DBCS lead-byte rules remain interpreter
  policy; they are not broadened by the platform extraction.
- POSIX conversion retains the existing explicit `iconv` encoding map.
  Unsupported source or target code pages continue to fail rather than being
  guessed from locale text.

## Regression contract

`test_prg_engine_locale_code_page` directly exercises locale parsing,
fallback precedence, DBCS boundaries, identity conversion, a known
CP437-to-1252 conversion, and unsupported conversion rejection. Existing
runtime-surface coverage remains authoritative for the VFP functions.
`test_platform_code_page_boundary_contract` rejects Windows, POSIX-locale,
or `iconv` implementation tokens in the public header and PRG runtime;
requires the private implementations and exact delegations; preserves private
`Iconv::Iconv` linkage; and requires the behavior and source contract on the
Windows, Ubuntu, and macOS generated-launcher hosts. The source-only contract
is explicitly classified as portable, read-only, child-process-free,
resource-free, and parallel-safe.

The pre-change GCC Release baseline passed the locale/code-page and broad
runtime-surface tests `2/2`. After extraction, the focused locale/code-page
target builds warning-free, and the affected behavior, boundary, workflow,
and isolation selection passes `6/6`. Deliberately restoring a `GetACP`
ownership token to the interpreter and separately breaking CPCONVERT's exact
platform delegation each fail at the intended assertion; restoration returns
the selection to green. Clang ASan/UBSan passes the behavior, boundary, and
workflow selection `4/4` with no sanitizer findings. Hosted matrices and
independent review remain required before merge.

## Scope

This is a bounded J1 ownership correction. It does not add new encodings,
change VFP9 code-page policy, redesign database code-page metadata, or claim
the broader macOS/Linux host ports. OLE/COM, report rendering, and remaining
native seams remain separate.
