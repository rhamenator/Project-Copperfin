// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Collections.Generic;
using System.IO;

namespace Copperfin.VisualStudio;

internal enum CopperfinStudioTargetPreference
{
    ActiveDocument,
    SelectedItems
}

internal sealed class CopperfinStudioCommandRegistration
{
    public CopperfinStudioCommandRegistration(
        int commandId,
        CopperfinStudioTargetPreference preference)
    {
        CommandId = commandId;
        Preference = preference;
    }

    public int CommandId { get; }
    public CopperfinStudioTargetPreference Preference { get; }
}

internal static class CopperfinStudioCommandRegistrations
{
    public const int ToolsCommandId = 0x0100;
    public const int SelectedItemCommandId = 0x0101;

    public static IReadOnlyList<CopperfinStudioCommandRegistration> All { get; } =
        new[]
        {
            new CopperfinStudioCommandRegistration(
                ToolsCommandId,
                CopperfinStudioTargetPreference.ActiveDocument),
            new CopperfinStudioCommandRegistration(
                SelectedItemCommandId,
                CopperfinStudioTargetPreference.SelectedItems)
        };
}

internal sealed class CopperfinStudioSelectedTarget
{
    public CopperfinStudioSelectedTarget(
        IReadOnlyList<string?> projectItemPaths,
        string? projectPath)
    {
        ProjectItemPaths = projectItemPaths;
        ProjectPath = projectPath;
    }

    public IReadOnlyList<string?> ProjectItemPaths { get; }
    public string? ProjectPath { get; }
}

internal static class CopperfinStudioTargetSelection
{
    private static readonly HashSet<string> SupportedExtensions = new(
        new[] { ".pjx", ".prg", ".scx", ".vcx", ".frx", ".lbx", ".mnx" },
        StringComparer.OrdinalIgnoreCase);

    public static bool IsSupportedTargetPath(string? candidate)
    {
        return !string.IsNullOrWhiteSpace(candidate) &&
               SupportedExtensions.Contains(Path.GetExtension(candidate));
    }

    public static string? Resolve(
        string? activeDocumentPath,
        IEnumerable<CopperfinStudioSelectedTarget> selectedTargets,
        CopperfinStudioTargetPreference preference)
    {
        if (preference == CopperfinStudioTargetPreference.ActiveDocument)
        {
            var activeDocument = ExistingPathOrNull(activeDocumentPath);
            if (activeDocument is not null)
            {
                return activeDocument;
            }
        }

        foreach (var selectedTarget in selectedTargets)
        {
            foreach (var projectItemPath in selectedTarget.ProjectItemPaths)
            {
                var projectItem = ExistingPathOrNull(projectItemPath);
                if (projectItem is not null)
                {
                    return projectItem;
                }
            }

            var project = ExistingPathOrNull(selectedTarget.ProjectPath);
            if (project is not null)
            {
                return project;
            }
        }

        return ExistingPathOrNull(activeDocumentPath);
    }

    private static string? ExistingPathOrNull(string? candidate)
    {
        return IsSupportedTargetPath(candidate) && File.Exists(candidate)
            ? candidate
            : null;
    }
}
