# Workspace-Agent Windows Process-Image Traceability Report

Date: 2026-08-15

Scope: implementation-complete candidate v1 `RQ-CF-AGENT-017` structural
Windows process-image compatibility preflight

Allowed requirement source: explicit repository-owner Windows-first product
and security policy under H3/I2; derived from `RQ-CF-AGENT-010`,
`RQ-CF-AGENT-015`, and the listed hazards

Implementation and verification:

- `include/copperfin/platform/windows_pe_image.h`
- `src/platform/windows_pe_image.cpp`
- `src/security/workspace_agent_process_containment.cpp`
- `src/runtime/managed_pe_image.cpp`
- `tests/test_windows_pe_image.cpp`
- `tests/test_workspace_agent_process_containment.cpp`
- `tests/test_prg_engine_dotnet_dispatch.cpp`
- `tests/run_windows_pe_image_boundary_contract_check.cmake`
- `docs/64-workspace-agent-access-policy.md`
- `docs/32-recovered-requirements-traceability.md`

This report records DO-178C-inspired development assurance adapted to a
general-purpose C++/.NET platform. It is not a claim of formal DO-178C
compliance, certification, an assigned software level, a proof of Windows
loader acceptance, or suitability for a safety-critical deployment.

## Derived and verification requirements

| Derived requirement | Verification requirement | Hazard link |
| --- | --- | --- |
| `DQ-workspace-agent-windows-process-image-001`: parse one direct file through bounded DOS, PE/COFF, optional, and section-header reads without mapping or execution | `DV-workspace-agent-windows-process-image-001`; `DV-workspace-agent-windows-process-image-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-process-image-002`: admit only coherent machine/magic form, bounded section tables and raw ranges, an executable entry-point section, executable-image flags, and GUI/console subsystem | `DV-workspace-agent-windows-process-image-001`; `DV-workspace-agent-windows-process-image-002`; `DV-workspace-agent-windows-process-image-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-process-image-003`: reject DLL, system, malformed, truncated, unsupported, and host-incompatible images with content-free diagnostics | `DV-workspace-agent-windows-process-image-001`; `DV-workspace-agent-windows-process-image-003`; `DV-workspace-agent-windows-process-image-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-process-image-004`: on Windows prevent writes during image parsing and require complete physical identity equality after inspection | `DV-workspace-agent-windows-process-image-002`; `DV-workspace-agent-windows-process-image-003`; `DV-workspace-agent-windows-process-image-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |
| `DQ-workspace-agent-windows-process-image-005`: preserve the existing managed/native classifier and POSIX process-target behavior without claiming parser authority or execution | `DV-workspace-agent-windows-process-image-002`; `DV-workspace-agent-windows-process-image-004` | `HZ-system-failure-01`; `HZ-data-corruption-01` |

- `DV-workspace-agent-windows-process-image-001`: portable synthetic fixtures
  prove positive x86, x64, ARM64, native, managed, GUI/console, and CLR-slot
  classification plus malformed, truncated, DLL, system, subsystem, machine,
  entry-point, and raw-range denial.
- `DV-workspace-agent-windows-process-image-002`: focused process-target and
  existing managed-PE regressions prove integration without weakening either
  boundary.
- `DV-workspace-agent-windows-process-image-003`: exact Windows execution proves
  text/shell-dispatch input denial, real test-image admission, native host
  selection, strict-versus-legacy write-sharing behavior, and post-inspection
  identity checks.
- `DV-workspace-agent-windows-process-image-004`: warning-free Release, fresh
  sanitizer, broader security/community/isolation/safety tests, protected
  Windows/Ubuntu/macOS execution, diff validation, and exact-head review are
  required before implementation evidence is complete.

## Requirement delta

- Before: Windows process-target preflight treated any direct singly linked
  regular file as executable-eligible, and argument serialization explicitly
  left executable-format compatibility unresolved.
- After: Windows target preflight additionally requires a conservative
  structurally launchable and native-host-compatible PE image, inspected without
  write sharing and followed by complete physical-identity equality.

Potential Severity If Misused: high

## Hazard, misuse, and boundary analysis

Hazards: `HZ-system-failure-01` and `HZ-data-corruption-01`.

- Shell-dispatch confusion: text scripts, batch files, command-shell wrappers,
  and arbitrary regular data fail the direct PE signature and structure gate.
- Image-kind confusion: DLL and system-image characteristics are distinct from
  process executables and fail before a target plan is returned.
- Parser bounds: every offset and length is checked against the captured file
  size; section count is bounded to ninety-six; overflow, truncation, and raw
  section ranges outside the file fail closed.
- Entry semantics: the entry point must be nonzero and fall within a section
  marked executable. GUI and console are the only admitted subsystems.
- Machine compatibility: x86 hosts admit x86; x64 hosts admit x86 and x64;
  ARM64 hosts admit ARM64. Unsupported native hosts and unverified emulation
  combinations deny rather than extrapolate from product intent.
- Mutation window: the Windows reader denies write sharing during inspection.
  After it closes, process-target containment repeats inspection and requires
  the complete storage/file/size/time/link identity to match. This is still a
  point-in-time result; launch-adjacent handle pinning remains mandatory.
- Shared parser: DECLARE managed/native classification uses the same parser but
  retains its additional CLR-directory-slot requirement. This avoids parallel
  PE structure interpretations while preserving its established failures and
  its legacy read sharing when another process has the DLL open for writing.
  The workspace-agent call path retains the default no-write-sharing mode.
- POSIX boundary: the new structural gate is used only for Windows process
  targets. Existing POSIX direct-file and execute-permission semantics remain
  unchanged.
- Argument-parser boundary: PE structure cannot establish how an arbitrary
  process parses `GetCommandLineW()`. Product-owned parser authority remains a
  required separate admission boundary.
- Execution boundary: this slice launches nothing, grants no reusable authority,
  applies no sandbox or endpoint rule, manages no descendants, and records no
  tool outcome.
- Information exposure: denial returns stable diagnostic identities without
  path, header bytes, executable content, prompt, argument, or environment data.

## Rollback

Rollback removes the shared PE parser, restores the private managed-image
reader, removes the Windows target gate, and withdraws candidate
`RQ-CF-AGENT-017` and its regressions. Rollback must not claim that an arbitrary
regular Windows file is format-compatible merely because the older preflight
accepted it.

## Verification

Current candidate evidence:

- warning-free GCC Release build of the shared parser and integrated security
  targets passes;
- focused `test_windows_pe_image`, source-contract, process-target, invocation,
  and isolated-environment verification passes `5/5`;
- broader warning-free Release security/community/isolation verification passes
  `17/17`, the safety workflow passes `1/1`, and the generated isolation
  inventory covers `382` tests;
- fresh Clang 21 ASan/UBSan with explicit leak detection passes parser,
  process-target, and isolated-environment verification `3/3` with no finding;
- portable fixtures cover the direct machine matrix, CLR classification,
  malformed/truncated bounds, DLL/system/subsystem, entry-point, and raw-range
  cases;
- `git diff --check` passes;
- exact signed/DCO implementation head `8c24c0258` passed all eleven protected
  checks: contributor sign-off run `31893305926`, Clang/GCC executable-paths
  run `31893307114`, Windows/Ubuntu/macOS generated-launcher run
  `31893307124`, Win32/x64 DECLARE run `31893307104`, Windows environment and
  executable-paths run `31893307106`, and both Socket checks;
- the first Windows generated-launcher attempt passed `33/34` before the
  existing Python sidecar exceeded its bounded candidate latency window; the
  unchanged exact-head rerun passed the complete Windows job, including the
  sidecar and the private workspace-agent regressions;
- exact-head automated review reported no major issue, the sole earlier
  actionable thread was addressed and resolved, read-only agent review found
  no defect with Linux Debug `382/382` plus current-head focused `11/11`, and
  PR `#5024` merged into `v1-development` as `75d8cf786`.

Implementation evidence is complete at merge commit `75d8cf786`.

Still required before `RQ-CF-AGENT-017` is defined:

- qualified independent human sign-off for the retained `high` risk
  classification, bound to the governing review record as required by the
  repository assurance policy.

## Review evidence

- mode: high-severity maintainer self-review; no independent final safety
  approval claimed
- reviewer: rhamenator
- verification: parser bounds, image-kind/machine/subsystem denial, Windows
  write exclusion and identity recheck, managed-classifier compatibility,
  POSIX non-regression, rollback, and traceability
- result: focused Release `5/5`, broader Release `17/17`, safety `1/1`, fresh
  sanitizer `3/3`, all eleven protected checks, DCO, both supply-chain checks,
  resolved thread-aware exact-head review, and read-only agent corroboration
  pass; the implementation merged as `75d8cf786`; qualified independent human
  sign-off remains pending before assurance closure
