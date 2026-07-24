// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using DiagnosticsProcess = System.Diagnostics.Process;
using DiagnosticsStartInfo = System.Diagnostics.ProcessStartInfo;

namespace Copperfin.VisualStudio;

internal static class CopperfinStudioHostBridge
{
    private const int PosixExecutePermission = 1;
    private const string WindowsScriptWrapperCommandVariable = "COPPERFIN_SCRIPT_WRAPPER_COMMAND";
    private static readonly string[] HostBuildConfigurations = { "Release", "RelWithDebInfo", "Debug" };

    public static string? ResolveStudioHostPath(string? baseDirectory = null)
    {
        return ResolveHostPath(
            "COPPERFIN_STUDIO_HOST_PATH",
            "copperfin_studio_host",
            baseDirectory,
            enforceConfiguredPosixExecutable: true);
    }

    public static string BuildArguments(
        string documentPath,
        bool readOnly = false,
        string? objectName = null,
        string? uniqueId = null)
    {
        var arguments = readOnly
            ? $"--from-vs --read-only --path {Quote(documentPath)}"
            : $"--from-vs --path {Quote(documentPath)}";
        if (!string.IsNullOrWhiteSpace(objectName))
        {
            arguments += $" --object-name {Quote(objectName!)}";
        }
        if (!string.IsNullOrWhiteSpace(uniqueId))
        {
            arguments += $" --unique-id {Quote(uniqueId!)}";
        }

        return arguments;
    }

    public static string BuildPropertyUpdateArguments(string documentPath, int recordIndex, string propertyName, string propertyValue)
    {
        return $"--from-vs --json --set-property --record {recordIndex} --property-name {Quote(propertyName)} --property-value {Quote(propertyValue)} --path {Quote(documentPath)}";
    }

    public static string BuildToolboxPaletteQueryArguments(string toolboxContext)
    {
        return $"--json --toolbox-palette-query --toolbox-context {Quote(toolboxContext)}";
    }

    public static string BuildToolboxCreateArguments(
        string documentPath,
        string toolboxItemId,
        string toolboxContext)
    {
        return $"--json --toolbox-create {Quote(toolboxItemId)}" +
               $" --toolbox-context {Quote(toolboxContext)} --path {Quote(documentPath)}";
    }

    public static string BuildBuilderLaunchCatalogArguments(string builderContext)
    {
        return $"--json --builder-launch-catalog --builder-context {Quote(builderContext)}";
    }

    public static string BuildBuilderLaunchPlanArguments(
        string builderId,
        string builderContext,
        string? assetPath = null,
        int? recordIndex = null,
        string? objectName = null,
        string? uniqueId = null)
    {
        var arguments = $"--json --builder-launch-plan {Quote(builderId)} --builder-context {Quote(builderContext)}";
        if (!string.IsNullOrWhiteSpace(assetPath))
        {
            arguments += $" --path {Quote(assetPath!)}";
        }

        if (recordIndex.HasValue && recordIndex.Value >= 0)
        {
            arguments += $" --record {recordIndex.Value.ToString(CultureInfo.InvariantCulture)}";
        }

        if (!string.IsNullOrWhiteSpace(objectName))
        {
            arguments += $" --object-name {Quote(objectName!)}";
        }

        if (!string.IsNullOrWhiteSpace(uniqueId))
        {
            arguments += $" --unique-id {Quote(uniqueId!)}";
        }

        return arguments;
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
        CopperfinLocalization? localization = null,
        string? objectName = null,
        string? uniqueId = null)
    {
        var startInfo = CreateProcessStartInfo(
            studioHostPath,
            BuildArguments(documentPath, readOnly, objectName, uniqueId),
            localization: localization);

        try
        {
            return DiagnosticsProcess.Start(startInfo) is not null;
        }
        catch (Exception)
        {
            return false;
        }
    }

    internal static string? ResolveHostPath(
        string environmentVariableName,
        string executableStem,
        string? baseDirectory = null,
        Func<string, bool>? fileExists = null,
        Func<string, bool>? fileIsExecutable = null,
        bool? isWindowsOverride = null,
        bool enforceConfiguredPosixExecutable = false)
    {
        fileExists ??= File.Exists;
        var isWindows = isWindowsOverride ?? Path.DirectorySeparatorChar == '\\';
        fileIsExecutable ??= IsPosixExecutableFile;
        var configured = Environment.GetEnvironmentVariable(environmentVariableName);
        if (!string.IsNullOrWhiteSpace(configured))
        {
            return fileExists(configured) &&
                   (isWindows || !enforceConfiguredPosixExecutable || fileIsExecutable(configured))
                ? configured
                : null;
        }

        foreach (var candidate in EnumerateHostCandidatePaths(executableStem, baseDirectory, isWindows))
        {
            if (fileExists(candidate) && (isWindows || fileIsExecutable(candidate)))
            {
                return candidate;
            }
        }

        return null;
    }

    internal static IEnumerable<string> EnumerateHostCandidatePaths(
        string executableStem,
        string? baseDirectory = null,
        bool? isWindowsOverride = null)
    {
        var isWindows = isWindowsOverride ?? Path.DirectorySeparatorChar == '\\';
        var seen = new HashSet<string>(isWindows
            ? StringComparer.OrdinalIgnoreCase
            : StringComparer.Ordinal);

        foreach (var searchRoot in EnumerateHostSearchRoots(baseDirectory))
        {
            foreach (var fileName in EnumerateHostFileNames(executableStem, isWindows))
            {
                if (TryAddCandidatePath(seen, Path.Combine(searchRoot, fileName), out var sameDirectoryCandidate))
                {
                    yield return sameDirectoryCandidate;
                }

                if (TryAddCandidatePath(
                        seen,
                        Path.Combine(searchRoot, "build", fileName),
                        out var directBuildCandidate))
                {
                    yield return directBuildCandidate;
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

    private static IEnumerable<string> EnumerateHostFileNames(string executableStem, bool isWindows)
    {
        if (Path.HasExtension(executableStem))
        {
            yield return executableStem;
            yield break;
        }

        if (isWindows)
        {
            yield return executableStem + ".exe";
            yield return executableStem;
        }
        else
        {
            yield return executableStem;
            yield return executableStem + ".exe";
        }
    }

    private static bool IsPosixExecutableFile(string path)
    {
        try
        {
            return PosixAccess(path, PosixExecutePermission) == 0;
        }
        catch (DllNotFoundException)
        {
            return false;
        }
        catch (EntryPointNotFoundException)
        {
            return false;
        }
    }

    [DllImport("libc", EntryPoint = "access", CharSet = CharSet.Ansi, SetLastError = true)]
    private static extern int PosixAccess(string path, int mode);

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
        string? wrapperCommand = null;

        if (isWindows && IsWindowsScriptWrapper(studioHostPath))
        {
            commandPath = Environment.GetEnvironmentVariable("COMSPEC");
            if (string.IsNullOrWhiteSpace(commandPath))
            {
                commandPath = "cmd.exe";
            }

            wrapperCommand = $"{Quote(studioHostPath)}{(string.IsNullOrWhiteSpace(arguments) ? string.Empty : " " + arguments)}";
            commandArguments = $"/d /c %{WindowsScriptWrapperCommandVariable}%";
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
        if (redirectOutput)
        {
            startInfo.StandardOutputEncoding = Encoding.UTF8;
            startInfo.StandardErrorEncoding = Encoding.UTF8;
        }
        if (wrapperCommand is not null)
        {
            startInfo.EnvironmentVariables[WindowsScriptWrapperCommandVariable] = wrapperCommand;
        }
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

    internal static string QuoteProcessArgument(string value)
    {
        var builder = new StringBuilder(value.Length + 2);
        var backslashes = 0;
        builder.Append('"');
        foreach (var character in value)
        {
            if (character == '\\')
            {
                backslashes++;
                continue;
            }

            if (character == '"')
            {
                builder.Append('\\', backslashes * 2 + 1);
            }
            else
            {
                builder.Append('\\', backslashes);
            }

            builder.Append(character);
            backslashes = 0;
        }

        builder.Append('\\', backslashes * 2);
        builder.Append('"');
        return builder.ToString();
    }

    private static string Quote(string value)
    {
        return QuoteProcessArgument(value);
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
