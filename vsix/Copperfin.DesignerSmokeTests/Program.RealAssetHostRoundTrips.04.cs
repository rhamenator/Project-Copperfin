
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
    private static void SmokeRealAssetHostBackedDeletedBatchPropertyRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges,
        IReadOnlyList<KeyValuePair<string, string>> originalRawValues,
        IReadOnlyList<KeyValuePair<string, int?>> originalLayoutValues,
        IReadOnlyList<KeyValuePair<string, int?>> updatedLayoutValues,
        int expectedSectionCount,
        bool expectLabel,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted batch candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletedBatchWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real deleted batch smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial deleted batch snapshot should preserve live preview bounds for {sourcePath}");
            if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
            {
                return;
            }

            var initialLayout = loaded.Document.ReportLayout;
            var expectedPreviewBoundsLeft = initialLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = initialLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = initialLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = initialLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = initialLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = initialLayout.PreviewBoundsHeight;

            var deleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(deleteResult.Success && deleteResult.Document is not null,
                $"real deleted batch smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            foreach (var property in originalRawValues)
            {
                var expectedLayoutValue = originalLayoutValues
                    .FirstOrDefault(candidate => string.Equals(candidate.Key, property.Key, StringComparison.OrdinalIgnoreCase))
                    .Value;
                AssertRealAssetDeletedObjectPropertySnapshot(
                    deleteResult.Document,
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
                    $"initial deleted real asset batch snapshot should preserve {property.Key}");
            }
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                deleteResult.Document,
                $"initial deleted real asset batch snapshot should preserve deleted preview metadata");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperties(assetPath, recordIndex, propertyChanges);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real deleted batch smoke should update {propertyChanges.Count} properties for deleted record {recordIndex} in {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            foreach (var property in propertyChanges)
            {
                var expectedLayoutValue = updatedLayoutValues
                    .FirstOrDefault(candidate => string.Equals(candidate.Key, property.Key, StringComparison.OrdinalIgnoreCase))
                    .Value;
                AssertRealAssetDeletedObjectPropertySnapshot(
                    updateResult.Document,
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
                    $"updated deleted real asset batch snapshot should preserve {property.Key}");
            }
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                updateResult.Document,
                $"updated deleted real asset batch snapshot should preserve deleted preview metadata");
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real deleted batch smoke should expose undo after updating {propertyChanges.Count} properties for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real deleted batch smoke should reload updated deleted snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            foreach (var property in propertyChanges)
            {
                var expectedLayoutValue = updatedLayoutValues
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
                    $"reloaded updated deleted real asset batch snapshot should preserve {property.Key}");
            }
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                reloadedAfterUpdate.Document,
                $"reloaded updated deleted real asset batch snapshot should preserve deleted preview metadata");
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded deleted batch snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real deleted batch smoke should undo {propertyChanges.Count} properties in one command for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            foreach (var property in originalRawValues)
            {
                var expectedLayoutValue = originalLayoutValues
                    .FirstOrDefault(candidate => string.Equals(candidate.Key, property.Key, StringComparison.OrdinalIgnoreCase))
                    .Value;
                AssertRealAssetDeletedObjectPropertySnapshot(
                    undoResult.Document,
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
                    $"undone deleted real asset batch snapshot should preserve {property.Key}");
            }
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                undoResult.Document,
                $"undone deleted real asset batch snapshot should preserve deleted preview metadata");
            Expect(undoResult.Document.CommandUndoAvailable,
                $"undone deleted batch snapshot should retain the earlier delete-state undo for {sourcePath}");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real deleted batch smoke should restore record {recordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            foreach (var property in originalRawValues)
            {
                AssertRealAssetRoundTripSnapshot(
                    restoreResult.Document,
                    recordIndex,
                    property.Key,
                    property.Value,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"restored deleted-batch real asset snapshot should preserve {property.Key}");
            }
            AssertRealAssetLivePreviewBounds(
                restoreResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"restored deleted-batch real asset snapshot should preserve live preview metadata");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real deleted batch smoke should reload restored live snapshot data for {sourcePath}");
            if (!reloadedAfterRestore.Success || reloadedAfterRestore.Document is null)
            {
                return;
            }

            foreach (var property in originalRawValues)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterRestore.Document,
                    recordIndex,
                    property.Key,
                    property.Value,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded restored deleted-batch real asset snapshot should preserve {property.Key}");
            }
            AssertRealAssetLivePreviewBounds(
                reloadedAfterRestore.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded restored deleted-batch real asset snapshot should preserve live preview metadata");
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

    private static void SmokeRealAssetHostBackedDeletedRenameRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedOriginalUniqueId,
        string expectedRenamedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted rename candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletedRenames-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real deleted rename smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial deleted rename snapshot should preserve live preview bounds for {sourcePath}");
            if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
            {
                return;
            }

            var initialLayout = loaded.Document.ReportLayout;
            var expectedPreviewBoundsLeft = initialLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = initialLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = initialLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = initialLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = initialLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = initialLayout.PreviewBoundsHeight;

            var deleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId);
            Expect(deleteResult.Success && deleteResult.Document is not null,
                $"real deleted rename smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                deleteResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "initial deleted real asset rename snapshot should preserve original identity");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                deleteResult.Document,
                "initial deleted real asset rename snapshot should preserve deleted preview metadata");

            var renameResult = CopperfinStudioSnapshotClient.TryRenameObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId,
                expectedRenamedUniqueId);
            Expect(renameResult.Success && renameResult.Document is not null,
                $"real deleted rename smoke should rename record {recordIndex} for {sourcePath}");
            if (!renameResult.Success || renameResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                renameResult.Document,
                recordIndex,
                expectedRenamedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "renamed deleted real asset snapshot should preserve renamed identity");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                renameResult.Document,
                "renamed deleted real asset snapshot should preserve deleted preview metadata");
            Expect(renameResult.Document.CommandUndoAvailable,
                $"real deleted rename smoke should expose undo after renaming {recordIndex} for {sourcePath}");

            var reloadedAfterRename = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRename.Success && reloadedAfterRename.Document is not null,
                $"real deleted rename smoke should reload renamed deleted snapshot data for {sourcePath}");
            if (!reloadedAfterRename.Success || reloadedAfterRename.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterRename.Document,
                recordIndex,
                expectedRenamedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "reloaded renamed deleted real asset snapshot should preserve renamed identity");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                reloadedAfterRename.Document,
                "reloaded renamed deleted real asset snapshot should preserve deleted preview metadata");
            Expect(reloadedAfterRename.Document.CommandUndoAvailable,
                $"reloaded deleted rename snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real deleted rename smoke should undo rename for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                undoResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "undone deleted real asset rename snapshot should preserve original identity");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                undoResult.Document,
                "undone deleted real asset rename snapshot should preserve deleted preview metadata");
            Expect(undoResult.Document.CommandUndoAvailable,
                $"undone deleted rename snapshot should retain the earlier delete-state undo for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real deleted rename smoke should reload restored deleted identity for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterUndo.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "reloaded undone deleted real asset rename snapshot should preserve original identity");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                reloadedAfterUndo.Document,
                "reloaded undone deleted real asset rename snapshot should preserve deleted preview metadata");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real deleted rename smoke should restore record {recordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                restoreResult.Document,
                recordIndex,
                "UNIQUEID",
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                "restored deleted-rename real asset snapshot should preserve original identity");
            AssertRealAssetLivePreviewBounds(
                restoreResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                "restored deleted-rename real asset snapshot should preserve live preview metadata");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real deleted rename smoke should reload restored live snapshot data for {sourcePath}");
            if (!reloadedAfterRestore.Success || reloadedAfterRestore.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterRestore.Document,
                recordIndex,
                "UNIQUEID",
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                "reloaded restored deleted-rename real asset snapshot should preserve original identity");
            AssertRealAssetLivePreviewBounds(
                reloadedAfterRestore.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                "reloaded restored deleted-rename real asset snapshot should preserve live preview metadata");
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

    private static void SmokeRealAssetHostBackedDeletedDuplicateRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedOriginalUniqueId,
        string expectedDuplicatedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted duplicate candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletedDuplicates-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real deleted duplicate smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial deleted duplicate snapshot should preserve live preview bounds for {sourcePath}");
            if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
            {
                return;
            }

            var initialLayout = loaded.Document.ReportLayout;
            var expectedPreviewBoundsLeft = initialLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = initialLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = initialLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = initialLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = initialLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = initialLayout.PreviewBoundsHeight;

            var deleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId);
            Expect(deleteResult.Success && deleteResult.Document is not null,
                $"real deleted duplicate smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                deleteResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 1,
                "initial deleted real asset duplicate snapshot should preserve original identity");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                deleteResult.Document,
                "initial deleted real asset duplicate snapshot should preserve deleted preview metadata");

            var duplicateResult = CopperfinStudioSnapshotClient.TryDuplicateObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId,
                expectedDuplicatedUniqueId);
            Expect(duplicateResult.Success && duplicateResult.Document is not null,
                $"real deleted duplicate smoke should duplicate deleted record {recordIndex} for {sourcePath}");
            if (!duplicateResult.Success || duplicateResult.Document is null)
            {
                return;
            }

            var duplicatedObject = FindSnapshotObjectByUniqueId(duplicateResult.Document, expectedDuplicatedUniqueId);
            Expect(duplicatedObject is not null,
                $"real deleted duplicate smoke should surface duplicated deleted UNIQUEID {expectedDuplicatedUniqueId} for {sourcePath}");
            if (duplicatedObject is null)
            {
                return;
            }

            Expect(duplicatedObject.RecordIndex != recordIndex,
                $"real deleted duplicate smoke should assign a distinct record to the duplicated deleted row for {sourcePath}");

            AssertRealAssetDeletedObjectSnapshot(
                duplicateResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                expectedDeletedSectionObjectCount: 2,
                "duplicated deleted real asset snapshot should preserve original deleted identity");
            AssertRealAssetDeletedObjectSnapshot(
                duplicateResult.Document,
                duplicatedObject.RecordIndex,
                expectedDuplicatedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                expectedDeletedSectionObjectCount: 2,
                "duplicated deleted real asset snapshot should preserve duplicated deleted identity");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                duplicateResult.Document,
                "duplicated deleted real asset snapshot should preserve deleted preview metadata");
            Expect(duplicateResult.Document.CommandUndoAvailable,
                $"real deleted duplicate smoke should retain the earlier delete-state undo after duplicating a deleted row for {sourcePath}");

            var reloadedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null,
                $"real deleted duplicate smoke should reload duplicated deleted snapshot data for {sourcePath}");
            if (!reloadedAfterDuplicate.Success || reloadedAfterDuplicate.Document is null)
            {
                return;
            }

            var reloadedDuplicatedObject = FindSnapshotObjectByUniqueId(reloadedAfterDuplicate.Document, expectedDuplicatedUniqueId);
            Expect(reloadedDuplicatedObject is not null,
                $"reloaded deleted duplicate snapshot should preserve duplicated UNIQUEID {expectedDuplicatedUniqueId} for {sourcePath}");
            if (reloadedDuplicatedObject is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterDuplicate.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                expectedDeletedSectionObjectCount: 2,
                "reloaded duplicated deleted real asset snapshot should preserve original deleted identity");
            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterDuplicate.Document,
                reloadedDuplicatedObject.RecordIndex,
                expectedDuplicatedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                expectedDeletedSectionObjectCount: 2,
                "reloaded duplicated deleted real asset snapshot should preserve duplicated deleted identity");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                reloadedAfterDuplicate.Document,
                "reloaded duplicated deleted real asset snapshot should preserve deleted preview metadata");
            Expect(reloadedAfterDuplicate.Document.CommandUndoAvailable,
                $"reloaded deleted duplicate snapshot should retain the earlier delete-state undo for {sourcePath}");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                duplicatedObject.RecordIndex,
                expectedDuplicatedUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real deleted duplicate smoke should restore duplicated record {duplicatedObject.RecordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                restoreResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                "restored deleted duplicate snapshot should preserve the original deleted source row");
            AssertRealAssetRoundTripSnapshot(
                restoreResult.Document,
                duplicatedObject.RecordIndex,
                "UNIQUEID",
                expectedDuplicatedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                "restored deleted duplicate snapshot should preserve the restored duplicated identity");
            AssertRealAssetPreviewBoundsGeometry(
                restoreResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                "restored deleted duplicate snapshot should preserve live preview metadata");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                restoreResult.Document,
                "restored deleted duplicate snapshot should preserve remaining deleted preview metadata");
            AssertRealAssetSectionObjectCount(
                restoreResult.Document,
                expectedSectionTitle,
                expectedObjectCount: 1,
                "restored deleted duplicate snapshot should preserve section live object counts");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real deleted duplicate smoke should reload restored live snapshot data for {sourcePath}");
            if (!reloadedAfterRestore.Success || reloadedAfterRestore.Document is null)
            {
                return;
            }

            var reloadedRestoredObject = FindSnapshotObjectByUniqueId(reloadedAfterRestore.Document, expectedDuplicatedUniqueId);
            Expect(reloadedRestoredObject is not null,
                $"reloaded restored deleted duplicate snapshot should preserve duplicated UNIQUEID {expectedDuplicatedUniqueId} for {sourcePath}");
            if (reloadedRestoredObject is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterRestore.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                "reloaded restored deleted duplicate snapshot should preserve the original deleted source row");
            AssertRealAssetRoundTripSnapshot(
                reloadedAfterRestore.Document,
                reloadedRestoredObject.RecordIndex,
                "UNIQUEID",
                expectedDuplicatedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                "reloaded restored deleted duplicate snapshot should preserve the restored duplicated identity");
            AssertRealAssetPreviewBoundsGeometry(
                reloadedAfterRestore.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                "reloaded restored deleted duplicate snapshot should preserve live preview metadata");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                reloadedAfterRestore.Document,
                "reloaded restored deleted duplicate snapshot should preserve remaining deleted preview metadata");
            AssertRealAssetSectionObjectCount(
                reloadedAfterRestore.Document,
                expectedSectionTitle,
                expectedObjectCount: 1,
                "reloaded restored deleted duplicate snapshot should preserve section live object counts");
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
