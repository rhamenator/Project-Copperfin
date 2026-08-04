
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
    private static void SmokeAssetEditorResizeToAnchorHeightCommandWithRealAsset(
        string? sourcePath,
        int anchorRecordIndex,
        int targetRecordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedAnchorUniqueId,
        string expectedTargetUniqueId,
        int expectedSectionCount,
        string expectedOriginalTargetRawWidth,
        string expectedOriginalTargetRawHeight,
        string expectedUpdatedTargetRawWidth,
        string expectedUpdatedTargetRawHeight,
        int expectedOriginalTargetLayoutWidth,
        int expectedOriginalTargetLayoutHeight,
        int expectedUpdatedTargetLayoutWidth,
        int expectedUpdatedTargetLayoutHeight)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor resize-to-anchor height candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorResizeToAnchorHeight-" + Guid.NewGuid().ToString("N"));
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
            var alignLeftButton = GetPrivateButton(control, "alignLeftObjectButton");
            var matchHeightButton = GetPrivateButton(control, "matchHeightObjectButton");
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real resize-to-anchor height smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor resize-to-anchor height smoke should load section data for {sourcePath}");
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

            var objectsLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>().Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject && snapshotObject.RecordIndex == anchorRecordIndex) &&
                      objectListView.Items.Cast<ListViewItem>().Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject && snapshotObject.RecordIndex == targetRecordIndex));
            Expect(objectsLoaded, $"real asset editor resize-to-anchor height smoke should surface anchor and target objects for {sourcePath}");
            if (!objectsLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                (snapshotObject.RecordIndex == anchorRecordIndex || snapshotObject.RecordIndex == targetRecordIndex);
                item.Focused = item.Tag is CopperfinStudioSnapshotObject focusedObject &&
                               focusedObject.RecordIndex == anchorRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   matchHeightButton.Visible &&
                   matchHeightButton.Enabled &&
                   !distributeHorizontalButton.Visible &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   objectListView.SelectedItems.Count == 2 &&
                   objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag is CopperfinStudioSnapshotObject initialAnchorObject &&
                   initialAnchorObject.RecordIndex == anchorRecordIndex &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(initialAnchorObject, "UNIQUEID"), expectedAnchorUniqueId, StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == anchorRecordIndex &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal),
                $"real asset editor resize-to-anchor height smoke should start from a live multi-selection with an anchor property-grid selection for {sourcePath}");

            matchHeightButton.PerformClick();
            Application.DoEvents();

            var resizedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != anchorRecordIndex)
                    {
                        return false;
                    }

                    var refreshedSnapshot = GetCurrentSnapshot(control);
                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedAnchorObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag as CopperfinStudioSnapshotObject;
                    var resizedTarget = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == targetRecordIndex);
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           objectListView.SelectedItems.Count == 2 &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal) &&
                           alignLeftButton.Visible &&
                           alignLeftButton.Enabled &&
                           matchHeightButton.Visible &&
                           matchHeightButton.Enabled &&
                           selectedAnchorObject is not null &&
                           selectedAnchorObject.RecordIndex == anchorRecordIndex &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedAnchorObject, "UNIQUEID"), expectedAnchorUniqueId, StringComparison.Ordinal) &&
                           resizedTarget is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(resizedTarget, "UNIQUEID"), expectedTargetUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(resizedTarget, "WIDTH"), expectedUpdatedTargetRawWidth, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(resizedTarget, "HEIGHT"), expectedUpdatedTargetRawHeight, StringComparison.Ordinal) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == anchorRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(resizedSelection,
                $"real asset editor resize-to-anchor height smoke should preserve anchor selection continuity and resize the target height for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor resize-to-anchor height smoke should expose undo after resizing for {sourcePath}");
            if (!resizedSelection)
            {
                return;
            }

            var reloadedAfterResize = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterResize.Success && reloadedAfterResize.Document is not null,
                $"real asset editor resize-to-anchor height smoke should reload resized on-disk state for {sourcePath}");
            if (reloadedAfterResize.Success && reloadedAfterResize.Document is not null)
            {
                var reloadedDocument = reloadedAfterResize.Document;
                Expect(reloadedDocument.ReportLayout is not null,
                    $"reloaded real asset editor resize-to-anchor height snapshot should include a report layout for {sourcePath}");
                if (reloadedDocument.ReportLayout is not null)
                {
                    Expect(!reloadedDocument.ReportLayout.IsLabel,
                        $"reloaded real asset editor resize-to-anchor height snapshot should preserve report identity for {sourcePath}");
                    Expect(reloadedDocument.ReportLayout.Sections.Count == expectedSectionCount,
                        $"reloaded real asset editor resize-to-anchor height snapshot should preserve section counts for {sourcePath}");
                }

                var reloadedTarget = reloadedDocument.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedTarget is not null,
                    $"reloaded real asset editor resize-to-anchor height snapshot should preserve raw target object {targetRecordIndex} for {sourcePath}");
                if (reloadedTarget is not null)
                {
                    Expect(string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "UNIQUEID"), expectedTargetUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "WIDTH"), expectedUpdatedTargetRawWidth, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "HEIGHT"), expectedUpdatedTargetRawHeight, StringComparison.Ordinal),
                        $"reloaded real asset editor resize-to-anchor height snapshot should preserve target unique id and resized raw WIDTH/HEIGHT for {sourcePath}");
                }

                var reloadedSection = reloadedDocument.ReportLayout?.Sections.FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor resize-to-anchor height snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "WIDTH") == expectedUpdatedTargetLayoutWidth &&
                           TryGetReportLayoutObjectValue(reloadedLayoutObject, "HEIGHT") == expectedUpdatedTargetLayoutHeight,
                        $"reloaded real asset editor resize-to-anchor height snapshot should expose layout WIDTH/HEIGHT={expectedUpdatedTargetLayoutWidth}/{expectedUpdatedTargetLayoutHeight} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor resize-to-anchor height smoke should execute undo after resizing for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != anchorRecordIndex)
                    {
                        return false;
                    }

                    var refreshedSnapshot = GetCurrentSnapshot(control);
                    var selectedAnchorObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag as CopperfinStudioSnapshotObject;
                    var restoredTarget = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == targetRecordIndex);
                    return objectListView.SelectedItems.Count == 2 &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal) &&
                           selectedAnchorObject is not null &&
                           selectedAnchorObject.RecordIndex == anchorRecordIndex &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedAnchorObject, "UNIQUEID"), expectedAnchorUniqueId, StringComparison.Ordinal) &&
                           restoredTarget is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredTarget, "UNIQUEID"), expectedTargetUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredTarget, "WIDTH"), expectedOriginalTargetRawWidth, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredTarget, "HEIGHT"), expectedOriginalTargetRawHeight, StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == anchorRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor resize-to-anchor height smoke should preserve anchor selection continuity after undoing for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor resize-to-anchor height smoke should clear undo after restoring original target geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor resize-to-anchor height smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    targetRecordIndex,
                    "WIDTH",
                    expectedOriginalTargetRawWidth,
                    "customerid",
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded undone real asset editor resize-to-anchor height snapshot should preserve original target WIDTH");

                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    targetRecordIndex,
                    "HEIGHT",
                    expectedOriginalTargetRawHeight,
                    "customerid",
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded undone real asset editor resize-to-anchor height snapshot should preserve original target HEIGHT");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections.FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor resize-to-anchor height snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "WIDTH") == expectedOriginalTargetLayoutWidth &&
                           TryGetReportLayoutObjectValue(reloadedLayoutObject, "HEIGHT") == expectedOriginalTargetLayoutHeight,
                        $"reloaded undone real asset editor resize-to-anchor height snapshot should expose layout WIDTH/HEIGHT={expectedOriginalTargetLayoutWidth}/{expectedOriginalTargetLayoutHeight} for {sourcePath}");
                }
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

    private static void SmokeAssetEditorSnapVerticallyCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedUniqueId,
        string expectedOriginalRawHpos,
        string expectedOriginalRawVpos,
        string expectedUpdatedRawHpos,
        string expectedUpdatedRawVpos,
        int expectedOriginalLayoutHpos,
        int expectedOriginalLayoutVpos,
        int expectedUpdatedLayoutHpos,
        int expectedUpdatedLayoutVpos)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor snap-vertical candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorSnapVertical-" + Guid.NewGuid().ToString("N"));
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
            var snapVerticalButton = GetPrivateButton(control, "snapVerticalObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real snap-vertical smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor snap-vertical smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor snap-vertical smoke should surface object {recordIndex} for {sourcePath}");
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

            Expect(snapVerticalButton.Visible &&
                   snapVerticalButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), expectedOriginalLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), expectedOriginalLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal),
                $"real asset editor snap-vertical smoke should start from a live object selection with original geometry exposed for {sourcePath}");

            snapVerticalButton.PerformClick();
            Application.DoEvents();

            var snappedSelection = WaitUntil(
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
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), expectedUpdatedLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), expectedUpdatedLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           snapVerticalButton.Visible &&
                           snapVerticalButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(snappedSelection,
                $"real asset editor snap-vertical smoke should preserve live section/object continuity after snapping for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor snap-vertical smoke should expose undo after snapping for {sourcePath}");
            if (!snappedSelection)
            {
                return;
            }

            var reloadedAfterSnap = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterSnap.Success && reloadedAfterSnap.Document is not null,
                $"real asset editor snap-vertical smoke should reload snapped on-disk state for {sourcePath}");
            if (reloadedAfterSnap.Success && reloadedAfterSnap.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterSnap.Document,
                    recordIndex,
                    "HPOS",
                    expectedUpdatedRawHpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded real asset editor snap-vertical snapshot should preserve HPOS");
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterSnap.Document,
                    recordIndex,
                    "VPOS",
                    expectedUpdatedRawVpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded real asset editor snap-vertical snapshot should preserve VPOS");

                var reloadedSection = reloadedAfterSnap.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor snap-vertical snapshot should preserve layout object {recordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedUpdatedLayoutHpos,
                        $"reloaded real asset editor snap-vertical snapshot should expose layout HPOS={expectedUpdatedLayoutHpos} for {sourcePath}");
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedUpdatedLayoutVpos,
                        $"reloaded real asset editor snap-vertical snapshot should expose layout VPOS={expectedUpdatedLayoutVpos} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor snap-vertical smoke should execute undo after snapping for {sourcePath}");
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
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), expectedOriginalLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), expectedOriginalLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor snap-vertical smoke should preserve live section/object continuity after undoing snap for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor snap-vertical smoke should clear undo after restoring original geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor snap-vertical smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    "HPOS",
                    expectedOriginalRawHpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded undone real asset editor snap-vertical snapshot should preserve original HPOS");
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    "VPOS",
                    expectedOriginalRawVpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded undone real asset editor snap-vertical snapshot should preserve original VPOS");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor snap-vertical snapshot should preserve layout object {recordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedOriginalLayoutHpos,
                        $"reloaded undone real asset editor snap-vertical snapshot should expose layout HPOS={expectedOriginalLayoutHpos} for {sourcePath}");
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedOriginalLayoutVpos,
                        $"reloaded undone real asset editor snap-vertical snapshot should expose layout VPOS={expectedOriginalLayoutVpos} for {sourcePath}");
                }
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
