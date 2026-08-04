// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal sealed partial class CopperfinAssetEditorControl
{
    private TreeView projectExplorerView = null!;
    private TextBox projectExplorerFilterBox = null!;

    private TabPage CreateProjectExplorerPage()
    {
        projectExplorerView = new TreeView
        {
            Dock = DockStyle.Fill,
            HideSelection = false,
            FullRowSelect = true,
            ShowLines = true,
            ShowNodeToolTips = true,
            BackColor = Color.White,
            ForeColor = Color.FromArgb(28, 32, 39)
        };
        projectExplorerView.NodeMouseDoubleClick += (_, _) => TryActivateSelectedProjectExplorerEntry();
        projectExplorerView.KeyDown += (_, e) =>
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            e.Handled = true;
            e.SuppressKeyPress = true;
            TryActivateSelectedProjectExplorerEntry();
        };

        projectExplorerFilterBox = new TextBox
        {
            Dock = DockStyle.Top,
            AccessibleName = this.localization.Text("AssetEditor.ProjectExplorer.FilterAccessibleName")
        };
        projectExplorerFilterBox.TextChanged += (_, _) => RefreshProjectExplorer();

        var page = new TabPage(this.localization.Text("AssetEditor.Tab.ProjectExplorer"));
        page.Controls.Add(projectExplorerView);
        page.Controls.Add(projectExplorerFilterBox);
        return page;
    }

    private void RefreshProjectExplorer()
    {
        if (projectExplorerView.IsDisposed)
        {
            return;
        }

        projectExplorerView.BeginUpdate();
        try
        {
            projectExplorerView.Nodes.Clear();
            var workspace = currentSnapshot?.ProjectWorkspace;
            if (workspace is null)
            {
                return;
            }

            var normalizedFilter = projectExplorerFilterBox.Text.Trim();
            bool Matches(string? value) => string.IsNullOrWhiteSpace(normalizedFilter) ||
                                           (value is not null &&
                                            value.IndexOf(normalizedFilter, StringComparison.OrdinalIgnoreCase) >= 0);
            bool MatchesEntry(CopperfinStudioProjectEntry entry) =>
                Matches(entry.RelativePath) ||
                Matches(entry.Name) ||
                Matches(entry.TypeTitle) ||
                Matches(entry.GroupTitle);

            var rootTitle = string.IsNullOrWhiteSpace(workspace.ProjectTitle)
                ? workspace.ProjectKey
                : workspace.ProjectTitle;
            var root = new TreeNode(this.localization.Format(
                "AssetEditor.ProjectExplorer.Root",
                rootTitle));
            root.Tag = workspace;

            var entriesByRecord = workspace.Entries
                .GroupBy(entry => entry.RecordIndex)
                .ToDictionary(group => group.Key, group => group.First());
            var groupedRecordIndexes = new HashSet<int>();

            foreach (var group in workspace.Groups)
            {
                var groupNode = new TreeNode(BuildProjectWorkspaceGroupTitleDisplayText(group.Title, group.Id))
                {
                    Tag = group,
                    ToolTipText = group.Title
                };
                foreach (var recordIndex in group.RecordIndexes)
                {
                    if (!entriesByRecord.TryGetValue(recordIndex, out var entry))
                    {
                        continue;
                    }

                    if (!Matches(group.Title) && !Matches(group.Id) && !MatchesEntry(entry))
                    {
                        continue;
                    }

                    groupedRecordIndexes.Add(recordIndex);
                    groupNode.Nodes.Add(CreateProjectExplorerEntryNode(entry));
                }

                if (groupNode.Nodes.Count > 0)
                {
                    root.Nodes.Add(groupNode);
                }
            }

            foreach (var entry in workspace.Entries.Where(entry =>
                         !groupedRecordIndexes.Contains(entry.RecordIndex) && MatchesEntry(entry)))
            {
                root.Nodes.Add(CreateProjectExplorerEntryNode(entry));
            }

            if (root.Nodes.Count == 0)
            {
                root.Nodes.Add(new TreeNode(
                    string.IsNullOrWhiteSpace(normalizedFilter)
                        ? this.localization.Text("AssetEditor.ProjectExplorer.Empty")
                        : this.localization.Text("AssetEditor.ProjectExplorer.NoMatches")));
            }

            root.Expand();
            projectExplorerView.Nodes.Add(root);
            projectExplorerView.SelectedNode = root;
        }
        finally
        {
            projectExplorerView.EndUpdate();
        }
    }

    private TreeNode CreateProjectExplorerEntryNode(CopperfinStudioProjectEntry entry)
    {
        var title = string.IsNullOrWhiteSpace(entry.RelativePath)
            ? entry.Name
            : entry.RelativePath;
        if (string.IsNullOrWhiteSpace(title))
        {
            title = entry.TypeTitle + " #" + entry.RecordIndex;
        }

        var node = new TreeNode(title)
        {
            Tag = entry,
            ToolTipText = entry.RelativePath
        };
        if (entry.Excluded)
        {
            node.ForeColor = Color.Firebrick;
            node.Text += this.localization.Text("AssetEditor.Summary.ExcludedSuffix");
        }

        return node;
    }

    public bool TryActivateSelectedProjectExplorerEntry()
    {
        if (currentSnapshot?.AssetFamily != "project" ||
            string.IsNullOrWhiteSpace(currentPath) ||
            projectExplorerView.SelectedNode?.Tag is not CopperfinStudioProjectEntry entry ||
            entry.Excluded ||
            !CopperfinProjectEntryActivation.TryResolve(currentPath!, entry, out var resolvedPath) ||
            !System.IO.File.Exists(resolvedPath))
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
}
