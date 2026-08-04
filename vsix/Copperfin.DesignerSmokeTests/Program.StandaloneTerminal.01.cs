// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Drawing;
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
}
