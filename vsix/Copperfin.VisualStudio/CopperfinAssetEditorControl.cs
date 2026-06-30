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
    private sealed class ReportUnplacedObjectScope
    {
        public List<int> RecordIndexes { get; } = new();
    }

    private sealed class ExplorerSelectionState
    {
        public int? ReportSectionRecordIndex { get; set; }
        public bool ReportUnplacedObjects { get; set; }
        public string? ProjectGroupId { get; set; }
    }

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

        designSurface = new CopperfinDesignSurfaceControl(this.localization)
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
        var explorerSelection = CaptureExplorerSelectionState();
        var selectedObjectRecordIndex = TryReadSelectedRecordIndex();
        snapshotStatusLabel.Text = BuildUndoExecutingStatus(priorLabel);

        var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(currentPath!);
        if (!undoResult.Success || undoResult.Document is null)
        {
            snapshotStatusLabel.Text = BuildUndoFailedStatus(undoResult.Error);
            return false;
        }

        currentSnapshot = undoResult.Document;
        snapshotStatusLabel.Text = BuildUndoCompletedStatus(priorLabel, currentSnapshot);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        if (selectedObjectRecordIndex >= 0)
        {
            designSurface.SelectRecord(selectedObjectRecordIndex);
            SyncSelectionFromSurface(selectedObjectRecordIndex);
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
        detailsLabel.Text = BuildSnapshotDetailsText(info, null);
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
            detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(path), currentSnapshot);
            snapshotStatusLabel.Text = BuildSnapshotLoadedStatus(currentSnapshot);
            guidanceLabel.Text = BuildGuidanceText(currentSnapshot.AssetFamily);
            UpdateProjectCommandVisibility();
            PopulateSectionList();
            SyncExplorerSelection();
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

    private void PopulateSectionList(ExplorerSelectionState? selectionState = null)
    {
        sectionListView.BeginUpdate();
        sectionListView.Items.Clear();

        if (currentSnapshot?.ReportLayout is { } reportLayout &&
            (reportLayout.Sections.Count > 0 || reportLayout.DeletedSections.Count > 0 || reportLayout.UnplacedObjects.Count > 0))
        {
            sectionListView.Visible = true;
            leftExplorerSplit.Panel1Collapsed = false;
            sectionListView.Columns[0].Text = this.localization.Text("AssetEditor.Column.Section");
            sectionListView.Columns[1].Text = this.localization.Text("AssetEditor.Column.Objects");
            sectionListView.Columns[2].Text = this.localization.Text("AssetEditor.Column.Top");
            foreach (var section in reportLayout.Sections)
            {
                var item = new ListViewItem(BuildReportSectionListTitle(section));
                item.SubItems.Add(section.Objects.Count.ToString());
                item.SubItems.Add(section.Top.ToString());
                item.Tag = section;
                sectionListView.Items.Add(item);
            }

            foreach (var section in reportLayout.DeletedSections)
            {
                var item = new ListViewItem(BuildDeletedReportSectionListTitle(section));
                item.SubItems.Add(section.Objects.Count.ToString());
                item.SubItems.Add(section.Top.ToString());
                item.Tag = section;
                item.ForeColor = Color.Firebrick;
                sectionListView.Items.Add(item);
            }

            if (reportLayout.UnplacedObjects.Count > 0)
            {
                var unplacedScope = new ReportUnplacedObjectScope();
                unplacedScope.RecordIndexes.AddRange(reportLayout.UnplacedObjects.Select(item => item.RecordIndex));

                var item = new ListViewItem(L("AssetEditor.ReportSection.UnplacedObjects"));
                item.SubItems.Add(reportLayout.UnplacedObjects.Count.ToString());
                item.SubItems.Add(string.Empty);
                item.Tag = unplacedScope;
                sectionListView.Items.Add(item);
            }

            ApplyExplorerSelectionState(selectionState);

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

            ApplyExplorerSelectionState(selectionState);

            sectionListView.EndUpdate();
            return;
        }

        sectionListView.Visible = false;
        leftExplorerSplit.Panel1Collapsed = true;
        sectionListView.EndUpdate();
    }

    private void PopulateObjectList(bool autoSelectFirstItem = true)
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

        if (autoSelectFirstItem && objectListView.Items.Count > 0)
        {
            objectListView.Items[0].Selected = true;
        }
        else if (objectListView.Items.Count == 0)
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

        try
        {
            suppressSelectionSync = true;
            var selectedExplorerTag = TryReadSelectedExplorerTag();
            var isReportSectionSelection = selectedExplorerTag is CopperfinStudioReportSection || selectedExplorerTag is ReportUnplacedObjectScope;
            PopulateObjectList(autoSelectFirstItem: !isReportSectionSelection);

            if (selectedExplorerTag is CopperfinStudioReportSection reportSection)
            {
                foreach (ListViewItem item in objectListView.Items)
                {
                    item.Selected = false;
                }

                propertyGrid.SelectedObject = CopperfinDesignerSelection.FromReportSection(reportSection, localization);
                designSurface.SelectRecord(null);
                return;
            }

            if (selectedExplorerTag is ReportUnplacedObjectScope)
            {
                foreach (ListViewItem item in objectListView.Items)
                {
                    item.Selected = false;
                }

                propertyGrid.SelectedObject = null;
                designSurface.SelectRecord(null);
                return;
            }
        }
        finally
        {
            suppressSelectionSync = false;
        }
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

        var explorerSelection = CaptureExplorerSelectionState();
        var selectedObjectRecordIndex = TryReadSelectedRecordIndex();
        snapshotStatusLabel.Text = BuildPropertyApplyingStatus(propertyName);
        var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(currentPath!, recordIndex, propertyName, propertyValue);
        if (!updateResult.Success || updateResult.Document is null)
        {
            snapshotStatusLabel.Text = BuildPropertyUpdateFailedStatus(updateResult.Error);
            return;
        }

        currentSnapshot = updateResult.Document;
        snapshotStatusLabel.Text = BuildPropertyUpdatedStatus(propertyName, currentSnapshot);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        if (selectedObjectRecordIndex >= 0)
        {
            designSurface.SelectRecord(selectedObjectRecordIndex);
            SyncSelectionFromSurface(selectedObjectRecordIndex);
        }
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
            MessageBox.Show(this, BuildAssetPathUnavailableMessage(), DialogTitle, MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (studioHostPath is null)
        {
            MessageBox.Show(
                this,
                BuildStudioHostMissingMessage(),
                DialogTitle,
                MessageBoxButtons.OK,
                MessageBoxIcon.Warning);
            return;
        }

        if (!CopperfinStudioHostBridge.Launch(studioHostPath, currentPath!))
        {
            MessageBox.Show(this, BuildStudioLaunchFailedMessage(), DialogTitle, MessageBoxButtons.OK, MessageBoxIcon.Warning);
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
            MessageBox.Show(this, ex.Message, DialogTitle, MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }
    }

    private async Task RunProjectWorkflowAsync(CopperfinProjectOperation operation)
    {
        if (!CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath))
        {
            MessageBox.Show(this, BuildOpenProjectFirstMessage(), DialogTitle, MessageBoxButtons.OK, MessageBoxIcon.Information);
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
            MessageBox.Show(this, result.Message, DialogTitle, MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        if (operation == CopperfinProjectOperation.Build)
        {
            MessageBox.Show(
                this,
                BuildWorkflowLauncherMessage(result.Message, result.LauncherPath),
                DialogTitle,
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
            MessageBox.Show(this, this.localization.Text("AssetEditor.Debugger.StartSessionFirstMessage"), DialogTitle, MessageBoxButtons.OK, MessageBoxIcon.Information);
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
                MessageBox.Show(this, session.Error, DialogTitle, MessageBoxButtons.OK, MessageBoxIcon.Warning);
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
            if ((currentSnapshot.AssetFamily == "report" || currentSnapshot.AssetFamily == "label") &&
                currentSnapshot.ReportLayout is not null)
            {
                var selectedSection = TryReadSelectedExplorerTag();

                if (selectedSection is CopperfinStudioReportSection reportSection)
                {
                    var sectionRecords = reportSection.Objects.Select(item => item.RecordIndex).ToHashSet();
                    return currentSnapshot.Objects.Where(item => sectionRecords.Contains(item.RecordIndex)).ToList();
                }

                if (selectedSection is ReportUnplacedObjectScope unplacedScope)
                {
                    var unplacedRecords = unplacedScope.RecordIndexes.ToHashSet();
                    return currentSnapshot.Objects.Where(item => unplacedRecords.Contains(item.RecordIndex)).ToList();
                }
            }

            return currentSnapshot.Objects;
        }

        var selectedGroup = TryReadSelectedExplorerTag() as CopperfinStudioProjectGroup;
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

    private object? TryReadSelectedExplorerTag()
    {
        var selectedTag = sectionListView.SelectedItems
            .Cast<ListViewItem>()
            .Select(item => item.Tag)
            .FirstOrDefault(item => item is not null);
        if (selectedTag is not null)
        {
            return selectedTag;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            if (item.Selected)
            {
                return item.Tag;
            }
        }

        return sectionListView.Items.Count > 0 ? sectionListView.Items[0].Tag : null;
    }

    private ExplorerSelectionState? CaptureExplorerSelectionState()
    {
        var selectedTag = TryReadSelectedExplorerTag();
        if (selectedTag is CopperfinStudioReportSection reportSection)
        {
            return new ExplorerSelectionState
            {
                ReportSectionRecordIndex = reportSection.RecordIndex
            };
        }

        if (selectedTag is ReportUnplacedObjectScope)
        {
            return new ExplorerSelectionState
            {
                ReportUnplacedObjects = true
            };
        }

        if (selectedTag is CopperfinStudioProjectGroup projectGroup)
        {
            return new ExplorerSelectionState
            {
                ProjectGroupId = projectGroup.Id
            };
        }

        return null;
    }

    private void ApplyExplorerSelectionState(ExplorerSelectionState? selectionState)
    {
        ListViewItem? selectedItem = null;
        if (selectionState?.ReportSectionRecordIndex is int reportSectionRecordIndex)
        {
            selectedItem = sectionListView.Items
                .Cast<ListViewItem>()
                .FirstOrDefault(item => item.Tag is CopperfinStudioReportSection section &&
                                        section.RecordIndex == reportSectionRecordIndex);
        }
        else if (selectionState?.ReportUnplacedObjects == true)
        {
            selectedItem = sectionListView.Items
                .Cast<ListViewItem>()
                .FirstOrDefault(item => item.Tag is ReportUnplacedObjectScope);
        }
        else if (!string.IsNullOrWhiteSpace(selectionState?.ProjectGroupId))
        {
            var projectGroupId = selectionState!.ProjectGroupId!;
            selectedItem = sectionListView.Items
                .Cast<ListViewItem>()
                .FirstOrDefault(item => item.Tag is CopperfinStudioProjectGroup group &&
                                        string.Equals(group.Id, projectGroupId, StringComparison.Ordinal));
        }

        if (selectedItem is null && sectionListView.Items.Count > 0)
        {
            selectedItem = sectionListView.Items[0];
        }

        if (selectedItem is not null)
        {
            selectedItem.Selected = true;
        }
    }

    private string BuildSnapshotDetailsText(FileInfo info, CopperfinStudioSnapshotDocument? snapshot)
    {
        var details = F(
            "AssetEditor.Details.FileMetadata",
            info.Length,
            info.LastWriteTime,
            info.Extension.ToLowerInvariant());

        if (snapshot?.ReportLayout is null || (snapshot.AssetFamily != "report" && snapshot.AssetFamily != "label"))
        {
            return details;
        }

        details += Environment.NewLine + F(
            "AssetEditor.Details.ReportLayoutSummary",
            snapshot.ReportLayout.Sections.Count,
            snapshot.ReportLayout.Groupings.Count,
            snapshot.ReportLayout.Settings.Count,
            snapshot.ReportLayout.UnplacedObjects.Count);

        if (snapshot.ReportLayout.PreviewBoundsAvailable)
        {
            details += Environment.NewLine + F(
                "AssetEditor.Details.ReportPreviewBoundsSummary",
                snapshot.ReportLayout.PreviewBoundsLeft,
                snapshot.ReportLayout.PreviewBoundsTop,
                snapshot.ReportLayout.PreviewBoundsRight,
                snapshot.ReportLayout.PreviewBoundsBottom,
                snapshot.ReportLayout.PreviewBoundsWidth,
                snapshot.ReportLayout.PreviewBoundsHeight);
        }

        if (snapshot.ReportLayout.DeletedPreviewBoundsAvailable)
        {
            details += Environment.NewLine + F(
                "AssetEditor.Details.DeletedReportPreviewBoundsSummary",
                snapshot.ReportLayout.DeletedPreviewBoundsLeft,
                snapshot.ReportLayout.DeletedPreviewBoundsTop,
                snapshot.ReportLayout.DeletedPreviewBoundsRight,
                snapshot.ReportLayout.DeletedPreviewBoundsBottom,
                snapshot.ReportLayout.DeletedPreviewBoundsWidth,
                snapshot.ReportLayout.DeletedPreviewBoundsHeight);
        }

        return details;
    }

    private string BuildReportSectionListTitle(CopperfinStudioReportSection section)
    {
        if (!section.GroupingContextAvailable || string.IsNullOrWhiteSpace(section.GroupingExpression))
        {
            return section.Title ?? string.Empty;
        }

        return F(
            "AssetEditor.ReportSection.WithGroupingExpression",
            section.Title ?? string.Empty,
            section.GroupingExpression ?? string.Empty);
    }

    private string BuildDeletedReportSectionListTitle(CopperfinStudioReportSection section)
    {
        return F("AssetEditor.ReportSection.Deleted", BuildReportSectionListTitle(section));
    }

    private string L(string key) => this.localization.Text(key);

    private string F(string key, params object[] args) => this.localization.Format(key, args);

    private string DialogTitle => L("AssetEditor.Title");

    private string BuildProjectWorkspaceSummary(CopperfinStudioProjectWorkspace workspace)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.ProjectWorkspace"));
        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.LabelProject")}: {workspace.ProjectTitle}");
        if (!string.IsNullOrWhiteSpace(workspace.ProjectKey))
        {
            summary.AppendLine($"{L("AssetEditor.Summary.LabelKey")}: {workspace.ProjectKey}");
        }
        if (!string.IsNullOrWhiteSpace(workspace.HomeDirectory))
        {
            summary.AppendLine($"{L("AssetEditor.Summary.LabelHomeDirectory")}: {workspace.HomeDirectory}");
        }
        summary.AppendLine($"{L("AssetEditor.Summary.LabelPlannedOutput")}: {workspace.BuildPlan.OutputPath}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelBuildTarget")}: {workspace.BuildPlan.BuildTarget}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelStartupItem")}: {workspace.BuildPlan.StartupItem}");
        summary.AppendLine(
            F(
                "AssetEditor.Summary.LabelItems",
                workspace.BuildPlan.TotalItems,
                workspace.BuildPlan.ExcludedItems));
        summary.AppendLine($"{L("AssetEditor.Summary.LabelDebug")}: {workspace.BuildPlan.DebugEnabled}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelEncrypt")}: {workspace.BuildPlan.EncryptEnabled}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelSaveCode")}: {workspace.BuildPlan.SaveCode}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelNoLogo")}: {workspace.BuildPlan.NoLogo}");
        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.GroupsHeading"));
        foreach (var group in workspace.Groups)
        {
            summary.AppendLine(
                F(
                    "AssetEditor.Summary.GroupLine",
                    group.Title,
                    group.ItemCount,
                    group.ExcludedCount));
        }

        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.BuildWorkflowHeading")}:");
        summary.AppendLine(L("AssetEditor.Summary.BuildWorkflowText"));
        return summary.ToString();
    }

    private string BuildProjectWorkspaceSummary(CopperfinStudioSnapshotDocument snapshot)
    {
        var summary = new StringBuilder(BuildProjectWorkspaceSummary(snapshot.ProjectWorkspace!));

        if (snapshot.SecurityProfile.Available)
        {
            summary.AppendLine();
            summary.AppendLine(L("AssetEditor.Summary.NativeSecurity"));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelMode"), snapshot.SecurityProfile.Mode));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelRoles"), snapshot.SecurityProfile.Roles.Count));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelIdentityProviders"), snapshot.SecurityProfile.IdentityProviders.Count));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelPackagePolicy"), snapshot.SecurityProfile.PackagePolicy));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelManagedInteropPolicy"), snapshot.SecurityProfile.ManagedInteropPolicy));
            if (snapshot.SecurityProfile.HardeningProfiles.Count > 0)
            {
                summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelHardening"), snapshot.SecurityProfile.HardeningProfiles[0]));
            }
        }

        if (snapshot.ExtensibilityProfile.Available)
        {
            summary.AppendLine();
            summary.AppendLine(L("AssetEditor.Summary.DotNetExtensibility"));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelDotNetStory"), snapshot.ExtensibilityProfile.DotNetOutput.PrimaryStory));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelLanguages"), snapshot.ExtensibilityProfile.Languages.Count));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelAiFeatures"), snapshot.ExtensibilityProfile.AiFeatures.Count));
            var python = snapshot.ExtensibilityProfile.Languages.FirstOrDefault(language => language.Id == "python");
            if (python is not null)
            {
                summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelPython"), python.OutputStory));
            }
            var rLanguage = snapshot.ExtensibilityProfile.Languages.FirstOrDefault(language => language.Id == "r");
            if (rLanguage is not null)
            {
                summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelR"), rLanguage.OutputStory));
            }
            var mcp = snapshot.ExtensibilityProfile.AiFeatures.FirstOrDefault(feature => feature.Id == "mcp-host");
            if (mcp is not null)
            {
                summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelMcp"), mcp.Description));
            }
            var modelSelection = snapshot.ExtensibilityProfile.AiFeatures.FirstOrDefault(feature => feature.Id == "model-selection");
            if (modelSelection is not null)
            {
                summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelAiModelSelection"), modelSelection.Description));
            }
        }

        if (snapshot.DatabaseProfile.Available)
        {
            summary.AppendLine();
            summary.AppendLine(L("AssetEditor.Summary.DatabaseFederation"));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelConnectors"), snapshot.DatabaseProfile.Connectors.Count));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelQueryPaths"), snapshot.DatabaseProfile.QueryPaths.Count));
            var directRelational = snapshot.DatabaseProfile.Connectors.Count(connector => connector.FoxSqlTranslationDirect);
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelDirectFoxSqlTargets"), directRelational));
            var aiOptional = snapshot.DatabaseProfile.QueryPaths.Count(path => path.AiOptional);
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelOptionalAiPlanningPaths"), aiOptional));
        }

        return summary.ToString();
    }

    private string BuildDebugSessionSummary(CopperfinRuntimeDebugSession session)
    {
        var state = session.State;
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.DebugSession"));
        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.LabelPauseReason")}: {state.Reason}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelLocation")}: {state.Location}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelStatement")}: {state.Statement}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelMessage")}: {state.Message}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelExecutedStatements")}: {state.ExecutedStatements}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelCommandHistory")}: {string.Join(", ", session.Commands)}");

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.CallStack"));
        if (state.Frames.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoFrames"));
        }
        else
        {
            foreach (var frame in state.Frames)
            {
                summary.AppendLine($"- {frame.RoutineName} @ {frame.Location}");
                if (frame.Locals.Count == 0)
                {
                    summary.AppendLine(F("AssetEditor.Summary.FrameLocalsNone", L("AssetEditor.Summary.LabelLocals")));
                }
                else
                {
                    foreach (var local in frame.Locals)
                    {
                        summary.AppendLine($"{L("AssetEditor.Summary.FrameLocal")}: {local.Name} = {local.Value}");
                    }
                }
            }
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.Globals"));
        if (state.Globals.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoGlobals"));
        }
        else
        {
            foreach (var global in state.Globals)
            {
                summary.AppendLine($"- {global.Name} = {global.Value}");
            }
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.RuntimeEvents"));
        if (state.Events.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoRuntimeEvents"));
        }
        else
        {
            foreach (var runtimeEvent in state.Events)
            {
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.RuntimeEventLine",
                        runtimeEvent.Category,
                        runtimeEvent.Detail,
                        runtimeEvent.Location));
            }
        }

        return summary.ToString();
    }

    private string BuildTaskListSummary(CopperfinProjectInsights? insights)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.TaskList"));
        summary.AppendLine();
        if (insights is null)
        {
            summary.AppendLine(L("AssetEditor.Summary.ProjectInsightsUnavailable"));
            return summary.ToString();
        }

        summary.AppendLine($"{L("AssetEditor.Summary.LabelProjectRoot")}: {insights.ProjectRoot}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelTasks")}: {insights.TaskItems.Count}");
        if (insights.Warnings.Count > 0)
        {
            summary.AppendLine($"{L("AssetEditor.Summary.LabelWarnings")}: {insights.Warnings.Count}");
        }

        summary.AppendLine();
        if (insights.TaskItems.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoTaskItems"));
        }
        else
        {
            foreach (var task in insights.TaskItems.Take(40))
            {
                summary.AppendLine($"- [{task.Category}] {Path.GetFileName(task.FilePath)}:{task.Line}  {task.Message}");
            }
            if (insights.TaskItems.Count > 40)
            {
                summary.AppendLine(F("AssetEditor.Summary.MoreItems", insights.TaskItems.Count - 40, L("AssetEditor.Summary.LabelTaskItems")));
            }
        }

        if (insights.Warnings.Count > 0)
        {
            summary.AppendLine();
            summary.AppendLine(L("AssetEditor.Summary.ScanWarnings"));
            foreach (var warning in insights.Warnings.Take(10))
            {
                summary.AppendLine($"- {warning}");
            }
        }

        return summary.ToString();
    }

    private string BuildCodeReferenceSummary(CopperfinProjectInsights? insights)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.CodeReferences"));
        summary.AppendLine();
        if (insights is null)
        {
            summary.AppendLine(L("AssetEditor.Summary.ProjectInsightsUnavailable"));
            return summary.ToString();
        }

        summary.AppendLine($"{L("AssetEditor.Summary.LabelProjectRoot")}: {insights.ProjectRoot}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelDefinitions")}: {insights.DefinedSymbols.Count}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelRuntimeReferences")}: {insights.RuntimeReferences.Count}");
        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.Definitions"));
        if (insights.DefinedSymbols.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoDefinitions"));
        }
        else
        {
            foreach (var symbol in insights.DefinedSymbols.Take(40))
            {
                summary.AppendLine($"- [{symbol.Kind}] {symbol.Name}  {Path.GetFileName(symbol.FilePath)}:{symbol.Line}");
            }
            if (insights.DefinedSymbols.Count > 40)
            {
                summary.AppendLine(F("AssetEditor.Summary.MoreItems", insights.DefinedSymbols.Count - 40, L("AssetEditor.Summary.LabelDefinitions")));
            }
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.RuntimeReferences"));
        if (insights.RuntimeReferences.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoRuntimeReferences"));
        }
        else
        {
            foreach (var symbol in insights.RuntimeReferences.Take(40))
            {
                summary.AppendLine($"- [{symbol.Kind}] {symbol.Name}  {Path.GetFileName(symbol.FilePath)}:{symbol.Line}");
            }
            if (insights.RuntimeReferences.Count > 40)
            {
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.MoreItems",
                        insights.RuntimeReferences.Count - 40,
                        L("AssetEditor.Summary.LabelRuntimeReferences")));
            }
        }

        return summary.ToString();
    }

    private string BuildDataExplorerSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinProjectInsights? insights, string? filter)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.DataExplorer"));
        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.LabelProject")}: {snapshot.ProjectWorkspace?.ProjectTitle}");
        if (insights is null)
        {
            summary.AppendLine(L("AssetEditor.Summary.ProjectInsightsUnavailable"));
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

        summary.AppendLine($"{L("AssetEditor.Summary.LabelDiscoveredDataAssets")}: {insights.DataAssets.Count}");
        if (!string.IsNullOrWhiteSpace(normalizedFilter))
        {
            summary.AppendLine($"{L("AssetEditor.Summary.LabelFilter")}: {normalizedFilter}");
        }
        summary.AppendLine();
        if (filteredAssets.Count == 0)
        {
            summary.AppendLine(string.IsNullOrWhiteSpace(normalizedFilter)
                ? L("AssetEditor.Summary.NoDataAssets")
                : L("AssetEditor.Summary.NoDataAssetsFiltered"));
        }
        else
        {
            foreach (var asset in filteredAssets.Take(40))
            {
                var excludedSuffix = asset.Excluded ? L("AssetEditor.Summary.ExcludedSuffix") : string.Empty;
                summary.AppendLine($"- [{asset.Kind}] {asset.Title}{excludedSuffix}");
                if (!string.IsNullOrWhiteSpace(asset.FilePath))
                {
                    summary.AppendLine($"  {asset.FilePath}");
                }
            }
            if (filteredAssets.Count > 40)
            {
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.MoreItems",
                        filteredAssets.Count - 40,
                        L("AssetEditor.Summary.LabelDataAssets")));
            }
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.ModernConnectorHeading"));
        summary.AppendLine(F("AssetEditor.Summary.IndentedLine", L("AssetEditor.Summary.ModernConnectorBulletOne")));
        summary.AppendLine(F("AssetEditor.Summary.IndentedLine", L("AssetEditor.Summary.ModernConnectorBulletTwo")));
        return summary.ToString();
    }

    private string BuildObjectBrowserSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinProjectInsights? insights, string? filter, bool hideProjectRecords)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.ObjectBrowser"));
        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.LabelProject")}: {snapshot.ProjectWorkspace?.ProjectTitle}");
        if (insights is null)
        {
            summary.AppendLine(L("AssetEditor.Summary.ProjectInsightsUnavailable"));
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

        summary.AppendLine($"{L("AssetEditor.Summary.LabelObjectNodes")}: {insights.ObjectNodes.Count}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelDefinitions")}: {insights.DefinedSymbols.Count}");
        if (!string.IsNullOrWhiteSpace(normalizedFilter))
        {
            summary.AppendLine($"{L("AssetEditor.Summary.LabelFilter")}: {normalizedFilter}");
        }
        if (hideProjectRecords)
        {
            summary.AppendLine(L("AssetEditor.Summary.LabelProjectRecordsHidden"));
        }
        summary.AppendLine();
        if (filteredNodes.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoObjectBrowserNodes"));
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
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.MoreItems",
                        filteredNodes.Count - 50,
                        L("AssetEditor.Summary.LabelObjectNodes")));
            }
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.NextBrowserStep"));
        summary.AppendLine(F("AssetEditor.Summary.IndentedLine", L("AssetEditor.Summary.NextBrowserStepDetails")));
        return summary.ToString();
    }

    private string BuildToolboxSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinProjectInsights? insights)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.Toolbox"));
        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.LabelProject")}: {snapshot.ProjectWorkspace?.ProjectTitle}");
        summary.AppendLine(F("AssetEditor.Summary.LabelWorkspaceGroups", snapshot.ProjectWorkspace?.Groups.Count ?? 0));
        summary.AppendLine(F("AssetEditor.Summary.LabelLanguageIntegrations", snapshot.ExtensibilityProfile.Languages.Count));
        summary.AppendLine(F("AssetEditor.Summary.LabelAiMcpFeatures", snapshot.ExtensibilityProfile.AiFeatures.Count));
        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.SuggestedToolboxSeeds"));
        foreach (var group in snapshot.ProjectWorkspace?.Groups.Take(8) ?? Enumerable.Empty<CopperfinStudioProjectGroup>())
        {
            summary.AppendLine(F("AssetEditor.Summary.ToolboxGroupLine", group.Title, group.ItemCount));
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.AddInSurfaces"));
        foreach (var feature in snapshot.ExtensibilityProfile.AiFeatures.Take(6))
        {
            summary.AppendLine($"- {feature.Title}: {feature.Description}");
        }

        if (insights is not null && insights.RuntimeReferences.Count > 0)
        {
            summary.AppendLine();
            summary.AppendLine(L("AssetEditor.Summary.HighValueShortcuts"));
            foreach (var reference in insights.RuntimeReferences.Take(6))
            {
                summary.AppendLine($"- [{reference.Kind}] {reference.Name}");
            }
        }

        return summary.ToString();
    }

    private string BuildBuilderSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinProjectInsights? insights)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.Builders"));
        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.LabelProject")}: {snapshot.ProjectWorkspace?.ProjectTitle}");
        summary.AppendLine(L("AssetEditor.Summary.RecommendedBuilderContexts"));
        summary.AppendLine(F("AssetEditor.Summary.IndentedLine", L("AssetEditor.Summary.BuilderContextForms")));
        summary.AppendLine(F("AssetEditor.Summary.IndentedLine", L("AssetEditor.Summary.BuilderContextReports")));
        summary.AppendLine(F("AssetEditor.Summary.IndentedLine", L("AssetEditor.Summary.BuilderContextMenus")));
        summary.AppendLine(F("AssetEditor.Summary.IndentedLine", L("AssetEditor.Summary.BuilderContextSecurity")));

        if (insights is not null)
        {
            summary.AppendLine();
            summary.AppendLine(L("AssetEditor.Summary.CurrentBuilderTargets"));
            foreach (var node in insights.ObjectNodes.Take(8))
            {
                summary.AppendLine($"- [{node.Kind}] {node.Title}");
            }
        }

        return summary.ToString();
    }

    private string BuildCoverageSummary(CopperfinStudioSnapshotDocument snapshot, CopperfinRuntimeDebugSession? session)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.Coverage"));
        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.LabelProject")}: {snapshot.ProjectWorkspace?.ProjectTitle}");
        if (session is null || !session.Success)
        {
            summary.AppendLine(L("AssetEditor.Summary.CoverageNoSession"));
            return summary.ToString();
        }

        var state = session.State;
        var executedLocations = state.Events
            .Where(runtimeEvent => !string.IsNullOrWhiteSpace(runtimeEvent.Location))
            .Select(runtimeEvent => runtimeEvent.Location)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToList();

        summary.AppendLine($"{L("AssetEditor.Summary.LabelPauseReason")}: {state.Reason}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelExecutedStatements")}: {state.ExecutedStatements}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelDistinctRuntimeLocations")}: {executedLocations.Count}");
        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.RecentCoverageSignals"));
        foreach (var location in executedLocations.Take(12))
        {
            summary.AppendLine($"- {location}");
        }

        if (executedLocations.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoRuntimeLocations"));
        }

        return summary.ToString();
    }

    private string BuildDatabaseFederationSummary(CopperfinStudioSnapshotDocument snapshot, string? filter)
    {
        var summary = new StringBuilder();
        summary.AppendLine(L("AssetEditor.Summary.DatabaseFederation"));
        summary.AppendLine();
        if (!snapshot.DatabaseProfile.Available)
        {
            summary.AppendLine(L("AssetEditor.Summary.DatabaseFederationUnavailable"));
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

        summary.AppendLine($"{L("AssetEditor.Summary.LabelConnectors")}: {snapshot.DatabaseProfile.Connectors.Count}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelQueryPaths")}: {snapshot.DatabaseProfile.QueryPaths.Count}");
        if (!string.IsNullOrWhiteSpace(normalizedFilter))
        {
            summary.AppendLine($"{L("AssetEditor.Summary.LabelFilter")}: {normalizedFilter}");
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.ConnectorTargets"));
        foreach (var connector in filteredConnectors.Take(10))
        {
            summary.AppendLine($"- [{connector.Family}] {connector.Title}");
            summary.AppendLine($"  {connector.TranslationStory}");
        }
        if (filteredConnectors.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoConnectorTargets"));
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.QueryTranslationPaths"));
        foreach (var path in filteredPaths.Take(8))
        {
            summary.AppendLine($"- {path.Title} ({path.Complexity})");
            summary.AppendLine($"  {path.Strategy}");
        }
        if (filteredPaths.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoQueryPaths"));
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.Guardrails"));
        foreach (var guardrail in snapshot.DatabaseProfile.Guardrails.Take(6))
        {
            summary.AppendLine($"- {guardrail}");
        }

        return summary.ToString();
    }
}
