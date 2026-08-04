// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
