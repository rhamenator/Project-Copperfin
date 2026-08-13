# Windows VSIX Lifecycle Traceability Report

## Scope And Assurance Boundary

This report covers `RQ-CF-REL-003`: direct Windows VSIX installation,
identity/version, package-load/command, uninstall, and residue evidence. It
uses Copperfin's DO-178C-inspired general-purpose quality baseline; it is not a
claim of DO-178C compliance, certification, an assigned software level, or
suitability for a safety-critical deployment.

## Requirement And Verification Map

| Requirement | Verification | Controlled hazards |
| --- | --- | --- |
| `RQ-CF-REL-003`; `DQ-windows-vsix-lifecycle-scope` — lifecycle evidence must be direct, bounded, exact-artifact-bound, instance-scoped, and must not infer unexecuted maintenance operations | `DV-windows-vsix-lifecycle-contract`; helper and assembler self-tests; exact-head hosted Windows execution required | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01` |

Reverse traceability is carried by `.github/workflows/build-vsix.yml`,
`scripts/test-windows-vsix-lifecycle.ps1`, `scripts/assemble-rc-candidate.py`,
the schema-v3 contract, durable matrix row, RC guide, and focused contract.

## Hazard, Misuse, Boundary, And Rollback Analysis

- A VSIX that compiles can still be rejected by VSIXInstaller, target the wrong
  Visual Studio instance, register but fail to load, or leave an extension
  behind after uninstall.
- The workflow runs on a disposable hosted Windows VM and selects exactly one
  instance containing the required Core Editor component. Install and uninstall
  receive that instance ID and the exact `Copperfin.VisualStudio` identity;
  both operations use VSIXInstaller's quiet mode without restart/elevation
  behavior being inferred.
  Profile inventory derives the registry/profile major from the selected
  installation version rather than assuming one Visual Studio release.
- The authoritative hosted lifecycle lane pins GitHub's `windows-2022` image,
  matching the shipping VSSDK 17.x package toolchain and preventing the moving
  `windows-latest` alias from silently changing the release gate. Visual Studio
  18/2026 remains a separately exercised compatibility seam; it is neither
  inferred from this lane nor declared unsupported by it.
- The precondition rejects an already installed Copperfin extension. The
  verifier inventories only that selected instance and never deletes extension
  directories to manufacture a clean result.
- Lifecycle execution immediately follows package construction, before managed
  test programs run, so a test descendant cannot become a VSIXInstaller
  blocking process. MSBuild node reuse is disabled so the package-construction
  process does not remain as an installer blocker. Later managed tests consume
  the same built package only after uninstall and residue verification succeed.
- After installation and exact installed-payload verification, the workflow
  runs the selected IDE's bounded `/updateconfiguration` operation before first
  launch. This admits the VSIX-generated package registration into a fresh
  hosted profile; merely finding copied extension files is not load evidence.
- The PRG fixture, activity log, and extracted package live under one explicit
  runner-temporary evidence root. Visual Studio launches with that directory as
  its working directory, so the smoke does not rely on the source checkout.
- VSIXInstaller has a bounded wait and timed-out process-tree termination.
  Visual Studio has a bounded observation window, then receives a normal close
  request before bounded tree termination fallback.
- A same-instance DTE observation must find the exact runner-owned PRG in the
  running IDE's Documents collection before invoking the registered
  `Copperfin.ShowCommandWindow` command. A visible window or launched process
  alone is not PRG-open or command evidence. After bounded IDE shutdown flushes
  the activity log, XML entry parsing requires the exact informational
  `End package load [CopperfinPackage]` record and package GUID and rejects any
  Copperfin-related error entry. Package-name, assembly-path, or GUID substring
  mentions elsewhere in the log cannot admit lifecycle evidence.
- Same-version reinstall, previous-version upgrade/coexistence, and disablement
  remain explicit `NOT_RUN` states. They cannot be promoted by this slice.

Potential severity is **high** because false VSIX lifecycle evidence could
admit a package that destabilizes a developer's IDE or cannot be removed. No
customer project, installed VFP9 asset, credential, or user-owned data is used.

Before an RC consumes this contract, rollback is a coordinated revert of the
workflow, helper, assembler/schema, and documentation. After consumption, do
not rewrite its tag or evidence: withdraw the candidate, publish the affected
VSIX digest and failed stage, correct the package or verifier with regression
coverage, and issue the next sequential immutable candidate.

## Verification And Residual Gaps

Local helper self-test, focused contract, RC assembler self-test, and schema
validation pass. Exact-head hosted VS2022 run `31711406714` completed the direct
lifecycle and every existing VSIX/managed step at signed head `f26086a09`.
The independently downloaded producer and lifecycle result files are identical;
their VSIX SHA-256
`a02b11f77c35d798642780641b40a940b02c001105a1c64da6e4c9ebb8dc922c`
matches an independent digest of the downloaded package. Producer artifact
`9185674193` has GitHub digest
`sha256:76e659eecec432073033ce20740e0b3efe01343b331645f5782c786e37bf618a`;
diagnostic artifact `9185671767` has digest
`sha256:cc7308165fe5800a29be43e3c03063a46d0a91c7a8d5bb6cfe4fab268b844de6`;
both report expiry on 2026-11-11. Independent review found that the old helper
could emit that nominal result without proving two required relationships: a
main window did not prove the supplied PRG was an open document, and a package
GUID/assembly substring could be a registration or failed-load mention rather
than successful initialization. The corrected helper uses same-instance DTE to
observe the exact document and invoke the command, then requires the exact
informational `End package load [CopperfinPackage]` XML entry/package GUID and
rejects matching error entries. The self-test includes nominal-success and
success-plus-error records. Corrected exact-head hosted execution is pending;
`RQ-CF-REL-003` is reset to `gap`.

Failed exploratory VS18 hosted runs remain negative diagnostic evidence: the
moving `windows-latest` image installed the package but did not admit its
per-user pkgdef path into that fresh hosted profile. This does not supersede
separate direct VS18 host evidence and is not generalized into a product
compatibility claim. VSIX signing, same-version reinstall, previous-version
upgrade, disablement, and human visual review remain separate.
