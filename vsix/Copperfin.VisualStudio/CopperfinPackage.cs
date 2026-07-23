// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;

namespace Copperfin.VisualStudio;

[PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
[InstalledProductRegistration(
    "#110",
    "#112",
    "0.1.0")]
[ProvideMenuResource("Menus.ctmenu", 1)]
[ProvideToolWindow(
    typeof(CopperfinCommandWindowPane),
    Style = VsDockStyle.Tabbed,
    Orientation = ToolWindowOrientation.Bottom,
    Window = ToolWindowGuids80.Outputwindow,
    DockedHeight = 240)]
[ProvideToolWindow(
    typeof(CopperfinTerminalWindowPane),
    Style = VsDockStyle.Tabbed,
    Orientation = ToolWindowOrientation.Bottom,
    Window = ToolWindowGuids80.Outputwindow,
    DockedHeight = 240)]
[ProvideEditorFactory(typeof(CopperfinAssetEditorFactory), 200)]
[ProvideEditorExtension(typeof(CopperfinAssetEditorFactory), ".pjx", 50, NameResourceID = 200, DefaultName = "Copperfin Visual Designer")]
[ProvideEditorExtension(typeof(CopperfinAssetEditorFactory), ".scx", 50, NameResourceID = 200, DefaultName = "Copperfin Visual Designer")]
[ProvideEditorExtension(typeof(CopperfinAssetEditorFactory), ".vcx", 50, NameResourceID = 200, DefaultName = "Copperfin Visual Designer")]
[ProvideEditorExtension(typeof(CopperfinAssetEditorFactory), ".frx", 50, NameResourceID = 200, DefaultName = "Copperfin Visual Designer")]
[ProvideEditorExtension(typeof(CopperfinAssetEditorFactory), ".lbx", 50, NameResourceID = 200, DefaultName = "Copperfin Visual Designer")]
[ProvideEditorExtension(typeof(CopperfinAssetEditorFactory), ".mnx", 50, NameResourceID = 200, DefaultName = "Copperfin Visual Designer")]
[Guid(PackageGuids.PackageString)]
public sealed class CopperfinPackage : AsyncPackage
{
    protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress)
    {
        await JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
        RegisterEditorFactory(new CopperfinAssetEditorFactory(this));
        await OpenInCopperfinStudioCommand.InitializeAsync(this);
        await CopperfinProjectCommands.InitializeAsync(this);
        await ShowCopperfinCommandWindowCommand.InitializeAsync(this);
        await ShowCopperfinTerminalWindowCommand.InitializeAsync(this);
    }
}
