// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

    internal static bool TryNormalize(
        string? path,
        out string normalizedPath,
        bool? isWindowsOverride = null)
    {
        normalizedPath = string.Empty;
        if (path is null || string.IsNullOrWhiteSpace(path))
        {
            return false;
        }

        try
        {
            normalizedPath = Normalize(path, isWindowsOverride);
            return true;
        }
        catch (ArgumentException)
        {
            return false;
        }
    }

    internal static bool IsWithinRoot(
        string root,
        string candidate,
        bool? isWindowsOverride = null)
    {
        var normalizedRoot = Normalize(root, isWindowsOverride).TrimEnd(
            Path.DirectorySeparatorChar,
            Path.AltDirectorySeparatorChar);
        var normalizedCandidate = Normalize(candidate, isWindowsOverride);
        var comparison = IsWindows(isWindowsOverride)
            ? StringComparison.OrdinalIgnoreCase
            : StringComparison.Ordinal;
        var rootWithSeparator = normalizedRoot + Path.DirectorySeparatorChar;
        return normalizedCandidate.Equals(normalizedRoot, comparison) ||
               normalizedCandidate.StartsWith(rootWithSeparator, comparison);
    }

    internal static bool LooksWindowsRooted(string path)
    {
        return path.StartsWith("\\\\", StringComparison.Ordinal) ||
               path.StartsWith("//", StringComparison.Ordinal) ||
               (path.Length >= 2 && char.IsLetter(path[0]) && path[1] == ':');
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
