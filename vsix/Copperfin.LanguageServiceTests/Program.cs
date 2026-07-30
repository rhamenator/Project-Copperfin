// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private const string HoldOutputHandlesArgument = "--copperfin-test-hold-output-handles";
    private const string HoldOutputTreeArgument = "--copperfin-test-hold-output-tree";
    private static int failures;

    private static int Main(string[] args)
    {
        if (args.Length == 2 && string.Equals(args[0], HoldOutputHandlesArgument, StringComparison.Ordinal))
        {
            File.WriteAllText(args[1], Environment.ProcessId.ToString(CultureInfo.InvariantCulture));
            Thread.Sleep(TimeSpan.FromSeconds(30));
            return 0;
        }
        if (args.Length == 3 && string.Equals(args[0], HoldOutputTreeArgument, StringComparison.Ordinal))
        {
            File.WriteAllText(args[1], Environment.ProcessId.ToString(CultureInfo.InvariantCulture));
            using var grandchild = Process.Start(CreateManagedTestChildStartInfo(HoldOutputHandlesArgument, args[2]));
            if (grandchild is null)
            {
                return 97;
            }
            grandchild.WaitForExit();
            return grandchild.ExitCode;
        }

        TestLocalizationCatalogNormalizesSpanishAndPortugueseLocales();
        TestLocalizationCatalogSupportsPseudoLocale();
        TestLocalizationCatalogFallsBackToEnglish();
        TestLocalizationCatalogKeepsSupportedLocaleKeysAligned();
        TestVsixCommandTableLocalizesCommandCaptions();
        TestVsixLongLivedSurfacesRefreshLocalization();
        TestVsixEditorHostThemeContract();
        TestVsixDebuggerRestartInvalidatesStaleSessions();
        TestVsixCommandWindowRegistration();
        TestVsixInstalledProductRegistrationLocalizesMetadata();
        TestVsixEditorRegistrationLocalizesName();
        TestVsixEditorPaneUsesCurrentUiCulture();
        TestReportLayoutObjectPreservesNullableSectionIndex();
        TestReportSectionsPreserveNullableOrdinalMetadata();
        TestHostSnapshotPreservesUnplacedReportLayoutObjects();
        TestSelectIntelliSenseDescriptionsLocalizeWithoutChangingIdentity();
        TestCompletionSetDisplayNameLocalizesWithoutChangingIdentity();
        TestIntelliSenseUsesCurrentUiCultureWhenEnvironmentIsUnset();
        TestLocalizationCatalogFormatsWithInvariantCulture();
        TestLocalizationCatalogFormatsUnexpectedEditorFailure();
        TestStudioOpenDialogFilterPreservesInvariantPatterns();
        TestLocalizationCatalogLocalizesCommandBootstrapErrors();
        TestLocalizationCatalogUsesInstalledSharedCatalogs();
        TestLocalizationCatalogDiscoversInstalledStudioLayout();
        TestLocalizationCatalogLocalizesStudioAssetKinds();
        TestReportLayoutObjectPreservesPictureMetadata();
        TestDesignerSelectionExposesPictureForLabelAndReportExpressionObjects();
        TestDesignerSelectionExposesReportControlBehaviorProperties();
        TestDesignerSelectionDefaultsToEnvironmentLocalization();
        TestDesignerSelectionHonorsReadOnlyDocumentState();
        TestStudioHostProcessStartInfoKeepsExecutableLaunchArguments();
        TestStudioHostProcessStartInfoUsesUtf8ForRedirectedStreams();
        TestStudioStartupArgumentsPreserveSelectors();
        TestStudioStartupArgumentsRejectMalformedSelectors();
        TestStudioHostBuildArgumentsPreserveStartupSelectors();
        TestStudioHostProcessStartInfoWrapsWindowsBatchHosts();
        TestStudioHostProcessStartInfoAppliesExplicitLocalizationEnvironment();
        TestStudioHostBatchArgumentsKeepVisualStudioProvenance();
        TestStudioTargetSelectionPrefersSelectedItemsForItemCommands();
        TestProjectSelectionResolvesContainingProjectForActiveAssets();
        TestManagedHostResolutionHonorsEnvironmentOverrides();
        TestManagedHostResolutionAppliesConfiguredPosixExecutePolicy();
        TestManagedHostResolutionUsesPlatformNativeCandidateRules();
        TestManagedHostResolutionChecksRealPosixExecutePermission();
        TestManagedHostResolutionFindsSiblingAndRepoBuildLayouts();
        TestStudioHostLaunchContainsStartupFailures();
        TestDocumentPathIdentityUsesFilesystemSemantics();
        TestLocalizationCatalogDoesNotLeakMachineSpecificHostPaths();
        TestProjectWorkflowThreadsExplicitLocaleToBuildHostOnPosix();
        TestProjectWorkflowUsesDistinctOutputDirectoriesForBackToBackBuildsOnPosix();
        TestRuntimeDebugClientThreadsExplicitLocaleToRuntimeHostOnPosix();
        TestRuntimeDebugClientCleansTransientReplayManifestOnPosix();
        TestRuntimeDebugParserUnescapesEscapedLineValues();
        TestRuntimeDebugFallbackErrorLocalizesWithoutChangingPrecedence();
        TestRuntimeDebugTransportFailuresLocalizeAcrossSupportedLocales();
        TestProcessRunnerCapturesLargeConcurrentOutput();
        TestProcessRunnerPreservesSuccessfulExitWhenDescendantHoldsPipe();
        TestProcessRunnerEnforcesTimeoutWithoutPipeDeadlock();
        TestDottedClassMemberResolvesToLongestProjectSymbolPrefix();
        TestDottedMemberFallsBackToTrailingProcedureName();
        TestQuickInfoUsesResolvedProjectSymbolDescriptionForDottedMemberAccess();
        TestProjectProcedureSignatureHelpUsesLparameters();
        TestProjectProcedureSignatureHelpPreservesNestedDefaultExpressions();
        TestProjectSignatureHelpUsesInlineRoutineParameters();
        TestProjectLanguageServiceRecognizesProcAbbreviation();
        TestProjectLanguageServiceDiscoversVisibilityQualifiedMethods();
        TestProjectProcedureSignatureHelpUsesSingularLparameterForDottedMethod();
        TestProjectProcedureSignatureHelpRejectsBareParameter();
        TestProjectProcedureSignatureHelpFallsBackFromDottedInvocation();
        TestSignatureInvocationParserIgnoresCommentsAndStrings();
        TestProjectInsightsCollectDirectAndDottedProcedureCallReferences();
        TestProjectInsightsRejectPathsOutsideProjectRoot();
        TestRenamePreviewCollectsDefinitionAndNormalizedReferences();
        TestProjectInsightsCollectQualifiedAndInstanceStyleMethodCallReferences();
        TestRenamePreviewCollectsProjectMethodDefinitionAndReferences();
        TestCompletionCatalogIngestsCreateCursorAndIntoCursorAliases();
        TestCompletionCatalogIngestsImplicitUseAndSqlExecAliases();
        TestSelectContextKeepsAliasCompletionsAheadOfGlobalProcedureSymbols();
        TestQualifiedProjectMethodSignatureHelpAndDefinition();
        TestMemberAccessCompletionsIncludeProjectMethodsAheadOfGenericMembers();
        TestInstanceStyleProjectMethodFallbackUsesUniqueTrailingMethodName();
        TestInstanceStyleProjectMethodFallbackAvoidsAmbiguousMatches();
        TestUnquotedIncludesFeedRecursiveDefineResolution();
        TestExternalMixedCaseIncludesResolveUniqueFilesystemSpelling();
        TestIncludedHeaderOutsideProjectRootFeedsDefineResolution();
        TestCrossFileProjectBoundaryResolvesProcedureDefinition();
        TestCrossFileProjectBoundaryCompletions();
        TestLanguageServiceMetadataLocalizesThroughCatalogs();

        if (failures != 0)
        {
            Console.Error.WriteLine($"{failures} language-service test(s) failed.");
            return 1;
        }

        Console.WriteLine("All language-service tests passed.");
        return 0;
    }

    private static void TestLocalizationCatalogNormalizesSpanishAndPortugueseLocales()
    {
        var spanish = new CopperfinLocalization("es-MX");
        Expect(spanish.Locale == CopperfinLocalization.SpanishLatinAmericaLocale,
            "Spanish regional locales should normalize to the Latin America catalog");
        Expect(spanish.Text("Studio.OpenDialogTitle") == "Abrir activo de Copperfin",
            "Spanish catalog should localize the standalone Studio open dialog title");
        Expect(spanish.Text("Studio.OpenDocumentStatus").Contains("Pestañas abiertas", StringComparison.Ordinal),
            "Spanish catalog should localize standalone Studio tab status text");

        var portuguese = new CopperfinLocalization("pt");
        Expect(portuguese.Locale == CopperfinLocalization.PortugueseBrazilLocale,
            "Portuguese neutral locale should normalize to the Brazilian Portuguese catalog");
        Expect(portuguese.Text("Studio.OpenDialogTitle") == "Abrir ativo do Copperfin",
            "Portuguese catalog should localize the standalone Studio open dialog title");
        Expect(portuguese.Text("Studio.OpenDocumentStatus").Contains("Abas abertas", StringComparison.Ordinal),
            "Portuguese catalog should localize standalone Studio tab status text");
    }

    private static void TestLocalizationCatalogFallsBackToEnglish()
    {
        var unsupported = new CopperfinLocalization("de-DE");
        Expect(unsupported.Locale == CopperfinLocalization.DefaultLocale,
            "unsupported locales should normalize to English");
        Expect(unsupported.Text("Studio.OpenDialogTitle") == "Open Copperfin Asset",
            "unsupported locales should use the English catalog");

        var spanish = new CopperfinLocalization("es-419");
        Expect(spanish.Text("Missing.Key") == "Missing.Key",
            "missing localization keys should fall back to the stable key instead of returning blank text");
    }

    private static void TestLocalizationCatalogFormatsUnexpectedEditorFailure()
    {
        const string diagnostic = "fixture failure";
        var english = new CopperfinLocalization("en-US").Format(
            "AssetEditor.Dialog.UnexpectedFailure", diagnostic);
        var spanish = new CopperfinLocalization("es-419").Format(
            "AssetEditor.Dialog.UnexpectedFailure", diagnostic);
        var portuguese = new CopperfinLocalization("pt-BR").Format(
            "AssetEditor.Dialog.UnexpectedFailure", diagnostic);
        var pseudo = new CopperfinLocalization("qps-ploc").Format(
            "AssetEditor.Dialog.UnexpectedFailure", diagnostic);

        Expect(english == "The editor operation failed: fixture failure",
            "English unexpected-editor failure wrapper should preserve the diagnostic placeholder");
        Expect(spanish.StartsWith("La operación del editor falló:", StringComparison.Ordinal) &&
               spanish.EndsWith(diagnostic, StringComparison.Ordinal),
            "Spanish unexpected-editor failure wrapper should localize its surrounding text");
        Expect(portuguese.StartsWith("A operação do editor falhou:", StringComparison.Ordinal) &&
               portuguese.EndsWith(diagnostic, StringComparison.Ordinal),
            "Portuguese unexpected-editor failure wrapper should localize its surrounding text");
        Expect(pseudo.Contains(diagnostic, StringComparison.Ordinal) &&
               pseudo.StartsWith("[!! ", StringComparison.Ordinal),
            "pseudo-localized unexpected-editor failure wrapper should preserve and decorate the diagnostic");
    }

    private static void TestLocalizationCatalogSupportsPseudoLocale()
    {
        var pseudo = new CopperfinLocalization("qps-ploc");
        Expect(pseudo.Locale == CopperfinLocalization.PseudoLocale,
            "qps-ploc should normalize to the shared pseudo-localization catalog");

        var english = new CopperfinLocalization("en-US");
        var pseudoTitle = pseudo.Text("Studio.OpenDialogTitle");
        Expect(!string.Equals(pseudoTitle, english.Text("Studio.OpenDialogTitle"), StringComparison.Ordinal),
            "qps-ploc should not fall back to raw English for catalog-backed text");
        Expect(pseudoTitle.StartsWith("[!! ", StringComparison.Ordinal) &&
               pseudoTitle.EndsWith(" !!]", StringComparison.Ordinal),
            "qps-ploc should decorate catalog-backed text so hard-coded UI stands out");

        var formatted = pseudo.Format("Studio.OpenDocumentStatus", @"C:\demo\orders.scx", "Visual form", 3);
        Expect(formatted.Contains(@"C:\demo\orders.scx", StringComparison.Ordinal),
            "pseudo-localized formatting should preserve path placeholders");
        Expect(formatted.Contains("Visual form", StringComparison.Ordinal),
            "pseudo-localized formatting should preserve dynamic string arguments");
        Expect(formatted.Contains("3", StringComparison.Ordinal),
            "pseudo-localized formatting should preserve numeric placeholders");
    }

    private static void TestLocalizationCatalogKeepsSupportedLocaleKeysAligned()
    {
        var englishKeys = new HashSet<string>(CopperfinLocalization.CatalogKeys(CopperfinLocalization.DefaultLocale));
        var english = new CopperfinLocalization(CopperfinLocalization.DefaultLocale);
        foreach (var locale in CopperfinLocalization.SupportedLocales)
        {
            var keys = new HashSet<string>(CopperfinLocalization.CatalogKeys(locale));
            Expect(keys.SetEquals(englishKeys), $"{locale} catalog should expose the same key set as English");

            var localization = new CopperfinLocalization(locale);
            foreach (var key in englishKeys)
            {
                var localizedText = localization.Text(key);
                Expect(!string.IsNullOrWhiteSpace(localizedText), $"{locale} catalog value for {key} should not be blank");
                Expect(
                    CompositeFormatPlaceholders(localizedText).SequenceEqual(
                        CompositeFormatPlaceholders(english.Text(key))),
                    $"{locale} catalog value for {key} should preserve the English placeholder set");
            }
        }
    }

    private static void TestSelectIntelliSenseDescriptionsLocalizeWithoutChangingIdentity()
    {
        const string key = "LanguageService.IntelliSense.Keyword.Select";
        const string englishText =
            "Command: selects a work area or evaluates a SELECT() call depending on context.";
        const string spanishText =
            "Comando: selecciona un área de trabajo o evalúa una llamada a SELECT() según el contexto.";
        const string portugueseText =
            "Comando: seleciona uma área de trabalho ou avalia uma chamada a SELECT() conforme o contexto.";

        var english = new CopperfinLocalization("en-US");
        var spanish = new CopperfinLocalization("es-419");
        var portuguese = new CopperfinLocalization("pt-BR");
        var pseudo = new CopperfinLocalization("qps-ploc");
        Expect(english.Text(key) == englishText,
            "English SELECT IntelliSense description should remain unchanged");
        Expect(spanish.Text(key) == spanishText && spanish.Text(key) != englishText,
            "es-419 SELECT IntelliSense description should be translated");
        Expect(portuguese.Text(key) == portugueseText && portuguese.Text(key) != englishText,
            "pt-BR SELECT IntelliSense description should be translated");
        Expect(pseudo.Text(key) != englishText && pseudo.Text(key).StartsWith("[!! ", StringComparison.Ordinal),
            "qps-ploc SELECT IntelliSense description should remain distinct from English");

        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        try
        {
            foreach (var localeCase in new[]
            {
                (Locale: "es-419", Description: spanishText),
                (Locale: "pt-BR", Description: portugueseText),
                (Locale: "qps-ploc", Description: pseudo.Text(key))
            })
            {
                Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", localeCase.Locale);
                var selectCompletion = FoxProIntelliSenseCatalog
                    .BuildEntries(null, string.Empty, "SEL")
                    .SingleOrDefault(entry => entry.DisplayText == "SELECT");
                Expect(selectCompletion is not null,
                    $"{localeCase.Locale} completion catalog should retain the invariant SELECT entry");
                if (selectCompletion is not null)
                {
                    Expect(selectCompletion.Description == localeCase.Description,
                        $"{localeCase.Locale} SELECT completion should consume the shared catalog description");
                    Expect(selectCompletion.Kind == "keyword",
                        $"{localeCase.Locale} SELECT completion kind should remain invariant");
                }

                Expect(FoxProIntelliSenseCatalog.DescribeToken("SELECT") == localeCase.Description,
                    $"{localeCase.Locale} SELECT Quick Info should consume the shared catalog description");
            }
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousLocale);
        }
    }

    private static void TestCompletionSetDisplayNameLocalizesWithoutChangingIdentity()
    {
        var english = new CopperfinLocalization("en-US");
        var spanish = new CopperfinLocalization("es-419");
        var portuguese = new CopperfinLocalization("pt-BR");
        var pseudo = new CopperfinLocalization("qps-ploc");

        Expect(FoxProCompletionSetContract.Identity == "CopperfinFoxPro",
            "FoxPro completion-set identity must remain machine-invariant");
        Expect(FoxProCompletionSetContract.GetDisplayName(english) == "Copperfin FoxPro",
            "English FoxPro completion-set display name should remain unchanged");
        Expect(FoxProCompletionSetContract.GetDisplayName(spanish) == "FoxPro de Copperfin",
            "es-419 FoxPro completion-set display name should use the catalog");
        Expect(FoxProCompletionSetContract.GetDisplayName(portuguese) == "FoxPro do Copperfin",
            "pt-BR FoxPro completion-set display name should use the catalog");

        var pseudoText = FoxProCompletionSetContract.GetDisplayName(pseudo);
        Expect(pseudoText.StartsWith("[!! ", StringComparison.Ordinal) &&
               pseudoText.EndsWith(" !!]", StringComparison.Ordinal),
            "qps-ploc FoxPro completion-set display name should be pseudo-localized");
        Expect(pseudoText != FoxProCompletionSetContract.GetDisplayName(english),
            "qps-ploc FoxPro completion-set display name should not fall back to English");
    }

    private static void TestIntelliSenseUsesCurrentUiCultureWhenEnvironmentIsUnset()
    {
        var previousUiLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_LOCALE");
        var previousCulture = CultureInfo.CurrentUICulture;
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", null);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", null);
            CultureInfo.CurrentUICulture = new CultureInfo("es-419");

            var completions = FoxProIntelliSenseCatalog.BuildEntries(null, string.Empty, "SEL");
            var selectCompletion = completions.Single(entry => entry.DisplayText == "SELECT");
            Expect(selectCompletion.Description == "Comando: selecciona un área de trabajo o evalúa una llamada a SELECT() según el contexto.",
                "Visual Studio IntelliSense completions should follow CurrentUICulture when no explicit locale override exists");

            Expect(FoxProIntelliSenseCatalog.DescribeToken("SELECT") == selectCompletion.Description,
                "Visual Studio quick info should use the same CurrentUICulture as completion descriptions");

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(null, "MESSAGEBOX");
            Expect(signatures.Count == 1 &&
                   signatures[0].Documentation == "Muestra un cuadro de diálogo modal y devuelve el botón presionado.",
                "Visual Studio signature help should follow CurrentUICulture when no explicit locale override exists");
            Expect(selectCompletion.InsertionText == "SELECT" && selectCompletion.Kind == "keyword",
                "Visual Studio IntelliSense syntax identity must remain invariant across UI cultures");
        }
        finally
        {
            CultureInfo.CurrentUICulture = previousCulture;
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousUiLocale);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", previousLocale);
        }
    }

    private static IReadOnlyList<string> CompositeFormatPlaceholders(string text)
    {
        var placeholders = new List<string>();
        for (var index = 0; index < text.Length; index++)
        {
            if (text[index] != '{')
            {
                continue;
            }
            if (index + 1 < text.Length && text[index + 1] == '{')
            {
                index++;
                continue;
            }

            var end = text.IndexOf('}', index + 1);
            if (end < 0)
            {
                break;
            }

            var token = text.Substring(index + 1, end - index - 1);
            var delimiter = token.IndexOfAny(new[] { ',', ':' });
            var argumentIndex = (delimiter >= 0 ? token.Substring(0, delimiter) : token).Trim();
            if (argumentIndex.Length > 0 && argumentIndex.All(char.IsDigit))
            {
                placeholders.Add(argumentIndex);
            }
            index = end;
        }
        return placeholders.OrderBy(value => value, StringComparer.Ordinal).ToList();
    }

    private static void TestLocalizationCatalogFormatsWithInvariantCulture()
    {
        var portuguese = new CopperfinLocalization("pt-BR");
        var status = portuguese.Format("Studio.OpenDocumentStatus", @"C:\demo\orders.scx", "Visual form", 3);
        Expect(status.Contains(@"C:\demo\orders.scx", StringComparison.Ordinal),
            "localized formatting should preserve path arguments");
        Expect(status.Contains("Visual form", StringComparison.Ordinal),
            "localized formatting should preserve asset kind arguments");
        Expect(status.Contains("Abas abertas: 3", StringComparison.Ordinal),
            "localized formatting should preserve numeric arguments with invariant formatting");
    }

    private static void TestStudioOpenDialogFilterPreservesInvariantPatterns()
    {
        var english = new CopperfinLocalization("en-US");
        var spanish = new CopperfinLocalization("es-419");
        var portuguese = new CopperfinLocalization("pt-BR");
        var pseudo = new CopperfinLocalization("qps-ploc");

        foreach (var localization in new[] { english, spanish, portuguese, pseudo })
        {
            var filter = CopperfinStudioOpenDialogFilter.Build(localization);
            var segments = filter.Split('|');
            Expect(segments.Length == 4,
                $"{localization.Locale} Studio open filter should retain WinForms description/pattern pairs");
            Expect(segments[1] == CopperfinStudioOpenDialogFilter.AssetPatterns &&
                   segments[3] == CopperfinStudioOpenDialogFilter.AllFilesPattern,
                $"{localization.Locale} Studio open filter patterns must remain invariant");
        }

        var pseudoFilter = CopperfinStudioOpenDialogFilter.Build(pseudo);
        var pseudoSegments = pseudoFilter.Split('|');
        Expect(pseudoSegments[0].StartsWith("[!! ", StringComparison.Ordinal) &&
               pseudoSegments[0].EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoSegments[2].StartsWith("[!! ", StringComparison.Ordinal) &&
               pseudoSegments[2].EndsWith(" !!]", StringComparison.Ordinal),
            "qps-ploc Studio open filter descriptions should remain pseudo-localized");
    }

    private static void TestLocalizationCatalogLocalizesCommandBootstrapErrors()
    {
        var english = new CopperfinLocalization("en-US");
        Expect(english.Text("AssetEditor.Error.MenuCommandServiceUnavailable") == "Unable to get menu command service.",
            "English catalog should preserve the VSIX menu-command-service error text");

        var spanish = new CopperfinLocalization("es-MX");
        Expect(spanish.Text("AssetEditor.Error.MenuCommandServiceUnavailable") == "No se pudo obtener el servicio de comandos del menú.",
            "Spanish catalog should localize the VSIX menu-command-service error text");

        var portuguese = new CopperfinLocalization("pt");
        Expect(portuguese.Text("AssetEditor.Error.MenuCommandServiceUnavailable") == "Não foi possível obter o serviço de comandos do menu.",
            "Portuguese catalog should localize the VSIX menu-command-service error text");

        var pseudo = new CopperfinLocalization("qps-ploc");
        var pseudoText = pseudo.Text("AssetEditor.Error.MenuCommandServiceUnavailable");
        Expect(!string.Equals(pseudoText, english.Text("AssetEditor.Error.MenuCommandServiceUnavailable"), StringComparison.Ordinal),
            "qps-ploc should not fall back to raw English for VSIX menu-command-service error text");
        Expect(pseudoText.StartsWith("[!! ", StringComparison.Ordinal) &&
               pseudoText.EndsWith(" !!]", StringComparison.Ordinal),
            "qps-ploc should decorate the VSIX menu-command-service error text");
    }

    private static void TestLocalizationCatalogLocalizesStudioAssetKinds()
    {
        var english = new CopperfinLocalization("en-US");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.pjx", english) == "Visual project",
            "English catalog should preserve the Studio project asset-kind label");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.scx", english) == "Visual form",
            "English catalog should preserve the Studio form asset-kind label");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.vcx", english) == "Visual class library",
            "English catalog should preserve the Studio class-library asset-kind label");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.frx", english) == "Visual report",
            "English catalog should preserve the Studio report asset-kind label");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.lbx", english) == "Visual label",
            "English catalog should preserve the Studio label asset-kind label");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.mnx", english) == "Visual menu",
            "English catalog should preserve the Studio menu asset-kind label");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.txt", english) == "Copperfin asset",
            "English catalog should preserve the generic Studio asset-kind label");

        var spanish = new CopperfinLocalization("es-MX");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.frx", spanish) == "Informe visual",
            "Spanish catalog should localize report asset-kind labels");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.lbx", spanish) == "Etiqueta visual",
            "Spanish catalog should localize label asset-kind labels");

        var portuguese = new CopperfinLocalization("pt");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.frx", portuguese) == "Relatório visual",
            "Portuguese catalog should localize report asset-kind labels");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.txt", portuguese) == "Ativo Copperfin",
            "Portuguese catalog should localize generic asset-kind labels");

        var unsupported = new CopperfinLocalization("de-DE");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("customer.frx", unsupported) == "Visual report",
            "unsupported locales should keep English asset-kind fallback labels");
    }

    private static void TestDesignerSelectionExposesPictureForLabelAndReportExpressionObjects()
    {
        CopperfinStudioSnapshotObject Snapshot(string objectType) => new()
        {
            Properties = new List<CopperfinStudioSnapshotProperty>
            {
                new() { Name = "OBJTYPE", Value = objectType },
                new() { Name = "PICTURE", Value = "@J" },
                new() { Name = "RULERLINES", Value = "4" },
                new() { Name = "OFFSET", Value = "1" },
                new() { Name = "FILLCHAR", Value = "N" },
                new() { Name = "TOTALTYPE", Value = "2" },
                new() { Name = "RESETTOTAL", Value = "1" },
                new() { Name = "SPACING", Value = "1" },
                new() { Name = "GENERAL", Value = "1" },
                new() { Name = "TAG2", Value = "Hover tip" }
            }
        };

        var labelSelection = CopperfinDesignerSelection.FromSnapshot(
            "label",
            Snapshot("5"),
            new CopperfinLocalization("en-US"));
        var reportExpressionSelection = CopperfinDesignerSelection.FromSnapshot(
            "report",
            Snapshot("8"),
            new CopperfinLocalization("en-US"));
        var imageSelection = CopperfinDesignerSelection.FromSnapshot(
            "report",
            Snapshot("17"),
            new CopperfinLocalization("en-US"));

        var labelPictureProperty = labelSelection?.GetProperties().Find("PICTURE", false);
        var reportPictureProperty = reportExpressionSelection?.GetProperties().Find("PICTURE", false);
        var imagePictureProperty = imageSelection?.GetProperties().Find("PICTURE", false);
        var reportRulerLinesProperty = reportExpressionSelection?.GetProperties().Find("RULERLINES", false);
        var labelRulerLinesProperty = labelSelection?.GetProperties().Find("RULERLINES", false);
        var imageRulerLinesProperty = imageSelection?.GetProperties().Find("RULERLINES", false);
        var reportOffsetProperty = reportExpressionSelection?.GetProperties().Find("OFFSET", false);
        var labelOffsetProperty = labelSelection?.GetProperties().Find("OFFSET", false);
        var imageOffsetProperty = imageSelection?.GetProperties().Find("OFFSET", false);
        var reportExpressionDataTypeProperty = reportExpressionSelection?.GetProperties().Find("FILLCHAR", false);
        var labelDataTypeProperty = labelSelection?.GetProperties().Find("FILLCHAR", false);
        var imageDataTypeProperty = imageSelection?.GetProperties().Find("FILLCHAR", false);
        var reportTotalTypeProperty = reportExpressionSelection?.GetProperties().Find("TOTALTYPE", false);
        var labelTotalTypeProperty = labelSelection?.GetProperties().Find("TOTALTYPE", false);
        var imageTotalTypeProperty = imageSelection?.GetProperties().Find("TOTALTYPE", false);
        var reportResetTotalProperty = reportExpressionSelection?.GetProperties().Find("RESETTOTAL", false);
        var labelResetTotalProperty = labelSelection?.GetProperties().Find("RESETTOTAL", false);
        var imageResetTotalProperty = imageSelection?.GetProperties().Find("RESETTOTAL", false);
        var labelSpacingProperty = labelSelection?.GetProperties().Find("SPACING", false);
        var reportSpacingProperty = reportExpressionSelection?.GetProperties().Find("SPACING", false);
        var imageSpacingProperty = imageSelection?.GetProperties().Find("SPACING", false);
        var imageGeneralProperty = imageSelection?.GetProperties().Find("GENERAL", false);
        var reportGeneralProperty = reportExpressionSelection?.GetProperties().Find("GENERAL", false);
        var labelGeneralProperty = labelSelection?.GetProperties().Find("GENERAL", false);
        var reportTooltipProperty = reportExpressionSelection?.GetProperties().Find("TAG2", false);
        var labelTooltipProperty = labelSelection?.GetProperties().Find("TAG2", false);
        var imageTooltipProperty = imageSelection?.GetProperties().Find("TAG2", false);
        Expect(labelPictureProperty is not null && reportPictureProperty is not null && imagePictureProperty is not null,
            "label, report-expression, and image selections should expose editable PICTURE");
        Expect(labelPictureProperty?.DisplayName == "Picture" && reportPictureProperty?.DisplayName == "Picture" &&
               imagePictureProperty?.DisplayName == "Picture",
            "label, report-expression, and image PICTURE should use the localized property label");
        Expect(reportRulerLinesProperty is not null && labelRulerLinesProperty is null && imageRulerLinesProperty is null &&
               reportRulerLinesProperty.DisplayName == "String Trimming",
            "only report-expression selections should expose the localized RULERLINES property");
        Expect(reportOffsetProperty is not null && labelOffsetProperty is null && imageOffsetProperty is not null &&
               reportOffsetProperty.DisplayName == "Expression Alignment",
            "report-expression and image selections should expose their distinct localized OFFSET properties");
        Expect(imageOffsetProperty?.DisplayName == "Image Source Mode",
            "image selections should use the localized image-source OFFSET label");
        Expect(reportExpressionDataTypeProperty is not null && labelDataTypeProperty is null && imageDataTypeProperty is null &&
               reportExpressionDataTypeProperty.DisplayName == "Expression Data Type",
            "only report-expression selections should expose the localized FILLCHAR property");
        Expect(reportTotalTypeProperty is not null && labelTotalTypeProperty is null && imageTotalTypeProperty is null &&
               reportTotalTypeProperty.DisplayName == "Calculation Type",
            "only report-expression selections should expose the localized TOTALTYPE property");
        Expect(reportResetTotalProperty is not null && labelResetTotalProperty is null && imageResetTotalProperty is null &&
               reportResetTotalProperty.DisplayName == "Reset Total",
            "only report-expression selections should expose the localized RESETTOTAL property");
        Expect(labelSpacingProperty is not null && reportSpacingProperty is null && imageSpacingProperty is null &&
               labelSpacingProperty.DisplayName == "Line Spacing",
            "only label selections should expose the localized SPACING property");
        Expect(imageGeneralProperty is not null && reportGeneralProperty is null && labelGeneralProperty is null &&
               imageGeneralProperty.DisplayName == "Image Scale Mode",
            "only image selections should expose the localized GENERAL property");
        Expect(reportTooltipProperty is not null && labelTooltipProperty is not null && imageTooltipProperty is not null &&
               reportTooltipProperty.DisplayName == "ToolTip Text" &&
               labelTooltipProperty.DisplayName == "ToolTip Text" &&
               imageTooltipProperty.DisplayName == "ToolTip Text",
            "report-control selections should expose the localized TAG2 tooltip property");
        reportPictureProperty?.SetValue(reportExpressionSelection, "@N");
        Expect(reportExpressionSelection?.TryGetUpdate("PICTURE", out var pictureTarget, out var pictureValue) == true &&
               pictureTarget == "PICTURE" && pictureValue == "@N",
            "report-expression PICTURE edits should preserve the invariant formatting-field update target");
        imagePictureProperty?.SetValue(imageSelection, "images\\logo.bmp");
        Expect(imageSelection?.TryGetUpdate("PICTURE", out var imagePictureTarget, out var imagePictureValue) == true &&
               imagePictureTarget == "PICTURE" && imagePictureValue == "images\\logo.bmp",
            "image PICTURE edits should preserve the invariant source-field update target");
        reportRulerLinesProperty?.SetValue(reportExpressionSelection, 2);
        Expect(reportExpressionSelection?.TryGetUpdate("RULERLINES", out var rulerLinesTarget, out var rulerLinesValue) == true &&
               rulerLinesTarget == "RULERLINES" && rulerLinesValue == "2",
            "report-expression RULERLINES edits should preserve the invariant update target");
        reportRulerLinesProperty?.SetValue(reportExpressionSelection, string.Empty);
        Expect(reportExpressionSelection?.TryGetMutation("RULERLINES", out var clearedRulerLinesTarget, out var clearedRulerLinesValue, out var clearRulerLines) == true &&
               clearedRulerLinesTarget == "RULERLINES" &&
               clearedRulerLinesValue == string.Empty &&
               clearRulerLines,
            "report-expression empty numeric edits should preserve explicit clear intent instead of serializing zero");
        reportOffsetProperty?.SetValue(reportExpressionSelection, 2);
        Expect(reportExpressionSelection?.TryGetUpdate("OFFSET", out var offsetTarget, out var offsetValue) == true &&
               offsetTarget == "OFFSET" && offsetValue == "2",
            "report-expression OFFSET edits should preserve the invariant update target");
        reportExpressionDataTypeProperty?.SetValue(reportExpressionSelection, "D");
        Expect(reportExpressionSelection?.TryGetUpdate("FILLCHAR", out var dataTypeTarget, out var dataTypeValue) == true &&
               dataTypeTarget == "FILLCHAR" && dataTypeValue == "D",
            "report-expression FILLCHAR edits should preserve the invariant update target");
        reportTotalTypeProperty?.SetValue(reportExpressionSelection, 3);
        Expect(reportExpressionSelection?.TryGetUpdate("TOTALTYPE", out var totalTypeTarget, out var totalTypeValue) == true &&
               totalTypeTarget == "TOTALTYPE" && totalTypeValue == "3",
            "report-expression TOTALTYPE edits should preserve the invariant update target");
        reportResetTotalProperty?.SetValue(reportExpressionSelection, 2);
        Expect(reportExpressionSelection?.TryGetUpdate("RESETTOTAL", out var resetTotalTarget, out var resetTotalValue) == true &&
               resetTotalTarget == "RESETTOTAL" && resetTotalValue == "2",
            "report-expression RESETTOTAL edits should preserve the invariant update target");
        labelSpacingProperty?.SetValue(labelSelection, 2);
        Expect(labelSelection?.TryGetUpdate("SPACING", out var spacingTarget, out var spacingValue) == true &&
               spacingTarget == "SPACING" && spacingValue == "2",
            "label SPACING edits should preserve the invariant update target");
        imageGeneralProperty?.SetValue(imageSelection, 2);
        Expect(imageSelection?.TryGetUpdate("GENERAL", out var generalTarget, out var generalValue) == true &&
               generalTarget == "GENERAL" && generalValue == "2",
            "image GENERAL edits should preserve the invariant update target");
        imageOffsetProperty?.SetValue(imageSelection, 2);
        Expect(imageSelection?.TryGetUpdate("OFFSET", out var imageOffsetTarget, out var imageOffsetValue) == true &&
               imageOffsetTarget == "OFFSET" && imageOffsetValue == "2",
            "image OFFSET edits should preserve the invariant update target");
        reportTooltipProperty?.SetValue(reportExpressionSelection, "Updated tip");
        Expect(reportExpressionSelection?.TryGetUpdate("TAG2", out var tooltipTarget, out var tooltipValue) == true &&
               tooltipTarget == "TAG2" && tooltipValue == "Updated tip",
            "report-control TAG2 edits should preserve the invariant memo-backed update target");
    }

    private static void TestDesignerSelectionExposesReportControlBehaviorProperties()
    {
        CopperfinStudioSnapshotObject Snapshot(
            string objectType,
            string floatValue,
            string noRepeatValue,
            string stretchValue = "true",
            string stretchTopValue = "false",
            string printWhenValue = "amount > 0",
            string printWhenGroupValue = "6",
            string printWhenRepeatedValue = "true",
            string printWhenValueChangesValue = "false",
            string printWhenNewPageColumnValue = "0",
            string printWhenOverflowValue = "false",
            string bottomValue = "false",
            string topValue = "false",
            string modeValue = "0",
            string fillCharValue = "N",
            string totalTypeValue = "2") => new()
        {
            Properties = new List<CopperfinStudioSnapshotProperty>
            {
                new() { Name = "OBJTYPE", Value = objectType },
                new() { Name = "SUPEXPR", Value = printWhenValue },
                new() { Name = "SUPGROUP", Value = printWhenGroupValue },
                new() { Name = "SUPALWAYS", Value = printWhenRepeatedValue },
                new() { Name = "SUPVALCHNG", Value = printWhenValueChangesValue },
                new() { Name = "SUPRPCOL", Value = printWhenNewPageColumnValue },
                new() { Name = "SUPOVFLOW", Value = printWhenOverflowValue },
                new() { Name = "BOTTOM", Value = bottomValue },
                new() { Name = "TOP", Value = topValue },
                new() { Name = "MODE", Value = modeValue },
                new() { Name = "FILLCHAR", Value = fillCharValue },
                new() { Name = "TOTALTYPE", Value = totalTypeValue },
                new() { Name = "FLOAT", Value = floatValue },
                new() { Name = "NOREPEAT", Value = noRepeatValue },
                new() { Name = "STRETCH", Value = stretchValue },
                new() { Name = "STRETCHTOP", Value = stretchTopValue }
            }
        };

        foreach (var assetFamily in new[] { "report", "label" })
        {
            var selection = CopperfinDesignerSelection.FromSnapshot(
                assetFamily,
                Snapshot("8", "true", "false"),
                new CopperfinLocalization("en-US"));
            var properties = selection?.GetProperties();
            var floatProperty = properties?.Find("FLOAT", false);
            var noRepeatProperty = properties?.Find("NOREPEAT", false);
            var stretchProperty = properties?.Find("STRETCH", false);
            var stretchTopProperty = properties?.Find("STRETCHTOP", false);
            var printWhenProperty = properties?.Find("SUPEXPR", false);
            var printWhenGroupProperty = properties?.Find("SUPGROUP", false);
            var printWhenRepeatedProperty = properties?.Find("SUPALWAYS", false);
            var printWhenValueChangesProperty = properties?.Find("SUPVALCHNG", false);
            var printWhenNewPageColumnProperty = properties?.Find("SUPRPCOL", false);
            var printWhenOverflowProperty = properties?.Find("SUPOVFLOW", false);
            var bottomProperty = properties?.Find("BOTTOM", false);
            var topProperty = properties?.Find("TOP", false);
            var modeProperty = properties?.Find("MODE", false);
            var fillCharProperty = properties?.Find("FILLCHAR", false);
            var totalTypeProperty = properties?.Find("TOTALTYPE", false);

            Expect(floatProperty is not null && noRepeatProperty is not null &&
                   stretchProperty is not null && stretchTopProperty is not null && printWhenProperty is not null &&
                   printWhenGroupProperty is not null && printWhenRepeatedProperty is not null &&
                   printWhenValueChangesProperty is not null && printWhenNewPageColumnProperty is not null &&
                   printWhenOverflowProperty is not null && bottomProperty is not null && topProperty is not null &&
                   modeProperty is not null && fillCharProperty is not null && totalTypeProperty is not null,
                $"{assetFamily} object selections should expose editable report-control behavior properties");
            Expect(floatProperty?.DisplayName == "Float" && noRepeatProperty?.DisplayName == "No Repeat" &&
                   stretchProperty?.DisplayName == "Stretch with Overflow" &&
                   stretchTopProperty?.DisplayName == "Stretch Relative to Top" &&
                   printWhenProperty?.DisplayName == "Print When" &&
                   printWhenGroupProperty?.DisplayName == "When Group Changes" &&
                   printWhenRepeatedProperty?.DisplayName == "Print Repeated Values" &&
                   printWhenValueChangesProperty?.DisplayName == "Print Only When Value Changes" &&
                   printWhenNewPageColumnProperty?.DisplayName == "In First Whole Band of New Page/Column" &&
                   printWhenOverflowProperty?.DisplayName == "When Detail Overflows to New Page/Column" &&
                   bottomProperty?.DisplayName == "Fix Relative to Bottom of Band" &&
                   topProperty?.DisplayName == "Fix Relative to Top of Band" &&
                   modeProperty?.DisplayName == "Back Style / Direction Mode" &&
                   fillCharProperty?.DisplayName == "Expression Data Type" &&
                   totalTypeProperty?.DisplayName == "Calculation Type",
                $"{assetFamily} object behavior properties should use the English catalog labels");
            if (selection is not null)
            {
                floatProperty?.SetValue(selection, false);
                noRepeatProperty?.SetValue(selection, true);
                stretchProperty?.SetValue(selection, false);
                stretchTopProperty?.SetValue(selection, true);
                printWhenProperty?.SetValue(selection, "amount > 100");
                printWhenGroupProperty?.SetValue(selection, 7);
                printWhenRepeatedProperty?.SetValue(selection, false);
                printWhenValueChangesProperty?.SetValue(selection, true);
                printWhenNewPageColumnProperty?.SetValue(selection, 3);
                printWhenOverflowProperty?.SetValue(selection, true);
                bottomProperty?.SetValue(selection, true);
                topProperty?.SetValue(selection, true);
                modeProperty?.SetValue(selection, 6);
                fillCharProperty?.SetValue(selection, "D");
                totalTypeProperty?.SetValue(selection, 3);
                Expect(selection.TryGetUpdate("FLOAT", out var floatTarget, out var serializedFloatValue) &&
                       floatTarget == "FLOAT" && serializedFloatValue == "false" &&
                       selection.TryGetUpdate("NOREPEAT", out var noRepeatTarget, out var serializedNoRepeatValue) &&
                       noRepeatTarget == "NOREPEAT" && serializedNoRepeatValue == "true" &&
                       selection.TryGetUpdate("STRETCH", out var stretchTarget, out var serializedStretchValue) &&
                       stretchTarget == "STRETCH" && serializedStretchValue == "false" &&
                       selection.TryGetUpdate("STRETCHTOP", out var stretchTopTarget, out var serializedStretchTopValue) &&
                       stretchTopTarget == "STRETCHTOP" && serializedStretchTopValue == "true",
                    $"{assetFamily} object behavior edits should preserve invariant FRX/LBX update targets");
                Expect(selection.TryGetUpdate("SUPEXPR", out var printWhenTarget, out var serializedPrintWhenValue) &&
                       printWhenTarget == "SUPEXPR" && serializedPrintWhenValue == "amount > 100",
                    $"{assetFamily} Print When edits should preserve the invariant memo-field update target");
                Expect(selection.TryGetUpdate("SUPGROUP", out var printWhenGroupTarget, out var serializedPrintWhenGroupValue) &&
                       printWhenGroupTarget == "SUPGROUP" && serializedPrintWhenGroupValue == "7",
                    $"{assetFamily} Print When group edits should preserve the invariant numeric update target");
                Expect(selection.TryGetUpdate("SUPALWAYS", out var printWhenRepeatedTarget, out var serializedPrintWhenRepeatedValue) &&
                       printWhenRepeatedTarget == "SUPALWAYS" && serializedPrintWhenRepeatedValue == "false",
                    $"{assetFamily} repeated-value edits should preserve the invariant logical update target");
                Expect(selection.TryGetUpdate("SUPVALCHNG", out var printWhenValueChangesTarget, out var serializedPrintWhenValueChangesValue) &&
                       printWhenValueChangesTarget == "SUPVALCHNG" && serializedPrintWhenValueChangesValue == "true",
                    $"{assetFamily} value-change edits should preserve the invariant logical update target");
                Expect(selection.TryGetUpdate("SUPRPCOL", out var printWhenNewPageColumnTarget, out var serializedPrintWhenNewPageColumnValue) &&
                       printWhenNewPageColumnTarget == "SUPRPCOL" && serializedPrintWhenNewPageColumnValue == "3",
                    $"{assetFamily} page-column edits should preserve the invariant numeric update target");
                Expect(selection.TryGetUpdate("SUPOVFLOW", out var printWhenOverflowTarget, out var serializedPrintWhenOverflowValue) &&
                       printWhenOverflowTarget == "SUPOVFLOW" && serializedPrintWhenOverflowValue == "true",
                    $"{assetFamily} overflow edits should preserve the invariant logical update target");
                Expect(selection.TryGetUpdate("BOTTOM", out var bottomTarget, out var serializedBottomValue) &&
                       bottomTarget == "BOTTOM" && serializedBottomValue == "true",
                    $"{assetFamily} bottom-position edits should preserve the invariant logical update target");
                Expect(selection.TryGetUpdate("TOP", out var topTarget, out var serializedTopValue) &&
                       topTarget == "TOP" && serializedTopValue == "true",
                    $"{assetFamily} top-position edits should preserve the invariant logical update target");
                Expect(selection.TryGetUpdate("MODE", out var modeTarget, out var serializedModeValue) &&
                       modeTarget == "MODE" && serializedModeValue == "6",
                    $"{assetFamily} mode edits should preserve the invariant numeric update target");
                Expect(selection.TryGetUpdate("FILLCHAR", out var fillCharTarget, out var serializedFillCharValue) &&
                       fillCharTarget == "FILLCHAR" && serializedFillCharValue == "D",
                    $"{assetFamily} expression data-type edits should preserve the invariant update target");
                Expect(selection.TryGetUpdate("TOTALTYPE", out var totalTypeTarget, out var serializedTotalTypeValue) &&
                       totalTypeTarget == "TOTALTYPE" && serializedTotalTypeValue == "3",
                    $"{assetFamily} calculation-type edits should preserve the invariant update target");
            }
        }

        var blankSelection = CopperfinDesignerSelection.FromSnapshot(
            "report",
            Snapshot("8", string.Empty, "false", string.Empty, "false", string.Empty, string.Empty, string.Empty, string.Empty, string.Empty, string.Empty, string.Empty, string.Empty, string.Empty, string.Empty, string.Empty),
            new CopperfinLocalization("en-US"));
        Expect(blankSelection?.GetProperties().Find("FLOAT", false)?.GetValue(blankSelection) is bool floatValue && !floatValue &&
               blankSelection.GetProperties().Find("NOREPEAT", false)?.GetValue(blankSelection) is bool noRepeatValue && !noRepeatValue &&
               blankSelection.GetProperties().Find("STRETCH", false)?.GetValue(blankSelection) is bool stretchValue && !stretchValue &&
               blankSelection.GetProperties().Find("STRETCHTOP", false)?.GetValue(blankSelection) is bool stretchTopValue && !stretchTopValue &&
               string.Equals(blankSelection.GetProperties().Find("SUPEXPR", false)?.GetValue(blankSelection)?.ToString(), string.Empty, StringComparison.Ordinal) &&
               blankSelection.GetProperties().Find("SUPGROUP", false)?.GetValue(blankSelection) is int blankGroupValue && blankGroupValue == 0 &&
               blankSelection.GetProperties().Find("SUPALWAYS", false)?.GetValue(blankSelection) is bool blankRepeatedValue && !blankRepeatedValue &&
               blankSelection.GetProperties().Find("SUPVALCHNG", false)?.GetValue(blankSelection) is bool blankValueChangesValue && !blankValueChangesValue &&
               blankSelection.GetProperties().Find("SUPRPCOL", false)?.GetValue(blankSelection) is int blankPageColumnValue && blankPageColumnValue == 0 &&
               blankSelection.GetProperties().Find("SUPOVFLOW", false)?.GetValue(blankSelection) is bool blankOverflowValue && !blankOverflowValue &&
               blankSelection.GetProperties().Find("BOTTOM", false)?.GetValue(blankSelection) is bool blankBottomValue && !blankBottomValue &&
               blankSelection.GetProperties().Find("TOP", false)?.GetValue(blankSelection) is bool blankTopValue && !blankTopValue &&
               blankSelection.GetProperties().Find("MODE", false)?.GetValue(blankSelection) is int blankModeValue && blankModeValue == 0 &&
               string.Equals(blankSelection.GetProperties().Find("FILLCHAR", false)?.GetValue(blankSelection)?.ToString(), string.Empty, StringComparison.Ordinal) &&
               blankSelection.GetProperties().Find("TOTALTYPE", false)?.GetValue(blankSelection) is int blankTotalTypeValue && blankTotalTypeValue == 0,
            "blank and false report-control logical values should remain stable as false until edited");

        var pseudoSelection = CopperfinDesignerSelection.FromSnapshot(
            "report",
            Snapshot("8", "true", "false"),
            new CopperfinLocalization("qps-ploc"));
        var pseudoFloatLabel = pseudoSelection?.GetProperties().Find("FLOAT", false)?.DisplayName;
        var pseudoNoRepeatLabel = pseudoSelection?.GetProperties().Find("NOREPEAT", false)?.DisplayName;
        var pseudoStretchLabel = pseudoSelection?.GetProperties().Find("STRETCH", false)?.DisplayName;
        var pseudoStretchTopLabel = pseudoSelection?.GetProperties().Find("STRETCHTOP", false)?.DisplayName;
        var pseudoPrintWhenLabel = pseudoSelection?.GetProperties().Find("SUPEXPR", false)?.DisplayName;
        var pseudoPrintWhenGroupLabel = pseudoSelection?.GetProperties().Find("SUPGROUP", false)?.DisplayName;
        var pseudoPrintWhenRepeatedLabel = pseudoSelection?.GetProperties().Find("SUPALWAYS", false)?.DisplayName;
        var pseudoPrintWhenValueChangesLabel = pseudoSelection?.GetProperties().Find("SUPVALCHNG", false)?.DisplayName;
        var pseudoPrintWhenNewPageColumnLabel = pseudoSelection?.GetProperties().Find("SUPRPCOL", false)?.DisplayName;
        var pseudoPrintWhenOverflowLabel = pseudoSelection?.GetProperties().Find("SUPOVFLOW", false)?.DisplayName;
        var pseudoBottomLabel = pseudoSelection?.GetProperties().Find("BOTTOM", false)?.DisplayName;
        var pseudoTopLabel = pseudoSelection?.GetProperties().Find("TOP", false)?.DisplayName;
        var pseudoModeLabel = pseudoSelection?.GetProperties().Find("MODE", false)?.DisplayName;
        var pseudoFillCharLabel = pseudoSelection?.GetProperties().Find("FILLCHAR", false)?.DisplayName;
        var pseudoTotalTypeLabel = pseudoSelection?.GetProperties().Find("TOTALTYPE", false)?.DisplayName;
        Expect(pseudoFloatLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoFloatLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoNoRepeatLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoNoRepeatLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoStretchLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoStretchLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoStretchTopLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoStretchTopLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoPrintWhenLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoPrintWhenLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoPrintWhenGroupLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoPrintWhenGroupLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoPrintWhenRepeatedLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoPrintWhenRepeatedLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoPrintWhenValueChangesLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoPrintWhenValueChangesLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoPrintWhenNewPageColumnLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoPrintWhenNewPageColumnLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoPrintWhenOverflowLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoPrintWhenOverflowLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoBottomLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoBottomLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoTopLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoTopLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoModeLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoModeLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoFillCharLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoFillCharLabel.EndsWith(" !!]", StringComparison.Ordinal) &&
               pseudoTotalTypeLabel?.StartsWith("[!! ", StringComparison.Ordinal) == true &&
               pseudoTotalTypeLabel.EndsWith(" !!]", StringComparison.Ordinal),
            "pseudo-localized report-control property labels should remain visibly localized");

        var readOnlySelection = CopperfinDesignerSelection.FromSnapshot(
            "report",
            Snapshot("8", "true", "false"),
            new CopperfinLocalization("en-US"),
            documentReadOnly: true);
        Expect(readOnlySelection?.GetProperties().Find("FLOAT", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("NOREPEAT", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("STRETCH", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("STRETCHTOP", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("SUPEXPR", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("SUPGROUP", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("SUPALWAYS", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("SUPVALCHNG", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("SUPRPCOL", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("SUPOVFLOW", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("BOTTOM", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("TOP", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("MODE", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("FILLCHAR", false)?.IsReadOnly == true &&
               readOnlySelection.GetProperties().Find("TOTALTYPE", false)?.IsReadOnly == true,
            "read-only report documents should keep report-control behavior properties read-only");
    }

    private static void TestDesignerSelectionDefaultsToEnvironmentLocalization()
    {
        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");

        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", "qps-ploc");

            var selection = CopperfinDesignerSelection.FromSnapshot(
                "report",
                new CopperfinStudioSnapshotObject
                {
                    RecordIndex = 7,
                    Deleted = true,
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "OBJTYPE", Value = "8" },
                        new() { Name = "OBJCODE", Value = "53" },
                        new() { Name = "EXPR", Value = "\"customer.name\"" },
                        new() { Name = "HPOS", Value = "10" },
                        new() { Name = "VPOS", Value = "20" },
                        new() { Name = "WIDTH", Value = "120" },
                        new() { Name = "HEIGHT", Value = "24" },
                        new() { Name = "FONTFACE", Value = "\"Arial\"" },
                        new() { Name = "FONTSIZE", Value = "9" },
                        new() { Name = "FONTSTYLE", Value = "0" }
                    }
                });

            Expect(selection is not null, "designer selection should materialize from report snapshot data");
            if (selection is null)
            {
                return;
            }

            var properties = selection.GetProperties();
            var objectState = properties.Find("OBJECTSTATE", false);
            Expect(objectState is not null, "designer selection should expose the report object-state field");
            if (objectState is null)
            {
                return;
            }

            var value = objectState.GetValue(selection)?.ToString() ?? string.Empty;
            Expect(objectState.DisplayName.StartsWith("[!! ", StringComparison.Ordinal),
                "environment-defaulted designer selections should pseudo-localize display names");
            Expect(value.StartsWith("[!! ", StringComparison.Ordinal) &&
                   value.EndsWith(" !!]", StringComparison.Ordinal),
                "environment-defaulted designer selections should pseudo-localize state values");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousLocale);
        }
    }

    private static void TestDesignerSelectionHonorsReadOnlyDocumentState()
    {
        var selection = CopperfinDesignerSelection.FromSnapshot(
            "report",
            new CopperfinStudioSnapshotObject
            {
                RecordIndex = 11,
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "EXPR", Value = "customer.name" },
                    new() { Name = "HPOS", Value = "10" }
                }
            },
            new CopperfinLocalization("en-US"),
            documentReadOnly: true);

        Expect(selection is not null, "read-only designer selection should materialize from report snapshot data");
        if (selection is null)
        {
            return;
        }

        var expression = selection.GetProperties().Find("EXPR", false);
        Expect(expression is not null && expression.IsReadOnly,
            "read-only document selections should expose editable fields as read-only");
        Expect(!selection.TryGetUpdate("EXPR", out _, out _),
            "read-only document selections should reject property updates");
    }

    private static void TestStudioHostProcessStartInfoKeepsExecutableLaunchArguments()
    {
        var startInfo = CopperfinStudioHostBridge.CreateProcessStartInfo(
            @"C:\tools\copperfin_studio_host.exe",
            "--from-vs --path \"C:\\Samples\\invoice.frx\"",
            redirectOutput: true,
            createNoWindow: true,
            isWindowsOverride: true);

        Expect(startInfo.FileName == @"C:\tools\copperfin_studio_host.exe",
            "direct Studio host executables should launch without an intermediate wrapper");
        Expect(startInfo.Arguments == "--from-vs --path \"C:\\Samples\\invoice.frx\"",
            "direct Studio host executables should preserve the invariant command arguments");
        Expect(!startInfo.UseShellExecute &&
               startInfo.RedirectStandardOutput &&
               startInfo.RedirectStandardError &&
               startInfo.CreateNoWindow,
            "direct Studio host executable launch info should preserve non-shell redirected execution");
    }

    private static void TestStudioHostProcessStartInfoUsesUtf8ForRedirectedStreams()
    {
        var startInfo = CopperfinStudioHostBridge.CreateProcessStartInfo(
            @"C:\tools\copperfin_studio_host.exe",
            "--from-vs --json --path \"C:\\Samples\\café.frx\"",
            redirectOutput: true,
            createNoWindow: true,
            isWindowsOverride: true);

        Expect(startInfo.StandardOutputEncoding == Encoding.UTF8 &&
               startInfo.StandardErrorEncoding == Encoding.UTF8,
            "redirected Studio host stdout and stderr should decode native UTF-8 JSON and diagnostics");
    }

    private static void TestStudioStartupArgumentsPreserveSelectors()
    {
        var localization = new CopperfinLocalization("en-US");
        var parsed = CopperfinStudioStartupArguments.TryParse(
            new[] { "--locale", "en-US", "--path", @"C:\Samples\form.scx", "--object-name", "cmdSave", "--unique-id", "cmd-save-guid" },
            localization,
            out var documents,
            out var error);

        Expect(parsed && error is null && documents.Count == 1,
            "standalone Studio startup parsing should accept one form path with both selectors");
        if (documents.Count == 1)
        {
            Expect(documents[0].Path == @"C:\Samples\form.scx" &&
                   documents[0].ObjectName == "cmdSave" &&
                   documents[0].UniqueId == "cmd-save-guid",
                "standalone Studio startup parsing should preserve selector values and the document path");
        }
    }

    private static void TestStudioStartupArgumentsRejectMalformedSelectors()
    {
        var localization = new CopperfinLocalization("es-419");

        var cases = new[]
        {
            (Arguments: new[] { @"C:\Samples\form.scx", "--object-name" },
             Expected: "La opción de inicio de Copperfin Studio '"),
            (Arguments: new[] { @"C:\Samples\form.scx", "--unique-id", "one", "--unique-id", "two" },
             Expected: "El selector de inicio de Copperfin Studio '"),
            (Arguments: new[] { @"C:\Samples\form.scx", "--not-a-startup-option" },
             Expected: "El argumento de inicio de Copperfin Studio '"),
            (Arguments: new[] { @"C:\Samples\invoice.frx", "--unique-id", "invoice-guid" },
             Expected: "Los selectores de inicio se aplican solo a activos de formulario y biblioteca de clases.")
        };

        foreach (var testCase in cases)
        {
            var parsed = CopperfinStudioStartupArguments.TryParse(
                testCase.Arguments,
                localization,
                out var documents,
                out var error);
            Expect(!parsed && documents.Count == 0 &&
                   !string.IsNullOrWhiteSpace(error) && error!.StartsWith(testCase.Expected, StringComparison.Ordinal),
                "malformed standalone Studio selectors should fail with localized diagnostics");
        }
    }

    private static void TestStudioHostBuildArgumentsPreserveStartupSelectors()
    {
        var arguments = CopperfinStudioHostBridge.BuildArguments(
            @"C:\Samples\form.scx",
            objectName: "cmd Save",
            uniqueId: "cmd-save-guid");

        Expect(arguments == "--from-vs --path \"C:\\Samples\\form.scx\" --object-name \"cmd Save\" --unique-id \"cmd-save-guid\"",
            "managed Studio host bridge should preserve optional startup selectors after the invariant path argument");
    }

    private static void TestStudioHostProcessStartInfoWrapsWindowsBatchHosts()
    {
        var previousComSpec = Environment.GetEnvironmentVariable("COMSPEC");

        try
        {
            Environment.SetEnvironmentVariable("COMSPEC", @"C:\Windows\System32\cmd.exe");

            var startInfo = CopperfinStudioHostBridge.CreateProcessStartInfo(
                @"C:\temp\fake studio host.cmd",
                "--from-vs --json --set-property --path \"C:\\Samples\\invoice.frx\"",
                redirectOutput: true,
                createNoWindow: true,
                isWindowsOverride: true);

            Expect(startInfo.FileName == @"C:\Windows\System32\cmd.exe",
                "Windows batch-backed Studio hosts should launch through COMSPEC");
            Expect(startInfo.Arguments == "/d /c %COPPERFIN_SCRIPT_WRAPPER_COMMAND%" &&
                   startInfo.EnvironmentVariables["COPPERFIN_SCRIPT_WRAPPER_COMMAND"] ==
                       "\"C:\\temp\\fake studio host.cmd\" --from-vs --json --set-property --path \"C:\\Samples\\invoice.frx\"",
                "Windows batch-backed Studio hosts should preserve the invariant command arguments inside the cmd wrapper");
            Expect(!startInfo.UseShellExecute &&
                   startInfo.RedirectStandardOutput &&
                   startInfo.RedirectStandardError &&
                   startInfo.CreateNoWindow,
                "Windows batch-backed Studio hosts should preserve non-shell redirected execution");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COMSPEC", previousComSpec);
        }
    }

    private static void TestStudioHostProcessStartInfoAppliesExplicitLocalizationEnvironment()
    {
        var localization = new CopperfinLocalization("pt-BR");
        var startInfo = CopperfinStudioHostBridge.CreateProcessStartInfo(
            "/tmp/copperfin_studio_host",
            "--from-vs --json",
            localization: localization,
            redirectOutput: true,
            createNoWindow: true,
            isWindowsOverride: false);

        var uiLocale = startInfo.EnvironmentVariables["COPPERFIN_UI_LOCALE"];
        var locale = startInfo.EnvironmentVariables["COPPERFIN_LOCALE"];
        Expect(string.Equals(uiLocale, localization.Locale, StringComparison.Ordinal),
            "Studio host launch info should stamp the selected UI locale into the child environment");
        Expect(string.Equals(locale, localization.Locale, StringComparison.Ordinal),
            "Studio host launch info should stamp the selected runtime locale into the child environment");

        var previousDirectory = Environment.GetEnvironmentVariable("COPPERFIN_LOCALE_DIR");
        var temporaryRoot = Path.Combine(
            Path.GetTempPath(),
            "copperfin-host-locale-root-" + Guid.NewGuid().ToString("N"));
        var hostDirectory = Path.Combine(temporaryRoot, "bin", "studio");
        var catalogDirectory = Path.Combine(temporaryRoot, "share", "copperfin", "locales");
        Directory.CreateDirectory(Path.Combine(catalogDirectory, "en-US"));
        try
        {
            File.WriteAllText(
                Path.Combine(catalogDirectory, "en-US", "strings.json"),
                "{\"Studio.AppTitle\":\"Copperfin Studio\"}");
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE_DIR", null);
            var hostStartInfo = CopperfinStudioHostBridge.CreateProcessStartInfo(
                Path.Combine(hostDirectory, "copperfin_studio_host.exe"),
                "--json",
                localization: localization,
                isWindowsOverride: true);
            var hostLocaleDirectory = hostStartInfo.EnvironmentVariables.ContainsKey("COPPERFIN_LOCALE_DIR")
                ? hostStartInfo.EnvironmentVariables["COPPERFIN_LOCALE_DIR"]
                : null;
            Expect(
                string.Equals(hostLocaleDirectory, catalogDirectory, StringComparison.OrdinalIgnoreCase),
                "Studio host launch info should propagate the installed catalog root beside the host executable");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE_DIR", previousDirectory);
            Directory.Delete(temporaryRoot, recursive: true);
        }
    }

    private static void TestStudioHostBatchArgumentsKeepVisualStudioProvenance()
    {
        var propertyArguments = CopperfinStudioHostBridge.BuildPropertyBatchUpdateArguments(
            @"C:\Samples\invoice.frx",
            7,
            new List<KeyValuePair<string, string>>
            {
                new("CAPTION", "Say \"Hi\""),
                new("WIDTH", "120")
            });
        Expect(propertyArguments.StartsWith(
                "--from-vs --visual-object-update-batch --json ",
                StringComparison.Ordinal),
            "VSIX property-batch arguments should lead with Visual Studio provenance and preserve command tokens");
        Expect(propertyArguments.Contains(
                "--path \"C:\\Samples\\invoice.frx\" --selected-record 7",
                StringComparison.Ordinal),
            "VSIX property-batch arguments should preserve path quoting and record identity");
        Expect(propertyArguments.Contains(
                "--property-name \"CAPTION\" --property-value \"Say \\\"Hi\\\"\"",
                StringComparison.Ordinal) &&
               propertyArguments.Contains(
                   "--property-name \"WIDTH\" --property-value \"120\"",
                   StringComparison.Ordinal),
            "VSIX property-batch arguments should preserve ordered property names and quoted values");
        Expect(propertyArguments.Split("--from-vs", StringSplitOptions.None).Length == 2,
            "VSIX property-batch arguments should emit Visual Studio provenance exactly once");

        var debugArguments = CopperfinRuntimeDebugClient.BuildReplayArguments(
            @"C:\debug manifest\app.cfdebug",
            "es-419",
            new[] { "Say \"Hi\"" });
        Expect(debugArguments.Contains(
                "--debug-command \"Say \\\"Hi\\\"\"",
                StringComparison.Ordinal),
            "runtime debug arguments should preserve embedded quotes through process quoting");

        var trailingBackslashArguments = CopperfinStudioHostBridge.BuildPropertyUpdateArguments(
            @"C:\Samples\invoice.frx",
            7,
            "OUTPUT",
            @"C:\temp\");
        Expect(trailingBackslashArguments.Contains(
                "--property-value \"C:\\temp\\\\\"",
                StringComparison.Ordinal),
            "VSIX process arguments should preserve trailing backslashes before a closing quote");

        var deletedStateArguments = CopperfinStudioHostBridge.BuildDeletedStatesArguments(
            @"C:\Samples\invoice.frx",
            new List<KeyValuePair<string, bool>>
            {
                new("save-\"guid", true),
                new("name-guid", false)
            });
        Expect(deletedStateArguments.StartsWith(
                "--from-vs --path \"C:\\Samples\\invoice.frx\" --deleted-states",
                StringComparison.Ordinal),
            "VSIX deleted-state batch arguments should lead with Visual Studio provenance and preserve command tokens");
        Expect(deletedStateArguments.Contains(
                "--deleted-state-target-unique-id \"save-\\\"guid\" --deleted-state true",
                StringComparison.Ordinal) &&
               deletedStateArguments.Contains(
                   "--deleted-state-target-unique-id \"name-guid\" --deleted-state false",
                   StringComparison.Ordinal),
            "VSIX deleted-state batch arguments should preserve selector quoting and invariant boolean values");
        Expect(deletedStateArguments.EndsWith(" --json", StringComparison.Ordinal),
            "VSIX deleted-state batch arguments should preserve the invariant JSON switch");
        Expect(deletedStateArguments.Split("--from-vs", StringSplitOptions.None).Length == 2,
            "VSIX deleted-state batch arguments should emit Visual Studio provenance exactly once");
    }

    private static void TestManagedHostResolutionHonorsEnvironmentOverrides()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", "managed_host_override", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        var configuredHostPath = Path.Combine(root, "configured", "copperfin_studio_host.cmd");
        Directory.CreateDirectory(Path.GetDirectoryName(configuredHostPath)!);
        File.WriteAllText(configuredHostPath, "@echo off");
        if (!OperatingSystem.IsWindows())
        {
            File.SetUnixFileMode(
                configuredHostPath,
                UnixFileMode.UserRead |
                UnixFileMode.UserWrite |
                UnixFileMode.UserExecute);
        }
        var automaticRoot = Path.Combine(root, "managed", "bin");
        Directory.CreateDirectory(automaticRoot);
        var automaticHostPath = Path.Combine(
            automaticRoot,
            "copperfin_studio_host" + (OperatingSystem.IsWindows() ? ".exe" : string.Empty));
        WriteAutomaticHostCandidate(automaticHostPath);

        var previousStudioHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", configuredHostPath);
            var resolved = CopperfinStudioHostBridge.ResolveStudioHostPath(automaticRoot);
            Expect(string.Equals(resolved, configuredHostPath, StringComparison.Ordinal),
                "explicit COPPERFIN_STUDIO_HOST_PATH should remain exact and bypass automatic candidate policy");

            var missingConfiguredPath = Path.Combine(root, "configured", "missing-copperfin-studio-host");
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", missingConfiguredPath);
            resolved = CopperfinStudioHostBridge.ResolveStudioHostPath(automaticRoot);
            Expect(resolved is null,
                "a missing explicit COPPERFIN_STUDIO_HOST_PATH should fail closed instead of silently selecting an automatic host");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousStudioHostPath);
            TryDelete(root);
        }
    }

    private static void TestManagedHostResolutionAppliesConfiguredPosixExecutePolicy()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", "managed_host_configured_execute", Guid.NewGuid().ToString("N"));
        var automaticRoot = Path.Combine(root, "managed", "bin");
        var configuredHostPath = Path.Combine(root, "configured", "copperfin_studio_host");
        var automaticHostPath = Path.Combine(automaticRoot, "copperfin_studio_host");
        Directory.CreateDirectory(Path.GetDirectoryName(configuredHostPath)!);
        Directory.CreateDirectory(automaticRoot);

        var previousStudioHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", configuredHostPath);
            bool Exists(string candidate) =>
                string.Equals(candidate, configuredHostPath, StringComparison.Ordinal) ||
                string.Equals(candidate, automaticHostPath, StringComparison.Ordinal);

            var rejectedPosix = CopperfinStudioHostBridge.ResolveHostPath(
                "COPPERFIN_STUDIO_HOST_PATH",
                "copperfin_studio_host",
                automaticRoot,
                fileExists: Exists,
                fileIsExecutable: candidate =>
                    string.Equals(candidate, automaticHostPath, StringComparison.Ordinal),
                isWindowsOverride: false,
                enforceConfiguredPosixExecutable: true);
            Expect(rejectedPosix is null,
                "a configured POSIX Studio host without execute permission should fail closed without falling through to automatic discovery");

            var windowsResolved = CopperfinStudioHostBridge.ResolveHostPath(
                "COPPERFIN_STUDIO_HOST_PATH",
                "copperfin_studio_host",
                automaticRoot,
                fileExists: Exists,
                fileIsExecutable: candidate =>
                    string.Equals(candidate, automaticHostPath, StringComparison.Ordinal),
                isWindowsOverride: true,
                enforceConfiguredPosixExecutable: true);
            Expect(string.Equals(windowsResolved, configuredHostPath, StringComparison.Ordinal),
                "Windows should preserve an existing configured Studio host without applying POSIX execute-bit policy");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousStudioHostPath);
            TryDelete(root);
        }
    }

    private static void TestManagedHostResolutionUsesPlatformNativeCandidateRules()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", "managed_host_platform", Guid.NewGuid().ToString("N"));
        var searchRoot = Path.Combine(root, "managed", "bin");
        Directory.CreateDirectory(searchRoot);
        var nativePath = Path.Combine(searchRoot, "copperfin_studio_host");
        var windowsPath = nativePath + ".exe";
        var previousStudioHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", null);
            bool Exists(string candidate) =>
                string.Equals(candidate, nativePath, StringComparison.Ordinal) ||
                string.Equals(candidate, windowsPath, StringComparison.Ordinal);

            var posixResolved = CopperfinStudioHostBridge.ResolveHostPath(
                "COPPERFIN_STUDIO_HOST_PATH",
                "copperfin_studio_host",
                searchRoot,
                fileExists: Exists,
                fileIsExecutable: _ => true,
                isWindowsOverride: false);
            Expect(string.Equals(posixResolved, nativePath, StringComparison.Ordinal),
                "POSIX automatic discovery should prefer the extensionless host even when an executable .exe sibling exists");

            var rejectedPosix = CopperfinStudioHostBridge.ResolveHostPath(
                "COPPERFIN_STUDIO_HOST_PATH",
                "copperfin_studio_host",
                searchRoot,
                fileExists: candidate => string.Equals(candidate, nativePath, StringComparison.Ordinal),
                fileIsExecutable: _ => false,
                isWindowsOverride: false);
            Expect(rejectedPosix is null,
                "POSIX automatic discovery should reject a host candidate without execute permission");

            var windowsResolved = CopperfinStudioHostBridge.ResolveHostPath(
                "COPPERFIN_STUDIO_HOST_PATH",
                "copperfin_studio_host",
                searchRoot,
                fileExists: Exists,
                fileIsExecutable: _ => false,
                isWindowsOverride: true);
            Expect(string.Equals(windowsResolved, windowsPath, StringComparison.Ordinal),
                "Windows automatic discovery should prefer the .exe host and should not apply POSIX execute-bit policy");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousStudioHostPath);
            TryDelete(root);
        }
    }

    private static void TestManagedHostResolutionChecksRealPosixExecutePermission()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", "managed_host_execute", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        var staleWindowsPath = Path.Combine(root, "copperfin_studio_host.exe");
        var nativePath = Path.Combine(root, "copperfin_studio_host");
        File.WriteAllText(staleWindowsPath, "stale");
        WriteAutomaticHostCandidate(nativePath);
        var previousStudioHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", null);
            var resolved = CopperfinStudioHostBridge.ResolveStudioHostPath(root);
            Expect(string.Equals(resolved, nativePath, StringComparison.Ordinal),
                "real POSIX discovery should ignore a non-executable .exe and select the executable native host");

            File.SetUnixFileMode(nativePath, UnixFileMode.UserRead | UnixFileMode.UserWrite);
            resolved = CopperfinStudioHostBridge.ResolveStudioHostPath(root);
            Expect(resolved is null,
                "real POSIX discovery should reject all automatic candidates when none has execute permission");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousStudioHostPath);
            TryDelete(root);
        }
    }

    private static void TestManagedHostResolutionFindsSiblingAndRepoBuildLayouts()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", "managed_host_layouts", Guid.NewGuid().ToString("N"));
        var managedOutput = Path.Combine(root, "vsix", "Copperfin.Studio", "bin", "Release", "net472");
        Directory.CreateDirectory(managedOutput);

        var hostSuffix = OperatingSystem.IsWindows() ? ".exe" : string.Empty;
        var siblingStudioHostPath = Path.Combine(managedOutput, "copperfin_studio_host" + hostSuffix);
        WriteAutomaticHostCandidate(siblingStudioHostPath);

        var buildOutputDirectory = Path.Combine(root, "build", "Release");
        Directory.CreateDirectory(buildOutputDirectory);
        var buildHostPath = Path.Combine(buildOutputDirectory, "copperfin_build_host" + hostSuffix);
        var runtimeHostPath = Path.Combine(buildOutputDirectory, "copperfin_runtime_host" + hostSuffix);
        WriteAutomaticHostCandidate(buildHostPath);
        WriteAutomaticHostCandidate(runtimeHostPath);

        var previousStudioHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousBuildHostPath = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        var previousRuntimeHostPath = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", null);
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", null);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", null);

            var resolvedStudioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath(managedOutput);
            var resolvedBuildHostPath = CopperfinStudioHostBridge.ResolveHostPath(
                "COPPERFIN_BUILD_HOST_PATH",
                "copperfin_build_host",
                managedOutput);
            var resolvedRuntimeHostPath = CopperfinStudioHostBridge.ResolveHostPath(
                "COPPERFIN_RUNTIME_HOST_PATH",
                "copperfin_runtime_host",
                managedOutput);

            Expect(string.Equals(resolvedStudioHostPath, siblingStudioHostPath, StringComparison.Ordinal),
                "managed Studio shell should find a sibling Studio host before walking repo-style build layouts");
            Expect(string.Equals(resolvedBuildHostPath, buildHostPath, StringComparison.Ordinal),
                "managed project workflow should find the repo-style build host from the shell output directory");
            Expect(string.Equals(resolvedRuntimeHostPath, runtimeHostPath, StringComparison.Ordinal),
                "managed project workflow should find the repo-style runtime host from the shell output directory");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousStudioHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", previousBuildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", previousRuntimeHostPath);
            TryDelete(root);
        }
    }

    private static void TestStudioHostLaunchContainsStartupFailures()
    {
        var missingHostPath = Path.Combine(
            Path.GetTempPath(),
            "copperfin-language-service-missing-host-" + Guid.NewGuid().ToString("N"));
        var localization = new CopperfinLocalization("es-419");
        try
        {
            var launched = CopperfinStudioHostBridge.Launch(
                missingHostPath,
                Path.Combine(Path.GetTempPath(), "invoice.frx"),
                localization: localization);
            Expect(!launched,
                "Studio fire-and-forget launch should return failure when process startup throws");
        }
        catch (Exception ex)
        {
            Expect(false,
                "Studio fire-and-forget launch should contain process-start exceptions: " + ex.GetType().Name);
        }
        Expect(
            string.Equals(
                localization.Text("AssetEditor.Dialog.StudioLaunchFailed"),
                "Copperfin Studio no se inició correctamente.",
                StringComparison.Ordinal),
            "contained Studio launch failure should continue through the existing localized caller contract");
    }

    private static void TestLocalizationCatalogDoesNotLeakMachineSpecificHostPaths()
    {
        var english = new CopperfinLocalization("en-US");
        var spanish = new CopperfinLocalization("es-419");
        var portuguese = new CopperfinLocalization("pt-BR");

        var keys = new[]
        {
            "AssetEditor.Project.Workflow.BuildHostMissing",
            "AssetEditor.Project.Workflow.RuntimeHostMissing",
            "AssetEditor.Dialog.StudioHostMissing"
        };

        foreach (var key in keys)
        {
            Expect(english.Text(key).IndexOf(@"E:\Project-Copperfin", StringComparison.OrdinalIgnoreCase) < 0,
                $"{key} should not leak a machine-specific Windows fallback path in English");
            Expect(spanish.Text(key).IndexOf(@"E:\Project-Copperfin", StringComparison.OrdinalIgnoreCase) < 0,
                $"{key} should not leak a machine-specific Windows fallback path in Spanish");
            Expect(portuguese.Text(key).IndexOf(@"E:\Project-Copperfin", StringComparison.OrdinalIgnoreCase) < 0,
                $"{key} should not leak a machine-specific Windows fallback path in Portuguese");
        }
    }

    private static void TestProjectWorkflowThreadsExplicitLocaleToBuildHostOnPosix()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var root = CreateProjectRoot("workflow_locale_threading");
        var captureArgsPath = Path.Combine(root, "build_args.txt");
        var captureEnvPath = Path.Combine(root, "build_env.txt");
        var manifestPath = Path.Combine(root, "app.cfmanifest");
        var debugManifestPath = Path.Combine(root, "app.cfdebug");
        var launcherPath = Path.Combine(root, "launcher");
        var runtimeHostPath = Path.Combine(root, "runtime_host.sh");
        var buildHostPath = CreateTestExecutableScriptPath(
            root,
            "build_host",
            [
                "#!/bin/sh",
                $"printf '%s\\n' \"$@\" > \"{captureArgsPath}\"",
                $"printf 'COPPERFIN_UI_LOCALE=%s\\n' \"${{COPPERFIN_UI_LOCALE:-}}\" > \"{captureEnvPath}\"",
                $"printf 'COPPERFIN_LOCALE=%s\\n' \"${{COPPERFIN_LOCALE:-}}\" >> \"{captureEnvPath}\"",
                $"printf 'status: ok\\nmanifest.path: %s\\ndebug.manifest.path: %s\\nlauncher.output: %s\\n' \"{manifestPath}\" \"{debugManifestPath}\" \"{launcherPath}\""
            ]);
        File.WriteAllText(runtimeHostPath, string.Empty);
        File.WriteAllText(manifestPath, string.Empty);
        File.WriteAllText(debugManifestPath, string.Empty);
        File.WriteAllText(launcherPath, string.Empty);

        var projectPath = Path.Combine(root, "testapp.pjx");
        var previousBuildHostPath = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        var previousRuntimeHostPath = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        var previousUiLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_LOCALE");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", buildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", runtimeHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", "en-US");
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", "en-US");

            var result = CopperfinProjectWorkflow.ExecuteAsync(
                projectPath,
                CopperfinProjectOperation.Build,
                new CopperfinLocalization("qps-ploc")).GetAwaiter().GetResult();

            Expect(result.Success, "project workflow should succeed against the fake build host");

            var capturedArgs = File.ReadAllText(captureArgsPath);
            var capturedEnv = File.ReadAllText(captureEnvPath);
            Expect(capturedArgs.Contains("--locale", StringComparison.Ordinal) &&
                   capturedArgs.Contains("qps-ploc", StringComparison.Ordinal),
                "project workflow should pass the explicit locale through the build-host command line");
            Expect(capturedEnv.Contains("COPPERFIN_UI_LOCALE=qps-ploc", StringComparison.Ordinal),
                "project workflow should stamp COPPERFIN_UI_LOCALE for the build host");
            Expect(capturedEnv.Contains("COPPERFIN_LOCALE=qps-ploc", StringComparison.Ordinal),
                "project workflow should stamp COPPERFIN_LOCALE for the build host");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", previousBuildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", previousRuntimeHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousUiLocale);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", previousLocale);
            TryDelete(root);
        }
    }

    private static void TestRuntimeDebugParserUnescapesEscapedLineValues()
    {
        var state = CopperfinRuntimeDebugClient.ParsePauseState(
            "debug.command[0]: continue\n" +
            "debug.global.cGlobal: line1\\r\\n\\t\\\\tail\n" +
            "debug.global.cSpaced:   abcd   \n" +
            "debug.frame[0].local.cLocal: line1\\r\\n\\t\\\\tail\n" +
            "debug.frame[0].local.cSpaced:   local   \n" +
            "debug.location: C:\\notes\\nfile.prg:3\n");
        const string expected = "line1\r\n\t\\tail";

        Expect(state.Globals.Count >= 1 && state.Globals[0].Value == expected,
            "runtime debug parser should decode escaped multiline global values");
        Expect(state.Globals.Count == 2 && state.Globals[1].Value == "  abcd   ",
            "runtime debug parser should preserve leading and trailing global value spaces");
        Expect(state.Frames.Count == 1 && state.Frames[0].Locals.Count >= 1 &&
               state.Frames[0].Locals[0].Value == expected,
            "runtime debug parser should decode escaped multiline local values");
        Expect(state.Frames.Count == 1 && state.Frames[0].Locals.Count == 2 &&
               state.Frames[0].Locals[1].Value == "  local   ",
            "runtime debug parser should preserve leading and trailing local value spaces");
        Expect(state.Location == @"C:\notes\nfile.prg:3",
            "runtime debug parser should preserve unencoded path fields containing escape-like text");
    }

    private static void TestRuntimeDebugFallbackErrorLocalizesWithoutChangingPrecedence()
    {
        var locales = new[]
        {
            CopperfinLocalization.DefaultLocale,
            CopperfinLocalization.SpanishLatinAmericaLocale,
            CopperfinLocalization.PortugueseBrazilLocale,
            CopperfinLocalization.PseudoLocale
        };
        foreach (var locale in locales)
        {
            var localization = new CopperfinLocalization(locale);
            var fallback = CopperfinRuntimeDebugTransport.ResolveResponseError(
                "debug.response.error: true\n",
                string.Empty,
                localization);
            Expect(
                fallback == localization.Text("AssetEditor.Debugger.CommandFailed"),
                $"runtime debug fallback should use the active {locale} catalog");
            if (locale == CopperfinLocalization.PseudoLocale)
            {
                Expect(
                    fallback.StartsWith("[!! ", StringComparison.Ordinal),
                    "runtime debug fallback should remain visible in the pseudo-locale");
            }
        }

        var localizationForPrecedence =
            new CopperfinLocalization(CopperfinLocalization.PortugueseBrazilLocale);
        foreach (var locale in locales)
        {
            var localization = new CopperfinLocalization(locale);
            var prefix = localization.Text("RuntimeHost.Prefix.Error");
            Expect(
                CopperfinRuntimeDebugTransport.ResolveResponseError(
                    $"debug.response.error: true\n{prefix}localized protocol failure\n",
                    string.Empty,
                    localization) == "localized protocol failure",
                $"runtime debug should parse the localized {locale} error prefix without changing the diagnostic body");
        }
        Expect(
            CopperfinRuntimeDebugTransport.ResolveResponseError(
                "debug.response.error: true\nerror: protocol failure\n",
                "stderr failure",
                localizationForPrecedence) == "stderr failure",
            "runtime debug stderr should retain precedence over protocol error lines and fallback text");
        Expect(
            CopperfinRuntimeDebugTransport.ResolveResponseError(
                "debug.response.error: true\nerror: protocol failure\n",
                string.Empty,
                localizationForPrecedence) == "protocol failure",
            "runtime debug protocol error lines should retain precedence over localized fallback text");
    }

    private static void TestRuntimeDebugTransportFailuresLocalizeAcrossSupportedLocales()
    {
        var failuresToCheck = new[]
        {
            CopperfinRuntimeDebugTransportFailure.ProcessDidNotStart,
            CopperfinRuntimeDebugTransportFailure.ProcessExited,
            CopperfinRuntimeDebugTransportFailure.ProcessTimedOut,
            CopperfinRuntimeDebugTransportFailure.ProcessClosedOutput
        };

        foreach (var locale in CopperfinLocalization.SupportedLocales)
        {
            var localization = new CopperfinLocalization(locale);
            foreach (var failure in failuresToCheck)
            {
                var message = CopperfinRuntimeDebugTransport.TransportFailureMessage(localization, failure);
                Expect(!string.IsNullOrWhiteSpace(message),
                    $"runtime debug transport failure {failure} should be nonblank in {locale}");
                if (locale == CopperfinLocalization.PseudoLocale)
                {
                    Expect(message.StartsWith("[!! ", StringComparison.Ordinal) &&
                           message.EndsWith(" !!]", StringComparison.Ordinal),
                        $"runtime debug transport failure {failure} should be pseudo-localized");
                }
            }
        }
    }

    private static void TestRuntimeDebugClientThreadsExplicitLocaleToRuntimeHostOnPosix()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var root = CreateProjectRoot("runtime_debug_locale_threading");
        var manifestPath = Path.Combine(root, "app.cfmanifest");
        var debugManifestPath = Path.Combine(root, "app.cfdebug");
        var launcherPath = Path.Combine(root, "launcher");
        var buildHostPath = CreateTestExecutableScriptPath(
            root,
            "build_host",
            [
                "#!/bin/sh",
                $"printf 'status: ok\\nmanifest.path: %s\\ndebug.manifest.path: %s\\nlauncher.output: %s\\n' \"{manifestPath}\" \"{debugManifestPath}\" \"{launcherPath}\""
            ]);
        var runtimeArgsPath = Path.Combine(root, "runtime_args.txt");
        var runtimeEnvPath = Path.Combine(root, "runtime_env.txt");
        var runtimeHostPath = CreateTestExecutableScriptPath(
            root,
            "runtime_host",
            [
                "#!/bin/sh",
                $"printf '%s\\n' \"$@\" > \"{runtimeArgsPath}\"",
                $"printf 'COPPERFIN_UI_LOCALE=%s\\n' \"${{COPPERFIN_UI_LOCALE:-}}\" > \"{runtimeEnvPath}\"",
                $"printf 'COPPERFIN_LOCALE=%s\\n' \"${{COPPERFIN_LOCALE:-}}\" >> \"{runtimeEnvPath}\"",
                "server=false",
                "for arg in \"$@\"; do",
                "  if [ \"$arg\" = '--debug-server' ]; then server=true; fi",
                "done",
                "if [ \"$server\" = true ]; then",
                "  printf 'debug.server.protocol: 1\\n'",
                "  printf 'debug.server.ready: true\\n'",
                "  while IFS= read -r command; do",
                "    printf 'debug.response.begin\\n'",
                "    if [ \"$command\" = 'exit' ]; then",
                "      printf 'debug.command[1]: exit\\n'",
                "      printf 'debug.exit: true\\n'",
                "      printf 'debug.response.end\\n'",
                "      exit 0",
                "    fi",
                "    if [ \"$command\" = 'step' ]; then reason=step; else reason=entry; fi",
                "    printf 'debug.command[0]: %s\\n' \"$command\"",
                "    printf 'debug.reason: %s\\n' \"$reason\"",
                "    printf 'debug.location: app/main.prg:12\\n'",
                "    printf 'debug.statement: WAIT WINDOW \"hello\"\\n'",
                "    printf 'debug.stack.depth: 1\\n'",
                "    printf 'debug.executed.statements: 1\\n'",
                "    printf 'debug.response.end\\n'",
                "  done",
                "else",
                "  printf 'debug.command[0]: continue\\n'",
                "  printf 'debug.reason: entry\\n'",
                "  printf 'debug.location: app/main.prg:12\\n'",
                "  printf 'debug.statement: WAIT WINDOW \"hello\"\\n'",
                "  printf 'debug.stack.depth: 1\\n'",
                "  printf 'debug.executed.statements: 1\\n'",
                "fi"
            ]);
        File.WriteAllText(manifestPath, string.Empty);
        File.WriteAllText(debugManifestPath, "security_enabled=false\n");
        File.WriteAllText(launcherPath, string.Empty);

        var projectPath = Path.Combine(root, "testapp.pjx");
        var previousBuildHostPath = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        var previousRuntimeHostPath = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        var previousUiLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_LOCALE");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", buildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", runtimeHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", "en-US");
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", "en-US");

            var session = CopperfinRuntimeDebugClient.StartSessionAsync(
                projectPath,
                new CopperfinLocalization("pt-BR")).GetAwaiter().GetResult();

            Expect(session.Success, "runtime debug client should succeed against the fake runtime host");
            Expect(string.Equals(session.State.Reason, "entry", StringComparison.Ordinal),
                "runtime debug client should stop the initial session at entry");

            var steppedSession = CopperfinRuntimeDebugClient.StepIntoAsync(
                session,
                new CopperfinLocalization("pt-BR")).GetAwaiter().GetResult();
            Expect(steppedSession.Success &&
                   string.Equals(steppedSession.State.Reason, "step", StringComparison.Ordinal),
                "runtime debug client should advance the live runtime process without replaying the project");
            Expect(steppedSession.Commands.Count == 2 &&
                   string.Equals(steppedSession.Commands[0], "continue", StringComparison.Ordinal) &&
                   string.Equals(steppedSession.Commands[1], "step", StringComparison.Ordinal),
                "runtime debug client should preserve live command history without adding an exit handshake");
            Expect(steppedSession.TransportProcessId > 0,
                "runtime debug client should retain the live runtime-host process identity for diagnostics");
            CopperfinRuntimeDebugClient.Stop(steppedSession);
            Expect(steppedSession.TransportStopCompleted,
                "runtime debug client should confirm that the live runtime-host process stopped");

            var capturedArgs = File.ReadAllText(runtimeArgsPath);
            var capturedEnv = File.ReadAllText(runtimeEnvPath);
            Expect(capturedArgs.Contains("--locale", StringComparison.Ordinal) &&
                   capturedArgs.Contains("pt-BR", StringComparison.Ordinal),
                "runtime debug client should pass the explicit locale through the runtime-host command line");
            Expect(capturedArgs.Contains("--debug-stop-on-entry", StringComparison.Ordinal),
                "runtime debug client should request the explicit entry-stop contract");
            Expect(capturedEnv.Contains("COPPERFIN_UI_LOCALE=pt-BR", StringComparison.Ordinal),
                "runtime debug client should stamp COPPERFIN_UI_LOCALE for the runtime host");
            Expect(capturedEnv.Contains("COPPERFIN_LOCALE=pt-BR", StringComparison.Ordinal),
                "runtime debug client should stamp COPPERFIN_LOCALE for the runtime host");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", previousBuildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", previousRuntimeHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousUiLocale);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", previousLocale);
            TryDelete(root);
        }
    }

    private static void TestProjectWorkflowUsesDistinctOutputDirectoriesForBackToBackBuildsOnPosix()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var root = CreateProjectRoot("workflow_output_dir_uniqueness");
        var captureCountPath = Path.Combine(root, "build_count.txt");
        var outputDir1Path = Path.Combine(root, "output_dir_1.txt");
        var outputDir2Path = Path.Combine(root, "output_dir_2.txt");
        var manifestPath = Path.Combine(root, "app.cfmanifest");
        var debugManifestPath = Path.Combine(root, "app.cfdebug");
        var launcherPath = Path.Combine(root, "launcher");
        var runtimeHostPath = Path.Combine(root, "runtime_host.sh");
        var buildHostPath = CreateTestExecutableScriptPath(
            root,
            "build_host_unique_output",
            [
                "#!/bin/sh",
                $"count=$(cat \"{captureCountPath}\" 2>/dev/null || printf '0')",
                "count=$((count + 1))",
                $"printf '%s' \"$count\" > \"{captureCountPath}\"",
                "output_dir=''",
                "prev=''",
                "for arg in \"$@\"; do",
                "  if [ \"$prev\" = '--output-dir' ]; then",
                "    output_dir=\"$arg\"",
                "    break",
                "  fi",
                "  prev=\"$arg\"",
                "done",
                $"if [ \"$count\" -eq 1 ]; then printf '%s' \"$output_dir\" > \"{outputDir1Path}\"; else printf '%s' \"$output_dir\" > \"{outputDir2Path}\"; fi",
                $"printf 'status: ok\\nmanifest.path: %s\\ndebug.manifest.path: %s\\nlauncher.output: %s\\n' \"{manifestPath}\" \"{debugManifestPath}\" \"{launcherPath}\""
            ]);
        File.WriteAllText(runtimeHostPath, string.Empty);
        File.WriteAllText(manifestPath, string.Empty);
        File.WriteAllText(debugManifestPath, string.Empty);
        File.WriteAllText(launcherPath, string.Empty);

        var projectPath = Path.Combine(root, "testapp.pjx");
        var previousBuildHostPath = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        var previousRuntimeHostPath = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", buildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", runtimeHostPath);

            var first = CopperfinProjectWorkflow.ExecuteAsync(
                projectPath,
                CopperfinProjectOperation.Build,
                new CopperfinLocalization("en-US")).GetAwaiter().GetResult();
            var second = CopperfinProjectWorkflow.ExecuteAsync(
                projectPath,
                CopperfinProjectOperation.Build,
                new CopperfinLocalization("en-US")).GetAwaiter().GetResult();

            Expect(first.Success && second.Success,
                "project workflow should succeed twice against the fake build host");

            var firstOutputDir = File.ReadAllText(outputDir1Path).Trim();
            var secondOutputDir = File.ReadAllText(outputDir2Path).Trim();
            Expect(!string.IsNullOrWhiteSpace(firstOutputDir) &&
                   !string.IsNullOrWhiteSpace(secondOutputDir),
                "project workflow should pass an explicit output directory on each build");
            Expect(!string.Equals(firstOutputDir, secondOutputDir, StringComparison.Ordinal),
                "project workflow should not reuse the same temp output directory for back-to-back builds");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", previousBuildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", previousRuntimeHostPath);
            TryDelete(root);
        }
    }

    private static void TestRuntimeDebugClientCleansTransientReplayManifestOnPosix()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var root = CreateProjectRoot("runtime_debug_manifest_cleanup");
        var manifestPath = Path.Combine(root, "app.cfmanifest");
        var debugManifestPath = Path.Combine(root, "app.cfdebug");
        var launcherPath = Path.Combine(root, "launcher");
        var capturedManifestPath = Path.Combine(root, "runtime_manifest_path.txt");
        var buildHostPath = CreateTestExecutableScriptPath(
            root,
            "build_host_manifest_cleanup",
            [
                "#!/bin/sh",
                $"printf 'status: ok\\nmanifest.path: %s\\ndebug.manifest.path: %s\\nlauncher.output: %s\\n' \"{manifestPath}\" \"{debugManifestPath}\" \"{launcherPath}\""
            ]);
        var runtimeHostPath = CreateTestExecutableScriptPath(
            root,
            "runtime_host_manifest_cleanup",
            [
                "#!/bin/sh",
                "manifest_path=''",
                "prev=''",
                "for arg in \"$@\"; do",
                "  if [ \"$prev\" = '--manifest' ]; then",
                "    manifest_path=\"$arg\"",
                "    break",
                "  fi",
                "  prev=\"$arg\"",
                "done",
                $"printf '%s' \"$manifest_path\" > \"{capturedManifestPath}\"",
                "printf 'debug.command[0]: continue\\n'",
                "printf 'debug.reason: breakpoint\\n'",
                "printf 'debug.location: app/main.prg:12\\n'",
                "printf 'debug.statement: WAIT WINDOW \"hello\"\\n'",
                "printf 'debug.stack.depth: 1\\n'",
                "printf 'debug.executed.statements: 1\\n'"
            ]);
        File.WriteAllText(manifestPath, string.Empty);
        File.WriteAllText(
            debugManifestPath,
            "security_enabled=true\nsecurity_role=build-engineer\n");
        File.WriteAllText(launcherPath, string.Empty);

        var projectPath = Path.Combine(root, "testapp.pjx");
        var previousBuildHostPath = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        var previousRuntimeHostPath = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        var previousSecurityRole = Environment.GetEnvironmentVariable("COPPERFIN_SECURITY_ROLE");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", buildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", runtimeHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SECURITY_ROLE", null);

            var session = CopperfinRuntimeDebugClient.StartSessionAsync(
                projectPath,
                new CopperfinLocalization("en-US")).GetAwaiter().GetResult();

            Expect(session.Success, "runtime debug client should succeed against the fake runtime host");
            var effectiveManifestPath = File.ReadAllText(capturedManifestPath).Trim();
            Expect(!string.IsNullOrWhiteSpace(effectiveManifestPath),
                "runtime debug client should pass a manifest path to the runtime host");
            Expect(!string.Equals(effectiveManifestPath, debugManifestPath, StringComparison.Ordinal),
                "runtime debug client should materialize a transient replay manifest when role remapping is needed");
            Expect(File.Exists(debugManifestPath),
                "runtime debug client should preserve the original debug manifest");
            Expect(!File.Exists(effectiveManifestPath),
                "runtime debug client should clean up the transient replay manifest after the replay completes");
            Expect(!Directory.EnumerateFiles(root, "*.runtime-debug-*.cfdebug", SearchOption.TopDirectoryOnly).Any(),
                "runtime debug client should not leave replay-manifest temp files behind");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", previousBuildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", previousRuntimeHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SECURITY_ROLE", previousSecurityRole);
            TryDelete(root);
        }
    }

    private static void TestProcessRunnerCapturesLargeConcurrentOutput()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", "process_runner_large_output", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        try
        {
            var startInfo = CreateTestScriptStartInfo(
                root,
                "large_output",
                windowsBody:
                [
                    "@echo off",
                    "setlocal EnableExtensions",
                    "for /L %%I in (1,1,4000) do (",
                    "  echo stdout-%%I-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
                    "  echo stderr-%%I-yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy 1>&2",
                    ")",
                    "exit /b 0"
                ],
                unixBody:
                [
                    "#!/bin/sh",
                    "i=1",
                    "while [ \"$i\" -le 4000 ]; do",
                    "  echo \"stdout-$i-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"",
                    "  echo \"stderr-$i-yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\" 1>&2",
                    "  i=$((i + 1))",
                    "done"
                ]);

            var result = CopperfinProcessRunner.Run(startInfo, timeoutMilliseconds: 15000);
            Expect(result.Started, "process runner should start the large-output script");
            Expect(!result.TimedOut, "process runner should drain large concurrent output without timing out");
            Expect(result.ExitCode == 0, "process runner should preserve the child exit code after draining large concurrent output");
            Expect(result.StandardOutput.Contains("stdout-1-", StringComparison.Ordinal) &&
                   result.StandardOutput.Contains("stdout-4000-", StringComparison.Ordinal),
                "process runner should capture the full stdout stream across a large concurrent-output run");
            Expect(result.StandardError.Contains("stderr-1-", StringComparison.Ordinal) &&
                   result.StandardError.Contains("stderr-4000-", StringComparison.Ordinal),
                "process runner should capture the full stderr stream across a large concurrent-output run");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProcessRunnerEnforcesTimeoutWithoutPipeDeadlock()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", "process_runner_timeout", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        var childPidPath = Path.Combine(root, "child.pid");
        var grandchildPidPath = Path.Combine(root, "grandchild.pid");
        var childPid = 0;
        var grandchildPid = 0;
        try
        {
            var processHostPath = Environment.ProcessPath ?? throw new InvalidOperationException("The managed test host path is unavailable.");
            var assemblyPath = typeof(Program).Assembly.Location;
            var assemblyArgument = string.Equals(
                    Path.GetFileNameWithoutExtension(processHostPath),
                    "dotnet",
                    StringComparison.OrdinalIgnoreCase)
                ? " \"" + assemblyPath + "\""
                : string.Empty;
            var startInfo = CreateTestScriptStartInfo(
                root,
                "timeout_output",
                windowsBody:
                [
                    "@echo off",
                    "echo before-timeout-stdout",
                    "echo before-timeout-stderr 1>&2",
                    "\"" + processHostPath + "\"" + assemblyArgument + " " + HoldOutputTreeArgument + " \"" + childPidPath + "\" \"" + grandchildPidPath + "\"",
                    "exit /b 0"
                ],
                unixBody:
                [
                    "#!/bin/sh",
                    "echo before-timeout-stdout",
                    "echo before-timeout-stderr 1>&2",
                    "\"" + processHostPath + "\"" + assemblyArgument + " " + HoldOutputTreeArgument + " \"" + childPidPath + "\" \"" + grandchildPidPath + "\""
                ]);

            // Allow the Windows fixture to start cmd.exe and both managed helper processes before timing out.
            var timeoutMilliseconds = OperatingSystem.IsWindows() ? 5000 : 300;
            var stopwatch = Stopwatch.StartNew();
            var result = CopperfinProcessRunner.Run(startInfo, timeoutMilliseconds);
            stopwatch.Stop();
            Expect(result.Started, "process runner should start the timeout script");
            Expect(result.TimedOut, "process runner should report timeout when the child outlives the configured limit");
            Expect(stopwatch.ElapsedMilliseconds < timeoutMilliseconds + 5000,
                "process runner should return within the bounded timeout cleanup grace period");
            Expect(result.StandardOutput.Contains("before-timeout-stdout", StringComparison.Ordinal),
                "process runner should retain stdout produced before the timeout cut-off");
            Expect(result.StandardError.Contains("before-timeout-stderr", StringComparison.Ordinal),
                "process runner should retain stderr produced before the timeout cut-off");
            Expect(TryReadProcessId(childPidPath, out childPid),
                "process runner timeout fixture should record the child PID");
            Expect(TryReadProcessId(grandchildPidPath, out grandchildPid),
                "process runner timeout fixture should record the grandchild PID");
            if (childPid > 0)
            {
                Expect(WaitForProcessExit(childPid, timeoutMilliseconds: 2000),
                    "process runner timeout cleanup should terminate the child process");
            }
            if (grandchildPid > 0)
            {
                Expect(WaitForProcessExit(grandchildPid, timeoutMilliseconds: 2000),
                    "process runner timeout cleanup should terminate the grandchild process");
            }
        }
        finally
        {
            TryTerminateProcess(grandchildPid);
            TryTerminateProcess(childPid);
            TryDelete(root);
        }
    }

    private static void TestProcessRunnerPreservesSuccessfulExitWhenDescendantHoldsPipe()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", "process_runner_success", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        var descendantPidPath = Path.Combine(root, "descendant.pid");
        var descendantPid = 0;
        try
        {
            var processHostPath = Environment.ProcessPath ?? throw new InvalidOperationException("The managed test host path is unavailable.");
            var assemblyPath = typeof(Program).Assembly.Location;
            var assemblyArgument = string.Equals(
                    Path.GetFileNameWithoutExtension(processHostPath),
                    "dotnet",
                    StringComparison.OrdinalIgnoreCase)
                ? " \"" + assemblyPath + "\""
                : string.Empty;
            var startInfo = CreateTestScriptStartInfo(
                root,
                "successful_output_holder",
                windowsBody:
                [
                    "@echo off",
                    "echo successful-stdout",
                    "start \"\" /b \"" + processHostPath + "\"" + assemblyArgument + " " + HoldOutputHandlesArgument + " \"" + descendantPidPath + "\"",
                    ":wait_for_descendant",
                    "if exist \"" + descendantPidPath + "\" goto descendant_ready",
                    ">nul 2>&1 ping.exe -n 2 127.0.0.1",
                    "goto wait_for_descendant",
                    ":descendant_ready",
                    "exit /b 0"
                ],
                unixBody:
                [
                    "#!/bin/sh",
                    "echo successful-stdout",
                    "\"" + processHostPath + "\"" + assemblyArgument + " " + HoldOutputHandlesArgument + " \"" + descendantPidPath + "\" &",
                    "while [ ! -s \"" + descendantPidPath + "\" ]; do sleep 0.05; done",
                    "exit 0"
                ]);

            const int timeoutMilliseconds = 10000;
            var stopwatch = Stopwatch.StartNew();
            var result = CopperfinProcessRunner.Run(startInfo, timeoutMilliseconds);
            stopwatch.Stop();
            Expect(result.Started, "process runner should start the successful descendant fixture");
            Expect(!result.TimedOut,
                "process runner should preserve successful completion when only a descendant retains output handles");
            Expect(result.ExitCode == 0,
                "process runner should preserve the root exit code when a descendant retains output handles");
            Expect(result.StandardOutput.Contains("successful-stdout", StringComparison.Ordinal),
                "process runner should retain output captured before successful root completion");
            Expect(stopwatch.ElapsedMilliseconds < timeoutMilliseconds,
                "process runner should bound post-exit output draining without converting success into a timeout");
            Expect(TryReadProcessId(descendantPidPath, out descendantPid),
                "successful descendant fixture should record its PID");
        }
        finally
        {
            TryTerminateProcess(descendantPid);
            TryDelete(root);
        }
    }

    private static void TestDottedClassMemberResolvesToLongestProjectSymbolPrefix()
    {
        var root = CreateProjectRoot("dotted_class_prefix");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var resolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "app.customer.editor.Init", out var definition);
            Expect(resolved, "member access should resolve through the longest dotted class prefix");
            if (resolved)
            {
                Expect(definition.Name == "app.customer.editor", "resolved class definition should preserve the full dotted class name");
                Expect(definition.Kind == "class", "resolved dotted class prefix should remain a class definition");
                Expect(definition.FilePath == sourcePath, "resolved dotted class definition should point to the declaring source file");
                Expect(definition.LineNumber == 1, "resolved dotted class definition should keep its declaring line number");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestDottedMemberFallsBackToTrailingProcedureName()
    {
        var root = CreateProjectRoot("trailing_procedure_fallback");
        try
        {
            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var resolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "oToolbar.SaveOrder", out var definition);
            Expect(resolved, "dotted member lookup should still fall back to a trailing procedure symbol");
            if (resolved)
            {
                Expect(definition.Name == "SaveOrder", "trailing procedure fallback should resolve the member name itself");
                Expect(definition.Kind == "procedure", "trailing procedure fallback should resolve a procedure definition");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestQuickInfoUsesResolvedProjectSymbolDescriptionForDottedMemberAccess()
    {
        var root = CreateProjectRoot("dotted_quick_info");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var description = FoxProIntelliSenseCatalog.DescribeToken(sourcePath, "app.customer.editor.Refresh");
            Expect(
                string.Equals(
                    description,
                    CopperfinLocalization.FromVisualStudioUiCulture().Format(
                        "LanguageService.IntelliSense.Project.ClassSymbolDerivingFrom",
                        "custom"),
                    StringComparison.Ordinal),
                "quick info should reuse the resolved dotted class definition description");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectProcedureSignatureHelpUsesLparameters()
    {
        var root = CreateProjectRoot("procedure_signature_help");
        try
        {
            var sourcePath = Path.Combine(root, "orders.prg");
            File.WriteAllText(
                sourcePath,
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "LPARAMETERS tcCustomerId, tlPreview = .F." + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "SaveOrder");
            Expect(signatures.Count == 1, "project procedures with LPARAMETERS should surface one signature entry");
            if (signatures.Count == 1)
            {
                Expect(signatures[0].Content == "SaveOrder(tcCustomerId, tlPreview = .F.)", "project procedure signature help should preserve raw parameter text in the signature content");
                Expect(signatures[0].Parameters.Count == 2, "project procedure signature help should surface each LPARAMETERS argument");
                Expect(signatures[0].Parameters[0].Name == "tcCustomerId", "project procedure signature help should normalize the first parameter name");
                Expect(signatures[0].Parameters[1].Name == "tlPreview", "project procedure signature help should normalize defaulted parameter names");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectProcedureSignatureHelpPreservesNestedDefaultExpressions()
    {
        var root = CreateProjectRoot("nested_parameter_default_signature_help");
        try
        {
            var sourcePath = Path.Combine(root, "defaults.prg");
            File.WriteAllText(
                sourcePath,
                "PROCEDURE Configure" + Environment.NewLine +
                "LPARAMETERS toOptions = CREATEOBJECT(\"Collection\", 1), tcCaption = \"Save, then close\", tcEscaped = 'it''s, safe', taBounds = [1, 2], tcNext" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "Configure");
            Expect(signatures.Count == 1,
                "project signature help should surface a declaration with nested and quoted default-expression commas");
            if (signatures.Count == 1)
            {
                var signature = signatures[0];
                Expect(signature.Parameters.Count == 5,
                    "nested calls, bracket expressions, and quoted commas should not create phantom parameters");
                Expect(signature.Parameters.Select(parameter => parameter.Name).SequenceEqual(
                        new[] { "toOptions", "tcCaption", "tcEscaped", "taBounds", "tcNext" }),
                    "nested default expressions should preserve invariant project parameter names");
                Expect(signature.Parameters[0].Documentation == "toOptions = CREATEOBJECT(\"Collection\", 1)" &&
                       signature.Parameters[1].Documentation == "tcCaption = \"Save, then close\"" &&
                       signature.Parameters[2].Documentation == "tcEscaped = 'it''s, safe'" &&
                       signature.Parameters[3].Documentation == "taBounds = [1, 2]",
                    "project signature help should preserve raw default-expression text");
                Expect(signature.Content ==
                       "Configure(toOptions = CREATEOBJECT(\"Collection\", 1), tcCaption = \"Save, then close\", tcEscaped = 'it''s, safe', taBounds = [1, 2], tcNext)",
                    "project signature content should retain nested and quoted default-expression commas");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectSignatureHelpUsesInlineRoutineParameters()
    {
        var root = CreateProjectRoot("inline_parameter_signature_help");
        try
        {
            var sourcePath = Path.Combine(root, "inline.prg");
            File.WriteAllText(
                sourcePath,
                "PROCEDURE SaveOrder(tcCustomerId, tlPreview = .F.)" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "FUNCTION BuildOptions(toSeed = CREATEOBJECT(\"Collection\", 1), tcCaption = 'Save, close')" + Environment.NewLine +
                "ENDFUNC" + Environment.NewLine +
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROCEDURE Refresh(tcScope, taBounds = [1, 2])" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var procedure = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "SaveOrder");
            Expect(procedure.Count == 1 &&
                   procedure[0].Content == "SaveOrder(tcCustomerId, tlPreview = .F.)" &&
                   procedure[0].Parameters.Select(parameter => parameter.Name).SequenceEqual(
                       new[] { "tcCustomerId", "tlPreview" }),
                "inline PROCEDURE parameters should surface in project signature help");

            var function = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "BuildOptions");
            Expect(function.Count == 1 && function[0].Parameters.Count == 2 &&
                   function[0].Content ==
                       "BuildOptions(toSeed = CREATEOBJECT(\"Collection\", 1), tcCaption = 'Save, close')" &&
                   function[0].Parameters[0].Name == "toSeed" &&
                   function[0].Parameters[1].Name == "tcCaption",
                "inline FUNCTION parameters should retain nested and quoted default-expression commas");

            var method = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "app.customer.editor.Refresh");
            Expect(method.Count == 1 && method[0].Parameters.Count == 2 &&
                   method[0].Content == "app.customer.editor.Refresh(tcScope, taBounds = [1, 2])" &&
                   method[0].Parameters[0].Name == "tcScope" &&
                   method[0].Parameters[1].Name == "taBounds",
                "inline class-method parameters should surface with qualified project identity");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectLanguageServiceRecognizesProcAbbreviation()
    {
        var root = CreateProjectRoot("proc_abbreviation_project_symbols");
        try
        {
            var sourcePath = Path.Combine(root, "abbreviated.prg");
            File.WriteAllText(
                sourcePath,
                "PROC SaveOrder" + Environment.NewLine +
                "LPARAMETERS tcCustomerId" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROC Refresh(tcScope, tlForce = .F.)" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine +
                "PROCEDURES NotARoutine" + Environment.NewLine);

            var procedure = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "SaveOrder");
            Expect(procedure.Count == 1 && procedure[0].Content == "SaveOrder(tcCustomerId)" &&
                   procedure[0].Parameters.Count == 1 && procedure[0].Parameters[0].Name == "tcCustomerId",
                "PROC declarations should retain following-line project signature discovery");
            Expect(FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "SaveOrder", out var procedureDefinition) &&
                   procedureDefinition.Kind == "procedure" && procedureDefinition.LineNumber == 1,
                "PROC declarations should retain project definition provenance");

            var methodName = "app.customer.editor.Refresh";
            var method = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, methodName);
            Expect(method.Count == 1 && method[0].Content ==
                       "app.customer.editor.Refresh(tcScope, tlForce = .F.)" &&
                   method[0].Parameters.Select(parameter => parameter.Name).SequenceEqual(
                       new[] { "tcScope", "tlForce" }),
                "class PROC declarations should retain qualified inline signature discovery");
            Expect(FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, methodName, out var methodDefinition) &&
                   methodDefinition.Kind == "method" && methodDefinition.LineNumber == 5,
                "class PROC declarations should retain qualified definition provenance");

            Expect(FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "NotARoutine").Count == 0 &&
                   !FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "NotARoutine", out _),
                "longer identifiers beginning with PROCEDURE should not become routine declarations");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectLanguageServiceDiscoversVisibilityQualifiedMethods()
    {
        var root = CreateProjectRoot("visibility_qualified_project_methods");
        try
        {
            var sourcePath = Path.Combine(root, "visibility.prg");
            File.WriteAllText(
                sourcePath,
                "PROTECTED PROCEDURE NotGlobal" + Environment.NewLine +
                "HIDDEN FUNCTION AlsoNotGlobal" + Environment.NewLine +
                "DEFINE CLASS app.secure.editor AS custom" + Environment.NewLine +
                "PROTECTED PROC SaveInternal" + Environment.NewLine +
                "LPARAMETERS tcRecordId" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "HIDDEN FUNCTION CalculateSecret(tnSeed, tlRefresh = .F.)" + Environment.NewLine +
                "ENDFUNC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var protectedName = "app.secure.editor.SaveInternal";
            var protectedSignature = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, protectedName);
            Expect(protectedSignature.Count == 1 &&
                   protectedSignature[0].Content == "app.secure.editor.SaveInternal(tcRecordId)" &&
                   protectedSignature[0].Parameters.Count == 1 &&
                   protectedSignature[0].Parameters[0].Name == "tcRecordId",
                "PROTECTED PROC methods should retain following-line project signatures");
            Expect(FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, protectedName, out var protectedDefinition) &&
                   protectedDefinition.Kind == "method" && protectedDefinition.LineNumber == 4,
                "PROTECTED PROC methods should retain qualified definition provenance");

            var hiddenName = "app.secure.editor.CalculateSecret";
            var hiddenSignature = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, hiddenName);
            Expect(hiddenSignature.Count == 1 &&
                   hiddenSignature[0].Content ==
                       "app.secure.editor.CalculateSecret(tnSeed, tlRefresh = .F.)" &&
                   hiddenSignature[0].Parameters.Select(parameter => parameter.Name).SequenceEqual(
                       new[] { "tnSeed", "tlRefresh" }),
                "HIDDEN FUNCTION methods should retain inline project signatures");
            Expect(FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, hiddenName, out var hiddenDefinition) &&
                   hiddenDefinition.Kind == "method" && hiddenDefinition.LineNumber == 7,
                "HIDDEN FUNCTION methods should retain qualified definition provenance");

            Expect(FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "NotGlobal").Count == 0 &&
                   !FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "NotGlobal", out _) &&
                   FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "AlsoNotGlobal").Count == 0 &&
                   !FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "AlsoNotGlobal", out _),
                "visibility-qualified declarations outside a class should not become global symbols");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectProcedureSignatureHelpUsesSingularLparameterForDottedMethod()
    {
        var root = CreateProjectRoot("singular_lparameter_signature_help");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "LPARAMETER tcCustomerId, tlPreview = .F." + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "oEditor.SaveOrder");
            Expect(signatures.Count == 1,
                "singular LPARAMETER declarations should surface dotted project method signature help");
            if (signatures.Count == 1)
            {
                Expect(signatures[0].Content == "app.customer.editor.SaveOrder(tcCustomerId, tlPreview = .F.)",
                    "singular LPARAMETER signature help should preserve the qualified method and defaulted parameter text");
                Expect(signatures[0].Parameters.Count == 2,
                    "singular LPARAMETER signature help should expose each parameter");
                Expect(signatures[0].Parameters[0].Name == "tcCustomerId" &&
                       signatures[0].Parameters[1].Name == "tlPreview",
                    "singular LPARAMETER signature help should normalize parameter names");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectProcedureSignatureHelpRejectsBareParameter()
    {
        var root = CreateProjectRoot("bare_parameter_signature_help");
        try
        {
            var sourcePath = Path.Combine(root, "orders.prg");
            File.WriteAllText(
                sourcePath,
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "PARAMETER tcCustomerId" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "SaveOrder");
            Expect(signatures.Count == 1,
                "bare PARAMETER should still leave the procedure symbol available");
            if (signatures.Count == 1)
            {
                Expect(signatures[0].Content == "SaveOrder()" && signatures[0].Parameters.Count == 0,
                    "bare PARAMETER should not be treated as a VFP parameter declaration");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectProcedureSignatureHelpFallsBackFromDottedInvocation()
    {
        var root = CreateProjectRoot("dotted_signature_help");
        try
        {
            var sourcePath = Path.Combine(root, "orders.prg");
            File.WriteAllText(
                sourcePath,
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "PARAMETERS tcCustomerId" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "oToolbar.SaveOrder");
            Expect(signatures.Count == 1, "dotted invocation names should still surface project procedure signature help");
            if (signatures.Count == 1)
            {
                Expect(signatures[0].Content == "SaveOrder(tcCustomerId)", "dotted invocation signature help should fall back to the trailing procedure symbol");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestSignatureInvocationParserIgnoresCommentsAndStrings()
    {
        var quoted = "MESSAGEBOX(\"text, with (punctuation)\", 2, ";
        Expect(FoxProInvocationParser.TryParse(quoted, quoted.Length, out var quotedResult) &&
               quotedResult.InvocationName == "MESSAGEBOX" &&
               quotedResult.ParameterIndex == 2,
               "signature invocation parsing should ignore commas and parentheses inside quoted strings");

        var nested = "MESSAGEBOX(Other(1, 2), 3, ";
        Expect(FoxProInvocationParser.TryParse(nested, nested.Length, out var nestedResult) &&
               nestedResult.ParameterIndex == 2,
               "signature invocation parsing should count only top-level commas in nested calls");

        var inlineComment = "MESSAGEBOX(1 && ignored(2, 3)";
        var beforeComment = inlineComment.IndexOf("&&", StringComparison.Ordinal);
        Expect(beforeComment > 0 &&
               FoxProInvocationParser.TryParse(inlineComment, beforeComment, out var inlineCommentResult) &&
               inlineCommentResult.InvocationName == "MESSAGEBOX" &&
               inlineCommentResult.ParameterIndex == 0,
               "signature invocation parsing should ignore FoxPro inline comments");

        var commentLine = "* MESSAGEBOX(fake(1, 2)" + Environment.NewLine + "MESSAGEBOX(1, ";
        Expect(FoxProInvocationParser.TryParse(commentLine, commentLine.Length, out var commentLineResult) &&
               commentLineResult.InvocationName == "MESSAGEBOX" &&
               commentLineResult.ParameterIndex == 1,
               "signature invocation parsing should ignore star comment lines");

        var insideString = "MESSAGEBOX(\"still typing";
        Expect(!FoxProInvocationParser.TryParse(insideString, insideString.Length, out _),
               "signature invocation parsing should not activate inside a quoted string");
    }

    private static void TestProjectInsightsCollectDirectAndDottedProcedureCallReferences()
    {
        var root = CreateProjectRoot("call_reference_navigation");
        try
        {
            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "? SaveOrder('ALFKI')" + Environment.NewLine +
                "? oToolbar.SaveOrder('ANTON')" + Environment.NewLine);

            var snapshot = new CopperfinStudioSnapshotDocument
            {
                Path = Path.Combine(root, "testapp.pjx"),
                AssetFamily = "project",
                ProjectWorkspace = new CopperfinStudioProjectWorkspace
                {
                    Entries =
                    {
                        new CopperfinStudioProjectEntry
                        {
                            Name = "main.prg",
                            RelativePath = "main.prg",
                            GroupId = "programs",
                            GroupTitle = "Programs",
                            TypeTitle = "Program"
                        }
                    }
                }
            };

            var insights = CopperfinProjectInsightClient.BuildInsights(snapshot);
            var references = insights.RuntimeReferences.FindAll(reference => reference.Name == "SaveOrder");
            Expect(references.Count == 2, "project insights should collect direct and dotted procedure call references");
            Expect(references.Exists(reference => reference.Kind == "call" && reference.Detail.Contains("SaveOrder('ALFKI')", StringComparison.Ordinal)),
                "project insights should keep direct procedure call detail");
            Expect(references.Exists(reference => reference.Kind == "call.member" && reference.Detail.Contains("oToolbar.SaveOrder('ANTON')", StringComparison.Ordinal)),
                "project insights should keep dotted procedure call detail");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectInsightsRejectPathsOutsideProjectRoot()
    {
        var root = CreateProjectRoot("project_insight_path_containment");
        var outsidePath = Path.Combine(Path.GetDirectoryName(root)!, "outside-insight.prg");
        try
        {
            var insidePath = Path.Combine(root, "inside.prg");
            File.WriteAllText(
                insidePath,
                "PROCEDURE InsideSymbol" + Environment.NewLine +
                "* TODO: keep in-project insight coverage" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);
            File.WriteAllText(
                outsidePath,
                "PROCEDURE OutsideSymbol" + Environment.NewLine +
                "* TODO: must not be inspected" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var snapshot = new CopperfinStudioSnapshotDocument
            {
                Path = Path.Combine(root, "testapp.pjx"),
                AssetFamily = "project",
                ProjectWorkspace = new CopperfinStudioProjectWorkspace
                {
                    Entries =
                    {
                        new CopperfinStudioProjectEntry
                        {
                            Name = "inside.prg",
                            RelativePath = "inside.prg",
                            GroupId = "programs",
                            GroupTitle = "Programs",
                            TypeTitle = "Program"
                        },
                        new CopperfinStudioProjectEntry
                        {
                            Name = "outside-traversal.prg",
                            RelativePath = "..\\outside-insight.prg",
                            GroupId = "programs",
                            GroupTitle = "Programs",
                            TypeTitle = "Program"
                        },
                        new CopperfinStudioProjectEntry
                        {
                            Name = outsidePath,
                            GroupId = "programs",
                            GroupTitle = "Programs",
                            TypeTitle = "Program"
                        }
                    }
                }
            };

            var insights = CopperfinProjectInsightClient.BuildInsights(snapshot);
            Expect(insights.DefinedSymbols.Exists(symbol => symbol.Name == "InsideSymbol"),
                "project insights should continue reading valid in-root PRG entries");
            Expect(!insights.DefinedSymbols.Exists(symbol => symbol.Name == "OutsideSymbol"),
                "project insights should not read PRG entries through parent traversal or rooted names");
            Expect(insights.TaskItems.All(task => task.FilePath == insidePath),
                "project insights should keep task items inside the normalized PJX root");
            Expect(insights.Warnings.Count >= 2,
                "project insights should report unresolved traversal and rooted entries without reading them");
        }
        finally
        {
            TryDelete(root);
            TryDelete(outsidePath);
        }
    }

    private static void TestRenamePreviewCollectsDefinitionAndNormalizedReferences()
    {
        var root = CreateProjectRoot("rename_preview");
        try
        {
            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "? SaveOrder('ALFKI')" + Environment.NewLine +
                "? oToolbar.SaveOrder('ANTON')" + Environment.NewLine);

            var snapshot = new CopperfinStudioSnapshotDocument
            {
                Path = Path.Combine(root, "testapp.pjx"),
                AssetFamily = "project",
                ProjectWorkspace = new CopperfinStudioProjectWorkspace
                {
                    Entries =
                    {
                        new CopperfinStudioProjectEntry
                        {
                            Name = "main.prg",
                            RelativePath = "main.prg",
                            GroupId = "programs",
                            GroupTitle = "Programs",
                            TypeTitle = "Program"
                        }
                    }
                }
            };

            var preview = CopperfinProjectInsightClient.BuildRenamePreview(snapshot, "oToolbar.SaveOrder");
            Expect(preview.SymbolName == "SaveOrder", "rename preview should normalize dotted symbols to the trailing project symbol");
            Expect(preview.Occurrences.Count == 3, "rename preview should include the definition and both collected call references");
            Expect(preview.Occurrences.Exists(occurrence => occurrence.Kind == "definition" && occurrence.Detail.Contains("PROCEDURE SaveOrder", StringComparison.Ordinal)),
                "rename preview should include the defining procedure");
            Expect(preview.Occurrences.Exists(occurrence => occurrence.Kind == "reference" && occurrence.Detail.Contains("SaveOrder('ALFKI')", StringComparison.Ordinal)),
                "rename preview should include direct call references");
            Expect(preview.Occurrences.Exists(occurrence => occurrence.Kind == "reference" && occurrence.Detail.Contains("oToolbar.SaveOrder('ANTON')", StringComparison.Ordinal)),
                "rename preview should include dotted call references");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestProjectInsightsCollectQualifiedAndInstanceStyleMethodCallReferences()
    {
        var root = CreateProjectRoot("method_call_reference_navigation");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine +
                "? app.customer.editor.SaveOrder('ALFKI')" + Environment.NewLine +
                "? oToolbar.SaveOrder('ANTON')" + Environment.NewLine);

            var snapshot = new CopperfinStudioSnapshotDocument
            {
                Path = Path.Combine(root, "testapp.pjx"),
                AssetFamily = "project",
                ProjectWorkspace = new CopperfinStudioProjectWorkspace
                {
                    Entries =
                    {
                        new CopperfinStudioProjectEntry
                        {
                            Name = "classes.prg",
                            RelativePath = "classes.prg",
                            GroupId = "programs",
                            GroupTitle = "Programs",
                            TypeTitle = "Program"
                        }
                    }
                }
            };

            var insights = CopperfinProjectInsightClient.BuildInsights(snapshot);
            var references = insights.RuntimeReferences.FindAll(reference => reference.Name == "app.customer.editor.SaveOrder");
            Expect(references.Count == 2, "project insights should collect qualified and instance-style method call references against the method symbol");
            Expect(references.Exists(reference => reference.Kind == "call.member" && reference.Detail.Contains("app.customer.editor.SaveOrder('ALFKI')", StringComparison.Ordinal)),
                "project insights should keep fully-qualified method calls in the method reference set");
            Expect(references.Exists(reference => reference.Kind == "call.member" && reference.Detail.Contains("oToolbar.SaveOrder('ANTON')", StringComparison.Ordinal)),
                "project insights should normalize instance-style method calls onto the project method symbol");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestRenamePreviewCollectsProjectMethodDefinitionAndReferences()
    {
        var root = CreateProjectRoot("method_rename_preview");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine +
                "? app.customer.editor.SaveOrder('ALFKI')" + Environment.NewLine +
                "? oToolbar.SaveOrder('ANTON')" + Environment.NewLine);

            var snapshot = new CopperfinStudioSnapshotDocument
            {
                Path = Path.Combine(root, "testapp.pjx"),
                AssetFamily = "project",
                ProjectWorkspace = new CopperfinStudioProjectWorkspace
                {
                    Entries =
                    {
                        new CopperfinStudioProjectEntry
                        {
                            Name = "classes.prg",
                            RelativePath = "classes.prg",
                            GroupId = "programs",
                            GroupTitle = "Programs",
                            TypeTitle = "Program"
                        }
                    }
                }
            };

            var preview = CopperfinProjectInsightClient.BuildRenamePreview(snapshot, "oToolbar.SaveOrder");
            Expect(preview.SymbolName == "app.customer.editor.SaveOrder",
                "rename preview should normalize instance-style method names to the unique fully-qualified project method");
            Expect(preview.Occurrences.Count == 3, "rename preview should include the method definition and both collected method call references");
            Expect(preview.Occurrences.Exists(occurrence => occurrence.Kind == "definition" && occurrence.Detail.Contains("PROCEDURE SaveOrder", StringComparison.Ordinal)),
                "rename preview should include the defining method");
            Expect(preview.Occurrences.Exists(occurrence => occurrence.Kind == "reference" && occurrence.Detail.Contains("app.customer.editor.SaveOrder('ALFKI')", StringComparison.Ordinal)),
                "rename preview should include fully-qualified method call references");
            Expect(preview.Occurrences.Exists(occurrence => occurrence.Kind == "reference" && occurrence.Detail.Contains("oToolbar.SaveOrder('ANTON')", StringComparison.Ordinal)),
                "rename preview should include normalized instance-style method call references");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestCompletionCatalogIngestsCreateCursorAndIntoCursorAliases()
    {
        var root = CreateProjectRoot("cursor_metadata_ingestion");
        try
        {
            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                "CREATE CURSOR curLocal (id I)" + Environment.NewLine +
                "SELECT * FROM customer INTO CURSOR curRemote" + Environment.NewLine);

            var completions = FoxProIntelliSenseCatalog.BuildEntries(sourcePath, "SELECT ", string.Empty);
            Expect(completions.Any(entry => entry.DisplayText == "curLocal" && entry.Kind == "alias"),
                "completion catalog should ingest CREATE CURSOR aliases");
            Expect(completions.Any(entry => entry.DisplayText == "curRemote" && entry.Kind == "alias"),
                "completion catalog should ingest SELECT ... INTO CURSOR aliases");

            var description = FoxProIntelliSenseCatalog.DescribeToken(sourcePath, "curRemote");
            Expect(
                string.Equals(
                    description,
                    CopperfinLocalization.FromVisualStudioUiCulture().Text("LanguageService.IntelliSense.Project.CursorAliasDiscovered"),
                    StringComparison.Ordinal),
                "described cursor aliases should reuse the alias metadata description");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestCompletionCatalogIngestsImplicitUseAndSqlExecAliases()
    {
        var root = CreateProjectRoot("source_derived_alias_ingestion");
        try
        {
            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                "USE data/orderslive.dbf IN 0 SHARED" + Environment.NewLine +
                "nResult = SQLEXEC(nConn, \"SELECT id, company FROM customer\", \"sqlorders\")" + Environment.NewLine);

            var completions = FoxProIntelliSenseCatalog.BuildEntries(sourcePath, "SELECT ", string.Empty);
            Expect(completions.Any(entry => entry.DisplayText == "orderslive" && entry.Kind == "alias"),
                "completion catalog should infer the default alias from USE path opens");
            Expect(completions.Any(entry => entry.DisplayText == "sqlorders" && entry.Kind == "alias"),
                "completion catalog should ingest literal SQLEXEC target cursor aliases");

            var useDescription = FoxProIntelliSenseCatalog.DescribeToken(sourcePath, "orderslive");
            Expect(
                string.Equals(
                    useDescription,
                    CopperfinLocalization.FromVisualStudioUiCulture().Text("LanguageService.IntelliSense.Project.WorkAreaAliasDiscovered"),
                    StringComparison.Ordinal),
                "implicit USE aliases should reuse the work-area alias description");

            var sqlDescription = FoxProIntelliSenseCatalog.DescribeToken(sourcePath, "sqlorders");
            Expect(
                string.Equals(
                    sqlDescription,
                    CopperfinLocalization.FromVisualStudioUiCulture().Text("LanguageService.IntelliSense.Project.CursorAliasDiscovered"),
                    StringComparison.Ordinal),
                "SQLEXEC target aliases should reuse the cursor alias description");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestSelectContextKeepsAliasCompletionsAheadOfGlobalProcedureSymbols()
    {
        var root = CreateProjectRoot("select_context_ranking");
        try
        {
            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                "CREATE CURSOR curLocal (id I)" + Environment.NewLine +
                "PROCEDURE curProc" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var completions = FoxProIntelliSenseCatalog.BuildEntries(sourcePath, "SELECT ", "cur");
            Expect(completions.Count >= 2, "select context should return both alias and procedure candidates for the shared prefix");
            if (completions.Count >= 2)
            {
                Expect(completions[0].DisplayText == "curLocal" && completions[0].Kind == "alias",
                    "select context should rank alias completions ahead of unrelated global procedure symbols");
                Expect(
                    completions[0].Description == CopperfinLocalization.FromVisualStudioUiCulture().Text("LanguageService.IntelliSense.Project.WorkAreaAliasFromProjectSource"),
                    "select context should preserve the context-specific alias description instead of overwriting it with the global symbol entry");
                Expect(completions.Any(entry => entry.DisplayText == "curProc" && entry.Kind == "symbol"),
                    "select context should still keep global procedure symbols available after the context-ranked alias");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestQualifiedProjectMethodSignatureHelpAndDefinition()
    {
        var root = CreateProjectRoot("qualified_method_signature_help");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "LPARAMETERS tcCustomerId, tlPreview = .F." + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var qualifiedName = "app.customer.editor.SaveOrder";
            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, qualifiedName);
            Expect(signatures.Count == 1, "qualified project methods should surface signature help");
            if (signatures.Count == 1)
            {
                Expect(signatures[0].Content == "app.customer.editor.SaveOrder(tcCustomerId, tlPreview = .F.)",
                    "qualified project method signature help should preserve the full method path and raw parameter text");
            }

            var resolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, qualifiedName, out var definition);
            Expect(resolved, "qualified project methods should resolve to their method definition");
            if (resolved)
            {
                Expect(definition.Kind == "method", "qualified project methods should resolve as method definitions");
                Expect(definition.LineNumber == 2, "qualified project methods should resolve to the procedure line inside the class");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestMemberAccessCompletionsIncludeProjectMethodsAheadOfGenericMembers()
    {
        var root = CreateProjectRoot("project_member_completion");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var completions = FoxProIntelliSenseCatalog.BuildEntries(sourcePath, "oEditor.", "S");
            Expect(completions.Count > 0, "member access context should return completion candidates");
            if (completions.Count > 0)
            {
                Expect(completions[0].DisplayText == "SaveOrder" && completions[0].Kind == "member",
                    "member access context should rank project-defined methods ahead of generic fallback members");
                Expect(
                    completions[0].Description == CopperfinLocalization.FromVisualStudioUiCulture().Format(
                        "LanguageService.IntelliSense.Project.MethodMemberFromType",
                        "app.customer.editor"),
                    "member access context should surface the originating class path for project method members");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestInstanceStyleProjectMethodFallbackUsesUniqueTrailingMethodName()
    {
        var root = CreateProjectRoot("instance_method_fallback");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "LPARAMETERS tcCustomerId" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "oEditor.SaveOrder");
            Expect(signatures.Count == 1, "instance-style method tokens should surface signature help when the trailing method name is unique");
            if (signatures.Count == 1)
            {
                Expect(signatures[0].Content == "app.customer.editor.SaveOrder(tcCustomerId)",
                    "instance-style method fallback should reuse the unique project method signature");
            }

            var resolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "oEditor.SaveOrder", out var definition);
            Expect(resolved, "instance-style method tokens should resolve to a unique project method definition");
            if (resolved)
            {
                Expect(definition.Kind == "method", "instance-style method fallback should resolve a method definition");
                Expect(definition.Name == "app.customer.editor.SaveOrder", "instance-style method fallback should resolve the unique fully-qualified project method");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestInstanceStyleProjectMethodFallbackAvoidsAmbiguousMatches()
    {
        var root = CreateProjectRoot("ambiguous_instance_method_fallback");
        try
        {
            var sourcePath = Path.Combine(root, "classes.prg");
            File.WriteAllText(
                sourcePath,
                "DEFINE CLASS app.customer.editor AS custom" + Environment.NewLine +
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine +
                "DEFINE CLASS app.invoice.editor AS custom" + Environment.NewLine +
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine +
                "ENDDEFINE" + Environment.NewLine);

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "oEditor.SaveOrder");
            Expect(signatures.Count == 0, "instance-style method fallback should not guess when multiple project methods share the same trailing name");

            var resolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "oEditor.SaveOrder", out _);
            Expect(!resolved, "instance-style method fallback should not resolve an ambiguous trailing method name");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestIncludedHeaderOutsideProjectRootFeedsDefineResolution()
    {
        var root = CreateProjectRoot("include_define_resolution");
        var externalRoot = Path.Combine(Path.GetTempPath(), "copperfin_language_service_external_includes", Guid.NewGuid().ToString("N"));
        try
        {
            Directory.CreateDirectory(externalRoot);
            var headerPath = Path.Combine(externalRoot, "shareddefs.h");
            File.WriteAllText(headerPath, "#DEFINE ORDER_STATUS_READY" + Environment.NewLine);

            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                $"#INCLUDE \"{headerPath.Replace("\\", "\\\\")}\"" + Environment.NewLine +
                "? ORDER_STATUS_READY" + Environment.NewLine);

            var resolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "ORDER_STATUS_READY", out var definition);
            Expect(resolved, "included headers outside the project root should feed define resolution");
            if (resolved)
            {
                Expect(definition.FilePath == headerPath, "define resolution should point to the included external header path");
                Expect(definition.LineNumber == 1, "define resolution should point to the defining line inside the included header");
            }

            var description = FoxProIntelliSenseCatalog.DescribeToken(sourcePath, "ORDER_STATUS_READY");
            Expect(string.Equals(
                    description,
                    CopperfinLocalization.FromVisualStudioUiCulture().Text("LanguageService.IntelliSense.Project.PreprocessorSymbol"),
                    StringComparison.Ordinal),
                "included external header defines should participate in token description lookup");
        }
        finally
        {
            TryDelete(root);
            TryDelete(externalRoot);
        }
    }

    private static void TestUnquotedIncludesFeedRecursiveDefineResolution()
    {
        var root = CreateProjectRoot("unquoted_include_define_resolution");
        var externalRoot = Path.Combine(Path.GetTempPath(), "copperfin_language_service_unquoted_includes", Guid.NewGuid().ToString("N"));
        try
        {
            Directory.CreateDirectory(externalRoot);
            var nestedHeaderPath = Path.Combine(externalRoot, "foxpro_reporting.h");
            File.WriteAllText(nestedHeaderPath, "#DEFINE REPORT_STATUS_READY" + Environment.NewLine);

            var headerPath = Path.Combine(externalRoot, "frxBuilder.h");
            File.WriteAllText(
                headerPath,
                "#DEFINE BUILDER_STATUS_READY" + Environment.NewLine +
                "#include foxpro_reporting.h && common VFP include form" + Environment.NewLine);

            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                $"#include {headerPath} && trailing comments are not part of the path" + Environment.NewLine +
                "? BUILDER_STATUS_READY, REPORT_STATUS_READY" + Environment.NewLine);

            var directResolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "BUILDER_STATUS_READY", out var directDefinition);
            Expect(directResolved, "unquoted includes should feed define resolution");
            if (directResolved)
            {
                Expect(directDefinition.FilePath == headerPath, "unquoted include resolution should point to the direct header");
                Expect(directDefinition.LineNumber == 1, "unquoted include resolution should preserve the direct definition line");
            }

            var recursiveResolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "REPORT_STATUS_READY", out var recursiveDefinition);
            Expect(recursiveResolved, "unquoted includes should participate in recursive header scanning");
            if (recursiveResolved)
            {
                Expect(recursiveDefinition.FilePath == nestedHeaderPath, "recursive unquoted include resolution should point to the nested header");
                Expect(recursiveDefinition.LineNumber == 1, "recursive unquoted include resolution should preserve the nested definition line");
            }

            var description = FoxProIntelliSenseCatalog.DescribeToken(sourcePath, "REPORT_STATUS_READY");
            Expect(string.Equals(
                    description,
                    CopperfinLocalization.FromVisualStudioUiCulture().Text("LanguageService.IntelliSense.Project.PreprocessorSymbol"),
                    StringComparison.Ordinal),
                "defines from recursive unquoted includes should participate in token description lookup");
        }
        finally
        {
            TryDelete(root);
            TryDelete(externalRoot);
        }
    }

    private static void TestExternalMixedCaseIncludesResolveUniqueFilesystemSpelling()
    {
        var root = CreateProjectRoot("mixed_case_include_resolution");
        var externalRoot = Path.Combine(Path.GetTempPath(), "copperfin_language_service_mixed_case_includes", Guid.NewGuid().ToString("N"));
        try
        {
            Directory.CreateDirectory(externalRoot);

            var nestedHeaderPath = Path.Combine(externalRoot, "NESTEDDEFS.H");
            File.WriteAllText(nestedHeaderPath, "#DEFINE MIXED_CASE_NESTED_READY" + Environment.NewLine);

            var headerPath = Path.Combine(externalRoot, "frxbuilder.h");
            File.WriteAllText(
                headerPath,
                "#DEFINE MIXED_CASE_BUILDER_READY" + Environment.NewLine +
                "#include nesteddefs.h" + Environment.NewLine);

            var exactCollisionPath = Path.Combine(externalRoot, "exactcase.h");
            File.WriteAllText(exactCollisionPath, "#DEFINE EXACT_CASE_INCLUDE_READY" + Environment.NewLine);
            var exactFoldedSiblingPath = Path.Combine(externalRoot, "EXACTCASE.H");
            var supportsCaseDistinctFiles = !File.Exists(exactFoldedSiblingPath);
            if (supportsCaseDistinctFiles)
            {
                File.WriteAllText(exactFoldedSiblingPath, "#DEFINE EXACT_FOLDED_SIBLING_ONLY" + Environment.NewLine);
                File.WriteAllText(Path.Combine(externalRoot, "ambiguous.h"), "#DEFINE AMBIGUOUS_LOWER_ONLY" + Environment.NewLine);
                File.WriteAllText(Path.Combine(externalRoot, "AMBIGUOUS.H"), "#DEFINE AMBIGUOUS_UPPER_ONLY" + Environment.NewLine);
            }

            var sourcePath = Path.Combine(root, "main.prg");
            var source =
                $"#include {Path.Combine(externalRoot, "frxBuilder.h")}" + Environment.NewLine +
                $"#include {exactCollisionPath}" + Environment.NewLine;
            if (supportsCaseDistinctFiles)
            {
                source += $"#include {Path.Combine(externalRoot, "AmBiGuOuS.H")}" + Environment.NewLine;
            }
            File.WriteAllText(sourcePath, source);

            var directResolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "MIXED_CASE_BUILDER_READY", out var directDefinition);
            Expect(directResolved, "a unique mixed-case external include should feed define resolution");
            if (directResolved)
            {
                Expect(directDefinition.FilePath == headerPath, "mixed-case external include resolution should preserve actual filesystem spelling");
            }

            var recursiveResolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "MIXED_CASE_NESTED_READY", out var recursiveDefinition);
            Expect(recursiveResolved, "recursive external includes should use unique mixed-case filesystem resolution");
            if (recursiveResolved)
            {
                Expect(recursiveDefinition.FilePath == nestedHeaderPath, "recursive mixed-case resolution should preserve actual filesystem spelling");
            }

            var exactResolved = FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "EXACT_CASE_INCLUDE_READY", out var exactDefinition);
            Expect(exactResolved, "an exact include match should win when case-colliding external files exist");
            if (exactResolved)
            {
                Expect(exactDefinition.FilePath == exactCollisionPath, "exact include resolution should retain the exact matched path");
            }

            if (supportsCaseDistinctFiles)
            {
                Expect(!FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "AMBIGUOUS_LOWER_ONLY", out _),
                    "a case-folded external include ambiguity should not select the lowercase candidate");
                Expect(!FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "AMBIGUOUS_UPPER_ONLY", out _),
                    "a case-folded external include ambiguity should not select the uppercase candidate");
                Expect(!FoxProIntelliSenseCatalog.TryResolveDefinition(sourcePath, "EXACT_FOLDED_SIBLING_ONLY", out _),
                    "exact include precedence should not scan a case-colliding sibling");
            }
        }
        finally
        {
            TryDelete(root);
            TryDelete(externalRoot);
        }
    }

    private static void TestCrossFileProjectBoundaryResolvesProcedureDefinition()
    {
        var root = CreateProjectRoot("cross_file_project_boundary");
        try
        {
            var entryPath = Path.Combine(root, "main.prg");
            var libraryPath = Path.Combine(root, "orders.prg");
            File.WriteAllText(entryPath, "DO SaveOrder" + Environment.NewLine);
            File.WriteAllText(
                libraryPath,
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "LPARAMETERS tcCustomerId" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var resolved = FoxProIntelliSenseCatalog.TryResolveDefinition(entryPath, "SaveOrder", out var definition);
            Expect(resolved, "symbols defined in a sibling project file should resolve across the project boundary");
            if (resolved)
            {
                Expect(definition.FilePath == libraryPath, "cross-file project-boundary resolution should land on the defining project file");
                Expect(definition.LineNumber == 1, "cross-file project-boundary resolution should land on the defining line");
            }

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(entryPath, "SaveOrder");
            Expect(signatures.Count == 1, "cross-file project-boundary lookup should also surface the defining procedure signature");
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestCrossFileProjectBoundaryCompletions()
    {
        var root = CreateProjectRoot("cross_file_project_completions");
        try
        {
            var entryPath = Path.Combine(root, "main.prg");
            var libraryPath = Path.Combine(root, "orders.prg");
            File.WriteAllText(entryPath, "DO SaveOrder" + Environment.NewLine);
            File.WriteAllText(
                libraryPath,
                "PROCEDURE SaveOrder" + Environment.NewLine +
                "LPARAMETERS tcCustomerId" + Environment.NewLine +
                "ENDPROC" + Environment.NewLine);

            var completions = FoxProIntelliSenseCatalog.BuildEntries(entryPath, "DO ", "Sav");
            var saveOrder = completions.FirstOrDefault(entry => entry.DisplayText == "SaveOrder");
            Expect(saveOrder is not null,
                "completion results should include procedures defined in sibling project files");
            if (saveOrder is not null)
            {
                Expect(saveOrder.Kind == "symbol",
                    "cross-file procedure completions should preserve the invariant symbol kind");
                Expect(
                    saveOrder.Description == CopperfinLocalization.FromVisualStudioUiCulture().Text(
                        "LanguageService.IntelliSense.Project.ProcedureFunctionProgramSymbol"),
                    "cross-file procedure completions should use the existing project-symbol description");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static void TestLanguageServiceMetadataLocalizesThroughCatalogs()
    {
        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        var root = CreateProjectRoot("localized_language_service_metadata");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", "es-419");
            FoxProIntelliSenseCatalog.ClearCacheForTests();

            var sourcePath = Path.Combine(root, "main.prg");
            File.WriteAllText(
                sourcePath,
                "USE data/orderslive.dbf IN 0 SHARED" + Environment.NewLine +
                "nResult = SQLEXEC(nConn, \"SELECT id, company FROM customer\", \"sqlorders\")" + Environment.NewLine);

            var aliasDescription = FoxProIntelliSenseCatalog.DescribeToken(sourcePath, "orderslive");
            Expect(
                string.Equals(aliasDescription, "Alias de área de trabajo conocido detectado en el código fuente del proyecto.", StringComparison.Ordinal),
                "language-service quick info should localize project alias descriptions through the shared catalogs");

            var completions = FoxProIntelliSenseCatalog.BuildEntries(sourcePath, "SELECT ", "ord");
            var ordersLive = completions.FirstOrDefault(entry => entry.DisplayText == "orderslive");
            Expect(ordersLive is not null, "language-service completions should still include the inferred USE alias in localized mode");
            if (ordersLive is not null)
            {
                Expect(
                    string.Equals(ordersLive.Description, "Alias de área de trabajo conocido del código fuente del proyecto.", StringComparison.Ordinal),
                    "language-service completion metadata should localize context-specific alias descriptions");
                Expect(ordersLive.Kind == "alias", "language-service completion kinds must remain locale-invariant");
            }

            var signatures = FoxProIntelliSenseCatalog.GetSignatures(sourcePath, "MESSAGEBOX");
            Expect(signatures.Count == 1, "built-in language-service signature help should remain available in localized mode");
            if (signatures.Count == 1)
            {
                Expect(
                    string.Equals(signatures[0].Documentation, "Muestra un cuadro de diálogo modal y devuelve el botón presionado.", StringComparison.Ordinal),
                    "built-in language-service signature documentation should localize through the shared catalogs");
                Expect(
                    string.Equals(signatures[0].Parameters[0].Documentation, "Texto del mensaje que se mostrará.", StringComparison.Ordinal),
                    "built-in language-service parameter documentation should localize through the shared catalogs");
                Expect(
                    string.Equals(signatures[0].Content, "MESSAGEBOX(cMessage [, nDialogBoxType [, cTitleBarText]])", StringComparison.Ordinal),
                    "language-service signature content must remain locale-invariant");
            }
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousLocale);
            FoxProIntelliSenseCatalog.ClearCacheForTests();
            TryDelete(root);
        }
    }

    private static string CreateProjectRoot(string name)
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", name, Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        File.WriteAllText(Path.Combine(root, "testapp.pjx"), string.Empty);
        return root;
    }

    private static ProcessStartInfo CreateTestScriptStartInfo(
        string root,
        string name,
        IReadOnlyList<string> windowsBody,
        IReadOnlyList<string> unixBody)
    {
        if (OperatingSystem.IsWindows())
        {
            var scriptPath = Path.Combine(root, name + ".cmd");
            File.WriteAllLines(scriptPath, windowsBody);
            return new ProcessStartInfo
            {
                FileName = Environment.GetEnvironmentVariable("COMSPEC") ?? "cmd.exe",
                Arguments = $"/d /c \"\"{scriptPath}\"\"",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true
            };
        }

        var unixScriptPath = Path.Combine(root, name + ".sh");
        File.WriteAllLines(unixScriptPath, unixBody);
        File.SetUnixFileMode(
            unixScriptPath,
            UnixFileMode.UserRead |
            UnixFileMode.UserWrite |
            UnixFileMode.UserExecute);
        return new ProcessStartInfo
        {
            FileName = unixScriptPath,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true
        };
    }

    private static ProcessStartInfo CreateManagedTestChildStartInfo(params string[] arguments)
    {
        var processHostPath = Environment.ProcessPath ?? throw new InvalidOperationException("The managed test host path is unavailable.");
        var startInfo = new ProcessStartInfo
        {
            FileName = processHostPath,
            UseShellExecute = false,
            CreateNoWindow = true
        };
        if (string.Equals(Path.GetFileNameWithoutExtension(processHostPath), "dotnet", StringComparison.OrdinalIgnoreCase))
        {
            startInfo.ArgumentList.Add(typeof(Program).Assembly.Location);
        }
        foreach (var argument in arguments)
        {
            startInfo.ArgumentList.Add(argument);
        }
        return startInfo;
    }

    private static void WriteAutomaticHostCandidate(string path)
    {
        File.WriteAllText(path, OperatingSystem.IsWindows() ? string.Empty : "#!/bin/sh\nexit 0\n");
        if (!OperatingSystem.IsWindows())
        {
            File.SetUnixFileMode(
                path,
                UnixFileMode.UserRead |
                UnixFileMode.UserWrite |
                UnixFileMode.UserExecute);
        }
    }

    private static string CreateTestExecutableScriptPath(
        string root,
        string name,
        IReadOnlyList<string> unixBody)
    {
        if (OperatingSystem.IsWindows())
        {
            throw new PlatformNotSupportedException();
        }

        var scriptPath = Path.Combine(root, name + ".sh");
        File.WriteAllLines(scriptPath, unixBody);
        File.SetUnixFileMode(
            scriptPath,
            UnixFileMode.UserRead |
            UnixFileMode.UserWrite |
            UnixFileMode.UserExecute);
        return scriptPath;
    }

    private static void TryDelete(string root)
    {
        try
        {
            if (Directory.Exists(root))
            {
                Directory.Delete(root, recursive: true);
            }
        }
        catch
        {
        }
    }

    private static bool WaitForProcessExit(int processId, int timeoutMilliseconds)
    {
        var stopwatch = Stopwatch.StartNew();
        while (stopwatch.ElapsedMilliseconds < timeoutMilliseconds)
        {
            try
            {
                using var process = Process.GetProcessById(processId);
                if (process.HasExited)
                {
                    return true;
                }
            }
            catch (ArgumentException)
            {
                return true;
            }
            Thread.Sleep(50);
        }
        return false;
    }

    private static bool TryReadProcessId(string path, out int processId)
    {
        processId = 0;
        return File.Exists(path) && int.TryParse(File.ReadAllText(path).Trim(), out processId);
    }

    private static void TryTerminateProcess(int processId)
    {
        if (processId <= 0)
        {
            return;
        }
        try
        {
            using var process = Process.GetProcessById(processId);
            process.Kill(entireProcessTree: true);
            _ = process.WaitForExit(1000);
        }
        catch (ArgumentException)
        {
        }
        catch (InvalidOperationException)
        {
        }
    }

    private static void Expect(bool condition, string message)
    {
        if (!condition)
        {
            Console.Error.WriteLine($"FAIL: {message}");
            failures++;
        }
    }
}
