
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
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
    private static void AssertRealAssetDeletedPreviewBoundsMatchesDeletedObjects(
        CopperfinStudioSnapshotDocument document,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.DeletedPreviewBoundsAvailable,
            $"{failurePrefix} for {document.Path} should preserve deleted preview bounds");
        var deletedObjects = document.ReportLayout.DeletedObjects;
        Expect(deletedObjects.Count > 0,
            $"{failurePrefix} for {document.Path} should expose deleted layout objects");
        if (deletedObjects.Count <= 0)
        {
            return;
        }

        var expectedLeft = deletedObjects.Min(candidate => candidate.Left);
        var expectedTop = deletedObjects.Min(candidate => candidate.Top);
        var expectedRight = deletedObjects.Max(candidate => candidate.Left + candidate.Width);
        var expectedBottom = deletedObjects.Max(candidate => candidate.Top + candidate.Height);
        var expectedWidth = expectedRight - expectedLeft;
        var expectedHeight = expectedBottom - expectedTop;
        Expect(document.ReportLayout.DeletedPreviewBoundsLeft == expectedLeft &&
               document.ReportLayout.DeletedPreviewBoundsTop == expectedTop &&
               document.ReportLayout.DeletedPreviewBoundsRight == expectedRight &&
               document.ReportLayout.DeletedPreviewBoundsBottom == expectedBottom &&
               document.ReportLayout.DeletedPreviewBoundsWidth == expectedWidth &&
               document.ReportLayout.DeletedPreviewBoundsHeight == expectedHeight,
            $"{failurePrefix} for {document.Path} should keep deleted preview bounds aligned with deleted layout-object geometry");
    }

    private static void AssertRealAssetDeletedObjectPropertySnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string propertyName,
        string expectedRawPropertyValue,
        int? expectedLayoutPropertyValue,
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
            failurePrefix);

        if (document.ReportLayout is null)
        {
            return;
        }

        var deletedLayoutObject = document.ReportLayout.DeletedObjects
            .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(deletedLayoutObject is not null,
            $"{failurePrefix} for {document.Path} should preserve deleted layout object {recordIndex}");
        if (deletedLayoutObject is null)
        {
            return;
        }

        if (expectedLayoutPropertyValue.HasValue)
        {
            var layoutPropertyValue = TryGetReportLayoutObjectValue(deletedLayoutObject, propertyName);
            Expect(layoutPropertyValue.HasValue,
                $"{failurePrefix} for {document.Path} should expose deleted layout property {propertyName}");
            if (!layoutPropertyValue.HasValue)
            {
                return;
            }

            Expect(layoutPropertyValue.Value == expectedLayoutPropertyValue.Value,
                $"{failurePrefix} for {document.Path} should expose deleted layout {propertyName}={expectedLayoutPropertyValue.Value}");
        }

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw deleted snapshot object {recordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        var property = snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase));
        Expect(property is not null,
            $"{failurePrefix} for {document.Path} should preserve deleted property {propertyName}");
        if (property is null)
        {
            return;
        }

        Expect(string.Equals(property.Value, expectedRawPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should expose deleted {propertyName}={expectedRawPropertyValue}");
    }

    private static CopperfinStudioSnapshotObject? FindSnapshotObjectByUniqueId(
        CopperfinStudioSnapshotDocument document,
        string uniqueId)
    {
        return document.Objects.FirstOrDefault(candidate =>
            string.Equals(TryGetSnapshotObjectPropertyValue(candidate, "UNIQUEID"), uniqueId, StringComparison.OrdinalIgnoreCase));
    }

    private static string? TryGetSnapshotObjectPropertyValue(
        CopperfinStudioSnapshotObject snapshotObject,
        string propertyName)
    {
        return snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase))
            ?.Value;
    }

    private static int? TryGetReportSectionLayoutValue(
        CopperfinStudioReportSection section,
        string propertyName)
    {
        if (string.Equals(propertyName, "TOP", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(propertyName, "VPOS", StringComparison.OrdinalIgnoreCase))
        {
            return section.Top;
        }

        if (string.Equals(propertyName, "HEIGHT", StringComparison.OrdinalIgnoreCase))
        {
            return section.Height;
        }

        return null;
    }

    private static string? TryGetReportSectionLayoutTextValue(
        CopperfinStudioReportSection section,
        string propertyName)
    {
        if (string.Equals(propertyName, "EXPR", StringComparison.OrdinalIgnoreCase))
        {
            return section.Expression ?? string.Empty;
        }

        return null;
    }

    private static int? TryGetReportLayoutObjectValue(
        CopperfinStudioReportLayoutObject layoutObject,
        string propertyName)
    {
        if (string.Equals(propertyName, "HPOS", StringComparison.OrdinalIgnoreCase))
        {
            return layoutObject.Left;
        }

        if (string.Equals(propertyName, "VPOS", StringComparison.OrdinalIgnoreCase))
        {
            return layoutObject.Top;
        }

        if (string.Equals(propertyName, "WIDTH", StringComparison.OrdinalIgnoreCase))
        {
            return layoutObject.Width;
        }

        if (string.Equals(propertyName, "HEIGHT", StringComparison.OrdinalIgnoreCase))
        {
            return layoutObject.Height;
        }

        return null;
    }

    private static string CreateWritableAssetCopy(string sourcePath, string tempRoot)
    {
        var sourceFileInfo = new FileInfo(sourcePath);
        if (sourceFileInfo.Directory is null)
        {
            throw new InvalidOperationException($"Could not determine containing directory for {sourcePath}.");
        }

        var destinationDirectory = Path.Combine(tempRoot, sourceFileInfo.Directory.Name);
        CopyDirectoryRecursive(sourceFileInfo.Directory.FullName, destinationDirectory);
        return Path.Combine(destinationDirectory, sourceFileInfo.Name);
    }

    private static void CopyDirectoryRecursive(string sourceDirectory, string destinationDirectory)
    {
        Directory.CreateDirectory(destinationDirectory);

        foreach (var filePath in Directory.GetFiles(sourceDirectory))
        {
            var destinationPath = Path.Combine(destinationDirectory, Path.GetFileName(filePath));
            File.Copy(filePath, destinationPath, overwrite: true);
            var destinationAttributes = File.GetAttributes(destinationPath);
            if ((destinationAttributes & FileAttributes.ReadOnly) != 0)
            {
                File.SetAttributes(destinationPath, destinationAttributes & ~FileAttributes.ReadOnly);
            }

            MakeWritableCopy(destinationPath);
        }

        foreach (var childDirectory in Directory.GetDirectories(sourceDirectory))
        {
            var destinationChild = Path.Combine(destinationDirectory, Path.GetFileName(childDirectory));
            CopyDirectoryRecursive(childDirectory, destinationChild);
        }
    }

    private static void MakeWritableCopy(string path)
    {
        if (Environment.OSVersion.Platform == PlatformID.Win32NT)
        {
            return;
        }

        var processResult = CopperfinProcessRunner.Run(
            new ProcessStartInfo
            {
                FileName = "/bin/chmod",
                Arguments = $"u+w \"{path}\"",
                UseShellExecute = false,
                RedirectStandardError = true,
                RedirectStandardOutput = true,
                CreateNoWindow = true
            });
        if (!processResult.Started || processResult.ExitCode != 0)
        {
            throw new InvalidOperationException(
                string.IsNullOrWhiteSpace(processResult.StandardError)
                    ? $"Could not make copied asset writable: {path}."
                    : processResult.StandardError);
        }
    }

    private static void WithResolvedRealAssetToolchain(Action action)
    {
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousBuildHostPath = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        var previousRuntimeHostPath = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        var resolvedRuntimeHostPath = ResolveLocalToolPath(
            previousRuntimeHostPath,
            "copperfin_runtime_host");
        var resolvedStudioHostPath = ResolveLocalToolPath(
            Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH"),
            "copperfin_studio_host");
        var resolvedBuildHostPath = ResolveLocalToolPath(
            Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH"),
            "copperfin_build_host");

        // Keep an explicitly selected runtime host with its sibling tools. A
        // stale alternate build directory must not mix package contracts.
        if (string.IsNullOrWhiteSpace(previousBuildHostPath))
        {
            resolvedBuildHostPath = ResolveSiblingToolPath(resolvedRuntimeHostPath, "copperfin_build_host") ??
                resolvedBuildHostPath;
        }
        if (string.IsNullOrWhiteSpace(Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH")))
        {
            resolvedStudioHostPath = ResolveSiblingToolPath(resolvedRuntimeHostPath, "copperfin_studio_host") ??
                resolvedStudioHostPath;
        }

        if (!string.IsNullOrWhiteSpace(resolvedStudioHostPath))
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", resolvedStudioHostPath);
        }

        if (!string.IsNullOrWhiteSpace(resolvedBuildHostPath))
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", resolvedBuildHostPath);
        }

        if (!string.IsNullOrWhiteSpace(resolvedRuntimeHostPath))
        {
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", resolvedRuntimeHostPath);
        }

        try
        {
            action();
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", previousBuildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", previousRuntimeHostPath);
        }
    }

    private static string? ResolveLocalToolPath(string? configuredPath, string toolName)
    {
        var configuredCandidate = ExpandUserPath(configuredPath);
        if (!string.IsNullOrWhiteSpace(configuredCandidate) && File.Exists(configuredCandidate))
        {
            return configuredCandidate;
        }

        var candidates = new[]
        {
            ExpandUserPath("./build2/" + toolName),
            ExpandUserPath("./build2/" + toolName + ".exe"),
            ExpandUserPath("./build/" + toolName),
            ExpandUserPath("./build/" + toolName + ".exe"),
            ExpandUserPath("./build/Release/" + toolName),
            ExpandUserPath("./build/Release/" + toolName + ".exe"),
            ExpandUserPath("./.tmp/install-localization/bin/" + toolName),
            ExpandUserPath("./.tmp/install-localization/bin/" + toolName + ".exe")
        };

        return candidates.FirstOrDefault(candidate => !string.IsNullOrWhiteSpace(candidate) && File.Exists(candidate));
    }

    private static string? ResolveSiblingToolPath(string? runtimeHostPath, string toolName)
    {
        if (string.IsNullOrWhiteSpace(runtimeHostPath))
        {
            return null;
        }

        var directory = Path.GetDirectoryName(Path.GetFullPath(runtimeHostPath));
        if (string.IsNullOrWhiteSpace(directory))
        {
            return null;
        }

        foreach (var fileName in Path.DirectorySeparatorChar == '\\'
                     ? new[] { toolName + ".exe", toolName }
                     : new[] { toolName, toolName + ".exe" })
        {
            var candidate = Path.Combine(directory, fileName);
            if (File.Exists(candidate))
            {
                return candidate;
            }
        }

        return null;
    }

    private static string? TryResolveVfp9InstallAsset(string relativePath)
    {
        var normalizedRelativePath = relativePath.Replace('\\', '/');
        var configuredRoot = ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFP9_ROOT"));
        var defaultRoot = Path.DirectorySeparatorChar == '\\'
            ? @"C:\Program Files (x86)\Microsoft Visual FoxPro 9"
            : null;

        foreach (var root in new[] { configuredRoot, defaultRoot })
        {
            if (string.IsNullOrWhiteSpace(root))
            {
                continue;
            }

            var candidate = TryResolveAssetUnderRoot(root!, normalizedRelativePath);
            if (!string.IsNullOrWhiteSpace(candidate))
            {
                return candidate;
            }
        }

        var zipPath = ResolveVfp9ZipPath();
        if (!string.IsNullOrWhiteSpace(zipPath))
        {
            var extractedRoot = TryExtractArchive(zipPath!);
            if (!string.IsNullOrWhiteSpace(extractedRoot))
            {
                var extractedCandidate = TryResolveAssetUnderRoot(extractedRoot!, normalizedRelativePath);
                if (!string.IsNullOrWhiteSpace(extractedCandidate))
                {
                    return extractedCandidate;
                }
            }
        }

        return null;
    }

    private static string? ResolveVfp9ZipPath()
    {
        return ResolveFirstExistingRealAssetPath(
            ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFP9_ZIP")),
            ExpandUserPath("~/Downloads/VFP9Samples.zip"),
            ExpandUserPath("~/Downloads/VFP9-Samples.zip"),
            ExpandUserPath("~/Downloads/VFP9.zip"));
    }

    private static string? TryResolveVfpSourceAsset(string archiveRelativePath)
    {
        var configuredRoot = ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFPSOURCE_ROOT"));
        if (!string.IsNullOrWhiteSpace(configuredRoot))
        {
            var rootedCandidate = TryResolveAssetUnderRoot(configuredRoot!, archiveRelativePath);
            if (!string.IsNullOrWhiteSpace(rootedCandidate))
            {
                return rootedCandidate;
            }
        }

        var zipPath = ResolveFirstExistingRealAssetPath(
            ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFPSOURCE_ZIP")),
            ExpandUserPath("~/Downloads/VFPSource.zip"));
        if (string.IsNullOrWhiteSpace(zipPath))
        {
            return null;
        }

        return TryExtractArchiveContainingDirectory(zipPath!, archiveRelativePath);
    }

    private static string? TryResolveAssetUnderRoot(string root, string archiveRelativePath)
    {
        var normalizedRelativePath = archiveRelativePath.Replace('/', Path.DirectorySeparatorChar);
        var candidates = new List<string>
        {
            Path.Combine(root, normalizedRelativePath)
        };

        const string vfpSourcePrefix = "VFPSource/";
        if (archiveRelativePath.StartsWith(vfpSourcePrefix, StringComparison.OrdinalIgnoreCase))
        {
            candidates.Add(Path.Combine(root, archiveRelativePath.Substring(vfpSourcePrefix.Length).Replace('/', Path.DirectorySeparatorChar)));
        }

        return candidates.FirstOrDefault(File.Exists);
    }

    private static string? TryExtractArchiveContainingDirectory(string zipPath, string archiveRelativePath)
    {
        var normalizedRelativePath = archiveRelativePath.Replace('\\', '/');
        var directorySeparatorIndex = normalizedRelativePath.LastIndexOf('/');
        if (directorySeparatorIndex < 0)
        {
            return null;
        }

        var archiveDirectory = normalizedRelativePath.Substring(0, directorySeparatorIndex + 1);
        var assetFileName = normalizedRelativePath.Substring(directorySeparatorIndex + 1);
        var extractionRoot = Path.Combine(
            GetArchiveExtractionBaseRoot(zipPath),
            archiveDirectory.Replace('/', Path.DirectorySeparatorChar));
        var extractedAssetPath = Path.Combine(extractionRoot, assetFileName);
        var extractedAssetInfo = new FileInfo(extractedAssetPath);
        if (extractedAssetInfo.Exists && extractedAssetInfo.IsReadOnly)
        {
            return extractedAssetPath;
        }

        Directory.CreateDirectory(extractionRoot);

        using var archive = ZipFile.OpenRead(zipPath);
        var matchingEntries = archive.Entries
            .Where(entry => entry.FullName.StartsWith(archiveDirectory, StringComparison.OrdinalIgnoreCase) &&
                            !string.IsNullOrEmpty(entry.Name))
            .ToList();
        if (matchingEntries.Count == 0)
        {
            return null;
        }

        foreach (var entry in matchingEntries)
        {
            var relativeEntryPath = entry.FullName.Substring(archiveDirectory.Length).Replace('/', Path.DirectorySeparatorChar);
            var destinationPath = Path.Combine(extractionRoot, relativeEntryPath);
            var destinationDirectory = Path.GetDirectoryName(destinationPath);
            if (!string.IsNullOrWhiteSpace(destinationDirectory))
            {
                Directory.CreateDirectory(destinationDirectory!);
            }

            if (File.Exists(destinationPath))
            {
                var existingAttributes = File.GetAttributes(destinationPath);
                if ((existingAttributes & FileAttributes.ReadOnly) != 0)
                {
                    File.SetAttributes(destinationPath, existingAttributes & ~FileAttributes.ReadOnly);
                }
            }

            entry.ExtractToFile(destinationPath, overwrite: true);
            var extractedAttributes = File.GetAttributes(destinationPath);
            File.SetAttributes(destinationPath, extractedAttributes | FileAttributes.ReadOnly);
        }

        return File.Exists(extractedAssetPath) ? extractedAssetPath : null;
    }

    private static string? TryExtractArchiveRoot(string zipPath, string archiveRoot)
    {
        var normalizedArchiveRoot = archiveRoot.Replace('\\', '/').Trim('/');
        if (string.IsNullOrWhiteSpace(normalizedArchiveRoot))
        {
            return null;
        }

        var archivePrefix = normalizedArchiveRoot + "/";
        var extractionRoot = Path.Combine(
            GetArchiveExtractionBaseRoot(zipPath),
            normalizedArchiveRoot.Replace('/', Path.DirectorySeparatorChar));
        var completionMarker = Path.Combine(extractionRoot, ".copperfin-extract-complete");
        if (File.Exists(completionMarker))
        {
            return extractionRoot;
        }

        Directory.CreateDirectory(extractionRoot);

        using var archive = ZipFile.OpenRead(zipPath);
        var matchingEntries = archive.Entries
            .Where(entry => entry.FullName.StartsWith(archivePrefix, StringComparison.OrdinalIgnoreCase) &&
                            !string.IsNullOrEmpty(entry.Name))
            .ToList();
        if (matchingEntries.Count == 0)
        {
            return null;
        }

        foreach (var entry in matchingEntries)
        {
            var relativeEntryPath = entry.FullName.Substring(archivePrefix.Length).Replace('/', Path.DirectorySeparatorChar);
            var destinationPath = Path.Combine(extractionRoot, relativeEntryPath);
            var destinationDirectory = Path.GetDirectoryName(destinationPath);
            if (!string.IsNullOrWhiteSpace(destinationDirectory))
            {
                Directory.CreateDirectory(destinationDirectory!);
            }

            if (File.Exists(destinationPath))
            {
                var existingAttributes = File.GetAttributes(destinationPath);
                if ((existingAttributes & FileAttributes.ReadOnly) != 0)
                {
                    File.SetAttributes(destinationPath, existingAttributes & ~FileAttributes.ReadOnly);
                }
            }

            entry.ExtractToFile(destinationPath, overwrite: true);
            var extractedAttributes = File.GetAttributes(destinationPath);
            File.SetAttributes(destinationPath, extractedAttributes | FileAttributes.ReadOnly);
        }

        File.WriteAllText(completionMarker, "complete");
        return extractionRoot;
    }

    private static string? TryExtractArchive(string zipPath)
    {
        var extractionRoot = GetArchiveExtractionBaseRoot(zipPath);
        var completionMarker = Path.Combine(extractionRoot, ".copperfin-extract-complete");
        if (File.Exists(completionMarker))
        {
            return extractionRoot;
        }

        Directory.CreateDirectory(extractionRoot);

        using var archive = ZipFile.OpenRead(zipPath);
        var fileEntries = archive.Entries
            .Where(entry => !string.IsNullOrEmpty(entry.Name))
            .ToList();
        if (fileEntries.Count == 0)
        {
            return null;
        }

        foreach (var entry in fileEntries)
        {
            var destinationPath = Path.Combine(extractionRoot, entry.FullName.Replace('/', Path.DirectorySeparatorChar));
            var destinationDirectory = Path.GetDirectoryName(destinationPath);
            if (!string.IsNullOrWhiteSpace(destinationDirectory))
            {
                Directory.CreateDirectory(destinationDirectory!);
            }

            if (File.Exists(destinationPath))
            {
                var existingAttributes = File.GetAttributes(destinationPath);
                if ((existingAttributes & FileAttributes.ReadOnly) != 0)
                {
                    File.SetAttributes(destinationPath, existingAttributes & ~FileAttributes.ReadOnly);
                }
            }

            entry.ExtractToFile(destinationPath, overwrite: true);
            var extractedAttributes = File.GetAttributes(destinationPath);
            File.SetAttributes(destinationPath, extractedAttributes | FileAttributes.ReadOnly);
        }

        File.WriteAllText(completionMarker, "complete");
        return extractionRoot;
    }

    private static string GetArchiveExtractionBaseRoot(string zipPath)
    {
        var canonicalZipPath = Path.GetFullPath(zipPath);
        byte[] pathBytes = Encoding.UTF8.GetBytes(canonicalZipPath);
        byte[] pathHash;
        using (var sha256 = SHA256.Create())
        {
            pathHash = sha256.ComputeHash(pathBytes);
        }

        var pathHashText = BitConverter.ToString(pathHash).Replace("-", string.Empty).ToLowerInvariant();
        return Path.Combine(
            Path.GetTempPath(),
            "CopperfinDesignerSmokeRealAssets",
            Path.GetFileNameWithoutExtension(zipPath) + "-" + pathHashText.Substring(0, 16));
    }

    private static void DeleteReadOnlyDirectoryTree(string directoryPath)
    {
        if (!Directory.Exists(directoryPath))
        {
            return;
        }

        foreach (var filePath in Directory.EnumerateFiles(directoryPath, "*", SearchOption.AllDirectories))
        {
            File.SetAttributes(filePath, FileAttributes.Normal);
        }

        foreach (var nestedDirectoryPath in Directory.EnumerateDirectories(directoryPath, "*", SearchOption.AllDirectories))
        {
            File.SetAttributes(nestedDirectoryPath, FileAttributes.Normal);
        }

        Directory.Delete(directoryPath, recursive: true);
    }

    private static void CreateArchiveEntry(ZipArchive archive, string entryPath, string content)
    {
        var entry = archive.CreateEntry(entryPath);
        using var writer = new StreamWriter(entry.Open());
        writer.Write(content);
    }

    private static string? ExpandUserPath(string? path)
    {
        if (string.IsNullOrWhiteSpace(path))
        {
            return path;
        }

        var nonNullPath = path!;

        if (nonNullPath == "~")
        {
            return Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
        }

        if (nonNullPath.StartsWith("~/", StringComparison.Ordinal) || nonNullPath.StartsWith("~\\", StringComparison.Ordinal))
        {
            var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
            return Path.Combine(home, nonNullPath.Substring(2));
        }

        return nonNullPath;
    }

    private static IEnumerable<ListView> FindListViews(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is ListView listView)
            {
                yield return listView;
            }

            foreach (var nested in FindListViews(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasListViewColumnText(Control root, string text)
    {
        foreach (var listView in FindListViews(root))
        {
            foreach (ColumnHeader column in listView.Columns)
            {
                if (string.Equals(column.Text, text, StringComparison.Ordinal))
                {
                    return true;
                }
            }
        }

        return false;
    }

    private static void ApplyProjectSnapshotForColumnSmoke(CopperfinAssetEditorControl control)
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "project",
            ProjectWorkspace = new CopperfinStudioProjectWorkspace
            {
                Groups = new List<CopperfinStudioProjectGroup>
                {
                    new()
                    {
                        Id = "forms",
                        Title = "Forms",
                        ItemCount = 1,
                        ExcludedCount = 0
                    }
                }
            }
        };

        var controlType = typeof(CopperfinAssetEditorControl);
        var currentSnapshotField = controlType.GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        var populateSectionListMethod = controlType.GetMethod("PopulateSectionList", BindingFlags.Instance | BindingFlags.NonPublic);
        var configureObjectColumnsMethod = controlType.GetMethod("ConfigureObjectColumns", BindingFlags.Instance | BindingFlags.NonPublic);
        if (currentSnapshotField is null || populateSectionListMethod is null || configureObjectColumnsMethod is null)
        {
            throw new InvalidOperationException("Could not find CopperfinAssetEditorControl project-column smoke hooks.");
        }

        currentSnapshotField?.SetValue(control, snapshot);
        populateSectionListMethod.Invoke(control, new object?[] { null });
        configureObjectColumnsMethod.Invoke(control, Array.Empty<object>());
    }

    private static void ApplyProjectSnapshotForExplorerGroupTitleSmoke(CopperfinAssetEditorControl control, CopperfinStudioSnapshotDocument snapshot)
    {
        var controlType = typeof(CopperfinAssetEditorControl);
        var currentSnapshotField = controlType.GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        var populateSectionListMethod = ResolveNonPublicInstanceMethod(controlType, "PopulateSectionList", new object?[] { null });
        var populateObjectListMethod = ResolveNonPublicInstanceMethod(controlType, "PopulateObjectList", new object[] { true });
        if (currentSnapshotField is null)
        {
            throw new InvalidOperationException("Could not find CopperfinAssetEditorControl project-group smoke hooks.");
        }

        currentSnapshotField.SetValue(control, snapshot);
        populateSectionListMethod.Invoke(control, new object?[] { null });
        var projectGroupId = snapshot.ProjectWorkspace?.Entries.FirstOrDefault()?.GroupId;
        if (!string.IsNullOrWhiteSpace(projectGroupId))
        {
            var sectionListView = GetPrivateListView(control, "sectionListView");
            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioProjectGroup group &&
                                string.Equals(group.Id, projectGroupId, StringComparison.Ordinal);
            }
        }
        populateObjectListMethod.Invoke(control, new object[] { true });
    }

    private static void AssertProjectWorkspaceGroupTitles(
        CopperfinAssetEditorControl control,
        string[] expectedSectionTitles,
        string expectedObjectSubtitle,
        string message,
        string[]? rawLeakChecks = null)
    {
        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var actualSectionTitles = sectionListView.Items.Cast<ListViewItem>().Select(item => item.Text).ToArray();
        var actualObjectSubtitle = objectListView.Items.Cast<ListViewItem>().FirstOrDefault()?.SubItems[1].Text ?? string.Empty;
        var noRawLeaks = rawLeakChecks is null ||
                         !actualSectionTitles.Concat(new[] { actualObjectSubtitle })
                             .Any(text => rawLeakChecks.Any(raw => string.Equals(text, raw, StringComparison.Ordinal)));

        Expect(actualSectionTitles.SequenceEqual(expectedSectionTitles) &&
               string.Equals(actualObjectSubtitle, expectedObjectSubtitle, StringComparison.Ordinal) &&
               noRawLeaks,
            message);
    }

    private static void ApplyReportSnapshotForExplorerSmoke(CopperfinAssetEditorControl control, CopperfinStudioSnapshotDocument snapshot)
    {
        var controlType = typeof(CopperfinAssetEditorControl);
        var currentSnapshotField = controlType.GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        var populateSectionListMethod = controlType.GetMethod("PopulateSectionList", BindingFlags.Instance | BindingFlags.NonPublic);
        var populateObjectListMethod = controlType.GetMethod("PopulateObjectList", BindingFlags.Instance | BindingFlags.NonPublic);
        if (currentSnapshotField is null || populateSectionListMethod is null || populateObjectListMethod is null)
        {
            throw new InvalidOperationException("Could not find CopperfinAssetEditorControl report-explorer smoke hooks.");
        }

        currentSnapshotField.SetValue(control, snapshot);
        populateSectionListMethod.Invoke(control, new object?[] { null });
        populateObjectListMethod.Invoke(control, new object[] { true });
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorBatchUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorSectionUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

}
