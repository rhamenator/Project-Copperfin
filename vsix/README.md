# Copperfin Visual Studio Extension

This is the first installable VSIX baseline for Copperfin.

Current scope:

- targets Visual Studio 2022 and later compatible major versions
- builds an installable VSIX at `bin\Release\net472\Copperfin.VisualStudio.vsix`
- registers a `Copperfin Visual Designer` editor for `PJX`, `SCX`, `VCX`, `FRX`, `LBX`, and `MNX`
- adds an `Open In Copperfin Studio` command under `Tools`
- adds the same command to Solution Explorer item context menus
- opens a Copperfin document surface inside Visual Studio for registered asset types
- loads a structured object/property snapshot from the native Studio host for the open asset
- lets that document surface launch the native `copperfin_studio_host.exe`
- launches the native `copperfin_studio_host.exe` against the active document or selected project item

Current limitations:

- the registered document surface is a first designer shell, not yet a full inline form/report/menu/class designer
- the first useful fidelity slice is read-only object/property inspection, especially for `SCX/SCT`
- it does not yet own project build/debug/run pipelines for Copperfin applications
- it does not yet render forms/reports inline inside the Visual Studio document with VFP 9 fidelity
- it does not yet compile Copperfin executables from inside Visual Studio

Why VS 2022 as the baseline:

- Visual Studio 2022 is the 64-bit turning point that matches Copperfin's x64-first direction
- Microsoft ships the relevant managed extension SDK for VS 2022 through NuGet packages in the `17.x` line
- supporting older IDEs would add compatibility drag before we have the actual designer stack built
- Visual Studio Code is not the primary target for VFP-style designers because its extension model is much weaker for host-integrated visual editors, designers, and project-system experiences

Build note:

- the extension currently looks for `copperfin_studio_host.exe` at `COPPERFIN_STUDIO_HOST_PATH`
- if that variable is not set, it falls back to `E:\Project-Copperfin\build\Release\copperfin_studio_host.exe`

Suggested first-use flow:

1. Build `copperfin_studio_host.exe`
2. Rebuild the VSIX project
3. Install `bin\Release\net472\Copperfin.VisualStudio.vsix`
4. Open or select a `PJX`, `SCX`, `VCX`, `FRX`, `LBX`, or `MNX` file in Visual Studio
5. Double-click the asset to open the `Copperfin Visual Designer` shell, or use `Tools -> Open In Copperfin Studio`
