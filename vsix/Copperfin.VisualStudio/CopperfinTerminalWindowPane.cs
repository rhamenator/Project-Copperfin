// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Drawing;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.PlatformUI;
using Microsoft.VisualStudio.Shell;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

[Guid(PackageGuids.TerminalWindowString)]
internal sealed class CopperfinTerminalWindowPane : ToolWindowPane
{
    private readonly StudioTerminalWindowControl control;

    public CopperfinTerminalWindowPane()
    {
        var localization = CopperfinLocalization.FromVisualStudioUiCulture();
        Caption = localization.Text("VSIX.TerminalWindow.Title");

        control = new StudioTerminalWindowControl(localization);
        ApplyVisualStudioTheme(control);
    }

    public override IWin32Window Window => control;

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
