using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
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
    private readonly ListView objectListView;
    private readonly PropertyGrid propertyGrid;
    private readonly CopperfinDesignSurfaceControl designSurface;

    private string? currentPath;
    private CopperfinStudioSnapshotDocument? currentSnapshot;
    private bool suppressSelectionSync;

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
            ApplyVisualPropertyChange(recordIndex, "Left", left.ToString());
            ApplyVisualPropertyChange(recordIndex, "Top", top.ToString());
        };

        var rightSplit = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Horizontal,
            SplitterDistance = 320
        };
        rightSplit.Panel1.Controls.Add(designSurface);
        rightSplit.Panel2.Controls.Add(propertyGrid);

        var splitContainer = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Vertical,
            SplitterDistance = 360
        };
        splitContainer.Panel1.Controls.Add(objectListView);
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
    }

    public void LoadDocument(string path)
    {
        currentPath = path;

        var info = new FileInfo(path);
        titleLabel.Text = CopperfinStudioLauncher.DescribeAssetKind(path);
        pathLabel.Text = path;
        detailsLabel.Text =
            $"Size: {info.Length:N0} bytes   Last write: {info.LastWriteTime:G}   Extension: {info.Extension.ToLowerInvariant()}";
        launchButton.Enabled = true;
        revealButton.Enabled = true;
        refreshButton.Enabled = true;
        currentSnapshot = null;
        objectListView.Items.Clear();
        propertyGrid.SelectedObject = null;
        designSurface.LoadObjects(Array.Empty<CopperfinStudioSnapshotObject>());
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
            PopulateObjectList();
            designSurface.LoadObjects(currentSnapshot.Objects);
        }));
    }

    private void PopulateObjectList()
    {
        objectListView.BeginUpdate();
        objectListView.Items.Clear();

        if (currentSnapshot is null)
        {
            objectListView.EndUpdate();
            return;
        }

        foreach (var item in currentSnapshot.Objects)
        {
            var listItem = new ListViewItem(string.IsNullOrWhiteSpace(item.Title) ? $"Record {item.RecordIndex}" : item.Title);
            listItem.SubItems.Add(item.Subtitle);
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

        propertyGrid.SelectedObject = selectedObject is null ? null : CopperfinDesignerSelection.FromSnapshot(selectedObject);
        designSurface.SelectRecord(selectedObject?.RecordIndex);
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

            propertyGrid.SelectedObject = selectedObject is null ? null : CopperfinDesignerSelection.FromSnapshot(selectedObject);
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

        switch (propertyName)
        {
            case nameof(CopperfinDesignerSelection.Left):
                ApplyVisualPropertyChange(selection.RecordIndex, "Left", selection.Left.ToString());
                break;
            case nameof(CopperfinDesignerSelection.Top):
                ApplyVisualPropertyChange(selection.RecordIndex, "Top", selection.Top.ToString());
                break;
            case nameof(CopperfinDesignerSelection.Width):
                ApplyVisualPropertyChange(selection.RecordIndex, "Width", selection.Width.ToString());
                break;
            case nameof(CopperfinDesignerSelection.Height):
                ApplyVisualPropertyChange(selection.RecordIndex, "Height", selection.Height.ToString());
                break;
            case nameof(CopperfinDesignerSelection.Caption):
                ApplyVisualPropertyChange(selection.RecordIndex, "Caption", "\"" + selection.Caption.Replace("\"", "\"\"") + "\"");
                break;
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
        PopulateObjectList();
        designSurface.LoadObjects(currentSnapshot.Objects);
        designSurface.SelectRecord(recordIndex);
        SyncSelectionFromSurface(recordIndex);
    }

    private void LaunchStudio()
    {
        if (string.IsNullOrWhiteSpace(currentPath) || !File.Exists(currentPath))
        {
            MessageBox.Show(this, "The asset path is no longer available on disk.", "Copperfin", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        var studioHostPath = CopperfinStudioLauncher.ResolveStudioHostPath();
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

        if (!CopperfinStudioLauncher.Launch(studioHostPath, currentPath!))
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
}
