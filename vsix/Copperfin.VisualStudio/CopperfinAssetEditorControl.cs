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
    private readonly SplitContainer leftExplorerSplit;
    private readonly ListView sectionListView;
    private readonly ListView objectListView;
    private readonly PropertyGrid propertyGrid;
    private readonly CopperfinDesignSurfaceControl designSurface;
    private readonly RichTextBox workspaceSummaryBox;

    private string? currentPath;
    private CopperfinStudioSnapshotDocument? currentSnapshot;
    private bool suppressSelectionSync;
    private bool embeddedStudioShell;

    public bool EmbeddedStudioShell
    {
        get => embeddedStudioShell;
        set
        {
            embeddedStudioShell = value;
            ApplyHostMode();
        }
    }

    public CopperfinAssetEditorControl()
    {
        BackColor = Color.FromArgb(248, 249, 252);
        ForeColor = Color.FromArgb(28, 32, 39);
        Padding = new Padding(24);

        titleLabel = new Label
        {
            AutoSize = true,
            Font = new Font("Segoe UI Semibold", 16.0F, FontStyle.Bold, GraphicsUnit.Point),
            Text = "Copperfin Visual Designer"
        };

        subtitleLabel = new Label
        {
            AutoSize = true,
            MaximumSize = new Size(960, 0),
            Font = new Font("Segoe UI", 10.0F, FontStyle.Regular, GraphicsUnit.Point),
            Text = "This Visual Studio editor is the handoff point into Copperfin Studio. It is meant for VFP visual assets such as forms, reports, labels, menus, class libraries, and projects."
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
            Text = "This shell now pulls a structured snapshot from the native Copperfin Studio host. For VFP visual assets, that gives us a real object/property view while we work toward high-fidelity inline designers."
        };

        launchButton = new Button
        {
            AutoSize = true,
            Text = "Open In Native Studio"
        };
        launchButton.Click += (_, _) => LaunchStudio();

        revealButton = new Button
        {
            AutoSize = true,
            Text = "Reveal In Explorer"
        };
        revealButton.Click += (_, _) => RevealInExplorer();

        refreshButton = new Button
        {
            AutoSize = true,
            Text = "Refresh"
        };
        refreshButton.Click += (_, _) =>
        {
            if (!string.IsNullOrWhiteSpace(currentPath))
            {
                LoadDocument(currentPath!);
            }
        };

        snapshotStatusLabel = new Label
        {
            AutoSize = true,
            MaximumSize = new Size(960, 0),
            Font = new Font("Segoe UI", 9.5F, FontStyle.Regular, GraphicsUnit.Point),
            Text = "Loading Copperfin Studio snapshot..."
        };

        objectListView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        objectListView.Columns.Add("Object", 240);
        objectListView.Columns.Add("Type", 180);
        objectListView.Columns.Add("Record", 70);
        objectListView.SelectedIndexChanged += (_, _) => SyncSelectionFromList();

        sectionListView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        sectionListView.Columns.Add("Section", 200);
        sectionListView.Columns.Add("Objects", 70);
        sectionListView.Columns.Add("Top", 80);
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

        var surfaceHost = new Panel
        {
            Dock = DockStyle.Fill
        };
        surfaceHost.Controls.Add(workspaceSummaryBox);
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

    public void LoadDocument(string path)
    {
        currentPath = path;

        var info = new FileInfo(path);
        titleLabel.Text = CopperfinStudioHostBridge.DescribeAssetKind(path);
        pathLabel.Text = path;
        detailsLabel.Text =
            $"Size: {info.Length:N0} bytes   Last write: {info.LastWriteTime:G}   Extension: {info.Extension.ToLowerInvariant()}";
        launchButton.Enabled = true;
        revealButton.Enabled = true;
        refreshButton.Enabled = true;
        currentSnapshot = null;
        sectionListView.Items.Clear();
        objectListView.Items.Clear();
        propertyGrid.SelectedObject = null;
        designSurface.LoadObjects(string.Empty, Array.Empty<CopperfinStudioSnapshotObject>());
        workspaceSummaryBox.Text = string.Empty;
        workspaceSummaryBox.Visible = false;
        designSurface.Visible = true;
        snapshotStatusLabel.Text = "Loading Copperfin Studio snapshot...";
        _ = LoadSnapshotAsync(path);
    }

    private async Task LoadSnapshotAsync(string path)
    {
        var snapshotResult = await Task.Run(() => CopperfinStudioSnapshotClient.TryLoad(path));
        if (IsDisposed || !string.Equals(currentPath, path, StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        BeginInvoke((Action)(() =>
        {
            if (!snapshotResult.Success || snapshotResult.Document is null)
            {
                snapshotStatusLabel.Text = "Snapshot unavailable: " + snapshotResult.Error;
                return;
            }

            currentSnapshot = snapshotResult.Document;
            snapshotStatusLabel.Text =
                $"Snapshot loaded: {currentSnapshot.Objects.Count} object rows, {currentSnapshot.FieldCount} fields, {currentSnapshot.IndexCount} companion indexes.";
            guidanceLabel.Text = BuildGuidanceText(currentSnapshot.AssetFamily);
            PopulateSectionList();
            PopulateObjectList();
            LoadSurface();
        }));
    }

    private void PopulateSectionList()
    {
        sectionListView.BeginUpdate();
        sectionListView.Items.Clear();

        if (currentSnapshot?.ReportLayout?.Sections is { Count: > 0 })
        {
            sectionListView.Visible = true;
            leftExplorerSplit.Panel1Collapsed = false;
            sectionListView.Columns[0].Text = "Section";
            sectionListView.Columns[1].Text = "Objects";
            sectionListView.Columns[2].Text = "Top";
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
            sectionListView.Columns[0].Text = "Group";
            sectionListView.Columns[1].Text = "Items";
            sectionListView.Columns[2].Text = "Excluded";
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

        snapshotStatusLabel.Text = $"Applying {propertyName} change...";
        var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(currentPath!, recordIndex, propertyName, propertyValue);
        if (!updateResult.Success || updateResult.Document is null)
        {
            snapshotStatusLabel.Text = "Property update failed: " + updateResult.Error;
            return;
        }

        currentSnapshot = updateResult.Document;
        snapshotStatusLabel.Text =
            $"Updated {propertyName}. Snapshot loaded: {currentSnapshot.Objects.Count} object rows, {currentSnapshot.FieldCount} fields.";
        PopulateSectionList();
        PopulateObjectList();
        LoadSurface();
        designSurface.SelectRecord(recordIndex);
        SyncSelectionFromSurface(recordIndex);
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
            workspaceSummaryBox.Text = BuildProjectWorkspaceSummary(currentSnapshot.ProjectWorkspace);
            workspaceSummaryBox.Visible = true;
            designSurface.Visible = false;
            return;
        }

        var objects = currentSnapshot?.Objects is null
            ? (IReadOnlyList<CopperfinStudioSnapshotObject>)Array.Empty<CopperfinStudioSnapshotObject>()
            : currentSnapshot.Objects;
        workspaceSummaryBox.Visible = false;
        designSurface.Visible = true;
        designSurface.LoadObjects(currentSnapshot?.AssetFamily ?? string.Empty, objects);
    }

    private string BuildGuidanceText(string assetFamily)
    {
        return assetFamily switch
        {
            "form" => "Copperfin is surfacing form objects from SCX/SCT assets and can now round-trip a safe editable subset inside Visual Studio.",
            "class_library" => "Copperfin is surfacing class-library objects from VCX/VCT assets so you can inspect and edit reusable visual classes in-place.",
            "report" => "Copperfin is surfacing report bands and objects from FRX/FRT assets in a more modern report-designer shape with section outlines, live layout panes, and inline editing for safe layout fields.",
            "label" => "Copperfin is surfacing label objects from LBX/LBT assets in the shared report/label designer shell so label layouts feel closer to current Visual Studio tooling than to legacy modal editors.",
            "menu" => "Copperfin is surfacing menu structures from MNX/MNT assets. Prompt, command, procedure, and message fields can be edited from the property grid.",
            "project" => "Copperfin is surfacing PJX/PJT files as grouped workspaces with project entries, startup/build settings, and a first Copperfin build summary while the project manager grows toward full VFP parity.",
            _ => "This shell now pulls a structured snapshot from the native Copperfin Studio host so each VFP asset family can grow toward a high-fidelity Visual Studio editor."
        };
    }

    private void LaunchStudio()
    {
        if (embeddedStudioShell)
        {
            return;
        }

        if (string.IsNullOrWhiteSpace(currentPath) || !File.Exists(currentPath))
        {
            MessageBox.Show(this, "The asset path is no longer available on disk.", "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        var studioHostPath = CopperfinStudioHostBridge.ResolveStudioHostPath();
        if (studioHostPath is null)
        {
            MessageBox.Show(
                this,
                "Copperfin Studio host was not found. Set COPPERFIN_STUDIO_HOST_PATH or build E:\\Project-Copperfin\\build\\Release\\copperfin_studio_host.exe.",
                "Copperfin",
                MessageBoxButtons.OK,
                MessageBoxIcon.Warning);
            return;
        }

        if (!CopperfinStudioHostBridge.Launch(studioHostPath, currentPath!))
        {
            MessageBox.Show(this, "Copperfin Studio did not start successfully.", "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Warning);
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
        subtitleLabel.Text = embeddedStudioShell
            ? "This standalone Copperfin Studio shell hosts the same designer surface used inside Visual Studio, so report, label, form, menu, class, and project work can evolve on one shared editor stack."
            : "This Visual Studio editor is the handoff point into Copperfin Studio. It is meant for VFP visual assets such as forms, reports, labels, menus, class libraries, and projects.";
    }

    private void ConfigureObjectColumns()
    {
        objectListView.Columns[0].Text = currentSnapshot?.AssetFamily == "project" ? "Item" : "Object";
        objectListView.Columns[1].Text = currentSnapshot?.AssetFamily == "project" ? "Group" : "Type";
        objectListView.Columns[2].Text = "Record";
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
        summary.AppendLine("Copperfin can now inspect the project structure and derive a build plan, but executable generation still needs the runtime/compiler pipeline behind this workspace.");
        return summary.ToString();
    }
}
