# Runtime, Build, And Debug Pipeline

Copperfin now has a first real runtime/build/debug pipeline slice instead of only design-time inspection.

The ReportListener bridge now supplies the VFP9 default `HadError` state, a built-in `DODEFAULT()` result for an inherited `Init` with no source-defined base method, existing-file resolution for `GetConfigTable()` through `OutputConfig.dbf` or `_ReportOutputConfig.dbf`, supported-schema validation through `VerifyConfigTable()`, and portable five-field creation through `CreateConfigTable()` under #3217/#4717/#4718/#4719/#4739. In strict sessions, #4720 routes lookup and schema validation through admitted DBF bytes and verified memo sidecars, rejecting unadmitted or ambiguous physical entries; `CreateConfigTable()` also rejects unadmitted writes. Ordinary sessions retain filesystem behavior. Verification accepts the shared DBF shape used by VFPSource (`OBJTYPE`, `OBJCODE`, `OBJNAME`, `OBJVALUE`, and `OBJINFO`), reports deterministic valid/missing/unsupported results, and updates `HadError`. The PRG parser also recognizes VFPSource's bare `NULL` sentinels, so the packaged ReportOutput sample now completes in normal and debugger-host modes. Error-object parity and complete report configuration behavior remain separate.

Strict database lifecycle and local DBF reads now use exact normalized verified-byte keys on POSIX under #3217/#4726. A differently cased admitted entry cannot satisfy a distinct case-sensitive path, while Windows retains case-insensitive verified matching and ordinary VFP filesystem lookup remains case-insensitive. Missing or mismatched strict components continue to use the invariant verified-bytes-unavailable diagnostic.

Strict generic and database-component readers now reject ambiguous case-folded
admitted-byte matches under #3866/#4770 instead of selecting an arbitrary
override. Exact keys remain authoritative, unique case-folded matches preserve
the existing Windows/VFP compatibility behavior, and ordinary non-strict
filesystem reads are unchanged. At product head `f9f44a786`, strict table
snapshot materialization and cursor-order inspection also use that helper;
the focused verified-file, database-lifecycle, and runtime-surface tests pass
`3/3` on POSIX. Exact-head Windows validation seq778 also passes those three
targets `3/3`, including the previously failing folded-only CDX collision;
macOS/full-matrix validation remains separate release evidence.

Strict PRG and header source-text overrides now use the same exact-key,
unique-folded, fail-closed-on-ambiguity rule under #3866/#4772. The rule covers
direct program loading, `#INCLUDE` expansion, and strict DO admission probes;
non-strict source parsing retains its existing filesystem behavior. Product
head `4957a62c3` passes the focused PRG, dynamic-xAsset security, and runtime
surface targets locally; exact-head hosted review remains pending.

The modeled `WM_KEYDOWN` path now supplies the missing headless default-action
boundary under #3217/#4771: ENTER selects the owning Form's deterministic
`CommandButton.Default` candidate, while ESC selects `CommandButton.Cancel`,
after `KeyPress` permits default processing. A selected ordinary no-argument
`Click` handler runs through the heap-backed frame machine; missing handlers,
`NODEFAULT`, event-loop restoration, and invariant click telemetry retain their
existing contracts. Focus traversal, keyboard buffering, toolbar arbitration,
pixel UI, and hosted Windows behavior remain separate.

#4771's focused implementation is closed after Linux, Windows, Claude, and
macOS Native Validation `30353802078` corroboration at product head
`f9f44a786`. Full RC matrix coverage, focus traversal, keyboard buffering,
toolbar arbitration, and hosted pixel UI remain separate contracts.

Packaged PRG startup now discovers literal, non-dynamic `DO <program>` dependencies from staged PRG sources, preserves VFP case-insensitive path resolution, stages only project-contained `.PRG`/`.MPR` targets, and recursively scans admitted targets under #110/#4714. The parser also accepts VFP9's `PROC name` abbreviation as a procedure declaration, so same-file `DO` calls in corpus sources such as ReportOutput resolve through the ordinary stack-frugal frame path under #3217/#4715. VFP's indirect `STORE ... TO ([NAME])` form now expands a defined target macro before assignment under #3217/#4716. Parenthesized dynamic targets such as `DO (cTarget)` and `DO (&cTargetHolder)` now evaluate through the same heap-backed iterative frame machine, preserving `WITH` arguments and deterministic missing-target behavior under #3217/#4722. Package dependency discovery remains literal-only; arbitrary external paths, dynamic xAssets, and control-flow forms remain separate runtime boundaries.

Current native components:

- `copperfin_build_host.exe`
  - opens a `PJX/PJT` project
  - derives a runtime package plan from the project workspace
  - stages project assets into a package content tree
  - emits a runtime manifest and a debug manifest
  - bundles the native Copperfin runtime host
  - can generate and publish a `.NET` launcher executable beside the native host
- `copperfin_runtime_host.exe`
  - reads the Copperfin runtime manifest
  - executes `PRG` startup code through a native xBase runtime session
  - supports real breakpoints plus `step`, `next`, and `out` debugger actions for `PRG` execution
  - now emits structured-enough pause-state text for shared debugger panes, including call stack, locals, globals, and runtime events
  - now also bootstraps runnable `SCX/VCX/MNX` startup behavior through generated `PRG` wrappers
  - now also boots `FRX/LBX` startup assets into direct preview/event-loop mode
  - now loads full executable xAsset tables instead of only the small Studio preview slice
  - now treats `ACTIVATE MENU` and `ACTIVATE POPUP` as event-loop operations in the native runtime
  - now supports runtime action dispatch commands such as `select:<action-id>` and `invoke:<action-id>` while paused in a waiting xAsset
  - still falls back to compatibility-mode launch reporting for non-runnable xAssets and other non-`PRG` startup assets

Current package layout:

- `app.cfmanifest`
  - runtime-facing manifest
  - points at staged package content
- `app.cfdebug`
  - source/debug-facing manifest
  - points at original source assets for IDE/debug workflows
- `content/`
  - staged `PRG`, `SCX`, `VCX`, `FRX`, `LBX`, `MNX`, and related project assets
  - now also stages memo/index sidecars needed for packaged runtime fidelity, such as `SCT`, `VCT`, `FRT`, `LBT`, `MNT`, `PJT`, `FPT`, and structural index companions when present
- `copperfin_runtime_host.exe`
  - native runtime/debug launch host
- optional generated launcher
  - configured public `.NET` apphost published beside the native runtime host
  - stable internal runtime sidecars `Copperfin.GeneratedLauncher.dll`, `Copperfin.GeneratedLauncher.deps.json`, and `Copperfin.GeneratedLauncher.runtimeconfig.json`
  - optional `Copperfin.GeneratedLauncher.pdb` classified separately as debug metadata
  - both manifests record package-relative `launcher_artifact` provenance and SHA-256 digests; these lines are inventory, not a claim that the post-launch runtime manifest protects files that execute before it is read
  - the additive `app.cftrust`/`app.cftrust.sig` Windows trust contract is defined in `docs/29-package-trust-contract.md`; the launcher guard now verifies present trust sidecars before managed startup, and release builds must enable `COPPERFIN_ENFORCE_LAUNCHER_TRUST=ON` with an out-of-tree approved public-key registry before claiming the Windows fail-closed boundary

Current CLI flow:

```powershell
$env:COPPERFIN_ROOT = (Get-Location).Path
& "$env:COPPERFIN_ROOT\build\Release\copperfin_build_host.exe" build `
  --project "C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\solution.pjx" `
  --output-dir "$env:COPPERFIN_ROOT\artifacts\runtime-smoke" `
  --configuration debug `
  --enable-security `
  --emit-dotnet-launcher
```

```powershell
& "$env:COPPERFIN_ROOT\artifacts\runtime-smoke\SOLUTION\SOLUTION.exe" --debug
```

```powershell
& "$env:COPPERFIN_ROOT\artifacts\runtime-smoke\SOLUTION\copperfin_runtime_host.exe" `
  --manifest "$env:COPPERFIN_ROOT\artifacts\runtime-smoke\SOLUTION\app.cfmanifest" `
  --debug `
  --breakpoint 12 `
  --debug-command continue `
  --debug-command step `
  --debug-command out
```

```powershell
& "$env:COPPERFIN_ROOT\build\Release\copperfin_runtime_host.exe" `
  --manifest "$env:COPPERFIN_ROOT\artifacts\menu-debug-smoke\app.cfmanifest" `
  --debug `
  --debug-command continue `
  --debug-command select:shortcut.item1 `
  --debug-command select:shortcut.item3 `
  --debug-command select:thisitemha.item3
```

```powershell
& "$env:COPPERFIN_ROOT\build\Release\copperfin_runtime_host.exe" `
  --manifest "$env:COPPERFIN_ROOT\artifacts\xasset-debug-smoke\app.cfmanifest" `
  --debug `
  --debug-command continue `
  --debug-command break:add-action:frmbooks.release `
  --debug-command invoke:frmbooks.release
```

```powershell
& "$env:COPPERFIN_ROOT\build\Release\copperfin_runtime_host.exe" `
  --manifest "$env:COPPERFIN_ROOT\artifacts\report-debug-smoke\app.cfmanifest" `
  --debug `
  --debug-command continue
```

## MVP Recovery Walkthrough

Use this walkthrough for the release evidence ledger before tagging an MVP build. It is intentionally based on the existing package, debug, localization, and native test contracts; it does not replace the full platform validation matrix.

1. Build or select a release tree, then run the focused recovery contract set:

   ```powershell
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCOPPERFIN_BUILD_TESTS=ON
   cmake --build build --parallel 2
   ctest --test-dir build --output-on-failure --timeout 180 `
     -R "^(test_runtime_pipeline|test_runtime_host_debug_output_formatting|test_prg_engine_debugger|test_localization)$"
   ```

   Record the commit, platform, test count, failures, and skipped tests in the release issue. A passing run must cover package finalization and cleanup, structured pause output, debugger recovery commands, and localized diagnostic routing.

2. Inspect both package contracts before starting the runtime:

   ```powershell
   $package = "$env:COPPERFIN_ROOT\artifacts\runtime-smoke\SOLUTION"
   $runtime = Get-Content "$package\app.cfmanifest"
   $debug = Get-Content "$package\app.cfdebug"
   $runtime | Select-String '^(manifest_version|content_root|launcher_artifact)='
   $debug | Select-String '^(debug_manifest_version|source_root|launcher_artifact)='
   ```

   These manifests are line-based `key=value` contracts, not JSON. Confirm that `app.cfmanifest` points only to staged package content and that `app.cfdebug` retains source-side paths. Do not copy source paths into the runtime manifest or treat localized display text as a contract field.

3. Reproduce a paused runtime and recover it in order:

   ```powershell
   & "$package\copperfin_runtime_host.exe" `
     --manifest "$package\app.cfmanifest" `
     --debug `
     --breakpoint 12 `
     --debug-command continue `
     --debug-command step `
     --debug-command out
   ```

   Capture the pause state, call-stack/local-variable output, runtime event records, exit code, and final process state. A fault or rejected command must remain a structured runtime/debug result; the operator must not resume blindly after the runtime host has exited. For a resumable PRG fault, the host must retain the pause state and accept subsequent watch/breakpoint/continue commands in the same process, returning to the event loop or completion after recovery (#4623/#4625/#4626/#4627). Executable SCX/VCX/MNX action faults and FRX/LBX preview-method faults are covered by the same structured boundary; native OS-level access violations remain outside the C++ exception contract.

4. Verify the recovery and localization boundaries independently:

   - Confirm failed package publication removes only its owned temporary outputs and leaves unrelated `runtime-temp` content untouched.
   - Run the focused contract set once with its default `en-US` catalog, then run the locale-specific localization target with `COPPERFIN_LOCALE=es-419`, `pt-BR`, and `qps-ploc` where the host supports environment selection. Do not apply a non-English process-wide override to the entire recovery set: several runtime-host tests intentionally assert invariant `en-US` baseline text while separately checking localized catalogs. Record human diagnostic language separately from invariant codes, JSON keys, and debugger command names.
   - Archive the command transcript, manifest excerpts, test output, package/VSIX artifact names, and the reviewer sign-off with the DQ/DV/HZ issue ledger.

This walkthrough is evidence for `DQ-MVP-release-4403-runtime-recovery`, `DQ-MVP-release-4403-localized-operator-guidance`, `DV-MVP-release-4403-cross-platform-validation`, and `DV-MVP-release-4403-recovery-walkthrough`; it does not satisfy `DV-MVP-release-4403-independent-review` by itself.

Current behavior:

- runtime packaging is `Windows-first`
- generated packages are `x64`
- `.NET` launchers are emitted as `net8.0-windows` executables
- generated-launcher publication ignores ambient MSBuild response files and inherited `Directory.Build.props` / `Directory.Build.targets`, then finalization rejects missing, ambiguous, redirected, non-regular, or unexpected internal launcher artifacts
- packaged runtime manifests now point at staged package content instead of stale legacy source paths
- debug manifests keep source-side paths so Visual Studio and the standalone Studio shell can debug against the editable source tree
- `PRG` startup paths now advertise real breakpoint and step-debugging support in the debug manifest
- the shared Visual Studio and standalone Studio project shells can now build a project and surface a first integrated debugger pane on top of the native runtime host
- runnable `SCX/VCX` startup assets can now be executed through generated method bootstraps from both source trees and packaged content
- runnable `FRX/LBX` startup assets can now be executed as direct preview/event-loop surfaces
- runnable `MNX/MNT` startup assets can now execute setup code and activate shortcut/menu event loops through a dedicated menu bootstrap model
- waiting menu runtimes can now dispatch concrete menu-item actions back into the native runtime
- deeper menu trees now participate in runtime execution because xAsset bootstrapping reads the full `MNX` table instead of the eight-record Studio preview
- waiting form/class/report/label xAssets now expose extracted methods as dispatchable runtime actions for debugger-driven invocation
- `REPORT FORM` and `LABEL FORM` text output now evaluates non-deleted layout-object `EXPR` values per qualifying cursor record and appends them as optional `object_exprs=` row metadata, while preserving existing row/filter output
- xAsset-backed breakpoints now surface designer-facing action ids and titles both in explicit breakpoint inventories and in ordinary pause-state breakpoint listings, so debugger clients can identify active SCX/VCX/MNX breakpoints without reverse-mapping generated bootstrap file/line pairs
- `DO FORM` now resolves quoted/space-containing paths through the same normalized asset-path flow used by other surface-launch commands
- startup assets that legacy projects mark as excluded are now still staged when they are required for runtime startup
- packaged xAsset startup paths now carry their memo sidecars forward so the bootstrap runtime can open real designer assets instead of dead table shells
- PRG-style execution currently uses a heap-backed frame stack inside the native runtime session rather than recursive host-stack growth, with a tested `MAXCALLDEPTH` guardrail so parity work does not recreate the stack-overflow failure profile of the original `VFP.exe`

Current limitations:

- the native execution engine is `PRG-first`, not yet the full FoxPro/VFP command/runtime surface
- expression-level user-defined routine calls and native `DEFINE CLASS` / `ENDDEFINE` support are implemented through the iterative frame machine; future parity work must preserve that stack-frugal execution model instead of routing through unbounded native recursion
- xBase code embedded in `SCX/VCX` assets is now partially executable through `METHODS` bootstrapping, but deeper event/lifecycle fidelity still needs work
- `MNX` startup activation plus first nested submenu dispatch now work, but richer menu navigation, broader command routing, and broader surface parity still need work
- `FRX/LBX` now launch directly into preview/event-loop mode, and text output evaluates layout-object expressions, but pagination, printer/device output, memo formatting, grouping/summary evaluation, report variables, event expressions, richer output generation, and designer/runtime parity still need work
- package manifests are line-based metadata, not the finished long-term runtime format
- build output planning is still driven by current `PJX` heuristics rather than a fully compatible FoxPro compiler/runtime

What this unlocks next:

- Visual Studio build/run/debug commands can target a real package pipeline
- the standalone Studio shell can run the same build/package flow and reuse the same debugger surface
- the future Copperfin runtime can take over the same manifest/package contract without throwing away the host and launcher work
