
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
    private static bool WaitUntil(TimeSpan timeout, Func<bool> condition)
    {
        var deadline = DateTime.UtcNow + timeout;
        while (DateTime.UtcNow < deadline)
        {
            Application.DoEvents();
            if (condition())
            {
                return true;
            }

            Thread.Sleep(50);
        }

        Application.DoEvents();
        return condition();
    }

    private static string? ResolveFirstExistingRealAssetPath(params string?[] candidates)
    {
        return candidates.FirstOrDefault(candidate => !string.IsNullOrWhiteSpace(candidate) && File.Exists(candidate));
    }

    private static IEnumerable<string> EnumerateResolvedRealReportAssetPaths(params string?[] preferredCandidates)
    {
        var yielded = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

        foreach (var candidate in preferredCandidates)
        {
            if (!string.IsNullOrWhiteSpace(candidate) &&
                File.Exists(candidate) &&
                yielded.Add(candidate!))
            {
                yield return candidate!;
            }
        }

        foreach (var root in EnumerateResolvedRealAssetRoots())
        {
            foreach (var path in EnumerateAccessibleRealAssetFiles(root))
            {
                if ((path.EndsWith(".frx", StringComparison.OrdinalIgnoreCase) ||
                     path.EndsWith(".lbx", StringComparison.OrdinalIgnoreCase)) &&
                    yielded.Add(path))
                {
                    yield return path;
                }
            }
        }
    }

    // Keep enumeration failures local to one directory. Directory.EnumerateFiles
    // is lazy on net472, so wrapping only its construction does not contain an
    // access failure raised while the iterator advances.
    private static IEnumerable<string> EnumerateAccessibleRealAssetFiles(string root)
    {
        var pendingDirectories = new Stack<string>();
        pendingDirectories.Push(root);

        while (pendingDirectories.Count > 0)
        {
            var currentDirectory = pendingDirectories.Pop();
            FileInfo[] files;
            try
            {
                files = new DirectoryInfo(currentDirectory).GetFiles("*.*", SearchOption.TopDirectoryOnly);
            }
            catch (IOException)
            {
                continue;
            }
            catch (UnauthorizedAccessException)
            {
                continue;
            }

            foreach (var file in files)
            {
                yield return file.FullName;
            }

            DirectoryInfo[] childDirectories;
            try
            {
                childDirectories = new DirectoryInfo(currentDirectory).GetDirectories();
            }
            catch (IOException)
            {
                continue;
            }
            catch (UnauthorizedAccessException)
            {
                continue;
            }

            foreach (var childDirectory in childDirectories)
            {
                try
                {
                    if ((childDirectory.Attributes & FileAttributes.ReparsePoint) == FileAttributes.ReparsePoint)
                    {
                        continue;
                    }

                    pendingDirectories.Push(childDirectory.FullName);
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

    private static IEnumerable<string> EnumerateResolvedRealAssetRoots()
    {
        var yielded = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

        var configuredVfpSourceRoot = ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFPSOURCE_ROOT"));
        if (!string.IsNullOrWhiteSpace(configuredVfpSourceRoot) &&
            Directory.Exists(configuredVfpSourceRoot) &&
            yielded.Add(configuredVfpSourceRoot!))
        {
            yield return configuredVfpSourceRoot!;
        }

        var vfpSourceZipPath = ResolveFirstExistingRealAssetPath(
            ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFPSOURCE_ZIP")),
            ExpandUserPath("~/Downloads/VFPSource.zip"));
        if (!string.IsNullOrWhiteSpace(vfpSourceZipPath))
        {
            var extractedVfpSourceRoot = TryExtractArchiveRoot(vfpSourceZipPath!, "VFPSource");
            if (!string.IsNullOrWhiteSpace(extractedVfpSourceRoot) &&
                Directory.Exists(extractedVfpSourceRoot) &&
                yielded.Add(extractedVfpSourceRoot!))
            {
                yield return extractedVfpSourceRoot!;
            }
        }

        var configuredVfp9Root = ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFP9_ROOT"));
        var defaultVfp9Root = Path.DirectorySeparatorChar == '\\'
            ? @"C:\Program Files (x86)\Microsoft Visual FoxPro 9"
            : null;
        foreach (var root in new[] { configuredVfp9Root, defaultVfp9Root })
        {
            if (!string.IsNullOrWhiteSpace(root) &&
                Directory.Exists(root) &&
                yielded.Add(root!))
            {
                yield return root!;
            }
        }

        var vfp9ZipPath = ResolveVfp9ZipPath();
        if (!string.IsNullOrWhiteSpace(vfp9ZipPath))
        {
            var extractedVfp9Root = TryExtractArchive(vfp9ZipPath!);
            if (!string.IsNullOrWhiteSpace(extractedVfp9Root) &&
                Directory.Exists(extractedVfp9Root) &&
                yielded.Add(extractedVfp9Root!))
            {
                yield return extractedVfp9Root!;
            }
        }
    }

    private static void AssertRealAssetRoundTripSnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string propertyName,
        string expectedPropertyValue,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        bool expectUnplacedObject,
        string failurePrefix,
        bool assertObjectTitle = true)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.IsLabel == expectLabel,
            $"{failurePrefix} for {document.Path} should preserve report/label identity");
        Expect(document.ReportLayout.Sections.Count == expectedSectionCount,
            $"{failurePrefix} for {document.Path} should preserve section counts");

        CopperfinStudioReportLayoutObject? layoutObject;
        if (expectUnplacedObject)
        {
            layoutObject = document.ReportLayout.UnplacedObjects
                .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
            Expect(layoutObject is not null,
                $"{failurePrefix} for {document.Path} should preserve unplaced object {recordIndex}");
            if (layoutObject is null)
            {
                return;
            }
        }
        else
        {
            var section = document.ReportLayout.Sections
                .FirstOrDefault(candidate => string.Equals(candidate.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase));
            Expect(section is not null,
                $"{failurePrefix} for {document.Path} should preserve section '{expectedSectionTitle}'");
            if (section is null)
            {
                return;
            }

            layoutObject = section.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
            Expect(layoutObject is not null,
                $"{failurePrefix} for {document.Path} should preserve placed object {recordIndex}");
            if (layoutObject is null)
            {
                return;
            }
        }

        if (assertObjectTitle)
        {
            Expect(string.Equals(layoutObject.Title, expectedObjectTitle, StringComparison.Ordinal),
                $"{failurePrefix} for {document.Path} should preserve the selected object title");
        }

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw snapshot object {recordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        var property = snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase));
        Expect(property is not null,
            $"{failurePrefix} for {document.Path} should preserve property {propertyName}");
        if (property is null)
        {
            return;
        }

        Expect(string.Equals(property.Value, expectedPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should expose {propertyName}={expectedPropertyValue}");
    }

    private static void AssertRealAssetSettingMissingSnapshot(
        CopperfinStudioSnapshotDocument document,
        string propertyName,
        int expectedSectionCount,
        bool expectLabel,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.IsLabel == expectLabel,
            $"{failurePrefix} for {document.Path} should preserve report/label identity");
        Expect(document.ReportLayout.Sections.Count == expectedSectionCount,
            $"{failurePrefix} for {document.Path} should preserve section counts");
        Expect(!document.ReportLayout.Settings.Any(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase)),
            $"{failurePrefix} for {document.Path} should not expose settings property {propertyName}");
        Expect(!(document.SelectedReportSettings?.Any(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase)) ?? false),
            $"{failurePrefix} for {document.Path} should not expose selected settings metadata for {propertyName}");
    }

    private static void AssertRealAssetSettingsSnapshot(
        CopperfinStudioSnapshotDocument document,
        CopperfinStudioNamedValue expectedSetting,
        string expectedPropertyValue,
        int expectedSectionCount,
        bool expectLabel,
        bool requireSelectedSettings,
        bool expectRawSnapshotProperty,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.IsLabel == expectLabel,
            $"{failurePrefix} for {document.Path} should preserve report/label identity");
        Expect(document.ReportLayout.Sections.Count == expectedSectionCount,
            $"{failurePrefix} for {document.Path} should preserve section counts");

        var layoutSetting = document.ReportLayout.Settings
            .FirstOrDefault(candidate => string.Equals(candidate.Name, expectedSetting.Name, StringComparison.OrdinalIgnoreCase));
        Expect(layoutSetting is not null,
            $"{failurePrefix} for {document.Path} should preserve settings property {expectedSetting.Name}");
        if (layoutSetting is null)
        {
            return;
        }

        Expect(layoutSetting.RecordIndex == expectedSetting.RecordIndex &&
               layoutSetting.FieldIndex == expectedSetting.FieldIndex &&
               layoutSetting.SourceLineIndex == expectedSetting.SourceLineIndex &&
               layoutSetting.MemoBlockNumber == expectedSetting.MemoBlockNumber &&
               string.Equals(layoutSetting.Value, expectedPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should preserve settings metadata for {expectedSetting.Name}");

        if (!expectRawSnapshotProperty)
        {
            return;
        }

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == expectedSetting.RecordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw snapshot object {expectedSetting.RecordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        var property = snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, expectedSetting.Name, StringComparison.OrdinalIgnoreCase));
        Expect(property is not null,
            $"{failurePrefix} for {document.Path} should preserve property {expectedSetting.Name}");
        if (property is null)
        {
            return;
        }

        Expect(string.Equals(property.Value, expectedPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should expose {expectedSetting.Name}={expectedPropertyValue}");

        if (!requireSelectedSettings)
        {
            return;
        }

        Expect(document.SelectedReportSettingsAvailable,
            $"{failurePrefix} for {document.Path} should expose selected report settings");
        Expect(string.Equals(document.SelectedReportSelectionKind, "settings", StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should preserve settings selection kind");

        var selectedSetting = document.SelectedReportSettings?
            .FirstOrDefault(candidate => string.Equals(candidate.Name, expectedSetting.Name, StringComparison.OrdinalIgnoreCase));
        Expect(selectedSetting is not null,
            $"{failurePrefix} for {document.Path} should expose selected settings metadata for {expectedSetting.Name}");
        if (selectedSetting is null)
        {
            return;
        }

        Expect(selectedSetting.RecordIndex == expectedSetting.RecordIndex &&
               selectedSetting.FieldIndex == expectedSetting.FieldIndex &&
               selectedSetting.SourceLineIndex == expectedSetting.SourceLineIndex &&
               selectedSetting.MemoBlockNumber == expectedSetting.MemoBlockNumber &&
               string.Equals(selectedSetting.Value, expectedPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should preserve selected settings provenance for {expectedSetting.Name}");
    }

    private static void AssertRealAssetSectionSnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string expectedSectionTitle,
        string propertyName,
        string expectedRawPropertyValue,
        int? expectedLayoutPropertyValue,
        int expectedSectionCount,
        bool expectLabel,
        int expectedObjectCount,
        ExpectedSectionGroupingMetadata? expectedGrouping,
        string? expectedLayoutTextValue,
        ExpectedSectionContainedObjectGeometry[]? expectedContainedObjects,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.IsLabel == expectLabel,
            $"{failurePrefix} for {document.Path} should preserve report/label identity");
        Expect(document.ReportLayout.Sections.Count == expectedSectionCount,
            $"{failurePrefix} for {document.Path} should preserve section counts");

        var section = document.ReportLayout.Sections
            .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(section is not null,
            $"{failurePrefix} for {document.Path} should preserve section record {recordIndex}");
        if (section is null)
        {
            return;
        }

        Expect(string.Equals(section.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase),
            $"{failurePrefix} for {document.Path} should preserve section '{expectedSectionTitle}'");
        Expect(section.Objects.Count == expectedObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedObjectCount} objects in section '{expectedSectionTitle}'");
        if (expectedGrouping is not null)
        {
            Expect(section.GroupingContextAvailable,
                $"{failurePrefix} for {document.Path} should preserve grouping context for section '{expectedSectionTitle}'");
            Expect(string.Equals(section.GroupRole, expectedGrouping.GroupRole, StringComparison.Ordinal),
                $"{failurePrefix} for {document.Path} should preserve group role {expectedGrouping.GroupRole}");
            Expect(section.GroupingIndex == expectedGrouping.GroupingIndex,
                $"{failurePrefix} for {document.Path} should preserve grouping index {expectedGrouping.GroupingIndex}");
            Expect(section.GroupingNestingDepth == expectedGrouping.GroupingNestingDepth,
                $"{failurePrefix} for {document.Path} should preserve grouping nesting depth {expectedGrouping.GroupingNestingDepth}");
            Expect(string.Equals(section.GroupingExpression, expectedGrouping.GroupingExpression, StringComparison.Ordinal),
                $"{failurePrefix} for {document.Path} should preserve grouping expression {expectedGrouping.GroupingExpression}");
            Expect(section.GroupingExpressionFieldIndex == expectedGrouping.GroupingExpressionFieldIndex,
                $"{failurePrefix} for {document.Path} should preserve grouping expression field index {expectedGrouping.GroupingExpressionFieldIndex}");
            Expect(section.GroupingExpressionMemoBlockNumber == expectedGrouping.GroupingExpressionMemoBlockNumber,
                $"{failurePrefix} for {document.Path} should preserve grouping expression memo block {expectedGrouping.GroupingExpressionMemoBlockNumber}");
            Expect(string.Equals(section.GroupPartnerSectionId, expectedGrouping.GroupPartnerSectionId, StringComparison.Ordinal),
                $"{failurePrefix} for {document.Path} should preserve group partner section id {expectedGrouping.GroupPartnerSectionId}");
            Expect(section.GroupPartnerRecordIndex == expectedGrouping.GroupPartnerRecordIndex,
                $"{failurePrefix} for {document.Path} should preserve group partner record {expectedGrouping.GroupPartnerRecordIndex}");
            Expect(section.GroupPartnerDeleted == expectedGrouping.GroupPartnerDeleted,
                $"{failurePrefix} for {document.Path} should preserve group partner deleted state {expectedGrouping.GroupPartnerDeleted}");
        }

        if (expectedContainedObjects is not null)
        {
            foreach (var expectedContainedObject in expectedContainedObjects)
            {
                var containedObject = section.Objects.FirstOrDefault(candidate => candidate.RecordIndex == expectedContainedObject.RecordIndex);
                Expect(containedObject is not null,
                    $"{failurePrefix} for {document.Path} should preserve section object {expectedContainedObject.RecordIndex}");
                if (containedObject is null)
                {
                    return;
                }

                Expect(containedObject.Top == expectedContainedObject.Top,
                    $"{failurePrefix} for {document.Path} should expose contained object top {expectedContainedObject.Top}");
                Expect(containedObject.SectionRelativeTop == expectedContainedObject.SectionRelativeTop,
                    $"{failurePrefix} for {document.Path} should expose contained object relative top {expectedContainedObject.SectionRelativeTop}");
                Expect((containedObject.Top + containedObject.Height) == expectedContainedObject.Bottom,
                    $"{failurePrefix} for {document.Path} should expose contained object bottom {expectedContainedObject.Bottom}");
                Expect(containedObject.SectionRelativeBottom == expectedContainedObject.SectionRelativeBottom,
                    $"{failurePrefix} for {document.Path} should expose contained object relative bottom {expectedContainedObject.SectionRelativeBottom}");
            }
        }

        var layoutTextValue = TryGetReportSectionLayoutTextValue(section, propertyName);
        if (layoutTextValue is not null)
        {
            Expect(string.Equals(layoutTextValue, expectedLayoutTextValue ?? string.Empty, StringComparison.Ordinal),
                $"{failurePrefix} for {document.Path} should expose section {propertyName}={expectedLayoutTextValue ?? string.Empty}");
        }
        else
        {
            var layoutPropertyValue = TryGetReportSectionLayoutValue(section, propertyName);
            Expect(layoutPropertyValue.HasValue,
                $"{failurePrefix} for {document.Path} should expose section layout property {propertyName}");
            if (!layoutPropertyValue.HasValue || !expectedLayoutPropertyValue.HasValue)
            {
                return;
            }

            Expect(layoutPropertyValue.Value == expectedLayoutPropertyValue.Value,
                $"{failurePrefix} for {document.Path} should expose section {propertyName}={expectedLayoutPropertyValue.Value}");
        }

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw snapshot object {recordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        var property = snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase));
        Expect(property is not null,
            $"{failurePrefix} for {document.Path} should preserve property {propertyName}");
        if (property is null)
        {
            return;
        }

        Expect(string.Equals(property.Value, expectedRawPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should expose {propertyName}={expectedRawPropertyValue}");
    }

    private static void AssertRealAssetUntouchedSectionsSnapshot(
        CopperfinStudioSnapshotDocument document,
        ExpectedUntouchedSectionSnapshot[]? expectedSections,
        string failurePrefix)
    {
        if (expectedSections is null || expectedSections.Length == 0)
        {
            return;
        }

        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        foreach (var expectedSection in expectedSections)
        {
            var section = document.ReportLayout.Sections
                .FirstOrDefault(candidate => candidate.RecordIndex == expectedSection.RecordIndex);
            Expect(section is not null,
                $"{failurePrefix} for {document.Path} should preserve sibling section record {expectedSection.RecordIndex}");
            if (section is null)
            {
                continue;
            }

            Expect(string.Equals(section.Title, expectedSection.Title, StringComparison.OrdinalIgnoreCase),
                $"{failurePrefix} for {document.Path} should preserve sibling section '{expectedSection.Title}'");
            Expect(section.Top == expectedSection.Top,
                $"{failurePrefix} for {document.Path} should preserve sibling section top {expectedSection.Top}");
            Expect(section.Height == expectedSection.Height,
                $"{failurePrefix} for {document.Path} should preserve sibling section height {expectedSection.Height}");
            Expect(section.Objects.Count == expectedSection.ObjectCount,
                $"{failurePrefix} for {document.Path} should preserve sibling section object count {expectedSection.ObjectCount}");
            if (expectedSection.Grouping is not null)
            {
                Expect(SectionMatchesExpectedGrouping(section, expectedSection.Grouping),
                    $"{failurePrefix} for {document.Path} should preserve sibling section grouping context");
            }
        }
    }

    private static bool SectionMatchesExpectedGrouping(
        CopperfinStudioReportSection? section,
        ExpectedSectionGroupingMetadata? expectedGrouping)
    {
        if (expectedGrouping is null)
        {
            return true;
        }

        return section is not null &&
               section.GroupingContextAvailable &&
               string.Equals(section.GroupRole, expectedGrouping.GroupRole, StringComparison.Ordinal) &&
               section.GroupingIndex == expectedGrouping.GroupingIndex &&
               section.GroupingNestingDepth == expectedGrouping.GroupingNestingDepth &&
               string.Equals(section.GroupingExpression, expectedGrouping.GroupingExpression, StringComparison.Ordinal) &&
               section.GroupingExpressionFieldIndex == expectedGrouping.GroupingExpressionFieldIndex &&
               section.GroupingExpressionMemoBlockNumber == expectedGrouping.GroupingExpressionMemoBlockNumber &&
               string.Equals(section.GroupPartnerSectionId, expectedGrouping.GroupPartnerSectionId, StringComparison.Ordinal) &&
               section.GroupPartnerRecordIndex == expectedGrouping.GroupPartnerRecordIndex &&
               section.GroupPartnerDeleted == expectedGrouping.GroupPartnerDeleted;
    }

    private static bool SelectionMatchesExpectedSectionGrouping(
        CopperfinDesignerSelection selection,
        ExpectedSectionGroupingMetadata? expectedGrouping)
    {
        if (expectedGrouping is null)
        {
            return true;
        }

        var expectedRoleDisplay = expectedGrouping.GroupRoleDisplay ?? expectedGrouping.GroupRole;
        var expectedPartnerStateDisplay = expectedGrouping.GroupPartnerStateDisplay ??
                                          (expectedGrouping.GroupPartnerDeleted ? "Deleted" : "Live");
        return string.Equals(ReadSelectionPropertyValue(selection, "GROUPROLE"), expectedRoleDisplay, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGINDEX"), expectedGrouping.GroupingIndex.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGNESTINGDEPTH"), expectedGrouping.GroupingNestingDepth.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "EXPR"), expectedGrouping.SectionExpression, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGEXPRESSION"), expectedGrouping.GroupingExpression, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGEXPRESSIONFIELD"), expectedGrouping.GroupingExpressionFieldIndex?.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGEXPRESSIONMEMO"), expectedGrouping.GroupingExpressionMemoBlockNumber.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPPARTNERSECTIONID"), expectedGrouping.GroupPartnerSectionId, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPPARTNERRECORD"), expectedGrouping.GroupPartnerRecordIndex.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPPARTNERSTATE"), expectedPartnerStateDisplay, StringComparison.Ordinal);
    }

    private static bool SelectionMatchesExpectedReportGrouping(
        CopperfinDesignerSelection selection,
        ExpectedReportGroupingMetadata expectedGrouping)
    {
        var expectedHeaderStateDisplay = expectedGrouping.HeaderStateDisplay ??
                                         (expectedGrouping.HeaderDeleted ? "Deleted" : "Live");
        var expectedFooterStateDisplay = expectedGrouping.FooterStateDisplay ??
                                         (expectedGrouping.FooterDeleted ? "Deleted" : "Live");
        return string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGINDEX"), expectedGrouping.GroupingIndex.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGNESTINGDEPTH"), expectedGrouping.GroupingNestingDepth.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGEXPRESSION"), expectedGrouping.GroupingExpression, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGEXPRESSIONFIELD"), expectedGrouping.GroupingExpressionFieldIndex?.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPINGEXPRESSIONMEMO"), expectedGrouping.GroupingExpressionMemoBlockNumber.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPHEADERSECTIONID"), expectedGrouping.HeaderSectionId, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPHEADERRECORD"), expectedGrouping.HeaderRecordIndex?.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPHEADERSTATE"), expectedHeaderStateDisplay, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPFOOTERSECTIONID"), expectedGrouping.FooterSectionId, StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPFOOTERRECORD"), expectedGrouping.FooterRecordIndex?.ToString(), StringComparison.Ordinal) &&
               string.Equals(ReadSelectionPropertyValue(selection, "GROUPFOOTERSTATE"), expectedFooterStateDisplay, StringComparison.Ordinal);
    }

    private static string? ReadSelectionPropertyValue(CopperfinDesignerSelection selection, string propertyName)
    {
        return TypeDescriptor.GetProperties(selection)[propertyName]?.GetValue(selection)?.ToString();
    }

    private static string NormalizeSectionSelectionPropertyName(string propertyName)
    {
        return string.Equals(propertyName, "VPOS", StringComparison.OrdinalIgnoreCase)
            ? "TOP"
            : propertyName;
    }

    private static void AssertRealAssetUnplacedObjectCount(
        CopperfinStudioSnapshotDocument document,
        int expectedUnplacedObjectCount,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.UnplacedObjects.Count == expectedUnplacedObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedUnplacedObjectCount} unplaced objects");
    }

    private static void AssertRealAssetSectionObjectCount(
        CopperfinStudioSnapshotDocument document,
        string expectedSectionTitle,
        int expectedObjectCount,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        var section = document.ReportLayout.Sections
            .FirstOrDefault(candidate => string.Equals(candidate.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase));
        Expect(section is not null,
            $"{failurePrefix} for {document.Path} should preserve section '{expectedSectionTitle}'");
        if (section is null)
        {
            return;
        }

        Expect(section.Objects.Count == expectedObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedObjectCount} objects in section '{expectedSectionTitle}'");
    }

    private static void AssertRealAssetSectionRecordOrder(
        CopperfinStudioSnapshotDocument document,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        IReadOnlyList<int> expectedRecordOrder,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        var section = document.ReportLayout.Sections
            .FirstOrDefault(candidate => string.Equals(candidate.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase));
        Expect(section is not null,
            $"{failurePrefix} for {document.Path} should preserve section '{expectedSectionTitle}'");
        if (section is null)
        {
            return;
        }

        Expect(section.RecordIndex == expectedSectionRecordIndex,
            $"{failurePrefix} for {document.Path} should preserve section record {expectedSectionRecordIndex}");
        var actualRecordOrder = section.Objects.Select(candidate => candidate.RecordIndex).ToArray();
        Expect(actualRecordOrder.SequenceEqual(expectedRecordOrder),
            $"{failurePrefix} for {document.Path} should preserve section record order [{string.Join(", ", expectedRecordOrder)}]");
    }

    private static void AssertRealAssetDeletedObjectSnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedVisibleSectionObjectCount,
        string failurePrefix)
    {
        AssertRealAssetDeletedObjectSnapshot(
            document,
            recordIndex,
            expectedUniqueId,
            expectedObjectTitle,
            expectedSectionTitle,
            expectedSectionRecordIndex,
            expectedSectionCount,
            expectLabel,
            expectedVisibleSectionObjectCount,
            expectedDeletedSectionObjectCount: 1,
            failurePrefix);
    }

    private static void AssertRealAssetDeletedObjectSnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedVisibleSectionObjectCount,
        int expectedDeletedSectionObjectCount,
        string failurePrefix,
        bool assertTitles = true)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.IsLabel == expectLabel,
            $"{failurePrefix} for {document.Path} should preserve report/label identity");
        Expect(document.ReportLayout.Sections.Count == expectedSectionCount,
            $"{failurePrefix} for {document.Path} should preserve section counts");
        Expect(document.ReportLayout.DeletedPreviewBoundsAvailable,
            $"{failurePrefix} for {document.Path} should preserve deleted preview bounds");

        var section = document.ReportLayout.Sections
            .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
        Expect(section is not null,
            $"{failurePrefix} for {document.Path} should preserve containing section record {expectedSectionRecordIndex}");
        if (section is null)
        {
            return;
        }

        Expect(string.Equals(section.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase),
            $"{failurePrefix} for {document.Path} should preserve containing section '{expectedSectionTitle}'");
        Expect(section.DeletedObjectCount == expectedDeletedSectionObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedDeletedSectionObjectCount} deleted objects in section '{expectedSectionTitle}'");

        var deletedLayoutObject = document.ReportLayout.DeletedObjects
            .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(deletedLayoutObject is not null,
            $"{failurePrefix} for {document.Path} should expose deleted layout object {recordIndex}");
        if (deletedLayoutObject is null)
        {
            return;
        }

        if (assertTitles)
        {
            Expect(string.Equals(deletedLayoutObject.Title, expectedObjectTitle, StringComparison.Ordinal),
                $"{failurePrefix} for {document.Path} should preserve the deleted object title");
        }
        Expect(deletedLayoutObject.ContainingSectionRecordIndex == expectedSectionRecordIndex,
            $"{failurePrefix} for {document.Path} should preserve the deleted object's containing section");

        var visibleSectionObjectCount = section.Objects.Count +
                                        document.ReportLayout.DeletedObjects.Count(candidate =>
                                            candidate.ContainingSectionRecordIndex == expectedSectionRecordIndex);
        Expect(visibleSectionObjectCount == expectedVisibleSectionObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedVisibleSectionObjectCount} visible objects in section '{expectedSectionTitle}'");

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw snapshot object {recordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        Expect(snapshotObject.Deleted,
            $"{failurePrefix} for {document.Path} should preserve deleted object state");
        if (assertTitles)
        {
            Expect(string.Equals(snapshotObject.Title, expectedObjectTitle, StringComparison.Ordinal),
                $"{failurePrefix} for {document.Path} should preserve the raw deleted object title");
        }
        Expect(string.Equals(TryGetSnapshotObjectPropertyValue(snapshotObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should preserve the deleted UNIQUEID");
    }

    private static void AssertRealAssetDeletedObjectOrder(
        CopperfinStudioSnapshotDocument document,
        int expectedSectionRecordIndex,
        IReadOnlyList<int> expectedRecordOrder,
        IReadOnlyList<string> expectedUniqueIdOrder,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        var deletedObjects = document.ReportLayout.DeletedObjects
            .Where(candidate => candidate.ContainingSectionRecordIndex == expectedSectionRecordIndex)
            .ToList();
        var actualRecordOrder = deletedObjects.Select(candidate => candidate.RecordIndex).ToArray();
        Expect(actualRecordOrder.SequenceEqual(expectedRecordOrder),
            $"{failurePrefix} for {document.Path} should preserve deleted record order [{string.Join(", ", expectedRecordOrder)}]");

        var actualUniqueIdOrder = deletedObjects
            .Select(candidate => document.Objects.FirstOrDefault(snapshotObject => snapshotObject.RecordIndex == candidate.RecordIndex))
            .Select(snapshotObject => snapshotObject is null ? null : TryGetSnapshotObjectPropertyValue(snapshotObject, "UNIQUEID"))
            .ToArray();
        Expect(actualUniqueIdOrder.SequenceEqual(expectedUniqueIdOrder),
            $"{failurePrefix} for {document.Path} should preserve deleted UNIQUEID order [{string.Join(", ", expectedUniqueIdOrder)}]");
    }

    private static void AssertRealAssetLivePreviewBounds(
        CopperfinStudioSnapshotDocument document,
        int expectedLeft,
        int expectedTop,
        int expectedRight,
        int expectedBottom,
        int expectedWidth,
        int expectedHeight,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.PreviewBoundsAvailable,
            $"{failurePrefix} for {document.Path} should preserve live preview bounds");
        Expect(!document.ReportLayout.DeletedPreviewBoundsAvailable,
            $"{failurePrefix} for {document.Path} should clear deleted preview bounds");
        Expect(document.ReportLayout.PreviewBoundsLeft == expectedLeft &&
               document.ReportLayout.PreviewBoundsTop == expectedTop &&
               document.ReportLayout.PreviewBoundsRight == expectedRight &&
               document.ReportLayout.PreviewBoundsBottom == expectedBottom &&
               document.ReportLayout.PreviewBoundsWidth == expectedWidth &&
               document.ReportLayout.PreviewBoundsHeight == expectedHeight,
            $"{failurePrefix} for {document.Path} should preserve the source preview-bounds geometry " +
            $"(actual: L {document.ReportLayout.PreviewBoundsLeft} T {document.ReportLayout.PreviewBoundsTop} " +
            $"R {document.ReportLayout.PreviewBoundsRight} B {document.ReportLayout.PreviewBoundsBottom} " +
            $"W {document.ReportLayout.PreviewBoundsWidth} H {document.ReportLayout.PreviewBoundsHeight}; " +
            $"expected: L {expectedLeft} T {expectedTop} R {expectedRight} B {expectedBottom} W {expectedWidth} H {expectedHeight})");
    }

    private static void AssertRealAssetPreviewBoundsGeometry(
        CopperfinStudioSnapshotDocument document,
        int expectedLeft,
        int expectedTop,
        int expectedRight,
        int expectedBottom,
        int expectedWidth,
        int expectedHeight,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.PreviewBoundsAvailable,
            $"{failurePrefix} for {document.Path} should preserve live preview bounds");
        Expect(document.ReportLayout.PreviewBoundsLeft == expectedLeft &&
               document.ReportLayout.PreviewBoundsTop == expectedTop &&
               document.ReportLayout.PreviewBoundsRight == expectedRight &&
               document.ReportLayout.PreviewBoundsBottom == expectedBottom &&
               document.ReportLayout.PreviewBoundsWidth == expectedWidth &&
               document.ReportLayout.PreviewBoundsHeight == expectedHeight,
            $"{failurePrefix} for {document.Path} should preserve the expected live preview-bounds geometry");
    }

}
