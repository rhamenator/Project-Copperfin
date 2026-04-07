using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class StudioMainForm : Form
{
    private readonly CopperfinAssetEditorControl editorControl;
    private readonly ToolStripStatusLabel statusLabel;

    public StudioMainForm()
    {
        Text = "Copperfin Studio";
        Width = 1480;
        Height = 980;
        StartPosition = FormStartPosition.CenterScreen;

        var menuStrip = new MenuStrip();
        var fileMenu = new ToolStripMenuItem("&File");
        var openItem = new ToolStripMenuItem("&Open...", null, (_, _) => OpenFromPicker());
        var exitItem = new ToolStripMenuItem("E&xit", null, (_, _) => Close());
        fileMenu.DropDownItems.Add(openItem);
        fileMenu.DropDownItems.Add(new ToolStripSeparator());
        fileMenu.DropDownItems.Add(exitItem);
        menuStrip.Items.Add(fileMenu);
        MainMenuStrip = menuStrip;

        editorControl = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill,
            EmbeddedStudioShell = true
        };

        var statusStrip = new StatusStrip();
        statusLabel = new ToolStripStatusLabel
        {
            Text = "Open a VFP asset to inspect and edit it in Copperfin Studio."
        };
        statusStrip.Items.Add(statusLabel);

        Controls.Add(editorControl);
        Controls.Add(statusStrip);
        Controls.Add(menuStrip);
    }

    public void OpenDocument(string path)
    {
        if (!File.Exists(path))
        {
            MessageBox.Show(this, "The selected asset does not exist.", "Copperfin Studio", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        editorControl.LoadDocument(path);
        Text = $"Copperfin Studio - {CopperfinStudioHostBridge.DescribeAssetKind(path)}";
        statusLabel.Text = path;
    }

    private void OpenFromPicker()
    {
        using var dialog = new OpenFileDialog
        {
            Title = "Open Copperfin Asset",
            Filter = "Copperfin/VFP assets|*.pjx;*.scx;*.vcx;*.frx;*.lbx;*.mnx|All files|*.*",
            Multiselect = false,
            RestoreDirectory = true
        };

        if (dialog.ShowDialog(this) == DialogResult.OK)
        {
            OpenDocument(dialog.FileName);
        }
    }
}
