// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
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
        return CopperfinProjectPathResolver.ResolveProjectWorkflowPath(
            activeDocumentPath,
            containingProjectPath,
            EnumerateSelectedProjectPaths(dte));
    }

    private static IEnumerable<string?> EnumerateSelectedProjectPaths(DTE? dte)
    {
        ThreadHelper.ThrowIfNotOnUIThread();

        var selectedItems = dte?.SelectedItems;
        if (selectedItems is null || selectedItems.Count <= 0)
        {
            yield break;
        }

        for (var index = 1; index <= selectedItems.Count; ++index)
        {
            var selectedItem = selectedItems.Item(index);
            if (selectedItem?.ProjectItem is ProjectItem projectItem)
            {
                string? selectedProjectPath = null;
                string? containingProjectPath = null;
                try
                {
                    for (short fileIndex = 1; fileIndex <= projectItem.FileCount; ++fileIndex)
                    {
                        var candidate = projectItem.FileNames[fileIndex];
                        if (CopperfinProjectWorkflow.IsCopperfinProjectPath(candidate) && File.Exists(candidate))
                        {
                            selectedProjectPath = candidate;
                            break;
                        }
                    }

                    containingProjectPath = projectItem.ContainingProject?.FullName;
                }
                catch (ArgumentException)
                {
                }

                if (selectedProjectPath is not null)
                {
                    yield return selectedProjectPath;
                }

                yield return containingProjectPath;
            }

            yield return selectedItem?.Project?.FullName;
        }
    }
}
