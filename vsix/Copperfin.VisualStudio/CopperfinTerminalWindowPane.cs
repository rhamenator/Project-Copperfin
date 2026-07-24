// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Drawing;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.PlatformUI;
using Microsoft.VisualStudio.Shell;

namespace Copperfin.VisualStudio;

[Guid(PackageGuids.TerminalWindowString)]
internal sealed class CopperfinTerminalWindowPane : ToolWindowPane
{
    public CopperfinTerminalWindowPane()
    {
        var localization = CopperfinLocalization.FromVisualStudioUiCulture();
        Caption = localization.Text("VSIX.TerminalWindow.Title");

        var control = new StudioTerminalWindowControl(localization);
        ApplyVisualStudioTheme(control);
        Content = control;
    }

    private static void ApplyVisualStudioTheme(StudioTerminalWindowControl control)
    {
        try
        {
            control.ApplyTheme(
                VSColorTheme.GetThemedColor(EnvironmentColors.ToolWindowBackgroundColorKey),
                VSColorTheme.GetThemedColor(EnvironmentColors.ToolWindowTextColorKey));
        }
        catch (Exception)
        {
            control.ApplyTheme(SystemColors.Window, SystemColors.WindowText);
        }
    }
}
