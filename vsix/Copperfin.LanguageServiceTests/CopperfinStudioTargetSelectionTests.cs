// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.IO;
using System.Linq;
using System.Xml.Linq;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestStudioTargetSelectionPrefersSelectedItemsForItemCommands()
    {
        var registrations = CopperfinStudioCommandRegistrations.All;
        Expect(registrations.Count == 2,
            "Studio launcher should register exactly the Tools and Solution Explorer commands");
        Expect(registrations[0].CommandId == 0x0100 &&
               registrations[0].Preference == CopperfinStudioTargetPreference.ActiveDocument,
            "the Tools command should register active-document precedence at command ID 0x0100");
        Expect(registrations[1].CommandId == 0x0101 &&
               registrations[1].Preference == CopperfinStudioTargetPreference.SelectedItems,
            "the Solution Explorer command should register selected-item precedence at command ID 0x0101");

        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin_language_service_tests",
            "studio_target_selection",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        try
        {
            var activeDocument = Path.Combine(root, "active.prg");
            File.WriteAllText(activeDocument, "RETURN" + Environment.NewLine);

            foreach (var extension in new[] { ".pjx", ".prg", ".scx", ".vcx", ".frx", ".lbx", ".mnx" })
            {
                var selectedAsset = Path.Combine(root, "selected" + extension);
                File.WriteAllText(selectedAsset, extension);
                var resolved = CopperfinStudioTargetSelection.Resolve(
                    activeDocument,
                    new[] { SelectedTarget(selectedAsset) },
                    CopperfinStudioTargetPreference.SelectedItems);
                Expect(resolved == selectedAsset,
                    $"Solution Explorer item commands should prefer the selected {extension} asset over the active document");
            }

            var missingSelectedAsset = Path.Combine(root, "missing.scx");
            var selectedFallback = CopperfinStudioTargetSelection.Resolve(
                activeDocument,
                new[] { SelectedTarget(null, " ", missingSelectedAsset) },
                CopperfinStudioTargetPreference.SelectedItems);
            Expect(selectedFallback == activeDocument,
                "Solution Explorer item commands should fall back to the active document when no selected path exists");

            var laterSelectedAsset = Path.Combine(root, "later.frx");
            File.WriteAllText(laterSelectedAsset, "report");
            var laterSelection = CopperfinStudioTargetSelection.Resolve(
                activeDocument,
                new[]
                {
                    SelectedTarget(missingSelectedAsset),
                    SelectedTarget(laterSelectedAsset)
                },
                CopperfinStudioTargetPreference.SelectedItems);
            Expect(laterSelection == laterSelectedAsset,
                "Solution Explorer item commands should skip missing selected candidates before falling back");

            var activeFirst = CopperfinStudioTargetSelection.Resolve(
                activeDocument,
                new[] { SelectedTarget(laterSelectedAsset) },
                CopperfinStudioTargetPreference.ActiveDocument);
            Expect(activeFirst == activeDocument,
                "active-document-first launcher callers should preserve their existing precedence");

            var noTarget = CopperfinStudioTargetSelection.Resolve(
                Path.Combine(root, "missing-active.prg"),
                new[] { SelectedTarget(null, missingSelectedAsset) },
                CopperfinStudioTargetPreference.SelectedItems);
            Expect(noTarget is null,
                "Studio target selection should return no target when active and selected paths are missing");

            var selectedProject = Path.Combine(root, "selected-project.pjx");
            File.WriteAllText(selectedProject, "project");
            var projectFallback = CopperfinStudioTargetSelection.Resolve(
                activeDocument,
                new[]
                {
                    new CopperfinStudioSelectedTarget(
                        new string?[] { missingSelectedAsset },
                        selectedProject)
                },
                CopperfinStudioTargetPreference.SelectedItems);
            Expect(projectFallback == selectedProject,
                "selected project paths should follow missing project-item file candidates");

            var selectedProjectItem = Path.Combine(root, "selected-item.scx");
            File.WriteAllText(selectedProjectItem, "form");
            var projectItemFirst = CopperfinStudioTargetSelection.Resolve(
                activeDocument,
                new[]
                {
                    new CopperfinStudioSelectedTarget(
                        new string?[] { selectedProjectItem },
                        selectedProject)
                },
                CopperfinStudioTargetPreference.SelectedItems);
            Expect(projectItemFirst == selectedProjectItem,
                "selected project-item files should precede the containing project path");

            var repositoryRoot = FindRepositoryRoot();
            Expect(repositoryRoot is not null,
                "Studio target selection test should locate the repository root");
            if (repositoryRoot is not null)
            {
                var commandSource = File.ReadAllText(Path.Combine(
                    repositoryRoot,
                    "vsix",
                    "Copperfin.VisualStudio",
                    "OpenInCopperfinStudioCommand.cs"));
                Expect(commandSource.Contains(
                        "foreach (var registration in CopperfinStudioCommandRegistrations.All)",
                        StringComparison.Ordinal) &&
                       commandSource.Contains(
                           "AddCommand(commandService, registration.CommandId, registration.Preference)",
                           StringComparison.Ordinal),
                    "Open In Studio command registration should consume the verified command mapping");

                var commandTablePath = Path.Combine(
                    repositoryRoot,
                    "vsix",
                    "Copperfin.VisualStudio",
                    "Copperfin.vsct");
                var commandTable = XDocument.Load(commandTablePath);
                var commandNamespace = commandTable.Root?.Name.Namespace ?? XNamespace.None;
                var buttons = commandTable
                    .Descendants(commandNamespace + "Button")
                    .ToDictionary(
                        element => (string?)element.Attribute("id") ?? string.Empty,
                        StringComparer.Ordinal);
                Expect(buttons.TryGetValue("OpenInCopperfinStudioCommand", out var toolsButton) &&
                       string.Equals(
                           (string?)toolsButton.Element(commandNamespace + "Parent")?.Attribute("id"),
                           "CopperfinGroup",
                           StringComparison.Ordinal),
                    "Tools-menu Open In Studio should retain its original command and group");
                Expect(buttons.TryGetValue("OpenSelectedInCopperfinStudioCommand", out var itemButton) &&
                       string.Equals(
                           (string?)itemButton.Element(commandNamespace + "Parent")?.Attribute("id"),
                           "CopperfinItemGroup",
                           StringComparison.Ordinal),
                    "Solution Explorer should use a distinct selected-item command placement");
                var itemGroup = commandTable
                    .Descendants(commandNamespace + "Group")
                    .SingleOrDefault(element => string.Equals(
                        (string?)element.Attribute("id"),
                        "CopperfinItemGroup",
                        StringComparison.Ordinal));
                Expect(itemGroup is not null &&
                       string.Equals(
                           (string?)itemGroup.Attribute("priority"),
                           "0x0100",
                           StringComparison.OrdinalIgnoreCase),
                    "the Copperfin item group should retain the original context-menu position");
                foreach (var commandId in new[]
                         {
                             "BuildCopperfinProjectCommand",
                             "RunCopperfinProjectCommand",
                             "DebugCopperfinProjectCommand"
                         })
                {
                    Expect(commandTable
                            .Descendants(commandNamespace + "CommandPlacement")
                            .Any(element => string.Equals(
                                                (string?)element.Attribute("id"),
                                                commandId,
                                                StringComparison.Ordinal) &&
                                            string.Equals(
                                                (string?)element
                                                    .Element(commandNamespace + "Parent")
                                                    ?.Attribute("id"),
                                                "CopperfinItemGroup",
                                                StringComparison.Ordinal)),
                        $"{commandId} should remain grouped with the item-node Open command");
                }
                Expect(!commandTable
                        .Descendants(commandNamespace + "CommandPlacement")
                        .Any(element => string.Equals(
                            (string?)element.Attribute("id"),
                            "OpenInCopperfinStudioCommand",
                            StringComparison.Ordinal)),
                    "the Tools command should not also be placed in the item-node menu");
            }
        }
        finally
        {
            TryDelete(root);
        }
    }

    private static CopperfinStudioSelectedTarget SelectedTarget(params string?[] projectItemPaths)
    {
        return new CopperfinStudioSelectedTarget(projectItemPaths, projectPath: null);
    }
}
