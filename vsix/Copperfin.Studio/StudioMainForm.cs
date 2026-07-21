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
    private readonly SplitContainer shellSplitContainer;
    private readonly TabControl toolWindowTabs;
    private readonly TabPage commandWindowPage;
    private readonly TabPage terminalWindowPage;
    private readonly StudioTerminalWindowControl terminalWindowControl;
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

        var viewMenu = new ToolStripMenuItem(this.localization.Text("Studio.ViewMenu"));
        var commandWindowItem = new ToolStripMenuItem(this.localization.Text("Studio.CommandWindowMenu"))
        {
            CheckOnClick = true,
            Checked = true
        };
        commandWindowItem.CheckedChanged += (_, _) => SetCommandWindowVisible(commandWindowItem.Checked);
        var terminalWindowItem = new ToolStripMenuItem(this.localization.Text("Studio.TerminalWindowMenu"))
        {
            CheckOnClick = true,
            Checked = true
        };
        terminalWindowItem.CheckedChanged += (_, _) => SetTerminalWindowVisible(terminalWindowItem.Checked);
        viewMenu.DropDownItems.Add(commandWindowItem);
        viewMenu.DropDownItems.Add(terminalWindowItem);
        menuStrip.Items.Add(viewMenu);
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

        toolWindowTabs = new TabControl
        {
            Dock = DockStyle.Fill,
            Alignment = TabAlignment.Top,
            Multiline = false
        };
        commandWindowPage = new TabPage(this.localization.Text("VSIX.CommandWindow.Title"));
        commandWindowPage.Controls.Add(new StudioCommandWindowControl(this.localization));
        toolWindowTabs.TabPages.Add(commandWindowPage);
        terminalWindowPage = new TabPage(this.localization.Text("VSIX.TerminalWindow.Title"));
        terminalWindowControl = new StudioTerminalWindowControl(this.localization);
        terminalWindowPage.Controls.Add(terminalWindowControl);
        toolWindowTabs.TabPages.Add(terminalWindowPage);
        toolWindowTabs.SelectedIndexChanged += (_, _) =>
        {
            if (toolWindowTabs.SelectedTab == terminalWindowPage)
            {
                terminalWindowControl.StartShell();
            }
        };

        shellSplitContainer = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Horizontal,
            FixedPanel = FixedPanel.Panel2,
            Panel2MinSize = 160,
            SplitterDistance = 720,
            IsSplitterFixed = false
        };
        shellSplitContainer.Panel1.Controls.Add(documentTabs);
        shellSplitContainer.Panel2.Controls.Add(toolWindowTabs);

        var statusStrip = new StatusStrip();
        statusLabel = new ToolStripStatusLabel
        {
            Text = this.localization.Text("Studio.InitialStatus")
        };
        statusStrip.Items.Add(statusLabel);

        Controls.Add(shellSplitContainer);
        Controls.Add(statusStrip);
        Controls.Add(menuStrip);

        UpdateStatus(this.localization.Text("Studio.EmptyDocumentStatus"));
    }

    internal bool IsCommandWindowVisible => toolWindowTabs.TabPages.Contains(commandWindowPage);

    internal bool IsTerminalWindowVisible => toolWindowTabs.TabPages.Contains(terminalWindowPage);

    internal string CommandWindowTabTitle => commandWindowPage.Text;

    internal string TerminalWindowTabTitle => terminalWindowPage.Text;

    internal bool IsTerminalShellRunning => terminalWindowControl.IsShellRunning;

    internal string TerminalTranscript => terminalWindowControl.TranscriptText;

    internal void SetCommandWindowVisible(bool visible)
    {
        SetToolWindowVisible(commandWindowPage, visible);
    }

    internal void SetTerminalWindowVisible(bool visible)
    {
        SetToolWindowVisible(terminalWindowPage, visible);
    }

    internal void SelectTerminalWindow()
    {
        if (IsTerminalWindowVisible)
        {
            toolWindowTabs.SelectedTab = terminalWindowPage;
        }
    }

    internal void StartTerminalShell()
    {
        terminalWindowControl.StartShell();
    }

    internal void SubmitTerminalCommandForTest(string command)
    {
        terminalWindowControl.SubmitCommandForTest(command);
    }

    private void SetToolWindowVisible(TabPage page, bool visible)
    {
        var isVisible = toolWindowTabs.TabPages.Contains(page);
        if (visible && !isVisible)
        {
            toolWindowTabs.TabPages.Add(page);
        }
        else if (!visible && isVisible)
        {
            toolWindowTabs.TabPages.Remove(page);
        }

        shellSplitContainer.Panel2Collapsed = toolWindowTabs.TabPages.Count == 0;
    }

    public void OpenDocument(string path, string? objectName = null, string? uniqueId = null)
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
            if (objectName is not null || uniqueId is not null)
            {
                var existingEditor = existingPage.Controls
                    .OfType<CopperfinAssetEditorControl>()
                    .SingleOrDefault();
                existingEditor?.LoadDocument(normalizedPath, objectName, uniqueId);
            }

            documentTabs.SelectedTab = existingPage;
            UpdateStatus(existingPage.Text);
            return;
        }

        var editorControl = new CopperfinAssetEditorControl(localization)
        {
            Dock = DockStyle.Fill,
            EmbeddedStudioShell = true
        };
        editorControl.OpenDocumentRequested += path => OpenDocument(path);

        var page = new TabPage(Path.GetFileName(normalizedPath))
        {
            ToolTipText = normalizedPath
        };
        page.Controls.Add(editorControl);
        documentTabs.TabPages.Add(page);
        documentTabs.SelectedTab = page;
        openDocuments[normalizedPath] = page;
        editorControl.LoadDocument(normalizedPath, objectName, uniqueId);

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
            Filter = CopperfinStudioOpenDialogFilter.Build(localization),
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
