// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.IO;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestLocalizationCatalogUsesInstalledSharedCatalogs()
    {
        var repositoryRoot = FindRepositoryRoot();
        Expect(repositoryRoot is not null,
            "shared localization catalog test should locate the repository root");
        if (repositoryRoot is null)
        {
            return;
        }

        var catalogRoot = Path.Combine(repositoryRoot, "resources", "locales");
        var spanish = new CopperfinLocalization("es-419", catalogRoot);
        Expect(spanish.Text("Studio.OpenDialogTitle") == "Abrir activo de Copperfin",
            "standalone Studio should consume the installed Spanish shared catalog");

        var temporaryRoot = Path.Combine(Path.GetTempPath(), "copperfin-managed-catalog-test-" + Guid.NewGuid().ToString("N"));
        var temporaryLocale = Path.Combine(temporaryRoot, "es-419");
        Directory.CreateDirectory(temporaryLocale);
        var previousUiLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_LOCALE");
        var previousDirectory = Environment.GetEnvironmentVariable("COPPERFIN_LOCALE_DIR");
        try
        {
            File.WriteAllText(
                Path.Combine(temporaryLocale, "strings.json"),
                "{\"Studio.OpenDialogTitle\":\"Installed catalog override\"}");
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", null);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", "es-419");
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE_DIR", temporaryRoot);
            var fromEnvironment = CopperfinLocalization.FromEnvironment();
            Expect(fromEnvironment.Text("Studio.OpenDialogTitle") == "Installed catalog override",
                "COPPERFIN_LOCALE should select managed localization when UI locale is unset");
            Expect(fromEnvironment.Text("Studio.OpenMenu") == "&Abrir...",
                "partial installed catalogs should retain the compiled localized fallback");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousUiLocale);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", previousLocale);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE_DIR", previousDirectory);
            Directory.Delete(temporaryRoot, recursive: true);
        }
    }

    private static void TestLocalizationCatalogDiscoversInstalledStudioLayout()
    {
        var temporaryRoot = Path.Combine(
            Path.GetTempPath(),
            "copperfin-installed-layout-catalog-test-" + Guid.NewGuid().ToString("N"));
        var studioDirectory = Path.Combine(temporaryRoot, "bin", "studio");
        var localeDirectory = Path.Combine(temporaryRoot, "share", "copperfin", "locales", "es-419");
        Directory.CreateDirectory(studioDirectory);
        Directory.CreateDirectory(localeDirectory);

        try
        {
            File.WriteAllText(
                Path.Combine(localeDirectory, "strings.json"),
                "{\"Studio.OpenDialogTitle\":\"Installed package catalog\"}");
            var catalog = CopperfinExternalLocaleCatalog.LoadFromBaseDirectory(
                "es-419",
                null,
                studioDirectory);
            Expect(catalog is not null &&
                       catalog.TryGetValue("Studio.OpenDialogTitle", out var title) &&
                       title == "Installed package catalog",
                "standalone Studio should discover catalogs from the installed bin/studio layout");
        }
        finally
        {
            Directory.Delete(temporaryRoot, recursive: true);
        }
    }
}
