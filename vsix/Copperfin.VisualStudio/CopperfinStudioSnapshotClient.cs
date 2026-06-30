using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Web.Script.Serialization;

namespace Copperfin.VisualStudio;

internal static class CopperfinStudioSnapshotClient
{
    private sealed class StudioHostCommandResult
    {
        public bool Success { get; set; }
        public string Stdout { get; set; } = string.Empty;
        public string Error { get; set; } = string.Empty;
    }

    private sealed class CopperfinStudioVisualObjectBatchEnvelope
    {
        public string Status { get; set; } = string.Empty;
        public string Error { get; set; } = string.Empty;
        public CopperfinStudioVisualObjectBatchResult? VisualObjectUpdateBatch { get; set; }
    }

    private sealed class CopperfinStudioVisualObjectBatchResult
    {
        public bool Ok { get; set; }
        public string Error { get; set; } = string.Empty;
        public bool UndoAvailable { get; set; }
        public string UndoLabel { get; set; } = string.Empty;
    }

    private static readonly CopperfinLocalization Localization = CopperfinLocalization.FromEnvironment();

    private static bool HasDocumentContent(CopperfinStudioSnapshotDocument? document)
    {
        return document is not null &&
               (!string.IsNullOrWhiteSpace(document.Path) ||
                !string.IsNullOrWhiteSpace(document.AssetFamily) ||
                document.Objects.Count > 0 ||
                document.FieldCount > 0 ||
                document.ReportLayout is not null ||
                document.ProjectWorkspace is not null);
    }

    private static StudioHostCommandResult RunCommand(string studioHostPath, string arguments)
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
            return new StudioHostCommandResult
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

            return new StudioHostCommandResult
            {
                Success = false,
                Error = Localization.Text("AssetEditor.Dialog.StudioHostTimedOut")
            };
        }

        if (process.ExitCode != 0)
        {
            return new StudioHostCommandResult
            {
                Success = false,
                Error = string.IsNullOrWhiteSpace(stderr) ? stdout.Trim() : stderr.Trim()
            };
        }

        return new StudioHostCommandResult
        {
            Success = true,
            Stdout = stdout
        };
    }

    private static CopperfinStudioSnapshotResult RunSnapshotCommand(string studioHostPath, string arguments)
    {
        var commandResult = RunCommand(studioHostPath, arguments);
        if (!commandResult.Success)
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = commandResult.Error
            };
        }

        try
        {
            var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 * 8 };
            var envelope = serializer.Deserialize<CopperfinStudioSnapshotEnvelope>(commandResult.Stdout);
            var document = envelope?.Document;
            if (!HasDocumentContent(document))
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
                Document = document
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

    private static CopperfinStudioSnapshotResult RunBatchPropertyUpdateAndReload(
        string studioHostPath,
        string assetPath,
        int recordIndex,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges)
    {
        var commandResult = RunCommand(
            studioHostPath,
            CopperfinStudioHostBridge.BuildPropertyBatchUpdateArguments(assetPath, recordIndex, propertyChanges));
        if (!commandResult.Success)
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = commandResult.Error
            };
        }

        try
        {
            var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 * 8 };
            var snapshotEnvelope = serializer.Deserialize<CopperfinStudioSnapshotEnvelope>(commandResult.Stdout);
            var snapshotDocument = snapshotEnvelope?.Document;
            if (HasDocumentContent(snapshotDocument))
            {
                return new CopperfinStudioSnapshotResult
                {
                    Success = true,
                    Document = snapshotDocument
                };
            }

            var envelope = serializer.Deserialize<CopperfinStudioVisualObjectBatchEnvelope>(commandResult.Stdout);
            if (envelope?.VisualObjectUpdateBatch is null)
            {
                return new CopperfinStudioSnapshotResult
                {
                    Success = false,
                    Error = Localization.Text("AssetEditor.Dialog.StudioSnapshotEmpty")
                };
            }

            if (!envelope.VisualObjectUpdateBatch.Ok)
            {
                return new CopperfinStudioSnapshotResult
                {
                    Success = false,
                    Error = !string.IsNullOrWhiteSpace(envelope.VisualObjectUpdateBatch.Error)
                        ? envelope.VisualObjectUpdateBatch.Error
                        : envelope.Error
                };
            }

            return RunSnapshotCommand(
                studioHostPath,
                CopperfinStudioHostBridge.BuildArguments(assetPath, readOnly: true) + " --json");
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

    public static CopperfinStudioSnapshotResult TryUpdateProperties(
        string assetPath,
        int recordIndex,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges)
    {
        if (propertyChanges.Count == 0)
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = Localization.Text("AssetEditor.Dialog.StudioSnapshotEmpty")
            };
        }

        if (propertyChanges.Count == 1)
        {
            return TryUpdateProperty(assetPath, recordIndex, propertyChanges[0].Key, propertyChanges[0].Value);
        }

        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = Localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunBatchPropertyUpdateAndReload(
            studioHostPath!,
            assetPath,
            recordIndex,
            propertyChanges);
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

    public static CopperfinStudioSnapshotResult TryDeleteObject(string assetPath, int recordIndex, string? uniqueId)
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
            CopperfinStudioHostBridge.BuildDeleteObjectArguments(assetPath, recordIndex, uniqueId));
    }

    public static CopperfinStudioSnapshotResult TryRestoreObject(string assetPath, int recordIndex, string? uniqueId)
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
            CopperfinStudioHostBridge.BuildRestoreObjectArguments(assetPath, recordIndex, uniqueId));
    }

    public static CopperfinStudioSnapshotResult TryDuplicateObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        string newUniqueId)
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
            CopperfinStudioHostBridge.BuildDuplicateObjectArguments(assetPath, recordIndex, uniqueId, newUniqueId));
    }

    public static CopperfinStudioSnapshotResult TryRenameObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        string newUniqueId)
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
            CopperfinStudioHostBridge.BuildRenameObjectArguments(assetPath, recordIndex, uniqueId, newUniqueId));
    }

    public static CopperfinStudioSnapshotResult TryReorderObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        string placement)
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
            CopperfinStudioHostBridge.BuildReorderObjectArguments(assetPath, recordIndex, uniqueId, placement));
    }
}
