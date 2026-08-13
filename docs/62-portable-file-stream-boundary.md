# Portable File-Stream Boundary

Copperfin's PRG file-I/O runtime owns VFP-facing handle allocation, path and
mode interpretation, verified-byte admission, `FERROR()` mapping, and the
semantics of `FOPEN()`, `FCHSIZE()`, and related functions. The host operations
needed to open a standard C stream for a native path and resize its underlying
file now belong to `cf_platform_support` through
`copperfin::platform::open_file_stream()` and
`copperfin::platform::resize_file_stream()`.

## Preserved behavior

- Windows continues to open Unicode native paths with `_wfopen`; POSIX
  continues to use the path's native byte spelling with `fopen`.
- The runtime still chooses and validates VFP modes, creates a missing file for
  numeric read/write modes, allocates VFP handle numbers, flushes before
  `FCHSIZE()`, maps `errno` to VFP `FERROR()` values, and closes streams.
- Windows resizing retains `_fileno`/`_chsize_s`; POSIX retains
  `fileno`/`ftruncate`.
- Sizes that cannot be represented by a native signed file offset fail with
  `EFBIG` before narrowing. Existing VFP runtime requests are already bounded
  to nonnegative signed 64-bit values.
- A stream that cannot yield a native descriptor fails with `EBADF`, so the
  runtime never maps stale `errno` state on that failure path.

The platform API exposes only standard C++ filesystem, string-view, integer,
and `std::FILE` types. It does not expose native descriptors, create parent
directories, choose sharing policy beyond the existing C mode, flush or close
the caller-owned stream, or change verified-byte security policy.

## Verification

`test_platform_file_stream` directly covers Unicode-path creation, exact binary
content, shrinking, extension with zero-filled bytes, missing-parent refusal,
null-stream failure, and native-size narrowing rejection. The existing
`test_prg_engine_file_io_functions` remains the VFP-facing consumer regression
for `FOPEN()`, `FCHSIZE()`, `FERROR()`, Unicode paths, handle lifecycle, and
read/write results.

The source boundary contract rejects `_wfopen`, `_chsize_s`, `ftruncate`, and
native descriptor ownership in the PRG runtime, requires the exact two platform
delegations, and protects build and hosted scheduling. Local GCC Release direct
behavior, ownership, PRG consumer, workflow, GitHub Actions, and isolation
coverage passes `6/6`; Clang 21 ASan/UBSan passes the focused selection `5/5`
with no findings. Restoring a forbidden native-resize token to the PRG runtime
fails the ownership contract, and substituting a successful no-op POSIX resize
fails the direct shrink and extension assertions. Both mutations were restored
before the final passing runs.

Corrected exact implementation head `0c8e61d94` passes all eleven protected
checks. Generated Launcher Validation run `31656236358` executes the direct
behavior, ownership, and complete PRG file-I/O consumer selection on Windows,
Ubuntu, and macOS; Windows environment/path run `31656236341`, Win32/x64
DECLARE run `31656236369`, GCC/Clang executable-path run `31656236350`, DCO
run `31656235157`, and both Socket checks pass. Automated review identified a
real stale-`errno` path when `_fileno` or `fileno` returns a negative descriptor;
both host branches now normalize that failure to `EBADF`, and the resolved
source contract requires exactly two complete guards. The first corrected-head
Ubuntu run then exposed a verifier-only CMake policy-baseline gap: all other 31
selected tests passed, but `while(TRUE)` did not iterate under the newer policy
state. The script now declares CMake 3.20 as its policy baseline. Local `6/6`
passes again, and changing either one of the two errno guards still makes the
counter fail at `1/2` before restoration.

This is one bounded J1 ownership correction. It does not change VFP file-I/O
syntax or return contracts, add asynchronous I/O, claim directory-entry
durability, or complete the broader macOS/Linux host ports.
