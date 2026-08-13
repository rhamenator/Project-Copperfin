// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class StudioMainForm : Form
{
    private readonly TabControl documentTabs;
    private readonly SplitContainer shellSplitContainer;
    private readonly TabControl toolWindowTabs;
    private readonly TabPage commandWindowPage;
    private readonly TabPage terminalWindowPage;
    private readonly StudioCommandWindowControl commandWindowControl;
    private readonly StudioTerminalWindowControl terminalWindowControl;
    private readonly ToolStripStatusLabel statusLabel;
    private readonly ToolStripMenuItem openDocumentMenuItem;
    private readonly ToolStripMenuItem closeDocumentMenuItem;
    private readonly ToolStripMenuItem editMenuItem;
    private readonly ToolStripMenuItem undoMenuItem;
    private readonly ToolStripMenuItem buildProjectMenuItem;
    private readonly ToolStripMenuItem runProjectMenuItem;
    private readonly ToolStripMenuItem debugProjectMenuItem;
    private readonly ToolStripMenuItem commandWindowMenuItem;
    private readonly ToolStripMenuItem terminalWindowMenuItem;
    private readonly ToolStripMenuItem workspaceAgentPolicyMenuItem;
    private readonly ToolStripMenuItem floatCommandWindowMenuItem;
    private readonly ToolStripMenuItem floatTerminalWindowMenuItem;
    private readonly CopperfinLocalization localization;
    private readonly IStudioShellLayoutStore shellLayoutStore;
    private readonly Func<CopperfinLocalization, CopperfinWorkspaceAgentPolicyResult>
        workspaceAgentPolicyLoader;
    private readonly Dictionary<string, TabPage> openDocuments =
        new(CopperfinDocumentPathIdentity.CreateComparer());
    private const int DefaultSplitterDistance = 720;
    private const int MinimumToolWindowHeight = 160;
    private string selectedToolWindowKey = StudioShellLayoutState.CommandWindowKey;
    private Form? commandFloatingForm;
    private Form? terminalFloatingForm;
    private Rectangle? commandFloatingBounds;
    private Rectangle? terminalFloatingBounds;
    private bool shellTransitionInProgress;
    private bool formClosing;
    private bool commandWindowFloatingState;
    private bool terminalWindowFloatingState;

    public StudioMainForm(
        CopperfinLocalization? localization = null,
        IStudioShellLayoutStore? shellLayoutStore = null)
        : this(
            localization,
            shellLayoutStore,
            CopperfinWorkspaceAgentPolicyClient.TryLoad)
    {
    }

    internal StudioMainForm(
        CopperfinLocalization? localization,
        IStudioShellLayoutStore? shellLayoutStore,
        Func<CopperfinLocalization, CopperfinWorkspaceAgentPolicyResult> workspaceAgentPolicyLoader)
    {
        this.localization = localization ?? CopperfinLocalization.FromEnvironment();
        this.shellLayoutStore = shellLayoutStore ?? StudioShellLayoutFileStore.CreateDefault();
        this.workspaceAgentPolicyLoader = workspaceAgentPolicyLoader ??
            throw new ArgumentNullException(nameof(workspaceAgentPolicyLoader));

        Text = this.localization.Text("Studio.AppTitle");
        Width = 1480;
        Height = 980;
        StartPosition = FormStartPosition.CenterScreen;

        var menuStrip = new MenuStrip();
        var fileMenu = new ToolStripMenuItem(this.localization.Text("Studio.FileMenu"));
        openDocumentMenuItem = new ToolStripMenuItem(
            this.localization.Text("Studio.OpenMenu"),
            null,
            (_, _) => OpenFromPicker())
        {
            ShortcutKeys = Keys.Control | Keys.O
        };
        closeDocumentMenuItem = new ToolStripMenuItem(this.localization.Text("Studio.CloseMenu"), null, (_, _) => CloseActiveDocument())
        {
            ShortcutKeys = Keys.Control | Keys.F4,
            Enabled = false
        };
        var exitItem = new ToolStripMenuItem(this.localization.Text("Studio.ExitMenu"), null, (_, _) => Close());
        fileMenu.DropDownItems.Add(openDocumentMenuItem);
        fileMenu.DropDownItems.Add(closeDocumentMenuItem);
        fileMenu.DropDownItems.Add(new ToolStripSeparator());
        fileMenu.DropDownItems.Add(exitItem);
        menuStrip.Items.Add(fileMenu);

        editMenuItem = new ToolStripMenuItem(this.localization.Text("Studio.EditMenu"));
        undoMenuItem = new ToolStripMenuItem(
            this.localization.Text("AssetEditor.Undo.Command"),
            null,
            (_, _) => TryUndoActiveDocument())
        {
            ShortcutKeys = Keys.Control | Keys.Z,
            Enabled = false
        };
        editMenuItem.DropDownOpening += (_, _) => RefreshUndoCommandState();
        editMenuItem.DropDownItems.Add(undoMenuItem);
        menuStrip.Items.Add(editMenuItem);

        var projectMenu = new ToolStripMenuItem(this.localization.Text("Studio.ProjectMenu"));
        buildProjectMenuItem = new ToolStripMenuItem(
            this.localization.Text("Studio.BuildProjectMenu"),
            null,
            (_, _) => RunActiveProjectWorkflow(CopperfinProjectOperation.Build))
        {
            ShortcutKeys = Keys.Control | Keys.Shift | Keys.B,
            Enabled = false
        };
        runProjectMenuItem = new ToolStripMenuItem(
            this.localization.Text("Studio.RunProjectMenu"),
            null,
            (_, _) => RunActiveProjectWorkflow(CopperfinProjectOperation.Run))
        {
            ShortcutKeys = Keys.Control | Keys.F5,
            Enabled = false
        };
        debugProjectMenuItem = new ToolStripMenuItem(
            this.localization.Text("Studio.DebugProjectMenu"),
            null,
            (_, _) => RunActiveProjectWorkflow(CopperfinProjectOperation.Debug))
        {
            ShortcutKeys = Keys.F5,
            Enabled = false
        };
        projectMenu.DropDownItems.Add(buildProjectMenuItem);
        projectMenu.DropDownItems.Add(runProjectMenuItem);
        projectMenu.DropDownItems.Add(debugProjectMenuItem);
        menuStrip.Items.Add(projectMenu);

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
        workspaceAgentPolicyMenuItem = new ToolStripMenuItem(
            this.localization.Text("Studio.WorkspaceAgent.Menu"),
            null,
            async (_, _) => await ShowWorkspaceAgentPolicyAsync());
        floatCommandWindowMenuItem = new ToolStripMenuItem(this.localization.Text("Studio.FloatCommandWindowMenu"))
        {
            CheckOnClick = true
        };
        floatCommandWindowMenuItem.CheckedChanged += (_, _) =>
        {
            if (!shellTransitionInProgress)
            {
                SetCommandWindowFloating(floatCommandWindowMenuItem.Checked);
            }
        };
        floatTerminalWindowMenuItem = new ToolStripMenuItem(this.localization.Text("Studio.FloatTerminalWindowMenu"))
        {
            CheckOnClick = true
        };
        floatTerminalWindowMenuItem.CheckedChanged += (_, _) =>
        {
            if (!shellTransitionInProgress)
            {
                SetTerminalWindowFloating(floatTerminalWindowMenuItem.Checked);
            }
        };
        viewMenu.DropDownItems.Add(commandWindowMenuItem);
        viewMenu.DropDownItems.Add(terminalWindowMenuItem);
        viewMenu.DropDownItems.Add(workspaceAgentPolicyMenuItem);
        viewMenu.DropDownItems.Add(new ToolStripSeparator());
        viewMenu.DropDownItems.Add(floatCommandWindowMenuItem);
        viewMenu.DropDownItems.Add(floatTerminalWindowMenuItem);
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
        commandWindowControl = new StudioCommandWindowControl(this.localization, ExecuteCommandWindowInput);
        commandWindowPage.Controls.Add(commandWindowControl);
        toolWindowTabs.TabPages.Add(commandWindowPage);
        terminalWindowPage = new TabPage(this.localization.Text("VSIX.TerminalWindow.Title"));
        terminalWindowControl = new StudioTerminalWindowControl(this.localization);
        terminalWindowPage.Controls.Add(terminalWindowControl);
        toolWindowTabs.TabPages.Add(terminalWindowPage);
        toolWindowTabs.SelectedIndexChanged += (_, _) =>
        {
            if (toolWindowTabs.SelectedTab == commandWindowPage)
            {
                selectedToolWindowKey = StudioShellLayoutState.CommandWindowKey;
            }
            else if (toolWindowTabs.SelectedTab == terminalWindowPage)
            {
                selectedToolWindowKey = StudioShellLayoutState.TerminalWindowKey;
            }
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
        UpdateStatus(this.localization.Text("Studio.EmptyDocumentStatus"));
    }

    // RQ-CF-AGENT-004 owns this trusted product-UI preview and error boundary.
    internal string WorkspaceAgentPolicyMenuText => workspaceAgentPolicyMenuItem.Text;

    internal StudioWorkspaceAgentPolicyDialog CreateWorkspaceAgentPolicyDialogForTest(
        CopperfinWorkspaceAgentPolicyDescriptor descriptor)
    {
        return new StudioWorkspaceAgentPolicyDialog(descriptor, localization);
    }

    private async Task ShowWorkspaceAgentPolicyAsync()
    {
        workspaceAgentPolicyMenuItem.Enabled = false;
        try
        {
            var result = await LoadWorkspaceAgentPolicyAsyncForTest();
            if (IsDisposed || Disposing)
            {
                return;
            }
            if (!result.Success || result.Descriptor is null)
            {
                MessageBox.Show(
                    this,
                    WorkspaceAgentPolicyErrorText(result),
                    localization.Text("Studio.WorkspaceAgent.Title"),
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Warning);
                return;
            }

            using var dialog = new StudioWorkspaceAgentPolicyDialog(result.Descriptor, localization);
            dialog.ShowDialog(this);
        }
        finally
        {
            if (!IsDisposed && !Disposing)
            {
                workspaceAgentPolicyMenuItem.Enabled = true;
            }
        }
    }

    internal Task<CopperfinWorkspaceAgentPolicyResult> LoadWorkspaceAgentPolicyAsyncForTest()
    {
        return Task.Run(() => workspaceAgentPolicyLoader(localization));
    }

    internal string WorkspaceAgentPolicyErrorTextForTest(CopperfinWorkspaceAgentPolicyResult result)
    {
        return WorkspaceAgentPolicyErrorText(result);
    }

    private string WorkspaceAgentPolicyErrorText(CopperfinWorkspaceAgentPolicyResult result)
    {
        // Result.Error remains available to diagnostics, but raw parser,
        // process, and host output are not trusted user-facing prose.
        return result.DiagnosticCode switch
        {
            "workspace-agent-policy.host-missing" =>
                localization.Text("AssetEditor.Dialog.StudioHostMissing"),
            "workspace-agent-policy.host-timed-out" =>
                localization.Text("AssetEditor.Dialog.StudioHostTimedOut"),
            _ => localization.Text("Studio.WorkspaceAgent.InvalidPolicy")
        };
    }

    protected override void OnFormClosing(FormClosingEventArgs e)
    {
        // Capture the floating state before Form's owner teardown closes the
        // owned tool windows and their close handlers re-dock the controls.
        formClosing = true;
        SaveShellLayout();
        DockFloatingToolWindows();
        base.OnFormClosing(e);
    }

    protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
    {
        if (keyData == (Keys.Control | Keys.Z))
        {
            // Do not fall through to the menu strip for Ctrl+Z: a focused tool
            // window must retain its own undo stack even when the document host
            // also advertises an undo operation.
            return documentTabs.ContainsFocus && TryUndoActiveDocument();
        }

        return base.ProcessCmdKey(ref msg, keyData);
    }

    internal bool IsCommandWindowVisible => toolWindowTabs.TabPages.Contains(commandWindowPage) ||
                                            commandFloatingForm is not null;

    internal bool IsTerminalWindowVisible => toolWindowTabs.TabPages.Contains(terminalWindowPage) ||
                                             terminalFloatingForm is not null;

    internal bool IsCommandWindowFloating => commandWindowFloatingState;

    internal bool IsTerminalWindowFloating => terminalWindowFloatingState;

    internal Rectangle? CommandWindowFloatingBoundsForTest =>
        commandFloatingForm?.Bounds ?? commandFloatingBounds;

    internal Rectangle? TerminalWindowFloatingBoundsForTest =>
        terminalFloatingForm?.Bounds ?? terminalFloatingBounds;

    internal string CommandWindowTabTitle => commandWindowPage.Text;

    internal string TerminalWindowTabTitle => terminalWindowPage.Text;

    internal bool IsTerminalShellRunning => terminalWindowControl.IsShellRunning;

    internal string TerminalTranscript => terminalWindowControl.TranscriptText;

    internal int DocumentTabCount => documentTabs.TabPages.Count;

    internal string? ActiveDocumentPath => documentTabs.SelectedTab?.ToolTipText;

    internal string CloseDocumentMenuText => closeDocumentMenuItem.Text;

    internal Keys OpenDocumentShortcutKeys => openDocumentMenuItem.ShortcutKeys;

    internal string EditMenuText => editMenuItem.Text;

    internal string UndoMenuText => undoMenuItem.Text;

    internal Keys UndoShortcutKeys => undoMenuItem.ShortcutKeys;

    internal bool UndoCommandEnabled => undoMenuItem.Enabled;

    internal string BuildProjectMenuText => buildProjectMenuItem.Text;

    internal string RunProjectMenuText => runProjectMenuItem.Text;

    internal string DebugProjectMenuText => debugProjectMenuItem.Text;

    internal Keys BuildProjectShortcutKeys => buildProjectMenuItem.ShortcutKeys;

    internal Keys RunProjectShortcutKeys => runProjectMenuItem.ShortcutKeys;

    internal Keys DebugProjectShortcutKeys => debugProjectMenuItem.ShortcutKeys;

    internal bool ProjectCommandsEnabled => buildProjectMenuItem.Enabled &&
                                            runProjectMenuItem.Enabled &&
                                            debugProjectMenuItem.Enabled;

    internal string FloatCommandWindowMenuText => floatCommandWindowMenuItem.Text;

    internal string FloatTerminalWindowMenuText => floatTerminalWindowMenuItem.Text;

    internal int ShellSplitterDistance => shellSplitContainer.SplitterDistance;

    internal string SelectedToolWindowKey => selectedToolWindowKey;

    internal void SetCommandWindowVisible(bool visible)
    {
        if (!visible && commandFloatingForm is not null)
        {
            commandWindowFloatingState = false;
            DockCommandWindow(addPage: false);
        }

        SetToolWindowVisible(commandWindowPage, visible);
        if (!visible)
        {
            SetMenuChecked(floatCommandWindowMenuItem, false);
        }
    }

    internal void SetTerminalWindowVisible(bool visible)
    {
        if (!visible && terminalFloatingForm is not null)
        {
            terminalWindowFloatingState = false;
            DockTerminalWindow(addPage: false);
        }

        SetToolWindowVisible(terminalWindowPage, visible);
        if (!visible)
        {
            SetMenuChecked(floatTerminalWindowMenuItem, false);
        }
    }

    internal void SetCommandWindowFloatingForTest(bool floating)
    {
        SetCommandWindowFloating(floating);
    }

    internal void SetTerminalWindowFloatingForTest(bool floating)
    {
        SetTerminalWindowFloating(floating);
    }

    internal void SetCommandWindowFloatingBoundsForTest(Rectangle bounds)
    {
        commandFloatingBounds = bounds;
        if (commandFloatingForm is not null)
        {
            commandFloatingForm.Bounds = bounds;
        }
    }

    internal void SetTerminalWindowFloatingBoundsForTest(Rectangle bounds)
    {
        terminalFloatingBounds = bounds;
        if (terminalFloatingForm is not null)
        {
            terminalFloatingForm.Bounds = bounds;
        }
    }

    internal void CloseCommandFloatingWindowForTest()
    {
        commandFloatingForm?.Close();
    }

    internal void CloseTerminalFloatingWindowForTest()
    {
        terminalFloatingForm?.Close();
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

    internal string CommandWindowTranscriptText => commandWindowControl.TranscriptText;

    internal void SubmitCommandForTest(string command)
    {
        commandWindowControl.SubmitCommandForTest(command);
    }

    internal bool RunActiveProjectWorkflowForTest(CopperfinProjectOperation operation)
    {
        return RunActiveProjectWorkflow(operation);
    }

    internal void OpenEditMenuForTest()
    {
        editMenuItem.ShowDropDown();
        editMenuItem.HideDropDown();
    }

    internal bool TryUndoActiveDocumentForTest()
    {
        return TryUndoActiveDocument();
    }

    internal TextBox CommandWindowInputForTest => commandWindowControl.CommandInputForTest;

    internal bool ProcessCmdKeyForTest(Keys keyData)
    {
        var message = new Message();
        return ProcessCmdKey(ref message, keyData);
    }

    private string ExecuteCommandWindowInput(string command)
    {
        var editor = documentTabs.SelectedTab?.Controls
            .OfType<CopperfinAssetEditorControl>()
            .SingleOrDefault();
        return editor?.ExecuteCommandWindowInput(command) ??
               localization.Text("VSIX.CommandWindow.NoActiveSession");
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
        commandFloatingBounds = ReadFloatingBounds(
            state.CommandWindowFloatingX,
            state.CommandWindowFloatingY,
            state.CommandWindowFloatingWidth,
            state.CommandWindowFloatingHeight);
        terminalFloatingBounds = ReadFloatingBounds(
            state.TerminalWindowFloatingX,
            state.TerminalWindowFloatingY,
            state.TerminalWindowFloatingWidth,
            state.TerminalWindowFloatingHeight);
        commandWindowFloatingState = state.CommandWindowFloating;
        terminalWindowFloatingState = state.TerminalWindowFloating;
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

        if (state.CommandWindowFloating && IsCommandWindowVisible)
        {
            SetCommandWindowFloating(true);
        }
        if (state.TerminalWindowFloating && IsTerminalWindowVisible)
        {
            SetTerminalWindowFloating(true);
        }
        selectedToolWindowKey = state.SelectedToolWindow;
    }

    private void SaveShellLayout()
    {
        var commandBounds = CaptureFloatingBounds(commandFloatingForm, commandFloatingBounds);
        var terminalBounds = CaptureFloatingBounds(terminalFloatingForm, terminalFloatingBounds);
        shellLayoutStore.Save(new StudioShellLayoutState
        {
            CommandWindowVisible = IsCommandWindowVisible,
            TerminalWindowVisible = IsTerminalWindowVisible,
            SelectedToolWindow = SelectedToolWindowKey,
            SplitterDistance = NormalizeSplitterDistance(shellSplitContainer.SplitterDistance),
            CommandWindowFloating = commandWindowFloatingState,
            TerminalWindowFloating = terminalWindowFloatingState,
            CommandWindowFloatingX = commandBounds?.X,
            CommandWindowFloatingY = commandBounds?.Y,
            CommandWindowFloatingWidth = commandBounds?.Width,
            CommandWindowFloatingHeight = commandBounds?.Height,
            TerminalWindowFloatingX = terminalBounds?.X,
            TerminalWindowFloatingY = terminalBounds?.Y,
            TerminalWindowFloatingWidth = terminalBounds?.Width,
            TerminalWindowFloatingHeight = terminalBounds?.Height
        });
    }

    private bool IsValidShellLayout(StudioShellLayoutState state)
    {
        if ((state.Version != 1 && state.Version != 2 && state.Version != StudioShellLayoutState.CurrentVersion) ||
            (state.SelectedToolWindow != StudioShellLayoutState.CommandWindowKey &&
             state.SelectedToolWindow != StudioShellLayoutState.TerminalWindowKey) ||
            state.SplitterDistance < MinimumToolWindowHeight ||
            state.SplitterDistance > MaximumSplitterDistance)
        {
            return false;
        }

        if (state.Version >= StudioShellLayoutState.CurrentVersion &&
            ((!state.CommandWindowVisible && state.CommandWindowFloating) ||
             (!state.TerminalWindowVisible && state.TerminalWindowFloating)))
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

    private static Rectangle? CaptureFloatingBounds(Form? form, Rectangle? fallback)
    {
        return form is null || form.IsDisposed ? fallback : form.Bounds;
    }

    private static Rectangle? ReadFloatingBounds(int? x, int? y, int? width, int? height)
    {
        if (!x.HasValue || !y.HasValue || !width.HasValue || !height.HasValue ||
            width.Value < 560 || height.Value < MinimumToolWindowHeight)
        {
            return null;
        }

        var bounds = new Rectangle(x.Value, y.Value, width.Value, height.Value);
        return Screen.AllScreens.Any(screen => screen.WorkingArea.IntersectsWith(bounds))
            ? bounds
            : null;
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
            if (toolWindowTabs.SelectedTab == page)
            {
                var replacementPage = toolWindowTabs.TabPages
                    .Cast<TabPage>()
                    .FirstOrDefault(candidate => candidate != page);
                toolWindowTabs.SelectedTab = replacementPage;
            }
            toolWindowTabs.TabPages.Remove(page);
        }

        shellSplitContainer.Panel2Collapsed = toolWindowTabs.TabPages.Count == 0;
    }

    private void SetCommandWindowFloating(bool floating)
    {
        if (floating)
        {
            if (!IsCommandWindowVisible || commandFloatingForm is not null)
            {
                if (commandFloatingForm is not null)
                {
                    commandWindowFloatingState = true;
                }
                return;
            }

            commandWindowFloatingState = true;
            selectedToolWindowKey = StudioShellLayoutState.CommandWindowKey;
            RemoveToolWindowPage(commandWindowPage);
            commandFloatingForm = CreateFloatingToolWindow(
                localization.Text("VSIX.CommandWindow.Title"),
                commandWindowControl,
                () => SetCommandWindowFloating(false),
                commandFloatingBounds);
            SetMenuChecked(floatCommandWindowMenuItem, true);
            commandFloatingForm.Show(this);
            return;
        }

        commandWindowFloatingState = false;
        DockCommandWindow(addPage: true);
    }

    private void SetTerminalWindowFloating(bool floating)
    {
        if (floating)
        {
            if (!IsTerminalWindowVisible || terminalFloatingForm is not null)
            {
                if (terminalFloatingForm is not null)
                {
                    terminalWindowFloatingState = true;
                }
                return;
            }

            terminalWindowFloatingState = true;
            selectedToolWindowKey = StudioShellLayoutState.TerminalWindowKey;
            RemoveToolWindowPage(terminalWindowPage);
            terminalFloatingForm = CreateFloatingToolWindow(
                localization.Text("VSIX.TerminalWindow.Title"),
                terminalWindowControl,
                () => SetTerminalWindowFloating(false),
                terminalFloatingBounds);
            SetMenuChecked(floatTerminalWindowMenuItem, true);
            terminalFloatingForm.Show(this);
            return;
        }

        terminalWindowFloatingState = false;
        DockTerminalWindow(addPage: true);
    }

    private Form CreateFloatingToolWindow(
        string title,
        Control control,
        Action dockAction,
        Rectangle? initialBounds)
    {
        var form = new Form
        {
            Text = title,
            FormBorderStyle = FormBorderStyle.SizableToolWindow,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Size = new Size(720, 420),
            MinimumSize = new Size(560, MinimumToolWindowHeight)
        };
        if (initialBounds.HasValue)
        {
            form.Bounds = initialBounds.Value;
        }
        form.Controls.Add(control);
        form.FormClosing += (_, e) =>
        {
            if (formClosing || e.CloseReason == CloseReason.FormOwnerClosing)
            {
                return;
            }

            if (shellTransitionInProgress)
            {
                return;
            }

            e.Cancel = true;
            dockAction();
        };
        return form;
    }

    private void RemoveToolWindowPage(TabPage page)
    {
        if (toolWindowTabs.SelectedTab == page)
        {
            toolWindowTabs.SelectedTab = toolWindowTabs.TabPages
                .Cast<TabPage>()
                .FirstOrDefault(candidate => candidate != page);
        }

        toolWindowTabs.TabPages.Remove(page);
        shellSplitContainer.Panel2Collapsed = toolWindowTabs.TabPages.Count == 0;
    }

    private void DockCommandWindow(bool addPage)
    {
        if (commandFloatingForm is null)
        {
            if (addPage && !toolWindowTabs.TabPages.Contains(commandWindowPage))
            {
                toolWindowTabs.TabPages.Add(commandWindowPage);
            }
            SetMenuChecked(floatCommandWindowMenuItem, false);
            shellSplitContainer.Panel2Collapsed = toolWindowTabs.TabPages.Count == 0;
            return;
        }

        var floatingForm = commandFloatingForm;
        commandFloatingBounds = floatingForm.Bounds;
        commandFloatingForm = null;
        shellTransitionInProgress = true;
        try
        {
            floatingForm.Controls.Remove(commandWindowControl);
            commandWindowPage.Controls.Add(commandWindowControl);
            if (addPage && !toolWindowTabs.TabPages.Contains(commandWindowPage))
            {
                toolWindowTabs.TabPages.Add(commandWindowPage);
            }
            SetMenuChecked(floatCommandWindowMenuItem, false);
            floatingForm.Hide();
            DisposeFloatingToolWindow(floatingForm);
        }
        finally
        {
            shellTransitionInProgress = false;
        }

        shellSplitContainer.Panel2Collapsed = toolWindowTabs.TabPages.Count == 0;
    }

    private void DockTerminalWindow(bool addPage)
    {
        if (terminalFloatingForm is null)
        {
            if (addPage && !toolWindowTabs.TabPages.Contains(terminalWindowPage))
            {
                toolWindowTabs.TabPages.Add(terminalWindowPage);
            }
            SetMenuChecked(floatTerminalWindowMenuItem, false);
            shellSplitContainer.Panel2Collapsed = toolWindowTabs.TabPages.Count == 0;
            return;
        }

        var floatingForm = terminalFloatingForm;
        terminalFloatingBounds = floatingForm.Bounds;
        terminalFloatingForm = null;
        shellTransitionInProgress = true;
        try
        {
            floatingForm.Controls.Remove(terminalWindowControl);
            terminalWindowPage.Controls.Add(terminalWindowControl);
            if (addPage && !toolWindowTabs.TabPages.Contains(terminalWindowPage))
            {
                toolWindowTabs.TabPages.Add(terminalWindowPage);
            }
            SetMenuChecked(floatTerminalWindowMenuItem, false);
            floatingForm.Hide();
            DisposeFloatingToolWindow(floatingForm);
        }
        finally
        {
            shellTransitionInProgress = false;
        }

        shellSplitContainer.Panel2Collapsed = toolWindowTabs.TabPages.Count == 0;
    }

    private void DockFloatingToolWindows()
    {
        DockCommandWindow(addPage: false);
        DockTerminalWindow(addPage: false);
    }

    private void DisposeFloatingToolWindow(Form floatingForm)
    {
        if (floatingForm.IsDisposed)
        {
            return;
        }

        if (formClosing)
        {
            floatingForm.Dispose();
            return;
        }

        try
        {
            if (IsHandleCreated && !IsDisposed)
            {
                BeginInvoke(new Action(() =>
                {
                    if (!floatingForm.IsDisposed)
                    {
                        floatingForm.Dispose();
                    }
                }));
                return;
            }
        }
        catch (InvalidOperationException)
        {
            // The owner may be between handle teardown and disposal; use the
            // normal form path when no UI queue remains available.
        }

        floatingForm.Dispose();
    }

    private void SetMenuChecked(ToolStripMenuItem menuItem, bool checkedState)
    {
        if (menuItem == floatCommandWindowMenuItem)
        {
            menuItem.Text = localization.Text(
                checkedState ? "Studio.DockCommandWindowMenu" : "Studio.FloatCommandWindowMenu");
        }
        else if (menuItem == floatTerminalWindowMenuItem)
        {
            menuItem.Text = localization.Text(
                checkedState ? "Studio.DockTerminalWindowMenu" : "Studio.FloatTerminalWindowMenu");
        }

        if (menuItem.Checked == checkedState)
        {
            return;
        }

        shellTransitionInProgress = true;
        try
        {
            menuItem.Checked = checkedState;
        }
        finally
        {
            shellTransitionInProgress = false;
        }
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
        UpdateSelectedDocumentChrome();

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
        var activeEditor = page?.Controls.OfType<CopperfinAssetEditorControl>().SingleOrDefault();
        RefreshUndoCommandState(activeEditor);
        var projectCommandsEnabled = activeEditor?.CanRunProjectWorkflow == true;
        buildProjectMenuItem.Enabled = projectCommandsEnabled;
        runProjectMenuItem.Enabled = projectCommandsEnabled;
        debugProjectMenuItem.Enabled = projectCommandsEnabled;
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

    private bool RunActiveProjectWorkflow(CopperfinProjectOperation operation)
    {
        var editor = documentTabs.SelectedTab?.Controls
            .OfType<CopperfinAssetEditorControl>()
            .SingleOrDefault();
        return editor?.TryRunProjectWorkflow(operation) == true;
    }

    private bool TryUndoActiveDocument()
    {
        var editor = documentTabs.SelectedTab?.Controls
            .OfType<CopperfinAssetEditorControl>()
            .SingleOrDefault();
        var handled = editor?.TryHandleUndoCommand() == true;
        RefreshUndoCommandState(editor);
        return handled;
    }

    private void RefreshUndoCommandState(CopperfinAssetEditorControl? editor = null)
    {
        editor ??= documentTabs.SelectedTab?.Controls
            .OfType<CopperfinAssetEditorControl>()
            .SingleOrDefault();
        undoMenuItem.Text = editor?.GetUndoCommandText() ??
                            localization.Text("AssetEditor.Undo.Command");
        undoMenuItem.Enabled = editor?.CanHandleUndoCommand() == true;
    }
}
