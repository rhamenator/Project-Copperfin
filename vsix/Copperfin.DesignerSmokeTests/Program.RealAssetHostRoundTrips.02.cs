
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
    private static void SmokeRealAssetHostBackedSectionRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        string propertyName,
        string originalRawValue,
        string updatedRawValue,
        int? expectedOriginalLayoutValue,
        int? expectedUpdatedLayoutValue,
        int expectedSectionCount,
        bool expectLabel,
        int expectedObjectCount,
        ExpectedSectionGroupingMetadata? expectedGrouping = null,
        ExpectedUntouchedSectionSnapshot[]? expectedUntouchedSections = null,
        ExpectedSectionGroupingMetadata? expectedUpdatedGrouping = null,
        ExpectedUntouchedSectionSnapshot[]? expectedUpdatedUntouchedSections = null,
        ExpectedSectionGroupingMetadata? expectedUndoneGrouping = null,
        ExpectedUntouchedSectionSnapshot[]? expectedUndoneUntouchedSections = null,
        string? expectedOriginalLayoutTextValue = null,
        string? expectedUpdatedLayoutTextValue = null,
        ExpectedSectionContainedObjectGeometry[]? expectedOriginalContainedObjects = null,
        ExpectedSectionContainedObjectGeometry[]? expectedUpdatedContainedObjects = null,
        ExpectedPreviewBoundsGeometry? expectedUpdatedPreviewBounds = null)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset section write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetSectionWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset section smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            var expectedPreviewBoundsLeft = loaded.Document.ReportLayout?.PreviewBoundsLeft ?? 0;
            var expectedPreviewBoundsTop = loaded.Document.ReportLayout?.PreviewBoundsTop ?? 0;
            var expectedPreviewBoundsRight = loaded.Document.ReportLayout?.PreviewBoundsRight ?? 0;
            var expectedPreviewBoundsBottom = loaded.Document.ReportLayout?.PreviewBoundsBottom ?? 0;
            var expectedPreviewBoundsWidth = loaded.Document.ReportLayout?.PreviewBoundsWidth ?? 0;
            var expectedPreviewBoundsHeight = loaded.Document.ReportLayout?.PreviewBoundsHeight ?? 0;
            if (expectedUpdatedPreviewBounds is not null)
            {
                Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                    $"initial real asset section snapshot should preserve live preview bounds for {sourcePath}");
                if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
                {
                    return;
                }

                AssertRealAssetLivePreviewBounds(
                    loaded.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"initial real asset section snapshot should preserve live preview metadata");
            }

            AssertRealAssetSectionSnapshot(
                loaded.Document,
                recordIndex,
                expectedSectionTitle,
                propertyName,
                originalRawValue,
                expectedOriginalLayoutValue,
                expectedSectionCount,
                expectLabel,
                expectedObjectCount,
                expectedGrouping,
                expectedOriginalLayoutTextValue,
                expectedOriginalContainedObjects,
                $"initial real asset section snapshot should preserve {propertyName}");
            AssertRealAssetUntouchedSectionsSnapshot(
                loaded.Document,
                expectedUntouchedSections,
                $"initial real asset section snapshot should preserve sibling rows while editing {propertyName}");

            expectedUpdatedGrouping ??= expectedGrouping;
            expectedUpdatedUntouchedSections ??= expectedUntouchedSections;
            expectedUndoneGrouping ??= expectedGrouping;
            expectedUndoneUntouchedSections ??= expectedUntouchedSections;

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                recordIndex,
                propertyName,
                updatedRawValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset section smoke should update {propertyName} for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetSectionSnapshot(
                updateResult.Document,
                recordIndex,
                expectedSectionTitle,
                propertyName,
                updatedRawValue,
                expectedUpdatedLayoutValue,
                expectedSectionCount,
                expectLabel,
                expectedObjectCount,
                expectedUpdatedGrouping,
                expectedUpdatedLayoutTextValue,
                expectedUpdatedContainedObjects,
                $"updated real asset section snapshot should preserve {propertyName}");
            AssertRealAssetUntouchedSectionsSnapshot(
                updateResult.Document,
                expectedUpdatedUntouchedSections,
                $"updated real asset section snapshot should preserve sibling rows while editing {propertyName}");
            if (expectedUpdatedPreviewBounds is not null)
            {
                AssertRealAssetLivePreviewBounds(
                    updateResult.Document,
                    expectedUpdatedPreviewBounds.Left,
                    expectedUpdatedPreviewBounds.Top,
                    expectedUpdatedPreviewBounds.Right,
                    expectedUpdatedPreviewBounds.Bottom,
                    expectedUpdatedPreviewBounds.Width,
                    expectedUpdatedPreviewBounds.Height,
                    $"updated real asset section snapshot should preserve live preview metadata");
            }
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset section smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset section smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetSectionSnapshot(
                reloadedAfterUpdate.Document,
                recordIndex,
                expectedSectionTitle,
                propertyName,
                updatedRawValue,
                expectedUpdatedLayoutValue,
                expectedSectionCount,
                expectLabel,
                expectedObjectCount,
                expectedUpdatedGrouping,
                expectedUpdatedLayoutTextValue,
                expectedUpdatedContainedObjects,
                $"reloaded updated real asset section snapshot should preserve {propertyName}");
            AssertRealAssetUntouchedSectionsSnapshot(
                reloadedAfterUpdate.Document,
                expectedUpdatedUntouchedSections,
                $"reloaded updated real asset section snapshot should preserve sibling rows while editing {propertyName}");
            if (expectedUpdatedPreviewBounds is not null)
            {
                AssertRealAssetLivePreviewBounds(
                    reloadedAfterUpdate.Document,
                    expectedUpdatedPreviewBounds.Left,
                    expectedUpdatedPreviewBounds.Top,
                    expectedUpdatedPreviewBounds.Right,
                    expectedUpdatedPreviewBounds.Bottom,
                    expectedUpdatedPreviewBounds.Width,
                    expectedUpdatedPreviewBounds.Height,
                    $"reloaded updated real asset section snapshot should preserve live preview metadata");
            }
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset section snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset section smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }
            if (expectedUpdatedPreviewBounds is not null)
            {
                AssertRealAssetLivePreviewBounds(
                    undoResult.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"undone real asset section snapshot should preserve live preview metadata");
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset section smoke should reload undone snapshot data for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetSectionSnapshot(
                reloadedAfterUndo.Document,
                recordIndex,
                expectedSectionTitle,
                propertyName,
                originalRawValue,
                expectedOriginalLayoutValue,
                expectedSectionCount,
                expectLabel,
                expectedObjectCount,
                expectedUndoneGrouping,
                expectedOriginalLayoutTextValue,
                expectedOriginalContainedObjects,
                $"reloaded undone real asset section snapshot should preserve {propertyName}");
            AssertRealAssetUntouchedSectionsSnapshot(
                reloadedAfterUndo.Document,
                expectedUndoneUntouchedSections,
                $"reloaded undone real asset section snapshot should preserve sibling rows while editing {propertyName}");
            if (expectedUpdatedPreviewBounds is not null)
            {
                AssertRealAssetLivePreviewBounds(
                    reloadedAfterUndo.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"reloaded undone real asset section snapshot should preserve live preview metadata");
            }
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset section smoke should clear undo after restoring {propertyName} for {sourcePath}");
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

    private static void SmokeRealAssetHostBackedBatchPropertyRoundTrip(
        string? sourcePath,
        int recordIndex,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges,
        IReadOnlyList<KeyValuePair<string, string>> originalValues,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        ExpectedPreviewBoundsGeometry? expectedUpdatedPreviewBounds = null)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset batch write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetBatchWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset batch smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            var expectedPreviewBoundsLeft = loaded.Document.ReportLayout?.PreviewBoundsLeft ?? 0;
            var expectedPreviewBoundsTop = loaded.Document.ReportLayout?.PreviewBoundsTop ?? 0;
            var expectedPreviewBoundsRight = loaded.Document.ReportLayout?.PreviewBoundsRight ?? 0;
            var expectedPreviewBoundsBottom = loaded.Document.ReportLayout?.PreviewBoundsBottom ?? 0;
            var expectedPreviewBoundsWidth = loaded.Document.ReportLayout?.PreviewBoundsWidth ?? 0;
            var expectedPreviewBoundsHeight = loaded.Document.ReportLayout?.PreviewBoundsHeight ?? 0;
            if (expectedUpdatedPreviewBounds is not null)
            {
                Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                    $"initial real asset batch snapshot should preserve live preview bounds for {sourcePath}");
                if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
                {
                    return;
                }

                AssertRealAssetLivePreviewBounds(
                    loaded.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"initial real asset batch snapshot should preserve live preview metadata");
            }

            foreach (var property in originalValues)
            {
                AssertRealAssetRoundTripSnapshot(
                    loaded.Document,
                    recordIndex,
                    property.Key,
                    property.Value,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"initial real asset batch snapshot should preserve {property.Key}");
            }

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperties(assetPath, recordIndex, propertyChanges);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset batch smoke should update {propertyChanges.Count} properties for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            foreach (var property in propertyChanges)
            {
                AssertRealAssetRoundTripSnapshot(
                    updateResult.Document,
                    recordIndex,
                    property.Key,
                    property.Value,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"updated real asset batch snapshot should preserve {property.Key}");
            }
            if (expectedUpdatedPreviewBounds is not null)
            {
                AssertRealAssetLivePreviewBounds(
                    updateResult.Document,
                    expectedUpdatedPreviewBounds.Left,
                    expectedUpdatedPreviewBounds.Top,
                    expectedUpdatedPreviewBounds.Right,
                    expectedUpdatedPreviewBounds.Bottom,
                    expectedUpdatedPreviewBounds.Width,
                    expectedUpdatedPreviewBounds.Height,
                    $"updated real asset batch snapshot should preserve live preview metadata");
            }
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset batch smoke should expose undo after updating {propertyChanges.Count} properties for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset batch smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

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
                    $"reloaded updated real asset batch snapshot should preserve {property.Key}");
            }
            if (expectedUpdatedPreviewBounds is not null)
            {
                AssertRealAssetLivePreviewBounds(
                    reloadedAfterUpdate.Document,
                    expectedUpdatedPreviewBounds.Left,
                    expectedUpdatedPreviewBounds.Top,
                    expectedUpdatedPreviewBounds.Right,
                    expectedUpdatedPreviewBounds.Bottom,
                    expectedUpdatedPreviewBounds.Width,
                    expectedUpdatedPreviewBounds.Height,
                    $"reloaded updated real asset batch snapshot should preserve live preview metadata");
            }
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset batch snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset batch smoke should undo {propertyChanges.Count} properties in one command for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }
            if (expectedUpdatedPreviewBounds is not null)
            {
                AssertRealAssetLivePreviewBounds(
                    undoResult.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"undone real asset batch snapshot should preserve live preview metadata");
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset batch smoke should reload restored snapshot data after the command undo for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            foreach (var property in originalValues)
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
                    $"reloaded undone real asset batch snapshot should preserve {property.Key}");
            }
            if (expectedUpdatedPreviewBounds is not null)
            {
                AssertRealAssetLivePreviewBounds(
                    reloadedAfterUndo.Document,
                    expectedPreviewBoundsLeft,
                    expectedPreviewBoundsTop,
                    expectedPreviewBoundsRight,
                    expectedPreviewBoundsBottom,
                    expectedPreviewBoundsWidth,
                    expectedPreviewBoundsHeight,
                    $"reloaded undone real asset batch snapshot should preserve live preview metadata");
            }

            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset batch smoke should clear undo after restoring the batch update for {sourcePath}");
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

    private static void SmokeRealAssetHostBackedPlacementRoundTrip(
        string? sourcePath,
        int recordIndex,
        string propertyName,
        string originalValue,
        string updatedValue,
        string expectedObjectTitle,
        string initialSectionTitle,
        string updatedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalUnplacedObjectCount,
        int expectedUpdatedUnplacedObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset placement candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetPlacements-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset placement smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial real asset placement snapshot should preserve live preview bounds for {sourcePath}");
            if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                propertyName,
                originalValue,
                expectedObjectTitle,
                initialSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: true,
                $"initial real asset placement snapshot should preserve {propertyName}");
            AssertRealAssetUnplacedObjectCount(
                loaded.Document,
                expectedOriginalUnplacedObjectCount,
                $"initial real asset placement snapshot should preserve unplaced-object counts");
            var initialLayout = loaded.Document.ReportLayout;
            var expectedPreviewBoundsLeft = initialLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = initialLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = initialLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = initialLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = initialLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = initialLayout.PreviewBoundsHeight;
            AssertRealAssetPreviewBoundsGeometry(
                loaded.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"initial real asset placement snapshot should preserve live preview metadata");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                recordIndex,
                propertyName,
                updatedValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset placement smoke should update {propertyName} for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                updateResult.Document,
                recordIndex,
                propertyName,
                updatedValue,
                expectedObjectTitle,
                updatedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"updated real asset placement snapshot should preserve {propertyName}");
            AssertRealAssetUnplacedObjectCount(
                updateResult.Document,
                expectedUpdatedUnplacedObjectCount,
                $"updated real asset placement snapshot should preserve unplaced-object counts");
            AssertRealAssetPreviewBoundsGeometry(
                updateResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"updated real asset placement snapshot should preserve live preview metadata");
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset placement smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset placement smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterUpdate.Document,
                recordIndex,
                propertyName,
                updatedValue,
                expectedObjectTitle,
                updatedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded updated real asset placement snapshot should preserve {propertyName}");
            AssertRealAssetUnplacedObjectCount(
                reloadedAfterUpdate.Document,
                expectedUpdatedUnplacedObjectCount,
                $"reloaded updated real asset placement snapshot should preserve unplaced-object counts");
            AssertRealAssetPreviewBoundsGeometry(
                reloadedAfterUpdate.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded updated real asset placement snapshot should preserve live preview metadata");
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset placement snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset placement smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset placement smoke should reload undone snapshot data for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterUndo.Document,
                recordIndex,
                propertyName,
                originalValue,
                expectedObjectTitle,
                initialSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: true,
                $"reloaded undone real asset placement snapshot should preserve {propertyName}");
            AssertRealAssetUnplacedObjectCount(
                reloadedAfterUndo.Document,
                expectedOriginalUnplacedObjectCount,
                $"reloaded undone real asset placement snapshot should preserve unplaced-object counts");
            AssertRealAssetPreviewBoundsGeometry(
                reloadedAfterUndo.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded undone real asset placement snapshot should preserve live preview metadata");
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset placement smoke should clear undo after restoring {propertyName} for {sourcePath}");
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
