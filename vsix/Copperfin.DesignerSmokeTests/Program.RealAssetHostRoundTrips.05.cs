
// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
    private static void SmokeRealAssetHostBackedDeletedReorderRoundTrip(
        string? sourcePath,
        int reorderedSourceRecordIndex,
        string reorderedSourceUniqueId,
        string reorderedSourceObjectTitle,
        int companionRecordIndex,
        string companionUniqueId,
        string companionObjectTitle,
        string expectedSectionTitle,
        int initialSectionRecordIndex,
        int reorderedSectionRecordIndex,
        int expectedSectionCount,
        int expectedVisibleSectionObjectCount,
        int expectedReorderedRecordIndex,
        int expectedCompanionRecordIndex,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted reorder candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletedReorders-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real deleted reorder smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            Expect(loaded.Document.ReportLayout?.PreviewBoundsAvailable == true,
                $"initial deleted reorder snapshot should preserve live preview bounds for {sourcePath}");
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

            var firstDeleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                reorderedSourceRecordIndex,
                reorderedSourceUniqueId);
            Expect(firstDeleteResult.Success && firstDeleteResult.Document is not null,
                $"real deleted reorder smoke should delete record {reorderedSourceRecordIndex} for {sourcePath}");
            if (!firstDeleteResult.Success || firstDeleteResult.Document is null)
            {
                return;
            }

            var secondDeleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                companionRecordIndex,
                companionUniqueId);
            Expect(secondDeleteResult.Success && secondDeleteResult.Document is not null,
                $"real deleted reorder smoke should delete companion record {companionRecordIndex} for {sourcePath}");
            if (!secondDeleteResult.Success || secondDeleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                secondDeleteResult.Document,
                reorderedSourceRecordIndex,
                reorderedSourceUniqueId,
                reorderedSourceObjectTitle,
                expectedSectionTitle,
                initialSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount,
                expectedDeletedSectionObjectCount: 2,
                "initial deleted real asset reorder snapshot should preserve the reordered-source identity",
                assertTitles: false);
            AssertRealAssetDeletedObjectSnapshot(
                secondDeleteResult.Document,
                companionRecordIndex,
                companionUniqueId,
                companionObjectTitle,
                expectedSectionTitle,
                initialSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount,
                expectedDeletedSectionObjectCount: 2,
                "initial deleted real asset reorder snapshot should preserve the companion identity");
            AssertRealAssetDeletedObjectOrder(
                secondDeleteResult.Document,
                initialSectionRecordIndex,
                new[] { companionRecordIndex, reorderedSourceRecordIndex },
                new[] { companionUniqueId, reorderedSourceUniqueId },
                "initial deleted real asset reorder snapshot should preserve deleted-row order");
            AssertRealAssetPreviewBoundsGeometry(
                secondDeleteResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                "initial deleted real asset reorder snapshot should preserve live preview metadata");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                secondDeleteResult.Document,
                "initial deleted real asset reorder snapshot should preserve deleted preview metadata");

            var reorderResult = CopperfinStudioSnapshotClient.TryReorderObject(
                assetPath,
                reorderedSourceRecordIndex,
                reorderedSourceUniqueId,
                "front");
            Expect(reorderResult.Success && reorderResult.Document is not null,
                $"real deleted reorder smoke should reorder deleted record {reorderedSourceRecordIndex} for {sourcePath}");
            if (!reorderResult.Success || reorderResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reorderResult.Document,
                expectedReorderedRecordIndex,
                reorderedSourceUniqueId,
                reorderedSourceObjectTitle,
                expectedSectionTitle,
                reorderedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount,
                expectedDeletedSectionObjectCount: 2,
                "reordered deleted real asset snapshot should preserve the reordered-source identity",
                assertTitles: false);
            AssertRealAssetDeletedObjectSnapshot(
                reorderResult.Document,
                expectedCompanionRecordIndex,
                companionUniqueId,
                companionObjectTitle,
                expectedSectionTitle,
                reorderedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount,
                expectedDeletedSectionObjectCount: 2,
                "reordered deleted real asset snapshot should preserve the companion identity");
            AssertRealAssetDeletedObjectOrder(
                reorderResult.Document,
                reorderedSectionRecordIndex,
                new[] { expectedReorderedRecordIndex, expectedCompanionRecordIndex },
                new[] { reorderedSourceUniqueId, companionUniqueId },
                "reordered deleted real asset snapshot should preserve deleted-row order");
            AssertRealAssetPreviewBoundsGeometry(
                reorderResult.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                "reordered deleted real asset snapshot should preserve live preview metadata");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                reorderResult.Document,
                "reordered deleted real asset snapshot should preserve deleted preview metadata");
            Expect(reorderResult.Document.CommandUndoAvailable,
                $"real deleted reorder smoke should retain the earlier delete-state undo after reordering a deleted row for {sourcePath}");

            var reloadedAfterReorder = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterReorder.Success && reloadedAfterReorder.Document is not null,
                $"real deleted reorder smoke should reload reordered deleted snapshot data for {sourcePath}");
            if (!reloadedAfterReorder.Success || reloadedAfterReorder.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterReorder.Document,
                expectedReorderedRecordIndex,
                reorderedSourceUniqueId,
                reorderedSourceObjectTitle,
                expectedSectionTitle,
                reorderedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount,
                expectedDeletedSectionObjectCount: 2,
                "reloaded reordered deleted real asset snapshot should preserve the reordered-source identity",
                assertTitles: false);
            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterReorder.Document,
                expectedCompanionRecordIndex,
                companionUniqueId,
                companionObjectTitle,
                expectedSectionTitle,
                reorderedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount,
                expectedDeletedSectionObjectCount: 2,
                "reloaded reordered deleted real asset snapshot should preserve the companion identity");
            AssertRealAssetDeletedObjectOrder(
                reloadedAfterReorder.Document,
                reorderedSectionRecordIndex,
                new[] { expectedReorderedRecordIndex, expectedCompanionRecordIndex },
                new[] { reorderedSourceUniqueId, companionUniqueId },
                "reloaded reordered deleted real asset snapshot should preserve deleted-row order");
            AssertRealAssetPreviewBoundsGeometry(
                reloadedAfterReorder.Document,
                expectedPreviewBoundsLeft,
                expectedPreviewBoundsTop,
                expectedPreviewBoundsRight,
                expectedPreviewBoundsBottom,
                expectedPreviewBoundsWidth,
                expectedPreviewBoundsHeight,
                "reloaded reordered deleted real asset snapshot should preserve live preview metadata");
            AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
                reloadedAfterReorder.Document,
                "reloaded reordered deleted real asset snapshot should preserve deleted preview metadata");
            Expect(reloadedAfterReorder.Document.CommandUndoAvailable,
                $"reloaded reordered deleted real asset snapshot should retain the earlier delete-state undo for {sourcePath}");
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
