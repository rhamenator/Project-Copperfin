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
- VSIXInstaller has a separate bounded 600-second wait, timed-out process-tree
  termination, and retained structured install/uninstall timing and outcome
  diagnostics. Failure cleanup re-inventories the selected instance before
  deciding whether an exact-identity uninstall is required; a timeout cannot
  make an installed extension invisible to cleanup merely because the normal
  install-return path was not reached.
  Visual Studio has a bounded observation window, then receives a normal close
  request before bounded tree termination fallback.
- Process-scoped Windows UI Automation accepts only descendants of the exact
  IDE process launched by the helper. The helper foregrounds that exact IDE,
  verifies foreground PID ownership, sends Visual Studio's English Command
  Window shortcut, verifies foreground PID ownership again, and submits only
  the invariant `Copperfin.ShowCommandWindow` command. None of those input
  steps admits evidence by itself: the exact Copperfin Command surface and
  runner-owned PRG must be observed in that same IDE process. A visible window,
  launched process, command-line attempt, keystroke attempt, or surface
  discovery alone is not PRG-open or command evidence. After bounded IDE
  shutdown flushes
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
further-corrected verifier resolved the foreground HWND's Win32 process owner
and required the exact controlled IDE PID. Run `31732427063` passed the widened
installer boundary, completing installation in about 234 seconds, but the
second input was owned by another process. That confirms synthetic foreground
input is not a stable evidence boundary on the hosted desktop. The next
correction invokes only `devenv /Command Copperfin.ShowCommandWindow` through
Visual Studio's documented semantic command interface and proves the resulting
pane in the already controlled IDE PID. It then uses the documented `/Edit`
interface for the exact runner-owned PRG, after which process-scoped UI
Automation proves either an
exact document descendant or the exact fixture leaf followed by Visual
Studio's title separator. It then requires the exact
informational `End package load [CopperfinPackage]` XML entry/package GUID and
rejects matching error entries. The self-test includes nominal-success and
success-plus-error records. Corrected exact-head hosted execution is pending;
`RQ-CF-REL-003` is reset to `gap`.

Run `31731652063` did not exercise that foreground-owner correction: hosted
VSIX installation exceeded the 240-second process bound after prior installs
had approached 238 seconds. The default remains bounded with process-tree
termination but is widened to 300 seconds for demonstrated hosted scheduling
variation; the run is retained as installer-timeout diagnostics, not command
or lifecycle evidence.

Run `31732427063` is retained as negative command-automation evidence, not
lifecycle evidence. Its ActivityLog remains retained, but no result JSON was
admitted because the exact-process foreground check failed before pane and
document proof. Run `31733561020` then showed that the generic
`VisualStudio.DTE.17.0` active object was unavailable throughout the bounded
interval. Following Microsoft's documented launch-and-DTE pattern, the next
correction enumerates the COM Running Object Table for only the version- and
process-specific `VisualStudio.DTE.<version>:<PID>` suffix, then independently
verifies that object's main-window owner before use. Run `31734701283` proved
that the exact per-process DTE moniker was also absent. The current correction
therefore uses the documented `devenv /Command` and `/Edit` interfaces while
requiring all resulting UI evidence in the already controlled IDE PID. This
remains a gap pending fresh exact-head evidence.

Run `31736024834` proved that the documented `/Command` routing process can
remain attached to the existing IDE beyond the full 300-second process bound.
Because the verifier synchronously waited for that routing process, it never
reached the independent pane observation. The corrected sequence starts each
documented `/Command` or `/Edit` request as a distinct process, requires its PID
to differ from the controlled IDE, observes the pane or exact document in the
controlled IDE PID, and then terminates any still-attached routing process tree.
Routing-process lifetime or exit cannot itself admit evidence.

Run `31737231439` did not exercise the concurrency correction because hosted
VSIX installation exceeded the 300-second bound. The default is widened to 360
seconds within the existing validated 30–600 range; timed-out process-tree
termination remains mandatory. This run is installer scheduling diagnostics,
not command or lifecycle evidence.

Run `31738142144` completed installation and opened the controlled IDE, but the
Copperfin command surface was not observable before the bounded interval
expired. Its retained ActivityLog shows the installed Copperfin registration
paths but neither package-load completion nor a Copperfin package error. This
is negative command-readiness evidence, not lifecycle evidence. The corrected
sequence launches the controlled IDE without a queued startup command, routes
the built-in `View.CommandWindow` command from a distinct bounded process, and
requires the built-in Command Window to be observable in the controlled IDE
PID before routing `Copperfin.ShowCommandWindow`. The semantic readiness proof
replaces an unevidenced timing assumption; it does not weaken the later exact
Copperfin pane, PRG document, or package-load requirements. Run
`31739502436` then proved the second `devenv /Command` process did not route
even the built-in command into the controlled IDE, despite completed
environment and window-management package loads. The current correction
eliminates cross-process routing: the one controlled IDE receives the exact
runner-owned PRG path and `Copperfin.ShowCommandWindow` on its startup command
line, after which UI Automation must independently prove both surfaces in that
same process. Startup inputs do not themselves count as proof.

Run `31740807424` proved that single-process startup still evaluated the
Copperfin command during the same launch that regenerated the per-user PkgDef
cache. The retained ActivityLog explicitly reports that the cache was stale,
imports the installed Copperfin PkgDef during startup, and never begins loading
`CopperfinPackage`; no pane was admitted. The corrected lifecycle performs a
separate bounded registration-prime launch after `/updateconfiguration`, requests
a normal close with bounded process-tree termination as fallback, and parses
its retained ActivityLog to require an exact
case-insensitive path match for the installed Copperfin PkgDef import. Only a
subsequent clean IDE process receives the PRG input; process-scoped UI
Automation expands its Tools menu, invokes the exact `Copperfin Command` item,
and independently observes the pane and package-load evidence. A prime launch
and PkgDef import are registration evidence only; a startup command input is
not accepted as command or package-load evidence.

Run `31742224692` reached the registration-prime main window and input-idle
state, but its normal close took longer than the initial 30-second shutdown
allowance. It therefore never reached PkgDef-path verification or the evidence
launch, and the workflow's diagnostic staging initially omitted the new prime
log. Run `31742937078` retained that log and directly proved import of the exact
installed `Copperfin.VisualStudio.pkgdef`, but the hosted IDE remained alive
after a 120-second close allowance and again stopped before the evidence IDE.
The prime IDE is runner-owned, disposable registration state: the corrected
contract requests a normal close, then performs bounded process-tree
termination if necessary, and still requires the exact PkgDef-path proof before
the evidence launch. Always-run diagnostics retain both prime and evidence
ActivityLogs. These runs are process-scheduling and diagnostic-retention
evidence only, not product lifecycle results.

Run `31743866527` did not reach registration or product evidence because
`VSIXInstaller.exe` exceeded its existing 360-second bound. An unchanged-head
rerun, `31744711631`, completed installation, proved exact installed-PkgDef
import, bounded the non-exiting prime IDE with process-tree cleanup, and reached
the separate evidence IDE. Its cached extension was enabled and current, but
the startup `/Command Copperfin.ShowCommandWindow` input never began loading
`CopperfinPackage` and no pane appeared during the bounded observation. The
corrected verifier removes startup-command inference and invokes the exact
installed Tools-menu item through UI Automation in the controlled IDE process;
the resulting pane and successful package-load record remain independently
required. The run is retained negative command-activation evidence only.

Run `31745965381` exercised the process-scoped Tools-menu approach but again
did not observe the pane or any Copperfin package-load record. Its retained
ActivityLogs prove the exact installed PkgDef import and contain no matching
Copperfin error, while the initial UI Automation failure text did not identify
whether Tools, the exact command item, or its invocation pattern was absent.
The corrected observer retains separate versioned JSON state for command and
PRG observation, including each menu-discovery and invocation boundary. No
product behavior is changed until that direct diagnostic identifies the next
gap.

Run `31747101809` supplied that direct diagnostic: Visual Studio exposed its
main window in the exact evidence PID, but its top-level Tools menu was not a
discoverable UI Automation `MenuItem`; consequently the command item was never
searched in an open menu and was not invoked. The correction foregrounds that
exact IDE window, independently verifies the foreground window belongs to the
expected PID, and sends the English Visual Studio Tools accelerator once. It
then searches for and invokes only the exact `Copperfin Command` menu item by
process-scoped UI Automation. The accelerator never selects a product command,
and no keystroke is sent unless foreground ownership is proven.

Run `31748419105` proved the exact IDE foreground boundary and Tools
accelerator both executed, but the open menu still exposed no exact command
item through UI Automation. The correction no longer depends on menu-tree
exposure. After exact-PID foreground proof it sends Visual Studio's English
Command Window shortcut, independently verifies the command-input foreground
still belongs to the exact IDE PID, and submits only the invariant canonical
`Copperfin.ShowCommandWindow` command. The same-process pane, runner-owned PRG,
successful package-load record, and absence of Copperfin load errors remain
independent admission conditions. The run is retained negative activation
evidence and does not advance `RQ-CF-REL-003`.

Exact-head run `31749596542` and its unchanged-head rerun `31750359174`
both stopped at exactly the existing 360-second VSIXInstaller bound. They did
not reach registration, the evidence IDE, or canonical-command submission and
therefore say nothing about the corrected command path. They are retained as
bounded installer-scheduling evidence. The correction separates the installer
allowance from the 360-second IDE/process allowance, keeps installation and
uninstallation bounded at 600 seconds, retains a versioned installer-operation
JSON diagnostic on success or failure, and discovers any installed Copperfin
extension again during failure cleanup before deciding whether uninstall is
needed. This is a containment and evidence correction, not lifecycle success;
`RQ-CF-REL-003` remains `gap` pending fresh exact-head execution.

Run `31751652834` then completed exact-artifact installation in 337.930
seconds, exact PkgDef priming, evidence-IDE launch, and exact-identity uninstall
in 21.688 seconds. Retained digest- and instance-bound installer diagnostics
prove both operations passed. The command diagnostic proves the exact IDE was
foreground, the Command Window shortcut was sent, foreground PID ownership was
reverified, and only `Copperfin.ShowCommandWindow` was submitted. No pane or
Copperfin package-load record appeared while that sender remained alive. This
matches the earlier directly observed hosted dispatch boundary: queued input is
serviced only after the external sender exits. The corrected sequence therefore
uses one bounded process solely to submit and retain the exact input, lets that
process exit, and uses a separate bounded process to observe the pane in the
same IDE PID. Submission is not surface or package-load evidence; all later
admission conditions remain unchanged. The run is negative command-dispatch
evidence and `RQ-CF-REL-003` remains `gap`.

Run `31753021409` separated command submission from pane observation and
retained successful install/uninstall plus both input diagnostics. It still
observed no pane or Copperfin package load. The ActivityLog instead shows the
lazy Visual Studio Common IDE package loaded only after the combined Command
Window-request/command sender exited. Sender exit alone was therefore
insufficient: the canonical command had already been queued before its input
surface existed. The correction uses three separately bounded stages: request
the Command Window and exit; allow the directly demonstrated lazy initialization
a bounded ten-second settlement interval; foreground and reverify the same IDE
before submitting only the canonical command and exiting; then independently
observe the pane. Each input stage has distinct retained JSON. Neither the
settlement interval nor either input can admit lifecycle evidence.

Run `31754211038` proved that a ten-second delay after the original shortcut
sender was not itself the dispatch boundary. Both input diagnostics again
proved exact-PID foreground ownership and exact canonical submission, while
install and uninstall passed in 218.570 and 25.569 seconds. The retained
ActivityLog timestamps show the Common IDE package began loading at
`23:39:49.182`, approximately 0.19 seconds after the later canonical-command
sender exited. The later helper's foreground transition released the previously
queued shortcut; the command had already been sent before that lazy package was
ready. The correction adds an exact-PID foreground-only helper between Command
Window request and the bounded settlement interval. It emits its own retained
diagnostic, sends no product command, and exits before the later canonical
sender begins. Pane, package-load, PRG, error, uninstall, and residue admission
remain independent. This run is negative dispatch-order evidence and
`RQ-CF-REL-003` remains `gap`.

Run `31755148469` showed that an exact-PID foreground-only helper generated no
input and therefore did not release the queued shortcut. Install and uninstall
again passed (202.112 and 26.615 seconds), every retained foreground boundary
passed, and the Common IDE package again began only after the later
canonical-input sender exited. The correction replaces that ineffective
foreground-only stage with a second isolated built-in Command Window shortcut
request. It exits before the bounded settlement interval, carries no Copperfin
command, and exists only to release the first queued built-in request. The
later canonical sender and all independent admission conditions remain
unchanged. Microsoft documents `>`-prefixed examples in the Command Window, so
the exact submitted canonical syntax is retained rather than changed from
speculation.

Run `31756018725` retained PASS install and uninstall (242.154 and 25.911
seconds), exact PkgDef priming, two exact-PID built-in shortcut requests, and
exact canonical-command submission, but no pane or Copperfin package load. Its
ActivityLog timestamps show the Common IDE package began at `00:10:36.644`,
approximately 0.25 seconds after the canonical sender exited and observation
began—not during the ten-second interval following the second shortcut sender.
The evidence therefore proves each later input sender releases the preceding
queued input on this hosted desktop. The correction adds one final isolated,
exact-PID, built-in Command Window request after canonical submission and
before observation. It carries no Copperfin command, emits separate retained
JSON, and cannot admit success; pane, package-load, PRG, error, uninstall, and
residue gates remain independent. Artifact `9203013733` has digest
`sha256:243d67035af65dccaccc86686092cf23f8957f6d453c4773c6d22c9ad7adb651`
and expires 2026-11-12. `RQ-CF-REL-003` remains `gap` pending corrected
exact-head execution.

Run `31757029866` retained PASS installation in 186.447 seconds and all four
exact-PID input diagnostics. The Common IDE package began at `00:26:59.337`,
only 0.151 seconds after canonical-sender exit and while the new final dispatch
sender was already starting. The dispatch therefore raced the same lazy
surface initialization; it did not prove or activate Copperfin. The correction
adds a bounded ten-second settlement after canonical-sender exit and before the
final built-in dispatch. The run also exposed cleanup exit `2004`, which
overwrote the primary pane-observation failure. Primary and cleanup failures
are now captured separately, residue is still inventoried, and a combined
fail-closed diagnostic preserves both without claiming successful cleanup.
Artifact `9203383723` has digest
`sha256:e0d429358e14bda7040464a73ae41659dfe69c80bf16cd09f912487311aef163`
and expires 2026-11-12. `RQ-CF-REL-003` remains `gap`.

Run `31758122162` disproved the queued-input-release model. Installation and
uninstall passed (466.424 and 22.283 seconds), Common IDE completed at
`00:54:20.706`, and the final shortcut sender did not begin until
`00:54:29.224`; nevertheless `SendKeys.SendWait` blocked for approximately
203.9 seconds and no Copperfin pane or package load followed. Primary failure,
successful cleanup, and zero residue were preserved independently. The
correction replaces `SendKeys` with Win32 `SendInput`: six explicit virtual-key
events request the Command Window, Unicode key-down/key-up pairs enter only the
invariant canonical command, and Enter is an explicit down/up pair. The helper
retains expected and inserted counts and fails unless every event was inserted.
Microsoft documents both the returned inserted-event count and UIPI boundary in
[SendInput](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput),
and the Unicode foreground-queue behavior in
[KEYBDINPUT](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-keybdinput).
The verifier still proves the exact foreground PID immediately before input,
does not elevate either process, and independently requires pane, PRG,
package-load, error-free, uninstall, and residue evidence. Artifact
`9204004929` has digest
`sha256:a85b55fff25621f7410730b21200eb527cb616b2217f19d95364e7d676b83c8a`
and expires 2026-11-12. `RQ-CF-REL-003` remains `gap` pending exact-head
execution of this input correction.

Run `31759758376` proved the direct-input mechanism itself is complete but
exposed an exact command-text defect. Installation and uninstall passed
(296.502 and 21.490 seconds), all six shortcut events and all 58 requested
command/Enter events were inserted into the exact foreground IDE process, and
cleanup left zero Copperfin residue; no Copperfin pane or package load
followed. The harness had incorrectly included a literal `>` before
`Copperfin.ShowCommandWindow`. Microsoft's
[Command Window documentation](https://learn.microsoft.com/en-us/visualstudio/ide/command-window?view=visualstudio)
defines `>` as the prompt displayed at the left edge of new input lines and
instructs users to enter Command Window input without that sign. The corrected
helper therefore injected only the invariant command plus Enter, and its
focused contract required that prompt-free assignment and rejected the old
prefixed form. Pane, package-load, PRG, error, uninstall, and residue gates
remain independent. Diagnostic artifact `9204474322` has digest
`sha256:5675ea5273edace48be9d22b950fa5168e40b9ab4667ba52a191b9a42e023a53`
and expires 2026-11-12. `RQ-CF-REL-003` remains `gap` pending corrected
exact-head execution.

Run `31761029410` confirmed that removing the displayed prompt corrected the
retained command event count from 58 to 56, but did not make external desktop
input a reliable semantic command boundary. All 56 prompt-free command/Enter
events and all shortcut events were inserted into the exact foreground IDE;
the final sender still remained attached for approximately 204 seconds, and
no Copperfin pane or package load followed. Installation and uninstall passed
(203.984 and 26.464 seconds) and cleanup left zero product residue. Diagnostic
artifact `9204899781` has digest
`sha256:9cf4476e59d6b402898a3ca31e0104cc997d4604091f79acd20688a444b4e13e`
and expires 2026-11-12.

The current correction retires synthetic desktop input as the hosted command
activation mechanism. The workflow builds and installs a separate test-only
lifecycle-driver VSIX alongside the unchanged product VSIX. The driver is an
`AsyncPackage` and is inert unless the controlled evidence process supplies
the runner-owned result, PRG, and solution paths. In that process it opens the
exact temporary PRG
through the in-process DTE service and posts the exact public Copperfin command
group/ID through `IVsUIShell.PostExecCommand`. It retains a versioned dispatch
result with the command identity and HRESULT. The outer verifier still admits
nothing until same-process UI Automation proves the pane and exact PRG,
ActivityLog proves an error-free successful `CopperfinPackage` load, both
VSIXInstaller uninstall operations pass, and both extension identities have
zero residue. The driver is never included in the shipping VSIX or release
artifact. Microsoft documents background `AsyncPackage` autoload and the
asynchronous in-process command contract in
[Use AsyncPackage to load VSPackages in the background](https://learn.microsoft.com/en-us/visualstudio/extensibility/how-to-use-asyncpackage-to-load-vspackages-in-the-background?view=visualstudio)
and
[IVsUIShell.PostExecCommand](https://learn.microsoft.com/en-us/dotnet/api/microsoft.visualstudio.shell.interop.ivsuishell.postexeccommand?view=visualstudiosdk-2022).
`RQ-CF-REL-003` remains `gap` pending exact-head hosted execution.

Run `31762614594` stopped during the test-only driver build before any
installation or IDE mutation because `OLECMDEXECOPT` lacked its explicit
`Microsoft.VisualStudio.OLE.Interop` namespace import. The corrected source and
focused contract now carry that VSSDK type boundary. This run is compile
diagnostic evidence only; it does not alter the lifecycle status.

Run `31762774763` compiled the corrected net472 driver successfully, then
stopped during VSIX packaging because the minimal project had not declared the
VSSDK `TemplateOutputDirectory` required by its template-manifest target. The
driver deploys no templates; the corrected project explicitly sets
`DeployVSTemplates=false` and a deterministic intermediate template-output
directory, matching the shipping VSIX project's package boundary. No
installation or IDE mutation occurred.

Run `31762937993` built and packaged both VSIXes, installed the product and
test-only driver, and proved exact PkgDef import for each. The driver did not
autoload in the no-solution startup context, so it retained no dispatch result;
the harness then uninstalled both extensions and left zero identity residue.
This is negative activation evidence, not product lifecycle evidence.
Microsoft documents that `UIContextGuids80.SolutionExists` becomes active when
a solution is loaded or created, including an empty solution. The next harness
attempt therefore created and opened an exact runner-owned empty solution and
registered the driver for that documented context. Before opening the PRG or posting the
product command, it independently compares the in-process DTE solution path to
the expected runner-owned path and retains the verified identity in its JSON
result. The solution, driver, and result remain test-only and outside the
shipping VSIX. Diagnostic artifact `9205428325` has digest
`sha256:b09b07874cd2dc368cdef7433edad210209eb0928df477eb1bb865aae3ebce9f`
and expires 2026-11-12. `RQ-CF-REL-003` remains `gap` pending corrected
exact-head hosted execution.

Run `31763800302` built and packaged the corrected driver, installed both
VSIXes (218.350 and 85.203 seconds), loaded the runner-owned solution, and
again retained no driver result: this Visual Studio host did not request the
driver package through either well-known autoload context. Both uninstalls
passed (22.040 and 31.477 seconds) with zero residue. The diagnostic artifact
`9205723106` has digest
`sha256:2d9f5480c7a9120787faac578a466041a64075b9720cff6a02a829e30012845e`
and expires 2026-11-12.

The current correction removes ambient autoload as the driver trigger. The
test-only driver declares a canonical activation command in its own
compiled command table, and the controlled evidence IDE starts with
`/Command Copperfin.LifecycleDriver.Activate`. Microsoft documents that
`/Command` executes the named command after IDE startup; command resolution
therefore semantically requests the owning driver package. Initialization
registers the command handler, verifies the exact runner-owned solution, opens
the PRG, and posts the exact public product command before the no-op driver
command completes. The driver command, package, and solution remain test-only;
all independent product and cleanup gates remain unchanged.

Run `31764541030` stopped while compiling that test-only command registration
because the minimal net472 project did not explicitly reference
`System.Design`, which defines the inherited menu-command service API. The
project and focused contract now carry that framework reference. No VSIX was
installed and no lifecycle mutation occurred in this compile-only run.

Run `31764703993` compiled and packaged the command-triggered driver, installed
both extensions (195.784 and 55.465 seconds), and loaded the evidence IDE, but
the command marked `DefaultInvisible` was not resolved by startup `/Command`;
the driver package did not load or retain a result. Both uninstalls passed
(22.532 and 31.768 seconds) with zero residue. The corrected test-only command
remains scoped to the separately installed driver VSIX but is no longer hidden,
so Visual Studio can resolve it semantically during the controlled run. The
driver is still absent from every shipping artifact. Diagnostic artifact
`9206005192` has digest
`sha256:c909f3f5e022f7576418afe0653c0d1be1491dcefb47d7baed7f4e950140e968`
and expires 2026-11-12.

Run `31765250426` proved command visibility was not the governing boundary. It
installed both extensions, imported both exact pkgdefs, opened the evidence IDE,
and still never requested the driver package. Both uninstall operations passed
and left zero residue. Diagnostic artifact `9206231192` has installer-operation
evidence bound to product SHA-256
`15bd058aee8f36fea9dfeaacaadd3e41ae9a0b9216d5cead5af7b5fd52878e98`.
The corrected design removes the test VSIX and its command-cache dependency.
The evidence IDE starts in COM-automation `/Embedding` mode while retaining
`/Log`. A transient Windows PowerShell helper enumerates its Running Object
Table entry and accepts only the DTE object whose moniker ends in the exact
launched IDE PID. It
then opens and verifies the runner-owned solution path, opens the exact PRG
through the installed editor association (which semantically requests the
product package), and raises the exact public Copperfin command GUID/ID. A
versioned JSON result binds PID, paths, command identity, and completion.
Independent same-process UI Automation, ActivityLog package-load/error checks,
uninstall, and zero-residue gates remain mandatory. `RQ-CF-REL-003` remains
`gap` pending corrected exact-head hosted execution.

Failed exploratory VS18 hosted runs remain negative diagnostic evidence: the
moving `windows-latest` image installed the package but did not admit its
per-user pkgdef path into that fresh hosted profile. This does not supersede
separate direct VS18 host evidence and is not generalized into a product
compatibility claim. VSIX signing, same-version reinstall, previous-version
upgrade, disablement, and human visual review remain separate.
