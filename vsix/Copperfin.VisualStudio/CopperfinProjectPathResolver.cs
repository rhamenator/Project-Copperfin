// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System.Collections.Generic;
using System.IO;

namespace Copperfin.VisualStudio;

internal static class CopperfinProjectPathResolver
{
    public static string? ResolveActiveDocumentProjectPath(
        string? activeDocumentPath,
        string? containingProjectPath)
    {
        if (IsExistingProjectPath(activeDocumentPath))
        {
            return activeDocumentPath;
        }

        return IsExistingProjectPath(containingProjectPath)
            ? containingProjectPath
            : null;
    }

    public static string? ResolveProjectWorkflowPath(
        string? activeDocumentPath,
        string? containingProjectPath,
        IEnumerable<string?> selectedProjectPaths)
    {
        foreach (var selectedProjectPath in selectedProjectPaths)
        {
            if (IsExistingProjectPath(selectedProjectPath))
            {
                return selectedProjectPath;
            }
        }

        return ResolveActiveDocumentProjectPath(activeDocumentPath, containingProjectPath);
    }

    private static bool IsExistingProjectPath(string? path)
    {
        return CopperfinProjectWorkflow.IsCopperfinProjectPath(path) &&
               File.Exists(path);
    }
}
