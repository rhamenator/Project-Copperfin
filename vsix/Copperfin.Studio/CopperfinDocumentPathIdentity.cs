// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.IO;
using System.Security;

namespace Copperfin.VisualStudio;

internal static class CopperfinDocumentPathIdentity
{
    internal static StringComparer CreateComparer(bool? isWindowsOverride = null)
    {
        return IsWindows(isWindowsOverride)
            ? StringComparer.OrdinalIgnoreCase
            : StringComparer.Ordinal;
    }

    internal static string Normalize(string path, bool? isWindowsOverride = null)
    {
        var fullPath = Path.GetFullPath(path);
        return IsWindows(isWindowsOverride) || !File.Exists(fullPath)
            ? fullPath
            : CanonicalizeExistingPathSpelling(fullPath);
    }

    private static bool IsWindows(bool? isWindowsOverride)
    {
        return isWindowsOverride ?? Path.DirectorySeparatorChar == '\\';
    }

    private static string CanonicalizeExistingPathSpelling(string fullPath)
    {
        var root = Path.GetPathRoot(fullPath);
        if (string.IsNullOrEmpty(root))
        {
            return fullPath;
        }

        var currentPath = root;
        var relativePath = fullPath.Substring(root.Length);
        var segments = relativePath.Split(
            new[] { Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar },
            StringSplitOptions.RemoveEmptyEntries);

        foreach (var segment in segments)
        {
            string? exactMatch = null;
            string? insensitiveMatch = null;
            var insensitiveMatchCount = 0;
            try
            {
                foreach (var entry in Directory.EnumerateFileSystemEntries(currentPath))
                {
                    var entryName = Path.GetFileName(entry);
                    if (string.Equals(entryName, segment, StringComparison.Ordinal))
                    {
                        exactMatch = entry;
                        break;
                    }

                    if (string.Equals(entryName, segment, StringComparison.OrdinalIgnoreCase))
                    {
                        insensitiveMatch = entry;
                        insensitiveMatchCount++;
                    }
                }
            }
            catch (IOException)
            {
                return fullPath;
            }
            catch (UnauthorizedAccessException)
            {
                return fullPath;
            }
            catch (SecurityException)
            {
                return fullPath;
            }

            currentPath = exactMatch ??
                (insensitiveMatchCount == 1 ? insensitiveMatch : null) ??
                Path.Combine(currentPath, segment);
        }

        return currentPath;
    }
}
