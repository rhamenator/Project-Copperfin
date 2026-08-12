# Portable File-Version Boundary

`AGETFILEVERSION()` now reads file-version metadata through the standard-C++
`copperfin::platform::read_file_version_metadata()` contract. The PRG
interpreter retains the VFP-facing seven-row array, verified-snapshot admission,
missing-file behavior, and localized failure path, but it no longer owns
Windows version-resource APIs, native records, or UTF conversion.

## Preserved behavior

- Existing files without readable version resources return seven stable rows:
  `0.0.0.0`, the file name, an empty company, `0.0.0.0`, an empty product,
  `0.0.0.0`, and empty trademark/copyright text.
- On Windows, the private implementation queries the first declared resource
  translation through Unicode version APIs and preserves the prior field and
  legal-text preference order.
- On macOS and Linux, the private implementation retains the bounded PE
  UTF-16LE resource-string fallback used for Windows binaries such as VFP9.
- Security-enabled sessions still materialize and inspect only the immutable
  admitted snapshot. The logical pathname cannot redirect metadata extraction
  to later filesystem content.
- The public record contains only standard C++ path and string types. It does
  not expose Windows SDK types, handles, buffers, or error codes.

## Regression contract

`test_prg_engine_arrays` remains the behavior authority for the seven-row
shape, plain-file fallbacks, real Windows resource extraction, mounted VFP9
resource extraction when available, missing files, and strict admitted-byte
behavior. `test_platform_file_version_boundary_contract` rejects native
version-resource ownership in the public header or interpreter, requires the
private Windows and POSIX implementations, preserves private `version` library
linkage, and requires all three generated-launcher hosts to schedule the
contract.

The focused GCC Release selection passes `5/5`: the new boundary, existing path
boundary, AGETFILEVERSION behavior, GitHub Actions workflow contract, and native
platform workflow contract. Deliberately restoring a `GetFileVersionInfo`
token to the interpreter and separately breaking the exact portable delegation
both fail at their intended assertions; restoration returns the selection to
green. Clang ASan/UBSan passes the affected behavior, boundary, and workflow
selection `4/4` with no sanitizer findings. Exact-head Windows, macOS, and
Ubuntu hosted validation remains required before merge.

## Scope

This is a bounded J1 ownership correction. It does not change the VFP-visible
array schema, add non-Windows executable metadata formats, unify the security
subsystem's separate executable-signature version check, complete OLE/COM or
report-printing seams, or claim the J2/J3 host ports.
