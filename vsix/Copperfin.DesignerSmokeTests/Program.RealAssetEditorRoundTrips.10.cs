
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
    private static void SmokeAssetEditorDeletedDuplicateCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedOriginalUniqueId,
        string expectedDuplicatedUniqueId)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor deleted duplicate candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletedDuplicates-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var previousDuplicateUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID");

        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", expectedDuplicatedUniqueId);

            var initialSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(initialSnapshot.Success && initialSnapshot.Document is not null,
                $"real asset editor deleted-duplicate smoke should load initial snapshot data for {sourcePath}");
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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real deleted-duplicate smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor deleted-duplicate smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor deleted-duplicate smoke should surface object {recordIndex} for {sourcePath}");
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
                   objectListView.Items.Count == 1 &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor deleted-duplicate smoke should start from a live object selection with rename, duplicate, and delete exposed for {sourcePath}");

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
                           objectListView.Items.Count == 1 &&
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
                $"real asset editor deleted-duplicate smoke should preserve deleted selection continuity before duplicating for {sourcePath}");
            if (!deletedSelection)
            {
                return;
            }

            var snapshotAfterDelete = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(snapshotAfterDelete.Success && snapshotAfterDelete.Document is not null,
                $"real asset editor deleted-duplicate smoke should load deleted on-disk state before duplicating for {sourcePath}");
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
                    expectedVisibleSectionObjectCount: 1,
                    "updated real asset editor deleted-duplicate snapshot before duplicating should preserve original identity");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    snapshotAfterDelete.Document,
                    "updated real asset editor deleted-duplicate snapshot before duplicating should preserve deleted preview metadata");
            }

            duplicateButton.PerformClick();
            Application.DoEvents();

            int duplicatedRecordIndex = -1;
            var duplicatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex == recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    var selectedUniqueId = selectedObject is null
                        ? null
                        : TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID");
                    if (string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                        selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                        selectedSectionModel?.DeletedObjectCount == 2 &&
                        selectedObject?.RecordIndex == refreshedSelection.RecordIndex &&
                        selectedObject.Deleted &&
                        string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                        string.Equals(selectedUniqueId, expectedDuplicatedUniqueId, StringComparison.Ordinal) &&
                        string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                        objectListView.Items.Count == 2 &&
                        renameButton.Visible &&
                        renameButton.Enabled &&
                        duplicateButton.Visible &&
                        duplicateButton.Enabled &&
                        !deleteButton.Visible &&
                        restoreButton.Visible &&
                        restoreButton.Enabled &&
                        string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") == refreshedSelection.RecordIndex &&
                        ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                        !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))
                    {
                        duplicatedRecordIndex = refreshedSelection.RecordIndex;
                        return true;
                    }

                    return false;
                });
            Expect(duplicatedSelection,
                $"real asset editor deleted-duplicate smoke should preserve deleted section/object continuity after duplicating for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-duplicate smoke should retain the earlier delete-state undo after duplicating a deleted row for {sourcePath}");
            if (!duplicatedSelection)
            {
                return;
            }

            var updatedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(updatedAfterDuplicate.Success && updatedAfterDuplicate.Document is not null,
                $"real asset editor deleted-duplicate smoke should load duplicated deleted on-disk state before reload verification for {sourcePath}");
            if (updatedAfterDuplicate.Success && updatedAfterDuplicate.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    updatedAfterDuplicate.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    expectedDeletedSectionObjectCount: 2,
                    "updated real asset editor deleted-duplicate snapshot should preserve original deleted identity");
                AssertRealAssetDeletedObjectSnapshot(
                    updatedAfterDuplicate.Document,
                    duplicatedRecordIndex,
                    expectedDuplicatedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    expectedDeletedSectionObjectCount: 2,
                    "updated real asset editor deleted-duplicate snapshot should preserve duplicated deleted identity");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    updatedAfterDuplicate.Document,
                    "updated real asset editor deleted-duplicate snapshot should preserve deleted preview metadata");
            }

            var reloadedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null,
                $"real asset editor deleted-duplicate smoke should reload duplicated deleted on-disk state for {sourcePath}");
            if (reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterDuplicate.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    expectedDeletedSectionObjectCount: 2,
                    "reloaded real asset editor deleted-duplicate snapshot should preserve original deleted identity");
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterDuplicate.Document,
                    duplicatedRecordIndex,
                    expectedDuplicatedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    expectedDeletedSectionObjectCount: 2,
                    "reloaded real asset editor deleted-duplicate snapshot should preserve duplicated deleted identity");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    reloadedAfterDuplicate.Document,
                    "reloaded real asset editor deleted-duplicate snapshot should preserve deleted preview metadata");
            }

            restoreButton.PerformClick();
            Application.DoEvents();

            var restoredSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != duplicatedRecordIndex)
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
                           selectedObject?.RecordIndex == duplicatedRecordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedDuplicatedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           objectListView.Items.Count == 2 &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == duplicatedRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(restoredSelection,
                $"real asset editor deleted-duplicate smoke should preserve live continuity after restoring the duplicated row for {sourcePath}");
            if (!restoredSelection)
            {
                return;
            }

            var updatedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(updatedAfterRestore.Success && updatedAfterRestore.Document is not null,
                $"real asset editor deleted-duplicate smoke should load restored live state before reload verification for {sourcePath}");
            if (updatedAfterRestore.Success && updatedAfterRestore.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    updatedAfterRestore.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    "updated restored real asset editor deleted-duplicate snapshot should preserve the original deleted source row");
                AssertRealAssetRoundTripSnapshot(
                    updatedAfterRestore.Document,
                    duplicatedRecordIndex,
                    "UNIQUEID",
                    expectedDuplicatedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "updated restored real asset editor deleted-duplicate snapshot should preserve duplicated live identity");
                AssertRealAssetPreviewBoundsGeometry(
                    updatedAfterRestore.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    "updated restored real asset editor deleted-duplicate snapshot should preserve live preview metadata");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    updatedAfterRestore.Document,
                    "updated restored real asset editor deleted-duplicate snapshot should preserve remaining deleted preview metadata");
            }

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real asset editor deleted-duplicate smoke should reload restored live state for {sourcePath}");
            if (reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterRestore.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    "reloaded restored real asset editor deleted-duplicate snapshot should preserve the original deleted source row");
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterRestore.Document,
                    duplicatedRecordIndex,
                    "UNIQUEID",
                    expectedDuplicatedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded restored real asset editor deleted-duplicate snapshot should preserve duplicated live identity");
                AssertRealAssetPreviewBoundsGeometry(
                    reloadedAfterRestore.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    "reloaded restored real asset editor deleted-duplicate snapshot should preserve live preview metadata");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    reloadedAfterRestore.Document,
                    "reloaded restored real asset editor deleted-duplicate snapshot should preserve remaining deleted preview metadata");
            }

            TearDownForm(hostForm);
        }
        finally
        {
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

    private static void SmokeRealAssetHostBackedReorderRoundTrip(
        string? sourcePath,
        int reorderedSourceRecordIndex,
        string reorderedSourceUniqueId,
        string reorderedSourceObjectTitle,
        int companionRecordIndex,
        string companionUniqueId,
        string companionObjectTitle,
        string expectedSectionTitle,
        int initialSectionRecordIndex,
        int reorderedSectionRecordIndex,
        int expectedSectionCount,
        int expectedSectionObjectCount,
        IReadOnlyList<int> expectedInitialSectionRecordOrder,
        IReadOnlyList<int> expectedReorderedSectionRecordOrder,
        int expectedReorderedRecordIndex,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real live reorder candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetReorders-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real live reorder smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial real live reorder snapshot should preserve live preview bounds for {sourcePath}");
            if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                reorderedSourceRecordIndex,
                "UNIQUEID",
                reorderedSourceUniqueId,
                reorderedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"initial real live reorder snapshot should preserve the reordered-source identity");
            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                companionRecordIndex,
                "UNIQUEID",
                companionUniqueId,
                companionObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"initial real live reorder snapshot should preserve the companion identity");
            AssertRealAssetSectionObjectCount(
                loaded.Document,
                expectedSectionTitle,
                expectedSectionObjectCount,
                $"initial real live reorder snapshot should preserve section object counts");
            AssertRealAssetSectionRecordOrder(
                loaded.Document,
                expectedSectionTitle,
                initialSectionRecordIndex,
                expectedInitialSectionRecordOrder,
                $"initial real live reorder snapshot should preserve section record order");
            var initialLayout = loaded.Document.ReportLayout;
            var expectedPreviewBoundsLeft = initialLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = initialLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = initialLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = initialLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = initialLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = initialLayout.PreviewBoundsHeight;
            AssertRealAssetPreviewBoundsGeometry(
                loaded.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"initial real live reorder snapshot should preserve live preview metadata");

            var reorderResult = CopperfinStudioSnapshotClient.TryReorderObject(
                assetPath,
                reorderedSourceRecordIndex,
                reorderedSourceUniqueId,
                "front");
            Expect(reorderResult.Success && reorderResult.Document is not null,
                $"real live reorder smoke should reorder record {reorderedSourceRecordIndex} for {sourcePath}");
            if (!reorderResult.Success || reorderResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reorderResult.Document,
                expectedReorderedRecordIndex,
                "UNIQUEID",
                reorderedSourceUniqueId,
                reorderedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reordered real live asset snapshot should preserve the reordered-source identity");
            AssertRealAssetRoundTripSnapshot(
                reorderResult.Document,
                companionRecordIndex,
                "UNIQUEID",
                companionUniqueId,
                companionObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reordered real live asset snapshot should preserve the companion identity");
            AssertRealAssetSectionObjectCount(
                reorderResult.Document,
                expectedSectionTitle,
                expectedSectionObjectCount,
                $"reordered real live asset snapshot should preserve section object counts");
            AssertRealAssetSectionRecordOrder(
                reorderResult.Document,
                expectedSectionTitle,
                reorderedSectionRecordIndex,
                expectedReorderedSectionRecordOrder,
                $"reordered real live asset snapshot should preserve section record order");
            AssertRealAssetPreviewBoundsGeometry(
                reorderResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reordered real live asset snapshot should preserve live preview metadata");
            Expect(!reorderResult.Document.CommandUndoAvailable,
                $"real live reorder smoke should not expose command undo after reordering {sourcePath}");

            var reloadedAfterReorder = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterReorder.Success && reloadedAfterReorder.Document is not null,
                $"real live reorder smoke should reload reordered snapshot data for {sourcePath}");
            if (!reloadedAfterReorder.Success || reloadedAfterReorder.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterReorder.Document,
                expectedReorderedRecordIndex,
                "UNIQUEID",
                reorderedSourceUniqueId,
                reorderedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded reordered real live asset snapshot should preserve the reordered-source identity");
            AssertRealAssetRoundTripSnapshot(
                reloadedAfterReorder.Document,
                companionRecordIndex,
                "UNIQUEID",
                companionUniqueId,
                companionObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded reordered real live asset snapshot should preserve the companion identity");
            AssertRealAssetSectionObjectCount(
                reloadedAfterReorder.Document,
                expectedSectionTitle,
                expectedSectionObjectCount,
                $"reloaded reordered real live asset snapshot should preserve section object counts");
            AssertRealAssetSectionRecordOrder(
                reloadedAfterReorder.Document,
                expectedSectionTitle,
                reorderedSectionRecordIndex,
                expectedReorderedSectionRecordOrder,
                $"reloaded reordered real live asset snapshot should preserve section record order");
            AssertRealAssetPreviewBoundsGeometry(
                reloadedAfterReorder.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded reordered real live asset snapshot should preserve live preview metadata");
            Expect(!reloadedAfterReorder.Document.CommandUndoAvailable,
                $"reloaded real live reorder snapshot should not expose command undo for {sourcePath}");
        }
        finally
        {
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
