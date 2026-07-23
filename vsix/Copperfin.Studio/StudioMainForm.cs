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
    private readonly ToolStripMenuItem closeDocumentMenuItem;
    private readonly ToolStripMenuItem commandWindowMenuItem;
    private readonly ToolStripMenuItem terminalWindowMenuItem;
    private readonly CopperfinLocalization localization;
    private readonly IStudioShellLayoutStore shellLayoutStore;
    private readonly Dictionary<string, TabPage> openDocuments =
        new(CopperfinDocumentPathIdentity.CreateComparer());
    private const int DefaultSplitterDistance = 720;
    private const int MinimumToolWindowHeight = 160;

    public StudioMainForm(
        CopperfinLocalization? localization = null,
        IStudioShellLayoutStore? shellLayoutStore = null)
    {
        this.localization = localization ?? CopperfinLocalization.FromEnvironment();
        this.shellLayoutStore = shellLayoutStore ?? StudioShellLayoutFileStore.CreateDefault();

        Text = this.localization.Text("Studio.AppTitle");
        Width = 1480;
        Height = 980;
        StartPosition = FormStartPosition.CenterScreen;

        var menuStrip = new MenuStrip();
        var fileMenu = new ToolStripMenuItem(this.localization.Text("Studio.FileMenu"));
        var openItem = new ToolStripMenuItem(this.localization.Text("Studio.OpenMenu"), null, (_, _) => OpenFromPicker());
        closeDocumentMenuItem = new ToolStripMenuItem(this.localization.Text("Studio.CloseMenu"), null, (_, _) => CloseActiveDocument())
        {
            ShortcutKeys = Keys.Control | Keys.F4,
            Enabled = false
        };
        var exitItem = new ToolStripMenuItem(this.localization.Text("Studio.ExitMenu"), null, (_, _) => Close());
        fileMenu.DropDownItems.Add(openItem);
        fileMenu.DropDownItems.Add(closeDocumentMenuItem);
        fileMenu.DropDownItems.Add(new ToolStripSeparator());
        fileMenu.DropDownItems.Add(exitItem);
        menuStrip.Items.Add(fileMenu);

        var viewMenu = new ToolStripMenuItem(this.localization.Text("Studio.ViewMenu"));
        commandWindowMenuItem = new ToolStripMenuItem(this.localization.Text("Studio.CommandWindowMenu"))
        {
            CheckOnClick = true,
            Checked = true
        };
        commandWindowMenuItem.CheckedChanged += (_, _) => SetCommandWindowVisible(commandWindowMenuItem.Checked);
        terminalWindowMenuItem = new ToolStripMenuItem(this.localization.Text("Studio.TerminalWindowMenu"))
        {
            CheckOnClick = true,
            Checked = true
        };
        terminalWindowMenuItem.CheckedChanged += (_, _) => SetTerminalWindowVisible(terminalWindowMenuItem.Checked);
        viewMenu.DropDownItems.Add(commandWindowMenuItem);
        viewMenu.DropDownItems.Add(terminalWindowMenuItem);
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
            UpdateSelectedDocumentChrome();
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

        Load += (_, _) =>
        {
            // WinForms validates Panel2MinSize against the current client height;
            // defer size-sensitive initialization until the form has been laid out.
            shellSplitContainer.Panel2MinSize = MinimumToolWindowHeight;
            RestoreShellLayout();
        };
        FormClosed += (_, _) => SaveShellLayout();
        UpdateStatus(this.localization.Text("Studio.EmptyDocumentStatus"));
    }

    internal bool IsCommandWindowVisible => toolWindowTabs.TabPages.Contains(commandWindowPage);

    internal bool IsTerminalWindowVisible => toolWindowTabs.TabPages.Contains(terminalWindowPage);

    internal string CommandWindowTabTitle => commandWindowPage.Text;

    internal string TerminalWindowTabTitle => terminalWindowPage.Text;

    internal bool IsTerminalShellRunning => terminalWindowControl.IsShellRunning;

    internal string TerminalTranscript => terminalWindowControl.TranscriptText;

    internal int DocumentTabCount => documentTabs.TabPages.Count;

    internal string? ActiveDocumentPath => documentTabs.SelectedTab?.ToolTipText;

    internal string CloseDocumentMenuText => closeDocumentMenuItem.Text;

    internal int ShellSplitterDistance => shellSplitContainer.SplitterDistance;

    internal string SelectedToolWindowKey => toolWindowTabs.SelectedTab == terminalWindowPage
        ? StudioShellLayoutState.TerminalWindowKey
        : StudioShellLayoutState.CommandWindowKey;

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

    internal void SetShellSplitterDistanceForTest(int distance)
    {
        shellSplitContainer.SplitterDistance = NormalizeSplitterDistance(distance);
    }

    private void RestoreShellLayout()
    {
        var state = shellLayoutStore.Load();
        if (state is null || !IsValidShellLayout(state))
        {
            state = CreateDefaultShellLayout();
        }

        shellSplitContainer.SplitterDistance = NormalizeSplitterDistance(state.SplitterDistance);
        commandWindowMenuItem.Checked = state.CommandWindowVisible;
        terminalWindowMenuItem.Checked = state.TerminalWindowVisible;

        if (state.SelectedToolWindow == StudioShellLayoutState.TerminalWindowKey &&
            IsTerminalWindowVisible)
        {
            toolWindowTabs.SelectedTab = terminalWindowPage;
        }
        else if (IsCommandWindowVisible)
        {
            toolWindowTabs.SelectedTab = commandWindowPage;
        }
        else if (IsTerminalWindowVisible)
        {
            toolWindowTabs.SelectedTab = terminalWindowPage;
        }
    }

    private void SaveShellLayout()
    {
        shellLayoutStore.Save(new StudioShellLayoutState
        {
            CommandWindowVisible = IsCommandWindowVisible,
            TerminalWindowVisible = IsTerminalWindowVisible,
            SelectedToolWindow = SelectedToolWindowKey,
            SplitterDistance = NormalizeSplitterDistance(shellSplitContainer.SplitterDistance)
        });
    }

    private bool IsValidShellLayout(StudioShellLayoutState state)
    {
        if (state.Version != StudioShellLayoutState.CurrentVersion ||
            (state.SelectedToolWindow != StudioShellLayoutState.CommandWindowKey &&
             state.SelectedToolWindow != StudioShellLayoutState.TerminalWindowKey) ||
            state.SplitterDistance < MinimumToolWindowHeight ||
            state.SplitterDistance > MaximumSplitterDistance)
        {
            return false;
        }

        return state.CommandWindowVisible || state.TerminalWindowVisible
            ? state.SelectedToolWindow == StudioShellLayoutState.CommandWindowKey
                ? state.CommandWindowVisible
                : state.TerminalWindowVisible
            : true;
    }

    private StudioShellLayoutState CreateDefaultShellLayout()
    {
        return new StudioShellLayoutState
        {
            CommandWindowVisible = true,
            TerminalWindowVisible = true,
            SelectedToolWindow = StudioShellLayoutState.CommandWindowKey,
            SplitterDistance = NormalizeSplitterDistance(DefaultSplitterDistance)
        };
    }

    private int MaximumSplitterDistance => Math.Max(
        MinimumToolWindowHeight,
        shellSplitContainer.Height - MinimumToolWindowHeight);

    private int NormalizeSplitterDistance(int distance)
    {
        return Math.Max(
            MinimumToolWindowHeight,
            Math.Min(MaximumSplitterDistance, distance));
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
        editorControl.OpenDocumentAtLineRequested += (path, _) => OpenDocument(path);

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

    internal void CloseActiveDocument()
    {
        var page = documentTabs.SelectedTab;
        if (page is null)
        {
            return;
        }

        var normalizedPath = page.ToolTipText;
        if (!string.IsNullOrWhiteSpace(normalizedPath))
        {
            openDocuments.Remove(normalizedPath);
        }

        documentTabs.TabPages.Remove(page);
        page.Dispose();
        UpdateSelectedDocumentChrome();
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

    private void UpdateSelectedDocumentChrome()
    {
        var page = documentTabs.SelectedTab;
        closeDocumentMenuItem.Enabled = page is not null;
        if (page is null)
        {
            Text = localization.Text("Studio.AppTitle");
            UpdateStatus(localization.Text("Studio.EmptyDocumentStatus"));
            return;
        }

        var normalizedPath = page.ToolTipText ?? string.Empty;
        var assetKind = CopperfinStudioHostBridge.DescribeAssetKind(normalizedPath, localization);
        Text = localization.Format("Studio.WindowTitleWithAssetKind", assetKind);
        UpdateStatus(normalizedPath);
    }
}
