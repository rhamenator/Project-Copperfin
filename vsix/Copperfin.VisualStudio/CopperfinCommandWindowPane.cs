// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using Microsoft.VisualStudio.Shell;

namespace Copperfin.VisualStudio;

[Guid(PackageGuids.CommandWindowString)]
internal sealed class CopperfinCommandWindowPane : ToolWindowPane
{
    public CopperfinCommandWindowPane()
        : base(null)
    {
        var localization = CopperfinLocalization.FromVisualStudioUiCulture();
        Caption = localization.Text("VSIX.CommandWindow.Title");
        Content = new CopperfinCommandWindowControl(localization);
    }

    public void AppendLine(string message)
    {
        if (Content is CopperfinCommandWindowControl control)
        {
            control.AppendLine(message);
        }
    }
}
