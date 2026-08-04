// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.ComponentModel.Design;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.Shell;

namespace Copperfin.VisualStudio;

internal sealed class ShowCopperfinTerminalWindowCommand
{
    private const int CommandId = 0x0301;
    private static readonly Guid CommandSet = new(PackageGuids.CommandSetString);
    private readonly AsyncPackage package;

    private ShowCopperfinTerminalWindowCommand(AsyncPackage package, OleMenuCommandService commandService)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        this.package = package;

        var command = new OleMenuCommand(
            (_, _) => { _ = package.JoinableTaskFactory.RunAsync(ShowAsync); },
            new CommandID(CommandSet, CommandId))
        {
            Text = CopperfinLocalization.FromVisualStudioUiCulture().Text("VSIX.TerminalWindow.Title")
        };
        commandService.AddCommand(command);
    }

    public static async Task InitializeAsync(AsyncPackage package)
    {
        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

        var commandService = await package.GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
        if (commandService is null)
        {
            throw new InvalidOperationException(
                CopperfinLocalization.FromVisualStudioUiCulture().Text("AssetEditor.Error.MenuCommandServiceUnavailable"));
        }

        _ = new ShowCopperfinTerminalWindowCommand(package, commandService);
    }

    private async Task ShowAsync()
    {
        await package.JoinableTaskFactory.SwitchToMainThreadAsync(CancellationToken.None);
        var window = await package.ShowToolWindowAsync(
            typeof(CopperfinTerminalWindowPane),
            0,
            true,
            CancellationToken.None);
        if (window is null)
        {
            throw new InvalidOperationException(
                CopperfinLocalization.FromVisualStudioUiCulture().Text("VSIX.TerminalWindow.Unavailable"));
        }
    }
}
