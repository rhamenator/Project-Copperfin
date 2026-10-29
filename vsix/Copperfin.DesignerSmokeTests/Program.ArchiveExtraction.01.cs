// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
using System;
using System.IO;
using System.IO.Compression;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
    // RQ-CF-DESIGNER-ARCHIVE-7214: exercise actual ZIP consumers, not just path-string predicates.
    private static void SmokeArchiveExtractionContainment()
    {
        var fixtureRoot = Path.Combine(Path.GetTempPath(), "CopperfinArchiveContainment-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(fixtureRoot);
        try
        {
            foreach (var rootedPath in new[] { "C:\\escape.txt", "C:escape.txt", "\\\\server\\share\\escape.txt" })
            {
                var rejected = false;
                try { GetArchiveEntryDestinationPath(fixtureRoot, rootedPath); }
                catch (InvalidDataException) { rejected = true; }
                Expect(rejected, "drive and UNC names must be rejected without attempting filesystem writes");
            }

            foreach (var mode in new[] { "whole", "root", "directory" })
            {
                var escapeName = "escape-" + Guid.NewGuid().ToString("N") + ".txt";
                foreach (var attack in new[] { "../" + escapeName, "..\\" + escapeName, "nested/../../" + escapeName, Path.Combine(fixtureRoot, escapeName) })
                {
                    var zipPath = Path.Combine(fixtureRoot, Guid.NewGuid().ToString("N") + ".zip");
                    var extractionRoot = GetArchiveExtractionBaseRoot(zipPath);
                    var prefix = mode == "whole" ? "" : "assets/";
                    try
                    {
                        using (var archive = ZipFile.Open(zipPath, ZipArchiveMode.Create))
                        {
                            CreateArchiveEntry(archive, prefix + "valid.txt", "valid");
                            CreateArchiveEntry(archive, prefix + attack, "rejected");
                        }

                        var rejected = false;
                        try
                        {
                            if (mode == "whole") TryExtractArchive(zipPath);
                            else if (mode == "root") TryExtractArchiveRoot(zipPath, "assets");
                            else TryExtractArchiveContainingDirectory(zipPath, "assets/valid.txt");
                        }
                        catch (InvalidDataException) { rejected = true; }
                        Expect(rejected, "archive extraction must reject unsafe entry paths in " + mode + ": " + attack);
                        Expect(!File.Exists(Path.Combine(extractionRoot, mode == "whole" ? "valid.txt" : "assets/valid.txt")), "all selected entry paths must be validated before any file is extracted");
                        Expect(!File.Exists(Path.Combine(extractionRoot, escapeName)), "rejected entries must not escape the selected archive directory");
                        Expect(!File.Exists(Path.Combine(Path.GetDirectoryName(extractionRoot)!, escapeName)), "rejected entries must not escape the extraction root");
                        Expect(!File.Exists(Path.Combine(fixtureRoot, escapeName)), "rooted entries must not write outside the extraction root");
                        Expect(!File.Exists(Path.Combine(extractionRoot, ".copperfin-extract-complete")), "rejected archives must not receive completion markers");
                        Expect(!File.Exists(Path.Combine(extractionRoot, "assets", ".copperfin-extract-complete")), "rejected selected roots must not receive completion markers");
                    }
                    finally { DeleteReadOnlyDirectoryTree(extractionRoot); }
                }
            }

            foreach (var mode in new[] { "whole", "root", "directory" })
            {
                var zipPath = Path.Combine(fixtureRoot, "valid-" + mode + ".zip");
                var extractionRoot = GetArchiveExtractionBaseRoot(zipPath);
                try
                {
                    using (var archive = ZipFile.Open(zipPath, ZipArchiveMode.Create))
                    {
                        CreateArchiveEntry(archive, "assets/nested/valid.txt", "unchanged");
                    }
                    if (mode == "whole") TryExtractArchive(zipPath);
                    else if (mode == "root") TryExtractArchiveRoot(zipPath, "assets");
                    else TryExtractArchiveContainingDirectory(zipPath, "assets/nested/valid.txt");
                    var extracted = Path.Combine(extractionRoot, "assets", "nested", "valid.txt");
                    Expect(File.ReadAllText(extracted) == "unchanged", "valid nested archive content must remain unchanged");
                    Expect((File.GetAttributes(extracted) & FileAttributes.ReadOnly) != 0, "valid extracted assets remain read-only");
                }
                finally { DeleteReadOnlyDirectoryTree(extractionRoot); }
            }
        }
        finally { DeleteReadOnlyDirectoryTree(fixtureRoot); }
    }
}
