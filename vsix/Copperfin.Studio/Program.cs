// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal static class Program
{
    [STAThread]
    private static void Main(string[] args)
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);

        var localization = new CopperfinLocalization(ReadLocaleArgument(args));
        if (!CopperfinStudioStartupArguments.TryParse(args, localization, out var documents, out var error))
        {
            MessageBox.Show(
                error,
                localization.Text("Studio.AppTitle"),
                MessageBoxButtons.OK,
                MessageBoxIcon.Warning);
            return;
        }

        using var form = new StudioMainForm(localization, StudioShellLayoutFileStore.CreateDefault());
        foreach (var document in documents)
        {
            form.OpenDocument(document.Path, document.ObjectName, document.UniqueId);
        }

        Application.Run(form);
    }

    private static string? ReadLocaleArgument(string[] args)
    {
        for (var index = 0; index < args.Length; ++index)
        {
            if (string.Equals(args[index], "--locale", StringComparison.OrdinalIgnoreCase) &&
                index + 1 < args.Length)
            {
                return args[index + 1];
            }
        }

        var locale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        return string.IsNullOrWhiteSpace(locale)
            ? Environment.GetEnvironmentVariable("COPPERFIN_LOCALE")
            : locale;
    }

}
