// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
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
    private const int CleanupGraceMilliseconds = 1000;
    private const int ProcessTreeDiscoveryGraceMilliseconds = 2000;

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
        var stdoutLock = new object();
        var stderrLock = new object();

        if (startInfo.RedirectStandardOutput)
        {
            process.OutputDataReceived += (_, args) =>
            {
                if (args.Data is null)
                {
                    TrySetCompleted(stdoutCompleted);
                    return;
                }

                lock (stdoutLock)
                {
                    stdoutBuilder.AppendLine(args.Data);
                }
            };
        }

        if (startInfo.RedirectStandardError)
        {
            process.ErrorDataReceived += (_, args) =>
            {
                if (args.Data is null)
                {
                    TrySetCompleted(stderrCompleted);
                    return;
                }

                lock (stderrLock)
                {
                    stderrBuilder.AppendLine(args.Data);
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
        var timedOut = !exited;
        if (!exited)
        {
            TryKillProcessTree(process);
            if (!TryWaitForExit(process, CleanupGraceMilliseconds))
            {
                TryKill(process);
                _ = TryWaitForExit(process, CleanupGraceMilliseconds);
            }
        }
        else if (!timeoutMilliseconds.HasValue)
        {
            process.WaitForExit();
        }

        var captured = WaitForCapturedOutput(
            stdoutCompleted,
            stderrCompleted,
            timeoutMilliseconds.HasValue ? CleanupGraceMilliseconds : Timeout.Infinite);
        if (!captured)
        {
            TryCancelOutputRead(process, startInfo);
        }

        return new CopperfinCapturedProcessResult
        {
            Started = true,
            TimedOut = timedOut,
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

    private static void TryKillProcessTree(Process process)
    {
        if (TryInvokeProcessTreeKill(process))
        {
            return;
        }

        if (Environment.OSVersion.Platform == PlatformID.Win32NT)
        {
            if (!TryTaskKill(process.Id))
            {
                TryKill(process);
            }
        }
        else
        {
            TryKillPosixDescendants(process.Id);
            TryKill(process);
        }
    }

    private static bool TryInvokeProcessTreeKill(Process process)
    {
        var killTree = typeof(Process).GetMethod("Kill", new[] { typeof(bool) });
        if (killTree is null)
        {
            return false;
        }

        try
        {
            killTree.Invoke(process, new object[] { true });
            return true;
        }
        catch (Exception)
        {
            return false;
        }
    }

    private static bool TryTaskKill(int processId)
    {
        try
        {
            var systemDirectory = Environment.GetFolderPath(Environment.SpecialFolder.System);
            if (string.IsNullOrWhiteSpace(systemDirectory))
            {
                return false;
            }
            var taskKillPath = Path.Combine(systemDirectory, "taskkill.exe");
            if (!File.Exists(taskKillPath))
            {
                return false;
            }

            using var taskKill = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = taskKillPath,
                    Arguments = $"/PID {processId} /T /F",
                    UseShellExecute = false,
                    CreateNoWindow = true
                }
            };
            if (!taskKill.Start())
            {
                return false;
            }
            if (!taskKill.WaitForExit(CleanupGraceMilliseconds))
            {
                TryKill(taskKill);
                _ = TryWaitForExit(taskKill, CleanupGraceMilliseconds);
                return false;
            }
            return taskKill.ExitCode == 0;
        }
        catch (Exception)
        {
            return false;
        }
    }

    private static void TryKillPosixDescendants(int processId)
    {
        foreach (var descendantId in FindPosixDescendants(processId))
        {
            try
            {
                using var descendant = Process.GetProcessById(descendantId);
                descendant.Kill();
            }
            catch (Exception)
            {
            }
        }
    }

    private static IReadOnlyList<int> FindPosixDescendants(int rootProcessId)
    {
        var children = new Dictionary<int, List<int>>();
        var output = new StringBuilder();
        var outputLock = new object();
        using var outputCompleted = new ManualResetEventSlim(false);
        try
        {
            using var ps = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "ps",
                    Arguments = "-eo pid=,ppid=",
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    CreateNoWindow = true
                }
            };
            ps.OutputDataReceived += (_, args) =>
            {
                if (args.Data is null)
                {
                    TrySetCompleted(outputCompleted);
                    return;
                }

                lock (outputLock)
                {
                    output.AppendLine(args.Data);
                }
            };
            if (!ps.Start())
            {
                return Array.Empty<int>();
            }

            ps.BeginOutputReadLine();
            var snapshotStopwatch = Stopwatch.StartNew();
            if (!ps.WaitForExit(ProcessTreeDiscoveryGraceMilliseconds))
            {
                TryKill(ps);
                _ = TryWaitForExit(ps, CleanupGraceMilliseconds);
                TryCancelOutputRead(ps, ps.StartInfo);
                return Array.Empty<int>();
            }
            var outputWaitMilliseconds = Math.Max(
                0,
                ProcessTreeDiscoveryGraceMilliseconds - (int)snapshotStopwatch.ElapsedMilliseconds);
            if (!outputCompleted.Wait(outputWaitMilliseconds))
            {
                TryCancelOutputRead(ps, ps.StartInfo);
                return Array.Empty<int>();
            }

            foreach (var line in ReadCapturedOutput(output, outputLock).Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries))
            {
                var fields = line.Split(new[] { ' ', '\t' }, StringSplitOptions.RemoveEmptyEntries);
                if (fields.Length != 2 ||
                    !int.TryParse(fields[0], out var childId) ||
                    !int.TryParse(fields[1], out var parentId))
                {
                    continue;
                }
                if (!children.TryGetValue(parentId, out var childIds))
                {
                    childIds = new List<int>();
                    children[parentId] = childIds;
                }
                childIds.Add(childId);
            }
        }
        catch (Exception)
        {
            return Array.Empty<int>();
        }

        var descendants = new List<int>();
        CollectDescendants(rootProcessId, children, descendants, new HashSet<int>());
        descendants.Reverse();
        return descendants;
    }

    private static void CollectDescendants(
        int processId,
        IReadOnlyDictionary<int, List<int>> children,
        ICollection<int> descendants,
        ISet<int> visited)
    {
        if (!visited.Add(processId) || !children.TryGetValue(processId, out var childIds))
        {
            return;
        }
        foreach (var childId in childIds)
        {
            descendants.Add(childId);
            CollectDescendants(childId, children, descendants, visited);
        }
    }

    private static bool TryWaitForExit(Process process, int timeoutMilliseconds)
    {
        try
        {
            return process.WaitForExit(timeoutMilliseconds);
        }
        catch (InvalidOperationException)
        {
            return true;
        }
    }

    private static bool WaitForCapturedOutput(
        ManualResetEventSlim stdoutCompleted,
        ManualResetEventSlim stderrCompleted,
        int timeoutMilliseconds)
    {
        if (timeoutMilliseconds == Timeout.Infinite)
        {
            stdoutCompleted.Wait();
            stderrCompleted.Wait();
            return true;
        }

        var stopwatch = Stopwatch.StartNew();
        if (!stdoutCompleted.Wait(timeoutMilliseconds))
        {
            return false;
        }
        var remainingMilliseconds = Math.Max(
            0,
            timeoutMilliseconds - (int)stopwatch.ElapsedMilliseconds);
        return stderrCompleted.Wait(remainingMilliseconds);
    }

    private static void TryCancelOutputRead(Process process, ProcessStartInfo startInfo)
    {
        try
        {
            if (startInfo.RedirectStandardOutput)
            {
                process.CancelOutputRead();
            }
        }
        catch (InvalidOperationException)
        {
        }
        try
        {
            if (startInfo.RedirectStandardError)
            {
                process.CancelErrorRead();
            }
        }
        catch (InvalidOperationException)
        {
        }
    }

    private static void TrySetCompleted(ManualResetEventSlim completed)
    {
        try
        {
            completed.Set();
        }
        catch (ObjectDisposedException)
        {
        }
    }

    private static string ReadCapturedOutput(StringBuilder output, object outputLock)
    {
        lock (outputLock)
        {
            return output.ToString();
        }
    }
}
