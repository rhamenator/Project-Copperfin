
// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
    private static string DescribeDebugSession(CopperfinRuntimeDebugSession? session)
    {
        if (session is null)
        {
            return "session=<null>";
        }

        var error = (session.Error ?? string.Empty).Replace("\r", " ").Replace("\n", " ");
        if (error.Length > 240)
        {
            error = error.Substring(0, 240) + "...";
        }

        return $"success={session.Success}; error={error}; commands={string.Join(",", session.Commands)}; " +
               $"manifest={session.ManifestPath}; debugManifest={session.DebugManifestPath}; " +
               $"reason={session.State.Reason}; frames={session.State.Frames.Count}; events={session.State.Events.Count}; " +
               $"transportPid={session.TransportProcessId}; transportLive={session.Transport is not null}; " +
               $"transportStopCompleted={session.TransportStopCompleted}";
    }

    private static void SmokeProjectEditorWithRealAsset(
        string? path,
        string[] expectGroups,
        string expectedManifestAsset = "asset=6|wzcommon/registry.vcx|")
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real project asset candidate" : path)} not found.");
            return;
        }

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };
        control.SuppressProjectWorkflowDialogs = true;
        string? activatedPath = null;
        control.OpenDocumentRequested += path => activatedPath = path;

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();
        control.LoadDocument(path!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => FindListViews(control).Any(list => list.Items.Count > 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Project Workspace", StringComparison.OrdinalIgnoreCase) >= 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Task List", StringComparison.OrdinalIgnoreCase) >= 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Code References", StringComparison.OrdinalIgnoreCase) >= 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Data Explorer", StringComparison.OrdinalIgnoreCase) >= 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Object Browser", StringComparison.OrdinalIgnoreCase) >= 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Toolbox And Add-ins", StringComparison.OrdinalIgnoreCase) >= 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Builders", StringComparison.OrdinalIgnoreCase) >= 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Coverage", StringComparison.OrdinalIgnoreCase) >= 0) &&
                  FindRichTextBoxes(control).Any(box => box.Text.IndexOf("Copperfin Database Federation", StringComparison.OrdinalIgnoreCase) >= 0));
        Expect(loaded, $"project editor should load grouped workspace data for {path}");

        var groupFound = FindListViews(control)
            .SelectMany(list => list.Items.Cast<ListViewItem>())
            .Any(item => expectGroups.Any(expectGroup => string.Equals(item.Text, expectGroup, StringComparison.OrdinalIgnoreCase)));
        Expect(groupFound, $"project editor should surface one of the expected groups for {path}");

        var projectWorkspaceTabs = GetPrivateField<TabControl>(control, "projectWorkspaceTabs");
        var projectExplorer = GetPrivateField<TreeView>(control, "projectExplorerView");
        Expect(projectWorkspaceTabs is not null &&
               projectWorkspaceTabs.TabPages.Cast<TabPage>().Any(page => page.Text == "Project Explorer"),
            $"project editor should expose a localized Project Explorer tab for {path}");
        Expect(projectExplorer is not null && projectExplorer.Nodes.Count == 1,
            $"project editor should expose one Project Explorer root for {path}");
        var explorerEntryNode = projectExplorer is null
            ? null
            : EnumerateTreeNodes(projectExplorer.Nodes)
                .FirstOrDefault(node => node.Tag is CopperfinStudioProjectEntry entry &&
                                        !entry.Excluded &&
                                        CopperfinProjectEntryActivation.TryResolve(path!, entry, out var resolvedPath) &&
                                        File.Exists(resolvedPath));
        Expect(explorerEntryNode is not null,
            $"project explorer should expose an eligible real project entry for {path}");
        if (projectExplorer is not null && explorerEntryNode?.Tag is CopperfinStudioProjectEntry explorerEntry)
        {
            projectExplorer.SelectedNode = explorerEntryNode;
            Application.DoEvents();
            CopperfinProjectEntryActivation.TryResolve(path!, explorerEntry, out var expectedExplorerPath);
            activatedPath = null;
            var activatedFromExplorer = control.TryActivateSelectedProjectExplorerEntry();
            Expect(activatedFromExplorer &&
                   string.Equals(activatedPath, expectedExplorerPath, StringComparison.Ordinal),
                $"project explorer should activate the selected eligible child through the host callback for {path}");
        }

        var projectEntryList = FindListViews(control)
            .FirstOrDefault(list => list.Columns.Count >= 3 &&
                                    string.Equals(
                                        list.Columns[0].Text,
                                        "Item",
                                        StringComparison.OrdinalIgnoreCase) &&
                                    string.Equals(
                                        list.Columns[1].Text,
                                        "Group",
                                        StringComparison.OrdinalIgnoreCase) &&
                                    string.Equals(
                                        list.Columns[2].Text,
                                        "Record",
                                        StringComparison.OrdinalIgnoreCase));
        var projectEntryItem = projectEntryList?.Items
            .Cast<ListViewItem>()
            .FirstOrDefault(item => CopperfinProjectEntryActivation.TryResolve(
                path!,
                new CopperfinStudioProjectEntry { RelativePath = item.Text },
                out _));
        Expect(projectEntryItem is not null,
            $"project editor should list at least one activatable child asset for {path}");
        if (projectEntryList is not null && projectEntryItem is not null)
        {
            CopperfinProjectEntryActivation.TryResolve(
                path!,
                new CopperfinStudioProjectEntry { RelativePath = projectEntryItem.Text },
                out var expectedActivatedPath);
            projectEntryList.SelectedItems.Clear();
            projectEntryItem.Selected = true;
            projectEntryItem.Focused = true;
            Application.DoEvents();
            var activated = control.TryActivateSelectedProjectEntry();
            Expect(activated &&
                   !string.IsNullOrWhiteSpace(activatedPath) &&
                   string.Equals(
                       activatedPath,
                       expectedActivatedPath,
                       StringComparison.Ordinal),
                $"project editor should activate the selected child asset through the host callback for {path}");
        }

        var projectButtons = FindButtons(control).Select(button => button.Text).ToList();
        Expect(projectButtons.Contains("Build Copperfin Project"), $"project editor should expose a build command for {path}");
        Expect(projectButtons.Contains("Run Copperfin Project"), $"project editor should expose a run command for {path}");
        Expect(projectButtons.Contains("Debug Copperfin Project"), $"project editor should expose a debug command for {path}");
        var buildButton = FindButtons(control).FirstOrDefault(button => button.Text == "Build Copperfin Project");
        Expect(buildButton is not null, $"project editor should surface a buildable project command for {path}");
        if (buildButton is not null)
        {
            buildButton.PerformClick();
            var buildLoaded = WaitUntil(
                TimeSpan.FromSeconds(30),
                () =>
                {
                    var workflowResult = GetPrivateField<CopperfinProjectExecutionResult>(control, "currentProjectWorkflowResult");
                    var statusLabel = GetPrivateField<Label>(control, "snapshotStatusLabel");
                    return workflowResult is not null &&
                           workflowResult.Success &&
                           statusLabel is not null &&
                           string.Equals(statusLabel.Text, workflowResult.Message, StringComparison.Ordinal);
                });
            Expect(buildLoaded, $"project editor build command should preserve a completed workflow result for {path}");
            var workflowResult = GetPrivateField<CopperfinProjectExecutionResult>(control, "currentProjectWorkflowResult");
            Expect(workflowResult is not null, $"project editor should retain the latest build workflow result for {path}");
            if (workflowResult is not null)
            {
                Expect(!string.IsNullOrWhiteSpace(workflowResult.OutputDirectory) && Directory.Exists(workflowResult.OutputDirectory),
                    $"project editor build command should preserve an output directory for {path}");
                Expect(!string.IsNullOrWhiteSpace(workflowResult.ManifestPath) && File.Exists(workflowResult.ManifestPath),
                    $"project editor build command should preserve a runtime manifest for {path}");
                Expect(!string.IsNullOrWhiteSpace(workflowResult.LauncherPath) && File.Exists(workflowResult.LauncherPath),
                    $"project editor build command should preserve a launcher path for {path}");
                Expect(!string.IsNullOrWhiteSpace(workflowResult.DebugManifestPath) && File.Exists(workflowResult.DebugManifestPath),
                    $"project editor build command should preserve a debug manifest for {path}");
                Expect(workflowResult.WarningCount == 0,
                    $"project editor build command should inherit a warning-free build for {path}");
                Expect(workflowResult.Warnings.Count == 0,
                    $"project editor build command should not inherit build warning lines for {path}");
                if (!string.IsNullOrWhiteSpace(workflowResult.ManifestPath) && File.Exists(workflowResult.ManifestPath))
                {
                    var manifestText = File.ReadAllText(workflowResult.ManifestPath);
                    Expect(manifestText.IndexOf("warning=", StringComparison.OrdinalIgnoreCase) < 0,
                        $"project editor build manifest should remain warning-free for {path}");
                    Expect(manifestText.IndexOf(expectedManifestAsset, StringComparison.Ordinal) >= 0,
                        $"project editor build manifest should stage the expected project dependency for {path}");
                }
            }
        }
        var runButton = FindButtons(control).FirstOrDefault(button => button.Text == "Run Copperfin Project");
        Expect(runButton is not null, $"project editor should surface a runnable project command for {path}");
        if (runButton is not null)
        {
            runButton.PerformClick();
            var runLoaded = WaitUntil(
                TimeSpan.FromSeconds(30),
                () =>
                {
                    var workflowResult = GetPrivateField<CopperfinProjectExecutionResult>(control, "currentProjectWorkflowResult");
                    var statusLabel = GetPrivateField<Label>(control, "snapshotStatusLabel");
                    return workflowResult is not null &&
                           workflowResult.Success &&
                           statusLabel is not null &&
                           string.Equals(statusLabel.Text, workflowResult.Message, StringComparison.Ordinal);
                });
            Expect(runLoaded, $"project editor run command should preserve a completed workflow result for {path}");
            var workflowResult = GetPrivateField<CopperfinProjectExecutionResult>(control, "currentProjectWorkflowResult");
            Expect(workflowResult is not null, $"project editor should retain the latest run workflow result for {path}");
            if (workflowResult is not null)
            {
                Expect(!string.IsNullOrWhiteSpace(workflowResult.OutputDirectory) && Directory.Exists(workflowResult.OutputDirectory),
                    $"project editor run command should preserve an output directory for {path}");
                Expect(!string.IsNullOrWhiteSpace(workflowResult.ManifestPath) && File.Exists(workflowResult.ManifestPath),
                    $"project editor run command should preserve a runtime manifest for {path}");
                Expect(!string.IsNullOrWhiteSpace(workflowResult.LauncherPath) && File.Exists(workflowResult.LauncherPath),
                    $"project editor run command should preserve a launcher path for {path}");
                Expect(!string.IsNullOrWhiteSpace(workflowResult.DebugManifestPath) && File.Exists(workflowResult.DebugManifestPath),
                    $"project editor run command should preserve a debug manifest for {path}");
                Expect(workflowResult.WarningCount == 0,
                    $"project editor run command should inherit a warning-free build for {path}");
                Expect(workflowResult.Warnings.Count == 0,
                    $"project editor run command should not inherit build warning lines for {path}");
                if (!string.IsNullOrWhiteSpace(workflowResult.ManifestPath) && File.Exists(workflowResult.ManifestPath))
                {
                    var manifestText = File.ReadAllText(workflowResult.ManifestPath);
                    Expect(manifestText.IndexOf("warning=", StringComparison.OrdinalIgnoreCase) < 0,
                        $"project editor run manifest should remain warning-free for {path}");
                    Expect(manifestText.IndexOf(expectedManifestAsset, StringComparison.Ordinal) >= 0,
                        $"project editor run manifest should stage the expected project dependency for {path}");
                }
            }
        }

        var summary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Project Workspace", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(summary is not null, $"project editor should surface a workspace summary for {path}");
        if (summary is not null)
        {
            Expect(summary.Text.IndexOf("Planned Output:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project workspace summary should include a build output for {path}");
            Expect(summary.Text.IndexOf("Startup Item:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project workspace summary should include a startup item for {path}");
            Expect(summary.Text.IndexOf("Native Security:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project workspace summary should include native security for {path}");
            Expect(summary.Text.IndexOf(".NET And Extensibility:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project workspace summary should include .NET/extensibility guidance for {path}");
        }

        var taskListSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Task List", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(taskListSummary is not null, $"project editor should surface a task-list pane for {path}");

        var codeReferenceSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Code References", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(codeReferenceSummary is not null, $"project editor should surface a code-references pane for {path}");

        var dataExplorerSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Data Explorer", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(dataExplorerSummary is not null, $"project editor should surface a data-explorer pane for {path}");

        var objectBrowserSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Object Browser", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(objectBrowserSummary is not null, $"project editor should surface an object-browser pane for {path}");

        var toolboxSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Toolbox And Add-ins", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(toolboxSummary is not null, $"project editor should surface a toolbox pane for {path}");

        var buildersSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Builders", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(buildersSummary is not null, $"project editor should surface a builders pane for {path}");

        var coverageSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Coverage", StringComparison.OrdinalIgnoreCase) >= 0 ||
                                   box.Text.IndexOf("runtime coverage signals", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(coverageSummary is not null, $"project editor should surface a coverage pane for {path}");

        var databaseSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Database Federation", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(databaseSummary is not null, $"project editor should surface a database pane for {path}");

        var databaseList = FindListViews(control)
            .FirstOrDefault(list => list.Columns.Count >= 4 &&
                                    string.Equals(list.Columns[0].Text, "Kind", StringComparison.OrdinalIgnoreCase) &&
                                    string.Equals(list.Columns[1].Text, "Title", StringComparison.OrdinalIgnoreCase) &&
                                    string.Equals(list.Columns[2].Text, "Shape", StringComparison.OrdinalIgnoreCase) &&
                                    string.Equals(list.Columns[3].Text, "Details", StringComparison.OrdinalIgnoreCase));
        Expect(databaseList is not null, $"project editor should surface a database federation catalog for {path}");
        var databaseSnapshot = GetPrivateField<CopperfinStudioSnapshotDocument>(control, "currentSnapshot");
        if (databaseList is not null && databaseSnapshot is not null && databaseSnapshot.DatabaseProfile.Available)
        {
            Expect(databaseList.Items.Count == databaseSnapshot.DatabaseProfile.Connectors.Count +
                   databaseSnapshot.DatabaseProfile.QueryPaths.Count,
                $"database federation catalog should render every available entry for {path}");
        }

        TearDownForm(hostForm);
    }

    private static IEnumerable<TreeNode> EnumerateTreeNodes(TreeNodeCollection nodes)
    {
        foreach (TreeNode node in nodes)
        {
            yield return node;
            foreach (var child in EnumerateTreeNodes(node.Nodes))
            {
                yield return child;
            }
        }
    }

    private static void SmokeProgramEditorWithRealAsset(string? path)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real program asset candidate" : path)} not found.");
            return;
        }

        var loadedSnapshot = CopperfinStudioSnapshotClient.TryLoad(path!);
        Expect(loadedSnapshot.Success && loadedSnapshot.Document is not null,
            $"program editor smoke should load snapshot data for {path}");
        if (!loadedSnapshot.Success || loadedSnapshot.Document is null)
        {
            return;
        }

        Expect(string.Equals(loadedSnapshot.Document.Kind, "program", StringComparison.Ordinal) &&
               string.Equals(loadedSnapshot.Document.AssetFamily, "program", StringComparison.Ordinal),
            $"program editor smoke should preserve the native program document identity for {path}");

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();
        control.LoadDocument(path!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => HasLabelText(control, "Visual program") &&
                  HasLabelTextContaining(control, "PRG program sources") &&
                  HasLabelTextContaining(control, "Snapshot loaded: 0 object rows, 0 fields, 0 companion indexes."));
        Expect(loaded, $"program editor smoke should load program-specific editor guidance for {path}");

        Expect(HasLabelText(control, "Visual program"),
            $"program editor smoke should title PRG assets as visual programs for {path}");
        Expect(HasLabelTextContaining(control, "PRG program sources"),
            $"program editor smoke should surface program-specific guidance for {path}");
        Expect(!FindButtons(control).Any(button => button.Visible && button.Text == "Build Copperfin Project"),
            $"program editor smoke should keep PRG assets on the direct program path rather than the project workflow shell for {path}");

        TearDownForm(hostForm);
    }

    private static void SmokeVisualAssetEditorWithRealAsset(
        string? path,
        string expectedAssetFamily,
        string expectedTitle,
        string expectedGuidanceSnippet,
        string expectedSidecarExtension)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real visual asset candidate" : path)} not found.");
            return;
        }

        var expectedSidecarPath = Path.ChangeExtension(path, expectedSidecarExtension);
        Expect(File.Exists(expectedSidecarPath),
            $"visual asset editor smoke should preserve sidecar {expectedSidecarExtension} for {path}");

        var loadedSnapshot = CopperfinStudioSnapshotClient.TryLoad(path!);
        Expect(loadedSnapshot.Success && loadedSnapshot.Document is not null,
            $"visual asset editor smoke should load snapshot data for {path}");
        if (!loadedSnapshot.Success || loadedSnapshot.Document is null)
        {
            return;
        }

        Expect(string.Equals(loadedSnapshot.Document.AssetFamily, expectedAssetFamily, StringComparison.Ordinal),
            $"visual asset editor smoke should preserve the native {expectedAssetFamily} document identity for {path}");
        Expect(loadedSnapshot.Document.HasSidecar &&
               string.Equals(loadedSnapshot.Document.SidecarPath, expectedSidecarPath, StringComparison.OrdinalIgnoreCase),
            $"visual asset editor smoke should preserve the expected sidecar path for {path}");

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();
        control.LoadDocument(path!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => HasLabelText(control, expectedTitle) &&
                  HasLabelTextContaining(control, expectedGuidanceSnippet) &&
                  HasLabelTextContaining(control, "Snapshot loaded:") &&
                  FindListViews(control).Any(list => list.Items.Count > 0));
        Expect(loaded, $"visual asset editor smoke should load family-specific editor guidance for {path}");

        Expect(HasLabelText(control, expectedTitle),
            $"visual asset editor smoke should title {expectedAssetFamily} assets correctly for {path}");
        Expect(HasLabelTextContaining(control, expectedGuidanceSnippet),
            $"visual asset editor smoke should surface {expectedAssetFamily}-specific guidance for {path}");
        Expect(!FindButtons(control).Any(button => button.Visible && button.Text == "Build Copperfin Project"),
            $"visual asset editor smoke should keep {expectedAssetFamily} assets out of the project workflow shell for {path}");

        var designSurface = FindDesignSurface(control);
        Expect(designSurface is not null, $"visual asset editor smoke should surface a design canvas for {path}");
        if (designSurface is not null)
        {
            using var bitmap = new Bitmap(Math.Max(1, designSurface.Width), Math.Max(1, designSurface.Height));
            designSurface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
            var minimumVisiblePixelCount = expectedAssetFamily == "form" ? 1500 : 100;
            Expect(CountNonWhitePixels(bitmap) > minimumVisiblePixelCount,
                $"visual asset editor smoke should render visible design content for {path}");
        }

        TearDownForm(hostForm);
    }

    private static void SmokeProjectDebuggerWithRealAsset(string? path)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real project debug asset candidate" : path)} not found.");
            return;
        }

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();
        control.LoadDocument(path!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => FindButtons(control).Any(button => button.Text == "Debug Copperfin Project"));
        Expect(loaded, $"project debugger command should load for {path}");

        var debugButton = FindButtons(control).FirstOrDefault(button => button.Text == "Debug Copperfin Project");
        if (debugButton is null)
        {
            TearDownForm(hostForm);
            return;
        }

        debugButton.PerformClick();
        var debugLoaded = WaitUntil(
            TimeSpan.FromSeconds(30),
            () => FindRichTextBoxes(control)
                .Any(box => box.Text.IndexOf("Copperfin Debug Session", StringComparison.OrdinalIgnoreCase) >= 0 &&
                            box.Text.IndexOf("Pause Reason:", StringComparison.OrdinalIgnoreCase) >= 0));
        Expect(debugLoaded, $"project debugger should surface a runtime pause state for {path}");

        var debuggerSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Debug Session", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(debuggerSummary is not null, $"project debugger should surface a debug summary for {path}");
        var debugSession = GetPrivateField<CopperfinRuntimeDebugSession>(control, "currentDebugSession");
        Expect(debugSession is not null, $"project debugger should preserve the active debug session for {path}");
        if (debugSession is not null)
        {
            Expect(!string.IsNullOrWhiteSpace(debugSession.ManifestPath) && File.Exists(debugSession.ManifestPath),
                $"project debugger should preserve the runtime manifest path for {path}");
            Expect(debugSession.BuildWarningCount == 0,
                $"project debugger should inherit a warning-free build for {path}");
            Expect(debugSession.BuildWarnings.Count == 0,
                $"project debugger should not inherit build warning lines for {path}");
            if (!string.IsNullOrWhiteSpace(debugSession.ManifestPath) && File.Exists(debugSession.ManifestPath))
            {
                var manifestText = File.ReadAllText(debugSession.ManifestPath);
                Expect(manifestText.IndexOf("warning=", StringComparison.OrdinalIgnoreCase) < 0,
                    $"project debugger manifest should remain warning-free for {path}");
                Expect(manifestText.IndexOf("asset=6|wzcommon/registry.vcx|", StringComparison.Ordinal) >= 0,
                    $"project debugger manifest should stage the shared class dependency for {path}");
            }

            var initialDebugManifestPath = debugSession.DebugManifestPath;
            var restartButton = GetPrivateButton(control, "debugRestartButton");
            Expect(restartButton.Enabled, $"project debugger restart should remain enabled for {path}");
            restartButton.PerformClick();
            var restarted = WaitUntil(
                TimeSpan.FromSeconds(30),
                () =>
                {
                    var restartedSession = GetPrivateField<CopperfinRuntimeDebugSession>(control, "currentDebugSession");
                    return restartedSession is not null &&
                           !ReferenceEquals(restartedSession, debugSession) &&
                           restartedSession.Success &&
                           string.Equals(restartedSession.Commands.LastOrDefault(), "continue", StringComparison.Ordinal) &&
                           !string.IsNullOrWhiteSpace(restartedSession.DebugManifestPath) &&
                           !string.Equals(restartedSession.DebugManifestPath, initialDebugManifestPath, StringComparison.Ordinal);
                });
            var observedRestartSession = GetPrivateField<CopperfinRuntimeDebugSession>(control, "currentDebugSession");
            var sameSession = ReferenceEquals(observedRestartSession, debugSession);
            var successfulRestart = observedRestartSession?.Success == true;
            var restartCommandIsContinue = string.Equals(
                observedRestartSession?.Commands.LastOrDefault(),
                "continue",
                StringComparison.Ordinal);
            var restartManifestIsDistinct = observedRestartSession is not null &&
                                            !string.IsNullOrWhiteSpace(observedRestartSession.DebugManifestPath) &&
                                            !string.Equals(
                                                observedRestartSession.DebugManifestPath,
                                                initialDebugManifestPath,
                                                StringComparison.Ordinal);
            Expect(restarted,
                $"project debugger restart should materialize a fresh debug session for {path} " +
                $"(sameSession={sameSession}; success={successfulRestart}; " +
                $"lastCommandContinue={restartCommandIsContinue}; distinctDebugManifest={restartManifestIsDistinct}; " +
                $"initial={DescribeDebugSession(debugSession)}; observed={DescribeDebugSession(observedRestartSession)})");

            var restartedSession = GetPrivateField<CopperfinRuntimeDebugSession>(control, "currentDebugSession");
            Expect(restartedSession is not null, $"project debugger restart should retain the refreshed debug session for {path}");
            if (restartedSession is not null)
            {
                Expect(!string.IsNullOrWhiteSpace(restartedSession.ManifestPath) && File.Exists(restartedSession.ManifestPath),
                    $"project debugger restart should preserve the runtime manifest path for {path}");
                Expect(!string.IsNullOrWhiteSpace(restartedSession.DebugManifestPath) && File.Exists(restartedSession.DebugManifestPath),
                    $"project debugger restart should preserve a debug manifest path for {path}");
                Expect(restartedSession.BuildWarningCount == 0,
                    $"project debugger restart should inherit a warning-free build for {path}");
                Expect(restartedSession.BuildWarnings.Count == 0,
                    $"project debugger restart should not inherit build warning lines for {path}");
                Expect(restartedSession.Commands.Count == 1 &&
                       string.Equals(restartedSession.Commands[0], "continue", StringComparison.Ordinal),
                    $"project debugger restart should reset command history for {path}");
                if (!string.IsNullOrWhiteSpace(restartedSession.ManifestPath) && File.Exists(restartedSession.ManifestPath))
                {
                    var restartedManifestText = File.ReadAllText(restartedSession.ManifestPath);
                    Expect(restartedManifestText.IndexOf("warning=", StringComparison.OrdinalIgnoreCase) < 0,
                        $"project debugger restart manifest should remain warning-free for {path}");
                    Expect(restartedManifestText.IndexOf("asset=6|wzcommon/registry.vcx|", StringComparison.Ordinal) >= 0,
                        $"project debugger restart manifest should stage the shared class dependency for {path}");
                }
            }
        }
        if (debuggerSummary is not null)
        {
            Expect(debuggerSummary.Text.IndexOf("Call Stack:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project debugger should include a call stack for {path} " +
                $"(summary={DescribeDebugSummary(debuggerSummary.Text)})");
            Expect(debuggerSummary.Text.IndexOf("Runtime Events:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project debugger should include runtime events for {path} " +
                $"(summary={DescribeDebugSummary(debuggerSummary.Text)})");
        }

        var debuggerCallStackView = GetPrivateListView(control, "debuggerCallStackView");
        var debuggerLocalsView = GetPrivateListView(control, "debuggerLocalsView");
        var debuggerGlobalsView = GetPrivateListView(control, "debuggerGlobalsView");
        var debuggerEventsView = GetPrivateListView(control, "debuggerEventsView");
        Expect(debuggerCallStackView.Columns.Count == 2 &&
               debuggerLocalsView.Columns.Count == 3 &&
               debuggerGlobalsView.Columns.Count == 2 &&
               debuggerEventsView.Columns.Count == 3,
               $"project debugger should expose localized frame, local, global, and event table schemas for {path}");
        var appliedDebugSession = GetPrivateField<CopperfinRuntimeDebugSession>(control, "currentDebugSession");
        if (appliedDebugSession is not null && appliedDebugSession.Success)
        {
            var expectedLocals = appliedDebugSession.State.Frames.Sum(frame => frame.Locals.Count);
            Expect(debuggerCallStackView.Items.Count == appliedDebugSession.State.Frames.Count &&
                   debuggerLocalsView.Items.Count == expectedLocals &&
                   debuggerGlobalsView.Items.Count == appliedDebugSession.State.Globals.Count &&
                   debuggerEventsView.Items.Count == appliedDebugSession.State.Events.Count,
                   $"project debugger detail tables should mirror the active runtime pause state for {path}");
        }

        TearDownForm(hostForm);
    }

    private static string DescribeDebugSummary(string summary)
    {
        var compact = (summary ?? string.Empty).Replace("\r", " ").Replace("\n", " | ");
        return compact.Length <= 500 ? compact : compact.Substring(0, 500) + "...";
    }

    private static void SmokeProjectBuildWorkflowWithRealAsset(string? path)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real project build asset candidate" : path)} not found.");
            return;
        }

        #pragma warning disable VSTHRD002
        var result = CopperfinProjectWorkflow.ExecuteAsync(path!, CopperfinProjectOperation.Build)
            .GetAwaiter()
            .GetResult();
        #pragma warning restore VSTHRD002
        Expect(result.Success, $"project build workflow should succeed for {path}");
        if (!result.Success)
        {
            return;
        }

        Expect(!string.IsNullOrWhiteSpace(result.OutputDirectory) && Directory.Exists(result.OutputDirectory),
            $"project build workflow should materialize an output directory for {path}");
        Expect(!string.IsNullOrWhiteSpace(result.ManifestPath) && File.Exists(result.ManifestPath),
            $"project build workflow should materialize a runtime manifest for {path}");
        Expect(!string.IsNullOrWhiteSpace(result.LauncherPath) && File.Exists(result.LauncherPath),
            $"project build workflow should materialize a launcher path for {path}");
        Expect(!string.IsNullOrWhiteSpace(result.DebugManifestPath) && File.Exists(result.DebugManifestPath),
            $"project build workflow should materialize a debug manifest for {path}");
        Expect(result.WarningCount == 0,
            $"project build workflow should remain warning-free for {path}");
        Expect(result.Warnings.Count == 0,
            $"project build workflow should not surface warning lines for {path}");
        if (!string.IsNullOrWhiteSpace(result.ManifestPath) && File.Exists(result.ManifestPath))
        {
            var manifestText = File.ReadAllText(result.ManifestPath);
            Expect(manifestText.IndexOf("warning=", StringComparison.OrdinalIgnoreCase) < 0,
                $"project build workflow manifest should remain warning-free for {path}");
            Expect(manifestText.IndexOf("asset=6|wzcommon/registry.vcx|", StringComparison.Ordinal) >= 0,
                $"project build workflow manifest should stage the shared class dependency for {path}");
        }
    }

    private static void SmokeProjectWorkflowWarningParsingLocalization()
    {
        var parseWarningLinesMethod = typeof(CopperfinProjectWorkflow).GetMethod(
            "ParseWarningLines",
            BindingFlags.Static | BindingFlags.NonPublic);
        Expect(parseWarningLinesMethod is not null,
            "project workflow warning parsing smoke should locate the private warning parser");
        if (parseWarningLinesMethod is null)
        {
            return;
        }

        List<string> ParseWarnings(string locale, string text)
        {
            var result = parseWarningLinesMethod.Invoke(
                null,
                new object?[] { text, new CopperfinLocalization(locale) });
            return result as List<string> ?? new List<string>();
        }

        var englishWarnings = ParseWarnings(
            "en-US",
            "warning: English warning\nstatus: ok\n");
        Expect(englishWarnings.Count == 1 &&
               string.Equals(englishWarnings[0], "English warning", StringComparison.Ordinal),
            "project workflow warning parsing smoke should preserve the English warning body");

        var spanishWarnings = ParseWarnings(
            "es-419",
            "advertencia: Advertencia localizada\n");
        Expect(spanishWarnings.Count == 1 &&
               string.Equals(spanishWarnings[0], "Advertencia localizada", StringComparison.Ordinal),
            "project workflow warning parsing smoke should recognize the Spanish warning prefix");

        var spanishFallbackWarnings = ParseWarnings(
            "es-419",
            "warning: English fallback warning\n");
        Expect(spanishFallbackWarnings.Count == 1 &&
               string.Equals(spanishFallbackWarnings[0], "English fallback warning", StringComparison.Ordinal),
            "project workflow warning parsing smoke should keep the English warning fallback under Spanish locale");

        var portugueseWarnings = ParseWarnings(
            "pt-BR",
            "aviso: Aviso localizado\n");
        Expect(portugueseWarnings.Count == 1 &&
               string.Equals(portugueseWarnings[0], "Aviso localizado", StringComparison.Ordinal),
            "project workflow warning parsing smoke should recognize the Portuguese warning prefix");

        var pseudoWarnings = ParseWarnings(
            "qps-ploc",
            "warning: Pseudo invariant warning\n");
        Expect(pseudoWarnings.Count == 1 &&
               string.Equals(pseudoWarnings[0], "Pseudo invariant warning", StringComparison.Ordinal),
            "project workflow warning parsing smoke should keep the invariant native warning prefix under qps-ploc");
    }

    private static void SmokeManagedProjectProcessLaunchContracts()
    {
        var quotedProject = "\"C:\\work dir\\sample.pjx\"";
        var launchArguments = new[] { "build", "--project", quotedProject, "--empty", string.Empty };
        void ExpectArgumentEcho(CopperfinProcessExecutionResult result, string description)
        {
            var status = result.Values.TryGetValue("status", out var parsedStatus) ? parsedStatus : string.Empty;
            var first = result.Values.TryGetValue("first", out var parsedFirst) ? parsedFirst : string.Empty;
            var spaced = result.Values.TryGetValue("spaced", out var parsedSpaced) ? parsedSpaced : string.Empty;
            var empty = result.Values.TryGetValue("empty", out var parsedEmpty) ? parsedEmpty : string.Empty;
            Expect(result.ExitCode == 0 &&
                   string.Equals(status, "ok", StringComparison.OrdinalIgnoreCase) &&
                   string.Equals(first, "build", StringComparison.Ordinal) &&
                   string.Equals(spaced, "space value", StringComparison.Ordinal) &&
                   string.Equals(empty, "[]", StringComparison.Ordinal),
                description);
        }

        foreach (var directPath in new[] { @"C:\tools\copperfin_build_host.exe", @"C:\tools\copperfin_build_host" })
        {
            var directStartInfo = CopperfinProjectWorkflow.CreateProcessStartInfo(
                directPath,
                launchArguments,
                localization: new CopperfinLocalization("es-419"),
                redirectOutput: true,
                createNoWindow: true,
                isWindowsOverride: true);
            Expect(string.Equals(directStartInfo.FileName, directPath, StringComparison.Ordinal) &&
                   directStartInfo.Arguments.IndexOf(quotedProject, StringComparison.Ordinal) >= 0 &&
                   directStartInfo.Arguments.EndsWith("--empty \"\"", StringComparison.Ordinal) &&
                   !directStartInfo.UseShellExecute &&
                   directStartInfo.RedirectStandardOutput &&
                   directStartInfo.RedirectStandardError &&
                   directStartInfo.CreateNoWindow,
                $"managed project workflow should launch direct Windows host path without a command-shell wrapper: {directPath}");
            Expect(string.Equals(directStartInfo.EnvironmentVariables["COPPERFIN_LOCALE"], "es-419", StringComparison.Ordinal) &&
                   string.Equals(directStartInfo.EnvironmentVariables["COPPERFIN_UI_LOCALE"], "es-419", StringComparison.Ordinal),
                "managed project workflow should preserve invariant locale environment-variable names");
        }

        var expectedCommandInterpreter = Environment.GetEnvironmentVariable("COMSPEC");
        if (string.IsNullOrWhiteSpace(expectedCommandInterpreter))
        {
            expectedCommandInterpreter = "cmd.exe";
        }
        foreach (var extension in new[] { ".cmd", ".bat" })
        {
            var wrapperPath = @"C:\tools\build host" + extension;
            var wrapperStartInfo = CopperfinProjectWorkflow.CreateProcessStartInfo(
                wrapperPath,
                launchArguments,
                redirectOutput: true,
                createNoWindow: true,
                isWindowsOverride: true);
            Expect(string.Equals(wrapperStartInfo.FileName, expectedCommandInterpreter, StringComparison.OrdinalIgnoreCase) &&
                   string.Equals(wrapperStartInfo.Arguments, "/d /c %COPPERFIN_SCRIPT_WRAPPER_COMMAND%", StringComparison.Ordinal) &&
                   string.Equals(wrapperStartInfo.EnvironmentVariables["COPPERFIN_SCRIPT_WRAPPER_COMMAND"],
                       "\"" + wrapperPath + "\" build --project " + quotedProject + " --empty \"\"", StringComparison.Ordinal),
                $"managed project workflow should route Windows {extension} hosts through COMSPEC without dropping spaced or empty arguments");

            var posixStartInfo = CopperfinProjectWorkflow.CreateProcessStartInfo(
                wrapperPath,
                launchArguments,
                isWindowsOverride: false);
            Expect(string.Equals(posixStartInfo.FileName, wrapperPath, StringComparison.Ordinal) &&
                   string.Equals(posixStartInfo.Arguments, string.Join(" ", launchArguments.Take(launchArguments.Length - 1)) + " \"\"", StringComparison.Ordinal),
                $"managed project workflow should preserve direct-executable behavior for {extension} paths off Windows");

            var percentWrapperPath = @"C:\tools\%USERNAME%\build host" + extension;
            var percentWrapperStartInfo = CopperfinProjectWorkflow.CreateProcessStartInfo(
                percentWrapperPath,
                launchArguments,
                isWindowsOverride: true);
            Expect(string.Equals(percentWrapperStartInfo.Arguments, "/d /c %COPPERFIN_SCRIPT_WRAPPER_COMMAND%", StringComparison.Ordinal) &&
                   string.Equals(percentWrapperStartInfo.EnvironmentVariables["COPPERFIN_SCRIPT_WRAPPER_COMMAND"],
                       "\"" + percentWrapperPath + "\" build --project " + quotedProject + " --empty \"\"", StringComparison.Ordinal),
                $"managed project workflow should preserve literal percent characters in Windows {extension} wrapper paths");
        }

        const string debugArguments = "--manifest \"C:\\debug manifest\\app.cfdebug\" --debug --locale \"es-419\" --debug-command \"step into\" --debug-command \"\"";
        foreach (var directPath in new[] { @"C:\tools\copperfin_runtime_host.exe", @"C:\tools\copperfin_runtime_host" })
        {
            var directStartInfo = CopperfinRuntimeDebugClient.CreateReplayProcessStartInfo(
                directPath,
                debugArguments,
                localization: new CopperfinLocalization("es-419"),
                isWindowsOverride: true);
            Expect(string.Equals(directStartInfo.FileName, directPath, StringComparison.Ordinal) &&
                   string.Equals(directStartInfo.Arguments, debugArguments, StringComparison.Ordinal) &&
                   !directStartInfo.UseShellExecute &&
                   directStartInfo.RedirectStandardOutput &&
                   directStartInfo.RedirectStandardError &&
                   directStartInfo.CreateNoWindow &&
                   string.Equals(directStartInfo.EnvironmentVariables["COPPERFIN_LOCALE"], "es-419", StringComparison.Ordinal) &&
                   string.Equals(directStartInfo.EnvironmentVariables["COPPERFIN_UI_LOCALE"], "es-419", StringComparison.Ordinal),
                $"runtime debug replay should launch direct Windows host path without a command-shell wrapper: {directPath}");
        }

        foreach (var extension in new[] { ".cmd", ".bat" })
        {
            var wrapperPath = @"C:\tools\runtime host" + extension;
            var wrapperStartInfo = CopperfinRuntimeDebugClient.CreateReplayProcessStartInfo(
                wrapperPath,
                debugArguments,
                isWindowsOverride: true);
            Expect(string.Equals(wrapperStartInfo.FileName, expectedCommandInterpreter, StringComparison.OrdinalIgnoreCase) &&
                   string.Equals(wrapperStartInfo.Arguments, "/d /c %COPPERFIN_SCRIPT_WRAPPER_COMMAND%", StringComparison.Ordinal) &&
                   string.Equals(wrapperStartInfo.EnvironmentVariables["COPPERFIN_SCRIPT_WRAPPER_COMMAND"],
                       "\"" + wrapperPath + "\" " + debugArguments, StringComparison.Ordinal),
                $"runtime debug replay should route Windows {extension} hosts through COMSPEC without dropping manifest, debug-command, or empty arguments");

            var percentWrapperPath = @"C:\tools\%USERNAME%\runtime host" + extension;
            var percentWrapperStartInfo = CopperfinRuntimeDebugClient.CreateReplayProcessStartInfo(
                percentWrapperPath,
                debugArguments,
                isWindowsOverride: true);
            Expect(string.Equals(percentWrapperStartInfo.Arguments, "/d /c %COPPERFIN_SCRIPT_WRAPPER_COMMAND%", StringComparison.Ordinal) &&
                   string.Equals(percentWrapperStartInfo.EnvironmentVariables["COPPERFIN_SCRIPT_WRAPPER_COMMAND"],
                       "\"" + percentWrapperPath + "\" " + debugArguments, StringComparison.Ordinal),
                $"runtime debug replay should preserve literal percent characters in Windows {extension} wrapper paths");

            var posixStartInfo = CopperfinRuntimeDebugClient.CreateReplayProcessStartInfo(
                wrapperPath,
                debugArguments,
                isWindowsOverride: false);
            Expect(string.Equals(posixStartInfo.FileName, wrapperPath, StringComparison.Ordinal) &&
                   string.Equals(posixStartInfo.Arguments, debugArguments, StringComparison.Ordinal),
                $"runtime debug replay should preserve direct-executable behavior for {extension} paths off Windows");
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "copperfin-managed-process-launch-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var previousBuildHostPath = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        var previousRuntimeHostPath = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        var previousStudioHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        try
        {
            foreach (var configuredFileName in new[]
                     {
                         "configured-build-host.exe",
                         "configured-build-host",
                         "configured build host.cmd",
                         "configured build host.bat"
                     })
            {
                var configuredHostPath = Path.Combine(tempRoot, configuredFileName);
                File.WriteAllText(configuredHostPath, "@echo off\r\nexit /b 0\r\n");
                Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", configuredHostPath);
                Expect(string.Equals(CopperfinProjectWorkflow.ResolveBuildHostPath(tempRoot), configuredHostPath, StringComparison.Ordinal),
                    $"managed project workflow should preserve an explicitly configured host path: {configuredFileName}");
            }

            var directBuildDirectory = Path.Combine(tempRoot, "build");
            Directory.CreateDirectory(directBuildDirectory);
            var directStudioHostPath = Path.Combine(
                directBuildDirectory,
                "copperfin_studio_host" + (IsWindowsPlatform() ? ".exe" : string.Empty));
            File.WriteAllText(directStudioHostPath, IsWindowsPlatform() ? "stub" : "#!/bin/sh\nexit 0\n");
            if (!IsWindowsPlatform())
            {
                MakeExecutable(directStudioHostPath);
            }

            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", null);
            Expect(string.Equals(
                       CopperfinStudioHostBridge.ResolveStudioHostPath(tempRoot),
                       directStudioHostPath,
                       IsWindowsPlatform() ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal),
                "managed Studio host discovery should include the repository-style single-level build output");

            var missingHostPath = Path.Combine(
                tempRoot,
                "missing-build-host-" + Guid.NewGuid().ToString("N") + (IsWindowsPlatform() ? ".exe" : string.Empty));
            var capturedFailure = CopperfinProcessRunner.Run(new ProcessStartInfo
            {
                FileName = missingHostPath,
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true
            });
            Expect(!capturedFailure.Started &&
                   capturedFailure.ExitCode == -1 &&
                   !string.IsNullOrWhiteSpace(capturedFailure.StandardError),
                "managed process runner should contain process-start exceptions and retain the host diagnostic");

            var localizedFailure = CopperfinProjectWorkflow.RunProcess(
                missingHostPath,
                Array.Empty<string>(),
                localization: new CopperfinLocalization("es-419"));
            Expect(localizedFailure.ExitCode == -1 &&
                   localizedFailure.StandardError.StartsWith("No se pudo iniciar el proceso de Copperfin:", StringComparison.Ordinal) &&
                   localizedFailure.StandardError.Length > "No se pudo iniciar el proceso de Copperfin:".Length,
                "managed project workflow should localize process-start failures without discarding diagnostic detail");
            Expect(new CopperfinLocalization("qps-ploc")
                    .Format("AssetEditor.Project.Workflow.ProcessCouldNotStartWithMessage", "detail")
                    .IndexOf("detail", StringComparison.Ordinal) >= 0,
                "managed project workflow process-start detail should survive pseudo-localization");
            Expect(new CopperfinLocalization("pt-BR")
                    .Format("AssetEditor.Project.Workflow.ProcessCouldNotStartWithMessage", "detalhe")
                    .StartsWith("Não foi possível iniciar o processo do Copperfin:", StringComparison.Ordinal),
                "managed project workflow process-start detail should route through the Portuguese catalog");

            var unlaunchableRuntimeHostPath = Path.Combine(tempRoot, "unlaunchable runtime host.exe");
            var missingRuntimeHostPath = Path.Combine(tempRoot, "missing runtime host.exe");
            var debugManifestPath = Path.Combine(tempRoot, "app.cfdebug");
            File.WriteAllText(unlaunchableRuntimeHostPath, "not an executable runtime host");
            File.WriteAllText(debugManifestPath, "manifest_version=1\n");
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", missingRuntimeHostPath);
            #pragma warning disable VSTHRD002
            var debugMissingHost = CopperfinRuntimeDebugClient.ReplayAsync(
                    new CopperfinRuntimeDebugSession
                    {
                        Success = true,
                        DebugManifestPath = debugManifestPath,
                        Commands = new List<string> { "continue" }
                    },
                    new CopperfinLocalization("es-419"))
                .GetAwaiter()
                .GetResult();
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", unlaunchableRuntimeHostPath);
            var debugStartFailure = CopperfinRuntimeDebugClient.ReplayAsync(
                    new CopperfinRuntimeDebugSession
                    {
                        Success = true,
                        DebugManifestPath = debugManifestPath,
                        Commands = new List<string> { "continue" }
                    },
                    new CopperfinLocalization("es-419"))
                .GetAwaiter()
                .GetResult();
            #pragma warning restore VSTHRD002
            Expect(!debugMissingHost.Success &&
                   string.Equals(debugMissingHost.Error, "No se encontró el host de runtime de Copperfin.", StringComparison.Ordinal),
                "runtime debug replay should report a missing runtime host through the localized debug-client contract");
            Expect(!debugStartFailure.Success &&
                   debugStartFailure.Error.StartsWith("No se pudo iniciar el host de runtime de Copperfin:", StringComparison.Ordinal) &&
                   debugStartFailure.Error.Length > "No se pudo iniciar el host de runtime de Copperfin:".Length,
                "runtime debug replay should localize process-start failures without discarding runtime-host diagnostics");
            Expect(new CopperfinLocalization("qps-ploc")
                    .Format("AssetEditor.Dialog.RuntimeHostCouldNotStartWithMessage", "detail")
                    .IndexOf("detail", StringComparison.Ordinal) >= 0,
                "runtime debug replay process-start detail should survive pseudo-localization");
            Expect(new CopperfinLocalization("pt-BR")
                    .Format("AssetEditor.Dialog.RuntimeHostCouldNotStartWithMessage", "detalhe")
                    .StartsWith("Não foi possível iniciar o host de runtime do Copperfin:", StringComparison.Ordinal),
                "runtime debug replay process-start detail should route through the Portuguese catalog");

            if (IsWindowsPlatform())
            {
                var batchHostPath = Path.Combine(tempRoot, "working build host.cmd");
                File.WriteAllText(
                    batchHostPath,
                    "@echo off\r\n" +
                    "echo status: ok\r\n" +
                    "echo first: %~1\r\n" +
                    "echo spaced: %~3\r\n" +
                    "echo empty: [%~4]\r\n" +
                    "exit /b 0\r\n");
                var batchResult = CopperfinProjectWorkflow.RunProcess(
                    batchHostPath,
                    new[] { "build", "--value", "\"space value\"", string.Empty });
                ExpectArgumentEcho(
                    batchResult,
                    "managed project workflow should execute configured Windows batch hosts with spaced and empty arguments intact");
            }
            else
            {
                var posixHostPath = Path.Combine(tempRoot, "working build host");
                File.WriteAllText(
                    posixHostPath,
                    "#!/bin/sh\n" +
                    "printf 'status: ok\\n'\n" +
                    "printf 'first: %s\\n' \"$1\"\n" +
                    "printf 'spaced: %s\\n' \"$3\"\n" +
                    "printf 'empty: [%s]\\n' \"$4\"\n");
                MakeExecutable(posixHostPath);
                var posixResult = CopperfinProjectWorkflow.RunProcess(
                    posixHostPath,
                    new[] { "build", "--value", "\"space value\"", string.Empty });
                ExpectArgumentEcho(
                    posixResult,
                    "managed project workflow should execute extensionless POSIX hosts with spaced and empty arguments intact");
            }
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", previousBuildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", previousRuntimeHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousStudioHostPath);
            Directory.Delete(tempRoot, recursive: true);
        }
    }

    private static void SmokeProjectRunWorkflowWithRealAsset(string? path)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real project run asset candidate" : path)} not found.");
            return;
        }

        #pragma warning disable VSTHRD002
        var result = CopperfinProjectWorkflow.ExecuteAsync(path!, CopperfinProjectOperation.Run)
            .GetAwaiter()
            .GetResult();
        #pragma warning restore VSTHRD002
        Expect(result.Success, $"project run workflow should succeed for {path}");
        if (!result.Success)
        {
            return;
        }

        Expect(!string.IsNullOrWhiteSpace(result.OutputDirectory) && Directory.Exists(result.OutputDirectory),
            $"project run workflow should materialize an output directory for {path}");
        Expect(!string.IsNullOrWhiteSpace(result.ManifestPath) && File.Exists(result.ManifestPath),
            $"project run workflow should preserve a runtime manifest for {path}");
        Expect(!string.IsNullOrWhiteSpace(result.LauncherPath) && File.Exists(result.LauncherPath),
            $"project run workflow should preserve a launcher path for {path}");
        Expect(!string.IsNullOrWhiteSpace(result.DebugManifestPath) && File.Exists(result.DebugManifestPath),
            $"project run workflow should preserve a debug manifest for {path}");
        Expect(result.WarningCount == 0,
            $"project run workflow should inherit a warning-free build for {path}");
        Expect(result.Warnings.Count == 0,
            $"project run workflow should not surface warning lines for {path}");
        if (!string.IsNullOrWhiteSpace(result.ManifestPath) && File.Exists(result.ManifestPath))
        {
            var manifestText = File.ReadAllText(result.ManifestPath);
            Expect(manifestText.IndexOf("warning=", StringComparison.OrdinalIgnoreCase) < 0,
                $"project run workflow manifest should remain warning-free for {path}");
            Expect(manifestText.IndexOf("asset=6|wzcommon/registry.vcx|", StringComparison.Ordinal) >= 0,
                $"project run workflow manifest should stage the shared class dependency for {path}");
        }
    }

}
