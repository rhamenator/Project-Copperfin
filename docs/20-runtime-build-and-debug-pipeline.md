# Runtime, Build, And Debug Pipeline

Copperfin now has a first real runtime/build/debug pipeline slice instead of only design-time inspection.

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
  - now also bootstraps runnable `SCX/VCX/MNX` startup behavior through generated `PRG` wrappers
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
  - `.NET` executable published beside the native runtime host

Current CLI flow:

```powershell
E:\Project-Copperfin\build\Release\copperfin_build_host.exe build `
  --project "C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\solution.pjx" `
  --output-dir "E:\Project-Copperfin\artifacts\runtime-smoke" `
  --configuration debug `
  --enable-security `
  --emit-dotnet-launcher
```

```powershell
E:\Project-Copperfin\artifacts\runtime-smoke\SOLUTION\SOLUTION.exe --debug
```

```powershell
E:\Project-Copperfin\artifacts\runtime-smoke\SOLUTION\copperfin_runtime_host.exe `
  --manifest "E:\Project-Copperfin\artifacts\runtime-smoke\SOLUTION\app.cfmanifest" `
  --debug `
  --breakpoint 12 `
  --debug-command continue `
  --debug-command step `
  --debug-command out
```

```powershell
E:\Project-Copperfin\build\Release\copperfin_runtime_host.exe `
  --manifest "E:\Project-Copperfin\artifacts\menu-debug-smoke\app.cfmanifest" `
  --debug `
  --debug-command continue `
  --debug-command select:shortcut.item1
```

```powershell
E:\Project-Copperfin\build\Release\copperfin_runtime_host.exe `
  --manifest "E:\Project-Copperfin\artifacts\xasset-debug-smoke\app.cfmanifest" `
  --debug `
  --debug-command continue `
  --debug-command invoke:frmbooks.release
```

Current behavior:

- runtime packaging is `Windows-first`
- generated packages are `x64`
- `.NET` launchers are emitted as `net8.0-windows` executables
- packaged runtime manifests now point at staged package content instead of stale legacy source paths
- debug manifests keep source-side paths so Visual Studio and the standalone Studio shell can debug against the editable source tree
- `PRG` startup paths now advertise real breakpoint and step-debugging support in the debug manifest
- runnable `SCX/VCX` startup assets can now be executed through generated method bootstraps from both source trees and packaged content
- runnable `MNX/MNT` startup assets can now execute setup code and activate shortcut/menu event loops through a dedicated menu bootstrap model
- waiting menu runtimes can now dispatch concrete menu-item actions back into the native runtime
- waiting form/class/report/label xAssets now expose extracted methods as dispatchable runtime actions for debugger-driven invocation
- startup assets that legacy projects mark as excluded are now still staged when they are required for runtime startup
- packaged xAsset startup paths now carry their memo sidecars forward so the bootstrap runtime can open real designer assets instead of dead table shells

Current limitations:

- the native execution engine is `PRG-first`, not yet the full FoxPro/VFP command/runtime surface
- xBase code embedded in `SCX/VCX` assets is now partially executable through `METHODS` bootstrapping, but deeper event/lifecycle fidelity still needs work
- `MNX` startup activation plus first menu selection dispatch now work, but richer menu navigation, nested command routing, and broader surface parity still need work
- `FRX/LBX` method extraction exists as a model, but those designer/runtime families are not yet executed directly
- package manifests are line-based metadata, not the finished long-term runtime format
- build output planning is still driven by current `PJX` heuristics rather than a fully compatible FoxPro compiler/runtime

What this unlocks next:

- Visual Studio build/run/debug commands can target a real package pipeline
- the standalone Studio shell can run the same build/package flow
- the future Copperfin runtime can take over the same manifest/package contract without throwing away the host and launcher work
