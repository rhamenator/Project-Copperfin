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
    private readonly ListView objectListView;
    private readonly ListView propertyListView;

    private string? currentPath;
    private CopperfinStudioSnapshotDocument? currentSnapshot;

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
        objectListView.SelectedIndexChanged += (_, _) => PopulatePropertyList();

        propertyListView = new ListView
        {
            Dock = DockStyle.Fill,
            FullRowSelect = true,
            HideSelection = false,
            MultiSelect = false,
            View = View.Details
        };
        propertyListView.Columns.Add("Property", 180);
        propertyListView.Columns.Add("Type", 60);
        propertyListView.Columns.Add("Value", 520);

        var splitContainer = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Vertical,
            SplitterDistance = 360
        };
        splitContainer.Panel1.Controls.Add(objectListView);
        splitContainer.Panel2.Controls.Add(propertyListView);

        var buttonPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false,
            Margin = new Padding(0, 8, 0, 0)
        };
        buttonPanel.Controls.Add(launchButton);
        buttonPanel.Controls.Add(revealButton);

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
        currentSnapshot = null;
        objectListView.Items.Clear();
        propertyListView.Items.Clear();
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
            PopulatePropertyList();
        }
    }

    private void PopulatePropertyList()
    {
        propertyListView.BeginUpdate();
        propertyListView.Items.Clear();

        var selectedObject = objectListView.SelectedItems
            .Cast<ListViewItem>()
            .Select(item => item.Tag as CopperfinStudioSnapshotObject)
            .FirstOrDefault(item => item is not null);

        if (selectedObject is not null)
        {
            foreach (var property in selectedObject.Properties)
            {
                var listItem = new ListViewItem(property.Name);
                listItem.SubItems.Add(property.Type);
                listItem.SubItems.Add(property.Value);
                propertyListView.Items.Add(listItem);
            }
        }

        propertyListView.EndUpdate();
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
