// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
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
        using var form = new StudioMainForm(localization);
        foreach (var candidate in ReadAssetArguments(args))
        {
            if (string.IsNullOrWhiteSpace(candidate))
            {
                continue;
            }

            form.OpenDocument(candidate);
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

        return Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
    }

    private static IEnumerable<string> ReadAssetArguments(string[] args)
    {
        for (var index = 0; index < args.Length; ++index)
        {
            if (string.Equals(args[index], "--locale", StringComparison.OrdinalIgnoreCase))
            {
                ++index;
                continue;
            }

            yield return args[index];
        }
    }
}
