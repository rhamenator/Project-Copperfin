// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace Copperfin.VisualStudio;

internal enum CopperfinFileManagerPlatform
{
    Windows,
    MacOS,
    Linux
}

internal static class CopperfinFileManager
{
    internal static ProcessStartInfo CreateRevealStartInfo(
        string path,
        CopperfinFileManagerPlatform platform)
    {
        if (platform == CopperfinFileManagerPlatform.Windows)
        {
            return new ProcessStartInfo
            {
                FileName = "explorer.exe",
                Arguments = $"/select,\"{path}\"",
                UseShellExecute = true
            };
        }

        if (platform == CopperfinFileManagerPlatform.MacOS)
        {
            return new ProcessStartInfo
            {
                FileName = "open",
                Arguments = $"--reveal {QuoteArgument(path)}",
                UseShellExecute = false
            };
        }

        var directory = GetPosixContainingDirectory(path);
        return new ProcessStartInfo
        {
            FileName = "xdg-open",
            Arguments = QuoteArgument(string.IsNullOrWhiteSpace(directory) ? path : directory),
            UseShellExecute = false
        };
    }

    internal static bool TryReveal(string path)
    {
        if (!File.Exists(path))
        {
            return false;
        }

        try
        {
            using var process = Process.Start(CreateRevealStartInfo(path, DetectPlatform()));
            return process is not null;
        }
        catch (InvalidOperationException)
        {
            return false;
        }
        catch (System.ComponentModel.Win32Exception)
        {
            return false;
        }
    }

    private static CopperfinFileManagerPlatform DetectPlatform()
    {
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            return CopperfinFileManagerPlatform.Windows;
        }

        return RuntimeInformation.IsOSPlatform(OSPlatform.OSX)
            ? CopperfinFileManagerPlatform.MacOS
            : CopperfinFileManagerPlatform.Linux;
    }

    private static string QuoteArgument(string value)
    {
        return "\"" + value.Replace("\\", "\\\\").Replace("\"", "\\\"") + "\"";
    }

    private static string GetPosixContainingDirectory(string path)
    {
        var separator = path.LastIndexOf('/');
        if (separator < 0)
        {
            return path;
        }

        return separator == 0 ? "/" : path.Substring(0, separator);
    }
}
