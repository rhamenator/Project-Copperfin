
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
    private static void SmokeAssetEditorRealAssetSettingShouldRemainUnavailable(
        string? sourcePath,
        string propertyName,
        int expectedSectionCount,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor settings availability candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorSettingAvailability-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var settingsScopeTitle = new CopperfinLocalization("en-US").Text("AssetEditor.ReportSection.Settings");

        try
        {
            var loadedSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loadedSnapshot.Success && loadedSnapshot.Document is not null,
                $"real asset editor settings availability smoke should load snapshot data for {sourcePath}");
            if (!loadedSnapshot.Success || loadedSnapshot.Document is null || loadedSnapshot.Document.ReportLayout is null)
            {
                return;
            }

            AssertRealAssetSettingMissingSnapshot(
                loadedSnapshot.Document,
                propertyName,
                expectedSectionCount,
                expectLabel,
                $"initial editor real asset settings snapshot should keep {propertyName} absent");

            var settingsRecordIndex = loadedSnapshot.Document.ReportLayout.Settings.FirstOrDefault()?.RecordIndex ?? 0;

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
            var surface = FindDesignSurface(control);
            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal)));
            Expect(loaded, $"real asset editor settings availability smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                TearDownForm(hostForm);
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = string.Equals(item.Text, settingsScopeTitle, StringComparison.Ordinal);
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == settingsRecordIndex &&
                   objectListView.Items.Count == 0 &&
                   ReadSelectionPropertyValue(initialSelection, propertyName) is null &&
                   !initialSelection.TryGetUpdate(propertyName, out _, out _) &&
                   (surface is null ||
                    (ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                     ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                     !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))),
                $"real asset editor settings availability smoke should keep {propertyName} unavailable for {sourcePath}");
            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                TearDownForm(hostForm);
                return;
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)[propertyName];
            Expect(propertyDescriptor is null,
                $"real asset editor settings availability smoke should not surface editable property {propertyName} for {sourcePath}");

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

    private static void SmokeAssetEditorDeleteRestoreRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedUniqueId,
        int expectedOriginalSectionObjectCount,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor delete candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletes-" + Guid.NewGuid().ToString("N"));
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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real delete smoke.");
            var localization = new CopperfinLocalization("en-US");
            var settingsScopeTitle = localization.Text("AssetEditor.ReportSection.Settings");
            var deletedSettingsScopeTitle = localization.Format(
                "AssetEditor.ReportSection.Deleted",
                settingsScopeTitle);

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor delete smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor delete smoke should surface object {recordIndex} for {sourcePath}");
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
                   objectListView.Items.Count == expectedOriginalSectionObjectCount &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor delete smoke should start from a live object selection with only delete exposed for {sourcePath}");

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
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
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
                $"real asset editor delete smoke should preserve deleted object continuity inside the containing section for {sourcePath}");

            var reloadedAfterDelete = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDelete.Success && reloadedAfterDelete.Document is not null,
                $"real asset editor delete smoke should reload deleted on-disk state for {sourcePath}");
            if (reloadedAfterDelete.Success && reloadedAfterDelete.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterDelete.Document,
                    recordIndex,
                    expectedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    $"reloaded real asset editor delete snapshot should preserve deleted section-member continuity");
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
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 0 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedOriginalSectionObjectCount &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(restoredSelection,
                $"real asset editor delete smoke should preserve live object continuity after restore for {sourcePath}");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real asset editor delete smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterRestore.Document,
                    recordIndex,
                    "UNIQUEID",
                    expectedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded restored real asset editor delete snapshot should preserve source object identity");
                AssertRealAssetSectionObjectCount(
                    reloadedAfterRestore.Document,
                    expectedSectionTitle,
                    expectedOriginalSectionObjectCount,
                    $"reloaded restored real asset editor delete snapshot should preserve section object counts");

                Expect(reloadedAfterRestore.Document.ReportLayout is not null,
                    $"reloaded restored real asset editor delete snapshot should preserve report layout metadata for {sourcePath}");
                if (reloadedAfterRestore.Document.ReportLayout is not null)
                {
                    var restoredLayout = reloadedAfterRestore.Document.ReportLayout;
                    Expect(restoredLayout.PreviewBoundsAvailable,
                        $"reloaded restored real asset editor delete snapshot should preserve live preview bounds for {sourcePath}");
                    Expect(!restoredLayout.DeletedPreviewBoundsAvailable,
                        $"reloaded restored real asset editor delete snapshot should clear deleted preview bounds for {sourcePath}");

                    var expectedScopeTitle = restoredLayout.Settings.Count > 0
                        ? settingsScopeTitle
                        : restoredLayout.DeletedSettings.Count > 0
                            ? deletedSettingsScopeTitle
                            : null;
                    Expect(!string.IsNullOrWhiteSpace(expectedScopeTitle),
                        $"reloaded restored real asset editor delete snapshot should expose a settings scope for {sourcePath}");
                    if (!string.IsNullOrWhiteSpace(expectedScopeTitle))
                    {
                        foreach (ListViewItem item in sectionListView.Items)
                        {
                            item.Selected = string.Equals(item.Text, expectedScopeTitle, StringComparison.Ordinal);
                        }

                        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
                        Application.DoEvents();

                        var expectedPreviewBounds = localization.Format(
                            "AssetEditor.Property.BoundsValue",
                            restoredLayout.PreviewBoundsLeft,
                            restoredLayout.PreviewBoundsTop,
                            restoredLayout.PreviewBoundsRight,
                            restoredLayout.PreviewBoundsBottom,
                            restoredLayout.PreviewBoundsWidth,
                            restoredLayout.PreviewBoundsHeight);

                        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection restoredSettingsSelection &&
                               objectListView.Items.Count == 0 &&
                               string.Equals(ReadSelectionPropertyValue(restoredSettingsSelection, "PREVIEWBOUNDS"), expectedPreviewBounds, StringComparison.Ordinal) &&
                               ReadSelectionPropertyValue(restoredSettingsSelection, "DELETEDPREVIEWBOUNDS") is null,
                            $"real asset editor delete smoke should expose restored live preview bounds without stale deleted preview metadata for {sourcePath}");
                    }
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

    private static void SmokeFocusedRealAssetEditorDeleteRestoreRoundTrip()
    {
        WithResolvedRealAssetToolchain(() =>
        {
            SmokeAssetEditorDeleteRestoreRoundTripWithRealAsset(
                TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
                recordIndex: 7,
                expectedSectionTitle: "Title",
                expectedSectionRecordIndex: 1,
                expectedObjectTitle: "\"Titles By Author\"",
                expectedSectionCount: 6,
                expectLabel: false,
                expectedUniqueId: "_RC60MC40R",
                expectedOriginalSectionObjectCount: 1,
                expectedDeletedSectionVisibleObjectCount: 1);
            SmokeAssetEditorDeleteRestoreRoundTripWithRealAsset(
                TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
                recordIndex: 6,
                expectedSectionTitle: "Detail",
                expectedSectionRecordIndex: 3,
                expectedObjectTitle: "wiz_field",
                expectedSectionCount: 5,
                expectLabel: true,
                expectedUniqueId: "_QV30QY1DL",
                expectedOriginalSectionObjectCount: 1,
                expectedDeletedSectionVisibleObjectCount: 1);
        });
    }

    private static void SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedUniqueId,
        string propertyName,
        object updatedPropertyValue,
        string expectedUpdatedSelectionValue,
        string expectedOriginalRawValue,
        string expectedUpdatedRawValue,
        int? expectedOriginalLayoutValue,
        int? expectedUpdatedLayoutValue,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor deleted property candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletedWrites-" + Guid.NewGuid().ToString("N"));
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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real deleted-property smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor deleted-property smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor deleted-property smoke should surface object {recordIndex} for {sourcePath}");
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
                $"real asset editor deleted-property smoke should start from a live object selection with delete exposed for {sourcePath}");

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
                $"real asset editor deleted-property smoke should preserve deleted selection continuity before editing {propertyName} for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection deletedObjectSelection)
            {
                return;
            }

            var deletedSelectionValue = TypeDescriptor.GetProperties(deletedObjectSelection)[propertyName]?.GetValue(deletedObjectSelection)?.ToString() ?? string.Empty;
            var selectionProperties = TypeDescriptor.GetProperties(deletedObjectSelection);
            selectionProperties[propertyName]?.SetValue(deletedObjectSelection, updatedPropertyValue);
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

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           string.Equals(propertyValue, expectedUpdatedSelectionValue, StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(updatedSelection,
                $"real asset editor deleted-property smoke should preserve deleted section/object continuity after editing {propertyName} for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-property smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor deleted-property smoke should reload updated deleted on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetDeletedObjectPropertySnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    propertyName,
                    expectedUpdatedRawValue,
                    expectedUpdatedLayoutValue,
                    expectedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    $"reloaded real asset editor deleted-property snapshot should preserve {propertyName}");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor deleted-property smoke should execute undo after editing {propertyName} for {sourcePath}");
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
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           string.Equals(propertyValue, deletedSelectionValue, StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor deleted-property smoke should preserve deleted section/object continuity after undoing {propertyName} for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-property smoke should retain the earlier delete-state undo after restoring {propertyName} for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor deleted-property smoke should reload restored deleted on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetDeletedObjectPropertySnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    propertyName,
                    expectedOriginalRawValue,
                    expectedOriginalLayoutValue,
                    expectedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    $"reloaded undone real asset editor deleted-property snapshot should preserve {propertyName}");
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
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 0 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           string.Equals(propertyValue, deletedSelectionValue, StringComparison.Ordinal) &&
                           objectListView.Items.Count == 1 &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(restoredSelection,
                $"real asset editor deleted-property smoke should preserve live continuity after restoring the deleted row for {sourcePath}");

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
