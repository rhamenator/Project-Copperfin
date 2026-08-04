// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Globalization;
using System.IO;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestVsixLongLivedSurfacesRefreshLocalization()
    {
        var repositoryRoot = FindRepositoryRoot();
        Expect(repositoryRoot is not null,
            "VSIX long-lived localization test should locate the repository root");
        if (repositoryRoot is null)
        {
            return;
        }

        var openCommandSource = File.ReadAllText(Path.Combine(
            repositoryRoot,
            "vsix",
            "Copperfin.VisualStudio",
            "OpenInCopperfinStudioCommand.cs"));
        var projectCommandSource = File.ReadAllText(Path.Combine(
            repositoryRoot,
            "vsix",
            "Copperfin.VisualStudio",
            "CopperfinProjectCommands.cs"));
        var insightSource = File.ReadAllText(Path.Combine(
            repositoryRoot,
            "vsix",
            "Copperfin.VisualStudio",
            "CopperfinProjectInsightClient.cs"));

        Expect(openCommandSource.Contains(
                    "private static CopperfinLocalization Localization => CopperfinLocalization.FromVisualStudioUiCulture();",
                    StringComparison.Ordinal) &&
               openCommandSource.Contains("BeforeQueryStatus", StringComparison.Ordinal) &&
               openCommandSource.Contains("CommandLabelKey", StringComparison.Ordinal),
            "Open-in-Studio commands should refresh display labels and localization at menu-query time");
        Expect(projectCommandSource.Contains(
                    "private static CopperfinLocalization Localization => CopperfinLocalization.FromVisualStudioUiCulture();",
                    StringComparison.Ordinal) &&
               projectCommandSource.Contains("string labelKey", StringComparison.Ordinal) &&
               projectCommandSource.Contains("menuCommand.Text = Localization.Text(labelKey)", StringComparison.Ordinal),
            "project commands should retain localization keys and refresh their labels at menu-query time");
        Expect(insightSource.Contains(
                    "private static CopperfinLocalization Localization => CopperfinLocalization.FromVisualStudioUiCulture();",
                    StringComparison.Ordinal) &&
               !insightSource.Contains(
                    "private static readonly CopperfinLocalization Localization",
                    StringComparison.Ordinal),
            "project insights should resolve the Visual Studio UI catalog per request");

        var previousUiLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_LOCALE");
        var previousCulture = CultureInfo.CurrentUICulture;
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", null);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", null);

            var snapshot = new CopperfinStudioSnapshotDocument
            {
                Path = Path.Combine(repositoryRoot, "missing-project.pjx"),
                AssetFamily = "project"
            };
            var warningKey = "AssetEditor.ProjectInsights.Warning.MetadataUnavailable";

            foreach (var locale in new[] { "en-US", "es-419", "pt-BR", "qps-ploc" })
            {
                CultureInfo.CurrentUICulture = new CultureInfo(locale);
                var expected = new CopperfinLocalization(locale).Text(warningKey);
                Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", locale);
                var insights = CopperfinProjectInsightClient.BuildInsights(snapshot);

                Expect(insights.Warnings.Count == 1 && insights.Warnings[0] == expected,
                    $"{locale} project insight request should resolve the current catalog");
            }
        }
        finally
        {
            CultureInfo.CurrentUICulture = previousCulture;
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousUiLocale);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", previousLocale);
        }
    }
}
