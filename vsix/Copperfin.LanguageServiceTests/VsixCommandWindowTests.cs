// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.IO;
using System.Text.Json;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestVsixCommandWindowRegistration()
    {
        var repositoryRoot = FindRepositoryRoot();
        Expect(repositoryRoot is not null,
            "VSIX command-window registration test should locate the repository root");
        if (repositoryRoot is null)
        {
            return;
        }

        var vsixRoot = Path.Combine(repositoryRoot, "vsix", "Copperfin.VisualStudio");
        var packageSource = File.ReadAllText(Path.Combine(vsixRoot, "CopperfinPackage.cs"));
        var paneSource = File.ReadAllText(Path.Combine(vsixRoot, "CopperfinCommandWindowPane.cs"));
        var commandSource = File.ReadAllText(Path.Combine(vsixRoot, "ShowCopperfinCommandWindowCommand.cs"));
        var commandTable = File.ReadAllText(Path.Combine(vsixRoot, "Copperfin.vsct"));

        Expect(packageSource.Contains("ProvideToolWindow(", StringComparison.Ordinal) &&
               packageSource.Contains("typeof(CopperfinCommandWindowPane)", StringComparison.Ordinal),
            "VSIX package should register the Copperfin command window");
        Expect(packageSource.Contains("Style = VsDockStyle.Tabbed", StringComparison.Ordinal) &&
               packageSource.Contains("Orientation = ToolWindowOrientation.Bottom", StringComparison.Ordinal),
            "VSIX command window should default to the Visual Studio bottom tabbed tool-window region");
        Expect(packageSource.Contains("ShowCopperfinCommandWindowCommand.InitializeAsync(this)", StringComparison.Ordinal),
            "VSIX package should initialize the command-window command");
        Expect(paneSource.Contains("[Guid(PackageGuids.CommandWindowString)]", StringComparison.Ordinal) &&
               paneSource.Contains(": ToolWindowPane", StringComparison.Ordinal) &&
               paneSource.Contains("VSIX.CommandWindow.Title", StringComparison.Ordinal),
            "VSIX command window should be a localized ToolWindowPane with a stable identity");
        Expect(commandSource.Contains("ShowToolWindowAsync", StringComparison.Ordinal) &&
               commandSource.Contains("typeof(CopperfinCommandWindowPane)", StringComparison.Ordinal),
            "VSIX command should show the registered command window instead of launching a second shell");
        Expect(commandTable.Contains("ShowCopperfinCommandWindowCommand", StringComparison.Ordinal) &&
               commandTable.Contains("value=\"0x0300\"", StringComparison.Ordinal) &&
               commandTable.Contains(".Copperfin.ShowCommandWindow", StringComparison.Ordinal),
            "VSIX command table should expose the command-window command with a stable command identity");

        foreach (var locale in new[] { "en-US", "es-419", "pt-BR", "qps-ploc" })
        {
            var catalogPath = Path.Combine(repositoryRoot, "resources", "locales", locale, "strings.json");
            using var document = JsonDocument.Parse(File.ReadAllText(catalogPath));
            var root = document.RootElement;
            Expect(root.TryGetProperty("VSIX.CommandWindow.Title", out var title) &&
                   !string.IsNullOrWhiteSpace(title.GetString()),
                $"{locale} catalog should provide the command-window title");
            Expect(root.TryGetProperty("VSIX.CommandWindow.Ready", out var ready) &&
                   !string.IsNullOrWhiteSpace(ready.GetString()),
                $"{locale} catalog should provide the command-window ready status");
            Expect(root.TryGetProperty("VSIX.CommandWindow.Unavailable", out var unavailable) &&
                   !string.IsNullOrWhiteSpace(unavailable.GetString()),
                $"{locale} catalog should provide the command-window unavailable message");
        }
    }
}
