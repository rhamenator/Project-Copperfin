using System;
using System.IO;
using DiagnosticsProcess = System.Diagnostics.Process;
using DiagnosticsStartInfo = System.Diagnostics.ProcessStartInfo;

namespace Copperfin.VisualStudio;

internal static class CopperfinStudioHostBridge
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
        return readOnly
            ? $"--from-vs --read-only --path {Quote(documentPath)}"
            : $"--from-vs --path {Quote(documentPath)}";
    }

    public static string BuildPropertyUpdateArguments(string documentPath, int recordIndex, string propertyName, string propertyValue)
    {
        return $"--from-vs --json --set-property --record {recordIndex} --property-name {Quote(propertyName)} --property-value {Quote(propertyValue)} --path {Quote(documentPath)}";
    }

    public static string BuildUndoArguments(string documentPath)
    {
        return $"--from-vs --json --undo-mode command --path {Quote(documentPath)}";
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

    public static string DescribeAssetKind(string path, CopperfinLocalization? localization = null)
    {
        localization ??= new CopperfinLocalization();
        var key = Path.GetExtension(path).ToLowerInvariant() switch
        {
            ".pjx" => "Studio.AssetKind.Project",
            ".scx" => "Studio.AssetKind.Form",
            ".vcx" => "Studio.AssetKind.ClassLibrary",
            ".frx" => "Studio.AssetKind.Report",
            ".lbx" => "Studio.AssetKind.Label",
            ".mnx" => "Studio.AssetKind.Menu",
            _ => "Studio.AssetKind.Generic"
        };

        return localization.Text(key);
    }

    private static string Quote(string value)
    {
        return "\"" + value.Replace("\"", "\"\"") + "\"";
    }
}
