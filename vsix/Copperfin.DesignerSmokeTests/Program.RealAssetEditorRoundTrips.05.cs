
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
    private static void SmokeAssetEditorNudgeCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedUniqueId,
        int deltaHpos,
        int deltaVpos,
        string expectedOriginalRawHpos,
        string expectedOriginalRawVpos,
        string expectedUpdatedRawHpos,
        string expectedUpdatedRawVpos,
        int expectedOriginalLayoutHpos,
        int expectedOriginalLayoutVpos,
        int expectedUpdatedLayoutHpos,
        int expectedUpdatedLayoutVpos,
        ExpectedPreviewBoundsGeometry expectedUpdatedPreviewBounds)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor nudge candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorNudges-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var initialSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(initialSnapshot.Success && initialSnapshot.Document is not null,
                $"real asset editor nudge smoke should load initial snapshot data for {sourcePath}");
            if (!initialSnapshot.Success || initialSnapshot.Document is null)
            {
                return;
            }

            var initialPreviewBounds = new ExpectedPreviewBoundsGeometry
            {
                Left = initialSnapshot.Document.ReportLayout?.PreviewBoundsLeft ?? 0,
                Top = initialSnapshot.Document.ReportLayout?.PreviewBoundsTop ?? 0,
                Right = initialSnapshot.Document.ReportLayout?.PreviewBoundsRight ?? 0,
                Bottom = initialSnapshot.Document.ReportLayout?.PreviewBoundsBottom ?? 0,
                Width = initialSnapshot.Document.ReportLayout?.PreviewBoundsWidth ?? 0,
                Height = initialSnapshot.Document.ReportLayout?.PreviewBoundsHeight ?? 0
            };
            AssertRealAssetLivePreviewBounds(
                initialSnapshot.Document,
                initialPreviewBounds.Left,
                initialPreviewBounds.Top,
                initialPreviewBounds.Right,
                initialPreviewBounds.Bottom,
                initialPreviewBounds.Width,
                initialPreviewBounds.Height,
                "initial real asset editor nudge snapshot should preserve live preview metadata");

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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real nudge smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor nudge smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor nudge smoke should surface object {recordIndex} for {sourcePath}");
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
                   initialSelection.RecordIndex == recordIndex &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), expectedOriginalLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), expectedOriginalLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal),
                $"real asset editor nudge smoke should start from a live object selection with original geometry exposed for {sourcePath}");

            var nudgeHandled = InvokeAssetEditorObject(control, "TryHandleNudgeObjectCommand", "both", (double)deltaHpos, (double)deltaVpos);
            Expect(nudgeHandled is bool handled && handled,
                $"real asset editor nudge smoke should execute the shared nudge command for {sourcePath}");
            Application.DoEvents();

            var nudgedSelection = WaitUntil(
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
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), expectedUpdatedLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), expectedUpdatedLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(nudgedSelection,
                $"real asset editor nudge smoke should preserve live section/object continuity after nudging for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor nudge smoke should expose undo after nudging for {sourcePath}");
            if (!nudgedSelection)
            {
                return;
            }

            var updatedAfterNudge = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(updatedAfterNudge.Success && updatedAfterNudge.Document is not null,
                $"real asset editor nudge smoke should load nudged on-disk state before reload verification for {sourcePath}");
            if (updatedAfterNudge.Success && updatedAfterNudge.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    updatedAfterNudge.Document,
                    recordIndex,
                    "HPOS",
                    expectedUpdatedRawHpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "updated real asset editor nudge snapshot should preserve HPOS",
                    assertObjectTitle: false);
                AssertRealAssetRoundTripSnapshot(
                    updatedAfterNudge.Document,
                    recordIndex,
                    "VPOS",
                    expectedUpdatedRawVpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "updated real asset editor nudge snapshot should preserve VPOS",
                    assertObjectTitle: false);
                AssertRealAssetLivePreviewBounds(
                    updatedAfterNudge.Document,
                    expectedUpdatedPreviewBounds.Left,
                    expectedUpdatedPreviewBounds.Top,
                    expectedUpdatedPreviewBounds.Right,
                    expectedUpdatedPreviewBounds.Bottom,
                    expectedUpdatedPreviewBounds.Width,
                    expectedUpdatedPreviewBounds.Height,
                    "updated real asset editor nudge snapshot should preserve live preview metadata");
            }

            var reloadedAfterNudge = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterNudge.Success && reloadedAfterNudge.Document is not null,
                $"real asset editor nudge smoke should reload nudged on-disk state for {sourcePath}");
            if (reloadedAfterNudge.Success && reloadedAfterNudge.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterNudge.Document,
                    recordIndex,
                    "HPOS",
                    expectedUpdatedRawHpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded real asset editor nudge snapshot should preserve HPOS",
                    assertObjectTitle: false);
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterNudge.Document,
                    recordIndex,
                    "VPOS",
                    expectedUpdatedRawVpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded real asset editor nudge snapshot should preserve VPOS",
                    assertObjectTitle: false);

                var reloadedSection = reloadedAfterNudge.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor nudge snapshot should preserve layout object {recordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedUpdatedLayoutHpos,
                        $"reloaded real asset editor nudge snapshot should expose layout HPOS={expectedUpdatedLayoutHpos} for {sourcePath}");
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedUpdatedLayoutVpos,
                        $"reloaded real asset editor nudge snapshot should expose layout VPOS={expectedUpdatedLayoutVpos} for {sourcePath}");
                }
                AssertRealAssetLivePreviewBounds(
                    reloadedAfterNudge.Document,
                    expectedUpdatedPreviewBounds.Left,
                    expectedUpdatedPreviewBounds.Top,
                    expectedUpdatedPreviewBounds.Right,
                    expectedUpdatedPreviewBounds.Bottom,
                    expectedUpdatedPreviewBounds.Width,
                    expectedUpdatedPreviewBounds.Height,
                    "reloaded real asset editor nudge snapshot should preserve live preview metadata");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor nudge smoke should execute the first undo after nudging for {sourcePath}");
            Application.DoEvents();

            var additionalUndoCount = 0;
            while (control.CanHandleUndoCommand() && additionalUndoCount < 8)
            {
                var additionalUndoHandled = control.TryHandleUndoCommand();
                Expect(additionalUndoHandled,
                    $"real asset editor nudge smoke should execute follow-up undo {additionalUndoCount + 2} needed to restore original geometry for {sourcePath}");
                Application.DoEvents();
                additionalUndoCount++;
            }

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
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), expectedOriginalLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), expectedOriginalLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor nudge smoke should preserve live section/object continuity after undoing nudge for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor nudge smoke should clear undo after restoring original geometry for {sourcePath}");
            if (!undoneSelection)
            {
                return;
            }

            var updatedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(updatedAfterUndo.Success && updatedAfterUndo.Document is not null,
                $"real asset editor nudge smoke should load restored on-disk state before reload verification for {sourcePath}");
            if (updatedAfterUndo.Success && updatedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    updatedAfterUndo.Document,
                    recordIndex,
                    "HPOS",
                    expectedOriginalRawHpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "updated undone real asset editor nudge snapshot should preserve original HPOS",
                    assertObjectTitle: false);
                AssertRealAssetRoundTripSnapshot(
                    updatedAfterUndo.Document,
                    recordIndex,
                    "VPOS",
                    expectedOriginalRawVpos,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "updated undone real asset editor nudge snapshot should preserve original VPOS",
                    assertObjectTitle: false);
                AssertRealAssetLivePreviewBounds(
                    updatedAfterUndo.Document,
                    initialPreviewBounds.Left,
                    initialPreviewBounds.Top,
                    initialPreviewBounds.Right,
                    initialPreviewBounds.Bottom,
                    initialPreviewBounds.Width,
                    initialPreviewBounds.Height,
                    "updated undone real asset editor nudge snapshot should preserve live preview metadata");
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor nudge smoke should reload restored on-disk state for {sourcePath}");
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
                    "reloaded undone real asset editor nudge snapshot should preserve original HPOS",
                    assertObjectTitle: false);
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
                    "reloaded undone real asset editor nudge snapshot should preserve original VPOS",
                    assertObjectTitle: false);

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor nudge snapshot should preserve layout object {recordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedOriginalLayoutHpos,
                        $"reloaded undone real asset editor nudge snapshot should expose layout HPOS={expectedOriginalLayoutHpos} for {sourcePath}");
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedOriginalLayoutVpos,
                        $"reloaded undone real asset editor nudge snapshot should expose layout VPOS={expectedOriginalLayoutVpos} for {sourcePath}");
                }
                AssertRealAssetLivePreviewBounds(
                    reloadedAfterUndo.Document,
                    initialPreviewBounds.Left,
                    initialPreviewBounds.Top,
                    initialPreviewBounds.Right,
                    initialPreviewBounds.Bottom,
                    initialPreviewBounds.Width,
                    initialPreviewBounds.Height,
                    "reloaded undone real asset editor nudge snapshot should preserve live preview metadata");
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

    private static void SmokeAssetEditorAlignLeftCommandWithRealAsset(
        string? sourcePath,
        int anchorRecordIndex,
        int targetRecordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedAnchorObjectTitle,
        string expectedTargetObjectTitle,
        int expectedSectionCount,
        string expectedAnchorUniqueId,
        string expectedTargetUniqueId,
        string expectedOriginalTargetRawHpos,
        string expectedUpdatedTargetRawHpos,
        int expectedOriginalTargetLayoutHpos,
        int expectedUpdatedTargetLayoutHpos)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor align-left candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorAlignLeft-" + Guid.NewGuid().ToString("N"));
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
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real align-left smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor align-left smoke should load section data for {sourcePath}");
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
            Expect(objectsLoaded, $"real asset editor align-left smoke should surface anchor and target objects for {sourcePath}");
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
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag is CopperfinStudioSnapshotObject initialAnchorObject &&
                   initialAnchorObject.RecordIndex == anchorRecordIndex &&
                   string.Equals(initialAnchorObject.Title, expectedAnchorObjectTitle, StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(initialAnchorObject, "UNIQUEID"), expectedAnchorUniqueId, StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == anchorRecordIndex &&
                   string.Equals(TypeDescriptor.GetProperties(initialSelection)["OBJECTSTATE"]?.GetValue(initialSelection)?.ToString(), "Live", StringComparison.Ordinal),
                $"real asset editor align-left smoke should start from a live multi-selection with an anchor property-grid selection for {sourcePath}");

            alignLeftButton.PerformClick();
            Application.DoEvents();

            var alignedSelection = WaitUntil(
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
                    var alignedTarget = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == targetRecordIndex);
                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal) &&
                           alignLeftButton.Visible &&
                           alignLeftButton.Enabled &&
                           selectedAnchorObject is not null &&
                           selectedAnchorObject.RecordIndex == anchorRecordIndex &&
                           string.Equals(selectedAnchorObject.Title, expectedAnchorObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedAnchorObject, "UNIQUEID"), expectedAnchorUniqueId, StringComparison.Ordinal) &&
                           alignedTarget is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(alignedTarget, "UNIQUEID"), expectedTargetUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(alignedTarget, "HPOS"), expectedUpdatedTargetRawHpos, StringComparison.Ordinal) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == anchorRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(alignedSelection,
                $"real asset editor align-left smoke should preserve anchor selection continuity and align the target object for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor align-left smoke should expose undo after aligning for {sourcePath}");
            if (!alignedSelection)
            {
                return;
            }

            var reloadedAfterAlign = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterAlign.Success && reloadedAfterAlign.Document is not null,
                $"real asset editor align-left smoke should reload aligned on-disk state for {sourcePath}");
            if (reloadedAfterAlign.Success && reloadedAfterAlign.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterAlign.Document,
                    targetRecordIndex,
                    "HPOS",
                    expectedUpdatedTargetRawHpos,
                    expectedTargetObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded real asset editor align-left snapshot should preserve target HPOS");

                var reloadedSection = reloadedAfterAlign.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor align-left snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedUpdatedTargetLayoutHpos,
                        $"reloaded real asset editor align-left snapshot should expose layout HPOS={expectedUpdatedTargetLayoutHpos} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor align-left smoke should execute undo after aligning for {sourcePath}");
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
                    return string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal) &&
                           selectedAnchorObject is not null &&
                           selectedAnchorObject.RecordIndex == anchorRecordIndex &&
                           string.Equals(selectedAnchorObject.Title, expectedAnchorObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedAnchorObject, "UNIQUEID"), expectedAnchorUniqueId, StringComparison.Ordinal) &&
                           restoredTarget is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredTarget, "UNIQUEID"), expectedTargetUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredTarget, "HPOS"), expectedOriginalTargetRawHpos, StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == anchorRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor align-left smoke should preserve anchor selection continuity after undoing for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor align-left smoke should clear undo after restoring original target geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor align-left smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    targetRecordIndex,
                    "HPOS",
                    expectedOriginalTargetRawHpos,
                    expectedTargetObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded undone real asset editor align-left snapshot should preserve original target HPOS");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor align-left snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedOriginalTargetLayoutHpos,
                        $"reloaded undone real asset editor align-left snapshot should expose layout HPOS={expectedOriginalTargetLayoutHpos} for {sourcePath}");
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
