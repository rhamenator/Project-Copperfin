// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class StudioTerminalWindowControl : UserControl
{
    private readonly CopperfinLocalization localization;
    private readonly RichTextBox transcript;
    private readonly TextBox commandInput;
    private readonly Button sendButton;
    private readonly Label stateLabel;
    private Process? terminalProcess;
    private StreamWriter? terminalInputWriter;

    public StudioTerminalWindowControl(CopperfinLocalization localization)
    {
        this.localization = localization;
        Dock = DockStyle.Fill;
        Padding = new Padding(6);

        transcript = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            Multiline = true,
            BorderStyle = BorderStyle.FixedSingle,
            DetectUrls = false,
            BackColor = SystemColors.Window,
            ForeColor = SystemColors.WindowText,
            Font = new Font(FontFamily.GenericMonospace, 9F, FontStyle.Regular, GraphicsUnit.Point),
            Text = localization.Text("VSIX.TerminalWindow.Ready")
        };

        commandInput = new TextBox
        {
            Dock = DockStyle.Fill,
            MinimumSize = new Size(0, 24)
        };
        commandInput.KeyDown += (_, eventArgs) =>
        {
            if (eventArgs.KeyCode == Keys.Enter)
            {
                eventArgs.SuppressKeyPress = true;
                SubmitCommand();
            }
        };

        sendButton = new Button
        {
            AutoSize = false,
            Dock = DockStyle.Fill,
            Text = ">",
            AccessibleName = localization.Text("VSIX.TerminalWindow.SendTooltip")
        };
        sendButton.Click += (_, _) => SubmitCommand();
        var toolTip = new ToolTip();
        toolTip.SetToolTip(sendButton, localization.Text("VSIX.TerminalWindow.SendTooltip"));

        stateLabel = new Label
        {
            AutoSize = true,
            Dock = DockStyle.Fill,
            Text = localization.Text("VSIX.TerminalWindow.Ready"),
            TextAlign = ContentAlignment.MiddleLeft
        };

        var inputLayout = new TableLayoutPanel
        {
            ColumnCount = 3,
            RowCount = 1,
            Dock = DockStyle.Fill,
            AutoSize = true,
            Padding = new Padding(0, 6, 0, 0)
        };
        inputLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 28));
        inputLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        inputLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 32));
        inputLayout.Controls.Add(stateLabel, 0, 0);
        inputLayout.Controls.Add(commandInput, 1, 0);
        inputLayout.Controls.Add(sendButton, 2, 0);

        var layout = new TableLayoutPanel
        {
            ColumnCount = 1,
            RowCount = 2,
            Dock = DockStyle.Fill
        };
        layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 36));
        layout.Controls.Add(transcript, 0, 0);
        layout.Controls.Add(inputLayout, 0, 1);
        Controls.Add(layout);
    }

    internal bool IsShellRunning => terminalProcess is not null && !terminalProcess.HasExited;

    internal string TranscriptText => transcript.Text;

    internal void ApplyTheme(Color background, Color foreground)
    {
        BackColor = background;
        ForeColor = foreground;
        transcript.BackColor = background;
        transcript.ForeColor = foreground;
        commandInput.BackColor = background;
        commandInput.ForeColor = foreground;
        stateLabel.BackColor = background;
        stateLabel.ForeColor = foreground;
        sendButton.BackColor = background;
        sendButton.ForeColor = foreground;
    }

    internal static ProcessStartInfo CreateShellStartInfo(
        bool? isWindowsOverride = null,
        string? shellOverride = null)
    {
        var isWindows = isWindowsOverride ?? Environment.OSVersion.Platform == PlatformID.Win32NT;
        var shellPath = shellOverride;
        if (string.IsNullOrWhiteSpace(shellPath))
        {
            shellPath = Environment.GetEnvironmentVariable(isWindows ? "COMSPEC" : "SHELL");
        }

        if (string.IsNullOrWhiteSpace(shellPath))
        {
            shellPath = isWindows ? "cmd.exe" : "/bin/sh";
        }

        var startInfo = new ProcessStartInfo
        {
            FileName = shellPath,
            Arguments = isWindows ? "/Q" : "-i",
            UseShellExecute = false,
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
            WorkingDirectory = Environment.CurrentDirectory,
            StandardOutputEncoding = Encoding.UTF8,
            StandardErrorEncoding = Encoding.UTF8
        };
        return startInfo;
    }

    internal void StartShell()
    {
        if (IsShellRunning)
        {
            return;
        }

        if (terminalProcess is not null)
        {
            StopShell();
        }

        try
        {
            var process = new Process
            {
                StartInfo = CreateShellStartInfo(),
                EnableRaisingEvents = true
            };
            process.OutputDataReceived += OnOutputDataReceived;
            process.ErrorDataReceived += OnOutputDataReceived;
            process.Exited += OnProcessExited;
            if (!process.Start())
            {
                process.Dispose();
                AppendTranscript(localization.Text("VSIX.TerminalWindow.Unavailable"));
                stateLabel.Text = localization.Text("VSIX.TerminalWindow.Unavailable");
                return;
            }

            terminalProcess = process;
            terminalInputWriter = new StreamWriter(
                process.StandardInput.BaseStream,
                new UTF8Encoding(encoderShouldEmitUTF8Identifier: false))
            {
                AutoFlush = true
            };
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            stateLabel.Text = localization.Text("VSIX.TerminalWindow.Ready");
            commandInput.Focus();
        }
        catch (Exception exception) when (
            exception is InvalidOperationException ||
            exception is System.ComponentModel.Win32Exception)
        {
            AppendTranscript(localization.Text("VSIX.TerminalWindow.Unavailable"));
            stateLabel.Text = localization.Text("VSIX.TerminalWindow.Unavailable");
        }
    }

    internal void SubmitCommandForTest(string command)
    {
        commandInput.Text = command;
        SubmitCommand();
    }

    internal void StopShell()
    {
        var process = terminalProcess;
        terminalProcess = null;
        var inputWriter = terminalInputWriter;
        terminalInputWriter = null;
        if (process is null)
        {
            return;
        }

        try
        {
            inputWriter?.Close();
            if (!process.HasExited)
            {
                process.Kill();
                process.WaitForExit(1000);
            }
        }
        catch (InvalidOperationException)
        {
        }
        catch (System.ComponentModel.Win32Exception)
        {
        }
        finally
        {
            process.Dispose();
        }
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            StopShell();
        }

        base.Dispose(disposing);
    }

    private void SubmitCommand()
    {
        var command = commandInput.Text;
        if (string.IsNullOrWhiteSpace(command))
        {
            return;
        }

        StartShell();
        if (!IsShellRunning || terminalProcess is null)
        {
            return;
        }

        try
        {
            AppendTranscript($"> {command}");
            terminalInputWriter?.WriteLine(command);
            commandInput.Clear();
        }
        catch (ObjectDisposedException)
        {
            stateLabel.Text = localization.Text("VSIX.TerminalWindow.Unavailable");
        }
        catch (InvalidOperationException)
        {
            stateLabel.Text = localization.Text("VSIX.TerminalWindow.Unavailable");
        }
    }

    private void OnOutputDataReceived(object? sender, DataReceivedEventArgs eventArgs)
    {
        if (eventArgs.Data is not null)
        {
            AppendTranscript(eventArgs.Data);
        }
    }

    private void OnProcessExited(object? sender, EventArgs eventArgs)
    {
        AppendTranscript(localization.Text("VSIX.TerminalWindow.Exited"));
        UpdateState(localization.Text("VSIX.TerminalWindow.Exited"));
    }

    private void UpdateState(string text)
    {
        if (IsDisposed || Disposing)
        {
            return;
        }

        try
        {
            if (InvokeRequired)
            {
                BeginInvoke(new Action<string>(UpdateState), text);
                return;
            }

            stateLabel.Text = text;
        }
        catch (InvalidOperationException)
        {
        }
    }

    private void AppendTranscript(string text)
    {
        if (IsDisposed || Disposing)
        {
            return;
        }

        try
        {
            if (InvokeRequired)
            {
                BeginInvoke(new Action<string>(AppendTranscript), text);
                return;
            }

            transcript.AppendText(text + Environment.NewLine);
            transcript.SelectionStart = transcript.TextLength;
            transcript.ScrollToCaret();
        }
        catch (InvalidOperationException)
        {
        }
    }
}
