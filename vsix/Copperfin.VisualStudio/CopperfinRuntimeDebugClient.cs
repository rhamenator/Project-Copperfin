// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Copperfin.VisualStudio;

internal static class CopperfinRuntimeDebugClient
{
    private const string DefaultInjectedBuildRole = "build-engineer";
    private const string DefaultRuntimeDebugRole = "runtime-operator";

    public static async Task<CopperfinRuntimeDebugSession> StartSessionAsync(
        string projectPath,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var buildResult = await CopperfinProjectWorkflow.ExecuteAsync(projectPath, CopperfinProjectOperation.Build, localization);
        if (!buildResult.Success)
        {
            return new CopperfinRuntimeDebugSession
            {
                Success = false,
                Error = buildResult.Message,
                ManifestPath = buildResult.ManifestPath,
                DebugManifestPath = buildResult.DebugManifestPath,
                BuildWarningCount = buildResult.WarningCount,
                BuildWarnings = buildResult.Warnings.ToList()
            };
        }

        if (string.IsNullOrWhiteSpace(buildResult.DebugManifestPath) || !File.Exists(buildResult.DebugManifestPath))
        {
            return new CopperfinRuntimeDebugSession
            {
                Success = false,
                Error = localization.Text("AssetEditor.Dialog.DebugManifestMissing")
            };
        }

        var effectiveDebugManifestPath = PrepareReplayManifest(buildResult.DebugManifestPath);
        return await ReplayAsync(new CopperfinRuntimeDebugSession
        {
            Success = true,
            ManifestPath = buildResult.ManifestPath,
            DebugManifestPath = effectiveDebugManifestPath,
            BuildWarningCount = buildResult.WarningCount,
            BuildWarnings = buildResult.Warnings.ToList(),
            Commands = new List<string> { "continue" }
        }, localization);
    }

    public static Task<CopperfinRuntimeDebugSession> ContinueAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        return ReplayWithCommandAsync(session, "continue", localization);
    }

    public static Task<CopperfinRuntimeDebugSession> StepIntoAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        return ReplayWithCommandAsync(session, "step", localization);
    }

    public static Task<CopperfinRuntimeDebugSession> StepOverAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        return ReplayWithCommandAsync(session, "next", localization);
    }

    public static Task<CopperfinRuntimeDebugSession> StepOutAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        return ReplayWithCommandAsync(session, "out", localization);
    }

    private static Task<CopperfinRuntimeDebugSession> ReplayWithCommandAsync(
        CopperfinRuntimeDebugSession session,
        string command,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        var commands = session.Commands.ToList();
        commands.Add(command);
        return ReplayAsync(new CopperfinRuntimeDebugSession
        {
            Success = session.Success,
            Error = session.Error,
            ManifestPath = session.ManifestPath,
            DebugManifestPath = session.DebugManifestPath,
            BuildWarningCount = session.BuildWarningCount,
            BuildWarnings = session.BuildWarnings.ToList(),
            Commands = commands
        }, localization);
    }

    private static Task<CopperfinRuntimeDebugSession> ReplayAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        return Task.Run(() =>
        {
            var runtimeHostPath = CopperfinProjectWorkflow.ResolveRuntimeHostPath();
            if (string.IsNullOrWhiteSpace(runtimeHostPath) || !File.Exists(runtimeHostPath))
            {
                session.Success = false;
                session.Error = localization.Text("AssetEditor.Dialog.RuntimeHostMissing");
                return session;
            }

            var arguments = new StringBuilder();
            arguments.Append("--manifest ").Append(Quote(session.DebugManifestPath)).Append(" --debug");
            arguments.Append(" --locale ").Append(Quote(localization.Locale));
            foreach (var command in session.Commands)
            {
                arguments.Append(" --debug-command ").Append(Quote(command));
            }

            var startInfo = new ProcessStartInfo
            {
                FileName = runtimeHostPath,
                Arguments = arguments.ToString(),
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true
            };
            CopperfinStudioHostBridge.ApplyLocalizationEnvironment(startInfo, localization);

            var processResult = CopperfinProcessRunner.Run(startInfo, timeoutMilliseconds: 30000);
            if (!processResult.Started)
            {
                session.Success = false;
                session.Error = localization.Text("AssetEditor.Dialog.RuntimeHostCouldNotStart");
                return session;
            }

            if (processResult.TimedOut)
            {
                session.Success = false;
                session.Error = localization.Text("AssetEditor.Dialog.RuntimeHostTimedOut");
                return session;
            }

            var pauseState = ParsePauseState(processResult.StandardOutput);
            if (processResult.ExitCode != 0)
            {
                if (HasDebugData(pauseState))
                {
                    session.Success = true;
                    session.Error = string.Empty;
                    session.State = pauseState;
                    return session;
                }

                session.Success = false;
                session.Error = string.IsNullOrWhiteSpace(processResult.StandardError)
                    ? processResult.StandardOutput.Trim()
                    : processResult.StandardError.Trim();
                return session;
            }

            session.Success = true;
            session.Error = string.Empty;
            session.State = pauseState;
            return session;
        });
    }

    private static CopperfinRuntimePauseState ParsePauseState(string stdout)
    {
        var states = new List<CopperfinRuntimePauseState>();
        var current = new CopperfinRuntimePauseState();
        var sawCommand = false;

        using var reader = new StringReader(stdout);
        string? rawLine;
        while ((rawLine = reader.ReadLine()) is not null)
        {
            var line = rawLine.Trim();
            if (line.Length == 0)
            {
                continue;
            }

            if (line.StartsWith("debug.command[", StringComparison.OrdinalIgnoreCase))
            {
                if (sawCommand)
                {
                    states.Add(current);
                    current = new CopperfinRuntimePauseState();
                }

                sawCommand = true;
                continue;
            }

            if (!line.StartsWith("debug.", StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            ApplyDebugLine(current, line);
        }

        if (sawCommand || HasDebugData(current))
        {
            states.Add(current);
        }

        return states.Count == 0 ? new CopperfinRuntimePauseState() : states[states.Count - 1];
    }

    private static void ApplyDebugLine(CopperfinRuntimePauseState state, string line)
    {
        var separator = line.IndexOf(": ", StringComparison.Ordinal);
        if (separator <= 0)
        {
            return;
        }

        var key = line.Substring(0, separator);
        var value = line.Substring(separator + 2);

        if (string.Equals(key, "debug.reason", StringComparison.OrdinalIgnoreCase))
        {
            state.Reason = value;
            return;
        }

        if (string.Equals(key, "debug.location", StringComparison.OrdinalIgnoreCase))
        {
            state.Location = value;
            return;
        }

        if (string.Equals(key, "debug.statement", StringComparison.OrdinalIgnoreCase))
        {
            state.Statement = value;
            return;
        }

        if (string.Equals(key, "debug.message", StringComparison.OrdinalIgnoreCase))
        {
            state.Message = value;
            return;
        }

        if (string.Equals(key, "debug.stack.depth", StringComparison.OrdinalIgnoreCase))
        {
            state.StackDepth = ParseInt(value);
            return;
        }

        if (string.Equals(key, "debug.executed.statements", StringComparison.OrdinalIgnoreCase))
        {
            state.ExecutedStatements = ParseInt(value);
            return;
        }

        if (key.StartsWith("debug.global.", StringComparison.OrdinalIgnoreCase))
        {
            state.Globals.Add(new CopperfinRuntimeNamedValue
            {
                Name = key.Substring("debug.global.".Length),
                Value = value
            });
            return;
        }

        if (TryParseFrameLocal(state, key, value))
        {
            return;
        }

        if (TryParseFrame(state, key, value))
        {
            return;
        }

        _ = TryParseEvent(state, key, value);
    }

    private static bool TryParseFrame(CopperfinRuntimePauseState state, string key, string value)
    {
        if (!key.StartsWith("debug.frame[", StringComparison.OrdinalIgnoreCase) ||
            key.IndexOf(".local.", StringComparison.OrdinalIgnoreCase) >= 0)
        {
            return false;
        }

        var closeIndex = key.IndexOf(']');
        if (closeIndex <= "debug.frame[".Length)
        {
            return false;
        }

        var frameIndex = ParseInt(key.Substring("debug.frame[".Length, closeIndex - "debug.frame[".Length));
        EnsureFrameCount(state, frameIndex + 1);

        var separator = value.IndexOf('@');
        if (separator > 0)
        {
            state.Frames[frameIndex].RoutineName = value.Substring(0, separator);
            state.Frames[frameIndex].Location = value.Substring(separator + 1);
        }
        else
        {
            state.Frames[frameIndex].RoutineName = value;
        }

        return true;
    }

    private static bool TryParseFrameLocal(CopperfinRuntimePauseState state, string key, string value)
    {
        if (!key.StartsWith("debug.frame[", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        var localMarker = key.IndexOf("].local.", StringComparison.OrdinalIgnoreCase);
        if (localMarker <= "debug.frame[".Length)
        {
            return false;
        }

        var frameIndex = ParseInt(key.Substring("debug.frame[".Length, localMarker - "debug.frame[".Length));
        var localName = key.Substring(localMarker + "].local.".Length);
        EnsureFrameCount(state, frameIndex + 1);
        state.Frames[frameIndex].Locals.Add(new CopperfinRuntimeNamedValue
        {
            Name = localName,
            Value = value
        });
        return true;
    }

    private static bool TryParseEvent(CopperfinRuntimePauseState state, string key, string value)
    {
        if (!key.StartsWith("debug.event[", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        var closeIndex = key.IndexOf(']');
        if (closeIndex <= "debug.event[".Length || (closeIndex + 2) >= key.Length)
        {
            return false;
        }

        var eventIndex = ParseInt(key.Substring("debug.event[".Length, closeIndex - "debug.event[".Length));
        EnsureEventCount(state, eventIndex + 1);
        var propertyName = key.Substring(closeIndex + 2);
        if (string.Equals(propertyName, "category", StringComparison.OrdinalIgnoreCase))
        {
            state.Events[eventIndex].Category = value;
        }
        else if (string.Equals(propertyName, "detail", StringComparison.OrdinalIgnoreCase))
        {
            state.Events[eventIndex].Detail = value;
        }
        else if (string.Equals(propertyName, "location", StringComparison.OrdinalIgnoreCase))
        {
            state.Events[eventIndex].Location = value;
        }

        return true;
    }

    private static void EnsureFrameCount(CopperfinRuntimePauseState state, int count)
    {
        while (state.Frames.Count < count)
        {
            state.Frames.Add(new CopperfinRuntimeStackFrame());
        }
    }

    private static void EnsureEventCount(CopperfinRuntimePauseState state, int count)
    {
        while (state.Events.Count < count)
        {
            state.Events.Add(new CopperfinRuntimeEvent());
        }
    }

    private static int ParseInt(string value)
    {
        return int.TryParse(value, out var result) ? result : 0;
    }

    private static bool HasDebugData(CopperfinRuntimePauseState state)
    {
        return !string.IsNullOrWhiteSpace(state.Reason) ||
               !string.IsNullOrWhiteSpace(state.Location) ||
               !string.IsNullOrWhiteSpace(state.Message) ||
               state.Frames.Count > 0 ||
               state.Globals.Count > 0 ||
               state.Events.Count > 0;
    }

    private static string PrepareReplayManifest(string debugManifestPath)
    {
        if (string.IsNullOrWhiteSpace(debugManifestPath) || !File.Exists(debugManifestPath))
        {
            return debugManifestPath;
        }

        if (!string.IsNullOrWhiteSpace(Environment.GetEnvironmentVariable("COPPERFIN_SECURITY_ROLE")))
        {
            return debugManifestPath;
        }

        string manifestText;
        try
        {
            manifestText = File.ReadAllText(debugManifestPath);
        }
        catch (IOException)
        {
            return debugManifestPath;
        }
        catch (UnauthorizedAccessException)
        {
            return debugManifestPath;
        }

        if (!string.Equals(ReadManifestValue(manifestText, "security_enabled"), "true", StringComparison.OrdinalIgnoreCase) ||
            !string.Equals(ReadManifestValue(manifestText, "security_role"), DefaultInjectedBuildRole, StringComparison.OrdinalIgnoreCase))
        {
            return debugManifestPath;
        }

        var replayManifestText = ReplaceManifestValue(manifestText, "security_role", DefaultRuntimeDebugRole);
        if (string.Equals(replayManifestText, manifestText, StringComparison.Ordinal))
        {
            return debugManifestPath;
        }

        var replayManifestDirectory = Path.GetDirectoryName(debugManifestPath);
        if (string.IsNullOrWhiteSpace(replayManifestDirectory))
        {
            return debugManifestPath;
        }

        var replayManifestPath = Path.Combine(
            replayManifestDirectory,
            Path.GetFileNameWithoutExtension(debugManifestPath) + ".runtime-debug-" + Guid.NewGuid().ToString("N") + Path.GetExtension(debugManifestPath));

        try
        {
            File.WriteAllText(replayManifestPath, replayManifestText);
            return replayManifestPath;
        }
        catch (IOException)
        {
            return debugManifestPath;
        }
        catch (UnauthorizedAccessException)
        {
            return debugManifestPath;
        }
    }

    private static string ReadManifestValue(string manifestText, string key)
    {
        using var reader = new StringReader(manifestText);
        string? line;
        while ((line = reader.ReadLine()) is not null)
        {
            if (line.StartsWith(key + "=", StringComparison.Ordinal))
            {
                return line.Substring(key.Length + 1);
            }
        }

        return string.Empty;
    }

    private static string ReplaceManifestValue(string manifestText, string key, string replacementValue)
    {
        var updated = false;
        using var reader = new StringReader(manifestText);
        using var writer = new StringWriter();
        string? line;
        while ((line = reader.ReadLine()) is not null)
        {
            if (!updated && line.StartsWith(key + "=", StringComparison.Ordinal))
            {
                writer.Write(key);
                writer.Write('=');
                writer.Write(replacementValue);
                writer.WriteLine();
                updated = true;
                continue;
            }

            writer.WriteLine(line);
        }

        return updated ? writer.ToString() : manifestText;
    }

    private static string Quote(string value)
    {
        return "\"" + value.Replace("\"", "\"\"") + "\"";
    }
}
