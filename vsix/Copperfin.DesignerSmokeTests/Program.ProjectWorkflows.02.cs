
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
    private static void SmokeCrossPlatformFileRevealContracts()
    {
        const string assetPath = "/tmp/Copperfin Samples/orders.frx";
        var windows = CopperfinFileManager.CreateRevealStartInfo(
            assetPath,
            CopperfinFileManagerPlatform.Windows);
        Expect(windows.FileName == "explorer.exe" &&
               windows.Arguments == "/select,\"/tmp/Copperfin Samples/orders.frx\"" &&
               windows.UseShellExecute,
            "Windows file reveal should select the exact asset in Explorer");

        var macOs = CopperfinFileManager.CreateRevealStartInfo(
            assetPath,
            CopperfinFileManagerPlatform.MacOS);
        Expect(macOs.FileName == "open" &&
               macOs.Arguments == "--reveal \"/tmp/Copperfin Samples/orders.frx\"" &&
               !macOs.UseShellExecute,
            "macOS file reveal should ask Finder to reveal the exact asset");

        var linux = CopperfinFileManager.CreateRevealStartInfo(
            assetPath,
            CopperfinFileManagerPlatform.Linux);
        Expect(linux.FileName == "xdg-open" &&
               linux.Arguments == "\"/tmp/Copperfin Samples\"" &&
               !linux.UseShellExecute,
            "Linux file reveal should open the containing directory through the desktop opener");
    }

    private static void SmokeProjectWorkspaceEntryActivation()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin-designer-smoke",
            "project-entry-activation",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        var projectPath = Path.Combine(root, "sample.pjx");
        var childPath = Path.Combine(root, "forms", "orders.scx");
        var outsidePath = Path.Combine(root, "..", "outside.prg");
        try
        {
            File.WriteAllText(projectPath, "project");
            var supportedRelativePaths = new[]
            {
                "code\\main.prg",
                "forms\\orders.scx",
                "classes\\shared.vcx",
                "reports\\invoice.frx",
                "labels\\customer.lbx",
                "menus\\main.mnx"
            };
            foreach (var relativePath in supportedRelativePaths)
            {
                var supportedPath = Path.Combine(
                    root,
                    relativePath.Replace('\\', Path.DirectorySeparatorChar));
                Directory.CreateDirectory(Path.GetDirectoryName(supportedPath)!);
                File.WriteAllText(supportedPath, "asset");
            }
            var taskSourcePath = Path.Combine(root, "code", "main.prg");
            File.WriteAllText(taskSourcePath, "* Copperfin task source" + Environment.NewLine + "* TODO: wire the startup command" + Environment.NewLine);
            var dataAssetPath = Path.Combine(root, "data", "customers.dbf");
            Directory.CreateDirectory(Path.GetDirectoryName(dataAssetPath)!);
            File.WriteAllText(dataAssetPath, "data asset");
            File.WriteAllText(Path.GetFullPath(outsidePath), "outside");

            foreach (var relativePath in supportedRelativePaths)
            {
                var expectedPath = CopperfinDocumentPathIdentity.Normalize(
                    Path.Combine(root, relativePath.Replace('\\', Path.DirectorySeparatorChar)));
                Expect(
                    CopperfinProjectEntryActivation.TryResolve(
                        projectPath,
                        new CopperfinStudioProjectEntry { RelativePath = relativePath },
                        out var resolvedSupportedPath) &&
                    string.Equals(resolvedSupportedPath, expectedPath, StringComparison.Ordinal),
                    $"project workspace activation should resolve supported {Path.GetExtension(relativePath)} child assets");
            }

            var entry = new CopperfinStudioProjectEntry
            {
                RelativePath = "forms\\orders.scx"
            };
            Expect(
                CopperfinProjectEntryActivation.TryResolve(projectPath, entry, out var resolvedPath) &&
                string.Equals(
                    resolvedPath,
                    CopperfinDocumentPathIdentity.Normalize(childPath),
                    StringComparison.Ordinal),
                "project workspace activation should resolve supported child assets relative to the PJX directory");

            entry.RelativePath = "../outside.prg";
            Expect(
                !CopperfinProjectEntryActivation.TryResolve(projectPath, entry, out _),
                "project workspace activation should reject child paths that escape the PJX directory");

            entry.RelativePath = "missing.prg";
            Expect(
                !CopperfinProjectEntryActivation.TryResolve(projectPath, entry, out _),
                "project workspace activation should reject missing child assets");

            entry.RelativePath = "notes.txt";
            File.WriteAllText(Path.Combine(root, "notes.txt"), "unsupported");
            Expect(
                !CopperfinProjectEntryActivation.TryResolve(projectPath, entry, out _),
                "project workspace activation should reject unsupported child asset types");

            using var hostForm = new Form
            {
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };
            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };
            string? requestedPath = null;
            string? requestedTaskPath = null;
            var requestedTaskLine = 0;
            control.OpenDocumentRequested += path => requestedPath = path;
            control.OpenDocumentAtLineRequested += (path, line) =>
            {
                requestedTaskPath = path;
                requestedTaskLine = line;
            };
            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            SetPrivateField(control, "currentPath", projectPath);
            var projectEntry = new CopperfinStudioProjectEntry
            {
                RecordIndex = 7,
                RelativePath = "forms\\orders.scx"
            };
            var snapshotObject = new CopperfinStudioSnapshotObject
            {
                RecordIndex = 7,
                Title = "forms\\orders.scx"
            };
            SetCurrentSnapshot(control, new CopperfinStudioSnapshotDocument
            {
                Path = projectPath,
                AssetFamily = "project",
                ProjectWorkspace = new CopperfinStudioProjectWorkspace
                {
                    Entries = new List<CopperfinStudioProjectEntry> { projectEntry }
                },
                Objects = new List<CopperfinStudioSnapshotObject> { snapshotObject }
            });
            GetPrivateField<ListView>(control, "sectionListView")?.Items.Clear();
            typeof(CopperfinAssetEditorControl)
                .GetMethod("PopulateObjectList", BindingFlags.Instance | BindingFlags.NonPublic)
                ?.Invoke(control, new object[] { true });
            var objectList = FindListViews(control)
                .FirstOrDefault(list => list.Columns.Count >= 3 &&
                                        string.Equals(
                                            list.Columns[0].Text,
                                            "Item",
                                            StringComparison.OrdinalIgnoreCase) &&
                                        string.Equals(
                                            list.Columns[1].Text,
                                            "Group",
                                            StringComparison.OrdinalIgnoreCase));
            if (objectList is not null && objectList.Items.Count > 0)
            {
                objectList.Focus();
                objectList.Items[0].Selected = true;
            }
            Expect(objectList is not null && objectList.SelectedItems.Count == 1,
                "project workspace activation should select the first project entry in the shared editor list");
            Expect(
                control.TryActivateSelectedProjectEntry() &&
                string.Equals(
                    requestedPath,
                    CopperfinDocumentPathIdentity.Normalize(childPath),
                    StringComparison.Ordinal),
                "project workspace activation should invoke the host callback with the normalized child path");

            foreach (var relativePath in supportedRelativePaths)
            {
                projectEntry.RelativePath = relativePath;
                requestedPath = null;
                var expectedPath = CopperfinDocumentPathIdentity.Normalize(
                    Path.Combine(root, relativePath.Replace('\\', Path.DirectorySeparatorChar)));
                Expect(
                    control.TryActivateSelectedProjectEntry() &&
                    string.Equals(requestedPath, expectedPath, StringComparison.Ordinal),
                    $"project workspace activation should open supported {Path.GetExtension(relativePath)} child assets");
            }

            requestedPath = null;
            snapshotObject.Deleted = true;
            Expect(
                !control.TryActivateSelectedProjectEntry() && requestedPath is null,
                "project workspace activation should reject deleted snapshot entries without opening a document");

            snapshotObject.Deleted = false;
            projectEntry.Excluded = true;
            Expect(
                !control.TryActivateSelectedProjectEntry() && requestedPath is null,
                "project workspace activation should reject excluded project entries without opening a document");

            SetPrivateField(
                control,
                "currentProjectInsights",
                new CopperfinProjectInsights
                {
                    TaskItems = new List<CopperfinProjectTaskItem>
                    {
                        new CopperfinProjectTaskItem
                        {
                            Category = "TODO",
                            FilePath = taskSourcePath,
                            Line = 2,
                            Message = "TODO: wire the startup command"
                        }
                    },
                    DefinedSymbols = new List<CopperfinProjectCodeSymbol>
                    {
                        new CopperfinProjectCodeSymbol
                        {
                            Kind = "procedure",
                            Name = "SaveOrder",
                            FilePath = taskSourcePath,
                            Line = 3,
                            Detail = "PROCEDURE SaveOrder"
                        }
                    },
                    RuntimeReferences = new List<CopperfinProjectCodeSymbol>
                    {
                        new CopperfinProjectCodeSymbol
                        {
                            Kind = "call",
                            Name = "SaveOrder",
                            FilePath = taskSourcePath,
                            Line = 5,
                            Detail = "? SaveOrder()"
                        }
                    },
                    DataAssets = new List<CopperfinProjectDataAsset>
                    {
                        new CopperfinProjectDataAsset
                        {
                            Kind = "Table",
                            Title = "customers",
                            FilePath = dataAssetPath,
                            GroupTitle = "Tables"
                        }
                    },
                    ObjectNodes = new List<CopperfinProjectObjectNode>
                    {
                        new CopperfinProjectObjectNode
                        {
                            Kind = "Form",
                            Title = "orders",
                            FilePath = childPath,
                            GroupTitle = "Forms",
                            Detail = "Form asset"
                        },
                        new CopperfinProjectObjectNode
                        {
                            Kind = "Project Header",
                            Title = "sample",
                            FilePath = projectPath,
                            GroupTitle = "Project",
                            Detail = "Project record"
                        }
                    }
                });
            SetPrivateField(
                control,
                "currentDebugSession",
                new CopperfinRuntimeDebugSession
                {
                    Success = true,
                    State = new CopperfinRuntimePauseState
                    {
                        ExecutedStatements = 3,
                        Events = new List<CopperfinRuntimeEvent>
                        {
                            new CopperfinRuntimeEvent
                            {
                                Category = "runtime.dispatch",
                                Detail = "first hit",
                                Location = taskSourcePath + ":2"
                            },
                            new CopperfinRuntimeEvent
                            {
                                Category = "runtime.dispatch",
                                Detail = "second hit",
                                Location = taskSourcePath + ":2"
                            },
                            new CopperfinRuntimeEvent
                            {
                                Category = "runtime.call",
                                Detail = "another line",
                                Location = taskSourcePath + ":3"
                            },
                            new CopperfinRuntimeEvent
                            {
                                Category = "runtime.info",
                                Detail = "no source location",
                                Location = "runtime-only"
                            }
                        }
                    }
                });
            typeof(CopperfinAssetEditorControl)
                .GetMethod("RefreshProjectWorkspaceInsightViews", BindingFlags.Instance | BindingFlags.NonPublic)
                ?.Invoke(control, null);
            var projectWorkspaceTabs = GetPrivateField<TabControl>(control, "projectWorkspaceTabs");
            if (projectWorkspaceTabs is not null)
            {
                projectWorkspaceTabs.Visible = true;
                projectWorkspaceTabs.SelectTab(2);
            }
            Application.DoEvents();

            if (!string.IsNullOrWhiteSpace(CopperfinStudioHostBridge.ResolveStudioHostPath()))
            {
                var buildersLoaded = WaitUntil(
                    TimeSpan.FromSeconds(12),
                    () => FindListViews(control).Any(list =>
                        list.Columns.Count == 4 &&
                        string.Equals(list.Columns[0].Text, "Kind", StringComparison.OrdinalIgnoreCase) &&
                        string.Equals(list.Columns[1].Text, "Builder", StringComparison.OrdinalIgnoreCase) &&
                        list.Items.Count >= 4));
                var builders = FindListViews(control)
                    .FirstOrDefault(list => list.Columns.Count == 4 &&
                                            string.Equals(list.Columns[0].Text, "Kind", StringComparison.OrdinalIgnoreCase) &&
                                            string.Equals(list.Columns[1].Text, "Builder", StringComparison.OrdinalIgnoreCase));
                Expect(buildersLoaded && builders is not null && builders.Items.Count >= 4,
                    "project workspace Builders should expose the native builder catalog across supported contexts");
                if (builders is not null)
                {
                    Expect(builders.Items.Cast<ListViewItem>().Any(item => item.SubItems[1].Text == "Form Builder"),
                        "project workspace Builders should preserve localized native builder titles");
                }
            }
            else
            {
                Console.WriteLine("SKIP: project workspace builder catalog host was not found.");
            }

            var taskList = FindListViews(control)
                .FirstOrDefault(list => list.Columns.Count >= 4 &&
                                        string.Equals(list.Columns[0].Text, "Category", StringComparison.OrdinalIgnoreCase));
            Expect(taskList is not null && taskList.Items.Count == 1,
                "project workspace Task List should show every discovered task with stable columns");
            if (taskList is not null && taskList.Items.Count == 1)
            {
                taskList.Items[0].Selected = true;
                taskList.Items[0].Focused = true;
                requestedTaskPath = null;
                requestedTaskLine = 0;
                var taskActivated = control.TryActivateSelectedTask();
                Expect(
                    taskActivated &&
                    string.Equals(requestedTaskPath, taskSourcePath, StringComparison.Ordinal) &&
                    requestedTaskLine == 2,
                    "project workspace Task List activation should open the source file at its one-based source line");
            }

            if (projectWorkspaceTabs is not null)
            {
                projectWorkspaceTabs.SelectTab(3);
            }
            Application.DoEvents();
            var codeReferences = FindListViews(control)
                .FirstOrDefault(list => list.Columns.Count >= 5 &&
                                        string.Equals(list.Columns[0].Text, "Kind", StringComparison.OrdinalIgnoreCase));
            Expect(codeReferences is not null && codeReferences.Items.Count == 2,
                "project workspace Code References should show every discovered definition and reference with stable columns");
            if (codeReferences is not null && codeReferences.Items.Count == 2)
            {
                codeReferences.Items[1].Selected = true;
                codeReferences.Items[1].Focused = true;
                requestedTaskPath = null;
                requestedTaskLine = 0;
                var referenceActivated = control.TryActivateSelectedCodeReference();
                Expect(
                    referenceActivated &&
                    string.Equals(requestedTaskPath, taskSourcePath, StringComparison.Ordinal) &&
                    requestedTaskLine == 5,
                    "project workspace Code References activation should open the selected source line");
            }

            if (projectWorkspaceTabs is not null)
            {
                projectWorkspaceTabs.SelectTab(4);
            }
            var dataFilter = GetPrivateField<TextBox>(control, "dataExplorerFilterBox");
            if (dataFilter is not null)
            {
                dataFilter.Text = "customer";
            }
            Application.DoEvents();
            var dataExplorer = FindListViews(control)
                .FirstOrDefault(list => list.Columns.Count == 4 &&
                                        string.Equals(list.Columns[0].Text, "Kind", StringComparison.OrdinalIgnoreCase) &&
                                        string.Equals(list.Columns[1].Text, "Title", StringComparison.OrdinalIgnoreCase));
            Expect(dataExplorer is not null && dataExplorer.Items.Count == 1,
                "project workspace Data Explorer should apply its filter to all matching assets with stable columns");
            if (dataExplorer is not null && dataExplorer.Items.Count == 1)
            {
                dataExplorer.Items[0].Selected = true;
                dataExplorer.Items[0].Focused = true;
                requestedPath = null;
                var dataActivated = control.TryActivateSelectedDataAsset();
                Expect(
                    dataActivated &&
                    string.Equals(requestedPath, dataAssetPath, StringComparison.Ordinal),
                    "project workspace Data Explorer activation should open the selected asset path");
            }

            if (projectWorkspaceTabs is not null)
            {
                projectWorkspaceTabs.SelectTab(5);
            }
            var objectBrowserFilter = GetPrivateField<TextBox>(control, "objectBrowserFilterBox");
            var hideProjectRecords = GetPrivateField<CheckBox>(control, "objectBrowserHideProjectCheckBox");
            if (objectBrowserFilter is not null)
            {
                objectBrowserFilter.Text = string.Empty;
            }
            if (hideProjectRecords is not null)
            {
                hideProjectRecords.Checked = true;
            }
            Application.DoEvents();
            var objectBrowser = FindListViews(control)
                .FirstOrDefault(list => list.Columns.Count == 4 &&
                                        string.Equals(list.Columns[0].Text, "Kind", StringComparison.OrdinalIgnoreCase) &&
                                        string.Equals(list.Columns[1].Text, "Title", StringComparison.OrdinalIgnoreCase) &&
                                        string.Equals(list.Columns[3].Text, "Detail", StringComparison.OrdinalIgnoreCase));
            Expect(objectBrowser is not null && objectBrowser.Items.Count == 1,
                "project workspace Object Browser should apply its hide-project-records option to all matching nodes");
            if (objectBrowser is not null && objectBrowser.Items.Count == 1)
            {
                objectBrowser.Items[0].Selected = true;
                objectBrowser.Items[0].Focused = true;
                requestedPath = null;
                var objectActivated = control.TryActivateSelectedObjectNode();
                Expect(
                    objectActivated &&
                    string.Equals(requestedPath, CopperfinDocumentPathIdentity.Normalize(childPath), StringComparison.Ordinal),
                    "project workspace Object Browser activation should open the selected source asset");
            }

            if (projectWorkspaceTabs is not null)
            {
                projectWorkspaceTabs.SelectTab(8);
            }
            Application.DoEvents();
            var coverage = FindListViews(control)
                .FirstOrDefault(list => list.Columns.Count == 4 &&
                                        string.Equals(list.Columns[0].Text, "Location", StringComparison.OrdinalIgnoreCase) &&
                                        string.Equals(list.Columns[1].Text, "Hits", StringComparison.OrdinalIgnoreCase));
            Expect(coverage is not null && coverage.Items.Count == 2,
                "project workspace Coverage should deduplicate source locations and retain stable columns");
            if (coverage is not null && coverage.Items.Count == 2)
            {
                Expect(coverage.Items[0].SubItems[1].Text == "2",
                    "project workspace Coverage should count repeated runtime events at one source location");
                coverage.Items[0].Selected = true;
                coverage.Items[0].Focused = true;
                requestedTaskPath = null;
                requestedTaskLine = 0;
                var coverageActivated = control.TryActivateSelectedCoverage();
                Expect(
                    coverageActivated &&
                    string.Equals(requestedTaskPath, taskSourcePath, StringComparison.Ordinal) &&
                    requestedTaskLine == 2,
                    "project workspace Coverage activation should open the selected source line");
            }
            hostForm.Close();
        }
        finally
        {
            try
            {
                if (Directory.Exists(root))
                {
                    Directory.Delete(root, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeProjectDebugReplayWithRealAsset(string? path)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real project debug replay asset candidate" : path)} not found.");
            return;
        }

        #pragma warning disable VSTHRD002
        var session = CopperfinRuntimeDebugClient.StartSessionAsync(path!)
            .GetAwaiter()
            .GetResult();
        #pragma warning restore VSTHRD002
        Expect(session.Success, $"project debug replay should succeed for {path}");
        if (!session.Success)
        {
            return;
        }

        Expect(!string.IsNullOrWhiteSpace(session.ManifestPath) && File.Exists(session.ManifestPath),
            $"project debug replay should preserve the runtime manifest path for {path}");
        Expect(session.BuildWarningCount == 0,
            $"project debug replay should inherit a warning-free build for {path}");
        Expect(session.BuildWarnings.Count == 0,
            $"project debug replay should not inherit build warning lines for {path}");
        if (!string.IsNullOrWhiteSpace(session.ManifestPath) && File.Exists(session.ManifestPath))
        {
            var manifestText = File.ReadAllText(session.ManifestPath);
            Expect(manifestText.IndexOf("warning=", StringComparison.OrdinalIgnoreCase) < 0,
                $"project debug replay manifest should remain warning-free for {path}");
            Expect(manifestText.IndexOf("asset=6|wzcommon/registry.vcx|", StringComparison.Ordinal) >= 0,
                $"project debug replay manifest should stage the shared class dependency for {path}");
        }
        Expect(!string.IsNullOrWhiteSpace(session.State.Reason),
            $"project debug replay should surface a pause reason for {path}");
        Expect(session.State.Frames.Count > 0,
            $"project debug replay should surface a call stack for {path}");
        Expect(session.State.Events.Count > 0,
            $"project debug replay should surface runtime events for {path}");
    }

    private static void SmokeStandaloneStudioDocumentIdentity()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin-designer-smoke",
            "standalone-document-identity",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        var firstPath = Path.Combine(root, "first.prg");
        var secondPath = Path.Combine(root, "second.prg");
        try
        {
            File.WriteAllText(firstPath, "RETURN");
            File.WriteAllText(secondPath, "RETURN");
            SmokeStandaloneStudioWithMultipleAssets(firstPath, secondPath);
        }
        finally
        {
            try
            {
                if (Directory.Exists(root))
                {
                    Directory.Delete(root, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeStandaloneStudioRevisitingDocumentPreservesSelectors()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin-designer-smoke",
            "standalone-selector-revisit",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        var assetPath = Path.Combine(root, "selector.scx");
        try
        {
            File.WriteAllText(assetPath, string.Empty);

            using var form = new StudioMainForm(
                shellLayoutStore: new InMemoryStudioShellLayoutStore())
            {
                Width = 1500,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            form.OpenDocument(assetPath);
            form.Show();
            Application.DoEvents();

            var normalizedPath = CopperfinDocumentPathIdentity.Normalize(assetPath);
            var tabControl = FindTabControls(form)
                .First(tab => tab.TabPages.Cast<TabPage>()
                    .Any(page => string.Equals(page.ToolTipText, normalizedPath, StringComparison.Ordinal)));
            var page = tabControl.TabPages.Cast<TabPage>()
                .Single(page => string.Equals(page.ToolTipText, normalizedPath, StringComparison.Ordinal));
            var editor = page.Controls.OfType<CopperfinAssetEditorControl>().Single();

            form.OpenDocument(assetPath, objectName: "targetControl");
            Application.DoEvents();
            Expect(tabControl.TabPages.Count == 1,
                "revisiting a standalone Studio document should not duplicate its tab");
            Expect(ReadPrivateStringField(editor, "currentStartupObjectName") == "targetControl",
                "revisiting a standalone Studio document should apply a requested object-name selector");

            form.OpenDocument(assetPath);
            Application.DoEvents();
            Expect(ReadPrivateStringField(editor, "currentStartupObjectName") == "targetControl",
                "an unselected standalone Studio revisit should preserve the current object-name selector");

            form.OpenDocument(assetPath, uniqueId: "target-id");
            Application.DoEvents();
            var startupObjectNameField = typeof(CopperfinAssetEditorControl)
                .GetField("currentStartupObjectName", BindingFlags.Instance | BindingFlags.NonPublic);
            Expect(startupObjectNameField?.GetValue(editor) is null,
                "a unique-id selector should replace the prior object-name selector");
            Expect(ReadPrivateStringField(editor, "currentStartupUniqueId") == "target-id",
                "revisiting a standalone Studio document should apply a requested unique-id selector");

            TearDownForm(form);
        }
        finally
        {
            try
            {
                if (Directory.Exists(root))
                {
                    Directory.Delete(root, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeStandaloneStudioWithMultipleAssets(string? firstPath, string? secondPath)
    {
        if (string.IsNullOrWhiteSpace(firstPath) ||
            string.IsNullOrWhiteSpace(secondPath) ||
            !File.Exists(firstPath) ||
            !File.Exists(secondPath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(firstPath) ? "real asset candidate" : firstPath)} or {(string.IsNullOrWhiteSpace(secondPath) ? "real asset candidate" : secondPath)} not found.");
            return;
        }

        using var form = new StudioMainForm(
            shellLayoutStore: new InMemoryStudioShellLayoutStore())
        {
            Width = 1500,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        form.OpenDocument(firstPath!);
        form.Show();
        Application.DoEvents();
        form.OpenDocument(secondPath!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(10),
            () => FindTabControls(form).Any(tab => tab.TabPages.Count >= 2) &&
                  FindTabControls(form).SelectMany(tab => tab.TabPages.Cast<TabPage>())
                      .Any(page => page.Text.Equals(Path.GetFileName(firstPath), StringComparison.OrdinalIgnoreCase)) &&
                  FindTabControls(form).SelectMany(tab => tab.TabPages.Cast<TabPage>())
                      .Any(page => page.Text.Equals(Path.GetFileName(secondPath), StringComparison.OrdinalIgnoreCase)));
        Expect(loaded, "standalone Studio should open multiple assets as separate tabs");

        var firstEditor = FindTabControls(form)
            .SelectMany(tab => tab.TabPages.Cast<TabPage>())
            .Where(page => page.Text.Equals(Path.GetFileName(firstPath), StringComparison.OrdinalIgnoreCase))
            .SelectMany(page => page.Controls.OfType<CopperfinAssetEditorControl>())
            .SingleOrDefault();
        var preHandleLoadFinished = WaitUntil(
            TimeSpan.FromSeconds(10),
            () => firstEditor?.SnapshotLoadFinished == true);
        Expect(preHandleLoadFinished,
            "standalone Studio should finish an asset snapshot that started before the form handle was created");

        var tabControl = FindTabControls(form)
            .FirstOrDefault(tab => tab.TabPages.Cast<TabPage>()
                .Any(page => page.Controls.OfType<CopperfinAssetEditorControl>().Any()));
        Expect(tabControl is not null, "standalone Studio should surface a document tab control");
        if (tabControl is not null)
        {
            foreach (var expectedPath in new[]
            {
                CopperfinDocumentPathIdentity.Normalize(firstPath!),
                CopperfinDocumentPathIdentity.Normalize(secondPath!)
            })
            {
                var matchingPage = tabControl.TabPages.Cast<TabPage>()
                    .SingleOrDefault(page => string.Equals(page.ToolTipText, expectedPath, StringComparison.Ordinal));
                Expect(matchingPage is not null,
                    $"standalone Studio should retain an exact tab path binding for {expectedPath}");
                var editor = matchingPage?.Controls.OfType<CopperfinAssetEditorControl>().SingleOrDefault();
                Expect(editor is not null,
                    $"standalone Studio should retain an asset editor for {expectedPath}");
                if (editor is not null)
                {
                    Expect(
                        string.Equals(ReadPrivateStringField(editor, "currentPath"), expectedPath, StringComparison.Ordinal),
                        $"standalone Studio commands should remain bound to the exact selected document path {expectedPath}");
                }
            }

            var beforeDuplicateOpen = tabControl.TabPages.Count;
            form.OpenDocument(firstPath!);
            Application.DoEvents();
            Expect(tabControl.TabPages.Count == beforeDuplicateOpen, "opening an already open asset should not duplicate tabs");
            Expect(tabControl.SelectedTab is not null, "standalone Studio should keep a selected tab");
            Expect(tabControl.SelectedTab?.Text == Path.GetFileName(firstPath) || tabControl.SelectedTab?.Text == Path.GetFileName(secondPath),
                "standalone Studio should keep a valid selected asset tab");
        }

        TearDownForm(form);
    }

    private static void SmokeFreshRunVfpSourceStartupPaths()
    {
        var vfpSourceZipPath = ResolveFirstExistingRealAssetPath(
            ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFPSOURCE_ZIP")),
            ExpandUserPath("~/Downloads/VFPSource.zip"));
        if (string.IsNullOrWhiteSpace(vfpSourceZipPath) || !File.Exists(vfpSourceZipPath))
        {
            Console.WriteLine("SKIP: fresh-run VFPSource startup-path smoke requires a VFPSource.zip archive.");
            return;
        }

        var extractionBaseRoot = GetArchiveExtractionBaseRoot(vfpSourceZipPath!);
        try
        {
            if (Directory.Exists(extractionBaseRoot))
            {
                Directory.Delete(extractionBaseRoot, recursive: true);
            }
        }
        catch (IOException)
        {
        }
        catch (UnauthorizedAccessException)
        {
        }

        var extractedVfpSourceRoot = EnumerateResolvedRealAssetRoots()
            .FirstOrDefault(root =>
                string.Equals(
                    root,
                    Path.Combine(extractionBaseRoot, "VFPSource"),
                    StringComparison.OrdinalIgnoreCase));
        Expect(!string.IsNullOrWhiteSpace(extractedVfpSourceRoot),
            "fresh-run VFPSource startup-path smoke should materialize an extracted VFPSource root during root enumeration");
        if (string.IsNullOrWhiteSpace(extractedVfpSourceRoot))
        {
            return;
        }

        var projectPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "tasklist/tasklist.PJX");
        var programPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "tasklist/main.prg");
        var formPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "Wizards/wzapp/template/Books/Forms/books.scx");
        var classLibraryPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "EnvMgr/envmgr.vcx");
        var menuPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "ReportBuilder/handler_context.mnx");
        var reportPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "Wizards/wzapp/template/Books/Reports/by_author.FRX");
        var labelPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "Wizards/wzreport/STYLES/STYLELBL.LBX");

        Expect(!string.IsNullOrWhiteSpace(projectPath) && !string.IsNullOrWhiteSpace(programPath) &&
               !string.IsNullOrWhiteSpace(formPath) && !string.IsNullOrWhiteSpace(classLibraryPath) &&
               !string.IsNullOrWhiteSpace(menuPath) && !string.IsNullOrWhiteSpace(reportPath) &&
               !string.IsNullOrWhiteSpace(labelPath),
            "fresh-run VFPSource startup-path smoke should resolve project, program, form, class-library, menu, report, and label assets from the extracted root");
        if (string.IsNullOrWhiteSpace(projectPath) || string.IsNullOrWhiteSpace(programPath) ||
            string.IsNullOrWhiteSpace(formPath) || string.IsNullOrWhiteSpace(classLibraryPath) ||
            string.IsNullOrWhiteSpace(menuPath) || string.IsNullOrWhiteSpace(reportPath) ||
            string.IsNullOrWhiteSpace(labelPath))
        {
            return;
        }

        var labelSidecarPath = Path.ChangeExtension(labelPath, ".LBT");
        Expect(File.Exists(labelSidecarPath),
            "fresh-run VFPSource startup-path smoke should preserve the label sidecar beside STYLELBL.LBX");

        SmokeProjectEditorWithRealAsset(
            projectPath,
            expectGroups: new[] { "Forms", "Programs", "Class Libraries", "Classes", "Other Assets" },
            expectedManifestAsset: "asset=5|tasklistui.vcx|");
        SmokeProgramEditorWithRealAsset(programPath);
        SmokeStandaloneStudioWithMultipleAssets(formPath, reportPath);
        SmokeVisualAssetEditorWithRealAsset(formPath, "form", "Visual form", "SCX/SCT assets", ".SCT");
        SmokeVisualAssetEditorWithRealAsset(classLibraryPath, "class_library", "Visual class library", "VCX/VCT assets", ".VCT");
        SmokeVisualAssetEditorWithRealAsset(menuPath, "menu", "Visual menu", "MNX/MNT assets", ".MNT");
        SmokeAssetEditorWithRealAsset(labelPath, expectSection: "Detail");
    }

    private static void SmokeConfiguredVfp9ZipAssetDiscovery()
    {
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeVfp9ZipDiscovery-" + Guid.NewGuid().ToString("N"));
        var zipPath = Path.Combine(tempRoot, "VFP9Samples.zip");
        var extractionBaseRoot = GetArchiveExtractionBaseRoot(zipPath);
        var previousVfp9Root = Environment.GetEnvironmentVariable("COPPERFIN_VFP9_ROOT");
        var previousVfp9Zip = Environment.GetEnvironmentVariable("COPPERFIN_VFP9_ZIP");

        Directory.CreateDirectory(tempRoot);

        try
        {
            if (Directory.Exists(extractionBaseRoot))
            {
                Directory.Delete(extractionBaseRoot, recursive: true);
            }

            using (var archive = ZipFile.Open(zipPath, ZipArchiveMode.Create))
            {
                CreateArchiveEntry(archive, "Samples/Solution/Reports/invoice.frx", "invoice");
                CreateArchiveEntry(archive, "Samples/Solution/Reports/cust.lbx", "cust");
                CreateArchiveEntry(archive, "Samples/Solution/solution.pjx", "solution");
                CreateArchiveEntry(archive, "Wizards/Template/Books/Forms/books.scx", "books");
            }

            Environment.SetEnvironmentVariable("COPPERFIN_VFP9_ROOT", null);
            Environment.SetEnvironmentVariable("COPPERFIN_VFP9_ZIP", zipPath);

            var resolvedInvoice = TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx");
            var resolvedLabel = TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx");
            var resolvedSolution = TryResolveVfp9InstallAsset(@"Samples\Solution\solution.pjx");
            var resolvedBooksForm = TryResolveVfp9InstallAsset(@"Wizards\Template\Books\Forms\books.scx");

            Expect(string.Equals(
                    resolvedInvoice,
                    Path.Combine(extractionBaseRoot, "Samples", "Solution", "Reports", "invoice.frx"),
                    StringComparison.OrdinalIgnoreCase),
                "configured VFP9 zip discovery should resolve invoice.frx from the extracted sample root");
            Expect(string.Equals(
                    resolvedLabel,
                    Path.Combine(extractionBaseRoot, "Samples", "Solution", "Reports", "cust.lbx"),
                    StringComparison.OrdinalIgnoreCase),
                "configured VFP9 zip discovery should resolve cust.lbx from the extracted sample root");
            Expect(string.Equals(
                    resolvedSolution,
                    Path.Combine(extractionBaseRoot, "Samples", "Solution", "solution.pjx"),
                    StringComparison.OrdinalIgnoreCase),
                "configured VFP9 zip discovery should resolve solution.pjx from the extracted sample root");
            Expect(string.Equals(
                    resolvedBooksForm,
                    Path.Combine(extractionBaseRoot, "Wizards", "Template", "Books", "Forms", "books.scx"),
                    StringComparison.OrdinalIgnoreCase),
                "configured VFP9 zip discovery should resolve books.scx from the extracted sample root");

            var enumeratedRoots = EnumerateResolvedRealAssetRoots().ToList();
            Expect(enumeratedRoots.Any(root => string.Equals(root, extractionBaseRoot, StringComparison.OrdinalIgnoreCase)),
                "configured VFP9 zip discovery should surface the extracted sample root during real-asset root enumeration");

            var enumeratedReportAssets = EnumerateResolvedRealReportAssetPaths().ToList();
            Expect(enumeratedReportAssets.Any(path => string.Equals(path, resolvedInvoice, StringComparison.OrdinalIgnoreCase)),
                "configured VFP9 zip discovery should surface invoice.frx during report/label asset enumeration");
            Expect(enumeratedReportAssets.Any(path => string.Equals(path, resolvedLabel, StringComparison.OrdinalIgnoreCase)),
                "configured VFP9 zip discovery should surface cust.lbx during report/label asset enumeration");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_VFP9_ROOT", previousVfp9Root);
            Environment.SetEnvironmentVariable("COPPERFIN_VFP9_ZIP", previousVfp9Zip);

            try
            {
                if (Directory.Exists(extractionBaseRoot))
                {
                    Directory.Delete(extractionBaseRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void TearDownForm(Form form)
    {
        if (form.IsDisposed)
        {
            return;
        }

        form.Hide();
        Application.DoEvents();
        Thread.Sleep(150);
        Application.DoEvents();
        form.Close();
        Application.DoEvents();
    }

}
