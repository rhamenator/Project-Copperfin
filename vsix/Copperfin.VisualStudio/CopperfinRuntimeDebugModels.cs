// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Threading;

// The transport reads are bounded and run off the UI thread; the synchronous wait keeps net472 compatibility.
#pragma warning disable VSTHRD002

namespace Copperfin.VisualStudio;

internal enum CopperfinRuntimeDebugTransportFailure
{
    ProcessDidNotStart,
    ProcessExited,
    ProcessTimedOut,
    ProcessClosedOutput
}

internal sealed class CopperfinRuntimeDebugSession
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public string ManifestPath { get; set; } = string.Empty;
    public string DebugManifestPath { get; set; } = string.Empty;
    public int BuildWarningCount { get; set; }
    public List<string> BuildWarnings { get; set; } = new();
    public List<string> Commands { get; set; } = new();
    public bool StopOnEntry { get; set; }
    public CopperfinRuntimePauseState State { get; set; } = new();
    internal CopperfinRuntimeDebugTransport? Transport { get; set; }
    internal string TransportManifestPath { get; set; } = string.Empty;
}

internal sealed class CopperfinRuntimePauseState
{
    public string Reason { get; set; } = string.Empty;
    public string Location { get; set; } = string.Empty;
    public string Statement { get; set; } = string.Empty;
    public string Message { get; set; } = string.Empty;
    public int StackDepth { get; set; }
    public int ExecutedStatements { get; set; }
    public List<CopperfinRuntimeStackFrame> Frames { get; set; } = new();
    public List<CopperfinRuntimeNamedValue> Globals { get; set; } = new();
    public List<CopperfinRuntimeEvent> Events { get; set; } = new();
    public List<CopperfinRuntimeWatch> Watches { get; set; } = new();
}

internal sealed class CopperfinRuntimeStackFrame
{
    public string RoutineName { get; set; } = string.Empty;
    public string Location { get; set; } = string.Empty;
    public List<CopperfinRuntimeNamedValue> Locals { get; set; } = new();
}

internal sealed class CopperfinRuntimeNamedValue
{
    public string Name { get; set; } = string.Empty;
    public string Value { get; set; } = string.Empty;
}

internal sealed class CopperfinRuntimeEvent
{
    public string Category { get; set; } = string.Empty;
    public string Detail { get; set; } = string.Empty;
    public string Location { get; set; } = string.Empty;
}

internal sealed class CopperfinRuntimeWatch
{
    public string Expression { get; set; } = string.Empty;
    public bool Success { get; set; }
    public string Value { get; set; } = string.Empty;
    public string Error { get; set; } = string.Empty;
}

internal sealed class CopperfinRuntimeTransportResponse
{
    public bool Success { get; set; }
    public bool ExitRequested { get; set; }
    public string Error { get; set; } = string.Empty;
    public CopperfinRuntimePauseState State { get; set; } = new();
}

internal sealed class CopperfinRuntimeDebugTransport : IDisposable
{
    private readonly CopperfinLocalization localization;
    private readonly Process process;
    private readonly StreamReader output;
    private readonly StreamWriter input;
    private readonly StringBuilder standardError = new();
    private readonly object standardErrorLock = new();
    private readonly SemaphoreSlim commandGate = new(1, 1);
    private bool disposed;

    private CopperfinRuntimeDebugTransport(Process process, CopperfinLocalization localization)
    {
        this.localization = localization;
        this.process = process;
        output = process.StandardOutput;
        input = new StreamWriter(process.StandardInput.BaseStream, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
        process.ErrorDataReceived += (_, args) =>
        {
            if (args.Data is null)
            {
                return;
            }

            lock (standardErrorLock)
            {
                standardError.AppendLine(args.Data);
            }
        };
        process.BeginErrorReadLine();
    }

    public static CopperfinRuntimeDebugTransport? Start(
        ProcessStartInfo startInfo,
        int timeoutMilliseconds,
        CopperfinLocalization localization,
        out string error)
    {
        error = string.Empty;
        Process? process = null;
        try
        {
            process = new Process { StartInfo = startInfo };
            if (!process.Start())
            {
                error = TransportFailureMessage(
                    localization,
                    CopperfinRuntimeDebugTransportFailure.ProcessDidNotStart);
                process.Dispose();
                return null;
            }

            var transport = new CopperfinRuntimeDebugTransport(process, localization);
            if (!transport.WaitForReady(timeoutMilliseconds))
            {
                error = transport.ReadStandardError();
                transport.DisposeWithoutHandshake();
                return null;
            }

            return transport;
        }
        catch (Exception ex)
        {
            error = ex.Message;
            try
            {
                process?.Kill();
            }
            catch (Exception)
            {
            }
            process?.Dispose();
            return null;
        }
    }

    public CopperfinRuntimeTransportResponse Send(string command, int timeoutMilliseconds)
    {
        commandGate.Wait();
        try
        {
            if (disposed || process.HasExited)
            {
                return Failure(TransportFailureMessage(
                    localization,
                    CopperfinRuntimeDebugTransportFailure.ProcessExited));
            }

            input.WriteLine(command);
            input.Flush();

            var responseLines = new StringBuilder();
            var sawBegin = false;
            while (true)
            {
                var lineTask = output.ReadLineAsync();
                if (!lineTask.Wait(timeoutMilliseconds))
                {
                    return Failure(TransportFailureMessage(
                        localization,
                        CopperfinRuntimeDebugTransportFailure.ProcessTimedOut));
                }

                var line = lineTask.Result;
                if (line is null)
                {
                    return Failure(string.IsNullOrWhiteSpace(ReadStandardError())
                        ? TransportFailureMessage(
                            localization,
                            CopperfinRuntimeDebugTransportFailure.ProcessClosedOutput)
                        : ReadStandardError());
                }

                if (string.Equals(line.Trim(), "debug.response.begin", StringComparison.OrdinalIgnoreCase))
                {
                    sawBegin = true;
                    continue;
                }

                if (sawBegin && string.Equals(line.Trim(), "debug.response.end", StringComparison.OrdinalIgnoreCase))
                {
                    var outputText = responseLines.ToString();
                    var state = CopperfinRuntimeDebugClient.ParsePauseState(outputText);
                    var hasError = outputText.IndexOf("debug.response.error: true", StringComparison.OrdinalIgnoreCase) >= 0 ||
                                   outputText.IndexOf("status: error", StringComparison.OrdinalIgnoreCase) >= 0;
                    var exitRequested = outputText.IndexOf("debug.exit: true", StringComparison.OrdinalIgnoreCase) >= 0;
                    return new CopperfinRuntimeTransportResponse
                    {
                        Success = !hasError,
                        ExitRequested = exitRequested,
                        Error = hasError ? ReadResponseError(outputText) : string.Empty,
                        State = state
                    };
                }

                if (sawBegin)
                {
                    responseLines.AppendLine(line);
                }
            }
        }
        catch (Exception ex)
        {
            return Failure(ex.Message);
        }
        finally
        {
            commandGate.Release();
        }
    }

    public void Dispose()
    {
        if (disposed)
        {
            return;
        }

        disposed = true;
        try
        {
            if (!process.HasExited)
            {
                input.WriteLine("exit");
                input.Flush();
                _ = process.WaitForExit(2000);
            }
        }
        catch (Exception)
        {
        }

        DisposeWithoutHandshake();
    }

    private bool WaitForReady(int timeoutMilliseconds)
    {
        var deadline = DateTime.UtcNow.AddMilliseconds(timeoutMilliseconds);
        var protocolIsSupported = false;
        while (DateTime.UtcNow < deadline)
        {
            var remaining = Math.Max(1, (int)(deadline - DateTime.UtcNow).TotalMilliseconds);
            var lineTask = output.ReadLineAsync();
            if (!lineTask.Wait(remaining))
            {
                return false;
            }

            var line = lineTask.Result;
            if (line is null)
            {
                return false;
            }

            if (string.Equals(line.Trim(), "debug.server.protocol: 1", StringComparison.OrdinalIgnoreCase))
            {
                protocolIsSupported = true;
                continue;
            }

            if (protocolIsSupported &&
                string.Equals(line.Trim(), "debug.server.ready: true", StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }
        }

        return false;
    }

    private string ReadStandardError()
    {
        lock (standardErrorLock)
        {
            return standardError.ToString().Trim();
        }
    }

    private string ReadResponseError(string responseText)
    {
        return ResolveResponseError(responseText, ReadStandardError(), localization);
    }

    internal static string ResolveResponseError(
        string responseText,
        string standardErrorText,
        CopperfinLocalization localization)
    {
        if (!string.IsNullOrWhiteSpace(standardErrorText))
        {
            return standardErrorText;
        }

        using var reader = new StringReader(responseText);
        var errorPrefix = localization.Text("RuntimeHost.Prefix.Error");
        var invariantErrorPrefix = "error: ";
        string? line;
        while ((line = reader.ReadLine()) is not null)
        {
            if (!string.IsNullOrEmpty(errorPrefix) &&
                line.StartsWith(errorPrefix, StringComparison.OrdinalIgnoreCase))
            {
                return line.Substring(errorPrefix.Length).Trim();
            }

            // Locale propagation can be unavailable for a host started outside Copperfin.
            if (!string.Equals(errorPrefix, invariantErrorPrefix, StringComparison.Ordinal) &&
                line.StartsWith(invariantErrorPrefix, StringComparison.OrdinalIgnoreCase))
            {
                return line.Substring(invariantErrorPrefix.Length).Trim();
            }
        }

        return localization.Text("AssetEditor.Debugger.CommandFailed");
    }

    private static CopperfinRuntimeTransportResponse Failure(string error)
    {
        return new CopperfinRuntimeTransportResponse
        {
            Success = false,
            Error = error
        };
    }

    internal static string TransportFailureMessage(
        CopperfinLocalization localization,
        CopperfinRuntimeDebugTransportFailure failure)
    {
        var key = failure switch
        {
            CopperfinRuntimeDebugTransportFailure.ProcessDidNotStart =>
                "AssetEditor.Debugger.Transport.ProcessDidNotStart",
            CopperfinRuntimeDebugTransportFailure.ProcessExited =>
                "AssetEditor.Debugger.Transport.ProcessExited",
            CopperfinRuntimeDebugTransportFailure.ProcessTimedOut =>
                "AssetEditor.Debugger.Transport.ProcessTimedOut",
            CopperfinRuntimeDebugTransportFailure.ProcessClosedOutput =>
                "AssetEditor.Debugger.Transport.ProcessClosedOutput",
            _ => "AssetEditor.Debugger.CommandFailed"
        };
        return localization.Text(key);
    }

    private void DisposeWithoutHandshake()
    {
        try
        {
            if (!process.HasExited)
            {
                process.Kill();
                _ = process.WaitForExit(2000);
            }
        }
        catch (Exception)
        {
        }

        process.Dispose();
        input.Dispose();
        output.Dispose();
        commandGate.Dispose();
    }
}
