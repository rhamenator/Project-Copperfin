// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
#if COPPERFIN_VISUAL_STUDIO
using Microsoft.VisualStudio.PlatformUI;
using Microsoft.VisualStudio.Shell;
#endif

namespace Copperfin.VisualStudio;

internal sealed partial class CopperfinAssetEditorControl : UserControl
{
    public event Action<string>? OpenDocumentRequested;
    public event Action<string, int>? OpenDocumentAtLineRequested;

    private sealed class ReportUnplacedObjectScope
    {
        public List<int> RecordIndexes { get; } = new();
    }

    private sealed class ReportSettingsScope
    {
        public int RecordIndex { get; set; }
        public bool Deleted { get; set; }
        public List<CopperfinStudioNamedValue> Settings { get; } = new();
        public List<string> AvailablePropertyNames { get; } = new();
    }

    private sealed class ReportGroupingScope
    {
        public CopperfinStudioReportGrouping Grouping { get; set; } = new();
    }

    private sealed class ToolboxContextOption
    {
        public ToolboxContextOption(string id, string displayName)
        {
            Id = id;
            DisplayName = displayName;
        }

        public string Id { get; }
        public string DisplayName { get; }

        public override string ToString() => DisplayName;
    }

    private sealed class CoverageLocation
    {
        public string FilePath { get; set; } = string.Empty;
        public int Line { get; set; }
        public int HitCount { get; set; }
        public string Category { get; set; } = string.Empty;
        public string Detail { get; set; } = string.Empty;
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
    private readonly Button snapVerticalObjectButton;
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
    private readonly TextBox debuggerWatchExpressionBox;
    private readonly Button debuggerEvaluateWatchButton;
    private readonly TextBox debuggerBreakpointSpecificationBox;
    private readonly Button debuggerAddBreakpointButton;
    private readonly Button debuggerRemoveBreakpointButton;
    private readonly Button debuggerClearBreakpointsButton;
    private readonly SplitContainer leftExplorerSplit;
    private readonly ListView sectionListView;
    private readonly ListView objectListView;
    private readonly PropertyGrid propertyGrid;
    private readonly CopperfinDesignSurfaceControl designSurface;
    private readonly RichTextBox workspaceSummaryBox;
    private readonly TabControl projectWorkspaceTabs;
    private readonly RichTextBox debuggerSummaryBox;
    private readonly TabControl debuggerDetailTabs;
    private readonly ListView debuggerCallStackView;
    private readonly ListView debuggerLocalsView;
    private readonly ListView debuggerGlobalsView;
    private readonly ListView debuggerEventsView;
    private readonly ListView debuggerWatchesView;
    private readonly ListView debuggerBreakpointsView;
    private readonly RichTextBox taskListSummaryBox;
    private readonly ListView taskListView;
    private readonly RichTextBox codeReferencesSummaryBox;
    private readonly ListView codeReferencesView;
    private readonly RichTextBox dataExplorerSummaryBox;
    private readonly ListView dataExplorerView;
    private readonly RichTextBox objectBrowserSummaryBox;
    private readonly ListView objectBrowserView;
    private readonly RichTextBox toolboxSummaryBox;
    private readonly ListView toolboxPaletteList;
    private readonly Button toolboxCreateButton;
    private readonly Label toolboxContextLabel;
    private readonly ComboBox toolboxContextComboBox;
    private readonly Label toolboxStatusLabel;
    private readonly RichTextBox buildersSummaryBox;
    private readonly ListView buildersView;
    private readonly Button buildersExecuteButton;
    private readonly Label buildersStatusLabel;
    private readonly RichTextBox coverageSummaryBox;
    private readonly ListView coverageView;
    private readonly RichTextBox databaseSummaryBox;
    private readonly ListView databaseView;
    private readonly TextBox dataExplorerFilterBox;
    private readonly TextBox objectBrowserFilterBox;
    private readonly CheckBox objectBrowserHideProjectCheckBox;
    private readonly Label debuggerStatusLabel;
    private readonly CopperfinLocalization localization;

    private string? currentPath;
    private string? currentStartupObjectName;
    private string? currentStartupUniqueId;
    private CopperfinStudioSnapshotDocument? currentSnapshot;
    private CopperfinRuntimeDebugSession? currentDebugSession;
    private CopperfinProjectExecutionResult? currentProjectWorkflowResult;
    private CopperfinProjectInsights? currentProjectInsights;
    private int debugSessionGeneration;
    private bool suppressSelectionSync;
    private bool suppressToolboxContextChange;
    private bool embeddedStudioShell;
    private readonly Dictionary<Control, (Color BackColor, Color ForeColor)> standaloneControlStyles = new();
    private bool standaloneControlStylesCaptured;
    private int loadGeneration;
    private readonly object uiActionGate = new();
    private readonly Queue<Action> pendingUiActions = new();

    public bool EmbeddedStudioShell
    {
        get => embeddedStudioShell;
        set
        {
            embeddedStudioShell = value;
            ApplyHostMode();
        }
    }

    internal bool SuppressProjectWorkflowDialogs { get; set; }

    internal bool SuppressDebuggerDialogs { get; set; }

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
                LoadDocument(currentPath!, currentStartupObjectName, currentStartupUniqueId);
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

        snapVerticalObjectButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.ObjectSnap.VerticalButton"),
            Visible = false
        };
        snapVerticalObjectButton.Click += (_, _) => TryHandleSnapObjectCommand(
            snapMode: "vertical",
            executingKey: "AssetEditor.ObjectSnap.Vertical.Executing",
            failedKey: "AssetEditor.ObjectSnap.Vertical.Failed",
            completedKey: "AssetEditor.ObjectSnap.Vertical.Completed");

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
        debugContinueButton.Click += (_, _) => QueueUiAction(() => AdvanceDebugSessionAsync(session => CopperfinRuntimeDebugClient.ContinueAsync(session, this.localization)));

        debugStepButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.StepButton")
        };
        debugStepButton.Click += (_, _) => QueueUiAction(() => AdvanceDebugSessionAsync(session => CopperfinRuntimeDebugClient.StepIntoAsync(session, this.localization)));

        debugNextButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.NextButton")
        };
        debugNextButton.Click += (_, _) => QueueUiAction(() => AdvanceDebugSessionAsync(session => CopperfinRuntimeDebugClient.StepOverAsync(session, this.localization)));

        debugOutButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.OutButton")
        };
        debugOutButton.Click += (_, _) => QueueUiAction(() => AdvanceDebugSessionAsync(session => CopperfinRuntimeDebugClient.StepOutAsync(session, this.localization)));

        debuggerWatchExpressionBox = new TextBox
        {
            Width = 360,
            AccessibleName = this.localization.Text("AssetEditor.Debugger.WatchExpressionLabel")
        };
        debuggerEvaluateWatchButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.EvaluateWatchButton")
        };
        debuggerEvaluateWatchButton.Click += (_, _) => QueueUiAction(EvaluateWatchAsync);

        debuggerBreakpointSpecificationBox = new TextBox
        {
            Width = 360,
            AccessibleName = this.localization.Text("AssetEditor.Debugger.BreakpointSpecificationLabel")
        };
        debuggerAddBreakpointButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.AddBreakpointButton")
        };
        debuggerAddBreakpointButton.Click += (_, _) => QueueUiAction(AddBreakpointAsync);
        debuggerRemoveBreakpointButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.RemoveBreakpointButton")
        };
        debuggerRemoveBreakpointButton.Click += (_, _) => QueueUiAction(RemoveBreakpointAsync);
        debuggerClearBreakpointsButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Debugger.ClearBreakpointsButton")
        };
        debuggerClearBreakpointsButton.Click += (_, _) => QueueUiAction(ClearBreakpointsAsync);

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
        objectListView.DoubleClick += (_, _) => TryActivateSelectedProjectEntry();
        objectListView.KeyDown += (_, e) =>
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            e.Handled = true;
            e.SuppressKeyPress = true;
            TryActivateSelectedProjectEntry();
        };

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
            Dock = DockStyle.Top,
            Height = 180,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Debugger.InitialSummary")
        };

        debuggerCallStackView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        debuggerCallStackView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Routine"), 300);
        debuggerCallStackView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Location"), 520);

        debuggerLocalsView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        debuggerLocalsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Frame"), 260);
        debuggerLocalsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Name"), 220);
        debuggerLocalsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Value"), 520);

        debuggerGlobalsView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        debuggerGlobalsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Name"), 300);
        debuggerGlobalsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Value"), 700);

        debuggerEventsView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        debuggerEventsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Category"), 220);
        debuggerEventsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Detail"), 500);
        debuggerEventsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Location"), 300);

        debuggerWatchesView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        debuggerWatchesView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Expression"), 360);
        debuggerWatchesView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Value"), 520);
        debuggerWatchesView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Status"), 180);

        debuggerBreakpointsView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        debuggerBreakpointsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.File"), 520);
        debuggerBreakpointsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Line"), 90);
        debuggerBreakpointsView.Columns.Add(this.localization.Text("AssetEditor.Debugger.Column.Action"), 360);
        debuggerBreakpointsView.SelectedIndexChanged += (_, _) =>
        {
            debuggerRemoveBreakpointButton.Enabled = currentDebugSession?.Success == true &&
                                                      debuggerBreakpointsView.SelectedItems.Count == 1;
        };

        debuggerDetailTabs = new TabControl
        {
            Dock = DockStyle.Fill
        };
        var debuggerCallStackPage = new TabPage(this.localization.Text("AssetEditor.Debugger.Tab.CallStack"));
        debuggerCallStackPage.Controls.Add(debuggerCallStackView);
        var debuggerLocalsPage = new TabPage(this.localization.Text("AssetEditor.Debugger.Tab.Locals"));
        debuggerLocalsPage.Controls.Add(debuggerLocalsView);
        var debuggerGlobalsPage = new TabPage(this.localization.Text("AssetEditor.Debugger.Tab.Globals"));
        debuggerGlobalsPage.Controls.Add(debuggerGlobalsView);
        var debuggerEventsPage = new TabPage(this.localization.Text("AssetEditor.Debugger.Tab.RuntimeEvents"));
        debuggerEventsPage.Controls.Add(debuggerEventsView);
        var debuggerWatchesPage = new TabPage(this.localization.Text("AssetEditor.Debugger.Tab.Watches"));
        debuggerWatchesPage.Controls.Add(debuggerWatchesView);
        var debuggerBreakpointsPage = new TabPage(this.localization.Text("AssetEditor.Debugger.Tab.Breakpoints"));
        debuggerBreakpointsPage.Controls.Add(debuggerBreakpointsView);
        debuggerDetailTabs.TabPages.Add(debuggerCallStackPage);
        debuggerDetailTabs.TabPages.Add(debuggerLocalsPage);
        debuggerDetailTabs.TabPages.Add(debuggerGlobalsPage);
        debuggerDetailTabs.TabPages.Add(debuggerEventsPage);
        debuggerDetailTabs.TabPages.Add(debuggerWatchesPage);
        debuggerDetailTabs.TabPages.Add(debuggerBreakpointsPage);

        taskListSummaryBox = new RichTextBox
        {
            Dock = DockStyle.Fill,
            ReadOnly = true,
            BorderStyle = BorderStyle.None,
            BackColor = Color.White,
            Font = new Font("Consolas", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = this.localization.Text("AssetEditor.Placeholder.TaskList")
        };

        taskListView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        taskListView.Columns.Add(this.localization.Text("AssetEditor.TaskList.Column.Category"), 90);
        taskListView.Columns.Add(this.localization.Text("AssetEditor.TaskList.Column.File"), 220);
        taskListView.Columns.Add(this.localization.Text("AssetEditor.TaskList.Column.Line"), 70);
        taskListView.Columns.Add(this.localization.Text("AssetEditor.TaskList.Column.Message"), 560);
        taskListView.DoubleClick += (_, _) => TryActivateSelectedTask();
        taskListView.KeyDown += (_, e) =>
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            e.Handled = true;
            e.SuppressKeyPress = true;
            TryActivateSelectedTask();
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

        codeReferencesView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        codeReferencesView.Columns.Add(this.localization.Text("AssetEditor.CodeReferences.Column.Kind"), 100);
        codeReferencesView.Columns.Add(this.localization.Text("AssetEditor.CodeReferences.Column.Symbol"), 200);
        codeReferencesView.Columns.Add(this.localization.Text("AssetEditor.CodeReferences.Column.File"), 220);
        codeReferencesView.Columns.Add(this.localization.Text("AssetEditor.CodeReferences.Column.Line"), 70);
        codeReferencesView.Columns.Add(this.localization.Text("AssetEditor.CodeReferences.Column.Detail"), 520);
        codeReferencesView.DoubleClick += (_, _) => TryActivateSelectedCodeReference();
        codeReferencesView.KeyDown += (_, e) =>
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            e.Handled = true;
            e.SuppressKeyPress = true;
            TryActivateSelectedCodeReference();
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

        dataExplorerView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        dataExplorerView.Columns.Add(this.localization.Text("AssetEditor.DataExplorer.Column.Kind"), 110);
        dataExplorerView.Columns.Add(this.localization.Text("AssetEditor.DataExplorer.Column.Title"), 220);
        dataExplorerView.Columns.Add(this.localization.Text("AssetEditor.DataExplorer.Column.File"), 320);
        dataExplorerView.Columns.Add(this.localization.Text("AssetEditor.DataExplorer.Column.Group"), 180);
        dataExplorerView.DoubleClick += (_, _) => TryActivateSelectedDataAsset();
        dataExplorerView.KeyDown += (_, e) =>
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            e.Handled = true;
            e.SuppressKeyPress = true;
            TryActivateSelectedDataAsset();
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

        objectBrowserView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        objectBrowserView.Columns.Add(this.localization.Text("AssetEditor.ObjectBrowser.Column.Kind"), 120);
        objectBrowserView.Columns.Add(this.localization.Text("AssetEditor.ObjectBrowser.Column.Title"), 220);
        objectBrowserView.Columns.Add(this.localization.Text("AssetEditor.ObjectBrowser.Column.File"), 320);
        objectBrowserView.Columns.Add(this.localization.Text("AssetEditor.ObjectBrowser.Column.Detail"), 420);
        objectBrowserView.DoubleClick += (_, _) => TryActivateSelectedObjectNode();
        objectBrowserView.KeyDown += (_, e) =>
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            e.Handled = true;
            e.SuppressKeyPress = true;
            TryActivateSelectedObjectNode();
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

        toolboxPaletteList = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        toolboxPaletteList.Columns.Add(this.localization.Text("AssetEditor.Toolbox.Column.Item"), 220);
        toolboxPaletteList.Columns.Add(this.localization.Text("AssetEditor.Toolbox.Column.Category"), 150);
        toolboxPaletteList.Columns.Add(this.localization.Text("AssetEditor.Toolbox.Column.Class"), 150);
        toolboxCreateButton = new Button
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Toolbox.CreateButton"),
            Enabled = false
        };
        toolboxCreateButton.Click += (_, _) => QueueUiAction(TryCreateSelectedToolboxItemAsync);
        toolboxPaletteList.SelectedIndexChanged += (_, _) =>
        {
            toolboxCreateButton.Enabled = toolboxPaletteList.SelectedItems.Count == 1 &&
                                          currentSnapshot?.ReadOnly == false &&
                                          !string.IsNullOrWhiteSpace(currentPath);
        };

        toolboxContextLabel = new Label
        {
            AutoSize = true,
            Text = this.localization.Text("AssetEditor.Toolbox.ContextLabel")
        };
        toolboxContextComboBox = new ComboBox
        {
            DropDownStyle = ComboBoxStyle.DropDownList,
            Enabled = false,
            Width = 150
        };
        toolboxContextComboBox.SelectedIndexChanged += (_, _) =>
        {
            var path = currentPath;
            if (suppressToolboxContextChange ||
                toolboxContextComboBox.SelectedItem is not ToolboxContextOption toolboxContextOption ||
                currentSnapshot is null ||
                path is null ||
                string.IsNullOrWhiteSpace(path))
            {
                return;
            }

            _ = LoadToolboxPaletteAsync(
                path,
                currentSnapshot.AssetFamily,
                loadGeneration,
                toolboxContextOption.Id);
        };

        toolboxStatusLabel = new Label
        {
            AutoSize = true,
            Dock = DockStyle.Fill,
            Text = this.localization.Text("AssetEditor.Toolbox.Loading")
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

        buildersView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        buildersView.Columns.Add(this.localization.Text("AssetEditor.Builders.Column.Kind"), 90);
        buildersView.Columns.Add(this.localization.Text("AssetEditor.Builders.Column.Builder"), 220);
        buildersView.Columns.Add(this.localization.Text("AssetEditor.Builders.Column.Context"), 150);
        buildersView.Columns.Add(this.localization.Text("AssetEditor.Builders.Column.Description"), 520);
        buildersView.DoubleClick += (_, _) => QueueUiAction(PlanSelectedBuilderAsync);
        buildersView.KeyDown += (_, e) =>
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            e.Handled = true;
            e.SuppressKeyPress = true;
            QueueUiAction(PlanSelectedBuilderAsync);
        };

        buildersStatusLabel = new Label
        {
            AutoSize = true,
            Dock = DockStyle.Top,
            Text = this.localization.Text("AssetEditor.Builders.Status.Ready")
        };

        buildersExecuteButton = new Button
        {
            AutoSize = true,
            Enabled = false,
            Text = this.localization.Text("AssetEditor.Builders.ExecuteButton")
        };
        buildersExecuteButton.Click += (_, _) => QueueUiAction(ExecuteSelectedBuilderAsync);
        buildersView.SelectedIndexChanged += (_, _) =>
        {
            buildersExecuteButton.Enabled = buildersView.SelectedItems.Count == 1;
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

        coverageView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        coverageView.Columns.Add(this.localization.Text("AssetEditor.Coverage.Column.Location"), 320);
        coverageView.Columns.Add(this.localization.Text("AssetEditor.Coverage.Column.Hits"), 70);
        coverageView.Columns.Add(this.localization.Text("AssetEditor.Coverage.Column.Category"), 180);
        coverageView.Columns.Add(this.localization.Text("AssetEditor.Coverage.Column.Detail"), 460);
        coverageView.DoubleClick += (_, _) => TryActivateSelectedCoverage();
        coverageView.KeyDown += (_, e) =>
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            e.Handled = true;
            e.SuppressKeyPress = true;
            TryActivateSelectedCoverage();
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

        databaseView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        databaseView.Columns.Add(this.localization.Text("AssetEditor.Database.Column.Kind"), 130);
        databaseView.Columns.Add(this.localization.Text("AssetEditor.Database.Column.Title"), 240);
        databaseView.Columns.Add(this.localization.Text("AssetEditor.Database.Column.Shape"), 300);
        databaseView.Columns.Add(this.localization.Text("AssetEditor.Database.Column.Details"), 620);

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

        var debuggerWatchPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Top,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false,
            Padding = new Padding(8, 0, 8, 4)
        };
        debuggerWatchPanel.Controls.Add(new Label
        {
            AutoSize = true,
            Padding = new Padding(0, 6, 6, 0),
            Text = this.localization.Text("AssetEditor.Debugger.WatchExpressionLabel")
        });
        debuggerWatchPanel.Controls.Add(debuggerWatchExpressionBox);
        debuggerWatchPanel.Controls.Add(debuggerEvaluateWatchButton);

        var debuggerBreakpointPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Top,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false,
            Padding = new Padding(8, 0, 8, 4)
        };
        debuggerBreakpointPanel.Controls.Add(new Label
        {
            AutoSize = true,
            Padding = new Padding(0, 6, 6, 0),
            Text = this.localization.Text("AssetEditor.Debugger.BreakpointSpecificationLabel")
        });
        debuggerBreakpointPanel.Controls.Add(debuggerBreakpointSpecificationBox);
        debuggerBreakpointPanel.Controls.Add(debuggerAddBreakpointButton);
        debuggerBreakpointPanel.Controls.Add(debuggerRemoveBreakpointButton);
        debuggerBreakpointPanel.Controls.Add(debuggerClearBreakpointsButton);

        var debuggerPageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        debuggerPageHost.Controls.Add(debuggerDetailTabs);
        debuggerPageHost.Controls.Add(debuggerSummaryBox);
        debuggerPageHost.Controls.Add(debuggerStatusPanel);
        debuggerPageHost.Controls.Add(debuggerButtonPanel);
        debuggerPageHost.Controls.Add(debuggerWatchPanel);
        debuggerPageHost.Controls.Add(debuggerBreakpointPanel);

        projectWorkspaceTabs = new TabControl
        {
            Dock = DockStyle.Fill,
            Visible = false
        };
        var summaryPage = new TabPage(this.localization.Text("AssetEditor.Tab.Summary"));
        summaryPage.Controls.Add(workspaceSummaryBox);
        var debuggerPage = new TabPage(this.localization.Text("AssetEditor.Tab.Debugger"));
        debuggerPage.Controls.Add(debuggerPageHost);
        var taskListPageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        var taskListSummaryPanel = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 112
        };
        taskListSummaryPanel.Controls.Add(taskListSummaryBox);
        taskListPageHost.Controls.Add(taskListView);
        taskListPageHost.Controls.Add(taskListSummaryPanel);
        var taskListPage = new TabPage(this.localization.Text("AssetEditor.Tab.TaskList"));
        taskListPage.Controls.Add(taskListPageHost);
        var codeReferencesPageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        var codeReferencesSummaryPanel = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 112
        };
        codeReferencesSummaryPanel.Controls.Add(codeReferencesSummaryBox);
        codeReferencesPageHost.Controls.Add(codeReferencesView);
        codeReferencesPageHost.Controls.Add(codeReferencesSummaryPanel);
        var codeReferencesPage = new TabPage(this.localization.Text("AssetEditor.Tab.CodeReferences"));
        codeReferencesPage.Controls.Add(codeReferencesPageHost);
        var dataExplorerPageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        var dataExplorerSummaryPanel = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 150
        };
        dataExplorerSummaryPanel.Controls.Add(dataExplorerSummaryBox);
        dataExplorerPageHost.Controls.Add(dataExplorerView);
        dataExplorerPageHost.Controls.Add(dataExplorerSummaryPanel);
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
        var objectBrowserSummaryPanel = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 150
        };
        objectBrowserSummaryPanel.Controls.Add(objectBrowserSummaryBox);
        objectBrowserPageHost.Controls.Add(objectBrowserView);
        objectBrowserPageHost.Controls.Add(objectBrowserSummaryPanel);
        objectBrowserPageHost.Controls.Add(objectBrowserOptionsPanel);
        objectBrowserPageHost.Controls.Add(objectBrowserFilterBox);
        var objectBrowserPage = new TabPage(this.localization.Text("AssetEditor.Tab.ObjectBrowser"));
        objectBrowserPage.Controls.Add(objectBrowserPageHost);
        var toolboxPage = new TabPage(this.localization.Text("AssetEditor.Tab.Toolbox"));
        var toolboxActionPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Top,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false,
            Padding = new Padding(6)
        };
        toolboxActionPanel.Controls.Add(toolboxContextLabel);
        toolboxActionPanel.Controls.Add(toolboxContextComboBox);
        toolboxActionPanel.Controls.Add(toolboxCreateButton);
        toolboxActionPanel.Controls.Add(toolboxStatusLabel);
        var toolboxSummaryPanel = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 150
        };
        toolboxSummaryPanel.Controls.Add(toolboxSummaryBox);
        toolboxPage.Controls.Add(toolboxPaletteList);
        toolboxPage.Controls.Add(toolboxSummaryPanel);
        toolboxPage.Controls.Add(toolboxActionPanel);
        var buildersPage = new TabPage(this.localization.Text("AssetEditor.Tab.Builders"));
        var buildersPageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        var buildersSummaryPanel = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 150
        };
        var buildersActionPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Top,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false,
            Padding = new Padding(6)
        };
        buildersActionPanel.Controls.Add(buildersExecuteButton);
        buildersActionPanel.Controls.Add(buildersStatusLabel);
        buildersSummaryPanel.Controls.Add(buildersSummaryBox);
        buildersPageHost.Controls.Add(buildersView);
        buildersPageHost.Controls.Add(buildersSummaryPanel);
        buildersPageHost.Controls.Add(buildersActionPanel);
        buildersPage.Controls.Add(buildersPageHost);
        var coveragePage = new TabPage(this.localization.Text("AssetEditor.Tab.Coverage"));
        var coveragePageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        var coverageSummaryPanel = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 150
        };
        coverageSummaryPanel.Controls.Add(coverageSummaryBox);
        coveragePageHost.Controls.Add(coverageView);
        coveragePageHost.Controls.Add(coverageSummaryPanel);
        coveragePage.Controls.Add(coveragePageHost);
        var databasePage = new TabPage(this.localization.Text("AssetEditor.Tab.Database"));
        var databasePageHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        var databaseSummaryPanel = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 180
        };
        databaseSummaryPanel.Controls.Add(databaseSummaryBox);
        databasePageHost.Controls.Add(databaseView);
        databasePageHost.Controls.Add(databaseSummaryPanel);
        databasePage.Controls.Add(databasePageHost);
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
        projectWorkspaceTabs.TabPages.Add(CreateProjectExplorerPage());

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
        buttonPanel.Controls.Add(snapVerticalObjectButton);
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
        HandleCreated += HandleCreatedForPendingUiActions;
        ApplyHostMode();
    }

    internal bool SnapshotLoadFinished =>
        currentSnapshot is not null ||
        !string.Equals(
            snapshotStatusLabel.Text,
            this.localization.Text("AssetEditor.Snapshot.LoadingStatus"),
            StringComparison.Ordinal);

    public bool CanHandleUndoCommand()
    {
        if (TryFindFocusedUndoTextBox() is not null)
        {
            return true;
        }

        return currentSnapshot?.CommandUndoAvailable == true && !string.IsNullOrWhiteSpace(currentPath);
    }

    public bool TryActivateSelectedProjectEntry()
    {
        if (currentSnapshot?.AssetFamily != "project" ||
            string.IsNullOrWhiteSpace(currentPath))
        {
            return false;
        }

        var selectedObject = TryGetSelectedSnapshotObject();
        var projectEntry = selectedObject is null
            ? null
            : LookupProjectEntry(selectedObject.RecordIndex);
        if (selectedObject is null ||
            selectedObject.Deleted ||
            projectEntry is null ||
            projectEntry.Excluded ||
            !CopperfinProjectEntryActivation.TryResolve(currentPath!, projectEntry, out var resolvedPath))
        {
            return false;
        }

        var openDocument = OpenDocumentRequested;
        if (openDocument is null)
        {
            return false;
        }

        openDocument(resolvedPath);
        return true;
    }

    public bool TryActivateSelectedTask()
    {
        if (currentSnapshot?.AssetFamily != "project" ||
            taskListView.SelectedItems.Count != 1 ||
            taskListView.SelectedItems[0].Tag is not CopperfinProjectTaskItem task ||
            !File.Exists(task.FilePath))
        {
            return false;
        }

        var openDocumentAtLine = OpenDocumentAtLineRequested;
        if (openDocumentAtLine is not null)
        {
            openDocumentAtLine(task.FilePath, task.Line);
            return true;
        }

        var openDocument = OpenDocumentRequested;
        if (openDocument is null)
        {
            return false;
        }

        openDocument(task.FilePath);
        return true;
    }

    public bool TryActivateSelectedCodeReference()
    {
        if (currentSnapshot?.AssetFamily != "project" ||
            codeReferencesView.SelectedItems.Count != 1 ||
            codeReferencesView.SelectedItems[0].Tag is not CopperfinProjectCodeSymbol symbol ||
            !File.Exists(symbol.FilePath))
        {
            return false;
        }

        var openDocumentAtLine = OpenDocumentAtLineRequested;
        if (openDocumentAtLine is not null)
        {
            openDocumentAtLine(symbol.FilePath, symbol.Line);
            return true;
        }

        var openDocument = OpenDocumentRequested;
        if (openDocument is null)
        {
            return false;
        }

        openDocument(symbol.FilePath);
        return true;
    }

    public bool TryActivateSelectedDataAsset()
    {
        if (currentSnapshot?.AssetFamily != "project" ||
            dataExplorerView.SelectedItems.Count != 1 ||
            dataExplorerView.SelectedItems[0].Tag is not CopperfinProjectDataAsset asset ||
            asset.Excluded ||
            !File.Exists(asset.FilePath))
        {
            return false;
        }

        var openDocument = OpenDocumentRequested;
        if (openDocument is null)
        {
            return false;
        }

        openDocument(asset.FilePath);
        return true;
    }

    public bool TryActivateSelectedObjectNode()
    {
        if (currentSnapshot?.AssetFamily != "project" ||
            objectBrowserView.SelectedItems.Count != 1 ||
            objectBrowserView.SelectedItems[0].Tag is not CopperfinProjectObjectNode node ||
            node.Excluded ||
            string.IsNullOrWhiteSpace(node.FilePath) ||
            !File.Exists(node.FilePath))
        {
            return false;
        }

        var openDocument = OpenDocumentRequested;
        if (openDocument is null)
        {
            return false;
        }

        openDocument(node.FilePath);
        return true;
    }

    public bool TryActivateSelectedCoverage()
    {
        if (coverageView.SelectedItems.Count != 1 ||
            coverageView.SelectedItems[0].Tag is not CoverageLocation coverage ||
            coverage.Line < 1 ||
            string.IsNullOrWhiteSpace(coverage.FilePath) ||
            !File.Exists(coverage.FilePath))
        {
            return false;
        }

        var openDocumentAtLine = OpenDocumentAtLineRequested;
        if (openDocumentAtLine is not null)
        {
            openDocumentAtLine(coverage.FilePath, coverage.Line);
            return true;
        }

        var openDocument = OpenDocumentRequested;
        if (openDocument is null)
        {
            return false;
        }

        openDocument(coverage.FilePath);
        return true;
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

        if (!CanMutateCurrentDocument())
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
                : null,
            localization);
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
        if (selectedObjectRecordIndex < 0)
        {
            SyncExplorerSelection();
        }
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

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            debugSessionGeneration++;
            if (currentDebugSession is not null)
            {
                CopperfinRuntimeDebugClient.Stop(currentDebugSession);
                currentDebugSession = null;
            }
        }

        base.Dispose(disposing);
    }

    public void LoadDocument(string path, string? objectName = null, string? uniqueId = null)
    {
        loadGeneration++;
        debugSessionGeneration++;
        if (!string.Equals(currentPath, path, StringComparison.OrdinalIgnoreCase) ||
            objectName is not null ||
            uniqueId is not null)
        {
            currentStartupObjectName = objectName;
            currentStartupUniqueId = uniqueId;
        }
        currentPath = path;

        var info = new FileInfo(path);
        titleLabel.Text = CopperfinStudioHostBridge.DescribeAssetKind(path, localization);
        pathLabel.Text = path;
        detailsLabel.Text = BuildSnapshotDetailsText(info, null);
        launchButton.Enabled = true;
        revealButton.Enabled = true;
        refreshButton.Enabled = true;
        currentSnapshot = null;
        if (currentDebugSession is not null)
        {
            CopperfinRuntimeDebugClient.Stop(currentDebugSession);
        }
        currentDebugSession = null;
        currentProjectWorkflowResult = null;
        currentProjectInsights = null;
        sectionListView.Items.Clear();
        objectListView.Items.Clear();
        propertyGrid.SelectedObject = null;
        designSurface.LoadObjects(string.Empty, Array.Empty<CopperfinStudioSnapshotObject>());
        workspaceSummaryBox.Text = string.Empty;
        workspaceSummaryBox.Visible = false;
        projectWorkspaceTabs.Visible = false;
        debuggerSummaryBox.Text = this.localization.Text("AssetEditor.Debugger.InitialSummary");
        ClearDebuggerDetails();
        taskListSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.TaskList");
        taskListView.Items.Clear();
        codeReferencesSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.CodeReferences");
        codeReferencesView.Items.Clear();
        dataExplorerSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.DataExplorer");
        dataExplorerView.Items.Clear();
        objectBrowserSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.ObjectBrowser");
        objectBrowserView.Items.Clear();
        toolboxSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.Toolbox");
        toolboxPaletteList.Items.Clear();
        toolboxContextComboBox.Items.Clear();
        toolboxContextComboBox.Enabled = false;
        toolboxCreateButton.Enabled = false;
        toolboxStatusLabel.Text = this.localization.Text("AssetEditor.Toolbox.Loading");
        buildersSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.Builders");
        buildersView.Items.Clear();
        coverageSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.Coverage");
        coverageView.Items.Clear();
        databaseSummaryBox.Text = this.localization.Text("AssetEditor.Placeholder.Database");
        databaseView.Items.Clear();
        dataExplorerFilterBox.Text = string.Empty;
        objectBrowserFilterBox.Text = string.Empty;
        objectBrowserHideProjectCheckBox.Checked = false;
        debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.ReadyStatus");
        SetDebuggerButtonsEnabled(false);
        designSurface.Visible = true;
        snapshotStatusLabel.Text = this.localization.Text("AssetEditor.Snapshot.LoadingStatus");
        UpdateProjectCommandVisibility();
        UpdateObjectLifecycleButtonVisibility();
        _ = LoadSnapshotAsync(path, currentStartupObjectName, currentStartupUniqueId);
    }

    private async Task LoadSnapshotAsync(string path, string? objectName, string? uniqueId)
    {
        var expectedGeneration = loadGeneration;
        var snapshotResult = await Task.Run(() => CopperfinStudioSnapshotClient.TryLoad(
            path,
            localization,
            objectName,
            uniqueId));
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
            ConfigureToolboxContexts(snapshotResult.Document.AssetFamily);
        });
    }

    private void ConfigureToolboxContexts(string assetFamily)
    {
        var contexts = new List<ToolboxContextOption>();
        if (assetFamily == "form")
        {
            contexts.Add(new ToolboxContextOption("form", localization.Text("AssetEditor.Toolbox.Context.Form")));
            contexts.Add(new ToolboxContextOption("container", localization.Text("AssetEditor.Toolbox.Context.Container")));
        }
        else if (assetFamily == "class")
        {
            contexts.Add(new ToolboxContextOption("class_designer", localization.Text("AssetEditor.Toolbox.Context.Class")));
        }
        else if (assetFamily is "report" or "label")
        {
            contexts.Add(new ToolboxContextOption("report", localization.Text("AssetEditor.Toolbox.Context.Report")));
        }

        suppressToolboxContextChange = true;
        try
        {
            var path = currentPath;
            toolboxContextComboBox.Items.Clear();
            foreach (var context in contexts)
            {
                toolboxContextComboBox.Items.Add(context);
            }

            toolboxContextComboBox.Enabled = contexts.Count > 0;
            if (contexts.Count > 0 && path is not null && !string.IsNullOrWhiteSpace(path))
            {
                toolboxContextComboBox.SelectedIndex = 0;
                _ = LoadToolboxPaletteAsync(
                    path,
                    assetFamily,
                    loadGeneration,
                    contexts[0].Id);
            }
        }
        finally
        {
            suppressToolboxContextChange = false;
        }

    }

    private async Task LoadToolboxPaletteAsync(
        string path,
        string assetFamily,
        int expectedGeneration,
        string toolboxContext)
    {
        if (assetFamily is not ("form" or "class" or "report" or "label"))
        {
            toolboxPaletteList.Items.Clear();
            toolboxCreateButton.Enabled = false;
            toolboxStatusLabel.Text = localization.Text("AssetEditor.Toolbox.Empty");
            return;
        }

        var paletteResult = await Task.Run(() =>
            CopperfinStudioSnapshotClient.TryLoadToolboxPalette(assetFamily, localization, toolboxContext));
        if (IsDisposed || Disposing || expectedGeneration != loadGeneration ||
            !string.Equals(currentPath, path, StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        PostToUi(() =>
        {
            if (!paletteResult.Success)
            {
                toolboxPaletteList.Items.Clear();
                toolboxCreateButton.Enabled = false;
                toolboxStatusLabel.Text = localization.Format(
                    "AssetEditor.Toolbox.Unavailable",
                    paletteResult.Error);
                return;
            }

            toolboxPaletteList.BeginUpdate();
            try
            {
                toolboxPaletteList.Items.Clear();
                foreach (var item in paletteResult.Items)
                {
                    var listItem = new ListViewItem(item.Title);
                    listItem.SubItems.Add(item.Category);
                    listItem.SubItems.Add(item.VfpClass);
                    listItem.ToolTipText = item.Description;
                    listItem.Tag = item;
                    toolboxPaletteList.Items.Add(listItem);
                }
            }
            finally
            {
                toolboxPaletteList.EndUpdate();
            }

            toolboxCreateButton.Enabled = !currentSnapshot!.ReadOnly &&
                                          toolboxPaletteList.SelectedItems.Count == 1;
            toolboxStatusLabel.Text = toolboxPaletteList.Items.Count == 0
                ? localization.Text("AssetEditor.Toolbox.Empty")
                : localization.Format("AssetEditor.Toolbox.Available", toolboxPaletteList.Items.Count);
        });
    }

    private async Task TryCreateSelectedToolboxItemAsync()
    {
        if (toolboxPaletteList.SelectedItems.Count != 1 ||
            currentSnapshot?.ReadOnly != false ||
            string.IsNullOrWhiteSpace(currentPath) ||
            toolboxPaletteList.SelectedItems[0].Tag is not CopperfinStudioToolboxItem item)
        {
            return;
        }

        var path = currentPath!;
        var generation = loadGeneration;
        if (toolboxContextComboBox.SelectedItem is not ToolboxContextOption toolboxContextOption)
        {
            return;
        }

        toolboxCreateButton.Enabled = false;
        toolboxStatusLabel.Text = localization.Text("AssetEditor.Toolbox.Creating");
        var createResult = await Task.Run(() => CopperfinStudioSnapshotClient.TryCreateToolboxItem(
            path,
            item.Id,
            currentSnapshot!.AssetFamily,
            localization,
            toolboxContextOption.Id));
        if (IsDisposed || Disposing || generation != loadGeneration ||
            !string.Equals(currentPath, path, StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        if (!createResult.Success)
        {
            toolboxCreateButton.Enabled = toolboxPaletteList.SelectedItems.Count == 1;
            toolboxStatusLabel.Text = localization.Format(
                "AssetEditor.Toolbox.CreateFailed",
                createResult.Error);
            return;
        }

        toolboxStatusLabel.Text = localization.Format(
            "AssetEditor.Toolbox.Created",
            createResult.ObjectName);
        LoadDocument(path, currentStartupObjectName, currentStartupUniqueId);
    }

    private async Task LoadBuilderCatalogAsync(int expectedGeneration)
    {
        if (currentSnapshot?.ProjectWorkspace is null || currentSnapshot.AssetFamily != "project")
        {
            buildersView.Items.Clear();
            return;
        }

        var catalogResult = await Task.Run(() => CopperfinStudioSnapshotClient.TryLoadBuilderCatalog(localization));
        if (IsDisposed || Disposing || expectedGeneration != loadGeneration)
        {
            return;
        }

        PostToUi(() =>
        {
            if (!catalogResult.Success)
            {
                buildersView.Items.Clear();
                return;
            }

            buildersView.BeginUpdate();
            try
            {
                buildersView.Items.Clear();
                foreach (var entry in catalogResult.Entries
                             .OrderBy(item => item.Context, StringComparer.OrdinalIgnoreCase)
                             .ThenBy(item => item.Title, StringComparer.OrdinalIgnoreCase))
                {
                    var item = new ListViewItem(BuildBuilderKindDisplayText(entry.Kind));
                    item.SubItems.Add(entry.Title);
                    item.SubItems.Add(BuildBuilderContextDisplayText(entry.Context));
                    item.SubItems.Add(entry.Description);
                    item.ToolTipText = entry.Vfp9EquivalentDisplay;
                    item.Tag = entry;
                    buildersView.Items.Add(item);
                }
            }
            finally
            {
                buildersView.EndUpdate();
            }
        });
    }

    private async Task PlanSelectedBuilderAsync()
    {
        if (buildersView.SelectedItems.Count != 1 ||
            buildersView.SelectedItems[0].Tag is not CopperfinStudioBuilderCatalogEntry entry)
        {
            buildersStatusLabel.Text = this.localization.Text("AssetEditor.Builders.Status.NoSelection");
            return;
        }

        var path = currentPath;
        if (string.IsNullOrWhiteSpace(path))
        {
            buildersStatusLabel.Text = this.localization.Text("AssetEditor.Builders.Status.NoPath");
            return;
        }

        var selectedObject = TryGetSelectedSnapshotObject();
        var recordIndex = selectedObject?.RecordIndex;
        var objectName = selectedObject?.Title;
        var uniqueId = selectedObject is null ? null : TryReadObjectUniqueId(selectedObject);
        buildersStatusLabel.Text = this.localization.Text("AssetEditor.Builders.Status.Planning");
        var planResult = await Task.Run(() => CopperfinStudioSnapshotClient.TryPlanBuilderLaunch(
            entry.BuilderId,
            entry.Context,
            path,
            recordIndex,
            objectName,
            uniqueId,
            localization));
        if (IsDisposed || Disposing || !string.Equals(currentPath, path, StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        buildersStatusLabel.Text = planResult.Success
            ? localization.Format(
                "AssetEditor.Builders.Status.PlanReady",
                planResult.Plan.Title,
                planResult.Plan.EntryPoint)
            : localization.Format(
                "AssetEditor.Builders.Status.PlanFailed",
                planResult.Error);
    }

    private async Task ExecuteSelectedBuilderAsync()
    {
        if (buildersView.SelectedItems.Count != 1 ||
            buildersView.SelectedItems[0].Tag is not CopperfinStudioBuilderCatalogEntry entry)
        {
            buildersStatusLabel.Text = this.localization.Text("AssetEditor.Builders.Status.NoSelection");
            return;
        }

        var path = currentPath;
        if (string.IsNullOrWhiteSpace(path))
        {
            buildersStatusLabel.Text = this.localization.Text("AssetEditor.Builders.Status.NoPath");
            return;
        }

        var selectedObject = TryGetSelectedSnapshotObject();
        var recordIndex = selectedObject?.RecordIndex;
        var objectName = selectedObject?.Title;
        var uniqueId = selectedObject is null ? null : TryReadObjectUniqueId(selectedObject);
        buildersExecuteButton.Enabled = false;
        buildersStatusLabel.Text = this.localization.Text("AssetEditor.Builders.Status.Executing");
        var executionResult = await Task.Run(() => CopperfinStudioSnapshotClient.TryExecuteBuilder(
            entry.BuilderId,
            entry.Context,
            path,
            recordIndex,
            objectName,
            uniqueId,
            localization));
        if (IsDisposed || Disposing || !string.Equals(currentPath, path, StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        buildersExecuteButton.Enabled = buildersView.SelectedItems.Count == 1;
        buildersStatusLabel.Text = executionResult.Success
            ? localization.Format(
                "AssetEditor.Builders.Status.ExecutionReady",
                entry.Title,
                executionResult.ObservedExitCode)
            : localization.Format(
                "AssetEditor.Builders.Status.ExecutionFailed",
                executionResult.Error);
    }

    private void HandleCreatedForPendingUiActions(object? sender, EventArgs e)
    {
        Action[] pendingActions;
        lock (uiActionGate)
        {
            pendingActions = pendingUiActions.ToArray();
            pendingUiActions.Clear();
        }

        foreach (var pendingAction in pendingActions)
        {
            PostToUi(pendingAction);
        }
    }

    private void PostToUi(Action action)
    {
        if (IsDisposed || Disposing)
        {
            return;
        }

        lock (uiActionGate)
        {
            if (!IsHandleCreated)
            {
                pendingUiActions.Enqueue(action);
                return;
            }
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
                settingsScope.AvailablePropertyNames.AddRange(
                    currentSnapshot?.Objects
                        .FirstOrDefault(candidate => candidate.RecordIndex == settingsScope.RecordIndex)?
                        .Properties
                        .Select(candidate => candidate.Name) ??
                    Enumerable.Empty<string>());

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
                deletedSettingsScope.AvailablePropertyNames.AddRange(
                    currentSnapshot?.Objects
                        .FirstOrDefault(candidate => candidate.RecordIndex == deletedSettingsScope.RecordIndex)?
                        .Properties
                        .Select(candidate => candidate.Name) ??
                    Enumerable.Empty<string>());

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
                var item = new ListViewItem(BuildProjectWorkspaceGroupTitleDisplayText(group.Title, group.Id));
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
                ? BuildProjectWorkspaceGroupTitleDisplayText(projectEntry?.GroupTitle ?? item.Subtitle, projectEntry?.GroupId)
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
            : CopperfinDesignerSelection.FromSnapshot(currentSnapshot.AssetFamily, selectedObject, localization, currentSnapshot.ReadOnly);
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

                propertyGrid.SelectedObject = CopperfinDesignerSelection.FromReportSection(
                    reportSection,
                    localization,
                    currentSnapshot?.ReadOnly == true);
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
                    localization,
                    currentSnapshot?.ReadOnly == true);
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
                    currentSnapshot?.ReportLayout,
                    settingsScope.AvailablePropertyNames,
                    currentSnapshot?.ReadOnly == true);
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
                : CopperfinDesignerSelection.FromSnapshot(currentSnapshot.AssetFamily, selectedObject, localization, currentSnapshot.ReadOnly);
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
            {
                if (item.Tag is CopperfinStudioReportSection section)
                {
                    if (section.Objects.Any(layoutObject => layoutObject.RecordIndex == recordIndex))
                    {
                        return true;
                    }

                    return currentSnapshot.ReportLayout.DeletedObjects.Any(layoutObject =>
                        layoutObject.RecordIndex == recordIndex &&
                        layoutObject.ContainingSectionRecordIndex == section.RecordIndex);
                }

                return item.Tag is ReportUnplacedObjectScope unplacedScope &&
                       unplacedScope.RecordIndexes.Contains(recordIndex);
            });
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
        if (propertyGrid.SelectedObject is not CopperfinDesignerSelection selection ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
        {
            return;
        }

        if (selection.TryGetMutation(
                propertyName,
                out var targetName,
                out var serializedValue,
                out var clearProperty))
        {
            if (clearProperty)
            {
                ApplyVisualPropertyClear(selection.RecordIndex, targetName);
            }
            else
            {
                ApplyVisualPropertyChange(selection.RecordIndex, targetName, serializedValue);
            }
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

    private void ApplyVisualPropertyClear(int recordIndex, string propertyName)
    {
        if (string.IsNullOrWhiteSpace(currentPath) || !CanMutateCurrentDocument())
        {
            return;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        var selectedObjectRecordIndex = TryReadSelectedRecordIndex();
        var statusLabel = ResolvePropertyStatusLabels(new[] { propertyName });
        snapshotStatusLabel.Text = BuildPropertyApplyingStatusLabel(statusLabel);
        var clearResult = CopperfinStudioSnapshotClient.TryClearProperty(
            currentPath!,
            recordIndex,
            propertyName,
            localization);
        if (!clearResult.Success || clearResult.Document is null)
        {
            snapshotStatusLabel.Text = BuildPropertyUpdateFailedStatus(clearResult.Error);
            return;
        }

        currentSnapshot = clearResult.Document;
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

    private void ApplyVisualPropertyChanges(int recordIndex, IReadOnlyList<KeyValuePair<string, string>> propertyChanges)
    {
        if (string.IsNullOrWhiteSpace(currentPath) || propertyChanges.Count == 0 || !CanMutateCurrentDocument())
        {
            return;
        }

        var explorerSelection = CaptureExplorerSelectionState();
        var selectedObjectRecordIndex = TryReadSelectedRecordIndex();
        var statusLabel = ResolvePropertyStatusLabels(propertyChanges.Select(change => change.Key));
        snapshotStatusLabel.Text = BuildPropertyApplyingStatusLabel(statusLabel);
        var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperties(currentPath!, recordIndex, propertyChanges, localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            ? CopperfinStudioSnapshotClient.TryRestoreObject(currentPath!, selectedObjectRecordIndex, uniqueId, localization)
            : CopperfinStudioSnapshotClient.TryDeleteObject(currentPath!, selectedObjectRecordIndex, uniqueId, localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            duplicateUniqueId,
            localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            renamedUniqueId,
            localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            placement,
            localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            targetUniqueIds,
            localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            targetUniqueIds,
            localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            targetUniqueIds,
            localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            targetUniqueIds,
            localization);
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
        if (currentSnapshot?.AssetFamily is not ("report" or "label") ||
            string.IsNullOrWhiteSpace(currentPath) ||
            !CanMutateCurrentDocument())
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
            deltaVpos,
            localization);
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
        RefreshProjectExplorer();
        PopulateTaskList(currentProjectInsights);
        taskListSummaryBox.Text = BuildTaskListSummary(currentProjectInsights);
        PopulateCodeReferences(currentProjectInsights);
        codeReferencesSummaryBox.Text = BuildCodeReferenceSummary(currentProjectInsights);
        PopulateDataExplorer(currentProjectInsights, dataExplorerFilterBox.Text);
        dataExplorerSummaryBox.Text = BuildDataExplorerSummary(currentSnapshot, currentProjectInsights, dataExplorerFilterBox.Text);
        PopulateObjectBrowser(currentProjectInsights, objectBrowserFilterBox.Text, objectBrowserHideProjectCheckBox.Checked);
        objectBrowserSummaryBox.Text = BuildObjectBrowserSummary(currentSnapshot, currentProjectInsights, objectBrowserFilterBox.Text, objectBrowserHideProjectCheckBox.Checked);
        toolboxSummaryBox.Text = BuildToolboxSummary(currentSnapshot, currentProjectInsights);
        buildersSummaryBox.Text = BuildBuilderSummary(currentSnapshot, currentProjectInsights);
        _ = LoadBuilderCatalogAsync(loadGeneration);
        PopulateCoverage(currentDebugSession);
        coverageSummaryBox.Text = BuildCoverageSummary(currentSnapshot, currentDebugSession);
        PopulateDatabaseFederation(currentSnapshot, dataExplorerFilterBox.Text);
        databaseSummaryBox.Text = BuildDatabaseFederationSummary(currentSnapshot, dataExplorerFilterBox.Text);
    }

    private void PopulateDatabaseFederation(CopperfinStudioSnapshotDocument snapshot, string? filter)
    {
        databaseView.Items.Clear();
        if (!snapshot.DatabaseProfile.Available)
        {
            return;
        }

        var normalizedFilter = (filter ?? string.Empty).Trim();
        foreach (var connector in snapshot.DatabaseProfile.Connectors.Where(connector =>
                     string.IsNullOrWhiteSpace(normalizedFilter) ||
                     connector.Title.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                     connector.Family.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                     connector.SchemaShape.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0))
        {
            var item = new ListViewItem(L("AssetEditor.Database.Kind.Connector"));
            item.SubItems.Add(connector.Title);
            item.SubItems.Add(connector.Family + " / " + connector.SchemaShape);
            item.SubItems.Add(connector.TranslationStory);
            item.Tag = connector.Id;
            databaseView.Items.Add(item);
        }

        foreach (var path in snapshot.DatabaseProfile.QueryPaths.Where(path =>
                     string.IsNullOrWhiteSpace(normalizedFilter) ||
                     path.Title.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                     path.SourceShape.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                     path.TargetShape.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0))
        {
            var item = new ListViewItem(L("AssetEditor.Database.Kind.QueryPath"));
            item.SubItems.Add(path.Title);
            item.SubItems.Add(path.SourceShape + " -> " + path.TargetShape);
            item.SubItems.Add(path.Complexity + ": " + path.Strategy);
            item.Tag = path.Id;
            databaseView.Items.Add(item);
        }
    }

    private void PopulateTaskList(CopperfinProjectInsights? insights)
    {
        taskListView.Items.Clear();
        if (insights is null)
        {
            return;
        }

        foreach (var task in insights.TaskItems)
        {
            var item = new ListViewItem(task.Category);
            item.SubItems.Add(Path.GetFileName(task.FilePath));
            item.SubItems.Add(task.Line.ToString(CultureInfo.InvariantCulture));
            item.SubItems.Add(task.Message);
            item.Tag = task;
            taskListView.Items.Add(item);
        }
    }

    private void PopulateCoverage(CopperfinRuntimeDebugSession? session)
    {
        coverageView.Items.Clear();
        if (session is null || !session.Success)
        {
            return;
        }

        var locations = new Dictionary<string, CoverageLocation>(CopperfinDocumentPathIdentity.CreateComparer());
        foreach (var runtimeEvent in session.State.Events)
        {
            if (string.IsNullOrWhiteSpace(runtimeEvent.Location))
            {
                continue;
            }

            var separator = runtimeEvent.Location.LastIndexOf(':');
            if (separator <= 0 ||
                separator == runtimeEvent.Location.Length - 1 ||
                !int.TryParse(
                    runtimeEvent.Location.Substring(separator + 1),
                    NumberStyles.Integer,
                    CultureInfo.InvariantCulture,
                    out var line) ||
                line < 1)
            {
                continue;
            }

            var filePath = runtimeEvent.Location.Substring(0, separator);
            if (!locations.TryGetValue(runtimeEvent.Location, out var coverage))
            {
                coverage = new CoverageLocation
                {
                    FilePath = filePath,
                    Line = line,
                    Category = runtimeEvent.Category,
                    Detail = runtimeEvent.Detail
                };
                locations.Add(runtimeEvent.Location, coverage);
            }

            coverage.HitCount++;
            if (string.IsNullOrWhiteSpace(coverage.Category))
            {
                coverage.Category = runtimeEvent.Category;
            }
            if (string.IsNullOrWhiteSpace(coverage.Detail))
            {
                coverage.Detail = runtimeEvent.Detail;
            }
        }

        foreach (var coverage in locations.Values.OrderBy(item => item.FilePath, StringComparer.OrdinalIgnoreCase).ThenBy(item => item.Line))
        {
            var item = new ListViewItem($"{Path.GetFileName(coverage.FilePath)}:{coverage.Line.ToString(CultureInfo.InvariantCulture)}");
            item.SubItems.Add(coverage.HitCount.ToString(CultureInfo.InvariantCulture));
            item.SubItems.Add(coverage.Category);
            item.SubItems.Add(coverage.Detail);
            item.Tag = coverage;
            coverageView.Items.Add(item);
        }
    }

    private void PopulateCodeReferences(CopperfinProjectInsights? insights)
    {
        codeReferencesView.Items.Clear();
        if (insights is null)
        {
            return;
        }

        foreach (var symbol in insights.DefinedSymbols.Concat(insights.RuntimeReferences))
        {
            var item = new ListViewItem(BuildCodeSymbolKindDisplayText(symbol.Kind));
            item.SubItems.Add(symbol.Name);
            item.SubItems.Add(Path.GetFileName(symbol.FilePath));
            item.SubItems.Add(symbol.Line.ToString(CultureInfo.InvariantCulture));
            item.SubItems.Add(symbol.Detail);
            item.Tag = symbol;
            codeReferencesView.Items.Add(item);
        }
    }

    private void PopulateDataExplorer(CopperfinProjectInsights? insights, string? filter)
    {
        dataExplorerView.Items.Clear();
        if (insights is null)
        {
            return;
        }

        foreach (var asset in FilterDataAssets(insights, filter))
        {
            var item = new ListViewItem(BuildProjectInsightArtifactKindDisplayText(asset.Kind));
            item.SubItems.Add(asset.Title);
            item.SubItems.Add(asset.FilePath);
            item.SubItems.Add(asset.GroupTitle + (asset.Excluded ? L("AssetEditor.Summary.ExcludedSuffix") : string.Empty));
            item.Tag = asset;
            if (asset.Excluded)
            {
                item.ForeColor = Color.Firebrick;
            }

            dataExplorerView.Items.Add(item);
        }
    }

    private List<CopperfinProjectDataAsset> FilterDataAssets(CopperfinProjectInsights insights, string? filter)
    {
        var normalizedFilter = (filter ?? string.Empty).Trim();
        return insights.DataAssets
            .Where(asset =>
                string.IsNullOrWhiteSpace(normalizedFilter) ||
                asset.Title.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                asset.Kind.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                BuildProjectInsightArtifactKindDisplayText(asset.Kind).IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                asset.FilePath.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0)
            .ToList();
    }

    private void PopulateObjectBrowser(CopperfinProjectInsights? insights, string? filter, bool hideProjectRecords)
    {
        objectBrowserView.Items.Clear();
        if (insights is null)
        {
            return;
        }

        foreach (var node in FilterObjectNodes(insights, filter, hideProjectRecords))
        {
            var item = new ListViewItem(BuildProjectInsightArtifactKindDisplayText(node.Kind));
            item.SubItems.Add(node.Title);
            item.SubItems.Add(node.FilePath);
            item.SubItems.Add(BuildObjectBrowserNodeDetailDisplayText(node));
            item.Tag = node;
            if (node.Excluded)
            {
                item.ForeColor = Color.Firebrick;
            }

            objectBrowserView.Items.Add(item);
        }
    }

    private List<CopperfinProjectObjectNode> FilterObjectNodes(
        CopperfinProjectInsights insights,
        string? filter,
        bool hideProjectRecords)
    {
        var normalizedFilter = (filter ?? string.Empty).Trim();
        return insights.ObjectNodes
            .Where(node =>
                (!hideProjectRecords || !string.Equals(node.Kind, "Project Header", StringComparison.OrdinalIgnoreCase)) &&
                (string.IsNullOrWhiteSpace(normalizedFilter) ||
                 node.Title.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                 node.Kind.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                 BuildProjectInsightArtifactKindDisplayText(node.Kind).IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                 BuildObjectBrowserNodeDetailDisplayText(node).IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                 node.Detail.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
                 node.FilePath.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0))
            .ToList();
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
        if (snapshot.ReadOnly)
        {
            status += this.localization.Text("AssetEditor.Snapshot.ReadOnly");
        }
        if (snapshot.CommandUndoAvailable && !string.IsNullOrWhiteSpace(snapshot.CommandUndoLabel))
        {
            status += this.localization.Format("AssetEditor.Snapshot.UndoAvailable", snapshot.CommandUndoLabel);
        }

        return status;
    }

    private bool CanMutateCurrentDocument()
    {
        if (currentSnapshot?.ReadOnly != true)
        {
            return true;
        }

        snapshotStatusLabel.Text = this.localization.Text("AssetEditor.Snapshot.ReadOnly");
        return false;
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

        if (!CopperfinStudioHostBridge.Launch(studioHostPath, currentPath!, localization: localization))
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

        _ = CopperfinFileManager.TryReveal(currentPath!);
    }

    private void ApplyHostMode()
    {
        CaptureStandaloneControlStyles();
        titleLabel.Visible = embeddedStudioShell;
        subtitleLabel.Visible = embeddedStudioShell;
        guidanceLabel.Visible = embeddedStudioShell;
        launchButton.Visible = !embeddedStudioShell;
        buildButton.Visible = !embeddedStudioShell && CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
        runButton.Visible = !embeddedStudioShell && CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
        debugButton.Visible = !embeddedStudioShell && CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
        Padding = embeddedStudioShell
            ? new Padding(24)
            : new Padding(12, 8, 12, 12);
        if (embeddedStudioShell)
        {
            RestoreStandaloneControlStyles();
        }
        else
        {
            ApplyVisualStudioHostTheme();
        }

        subtitleLabel.Text = embeddedStudioShell
            ? this.localization.Text("AssetEditor.StandaloneSubtitle")
            : this.localization.Text("AssetEditor.Subtitle");
    }

#if COPPERFIN_VISUAL_STUDIO
    private void ApplyVisualStudioHostTheme()
    {
        Color background;
        Color foreground;
        try
        {
            background = VSColorTheme.GetThemedColor(EnvironmentColors.ToolWindowBackgroundColorKey);
            foreground = VSColorTheme.GetThemedColor(EnvironmentColors.ToolWindowTextColorKey);
        }
        catch (Exception)
        {
            background = SystemColors.Control;
            foreground = SystemColors.ControlText;
        }

        ApplyHostTheme(background, foreground, SystemInformation.HighContrast);
    }
#else
    private void ApplyVisualStudioHostTheme()
    {
        // Smoke tests and non-VSSDK builds still exercise the same safe fallback
        // boundary used when Visual Studio theme services are unavailable.
        ApplyHostTheme(
            SystemColors.Control,
            SystemColors.ControlText,
            SystemInformation.HighContrast);
    }
#endif

    private void ApplyHostTheme(Color background, Color foreground, bool highContrast = false)
    {
        BackColor = background;
        ForeColor = foreground;
        designSurface.ApplyVisualStudioHostTheme(background, foreground, highContrast);
        ApplyVisualStudioHostThemeToChildren(this, background, foreground, highContrast);
    }

    private void CaptureStandaloneControlStyles()
    {
        if (standaloneControlStylesCaptured)
        {
            return;
        }

        CaptureStandaloneControlStyles(this);
        standaloneControlStylesCaptured = true;
    }

    private void CaptureStandaloneControlStyles(Control control)
    {
        standaloneControlStyles[control] = (control.BackColor, control.ForeColor);
        foreach (Control child in control.Controls)
        {
            CaptureStandaloneControlStyles(child);
        }
    }

    private void RestoreStandaloneControlStyles()
    {
        foreach (var style in standaloneControlStyles)
        {
            style.Key.BackColor = style.Value.BackColor;
            style.Key.ForeColor = style.Value.ForeColor;
        }

        designSurface.ResetVisualStudioHostTheme();
    }

    private static void ApplyVisualStudioHostThemeToChildren(
        Control parent,
        Color background,
        Color foreground,
        bool highContrast)
    {
        foreach (Control child in parent.Controls)
        {
            if (child is CopperfinDesignSurfaceControl)
            {
                ((CopperfinDesignSurfaceControl)child).ApplyVisualStudioHostTheme(
                    background,
                    foreground,
                    highContrast);
                continue;
            }

            // Labels stay transparent so they retain the host surface, while every
            // other shell control must leave the default light WinForms palette.
            if (child is not Label)
            {
                child.BackColor = background;
            }

            child.ForeColor = foreground;

            ApplyVisualStudioHostThemeToChildren(child, background, foreground, highContrast);
        }
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

        currentProjectWorkflowResult = null;
        var result = await CopperfinProjectWorkflow.ExecuteAsync(currentPath!, operation, localization);
        currentProjectWorkflowResult = result;
        snapshotStatusLabel.Text = result.Message;
        if (!result.Success)
        {
            if (!SuppressProjectWorkflowDialogs)
            {
                MessageBox.Show(this, result.Message, DialogTitle, MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }

            return;
        }

        if (operation == CopperfinProjectOperation.Build && !SuppressProjectWorkflowDialogs)
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
        var projectPath = currentPath;
        if (!CopperfinProjectWorkflow.IsCopperfinProjectPath(projectPath))
        {
            return;
        }

        var requestGeneration = ++debugSessionGeneration;
        debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.StartingStatus");
        SetDebuggerButtonsEnabled(false);
        if (currentDebugSession is not null)
        {
            CopperfinRuntimeDebugClient.Stop(currentDebugSession);
            currentDebugSession = null;
        }
        var session = await CopperfinRuntimeDebugClient.StartSessionAsync(projectPath!, localization);
        if (requestGeneration != debugSessionGeneration ||
            !string.Equals(currentPath, projectPath, StringComparison.OrdinalIgnoreCase))
        {
            CopperfinRuntimeDebugClient.Stop(session);
            return;
        }
        if (IsDisposed || Disposing || projectWorkspaceTabs.IsDisposed)
        {
            CopperfinRuntimeDebugClient.Stop(session);
            return;
        }
        ApplyDebugSession(session);
    }

    private async Task AdvanceDebugSessionAsync(Func<CopperfinRuntimeDebugSession, Task<CopperfinRuntimeDebugSession>> action)
    {
        if (currentDebugSession is null || !currentDebugSession.Success)
        {
            ShowDebuggerDialog(this.localization.Text("AssetEditor.Debugger.StartSessionFirstMessage"), MessageBoxIcon.Information);
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

            if (InvokeRequired && IsHandleCreated)
            {
                BeginInvoke(new Action(() => ApplyDebugSession(session)));
                return;
            }

            currentDebugSession = session;
            PopulateDebuggerDetails(session);
            if (!session.Success)
            {
                debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.UnavailableStatus");
                debuggerSummaryBox.Text = session.Error;
                SetDebuggerButtonsEnabled(false);
                ShowDebuggerDialog(session.Error, MessageBoxIcon.Warning);
                return;
            }

            debuggerStatusLabel.Text = session.State.Message;
            debuggerSummaryBox.Text = BuildDebugSessionSummary(session);
            if (currentSnapshot?.ProjectWorkspace is not null && currentSnapshot.AssetFamily == "project")
            {
                PopulateCoverage(session);
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

    private void ShowDebuggerDialog(string message, MessageBoxIcon icon)
    {
        if (!SuppressDebuggerDialogs)
        {
            MessageBox.Show(this, message, DialogTitle, MessageBoxButtons.OK, icon);
        }
    }

    private void ClearDebuggerDetails()
    {
        debuggerCallStackView.Items.Clear();
        debuggerLocalsView.Items.Clear();
        debuggerGlobalsView.Items.Clear();
        debuggerEventsView.Items.Clear();
        debuggerWatchesView.Items.Clear();
        debuggerBreakpointsView.Items.Clear();
    }

    private void PopulateDebuggerDetails(CopperfinRuntimeDebugSession? session)
    {
        ClearDebuggerDetails();
        if (session is null || !session.Success)
        {
            return;
        }

        var state = session.State;
        debuggerCallStackView.BeginUpdate();
        debuggerLocalsView.BeginUpdate();
        debuggerGlobalsView.BeginUpdate();
        debuggerEventsView.BeginUpdate();
        debuggerWatchesView.BeginUpdate();
        debuggerBreakpointsView.BeginUpdate();
        try
        {
            foreach (var frame in state.Frames)
            {
                debuggerCallStackView.Items.Add(
                    new ListViewItem(new[] { frame.RoutineName, frame.Location }));
                foreach (var local in frame.Locals)
                {
                    debuggerLocalsView.Items.Add(
                        new ListViewItem(new[] { frame.RoutineName, local.Name, local.Value }));
                }
            }

            foreach (var global in state.Globals)
            {
                debuggerGlobalsView.Items.Add(
                    new ListViewItem(new[] { global.Name, global.Value }));
            }

            foreach (var runtimeEvent in state.Events)
            {
                debuggerEventsView.Items.Add(
                    new ListViewItem(new[]
                    {
                        runtimeEvent.Category,
                        runtimeEvent.Detail,
                        runtimeEvent.Location
                    }));
            }

            foreach (var watch in state.Watches)
            {
                debuggerWatchesView.Items.Add(new ListViewItem(new[]
                {
                    watch.Expression,
                    watch.Success ? watch.Value : watch.Error,
                    this.localization.Text(watch.Success
                        ? "AssetEditor.Debugger.WatchSucceeded"
                        : "AssetEditor.Debugger.WatchFailed")
                }));
            }

            foreach (var breakpoint in state.Breakpoints)
            {
                var action = string.IsNullOrWhiteSpace(breakpoint.ActionTitle)
                    ? breakpoint.ActionId
                    : breakpoint.ActionTitle;
                debuggerBreakpointsView.Items.Add(new ListViewItem(new[]
                {
                    breakpoint.FilePath,
                    breakpoint.Line.ToString(System.Globalization.CultureInfo.InvariantCulture),
                    action
                }));
            }
        }
        finally
        {
            debuggerBreakpointsView.EndUpdate();
            debuggerWatchesView.EndUpdate();
            debuggerEventsView.EndUpdate();
            debuggerGlobalsView.EndUpdate();
            debuggerLocalsView.EndUpdate();
            debuggerCallStackView.EndUpdate();
        }
    }

    private void SetDebuggerButtonsEnabled(bool enabled)
    {
        debugContinueButton.Enabled = enabled;
        debugStepButton.Enabled = enabled;
        debugNextButton.Enabled = enabled;
        debugOutButton.Enabled = enabled;
        debugRestartButton.Enabled = CopperfinProjectWorkflow.IsCopperfinProjectPath(currentPath);
        debuggerEvaluateWatchButton.Enabled = enabled;
        debuggerAddBreakpointButton.Enabled = enabled;
        debuggerRemoveBreakpointButton.Enabled = enabled && debuggerBreakpointsView.SelectedItems.Count == 1;
        debuggerClearBreakpointsButton.Enabled = enabled && debuggerBreakpointsView.Items.Count > 0;
    }

    private async Task EvaluateWatchAsync()
    {
        var expression = debuggerWatchExpressionBox.Text.Trim();
        if (expression.Length == 0)
        {
            debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.WatchExpressionRequired");
            return;
        }

        if (currentDebugSession is null || !currentDebugSession.Success)
        {
            ShowDebuggerDialog(this.localization.Text("AssetEditor.Debugger.StartSessionFirstMessage"), MessageBoxIcon.Information);
            return;
        }

        debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.EvaluatingWatchStatus");
        SetDebuggerButtonsEnabled(false);
        var session = await CopperfinRuntimeDebugClient.EvaluateWatchAsync(currentDebugSession, expression, this.localization);
        if (IsDisposed || Disposing || projectWorkspaceTabs.IsDisposed)
        {
            return;
        }

        ApplyDebugSession(session);
        debuggerDetailTabs.SelectedIndex = debuggerDetailTabs.TabPages.Count - 1;
    }

    // The Command window is a synchronous WinForms executor boundary; the debug client performs transport work off the UI thread.
#pragma warning disable VSTHRD002
    internal string ExecuteCommandWindowInput(string command)
    {
        var trimmedCommand = command.Trim();
        if (trimmedCommand.Length < 2 ||
            trimmedCommand[0] != '?' ||
            !char.IsWhiteSpace(trimmedCommand[1]))
        {
            return localization.Text("VSIX.CommandWindow.Unsupported");
        }

        var expression = trimmedCommand.Substring(1).Trim();
        if (expression.Length == 0)
        {
            return localization.Text("VSIX.CommandWindow.Unsupported");
        }

        if (currentDebugSession is null || !currentDebugSession.Success)
        {
            return localization.Text("VSIX.CommandWindow.NoActiveSession");
        }

        var session = CopperfinRuntimeDebugClient
            .EvaluateWatchAsync(currentDebugSession, expression, localization)
            .GetAwaiter()
            .GetResult();
        if (IsDisposed || Disposing || projectWorkspaceTabs.IsDisposed)
        {
            return localization.Text("VSIX.CommandWindow.Unavailable");
        }

        if (session.Success)
        {
            ApplyDebugSession(session);
        }

        var watch = session.State.Watches.LastOrDefault(item =>
            string.Equals(item.Expression, expression, StringComparison.OrdinalIgnoreCase));
        if (watch?.Success == true)
        {
            return watch.Value;
        }

        return string.IsNullOrWhiteSpace(watch?.Error)
            ? localization.Text("VSIX.CommandWindow.Unavailable")
            : watch?.Error ?? string.Empty;
    }
#pragma warning restore VSTHRD002

    private async Task AddBreakpointAsync()
    {
        var specification = debuggerBreakpointSpecificationBox.Text.Trim();
        if (specification.Length == 0)
        {
            debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.BreakpointSpecificationRequired");
            return;
        }

        if (currentDebugSession is null || !currentDebugSession.Success)
        {
            ShowDebuggerDialog(this.localization.Text("AssetEditor.Debugger.StartSessionFirstMessage"), MessageBoxIcon.Information);
            return;
        }

        await ApplyBreakpointCommandAsync(
            session => CopperfinRuntimeDebugClient.AddBreakpointAsync(session, specification, this.localization));
    }

    private async Task RemoveBreakpointAsync()
    {
        if (currentDebugSession is null || !currentDebugSession.Success)
        {
            ShowDebuggerDialog(this.localization.Text("AssetEditor.Debugger.StartSessionFirstMessage"), MessageBoxIcon.Information);
            return;
        }

        if (debuggerBreakpointsView.SelectedItems.Count != 1)
        {
            debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.SelectBreakpointRequired");
            return;
        }

        var selected = debuggerBreakpointsView.SelectedItems[0];
        var specification = selected.SubItems[0].Text + ":" + selected.SubItems[1].Text;
        await ApplyBreakpointCommandAsync(
            session => CopperfinRuntimeDebugClient.RemoveBreakpointAsync(session, specification, this.localization));
    }

    private async Task ClearBreakpointsAsync()
    {
        if (currentDebugSession is null || !currentDebugSession.Success)
        {
            ShowDebuggerDialog(this.localization.Text("AssetEditor.Debugger.StartSessionFirstMessage"), MessageBoxIcon.Information);
            return;
        }

        await ApplyBreakpointCommandAsync(
            session => CopperfinRuntimeDebugClient.ClearBreakpointsAsync(session, this.localization));
    }

    private async Task ApplyBreakpointCommandAsync(
        Func<CopperfinRuntimeDebugSession, Task<CopperfinRuntimeDebugSession>> command)
    {
        debuggerStatusLabel.Text = this.localization.Text("AssetEditor.Debugger.UpdatingBreakpointsStatus");
        SetDebuggerButtonsEnabled(false);
        var session = await command(currentDebugSession!);
        if (IsDisposed || Disposing || projectWorkspaceTabs.IsDisposed)
        {
            return;
        }

        ApplyDebugSession(session);
        debuggerDetailTabs.SelectedIndex = debuggerDetailTabs.TabPages.Count - 1;
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
        var canMutate = currentSnapshot?.ReadOnly != true && !string.IsNullOrWhiteSpace(currentPath);
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
        var showSnapVertical = selectedObjects.Count >= 1 &&
                               selectedObjects.All(snapshotObject => !snapshotObject.Deleted) &&
                               currentSnapshot is not null &&
                               TryReadReportGridDimensions(currentSnapshot, out _, out _);
        var showDelete = singleSelection && selectedObject is not null && !selectedObject.Deleted;
        var showRestore = singleSelection && selectedObject is not null && selectedObject.Deleted;
        renameObjectButton.Visible = showRename;
        renameObjectButton.Enabled = showRename && canMutate;
        duplicateObjectButton.Visible = showDuplicate;
        duplicateObjectButton.Enabled = showDuplicate && canMutate;
        reorderFrontObjectButton.Visible = showReorder;
        reorderFrontObjectButton.Enabled = showReorder && canMutate;
        reorderBackObjectButton.Visible = showReorder;
        reorderBackObjectButton.Enabled = showReorder && canMutate;
        alignLeftObjectButton.Visible = showAlignLeft;
        alignLeftObjectButton.Enabled = showAlignLeft && canMutate;
        alignTopObjectButton.Visible = showAlignTop;
        alignTopObjectButton.Enabled = showAlignTop && canMutate;
        matchWidthObjectButton.Visible = showMatchWidth;
        matchWidthObjectButton.Enabled = showMatchWidth && canMutate;
        matchHeightObjectButton.Visible = showMatchHeight;
        matchHeightObjectButton.Enabled = showMatchHeight && canMutate;
        matchSizeObjectButton.Visible = showMatchSize;
        matchSizeObjectButton.Enabled = showMatchSize && canMutate;
        distributeHorizontalObjectButton.Visible = showDistributeHorizontal;
        distributeHorizontalObjectButton.Enabled = showDistributeHorizontal && canMutate;
        distributeVerticalObjectButton.Visible = showDistributeVertical;
        distributeVerticalObjectButton.Enabled = showDistributeVertical && canMutate;
        snapHorizontalObjectButton.Visible = showSnapHorizontal;
        snapHorizontalObjectButton.Enabled = showSnapHorizontal && canMutate;
        snapVerticalObjectButton.Visible = showSnapVertical;
        snapVerticalObjectButton.Enabled = showSnapVertical && canMutate;
        snapToGridObjectButton.Visible = showSnapToGrid;
        snapToGridObjectButton.Enabled = showSnapToGrid && canMutate;
        deleteObjectButton.Visible = showDelete;
        deleteObjectButton.Enabled = showDelete && canMutate;
        restoreObjectButton.Visible = showRestore;
        restoreObjectButton.Enabled = showRestore && canMutate;
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
                    localization,
                    currentSnapshot.ReadOnly);
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

        if (selectedItem is null && currentSnapshot?.AssetFamily == "project")
        {
            selectedItem = sectionListView.Items
                .Cast<ListViewItem>()
                .FirstOrDefault(item => item.Tag is CopperfinStudioProjectGroup group &&
                                        !string.Equals(group.Id, "project", StringComparison.Ordinal) &&
                                        group.RecordIndexes.Any(recordIndex =>
                                        {
                                            var entry = LookupProjectEntry(recordIndex);
                                            var snapshotObject = currentSnapshot?.Objects
                                                .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                                            return entry is not null &&
                                                   snapshotObject is not null &&
                                                   !snapshotObject.Deleted &&
                                                   !entry.Excluded &&
                                                   CopperfinProjectEntryActivation.TryResolve(
                                                       currentPath ?? string.Empty,
                                                       entry,
                                                       out _);
                                        }));
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

        var sortSetting = FindReportSetting(snapshot.ReportLayout, "TAG");
        if (!string.IsNullOrWhiteSpace(sortSetting?.Value))
        {
            details += Environment.NewLine +
                       L("AssetEditor.Property.ActiveSortExpression") +
                       ": " +
                       sortSetting!.Value;
        }

        var pageSetupParts = new List<string>();
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "ORIENTATION", "AssetEditor.Property.Orientation");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "PAPERSIZE", "AssetEditor.Property.PaperSize");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "PAPERLENGTH", "AssetEditor.Property.PaperLength");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "PAPERWIDTH", "AssetEditor.Property.PaperWidth");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "TOPMARGIN", "AssetEditor.Property.TopMargin");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "BOTMARGIN", "AssetEditor.Property.BottomMargin");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "LEFTMARGIN", "AssetEditor.Property.LeftMargin");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "RIGHTMARGIN", "AssetEditor.Property.RightMargin");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "GRIDV", "AssetEditor.Property.VerticalGrid");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "GRIDH", "AssetEditor.Property.HorizontalGrid");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "COLOR", "AssetEditor.Property.Color");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "COPIES", "AssetEditor.Property.Copies");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "DRIVER", "AssetEditor.Property.PrinterDriver");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "DEVICE", "AssetEditor.Property.PrinterDevice");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "OUTPUT", "AssetEditor.Property.PrinterOutput");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "DEFAULTSOURCE", "AssetEditor.Property.DefaultSource");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "PRINTQUALITY", "AssetEditor.Property.PrintQuality");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "YRESOLUTION", "AssetEditor.Property.YResolution");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "TTOPTION", "AssetEditor.Property.TrueTypeOption");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "ASCII", "AssetEditor.Property.Ascii");
        AppendReportSettingSummaryPart(pageSetupParts, snapshot.ReportLayout, "COLLATE", "AssetEditor.Property.Collate");
        if (pageSetupParts.Count > 0)
        {
            details += Environment.NewLine + string.Join("   ", pageSetupParts);
        }

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

    private CopperfinStudioNamedValue? FindReportSetting(CopperfinStudioReportLayout reportLayout, string name)
    {
        var liveSetting = reportLayout.Settings.FirstOrDefault(setting =>
            string.Equals(setting.Name, name, StringComparison.OrdinalIgnoreCase));
        if (liveSetting is not null)
        {
            return liveSetting;
        }

        return reportLayout.DeletedSettings.FirstOrDefault(setting =>
            string.Equals(setting.Name, name, StringComparison.OrdinalIgnoreCase));
    }

    private void AppendReportSettingSummaryPart(
        List<string> parts,
        CopperfinStudioReportLayout reportLayout,
        string settingName,
        string localizationKey)
    {
        var setting = FindReportSetting(reportLayout, settingName);
        if (setting is null || string.IsNullOrWhiteSpace(setting.Value))
        {
            return;
        }

        parts.Add(L(localizationKey) + ": " + setting.Value);
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

    private string BuildCodeSymbolKindDisplayText(string symbolKind)
    {
        var key = symbolKind switch
        {
            "class" => "AssetEditor.Summary.SymbolKind.Class",
            "procedure" => "AssetEditor.Summary.SymbolKind.Procedure",
            "function" => "AssetEditor.Summary.SymbolKind.Function",
            "define" => "AssetEditor.Summary.SymbolKind.Define",
            "method" => "AssetEditor.Summary.SymbolKind.Method",
            "do form" => "AssetEditor.Summary.SymbolKind.DoForm",
            "report form" => "AssetEditor.Summary.SymbolKind.ReportForm",
            "label form" => "AssetEditor.Summary.SymbolKind.LabelForm",
            "do" => "AssetEditor.Summary.SymbolKind.Do",
            "call" => "AssetEditor.Summary.SymbolKind.Call",
            "call.member" => "AssetEditor.Summary.SymbolKind.MemberCall",
            "definition" => "AssetEditor.Summary.SymbolKind.Definition",
            "reference" => "AssetEditor.Summary.SymbolKind.Reference",
            _ => string.Empty
        };

        if (!string.IsNullOrWhiteSpace(key))
        {
            return L(key);
        }

        return symbolKind;
    }

    private string BuildProjectInsightArtifactKindDisplayText(string kind)
    {
        var key = kind switch
        {
            "Project" => "AssetEditor.Summary.ArtifactKind.Project",
            "Project Header" => "AssetEditor.Summary.ArtifactKind.ProjectHeader",
            "Project Item" => "AssetEditor.Summary.ArtifactKind.ProjectItem",
            "Project Record" => "AssetEditor.Summary.ArtifactKind.ProjectRecord",
            "Form" => "AssetEditor.Summary.ArtifactKind.Form",
            "Class Library" => "AssetEditor.Summary.ArtifactKind.ClassLibrary",
            "Class" => "AssetEditor.Summary.ArtifactKind.Class",
            "Report" => "AssetEditor.Summary.ArtifactKind.Report",
            "Label" => "AssetEditor.Summary.ArtifactKind.Label",
            "Menu" => "AssetEditor.Summary.ArtifactKind.Menu",
            "Program" => "AssetEditor.Summary.ArtifactKind.Program",
            "Database" => "AssetEditor.Summary.ArtifactKind.Database",
            "Table" => "AssetEditor.Summary.ArtifactKind.Table",
            "Query" => "AssetEditor.Summary.ArtifactKind.Query",
            _ => string.Empty
        };

        if (!string.IsNullOrWhiteSpace(key))
        {
            return L(key);
        }

        return kind;
    }

    private string BuildProjectWorkspaceGroupTitleDisplayText(string title, string? groupId = null)
    {
        var normalizedId = (groupId ?? string.Empty).Trim().Replace('-', '_').Replace(' ', '_').ToLowerInvariant();
        var normalizedTitle = title.Trim();
        var key = normalizedId switch
        {
            "forms" => "AssetEditor.Summary.GroupTitle.Forms",
            "classes" => "AssetEditor.Summary.GroupTitle.ClassLibraries",
            "project_items" => "AssetEditor.Summary.GroupTitle.ProjectItems",
            "reports" => "AssetEditor.Summary.GroupTitle.Reports",
            "labels" => "AssetEditor.Summary.GroupTitle.Labels",
            "menus" => "AssetEditor.Summary.GroupTitle.Menus",
            "programs" => "AssetEditor.Summary.GroupTitle.Programs",
            "databases" => "AssetEditor.Summary.GroupTitle.Databases",
            "tables" => "AssetEditor.Summary.GroupTitle.Tables",
            "queries" => "AssetEditor.Summary.GroupTitle.Queries",
            "other_records" => "AssetEditor.Summary.GroupTitle.OtherRecords",
            "other" or "other_assets" => "AssetEditor.Summary.GroupTitle.OtherAssets",
            _ => normalizedTitle switch
            {
                "Forms" => "AssetEditor.Summary.GroupTitle.Forms",
                "Classes" => "AssetEditor.Summary.GroupTitle.Classes",
                "Class Libraries" => "AssetEditor.Summary.GroupTitle.ClassLibraries",
                "Project Items" => "AssetEditor.Summary.GroupTitle.ProjectItems",
                "Reports" => "AssetEditor.Summary.GroupTitle.Reports",
                "Labels" => "AssetEditor.Summary.GroupTitle.Labels",
                "Menus" => "AssetEditor.Summary.GroupTitle.Menus",
                "Programs" => "AssetEditor.Summary.GroupTitle.Programs",
                "Databases" => "AssetEditor.Summary.GroupTitle.Databases",
                "Tables" => "AssetEditor.Summary.GroupTitle.Tables",
                "Queries" => "AssetEditor.Summary.GroupTitle.Queries",
                "Other Records" => "AssetEditor.Summary.GroupTitle.OtherRecords",
                "Other Assets" => "AssetEditor.Summary.GroupTitle.OtherAssets",
                _ => string.Empty
            }
        };

        if (!string.IsNullOrWhiteSpace(key))
        {
            return L(key);
        }

        return title;
    }

    private string BuildObjectBrowserNodeDetailDisplayText(CopperfinProjectObjectNode node)
    {
        if (!string.IsNullOrWhiteSpace(node.GroupTitle))
        {
            return BuildProjectWorkspaceGroupTitleDisplayText(node.GroupTitle) +
                   (node.Excluded ? L("AssetEditor.Summary.ExcludedSuffix") : string.Empty);
        }

        return node.Detail;
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
        summary.AppendLine($"{L("AssetEditor.Summary.LabelDebug")}: {FormatLocalizedBoolean(workspace.BuildPlan.DebugEnabled)}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelEncrypt")}: {FormatLocalizedBoolean(workspace.BuildPlan.EncryptEnabled)}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelSaveCode")}: {FormatLocalizedBoolean(workspace.BuildPlan.SaveCode)}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelNoLogo")}: {FormatLocalizedBoolean(workspace.BuildPlan.NoLogo)}");
        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.GroupsHeading"));
        foreach (var group in workspace.Groups)
        {
            summary.AppendLine(
                F(
                    "AssetEditor.Summary.GroupLine",
                    BuildProjectWorkspaceGroupTitleDisplayText(group.Title, group.Id),
                    group.ItemCount,
                    group.ExcludedCount));
        }

        summary.AppendLine();
        summary.AppendLine($"{L("AssetEditor.Summary.BuildWorkflowHeading")}:");
        summary.AppendLine(L("AssetEditor.Summary.BuildWorkflowText"));
        return summary.ToString();
    }

    private string FormatLocalizedBoolean(bool value)
    {
        return L(value ? "AssetEditor.Summary.Boolean.True" : "AssetEditor.Summary.Boolean.False");
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

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.License"));
        summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelLicenseState"), snapshot.LicenseProfile.State));
        if (!string.IsNullOrEmpty(snapshot.LicenseProfile.Licensee))
        {
            summary.AppendLine(F("AssetEditor.Summary.IndentedLabelValue", L("AssetEditor.Summary.LabelLicensee"), snapshot.LicenseProfile.Licensee));
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
                summary.AppendLine(F("AssetEditor.Summary.StackFrameLine", frame.RoutineName, frame.Location));
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
                summary.AppendLine(F("AssetEditor.Summary.GlobalLine", global.Name, global.Value));
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
            summary.AppendLine(L("AssetEditor.Summary.TaskListActivation"));
        }

        if (insights.Warnings.Count > 0)
        {
            summary.AppendLine();
            summary.AppendLine(L("AssetEditor.Summary.ScanWarnings"));
            foreach (var warning in insights.Warnings.Take(10))
            {
                summary.AppendLine(F("AssetEditor.Summary.BulletLine", warning));
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
            foreach (var symbol in insights.DefinedSymbols.Take(12))
            {
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.ShortcutLine",
                        BuildCodeSymbolKindDisplayText(symbol.Kind),
                        symbol.Name));
            }

            summary.AppendLine(L("AssetEditor.Summary.CodeReferencesActivation"));
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.RuntimeReferences"));
        if (insights.RuntimeReferences.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoRuntimeReferences"));
        }
        else
        {
            foreach (var reference in insights.RuntimeReferences.Take(12))
            {
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.ShortcutLine",
                        BuildCodeSymbolKindDisplayText(reference.Kind),
                        reference.Name));
            }

            summary.AppendLine(L("AssetEditor.Summary.CodeReferencesActivation"));
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
        var filteredAssets = FilterDataAssets(insights, filter);

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
            foreach (var asset in filteredAssets.Take(12))
            {
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.DataAssetLine",
                        BuildProjectInsightArtifactKindDisplayText(asset.Kind),
                        asset.Title,
                        asset.Excluded ? L("AssetEditor.Summary.ExcludedSuffix") : string.Empty));
            }

            summary.AppendLine(L("AssetEditor.Summary.DataExplorerActivation"));
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
        var filteredNodes = FilterObjectNodes(insights, filter, hideProjectRecords);

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
            foreach (var node in filteredNodes.Take(12))
            {
                var title = string.IsNullOrWhiteSpace(node.GroupTitle)
                    ? node.Title
                    : $"{node.Title} {BuildObjectBrowserNodeDetailDisplayText(node)}";
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.ObjectNodeLine",
                        BuildProjectInsightArtifactKindDisplayText(node.Kind),
                        title));
            }

            summary.AppendLine(L("AssetEditor.Summary.ObjectBrowserActivation"));
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
            summary.AppendLine(F("AssetEditor.Summary.ToolboxGroupLine", BuildProjectWorkspaceGroupTitleDisplayText(group.Title, group.Id), group.ItemCount));
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.AddInSurfaces"));
        foreach (var feature in snapshot.ExtensibilityProfile.AiFeatures.Take(6))
        {
            summary.AppendLine(F("AssetEditor.Summary.AddInSurfaceLine", feature.Title, feature.Description));
        }

        if (insights is not null && insights.RuntimeReferences.Count > 0)
        {
            summary.AppendLine();
            summary.AppendLine(L("AssetEditor.Summary.HighValueShortcuts"));
            foreach (var reference in insights.RuntimeReferences.Take(6))
            {
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.ShortcutLine",
                        BuildCodeSymbolKindDisplayText(reference.Kind),
                        reference.Name));
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
                summary.AppendLine(
                    F(
                        "AssetEditor.Summary.ObjectNodeLine",
                        BuildProjectInsightArtifactKindDisplayText(node.Kind),
                        node.Title));
            }
        }

        return summary.ToString();
    }

    private string BuildBuilderKindDisplayText(string kind)
    {
        return string.Equals(kind, "wizard", StringComparison.OrdinalIgnoreCase)
            ? L("AssetEditor.Builders.Kind.Wizard")
            : L("AssetEditor.Builders.Kind.Builder");
    }

    private string BuildBuilderContextDisplayText(string context)
    {
        return context.ToLowerInvariant() switch
        {
            "form" => L("AssetEditor.Builders.Context.Form"),
            "class_designer" => L("AssetEditor.Builders.Context.Class"),
            "control" => L("AssetEditor.Builders.Context.Control"),
            "report" => L("AssetEditor.Builders.Context.Report"),
            "label" => L("AssetEditor.Builders.Context.Label"),
            "menu" => L("AssetEditor.Builders.Context.Menu"),
            "project" => L("AssetEditor.Builders.Context.Project"),
            "data_environment" => L("AssetEditor.Builders.Context.DataEnvironment"),
            _ => context
        };
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
            .Distinct(CopperfinDocumentPathIdentity.CreateComparer())
            .ToList();

        summary.AppendLine($"{L("AssetEditor.Summary.LabelPauseReason")}: {state.Reason}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelExecutedStatements")}: {state.ExecutedStatements}");
        summary.AppendLine($"{L("AssetEditor.Summary.LabelDistinctRuntimeLocations")}: {executedLocations.Count}");
        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.CoverageActivation"));
        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.RecentCoverageSignals"));
        foreach (var location in executedLocations.Take(12))
        {
            summary.AppendLine(F("AssetEditor.Summary.BulletLine", location));
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
            summary.AppendLine(F("AssetEditor.Summary.ConnectorLine", connector.Family, connector.Title));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLine", connector.TranslationStory));
        }
        if (filteredConnectors.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoConnectorTargets"));
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.QueryTranslationPaths"));
        foreach (var path in filteredPaths.Take(8))
        {
            summary.AppendLine(F("AssetEditor.Summary.QueryPathLine", path.Title, path.Complexity));
            summary.AppendLine(F("AssetEditor.Summary.IndentedLine", path.Strategy));
        }
        if (filteredPaths.Count == 0)
        {
            summary.AppendLine(L("AssetEditor.Summary.NoQueryPaths"));
        }

        summary.AppendLine();
        summary.AppendLine(L("AssetEditor.Summary.Guardrails"));
        foreach (var guardrail in snapshot.DatabaseProfile.Guardrails.Take(6))
        {
            summary.AppendLine(F("AssetEditor.Summary.BulletLine", guardrail));
        }

        return summary.ToString();
    }
}
