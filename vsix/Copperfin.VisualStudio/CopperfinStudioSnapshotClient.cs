using System;
using System.Diagnostics;
using System.IO;
using System.Web.Script.Serialization;

namespace Copperfin.VisualStudio;

internal static class CopperfinStudioSnapshotClient
{
    private static readonly CopperfinLocalization Localization = CopperfinLocalization.FromEnvironment();

    private static CopperfinStudioSnapshotResult RunSnapshotCommand(string studioHostPath, string arguments)
    {
        var startInfo = new ProcessStartInfo
        {
            FileName = studioHostPath,
            Arguments = arguments,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true
        };

        using var process = new Process { StartInfo = startInfo };
        if (!process.Start())
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = Localization.Text("AssetEditor.Dialog.StudioHostCouldNotStart")
            };
        }

        var stdout = process.StandardOutput.ReadToEnd();
        var stderr = process.StandardError.ReadToEnd();
        process.WaitForExit(15000);

        if (!process.HasExited)
        {
            try
            {
                process.Kill();
            }
            catch (InvalidOperationException)
            {
            }

                return new CopperfinStudioSnapshotResult
                {
                    Success = false,
                    Error = Localization.Text("AssetEditor.Dialog.StudioHostTimedOut")
                };
            }

        if (process.ExitCode != 0)
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = string.IsNullOrWhiteSpace(stderr) ? stdout.Trim() : stderr.Trim()
            };
        }

        try
        {
            var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 * 8 };
            var envelope = serializer.Deserialize<CopperfinStudioSnapshotEnvelope>(stdout);
            if (envelope is null || envelope.Document is null)
            {
                return new CopperfinStudioSnapshotResult
                {
                    Success = false,
                    Error = Localization.Text("AssetEditor.Dialog.StudioSnapshotEmpty")
                };
            }

            return new CopperfinStudioSnapshotResult
            {
                Success = true,
                Document = envelope.Document
            };
        }
        catch (InvalidOperationException ex)
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = Localization.Format("AssetEditor.Dialog.StudioSnapshotParseFailed", ex.Message)
            };
        }
    }

    public static CopperfinStudioSnapshotResult TryLoad(string assetPath)
    {
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = Localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(studioHostPath!, CopperfinStudioHostBridge.BuildArguments(assetPath, readOnly: true) + " --json");
    }

    public static CopperfinStudioSnapshotResult TryUpdateProperty(
        string assetPath,
        int recordIndex,
        string propertyName,
        string propertyValue)
    {
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = Localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildPropertyUpdateArguments(assetPath, recordIndex, propertyName, propertyValue));
    }

    public static CopperfinStudioSnapshotResult TryUndoCommand(string assetPath)
    {
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = Localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(studioHostPath!, CopperfinStudioHostBridge.BuildUndoArguments(assetPath));
    }
}
