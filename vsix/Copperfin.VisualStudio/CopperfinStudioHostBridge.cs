// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using DiagnosticsProcess = System.Diagnostics.Process;
using DiagnosticsStartInfo = System.Diagnostics.ProcessStartInfo;

namespace Copperfin.VisualStudio;

internal static class CopperfinStudioHostBridge
{
    private static readonly string[] HostBuildConfigurations = { "Release", "RelWithDebInfo", "Debug" };

    public static string? ResolveStudioHostPath(string? baseDirectory = null)
    {
        return ResolveHostPath("COPPERFIN_STUDIO_HOST_PATH", "copperfin_studio_host", baseDirectory);
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

    public static string BuildPropertyBatchUpdateArguments(
        string documentPath,
        int recordIndex,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges)
    {
        var arguments = $"--from-vs --visual-object-update-batch --json --path {Quote(documentPath)} --selected-record {recordIndex}";
        foreach (var propertyChange in propertyChanges)
        {
            arguments += $" --property-name {Quote(propertyChange.Key)} --property-value {Quote(propertyChange.Value)}";
        }

        return arguments;
    }

    public static string BuildUndoArguments(string documentPath, int? selectedRecordIndex = null)
    {
        var arguments = "--from-vs --json --undo-mode command";
        if (selectedRecordIndex.HasValue)
        {
            arguments += $" --record {selectedRecordIndex.Value}";
        }

        return $"{arguments} --path {Quote(documentPath)}";
    }

    public static string BuildDeletedStatesArguments(
        string documentPath,
        IReadOnlyList<KeyValuePair<string, bool>> deletedStateChanges)
    {
        var arguments = $"--from-vs --path {Quote(documentPath)} --deleted-states";
        foreach (var deletedStateChange in deletedStateChanges)
        {
            arguments += $" --deleted-state-target-unique-id {Quote(deletedStateChange.Key)}";
            arguments += $" --deleted-state {(deletedStateChange.Value ? "true" : "false")}";
        }

        return $"{arguments} --json";
    }

    public static string BuildDeleteObjectArguments(string documentPath, int recordIndex, string? uniqueId = null)
    {
        return BuildObjectLifecycleArguments(documentPath, "--delete-object", recordIndex, uniqueId);
    }

    public static string BuildRestoreObjectArguments(string documentPath, int recordIndex, string? uniqueId = null)
    {
        return BuildObjectLifecycleArguments(documentPath, "--restore-object", recordIndex, uniqueId);
    }

    public static string BuildDuplicateObjectArguments(
        string documentPath,
        int recordIndex,
        string? uniqueId,
        string newUniqueId)
    {
        var arguments = BuildObjectLifecycleArguments(documentPath, "--duplicate-object", recordIndex, uniqueId);
        return $"{arguments} --new-unique-id {Quote(newUniqueId)}";
    }

    public static string BuildRenameObjectArguments(
        string documentPath,
        int recordIndex,
        string? uniqueId,
        string newUniqueId)
    {
        var arguments = BuildObjectLifecycleArguments(documentPath, "--rename-object", recordIndex, uniqueId);
        return $"{arguments} --new-unique-id {Quote(newUniqueId)}";
    }

    public static string BuildReorderObjectArguments(
        string documentPath,
        int recordIndex,
        string? uniqueId,
        string placement)
    {
        var arguments = BuildObjectLifecycleArguments(documentPath, "--reorder-object", recordIndex, uniqueId);
        return $"{arguments} --placement {Quote(placement)}";
    }

    public static string BuildNudgeObjectArguments(
        string documentPath,
        int recordIndex,
        string? uniqueId,
        string mode,
        double deltaHpos,
        double deltaVpos)
    {
        var arguments = $"--from-vs --json --record {recordIndex} --nudge-object --nudge-mode {Quote(mode)}" +
                        $" --delta-hpos {deltaHpos.ToString(CultureInfo.InvariantCulture)}" +
                        $" --delta-vpos {deltaVpos.ToString(CultureInfo.InvariantCulture)}";
        if (!string.IsNullOrWhiteSpace(uniqueId))
        {
            arguments += $" --nudge-target-unique-id {Quote(uniqueId!)}";
        }

        return $"{arguments} --path {Quote(documentPath)}";
    }

    public static string BuildAlignObjectArguments(
        string documentPath,
        int recordIndex,
        string anchorUniqueId,
        string alignmentMode,
        IReadOnlyList<string> targetUniqueIds)
    {
        var arguments = $"--from-vs --json --record {recordIndex} --align-object --alignment-mode {Quote(alignmentMode)}" +
                        $" --anchor-unique-id {Quote(anchorUniqueId)}";
        foreach (var targetUniqueId in targetUniqueIds)
        {
            arguments += $" --align-target-unique-id {Quote(targetUniqueId)}";
        }

        return $"{arguments} --path {Quote(documentPath)}";
    }

    public static string BuildResizeObjectArguments(
        string documentPath,
        int recordIndex,
        string anchorUniqueId,
        string resizeMode,
        IReadOnlyList<string> targetUniqueIds)
    {
        var arguments = $"--from-vs --json --record {recordIndex} --resize-object --resize-mode {Quote(resizeMode)}" +
                        $" --anchor-unique-id {Quote(anchorUniqueId)}";
        foreach (var targetUniqueId in targetUniqueIds)
        {
            arguments += $" --resize-target-unique-id {Quote(targetUniqueId)}";
        }

        return $"{arguments} --path {Quote(documentPath)}";
    }

    public static string BuildDistributeObjectArguments(
        string documentPath,
        int recordIndex,
        string distributionMode,
        IReadOnlyList<string> targetUniqueIds)
    {
        var arguments = $"--from-vs --json --record {recordIndex} --distribute-object --distribution-mode {Quote(distributionMode)}";
        foreach (var targetUniqueId in targetUniqueIds)
        {
            arguments += $" --distribute-target-unique-id {Quote(targetUniqueId)}";
        }

        return $"{arguments} --path {Quote(documentPath)}";
    }

    public static string BuildSnapObjectArguments(
        string documentPath,
        int recordIndex,
        string snapMode,
        double gridWidth,
        double gridHeight,
        IReadOnlyList<string> targetUniqueIds)
    {
        var arguments = $"--from-vs --json --record {recordIndex} --snap-object --snap-mode {Quote(snapMode)}" +
                        $" --grid-width {gridWidth.ToString(CultureInfo.InvariantCulture)}" +
                        $" --grid-height {gridHeight.ToString(CultureInfo.InvariantCulture)}";
        foreach (var targetUniqueId in targetUniqueIds)
        {
            arguments += $" --snap-target-unique-id {Quote(targetUniqueId)}";
        }

        return $"{arguments} --path {Quote(documentPath)}";
    }

    public static bool Launch(
        string studioHostPath,
        string documentPath,
        bool readOnly = false,
        CopperfinLocalization? localization = null)
    {
        var startInfo = CreateProcessStartInfo(
            studioHostPath,
            BuildArguments(documentPath, readOnly),
            localization: localization);

        return DiagnosticsProcess.Start(startInfo) is not null;
    }

    internal static string? ResolveHostPath(
        string environmentVariableName,
        string executableStem,
        string? baseDirectory = null,
        Func<string, bool>? fileExists = null)
    {
        var configured = Environment.GetEnvironmentVariable(environmentVariableName);
        if (!string.IsNullOrWhiteSpace(configured) && File.Exists(configured))
        {
            return configured;
        }

        fileExists ??= File.Exists;
        foreach (var candidate in EnumerateHostCandidatePaths(executableStem, baseDirectory))
        {
            if (fileExists(candidate))
            {
                return candidate;
            }
        }

        return null;
    }

    internal static IEnumerable<string> EnumerateHostCandidatePaths(string executableStem, string? baseDirectory = null)
    {
        var seen = new HashSet<string>(Path.DirectorySeparatorChar == '\\'
            ? StringComparer.OrdinalIgnoreCase
            : StringComparer.Ordinal);

        foreach (var searchRoot in EnumerateHostSearchRoots(baseDirectory))
        {
            foreach (var fileName in EnumerateHostFileNames(executableStem))
            {
                if (TryAddCandidatePath(seen, Path.Combine(searchRoot, fileName), out var sameDirectoryCandidate))
                {
                    yield return sameDirectoryCandidate;
                }

                foreach (var configuration in HostBuildConfigurations)
                {
                    if (TryAddCandidatePath(
                            seen,
                            Path.Combine(searchRoot, "build", configuration, fileName),
                            out var buildCandidate))
                    {
                        yield return buildCandidate;
                    }
                }
            }
        }
    }

    private static IEnumerable<string> EnumerateHostSearchRoots(string? baseDirectory)
    {
        var current = Path.GetFullPath(string.IsNullOrWhiteSpace(baseDirectory) ? AppContext.BaseDirectory : baseDirectory);
        while (!string.IsNullOrWhiteSpace(current))
        {
            yield return current;

            var parent = Directory.GetParent(current);
            if (parent is null)
            {
                break;
            }

            current = parent.FullName;
        }
    }

    private static IEnumerable<string> EnumerateHostFileNames(string executableStem)
    {
        if (Path.HasExtension(executableStem))
        {
            yield return executableStem;
            yield break;
        }

        yield return executableStem + ".exe";
        yield return executableStem;
    }

    private static bool TryAddCandidatePath(HashSet<string> seen, string candidate, out string normalizedPath)
    {
        normalizedPath = Path.GetFullPath(candidate);
        return seen.Add(normalizedPath);
    }

    internal static DiagnosticsStartInfo CreateProcessStartInfo(
        string studioHostPath,
        string arguments,
        CopperfinLocalization? localization = null,
        bool redirectOutput = false,
        bool createNoWindow = false,
        bool? isWindowsOverride = null)
    {
        var isWindows = isWindowsOverride ?? Path.DirectorySeparatorChar == '\\';
        var commandPath = studioHostPath;
        var commandArguments = arguments;

        if (isWindows && IsWindowsScriptWrapper(studioHostPath))
        {
            commandPath = Environment.GetEnvironmentVariable("COMSPEC");
            if (string.IsNullOrWhiteSpace(commandPath))
            {
                commandPath = "cmd.exe";
            }

            commandArguments = $"/d /c \"{Quote(studioHostPath)}{(string.IsNullOrWhiteSpace(arguments) ? string.Empty : " " + arguments)}\"";
        }

        var startInfo = new DiagnosticsStartInfo
        {
            FileName = commandPath,
            Arguments = commandArguments,
            UseShellExecute = false,
            RedirectStandardOutput = redirectOutput,
            RedirectStandardError = redirectOutput,
            CreateNoWindow = createNoWindow
        };
        ApplyLocalizationEnvironment(startInfo, localization);
        return startInfo;
    }

    internal static void ApplyLocalizationEnvironment(
        DiagnosticsStartInfo startInfo,
        CopperfinLocalization? localization)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        if (string.IsNullOrWhiteSpace(localization.Locale))
        {
            return;
        }

        startInfo.EnvironmentVariables["COPPERFIN_UI_LOCALE"] = localization.Locale;
        startInfo.EnvironmentVariables["COPPERFIN_LOCALE"] = localization.Locale;
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
            ".prg" => "Studio.AssetKind.Program",
            _ => "Studio.AssetKind.Generic"
        };

        return localization.Text(key);
    }

    private static bool IsWindowsScriptWrapper(string path)
    {
        var extension = Path.GetExtension(path);
        return string.Equals(extension, ".cmd", StringComparison.OrdinalIgnoreCase) ||
               string.Equals(extension, ".bat", StringComparison.OrdinalIgnoreCase);
    }

    private static string Quote(string value)
    {
        return "\"" + value.Replace("\"", "\"\"") + "\"";
    }

    private static string BuildObjectLifecycleArguments(string documentPath, string command, int recordIndex, string? uniqueId)
    {
        var arguments = $"--from-vs --json {command} --record {recordIndex}";
        if (!string.IsNullOrWhiteSpace(uniqueId))
        {
            arguments += $" --unique-id {Quote(uniqueId!)}";
        }

        return $"{arguments} --path {Quote(documentPath)}";
    }
}
