
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
    private static void SmokeAssetEditorSnapHorizontallyCommandWithRealAsset(
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
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor snap-horizontal candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorSnapHorizontal-" + Guid.NewGuid().ToString("N"));
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
            var snapHorizontalButton = GetPrivateButton(control, "snapHorizontalObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real snap-horizontal smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor snap-horizontal smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor snap-horizontal smoke should surface object {recordIndex} for {sourcePath}");
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

            Expect(snapHorizontalButton.Visible &&
                   snapHorizontalButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), expectedOriginalLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), expectedOriginalLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal),
                $"real asset editor snap-horizontal smoke should start from a live object selection with original geometry exposed for {sourcePath}");

            snapHorizontalButton.PerformClick();
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
                           snapHorizontalButton.Visible &&
                           snapHorizontalButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(snappedSelection,
                $"real asset editor snap-horizontal smoke should preserve live section/object continuity after snapping for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor snap-horizontal smoke should expose undo after snapping for {sourcePath}");
            if (!snappedSelection)
            {
                return;
            }

            var reloadedAfterSnap = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterSnap.Success && reloadedAfterSnap.Document is not null,
                $"real asset editor snap-horizontal smoke should reload snapped on-disk state for {sourcePath}");
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
                    "reloaded real asset editor snap-horizontal snapshot should preserve HPOS");
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
                    "reloaded real asset editor snap-horizontal snapshot should preserve VPOS");

                var reloadedSection = reloadedAfterSnap.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor snap-horizontal snapshot should preserve layout object {recordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedUpdatedLayoutHpos,
                        $"reloaded real asset editor snap-horizontal snapshot should expose layout HPOS={expectedUpdatedLayoutHpos} for {sourcePath}");
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedUpdatedLayoutVpos,
                        $"reloaded real asset editor snap-horizontal snapshot should expose layout VPOS={expectedUpdatedLayoutVpos} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor snap-horizontal smoke should execute undo after snapping for {sourcePath}");
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
                $"real asset editor snap-horizontal smoke should preserve live section/object continuity after undoing snap for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor snap-horizontal smoke should clear undo after restoring original geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor snap-horizontal smoke should reload restored on-disk state for {sourcePath}");
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
                    "reloaded undone real asset editor snap-horizontal snapshot should preserve original HPOS");
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
                    "reloaded undone real asset editor snap-horizontal snapshot should preserve original VPOS");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor snap-horizontal snapshot should preserve layout object {recordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedOriginalLayoutHpos,
                        $"reloaded undone real asset editor snap-horizontal snapshot should expose layout HPOS={expectedOriginalLayoutHpos} for {sourcePath}");
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedOriginalLayoutVpos,
                        $"reloaded undone real asset editor snap-horizontal snapshot should expose layout VPOS={expectedOriginalLayoutVpos} for {sourcePath}");
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

    private static void SmokeAssetEditorSnapToGridCommandWithRealAsset(
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
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor snap-to-grid candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorSnapToGrid-" + Guid.NewGuid().ToString("N"));
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
            var snapToGridButton = GetPrivateButton(control, "snapToGridObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real snap-to-grid smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor snap-to-grid smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor snap-to-grid smoke should surface object {recordIndex} for {sourcePath}");
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

            Expect(snapToGridButton.Visible &&
                   snapToGridButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "HPOS"), expectedOriginalLayoutHpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal) &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, "VPOS"), expectedOriginalLayoutVpos.ToString(CultureInfo.InvariantCulture), StringComparison.Ordinal),
                $"real asset editor snap-to-grid smoke should start from a live object selection with original geometry exposed for {sourcePath}");

            snapToGridButton.PerformClick();
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
                           snapToGridButton.Visible &&
                           snapToGridButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(snappedSelection,
                $"real asset editor snap-to-grid smoke should preserve live section/object continuity after snapping for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor snap-to-grid smoke should expose undo after snapping for {sourcePath}");
            if (!snappedSelection)
            {
                return;
            }

            var reloadedAfterSnap = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterSnap.Success && reloadedAfterSnap.Document is not null,
                $"real asset editor snap-to-grid smoke should reload snapped on-disk state for {sourcePath}");
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
                    "reloaded real asset editor snap-to-grid snapshot should preserve HPOS");
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
                    "reloaded real asset editor snap-to-grid snapshot should preserve VPOS");

                var reloadedSection = reloadedAfterSnap.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded real asset editor snap-to-grid snapshot should preserve layout object {recordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedUpdatedLayoutHpos,
                        $"reloaded real asset editor snap-to-grid snapshot should expose layout HPOS={expectedUpdatedLayoutHpos} for {sourcePath}");
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedUpdatedLayoutVpos,
                        $"reloaded real asset editor snap-to-grid snapshot should expose layout VPOS={expectedUpdatedLayoutVpos} for {sourcePath}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor snap-to-grid smoke should execute undo after snapping for {sourcePath}");
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
                           snapToGridButton.Visible &&
                           snapToGridButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor snap-to-grid smoke should preserve live section/object continuity after undoing snap for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor snap-to-grid smoke should clear undo after restoring original geometry for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor snap-to-grid smoke should reload restored on-disk state for {sourcePath}");
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
                    "reloaded undone real asset editor snap-to-grid snapshot should preserve original HPOS");
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
                    "reloaded undone real asset editor snap-to-grid snapshot should preserve original VPOS");

                var reloadedSection = reloadedAfterUndo.Document.ReportLayout?.Sections
                    .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
                var reloadedLayoutObject = reloadedSection?.Objects
                    .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
                Expect(reloadedLayoutObject is not null,
                    $"reloaded undone real asset editor snap-to-grid snapshot should preserve layout object {recordIndex} for {sourcePath}");
                if (reloadedLayoutObject is not null)
                {
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "HPOS") == expectedOriginalLayoutHpos,
                        $"reloaded undone real asset editor snap-to-grid snapshot should expose layout HPOS={expectedOriginalLayoutHpos} for {sourcePath}");
                    Expect(TryGetReportLayoutObjectValue(reloadedLayoutObject, "VPOS") == expectedOriginalLayoutVpos,
                        $"reloaded undone real asset editor snap-to-grid snapshot should expose layout VPOS={expectedOriginalLayoutVpos} for {sourcePath}");
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
