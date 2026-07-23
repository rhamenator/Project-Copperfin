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

        return await StartPersistentAsync(new CopperfinRuntimeDebugSession
        {
            Success = true,
            ManifestPath = buildResult.ManifestPath,
            DebugManifestPath = buildResult.DebugManifestPath,
            BuildWarningCount = buildResult.WarningCount,
            BuildWarnings = buildResult.Warnings.ToList(),
            Commands = new List<string>(),
            StopOnEntry = true
        }, localization);
    }

    public static Task<CopperfinRuntimeDebugSession> ContinueAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        if (session.Transport is not null)
        {
            return AdvanceLiveAsync(session, "continue", localization);
        }

        return ReplayWithCommandAsync(session, "continue", localization);
    }

    public static Task<CopperfinRuntimeDebugSession> StepIntoAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        if (session.Transport is not null)
        {
            return AdvanceLiveAsync(session, "step", localization);
        }

        return ReplayWithCommandAsync(session, "step", localization);
    }

    public static Task<CopperfinRuntimeDebugSession> StepOverAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        if (session.Transport is not null)
        {
            return AdvanceLiveAsync(session, "next", localization);
        }

        return ReplayWithCommandAsync(session, "next", localization);
    }

    public static Task<CopperfinRuntimeDebugSession> StepOutAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        if (session.Transport is not null)
        {
            return AdvanceLiveAsync(session, "out", localization);
        }

        return ReplayWithCommandAsync(session, "out", localization);
    }

    public static Task<CopperfinRuntimeDebugSession> EvaluateWatchAsync(
        CopperfinRuntimeDebugSession session,
        string expression,
        CopperfinLocalization? localization = null)
    {
        if (session.Transport is not null)
        {
            return AdvanceLiveAsync(session, "watch:" + expression, localization);
        }

        return ReplayWithCommandAsync(session, "watch:" + expression, localization);
    }

    public static Task<CopperfinRuntimeDebugSession> AddBreakpointAsync(
        CopperfinRuntimeDebugSession session,
        string specification,
        CopperfinLocalization? localization = null)
    {
        return SendDebuggerCommandAsync(session, "break:add:" + specification, localization);
    }

    public static Task<CopperfinRuntimeDebugSession> RemoveBreakpointAsync(
        CopperfinRuntimeDebugSession session,
        string specification,
        CopperfinLocalization? localization = null)
    {
        return SendDebuggerCommandAsync(session, "break:remove:" + specification, localization);
    }

    public static Task<CopperfinRuntimeDebugSession> ClearBreakpointsAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization? localization = null)
    {
        return SendDebuggerCommandAsync(session, "break:clear", localization);
    }

    private static Task<CopperfinRuntimeDebugSession> SendDebuggerCommandAsync(
        CopperfinRuntimeDebugSession session,
        string command,
        CopperfinLocalization? localization)
    {
        return session.Transport is not null
            ? AdvanceLiveAsync(session, command, localization)
            : ReplayWithCommandAsync(session, command, localization);
    }

    internal static void Stop(CopperfinRuntimeDebugSession session)
    {
        session.Transport?.Dispose();
        session.Transport = null;
        TryDeleteReplayManifest(session.DebugManifestPath, session.TransportManifestPath);
        session.TransportManifestPath = string.Empty;
    }

    private static async Task<CopperfinRuntimeDebugSession> StartPersistentAsync(
        CopperfinRuntimeDebugSession session,
        CopperfinLocalization localization)
    {
        var runtimeHostPath = CopperfinProjectWorkflow.ResolveRuntimeHostPath();
        if (string.IsNullOrWhiteSpace(runtimeHostPath) || !File.Exists(runtimeHostPath))
        {
            session.Success = false;
            session.Error = localization.Text("AssetEditor.Dialog.RuntimeHostMissing");
            return session;
        }

        var effectiveDebugManifestPath = PrepareReplayManifest(session.DebugManifestPath);
        var arguments = BuildPersistentArguments(effectiveDebugManifestPath, localization.Locale);
        var startInfo = CreatePersistentProcessStartInfo(runtimeHostPath!, arguments, localization);
        var transport = await Task.Run(() => CopperfinRuntimeDebugTransport.Start(
            startInfo,
            timeoutMilliseconds: 30000,
            localization,
            out _));
        if (transport is null)
        {
            TryDeleteReplayManifest(session.DebugManifestPath, effectiveDebugManifestPath);
            session.Commands = new List<string> { "continue" };
            return await ReplayAsync(session, localization);
        }

        session.Transport = transport;
        session.TransportManifestPath = effectiveDebugManifestPath;
        try
        {
            return await AdvanceLiveAsync(session, "continue", localization);
        }
        catch
        {
            Stop(session);
            TryDeleteReplayManifest(session.DebugManifestPath, effectiveDebugManifestPath);
            throw;
        }
    }

    private static Task<CopperfinRuntimeDebugSession> AdvanceLiveAsync(
        CopperfinRuntimeDebugSession session,
        string command,
        CopperfinLocalization? localization = null)
    {
        localization ??= CopperfinLocalization.FromEnvironment();
        return Task.Run(() =>
        {
            var transport = session.Transport;
            if (transport is null)
            {
                return ReplayWithCommandAsync(session, command, localization).GetAwaiter().GetResult();
            }

            var response = transport.Send(command, timeoutMilliseconds: 30000);
            if (!response.Success)
            {
                session.Success = false;
                session.Error = string.IsNullOrWhiteSpace(response.Error)
                    ? localization.Text("AssetEditor.Dialog.RuntimeHostTimedOut")
                    : response.Error;
                Stop(session);
                return session;
            }

            session.Success = true;
            session.Error = string.Empty;
            session.State = response.State;
            session.Commands.Add(command);
            if (string.Equals(session.State.Reason, "completed", StringComparison.OrdinalIgnoreCase) ||
                string.Equals(session.State.Reason, "error", StringComparison.OrdinalIgnoreCase))
            {
                Stop(session);
            }

            return session;
        });
    }

    private static Task<CopperfinRuntimeDebugSession> ReplayWithCommandAsync(
        CopperfinRuntimeDebugSession session,
        string command,
        CopperfinLocalization? localization = null)
    {
        if (string.Equals(session.State.Reason, "completed", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(session.State.Reason, "error", StringComparison.OrdinalIgnoreCase))
        {
            return Task.FromResult(session);
        }

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
            Commands = commands,
            StopOnEntry = session.StopOnEntry
        }, localization);
    }

    internal static Task<CopperfinRuntimeDebugSession> ReplayAsync(
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

            var effectiveDebugManifestPath = PrepareReplayManifest(session.DebugManifestPath);
            var arguments = BuildReplayArguments(
                effectiveDebugManifestPath,
                localization.Locale,
                session.Commands,
                session.StopOnEntry);

            var startInfo = CreateReplayProcessStartInfo(runtimeHostPath!, arguments, localization);

            try
            {
                var processResult = CopperfinProcessRunner.Run(startInfo, timeoutMilliseconds: 30000);
                if (!processResult.Started)
                {
                    session.Success = false;
                    session.Error = string.IsNullOrWhiteSpace(processResult.StandardError)
                        ? localization.Text("AssetEditor.Dialog.RuntimeHostCouldNotStart")
                        : localization.Format(
                            "AssetEditor.Dialog.RuntimeHostCouldNotStartWithMessage",
                            processResult.StandardError.Trim());
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
            }
            finally
            {
                TryDeleteReplayManifest(session.DebugManifestPath, effectiveDebugManifestPath);
            }
        });
    }

    internal static ProcessStartInfo CreateReplayProcessStartInfo(
        string runtimeHostPath,
        string arguments,
        CopperfinLocalization? localization = null,
        bool? isWindowsOverride = null)
    {
        return CopperfinStudioHostBridge.CreateProcessStartInfo(
            runtimeHostPath,
            arguments,
            localization,
            redirectOutput: true,
            createNoWindow: true,
            isWindowsOverride);
    }

    internal static ProcessStartInfo CreatePersistentProcessStartInfo(
        string runtimeHostPath,
        string arguments,
        CopperfinLocalization? localization = null,
        bool? isWindowsOverride = null)
    {
        var startInfo = CreateReplayProcessStartInfo(
            runtimeHostPath,
            arguments,
            localization,
            isWindowsOverride);
        startInfo.RedirectStandardInput = true;
        startInfo.StandardOutputEncoding = Encoding.UTF8;
        startInfo.StandardErrorEncoding = Encoding.UTF8;
        return startInfo;
    }

    internal static string BuildPersistentArguments(string debugManifestPath, string locale)
    {
        return BuildReplayArguments(
                   debugManifestPath,
                   locale,
                   Array.Empty<string>(),
                   stopOnEntry: true) +
               " --debug-server";
    }

    internal static string BuildReplayArguments(
        string debugManifestPath,
        string locale,
        IEnumerable<string> commands,
        bool stopOnEntry = false)
    {
        var arguments = new StringBuilder();
        arguments.Append("--manifest ").Append(Quote(debugManifestPath)).Append(" --debug");
        arguments.Append(" --locale ").Append(Quote(locale));
        if (stopOnEntry)
        {
            arguments.Append(" --debug-stop-on-entry");
        }
        foreach (var command in commands)
        {
            arguments.Append(" --debug-command ").Append(Quote(command));
        }

        return arguments.ToString();
    }

    internal static CopperfinRuntimePauseState ParsePauseState(string stdout)
    {
        var states = new List<CopperfinRuntimePauseState>();
        var current = new CopperfinRuntimePauseState();
        var sawCommand = false;

        using var reader = new StringReader(stdout);
        string? rawLine;
        while ((rawLine = reader.ReadLine()) is not null)
        {
            // Protocol framing may be indented, but whitespace after the value separator is payload.
            var line = rawLine.TrimStart();
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
        if (IsEscapedDebugLineValueKey(key))
        {
            value = UnescapeDebugLineValue(value);
        }

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

        if (string.Equals(key, "debug.watch.expression", StringComparison.OrdinalIgnoreCase))
        {
            state.Watches.Add(new CopperfinRuntimeWatch { Expression = value });
            return;
        }

        if (string.Equals(key, "debug.watch.ok", StringComparison.OrdinalIgnoreCase))
        {
            GetCurrentWatch(state).Success = string.Equals(value, "true", StringComparison.OrdinalIgnoreCase);
            return;
        }

        if (string.Equals(key, "debug.watch.value", StringComparison.OrdinalIgnoreCase))
        {
            GetCurrentWatch(state).Value = value;
            return;
        }

        if (string.Equals(key, "debug.watch.error", StringComparison.OrdinalIgnoreCase))
        {
            GetCurrentWatch(state).Error = value;
            return;
        }

        if (key.StartsWith("debug.breakpoint[", StringComparison.OrdinalIgnoreCase))
        {
            ApplyBreakpointLine(state, key, value);
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

    private static bool IsEscapedDebugLineValueKey(string key)
    {
        return key.StartsWith("debug.global.", StringComparison.OrdinalIgnoreCase) ||
               (key.StartsWith("debug.frame[", StringComparison.OrdinalIgnoreCase) &&
                key.IndexOf("].local.", StringComparison.OrdinalIgnoreCase) >= 0) ||
               string.Equals(key, "debug.watch.expression", StringComparison.OrdinalIgnoreCase) ||
               string.Equals(key, "debug.watch.value", StringComparison.OrdinalIgnoreCase) ||
               string.Equals(key, "debug.watch.error", StringComparison.OrdinalIgnoreCase);
    }

    private static CopperfinRuntimeWatch GetCurrentWatch(CopperfinRuntimePauseState state)
    {
        if (state.Watches.Count == 0)
        {
            state.Watches.Add(new CopperfinRuntimeWatch());
        }

        return state.Watches[state.Watches.Count - 1];
    }

    private static void ApplyBreakpointLine(CopperfinRuntimePauseState state, string key, string value)
    {
        var closeIndex = key.IndexOf(']');
        if (closeIndex <= "debug.breakpoint[".Length)
        {
            return;
        }

        var breakpointIndex = ParseInt(key.Substring("debug.breakpoint[".Length, closeIndex - "debug.breakpoint[".Length));
        while (state.Breakpoints.Count <= breakpointIndex)
        {
            state.Breakpoints.Add(new CopperfinRuntimeBreakpoint());
        }

        var breakpoint = state.Breakpoints[breakpointIndex];
        var propertySeparator = key.IndexOf("].", StringComparison.Ordinal);
        if (propertySeparator < 0)
        {
            var separator = value.LastIndexOf(':');
            if (separator > 0 && int.TryParse(value.Substring(separator + 1), out var line))
            {
                breakpoint.FilePath = value.Substring(0, separator);
                breakpoint.Line = line;
            }
            else
            {
                breakpoint.FilePath = value;
            }
            return;
        }

        var property = key.Substring(propertySeparator + 2);
        if (string.Equals(property, "xasset.action_id", StringComparison.OrdinalIgnoreCase))
        {
            breakpoint.ActionId = value;
        }
        else if (string.Equals(property, "xasset.title", StringComparison.OrdinalIgnoreCase))
        {
            breakpoint.ActionTitle = value;
        }
    }

    private static string UnescapeDebugLineValue(string value)
    {
        if (value.IndexOf('\\') < 0)
        {
            return value;
        }

        var builder = new System.Text.StringBuilder(value.Length);
        for (var index = 0; index < value.Length; index++)
        {
            if (value[index] != '\\' || index + 1 >= value.Length)
            {
                builder.Append(value[index]);
                continue;
            }

            var escaped = value[++index];
            builder.Append(escaped switch
            {
                '\\' => '\\',
                'n' => '\n',
                'r' => '\r',
                't' => '\t',
                _ => '\\'
            });
            if (escaped is not ('\\' or 'n' or 'r' or 't'))
            {
                builder.Append(escaped);
            }
        }

        return builder.ToString();
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
               state.Events.Count > 0 ||
               state.Watches.Count > 0 ||
               state.Breakpoints.Count > 0;
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

    private static void TryDeleteReplayManifest(string originalDebugManifestPath, string effectiveDebugManifestPath)
    {
        if (string.IsNullOrWhiteSpace(effectiveDebugManifestPath) ||
            string.Equals(effectiveDebugManifestPath, originalDebugManifestPath, StringComparison.Ordinal))
        {
            return;
        }

        try
        {
            File.Delete(effectiveDebugManifestPath);
        }
        catch (IOException)
        {
        }
        catch (UnauthorizedAccessException)
        {
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
        return CopperfinStudioHostBridge.QuoteProcessArgument(value);
    }
}
