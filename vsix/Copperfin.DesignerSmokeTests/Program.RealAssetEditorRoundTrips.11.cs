
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
    private static void SmokeAssetEditorDeletedReorderCommandWithRealAsset(
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
        int expectedVisibleSectionObjectCount,
        int expectedReorderedRecordIndex,
        int expectedCompanionRecordIndex,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor deleted reorder candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletedReorders-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var initialSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(initialSnapshot.Success && initialSnapshot.Document is not null,
                $"real asset editor deleted-reorder smoke should load initial snapshot data for {sourcePath}");
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
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real deleted-reorder smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor deleted-reorder smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == initialSectionRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectsLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>().Any(item =>
                          item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                          (snapshotObject.RecordIndex == reorderedSourceRecordIndex || snapshotObject.RecordIndex == companionRecordIndex)));
            Expect(objectsLoaded, $"real asset editor deleted-reorder smoke should surface source objects for {sourcePath}");
            if (!objectsLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == reorderedSourceRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   objectListView.Items.Count == expectedVisibleSectionObjectCount &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == reorderedSourceRecordIndex,
                $"real asset editor deleted-reorder smoke should start from a live object selection with reorder commands exposed for {sourcePath}");

            deleteButton.PerformClick();
            Application.DoEvents();

            var firstDeletedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != reorderedSourceRecordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == initialSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == reorderedSourceRecordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), reorderedSourceUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == reorderedSourceRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == initialSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(firstDeletedSelection,
                $"real asset editor deleted-reorder smoke should preserve deleted selection continuity after the first delete for {sourcePath}");
            if (!firstDeletedSelection)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == companionRecordIndex &&
                                !snapshotObject.Deleted;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();
            deleteButton.PerformClick();
            Application.DoEvents();

            var secondDeletedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != companionRecordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == initialSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 2 &&
                           selectedObject?.RecordIndex == companionRecordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), companionUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           reorderFrontButton.Visible &&
                           reorderFrontButton.Enabled &&
                           reorderBackButton.Visible &&
                           reorderBackButton.Enabled &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == companionRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == initialSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(secondDeletedSelection,
                $"real asset editor deleted-reorder smoke should preserve deleted selection continuity before reordering for {sourcePath}");
            if (!secondDeletedSelection)
            {
                return;
            }

            var snapshotAfterDeletes = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(snapshotAfterDeletes.Success && snapshotAfterDeletes.Document is not null,
                $"real asset editor deleted-reorder smoke should reload deleted on-disk state before reordering for {sourcePath}");
            if (snapshotAfterDeletes.Success && snapshotAfterDeletes.Document is not null)
            {
                AssertRealAssetPreviewBoundsGeometry(
                    snapshotAfterDeletes.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    "reloaded real asset editor deleted-reorder snapshot before reordering should preserve live preview metadata");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    snapshotAfterDeletes.Document,
                    "reloaded real asset editor deleted-reorder snapshot before reordering should preserve deleted preview metadata");
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == reorderedSourceRecordIndex &&
                                snapshotObject.Deleted;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();
            reorderFrontButton.PerformClick();
            Application.DoEvents();

            var reorderedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    return string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == expectedReorderedRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == reorderedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(reorderedSelection,
                $"real asset editor deleted-reorder smoke should preserve deleted section/object continuity after reordering for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-reorder smoke should retain the earlier delete-state undo after reordering a deleted row for {sourcePath}");
            if (!reorderedSelection)
            {
                return;
            }

            var reloadedAfterReorder = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterReorder.Success && reloadedAfterReorder.Document is not null,
                $"real asset editor deleted-reorder smoke should reload reordered deleted on-disk state for {sourcePath}");
            if (reloadedAfterReorder.Success && reloadedAfterReorder.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterReorder.Document,
                    expectedReorderedRecordIndex,
                    reorderedSourceUniqueId,
                    reorderedSourceObjectTitle,
                    expectedSectionTitle,
                    reorderedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount,
                    expectedDeletedSectionObjectCount: 2,
                    "reloaded real asset editor deleted-reorder snapshot should preserve the reordered-source identity",
                    assertTitles: false);
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterReorder.Document,
                    expectedCompanionRecordIndex,
                    companionUniqueId,
                    companionObjectTitle,
                    expectedSectionTitle,
                    reorderedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount,
                    expectedDeletedSectionObjectCount: 2,
                    "reloaded real asset editor deleted-reorder snapshot should preserve the companion identity");
                AssertRealAssetDeletedObjectOrder(
                    reloadedAfterReorder.Document,
                    reorderedSectionRecordIndex,
                    new[] { expectedReorderedRecordIndex, expectedCompanionRecordIndex },
                    new[] { reorderedSourceUniqueId, companionUniqueId },
                    "reloaded real asset editor deleted-reorder snapshot should preserve deleted-row order");
                AssertRealAssetPreviewBoundsGeometry(
                    reloadedAfterReorder.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    "reloaded real asset editor deleted-reorder snapshot should preserve live preview metadata");
                AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                    reloadedAfterReorder.Document,
                    "reloaded real asset editor deleted-reorder snapshot should preserve deleted preview metadata");
            }
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

    private static void SmokeAssetEditorReorderCommandWithRealAsset(
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
        IReadOnlyList<int> expectedReorderedSectionRecordOrder,
        int expectedReorderedRecordIndex,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor live reorder candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorReorders-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var initialSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(initialSnapshot.Success && initialSnapshot.Document is not null,
                $"real asset editor live-reorder smoke should load initial snapshot data for {sourcePath}");
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
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real live-reorder smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor live-reorder smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == initialSectionRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == reorderedSourceRecordIndex));
            Expect(objectLoaded, $"real asset editor live-reorder smoke should surface object {reorderedSourceRecordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == reorderedSourceRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            var initialSectionModel = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioReportSection;
            var initialObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
            Expect(reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   objectListView.Items.Count == expectedSectionObjectCount &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == reorderedSourceRecordIndex &&
                   initialSectionModel?.RecordIndex == initialSectionRecordIndex &&
                   initialObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(initialObject, "UNIQUEID"), reorderedSourceUniqueId, StringComparison.Ordinal),
                $"real asset editor live-reorder smoke should start from a live object selection with reorder commands exposed for {sourcePath}");

            reorderFrontButton.PerformClick();
            Application.DoEvents();

            var reorderedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != expectedReorderedRecordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == reorderedSectionRecordIndex &&
                           objectListView.Items.Count == expectedSectionObjectCount &&
                           selectedObject?.RecordIndex == expectedReorderedRecordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), reorderedSourceUniqueId, StringComparison.Ordinal) &&
                           string.Equals(selectedObject.Title, reorderedSourceObjectTitle, StringComparison.Ordinal) &&
                           reorderFrontButton.Visible &&
                           reorderFrontButton.Enabled &&
                           reorderBackButton.Visible &&
                           reorderBackButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == expectedReorderedRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == reorderedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(reorderedSelection,
                $"real asset editor live-reorder smoke should preserve section/object continuity after reordering for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor live-reorder smoke should not expose undo after reordering a live row for {sourcePath}");
            if (!reorderedSelection)
            {
                return;
            }

            var reorderedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
            var companionObject = objectListView.Items.Cast<ListViewItem>()
                .Select(item => item.Tag as CopperfinStudioSnapshotObject)
                .FirstOrDefault(snapshotObject => snapshotObject?.RecordIndex == companionRecordIndex);
            Expect(reorderedObject is not null &&
                   companionObject is not null &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(reorderedObject, "UNIQUEID"), reorderedSourceUniqueId, StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(companionObject, "UNIQUEID"), companionUniqueId, StringComparison.Ordinal) &&
                   string.Equals(companionObject.Title, companionObjectTitle, StringComparison.Ordinal),
                $"real asset editor live-reorder smoke should preserve source and companion identities after reordering for {sourcePath}");

            var snapshotAfterReorder = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(snapshotAfterReorder.Success && snapshotAfterReorder.Document is not null,
                $"real asset editor live-reorder smoke should load reordered on-disk state before reload verification for {sourcePath}");
            if (snapshotAfterReorder.Success && snapshotAfterReorder.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    snapshotAfterReorder.Document,
                    expectedReorderedRecordIndex,
                    "UNIQUEID",
                    reorderedSourceUniqueId,
                    reorderedSourceObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"updated real asset editor live-reorder snapshot should preserve the reordered-source identity");
                AssertRealAssetRoundTripSnapshot(
                    snapshotAfterReorder.Document,
                    companionRecordIndex,
                    "UNIQUEID",
                    companionUniqueId,
                    companionObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"updated real asset editor live-reorder snapshot should preserve the companion identity");
                AssertRealAssetSectionObjectCount(
                    snapshotAfterReorder.Document,
                    expectedSectionTitle,
                    expectedSectionObjectCount,
                    $"updated real asset editor live-reorder snapshot should preserve section object counts");
                AssertRealAssetSectionRecordOrder(
                    snapshotAfterReorder.Document,
                    expectedSectionTitle,
                    reorderedSectionRecordIndex,
                    expectedReorderedSectionRecordOrder,
                    $"updated real asset editor live-reorder snapshot should preserve section record order");
                AssertRealAssetPreviewBoundsGeometry(
                    snapshotAfterReorder.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"updated real asset editor live-reorder snapshot should preserve live preview metadata");
            }

            var reloadedAfterReorder = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterReorder.Success && reloadedAfterReorder.Document is not null,
                $"real asset editor live-reorder smoke should reload reordered on-disk state for {sourcePath}");
            if (reloadedAfterReorder.Success && reloadedAfterReorder.Document is not null)
            {
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
                    $"reloaded real asset editor live-reorder snapshot should preserve the reordered-source identity");
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
                    $"reloaded real asset editor live-reorder snapshot should preserve the companion identity");
                AssertRealAssetSectionObjectCount(
                    reloadedAfterReorder.Document,
                    expectedSectionTitle,
                    expectedSectionObjectCount,
                    $"reloaded real asset editor live-reorder snapshot should preserve section object counts");
                AssertRealAssetSectionRecordOrder(
                    reloadedAfterReorder.Document,
                    expectedSectionTitle,
                    reorderedSectionRecordIndex,
                    expectedReorderedSectionRecordOrder,
                    $"reloaded real asset editor live-reorder snapshot should preserve section record order");
                AssertRealAssetPreviewBoundsGeometry(
                    reloadedAfterReorder.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"reloaded real asset editor live-reorder snapshot should preserve live preview metadata");
            }

            TearDownForm(hostForm);
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
