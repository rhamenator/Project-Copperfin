// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Copperfin.VisualStudio;

internal enum CopperfinProjectOperation
{
    Build,
    Run,
    Debug
}

internal sealed class CopperfinProjectExecutionResult
{
    public bool Success { get; set; }
    public string Message { get; set; } = string.Empty;
    public string ProjectPath { get; set; } = string.Empty;
    public string OutputDirectory { get; set; } = string.Empty;
    public string ManifestPath { get; set; } = string.Empty;
    public string LauncherPath { get; set; } = string.Empty;
    public string DebugManifestPath { get; set; } = string.Empty;
    public int WarningCount { get; set; }
    public List<string> Warnings { get; } = new();
    public int ExitCode { get; set; }
    public string StandardOutput { get; set; } = string.Empty;
    public string StandardError { get; set; } = string.Empty;
}

internal sealed class CopperfinProcessExecutionResult
{
    public int ExitCode { get; set; }
    public string StandardOutput { get; set; } = string.Empty;
    public string StandardError { get; set; } = string.Empty;
    public Dictionary<string, string> Values { get; } = new(StringComparer.OrdinalIgnoreCase);
}

internal static class CopperfinProjectWorkflow
{
    private const string DefaultSecurityBuildRole = "build-engineer";

    public static bool IsCopperfinProjectPath(string? path)
    {
        return !string.IsNullOrWhiteSpace(path) &&
               string.Equals(Path.GetExtension(path), ".pjx", StringComparison.OrdinalIgnoreCase);
    }

    public static string? ResolveBuildHostPath(string? baseDirectory = null)
    {
        return CopperfinStudioHostBridge.ResolveHostPath(
            "COPPERFIN_BUILD_HOST_PATH",
            "copperfin_build_host",
            baseDirectory);
    }

    public static string? ResolveRuntimeHostPath(string? baseDirectory = null)
    {
        return CopperfinStudioHostBridge.ResolveHostPath(
            "COPPERFIN_RUNTIME_HOST_PATH",
            "copperfin_runtime_host",
            baseDirectory);
    }

    public static Task<CopperfinProjectExecutionResult> ExecuteAsync(
        string projectPath,
        CopperfinProjectOperation operation,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        return Task.Run(() => ExecuteCore(projectPath, operation, localization));
    }

    private static CopperfinProjectExecutionResult ExecuteCore(
        string projectPath,
        CopperfinProjectOperation operation,
        CopperfinLocalization localization)
    {
        var buildHostPath = ResolveBuildHostPath();
        if (buildHostPath is null)
        {
            return Failure(projectPath, localization.Text("AssetEditor.Project.Workflow.BuildHostMissing"));
        }

        var runtimeHostPath = ResolveRuntimeHostPath();
        if (runtimeHostPath is null)
        {
            return Failure(projectPath, localization.Text("AssetEditor.Project.Workflow.RuntimeHostMissing"));
        }

        var outputDirectory = CreateOutputDirectory(projectPath);
        var buildArguments = new List<string>
        {
            "build",
            "--project", Quote(projectPath),
            "--output-dir", Quote(outputDirectory),
            "--configuration", "debug",
            "--enable-security",
            "--runtime-host", Quote(runtimeHostPath)
        };
        if (!string.IsNullOrWhiteSpace(localization.Locale))
        {
            buildArguments.Add("--locale");
            buildArguments.Add(Quote(localization.Locale));
        }
        if (ShouldEmitDotNetLauncher())
        {
            buildArguments.Add("--emit-dotnet-launcher");
        }

        var buildResult = RunProcess(
            buildHostPath,
            buildArguments,
            CreateSecurityEnabledBuildEnvironment(),
            localization);
        var warningCount = ParseIntOrDefault(GetValueOrDefault(buildResult.Values, "warnings"), 0);
        var warnings = ParseWarningLines(buildResult.StandardOutput, localization);
        if (warningCount == 0 && warnings.Count > 0)
        {
            warningCount = warnings.Count;
        }
        else if (warningCount > 0 && warnings.Count == 0)
        {
            warnings = ParseWarningLines(buildResult.StandardError, localization);
        }

        var manifestPath = GetValueOrDefault(buildResult.Values, "manifest.path");
        var debugManifestPath = GetValueOrDefault(buildResult.Values, "debug.manifest.path");
        if (buildResult.ExitCode != 0 || !string.Equals(GetValueOrDefault(buildResult.Values, "status"), "ok", StringComparison.OrdinalIgnoreCase))
        {
            return Failure(
                projectPath,
                localization.Text("AssetEditor.Project.Workflow.BuildFailed"),
                outputDirectory,
                manifestPath,
                string.Empty,
                debugManifestPath,
                warningCount,
                warnings,
                buildResult.ExitCode,
                buildResult.StandardOutput,
                buildResult.StandardError);
        }

        var launcherPath = buildResult.Values.TryGetValue("launcher.output", out var parsedLauncher)
            ? parsedLauncher
            : InferLauncherPath(buildResult.Values, projectPath, outputDirectory);

        if (string.IsNullOrWhiteSpace(launcherPath) || !File.Exists(launcherPath))
        {
            return Failure(
                projectPath,
                localization.Text("AssetEditor.Project.Workflow.LauncherMissing"),
                outputDirectory,
                manifestPath,
                launcherPath ?? string.Empty,
                debugManifestPath,
                warningCount,
                warnings,
                buildResult.ExitCode,
                buildResult.StandardOutput,
                buildResult.StandardError);
        }

        if (operation == CopperfinProjectOperation.Build)
        {
            var result = new CopperfinProjectExecutionResult
            {
                Success = true,
                Message = localization.Text("AssetEditor.Project.Workflow.BuildSuccess"),
                ProjectPath = projectPath,
                OutputDirectory = outputDirectory,
                ManifestPath = manifestPath,
                LauncherPath = launcherPath,
                DebugManifestPath = debugManifestPath,
                WarningCount = warningCount,
                ExitCode = buildResult.ExitCode,
                StandardOutput = buildResult.StandardOutput,
                StandardError = buildResult.StandardError
            };
            result.Warnings.AddRange(warnings);
            return result;
        }

        var launchArguments = operation == CopperfinProjectOperation.Debug ? new[] { "--debug" } : Array.Empty<string>();
        var launchResult = StartProcess(launcherPath, launchArguments, localization);
        if (!launchResult.Success)
        {
            return Failure(
                projectPath,
                launchResult.Message,
                outputDirectory,
                manifestPath,
                launcherPath,
                debugManifestPath,
                warningCount,
                warnings,
                buildResult.ExitCode,
                buildResult.StandardOutput,
                buildResult.StandardError);
        }

        var launchWorkflowResult = new CopperfinProjectExecutionResult
        {
            Success = true,
            Message = operation == CopperfinProjectOperation.Debug
                ? localization.Text("AssetEditor.Project.Workflow.LaunchDebugSuccess")
                : localization.Text("AssetEditor.Project.Workflow.LaunchSuccess"),
            ProjectPath = projectPath,
            OutputDirectory = outputDirectory,
            ManifestPath = manifestPath,
            LauncherPath = launcherPath,
            DebugManifestPath = debugManifestPath,
            WarningCount = warningCount,
            ExitCode = buildResult.ExitCode,
            StandardOutput = buildResult.StandardOutput,
            StandardError = buildResult.StandardError
        };
        launchWorkflowResult.Warnings.AddRange(warnings);
        return launchWorkflowResult;
    }

    private static CopperfinProjectExecutionResult Failure(
        string projectPath,
        string message,
        string outputDirectory = "",
        string manifestPath = "",
        string launcherPath = "",
        string debugManifestPath = "",
        int warningCount = 0,
        IReadOnlyCollection<string>? warnings = null,
        int exitCode = -1,
        string stdout = "",
        string stderr = "")
    {
        var result = new CopperfinProjectExecutionResult
        {
            Success = false,
            Message = message,
            ProjectPath = projectPath,
            OutputDirectory = outputDirectory,
            ManifestPath = manifestPath,
            LauncherPath = launcherPath,
            DebugManifestPath = debugManifestPath,
            WarningCount = warningCount,
            ExitCode = exitCode,
            StandardOutput = stdout,
            StandardError = stderr
        };
        if (warnings is not null)
        {
            result.Warnings.AddRange(warnings);
        }
        return result;
    }

    internal static CopperfinProcessExecutionResult RunProcess(
        string fileName,
        IEnumerable<string> arguments,
        IReadOnlyDictionary<string, string>? environmentVariables = null,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var startInfo = CreateProcessStartInfo(
            fileName,
            arguments,
            environmentVariables,
            localization,
            redirectOutput: true,
            createNoWindow: true);

        var processResult = CopperfinProcessRunner.Run(startInfo);
        if (!processResult.Started)
        {
            var failureMessage = string.IsNullOrWhiteSpace(processResult.StandardError)
                ? localization.Text("AssetEditor.Project.Workflow.ProcessCouldNotStart")
                : localization.Format(
                    "AssetEditor.Project.Workflow.ProcessCouldNotStartWithMessage",
                    processResult.StandardError.Trim());
            return new CopperfinProcessExecutionResult
            {
                ExitCode = -1,
                StandardError = failureMessage
            };
        }

        var result = new CopperfinProcessExecutionResult
        {
            ExitCode = processResult.ExitCode,
            StandardOutput = processResult.StandardOutput.Trim(),
            StandardError = processResult.StandardError.Trim()
        };

        foreach (var kvp in ParseKeyValueLines(processResult.StandardOutput))
        {
            result.Values[kvp.Key] = kvp.Value;
        }

        result.Values["stdout"] = result.StandardOutput;
        result.Values["stderr"] = result.StandardError;
        result.Values["exit_code"] = result.ExitCode.ToString();
        return result;
    }

    internal static ProcessStartInfo CreateProcessStartInfo(
        string fileName,
        IEnumerable<string> arguments,
        IReadOnlyDictionary<string, string>? environmentVariables = null,
        CopperfinLocalization? localization = null,
        bool redirectOutput = false,
        bool createNoWindow = false,
        bool? isWindowsOverride = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var startInfo = CopperfinStudioHostBridge.CreateProcessStartInfo(
            fileName,
            JoinProcessArguments(arguments),
            localization,
            redirectOutput,
            createNoWindow,
            isWindowsOverride);
        if (environmentVariables is not null)
        {
            foreach (var kvp in environmentVariables)
            {
                startInfo.EnvironmentVariables[kvp.Key] = kvp.Value;
            }
        }
        return startInfo;
    }

    private static CopperfinProjectExecutionResult StartProcess(
        string fileName,
        IEnumerable<string> arguments,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var startInfo = CreateProcessStartInfo(
            fileName,
            arguments,
            localization: localization,
            createNoWindow: true);

        try
        {
            if (System.Diagnostics.Process.Start(startInfo) is null)
            {
                return Failure(fileName, localization.Text("AssetEditor.Project.Workflow.LauncherCouldNotStart"));
            }
        }
        catch (Exception ex)
        {
            return Failure(fileName, localization.Format("AssetEditor.Project.Workflow.LauncherCouldNotStartWithMessage", ex.Message));
        }

        return new CopperfinProjectExecutionResult
        {
            Success = true,
            Message = localization.Text("AssetEditor.Project.Workflow.LauncherStarted"),
            ProjectPath = fileName
        };
    }

    private static string CreateOutputDirectory(string projectPath)
    {
        var projectName = Path.GetFileNameWithoutExtension(projectPath);
        var safeProjectName = string.IsNullOrWhiteSpace(projectName) ? "CopperfinProject" : projectName;
        var outputDirectory = Path.Combine(
            Path.GetTempPath(),
            "Copperfin",
            "VisualStudio",
            safeProjectName,
            DateTime.UtcNow.ToString("yyyyMMdd_HHmmss") + "_" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(outputDirectory);
        return outputDirectory;
    }

    private static Dictionary<string, string> ParseKeyValueLines(string text)
    {
        var values = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        using var reader = new StringReader(text);
        string? line;
        while ((line = reader.ReadLine()) is not null)
        {
            var separator = line.IndexOf(": ", StringComparison.Ordinal);
            if (separator <= 0)
            {
                continue;
            }

            var key = line.Substring(0, separator).Trim();
            var value = line.Substring(separator + 2).Trim();
            if (!string.IsNullOrWhiteSpace(key))
            {
                values[key] = value;
            }
        }

        return values;
    }

    private static List<string> ParseWarningLines(string text, CopperfinLocalization localization)
    {
        var warnings = new List<string>();
        var warningPrefixes = BuildWarningPrefixes(localization);
        using var reader = new StringReader(text);
        string? line;
        while ((line = reader.ReadLine()) is not null)
        {
            string? matchedPrefix = null;
            foreach (var warningPrefix in warningPrefixes)
            {
                if (line.StartsWith(warningPrefix, StringComparison.OrdinalIgnoreCase))
                {
                    matchedPrefix = warningPrefix;
                    break;
                }
            }

            if (matchedPrefix is null)
            {
                continue;
            }

            warnings.Add(line.Substring(matchedPrefix.Length).Trim());
        }

        return warnings;
    }

    private static IReadOnlyList<string> BuildWarningPrefixes(CopperfinLocalization localization)
    {
        var prefixes = new List<string>();
        var localizedPrefix = ResolveBuildWarningPrefix(localization);
        if (!string.IsNullOrWhiteSpace(localizedPrefix))
        {
            prefixes.Add(localizedPrefix);
        }

        var defaultPrefix = new CopperfinLocalization(CopperfinLocalization.DefaultLocale)
            .Text("BuildHost.Prefix.Warning");
        if (!string.IsNullOrWhiteSpace(defaultPrefix) &&
            !prefixes.Any(prefix => string.Equals(prefix, defaultPrefix, StringComparison.OrdinalIgnoreCase)))
        {
            prefixes.Add(defaultPrefix);
        }

        return prefixes;
    }

    private static string ResolveBuildWarningPrefix(CopperfinLocalization localization)
    {
        if (string.Equals(localization.Locale, CopperfinLocalization.PseudoLocale, StringComparison.OrdinalIgnoreCase))
        {
            return new CopperfinLocalization(CopperfinLocalization.DefaultLocale)
                .Text("BuildHost.Prefix.Warning");
        }

        return localization.Text("BuildHost.Prefix.Warning");
    }

    private static int ParseIntOrDefault(string value, int fallback)
    {
        return int.TryParse(value, out var parsed) ? parsed : fallback;
    }

    private static string GetValueOrDefault(IReadOnlyDictionary<string, string> values, string key)
    {
        return values.TryGetValue(key, out var value) ? value : string.Empty;
    }

    private static string InferLauncherPath(IReadOnlyDictionary<string, string> values, string projectPath, string outputDirectory)
    {
        if (values.TryGetValue("project.title", out var projectTitle) && !string.IsNullOrWhiteSpace(projectTitle))
        {
            var launcherFolder = Path.Combine(outputDirectory, projectTitle.Trim());
            var launcher = Path.Combine(launcherFolder, projectTitle.Trim() + ".exe");
            if (File.Exists(launcher))
            {
                return launcher;
            }
        }

        var fallbackName = Path.GetFileNameWithoutExtension(projectPath).ToUpperInvariant();
        var fallbackLauncher = Path.Combine(outputDirectory, fallbackName, fallbackName + ".exe");
        return File.Exists(fallbackLauncher) ? fallbackLauncher : string.Empty;
    }

    private static string Quote(string value)
    {
        return CopperfinStudioHostBridge.QuoteProcessArgument(value);
    }

    private static string JoinProcessArguments(IEnumerable<string> arguments)
    {
        return string.Join(" ", arguments.Select(argument => string.IsNullOrEmpty(argument) ? "\"\"" : argument));
    }

    private static bool ShouldEmitDotNetLauncher()
    {
        return Path.DirectorySeparatorChar == '\\';
    }

    private static IReadOnlyDictionary<string, string>? CreateSecurityEnabledBuildEnvironment()
    {
        if (!string.IsNullOrWhiteSpace(Environment.GetEnvironmentVariable("COPPERFIN_SECURITY_ROLE")))
        {
            return null;
        }

        return new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["COPPERFIN_SECURITY_ROLE"] = DefaultSecurityBuildRole
        };
    }
}
