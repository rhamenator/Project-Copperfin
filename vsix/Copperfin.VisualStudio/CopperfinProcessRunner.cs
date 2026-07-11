// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinCapturedProcessResult
{
    public bool Started { get; set; }
    public bool TimedOut { get; set; }
    public int ExitCode { get; set; } = -1;
    public string StandardOutput { get; set; } = string.Empty;
    public string StandardError { get; set; } = string.Empty;
}

internal static class CopperfinProcessRunner
{
    public static CopperfinCapturedProcessResult Run(ProcessStartInfo startInfo, int? timeoutMilliseconds = null)
    {
        using var process = new Process { StartInfo = startInfo };
        try
        {
            if (!process.Start())
            {
                return new CopperfinCapturedProcessResult
                {
                    Started = false
                };
            }
        }
        catch (Exception ex)
        {
            return new CopperfinCapturedProcessResult
            {
                Started = false,
                StandardError = ex.Message
            };
        }

        var stdoutBuilder = new StringBuilder();
        var stderrBuilder = new StringBuilder();
        using var stdoutCompleted = new ManualResetEventSlim(!startInfo.RedirectStandardOutput);
        using var stderrCompleted = new ManualResetEventSlim(!startInfo.RedirectStandardError);
        using var stdoutLock = new ReaderWriterLockSlim();
        using var stderrLock = new ReaderWriterLockSlim();

        if (startInfo.RedirectStandardOutput)
        {
            process.OutputDataReceived += (_, args) =>
            {
                if (args.Data is null)
                {
                    stdoutCompleted.Set();
                    return;
                }

                stdoutLock.EnterWriteLock();
                try
                {
                    stdoutBuilder.AppendLine(args.Data);
                }
                finally
                {
                    stdoutLock.ExitWriteLock();
                }
            };
        }

        if (startInfo.RedirectStandardError)
        {
            process.ErrorDataReceived += (_, args) =>
            {
                if (args.Data is null)
                {
                    stderrCompleted.Set();
                    return;
                }

                stderrLock.EnterWriteLock();
                try
                {
                    stderrBuilder.AppendLine(args.Data);
                }
                finally
                {
                    stderrLock.ExitWriteLock();
                }
            };
        }

        if (startInfo.RedirectStandardOutput)
        {
            process.BeginOutputReadLine();
        }
        if (startInfo.RedirectStandardError)
        {
            process.BeginErrorReadLine();
        }

        var exited = timeoutMilliseconds.HasValue
            ? process.WaitForExit(timeoutMilliseconds.Value)
            : WaitForExitWithoutTimeout(process);
        if (!exited)
        {
            TryKill(process);
            process.WaitForExit();
        }
        process.WaitForExit();
        WaitForCapturedOutput(stdoutCompleted, stderrCompleted);

        return new CopperfinCapturedProcessResult
        {
            Started = true,
            TimedOut = !exited,
            ExitCode = exited ? process.ExitCode : -1,
            StandardOutput = ReadCapturedOutput(stdoutBuilder, stdoutLock),
            StandardError = ReadCapturedOutput(stderrBuilder, stderrLock)
        };
    }

    private static bool WaitForExitWithoutTimeout(Process process)
    {
        process.WaitForExit();
        return true;
    }

    private static void TryKill(Process process)
    {
        try
        {
            process.Kill();
        }
        catch (InvalidOperationException)
        {
        }
    }

    private static void WaitForCapturedOutput(
        ManualResetEventSlim stdoutCompleted,
        ManualResetEventSlim stderrCompleted)
    {
        WaitHandle.WaitAll(
            new WaitHandle[]
            {
                stdoutCompleted.WaitHandle,
                stderrCompleted.WaitHandle
            });
    }

    private static string ReadCapturedOutput(StringBuilder output, ReaderWriterLockSlim outputLock)
    {
        outputLock.EnterReadLock();
        try
        {
            return output.ToString();
        }
        finally
        {
            outputLock.ExitReadLock();
        }
    }
}
