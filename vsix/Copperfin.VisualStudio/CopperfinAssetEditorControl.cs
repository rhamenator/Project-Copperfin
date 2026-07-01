using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
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

    private sealed class ReportSettingsScope
    {
        public int RecordIndex { get; set; }
        public bool Deleted { get; set; }
        public List<CopperfinStudioNamedValue> Settings { get; } = new();
    }

    private sealed class ReportGroupingScope
    {
        public CopperfinStudioReportGrouping Grouping { get; set; } = new();
    }

    private sealed class ExplorerSelectionState
    {
        public int? ReportSectionRecordIndex { get; set; }
        public int? ReportGroupingIndex { get; set; }
        public bool ReportSettings { get; set; }
        public bool ReportSettingsDeleted { get; set; }
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
    private readonly Button renameObjectButton;
    private readonly Button duplicateObjectButton;
    private readonly Button reorderFrontObjectButton;
    private readonly Button reorderBackObjectButton;
    private readonly Button alignLeftObjectButton;
    private readonly Button alignTopObjectButton;
    private readonly Button matchWidthObjectButton;
    private readonly Button matchHeightObjectButton;
    private readonly Button matchSizeObjectButton;
    private readonly Button distributeHorizontalObjectButton;
    private readonly Button distributeVerticalObjectButton;
    private readonly Button snapHorizontalObjectButton;
    private readonly Button snapToGridObjectButton;
    private readonly Button deleteObjectButton;
    private readonly Button restoreObjectButton;
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

        renameObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectLifecycle.RenameButton"),
            Visible = false
        };
        renameObjectButton.Click += (_, _) => TryHandleRenameObjectCommand();

        duplicateObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectLifecycle.DuplicateButton"),
            Visible = false
        };
        duplicateObjectButton.Click += (_, _) => TryHandleDuplicateObjectCommand();

        reorderFrontObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectLifecycle.ReorderFrontButton"),
            Visible = false
        };
        reorderFrontObjectButton.Click += (_, _) => TryHandleReorderObjectCommand(
            placement: "front",
            executingKey: "AssetEditor.ObjectLifecycle.ReorderFront.Executing",
            failedKey: "AssetEditor.ObjectLifecycle.ReorderFront.Failed",
            completedKey: "AssetEditor.ObjectLifecycle.ReorderFront.Completed");

        reorderBackObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectLifecycle.ReorderBackButton"),
            Visible = false
        };
        reorderBackObjectButton.Click += (_, _) => TryHandleReorderObjectCommand(
            placement: "back",
            executingKey: "AssetEditor.ObjectLifecycle.ReorderBack.Executing",
            failedKey: "AssetEditor.ObjectLifecycle.ReorderBack.Failed",
            completedKey: "AssetEditor.ObjectLifecycle.ReorderBack.Completed");

        alignLeftObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectAlignment.LeftButton"),
            Visible = false
        };
        alignLeftObjectButton.Click += (_, _) => TryHandleAlignObjectCommand(
            alignmentMode: "left",
            executingKey: "AssetEditor.ObjectAlignment.Left.Executing",
            failedKey: "AssetEditor.ObjectAlignment.Left.Failed",
            completedKey: "AssetEditor.ObjectAlignment.Left.Completed");

        alignTopObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectAlignment.TopButton"),
            Visible = false
        };
        alignTopObjectButton.Click += (_, _) => TryHandleAlignObjectCommand(
            alignmentMode: "top",
            executingKey: "AssetEditor.ObjectAlignment.Top.Executing",
            failedKey: "AssetEditor.ObjectAlignment.Top.Failed",
            completedKey: "AssetEditor.ObjectAlignment.Top.Completed");

        matchWidthObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectResize.WidthButton"),
            Visible = false
        };
        matchWidthObjectButton.Click += (_, _) => TryHandleResizeObjectCommand(
            resizeMode: "width",
            executingKey: "AssetEditor.ObjectResize.Width.Executing",
            failedKey: "AssetEditor.ObjectResize.Width.Failed",
            completedKey: "AssetEditor.ObjectResize.Width.Completed");

        matchHeightObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectResize.HeightButton"),
            Visible = false
        };
        matchHeightObjectButton.Click += (_, _) => TryHandleResizeObjectCommand(
            resizeMode: "height",
            executingKey: "AssetEditor.ObjectResize.Height.Executing",
            failedKey: "AssetEditor.ObjectResize.Height.Failed",
            completedKey: "AssetEditor.ObjectResize.Height.Completed");

        matchSizeObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectResize.SizeButton"),
            Visible = false
        };
        matchSizeObjectButton.Click += (_, _) => TryHandleResizeObjectCommand(
            resizeMode: "size",
            executingKey: "AssetEditor.ObjectResize.Size.Executing",
            failedKey: "AssetEditor.ObjectResize.Size.Failed",
            completedKey: "AssetEditor.ObjectResize.Size.Completed");

        distributeHorizontalObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectDistribution.HorizontalButton"),
            Visible = false
        };
        distributeHorizontalObjectButton.Click += (_, _) => TryHandleDistributeObjectCommand(
            distributionMode: "horizontal",
            executingKey: "AssetEditor.ObjectDistribution.Horizontal.Executing",
            failedKey: "AssetEditor.ObjectDistribution.Horizontal.Failed",
            completedKey: "AssetEditor.ObjectDistribution.Horizontal.Completed");

        distributeVerticalObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectDistribution.VerticalButton"),
            Visible = false
        };
        distributeVerticalObjectButton.Click += (_, _) => TryHandleDistributeObjectCommand(
            distributionMode: "vertical",
            executingKey: "AssetEditor.ObjectDistribution.Vertical.Executing",
            failedKey: "AssetEditor.ObjectDistribution.Vertical.Failed",
            completedKey: "AssetEditor.ObjectDistribution.Vertical.Completed");

        snapHorizontalObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectSnap.HorizontalButton"),
            Visible = false
        };
        snapHorizontalObjectButton.Click += (_, _) => TryHandleSnapObjectCommand(
            snapMode: "horizontal",
            executingKey: "AssetEditor.ObjectSnap.Horizontal.Executing",
            failedKey: "AssetEditor.ObjectSnap.Horizontal.Failed",
            completedKey: "AssetEditor.ObjectSnap.Horizontal.Completed");

        snapToGridObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectSnap.GridButton"),
            Visible = false
        };
        snapToGridObjectButton.Click += (_, _) => TryHandleSnapObjectCommand(
            snapMode: "both",
            executingKey: "AssetEditor.ObjectSnap.Grid.Executing",
            failedKey: "AssetEditor.ObjectSnap.Grid.Failed",
            completedKey: "AssetEditor.ObjectSnap.Grid.Completed");

        deleteObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectLifecycle.DeleteButton"),
            Visible = false
        };
        deleteObjectButton.Click += (_, _) => TryHandleObjectLifecycleCommand(restoring: false);

        restoreObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectLifecycle.RestoreButton"),
            Visible = false
        };
        restoreObjectButton.Click += (_, _) => TryHandleObjectLifecycleCommand(restoring: true);

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
            MultiSelect = true,
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
        designSurface.SelectedReportSectionChanged += recordIndex => SyncReportSectionSelectionFromSurface(recordIndex);
        designSurface.SelectedUnplacedObjectsChanged += SyncUnplacedObjectsSelectionFromSurface;
        designSurface.ObjectMoved += (recordIndex, left, top) =>
        {
            var horizontalName = currentSnapshot?.AssetFamily is "report" or "label" ? "HPOS" : "Left";
            var verticalName = currentSnapshot?.AssetFamily is "report" or "label" ? "VPOS" : "Top";
            ApplyVisualPropertyChanges(
                recordIndex,
                new[]
                {
                    new KeyValuePair<string, string>(horizontalName, left.ToString()),
                    new KeyValuePair<string, string>(verticalName, top.ToString())
                });
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
        buttonPanel.Controls.Add(renameObjectButton);
        buttonPanel.Controls.Add(duplicateObjectButton);
        buttonPanel.Controls.Add(reorderFrontObjectButton);
        buttonPanel.Controls.Add(reorderBackObjectButton);
        buttonPanel.Controls.Add(alignLeftObjectButton);
        buttonPanel.Controls.Add(alignTopObjectButton);
        buttonPanel.Controls.Add(matchWidthObjectButton);
        buttonPanel.Controls.Add(matchHeightObjectButton);
        buttonPanel.Controls.Add(matchSizeObjectButton);
        buttonPanel.Controls.Add(distributeHorizontalObjectButton);
        buttonPanel.Controls.Add(distributeVerticalObjectButton);
        buttonPanel.Controls.Add(snapHorizontalObjectButton);
        buttonPanel.Controls.Add(snapToGridObjectButton);
        buttonPanel.Controls.Add(deleteObjectButton);
        buttonPanel.Controls.Add(restoreObjectButton);
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
        var selectedObjectUniqueIds = currentSnapshot.AssetFamily is "report" or "label"
            ? TryGetSelectedSnapshotObjects()
                .Select(TryReadObjectUniqueId)
                .Where(uniqueId => !string.IsNullOrWhiteSpace(uniqueId))
                .Cast<string>()
                .ToList()
            : new List<string>();
        var focusedObjectUniqueId = currentSnapshot.AssetFamily is "report" or "label" && selectedObjectRecordIndex >= 0
            ? TryReadObjectUniqueId(TryGetSelectedSnapshotObject()!)
            : null;
        snapshotStatusLabel.Text = BuildUndoExecutingStatus(priorLabel);

        var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(
            currentPath!,
            currentSnapshot.AssetFamily is "report" or "label" && selectedObjectRecordIndex >= 0
                ? selectedObjectRecordIndex
                : null);
        if (!undoResult.Success || undoResult.Document is null)
        {
            snapshotStatusLabel.Text = BuildUndoFailedStatus(undoResult.Error);
            return false;
        }

        currentSnapshot = undoResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = BuildUndoCompletedStatus(priorLabel, currentSnapshot);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        if (selectedObjectRecordIndex >= 0)
        {
            designSurface.SelectRecord(selectedObjectRecordIndex);
            SyncSelectionFromSurface(selectedObjectRecordIndex);
            if (selectedObjectUniqueIds.Count > 0)
            {
                var restoredRecordIndexes = selectedObjectUniqueIds
                    .Select(uniqueId => TryReadObjectRecordIndex(currentSnapshot, uniqueId))
                    .Where(recordIndex => recordIndex.HasValue)
                    .Select(recordIndex => recordIndex!.Value)
                    .ToList();
                var focusedRecordIndex = !string.IsNullOrWhiteSpace(focusedObjectUniqueId)
                    ? TryReadObjectRecordIndex(currentSnapshot, focusedObjectUniqueId!) ?? selectedObjectRecordIndex
                    : selectedObjectRecordIndex;
                RestoreSnapshotObjectSelection(restoredRecordIndexes, focusedRecordIndex);
            }
        }

        return true;
    }

    private int TryReadSelectedRecordIndex()
    {
        return TryGetSelectedSnapshotObject()?.RecordIndex ?? -1;
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
        UpdateObjectLifecycleButtonVisibility();
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
                UpdateObjectLifecycleButtonVisibility();
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
            UpdateObjectLifecycleButtonVisibility();
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
            (reportLayout.Sections.Count > 0 ||
             reportLayout.DeletedSections.Count > 0 ||
             reportLayout.Groupings.Count > 0 ||
             reportLayout.Settings.Count > 0 ||
             reportLayout.DeletedSettings.Count > 0 ||
             reportLayout.UnplacedObjects.Count > 0))
        {
            sectionListView.Visible = true;
            leftExplorerSplit.Panel1Collapsed = false;
            sectionListView.Columns[0].Text = this.localization.Text("AssetEditor.Column.Section");
            sectionListView.Columns[1].Text = this.localization.Text("AssetEditor.Column.Objects");
            sectionListView.Columns[2].Text = this.localization.Text("AssetEditor.Column.Top");
            foreach (var section in reportLayout.Sections)
            {
                var item = new ListViewItem(BuildReportSectionListTitle(section));
                item.SubItems.Add(CountVisibleReportSectionObjects(reportLayout, section).ToString());
                item.SubItems.Add(section.Top.ToString());
                item.Tag = section;
                sectionListView.Items.Add(item);
            }

            foreach (var section in reportLayout.DeletedSections)
            {
                var item = new ListViewItem(BuildDeletedReportSectionListTitle(section));
                item.SubItems.Add(CountVisibleReportSectionObjects(reportLayout, section).ToString());
                item.SubItems.Add(section.Top.ToString());
                item.Tag = section;
                item.ForeColor = Color.Firebrick;
                sectionListView.Items.Add(item);
            }

            foreach (var grouping in reportLayout.Groupings)
            {
                var item = new ListViewItem(BuildReportGroupingListTitle(grouping));
                item.SubItems.Add(CountVisibleReportGroupingSections(grouping).ToString());
                item.SubItems.Add(string.Empty);
                item.Tag = new ReportGroupingScope
                {
                    Grouping = grouping
                };
                sectionListView.Items.Add(item);
            }

            if (reportLayout.Settings.Count > 0)
            {
                var settingsScope = new ReportSettingsScope
                {
                    RecordIndex = reportLayout.Settings.FirstOrDefault()?.RecordIndex ?? 0,
                    Deleted = false
                };
                settingsScope.Settings.AddRange(reportLayout.Settings);

                var item = new ListViewItem(L("AssetEditor.ReportSection.Settings"));
                item.SubItems.Add(reportLayout.Settings.Count.ToString());
                item.SubItems.Add(string.Empty);
                item.Tag = settingsScope;
                sectionListView.Items.Add(item);
            }

            if (reportLayout.DeletedSettings.Count > 0)
            {
                var deletedSettingsScope = new ReportSettingsScope
                {
                    RecordIndex = reportLayout.DeletedSettings.FirstOrDefault()?.RecordIndex ?? 0,
                    Deleted = true
                };
                deletedSettingsScope.Settings.AddRange(reportLayout.DeletedSettings);

                var item = new ListViewItem(F("AssetEditor.ReportSection.Deleted", L("AssetEditor.ReportSection.Settings")));
                item.SubItems.Add(reportLayout.DeletedSettings.Count.ToString());
                item.SubItems.Add(string.Empty);
                item.Tag = deletedSettingsScope;
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
            var title = string.IsNullOrWhiteSpace(item.Title) ? BuildFallbackObjectTitle(item.RecordIndex) : item.Title;
            if (currentSnapshot.AssetFamily == "project" && projectEntry is not null && !string.IsNullOrWhiteSpace(projectEntry.RelativePath))
            {
                title = projectEntry.RelativePath;
            }

            var subtitle = currentSnapshot.AssetFamily == "project"
                ? projectEntry?.GroupTitle ?? item.Subtitle
                : BuildObjectListSubtitle(currentSnapshot.AssetFamily, item.Subtitle);

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
        var selectedObject = TryGetSelectedSnapshotObject();

        propertyGrid.SelectedObject = selectedObject is null || currentSnapshot is null
            ? null
            : CopperfinDesignerSelection.FromSnapshot(currentSnapshot.AssetFamily, selectedObject, localization);
        designSurface.SelectRecord(selectedObject?.RecordIndex);
        UpdateObjectLifecycleButtonVisibility();
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
            var isReportScopeSelection = selectedExplorerTag is CopperfinStudioReportSection ||
                                         selectedExplorerTag is ReportGroupingScope ||
                                         selectedExplorerTag is ReportUnplacedObjectScope ||
                                         selectedExplorerTag is ReportSettingsScope;
            PopulateObjectList(autoSelectFirstItem: !isReportScopeSelection);

            if (selectedExplorerTag is CopperfinStudioReportSection reportSection)
            {
                foreach (ListViewItem item in objectListView.Items)
                {
                    item.Selected = false;
                }

                propertyGrid.SelectedObject = CopperfinDesignerSelection.FromReportSection(reportSection, localization);
                designSurface.SelectReportSection(reportSection.RecordIndex);
                return;
            }

            if (selectedExplorerTag is ReportGroupingScope groupingScope)
            {
                foreach (ListViewItem item in objectListView.Items)
                {
                    item.Selected = false;
                }

                propertyGrid.SelectedObject = CopperfinDesignerSelection.FromReportGrouping(
                    groupingScope.Grouping,
                    localization);
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
                designSurface.SelectUnplacedObjects();
                return;
            }

            if (selectedExplorerTag is ReportSettingsScope settingsScope)
            {
                foreach (ListViewItem item in objectListView.Items)
                {
                    item.Selected = false;
                }

                propertyGrid.SelectedObject = CopperfinDesignerSelection.FromReportSettings(
                    settingsScope.Settings,
                    localization,
                    settingsScope.Deleted,
                    currentSnapshot?.ReportLayout);
                designSurface.SelectRecord(null);
                return;
            }
        }
        finally
        {
            suppressSelectionSync = false;
            UpdateObjectLifecycleButtonVisibility();
        }
    }

    private void SyncReportSectionSelectionFromSurface(int recordIndex)
    {
        if (suppressSelectionSync)
        {
            return;
        }

        try
        {
            suppressSelectionSync = true;
            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == recordIndex;
            }
        }
        finally
        {
            suppressSelectionSync = false;
        }

        SyncExplorerSelection();
    }

    private void SyncUnplacedObjectsSelectionFromSurface()
    {
        if (suppressSelectionSync)
        {
            return;
        }

        try
        {
            suppressSelectionSync = true;
            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is ReportUnplacedObjectScope;
            }
        }
        finally
        {
            suppressSelectionSync = false;
        }

        SyncExplorerSelection();
    }

    private void SyncSelectionFromSurface(int recordIndex)
    {
        if (suppressSelectionSync) {
            return;
        }

        try
        {
            suppressSelectionSync = true;
            if ((currentSnapshot?.AssetFamily == "report" || currentSnapshot?.AssetFamily == "label") &&
                TrySelectReportScopeForRecord(recordIndex))
            {
                PopulateObjectList(autoSelectFirstItem: false);
            }

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
                : CopperfinDesignerSelection.FromSnapshot(currentSnapshot.AssetFamily, selectedObject, localization);
        }
        finally
        {
            suppressSelectionSync = false;
            UpdateObjectLifecycleButtonVisibility();
        }
    }

    private bool TrySelectReportScopeForRecord(int recordIndex)
    {
        if (currentSnapshot?.ReportLayout is null || currentSnapshot.AssetFamily is not ("report" or "label"))
        {
            return false;
        }

        var selectedItem = sectionListView.Items
            .Cast<ListViewItem>()
            .FirstOrDefault(item =>
                item.Tag is CopperfinStudioReportSection section
                    ? section.Objects.Any(layoutObject => layoutObject.RecordIndex == recordIndex)
                    : item.Tag is ReportUnplacedObjectScope unplacedScope &&
                      unplacedScope.RecordIndexes.Contains(recordIndex));
        if (selectedItem is null)
        {
            return false;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = ReferenceEquals(item, selectedItem);
        }

        return true;
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
        ApplyVisualPropertyChanges(
            recordIndex,
            new[]
            {
                new KeyValuePair<string, string>(propertyName, propertyValue)
            });
    }

    private void ApplyVisualPropertyChanges(int recordIndex, IReadOnlyList<KeyValuePair<string, string>> propertyChanges)
    {
        if (string.IsNullOrWhiteSpace(currentPath) || propertyChanges.Count == 0)
        {
            return;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        var selectedObjectRecordIndex = TryReadSelectedRecordIndex();
        var statusLabel = ResolvePropertyStatusLabels(propertyChanges.Select(change => change.Key));
        snapshotStatusLabel.Text = BuildPropertyApplyingStatusLabel(statusLabel);
        var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperties(currentPath!, recordIndex, propertyChanges);
        if (!updateResult.Success || updateResult.Document is null)
        {
            snapshotStatusLabel.Text = BuildPropertyUpdateFailedStatus(updateResult.Error);
            return;
        }

        currentSnapshot = updateResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = BuildPropertyUpdatedStatusLabel(statusLabel, currentSnapshot);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        if (selectedObjectRecordIndex >= 0)
        {
            designSurface.SelectRecord(selectedObjectRecordIndex);
            SyncSelectionFromSurface(selectedObjectRecordIndex);
        }
    }

    private bool TryHandleObjectLifecycleCommand(bool restoring)
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        var selectedObject = TryGetSelectedSnapshotObject();
        if (selectedObject is null || selectedObject.Deleted != restoring)
        {
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        var selectedObjectRecordIndex = selectedObject.RecordIndex;
        var uniqueId = TryReadObjectUniqueId(selectedObject);
        snapshotStatusLabel.Text = this.localization.Text(
            restoring
                ? "AssetEditor.ObjectLifecycle.Restore.Executing"
                : "AssetEditor.ObjectLifecycle.Delete.Executing");

        var lifecycleResult = restoring
            ? CopperfinStudioSnapshotClient.TryRestoreObject(currentPath!, selectedObjectRecordIndex, uniqueId)
            : CopperfinStudioSnapshotClient.TryDeleteObject(currentPath!, selectedObjectRecordIndex, uniqueId);
        if (!lifecycleResult.Success || lifecycleResult.Document is null)
        {
            snapshotStatusLabel.Text = this.localization.Format(
                restoring
                    ? "AssetEditor.ObjectLifecycle.Restore.Failed"
                    : "AssetEditor.ObjectLifecycle.Delete.Failed",
                lifecycleResult.Error ?? string.Empty);
            return false;
        }

        currentSnapshot = lifecycleResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = this.localization.Format(
            restoring
                ? "AssetEditor.ObjectLifecycle.Restore.Completed"
                : "AssetEditor.ObjectLifecycle.Delete.Completed",
            currentSnapshot.Objects.Count,
            currentSnapshot.FieldCount);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        designSurface.SelectRecord(selectedObjectRecordIndex);
        SyncSelectionFromSurface(selectedObjectRecordIndex);
        return true;
    }

    private bool TryHandleDuplicateObjectCommand()
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        var selectedObject = TryGetSelectedSnapshotObject();
        if (selectedObject is null)
        {
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        var sourceUniqueId = TryReadObjectUniqueId(selectedObject);
        var duplicateUniqueId = CreateDuplicateObjectUniqueId();
        snapshotStatusLabel.Text = this.localization.Text("AssetEditor.ObjectLifecycle.Duplicate.Executing");

        var duplicateResult = CopperfinStudioSnapshotClient.TryDuplicateObject(
            currentPath!,
            selectedObject.RecordIndex,
            sourceUniqueId,
            duplicateUniqueId);
        if (!duplicateResult.Success || duplicateResult.Document is null)
        {
            snapshotStatusLabel.Text = this.localization.Format(
                "AssetEditor.ObjectLifecycle.Duplicate.Failed",
                duplicateResult.Error ?? string.Empty);
            return false;
        }

        currentSnapshot = duplicateResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = this.localization.Format(
            "AssetEditor.ObjectLifecycle.Duplicate.Completed",
            currentSnapshot.Objects.Count,
            currentSnapshot.FieldCount);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();

        var duplicatedRecordIndex = TryReadObjectRecordIndex(currentSnapshot, duplicateUniqueId) ?? selectedObject.RecordIndex;
        designSurface.SelectRecord(duplicatedRecordIndex);
        SyncSelectionFromSurface(duplicatedRecordIndex);
        return true;
    }

    private bool TryHandleRenameObjectCommand()
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        var selectedObject = TryGetSelectedSnapshotObject();
        if (selectedObject is null)
        {
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        var sourceUniqueId = TryReadObjectUniqueId(selectedObject);
        var renamedUniqueId = CreateRenameObjectUniqueId();
        snapshotStatusLabel.Text = this.localization.Text("AssetEditor.ObjectLifecycle.Rename.Executing");

        var renameResult = CopperfinStudioSnapshotClient.TryRenameObject(
            currentPath!,
            selectedObject.RecordIndex,
            sourceUniqueId,
            renamedUniqueId);
        if (!renameResult.Success || renameResult.Document is null)
        {
            snapshotStatusLabel.Text = this.localization.Format(
                "AssetEditor.ObjectLifecycle.Rename.Failed",
                renameResult.Error ?? string.Empty);
            return false;
        }

        currentSnapshot = renameResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = this.localization.Format(
            "AssetEditor.ObjectLifecycle.Rename.Completed",
            currentSnapshot.Objects.Count,
            currentSnapshot.FieldCount);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();

        var renamedRecordIndex = TryReadObjectRecordIndex(currentSnapshot, renamedUniqueId) ?? selectedObject.RecordIndex;
        designSurface.SelectRecord(renamedRecordIndex);
        SyncSelectionFromSurface(renamedRecordIndex);
        return true;
    }

    private bool TryHandleReorderObjectCommand(
        string placement,
        string executingKey,
        string failedKey,
        string completedKey)
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        var selectedObject = TryGetSelectedSnapshotObject();
        if (selectedObject is null)
        {
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        var selectedObjectRecordIndex = selectedObject.RecordIndex;
        var uniqueId = TryReadObjectUniqueId(selectedObject);
        snapshotStatusLabel.Text = this.localization.Text(executingKey);

        var reorderResult = CopperfinStudioSnapshotClient.TryReorderObject(
            currentPath!,
            selectedObjectRecordIndex,
            uniqueId,
            placement);
        if (!reorderResult.Success || reorderResult.Document is null)
        {
            snapshotStatusLabel.Text = this.localization.Format(
                failedKey,
                reorderResult.Error ?? string.Empty);
            return false;
        }

        currentSnapshot = reorderResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = this.localization.Format(
            completedKey,
            currentSnapshot.Objects.Count,
            currentSnapshot.FieldCount);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        var reorderedRecordIndex = !string.IsNullOrWhiteSpace(uniqueId)
            ? TryReadObjectRecordIndex(currentSnapshot, uniqueId!) ?? selectedObjectRecordIndex
            : selectedObjectRecordIndex;
        designSurface.SelectRecord(reorderedRecordIndex);
        SyncSelectionFromSurface(reorderedRecordIndex);
        return true;
    }

    private bool TryHandleAlignObjectCommand(
        string alignmentMode,
        string executingKey,
        string failedKey,
        string completedKey)
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        if (TryReadSelectedExplorerTag() is not CopperfinStudioReportSection { Deleted: false })
        {
            return false;
        }

        var selectedObjects = TryGetSelectedSnapshotObjects();
        var anchorObject = TryGetSelectedSnapshotObject();
        if (anchorObject is null || anchorObject.Deleted || selectedObjects.Count < 2)
        {
            return false;
        }

        var anchorUniqueId = TryReadObjectUniqueId(anchorObject);
        var targetUniqueIds = selectedObjects
            .Where(selectedObject => selectedObject.RecordIndex != anchorObject.RecordIndex && !selectedObject.Deleted)
            .Select(TryReadObjectUniqueId)
            .Where(uniqueId => !string.IsNullOrWhiteSpace(uniqueId))
            .Cast<string>()
            .ToList();
        if (string.IsNullOrWhiteSpace(anchorUniqueId) || targetUniqueIds.Count != selectedObjects.Count - 1)
        {
            snapshotStatusLabel.Text = this.localization.Text("AssetEditor.ObjectAlignment.StableIdsRequired");
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        snapshotStatusLabel.Text = this.localization.Text(executingKey);

        var alignResult = CopperfinStudioSnapshotClient.TryAlignObject(
            currentPath!,
            anchorObject.RecordIndex,
            anchorUniqueId!,
            alignmentMode,
            targetUniqueIds);
        if (!alignResult.Success || alignResult.Document is null)
        {
            snapshotStatusLabel.Text = this.localization.Format(
                failedKey,
                alignResult.Error ?? string.Empty);
            return false;
        }

        currentSnapshot = alignResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = this.localization.Format(
            completedKey,
            currentSnapshot.Objects.Count,
            currentSnapshot.FieldCount);
        var selectedRecordIndexes = new List<int> { anchorObject.RecordIndex };
        selectedRecordIndexes.AddRange(targetUniqueIds
            .Select(uniqueId => TryReadObjectRecordIndex(currentSnapshot, uniqueId))
            .Where(recordIndex => recordIndex.HasValue)
            .Select(recordIndex => recordIndex!.Value));
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        designSurface.SelectRecord(anchorObject.RecordIndex);
        SyncSelectionFromSurface(anchorObject.RecordIndex);
        RestoreSnapshotObjectSelection(selectedRecordIndexes, anchorObject.RecordIndex);
        return true;
    }

    private bool TryHandleDistributeObjectCommand(
        string distributionMode,
        string executingKey,
        string failedKey,
        string completedKey)
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        if (TryReadSelectedExplorerTag() is not CopperfinStudioReportSection { Deleted: false })
        {
            return false;
        }

        var selectedObjects = TryGetSelectedSnapshotObjects();
        var focusedObject = TryGetSelectedSnapshotObject();
        if (focusedObject is null || focusedObject.Deleted || selectedObjects.Count < 3)
        {
            return false;
        }

        var targetUniqueIds = selectedObjects
            .Where(selectedObject => !selectedObject.Deleted)
            .Select(TryReadObjectUniqueId)
            .Where(uniqueId => !string.IsNullOrWhiteSpace(uniqueId))
            .Cast<string>()
            .ToList();
        if (targetUniqueIds.Count != selectedObjects.Count)
        {
            snapshotStatusLabel.Text = this.localization.Text("AssetEditor.ObjectDistribution.StableIdsRequired");
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        snapshotStatusLabel.Text = this.localization.Text(executingKey);

        var distributeResult = CopperfinStudioSnapshotClient.TryDistributeObject(
            currentPath!,
            focusedObject.RecordIndex,
            distributionMode,
            targetUniqueIds);
        if (!distributeResult.Success || distributeResult.Document is null)
        {
            snapshotStatusLabel.Text = this.localization.Format(
                failedKey,
                distributeResult.Error ?? string.Empty);
            return false;
        }

        currentSnapshot = distributeResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = this.localization.Format(
            completedKey,
            currentSnapshot.Objects.Count,
            currentSnapshot.FieldCount);
        var selectedRecordIndexes = targetUniqueIds
            .Select(uniqueId => TryReadObjectRecordIndex(currentSnapshot, uniqueId))
            .Where(recordIndex => recordIndex.HasValue)
            .Select(recordIndex => recordIndex!.Value)
            .ToList();
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        designSurface.SelectRecord(focusedObject.RecordIndex);
        SyncSelectionFromSurface(focusedObject.RecordIndex);
        RestoreSnapshotObjectSelection(selectedRecordIndexes, focusedObject.RecordIndex);
        return true;
    }

    private bool TryHandleResizeObjectCommand(
        string resizeMode,
        string executingKey,
        string failedKey,
        string completedKey)
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        if (TryReadSelectedExplorerTag() is not CopperfinStudioReportSection { Deleted: false })
        {
            return false;
        }

        var selectedObjects = TryGetSelectedSnapshotObjects();
        var anchorObject = TryGetSelectedSnapshotObject();
        if (anchorObject is null || anchorObject.Deleted || selectedObjects.Count < 2)
        {
            return false;
        }

        var anchorUniqueId = TryReadObjectUniqueId(anchorObject);
        var targetUniqueIds = selectedObjects
            .Where(selectedObject => selectedObject.RecordIndex != anchorObject.RecordIndex && !selectedObject.Deleted)
            .Select(TryReadObjectUniqueId)
            .Where(uniqueId => !string.IsNullOrWhiteSpace(uniqueId))
            .Cast<string>()
            .ToList();
        if (string.IsNullOrWhiteSpace(anchorUniqueId) || targetUniqueIds.Count != selectedObjects.Count - 1)
        {
            snapshotStatusLabel.Text = this.localization.Text("AssetEditor.ObjectResize.StableIdsRequired");
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        snapshotStatusLabel.Text = this.localization.Text(executingKey);

        var resizeResult = CopperfinStudioSnapshotClient.TryResizeObject(
            currentPath!,
            anchorObject.RecordIndex,
            anchorUniqueId!,
            resizeMode,
            targetUniqueIds);
        if (!resizeResult.Success || resizeResult.Document is null)
        {
            snapshotStatusLabel.Text = this.localization.Format(
                failedKey,
                resizeResult.Error ?? string.Empty);
            return false;
        }

        currentSnapshot = resizeResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = this.localization.Format(
            completedKey,
            currentSnapshot.Objects.Count,
            currentSnapshot.FieldCount);
        var selectedRecordIndexes = new List<int> { anchorObject.RecordIndex };
        selectedRecordIndexes.AddRange(targetUniqueIds
            .Select(uniqueId => TryReadObjectRecordIndex(currentSnapshot, uniqueId))
            .Where(recordIndex => recordIndex.HasValue)
            .Select(recordIndex => recordIndex!.Value));
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        designSurface.SelectRecord(anchorObject.RecordIndex);
        SyncSelectionFromSurface(anchorObject.RecordIndex);
        RestoreSnapshotObjectSelection(selectedRecordIndexes, anchorObject.RecordIndex);
        return true;
    }

    private bool TryHandleSnapObjectCommand(
        string snapMode,
        string executingKey,
        string failedKey,
        string completedKey)
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        var selectedObjects = TryGetSelectedSnapshotObjects();
        var focusedObject = TryGetSelectedSnapshotObject();
        if (focusedObject is null || focusedObject.Deleted || selectedObjects.Count < 1)
        {
            return false;
        }

        if (!TryReadReportGridDimensions(currentSnapshot, out var gridWidth, out var gridHeight))
        {
            snapshotStatusLabel.Text = this.localization.Text("AssetEditor.ObjectSnap.GridSettingsRequired");
            return false;
        }

        var focusedUniqueId = TryReadObjectUniqueId(focusedObject);
        var targetUniqueIds = selectedObjects
            .Where(selectedObject => !selectedObject.Deleted)
            .Select(TryReadObjectUniqueId)
            .Where(uniqueId => !string.IsNullOrWhiteSpace(uniqueId))
            .Cast<string>()
            .ToList();
        if (string.IsNullOrWhiteSpace(focusedUniqueId) || targetUniqueIds.Count != selectedObjects.Count)
        {
            snapshotStatusLabel.Text = this.localization.Text("AssetEditor.ObjectSnap.StableIdsRequired");
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        snapshotStatusLabel.Text = this.localization.Text(executingKey);

        var snapResult = CopperfinStudioSnapshotClient.TrySnapObject(
            currentPath!,
            focusedObject.RecordIndex,
            snapMode,
            gridWidth,
            gridHeight,
            targetUniqueIds);
        if (!snapResult.Success || snapResult.Document is null)
        {
            snapshotStatusLabel.Text = this.localization.Format(
                failedKey,
                snapResult.Error ?? string.Empty);
            return false;
        }

        currentSnapshot = snapResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = this.localization.Format(
            completedKey,
            currentSnapshot.Objects.Count,
            currentSnapshot.FieldCount);
        var selectedRecordIndexes = targetUniqueIds
            .Select(uniqueId => TryReadObjectRecordIndex(currentSnapshot, uniqueId))
            .Where(recordIndex => recordIndex.HasValue)
            .Select(recordIndex => recordIndex!.Value)
            .ToList();
        var refreshedFocusedRecordIndex = TryReadObjectRecordIndex(currentSnapshot, focusedUniqueId!) ?? focusedObject.RecordIndex;
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        designSurface.SelectRecord(refreshedFocusedRecordIndex);
        SyncSelectionFromSurface(refreshedFocusedRecordIndex);
        RestoreSnapshotObjectSelection(selectedRecordIndexes, refreshedFocusedRecordIndex);
        return true;
    }

    private bool TryHandleNudgeObjectCommand(string mode, double deltaHpos, double deltaVpos)
    {
        if (currentSnapshot?.AssetFamily is not ("report" or "label") || string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        var selectedObject = TryGetSelectedSnapshotObject();
        if (selectedObject is null)
        {
            return false;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        var selectedObjectRecordIndex = selectedObject.RecordIndex;
        var uniqueId = TryReadObjectUniqueId(selectedObject);
        var propertyNames = string.Equals(mode, "horizontal", StringComparison.OrdinalIgnoreCase)
            ? new[] { "HPOS" }
            : string.Equals(mode, "vertical", StringComparison.OrdinalIgnoreCase)
                ? new[] { "VPOS" }
                : new[] { "HPOS", "VPOS" };
        var statusLabel = ResolvePropertyStatusLabels(propertyNames);
        snapshotStatusLabel.Text = BuildPropertyApplyingStatusLabel(statusLabel);

        var nudgeResult = CopperfinStudioSnapshotClient.TryNudgeObject(
            currentPath!,
            selectedObjectRecordIndex,
            uniqueId,
            mode,
            deltaHpos,
            deltaVpos);
        if (!nudgeResult.Success || nudgeResult.Document is null)
        {
            snapshotStatusLabel.Text = BuildPropertyUpdateFailedStatus(nudgeResult.Error);
            return false;
        }

        currentSnapshot = nudgeResult.Document;
        detailsLabel.Text = BuildSnapshotDetailsText(new FileInfo(currentPath!), currentSnapshot);
        snapshotStatusLabel.Text = BuildPropertyUpdatedStatusLabel(statusLabel, currentSnapshot);
        PopulateSectionList(explorerSelection);
        SyncExplorerSelection();
        LoadSurface();
        designSurface.SelectRecord(selectedObjectRecordIndex);
        SyncSelectionFromSurface(selectedObjectRecordIndex);
        return true;
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
            var selectedExplorerTag = TryReadSelectedExplorerTag();
            if (selectedExplorerTag is CopperfinStudioReportSection reportSection)
            {
                designSurface.SelectReportSection(reportSection.RecordIndex);
            }
            else if (selectedExplorerTag is ReportGroupingScope)
            {
                designSurface.SelectRecord(null);
            }
            else if (selectedExplorerTag is ReportUnplacedObjectScope)
            {
                designSurface.SelectUnplacedObjects();
            }

            UpdateObjectLifecycleButtonVisibility();
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
            UpdateObjectLifecycleButtonVisibility();
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
        UpdateObjectLifecycleButtonVisibility();
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
            "program" => this.localization.Text("AssetEditor.Guidance.Program"),
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
        return BuildPropertyApplyingStatusLabel(ResolvePropertyStatusLabel(propertyName));
    }

    private string BuildPropertyApplyingStatusLabel(string propertyLabel)
    {
        return this.localization.Format("AssetEditor.Property.ApplyingChange", propertyLabel);
    }

    private string BuildPropertyUpdateFailedStatus(string? error)
    {
        return this.localization.Format("AssetEditor.Property.UpdateFailed", error ?? string.Empty);
    }

    private string BuildPropertyUpdatedStatus(string propertyName, CopperfinStudioSnapshotDocument snapshot)
    {
        return BuildPropertyUpdatedStatusLabel(ResolvePropertyStatusLabel(propertyName), snapshot);
    }

    private string BuildPropertyUpdatedStatusLabel(string propertyLabel, CopperfinStudioSnapshotDocument snapshot)
    {
        return this.localization.Format("AssetEditor.Property.Updated", propertyLabel, snapshot.Objects.Count, snapshot.FieldCount) +
            (snapshot.CommandUndoAvailable && !string.IsNullOrWhiteSpace(snapshot.CommandUndoLabel)
                ? this.localization.Format("AssetEditor.Snapshot.UndoAvailable", snapshot.CommandUndoLabel)
                : string.Empty);
    }

    private string ResolvePropertyStatusLabels(IEnumerable<string> propertyNames)
    {
        return string.Join(", ", propertyNames.Select(ResolvePropertyStatusLabel));
    }

    private string ResolvePropertyStatusLabel(string propertyName)
    {
        if (propertyGrid.SelectedObject is CopperfinDesignerSelection selection &&
            selection.TryGetDisplayName(propertyName, out var displayName) &&
            !string.IsNullOrWhiteSpace(displayName))
        {
            return displayName;
        }

        return propertyName;
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

    private void UpdateObjectLifecycleButtonVisibility()
    {
        var selectedObjects = currentSnapshot?.AssetFamily is "report" or "label"
            ? TryGetSelectedSnapshotObjects()
            : Array.Empty<CopperfinStudioSnapshotObject>();
        var selectedObject = selectedObjects.FirstOrDefault();
        var singleSelection = selectedObjects.Count == 1;
        var showRename = singleSelection && selectedObject is not null;
        var showDuplicate = singleSelection && selectedObject is not null;
        var showReorder = singleSelection && selectedObject is not null;
        var showAlignLeft = selectedObjects.Count >= 2 &&
                            selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                            TryReadSelectedExplorerTag() is CopperfinStudioReportSection { Deleted: false };
        var showAlignTop = selectedObjects.Count >= 2 &&
                           selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                           TryReadSelectedExplorerTag() is CopperfinStudioReportSection { Deleted: false };
        var showMatchWidth = selectedObjects.Count >= 2 &&
                             selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                             TryReadSelectedExplorerTag() is CopperfinStudioReportSection { Deleted: false };
        var showMatchHeight = selectedObjects.Count >= 2 &&
                              selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                              TryReadSelectedExplorerTag() is CopperfinStudioReportSection { Deleted: false };
        var showMatchSize = selectedObjects.Count >= 2 &&
                            selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                            TryReadSelectedExplorerTag() is CopperfinStudioReportSection { Deleted: false };
        var showDistributeHorizontal = selectedObjects.Count >= 3 &&
                                       selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                                       TryReadSelectedExplorerTag() is CopperfinStudioReportSection { Deleted: false };
        var showDistributeVertical = selectedObjects.Count >= 3 &&
                                     selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                                     TryReadSelectedExplorerTag() is CopperfinStudioReportSection { Deleted: false };
        var showSnapToGrid = selectedObjects.Count >= 1 &&
                             selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                             currentSnapshot is not null &&
                             TryReadReportGridDimensions(currentSnapshot, out _, out _);
        var showSnapHorizontal = selectedObjects.Count >= 1 &&
                                 selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                                 currentSnapshot is not null &&
                                 TryReadReportGridDimensions(currentSnapshot, out _, out _);
        var showDelete = singleSelection && selectedObject is not null && !selectedObject.Deleted;
        var showRestore = singleSelection && selectedObject is not null && selectedObject.Deleted;
        renameObjectButton.Visible = showRename;
        renameObjectButton.Enabled = showRename && !string.IsNullOrWhiteSpace(currentPath);
        duplicateObjectButton.Visible = showDuplicate;
        duplicateObjectButton.Enabled = showDuplicate && !string.IsNullOrWhiteSpace(currentPath);
        reorderFrontObjectButton.Visible = showReorder;
        reorderFrontObjectButton.Enabled = showReorder && !string.IsNullOrWhiteSpace(currentPath);
        reorderBackObjectButton.Visible = showReorder;
        reorderBackObjectButton.Enabled = showReorder && !string.IsNullOrWhiteSpace(currentPath);
        alignLeftObjectButton.Visible = showAlignLeft;
        alignLeftObjectButton.Enabled = showAlignLeft && !string.IsNullOrWhiteSpace(currentPath);
        alignTopObjectButton.Visible = showAlignTop;
        alignTopObjectButton.Enabled = showAlignTop && !string.IsNullOrWhiteSpace(currentPath);
        matchWidthObjectButton.Visible = showMatchWidth;
        matchWidthObjectButton.Enabled = showMatchWidth && !string.IsNullOrWhiteSpace(currentPath);
        matchHeightObjectButton.Visible = showMatchHeight;
        matchHeightObjectButton.Enabled = showMatchHeight && !string.IsNullOrWhiteSpace(currentPath);
        matchSizeObjectButton.Visible = showMatchSize;
        matchSizeObjectButton.Enabled = showMatchSize && !string.IsNullOrWhiteSpace(currentPath);
        distributeHorizontalObjectButton.Visible = showDistributeHorizontal;
        distributeHorizontalObjectButton.Enabled = showDistributeHorizontal && !string.IsNullOrWhiteSpace(currentPath);
        distributeVerticalObjectButton.Visible = showDistributeVertical;
        distributeVerticalObjectButton.Enabled = showDistributeVertical && !string.IsNullOrWhiteSpace(currentPath);
        snapHorizontalObjectButton.Visible = showSnapHorizontal;
        snapHorizontalObjectButton.Enabled = showSnapHorizontal && !string.IsNullOrWhiteSpace(currentPath);
        snapToGridObjectButton.Visible = showSnapToGrid;
        snapToGridObjectButton.Enabled = showSnapToGrid && !string.IsNullOrWhiteSpace(currentPath);
        deleteObjectButton.Visible = showDelete;
        deleteObjectButton.Enabled = showDelete && !string.IsNullOrWhiteSpace(currentPath);
        restoreObjectButton.Visible = showRestore;
        restoreObjectButton.Enabled = showRestore && !string.IsNullOrWhiteSpace(currentPath);
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
                    var sectionRecords = EnumerateVisibleReportSectionRecordIndexes(currentSnapshot.ReportLayout, reportSection)
                        .ToHashSet();
                    return currentSnapshot.Objects.Where(item => sectionRecords.Contains(item.RecordIndex)).ToList();
                }

                if (selectedSection is ReportGroupingScope)
                {
                    return Array.Empty<CopperfinStudioSnapshotObject>();
                }

                if (selectedSection is ReportSettingsScope)
                {
                    return Array.Empty<CopperfinStudioSnapshotObject>();
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

    private static int CountVisibleReportSectionObjects(
        CopperfinStudioReportLayout reportLayout,
        CopperfinStudioReportSection section)
    {
        return EnumerateVisibleReportSectionRecordIndexes(reportLayout, section).Count();
    }

    private static int CountVisibleReportGroupingSections(CopperfinStudioReportGrouping grouping)
    {
        var count = 0;
        if (grouping.HeaderRecordIndex.HasValue || !string.IsNullOrWhiteSpace(grouping.HeaderSectionId))
        {
            count++;
        }

        if (grouping.FooterRecordIndex.HasValue || !string.IsNullOrWhiteSpace(grouping.FooterSectionId))
        {
            count++;
        }

        return count;
    }

    private static IEnumerable<int> EnumerateVisibleReportSectionRecordIndexes(
        CopperfinStudioReportLayout reportLayout,
        CopperfinStudioReportSection section)
    {
        foreach (var layoutObject in section.Objects)
        {
            yield return layoutObject.RecordIndex;
        }

        foreach (var layoutObject in reportLayout.DeletedObjects)
        {
            if (layoutObject.ContainingSectionRecordIndex == section.RecordIndex)
            {
                yield return layoutObject.RecordIndex;
            }
        }
    }

    private IReadOnlyList<CopperfinStudioSnapshotObject> TryGetSelectedSnapshotObjects()
    {
        return objectListView.SelectedItems
            .Cast<ListViewItem>()
            .Select(item => item.Tag as CopperfinStudioSnapshotObject)
            .Where(item => item is not null)
            .Cast<CopperfinStudioSnapshotObject>()
            .ToList();
    }

    private CopperfinStudioSnapshotObject? TryGetSelectedSnapshotObject()
    {
        var selectedItem = objectListView.FocusedItem is { Selected: true }
            ? objectListView.FocusedItem
            : objectListView.SelectedItems
                .Cast<ListViewItem>()
                .OrderBy(item => item.Index)
                .FirstOrDefault();
        return selectedItem?.Tag as CopperfinStudioSnapshotObject;
    }

    private void RestoreSnapshotObjectSelection(IReadOnlyCollection<int> recordIndexes, int focusedRecordIndex)
    {
        if (recordIndexes.Count == 0)
        {
            return;
        }

        try
        {
            suppressSelectionSync = true;
            foreach (ListViewItem item in objectListView.Items)
            {
                var snapshotObject = item.Tag as CopperfinStudioSnapshotObject;
                item.Selected = snapshotObject is not null && recordIndexes.Contains(snapshotObject.RecordIndex);
                item.Focused = snapshotObject is not null && snapshotObject.RecordIndex == focusedRecordIndex;
            }

            var focusedObject = objectListView.Items
                .Cast<ListViewItem>()
                .Where(item => item.Focused && item.Selected)
                .Select(item => item.Tag as CopperfinStudioSnapshotObject)
                .FirstOrDefault(item => item is not null);
            if (focusedObject is not null && currentSnapshot is not null)
            {
                propertyGrid.SelectedObject = CopperfinDesignerSelection.FromSnapshot(
                    currentSnapshot.AssetFamily,
                    focusedObject,
                    localization);
            }
        }
        finally
        {
            suppressSelectionSync = false;
            UpdateObjectLifecycleButtonVisibility();
        }
    }

    private static string? TryReadObjectUniqueId(CopperfinStudioSnapshotObject snapshotObject)
    {
        return snapshotObject.Properties
            .FirstOrDefault(property =>
                string.Equals(property.Name, "UNIQUEID", StringComparison.OrdinalIgnoreCase) &&
                !string.IsNullOrWhiteSpace(property.Value))
            ?.Value;
    }

    private static int? TryReadObjectRecordIndex(CopperfinStudioSnapshotDocument snapshot, string uniqueId)
    {
        return snapshot.Objects
            .FirstOrDefault(snapshotObject =>
                string.Equals(TryReadObjectUniqueId(snapshotObject), uniqueId, StringComparison.OrdinalIgnoreCase))
            ?.RecordIndex;
    }

    private static bool TryReadReportGridDimensions(CopperfinStudioSnapshotDocument snapshot, out double gridWidth, out double gridHeight)
    {
        gridWidth = 0.0;
        gridHeight = 0.0;
        return TryReadReportGridDimension(snapshot, "GRIDH", out gridWidth) &&
               TryReadReportGridDimension(snapshot, "GRIDV", out gridHeight);
    }

    private static bool TryReadReportGridDimension(CopperfinStudioSnapshotDocument snapshot, string settingName, out double value)
    {
        value = 0.0;
        var rawValue = snapshot.ReportLayout?.Settings
            .FirstOrDefault(setting => string.Equals(setting.Name, settingName, StringComparison.OrdinalIgnoreCase))
            ?.Value;
        return !string.IsNullOrWhiteSpace(rawValue) &&
               double.TryParse(rawValue, NumberStyles.Float, CultureInfo.InvariantCulture, out value) &&
               value > 0.0;
    }

    private static string CreateDuplicateObjectUniqueId()
    {
        var configured = Environment.GetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID");
        return string.IsNullOrWhiteSpace(configured)
            ? Guid.NewGuid().ToString("D")
            : configured.Trim();
    }

    private static string CreateRenameObjectUniqueId()
    {
        var configured = Environment.GetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID");
        return string.IsNullOrWhiteSpace(configured)
            ? Guid.NewGuid().ToString("D")
            : configured.Trim();
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

        if (selectedTag is ReportGroupingScope groupingScope)
        {
            return new ExplorerSelectionState
            {
                ReportGroupingIndex = groupingScope.Grouping.GroupingIndex
            };
        }

        if (selectedTag is ReportSettingsScope)
        {
            return new ExplorerSelectionState
            {
                ReportSettings = true,
                ReportSettingsDeleted = ((ReportSettingsScope)selectedTag).Deleted
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
        else if (selectionState?.ReportGroupingIndex is int reportGroupingIndex)
        {
            selectedItem = sectionListView.Items
                .Cast<ListViewItem>()
                .FirstOrDefault(item => item.Tag is ReportGroupingScope scope &&
                                        scope.Grouping.GroupingIndex == reportGroupingIndex);
        }
        else if (selectionState?.ReportSettings == true)
        {
            selectedItem = sectionListView.Items
                .Cast<ListViewItem>()
                .FirstOrDefault(item => item.Tag is ReportSettingsScope scope &&
                                        scope.Deleted == selectionState.ReportSettingsDeleted);
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
            snapshot.ReportLayout.DeletedSections.Count,
            snapshot.ReportLayout.Groupings.Count,
            snapshot.ReportLayout.Settings.Count,
            snapshot.ReportLayout.DeletedSettings.Count,
            snapshot.ReportLayout.UnplacedObjects.Count,
            snapshot.ReportLayout.DeletedObjects.Count);

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

    private string BuildReportGroupingListTitle(CopperfinStudioReportGrouping grouping)
    {
        if (string.IsNullOrWhiteSpace(grouping.Expression))
        {
            return F("AssetEditor.ReportSection.Grouping", grouping.GroupingIndex);
        }

        return F(
            "AssetEditor.ReportSection.GroupingWithExpression",
            grouping.GroupingIndex,
            grouping.Expression);
    }

    private string BuildObjectListSubtitle(string assetFamily, string subtitle)
    {
        if (string.IsNullOrWhiteSpace(subtitle))
        {
            return subtitle;
        }

        if (assetFamily == "report" || assetFamily == "label")
        {
            return BuildReportObjectKindDisplayText(subtitle);
        }

        return subtitle;
    }

    private string BuildReportObjectKindDisplayText(string objectKind)
    {
        var key = objectKind switch
        {
            "label" => "AssetEditor.ReportObjectKind.Label",
            "line" => "AssetEditor.ReportObjectKind.Line",
            "rectangle" => "AssetEditor.ReportObjectKind.Rectangle",
            "field" => "AssetEditor.ReportObjectKind.Field",
            "band" => "AssetEditor.ReportObjectKind.Band",
            "group" => "AssetEditor.ReportObjectKind.Group",
            "picture" => "AssetEditor.ReportObjectKind.Picture",
            "variable" => "AssetEditor.ReportObjectKind.Variable",
            "object" => "AssetEditor.ReportObjectKind.Object",
            _ => string.Empty
        };

        if (!string.IsNullOrWhiteSpace(key))
        {
            return L(key);
        }

        return objectKind.Replace('_', ' ');
    }

    private string BuildFallbackObjectTitle(int recordIndex)
    {
        return F("AssetEditor.ObjectFallbackTitle", recordIndex);
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
