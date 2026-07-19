// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
        if (Path.IsPathRooted(relativePath) || LooksWindowsRooted(relativePath))
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

        var normalizedRoot = CopperfinDocumentPathIdentity.Normalize(projectDirectory);
        var comparison = Path.DirectorySeparatorChar == '\\'
            ? StringComparison.OrdinalIgnoreCase
            : StringComparison.Ordinal;
        var rootWithSeparator = normalizedRoot.TrimEnd(
            Path.DirectorySeparatorChar,
            Path.AltDirectorySeparatorChar) + Path.DirectorySeparatorChar;
        if (!candidate.Equals(normalizedRoot, comparison) &&
            !candidate.StartsWith(rootWithSeparator, comparison))
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

    private static bool LooksWindowsRooted(string path)
    {
        return path.StartsWith("\\\\", StringComparison.Ordinal) ||
               path.StartsWith("//", StringComparison.Ordinal) ||
               (path.Length >= 2 && char.IsLetter(path[0]) && path[1] == ':');
    }
}
