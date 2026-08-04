
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
    private static void SmokeAssetEditorDeletedRenameCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedOriginalUniqueId,
        string expectedRenamedUniqueId,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor deleted rename candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletedRenames-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var previousRenameUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID");

        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", expectedRenamedUniqueId);

            var initialSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(initialSnapshot.Success && initialSnapshot.Document is not null,
                $"real asset editor deleted-rename smoke should load initial snapshot data for {sourcePath}");
            if (!initialSnapshot.Success || initialSnapshot.Document is null || initialSnapshot.Document.ReportLayout is null)
            {
                return;
            }

            var expectedPreviewBoundsLeft = initialSnapshot.Document.ReportLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = initialSnapshot.Document.ReportLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = initialSnapshot.Document.ReportLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = initialSnapshot.Document.ReportLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = initialSnapshot.Document.ReportLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = initialSnapshot.Document.ReportLayout.PreviewBoundsHeight;

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
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var renameButton = GetPrivateButton(control, "renameObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real deleted-rename smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor deleted-rename smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == expectedSectionRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor deleted-rename smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor deleted-rename smoke should start from a live object selection with rename, duplicate, and delete exposed for {sourcePath}");

            deleteButton.PerformClick();
            Application.DoEvents();

            var deletedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedOriginalUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(deletedSelection,
                $"real asset editor deleted-rename smoke should preserve deleted selection continuity before renaming for {sourcePath}");
            if (!deletedSelection)
            {
                return;
            }

            var snapshotAfterDelete = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(snapshotAfterDelete.Success && snapshotAfterDelete.Document is not null,
                $"real asset editor deleted-rename smoke should load deleted on-disk state before renaming for {sourcePath}");
            if (snapshotAfterDelete.Success && snapshotAfterDelete.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    snapshotAfterDelete.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    "updated real asset editor deleted-rename snapshot before renaming should preserve original identity");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    snapshotAfterDelete.Document,
                    "updated real asset editor deleted-rename snapshot before renaming should preserve deleted preview metadata");
            }

            renameButton.PerformClick();
            Application.DoEvents();

            var renamedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedRenamedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(renamedSelection,
                $"real asset editor deleted-rename smoke should preserve deleted section/object continuity after renaming for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-rename smoke should expose undo after renaming for {sourcePath}");
            if (!renamedSelection)
            {
                return;
            }

            var updatedAfterRename = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(updatedAfterRename.Success && updatedAfterRename.Document is not null,
                $"real asset editor deleted-rename smoke should load renamed deleted on-disk state before reload verification for {sourcePath}");
            if (updatedAfterRename.Success && updatedAfterRename.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    updatedAfterRename.Document,
                    recordIndex,
                    expectedRenamedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    "updated real asset editor deleted-rename snapshot should preserve renamed identity");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    updatedAfterRename.Document,
                    "updated real asset editor deleted-rename snapshot should preserve deleted preview metadata");
            }

            var reloadedAfterRename = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRename.Success && reloadedAfterRename.Document is not null,
                $"real asset editor deleted-rename smoke should reload renamed deleted on-disk state for {sourcePath}");
            if (reloadedAfterRename.Success && reloadedAfterRename.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterRename.Document,
                    recordIndex,
                    expectedRenamedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    "reloaded real asset editor deleted-rename snapshot should preserve renamed identity");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    reloadedAfterRename.Document,
                    "reloaded real asset editor deleted-rename snapshot should preserve deleted preview metadata");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor deleted-rename smoke should execute undo after renaming for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedOriginalUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor deleted-rename smoke should preserve deleted section/object continuity after undoing rename for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-rename smoke should retain the earlier delete-state undo after restoring original identity for {sourcePath}");
            if (!undoneSelection)
            {
                return;
            }

            var updatedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(updatedAfterUndo.Success && updatedAfterUndo.Document is not null,
                $"real asset editor deleted-rename smoke should load restored deleted identity before reload verification for {sourcePath}");
            if (updatedAfterUndo.Success && updatedAfterUndo.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    updatedAfterUndo.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    "updated undone real asset editor deleted-rename snapshot should preserve original identity");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    updatedAfterUndo.Document,
                    "updated undone real asset editor deleted-rename snapshot should preserve deleted preview metadata");
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor deleted-rename smoke should reload restored deleted identity for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    "reloaded undone real asset editor deleted-rename snapshot should preserve original identity");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    reloadedAfterUndo.Document,
                    "reloaded undone real asset editor deleted-rename snapshot should preserve deleted preview metadata");
            }

            restoreButton.PerformClick();
            Application.DoEvents();

            var restoredSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 0 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedOriginalUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           objectListView.Items.Count == 1 &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(restoredSelection,
                $"real asset editor deleted-rename smoke should preserve live continuity after restoring the deleted row for {sourcePath}");
            if (!restoredSelection)
            {
                return;
            }

            var updatedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(updatedAfterRestore.Success && updatedAfterRestore.Document is not null,
                $"real asset editor deleted-rename smoke should load restored live state before reload verification for {sourcePath}");
            if (updatedAfterRestore.Success && updatedAfterRestore.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    updatedAfterRestore.Document,
                    recordIndex,
                    "UNIQUEID",
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "updated restored real asset editor deleted-rename snapshot should preserve original identity");
                AssertRealAssetLivePreviewBounds(
                    updatedAfterRestore.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    "updated restored real asset editor deleted-rename snapshot should preserve live preview metadata");
            }

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real asset editor deleted-rename smoke should reload restored live state for {sourcePath}");
            if (reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterRestore.Document,
                    recordIndex,
                    "UNIQUEID",
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded restored real asset editor deleted-rename snapshot should preserve original identity");
                AssertRealAssetLivePreviewBounds(
                    reloadedAfterRestore.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    "reloaded restored real asset editor deleted-rename snapshot should preserve live preview metadata");
            }

            TearDownForm(hostForm);
        }
        finally
        {
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

    private static void SmokeAssetEditorRenameCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedOriginalUniqueId,
        string expectedRenamedUniqueId)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor rename candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorRenames-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var previousRenameUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID");

        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", expectedRenamedUniqueId);

            var initialSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(initialSnapshot.Success && initialSnapshot.Document is not null,
                $"real asset editor rename smoke should load initial snapshot data for {sourcePath}");
            if (!initialSnapshot.Success || initialSnapshot.Document is null || initialSnapshot.Document.ReportLayout is null)
            {
                return;
            }

            var expectedPreviewBoundsLeft = initialSnapshot.Document.ReportLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = initialSnapshot.Document.ReportLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = initialSnapshot.Document.ReportLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = initialSnapshot.Document.ReportLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = initialSnapshot.Document.ReportLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = initialSnapshot.Document.ReportLayout.PreviewBoundsHeight;

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
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var renameButton = GetPrivateButton(control, "renameObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real rename smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor rename smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == expectedSectionRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor rename smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor rename smoke should start from a live object selection with rename, duplicate, and delete exposed for {sourcePath}");

            renameButton.PerformClick();
            Application.DoEvents();

            var renamedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedRenamedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(renamedSelection,
                $"real asset editor rename smoke should preserve live selection continuity after renaming for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor rename smoke should expose undo after renaming for {sourcePath}");
            if (!renamedSelection)
            {
                return;
            }

            var reloadedAfterRename = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRename.Success && reloadedAfterRename.Document is not null,
                $"real asset editor rename smoke should reload renamed on-disk state for {sourcePath}");
            if (reloadedAfterRename.Success && reloadedAfterRename.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterRename.Document,
                    recordIndex,
                    "UNIQUEID",
                    expectedRenamedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded real asset editor rename snapshot should preserve renamed identity");
                AssertRealAssetPreviewBoundsGeometry(
                    reloadedAfterRename.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"reloaded real asset editor rename snapshot should preserve live preview metadata");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor rename smoke should execute undo after renaming for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedOriginalUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor rename smoke should preserve live selection continuity after undoing rename for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor rename smoke should clear undo after restoring the original identity for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor rename smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    "UNIQUEID",
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded undone real asset editor rename snapshot should preserve original identity");
                AssertRealAssetPreviewBoundsGeometry(
                    reloadedAfterUndo.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"reloaded undone real asset editor rename snapshot should preserve live preview metadata");
            }

            TearDownForm(hostForm);
        }
        finally
        {
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

}
