using System;
using System.IO;
using EnvDTE;
using Microsoft.VisualStudio.Shell;
using DiagnosticsProcess = System.Diagnostics.Process;
using DiagnosticsStartInfo = System.Diagnostics.ProcessStartInfo;

namespace Copperfin.VisualStudio;

internal static class CopperfinStudioLauncher
{
    private const string RepoDevelopmentPath = @"E:\Project-Copperfin\build\Release\copperfin_studio_host.exe";

    public static string? ResolveStudioHostPath()
    {
        var configured = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        if (!string.IsNullOrWhiteSpace(configured) && File.Exists(configured))
        {
            return configured;
        }

        if (File.Exists(RepoDevelopmentPath))
        {
            return RepoDevelopmentPath;
        }

        return null;
    }

    public static string BuildArguments(string documentPath, bool readOnly = false)
    {
        var escaped = documentPath.Replace("\"", "\"\"");
        return readOnly
            ? $"--from-vs --read-only --path \"{escaped}\""
            : $"--from-vs --path \"{escaped}\"";
    }

    public static bool Launch(string studioHostPath, string documentPath, bool readOnly = false)
    {
        var startInfo = new DiagnosticsStartInfo
        {
            FileName = studioHostPath,
            Arguments = BuildArguments(documentPath, readOnly),
            UseShellExecute = false
        };

        return DiagnosticsProcess.Start(startInfo) is not null;
    }

    public static string? ResolveTargetPath(DTE dte)
    {
        ThreadHelper.ThrowIfNotOnUIThread();

        var activeDocumentPath = dte.ActiveDocument?.FullName;
        if (!string.IsNullOrWhiteSpace(activeDocumentPath) && File.Exists(activeDocumentPath))
        {
            return activeDocumentPath;
        }

        var selectedItems = dte.SelectedItems;
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
                        if (!string.IsNullOrWhiteSpace(candidate) && File.Exists(candidate))
                        {
                            return candidate;
                        }
                    }
                }
                catch (ArgumentException)
                {
                    // Some project systems expose non-file nodes. Skip them.
                }
            }

            var projectPath = selectedItem?.Project?.FullName;
            if (!string.IsNullOrWhiteSpace(projectPath) && File.Exists(projectPath))
            {
                return projectPath;
            }
        }

        return null;
    }

    public static string DescribeAssetKind(string path)
    {
        switch (Path.GetExtension(path).ToLowerInvariant())
        {
            case ".pjx":
                return "Visual project";
            case ".scx":
                return "Visual form";
            case ".vcx":
                return "Visual class library";
            case ".frx":
                return "Visual report";
            case ".lbx":
                return "Visual label";
            case ".mnx":
                return "Visual menu";
            default:
                return "Copperfin asset";
        }
    }
}
