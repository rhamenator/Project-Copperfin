using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Copperfin.VisualStudio;

internal static class Program
{
    private static int failures;

    private static int Main()
    {
        TestLocalizationCatalogNormalizesSpanishAndPortugueseLocales();
        TestLocalizationCatalogSupportsPseudoLocale();
        TestLocalizationCatalogFallsBackToEnglish();
        TestLocalizationCatalogKeepsSupportedLocaleKeysAligned();
        TestLocalizationCatalogFormatsWithInvariantCulture();
        TestLocalizationCatalogLocalizesStudioAssetKinds();
        TestStudioHostProcessStartInfoKeepsExecutableLaunchArguments();
        TestStudioHostProcessStartInfoWrapsWindowsBatchHosts();
        TestDottedClassMemberResolvesToLongestProjectSymbolPrefix();
        TestDottedMemberFallsBackToTrailingProcedureName();
        TestQuickInfoUsesResolvedProjectSymbolDescriptionForDottedMemberAccess();
        TestProjectProcedureSignatureHelpUsesLparameters();
        TestProjectProcedureSignatureHelpFallsBackFromDottedInvocation();
        TestProjectInsightsCollectDirectAndDottedProcedureCallReferences();
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
        TestIncludedHeaderOutsideProjectRootFeedsDefineResolution();
        TestCrossFileProjectBoundaryResolvesProcedureDefinition();

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
        foreach (var locale in CopperfinLocalization.SupportedLocales)
        {
            var keys = new HashSet<string>(CopperfinLocalization.CatalogKeys(locale));
            Expect(keys.SetEquals(englishKeys), $"{locale} catalog should expose the same key set as English");

            var localization = new CopperfinLocalization(locale);
            foreach (var key in englishKeys)
            {
                Expect(!string.IsNullOrWhiteSpace(localization.Text(key)), $"{locale} catalog value for {key} should not be blank");
            }
        }
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
            Expect(startInfo.Arguments == "/d /c \"\"C:\\temp\\fake studio host.cmd\" --from-vs --json --set-property --path \"C:\\Samples\\invoice.frx\"\"",
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
                string.Equals(description, "Project class symbol deriving from custom.", StringComparison.Ordinal),
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
                string.Equals(description, "Known cursor alias discovered in project source.", StringComparison.Ordinal),
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
                string.Equals(useDescription, "Known work-area alias discovered in project source.", StringComparison.Ordinal),
                "implicit USE aliases should reuse the work-area alias description");

            var sqlDescription = FoxProIntelliSenseCatalog.DescribeToken(sourcePath, "sqlorders");
            Expect(
                string.Equals(sqlDescription, "Known cursor alias discovered in project source.", StringComparison.Ordinal),
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
                Expect(completions[0].Description == "Known work-area alias from project source.",
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
                Expect(completions[0].Description == "Project method member from app.customer.editor.",
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
            Expect(string.Equals(description, "Project preprocessor symbol.", StringComparison.Ordinal),
                "included external header defines should participate in token description lookup");
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

    private static string CreateProjectRoot(string name)
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_language_service_tests", name, Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        File.WriteAllText(Path.Combine(root, "testapp.pjx"), string.Empty);
        return root;
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

    private static void Expect(bool condition, string message)
    {
        if (!condition)
        {
            Console.Error.WriteLine($"FAIL: {message}");
            failures++;
        }
    }
}
