// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class StudioMainForm : Form
{
    private readonly TabControl documentTabs;
    private readonly ToolStripStatusLabel statusLabel;
    private readonly CopperfinLocalization localization;
    private readonly Dictionary<string, TabPage> openDocuments =
        new(CopperfinDocumentPathIdentity.CreateComparer());

    public StudioMainForm(CopperfinLocalization? localization = null)
    {
        this.localization = localization ?? CopperfinLocalization.FromEnvironment();

        Text = this.localization.Text("Studio.AppTitle");
        Width = 1480;
        Height = 980;
        StartPosition = FormStartPosition.CenterScreen;

        var menuStrip = new MenuStrip();
        var fileMenu = new ToolStripMenuItem(this.localization.Text("Studio.FileMenu"));
        var openItem = new ToolStripMenuItem(this.localization.Text("Studio.OpenMenu"), null, (_, _) => OpenFromPicker());
        var exitItem = new ToolStripMenuItem(this.localization.Text("Studio.ExitMenu"), null, (_, _) => Close());
        fileMenu.DropDownItems.Add(openItem);
        fileMenu.DropDownItems.Add(new ToolStripSeparator());
        fileMenu.DropDownItems.Add(exitItem);
        menuStrip.Items.Add(fileMenu);
        MainMenuStrip = menuStrip;

        documentTabs = new TabControl
        {
            Dock = DockStyle.Fill,
            Alignment = TabAlignment.Top,
            Multiline = true
        };
        documentTabs.SelectedIndexChanged += (_, _) =>
        {
            if (documentTabs.SelectedTab is null)
            {
                UpdateStatus(this.localization.Text("Studio.EmptyDocumentStatus"));
                return;
            }

            UpdateStatus(documentTabs.SelectedTab.ToolTipText ?? documentTabs.SelectedTab.Text);
        };

        var statusStrip = new StatusStrip();
        statusLabel = new ToolStripStatusLabel
        {
            Text = this.localization.Text("Studio.InitialStatus")
        };
        statusStrip.Items.Add(statusLabel);

        Controls.Add(documentTabs);
        Controls.Add(statusStrip);
        Controls.Add(menuStrip);

        UpdateStatus(this.localization.Text("Studio.EmptyDocumentStatus"));
    }

    public void OpenDocument(string path)
    {
        if (!File.Exists(path))
        {
            MessageBox.Show(
                this,
                localization.Text("Studio.MissingAssetMessage"),
                localization.Text("Studio.AppTitle"),
                MessageBoxButtons.OK,
                MessageBoxIcon.Warning);
            return;
        }

        var normalizedPath = CopperfinDocumentPathIdentity.Normalize(path);
        if (openDocuments.TryGetValue(normalizedPath, out var existingPage))
        {
            documentTabs.SelectedTab = existingPage;
            UpdateStatus(existingPage.Text);
            return;
        }

        var editorControl = new CopperfinAssetEditorControl(localization)
        {
            Dock = DockStyle.Fill,
            EmbeddedStudioShell = true
        };

        var page = new TabPage(Path.GetFileName(normalizedPath))
        {
            ToolTipText = normalizedPath
        };
        page.Controls.Add(editorControl);
        documentTabs.TabPages.Add(page);
        documentTabs.SelectedTab = page;
        openDocuments[normalizedPath] = page;
        editorControl.LoadDocument(normalizedPath);

        var assetKind = CopperfinStudioHostBridge.DescribeAssetKind(normalizedPath, localization);
        Text = localization.Format("Studio.WindowTitleWithAssetKind", assetKind);
        UpdateStatus(localization.Format(
            "Studio.OpenDocumentStatus",
            normalizedPath,
            assetKind,
            documentTabs.TabPages.Count));
    }

    private void OpenFromPicker()
    {
        using var dialog = new OpenFileDialog
        {
            Title = localization.Text("Studio.OpenDialogTitle"),
            Filter = localization.Text("Studio.OpenDialogFilter"),
            Multiselect = false,
            RestoreDirectory = true
        };

        if (dialog.ShowDialog(this) == DialogResult.OK)
        {
            OpenDocument(dialog.FileName);
        }
    }

    private void UpdateStatus(string text)
    {
        statusLabel.Text = text;
    }
}
