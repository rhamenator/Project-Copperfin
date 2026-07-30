// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Threading;

namespace Copperfin.VisualStudio;

internal static class Program
{
    private const string HoldOutputHandlesArgument = "--hold-output-handles";
    private const string HoldOutputTreeArgument = "--hold-output-tree";
    private const string StartOutputHolderArgument = "--start-output-holder";
    private const int HelperLifetimeMilliseconds = 30000;
    private const int WindowsTimeoutFixtureMilliseconds = 10000;

    [STAThread]
    private static int Main(string[] args)
    {
        if (args.Length == 1 && string.Equals(args[0], HoldOutputHandlesArgument, StringComparison.Ordinal))
        {
            Thread.Sleep(HelperLifetimeMilliseconds);
            return 0;
        }

        if (args.Length == 3 && string.Equals(args[0], HoldOutputTreeArgument, StringComparison.Ordinal))
        {
            Console.WriteLine("before-timeout-stdout");
            Console.Error.WriteLine("before-timeout-stderr");
            File.WriteAllText(args[1], Process.GetCurrentProcess().Id.ToString());
            using var grandchild = Process.Start(CreateSelfStartInfo(HoldOutputHandlesArgument));
            if (grandchild is null)
            {
                return 2;
            }
            File.WriteAllText(args[2], grandchild.Id.ToString());
            grandchild.WaitForExit();
            return grandchild.ExitCode;
        }

        if (args.Length == 2 && string.Equals(args[0], StartOutputHolderArgument, StringComparison.Ordinal))
        {
            Console.WriteLine("successful-stdout");
            using var descendant = Process.Start(CreateSelfStartInfo(HoldOutputHandlesArgument));
            if (descendant is null)
            {
                return 2;
            }
            File.WriteAllText(args[1], descendant.Id.ToString());
            return 0;
        }

        if (Environment.OSVersion.Platform != PlatformID.Win32NT)
        {
            Console.WriteLine("Copperfin .NET Framework process-runner tests skipped: the fixture requires Windows process-tree and taskkill.exe behavior.");
            return 0;
        }

        var root = Path.Combine(Path.GetTempPath(), "copperfin_process_runner_net472", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        var childPidPath = Path.Combine(root, "child.pid");
        var grandchildPidPath = Path.Combine(root, "grandchild.pid");
        var childPid = 0;
        var grandchildPid = 0;

        try
        {
            var stopwatch = Stopwatch.StartNew();
            var result = CopperfinProcessRunner.Run(
                CreateSelfStartInfo(HoldOutputTreeArgument, childPidPath, grandchildPidPath, root),
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

    private static void TestSuccessfulExitWithHeldPipe(string root)
    {
        var descendantPidPath = Path.Combine(root, "successful-descendant.pid");
        var descendantPid = 0;
        try
        {
            var stopwatch = Stopwatch.StartNew();
            var result = CopperfinProcessRunner.Run(
                CreateSelfStartInfo(
                    StartOutputHolderArgument,
                    descendantPidPath,
                    workingDirectory: root),
                timeoutMilliseconds: WindowsTimeoutFixtureMilliseconds);
            stopwatch.Stop();

            Assert(result.Started, "net472 process runner should start the successful descendant fixture");
            Assert(!result.TimedOut,
                "net472 process runner should preserve successful completion while a descendant retains output handles");
            Assert(TryReadProcessId(descendantPidPath, out descendantPid),
                "net472 successful descendant fixture should record its PID");
            Assert(result.ExitCode == 0,
                "net472 process runner should preserve the successful root exit code (actual=" + result.ExitCode + ")");
            Assert(result.StandardOutput.Contains("successful-stdout"),
                "net472 process runner should retain output captured before successful root completion");
            Assert(stopwatch.ElapsedMilliseconds < WindowsTimeoutFixtureMilliseconds,
                "net472 process runner should bound output draining without converting success into a timeout");
            Assert(!WaitForProcessExit(descendantPid, 250),
                "net472 successful descendant should still hold inherited output handles after the root exits");
        }
        finally
        {
            TryTerminateProcessTree(descendantPid);
        }
    }

    private static ProcessStartInfo CreateSelfStartInfo(
        string mode,
        string? firstPath = null,
        string? secondPath = null,
        string? workingDirectory = null)
    {
        var executablePath = Assembly.GetExecutingAssembly().Location;
        var arguments = QuoteArgument(mode);
        if (firstPath is { Length: > 0 })
        {
            arguments += " " + QuoteArgument(firstPath);
        }
        if (secondPath is { Length: > 0 })
        {
            arguments += " " + QuoteArgument(secondPath);
        }

        return new ProcessStartInfo
        {
            FileName = executablePath,
            Arguments = arguments,
            WorkingDirectory = workingDirectory ?? Path.GetDirectoryName(executablePath) ?? Environment.CurrentDirectory,
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardOutput = workingDirectory is not null,
            RedirectStandardError = workingDirectory is not null
        };
    }

    private static string QuoteArgument(string value)
    {
        return "\"" + value.Replace("\"", "\\\"") + "\"";
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
