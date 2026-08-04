// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Collections.Generic;
using System.IO;

namespace Copperfin.VisualStudio;

internal static class CopperfinProjectEntryActivation
{
    private static readonly HashSet<string> SupportedExtensions = new(
        new[] { ".pjx", ".prg", ".scx", ".vcx", ".frx", ".lbx", ".mnx" },
        StringComparer.OrdinalIgnoreCase);

    internal static bool TryResolve(
        string projectPath,
        CopperfinStudioProjectEntry entry,
        out string resolvedPath)
    {
        resolvedPath = string.Empty;
        if (string.IsNullOrWhiteSpace(projectPath) ||
            string.IsNullOrWhiteSpace(entry.RelativePath))
        {
            return false;
        }

        var projectFullPath = CopperfinDocumentPathIdentity.Normalize(projectPath);
        var projectDirectory = Path.GetDirectoryName(projectFullPath);
        if (string.IsNullOrWhiteSpace(projectDirectory))
        {
            return false;
        }

        var relativePath = entry.RelativePath.Trim()
            .Replace('\\', Path.DirectorySeparatorChar)
            .Replace('/', Path.DirectorySeparatorChar);
        if (Path.IsPathRooted(relativePath) ||
            CopperfinDocumentPathIdentity.LooksWindowsRooted(relativePath))
        {
            return false;
        }

        string candidate;
        try
        {
            candidate = Path.GetFullPath(Path.Combine(projectDirectory, relativePath));
        }
        catch (ArgumentException)
        {
            return false;
        }
        catch (NotSupportedException)
        {
            return false;
        }

        if (!CopperfinDocumentPathIdentity.IsWithinRoot(projectDirectory, candidate))
        {
            return false;
        }

        if (!SupportedExtensions.Contains(Path.GetExtension(candidate)) ||
            !File.Exists(candidate))
        {
            return false;
        }

        resolvedPath = CopperfinDocumentPathIdentity.Normalize(candidate);
        return true;
    }

}
