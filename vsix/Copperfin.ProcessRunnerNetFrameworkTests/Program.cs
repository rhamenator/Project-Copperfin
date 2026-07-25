// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace Copperfin.VisualStudio;

internal static class Program
{
    private const int WindowsTimeoutFixtureMilliseconds = 5000;

    [STAThread]
    private static int Main()
    {
        var root = Path.Combine(Path.GetTempPath(), "copperfin_process_runner_net472", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        var scriptPath = Path.Combine(root, "timeout.cmd");
        var childPidPath = Path.Combine(root, "child.pid");
        var grandchildPidPath = Path.Combine(root, "grandchild.pid");
        var childPid = 0;
        var grandchildPid = 0;

        try
        {
            File.WriteAllLines(
                scriptPath,
                new[]
                {
                    "@echo off",
                    "echo before-timeout-stdout",
                    "echo before-timeout-stderr 1>&2",
                    "powershell.exe -NoProfile -Command \"$grandchild = Start-Process -FilePath 'powershell.exe' -ArgumentList '-NoProfile -Command Start-Sleep -Seconds 30' -NoNewWindow -PassThru; Set-Content -LiteralPath '" + EscapePowerShellLiteral(childPidPath) + "' -Value $PID; Set-Content -LiteralPath '" + EscapePowerShellLiteral(grandchildPidPath) + "' -Value $grandchild.Id; Wait-Process -Id $grandchild.Id\"",
                    "exit /b 0"
                });

            var stopwatch = Stopwatch.StartNew();
            var result = CopperfinProcessRunner.Run(
                new ProcessStartInfo
                {
                    FileName = Environment.GetEnvironmentVariable("COMSPEC") ?? "cmd.exe",
                    Arguments = "/d /s /c \"\"" + scriptPath + "\"\"",
                    WorkingDirectory = root,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true
                },
                timeoutMilliseconds: WindowsTimeoutFixtureMilliseconds);
            stopwatch.Stop();

            Assert(result.Started, "net472 process runner should start the timeout fixture");
            Assert(result.TimedOut, "net472 process runner should report the root timeout");
            Assert(stopwatch.ElapsedMilliseconds < WindowsTimeoutFixtureMilliseconds + 5000,
                "net472 process runner should return within the bounded cleanup grace period");
            Assert(result.StandardOutput.Contains("before-timeout-stdout"),
                "net472 process runner should retain stdout produced before timeout");
            Assert(result.StandardError.Contains("before-timeout-stderr"),
                "net472 process runner should retain stderr produced before timeout");
            Assert(TryReadProcessId(childPidPath, out childPid),
                "net472 fixture should record the child PID");
            Assert(TryReadProcessId(grandchildPidPath, out grandchildPid),
                "net472 fixture should record the grandchild PID");
            Assert(WaitForProcessExit(childPid, 2000),
                "net472 taskkill fallback should terminate the child process");
            Assert(WaitForProcessExit(grandchildPid, 2000),
                "net472 taskkill fallback should terminate the grandchild process");
            TestSuccessfulExitWithHeldPipe(root);

            Console.WriteLine("Copperfin .NET Framework process-runner tests passed.");
            return 0;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex.Message);
            return 1;
        }
        finally
        {
            TryTerminateProcessTree(grandchildPid);
            TryTerminateProcessTree(childPid);
            try
            {
                Directory.Delete(root, recursive: true);
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static string EscapePowerShellLiteral(string value)
    {
        return value.Replace("'", "''");
    }

    private static void TestSuccessfulExitWithHeldPipe(string root)
    {
        var scriptPath = Path.Combine(root, "successful.cmd");
        var descendantPidPath = Path.Combine(root, "successful-descendant.pid");
        var descendantPid = 0;
        try
        {
            File.WriteAllLines(
                scriptPath,
                new[]
                {
                    "@echo off",
                    "echo successful-stdout",
                    "powershell.exe -NoProfile -Command \"$child = Start-Process -FilePath 'powershell.exe' -ArgumentList '-NoProfile -Command Start-Sleep -Seconds 30' -NoNewWindow -PassThru; Set-Content -LiteralPath '" + EscapePowerShellLiteral(descendantPidPath) + "' -Value $child.Id\"",
                    "exit /b 0"
                });

            var stopwatch = Stopwatch.StartNew();
            var result = CopperfinProcessRunner.Run(
                new ProcessStartInfo
                {
                    FileName = Environment.GetEnvironmentVariable("COMSPEC") ?? "cmd.exe",
                    Arguments = "/d /s /c \"\"" + scriptPath + "\"\"",
                    WorkingDirectory = root,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true
                },
                timeoutMilliseconds: 5000);
            stopwatch.Stop();

            Assert(result.Started, "net472 process runner should start the successful descendant fixture");
            Assert(!result.TimedOut,
                "net472 process runner should preserve successful completion while a descendant retains output handles");
            Assert(result.ExitCode == 0,
                "net472 process runner should preserve the successful root exit code");
            Assert(result.StandardOutput.Contains("successful-stdout"),
                "net472 process runner should retain output captured before successful root completion");
            Assert(stopwatch.ElapsedMilliseconds < 5000,
                "net472 process runner should bound output draining without converting success into a timeout");
            Assert(TryReadProcessId(descendantPidPath, out descendantPid),
                "net472 successful descendant fixture should record its PID");
        }
        finally
        {
            TryTerminateProcessTree(descendantPid);
        }
    }

    private static bool TryReadProcessId(string path, out int processId)
    {
        processId = 0;
        return File.Exists(path) && int.TryParse(File.ReadAllText(path).Trim(), out processId);
    }

    private static bool WaitForProcessExit(int processId, int timeoutMilliseconds)
    {
        var stopwatch = Stopwatch.StartNew();
        while (stopwatch.ElapsedMilliseconds < timeoutMilliseconds)
        {
            try
            {
                using var process = Process.GetProcessById(processId);
                if (process.HasExited)
                {
                    return true;
                }
            }
            catch (ArgumentException)
            {
                return true;
            }
            Thread.Sleep(50);
        }
        return false;
    }

    private static void TryTerminateProcessTree(int processId)
    {
        if (processId <= 0)
        {
            return;
        }

        try
        {
            var systemDirectory = Environment.GetFolderPath(Environment.SpecialFolder.System);
            if (string.IsNullOrWhiteSpace(systemDirectory))
            {
                return;
            }
            using var taskKill = Process.Start(
                new ProcessStartInfo
                {
                    FileName = Path.Combine(systemDirectory, "taskkill.exe"),
                    Arguments = "/PID " + processId + " /T /F",
                    UseShellExecute = false,
                    CreateNoWindow = true
                });
            taskKill?.WaitForExit(1000);
        }
        catch (Exception)
        {
        }
    }

    private static void Assert(bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }
}
