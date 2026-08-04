// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.IO;
using EnvDTE;
using Microsoft.VisualStudio.Shell;

namespace Copperfin.VisualStudio;

internal static class CopperfinStudioLauncher
{
    public static string? ResolveTargetPath(
        DTE dte,
        CopperfinStudioTargetPreference preference = CopperfinStudioTargetPreference.ActiveDocument)
    {
        ThreadHelper.ThrowIfNotOnUIThread();

        var activeDocumentPath = dte.ActiveDocument?.FullName;
        if (preference == CopperfinStudioTargetPreference.ActiveDocument &&
            CopperfinStudioTargetSelection.IsSupportedTargetPath(activeDocumentPath) &&
            File.Exists(activeDocumentPath))
        {
            return activeDocumentPath;
        }

        var selectedTargets = new List<CopperfinStudioSelectedTarget>();
        var selectedItems = dte.SelectedItems;
        if (selectedItems is not null && selectedItems.Count > 0)
        {
            for (var index = 1; index <= selectedItems.Count; ++index)
            {
                var selectedItem = selectedItems.Item(index);
                var projectItemPaths = new List<string?>();
                if (selectedItem?.ProjectItem is ProjectItem projectItem)
                {
                    try
                    {
                        for (short fileIndex = 1; fileIndex <= projectItem.FileCount; ++fileIndex)
                        {
                            projectItemPaths.Add(projectItem.FileNames[fileIndex]);
                        }
                    }
                    catch (ArgumentException)
                    {
                        // Some project systems expose non-file nodes. Skip them.
                    }
                }

                selectedTargets.Add(new CopperfinStudioSelectedTarget(
                    projectItemPaths,
                    selectedItem?.Project?.FullName));
            }
        }

        return CopperfinStudioTargetSelection.Resolve(activeDocumentPath, selectedTargets, preference);
    }
}
