// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

            var activeProject = Path.Combine(root, "active-project.pjx");
            var selectedProject = Path.Combine(root, "selected-project.pjx");
            File.WriteAllText(activeProject, "active project");
            File.WriteAllText(selectedProject, "selected project");
            var selectedProjectResolution = CopperfinProjectPathResolver.ResolveProjectWorkflowPath(
                Path.Combine(root, "active.prg"),
                activeProject,
                new[] { selectedProject });
            Expect(selectedProjectResolution == selectedProject,
                "project workflow commands should prefer an explicitly selected project over the active document project");

            var activeProjectResolution = CopperfinProjectPathResolver.ResolveProjectWorkflowPath(
                Path.Combine(root, "active.prg"),
                activeProject,
                Array.Empty<string?>());
            Expect(activeProjectResolution == activeProject,
                "project workflow commands should retain active-document project fallback without a selected project");

            var invalidSelectionResolution = CopperfinProjectPathResolver.ResolveProjectWorkflowPath(
                Path.Combine(root, "active.prg"),
                activeProject,
                new[] { Path.Combine(root, "missing-selected.pjx") });
            Expect(invalidSelectionResolution == activeProject,
                "project workflow commands should ignore missing selected projects before active-document fallback");

            var repositoryRoot = FindStudioTargetSelectionRepositoryRoot();
            Expect(repositoryRoot is not null,
                "project selection tests should locate the repository root for command wiring checks");
            if (repositoryRoot is not null)
            {
                var commandSource = File.ReadAllText(Path.Combine(
                    repositoryRoot,
                    "vsix",
                    "Copperfin.VisualStudio",
                    "CopperfinProjectCommands.cs"));
                const string resolverCall = "CopperfinProjectSelection.ResolveProjectPath(dte)";
                var firstCall = commandSource.IndexOf(resolverCall, StringComparison.Ordinal);
                var secondCall = firstCall < 0
                    ? -1
                    : commandSource.IndexOf(resolverCall, firstCall + resolverCall.Length, StringComparison.Ordinal);
                Expect(firstCall >= 0 && secondCall >= 0,
                    "project workflow command status and execution should share the same project resolver");
            }
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
