// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Drawing;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinCommandWindowControl : UserControl
{
    private readonly RichTextBox transcript;

    public CopperfinCommandWindowControl(CopperfinLocalization localization)
    {
        Dock = DockStyle.Fill;
        Margin = Padding.Empty;

        transcript = new RichTextBox
        {
            BorderStyle = BorderStyle.None,
            DetectUrls = true,
            Dock = DockStyle.Fill,
            Font = SystemFonts.MessageBoxFont,
            ReadOnly = true,
            ScrollBars = RichTextBoxScrollBars.Both,
            WordWrap = false,
            BackColor = SystemColors.Window,
            ForeColor = SystemColors.WindowText,
            Text = localization.Text("VSIX.CommandWindow.Ready")
        };

        Controls.Add(transcript);
    }

    public void AppendLine(string message)
    {
        if (string.IsNullOrEmpty(message))
        {
            return;
        }

        if (transcript.TextLength > 0)
        {
            transcript.AppendText(Environment.NewLine);
        }

        transcript.AppendText(message);
        transcript.SelectionStart = transcript.TextLength;
        transcript.ScrollToCaret();
    }
}
