// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Drawing;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class StudioCommandWindowControl : UserControl
{
    private readonly CopperfinLocalization localization;
    private readonly RichTextBox transcript;
    private readonly TextBox commandInput;
    private readonly Button sendButton;
    private readonly Label stateLabel;
    private readonly Func<string, string?>? commandExecutor;

    internal StudioCommandWindowControl(
        CopperfinLocalization localization,
        Func<string, string?>? commandExecutor = null)
    {
        this.localization = localization;
        this.commandExecutor = commandExecutor;
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
            Text = localization.Text("VSIX.CommandWindow.Ready")
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
            AccessibleName = localization.Text("VSIX.CommandWindow.SendTooltip")
        };
        sendButton.Click += (_, _) => SubmitCommand();
        var toolTip = new ToolTip();
        toolTip.SetToolTip(sendButton, localization.Text("VSIX.CommandWindow.SendTooltip"));

        stateLabel = new Label
        {
            AutoSize = true,
            Dock = DockStyle.Fill,
            Text = localization.Text("VSIX.CommandWindow.Ready"),
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

    internal string TranscriptText => transcript.Text;

    internal TextBox CommandInputForTest => commandInput;

    internal void SubmitCommandForTest(string command)
    {
        commandInput.Text = command;
        SubmitCommand();
    }

    private void SubmitCommand()
    {
        var command = commandInput.Text.Trim();
        if (command.Length == 0)
        {
            return;
        }

        AppendTranscript("> " + command);
        commandInput.Clear();
        stateLabel.Text = localization.Text("VSIX.CommandWindow.Ready");

        if (commandExecutor is null)
        {
            AppendTranscript(localization.Text("VSIX.CommandWindow.Unavailable"));
            return;
        }

        try
        {
            var result = commandExecutor(command);
            if (result is not null && result.Trim().Length > 0)
            {
                AppendTranscript(result);
            }
        }
        catch (Exception)
        {
            AppendTranscript(localization.Text("VSIX.CommandWindow.Unavailable"));
            stateLabel.Text = localization.Text("VSIX.CommandWindow.Unavailable");
        }
    }

    private void AppendTranscript(string text)
    {
        if (transcript.TextLength > 0)
        {
            transcript.AppendText(Environment.NewLine);
        }

        transcript.AppendText(text);
        transcript.SelectionStart = transcript.TextLength;
        transcript.ScrollToCaret();
    }
}
