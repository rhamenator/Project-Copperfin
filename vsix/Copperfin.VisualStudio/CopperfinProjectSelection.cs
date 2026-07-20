// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.IO;
using EnvDTE;
using Microsoft.VisualStudio.Shell;

namespace Copperfin.VisualStudio;

internal static class CopperfinProjectSelection
{
    public static string? ResolveProjectPath(DTE? dte)
    {
        ThreadHelper.ThrowIfNotOnUIThread();

        var activeDocument = dte?.ActiveDocument;
        var activeDocumentPath = activeDocument?.FullName;
        var containingProjectPath = activeDocument?.ProjectItem?.ContainingProject?.FullName;
        var activeProjectPath = CopperfinProjectPathResolver.ResolveActiveDocumentProjectPath(
            activeDocumentPath,
            containingProjectPath);
        if (activeProjectPath is not null)
        {
            return activeProjectPath;
        }

        var selectedItems = dte?.SelectedItems;
        if (selectedItems is null || selectedItems.Count <= 0)
        {
            return null;
        }

        for (var index = 1; index <= selectedItems.Count; ++index)
        {
            var selectedItem = selectedItems.Item(index);
            if (selectedItem?.ProjectItem is ProjectItem projectItem)
            {
                try
                {
                    for (short fileIndex = 1; fileIndex <= projectItem.FileCount; ++fileIndex)
                    {
                        var candidate = projectItem.FileNames[fileIndex];
                        if (CopperfinProjectWorkflow.IsCopperfinProjectPath(candidate) && File.Exists(candidate))
                        {
                            return candidate;
                        }
                    }
                }
                catch (ArgumentException)
                {
                }
            }

            var projectPath = selectedItem?.Project?.FullName;
            if (CopperfinProjectWorkflow.IsCopperfinProjectPath(projectPath) && File.Exists(projectPath))
            {
                return projectPath;
            }
        }

        return null;
    }
}
