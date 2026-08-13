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
- Process-scoped Windows UI Automation accepts only descendants of the exact
  IDE process launched by the helper. The controlled IDE opens Visual Studio's
  built-in Command Window through startup `/Command View.CommandWindow`. After
  a parent-side bounded interval, the input helper requires the foreground
  window to be owned by that exact IDE process and enters only the invariant
  `Copperfin.ShowCommandWindow` pane-show
  command. Hosted Visual Studio may defer the startup command until that sender
  exits, so the parent waits within another bound and repeats only that same
  idempotent command. Neither attempt admits evidence by itself; the helper
  finds the exact Copperfin Command
  surface in that same IDE process, and explicitly forwards `/Edit` plus the
  runner-owned fixture to the now-running IDE. A visible window, launched
  process, command-line attempt, keystroke attempt, or surface discovery alone
  is not PRG-open or command evidence. After bounded IDE shutdown flushes
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
than successful initialization. The first corrected hosted attempt
(`31713227592`) timed out during installation and its exact-head retry
(`31713867127`) reached the observation step but proved that generic DTE ProgID
lookup could not reliably discover the launched IDE. Process-specific Running
Object Table run `31715126080` then proved that the hosted IDE published no
matching DTE moniker at all. None of these failures is admissible lifecycle
evidence. UI Automation run `31716407669` did not find an exact-named document
descendant, but that verifier excluded the process root where Visual Studio
can expose an active-document title; its result is inconclusive and its command
boundary was never reached. Run `31717503945` added the root-title check and
proved something stronger: the process title remained exactly `Microsoft
Visual Studio`, no fixture-named descendant existed, and the command boundary
was never reached. Passing the PRG as initial startup input therefore did not
open it on this hosted profile. Run `31718662961` launched the controlled IDE
bare and requested `/Command` from a second `devenv` process. The controlled
process remained responsive but exposed no Copperfin Command surface; its
ActivityLog recorded Copperfin pkgdef import but no `CopperfinPackage` load.
That run is diagnostic evidence, not lifecycle evidence. Run `31719897147`
started the controlled IDE itself with `/Command` and proved the same negative
boundary: no Copperfin Command surface and no `CopperfinPackage` load. Run
`31721057446` found no invocable exact item through direct UI Automation
menu-tree expansion, but its failure diagnostic did not distinguish an
inaccessible Tools menu from an absent extension item. Run `31722176750`
focused the exact IDE and opened Tools through its normal English access key,
but found zero same-process UI Automation `MenuItem` elements. That is retained
as a hosted UI Automation limitation, not evidence that the Copperfin command
is absent. Run `31723497158` then proved `Ctrl+Alt+A` started loading Visual
Studio's Common IDE package, but the helper entered the Copperfin command before
that package and the Command window finished loading; no Copperfin surface or
package load followed. Run `31724537954` confirmed that the Command Window is
not exposed as a UI Automation descendant on this hosted image. The
next run `31725371043` proved the ActivityLog is not flushed while the IDE is
running, so its load record cannot serve as a live readiness signal. The
next run `31726161585` proved a three-second in-helper delay still raced the
queued shortcut and Common IDE package load. Run `31727207629` then proved
that even two shortcuts and a ten-second in-helper delay were serviced only
after the external helper exited. Run `31728514403` proved that separating the
senders inside the same lifecycle process did not change that dispatch boundary:
the Common IDE package again began loading only after final command input, so
the Copperfin pane remained absent. Run `31729613650` proved the IDE-owned
startup command was likewise deferred until the first invariant input sender
exited; the Common IDE package then loaded successfully, but that first input
was already lost. Run `31730718257` reached the second attempt but exposed a
verification gap: exact-main-HWND equality could not distinguish an unrelated
foreground window from a same-process Visual Studio owned tool window. The
further-corrected verifier resolves the foreground HWND's Win32 process owner,
requires the exact controlled IDE PID, and repeats only the same idempotent
`Copperfin.ShowCommandWindow` pane-show command, and
proves the resulting pane in that process. It explicitly
forwards `/Edit` plus the fixture to the running IDE and uses process-scoped UI
Automation to prove either an exact document descendant or the exact fixture
leaf followed by Visual Studio's title separator. It then requires the exact
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
