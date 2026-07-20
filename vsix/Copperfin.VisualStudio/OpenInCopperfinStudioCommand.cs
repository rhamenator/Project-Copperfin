// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.ComponentModel.Design;
using System.IO;
using System.Threading.Tasks;
using EnvDTE;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;

namespace Copperfin.VisualStudio;

internal sealed class OpenInCopperfinStudioCommand
{
    private static readonly Guid CommandSet = new(PackageGuids.CommandSetString);
    private static readonly CopperfinLocalization Localization = CopperfinLocalization.FromVisualStudioUiCulture();

    private readonly AsyncPackage package;

    private OpenInCopperfinStudioCommand(AsyncPackage package, OleMenuCommandService commandService)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        this.package = package;

        foreach (var registration in CopperfinStudioCommandRegistrations.All)
        {
            AddCommand(commandService, registration.CommandId, registration.Preference);
        }
    }

    private void AddCommand(
        OleMenuCommandService commandService,
        int commandId,
        CopperfinStudioTargetPreference preference)
    {
        var menuCommandId = new CommandID(CommandSet, commandId);
        var menuItem = new OleMenuCommand(
            (_, _) => { _ = package.JoinableTaskFactory.RunAsync(() => ExecuteAsync(preference)); },
            menuCommandId)
        {
            Text = Localization.Text("VSIX.Command.OpenInStudio")
        };
        commandService.AddCommand(menuItem);
    }

    public static async Task InitializeAsync(AsyncPackage package)
    {
        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

        var commandService = await package.GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
        if (commandService is null)
        {
            throw new InvalidOperationException(Localization.Text("AssetEditor.Error.MenuCommandServiceUnavailable"));
        }

        _ = new OpenInCopperfinStudioCommand(package, commandService);
    }

    private async Task ExecuteAsync(CopperfinStudioTargetPreference preference)
    {
        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

        var dte = await package.GetServiceAsync(typeof(DTE)) as DTE;
        var documentPath = dte is null
            ? null
            : CopperfinStudioLauncher.ResolveTargetPath(dte, preference);
        if (!CopperfinStudioTargetSelection.IsSupportedTargetPath(documentPath) ||
            !File.Exists(documentPath))
        {
            VsShellUtilities.ShowMessageBox(
                package,
                Localization.Text("AssetEditor.Dialog.OpenAssetFirst"),
                Localization.Text("AssetEditor.Title"),
                OLEMSGICON.OLEMSGICON_INFO,
                OLEMSGBUTTON.OLEMSGBUTTON_OK,
                OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
            return;
        }

        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (studioHostPath is null)
        {
            VsShellUtilities.ShowMessageBox(
                package,
                Localization.Text("AssetEditor.Dialog.StudioHostMissing"),
                Localization.Text("AssetEditor.Title"),
                OLEMSGICON.OLEMSGICON_WARNING,
                OLEMSGBUTTON.OLEMSGBUTTON_OK,
                OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
            return;
        }

        if (!CopperfinStudioHostBridge.Launch(studioHostPath, documentPath!, localization: Localization))
        {
            VsShellUtilities.ShowMessageBox(
                package,
                Localization.Text("AssetEditor.Dialog.StudioLaunchFailed"),
                Localization.Text("AssetEditor.Title"),
                OLEMSGICON.OLEMSGICON_WARNING,
                OLEMSGBUTTON.OLEMSGBUTTON_OK,
                OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
        }
    }
}
