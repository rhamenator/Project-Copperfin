// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.IO;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestProjectSelectionResolvesContainingProjectForActiveAssets()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin_language_service_tests",
            "project_selection",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        try
        {
            var projectPath = Path.Combine(root, "demo.pjx");
            File.WriteAllText(projectPath, "project");

            foreach (var extension in new[] { ".prg", ".scx", ".vcx", ".frx", ".lbx", ".mnx" })
            {
                var activeDocumentPath = Path.Combine(root, "active" + extension);
                File.WriteAllText(activeDocumentPath, extension);
                var resolved = CopperfinProjectPathResolver.ResolveActiveDocumentProjectPath(
                    activeDocumentPath,
                    projectPath);
                Expect(resolved == projectPath,
                    $"an active {extension} document should resolve its containing PJX without a selected Solution Explorer item");
            }

            var directProject = CopperfinProjectPathResolver.ResolveActiveDocumentProjectPath(
                projectPath,
                null);
            Expect(directProject == projectPath,
                "an active PJX should retain direct project-path resolution");

            var missingContainingProject = CopperfinProjectPathResolver.ResolveActiveDocumentProjectPath(
                Path.Combine(root, "active.prg"),
                Path.Combine(root, "missing.pjx"));
            Expect(missingContainingProject is null,
                "a missing containing PJX should not enable project workflows");
        }
        finally
        {
            try
            {
                if (Directory.Exists(root))
                {
                    Directory.Delete(root, recursive: true);
                }
            }
            catch
            {
            }
        }
    }
}
