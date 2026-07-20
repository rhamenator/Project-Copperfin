// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.VisualStudio.PlatformUI;
using Microsoft.VisualStudio.Shell;

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
            Font = ResolveEnvironmentFont(),
            ReadOnly = true,
            ScrollBars = RichTextBoxScrollBars.Both,
            WordWrap = false,
            Text = localization.Text("VSIX.CommandWindow.Ready")
        };

        ApplyVisualStudioTheme();
        Controls.Add(transcript);
    }

    private static Font ResolveEnvironmentFont()
    {
        try
        {
            return VsShellUtilities.GetEnvironmentFont(ServiceProvider.GlobalProvider);
        }
        catch (Exception)
        {
            return SystemFonts.MessageBoxFont;
        }
    }

    private void ApplyVisualStudioTheme()
    {
        try
        {
            BackColor = VSColorTheme.GetThemedColor(EnvironmentColors.ToolWindowBackgroundColorKey);
            ForeColor = VSColorTheme.GetThemedColor(EnvironmentColors.ToolWindowTextColorKey);
            transcript.BackColor = BackColor;
            transcript.ForeColor = ForeColor;
        }
        catch (Exception)
        {
            BackColor = SystemColors.Window;
            ForeColor = SystemColors.WindowText;
            transcript.BackColor = BackColor;
            transcript.ForeColor = ForeColor;
        }
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
