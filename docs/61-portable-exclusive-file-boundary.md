# Portable Exclusive-File Boundary

Copperfin's strict verified-byte runtime materializes admitted package content
into temporary snapshots before parsers and runtime consumers open it. The
runtime continues to own admission, unambiguous path selection, snapshot-root
naming, sidecar policy, cleanup, and localized diagnostics. The low-level
operation that creates and durably writes one new snapshot file now belongs to
`cf_platform_support` through
`copperfin::platform::write_new_durable_file()`.

## Preserved security contract

- An existing filesystem entry is never replaced.
- The parent directory must already exist; the primitive does not broaden the
  caller's directory authority.
- Every supplied byte must be written, file contents must be flushed, and the
  handle or descriptor must close successfully before the call reports success.
- Windows retains `CREATE_NEW`, exclusive sharing, and explicit
  `FlushFileBuffers` behavior.
- POSIX retains `O_EXCL`, `O_NOFOLLOW`, `O_CLOEXEC`, owner-only mode `0600`,
  interrupted-write retry, `fsync`, and close-result checking.

The file flush preserves the existing content-durability boundary. This API
does not claim to make the containing directory entry crash-durable, create
parent directories, select snapshot names, or remove failed snapshots.

## Verification

`test_platform_exclusive_file` directly covers binary and empty content,
existing-entry refusal with byte preservation, missing-parent and directory
rejection, POSIX `0600` mode, and POSIX symlink non-following. The source
boundary contract rejects native creation/write/flush ownership in the PRG
runtime, requires the exact portable delegation, and protects the platform
flags and hosted scheduling. The full verified-DBF security suite runs beside
the direct behavior and ownership tests in Generated Launcher Validation on
Windows, Ubuntu, and macOS.

Local GCC Release validation passes the direct behavior, ownership, complete
verified-DBF consumer, workflow, GitHub Actions, and isolation selection `6/6`.
Clang 21 ASan/UBSan validation passes the focused behavior, ownership,
workflow, GitHub Actions, and isolation selection `5/5` with no findings.
Restoring a forbidden Windows native-creation token to the runtime fails the
ownership contract, and weakening POSIX creation mode from `0600` to `0644`
fails the direct owner-only permission assertion. Both mutations were restored
before the final passing runs.

Corrected exact implementation head `2452a00ac` passes all eleven protected
checks. Generated Launcher Validation run `31653594802` executes the direct
behavior, ownership, and complete verified-DBF consumer selection on Windows,
Ubuntu, and macOS; Windows environment/path run `31653594830`, Win32/x64
DECLARE run `31653594843`, GCC/Clang executable-path run `31653594824`, DCO
run `31653593588`, and both Socket checks pass. Automated review identified one
missing direct `<string>` include in the platform implementation. Signed/DCO
commit `2452a00ac` adds it, the focused GCC and Clang sanitizer selections pass
again, and the only review thread is answered, resolved, and outdated.

This is one bounded J1 increment. Snapshot-root creation and deletion still use
standard filesystem operations in the runtime, and other native file, shell,
printing, OLE/COM, and CLR-host seams remain separate work.
