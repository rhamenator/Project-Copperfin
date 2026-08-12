# Portable Disk-Space Boundary

The VFP-facing `DISKSPACE()`, `SYS(2020)`, and `SYS(2022)` surfaces now obtain
available-byte and allocation-unit data through `copperfin::platform`. The
public contract uses only `std::filesystem::path`, `std::optional`, and
standard unsigned integers. Windows volume APIs and POSIX `statvfs` remain
private to `cf_platform_support`.

## Preserved behavior

- The interpreter still resolves empty, relative, backslash-separated,
  drive-letter, UNC, file, and directory inputs according to its existing VFP
  path rules before calling the platform boundary.
- `DISKSPACE()` and `SYS(2020)` still return numeric available bytes and fall
  back to numeric zero when the host query fails.
- `SYS(2022)` still returns allocation-unit bytes as character text and falls
  back to character `"0"` for a missing or unavailable path.
- Windows still resolves the supplied path to its owning volume before calling
  `GetDiskFreeSpaceW`; POSIX still reports `f_frsize` from `statvfs`.

## Regression contract

`test_platform_disk_space` directly exercises positive available-byte and
allocation-unit queries for a test-owned directory and file, confirms both
paths identify the same allocation unit, and proves missing paths fail closed.
The existing broad runtime-surface test remains authoritative for VFP return
types, default and nested-path resolution, and zero fallback.

`test_platform_disk_space_boundary_contract` rejects Windows or `statvfs`
implementation tokens and direct `std::filesystem::space` ownership in the PRG
runtime, requires the two exact platform delegations and private native
implementations, and requires behavior and boundary coverage on Windows,
Ubuntu, and macOS. Both tests have explicit complete isolation metadata.

GCC Release builds the direct platform and complete runtime-surface targets
warning-free for the changed files. Direct platform behavior, complete runtime
behavior, code-page adjacency, disk-space ownership, both workflow contracts,
and the isolation meta-contract pass `7/7`. Deliberately restoring a Windows
disk API token to the interpreter and separately replacing the allocation-unit
delegation with the available-byte query each fail at the intended boundary
assertion; restoration returns the selection to green. Clang ASan/UBSan passes
the direct behavior, disk-space boundary, workflow, and isolation selection
`5/5` without a sanitizer finding. Hosted Windows, Ubuntu, and macOS execution
and independent review remain required before merge.

## Scope

This is a bounded J1 ownership correction. It does not change VFP path policy,
drive classification, free-space precision, filesystem allocation behavior,
report printing, spool jobs, or OLE/COM integration. Those adjacent surfaces
remain separate.
