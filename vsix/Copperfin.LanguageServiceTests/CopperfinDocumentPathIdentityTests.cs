// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.IO;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestDocumentPathIdentityUsesFilesystemSemantics()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin_language_service_tests",
            "document_path_identity",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        var upperPath = Path.Combine(root, "Invoice.frx");
        var lowerPath = Path.Combine(root, "invoice.frx");
        try
        {
            File.WriteAllText(upperPath, "upper");
            File.WriteAllText(lowerPath, "lower");
            var supportsCaseDistinctFiles =
                string.Equals(File.ReadAllText(upperPath), "upper", StringComparison.Ordinal) &&
                string.Equals(File.ReadAllText(lowerPath), "lower", StringComparison.Ordinal);
            if (OperatingSystem.IsLinux())
            {
                Expect(supportsCaseDistinctFiles,
                    "the Linux regression fixture should reside on a case-sensitive filesystem");
            }

            var defaultKeys = new HashSet<string>(CopperfinDocumentPathIdentity.CreateComparer())
            {
                CopperfinDocumentPathIdentity.Normalize(upperPath),
                CopperfinDocumentPathIdentity.Normalize(lowerPath)
            };
            Expect(
                defaultKeys.Count == (OperatingSystem.IsWindows() || !supportsCaseDistinctFiles ? 1 : 2),
                "default document identity should follow the current platform and filesystem semantics");

            var posixComparer = CopperfinDocumentPathIdentity.CreateComparer(isWindowsOverride: false);
            var upperKey = CopperfinDocumentPathIdentity.Normalize(upperPath, isWindowsOverride: false);
            var lowerKey = CopperfinDocumentPathIdentity.Normalize(lowerPath, isWindowsOverride: false);
            var posixKeys = new HashSet<string>(posixComparer) { upperKey, lowerKey };
            Expect(
                posixKeys.Count == (supportsCaseDistinctFiles ? 2 : 1),
                "POSIX document identity should preserve physically distinct case-only files and collapse case aliases on insensitive volumes");

            var relativeUpperPath = Path.GetRelativePath(Environment.CurrentDirectory, upperPath);
            var relativeUpperKey = CopperfinDocumentPathIdentity.Normalize(relativeUpperPath, isWindowsOverride: false);
            Expect(
                posixComparer.Equals(upperKey, relativeUpperKey),
                "relative and absolute paths to the same document should share one POSIX identity");
            Expect(
                CopperfinDocumentPathIdentity.TryNormalize(relativeUpperPath, out var tryRelativeUpperKey, isWindowsOverride: false) &&
                posixComparer.Equals(upperKey, tryRelativeUpperKey),
                "nullable-safe normalization should preserve valid relative document identity");
            Expect(
                CopperfinDocumentPathIdentity.TryNormalize(upperPath, out var tryUpperKey, isWindowsOverride: false) &&
                posixComparer.Equals(upperKey, tryUpperKey),
                "nullable-safe normalization should preserve valid absolute document identity");
            Expect(
                !CopperfinDocumentPathIdentity.TryNormalize(null, out var nullKey) && string.IsNullOrEmpty(nullKey),
                "nullable-safe normalization should reject null without leaking a path identity");
            Expect(
                !CopperfinDocumentPathIdentity.TryNormalize("  ", out var blankKey) && string.IsNullOrEmpty(blankKey),
                "nullable-safe normalization should reject blank input without leaking a path identity");
            Expect(
                !CopperfinDocumentPathIdentity.TryNormalize("invalid\0path", out var invalidKey) && string.IsNullOrEmpty(invalidKey),
                "nullable-safe normalization should contain invalid path arguments without leaking a path identity");

            var reportPath = Path.Combine(root, "Report.frx");
            var missingCaseAlias = Path.Combine(root, "REPORT.FRX");
            File.WriteAllText(reportPath, "report");
            if (!File.Exists(missingCaseAlias))
            {
                Expect(
                    string.Equals(
                        CopperfinDocumentPathIdentity.Normalize(missingCaseAlias, isWindowsOverride: false),
                        Path.GetFullPath(missingCaseAlias),
                        StringComparison.Ordinal),
                    "a nonexistent case-mismatched POSIX path should not rebind to an existing document");
            }

            var duplicateKeys = new HashSet<string>(posixComparer) { upperKey, upperKey };
            Expect(duplicateKeys.Count == 1,
                "opening an ordinary duplicate document path should retain one identity");

            var windowsComparer = CopperfinDocumentPathIdentity.CreateComparer(isWindowsOverride: true);
            var windowsUpperKey = CopperfinDocumentPathIdentity.Normalize(upperPath, isWindowsOverride: true);
            var windowsLowerKey = CopperfinDocumentPathIdentity.Normalize(lowerPath, isWindowsOverride: true);
            Expect(
                windowsComparer.Equals(windowsUpperKey, windowsLowerKey),
                "Windows document identity should remain case-insensitive");
        }
        finally
        {
            TryDelete(root);
        }
    }
}
