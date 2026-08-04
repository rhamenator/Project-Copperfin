
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
    private static void SmokeAssetEditorSectionRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        string propertyName,
        object updatedPropertyValue,
        string expectedOriginalSelectionValue,
        string expectedUpdatedSelectionValue,
        string expectedOriginalRawValue,
        string expectedUpdatedRawValue,
        int? expectedOriginalLayoutValue,
        int? expectedUpdatedLayoutValue,
        int expectedSectionCount,
        bool expectLabel,
        int expectedObjectCount,
        string? expectedExplorerSectionTitle = null,
        ExpectedSectionGroupingMetadata? expectedGrouping = null,
        ExpectedUntouchedSectionSnapshot[]? expectedUntouchedSections = null,
        string? expectedUpdatedExplorerSectionTitle = null,
        ExpectedSectionGroupingMetadata? expectedUpdatedGrouping = null,
        ExpectedUntouchedSectionSnapshot[]? expectedUpdatedUntouchedSections = null,
        ExpectedSectionGroupingMetadata? expectedUndoneGrouping = null,
        ExpectedUntouchedSectionSnapshot[]? expectedUndoneUntouchedSections = null,
        string? expectedOriginalLayoutTextValue = null,
        string? expectedUpdatedLayoutTextValue = null,
        ExpectedSectionContainedObjectGeometry[]? expectedOriginalContainedObjects = null,
        ExpectedSectionContainedObjectGeometry[]? expectedUpdatedContainedObjects = null)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor section candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorSections-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loadedSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loadedSnapshot.Success && loadedSnapshot.Document is not null,
                $"real asset editor section smoke should load baseline snapshot data for {sourcePath}");
            if (loadedSnapshot.Success && loadedSnapshot.Document is not null)
            {
                AssertRealAssetUntouchedSectionsSnapshot(
                    loadedSnapshot.Document,
                    expectedUntouchedSections,
                    $"initial real asset editor section snapshot should preserve sibling rows while editing {propertyName}");
            }

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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared design surface for the real section asset smoke.");
            var expectedSectionListTitle = expectedExplorerSectionTitle ?? expectedSectionTitle;
            var expectedUpdatedSectionListTitle = expectedUpdatedExplorerSectionTitle ?? expectedSectionListTitle;
            expectedUpdatedGrouping ??= expectedGrouping;
            expectedUpdatedUntouchedSections ??= expectedUntouchedSections;
            expectedUndoneGrouping ??= expectedGrouping;
            expectedUndoneUntouchedSections ??= expectedUntouchedSections;

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor section smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor section smoke should start from a section-rooted property-grid selection for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                return;
            }

            var selectionPropertyName = NormalizeSectionSelectionPropertyName(propertyName);
            var initialSelectionValue = TypeDescriptor.GetProperties(sectionSelection)[selectionPropertyName]?.GetValue(sectionSelection)?.ToString();
            Expect(string.Equals(initialSelectionValue, expectedOriginalSelectionValue, StringComparison.Ordinal),
                $"real asset editor section smoke should expose original property-grid value {propertyName} for {sourcePath}");
            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedSectionListTitle, StringComparison.OrdinalIgnoreCase) &&
                   SectionMatchesExpectedGrouping(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioReportSection, expectedGrouping) &&
                   SelectionMatchesExpectedSectionGrouping(sectionSelection, expectedGrouping),
                $"real asset editor section smoke should expose grouped section metadata for {sourcePath}");

            TypeDescriptor.GetProperties(sectionSelection)[selectionPropertyName]?.SetValue(sectionSelection, updatedPropertyValue);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", selectionPropertyName, 0);
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
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[selectionPropertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedUpdatedSectionListTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == recordIndex &&
                           SectionMatchesExpectedGrouping(selectedSectionModel, expectedUpdatedGrouping) &&
                           SelectionMatchesExpectedSectionGrouping(refreshedSelection, expectedUpdatedGrouping) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") &&
                           objectListView.SelectedItems.Count == 0 &&
                           string.Equals(propertyValue, expectedUpdatedSelectionValue, StringComparison.Ordinal);
                });
            Expect(updatedSelection,
                $"real asset editor section smoke should preserve section-rooted continuity after editing {propertyName} for {sourcePath}");

            Expect(control.CanHandleUndoCommand(),
                $"real asset editor section smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor section smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetSectionSnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    expectedSectionTitle,
                    propertyName,
                    expectedUpdatedRawValue,
                    expectedUpdatedLayoutValue,
                    expectedSectionCount,
                    expectLabel,
                    expectedObjectCount,
                    expectedUpdatedGrouping,
                    expectedUpdatedLayoutTextValue,
                    expectedUpdatedContainedObjects,
                    $"reloaded edited real asset section snapshot should preserve {propertyName}");
                AssertRealAssetUntouchedSectionsSnapshot(
                    reloadedAfterUpdate.Document,
                    expectedUpdatedUntouchedSections,
                    $"reloaded edited real asset section snapshot should preserve sibling rows while editing {propertyName}");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor section smoke should execute undo after editing {propertyName} for {sourcePath}");
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
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[selectionPropertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionListTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == recordIndex &&
                           SectionMatchesExpectedGrouping(selectedSectionModel, expectedUndoneGrouping) &&
                           SelectionMatchesExpectedSectionGrouping(refreshedSelection, expectedUndoneGrouping) &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") &&
                           objectListView.SelectedItems.Count == 0 &&
                           string.Equals(propertyValue, expectedOriginalSelectionValue, StringComparison.Ordinal);
                });
            Expect(undoneSelection,
                $"real asset editor section smoke should preserve section-rooted continuity after undoing {propertyName} for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor section smoke should clear undo after restoring {propertyName} for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor section smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetSectionSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    expectedSectionTitle,
                    propertyName,
                    expectedOriginalRawValue,
                    expectedOriginalLayoutValue,
                    expectedSectionCount,
                    expectLabel,
                    expectedObjectCount,
                    expectedUndoneGrouping,
                    expectedOriginalLayoutTextValue,
                    expectedOriginalContainedObjects,
                    $"reloaded undone editor real asset section snapshot should preserve {propertyName}");
                AssertRealAssetUntouchedSectionsSnapshot(
                    reloadedAfterUndo.Document,
                    expectedUndoneUntouchedSections,
                    $"reloaded undone editor real asset section snapshot should preserve sibling rows while editing {propertyName}");
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

    private static void SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionListTitle,
        IReadOnlyList<KeyValuePair<string, string>> expectedProperties,
        IReadOnlyList<string>? expectedMissingProperties = null,
        int expectedObjectListCount = 0)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor section-metadata candidate" : sourcePath)} not found.");
            return;
        }

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
        control.LoadDocument(sourcePath!);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => sectionListView.Items.Count > 0);
        Expect(loaded, $"real asset editor section-metadata smoke should load section data for {sourcePath}");
        if (!loaded)
        {
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = item.Tag is CopperfinStudioReportSection section &&
                            section.RecordIndex == recordIndex;
        }

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        Application.DoEvents();

        var selected = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => propertyGrid.SelectedObject is CopperfinDesignerSelection selection &&
                  selection.RecordIndex == recordIndex &&
                  string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedSectionListTitle, StringComparison.OrdinalIgnoreCase) &&
                  objectListView.Items.Count == expectedObjectListCount);
        Expect(selected,
            $"real asset editor section-metadata smoke should produce a section-rooted selection for {sourcePath}");
        if (!selected || propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
        {
            TearDownForm(hostForm);
            return;
        }

        foreach (var expectedProperty in expectedProperties)
        {
            Expect(string.Equals(
                    ReadSelectionPropertyValue(sectionSelection, expectedProperty.Key),
                    expectedProperty.Value,
                    StringComparison.Ordinal),
                $"real asset editor section-metadata smoke should expose {expectedProperty.Key}={expectedProperty.Value} for {sourcePath}");
        }

        if (expectedMissingProperties is not null)
        {
            foreach (var missingProperty in expectedMissingProperties)
            {
                Expect(
                    TypeDescriptor.GetProperties(sectionSelection)[missingProperty] is null,
                    $"real asset editor section-metadata smoke should omit {missingProperty} for {sourcePath}");
            }
        }

        TearDownForm(hostForm);
    }

    private static void SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
        string? sourcePath,
        int recordIndex,
        IReadOnlyList<KeyValuePair<string, string>> expectedProperties,
        IReadOnlyList<string>? expectedMissingProperties = null,
        int expectedObjectListCount = 0)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted section-metadata candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealDeletedSectionMetadata-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var initialSnapshot = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(initialSnapshot.Success && initialSnapshot.Document?.ReportLayout is not null,
                $"real deleted section-metadata smoke should load initial snapshot data for {sourcePath}");
            if (!initialSnapshot.Success || initialSnapshot.Document?.ReportLayout is null)
            {
                return;
            }

            var liveSection = initialSnapshot.Document.ReportLayout.Sections
                .FirstOrDefault(section => section.RecordIndex == recordIndex);
            Expect(liveSection is not null && !string.IsNullOrWhiteSpace(liveSection.Id),
                $"real deleted section-metadata smoke should resolve section identity for {sourcePath}");
            if (liveSection is null || string.IsNullOrWhiteSpace(liveSection.Id))
            {
                return;
            }

            var deleteResult = CopperfinStudioSnapshotClient.TryUpdateDeletedStates(
                assetPath,
                new[]
                {
                    new KeyValuePair<string, bool>(liveSection.Id, true)
                });
            Expect(deleteResult.Success && deleteResult.Document?.ReportLayout is not null,
                $"real deleted section-metadata smoke should delete section {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document?.ReportLayout is null)
            {
                return;
            }

            var deletedSection = deleteResult.Document.ReportLayout.DeletedSections
                .FirstOrDefault(section => section.RecordIndex == recordIndex);
            Expect(deletedSection is not null,
                $"real deleted section-metadata smoke should surface deleted section {recordIndex} for {sourcePath}");
            if (deletedSection is null)
            {
                return;
            }

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

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Cast<ListViewItem>().Any(item =>
                    item.Tag is CopperfinStudioReportSection section &&
                    section.RecordIndex == recordIndex &&
                    section.Deleted));
            Expect(loaded, $"real deleted section-metadata smoke should load deleted section data for {sourcePath}");
            if (!loaded)
            {
                TearDownForm(hostForm);
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == recordIndex &&
                                section.Deleted;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var expectedSectionListTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            var selected = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => propertyGrid.SelectedObject is CopperfinDesignerSelection selection &&
                      selection.RecordIndex == recordIndex &&
                      string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedSectionListTitle, StringComparison.Ordinal) &&
                      objectListView.Items.Count == expectedObjectListCount);
            Expect(selected,
                $"real deleted section-metadata smoke should produce a deleted section-rooted selection for {sourcePath}");
            if (!selected || propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                TearDownForm(hostForm);
                return;
            }

            foreach (var expectedProperty in expectedProperties)
            {
                Expect(string.Equals(
                        ReadSelectionPropertyValue(sectionSelection, expectedProperty.Key),
                        expectedProperty.Value,
                        StringComparison.Ordinal),
                    $"real deleted section-metadata smoke should expose {expectedProperty.Key}={expectedProperty.Value} for {sourcePath}");
            }

            if (expectedMissingProperties is not null)
            {
                foreach (var missingProperty in expectedMissingProperties)
                {
                    Expect(
                        TypeDescriptor.GetProperties(sectionSelection)[missingProperty] is null,
                        $"real deleted section-metadata smoke should omit {missingProperty} for {sourcePath}");
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

    private static void SmokeAssetEditorBatchPropertyRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges,
        IReadOnlyList<KeyValuePair<string, string>> expectedUpdatedSelectionValues,
        IReadOnlyList<KeyValuePair<string, string>> expectedOriginalRawValues,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor batch candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorBatchWrites-" + Guid.NewGuid().ToString("N"));
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
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real batch smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor batch smoke should load section data for {sourcePath}");
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
            Expect(objectLoaded, $"real asset editor batch smoke should surface object {recordIndex} for {sourcePath}");
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
                $"real asset editor batch smoke should start from an object-rooted property-grid selection for {sourcePath}");

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

                    if (!string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) ||
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") != recordIndex ||
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
                $"real asset editor batch smoke should preserve section/object continuity after applying the batch update for {sourcePath}");

            Expect(control.CanHandleUndoCommand(),
                $"real asset editor batch smoke should expose undo after applying the batch update for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor batch smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                foreach (var property in propertyChanges)
                {
                    AssertRealAssetRoundTripSnapshot(
                        reloadedAfterUpdate.Document,
                        recordIndex,
                        property.Key,
                        property.Value,
                        expectedObjectTitle,
                        expectedSectionTitle,
                        expectedSectionCount,
                        expectLabel,
                        expectUnplacedObject: false,
                        $"reloaded edited real asset batch snapshot should preserve {property.Key}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor batch smoke should execute one command undo after applying the batch update for {sourcePath}");
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
                    if (!string.Equals(selectedSection, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) ||
                        selectedObject?.RecordIndex != recordIndex ||
                        !string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) ||
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") != recordIndex ||
                        ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))
                    {
                        return false;
                    }

                    return true;
                });
            Expect(undoneSelection,
                $"real asset editor batch smoke should preserve section/object continuity after the command undo for {sourcePath}");

            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor batch smoke should clear undo after restoring the batch update for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor batch smoke should reload restored on-disk state after the command undo for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                foreach (var property in expectedOriginalRawValues)
                {
                    AssertRealAssetRoundTripSnapshot(
                        reloadedAfterUndo.Document,
                        recordIndex,
                        property.Key,
                        property.Value,
                        expectedObjectTitle,
                        expectedSectionTitle,
                        expectedSectionCount,
                        expectLabel,
                        expectUnplacedObject: false,
                        $"reloaded undone editor real asset batch snapshot should preserve {property.Key}");
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
