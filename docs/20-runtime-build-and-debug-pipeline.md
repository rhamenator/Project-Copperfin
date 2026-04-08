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
  - still falls back to compatibility-mode launch reporting for non-`PRG` startup assets

Current package layout:

- `app.cfmanifest`
  - runtime-facing manifest
  - points at staged package content
- `app.cfdebug`
  - source/debug-facing manifest
  - points at original source assets for IDE/debug workflows
- `content/`
  - staged `PRG`, `SCX`, `VCX`, `FRX`, `LBX`, `MNX`, and related project assets
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

Current behavior:

- runtime packaging is `Windows-first`
- generated packages are `x64`
- `.NET` launchers are emitted as `net8.0-windows` executables
- packaged runtime manifests now point at staged package content instead of stale legacy source paths
- debug manifests keep source-side paths so Visual Studio and the standalone Studio shell can debug against the editable source tree
- `PRG` startup paths now advertise real breakpoint and step-debugging support in the debug manifest
- legacy imported projects with only excluded assets now still get a real startup asset instead of falling back to the project header

Current limitations:

- the native execution engine is `PRG-first`, not yet the full FoxPro/VFP command/runtime surface
- xBase code embedded in `SCX/VCX/FRX/LBX/MNX` assets still needs dedicated runtime decoding and execution
- package manifests are line-based metadata, not the finished long-term runtime format
- build output planning is still driven by current `PJX` heuristics rather than a fully compatible FoxPro compiler/runtime

What this unlocks next:

- Visual Studio build/run/debug commands can target a real package pipeline
- the standalone Studio shell can run the same build/package flow
- the future Copperfin runtime can take over the same manifest/package contract without throwing away the host and launcher work
