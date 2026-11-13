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

    private static void TestLocalizationCatalogResolvesRegionalExternalFallbacks()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin-managed-fallback-" + Guid.NewGuid().ToString("N"));
        void Write(string locale, string json)
        {
            var directory = Path.Combine(root, locale);
            Directory.CreateDirectory(directory);
            File.WriteAllText(Path.Combine(directory, "strings.json"), json);
        }

        try
        {
            Write("fr-FR", "{\"Studio.OpenDialogTitle\":\"Ouvrir un actif\"}");
            Write("de-DE", "{\"Studio.OpenDialogTitle\":\"Asset öffnen\"}");
            Write("pt", "{\"Studio.CloseMenu\":\"&Fechar\"}");
            Write("pt-PT", "{\"Studio.OpenDialogTitle\":\"Abrir recurso\"}");
            Write("zh-Hant", "{\"Studio.CloseMenu\":\"關閉\"}");
            Write("zh-Hant-TW", "{\"Studio.OpenDialogTitle\":\"開啟資產\"}");
            Write("es-419", "{\"Studio.OpenDialogTitle\":\"Abrir recurso latinoamericano\"}");
            Write("en-US", "{\"Studio.CloseMenu\":\"English installed close\"}");

            var french = new CopperfinLocalization("fr-FR", root);
            var german = new CopperfinLocalization("de-DE", root);
            var portugal = new CopperfinLocalization("pt-PT", root);
            var taiwan = new CopperfinLocalization("zh-Hant-TW", root);
            var spain = new CopperfinLocalization("es-ES", root);
            var mexico = new CopperfinLocalization("es-MX", root);
            var neutralPortuguese = new CopperfinLocalization("pt", root);
            var neutralEnglish = new CopperfinLocalization(null, root);
            var britishEnglish = new CopperfinLocalization("en-GB", root);
            var nativeHostStart = new System.Diagnostics.ProcessStartInfo();
            CopperfinStudioHostBridge.ApplyLocalizationEnvironment(nativeHostStart, neutralPortuguese);
            Expect(neutralPortuguese.Locale == "pt" &&
                   neutralPortuguese.NativeHostLocale == "pt-BR" &&
                   nativeHostStart.EnvironmentVariables["COPPERFIN_UI_LOCALE"] == "pt-BR" &&
                   nativeHostStart.EnvironmentVariables["COPPERFIN_LOCALE"] == "pt-BR" &&
                   portugal.NativeHostLocale == "pt-PT",
                "#7071: neutral Portuguese retains UI identity but forwards pt-BR to native hosts");
            Expect(french.Locale == "fr-FR" && french.Text("Studio.OpenDialogTitle") == "Ouvrir un actif",
                "#7071: a valid French regional catalog should load without a compiled locale branch");
            Expect(german.Locale == "de-DE" && german.Text("Studio.OpenDialogTitle") == "Asset öffnen",
                "#7071: a valid German regional catalog should load without a compiled locale branch");
            Expect(portugal.Locale == "pt-PT" &&
                   portugal.Text("Studio.OpenDialogTitle") == "Abrir recurso" &&
                   portugal.Text("Studio.CloseMenu") == "&Fechar" &&
                   portugal.Text("Studio.ExitMenu") == "E&xit",
                "#7071: Portugal should use exact, neutral Portuguese, then English per key");
            Expect(taiwan.Text("Studio.OpenDialogTitle") == "開啟資產" &&
                   taiwan.Text("Studio.CloseMenu") == "關閉",
                "#7071: script-bearing locale should preserve exact and script-parent catalogs");
            Expect(spain.Locale == "es-ES" &&
                   spain.Text("Studio.OpenDialogTitle") == "Open Copperfin Asset",
                "#7071/#7074: Spain should not inherit Latin American Spanish prose");
            Expect(mexico.Text("Studio.OpenDialogTitle") == "Abrir recurso latinoamericano" &&
                   mexico.Text("Studio.CloseMenu") == "&Cerrar",
                "#7071: compiled Spanish fallback should win over a later installed English catalog");
            Expect(neutralEnglish.Text("Studio.CloseMenu") == "English installed close" &&
                   britishEnglish.Text("Studio.CloseMenu") == "English installed close" &&
                   CopperfinLocalization.LocaleFallbackChain(null).SequenceEqual(
                       new[] { "en-US", "en" }) &&
                   CopperfinLocalization.LocaleFallbackChain("en-GB").SequenceEqual(
                       new[] { "en-GB", "en-US", "en" }),
                "#7071: installed English catalogs should precede the final compiled en alias");
            Expect(CopperfinLocalization.LocaleFallbackChain("es-Latn-MX").SequenceEqual(
                       new[] { "es-Latn-MX", "es-Latn", "es-419", "es", "en-US", "en" }) &&
                   CopperfinLocalization.CatalogEntries("es-MX")["Studio.ExitMenu"] == "&Salir" &&
                   CopperfinLocalization.CatalogEntries("pt")["Studio.ExitMenu"] == "&Sair" &&
                   CopperfinLocalization.CatalogKeys("es-MX").Count ==
                       CopperfinLocalization.CatalogEntries("es-419").Count &&
                   CopperfinLocalization.LocaleFallbackChain("es-Latn-ES").SequenceEqual(
                       new[] { "es-Latn-ES", "es-Latn", "es", "en-US", "en" }) &&
                   CopperfinLocalization.LocaleFallbackChain("es-Latn-419").SequenceEqual(
                       new[] { "es-Latn-419", "es-Latn", "es-419", "es", "en-US", "en" }) &&
                   CopperfinLocalization.LocaleFallbackChain("es-US").SequenceEqual(
                       new[] { "es-US", "es-419", "es", "en-US", "en" }) &&
                   CopperfinLocalization.LocaleFallbackChain("es-JP").SequenceEqual(
                       new[] { "es-JP", "es-419", "es", "en-US", "en" }) &&
                   CopperfinLocalization.NormalizeLocale(null) == CopperfinLocalization.DefaultLocale &&
                   CopperfinLocalization.NormalizeLocale("qps-ploc-XX") == CopperfinLocalization.PseudoLocale &&
                   CopperfinLocalization.NormalizeLocale("zh-cmn-hANS-cn") == "zh-cmn-Hans-CN" &&
                   CopperfinLocalization.NormalizeLocale("fr/../es-419") == "en-US",
                "#7071: fallback is bounded and malformed filesystem-like tags cannot become catalog paths");
        }
        finally
        {
            Directory.Delete(root, recursive: true);
        }
    }
}
