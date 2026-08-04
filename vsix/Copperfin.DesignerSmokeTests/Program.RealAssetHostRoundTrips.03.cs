
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
    private static void SmokeRealAssetHostBackedDuplicateRoundTrip(
        string? sourcePath,
        int recordIndex,
        string duplicateUniqueId,
        string expectedSourceObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalSectionObjectCount,
        int expectedUpdatedSectionObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset duplicate candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDuplicates-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset duplicate smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial real asset duplicate snapshot should preserve live preview bounds for {sourcePath}");
            if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
            {
                return;
            }

            var sourceObject = loaded.Document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
            Expect(sourceObject is not null,
                $"real asset duplicate smoke should expose source object {recordIndex} for {sourcePath}");
            if (sourceObject is null)
            {
                return;
            }

            var sourceUniqueId = TryGetSnapshotObjectPropertyValue(sourceObject, "UNIQUEID");
            Expect(!string.IsNullOrWhiteSpace(sourceUniqueId),
                $"real asset duplicate smoke should expose source UNIQUEID for {sourcePath}");
            if (string.IsNullOrWhiteSpace(sourceUniqueId))
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                "UNIQUEID",
                sourceUniqueId!,
                expectedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"initial real asset duplicate snapshot should preserve source object identity");
            AssertRealAssetSectionObjectCount(
                loaded.Document,
                expectedSectionTitle,
                expectedOriginalSectionObjectCount,
                $"initial real asset duplicate snapshot should preserve section object counts");
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
                $"initial real asset duplicate snapshot should preserve live preview metadata");

            var duplicateResult = CopperfinStudioSnapshotClient.TryDuplicateObject(
                assetPath,
                recordIndex,
                sourceUniqueId,
                duplicateUniqueId);
            Expect(duplicateResult.Success && duplicateResult.Document is not null,
                $"real asset duplicate smoke should duplicate record {recordIndex} for {sourcePath}");
            if (!duplicateResult.Success || duplicateResult.Document is null)
            {
                return;
            }

            var duplicatedObject = FindSnapshotObjectByUniqueId(duplicateResult.Document, duplicateUniqueId);
            Expect(duplicatedObject is not null,
                $"real asset duplicate smoke should surface the duplicated UNIQUEID for {sourcePath}");
            if (duplicatedObject is null)
            {
                return;
            }

            Expect(duplicatedObject.RecordIndex != recordIndex,
                $"real asset duplicate smoke should assign a distinct record to the duplicated object for {sourcePath}");
            AssertRealAssetRoundTripSnapshot(
                duplicateResult.Document,
                duplicatedObject.RecordIndex,
                "UNIQUEID",
                duplicateUniqueId,
                expectedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"updated real asset duplicate snapshot should preserve duplicated object identity");
            AssertRealAssetSectionObjectCount(
                duplicateResult.Document,
                expectedSectionTitle,
                expectedUpdatedSectionObjectCount,
                $"updated real asset duplicate snapshot should preserve section object counts");
            AssertRealAssetPreviewBoundsGeometry(
                duplicateResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"updated real asset duplicate snapshot should preserve live preview metadata");

            var reloadedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null,
                $"real asset duplicate smoke should reload duplicated snapshot data for {sourcePath}");
            if (!reloadedAfterDuplicate.Success || reloadedAfterDuplicate.Document is null)
            {
                return;
            }

            var reloadedDuplicatedObject = FindSnapshotObjectByUniqueId(reloadedAfterDuplicate.Document, duplicateUniqueId);
            Expect(reloadedDuplicatedObject is not null,
                $"reloaded real asset duplicate snapshot should preserve the duplicated UNIQUEID for {sourcePath}");
            if (reloadedDuplicatedObject is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterDuplicate.Document,
                reloadedDuplicatedObject.RecordIndex,
                "UNIQUEID",
                duplicateUniqueId,
                expectedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded real asset duplicate snapshot should preserve duplicated object identity");
            AssertRealAssetSectionObjectCount(
                reloadedAfterDuplicate.Document,
                expectedSectionTitle,
                expectedUpdatedSectionObjectCount,
                $"reloaded real asset duplicate snapshot should preserve section object counts");
            AssertRealAssetPreviewBoundsGeometry(
                reloadedAfterDuplicate.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded real asset duplicate snapshot should preserve live preview metadata");
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

    private static void SmokeRealAssetHostBackedRenameRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedOriginalUniqueId,
        string expectedRenamedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset rename candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetRenames-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset rename smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial real asset rename snapshot should preserve live preview bounds for {sourcePath}");
            if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                "UNIQUEID",
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"initial real asset rename snapshot should preserve source object identity");
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
                $"initial real asset rename snapshot should preserve live preview metadata");

            var renameResult = CopperfinStudioSnapshotClient.TryRenameObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId,
                expectedRenamedUniqueId);
            Expect(renameResult.Success && renameResult.Document is not null,
                $"real asset rename smoke should rename record {recordIndex} for {sourcePath}");
            if (!renameResult.Success || renameResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                renameResult.Document,
                recordIndex,
                "UNIQUEID",
                expectedRenamedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"renamed real asset snapshot should preserve renamed identity");
            AssertRealAssetPreviewBoundsGeometry(
                renameResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"renamed real asset snapshot should preserve live preview metadata");
            Expect(renameResult.Document.CommandUndoAvailable,
                $"real asset rename smoke should expose undo after renaming {recordIndex} for {sourcePath}");

            var reloadedAfterRename = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRename.Success && reloadedAfterRename.Document is not null,
                $"real asset rename smoke should reload renamed snapshot data for {sourcePath}");
            if (!reloadedAfterRename.Success || reloadedAfterRename.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterRename.Document,
                recordIndex,
                "UNIQUEID",
                expectedRenamedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded renamed real asset snapshot should preserve renamed identity");
            AssertRealAssetPreviewBoundsGeometry(
                reloadedAfterRename.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded renamed real asset snapshot should preserve live preview metadata");
            Expect(reloadedAfterRename.Document.CommandUndoAvailable,
                $"reloaded real asset rename snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset rename smoke should undo rename for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                undoResult.Document,
                recordIndex,
                "UNIQUEID",
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"undone real asset rename snapshot should preserve original identity");
            AssertRealAssetPreviewBoundsGeometry(
                undoResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"undone real asset rename snapshot should preserve live preview metadata");
            Expect(!undoResult.Document.CommandUndoAvailable,
                $"undone real asset rename snapshot should clear command undo for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset rename smoke should reload restored identity for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterUndo.Document,
                recordIndex,
                "UNIQUEID",
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded undone real asset rename snapshot should preserve original identity");
            AssertRealAssetPreviewBoundsGeometry(
                reloadedAfterUndo.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded undone real asset rename snapshot should preserve live preview metadata");
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"reloaded undone real asset rename snapshot should clear command undo for {sourcePath}");
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

    private static void SmokeRealAssetHostBackedDeleteRestoreRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalSectionObjectCount,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset delete candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletes-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset delete smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial real asset delete snapshot should preserve live preview bounds for {sourcePath}");
            if (loaded.Document.ReportLayout?.PreviewBoundsAvailable != true)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                "UNIQUEID",
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"initial real asset delete snapshot should preserve source object identity");
            AssertRealAssetSectionObjectCount(
                loaded.Document,
                expectedSectionTitle,
                expectedOriginalSectionObjectCount,
                $"initial real asset delete snapshot should preserve section object counts");
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
                $"real asset delete smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                deleteResult.Document,
                recordIndex,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"updated real asset delete snapshot should preserve deleted section-member continuity");

            var reloadedAfterDelete = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDelete.Success && reloadedAfterDelete.Document is not null,
                $"real asset delete smoke should reload deleted snapshot data for {sourcePath}");
            if (!reloadedAfterDelete.Success || reloadedAfterDelete.Document is null)
            {
                return;
            }

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
                $"reloaded real asset delete snapshot should preserve deleted section-member continuity");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real asset delete smoke should restore record {recordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                restoreResult.Document,
                recordIndex,
                "UNIQUEID",
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"restored real asset delete snapshot should preserve source object identity");
            AssertRealAssetSectionObjectCount(
                restoreResult.Document,
                expectedSectionTitle,
                expectedOriginalSectionObjectCount,
                $"restored real asset delete snapshot should preserve section object counts");
            AssertRealAssetLivePreviewBounds(
                restoreResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"restored real asset delete snapshot should preserve live preview metadata");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real asset delete smoke should reload restored snapshot data for {sourcePath}");
            if (!reloadedAfterRestore.Success || reloadedAfterRestore.Document is null)
            {
                return;
            }

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
                $"reloaded restored real asset delete snapshot should preserve source object identity");
            AssertRealAssetSectionObjectCount(
                reloadedAfterRestore.Document,
                expectedSectionTitle,
                expectedOriginalSectionObjectCount,
                $"reloaded restored real asset delete snapshot should preserve section object counts");
            AssertRealAssetLivePreviewBounds(
                reloadedAfterRestore.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded restored real asset delete snapshot should preserve live preview metadata");
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

    private static void SmokeRealAssetHostBackedDeletedPropertyRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string propertyName,
        string originalRawValue,
        string updatedRawValue,
        int expectedOriginalLayoutValue,
        int expectedUpdatedLayoutValue,
        int expectedSectionCount,
        bool expectLabel,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted property candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletedWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real deleted property smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial deleted property snapshot should preserve live preview bounds for {sourcePath}");
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
                $"real deleted property smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectPropertySnapshot(
                deleteResult.Document,
                recordIndex,
                propertyName,
                originalRawValue,
                expectedOriginalLayoutValue,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"initial deleted real asset snapshot should preserve {propertyName}");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                deleteResult.Document,
                $"initial deleted real asset snapshot should preserve deleted preview metadata");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                recordIndex,
                propertyName,
                updatedRawValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real deleted property smoke should update {propertyName} for deleted record {recordIndex} in {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectPropertySnapshot(
                updateResult.Document,
                recordIndex,
                propertyName,
                updatedRawValue,
                expectedUpdatedLayoutValue,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"updated deleted real asset snapshot should preserve {propertyName}");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                updateResult.Document,
                $"updated deleted real asset snapshot should preserve deleted preview metadata");
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real deleted property smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real deleted property smoke should reload updated deleted snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectPropertySnapshot(
                reloadedAfterUpdate.Document,
                recordIndex,
                propertyName,
                updatedRawValue,
                expectedUpdatedLayoutValue,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"reloaded updated deleted real asset snapshot should preserve {propertyName}");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                reloadedAfterUpdate.Document,
                $"reloaded updated deleted real asset snapshot should preserve deleted preview metadata");
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded deleted property snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real deleted property smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectPropertySnapshot(
                undoResult.Document,
                recordIndex,
                propertyName,
                originalRawValue,
                expectedOriginalLayoutValue,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"undone deleted real asset snapshot should preserve {propertyName}");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                undoResult.Document,
                $"undone deleted real asset snapshot should preserve deleted preview metadata");
            Expect(undoResult.Document.CommandUndoAvailable,
                $"undone deleted property snapshot should retain the earlier delete-state undo for {sourcePath}");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real deleted property smoke should restore record {recordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                restoreResult.Document,
                recordIndex,
                propertyName,
                originalRawValue,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"restored deleted-property real asset snapshot should preserve {propertyName}");
            AssertRealAssetLivePreviewBounds(
                restoreResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"restored deleted-property real asset snapshot should preserve live preview metadata");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real deleted property smoke should reload restored live snapshot data for {sourcePath}");
            if (!reloadedAfterRestore.Success || reloadedAfterRestore.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterRestore.Document,
                recordIndex,
                propertyName,
                originalRawValue,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded restored deleted-property real asset snapshot should preserve {propertyName}");
            AssertRealAssetLivePreviewBounds(
                reloadedAfterRestore.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                $"reloaded restored deleted-property real asset snapshot should preserve live preview metadata");
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
