
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
    private static void SmokeAssetEditorAlignTopCommandWithRealAsset(
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
        string expectedOriginalTargetRawVpos,
        string expectedUpdatedTargetRawVpos,
        int expectedOriginalTargetLayoutVpos,
        int expectedUpdatedTargetLayoutVpos)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor align-top candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorAlignTop-" + Guid.NewGuid().ToString("N"));
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
            var alignTopButton = GetPrivateButton(control, "alignTopObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real align-top smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor align-top smoke should load section data for {sourcePath}");
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
            Expect(objectsLoaded, $"real asset editor align-top smoke should surface anchor and target objects for {sourcePath}");
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

            Expect(alignTopButton.Visible &&
                   alignTopButton.Enabled &&
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
                $"real asset editor align-top smoke should start from a live multi-selection with an anchor property-grid selection for {sourcePath}");

            alignTopButton.PerformClick();
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
                           alignTopButton.Visible &&
                           alignTopButton.Enabled &&
                           selectedAnchorObject is not null &&
                           selectedAnchorObject.RecordIndex == anchorRecordIndex &&
                           string.Equals(selectedAnchorObject.Title, expectedAnchorObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedAnchorObject, "UNIQUEID"), expectedAnchorUniqueId, StringComparison.Ordinal) &&
                           alignedTarget is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(alignedTarget, "UNIQUEID"), expectedTargetUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(alignedTarget, "VPOS"), expectedUpdatedTargetRawVpos, StringComparison.Ordinal) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == anchorRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(alignedSelection,
                $"real asset editor align-top smoke should preserve anchor selection continuity and align the target object for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor align-top smoke should expose undo after aligning for {sourcePath}");
            if (!alignedSelection)
            {
                return;
            }

            var reloadedAfterAlign = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterAlign.Success && reloadedAfterAlign.Document is not null,
                $"real asset editor align-top smoke should reload aligned on-disk state for {sourcePath}");
            if (reloadedAfterAlign.Success && reloadedAfterAlign.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterAlign.Document,
                    targetRecordIndex,
                    "VPOS",
                    expectedUpdatedTargetRawVpos,
                    expectedTargetObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded real asset editor align-top snapshot should preserve target VPOS");

                var reloadedSection = reloadedAfterAlign.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor align-top snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedUpdatedTargetLayoutVpos,
                        $"reloaded real asset editor align-top snapshot should expose layout VPOS={expectedUpdatedTargetLayoutVpos} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor align-top smoke should execute undo after aligning for {sourcePath}");
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
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredTarget, "VPOS"), expectedOriginalTargetRawVpos, StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == anchorRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor align-top smoke should preserve anchor selection continuity after undoing for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor align-top smoke should clear undo after restoring original target geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor align-top smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    targetRecordIndex,
                    "VPOS",
                    expectedOriginalTargetRawVpos,
                    expectedTargetObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded undone real asset editor align-top snapshot should preserve original target VPOS");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == targetRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor align-top snapshot should preserve target layout object {targetRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedOriginalTargetLayoutVpos,
                        $"reloaded undone real asset editor align-top snapshot should expose layout VPOS={expectedOriginalTargetLayoutVpos} for {sourcePath}");
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

    private static void SmokeAssetEditorDistributeHorizontallyCommandWithRealAsset(
        string? sourcePath,
        IReadOnlyList<int> selectedRecordIndexes,
        int focusedRecordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedFocusedObjectTitle,
        int expectedSectionCount,
        string expectedFocusedUniqueId,
        string expectedOriginalFocusedRawHpos,
        string expectedUpdatedFocusedRawHpos,
        int expectedOriginalFocusedLayoutHpos,
        int expectedUpdatedFocusedLayoutHpos)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor distribute-horizontal candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDistributeHorizontal-" + Guid.NewGuid().ToString("N"));
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
            var distributeHorizontalButton = GetPrivateButton(control, "distributeHorizontalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real distribute-horizontal smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor distribute-horizontal smoke should load section data for {sourcePath}");
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
                () => selectedRecordIndexes.All(recordIndex =>
                    objectListView.Items.Cast<ListViewItem>().Any(item =>
                        item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                        snapshotObject.RecordIndex == recordIndex)));
            Expect(objectsLoaded, $"real asset editor distribute-horizontal smoke should surface all selected objects for {sourcePath}");
            if (!objectsLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                selectedRecordIndexes.Contains(snapshotObject.RecordIndex);
                item.Focused = item.Tag is CopperfinStudioSnapshotObject focusedObject &&
                               focusedObject.RecordIndex == focusedRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   distributeHorizontalButton.Visible &&
                   distributeHorizontalButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   objectListView.SelectedItems.Count == selectedRecordIndexes.Count &&
                   objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag is CopperfinStudioSnapshotObject initialFocusedObject &&
                   initialFocusedObject.RecordIndex == focusedRecordIndex &&
                   string.Equals(initialFocusedObject.Title, expectedFocusedObjectTitle, StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(initialFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == focusedRecordIndex &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), expectedOriginalFocusedLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal),
                $"real asset editor distribute-horizontal smoke should start from a focused live multi-selection for {sourcePath}");

            distributeHorizontalButton.PerformClick();
            Application.DoEvents();

            var distributedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != focusedRecordIndex)
                    {
                        return false;
                    }

                    var refreshedSnapshot = GetCurrentSnapshot(control);
                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedFocusedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag as CopperfinStudioSnapshotObject;
                    var distributedFocusedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == focusedRecordIndex);
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           objectListView.SelectedItems.Count == selectedRecordIndexes.Count &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal) &&
                           alignLeftButton.Visible &&
                           alignLeftButton.Enabled &&
                           distributeHorizontalButton.Visible &&
                           distributeHorizontalButton.Enabled &&
                           selectedFocusedObject is not null &&
                           selectedFocusedObject.RecordIndex == focusedRecordIndex &&
                           string.Equals(selectedFocusedObject.Title, expectedFocusedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                           distributedFocusedObject is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(distributedFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(distributedFocusedObject, "HPOS"), expectedUpdatedFocusedRawHpos, StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), expectedUpdatedFocusedLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == focusedRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(distributedSelection,
                $"real asset editor distribute-horizontal smoke should preserve focused multi-selection continuity and distribute geometry for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor distribute-horizontal smoke should expose undo after distributing for {sourcePath}");
            if (!distributedSelection)
            {
                return;
            }

            var reloadedAfterDistribute = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDistribute.Success && reloadedAfterDistribute.Document is not null,
                $"real asset editor distribute-horizontal smoke should reload distributed on-disk state for {sourcePath}");
            if (reloadedAfterDistribute.Success && reloadedAfterDistribute.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterDistribute.Document,
                    focusedRecordIndex,
                    "HPOS",
                    expectedUpdatedFocusedRawHpos,
                    expectedFocusedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded real asset editor distribute-horizontal snapshot should preserve focused HPOS");

                var reloadedSection = reloadedAfterDistribute.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == focusedRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor distribute-horizontal snapshot should preserve focused layout object {focusedRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedUpdatedFocusedLayoutHpos,
                        $"reloaded real asset editor distribute-horizontal snapshot should expose layout HPOS={expectedUpdatedFocusedLayoutHpos} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor distribute-horizontal smoke should execute undo after distributing for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != focusedRecordIndex)
                    {
                        return false;
                    }

                    var refreshedSnapshot = GetCurrentSnapshot(control);
                    var selectedFocusedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag as CopperfinStudioSnapshotObject;
                    var restoredFocusedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == focusedRecordIndex);
                    return objectListView.SelectedItems.Count == selectedRecordIndexes.Count &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal) &&
                           selectedFocusedObject is not null &&
                           selectedFocusedObject.RecordIndex == focusedRecordIndex &&
                           string.Equals(selectedFocusedObject.Title, expectedFocusedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                           restoredFocusedObject is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredFocusedObject, "HPOS"), expectedOriginalFocusedRawHpos, StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "HPOS"), expectedOriginalFocusedLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == focusedRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor distribute-horizontal smoke should preserve focused multi-selection continuity after undoing for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor distribute-horizontal smoke should clear undo after restoring original geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor distribute-horizontal smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    focusedRecordIndex,
                    "HPOS",
                    expectedOriginalFocusedRawHpos,
                    expectedFocusedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded undone real asset editor distribute-horizontal snapshot should preserve original focused HPOS");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == focusedRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor distribute-horizontal snapshot should preserve focused layout object {focusedRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedOriginalFocusedLayoutHpos,
                        $"reloaded undone real asset editor distribute-horizontal snapshot should expose layout HPOS={expectedOriginalFocusedLayoutHpos} for {sourcePath}");
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

    private static void SmokeAssetEditorDistributeVerticallyCommandWithRealAsset(
        string? sourcePath,
        IReadOnlyList<int> selectedRecordIndexes,
        int focusedRecordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedFocusedObjectTitle,
        int expectedSectionCount,
        string expectedFocusedUniqueId,
        string expectedOriginalFocusedRawVpos,
        string expectedUpdatedFocusedRawVpos,
        int expectedOriginalFocusedLayoutVpos,
        int expectedUpdatedFocusedLayoutVpos)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor distribute-vertical candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDistributeVertical-" + Guid.NewGuid().ToString("N"));
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
            var distributeVerticalButton = GetPrivateButton(control, "distributeVerticalObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real distribute-vertical smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor distribute-vertical smoke should load section data for {sourcePath}");
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
                () => selectedRecordIndexes.All(recordIndex =>
                    objectListView.Items.Cast<ListViewItem>().Any(item =>
                        item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                        snapshotObject.RecordIndex == recordIndex)));
            Expect(objectsLoaded, $"real asset editor distribute-vertical smoke should surface all selected objects for {sourcePath}");
            if (!objectsLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                selectedRecordIndexes.Contains(snapshotObject.RecordIndex);
                item.Focused = item.Tag is CopperfinStudioSnapshotObject focusedObject &&
                               focusedObject.RecordIndex == focusedRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(alignLeftButton.Visible &&
                   alignLeftButton.Enabled &&
                   distributeVerticalButton.Visible &&
                   distributeVerticalButton.Enabled &&
                   !duplicateButton.Visible &&
                   !reorderFrontButton.Visible &&
                   !reorderBackButton.Visible &&
                   !deleteButton.Visible &&
                   !restoreButton.Visible &&
                   objectListView.SelectedItems.Count == selectedRecordIndexes.Count &&
                   objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag is CopperfinStudioSnapshotObject initialFocusedObject &&
                   initialFocusedObject.RecordIndex == focusedRecordIndex &&
                   string.Equals(initialFocusedObject.Title, expectedFocusedObjectTitle, StringComparison.Ordinal) &&
                   string.Equals(TryGetSnapshotObjectPropertyValue(initialFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == focusedRecordIndex &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), expectedOriginalFocusedLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal),
                $"real asset editor distribute-vertical smoke should start from a focused live multi-selection for {sourcePath}");

            distributeVerticalButton.PerformClick();
            Application.DoEvents();

            var distributedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != focusedRecordIndex)
                    {
                        return false;
                    }

                    var refreshedSnapshot = GetCurrentSnapshot(control);
                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedFocusedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag as CopperfinStudioSnapshotObject;
                    var distributedFocusedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == focusedRecordIndex);
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           objectListView.SelectedItems.Count == selectedRecordIndexes.Count &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal) &&
                           alignLeftButton.Visible &&
                           alignLeftButton.Enabled &&
                           distributeVerticalButton.Visible &&
                           distributeVerticalButton.Enabled &&
                           selectedFocusedObject is not null &&
                           selectedFocusedObject.RecordIndex == focusedRecordIndex &&
                           string.Equals(selectedFocusedObject.Title, expectedFocusedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                           distributedFocusedObject is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(distributedFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(distributedFocusedObject, "VPOS"), expectedUpdatedFocusedRawVpos, StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), expectedUpdatedFocusedLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == focusedRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(distributedSelection,
                $"real asset editor distribute-vertical smoke should preserve focused multi-selection continuity and distribute geometry for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor distribute-vertical smoke should expose undo after distributing for {sourcePath}");
            if (!distributedSelection)
            {
                return;
            }

            var reloadedAfterDistribute = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDistribute.Success && reloadedAfterDistribute.Document is not null,
                $"real asset editor distribute-vertical smoke should reload distributed on-disk state for {sourcePath}");
            if (reloadedAfterDistribute.Success && reloadedAfterDistribute.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterDistribute.Document,
                    focusedRecordIndex,
                    "VPOS",
                    expectedUpdatedFocusedRawVpos,
                    expectedFocusedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded real asset editor distribute-vertical snapshot should preserve focused VPOS");

                var reloadedSection = reloadedAfterDistribute.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == focusedRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor distribute-vertical snapshot should preserve focused layout object {focusedRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedUpdatedFocusedLayoutVpos,
                        $"reloaded real asset editor distribute-vertical snapshot should expose layout VPOS={expectedUpdatedFocusedLayoutVpos} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor distribute-vertical smoke should execute undo after distributing for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != focusedRecordIndex)
                    {
                        return false;
                    }

                    var refreshedSnapshot = GetCurrentSnapshot(control);
                    var selectedFocusedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault(item => item.Focused)?.Tag as CopperfinStudioSnapshotObject;
                    var restoredFocusedObject = refreshedSnapshot.Objects.FirstOrDefault(item => item.RecordIndex == focusedRecordIndex);
                    return objectListView.SelectedItems.Count == selectedRecordIndexes.Count &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "OBJECTSTATE"), "Live", StringComparison.Ordinal) &&
                           selectedFocusedObject is not null &&
                           selectedFocusedObject.RecordIndex == focusedRecordIndex &&
                           string.Equals(selectedFocusedObject.Title, expectedFocusedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                           restoredFocusedObject is not null &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredFocusedObject, "UNIQUEID"), expectedFocusedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(restoredFocusedObject, "VPOS"), expectedOriginalFocusedRawVpos, StringComparison.Ordinal) &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, "VPOS"), expectedOriginalFocusedLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == focusedRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor distribute-vertical smoke should preserve focused multi-selection continuity after undoing for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor distribute-vertical smoke should clear undo after restoring original geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor distribute-vertical smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    focusedRecordIndex,
                    "VPOS",
                    expectedOriginalFocusedRawVpos,
                    expectedFocusedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel: false,
                    expectUnplacedObject: false,
                    "reloaded undone real asset editor distribute-vertical snapshot should preserve original focused VPOS");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == focusedRecordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor distribute-vertical snapshot should preserve focused layout object {focusedRecordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedOriginalFocusedLayoutVpos,
                        $"reloaded undone real asset editor distribute-vertical snapshot should expose layout VPOS={expectedOriginalFocusedLayoutVpos} for {sourcePath}");
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
