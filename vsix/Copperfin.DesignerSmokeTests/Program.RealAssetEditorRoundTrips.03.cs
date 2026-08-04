
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
    private static void SmokeAssetEditorDeletedBatchPropertyRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedUniqueId,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges,
        IReadOnlyList<KeyValuePair<string, string>> expectedUpdatedSelectionValues,
        IReadOnlyList<KeyValuePair<string, string>> expectedOriginalRawValues,
        IReadOnlyList<KeyValuePair<string, int?>> expectedOriginalLayoutValues,
        IReadOnlyList<KeyValuePair<string, int?>> expectedUpdatedLayoutValues,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor deleted batch candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletedBatchWrites-" + Guid.NewGuid().ToString("N"));
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
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real deleted-batch smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor deleted-batch smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor deleted-batch smoke should surface object {recordIndex} for {sourcePath}");
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

            Expect(deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor deleted-batch smoke should start from a live object selection with delete exposed for {sourcePath}");

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
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(deletedSelection,
                $"real asset editor deleted-batch smoke should preserve deleted selection continuity before editing for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection deletedObjectSelection)
            {
                return;
            }

            var deletedSelectionValues = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            foreach (var property in propertyChanges)
            {
                deletedSelectionValues[property.Key] =
                    TypeDescriptor.GetProperties(deletedObjectSelection)[property.Key]?.GetValue(deletedObjectSelection)?.ToString() ?? string.Empty;
            }

            InvokeAssetEditorVoid(control, "ApplyVisualPropertyChanges", recordIndex, propertyChanges);
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

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    if (!string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) ||
                        selectedSectionModel?.RecordIndex != expectedSectionRecordIndex ||
                        selectedSectionModel?.DeletedObjectCount != 1 ||
                        selectedObject?.RecordIndex != recordIndex ||
                        !selectedObject.Deleted ||
                        !string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) ||
                        !string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) ||
                        !string.Equals(objectState, "Deleted", StringComparison.Ordinal) ||
                        objectListView.Items.Count != expectedDeletedSectionVisibleObjectCount ||
                        deleteButton.Visible ||
                        !restoreButton.Visible ||
                        !restoreButton.Enabled ||
                        !string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) ||
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") != recordIndex ||
                        ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") != expectedSectionRecordIndex ||
                        ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))
                    {
                        return false;
                    }

                    foreach (var property in expectedUpdatedSelectionValues)
                    {
                        var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[property.Key]?.GetValue(refreshedSelection)?.ToString();
                        if (!string.Equals(propertyValue, property.Value, StringComparison.Ordinal))
                        {
                            return false;
                        }
                    }

                    return true;
                });
            Expect(updatedSelection,
                $"real asset editor deleted-batch smoke should preserve deleted section/object continuity after editing for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-batch smoke should expose undo after editing for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor deleted-batch smoke should reload updated deleted on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                foreach (var property in propertyChanges)
                {
                    var expectedLayoutValue = expectedUpdatedLayoutValues
                        .FirstOrDefault(candidate => string.Equals(candidate.Key, property.Key, StringComparison.OrdinalIgnoreCase))
                        .Value;
                    AssertRealAssetDeletedObjectPropertySnapshot(
                        reloadedAfterUpdate.Document,
                        recordIndex,
                        property.Key,
                        property.Value,
                        expectedLayoutValue,
                        expectedUniqueId,
                        expectedObjectTitle,
                        expectedSectionTitle,
                        expectedSectionRecordIndex,
                        expectedSectionCount,
                        expectLabel,
                        expectedDeletedSectionVisibleObjectCount,
                        $"reloaded real asset editor deleted-batch snapshot should preserve {property.Key}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor deleted-batch smoke should execute undo after editing for {sourcePath}");
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
                    if (!string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) ||
                        selectedSectionModel?.RecordIndex != expectedSectionRecordIndex ||
                        selectedSectionModel?.DeletedObjectCount != 1 ||
                        selectedObject?.RecordIndex != recordIndex ||
                        !selectedObject.Deleted ||
                        !string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) ||
                        !string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) ||
                        !string.Equals(objectState, "Deleted", StringComparison.Ordinal) ||
                        objectListView.Items.Count != expectedDeletedSectionVisibleObjectCount ||
                        deleteButton.Visible ||
                        !restoreButton.Visible ||
                        !restoreButton.Enabled ||
                        !string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) ||
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") != recordIndex ||
                        ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") != expectedSectionRecordIndex ||
                        ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))
                    {
                        return false;
                    }

                    foreach (var property in deletedSelectionValues)
                    {
                        var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[property.Key]?.GetValue(refreshedSelection)?.ToString();
                        if (!string.Equals(propertyValue, property.Value, StringComparison.Ordinal))
                        {
                            return false;
                        }
                    }

                    return true;
                });
            Expect(undoneSelection,
                $"real asset editor deleted-batch smoke should preserve deleted section/object continuity after undo for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-batch smoke should retain the earlier delete-state undo after restoring the batch for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor deleted-batch smoke should reload restored deleted on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                foreach (var property in expectedOriginalRawValues)
                {
                    var expectedLayoutValue = expectedOriginalLayoutValues
                        .FirstOrDefault(candidate => string.Equals(candidate.Key, property.Key, StringComparison.OrdinalIgnoreCase))
                        .Value;
                    AssertRealAssetDeletedObjectPropertySnapshot(
                        reloadedAfterUndo.Document,
                        recordIndex,
                        property.Key,
                        property.Value,
                        expectedLayoutValue,
                        expectedUniqueId,
                        expectedObjectTitle,
                        expectedSectionTitle,
                        expectedSectionRecordIndex,
                        expectedSectionCount,
                        expectLabel,
                        expectedDeletedSectionVisibleObjectCount,
                        $"reloaded undone real asset editor deleted-batch snapshot should preserve {property.Key}");
                }
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
                    if (!string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) ||
                        selectedSectionModel?.RecordIndex != expectedSectionRecordIndex ||
                        selectedSectionModel?.DeletedObjectCount != 0 ||
                        selectedObject?.RecordIndex != recordIndex ||
                        selectedObject.Deleted ||
                        !string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) ||
                        !string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) ||
                        !string.Equals(objectState, "Live", StringComparison.Ordinal) ||
                        objectListView.Items.Count != 1 ||
                        !deleteButton.Visible ||
                        !deleteButton.Enabled ||
                        restoreButton.Visible ||
                        !string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) ||
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") != recordIndex ||
                        ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") != expectedSectionRecordIndex ||
                        ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))
                    {
                        return false;
                    }

                    foreach (var property in deletedSelectionValues)
                    {
                        var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[property.Key]?.GetValue(refreshedSelection)?.ToString();
                        if (!string.Equals(propertyValue, property.Value, StringComparison.Ordinal))
                        {
                            return false;
                        }
                    }

                    return true;
                });
            Expect(restoredSelection,
                $"real asset editor deleted-batch smoke should preserve live continuity after restore for {sourcePath}");

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
