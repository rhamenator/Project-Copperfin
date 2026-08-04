
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
    private static void SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        string propertyName,
        object updatedPropertyValue,
        string? expectedOriginalSelectionValue,
        string expectedUpdatedSelectionValue,
        string expectedUpdatedRawValue,
        string expectedOriginalRawValue,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        bool expectUnplacedObject = false)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorWrites-" + Guid.NewGuid().ToString("N"));
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
            var surface = expectUnplacedObject
                ? FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real unplaced asset smoke.")
                : null;

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor smoke should surface object {recordIndex} for {sourcePath}");
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
                $"real asset editor smoke should start from an object-rooted property-grid selection for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                return;
            }

            var initialSelectionValue = TypeDescriptor.GetProperties(objectSelection)[propertyName]?.GetValue(objectSelection)?.ToString() ?? string.Empty;
            var expectedUndoSelectionValue = expectedOriginalSelectionValue ?? initialSelectionValue;
            if (expectedOriginalSelectionValue is not null)
            {
                Expect(string.Equals(initialSelectionValue, expectedOriginalSelectionValue, StringComparison.Ordinal),
                    $"real asset editor smoke should expose original property-grid value {propertyName} for {sourcePath}");
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
                    return string.Equals(selectedSection, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedObject?.RecordIndex == recordIndex &&
                           (!expectUnplacedObject ||
                            (string.Equals(ReadPrivateStringField(surface!, "assetFamily"), "report", StringComparison.Ordinal) &&
                             ReadPrivateNullableInt(surface!, "selectedRecordIndex") == recordIndex &&
                             ReadPrivateNullableInt(surface!, "selectedReportSectionRecordIndex") is null &&
                             ReadPrivateBoolField(surface!, "unplacedReportObjectsSelected"))) &&
                           string.Equals(propertyValue, expectedUpdatedSelectionValue, StringComparison.Ordinal);
                });
            Expect(updatedSelection,
                $"real asset editor smoke should preserve section/object continuity after editing {propertyName} for {sourcePath}");

            Expect(control.CanHandleUndoCommand(),
                $"real asset editor smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    propertyName,
                    expectedUpdatedRawValue,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject,
                    $"reloaded edited real asset snapshot should preserve {propertyName}");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor smoke should execute undo after editing {propertyName} for {sourcePath}");
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
                    return string.Equals(selectedSection, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedObject?.RecordIndex == recordIndex &&
                           (!expectUnplacedObject ||
                            (string.Equals(ReadPrivateStringField(surface!, "assetFamily"), "report", StringComparison.Ordinal) &&
                             ReadPrivateNullableInt(surface!, "selectedRecordIndex") == recordIndex &&
                             ReadPrivateNullableInt(surface!, "selectedReportSectionRecordIndex") is null &&
                             ReadPrivateBoolField(surface!, "unplacedReportObjectsSelected"))) &&
                           string.Equals(propertyValue, expectedUndoSelectionValue, StringComparison.Ordinal);
                });
            Expect(undoneSelection,
                $"real asset editor smoke should preserve section/object continuity after undoing {propertyName} for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor smoke should clear undo after restoring {propertyName} for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    propertyName,
                    expectedOriginalRawValue,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject,
                    $"reloaded undone editor real asset snapshot should preserve {propertyName}");
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

    private static void SmokeAssetEditorSettingsRoundTripWithRealAsset(
        string? sourcePath,
        string propertyName,
        string expectedOriginalSelectionValue,
        int updatedPropertyValue,
        string expectedUpdatedSelectionValue,
        string expectedOriginalRawValue,
        string expectedUpdatedRawValue,
        int expectedSectionCount,
        bool expectLabel,
        CopperfinStudioNamedValue? expectedUpdatedSetting = null,
        CopperfinStudioNamedValue? expectedUndoneSetting = null,
        bool expectRawSnapshotProperty = true,
        bool verifyExplicitClearAfterUpdate = false)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor settings write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorSettings-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var settingsScopeTitle = new CopperfinLocalization("en-US").Text("AssetEditor.ReportSection.Settings");

        try
        {
            var loadedSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loadedSnapshot.Success && loadedSnapshot.Document is not null,
                $"real asset editor settings smoke should load snapshot data for {sourcePath}");
            if (!loadedSnapshot.Success || loadedSnapshot.Document is null || loadedSnapshot.Document.ReportLayout is null)
            {
                return;
            }

            var initialSetting = loadedSnapshot.Document.ReportLayout.Settings
                .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase));
            Expect(initialSetting is not null,
                $"real asset editor settings smoke should expose {propertyName} in live settings for {sourcePath}");
            if (initialSetting is null)
            {
                return;
            }

            expectedUpdatedSetting ??= initialSetting;
            expectedUndoneSetting ??= initialSetting;

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
            Expect(loaded, $"real asset editor settings smoke should load section data for {sourcePath}");
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
                   initialSelection.RecordIndex == initialSetting.RecordIndex &&
                   objectListView.Items.Count == 0 &&
                   string.Equals(ReadSelectionPropertyValue(initialSelection, propertyName), expectedOriginalSelectionValue, StringComparison.Ordinal),
                $"real asset editor settings smoke should start from a settings-rooted property-grid selection for {sourcePath}");
            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                return;
            }

            TypeDescriptor.GetProperties(settingsSelection)[propertyName]?.SetValue(settingsSelection, updatedPropertyValue);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, int.Parse(expectedOriginalSelectionValue));
            Application.DoEvents();

            var updatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != initialSetting.RecordIndex)
                    {
                        return false;
                    }

                    return string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, settingsScopeTitle, StringComparison.Ordinal) &&
                           objectListView.Items.Count == 0 &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, propertyName), expectedUpdatedSelectionValue, StringComparison.Ordinal) &&
                           (surface is null ||
                            (ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                             ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                             !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected")));
                });
            Expect(updatedSelection,
                $"real asset editor settings smoke should preserve settings-rooted continuity after editing {propertyName} for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor settings smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor settings smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetSettingsSnapshot(
                    reloadedAfterUpdate.Document,
                    expectedUpdatedSetting,
                    expectedUpdatedRawValue,
                    expectedSectionCount,
                    expectLabel,
                    requireSelectedSettings: false,
                    expectRawSnapshotProperty: expectRawSnapshotProperty,
                    $"reloaded edited real asset settings snapshot should preserve {propertyName}");
            }

            if (verifyExplicitClearAfterUpdate)
            {
                if (propertyGrid.SelectedObject is not CopperfinDesignerSelection selectionWithProperty)
                {
                    TearDownForm(hostForm);
                    return;
                }

                var clearDescriptor = TypeDescriptor.GetProperties(selectionWithProperty)[propertyName];
                Expect(clearDescriptor is not null,
                    $"real asset editor settings clear smoke should keep editable property {propertyName} available before clearing for {sourcePath}");
                if (clearDescriptor is null)
                {
                    TearDownForm(hostForm);
                    return;
                }

                clearDescriptor.SetValue(selectionWithProperty, string.Empty);
                Expect(selectionWithProperty.TryGetUpdate(propertyName, out _, out _),
                    $"real asset editor settings clear smoke should prepare a clear for {propertyName} for {sourcePath}");
                InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, expectedUpdatedSelectionValue);
                Application.DoEvents();

                var clearedSelection = WaitUntil(
                    TimeSpan.FromSeconds(8),
                    () =>
                    {
                        if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                            refreshedSelection.RecordIndex != initialSetting.RecordIndex)
                        {
                            return false;
                        }

                        return string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, settingsScopeTitle, StringComparison.Ordinal) &&
                               objectListView.Items.Count == 0 &&
                               string.Equals(ReadSelectionPropertyValue(refreshedSelection, propertyName) ?? string.Empty, string.Empty, StringComparison.Ordinal) &&
                               (surface is null ||
                                (ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                                 ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                                 !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected")));
                    });
                Expect(clearedSelection,
                    $"real asset editor settings clear smoke should preserve settings-rooted continuity after clearing {propertyName} for {sourcePath}");

                var reloadedAfterClear = CopperfinStudioSnapshotClient.TryLoad(assetPath);
                Expect(reloadedAfterClear.Success && reloadedAfterClear.Document is not null,
                    $"real asset editor settings clear smoke should reload cleared on-disk state for {sourcePath}");
                if (reloadedAfterClear.Success && reloadedAfterClear.Document is not null)
                {
                    AssertRealAssetSettingMissingSnapshot(
                        reloadedAfterClear.Document,
                        propertyName,
                        expectedSectionCount,
                        expectLabel,
                        $"reloaded cleared editor real asset settings snapshot should keep {propertyName} absent");
                }
            }
            else
            {
                var undoHandled = control.TryHandleUndoCommand();
                Expect(undoHandled,
                    $"real asset editor settings smoke should execute undo after editing {propertyName} for {sourcePath}");
                Application.DoEvents();

                var undoneSelection = WaitUntil(
                    TimeSpan.FromSeconds(8),
                    () =>
                    {
                        if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                            refreshedSelection.RecordIndex != initialSetting.RecordIndex)
                        {
                            return false;
                        }

                        return string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, settingsScopeTitle, StringComparison.Ordinal) &&
                               objectListView.Items.Count == 0 &&
                               string.Equals(ReadSelectionPropertyValue(refreshedSelection, propertyName), expectedOriginalSelectionValue, StringComparison.Ordinal) &&
                               (surface is null ||
                                (ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                                 ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                                 !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected")));
                    });
                Expect(undoneSelection,
                    $"real asset editor settings smoke should preserve settings-rooted continuity after undoing {propertyName} for {sourcePath}");
                Expect(!control.CanHandleUndoCommand(),
                    $"real asset editor settings smoke should clear undo after restoring {propertyName} for {sourcePath}");

                var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
                Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                    $"real asset editor settings smoke should reload restored on-disk state for {sourcePath}");
                if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
                {
                    AssertRealAssetSettingsSnapshot(
                        reloadedAfterUndo.Document,
                        expectedUndoneSetting,
                        expectedOriginalRawValue,
                        expectedSectionCount,
                        expectLabel,
                        requireSelectedSettings: false,
                        expectRawSnapshotProperty: expectRawSnapshotProperty,
                        $"reloaded undone editor real asset settings snapshot should preserve {propertyName}");
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

    private static void SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
        string? sourcePath,
        string propertyName,
        string expectedOriginalSelectionValue,
        object updatedPropertyValue,
        string expectedUpdatedSelectionValue,
        int expectedSectionCount,
        bool expectLabel,
        CopperfinStudioNamedValue expectedUpdatedSetting,
        bool expectRawSnapshotProperty = true,
        bool verifyExplicitClearAfterUpdate = false)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor settings add candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorSettingAdds-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var settingsScopeTitle = new CopperfinLocalization("en-US").Text("AssetEditor.ReportSection.Settings");

        try
        {
            var loadedSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loadedSnapshot.Success && loadedSnapshot.Document is not null,
                $"real asset editor settings add smoke should load snapshot data for {sourcePath}");
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
            Expect(loaded, $"real asset editor settings add smoke should load section data for {sourcePath}");
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
                   string.Equals(ReadSelectionPropertyValue(initialSelection, propertyName) ?? string.Empty, expectedOriginalSelectionValue, StringComparison.Ordinal),
                $"real asset editor settings add smoke should start from a settings-rooted property-grid selection for {sourcePath}");
            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection settingsSelection)
            {
                return;
            }

            var propertyDescriptor = TypeDescriptor.GetProperties(settingsSelection)[propertyName];
            Expect(propertyDescriptor is not null,
                $"real asset editor settings add smoke should surface editable property {propertyName} for {sourcePath}");
            if (propertyDescriptor is null)
            {
                TearDownForm(hostForm);
                return;
            }

            propertyDescriptor.SetValue(settingsSelection, updatedPropertyValue);
            Expect(settingsSelection.TryGetUpdate(propertyName, out _, out _),
                $"real asset editor settings add smoke should prepare a host update for {propertyName} for {sourcePath}");
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, expectedOriginalSelectionValue);
            Application.DoEvents();

            var updatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != settingsRecordIndex)
                    {
                        return false;
                    }

                    return string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, settingsScopeTitle, StringComparison.Ordinal) &&
                           objectListView.Items.Count == 0 &&
                           string.Equals(ReadSelectionPropertyValue(refreshedSelection, propertyName) ?? string.Empty, expectedUpdatedSelectionValue, StringComparison.Ordinal) &&
                           (surface is null ||
                            (ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                             ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                             !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected")));
                });
            Expect(updatedSelection,
                $"real asset editor settings add smoke should preserve settings-rooted continuity after editing {propertyName} for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor settings add smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor settings add smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetSettingsSnapshot(
                    reloadedAfterUpdate.Document,
                    expectedUpdatedSetting,
                    expectedUpdatedSelectionValue,
                    expectedSectionCount,
                    expectLabel,
                    requireSelectedSettings: false,
                    expectRawSnapshotProperty: expectRawSnapshotProperty,
                    $"reloaded edited real asset settings snapshot should preserve {propertyName}");
            }

            if (verifyExplicitClearAfterUpdate)
            {
                if (propertyGrid.SelectedObject is not CopperfinDesignerSelection selectionWithProperty)
                {
                    TearDownForm(hostForm);
                    return;
                }

                var clearDescriptor = TypeDescriptor.GetProperties(selectionWithProperty)[propertyName];
                Expect(clearDescriptor is not null,
                    $"real asset editor settings clear smoke should keep editable property {propertyName} available before clearing for {sourcePath}");
                if (clearDescriptor is null)
                {
                    TearDownForm(hostForm);
                    return;
                }

                clearDescriptor.SetValue(selectionWithProperty, string.Empty);
                Expect(selectionWithProperty.TryGetUpdate(propertyName, out _, out _),
                    $"real asset editor settings clear smoke should prepare a clear for {propertyName} for {sourcePath}");
                InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, expectedUpdatedSelectionValue);
                Application.DoEvents();

                var clearedSelection = WaitUntil(
                    TimeSpan.FromSeconds(8),
                    () =>
                    {
                        if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                            refreshedSelection.RecordIndex != settingsRecordIndex)
                        {
                            return false;
                        }

                        return string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, settingsScopeTitle, StringComparison.Ordinal) &&
                               objectListView.Items.Count == 0 &&
                               string.Equals(ReadSelectionPropertyValue(refreshedSelection, propertyName) ?? string.Empty, expectedOriginalSelectionValue, StringComparison.Ordinal) &&
                               (surface is null ||
                                (ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                                 ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                                 !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected")));
                    });
                Expect(clearedSelection,
                    $"real asset editor settings clear smoke should preserve settings-rooted continuity after clearing {propertyName} for {sourcePath}");

                var reloadedAfterClear = CopperfinStudioSnapshotClient.TryLoad(assetPath);
                Expect(reloadedAfterClear.Success && reloadedAfterClear.Document is not null,
                    $"real asset editor settings clear smoke should reload cleared on-disk state for {sourcePath}");
                if (reloadedAfterClear.Success && reloadedAfterClear.Document is not null)
                {
                    AssertRealAssetSettingMissingSnapshot(
                        reloadedAfterClear.Document,
                        propertyName,
                        expectedSectionCount,
                        expectLabel,
                        $"reloaded cleared editor real asset settings snapshot should keep {propertyName} absent");
                }
            }
            else
            {
                var undoHandled = control.TryHandleUndoCommand();
                Expect(undoHandled,
                    $"real asset editor settings add smoke should execute undo after editing {propertyName} for {sourcePath}");
                Application.DoEvents();

                var undoneSelection = WaitUntil(
                    TimeSpan.FromSeconds(8),
                    () =>
                    {
                        if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                            refreshedSelection.RecordIndex != settingsRecordIndex)
                        {
                            return false;
                        }

                        return string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, settingsScopeTitle, StringComparison.Ordinal) &&
                               objectListView.Items.Count == 0 &&
                               string.Equals(ReadSelectionPropertyValue(refreshedSelection, propertyName) ?? string.Empty, expectedOriginalSelectionValue, StringComparison.Ordinal) &&
                               (surface is null ||
                                (ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                                 ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                                 !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected")));
                    });
                Expect(undoneSelection,
                    $"real asset editor settings add smoke should preserve settings-rooted continuity after undoing {propertyName} for {sourcePath}");
                Expect(!control.CanHandleUndoCommand(),
                    $"real asset editor settings add smoke should clear undo after restoring {propertyName} for {sourcePath}");

                var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
                Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                    $"real asset editor settings add smoke should reload restored on-disk state for {sourcePath}");
                if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
                {
                    AssertRealAssetSettingMissingSnapshot(
                        reloadedAfterUndo.Document,
                        propertyName,
                        expectedSectionCount,
                        expectLabel,
                        $"reloaded undone editor real asset settings snapshot should keep {propertyName} absent");
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
