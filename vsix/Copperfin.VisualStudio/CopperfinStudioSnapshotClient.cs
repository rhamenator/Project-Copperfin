// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
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

    private static StudioHostCommandResult RunCommand(
        string studioHostPath,
        string arguments,
        CopperfinLocalization localization,
        bool allowNonZeroExit = false)
    {
        ProcessStartInfo startInfo;
        try
        {
            startInfo = CopperfinStudioHostBridge.CreateProcessStartInfo(
                studioHostPath,
                arguments,
                localization: localization,
                redirectOutput: true,
                createNoWindow: true);
        }
        catch (InvalidOperationException)
        {
            // EnsureSafeForCmdExeCommandLine() (#5432) throws instead of
            // returning a normal failure result when it refuses a launch;
            // route it through this function's existing StudioHostCommandResult
            // failure shape, which every one of its ~7 callers already checks
            // via commandResult.Success, so this single fix covers all of them
            // (issue #5446).
            return new StudioHostCommandResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostCouldNotStart")
            };
        }

        var processResult = CopperfinProcessRunner.Run(startInfo, timeoutMilliseconds: 15000);
        if (!processResult.Started)
        {
            return new StudioHostCommandResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostCouldNotStart")
            };
        }

        if (processResult.TimedOut)
        {
            return new StudioHostCommandResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostTimedOut")
            };
        }

        if (processResult.ExitCode != 0 && !allowNonZeroExit)
        {
            return new StudioHostCommandResult
            {
                Success = false,
                Error = string.IsNullOrWhiteSpace(processResult.StandardError)
                    ? processResult.StandardOutput.Trim()
                    : processResult.StandardError.Trim()
            };
        }

        return new StudioHostCommandResult
        {
            Success = true,
            Stdout = processResult.StandardOutput,
            Error = processResult.StandardError.Trim()
        };
    }

    private static CopperfinStudioSnapshotResult RunSnapshotCommand(
        string studioHostPath,
        string arguments,
        CopperfinLocalization localization)
    {
        var commandResult = RunCommand(studioHostPath, arguments, localization);
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
                    Error = localization.Text("AssetEditor.Dialog.StudioSnapshotEmpty")
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
                Error = localization.Format("AssetEditor.Dialog.StudioSnapshotParseFailed", ex.Message)
            };
        }
    }

    private static string ToolboxContextForAssetFamily(string assetFamily)
    {
        return assetFamily switch
        {
            "report" or "label" => "report",
            "class" => "class_designer",
            _ => "form"
        };
    }

    public static CopperfinStudioBuilderCatalogResult TryLoadBuilderCatalog(
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioBuilderCatalogResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        var contexts = new[]
        {
            "form", "class_designer", "control", "report", "label", "menu", "project", "data_environment"
        };
        var entries = new List<CopperfinStudioBuilderCatalogEntry>();
        var errors = new List<string>();
        foreach (var context in contexts)
        {
            var commandResult = RunCommand(
                studioHostPath!,
                CopperfinStudioHostBridge.BuildBuilderLaunchCatalogArguments(context),
                localization);
            if (!commandResult.Success)
            {
                errors.Add(commandResult.Error);
                continue;
            }

            try
            {
                var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 * 8 };
                var envelope = serializer.Deserialize<CopperfinStudioBuilderCatalogEnvelope>(commandResult.Stdout);
                var payload = envelope?.BuilderLaunchCatalog;
                if (payload is null || !payload.Ok)
                {
                    errors.Add(payload?.Error ?? envelope?.Error ?? context);
                    continue;
                }

                entries.AddRange(payload.Entries);
            }
            catch (InvalidOperationException ex)
            {
                errors.Add(localization.Format("AssetEditor.Dialog.StudioSnapshotParseFailed", ex.Message));
            }
        }

        return new CopperfinStudioBuilderCatalogResult
        {
            Success = entries.Count > 0,
            Error = entries.Count > 0 ? string.Empty : string.Join("; ", errors.Distinct(StringComparer.Ordinal)),
            Entries = entries
        };
    }

    public static CopperfinStudioBuilderLaunchPlanResult TryPlanBuilderLaunch(
        string builderId,
        string builderContext,
        string? assetPath = null,
        int? recordIndex = null,
        string? objectName = null,
        string? uniqueId = null,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioBuilderLaunchPlanResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        var commandResult = RunCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildBuilderLaunchPlanArguments(
                builderId,
                builderContext,
                assetPath,
                recordIndex,
                objectName,
                uniqueId),
            localization);
        if (!commandResult.Success)
        {
            return new CopperfinStudioBuilderLaunchPlanResult
            {
                Success = false,
                Error = commandResult.Error
            };
        }

        try
        {
            var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 * 8 };
            var envelope = serializer.Deserialize<CopperfinStudioBuilderLaunchPlanEnvelope>(commandResult.Stdout);
            var payload = envelope?.BuilderLaunchPlan;
            if (payload is null || !payload.Ok)
            {
                return new CopperfinStudioBuilderLaunchPlanResult
                {
                    Success = false,
                    Error = payload?.Error ?? envelope?.Error ?? localization.Text("AssetEditor.Builders.PlanUnavailable")
                };
            }

            return new CopperfinStudioBuilderLaunchPlanResult
            {
                Success = true,
                Plan = payload
            };
        }
        catch (InvalidOperationException ex)
        {
            return new CopperfinStudioBuilderLaunchPlanResult
            {
                Success = false,
                Error = localization.Format("AssetEditor.Dialog.StudioSnapshotParseFailed", ex.Message)
            };
        }
    }

    public static CopperfinStudioBuilderExecutionResult TryExecuteBuilder(
        string builderId,
        string builderContext,
        string? assetPath = null,
        int? recordIndex = null,
        string? objectName = null,
        string? uniqueId = null,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var launchCommand = Environment.GetEnvironmentVariable("COPPERFIN_BUILDER_LAUNCH_COMMAND");
        if (string.IsNullOrWhiteSpace(launchCommand))
        {
            return new CopperfinStudioBuilderExecutionResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Builders.Status.ExecutionNotConfigured")
            };
        }

        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioBuilderExecutionResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        var commandResult = RunCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildBuilderExecuteArguments(
                builderId,
                builderContext,
                launchCommand,
                assetPath,
                recordIndex,
                objectName,
                uniqueId),
            localization,
            allowNonZeroExit: true);
        if (!commandResult.Success)
        {
            return new CopperfinStudioBuilderExecutionResult
            {
                Success = false,
                Error = commandResult.Error
            };
        }

        try
        {
            var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 * 8 };
            var envelope = serializer.Deserialize<CopperfinStudioBuilderExecutionEnvelope>(commandResult.Stdout);
            var payload = envelope?.BuilderExecution;
            if (string.Equals(envelope?.Status, "ok", StringComparison.OrdinalIgnoreCase) && payload?.Ok == true)
            {
                return new CopperfinStudioBuilderExecutionResult
                {
                    Success = true,
                    ObservedExitCode = payload.ObservedExitCode,
                    Executed = payload.Executed
                };
            }

            var error = payload?.Error ?? string.Empty;
            if (string.IsNullOrWhiteSpace(error))
            {
                error = envelope?.Error ?? string.Empty;
            }

            if (string.IsNullOrWhiteSpace(error))
            {
                error = commandResult.Error ?? string.Empty;
            }

            if (string.IsNullOrWhiteSpace(error))
            {
                error = localization.Text("AssetEditor.Builders.Status.ExecutionFailed");
            }

            return new CopperfinStudioBuilderExecutionResult
            {
                Success = false,
                Error = error,
                ObservedExitCode = payload?.ObservedExitCode ?? envelope?.ObservedExitCode ?? 0,
                Executed = payload?.Executed ?? envelope?.Executed ?? false
            };
        }
        catch (InvalidOperationException ex)
        {
            return new CopperfinStudioBuilderExecutionResult
            {
                Success = false,
                Error = localization.Format("AssetEditor.Dialog.StudioSnapshotParseFailed", ex.Message)
            };
        }
    }

    public static CopperfinStudioToolboxPaletteResult TryLoadToolboxPalette(
        string assetFamily,
        CopperfinLocalization? localization = null,
        string? toolboxContext = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioToolboxPaletteResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        var resolvedToolboxContext = string.IsNullOrWhiteSpace(toolboxContext)
            ? ToolboxContextForAssetFamily(assetFamily)
            : toolboxContext!;

        var commandResult = RunCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildToolboxPaletteQueryArguments(
                resolvedToolboxContext),
            localization);
        if (!commandResult.Success)
        {
            return new CopperfinStudioToolboxPaletteResult
            {
                Success = false,
                Error = commandResult.Error
            };
        }

        try
        {
            var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 * 8 };
            var envelope = serializer.Deserialize<CopperfinStudioToolboxPaletteEnvelope>(commandResult.Stdout);
            var payload = envelope?.ToolboxPaletteQuery;
            if (payload is null || !payload.Ok)
            {
                return new CopperfinStudioToolboxPaletteResult
                {
                    Success = false,
                    Error = payload?.Error ?? envelope?.Error ?? localization.Text("AssetEditor.Toolbox.Unavailable")
                };
            }

            return new CopperfinStudioToolboxPaletteResult
            {
                Success = true,
                Items = payload.Items,
                Context = payload.ToolboxContext
            };
        }
        catch (InvalidOperationException ex)
        {
            return new CopperfinStudioToolboxPaletteResult
            {
                Success = false,
                Error = localization.Format("AssetEditor.Dialog.StudioSnapshotParseFailed", ex.Message)
            };
        }
    }

    public static CopperfinStudioToolboxCreateResult TryCreateToolboxItem(
        string assetPath,
        string toolboxItemId,
        string assetFamily,
        CopperfinLocalization? localization = null,
        string? toolboxContext = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioToolboxCreateResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        var resolvedToolboxContext = string.IsNullOrWhiteSpace(toolboxContext)
            ? ToolboxContextForAssetFamily(assetFamily)
            : toolboxContext!;

        var commandResult = RunCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildToolboxCreateArguments(
                assetPath,
                toolboxItemId,
                resolvedToolboxContext),
            localization);
        if (!commandResult.Success)
        {
            return new CopperfinStudioToolboxCreateResult
            {
                Success = false,
                Error = commandResult.Error
            };
        }

        try
        {
            var serializer = new JavaScriptSerializer { MaxJsonLength = 1024 * 1024 * 8 };
            var envelope = serializer.Deserialize<CopperfinStudioToolboxCreateEnvelope>(commandResult.Stdout);
            var payload = envelope?.ToolboxCreate;
            if (payload is null || !payload.Ok)
            {
                return new CopperfinStudioToolboxCreateResult
                {
                    Success = false,
                    Error = payload?.Error ?? envelope?.Error ?? localization.Text("AssetEditor.Toolbox.CreateFailed")
                };
            }

            return new CopperfinStudioToolboxCreateResult
            {
                Success = true,
                ObjectName = payload.ObjectName,
                UniqueId = payload.UniqueId
            };
        }
        catch (InvalidOperationException ex)
        {
            return new CopperfinStudioToolboxCreateResult
            {
                Success = false,
                Error = localization.Format("AssetEditor.Dialog.StudioSnapshotParseFailed", ex.Message)
            };
        }
    }

    private static CopperfinStudioSnapshotResult RunBatchPropertyUpdateAndReload(
        string studioHostPath,
        string assetPath,
        int recordIndex,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges,
        CopperfinLocalization localization)
    {
        var commandResult = RunCommand(
            studioHostPath,
            CopperfinStudioHostBridge.BuildPropertyBatchUpdateArguments(assetPath, recordIndex, propertyChanges),
            localization);
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
                    Error = localization.Text("AssetEditor.Dialog.StudioSnapshotEmpty")
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
                CopperfinStudioHostBridge.BuildArguments(assetPath, readOnly: false) + " --json",
                localization);
        }
        catch (InvalidOperationException ex)
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Format("AssetEditor.Dialog.StudioSnapshotParseFailed", ex.Message)
            };
        }
    }

    public static CopperfinStudioSnapshotResult TryLoad(
        string assetPath,
        CopperfinLocalization? localization = null,
        string? objectName = null,
        string? uniqueId = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildArguments(
                assetPath,
                readOnly: false,
                objectName: objectName,
                uniqueId: uniqueId) + " --json",
            localization);
    }

    public static CopperfinStudioSnapshotResult TryUpdateProperty(
        string assetPath,
        int recordIndex,
        string propertyName,
        string propertyValue,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildPropertyUpdateArguments(assetPath, recordIndex, propertyName, propertyValue),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryClearProperty(
        string assetPath,
        int recordIndex,
        string propertyName,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildPropertyClearArguments(assetPath, recordIndex, propertyName),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryUpdateProperties(
        string assetPath,
        int recordIndex,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        if (propertyChanges.Count == 0)
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioSnapshotEmpty")
            };
        }

        if (propertyChanges.Count == 1)
        {
            return TryUpdateProperty(assetPath, recordIndex, propertyChanges[0].Key, propertyChanges[0].Value, localization);
        }

        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunBatchPropertyUpdateAndReload(
            studioHostPath!,
            assetPath,
            recordIndex,
            propertyChanges,
            localization);
    }

    public static CopperfinStudioSnapshotResult TryUndoCommand(
        string assetPath,
        int? selectedRecordIndex = null,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildUndoArguments(assetPath, selectedRecordIndex),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryDeleteObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildDeleteObjectArguments(assetPath, recordIndex, uniqueId),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryRestoreObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildRestoreObjectArguments(assetPath, recordIndex, uniqueId),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryUpdateDeletedStates(
        string assetPath,
        IReadOnlyList<KeyValuePair<string, bool>> deletedStateChanges,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        if (deletedStateChanges.Count == 0)
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioSnapshotEmpty")
            };
        }

        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildDeletedStatesArguments(assetPath, deletedStateChanges),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryDuplicateObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        string newUniqueId,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildDuplicateObjectArguments(assetPath, recordIndex, uniqueId, newUniqueId),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryRenameObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        string newUniqueId,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildRenameObjectArguments(assetPath, recordIndex, uniqueId, newUniqueId),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryReorderObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        string placement,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildReorderObjectArguments(assetPath, recordIndex, uniqueId, placement),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryNudgeObject(
        string assetPath,
        int recordIndex,
        string? uniqueId,
        string mode,
        double deltaHpos,
        double deltaVpos,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildNudgeObjectArguments(
                assetPath,
                recordIndex,
                uniqueId,
                mode,
                deltaHpos,
                deltaVpos),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryAlignObject(
        string assetPath,
        int recordIndex,
        string anchorUniqueId,
        string alignmentMode,
        IReadOnlyList<string> targetUniqueIds,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildAlignObjectArguments(
                assetPath,
                recordIndex,
                anchorUniqueId,
                alignmentMode,
                targetUniqueIds),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryResizeObject(
        string assetPath,
        int recordIndex,
        string anchorUniqueId,
        string resizeMode,
        IReadOnlyList<string> targetUniqueIds,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildResizeObjectArguments(
                assetPath,
                recordIndex,
                anchorUniqueId,
                resizeMode,
                targetUniqueIds),
            localization);
    }

    public static CopperfinStudioSnapshotResult TryDistributeObject(
        string assetPath,
        int recordIndex,
        string distributionMode,
        IReadOnlyList<string> targetUniqueIds,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildDistributeObjectArguments(
                assetPath,
                recordIndex,
                distributionMode,
                targetUniqueIds),
            localization);
    }

    public static CopperfinStudioSnapshotResult TrySnapObject(
        string assetPath,
        int recordIndex,
        string snapMode,
        double gridWidth,
        double gridHeight,
        IReadOnlyList<string> targetUniqueIds,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (string.IsNullOrWhiteSpace(studioHostPath))
        {
            return new CopperfinStudioSnapshotResult
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.StudioHostMissing")
            };
        }

        return RunSnapshotCommand(
            studioHostPath!,
            CopperfinStudioHostBridge.BuildSnapObjectArguments(
                assetPath,
                recordIndex,
                snapMode,
                gridWidth,
                gridHeight,
                targetUniqueIds),
            localization);
    }
}
