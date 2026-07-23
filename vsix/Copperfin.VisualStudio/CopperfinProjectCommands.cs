// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.ComponentModel.Design;
using System.Threading.Tasks;
using EnvDTE;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinProjectCommands
{
    private const int BuildCommandId = 0x0200;
    private const int RunCommandId = 0x0201;
    private const int DebugCommandId = 0x0202;
    private static readonly Guid CommandSet = new(PackageGuids.CommandSetString);
    private static CopperfinLocalization Localization => CopperfinLocalization.FromVisualStudioUiCulture();

    private readonly AsyncPackage package;

    private CopperfinProjectCommands(AsyncPackage package, OleMenuCommandService commandService)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        this.package = package;

        AddCommand(commandService, BuildCommandId, "VSIX.Command.BuildProject", CopperfinProjectOperation.Build);
        AddCommand(commandService, RunCommandId, "VSIX.Command.RunProject", CopperfinProjectOperation.Run);
        AddCommand(commandService, DebugCommandId, "VSIX.Command.DebugProject", CopperfinProjectOperation.Debug);
    }

    public static async Task InitializeAsync(AsyncPackage package)
    {
        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

        var commandService = await package.GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
        if (commandService is null)
        {
            throw new InvalidOperationException(Localization.Text("AssetEditor.Error.MenuCommandServiceUnavailable"));
        }

        _ = new CopperfinProjectCommands(package, commandService);
    }

    private void AddCommand(OleMenuCommandService commandService, int commandId, string labelKey, CopperfinProjectOperation operation)
    {
        var menuCommand = new OleMenuCommand((_, _) =>
        {
            ThreadHelper.ThrowIfNotOnUIThread();
            _ = package.JoinableTaskFactory.RunAsync(() => ExecuteAsync(operation));
        }, new CommandID(CommandSet, commandId));
        menuCommand.Text = Localization.Text(labelKey);
        menuCommand.BeforeQueryStatus += (_, _) =>
        {
            ThreadHelper.ThrowIfNotOnUIThread();
            menuCommand.Text = Localization.Text(labelKey);
            UpdateQueryStatus(menuCommand);
        };
        commandService.AddCommand(menuCommand);
    }

    private void UpdateQueryStatus(OleMenuCommand menuCommand)
    {
        ThreadHelper.ThrowIfNotOnUIThread();

        var dte = ((IServiceProvider)package).GetService(typeof(DTE)) as DTE;
        var projectPath = CopperfinProjectSelection.ResolveProjectPath(dte);
        var enabled = CopperfinProjectWorkflow.IsCopperfinProjectPath(projectPath);
        menuCommand.Visible = true;
        menuCommand.Enabled = enabled;
    }

    private async Task ExecuteAsync(CopperfinProjectOperation operation)
    {
        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

        var dte = await package.GetServiceAsync(typeof(DTE)) as DTE;
        var projectPath = CopperfinProjectSelection.ResolveProjectPath(dte);
        if (!CopperfinProjectWorkflow.IsCopperfinProjectPath(projectPath))
        {
            ShowMessage(
                Localization.Text("AssetEditor.Dialog.OpenProjectFirst"),
                OLEMSGICON.OLEMSGICON_INFO);
            return;
        }

        var result = await CopperfinProjectWorkflow.ExecuteAsync(projectPath!, operation, Localization);
        if (!result.Success)
        {
            ShowMessage(result.Message, OLEMSGICON.OLEMSGICON_WARNING);
            return;
        }

        ShowMessage(Localization.Format("AssetEditor.Dialog.WorkflowLauncher", result.Message, result.LauncherPath), OLEMSGICON.OLEMSGICON_INFO);
    }

    private void ShowMessage(string message, OLEMSGICON icon)
    {
        VsShellUtilities.ShowMessageBox(
            package,
            message,
            Localization.Text("AssetEditor.Title"),
            icon,
            OLEMSGBUTTON.OLEMSGBUTTON_OK,
            OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
    }
}
