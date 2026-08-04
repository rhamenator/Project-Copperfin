
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
    private static void SmokeRealAssetHostBackedPropertyRoundTrip(
        string? sourcePath,
        int recordIndex,
        string propertyName,
        string originalValue,
        string updatedValue,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        bool expectUnplacedObject = false,
        ExpectedPreviewBoundsGeometry? expectedUpdatedPreviewBounds = null)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset write smoke should load snapshot data for {sourcePath}");
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
                    $"initial real asset snapshot should preserve live preview bounds for {sourcePath}");
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
                    $"initial real asset snapshot should preserve live preview metadata");
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                propertyName,
                originalValue,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject,
                $"initial real asset snapshot should preserve {propertyName}");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                recordIndex,
                propertyName,
                updatedValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset write smoke should update {propertyName} for {sourcePath}");
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
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject,
                $"updated real asset snapshot should preserve {propertyName}");
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
                    $"updated real asset snapshot should preserve live preview metadata");
            }
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset write smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset write smoke should reload updated snapshot data for {sourcePath}");
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
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject,
                $"reloaded updated real asset snapshot should preserve {propertyName}");
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
                    $"reloaded updated real asset snapshot should preserve live preview metadata");
            }
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset write smoke should undo {propertyName} for {sourcePath}");
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
                    $"undone real asset snapshot should preserve live preview metadata");
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset write smoke should reload undone snapshot data for {sourcePath}");
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
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject,
                $"reloaded undone real asset snapshot should preserve {propertyName}");
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
                    $"reloaded undone real asset snapshot should preserve live preview metadata");
            }
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset write smoke should clear undo after restoring {propertyName} for {sourcePath}");
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

    private static void SmokeRealAssetHostBackedSettingsRoundTrip(
        string? sourcePath,
        string propertyName,
        string originalValue,
        string updatedValue,
        int expectedSectionCount,
        bool expectLabel,
        CopperfinStudioNamedValue? expectedUpdatedSetting = null,
        CopperfinStudioNamedValue? expectedUndoneSetting = null,
        bool expectRawSnapshotProperty = true,
        ExpectedPreviewBoundsGeometry? expectedUpdatedPreviewBounds = null)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset settings write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealSettingsWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset settings smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null || loaded.Document.ReportLayout is null)
            {
                return;
            }

            var expectedPreviewBoundsLeft = loaded.Document.ReportLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = loaded.Document.ReportLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = loaded.Document.ReportLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = loaded.Document.ReportLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = loaded.Document.ReportLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = loaded.Document.ReportLayout.PreviewBoundsHeight;
            if (expectedUpdatedPreviewBounds is not null)
            {
                Expect(loaded.Document.ReportLayout.PreviewBoundsAvailable,
                    $"initial real asset settings snapshot should preserve live preview bounds for {sourcePath}");
                if (!loaded.Document.ReportLayout.PreviewBoundsAvailable)
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
                    "initial real asset settings snapshot should preserve live preview metadata");
            }

            var initialSetting = loaded.Document.ReportLayout.Settings
                .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase));
            Expect(initialSetting is not null,
                $"real asset settings smoke should expose {propertyName} in live settings for {sourcePath}");
            if (initialSetting is null)
            {
                return;
            }

            expectedUpdatedSetting ??= initialSetting;
            expectedUndoneSetting ??= initialSetting;

            AssertRealAssetSettingsSnapshot(
                loaded.Document,
                initialSetting,
                originalValue,
                expectedSectionCount,
                expectLabel,
                requireSelectedSettings: false,
                expectRawSnapshotProperty: expectRawSnapshotProperty,
                $"initial real asset settings snapshot should preserve {propertyName}");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                initialSetting.RecordIndex,
                propertyName,
                updatedValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset settings smoke should update {propertyName} for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetSettingsSnapshot(
                updateResult.Document,
                expectedUpdatedSetting,
                updatedValue,
                expectedSectionCount,
                expectLabel,
                requireSelectedSettings: true,
                expectRawSnapshotProperty: expectRawSnapshotProperty,
                $"updated real asset settings snapshot should preserve {propertyName}");
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
                    "updated real asset settings snapshot should preserve live preview metadata");
            }
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset settings smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset settings smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetSettingsSnapshot(
                reloadedAfterUpdate.Document,
                expectedUpdatedSetting,
                updatedValue,
                expectedSectionCount,
                expectLabel,
                requireSelectedSettings: false,
                expectRawSnapshotProperty: expectRawSnapshotProperty,
                $"reloaded updated real asset settings snapshot should preserve {propertyName}");
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
                    "reloaded updated real asset settings snapshot should preserve live preview metadata");
            }
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset settings snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset settings smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            AssertRealAssetSettingsSnapshot(
                undoResult.Document,
                expectedUndoneSetting,
                originalValue,
                expectedSectionCount,
                expectLabel,
                requireSelectedSettings: false,
                expectRawSnapshotProperty: expectRawSnapshotProperty,
                $"undone real asset settings snapshot should preserve {propertyName}");
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
                    "undone real asset settings snapshot should preserve live preview metadata");
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset settings smoke should reload undone snapshot data for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetSettingsSnapshot(
                reloadedAfterUndo.Document,
                expectedUndoneSetting,
                originalValue,
                expectedSectionCount,
                expectLabel,
                requireSelectedSettings: false,
                expectRawSnapshotProperty: expectRawSnapshotProperty,
                $"reloaded undone real asset settings snapshot should preserve {propertyName}");
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
                    "reloaded undone real asset settings snapshot should preserve live preview metadata");
            }
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset settings smoke should clear undo after restoring {propertyName} for {sourcePath}");
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

    private static void SmokeRealAssetHostBackedMissingSettingsRoundTrip(
        string? sourcePath,
        string propertyName,
        string updatedValue,
        int expectedSectionCount,
        bool expectLabel,
        CopperfinStudioNamedValue expectedUpdatedSetting,
        bool expectRawSnapshotProperty = true,
        ExpectedPreviewBoundsGeometry? expectedUpdatedPreviewBounds = null)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset settings add candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealSettingsAdds-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset settings add smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null || loaded.Document.ReportLayout is null)
            {
                return;
            }

            var expectedPreviewBoundsLeft = loaded.Document.ReportLayout.PreviewBoundsLeft;
            var expectedPreviewBoundsTop = loaded.Document.ReportLayout.PreviewBoundsTop;
            var expectedPreviewBoundsRight = loaded.Document.ReportLayout.PreviewBoundsRight;
            var expectedPreviewBoundsBottom = loaded.Document.ReportLayout.PreviewBoundsBottom;
            var expectedPreviewBoundsWidth = loaded.Document.ReportLayout.PreviewBoundsWidth;
            var expectedPreviewBoundsHeight = loaded.Document.ReportLayout.PreviewBoundsHeight;
            if (expectedUpdatedPreviewBounds is not null)
            {
                Expect(loaded.Document.ReportLayout.PreviewBoundsAvailable,
                    $"initial real asset settings add snapshot should preserve live preview bounds for {sourcePath}");
                if (!loaded.Document.ReportLayout.PreviewBoundsAvailable)
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
                    "initial real asset settings add snapshot should preserve live preview metadata");
            }

            AssertRealAssetSettingMissingSnapshot(
                loaded.Document,
                propertyName,
                expectedSectionCount,
                expectLabel,
                $"initial real asset settings snapshot should keep {propertyName} absent");

            var settingsRecordIndex = loaded.Document.ReportLayout.Settings.FirstOrDefault()?.RecordIndex ?? 0;
            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                settingsRecordIndex,
                propertyName,
                updatedValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset settings add smoke should update {propertyName} for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetSettingsSnapshot(
                updateResult.Document,
                expectedUpdatedSetting,
                updatedValue,
                expectedSectionCount,
                expectLabel,
                requireSelectedSettings: true,
                expectRawSnapshotProperty: expectRawSnapshotProperty,
                $"updated real asset settings snapshot should preserve {propertyName}");
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
                    "updated real asset settings add snapshot should preserve live preview metadata");
            }
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset settings add smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset settings add smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetSettingsSnapshot(
                reloadedAfterUpdate.Document,
                expectedUpdatedSetting,
                updatedValue,
                expectedSectionCount,
                expectLabel,
                requireSelectedSettings: false,
                expectRawSnapshotProperty: expectRawSnapshotProperty,
                $"reloaded updated real asset settings snapshot should preserve {propertyName}");
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
                    "reloaded updated real asset settings add snapshot should preserve live preview metadata");
            }
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset settings snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset settings add smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            AssertRealAssetSettingMissingSnapshot(
                undoResult.Document,
                propertyName,
                expectedSectionCount,
                expectLabel,
                $"undone real asset settings snapshot should keep {propertyName} absent");
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
                    "undone real asset settings add snapshot should preserve live preview metadata");
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset settings add smoke should reload undone snapshot data for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetSettingMissingSnapshot(
                reloadedAfterUndo.Document,
                propertyName,
                expectedSectionCount,
                expectLabel,
                $"reloaded undone real asset settings snapshot should keep {propertyName} absent");
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
                    "reloaded undone real asset settings add snapshot should preserve live preview metadata");
            }
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset settings add smoke should clear undo after restoring {propertyName} for {sourcePath}");
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
