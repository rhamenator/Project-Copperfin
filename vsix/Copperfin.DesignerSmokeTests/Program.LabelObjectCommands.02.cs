
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
    private static void SmokeAssetEditorUndoRefreshesDeletedLabelShellSummary()
    {
        var snapshot = BuildAssetEditorDeletedLabelUndoPreviewRefreshSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedLabelUndoPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

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

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetCurrentSnapshot(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13 &&
                   control.CanHandleUndoCommand() &&
                   string.Equals(control.GetUndoCommandText(), "Undo Move deleted.footer.total", StringComparison.Ordinal),
                "A deleted label undo summary-refresh smoke should start from an undo-capable deleted label object selection");

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A deleted label undo summary-refresh smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            Expect(control.TryHandleUndoCommand(),
                "The shared asset editor should accept an undo command for a deleted label selection when the snapshot exposes a host-backed undo label");
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Undoing through the shared asset editor for a deleted label selection should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--undo-mode") &&
                   invocationArguments.Contains("command") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Undoing through the shared asset editor for a deleted label selection should send one invariant undo command through the host contract");

            var deletedSection = snapshot.ReportLayout?.DeletedSections[0]
                ?? throw new InvalidOperationException("Could not read the deleted label section from the shared smoke snapshot.");
            var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1400 T 9400 R 5000 B 10000   Size: 3600 x 600") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1600", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Undoing through the shared asset editor for a deleted label selection should refresh the shell summary from the returned snapshot while preserving label identity, deleted-object selection, and deleted-section continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

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

    private static void SmokeAssetEditorDuplicateObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorDuplicateLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        const string duplicateUniqueId = "middle-copy-guid";
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");
        var previousDuplicateUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDuplicateLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", duplicateUniqueId);

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

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7,
                "A label duplicate-object smoke should start from a live object selection with duplicate and delete commands exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label duplicate-object smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            duplicateButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Duplicating a label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--duplicate-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--new-unique-id") &&
                   invocationArguments.Contains(duplicateUniqueId) &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Duplicating a label object through the shared asset editor should send one invariant duplicate-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1200 T 2600 R 6200 B 3200   Size: 5000 x 600") &&
                   HasLabelTextContaining(control, "Duplicated object. Snapshot loaded: 2 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.SubItems[2].Text, "10", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 10 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Live", StringComparison.Ordinal) &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 10 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Duplicating a label object through the shared asset editor should refresh the shell summary and move shared selection continuity to the duplicated label object");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", previousDuplicateUniqueId);

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

    private static void SmokeAssetEditorRenameObjectCommandRefreshesDeletedLabelShellSummary()
    {
        var snapshot = BuildAssetEditorRenameDeletedLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        const string renamedUniqueId = "deleted-footer-renamed-guid";
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");
        var previousRenameUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildRenameDeletedLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", renamedUniqueId);

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

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var renameButton = GetPrivateButton(control, "renameObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   !deleteButton.Visible &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13,
                "A deleted label rename-object smoke should start from a deleted object selection with rename, duplicate, and restore commands exposed");

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A deleted label rename-object smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            renameButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Renaming a deleted label object identity through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--rename-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("13") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("deleted-footer-guid") &&
                   invocationArguments.Contains("--new-unique-id") &&
                   invocationArguments.Contains(renamedUniqueId) &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Renaming a deleted label object identity through the shared asset editor should send one invariant rename-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1600 T 9400 R 5200 B 10000   Size: 3600 x 600") &&
                   HasLabelTextContaining(control, "Regenerated object id. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", snapshot.ReportLayout!.DeletedSections[0]), StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   !deleteButton.Visible &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Renaming a deleted label object identity through the shared asset editor should refresh the shell summary and preserve deleted label selection continuity on the renamed row");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", previousRenameUniqueId);

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

    private static void SmokeAssetEditorRestoreObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorRestoreLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildRestoreLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

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

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(!deleteButton.Visible &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13,
                "A label restore-object smoke should start from a deleted object selection with only the restore command exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label restore-object smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            restoreButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Restoring a label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--restore-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("13") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("deleted-footer-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Restoring a label object through the shared asset editor should send one invariant restore-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1700 T 2800 R 5300 B 3400   Size: 3600 x 600") &&
                   HasLabelTextContaining(control, "Restored object. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Live", StringComparison.Ordinal) &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   restoreButton.Visible == false &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Restoring a label object through the shared asset editor should refresh the shell summary, preserve record identity, and flip the shared command surface back to delete");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

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

    private static void SmokeAssetEditorDeleteObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorDeleteLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeleteLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

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

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A label delete-object smoke should start from a live object selection with only the delete command exposed");

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A label delete-object smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            deleteButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Deleting a label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--delete-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("live-detail-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Deleting a label object through the shared asset editor should send one invariant delete-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1500 T 2600 R 5500 B 3100   Size: 4000 x 500") &&
                   HasLabelTextContaining(control, "Deleted object. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail (deleted)", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   deleteButton.Visible == false &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 52 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Deleting a label object through the shared asset editor should refresh the shell summary, preserve record identity, and flip the shared command surface to restore");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

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

    private static void SmokeAssetEditorReorderBackObjectCommandRefreshesLabelShellSummary()
    {
        var snapshot = BuildAssetEditorReorderBackLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = CreateFakeStudioHostScriptPath(tempRoot);
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildReorderBackLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

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

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value", "last.value" }) &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A label reorder-back smoke should start from a live object selection with shared duplicate, reorder, and delete commands exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label reorder-back smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            reorderBackButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Reordering a label object to the back through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--reorder-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("--placement") &&
                   invocationArguments.Contains("back") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Reordering a label object to the back through the shared asset editor should send one invariant reorder-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1400 T 2600 R 6800 B 3200   Size: 5400 x 600") &&
                   HasLabelTextContaining(control, "Moved object to back. Snapshot loaded: 3 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "middle.value", "last.value", "first.value" }) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "first.value", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Reordering a label object to the back through the shared asset editor should refresh the shell summary, reorder visible rows, and preserve label selection continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

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

}
