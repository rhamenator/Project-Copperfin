// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System.Drawing;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class StudioCommandWindowControl : UserControl
{
    public StudioCommandWindowControl(CopperfinLocalization localization)
    {
        Dock = DockStyle.Fill;
        Padding = new Padding(6);

        var transcript = new RichTextBox
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

        Controls.Add(transcript);
    }
}
