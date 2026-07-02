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
    private static readonly CopperfinLocalization Localization = CopperfinLocalization.FromEnvironment();
    private const string DefaultSecurityBuildRole = "build-engineer";

    private const string RepoBuildHostPath = @"E:\Project-Copperfin\build\Release\copperfin_build_host.exe";
    private const string RepoRuntimeHostPath = @"E:\Project-Copperfin\build\Release\copperfin_runtime_host.exe";

    public static bool IsCopperfinProjectPath(string? path)
    {
        return !string.IsNullOrWhiteSpace(path) &&
               string.Equals(Path.GetExtension(path), ".pjx", StringComparison.OrdinalIgnoreCase);
    }

    public static string? ResolveBuildHostPath()
    {
        var configured = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        if (!string.IsNullOrWhiteSpace(configured) && File.Exists(configured))
        {
            return configured;
        }

        if (File.Exists(RepoBuildHostPath))
        {
            return RepoBuildHostPath;
        }

        return null;
    }

    public static string? ResolveRuntimeHostPath()
    {
        var configured = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        if (!string.IsNullOrWhiteSpace(configured) && File.Exists(configured))
        {
            return configured;
        }

        if (File.Exists(RepoRuntimeHostPath))
        {
            return RepoRuntimeHostPath;
        }

        return null;
    }

    public static Task<CopperfinProjectExecutionResult> ExecuteAsync(string projectPath, CopperfinProjectOperation operation)
    {
        return Task.Run(() => ExecuteCore(projectPath, operation));
    }

    private static CopperfinProjectExecutionResult ExecuteCore(string projectPath, CopperfinProjectOperation operation)
    {
        var buildHostPath = ResolveBuildHostPath();
        if (buildHostPath is null)
        {
            return Failure(projectPath, Localization.Text("AssetEditor.Project.Workflow.BuildHostMissing"));
        }

        var runtimeHostPath = ResolveRuntimeHostPath();
        if (runtimeHostPath is null)
        {
            return Failure(projectPath, Localization.Text("AssetEditor.Project.Workflow.RuntimeHostMissing"));
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
        if (ShouldEmitDotNetLauncher())
        {
            buildArguments.Add("--emit-dotnet-launcher");
        }

        var buildResult = RunProcess(
            buildHostPath,
            buildArguments,
            CreateSecurityEnabledBuildEnvironment());
        var warningCount = ParseIntOrDefault(GetValueOrDefault(buildResult.Values, "warnings"), 0);
        var warnings = ParseWarningLines(buildResult.StandardOutput);
        if (warningCount == 0 && warnings.Count > 0)
        {
            warningCount = warnings.Count;
        }
        else if (warningCount > 0 && warnings.Count == 0)
        {
            warnings = ParseWarningLines(buildResult.StandardError);
        }

        var manifestPath = GetValueOrDefault(buildResult.Values, "manifest.path");
        var debugManifestPath = GetValueOrDefault(buildResult.Values, "debug.manifest.path");
        if (buildResult.ExitCode != 0 || !string.Equals(GetValueOrDefault(buildResult.Values, "status"), "ok", StringComparison.OrdinalIgnoreCase))
        {
            return Failure(
                projectPath,
                Localization.Text("AssetEditor.Project.Workflow.BuildFailed"),
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
                Localization.Text("AssetEditor.Project.Workflow.LauncherMissing"),
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
                Message = Localization.Text("AssetEditor.Project.Workflow.BuildSuccess"),
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
        var launchResult = StartProcess(launcherPath, launchArguments);
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
                ? Localization.Text("AssetEditor.Project.Workflow.LaunchDebugSuccess")
                : Localization.Text("AssetEditor.Project.Workflow.LaunchSuccess"),
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

    private static CopperfinProcessExecutionResult RunProcess(
        string fileName,
        IEnumerable<string> arguments,
        IReadOnlyDictionary<string, string>? environmentVariables = null)
    {
        var startInfo = new ProcessStartInfo
        {
            FileName = fileName,
            Arguments = string.Join(" ", arguments),
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true
        };
        if (environmentVariables is not null)
        {
            foreach (var kvp in environmentVariables)
            {
                startInfo.EnvironmentVariables[kvp.Key] = kvp.Value;
            }
        }

        using var process = new System.Diagnostics.Process { StartInfo = startInfo };
        if (!process.Start())
        {
            return new CopperfinProcessExecutionResult
            {
                ExitCode = -1,
                StandardError = Localization.Text("AssetEditor.Project.Workflow.ProcessCouldNotStart")
            };
        }

        var stdout = process.StandardOutput.ReadToEnd();
        var stderr = process.StandardError.ReadToEnd();
        process.WaitForExit();

        var result = new CopperfinProcessExecutionResult
        {
            ExitCode = process.ExitCode,
            StandardOutput = stdout.Trim(),
            StandardError = stderr.Trim()
        };

        foreach (var kvp in ParseKeyValueLines(stdout))
        {
            result.Values[kvp.Key] = kvp.Value;
        }

        result.Values["stdout"] = result.StandardOutput;
        result.Values["stderr"] = result.StandardError;
        result.Values["exit_code"] = result.ExitCode.ToString();
        return result;
    }

    private static CopperfinProjectExecutionResult StartProcess(string fileName, IEnumerable<string> arguments)
    {
        var startInfo = new ProcessStartInfo
        {
            FileName = fileName,
            Arguments = string.Join(" ", arguments),
            UseShellExecute = false,
            CreateNoWindow = true
        };

        try
        {
            if (System.Diagnostics.Process.Start(startInfo) is null)
            {
                return Failure(fileName, Localization.Text("AssetEditor.Project.Workflow.LauncherCouldNotStart"));
            }
        }
        catch (Exception ex)
        {
            return Failure(fileName, Localization.Format("AssetEditor.Project.Workflow.LauncherCouldNotStartWithMessage", ex.Message));
        }

        return new CopperfinProjectExecutionResult
        {
            Success = true,
            Message = Localization.Text("AssetEditor.Project.Workflow.LauncherStarted"),
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
            DateTime.UtcNow.ToString("yyyyMMdd_HHmmss"));
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

    private static List<string> ParseWarningLines(string text)
    {
        var warnings = new List<string>();
        using var reader = new StringReader(text);
        string? line;
        while ((line = reader.ReadLine()) is not null)
        {
            if (!line.StartsWith("warning: ", StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            warnings.Add(line.Substring("warning: ".Length).Trim());
        }

        return warnings;
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
        return "\"" + value.Replace("\"", "\"\"") + "\"";
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
