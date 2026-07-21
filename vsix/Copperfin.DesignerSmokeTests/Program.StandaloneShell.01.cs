// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System.Drawing;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void SmokeStandaloneStudioCommandWindowDocking()
    {
        using var form = new StudioMainForm(new CopperfinLocalization("es-419"))
        {
            Width = 1200,
            Height = 800,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        form.Show();
        Application.DoEvents();

        Expect(form.IsCommandWindowVisible,
            "standalone Studio should show the docked Command window by default");
        Expect(form.CommandWindowTabTitle == "Comando de Copperfin",
            "standalone Command window should use the active locale catalog");

        form.SetCommandWindowVisible(false);
        Application.DoEvents();
        Expect(!form.IsCommandWindowVisible,
            "standalone Command window should be hideable without closing document tabs");

        form.SetCommandWindowVisible(true);
        Application.DoEvents();
        Expect(form.IsCommandWindowVisible,
            "standalone Command window should be restorable as a docked pane");

        TearDownForm(form);
    }
}
