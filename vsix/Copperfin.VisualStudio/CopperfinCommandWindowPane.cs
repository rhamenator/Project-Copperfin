// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Runtime.InteropServices;
using EnvDTE;
using Microsoft.VisualStudio.Shell;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

[Guid(PackageGuids.CommandWindowString)]
internal sealed class CopperfinCommandWindowPane : ToolWindowPane
{
    private readonly CopperfinCommandWindowControl control;

    public CopperfinCommandWindowPane()
        : base(null)
    {
        var localization = CopperfinLocalization.FromVisualStudioUiCulture();
        Caption = localization.Text("VSIX.CommandWindow.Title");
        control = new CopperfinCommandWindowControl(localization, ExecuteCommandWindowInput);
    }

    public override IWin32Window Window => control;

    public void AppendLine(string message)
    {
        control.AppendLine(message);
    }

    private string ExecuteCommandWindowInput(string command)
    {
        ThreadHelper.ThrowIfNotOnUIThread();

        var dte = ServiceProvider.GlobalProvider.GetService(typeof(DTE)) as DTE;
        var activeDocumentPath = dte?.ActiveDocument?.FullName;
        var editorPane = CopperfinAssetEditorPane.FindForDocument(activeDocumentPath);
        return editorPane?.ExecuteCommandWindowInput(command) ??
               CopperfinLocalization.FromVisualStudioUiCulture().Text("VSIX.CommandWindow.NoActiveSession");
    }
}
