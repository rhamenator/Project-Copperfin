
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
    private static void SmokeAssetEditorPlacementRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string initialSectionTitle,
        string updatedSectionTitle,
        int updatedSectionRecordIndex,
        string propertyName,
        object updatedPropertyValue,
        string? expectedOriginalSelectionValue,
        string expectedUpdatedSelectionValue,
        string expectedUpdatedRawValue,
        string expectedOriginalRawValue,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalUnplacedObjectCount,
        int expectedUpdatedUnplacedObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor placement candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorPlacements-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real placement smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor placement smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = string.Equals(item.Text, initialSectionTitle, StringComparison.OrdinalIgnoreCase);
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor placement smoke should surface object {recordIndex} for {sourcePath}");
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

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor placement smoke should start from an object-rooted property-grid selection for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                return;
            }

            var initialSelectionValue = TypeDescriptor.GetProperties(objectSelection)[propertyName]?.GetValue(objectSelection)?.ToString() ?? string.Empty;
            var expectedUndoSelectionValue = expectedOriginalSelectionValue ?? initialSelectionValue;
            if (expectedOriginalSelectionValue is not null)
            {
                Expect(string.Equals(initialSelectionValue, expectedOriginalSelectionValue, StringComparison.Ordinal),
                    $"real asset editor placement smoke should expose original property-grid value {propertyName} for {sourcePath}");
            }

            TypeDescriptor.GetProperties(objectSelection)[propertyName]?.SetValue(objectSelection, updatedPropertyValue);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, 0);
            Application.DoEvents();

            var updatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection, updatedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedObject?.RecordIndex == recordIndex &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == updatedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") &&
                           HasLabelTextContaining(control, $"Unplaced objects: {expectedUpdatedUnplacedObjectCount}") &&
                           string.Equals(propertyValue, expectedUpdatedSelectionValue, StringComparison.Ordinal);
                });
            Expect(updatedSelection,
                $"real asset editor placement smoke should preserve section/object continuity after editing {propertyName} for {sourcePath}");

            Expect(control.CanHandleUndoCommand(),
                $"real asset editor placement smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor placement smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    propertyName,
                    expectedUpdatedRawValue,
                    expectedObjectTitle,
                    updatedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded edited real asset placement snapshot should preserve {propertyName}");
                AssertRealAssetUnplacedObjectCount(
                    reloadedAfterUpdate.Document,
                    expectedUpdatedUnplacedObjectCount,
                    $"reloaded edited real asset placement snapshot should preserve unplaced-object counts");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor placement smoke should execute undo after editing {propertyName} for {sourcePath}");
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

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection, initialSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedObject?.RecordIndex == recordIndex &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                           ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") &&
                           HasLabelTextContaining(control, $"Unplaced objects: {expectedOriginalUnplacedObjectCount}") &&
                           string.Equals(propertyValue, expectedUndoSelectionValue, StringComparison.Ordinal);
                });
            Expect(undoneSelection,
                $"real asset editor placement smoke should preserve section/object continuity after undoing {propertyName} for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor placement smoke should clear undo after restoring {propertyName} for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor placement smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    propertyName,
                    expectedOriginalRawValue,
                    expectedObjectTitle,
                    initialSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: true,
                    $"reloaded undone editor real asset placement snapshot should preserve {propertyName}");
                AssertRealAssetUnplacedObjectCount(
                    reloadedAfterUndo.Document,
                    expectedOriginalUnplacedObjectCount,
                    $"reloaded undone editor real asset placement snapshot should preserve unplaced-object counts");
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

    private static void SmokeAssetEditorDuplicateCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string duplicateUniqueId,
        string expectedSourceObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalSectionObjectCount,
        int expectedUpdatedSectionObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor duplicate candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDuplicates-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var previousDuplicateUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID");

        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", duplicateUniqueId);

            var initialSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(initialSnapshot.Success && initialSnapshot.Document is not null,
                $"real asset editor duplicate smoke should load initial snapshot data for {sourcePath}");
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
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report/label design surface for the real duplicate smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor duplicate smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = string.Equals(item.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase);
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor duplicate smoke should surface object {recordIndex} for {sourcePath}");
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

            Expect(duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   objectListView.Items.Count == expectedOriginalSectionObjectCount &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor duplicate smoke should start from a live object selection with duplicate and delete commands exposed for {sourcePath}");

            duplicateButton.PerformClick();
            Application.DoEvents();

            var duplicatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex == recordIndex)
                    {
                        return false;
                    }

                    var selectedSectionModel = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioReportSection;
                    var selectedSnapshotObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var selectedUniqueId = selectedSnapshotObject is null
                        ? null
                        : TryGetSnapshotObjectPropertyValue(selectedSnapshotObject, "UNIQUEID");
                    return string.Equals(selectedSectionModel?.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           objectListView.Items.Count == expectedUpdatedSectionObjectCount &&
                           string.Equals(selectedUniqueId, duplicateUniqueId, StringComparison.Ordinal) &&
                           string.Equals(selectedSnapshotObject?.Title, expectedSourceObjectTitle, StringComparison.Ordinal) &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == refreshedSelection.RecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(duplicatedSelection,
                $"real asset editor duplicate smoke should preserve section/object continuity after duplicating the selected row for {sourcePath}");
            if (!duplicatedSelection)
            {
                return;
            }

            var updatedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(updatedAfterDuplicate.Success && updatedAfterDuplicate.Document is not null,
                $"real asset editor duplicate smoke should load duplicated on-disk state before reload verification for {sourcePath}");
            if (updatedAfterDuplicate.Success && updatedAfterDuplicate.Document is not null)
            {
                var updatedDuplicatedObject = FindSnapshotObjectByUniqueId(updatedAfterDuplicate.Document, duplicateUniqueId);
                Expect(updatedDuplicatedObject is not null,
                    $"updated real asset editor duplicate snapshot should preserve the duplicated UNIQUEID for {sourcePath}");
                if (updatedDuplicatedObject is not null)
                {
                    AssertRealAssetRoundTripSnapshot(
                        updatedAfterDuplicate.Document,
                        updatedDuplicatedObject.RecordIndex,
                        "UNIQUEID",
                        duplicateUniqueId,
                        expectedSourceObjectTitle,
                        expectedSectionTitle,
                        expectedSectionCount,
                        expectLabel,
                        expectUnplacedObject: false,
                        $"updated real asset editor duplicate snapshot should preserve duplicated object identity");
                }

                AssertRealAssetSectionObjectCount(
                    updatedAfterDuplicate.Document,
                    expectedSectionTitle,
                    expectedUpdatedSectionObjectCount,
                    $"updated real asset editor duplicate snapshot should preserve section object counts");
                AssertRealAssetPreviewBoundsGeometry(
                    updatedAfterDuplicate.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"updated real asset editor duplicate snapshot should preserve live preview metadata");
            }

            var reloadedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null,
                $"real asset editor duplicate smoke should reload duplicated on-disk state for {sourcePath}");
            if (reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null)
            {
                var reloadedDuplicatedObject = FindSnapshotObjectByUniqueId(reloadedAfterDuplicate.Document, duplicateUniqueId);
                Expect(reloadedDuplicatedObject is not null,
                    $"reloaded real asset editor duplicate snapshot should preserve the duplicated UNIQUEID for {sourcePath}");
                if (reloadedDuplicatedObject is not null)
                {
                    AssertRealAssetRoundTripSnapshot(
                        reloadedAfterDuplicate.Document,
                        reloadedDuplicatedObject.RecordIndex,
                        "UNIQUEID",
                        duplicateUniqueId,
                        expectedSourceObjectTitle,
                        expectedSectionTitle,
                        expectedSectionCount,
                        expectLabel,
                        expectUnplacedObject: false,
                        $"reloaded real asset editor duplicate snapshot should preserve duplicated object identity");
                }

                AssertRealAssetSectionObjectCount(
                    reloadedAfterDuplicate.Document,
                    expectedSectionTitle,
                    expectedUpdatedSectionObjectCount,
                    $"reloaded real asset editor duplicate snapshot should preserve section object counts");
                AssertRealAssetPreviewBoundsGeometry(
                    reloadedAfterDuplicate.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"reloaded real asset editor duplicate snapshot should preserve live preview metadata");
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

}
