using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinAssetEditorControl : UserControl
{
    private readonly Label titleLabel;
    private readonly Label subtitleLabel;
    private readonly Label pathLabel;
    private readonly Label detailsLabel;
    private readonly Label guidanceLabel;
    private readonly Label snapshotStatusLabel;
    private readonly Button launchButton;
    private readonly Button revealButton;
    private readonly Button refreshButton;
    private readonly Button buildButton;
    private readonly Button runButton;
    private readonly Button debugButton;
    private readonly Button debugRestartButton;
    private readonly Button debugContinueButton;
    private readonly Button debugStepButton;
    private readonly Button debugNextButton;
    private readonly Button debugOutButton;
    private readonly SplitContainer leftExplorerSplit;
    private readonly ListView sectionListView;
    private readonly ListView objectListView;
    private readonly PropertyGrid propertyGrid;
    private readonly CopperfinDesignSurfaceControl designSurface;
    private readonly RichTextBox workspaceSummaryBox;
    private readonly TabControl projectWorkspaceTabs;
    private readonly RichTextBox debuggerSummaryBox;
    private readonly RichTextBox taskListSummaryBox;
    private readonly RichTextBox codeReferencesSummaryBox;
    private readonly RichTextBox dataExplorerSummaryBox;
    private readonly RichTextBox objectBrowserSummaryBox;
    private readonly RichTextBox toolboxSummaryBox;
    private readonly RichTextBox buildersSummaryBox;
    private readonly RichTextBox coverageSummaryBox;
    private readonly RichTextBox databaseSummaryBox;
    private readonly TextBox dataExplorerFilterBox;
    private readonly TextBox objectBrowserFilterBox;
    private readonly CheckBox objectBrowserHideProjectCheckBox;
    private readonly Label debuggerStatusLabel;
    private readonly CopperfinLocalization localization;

    private string? currentPath;
    private CopperfinStudioSnapshotDocument? currentSnapshot;
    private CopperfinRuntimeDebugSession? currentDebugSession;
    private CopperfinProjectInsights? currentProjectInsights;
    private bool suppressSelectionSync;
    private bool embeddedStudioShell;
    private int loadGeneration;

    public bool EmbeddedStudioShell
    {
        get => embeddedStudioShell;
        set
        {
            embeddedStudioShell = value;
            ApplyHostMode();
        }
    }

    public CopperfinAssetEditorControl(CopperfinLocalization? localization = null)
    {
        this.localization = localization ?? CopperfinLocalization.FromEnvironment();

        BackColor = Color.FromArgb(248, 249, 252);
        ForeColor = Color.FromArgb(28, 32, 39);
        Padding = new Padding(24);

        titleLabel = new Label
        {
            AutoSize = true,
            Font = new Font("Segoe UI Semibold", 16.0F, FontStyle.Bold, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Title")
        };

        subtitleLabel = new Label
        {
            AutoSize = true,
            MaximumSize = new Size(960, 0),
            Font = new Font("Segoe UI", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Subtitle")
        };

        pathLabel = new Label
        {
            AutoSize = true,
            MaximumSize = new Size(960, 0),
            Font = new Font("Consolas", 9.5F, FontStyle.Regular, GraphicsUnit.Point)
        };

        detailsLabel = new Label
        {
            AutoSize = true,
            MaximumSize = new Size(960, 0),
            Font = new Font("Segoe UI", 10.0F, FontStyle.Regular, GraphicsUnit.Point)
        };

        guidanceLabel = new Label
        {
            AutoSize = true,
            MaximumSize = new Size(960, 0),
            Font = new Font("Segoe UI", 9.5F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Guidance")
        };

        launchButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.OpenNativeStudioButton")
        };
        launchButton.Click += (_, _) => LaunchStudio();

        revealButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.RevealInExplorerButton")
        };
        revealButton.Click += (_, _) => RevealInExplorer();

        refreshButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.RefreshButton")
        };
        refreshButton.Click += (_, _) =>
        {
            if (!string.IsNullOrWhiteSpace(currentPath))
            {
                LoadDocument(currentPath!);
            }
        };

        buildButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Project.BuildButton"),
            Visible = false
        };
        buildButton.Click += (_, _) => QueueUiAction(() => RunProjectWorkflowAsync(CopperfinProjectOperation.Build));

        runButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Project.RunButton"),
            Visible = false
        };
        runButton.Click += (_, _) => QueueUiAction(() => RunProjectWorkflowAsync(CopperfinProjectOperation.Run));

        debugButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Project.DebugButton"),
            Visible = false
        };
        debugButton.Click += (_, _) => QueueUiAction(() => RunProjectWorkflowAsync(CopperfinProjectOperation.Debug));

        debugRestartButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.StartSessionButton")
        };
        debugRestartButton.Click += (_, _) => QueueUiAction(StartDebugSessionAsync);

        debugContinueButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.ContinueButton")
        };
        debugContinueButton.Click += (_, _) => QueueUiAction(() => AdvanceDebugSessionAsync(CopperfinRuntimeDebugClient.ContinueAsync));

        debugStepButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.StepButton")
        };
        debugStepButton.Click += (_, _) => QueueUiAction(() => AdvanceDebugSessionAsync(CopperfinRuntimeDebugClient.StepIntoAsync));

        debugNextButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.NextButton")
        };
        debugNextButton.Click += (_, _) => QueueUiAction(() => AdvanceDebugSessionAsync(CopperfinRuntimeDebugClient.StepOverAsync));

        debugOutButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.OutButton")
        };
        debugOutButton.Click += (_, _) => QueueUiAction(() => AdvanceDebugSessionAsync(CopperfinRuntimeDebugClient.StepOutAsync));

        snapshotStatusLabel = new Label
        {
            AutoSize = true,
            MaximumSize = new Size(960, 0),
            Font = new Font("Segoe UI", 9.5F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Snapshot.LoadingStatus")
        };

        objectListView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        objectListView.Columns.Add(this.localization.Text("AssetEditor.Column.Object"), 240);
        objectListView.Columns.Add(this.localization.Text("AssetEditor.Column.Type"), 180);
        objectListView.Columns.Add(this.localization.Text("AssetEditor.Column.Record"), 70);
        objectListView.SelectedIndexChanged += (_, _) => SyncSelectionFromList();

        sectionListView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        sectionListView.Columns.Add(this.localization.Text("AssetEditor.Column.Section"), 200);
        sectionListView.Columns.Add(this.localization.Text("AssetEditor.Column.Objects"), 70);
        sectionListView.Columns.Add(this.localization.Text("AssetEditor.Column.Top"), 80);
        sectionListView.SelectedIndexChanged += (_, _) => SyncExplorerSelection();

        propertyGrid = new PropertyGrid
        {
            Dock = DockStyle.Fill,
            HelpVisible = true,
            ToolbarVisible = false
        };
        propertyGrid.PropertyValueChanged += (_, e) => ApplyPropertyGridChange(e.ChangedItem.PropertyDescriptor.Name, e.ChangedItem.Value);

        designSurface = new CopperfinDesignSurfaceControl
        {
            Dock = DockStyle.Fill
        };
        designSurface.SelectedRecordChanged += recordIndex => SyncSelectionFromSurface(recordIndex);
        designSurface.ObjectMoved += (recordIndex, left, top) =>
        {
            var horizontalName = currentSnapshot?.AssetFamily is "report" or "label" ? "HPOS" : "Left";
            var verticalName = currentSnapshot?.AssetFamily is "report" or "label" ? "VPOS" : "Top";
            ApplyVisualPropertyChange(recordIndex, horizontalName, left.ToString());
            ApplyVisualPropertyChange(recordIndex, verticalName, top.ToString());
        };

        workspaceSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Visible = false
        };

        debuggerSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Debugger.InitialSummary")
        };

        taskListSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.TaskList")
        };

        codeReferencesSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.CodeReferences")
        };

        dataExplorerSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.DataExplorer")
        };

        objectBrowserSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.ObjectBrowser")
        };

        toolboxSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.Toolbox")
        };

        buildersSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.Builders")
        };

        coverageSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.Coverage")
        };

        databaseSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.Database")
        };

        dataExplorerFilterBox = new TextBox
        {
            Dock = DockStyle.Top
        };
        dataExplorerFilterBox.TextChanged += (_, _) => RefreshProjectWorkspaceInsightViews();

        objectBrowserFilterBox = new TextBox
        {
            Dock = DockStyle.Top
        };
        objectBrowserFilterBox.TextChanged += (_, _) => RefreshProjectWorkspaceInsightViews();

        objectBrowserHideProjectCheckBox = new CheckBox
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectBrowser.HideProjectRecords"),
            Padding = new Padding(8, 6, 8, 6)
        };
        objectBrowserHideProjectCheckBox.CheckedChanged += (_, _) => RefreshProjectWorkspaceInsightViews();

        debuggerStatusLabel = new Label
        {
            AutoSize = true,
            MaximumSize = new Size(960, 0),
            Font = new Font("Segoe UI", 9.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Debugger.ReadyStatus")
        };

        var debuggerButtonPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Top,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false,
            Padding = new Padding(8)
        };
        debuggerButtonPanel.Controls.Add(debugRestartButton);
        debuggerButtonPanel.Controls.Add(debugContinueButton);
        debuggerButtonPanel.Controls.Add(debugStepButton);
        debuggerButtonPanel.Controls.Add(debugNextButton);
        debuggerButtonPanel.Controls.Add(debugOutButton);

        var debuggerStatusPanel = new Panel
        {
            Dock = DockStyle.Top,
            Height = 34,
            Padding = new Padding(8, 8, 8, 0)
        };
        debuggerStatusPanel.Controls.Add(debuggerStatusLabel);

        var debuggerPageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        debuggerPageHost.Controls.Add(debuggerSummaryBox);
        debuggerPageHost.Controls.Add(debuggerStatusPanel);
        debuggerPageHost.Controls.Add(debuggerButtonPanel);

        projectWorkspaceTabs = new TabControl
        {
            Dock = DockStyle.Fill,
            Visible = false
        };
        var summaryPage = new TabPage(this.localization.Text("AssetEditor.Tab.Summary"));
        summaryPage.Controls.Add(workspaceSummaryBox);
        var debuggerPage = new TabPage(this.localization.Text("AssetEditor.Tab.Debugger"));
        debuggerPage.Controls.Add(debuggerPageHost);
        var taskListPage = new TabPage(this.localization.Text("AssetEditor.Tab.TaskList"));
        taskListPage.Controls.Add(taskListSummaryBox);
        var codeReferencesPage = new TabPage(this.localization.Text("AssetEditor.Tab.CodeReferences"));
        codeReferencesPage.Controls.Add(codeReferencesSummaryBox);
        var dataExplorerPageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        dataExplorerPageHost.Controls.Add(dataExplorerSummaryBox);
        dataExplorerPageHost.Controls.Add(dataExplorerFilterBox);
        var dataExplorerPage = new TabPage(this.localization.Text("AssetEditor.Tab.DataExplorer"));
        dataExplorerPage.Controls.Add(dataExplorerPageHost);
        var objectBrowserOptionsPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Top,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false
        };
        objectBrowserOptionsPanel.Controls.Add(objectBrowserHideProjectCheckBox);
        var objectBrowserPageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        objectBrowserPageHost.Controls.Add(objectBrowserSummaryBox);
        objectBrowserPageHost.Controls.Add(objectBrowserOptionsPanel);
        objectBrowserPageHost.Controls.Add(objectBrowserFilterBox);
        var objectBrowserPage = new TabPage(this.localization.Text("AssetEditor.Tab.ObjectBrowser"));
        objectBrowserPage.Controls.Add(objectBrowserPageHost);
        var toolboxPage = new TabPage(this.localization.Text("AssetEditor.Tab.Toolbox"));
        toolboxPage.Controls.Add(toolboxSummaryBox);
        var buildersPage = new TabPage(this.localization.Text("AssetEditor.Tab.Builders"));
        buildersPage.Controls.Add(buildersSummaryBox);
        var coveragePage = new TabPage(this.localization.Text("AssetEditor.Tab.Coverage"));
        coveragePage.Controls.Add(coverageSummaryBox);
        var databasePage = new TabPage(this.localization.Text("AssetEditor.Tab.Database"));
        databasePage.Controls.Add(databaseSummaryBox);
        projectWorkspaceTabs.TabPages.Add(summaryPage);
        projectWorkspaceTabs.TabPages.Add(debuggerPage);
        projectWorkspaceTabs.TabPages.Add(taskListPage);
        projectWorkspaceTabs.TabPages.Add(codeReferencesPage);
        projectWorkspaceTabs.TabPages.Add(dataExplorerPage);
        projectWorkspaceTabs.TabPages.Add(objectBrowserPage);
        projectWorkspaceTabs.TabPages.Add(toolboxPage);
        projectWorkspaceTabs.TabPages.Add(buildersPage);
        projectWorkspaceTabs.TabPages.Add(coveragePage);
        projectWorkspaceTabs.TabPages.Add(databasePage);

        var surfaceHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        surfaceHost.Controls.Add(projectWorkspaceTabs);
        surfaceHost.Controls.Add(designSurface);

        var rightSplit = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Horizontal,
            SplitterDistance = 320
        };
        rightSplit.Panel1.Controls.Add(surfaceHost);
        rightSplit.Panel2.Controls.Add(propertyGrid);

        leftExplorerSplit = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Horizontal,
            SplitterDistance = 180
        };
        leftExplorerSplit.Panel1.Controls.Add(sectionListView);
        leftExplorerSplit.Panel2.Controls.Add(objectListView);

        var splitContainer = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Vertical,
            SplitterDistance = 360
        };
        splitContainer.Panel1.Controls.Add(leftExplorerSplit);
        splitContainer.Panel2.Controls.Add(rightSplit);

        var buttonPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false,
            Margin = new Padding(0, 8, 0, 0)
        };
        buttonPanel.Controls.Add(launchButton);
        buttonPanel.Controls.Add(revealButton);
        buttonPanel.Controls.Add(refreshButton);
        buttonPanel.Controls.Add(buildButton);
        buttonPanel.Controls.Add(runButton);
        buttonPanel.Controls.Add(debugButton);

        var stack = new FlowLayoutPanel
        {
            Dock = DockStyle.Fill,
            FlowDirection = FlowDirection.TopDown,
            WrapContents = false,
            AutoScroll = true
        };

        stack.Controls.Add(titleLabel);
        stack.Controls.Add(subtitleLabel);
        stack.Controls.Add(pathLabel);
        stack.Controls.Add(detailsLabel);
        stack.Controls.Add(guidanceLabel);
        stack.Controls.Add(buttonPanel);
        stack.Controls.Add(snapshotStatusLabel);
        stack.Controls.Add(splitContainer);

        Controls.Add(stack);
        ApplyHostMode();
    }

    public bool CanHandleUndoCommand()
    {
        if (TryFindFocusedUndoTextBox() is not null)
        {
            return true;
        }

        return currentSnapshot?.CommandUndoAvailable == true && !string.IsNullOrWhiteSpace(currentPath);
    }

    public string GetUndoCommandText()
    {
        if (TryFindFocusedUndoTextBox() is not null)
        {
            return this.localization.Text("AssetEditor.Undo.Command");
        }

        if (currentSnapshot?.CommandUndoAvailable == true)
        {
            return BuildUndoCommandText(currentSnapshot.CommandUndoLabel);
        }

        return this.localization.Text("AssetEditor.Undo.Command");
    }

    public bool TryHandleUndoCommand()
    {
        var focusedUndoTextBox = TryFindFocusedUndoTextBox();
        if (focusedUndoTextBox is not null)
        {
            focusedUndoTextBox.Undo();
            snapshotStatusLabel.Text = this.localization.Text("AssetEditor.Undo.EditExecuted");
            return true;
        }

        if (currentSnapshot?.CommandUndoAvailable != true || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        var priorLabel = currentSnapshot.CommandUndoLabel;
        var selectedRecordIndex = (propertyGrid.SelectedObject as CopperfinDesignerSelection)?.RecordIndex ?? TryReadSelectedRecordIndex();
        snapshotStatusLabel.Text = BuildUndoExecutingStatus(priorLabel);

        var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(currentPath!);
        if (!undoResult.Success || undoResult.Document is null)
        {
            snapshotStatusLabel.Text = BuildUndoFailedStatus(undoResult.Error);
            return false;
        }

        currentSnapshot = undoResult.Document;
        snapshotStatusLabel.Text = BuildUndoCompletedStatus(priorLabel, currentSnapshot);
        PopulateSectionList();
        PopulateObjectList();
        LoadSurface();
        if (selectedRecordIndex >= 0)
        {
            designSurface.SelectRecord(selectedRecordIndex);
            SyncSelectionFromSurface(selectedRecordIndex);
        }

        return true;
    }

    private int TryReadSelectedRecordIndex()
    {
        if (objectListView.SelectedItems.Count == 0)
        {
            return -1;
        }

        return int.TryParse(objectListView.SelectedItems[0].SubItems[2].Text, out var recordIndex)
            ? recordIndex
            : -1;
    }

    public void LoadDocument(string path)
    {
        loadGeneration++;
        currentPath = path;

        var info = new FileInfo(path);
        titleLabel.Text = CopperfinStudioHostBridge.DescribeAssetKind(path, localization);
        pathLabel.Text = path;
        detailsLabel.Text =
            $"Size: {info.Length:N0} bytes   Last write: {info.LastWriteTime:G}   Extension: {info.Extension.ToLowerInvariant()}";
        launchButton.Enabled = true;
        revealButton.Enabled = true;
        refreshButton.Enabled = true;
        currentSnapshot = null;
        currentDebugSession = null;
        currentProjectInsights = null;
        sectionListView.Items.Clear();
        objectListView.Items.Clear();
        propertyGrid.SelectedObject = null;
        designSurface.LoadObjects(string.Empty, Array.Empty<CopperfinStudioSnapshotObject>());
        workspaceSummaryBox.Text = string.Empty;
        workspaceSummaryBox.Visible = false;
        projectWorkspaceTabs.Visible = false;
        debuggerSummaryBox.Text = this.localization.Text("AssetEditor.Debugger.InitialSummary");
        taskListSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.TaskList");
        codeReferencesSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.CodeReferences");
        dataExplorerSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.DataExplorer");
        objectBrowserSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.ObjectBrowser");
        toolboxSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.Toolbox");
        buildersSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.Builders");
        coverageSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.Coverage");
        databaseSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.Database");
        dataExplorerFilterBox.Text = string.Empty;
        objectBrowserFilterBox.Text = string.Empty;
        objectBrowserHideProjectCheckBox.Checked = false;
        debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.ReadyStatus");
        SetDebuggerButtonsEnabled(false);
        designSurface.Visible = true;
        snapshotStatusLabel.Text = this.localization.Text("AssetEditor.Snapshot.LoadingStatus");
        UpdateProjectCommandVisibility();
        _ = LoadSnapshotAsync(path);
    }

    private async Task LoadSnapshotAsync(string path)
    {
        var expectedGeneration = loadGeneration;
        var snapshotResult = await Task.Run(() => CopperfinStudioSnapshotClient.TryLoad(path));
        if (IsDisposed || Disposing || expectedGeneration != loadGeneration || !string.Equals(currentPath, path, StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        PostToUi(() =>
        {
            if (IsDisposed || Disposing || expectedGeneration != loadGeneration || !string.Equals(currentPath, path, StringComparison.OrdinalIgnoreCase))
            {
                return;
            }

            if (!snapshotResult.Success || snapshotResult.Document is null)
            {
                snapshotStatusLabel.Text = BuildSnapshotUnavailableStatus(snapshotResult.Error);
                return;
            }

            currentSnapshot = snapshotResult.Document;
            snapshotStatusLabel.Text = BuildSnapshotLoadedStatus(currentSnapshot);
            guidanceLabel.Text = BuildGuidanceText(currentSnapshot.AssetFamily);
            UpdateProjectCommandVisibility();
            PopulateSectionList();
            PopulateObjectList();
            LoadSurface();
        });
    }

    private void PostToUi(Action action)
    {
        if (IsDisposed || Disposing)
        {
            return;
        }

        if (!IsHandleCreated)
        {
            return;
        }

        try
        {
            if (InvokeRequired)
            {
                BeginInvoke(action);
            }
            else
            {
                action();
            }
        }
        catch (ObjectDisposedException)
        {
        }
        catch (InvalidOperationException)
        {
        }
    }

    private void PopulateSectionList()
    {
        sectionListView.BeginUpdate();
        sectionListView.Items.Clear();

        if (currentSnapshot?.ReportLayout?.Sections is { Count: > 0 })
        {
            sectionListView.Visible = true;
            leftExplorerSplit.Panel1Collapsed = false;
            sectionListView.Columns[0].Text = this.localization.Text("AssetEditor.Column.Section");
            sectionListView.Columns[1].Text = this.localization.Text("AssetEditor.Column.Objects");
            sectionListView.Columns[2].Text = this.localization.Text("AssetEditor.Column.Top");
            foreach (var section in currentSnapshot.ReportLayout.Sections)
            {
                var item = new ListViewItem(section.Title);
                item.SubItems.Add(section.Objects.Count.ToString());
                item.SubItems.Add(section.Top.ToString());
                item.Tag = section;
                sectionListView.Items.Add(item);
            }

            if (sectionListView.Items.Count > 0)
            {
                sectionListView.Items[0].Selected = true;
            }

            sectionListView.EndUpdate();
            return;
        }

        if (currentSnapshot?.ProjectWorkspace?.Groups is { Count: > 0 })
        {
            sectionListView.Visible = true;
            leftExplorerSplit.Panel1Collapsed = false;
            sectionListView.Columns[0].Text = this.localization.Text("AssetEditor.Column.Group");
            sectionListView.Columns[1].Text = this.localization.Text("AssetEditor.Column.Items");
            sectionListView.Columns[2].Text = this.localization.Text("AssetEditor.Column.Excluded");
            foreach (var group in currentSnapshot.ProjectWorkspace.Groups)
            {
                var item = new ListViewItem(group.Title);
                item.SubItems.Add(group.ItemCount.ToString());
                item.SubItems.Add(group.ExcludedCount.ToString());
                item.Tag = group;
                sectionListView.Items.Add(item);
            }

            if (sectionListView.Items.Count > 0)
            {
                sectionListView.Items[0].Selected = true;
            }

            sectionListView.EndUpdate();
            return;
        }

        sectionListView.Visible = false;
        leftExplorerSplit.Panel1Collapsed = true;
        sectionListView.EndUpdate();
    }

    private void PopulateObjectList()
    {
        objectListView.BeginUpdate();
        objectListView.Items.Clear();
        ConfigureObjectColumns();

        if (currentSnapshot is null)
        {
            objectListView.EndUpdate();
            return;
        }

        foreach (var item in GetVisibleObjects())
        {
            var projectEntry = LookupProjectEntry(item.RecordIndex);
            var title = string.IsNullOrWhiteSpace(item.Title) ? $"Record {item.RecordIndex}" : item.Title;
            if (currentSnapshot.AssetFamily == "project" && projectEntry is not null && !string.IsNullOrWhiteSpace(projectEntry.RelativePath))
            {
                title = projectEntry.RelativePath;
            }

            var subtitle = currentSnapshot.AssetFamily == "project"
                ? projectEntry?.GroupTitle ?? item.Subtitle
                : item.Subtitle;

            var listItem = new ListViewItem(title);
            listItem.SubItems.Add(subtitle);
            listItem.SubItems.Add(item.RecordIndex.ToString());
            listItem.Tag = item;

            if (item.Deleted)
            {
                listItem.ForeColor = Color.Firebrick;
            }

            objectListView.Items.Add(listItem);
        }

        objectListView.EndUpdate();

        if (objectListView.Items.Count > 0)
        {
            objectListView.Items[0].Selected = true;
        }
        else
        {
            propertyGrid.SelectedObject = null;
        }
    }

    private void SyncSelectionFromList()
    {
        if (suppressSelectionSync) {
            return;
        }
        var selectedObject = objectListView.SelectedItems
            .Cast<ListViewItem>()
            .Select(item => item.Tag as CopperfinStudioSnapshotObject)
            .FirstOrDefault(item => item is not null);

        propertyGrid.SelectedObject = selectedObject is null || currentSnapshot is null
            ? null
            : CopperfinDesignerSelection.FromSnapshot(currentSnapshot.AssetFamily, selectedObject);
        designSurface.SelectRecord(selectedObject?.RecordIndex);
    }

    private void SyncExplorerSelection()
    {
        if (suppressSelectionSync)
        {
            return;
        }

        PopulateObjectList();
    }

    private void SyncSelectionFromSurface(int recordIndex)
    {
        if (suppressSelectionSync) {
            return;
        }

        try
        {
            suppressSelectionSync = true;
            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            var selectedObject = objectListView.Items
                .Cast<ListViewItem>()
                .Where(item => item.Selected)
                .Select(item => item.Tag as CopperfinStudioSnapshotObject)
                .FirstOrDefault(item => item is not null);

            propertyGrid.SelectedObject = selectedObject is null || currentSnapshot is null
                ? null
                : CopperfinDesignerSelection.FromSnapshot(currentSnapshot.AssetFamily, selectedObject);
        }
        finally
        {
            suppressSelectionSync = false;
        }
    }

    private void ApplyPropertyGridChange(string propertyName, object oldValue)
    {
        if (propertyGrid.SelectedObject is not CopperfinDesignerSelection selection || string.IsNullOrWhiteSpace(currentPath))
        {
            return;
        }

        if (selection.TryGetUpdate(propertyName, out var targetName, out var serializedValue))
        {
            ApplyVisualPropertyChange(selection.RecordIndex, targetName, serializedValue);
        }
    }

    private void ApplyVisualPropertyChange(int recordIndex, string propertyName, string propertyValue)
    {
        if (string.IsNullOrWhiteSpace(currentPath))
        {
            return;
        }

        snapshotStatusLabel.Text = BuildPropertyApplyingStatus(propertyName);
        var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(currentPath!, recordIndex, propertyName, propertyValue);
        if (!updateResult.Success || updateResult.Document is null)
        {
            snapshotStatusLabel.Text = BuildPropertyUpdateFailedStatus(updateResult.Error);
            return;
        }

        currentSnapshot = updateResult.Document;
        snapshotStatusLabel.Text = BuildPropertyUpdatedStatus(propertyName, currentSnapshot);
        PopulateSectionList();
        PopulateObjectList();
        LoadSurface();
        designSurface.SelectRecord(recordIndex);
        SyncSelectionFromSurface(recordIndex);
    }

    private TextBoxBase? TryFindFocusedUndoTextBox()
    {
        return TryFindFocusedUndoTextBox(this);
    }

    private static TextBoxBase? TryFindFocusedUndoTextBox(Control parent)
    {
        if (!parent.ContainsFocus)
        {
            return null;
        }

        if (parent is TextBoxBase textBoxBase && textBoxBase.CanUndo)
        {
            return textBoxBase;
        }

        foreach (Control child in parent.Controls)
        {
            var found = TryFindFocusedUndoTextBox(child);
            if (found is not null)
            {
                return found;
            }
        }

        return null;
    }

    private void LoadSurface()
    {
        if (currentSnapshot?.ReportLayout is not null &&
            (currentSnapshot.AssetFamily == "report" || currentSnapshot.AssetFamily == "label"))
        {
            workspaceSummaryBox.Visible = false;
            designSurface.Visible = true;
            designSurface.LoadReportLayout(currentSnapshot.ReportLayout, currentSnapshot.Objects);
            return;
        }

        if (currentSnapshot?.ProjectWorkspace is not null && currentSnapshot.AssetFamily == "project")
        {
            currentProjectInsights = CopperfinProjectInsightClient.BuildInsights(currentSnapshot);
            RefreshProjectWorkspaceInsightViews();
            workspaceSummaryBox.Visible = true;
            projectWorkspaceTabs.Visible = true;
            projectWorkspaceTabs.SelectedIndex = 0;
            designSurface.Visible = false;
            UpdateProjectCommandVisibility();
            return;
        }

        var objects = currentSnapshot?.Objects is null
            ? (IReadOnlyList<CopperfinStudioSnapshotObject>)Array.Empty<CopperfinStudioSnapshotObject>()
            : currentSnapshot.Objects;
        workspaceSummaryBox.Visible = false;
        projectWorkspaceTabs.Visible = false;
        designSurface.Visible = true;
        designSurface.LoadObjects(currentSnapshot?.AssetFamily ?? string.Empty, objects);
        UpdateProjectCommandVisibility();
    }

    private void RefreshProjectWorkspaceInsightViews()
    {
        if (currentSnapshot?.ProjectWorkspace is null || currentSnapshot.AssetFamily != "project")
        {
            return;
        }

        workspaceSummaryBox.Text = BuildProjectWorkspaceSummary(currentSnapshot);
        taskListSummaryBox.Text = BuildTaskListSummary(currentProjectInsights);
        codeReferencesSummaryBox.Text = BuildCodeReferenceSummary(currentProjectInsights);
        dataExplorerSummaryBox.Text = BuildDataExplorerSummary(currentSnapshot, currentProjectInsights, dataExplorerFilterBox.Text);
        objectBrowserSummaryBox.Text = BuildObjectBrowserSummary(currentSnapshot, currentProjectInsights, objectBrowserFilterBox.Text, objectBrowserHideProjectCheckBox.Checked);
        toolboxSummaryBox.Text = BuildToolboxSummary(currentSnapshot, currentProjectInsights);
        buildersSummaryBox.Text = BuildBuilderSummary(currentSnapshot, currentProjectInsights);
        coverageSummaryBox.Text = BuildCoverageSummary(currentSnapshot, currentDebugSession);
        databaseSummaryBox.Text = BuildDatabaseFederationSummary(currentSnapshot, dataExplorerFilterBox.Text);
    }

    private string BuildGuidanceText(string assetFamily)
    {
        return assetFamily switch
        {
            "form" => this.localization.Text("AssetEditor.Guidance.Form"),
            "class_library" => this.localization.Text("AssetEditor.Guidance.ClassLibrary"),
            "report" => this.localization.Text("AssetEditor.Guidance.Report"),
            "label" => this.localization.Text("AssetEditor.Guidance.Label"),
            "menu" => this.localization.Text("AssetEditor.Guidance.Menu"),
            "project" => this.localization.Text("AssetEditor.Guidance.Project"),
            _ => this.localization.Text("AssetEditor.Guidance.Generic")
        };
    }

    private string BuildUndoCommandText(string? commandUndoLabel)
    {
        return string.IsNullOrWhiteSpace(commandUndoLabel)
            ? this.localization.Text("AssetEditor.Undo.Command")
            : this.localization.Format("AssetEditor.Undo.WithLabel", commandUndoLabel!);
    }

    private string BuildUndoExecutingStatus(string? commandUndoLabel)
    {
        return string.IsNullOrWhiteSpace(commandUndoLabel)
            ? this.localization.Text("AssetEditor.Undo.Executing")
            : this.localization.Format("AssetEditor.Undo.ExecutingWithLabel", commandUndoLabel!);
    }

    private string BuildUndoFailedStatus(string? error)
    {
        return this.localization.Format("AssetEditor.Undo.Failed", error ?? string.Empty);
    }

    private string BuildUndoCompletedStatus(string? commandUndoLabel, CopperfinStudioSnapshotDocument snapshot)
    {
        return string.IsNullOrWhiteSpace(commandUndoLabel)
            ? this.localization.Format("AssetEditor.Undo.Completed", snapshot.Objects.Count, snapshot.FieldCount)
            : this.localization.Format("AssetEditor.Undo.CompletedWithLabel", commandUndoLabel!, snapshot.Objects.Count, snapshot.FieldCount);
    }

    private string BuildSnapshotUnavailableStatus(string? error)
    {
        return this.localization.Format("AssetEditor.Snapshot.Unavailable", error ?? string.Empty);
    }

    private string BuildSnapshotLoadedStatus(CopperfinStudioSnapshotDocument snapshot)
    {
        var status = this.localization.Format(
            "AssetEditor.Snapshot.Loaded",
            snapshot.Objects.Count,
            snapshot.FieldCount,
            snapshot.IndexCount);
        if (snapshot.CommandUndoAvailable && !string.IsNullOrWhiteSpace(snapshot.CommandUndoLabel))
        {
            status += this.localization.Format("AssetEditor.Snapshot.UndoAvailable", snapshot.CommandUndoLabel);
        }

        return status;
    }

    private string BuildPropertyApplyingStatus(string propertyName)
    {
        return this.localization.Format("AssetEditor.Property.ApplyingChange", propertyName);
    }

    private string BuildPropertyUpdateFailedStatus(string? error)
    {
        return this.localization.Format("AssetEditor.Property.UpdateFailed", error ?? string.Empty);
    }

    private string BuildPropertyUpdatedStatus(string propertyName, CopperfinStudioSnapshotDocument snapshot)
    {
        return this.localization.Format("AssetEditor.Property.Updated", propertyName, snapshot.Objects.Count, snapshot.FieldCount) +
            (snapshot.CommandUndoAvailable && !string.IsNullOrWhiteSpace(snapshot.CommandUndoLabel)
                ? this.localization.Format("AssetEditor.Snapshot.UndoAvailable", snapshot.CommandUndoLabel)
                : string.Empty);
    }

    private string BuildAssetPathUnavailableMessage()
    {
        return this.localization.Text("AssetEditor.Dialog.AssetPathUnavailable");
    }

    private string BuildStudioHostMissingMessage()
    {
        return this.localization.Text("AssetEditor.Dialog.StudioHostMissing");
    }

    private string BuildStudioLaunchFailedMessage()
    {
        return this.localization.Text("AssetEditor.Dialog.StudioLaunchFailed");
    }

    private string BuildOpenProjectFirstMessage()
    {
        return this.localization.Text("AssetEditor.Dialog.OpenProjectFirst");
    }

    private string BuildWorkflowLauncherMessage(string message, string launcherPath)
    {
        return this.localization.Format("AssetEditor.Dialog.WorkflowLauncher", message, launcherPath);
    }

    private void LaunchStudio()
    {
        if (embeddedStudioShell)
        {
            return;
        }

        if (string.IsNullOrWhiteSpace(currentPath) || !File.Exists(currentPath))
        {
            MessageBox.Show(this, BuildAssetPathUnavailableMessage(), "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (studioHostPath is null)
        {
            MessageBox.Show(
                this,
                BuildStudioHostMissingMessage(),
                "Copperfin",
                MessageBoxButtons.OK,
                MessageBoxIcon.Warning);
            return;
        }

        if (!CopperfinStudioHostBridge.Launch(studioHostPath, currentPath!))
        {
            MessageBox.Show(this, BuildStudioLaunchFailedMessage(), "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }
    }

    private void RevealInExplorer()
    {
        if (string.IsNullOrWhiteSpace(currentPath) || !File.Exists(currentPath))
        {
            return;
        }

        var startInfo = new ProcessStartInfo
        {
            FileName = "explorer.exe",
            Arguments = $"/select,\"{currentPath}\"",
            UseShellExecute = true
        };

        _ = Process.Start(startInfo);
    }

    private void ApplyHostMode()
    {
        launchButton.Visible = !embeddedStudioShell;
        buildButton.Visible = !embeddedStudioShell && CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
        runButton.Visible = !embeddedStudioShell && CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
        debugButton.Visible = !embeddedStudioShell && CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
        subtitleLabel.Text = embeddedStudioShell
            ? this.localization.Text("AssetEditor.StandaloneSubtitle")
            : this.localization.Text("AssetEditor.Subtitle");
    }

    private void QueueUiAction(Func<Task> action)
    {
        _ = QueueUiActionAsync(action);
    }

    private async Task QueueUiActionAsync(Func<Task> action)
    {
        try
        {
            await action();
        }
        catch (ObjectDisposedException)
        {
        }
        catch (InvalidOperationException)
        {
        }
        catch (System.ComponentModel.Win32Exception)
        {
        }
        catch (Exception ex)
        {
            if (IsDisposed || Disposing || snapshotStatusLabel.IsDisposed)
            {
                return;
            }

            snapshotStatusLabel.Text = ex.Message;
            MessageBox.Show(this, ex.Message, "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }
    }

    private async Task RunProjectWorkflowAsync(CopperfinProjectOperation operation)
    {
        if (!CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath))
        {
            MessageBox.Show(this, BuildOpenProjectFirstMessage(), "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Information);
            return;
        }

        if (operation == CopperfinProjectOperation.Debug)
        {
            await StartDebugSessionAsync();
            return;
        }

        var result = await CopperfinProjectWorkflow.ExecuteAsync(currentPath!, operation);
        snapshotStatusLabel.Text = result.Message;
        if (!result.Success)
        {
            MessageBox.Show(this, result.Message, "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        if (operation == CopperfinProjectOperation.Build)
        {
            MessageBox.Show(
                this,
                BuildWorkflowLauncherMessage(result.Message, result.LauncherPath),
                "Copperfin",
                MessageBoxButtons.OK,
                MessageBoxIcon.Information);
        }
    }

    private async Task StartDebugSessionAsync()
    {
        if (!CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath))
        {
            return;
        }

        debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.StartingStatus");
        SetDebuggerButtonsEnabled(false);
        var session = await CopperfinRuntimeDebugClient.StartSessionAsync(currentPath!);
        if (IsDisposed || Disposing || projectWorkspaceTabs.IsDisposed)
        {
            return;
        }
        ApplyDebugSession(session);
    }

    private async Task AdvanceDebugSessionAsync(Func<CopperfinRuntimeDebugSession, Task<CopperfinRuntimeDebugSession>> action)
    {
        if (currentDebugSession is null || !currentDebugSession.Success)
        {
            MessageBox.Show(this, this.localization.Text("AssetEditor.Debugger.StartSessionFirstMessage"), "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Information);
            return;
        }

        debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.UpdatingStatus");
        SetDebuggerButtonsEnabled(false);
        var session = await action(currentDebugSession);
        if (IsDisposed || Disposing || projectWorkspaceTabs.IsDisposed)
        {
            return;
        }
        ApplyDebugSession(session);
    }

    private void ApplyDebugSession(CopperfinRuntimeDebugSession session)
    {
        try
        {
            if (IsDisposed || Disposing || projectWorkspaceTabs.IsDisposed || debuggerSummaryBox.IsDisposed || debuggerStatusLabel.IsDisposed)
            {
                return;
            }

            currentDebugSession = session;
            if (!session.Success)
            {
                debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.UnavailableStatus");
                debuggerSummaryBox.Text = session.Error;
                SetDebuggerButtonsEnabled(false);
                MessageBox.Show(this, session.Error, "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            debuggerStatusLabel.Text = session.State.Message;
            debuggerSummaryBox.Text = BuildDebugSessionSummary(session);
            if (currentSnapshot?.ProjectWorkspace is not null && currentSnapshot.AssetFamily == "project")
            {
                coverageSummaryBox.Text = BuildCoverageSummary(currentSnapshot, session);
            }
            if (projectWorkspaceTabs.Visible)
            {
                projectWorkspaceTabs.SelectedIndex = 1;
            }

            var canContinue = !string.Equals(session.State.Reason, "completed", StringComparison.OrdinalIgnoreCase) &&
                              !string.Equals(session.State.Reason, "error", StringComparison.OrdinalIgnoreCase);
            SetDebuggerButtonsEnabled(canContinue);
            debugRestartButton.Enabled = true;
        }
        catch (ObjectDisposedException)
        {
        }
        catch (InvalidOperationException)
        {
        }
        catch (System.ComponentModel.Win32Exception)
        {
        }
    }

    private void SetDebuggerButtonsEnabled(bool enabled)
    {
        debugContinueButton.Enabled = enabled;
        debugStepButton.Enabled = enabled;
        debugNextButton.Enabled = enabled;
        debugOutButton.Enabled = enabled;
        debugRestartButton.Enabled = CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
    }

    private void UpdateProjectCommandVisibility()
    {
        var showProjectActions = CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
        buildButton.Visible = showProjectActions;
        runButton.Visible = showProjectActions;
        debugButton.Visible = showProjectActions;
        buildButton.Enabled = showProjectActions;
        runButton.Enabled = showProjectActions;
        debugButton.Enabled = showProjectActions;
    }

    private void ConfigureObjectColumns()
    {
        objectListView.Columns[0].Text = currentSnapshot?.AssetFamily == "project"
            ? this.localization.Text("AssetEditor.Column.Item")
            : this.localization.Text("AssetEditor.Column.Object");
        objectListView.Columns[1].Text = currentSnapshot?.AssetFamily == "project"
            ? this.localization.Text("AssetEditor.Column.Group")
            : this.localization.Text("AssetEditor.Column.Type");
        objectListView.Columns[2].Text = this.localization.Text("AssetEditor.Column.Record");
    }

    private IEnumerable<CopperfinStudioSnapshotObject> GetVisibleObjects()
    {
        if (currentSnapshot is null)
        {
            return Array.Empty<CopperfinStudioSnapshotObject>();
        }

        if (currentSnapshot.AssetFamily != "project")
        {
            return currentSnapshot.Objects;
        }

        var selectedGroup = sectionListView.SelectedItems
            .Cast<ListViewItem>()
            .Select(item => item.Tag as CopperfinStudioProjectGroup)
            .FirstOrDefault(item => item is not null);
        if (selectedGroup is null)
        {
            return currentSnapshot.Objects;
        }

        var includedRecords = selectedGroup.RecordIndexes.ToHashSet();
        return currentSnapshot.Objects.Where(item => includedRecords.Contains(item.RecordIndex)).ToList();
    }

    private CopperfinStudioProjectEntry? LookupProjectEntry(int recordIndex)
    {
        return currentSnapshot?.ProjectWorkspace?.Entries.FirstOrDefault(entry => entry.RecordIndex == recordIndex);
    }

    private static string BuildProjectWorkspaceSummary(CopperfinStudioProjectWorkspace workspace)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Project Workspace");
        summary.AppendLine();
        summary.AppendLine($"Project: {workspace.ProjectTitle}");
        if (!string.IsNullOrWhiteSpace(workspace.ProjectKey))
        {
            summary.AppendLine($"Key: {workspace.ProjectKey}");
        }
        if (!string.IsNullOrWhiteSpace(workspace.HomeDirectory))
        {
            summary.AppendLine($"Home Directory: {workspace.HomeDirectory}");
        }
        summary.AppendLine($"Planned Output: {workspace.BuildPlan.OutputPath}");
        summary.AppendLine($"Build Target: {workspace.BuildPlan.BuildTarget}");
        summary.AppendLine($"Startup Item: {workspace.BuildPlan.StartupItem}");
        summary.AppendLine($"Items: {workspace.BuildPlan.TotalItems} total, {workspace.BuildPlan.ExcludedItems} excluded");
        summary.AppendLine($"Debug: {workspace.BuildPlan.DebugEnabled}");
        summary.AppendLine($"Encrypt: {workspace.BuildPlan.EncryptEnabled}");
        summary.AppendLine($"Save Code: {workspace.BuildPlan.SaveCode}");
        summary.AppendLine($"No Logo: {workspace.BuildPlan.NoLogo}");
        summary.AppendLine();
        summary.AppendLine("Groups:");
        foreach (var group in workspace.Groups)
        {
            summary.AppendLine($"- {group.Title}: {group.ItemCount} item(s), {group.ExcludedCount} excluded");
        }

        summary.AppendLine();
        summary.AppendLine("Next build-workflow step:");
        summary.AppendLine("Copperfin can now inspect the project structure, launch build/run workflows, and surface a first integrated debugger pane from the shared project workspace.");
        return summary.ToString();
    }

    private string BuildProjectWorkspaceSummary(CopperfinStudioSnapshotDocument snapshot)
    {
        var summary = new StringBuilder(BuildProjectWorkspaceSummary(snapshot.ProjectWorkspace!));

        if (snapshot.SecurityProfile.Available)
        {
            summary.AppendLine();
            summary.AppendLine("Native Security:");
            summary.AppendLine($"- Mode: {snapshot.SecurityProfile.Mode}");
            summary.AppendLine($"- Roles: {snapshot.SecurityProfile.Roles.Count}");
            summary.AppendLine($"- Identity Providers: {snapshot.SecurityProfile.IdentityProviders.Count}");
            summary.AppendLine($"- Package Policy: {snapshot.SecurityProfile.PackagePolicy}");
            summary.AppendLine($"- Managed Interop Policy: {snapshot.SecurityProfile.ManagedInteropPolicy}");
            if (snapshot.SecurityProfile.HardeningProfiles.Count > 0)
            {
                summary.AppendLine($"- Hardening: {snapshot.SecurityProfile.HardeningProfiles[0]}");
            }
        }

        if (snapshot.ExtensibilityProfile.Available)
        {
            summary.AppendLine();
            summary.AppendLine(".NET And Extensibility:");
            summary.AppendLine($"- .NET Story: {snapshot.ExtensibilityProfile.DotNetOutput.PrimaryStory}");
            summary.AppendLine($"- Languages: {snapshot.ExtensibilityProfile.Languages.Count}");
            summary.AppendLine($"- AI/MCP Features: {snapshot.ExtensibilityProfile.AiFeatures.Count}");
            var python = snapshot.ExtensibilityProfile.Languages.FirstOrDefault(language => language.Id == "python");
            if (python is not null)
            {
                summary.AppendLine($"- Python: {python.OutputStory}");
            }
            var rLanguage = snapshot.ExtensibilityProfile.Languages.FirstOrDefault(language => language.Id == "r");
            if (rLanguage is not null)
            {
                summary.AppendLine($"- R: {rLanguage.OutputStory}");
            }
            var mcp = snapshot.ExtensibilityProfile.AiFeatures.FirstOrDefault(feature => feature.Id == "mcp-host");
            if (mcp is not null)
            {
                summary.AppendLine($"- MCP: {mcp.Description}");
            }
            var modelSelection = snapshot.ExtensibilityProfile.AiFeatures.FirstOrDefault(feature => feature.Id == "model-selection");
            if (modelSelection is not null)
            {
                summary.AppendLine($"- AI Model Selection: {modelSelection.Description}");
            }
        }

        if (snapshot.DatabaseProfile.Available)
        {
            summary.AppendLine();
            summary.AppendLine("Database Federation:");
            summary.AppendLine($"- Connectors: {snapshot.DatabaseProfile.Connectors.Count}");
            summary.AppendLine($"- Query Paths: {snapshot.DatabaseProfile.QueryPaths.Count}");
            var directRelational = snapshot.DatabaseProfile.Connectors.Count(connector => connector.FoxSqlTranslationDirect);
            summary.AppendLine($"- Direct Fox SQL Targets: {directRelational}");
            var aiOptional = snapshot.DatabaseProfile.QueryPaths.Count(path => path.AiOptional);
            summary.AppendLine($"- Optional AI Planning Paths: {aiOptional}");
        }

        return summary.ToString();
    }

    private static string BuildDebugSessionSummary(CopperfinRuntimeDebugSession session)
    {
        var state = session.State;
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Debug Session");
        summary.AppendLine();
        summary.AppendLine($"Pause Reason: {state.Reason}");
        summary.AppendLine($"Location: {state.Location}");
        summary.AppendLine($"Statement: {state.Statement}");
        summary.AppendLine($"Message: {state.Message}");
        summary.AppendLine($"Executed Statements: {state.ExecutedStatements}");
        summary.AppendLine($"Command History: {string.Join(", ", session.Commands)}");

        summary.AppendLine();
        summary.AppendLine("Call Stack:");
        if (state.Frames.Count == 0)
        {
            summary.AppendLine("- (no frames)");
        }
        else
        {
            foreach (var frame in state.Frames)
            {
                summary.AppendLine($"- {frame.RoutineName} @ {frame.Location}");
                if (frame.Locals.Count == 0)
                {
                    summary.AppendLine("  locals: (none)");
                }
                else
                {
                    foreach (var local in frame.Locals)
                    {
                        summary.AppendLine($"  local {local.Name} = {local.Value}");
                    }
                }
            }
        }

        summary.AppendLine();
        summary.AppendLine("Globals:");
        if (state.Globals.Count == 0)
        {
            summary.AppendLine("- (none)");
        }
        else
        {
            foreach (var global in state.Globals)
            {
                summary.AppendLine($"- {global.Name} = {global.Value}");
            }
        }

        summary.AppendLine();
        summary.AppendLine("Runtime Events:");
        if (state.Events.Count == 0)
        {
            summary.AppendLine("- (none)");
        }
        else
        {
            foreach (var runtimeEvent in state.Events)
            {
                summary.AppendLine($"- [{runtimeEvent.Category}] {runtimeEvent.Detail} @ {runtimeEvent.Location}");
            }
        }

        return summary.ToString();
    }

    private static string BuildTaskListSummary(CopperfinProjectInsights? insights)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Task List");
        summary.AppendLine();
        if (insights is null)
        {
            summary.AppendLine("Project insights are not available.");
            return summary.ToString();
        }

        summary.AppendLine($"Project Root: {insights.ProjectRoot}");
        summary.AppendLine($"Tasks: {insights.TaskItems.Count}");
        if (insights.Warnings.Count > 0)
        {
            summary.AppendLine($"Warnings: {insights.Warnings.Count}");
        }

        summary.AppendLine();
        if (insights.TaskItems.Count == 0)
        {
            summary.AppendLine("No TODO/FIXME/HACK/BUG markers were found in the scanned text-based project files.");
        }
        else
        {
            foreach (var task in insights.TaskItems.Take(40))
            {
                summary.AppendLine($"- [{task.Category}] {Path.GetFileName(task.FilePath)}:{task.Line}  {task.Message}");
            }
            if (insights.TaskItems.Count > 40)
            {
                summary.AppendLine($"... {insights.TaskItems.Count - 40} more task item(s)");
            }
        }

        if (insights.Warnings.Count > 0)
        {
            summary.AppendLine();
            summary.AppendLine("Scan Warnings:");
            foreach (var warning in insights.Warnings.Take(10))
            {
                summary.AppendLine($"- {warning}");
            }
        }

        return summary.ToString();
    }

    private static string BuildCodeReferenceSummary(CopperfinProjectInsights? insights)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Code References");
        summary.AppendLine();
        if (insights is null)
        {
            summary.AppendLine("Project insights are not available.");
            return summary.ToString();
        }

        summary.AppendLine($"Project Root: {insights.ProjectRoot}");
        summary.AppendLine($"Definitions: {insights.DefinedSymbols.Count}");
        summary.AppendLine($"Runtime References: {insights.RuntimeReferences.Count}");
        summary.AppendLine();
        summary.AppendLine("Definitions:");
        if (insights.DefinedSymbols.Count == 0)
        {
            summary.AppendLine("- No textual definitions were found in the scanned project files.");
        }
        else
        {
            foreach (var symbol in insights.DefinedSymbols.Take(40))
            {
                summary.AppendLine($"- [{symbol.Kind}] {symbol.Name}  {Path.GetFileName(symbol.FilePath)}:{symbol.Line}");
            }
            if (insights.DefinedSymbols.Count > 40)
            {
                summary.AppendLine($"... {insights.DefinedSymbols.Count - 40} more definition(s)");
            }
        }

        summary.AppendLine();
        summary.AppendLine("Runtime References:");
        if (insights.RuntimeReferences.Count == 0)
        {
            summary.AppendLine("- No runtime references were found in the scanned project files.");
        }
        else
        {
            foreach (var symbol in insights.RuntimeReferences.Take(40))
            {
                summary.AppendLine($"- [{symbol.Kind}] {symbol.Name}  {Path.GetFileName(symbol.FilePath)}:{symbol.Line}");
            }
            if (insights.RuntimeReferences.Count > 40)
            {
                summary.AppendLine($"... {insights.RuntimeReferences.Count - 40} more runtime reference(s)");
            }
        }

        return summary.ToString();
    }

    private static string BuildDataExplorerSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinProjectInsights? insights, string? filter)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Data Explorer");
        summary.AppendLine();
        summary.AppendLine($"Project: {snapshot.ProjectWorkspace?.ProjectTitle}");
        if (insights is null)
        {
            summary.AppendLine("Project insights are not available.");
            return summary.ToString();
        }

        var normalizedFilter = (filter ?? string.Empty).Trim();
        var filteredAssets = insights.DataAssets
            .Where(asset =>
                string.IsNullOrWhiteSpace(normalizedFilter) ||
                asset.Title.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                asset.Kind.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                asset.FilePath.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0)
            .ToList();

        summary.AppendLine($"Discovered Data Assets: {insights.DataAssets.Count}");
        if (!string.IsNullOrWhiteSpace(normalizedFilter))
        {
            summary.AppendLine($"Filter: {normalizedFilter}");
        }
        summary.AppendLine();
        if (filteredAssets.Count == 0)
        {
            summary.AppendLine(string.IsNullOrWhiteSpace(normalizedFilter)
                ? "No DBF/DBC/query assets were discovered in the current project workspace."
                : "No data assets matched the current filter.");
        }
        else
        {
            foreach (var asset in filteredAssets.Take(40))
            {
                var excludedSuffix = asset.Excluded ? " [excluded]" : string.Empty;
                summary.AppendLine($"- [{asset.Kind}] {asset.Title}{excludedSuffix}");
                if (!string.IsNullOrWhiteSpace(asset.FilePath))
                {
                    summary.AppendLine($"  {asset.FilePath}");
                }
            }
            if (filteredAssets.Count > 40)
            {
                summary.AppendLine($"... {filteredAssets.Count - 40} more data asset(s)");
            }
        }

        summary.AppendLine();
        summary.AppendLine("Modern Connector Direction:");
        summary.AppendLine("- SQLite, PostgreSQL, SQL Server, and Oracle remain first-class targets alongside DBF/DBC assets.");
        summary.AppendLine("- Data-science jobs can flow through Python or R sidecars without weakening the trusted native core.");
        return summary.ToString();
    }

    private static string BuildObjectBrowserSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinProjectInsights? insights, string? filter, bool hideProjectRecords)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Object Browser");
        summary.AppendLine();
        summary.AppendLine($"Project: {snapshot.ProjectWorkspace?.ProjectTitle}");
        if (insights is null)
        {
            summary.AppendLine("Project insights are not available.");
            return summary.ToString();
        }

        var normalizedFilter = (filter ?? string.Empty).Trim();
        var filteredNodes = insights.ObjectNodes
            .Where(node =>
                (!hideProjectRecords || !string.Equals(node.Kind, "Project Header", StringComparison.OrdinalIgnoreCase)) &&
                (string.IsNullOrWhiteSpace(normalizedFilter) ||
                 node.Title.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                 node.Kind.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                 node.Detail.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                 node.FilePath.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0))
            .ToList();

        summary.AppendLine($"Object Nodes: {insights.ObjectNodes.Count}");
        summary.AppendLine($"Definitions: {insights.DefinedSymbols.Count}");
        if (!string.IsNullOrWhiteSpace(normalizedFilter))
        {
            summary.AppendLine($"Filter: {normalizedFilter}");
        }
        if (hideProjectRecords)
        {
            summary.AppendLine("Project records hidden: true");
        }
        summary.AppendLine();
        if (filteredNodes.Count == 0)
        {
            summary.AppendLine("No object-browser nodes matched the current view.");
        }
        else
        {
            foreach (var node in filteredNodes.Take(50))
            {
                summary.AppendLine($"- [{node.Kind}] {node.Title}");
                if (!string.IsNullOrWhiteSpace(node.Detail))
                {
                    summary.AppendLine($"  {node.Detail}");
                }
                if (!string.IsNullOrWhiteSpace(node.FilePath))
                {
                    summary.AppendLine($"  {Path.GetFileName(node.FilePath)}");
                }
            }
            if (filteredNodes.Count > 50)
            {
                summary.AppendLine($"... {filteredNodes.Count - 50} more object node(s)");
            }
        }

        summary.AppendLine();
        summary.AppendLine("Next browser step:");
        summary.AppendLine("- Promote these summaries into navigable designers and code-navigation surfaces shared by Visual Studio and Copperfin Studio.");
        return summary.ToString();
    }

    private static string BuildToolboxSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinProjectInsights? insights)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Toolbox And Add-ins");
        summary.AppendLine();
        summary.AppendLine($"Project: {snapshot.ProjectWorkspace?.ProjectTitle}");
        summary.AppendLine($"Workspace Groups: {snapshot.ProjectWorkspace?.Groups.Count ?? 0}");
        summary.AppendLine($"Language Integrations: {snapshot.ExtensibilityProfile.Languages.Count}");
        summary.AppendLine($"AI/MCP Features: {snapshot.ExtensibilityProfile.AiFeatures.Count}");
        summary.AppendLine();
        summary.AppendLine("Suggested Toolbox Seeds:");
        foreach (var group in snapshot.ProjectWorkspace?.Groups.Take(8) ?? Enumerable.Empty<CopperfinStudioProjectGroup>())
        {
            summary.AppendLine($"- {group.Title}: {group.ItemCount} project item(s)");
        }

        summary.AppendLine();
        summary.AppendLine("Add-in Surfaces:");
        foreach (var feature in snapshot.ExtensibilityProfile.AiFeatures.Take(6))
        {
            summary.AppendLine($"- {feature.Title}: {feature.Description}");
        }

        if (insights is not null && insights.RuntimeReferences.Count > 0)
        {
            summary.AppendLine();
            summary.AppendLine("High-value shortcuts:");
            foreach (var reference in insights.RuntimeReferences.Take(6))
            {
                summary.AppendLine($"- [{reference.Kind}] {reference.Name}");
            }
        }

        return summary.ToString();
    }

    private static string BuildBuilderSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinProjectInsights? insights)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Builders");
        summary.AppendLine();
        summary.AppendLine($"Project: {snapshot.ProjectWorkspace?.ProjectTitle}");
        summary.AppendLine("Recommended Builder Contexts:");
        summary.AppendLine("- Form/Class designers: property sheet, event wiring, and layout helpers");
        summary.AppendLine("- Report/Label designers: band, expression, print, and export helpers");
        summary.AppendLine("- Menus/Projects: startup, deployment, and migration helpers");
        summary.AppendLine("- Security/AI/MCP: policy-controlled setup builders");

        if (insights is not null)
        {
            summary.AppendLine();
            summary.AppendLine("Current Builder Targets:");
            foreach (var node in insights.ObjectNodes.Take(8))
            {
                summary.AppendLine($"- [{node.Kind}] {node.Title}");
            }
        }

        return summary.ToString();
    }

    private static string BuildCoverageSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinRuntimeDebugSession? session)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Coverage");
        summary.AppendLine();
        summary.AppendLine($"Project: {snapshot.ProjectWorkspace?.ProjectTitle}");
        if (session is null || !session.Success)
        {
            summary.AppendLine("Start a Copperfin debug session to collect first-pass runtime coverage signals.");
            return summary.ToString();
        }

        var state = session.State;
        var executedLocations = state.Events
            .Where(runtimeEvent => !string.IsNullOrWhiteSpace(runtimeEvent.Location))
            .Select(runtimeEvent => runtimeEvent.Location)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToList();

        summary.AppendLine($"Pause Reason: {state.Reason}");
        summary.AppendLine($"Executed Statements: {state.ExecutedStatements}");
        summary.AppendLine($"Distinct Runtime Locations: {executedLocations.Count}");
        summary.AppendLine();
        summary.AppendLine("Recent Coverage Signals:");
        foreach (var location in executedLocations.Take(12))
        {
            summary.AppendLine($"- {location}");
        }

        if (executedLocations.Count == 0)
        {
            summary.AppendLine("- No runtime locations captured yet.");
        }

        return summary.ToString();
    }

    private static string BuildDatabaseFederationSummary(CopperfinStudioSnapshotDocument snapshot, string? filter)
    {
        var summary = new StringBuilder();
        summary.AppendLine("Copperfin Database Federation");
        summary.AppendLine();
        if (!snapshot.DatabaseProfile.Available)
        {
            summary.AppendLine("Database federation metadata is unavailable.");
            return summary.ToString();
        }

        var normalizedFilter = (filter ?? string.Empty).Trim();
        var filteredConnectors = snapshot.DatabaseProfile.Connectors
            .Where(connector =>
                string.IsNullOrWhiteSpace(normalizedFilter) ||
                connector.Title.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                connector.Family.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                connector.SchemaShape.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0)
            .ToList();
        var filteredPaths = snapshot.DatabaseProfile.QueryPaths
            .Where(path =>
                string.IsNullOrWhiteSpace(normalizedFilter) ||
                path.Title.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                path.SourceShape.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                path.TargetShape.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0)
            .ToList();

        summary.AppendLine($"Connectors: {snapshot.DatabaseProfile.Connectors.Count}");
        summary.AppendLine($"Query Paths: {snapshot.DatabaseProfile.QueryPaths.Count}");
        if (!string.IsNullOrWhiteSpace(normalizedFilter))
        {
            summary.AppendLine($"Filter: {normalizedFilter}");
        }

        summary.AppendLine();
        summary.AppendLine("Connector Targets:");
        foreach (var connector in filteredConnectors.Take(10))
        {
            summary.AppendLine($"- [{connector.Family}] {connector.Title}");
            summary.AppendLine($"  {connector.TranslationStory}");
        }
        if (filteredConnectors.Count == 0)
        {
            summary.AppendLine("- No connector targets matched the current filter.");
        }

        summary.AppendLine();
        summary.AppendLine("Query Translation Paths:");
        foreach (var path in filteredPaths.Take(8))
        {
            summary.AppendLine($"- {path.Title} ({path.Complexity})");
            summary.AppendLine($"  {path.Strategy}");
        }
        if (filteredPaths.Count == 0)
        {
            summary.AppendLine("- No query translation paths matched the current filter.");
        }

        summary.AppendLine();
        summary.AppendLine("Guardrails:");
        foreach (var guardrail in snapshot.DatabaseProfile.Guardrails.Take(6))
        {
            summary.AppendLine($"- {guardrail}");
        }

        return summary.ToString();
    }
}
