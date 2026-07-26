// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.IO;
using System.Text.Json;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestVsixDebuggerRestartInvalidatesStaleSessions()
    {
        var repositoryRoot = FindRepositoryRoot();
        Expect(repositoryRoot is not null,
            "VSIX debugger lifecycle test should locate the repository root");
        if (repositoryRoot is null)
        {
            return;
        }

        var editorSource = File.ReadAllText(Path.Combine(
            repositoryRoot,
            "vsix",
            "Copperfin.VisualStudio",
            "CopperfinAssetEditorControl.cs"));
        Expect(editorSource.Contains("private int debugSessionGeneration", StringComparison.Ordinal) &&
               editorSource.Contains("var requestGeneration = ++debugSessionGeneration", StringComparison.Ordinal) &&
               editorSource.Contains("requestGeneration != debugSessionGeneration", StringComparison.Ordinal) &&
               editorSource.Contains("CopperfinRuntimeDebugClient.Stop(session)", StringComparison.Ordinal),
            "VSIX debugger restart should discard stale asynchronous sessions instead of applying them after a newer request");
        Expect(editorSource.Contains("debugSessionGeneration++;", StringComparison.Ordinal) &&
               editorSource.Contains("public void LoadDocument", StringComparison.Ordinal),
            "VSIX document loading should invalidate an in-flight debugger session request");
    }

    private static void TestVsixEditorHostThemeContract()
    {
        var repositoryRoot = FindRepositoryRoot();
        Expect(repositoryRoot is not null,
            "VSIX editor theme test should locate the repository root");
        if (repositoryRoot is null)
        {
            return;
        }

        var editorSource = File.ReadAllText(Path.Combine(
            repositoryRoot,
            "vsix",
            "Copperfin.VisualStudio",
            "CopperfinAssetEditorControl.cs"));
        Expect(editorSource.Contains("ApplyVisualStudioHostTheme()", StringComparison.Ordinal) &&
               editorSource.Contains("VSColorTheme.GetThemedColor", StringComparison.Ordinal) &&
               editorSource.Contains("EnvironmentColors.ToolWindowBackgroundColorKey", StringComparison.Ordinal) &&
               editorSource.Contains("EnvironmentColors.ToolWindowTextColorKey", StringComparison.Ordinal),
            "VSIX editor host mode should read Visual Studio background and text theme colors");
        Expect(editorSource.Contains("background = SystemColors.Control", StringComparison.Ordinal) &&
               editorSource.Contains("foreground = SystemColors.ControlText", StringComparison.Ordinal),
            "VSIX editor host theme should fall back to system colors outside a live Visual Studio shell");
        Expect(editorSource.Contains("if (child is CopperfinDesignSurfaceControl)", StringComparison.Ordinal),
            "VSIX editor host theme should leave the designer canvas rendering contract independent from shell chrome");
        Expect(editorSource.Contains("if (child is not Label)", StringComparison.Ordinal) &&
               editorSource.Contains("child.BackColor = background", StringComparison.Ordinal) &&
               editorSource.Contains("child.ForeColor = foreground", StringComparison.Ordinal),
            "VSIX editor host theme should recolor buttons, lists, editors, and labels without forcing label backgrounds opaque");
    }

    private static void TestVsixCommandWindowRegistration()
    {
        var repositoryRoot = FindRepositoryRoot();
        Expect(repositoryRoot is not null,
            "VSIX command-window registration test should locate the repository root");
        if (repositoryRoot is null)
        {
            return;
        }

        var vsixRoot = Path.Combine(repositoryRoot, "vsix", "Copperfin.VisualStudio");
        var studioRoot = Path.Combine(repositoryRoot, "vsix", "Copperfin.Studio");
        var packageSource = File.ReadAllText(Path.Combine(vsixRoot, "CopperfinPackage.cs"));
        var paneSource = File.ReadAllText(Path.Combine(vsixRoot, "CopperfinCommandWindowPane.cs"));
        var editorPaneSource = File.ReadAllText(Path.Combine(vsixRoot, "CopperfinAssetEditorPane.cs"));
        var commandSource = File.ReadAllText(Path.Combine(vsixRoot, "ShowCopperfinCommandWindowCommand.cs"));
        var controlSource = File.ReadAllText(Path.Combine(vsixRoot, "CopperfinCommandWindowControl.cs"));
        var commandTable = File.ReadAllText(Path.Combine(vsixRoot, "Copperfin.vsct"));
        var projectSource = File.ReadAllText(Path.Combine(vsixRoot, "Copperfin.VisualStudio.csproj"));
        var standaloneSource = File.ReadAllText(Path.Combine(studioRoot, "StudioMainForm.cs"));
        var editorSource = File.ReadAllText(Path.Combine(vsixRoot, "CopperfinAssetEditorControl.cs"));

        Expect(packageSource.Contains("ProvideToolWindow(", StringComparison.Ordinal) &&
               packageSource.Contains("typeof(CopperfinCommandWindowPane)", StringComparison.Ordinal),
            "VSIX package should register the Copperfin command window");
        Expect(packageSource.Contains("Style = VsDockStyle.Tabbed", StringComparison.Ordinal) &&
               packageSource.Contains("Orientation = ToolWindowOrientation.Bottom", StringComparison.Ordinal) &&
               packageSource.Contains("Window = ToolWindowGuids80.Outputwindow", StringComparison.Ordinal),
            "VSIX command window should default to the Visual Studio Output-window bottom tabbed region");
        Expect(packageSource.Contains("ShowCopperfinCommandWindowCommand.InitializeAsync(this)", StringComparison.Ordinal),
            "VSIX package should initialize the command-window command");
        Expect(paneSource.Contains("[Guid(PackageGuids.CommandWindowString)]", StringComparison.Ordinal) &&
               paneSource.Contains(": ToolWindowPane", StringComparison.Ordinal) &&
               paneSource.Contains("VSIX.CommandWindow.Title", StringComparison.Ordinal) &&
               paneSource.Contains("new CopperfinCommandWindowControl(localization, ExecuteCommandWindowInput)", StringComparison.Ordinal) &&
               paneSource.Contains("CopperfinAssetEditorPane.FindForDocument", StringComparison.Ordinal),
            "VSIX command window should be a localized ToolWindowPane with a stable identity");
        Expect(editorPaneSource.Contains("FindForDocument", StringComparison.Ordinal) &&
               editorPaneSource.Contains("ExecuteCommandWindowInput", StringComparison.Ordinal) &&
               editorPaneSource.Contains("OpenPanes", StringComparison.Ordinal),
            "VSIX editor panes should expose a normalized active-document lookup for Command evaluation");
        Expect(commandSource.Contains("ShowToolWindowAsync", StringComparison.Ordinal) &&
               commandSource.Contains("typeof(CopperfinCommandWindowPane)", StringComparison.Ordinal),
            "VSIX command should show the registered command window instead of launching a second shell");
        Expect(controlSource.Contains("VSColorTheme.GetThemedColor", StringComparison.Ordinal) &&
               controlSource.Contains("EnvironmentColors.ToolWindowBackgroundColorKey", StringComparison.Ordinal) &&
               controlSource.Contains("VsShellUtilities.GetEnvironmentFont", StringComparison.Ordinal) &&
               controlSource.Contains("commandInput.KeyDown", StringComparison.Ordinal) &&
               controlSource.Contains("commandExecutor", StringComparison.Ordinal) &&
               controlSource.Contains("SubmitCommandForTest", StringComparison.Ordinal),
            "VSIX command window should use Visual Studio theme/font settings and expose an interactive executor boundary");
        Expect(standaloneSource.Contains("ExecuteCommandWindowInput", StringComparison.Ordinal) &&
               standaloneSource.Contains("new StudioCommandWindowControl(this.localization, ExecuteCommandWindowInput)", StringComparison.Ordinal) &&
               editorSource.Contains("VSIX.CommandWindow.Unsupported", StringComparison.Ordinal) &&
               editorSource.Contains("EvaluateWatchAsync(currentDebugSession", StringComparison.Ordinal),
            "standalone Command window should route its constrained expression form through the active debugger watch evaluator");
        Expect(File.ReadAllText(Path.Combine(vsixRoot, "OpenInCopperfinStudioCommand.cs"))
                   .Contains("FromVisualStudioUiCulture()", StringComparison.Ordinal) &&
               File.ReadAllText(Path.Combine(vsixRoot, "CopperfinProjectCommands.cs"))
                   .Contains("FromVisualStudioUiCulture()", StringComparison.Ordinal),
            "VSIX command captions should follow the Visual Studio UI culture");
        Expect(projectSource.Contains("Extension.vsixlangpack", StringComparison.Ordinal) &&
               projectSource.Contains("IncludeInVSIX", StringComparison.Ordinal),
            "VSIX project should package localized installation metadata");
        Expect(commandTable.Contains("ShowCopperfinCommandWindowCommand", StringComparison.Ordinal) &&
               commandTable.Contains("value=\"0x0300\"", StringComparison.Ordinal) &&
               commandTable.Contains(".Copperfin.ShowCommandWindow", StringComparison.Ordinal),
            "VSIX command table should expose the command-window command with a stable command identity");

        foreach (var locale in new[] { "en-US", "es-419", "pt-BR", "qps-ploc" })
        {
            var catalogPath = Path.Combine(repositoryRoot, "resources", "locales", locale, "strings.json");
            using var document = JsonDocument.Parse(File.ReadAllText(catalogPath));
            var root = document.RootElement;
            Expect(root.TryGetProperty("VSIX.CommandWindow.Title", out var title) &&
                   !string.IsNullOrWhiteSpace(title.GetString()),
                $"{locale} catalog should provide the command-window title");
            Expect(root.TryGetProperty("VSIX.CommandWindow.Ready", out var ready) &&
                   !string.IsNullOrWhiteSpace(ready.GetString()),
                $"{locale} catalog should provide the command-window ready status");
            Expect(root.TryGetProperty("VSIX.CommandWindow.Unavailable", out var unavailable) &&
                   !string.IsNullOrWhiteSpace(unavailable.GetString()),
                $"{locale} catalog should provide the command-window unavailable message");
        }

        foreach (var languageFolder in new[] { "es", "pt", "qps-ploc" })
        {
            var languagePackPath = Path.Combine(vsixRoot, languageFolder, "Extension.vsixlangpack");
            Expect(File.Exists(languagePackPath),
                $"VSIX should include an installation language pack for {languageFolder}");
            if (!File.Exists(languagePackPath))
            {
                continue;
            }

            var languagePack = File.ReadAllText(languagePackPath);
            Expect(languagePack.Contains("PackageLanguagePackManifest", StringComparison.Ordinal) &&
                   languagePack.Contains("<DisplayName>", StringComparison.Ordinal) &&
                   languagePack.Contains("<Description>", StringComparison.Ordinal),
                $"VSIX installation language pack for {languageFolder} should contain localized metadata");
        }
    }
}
