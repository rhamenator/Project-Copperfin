# Project Copperfin

[![Linux Native Validation](https://github.com/rhamenator/Project-Copperfin/actions/workflows/native-validation-linux.yml/badge.svg?branch=main)](https://github.com/rhamenator/Project-Copperfin/actions/workflows/native-validation-linux.yml)
[![macOS Native Validation](https://github.com/rhamenator/Project-Copperfin/actions/workflows/native-validation-macos.yml/badge.svg?branch=main)](https://github.com/rhamenator/Project-Copperfin/actions/workflows/native-validation-macos.yml)
[![Windows Native Validation](https://github.com/rhamenator/Project-Copperfin/actions/workflows/native-validation-windows.yml/badge.svg?branch=main)](https://github.com/rhamenator/Project-Copperfin/actions/workflows/native-validation-windows.yml)
[![Build Standalone Installers](https://github.com/rhamenator/Project-Copperfin/actions/workflows/build-installers.yml/badge.svg?branch=main)](https://github.com/rhamenator/Project-Copperfin/actions/workflows/build-installers.yml)
[![Build Visual Studio VSIX](https://github.com/rhamenator/Project-Copperfin/actions/workflows/build-vsix.yml/badge.svg?branch=main)](https://github.com/rhamenator/Project-Copperfin/actions/workflows/build-vsix.yml)
[![Security Supply Chain Gate](https://github.com/rhamenator/Project-Copperfin/actions/workflows/security-supply-chain.yml/badge.svg?branch=main)](https://github.com/rhamenator/Project-Copperfin/actions/workflows/security-supply-chain.yml)

Project Copperfin is a clean-room blueprint for a modern successor to Visual FoxPro-style business application development.

Current engineering priority:

- security and speed first
- language choice second

This folder captures the current plan for a new platform that can:

- preserve access to legacy xBase/FoxPro-era data and application assets
- work with modern relational databases such as SQLite, PostgreSQL, SQL Server, and Oracle
- interoperate deeply with the .NET ecosystem and produce .NET-consumable outputs
- provide a modern runtime, designer, reporting stack, and migration path
- add modern security, packaging, deployment, and observability
- avoid direct reuse of FoxPro/VFP branding

Interop maturity:

- Version 1 is anchored on Visual FoxPro 9 compatibility. VFP 6, VFP 7, and VFP 8 assets may work through shared DBF/FPT/CDX/DBC readers, but they are best-effort and untested rather than separate targets. Older Fox Software and xBase-family assets may become wishlist interpretation/inspection targets, but producing binaries for those products is not in the current scope.
- Current .NET support is an early modernization path, not a general managed-runtime surface: the build host can publish a generated C# launcher/stub that the native runtime pipeline starts as a child process, while generated C# transpilation output is only an emitted artifact today.
- Python and broader polyglot support are planning/scaffolding surfaces only; there is no Python runtime hook. These capabilities should stay behind a user-selected modernization target until they are implemented and tested end-to-end.

Requirements Recovery:

- Copperfin's eventual requirements-recovery goal is to build a DO-178-style low-level requirement, code, and test traceability matrix from the implementation and validated VFP9 behavior, because the project did not start with a complete requirements set.
- Recovered requirements must be grounded in real installed VFP9 behavior or shipped documentation, with known-bug and crash exceptions catalogued explicitly. Decompiled VFP binaries are not acceptable requirement sources under the clean-room rules.
- This is a standing product-quality goal. It is one subgoal in the roadmap tree and should be selected when its evidence and dependencies make it the highest-value unfinished work.

Why "Copperfin"?

- `Copper` signals durable infrastructure, wiring, and business systems.
- `Fin` signals navigation, movement, and forward compatibility.
- It does not reuse the Fox/FoxPro/VFP brand family.

Important note:

- `Copperfin` is a working codename and branding starter, not a legal clearance opinion.
- A formal trademark review should happen before public release, filing, or product launch.

License documents:

- [`LICENSE`](LICENSE) — operative GNU GPL v3.0-only text plus the Copperfin
  Application, Runtime, and Toolchain Exception 1.0
- [`LICENSE.md`](LICENSE.md) — concise current-license overview
- [`docs/33-application-runtime-license-exception.md`](docs/33-application-runtime-license-exception.md)
  — exception scope, offline evidence, and release-review rationale
- [`docs/archive/commercial-licensing-2026/`](docs/archive/commercial-licensing-2026/README.md)
  — inactive, preserved commercial/source-available planning documents
- [`SECURITY.md`](SECURITY.md)
- [`CONTRIBUTING.md`](CONTRIBUTING.md) — contribution licensing, provenance,
  sign-off, testing, and clean-room requirements
- [`CONTRIBUTORS.md`](CONTRIBUTORS.md) — durable contributor-credit policy
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md) — participation expectations
- [`SUPPORT.md`](SUPPORT.md) — public and private support routes
- [`GOVERNANCE.md`](GOVERNANCE.md) — project roles and decision process
- [`agents.md`](agents.md)
- [`agent-handoff.md`](agent-handoff.md)
- [`docs/01-product-charter.md`](docs/01-product-charter.md)
- [`docs/02-architecture.md`](docs/02-architecture.md)
- [`docs/03-compatibility-and-migration.md`](docs/03-compatibility-and-migration.md)
- [`docs/04-security-model.md`](docs/04-security-model.md)
- [`docs/05-roadmap.md`](docs/05-roadmap.md)
- [`docs/06-branding.md`](docs/06-branding.md)
- [`docs/07-clean-room-rules.md`](docs/07-clean-room-rules.md)
- [`docs/08-reference-map.md`](docs/08-reference-map.md)
- [`docs/09-adr-cpp-first.md`](docs/09-adr-cpp-first.md)
- [`docs/10-dotnet-interop.md`](docs/10-dotnet-interop.md)
- [`docs/11-engineering-spec-dotnet.md`](docs/11-engineering-spec-dotnet.md)
- [`docs/12-vfp-asset-editing-and-execution.md`](docs/12-vfp-asset-editing-and-execution.md)
- [`docs/13-index-format-notes.md`](docs/13-index-format-notes.md)
- [`docs/14-hybrid-studio-and-visual-studio-host.md`](docs/14-hybrid-studio-and-visual-studio-host.md)
- [`docs/15-local-product-archeology.md`](docs/15-local-product-archeology.md)
- [`docs/16-vfp9-equivalent-subsystems.md`](docs/16-vfp9-equivalent-subsystems.md)
- [`docs/17-modern-designer-direction.md`](docs/17-modern-designer-direction.md)
- [`docs/18-native-security-and-rbac.md`](docs/18-native-security-and-rbac.md)
- [`docs/19-polyglot-and-ai-subprojects.md`](docs/19-polyglot-and-ai-subprojects.md)
- [`docs/20-runtime-build-and-debug-pipeline.md`](docs/20-runtime-build-and-debug-pipeline.md)
- [`docs/21-database-federation-and-query-translation.md`](docs/21-database-federation-and-query-translation.md)
- [`docs/22-vfp-language-reference-coverage.md`](docs/22-vfp-language-reference-coverage.md)
- [`docs/23-vfp-help-and-component-mining.md`](docs/23-vfp-help-and-component-mining.md)
- [`docs/23-phase-a-dependency-breakdown.md`](docs/23-phase-a-dependency-breakdown.md)
- [`docs/24-system-uml.md`](docs/24-system-uml.md)
- [`docs/25-engine-concurrency-policy.md`](docs/25-engine-concurrency-policy.md)
- [`docs/26-localization-and-release-readiness.md`](docs/26-localization-and-release-readiness.md)
- [`docs/27-known-vfp9-bug-exceptions.md`](docs/27-known-vfp9-bug-exceptions.md)
- [`docs/32-recovered-requirements-traceability.md`](docs/32-recovered-requirements-traceability.md)
- [`assets/copperfin-logo.png`](assets/copperfin-logo.png)

Current implementation focus:

1. Complete the unfinished MVP workstream tree in [`docs/05-roadmap.md`](docs/05-roadmap.md), selecting work by evidence, risk, blockers, and user-visible impact.
2. Finish report/label fidelity, IDE workflows, runtime compatibility, localization, package/debug contracts, security, and platform seams without treating any one workstream as permanently active.
3. Validate standalone Studio and Visual Studio through the shared model, stable host contracts, and focused smoke coverage.
4. Treat implementation completion of the whole MVP tree as RC readiness; perform release evidence and artifact validation afterward.
5. Preserve modernization, .NET interop, portability, and Requirements Recovery as explicit v1 roadmap goals.

Implementation bias:

- native core for performance-sensitive and trust-sensitive subsystems
- optional higher-level companion tooling only when it stays outside the product core
- first-class .NET interoperability and 64-bit support
- selective Rust use is acceptable where it materially improves safety or speed
- first-class ability to open, inspect, edit, and eventually execute legacy VFP assets

Current runnable artifacts:

- `build\Release\copperfin_inspect.exe`
  - low-level file and index inspection for DBF/VFP asset families
- `build\Release\copperfin_studio_host.exe`
  - early human-facing Studio host for opening legacy VFP assets and previewing schema/records
  - now supports `--json` snapshots for Visual Studio designer integration
  - now supports `--list-subsystems` to inspect the planned VFP 9-equivalent Copperfin subsystem map
  - now emits a structured `reportLayout` snapshot for `FRX/FRT` and `LBX/LBT` assets with named sections and placed objects
  - now emits native database-federation metadata for relational, document, and vector connector planning in the shared VS/Studio shells
- `build\Release\copperfin_build_host.exe`
  - native package/build pipeline entry point for `PJX/PJT` projects
  - stages project assets, emits runtime and debug manifests, bundles the runtime host, and can publish a generated `.NET` launcher
- `build\Release\copperfin_runtime_host.exe`
  - native runtime/debug launch host for packaged Copperfin applications
  - reads `app.cfmanifest` and now executes `PRG` startup code through a native xBase runtime session
  - supports real breakpoint and step-debugging actions for `PRG` startup paths
  - now emits richer debug state including call stack, locals, globals, and runtime events for debugger surfaces in Visual Studio and standalone Studio
  - now tracks richer VFP-style compatibility state for work areas, aliases, table cursors, data sessions, SQL pass-through cursors, and OLE automation objects in debug output
  - runtime faults now pause on the offending source line and keep the host session alive for continued debugging
  - the VSIX now registers a FoxPro text content type for `PRG`, `H`, `QPR`, `MPR`, and `SPR` files with project-aware statement completion and first-pass hover help in Visual Studio
  - now bootstraps runnable `SCX/VCX/MNX` startup behavior through generated `PRG` wrappers
  - now boots runnable `FRX/LBX` startup assets into direct preview/event-loop mode instead of treating them as inert metadata
  - packaged `SCX/VCX` startup assets now stage their memo sidecars and can launch from packaged content instead of only from source trees
  - now recognizes `ACTIVATE MENU` / `ACTIVATE POPUP` as real event-loop runtime operations
  - now supports dispatching runtime actions back into waiting xAssets through `--debug-command select:<action-id>` and `--debug-command invoke:<action-id>`
  - menu startup assets can now dispatch real menu-item selections while paused in the event loop
  - runtime xAsset execution now loads the full backing table instead of only the small Studio preview slice, so deeper menu trees and later records can participate in execution
  - form/class/report/label method surfaces now expose dispatchable runtime actions derived from extracted xBase methods
  - `DO FORM` now resolves quoted paths cleanly and launches runnable forms into the event loop instead of treating them like one-shot helper scripts
  - still reports compatibility-mode launch information for non-runnable xAssets and other startup assets while those runtimes are under construction
- `vsix\Copperfin.VisualStudio\Copperfin.VisualStudio.csproj`
  - installable Visual Studio extension baseline for VS 2022+
  - current output: `vsix\Copperfin.VisualStudio\bin\Release\net472\Copperfin.VisualStudio.vsix`
  - registers a first `Copperfin Visual Designer` document shell for `PJX/SCX/VCX/FRX/LBX/MNX`
  - now provides first-pass FoxPro editor assistance for `PRG/H/QPR/MPR/SPR`, including project-aware completion, Quick Info, call signature help, and `F12` definition navigation for indexed project symbols and assets
  - current shell shows object/property snapshots sourced from the native Studio host
  - current `SCX/SCT` and `VCX/VCT` slices expose flattened VFP `PROPERTIES` data for inline object selection, design-surface layout, drag-move, and safe property edits
  - current `FRX/FRT` and `LBX/LBT` slices now surface named report sections in a more modern Visual Studio-style designer shell, with section-aware layout editing for `HPOS/VPOS/WIDTH/HEIGHT` plus key expression/font fields
  - current `MNX/MNT` slices support asset-aware property-grid editing for menu metadata
  - current `PJX/PJT` slices now surface a grouped project workspace with project-item grouping, startup/build-plan summary, and project-entry property editing
  - project workspaces now also surface the platform's native security/RBAC posture and `.NET`/Python/MCP extensibility story
  - project workspaces now include a first integrated debugger pane backed by the native runtime host, with pause reason, call stack, locals, globals, and runtime-event summaries
  - project workspaces now include shared `Task List`, `Code References`, `Data Explorer`, `Object Browser`, `Toolbox`, `Builders`, `Coverage`, and `Database` panes so the IDE surface is moving toward classic VFP utility windows instead of only showing a build summary
  - project workspaces now explicitly surface user-selected AI debugging assistance plus Python/R data-science sidecar guidance inside the modern extensibility story
  - project workspaces now expose a native database-federation profile that distinguishes deterministic relational translation from optional AI-assisted document/vector planning
- `vsix\Copperfin.Studio\Copperfin.Studio.csproj`
  - standalone Windows shell that reuses the same shared report/label/form/menu/project designer controls outside Visual Studio
  - now opens multiple assets as tabs so the managed shell feels more like an IDE workspace instead of a single-document dialog
  - now shares the same first integrated project debugger pane used by the Visual Studio designer shell
  - current output: `vsix\Copperfin.Studio\bin\Release\net472\Copperfin.Studio.exe`
- `vsix\Copperfin.DesignerSmokeTests\Copperfin.DesignerSmokeTests.csproj`
  - automated WinForms smoke tests for the shared designer UI
  - current output: `vsix\Copperfin.DesignerSmokeTests\bin\Release\net472\Copperfin.DesignerSmokeTests.exe`
  - exercises synthetic rendering plus real `invoice.frx`, `cust.lbx`, and `solution.pjx` assets from the local VFP 9 sample tree
  - now smoke-tests the shared project debugger, task list, code references, data explorer, object browser, toolbox, builders, coverage, and database surfaces against a real VFP sample project

Quick examples:

```powershell
E:\Project-Copperfin\build\Release\copperfin_studio_host.exe --from-vs --path "C:\Program Files (x86)\Microsoft Visual FoxPro 9\Wizards\Template\Books\Forms\books.scx"
E:\Project-Copperfin\build\Release\copperfin_studio_host.exe "C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\solution.pjx"
E:\Project-Copperfin\build\Release\copperfin_inspect.exe "E:\DBASE\DBFS\CHNGREAS.NDX"
E:\Project-Copperfin\build\Release\copperfin_build_host.exe build --project "C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\solution.pjx" --output-dir "E:\Project-Copperfin\artifacts\runtime-smoke" --configuration debug --enable-security --emit-dotnet-launcher
E:\Project-Copperfin\scripts\validate-windows.ps1
```

Native CMake validation defaults to two concurrent compile jobs to stay within hosted-runner and ordinary workstation memory limits. POSIX operators can override the local default with `COPPERFIN_BUILD_JOBS=4 scripts/validate-posix.sh`; the script also bounds each CTest case to 180 seconds. Windows operators can use `scripts\validate-windows.ps1 -BuildJobs 4`. The ordinary hosted validation workflows retain the two-job cap unless measured runner capacity supports a deliberate increase. Manual Windows Deep Validation exposes bounded two- and three-job trial choices; its JSON artifact and GitHub summary report per-phase duration plus sampled runner CPU and memory headroom, and do not authorize changing the default by themselves.

GitHub validation:

- `.github/workflows/native-validation-linux.yml`, `.github/workflows/native-validation-macos.yml`, and `.github/workflows/native-validation-windows.yml` provide independently dispatchable platform checks. All three call `.github/actions/native-validation/action.yml`, which keeps configure, bounded compilation, and the full CTest suite in one platform job while preserving the stable platform check names. The native suite uses two CTest workers on all platforms; CTest `RUN_SERIAL` and resource-lock properties continue to serialize unsafe tests.
- `.github/workflows/managed-ui-validation-linux.yml` runs the shared managed UI smoke harness on Linux with Mono and Xvfb, includes display-free test discovery, and uploads selector logs as portable evidence. macOS remains runnable locally through the same harness with an existing XQuartz display; hosted Visual Studio, Win32, installer, and VFP9 behavior remain Windows-only gates.
- Release readiness requires successful `Linux GCC`, `macOS Clang`, and `Windows MSVC` checks. Manual `.github/workflows/native-release-readiness.yml` runs all three shared contracts and exposes a final dependent gate. Native validation does not publish or reuse CMake build trees between jobs or workflow runs.
- `.github/workflows/build-installers.yml` remains focused on packaging standalone installer artifacts and intentionally builds with tests disabled.
- `.github/workflows/windows-deep-validation.yml` is manual dispatch only and runs a deeper Windows hosted build across native tests, VSIX, standalone Studio, and designer smoke test binaries. `scripts/measure-windows-validation.ps1` records configure, native compile/test, and managed compile/test evidence under `artifacts/windows-deep-validation-metrics`; those JSON files are uploaded even when a measured phase fails.
- `.github/workflows/windows-msvc-cache-evaluation.yml` is a manual, non-authoritative repeat-build experiment. It pins the sccache action and binary, uses Ninja/MSVC with `/Z7`, namespaces compiler intermediates by the relevant toolchain and build inputs, exercises source/generated-header/flag/configuration/architecture/compiler invalidation plus malformed-entry fallback, and still runs the complete warm CTest inventory. Hosted run `29503180834` reduced native compilation from `2069.669` to `237.902` seconds (`88.51%`) with a `98.93%` warm-hit ratio and passed all `281` tests. [GitHub documents that cache contents are not signed or otherwise verified](https://docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching#cache-poisoning), so required validation, deep validation, release readiness, installers, and VSIX builds remain cache-free; a warm-cache result must never substitute for an authoritative build or test result.
- Windows hosted measurements keep the ordinary native build at two jobs: three-job trials were memory-safe but did not materially reduce cold compile time. Macro-independent test support and exact matching compile-definition groups are reusable object targets in `tests/CMakeLists.txt`; configure-time guards require each documented consumer to retain the expected source. Keep semantically distinct macro variants separate.

Portable shared-UI validation:

- `scripts/run-designer-smoke-portable.sh` is the canonical non-Windows entry point for the shared managed WinForms surface. It builds `Copperfin.DesignerSmokeTests` with `EnableWindowsTargeting=true`, runs test selectors through Mono, and uses Xvfb on Linux or an existing XQuartz/X display on macOS. `--list-tests` works without a display; `--display-mode offscreen` is intentionally limited to test discovery because this target needs a display for actual WinForms initialization.
- `scripts/run-designer-smoke-headless.sh` remains a strict Xvfb wrapper for unattended Linux jobs and compatibility with existing automation. Portable smoke evidence covers shared managed controls and shell behavior. Live Visual Studio docking/theme behavior, Win32 rendering, installer validation, and hosted VFP9 behavior remain Windows-only evidence.

Current MVP scope:

- open `PJX/PJT`, `SCX/SCT`, `VCX/VCT`, `FRX/FRT`, `LBX/LBT`, `MNX/MNT`, `DBF/FPT`, `CDX/DCX`, `IDX`, `NDX`, and `MDX`
- inspect headers and companion files
- preview DBF-style schema and a small number of records
- decode memo-backed preview values for synthetic and many real sidecar-backed assets

Known limitation:

- memo-heavy real-world designer files still need deeper VFP-specific decoding and presentation polish before they feel like a true visual designer
- the Visual Studio extension and standalone Studio now provide a shared project debugger pane on top of the native runtime/debug host, but they still do not provide full VFP 9-style design fidelity or the full FoxPro/VFP runtime surface
- packaged runtime output is now good enough to launch runnable `SCX/VCX/MNX/FRX/LBX` startup assets from staged package content, but deeper event/lifecycle fidelity still needs work
- runtime action dispatch now covers menu selection and extracted method invocation, but higher-fidelity form/report object lifecycle, richer UI event simulation, and broader runtime semantics still need work
- runtime execution no longer truncates to the Studio preview record limit, which improves parity for deeper `MNX` submenu trees and other larger xAssets
- the new standalone Studio shell shares the same editor stack, but it is still an early shell rather than the finished full-fidelity Copperfin IDE
- the standalone Studio shell now supports tabbed multi-document editing, but it still needs richer IDE chrome and workflow parity before it feels like the full VFP 9 experience
- the new project utility panes are strong parity progress, but they are still summary-driven surfaces rather than the full interactive VFP 9 Project Manager, Data Explorer, Object Browser, Coverage, and builder ecosystem

## Licensing

Project Copperfin is free software licensed under the **GNU General Public
License, version 3 only** (`GPL-3.0-only`) with the **Copperfin Application,
Runtime, and Toolchain Exception 1.0**. See [`LICENSE`](LICENSE) for the
complete operative terms and [`LICENSE.md`](LICENSE.md) for the plain-language
boundary.

Independent applications remain under their owners' chosen terms—including
proprietary terms—when Copperfin inspects, edits, runs, interprets, analyzes,
transforms, modernizes, generates, compiles, assembles, links, packages, tests,
debugs, deploys, hosts, maintains, or supports them. The exception covers
static and dynamic linking, in-process and separate-process execution,
documented interfaces, distributable Copperfin runtime components, and
ordinary generated launchers, scaffolds, support code, and package material.
The GPL continues to cover Copperfin itself and modifications derived from
Copperfin source code; GPL obligations for those Copperfin portions do not
extend into an independent application merely because of these permitted
uses or combinations.

Contributors retain copyright in their contributions and receive durable
credit through Git authorship/co-authorship, pull requests, issues, and
GitHub's contributor history. Contributions are submitted under the same
GPL-3.0-only-with-exception terms; no copyright assignment is required. See
[`CONTRIBUTING.md`](CONTRIBUTING.md) and [`CONTRIBUTORS.md`](CONTRIBUTORS.md).

The earlier source-available/commercial model is inactive. Its documentation
and implementation are preserved for possible future reconsideration, but
normal builds ignore product license files, product-license environment
configuration, and entitlement signer registries. See the
[inactive archive](docs/archive/commercial-licensing-2026/README.md).

Release artifacts and launcher inventories remain cryptographically signed.
Those signatures establish provenance and integrity; they are not product
activation and do not alter the GPL terms.
