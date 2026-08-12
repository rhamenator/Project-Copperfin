// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void SmokeStandaloneStudioTerminalShellContract()
    {
        var posix = StudioTerminalWindowControl.CreateShellStartInfo(
            isWindowsOverride: false,
            shellOverride: "/custom/posix-shell");
        Expect(posix.FileName == "/custom/posix-shell" && posix.Arguments == "-i",
            "standalone Terminal should use the POSIX shell contract when requested");
        Expect(posix.RedirectStandardInput && posix.RedirectStandardOutput && posix.RedirectStandardError,
            "standalone Terminal should redirect all interactive shell streams");

        var windows = StudioTerminalWindowControl.CreateShellStartInfo(
            isWindowsOverride: true,
            shellOverride: "C:\\Windows\\System32\\cmd.exe");
        Expect(windows.FileName == "C:\\Windows\\System32\\cmd.exe" && windows.Arguments == "/Q",
            "standalone Terminal should use the Windows command-shell contract when requested");
    }

    private static void SmokeStandaloneStudioTerminalWindow()
    {
        using var form = new StudioMainForm(
            new CopperfinLocalization("es-419"),
            new InMemoryStudioShellLayoutStore())
        {
            Width = 1200,
            Height = 800,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        form.Show();
        Application.DoEvents();

        Expect(form.IsTerminalWindowVisible,
            "standalone Studio should show the docked Terminal window by default");
        Expect(form.TerminalWindowTabTitle == "Terminal de Copperfin",
            "standalone Terminal should use the active locale catalog");

        form.SelectTerminalWindow();
        form.StartTerminalShell();
        var shellStarted = WaitUntil(
            TimeSpan.FromSeconds(5),
            () => form.IsTerminalShellRunning);
        Expect(shellStarted,
            "standalone Terminal should start the platform shell when selected");

        if (shellStarted)
        {
            form.SubmitTerminalCommandForTest("printf 'CopperfinTerminalProbe\\n'");
            Expect(
                WaitUntil(TimeSpan.FromSeconds(5),
                    () => form.TerminalTranscript.IndexOf("CopperfinTerminalProbe", StringComparison.Ordinal) >= 0),
                "standalone Terminal should stream command output into its transcript");
        }

        form.SetTerminalWindowVisible(false);
        Application.DoEvents();
        Expect(!form.IsTerminalWindowVisible,
            "standalone Terminal should be hideable independently of Command");

        form.SetTerminalWindowVisible(true);
        Application.DoEvents();
        Expect(form.IsTerminalWindowVisible,
            "standalone Terminal should be restorable as a docked tab");

        TearDownForm(form);
        Expect(!form.IsTerminalShellRunning,
            "standalone Terminal should stop its child shell when the form is disposed");
    }

    private static void SmokeStandaloneStudioTerminalCallbackTeardown()
    {
        using var form = new Form
        {
            Width = 640,
            Height = 480,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };
        using var terminal = new StudioTerminalWindowControl(
            new CopperfinLocalization("en-US"))
        {
            Dock = DockStyle.Fill
        };
        form.Controls.Add(terminal);
        form.Show();
        Application.DoEvents();

        using var marshalEntered = new ManualResetEventSlim(false);
        using var allowMarshal = new ManualResetEventSlim(false);
        Exception? callbackFailure = null;
        terminal.BeforeCallbackMarshalForTest = () =>
        {
            marshalEntered.Set();
            allowMarshal.Wait(TimeSpan.FromSeconds(5));
        };

        var callbackThread = new Thread(() =>
        {
            try
            {
                terminal.AppendTranscriptForTest("pending terminal output");
            }
            catch (Exception exception)
            {
                callbackFailure = exception;
            }
        })
        {
            IsBackground = true
        };
        callbackThread.Start();
        Expect(marshalEntered.Wait(TimeSpan.FromSeconds(5)),
            "standalone Terminal teardown smoke should reach the background callback marshal boundary");

        var releaseThread = new Thread(() =>
        {
            Thread.Sleep(50);
            allowMarshal.Set();
        })
        {
            IsBackground = true
        };
        releaseThread.Start();
        terminal.Dispose();
        Expect(callbackThread.Join(TimeSpan.FromSeconds(5)) && releaseThread.Join(TimeSpan.FromSeconds(5)),
            "standalone Terminal teardown should drain the in-flight callback without deadlock");
        Application.DoEvents();
        Expect(callbackFailure is null,
            "standalone Terminal teardown should not marshal output through a disposed WinForms handle");
    }
}
