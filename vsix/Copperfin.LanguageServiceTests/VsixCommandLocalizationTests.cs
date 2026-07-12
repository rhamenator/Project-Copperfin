// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Xml.Linq;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestVsixCommandTableLocalizesCommandCaptions()
    {
        var repositoryRoot = FindRepositoryRoot();
        Expect(repositoryRoot is not null,
            "VSIX command localization test should locate the repository root");
        if (repositoryRoot is null)
        {
            return;
        }

        var commandTablePath = Path.Combine(
            repositoryRoot,
            "vsix",
            "Copperfin.VisualStudio",
            "Copperfin.vsct");
        var projectPath = Path.Combine(
            repositoryRoot,
            "vsix",
            "Copperfin.VisualStudio",
            "Copperfin.VisualStudio.csproj");
        var commandTable = XDocument.Load(commandTablePath);
        var project = XDocument.Load(projectPath);
        var commandNamespace = commandTable.Root?.Name.Namespace ?? XNamespace.None;
        var supportedLanguages = new[] { "en-US", "es-419", "pt-BR", "qps-ploc" };
        var commandSpecs = new[]
        {
            (Id: "OpenInCopperfinStudioCommand", Key: "VSIX.Command.OpenInStudio", EmbeddedKey: (string?)null,
                CanonicalName: ".Copperfin.OpenInStudio", SymbolValue: "0x0100"),
            (Id: "BuildCopperfinProjectCommand", Key: "VSIX.Command.BuildProject", EmbeddedKey: "AssetEditor.Project.BuildButton",
                CanonicalName: ".Copperfin.BuildProject", SymbolValue: "0x0200"),
            (Id: "RunCopperfinProjectCommand", Key: "VSIX.Command.RunProject", EmbeddedKey: "AssetEditor.Project.RunButton",
                CanonicalName: ".Copperfin.RunProject", SymbolValue: "0x0201"),
            (Id: "DebugCopperfinProjectCommand", Key: "VSIX.Command.DebugProject", EmbeddedKey: "AssetEditor.Project.DebugButton",
                CanonicalName: ".Copperfin.DebugProject", SymbolValue: "0x0202")
        };

        var buttons = commandTable
            .Descendants(commandNamespace + "Button")
            .Where(element => string.Equals(
                (string?)element.Attribute("guid"),
                "guidCopperfinCommandSet",
                StringComparison.Ordinal))
            .ToDictionary(
                element => (string?)element.Attribute("id") ?? string.Empty,
                StringComparer.Ordinal);
        var idSymbols = commandTable
            .Descendants(commandNamespace + "IDSymbol")
            .ToDictionary(
                element => (string?)element.Attribute("name") ?? string.Empty,
                element => (string?)element.Attribute("value") ?? string.Empty,
                StringComparer.Ordinal);

        foreach (var spec in commandSpecs)
        {
            Expect(buttons.TryGetValue(spec.Id, out var button),
                $"VSIX command table should retain command {spec.Id}");
            Expect(idSymbols.TryGetValue(spec.Id, out var symbolValue) && symbolValue == spec.SymbolValue,
                $"VSIX command table should preserve the invariant id for {spec.Id}");
            if (button is null)
            {
                continue;
            }

            var localizedStrings = button
                .Elements(commandNamespace + "Strings")
                .ToDictionary(
                    element => (string?)element.Attribute("Language") ?? string.Empty,
                    StringComparer.OrdinalIgnoreCase);
            Expect(localizedStrings.Count == supportedLanguages.Length &&
                   supportedLanguages.All(localizedStrings.ContainsKey),
                $"VSIX command {spec.Id} should define every supported command-table language");

            var englishText = new CopperfinLocalization("en-US").Text(spec.Key);
            foreach (var language in supportedLanguages)
            {
                if (!localizedStrings.TryGetValue(language, out var strings))
                {
                    continue;
                }

                var localization = new CopperfinLocalization(language);
                var buttonText = strings.Element(commandNamespace + "ButtonText")?.Value ?? string.Empty;
                var canonicalName = strings.Element(commandNamespace + "LocCanonicalName")?.Value ?? string.Empty;
                Expect(buttonText == localization.Text(spec.Key),
                    $"{language} VSIX command {spec.Id} should match the managed localization catalog");
                Expect(canonicalName == spec.CanonicalName,
                    $"{language} VSIX command {spec.Id} should preserve its invariant canonical name");
                if (!string.Equals(language, "en-US", StringComparison.OrdinalIgnoreCase))
                {
                    Expect(buttonText != englishText,
                        $"{language} VSIX command {spec.Id} should not expose raw English text");
                }
                if (spec.EmbeddedKey is not null)
                {
                    Expect(localization.Text(spec.Key) == localization.Text(spec.EmbeddedKey),
                        $"{language} VSIX command {spec.Id} should match the embedded editor command label");
                }

                var localePath = Path.Combine(repositoryRoot, "resources", "locales", language, "strings.json");
                using var localeDocument = JsonDocument.Parse(File.ReadAllText(localePath));
                var catalogValue = localeDocument.RootElement.TryGetProperty(spec.Key, out var catalogElement)
                    ? catalogElement.GetString() ?? string.Empty
                    : string.Empty;
                var expectedCatalogValue = string.Equals(language, "qps-ploc", StringComparison.OrdinalIgnoreCase)
                    ? englishText
                    : localization.Text(spec.Key);
                Expect(catalogValue == expectedCatalogValue,
                    $"{language} product catalog should define the VSIX command key {spec.Key}");
            }
        }

        var projectRoot = project.Root;
        var cultureOutputs = projectRoot?
            .Descendants("_CopperfinVsctCulture")
            .ToDictionary(
                element => (string?)element.Attribute("Include") ?? string.Empty,
                element => (
                    Language: element.Element("Language")?.Value ?? string.Empty,
                    OutputFile: element.Element("OutputFile")?.Value ?? string.Empty),
                StringComparer.OrdinalIgnoreCase) ??
            new Dictionary<string, (string Language, string OutputFile)>(StringComparer.OrdinalIgnoreCase);
        Expect(cultureOutputs.Count == supportedLanguages.Length &&
               cultureOutputs.TryGetValue("en-US", out var neutralOutput) &&
                   neutralOutput == ("en-US", "Copperfin.cto") &&
               cultureOutputs.TryGetValue("es", out var spanishOutput) &&
                   spanishOutput == ("es-419", "Copperfin.es.cto") &&
               cultureOutputs.TryGetValue("pt", out var portugueseOutput) &&
                   portugueseOutput == ("pt-BR", "Copperfin.pt.cto") &&
               cultureOutputs.TryGetValue("qps-ploc", out var pseudoOutput) &&
                   pseudoOutput == ("qps-ploc", "Copperfin.qps-ploc.cto"),
            "VSIX build should emit deterministic neutral and parent-culture fallback CTO resources");

        var compileTarget = projectRoot?
            .Elements("Target")
            .SingleOrDefault(element => string.Equals(
                (string?)element.Attribute("Name"),
                "CompileLocalizedCopperfinVsct",
                StringComparison.Ordinal));
        var compiler = compileTarget?.Element("VSCTCompiler");
        var generatedCultureOutputs = compileTarget?
            .Descendants("_GeneratedCTOFiles")
            .Select(element => (string?)element.Attribute("Include") ?? string.Empty)
            .ToHashSet(StringComparer.Ordinal) ?? new HashSet<string>(StringComparer.Ordinal);
        Expect(compileTarget is not null &&
               string.Equals((string?)compileTarget.Attribute("BeforeTargets"), "GenerateListOfCTO", StringComparison.Ordinal) &&
               string.Equals((string?)compileTarget.Attribute("DependsOnTargets"), "VSCTCompile", StringComparison.Ordinal) &&
               string.Equals((string?)compiler?.Attribute("Culture"), "%(_CopperfinVsctCulture.Language)", StringComparison.Ordinal) &&
               string.Equals((string?)compiler?.Attribute("OutputFile"), "%(_CopperfinVsctCulture.OutputFile)", StringComparison.Ordinal) &&
               generatedCultureOutputs.SetEquals(new[]
               {
                   "$(IntermediateOutputPath)Copperfin.es.cto",
                   "$(IntermediateOutputPath)Copperfin.pt.cto",
                   "$(IntermediateOutputPath)Copperfin.qps-ploc.cto"
               }),
            "VSIX build should compile the multilingual command table once per declared culture before CTO resource merging");

        var resourceAnchor = projectRoot?
            .Descendants("EmbeddedResource")
            .SingleOrDefault(element => string.Equals(
                (string?)element.Attribute("Update"),
                "CommandResources*.resx",
                StringComparison.Ordinal));
        Expect(resourceAnchor?.Element("MergeWithCTO")?.Value == "true" &&
               resourceAnchor.Element("ManifestResourceName")?.Value == "CopperfinCommandResources" &&
               new[] { "", ".es", ".pt", ".qps-ploc" }.All(suffix =>
                   File.Exists(Path.Combine(
                       repositoryRoot,
                       "vsix",
                       "Copperfin.VisualStudio",
                       $"CommandResources{suffix}.resx"))),
            "VSIX build should provide neutral and culture resource anchors for CTO satellite merging");
    }

    private static string? FindRepositoryRoot()
    {
        foreach (var startPath in new[] { Directory.GetCurrentDirectory(), AppContext.BaseDirectory })
        {
            var directory = new DirectoryInfo(startPath);
            while (directory is not null)
            {
                if (File.Exists(Path.Combine(directory.FullName, "resources", "locales", "en-US", "strings.json")) &&
                    File.Exists(Path.Combine(directory.FullName, "vsix", "Copperfin.VisualStudio", "Copperfin.vsct")))
                {
                    return directory.FullName;
                }
                directory = directory.Parent;
            }
        }

        return null;
    }
}
