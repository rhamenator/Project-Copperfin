
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
    private static void SmokeAssetEditorResizeToAnchorSizeCommandWithRealAsset(
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
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor resize-to-anchor candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorResizeToAnchor-" + Guid.NewGuid().ToString("N"));
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
            var matchSizeButton = GetPrivateButton(control, "matchSizeObjectButton");
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real resize-to-anchor smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor resize-to-anchor smoke should load section data for {sourcePath}");
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
            Expect(objectsLoaded, $"real asset editor resize-to-anchor smoke should surface anchor and target objects for {sourcePath}");
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
                   matchSizeButton.Visible &&
                   matchSizeButton.Enabled &&
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
                $"real asset editor resize-to-anchor smoke should start from a live multi-selection with an anchor property-grid selection for {sourcePath}");

            matchSizeButton.PerformClick();
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
                           matchSizeButton.Visible &&
                           matchSizeButton.Enabled &&
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
                $"real asset editor resize-to-anchor smoke should preserve anchor selection continuity and resize the target object for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor resize-to-anchor smoke should expose undo after resizing for {sourcePath}");
            if (!resizedSelection)
            {
                return;
            }

            var reloadedAfterResize = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterResize.Success && reloadedAfterResize.Document is not null,
                $"real asset editor resize-to-anchor smoke should reload resized on-disk state for {sourcePath}");
            if (reloadedAfterResize.Success && reloadedAfterResize.Document is not null)
            {
                var reloadedDocument = reloadedAfterResize.Document;
                Expect(reloadedDocument.ReportLayout is not null,
                    $"reloaded real asset editor resize-to-anchor snapshot should include a report layout for {sourcePath}");
                if (reloadedDocument.ReportLayout is not null)
                {
                    Expect(!reloadedDocument.ReportLayout.IsLabel,
                        $"reloaded real asset editor resize-to-anchor snapshot should preserve report identity for {sourcePath}");
                    Expect(reloadedDocument.ReportLayout.Sections.Count == expectedSectionCount,
                        $"reloaded real asset editor resize-to-anchor snapshot should preserve section counts for {sourcePath}");
                    var reloadedSection = reloadedDocument.ReportLayout.Sections.FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                    var reloadedLayoutObject = reloadedSection?.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                    Expect(reloadedSection is not null &&
                           string.Equals(reloadedSection.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase),
                        $"reloaded real asset editor resize-to-anchor snapshot should preserve section '{expectedSectionTitle}' for {sourcePath}");
                    Expect(reloadedLayoutObject is not null,
                        $"reloaded real asset editor resize-to-anchor snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                    if (reloadedLayoutObject is not null)
                    {
                        Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "WIDTH") == expectedUpdatedTargetLayoutWidth &&
                               TryGetReportLayoutObjectValue(reloadedLayoutObject, "HEIGHT") == expectedUpdatedTargetLayoutHeight,
                            $"reloaded real asset editor resize-to-anchor snapshot should expose layout WIDTH/HEIGHT={expectedUpdatedTargetLayoutWidth}/{expectedUpdatedTargetLayoutHeight} for {sourcePath}");
                    }
                }

                var reloadedTarget = reloadedDocument.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedTarget is not null,
                    $"reloaded real asset editor resize-to-anchor snapshot should preserve raw target object {targetRecordIndex} for {sourcePath}");
                if (reloadedTarget is not null)
                {
                    Expect(string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "UNIQUEID"), expectedTargetUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "WIDTH"), expectedUpdatedTargetRawWidth, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "HEIGHT"), expectedUpdatedTargetRawHeight, StringComparison.Ordinal),
                        $"reloaded real asset editor resize-to-anchor snapshot should preserve target unique id and resized raw WIDTH/HEIGHT for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor resize-to-anchor smoke should execute undo after resizing for {sourcePath}");
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
                $"real asset editor resize-to-anchor smoke should preserve anchor selection continuity after undoing for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor resize-to-anchor smoke should clear undo after restoring original target geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor resize-to-anchor smoke should reload restored on-disk state for {sourcePath}");
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
                    "reloaded undone real asset editor resize-to-anchor snapshot should preserve original target WIDTH");

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
                    "reloaded undone real asset editor resize-to-anchor snapshot should preserve original target HEIGHT");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections.FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor resize-to-anchor snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "WIDTH") == expectedOriginalTargetLayoutWidth &&
                           TryGetReportLayoutObjectValue(reloadedLayoutObject, "HEIGHT") == expectedOriginalTargetLayoutHeight,
                        $"reloaded undone real asset editor resize-to-anchor snapshot should expose layout WIDTH/HEIGHT={expectedOriginalTargetLayoutWidth}/{expectedOriginalTargetLayoutHeight} for {sourcePath}");
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

    private static void SmokeAssetEditorResizeToAnchorWidthCommandWithRealAsset(
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
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor resize-to-anchor width candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorResizeToAnchorWidth-" + Guid.NewGuid().ToString("N"));
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
            var matchWidthButton = GetPrivateButton(control, "matchWidthObjectButton");
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real resize-to-anchor width smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor resize-to-anchor width smoke should load section data for {sourcePath}");
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
            Expect(objectsLoaded, $"real asset editor resize-to-anchor width smoke should surface anchor and target objects for {sourcePath}");
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
                   matchWidthButton.Visible &&
                   matchWidthButton.Enabled &&
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
                $"real asset editor resize-to-anchor width smoke should start from a live multi-selection with an anchor property-grid selection for {sourcePath}");

            matchWidthButton.PerformClick();
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
                           matchWidthButton.Visible &&
                           matchWidthButton.Enabled &&
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
                $"real asset editor resize-to-anchor width smoke should preserve anchor selection continuity and resize the target width for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor resize-to-anchor width smoke should expose undo after resizing for {sourcePath}");
            if (!resizedSelection)
            {
                return;
            }

            var reloadedAfterResize = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterResize.Success && reloadedAfterResize.Document is not null,
                $"real asset editor resize-to-anchor width smoke should reload resized on-disk state for {sourcePath}");
            if (reloadedAfterResize.Success && reloadedAfterResize.Document is not null)
            {
                var reloadedDocument = reloadedAfterResize.Document;
                Expect(reloadedDocument.ReportLayout is not null,
                    $"reloaded real asset editor resize-to-anchor width snapshot should include a report layout for {sourcePath}");
                if (reloadedDocument.ReportLayout is not null)
                {
                    Expect(!reloadedDocument.ReportLayout.IsLabel,
                        $"reloaded real asset editor resize-to-anchor width snapshot should preserve report identity for {sourcePath}");
                    Expect(reloadedDocument.ReportLayout.Sections.Count == expectedSectionCount,
                        $"reloaded real asset editor resize-to-anchor width snapshot should preserve section counts for {sourcePath}");
                }

                var reloadedTarget = reloadedDocument.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedTarget is not null,
                    $"reloaded real asset editor resize-to-anchor width snapshot should preserve raw target object {targetRecordIndex} for {sourcePath}");
                if (reloadedTarget is not null)
                {
                    Expect(string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "UNIQUEID"), expectedTargetUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "WIDTH"), expectedUpdatedTargetRawWidth, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(reloadedTarget, "HEIGHT"), expectedUpdatedTargetRawHeight, StringComparison.Ordinal),
                        $"reloaded real asset editor resize-to-anchor width snapshot should preserve target unique id and resized raw WIDTH/HEIGHT for {sourcePath}");
                }

                var reloadedSection = reloadedDocument.ReportLayout?.Sections.FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor resize-to-anchor width snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "WIDTH") == expectedUpdatedTargetLayoutWidth &&
                           TryGetReportLayoutObjectValue(reloadedLayoutObject, "HEIGHT") == expectedUpdatedTargetLayoutHeight,
                        $"reloaded real asset editor resize-to-anchor width snapshot should expose layout WIDTH/HEIGHT={expectedUpdatedTargetLayoutWidth}/{expectedUpdatedTargetLayoutHeight} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor resize-to-anchor width smoke should execute undo after resizing for {sourcePath}");
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
                $"real asset editor resize-to-anchor width smoke should preserve anchor selection continuity after undoing for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor resize-to-anchor width smoke should clear undo after restoring original target geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor resize-to-anchor width smoke should reload restored on-disk state for {sourcePath}");
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
                    "reloaded undone real asset editor resize-to-anchor width snapshot should preserve original target WIDTH");

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
                    "reloaded undone real asset editor resize-to-anchor width snapshot should preserve original target HEIGHT");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections.FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects.FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor resize-to-anchor width snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "WIDTH") == expectedOriginalTargetLayoutWidth &&
                           TryGetReportLayoutObjectValue(reloadedLayoutObject, "HEIGHT") == expectedOriginalTargetLayoutHeight,
                        $"reloaded undone real asset editor resize-to-anchor width snapshot should expose layout WIDTH/HEIGHT={expectedOriginalTargetLayoutWidth}/{expectedOriginalTargetLayoutHeight} for {sourcePath}");
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
