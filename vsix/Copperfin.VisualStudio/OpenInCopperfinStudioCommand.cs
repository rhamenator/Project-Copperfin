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
    private const int CommandId = 0x0100;
    private static readonly Guid CommandSet = new(PackageGuids.CommandSetString);
    private static readonly CopperfinLocalization Localization = CopperfinLocalization.FromEnvironment();

    private readonly AsyncPackage package;

    private OpenInCopperfinStudioCommand(AsyncPackage package, OleMenuCommandService commandService)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        this.package = package;

        var menuCommandId = new CommandID(CommandSet, CommandId);
        var menuItem = new MenuCommand((_, _) => { _ = package.JoinableTaskFactory.RunAsync(ExecuteAsync); }, menuCommandId);
        commandService.AddCommand(menuItem);
    }

    public static async Task InitializeAsync(AsyncPackage package)
    {
        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

        var commandService = await package.GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
        if (commandService is null)
        {
            throw new InvalidOperationException("Unable to get menu command service.");
        }

        _ = new OpenInCopperfinStudioCommand(package, commandService);
    }

    private async Task ExecuteAsync()
    {
        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

        var dte = await package.GetServiceAsync(typeof(DTE)) as DTE;
        var documentPath = dte is null ? null : CopperfinStudioLauncher.ResolveTargetPath(dte);
        if (string.IsNullOrWhiteSpace(documentPath) || !File.Exists(documentPath))
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

        if (!CopperfinStudioHostBridge.Launch(studioHostPath, documentPath!))
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
