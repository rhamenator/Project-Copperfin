// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Linq;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void SmokeManagedToolboxPaletteContract()
    {
        var result = CopperfinStudioSnapshotClient.TryLoadToolboxPalette(
            "form",
            new CopperfinLocalization("es-419"));
        Expect(result.Success && result.Items.Count >= 6,
            "managed shared editor should load the native localized form toolbox palette");

        var textBox = result.Items.FirstOrDefault(item => item.Id == "textbox");
        Expect(textBox is not null &&
               textBox.Title == "Cuadro de texto" &&
               textBox.VfpClass == "TextBox" &&
               textBox.DefaultNamePrefix == "txt",
            "managed toolbox palette should preserve invariant item metadata and localized display text");

        var containerResult = CopperfinStudioSnapshotClient.TryLoadToolboxPalette(
            "form",
            new CopperfinLocalization("es-419"),
            "container");
        var grid = containerResult.Items.FirstOrDefault(item => item.Id == "grid");
        Expect(containerResult.Success &&
               grid is not null &&
               grid.Title == "Cuadricula" &&
               grid.VfpClass == "Grid",
            "managed shared editor should expose the localized container toolbox context with invariant metadata");

        var localization = new CopperfinLocalization("pt-BR");
        Expect(localization.Text("AssetEditor.Toolbox.ContextLabel") == "Contexto:" &&
               localization.Text("AssetEditor.Toolbox.Context.Container") == "Contêiner",
            "toolbox context controls should use the active locale catalog");

        var arguments = CopperfinStudioHostBridge.BuildToolboxCreateArguments(
            "sample.scx",
            "textbox",
            "form");
        Expect(arguments.Contains("--toolbox-create \"textbox\"", StringComparison.Ordinal) &&
               arguments.Contains("--toolbox-context \"form\"", StringComparison.Ordinal) &&
               arguments.Contains("--path \"sample.scx\"", StringComparison.Ordinal),
            "managed toolbox create bridge should preserve stable host command tokens");

        var builderPlanArguments = CopperfinStudioHostBridge.BuildBuilderLaunchPlanArguments(
            "form-builder",
            "form",
            "sample project.pjx",
            7,
            "Orders Form",
            "form-1");
        Expect(builderPlanArguments.Contains("--builder-launch-plan \"form-builder\"", StringComparison.Ordinal) &&
               builderPlanArguments.Contains("--builder-context \"form\"", StringComparison.Ordinal) &&
               builderPlanArguments.Contains("--path \"sample project.pjx\"", StringComparison.Ordinal) &&
               builderPlanArguments.Contains("--record 7", StringComparison.Ordinal) &&
               builderPlanArguments.Contains("--object-name \"Orders Form\"", StringComparison.Ordinal) &&
               builderPlanArguments.Contains("--unique-id \"form-1\"", StringComparison.Ordinal),
               "managed builder plan bridge should preserve invariant command tokens and selection identity");

        var builderExecuteArguments = CopperfinStudioHostBridge.BuildBuilderExecuteArguments(
            "form-builder",
            "form",
            "copperfin-builder --safe",
            "sample project.pjx",
            7,
            "Orders Form",
            "form-1");
        Expect(builderExecuteArguments.Contains("--builder-execute \"form-builder\"", StringComparison.Ordinal) &&
               builderExecuteArguments.Contains("--builder-launch-command \"copperfin-builder --safe\"", StringComparison.Ordinal) &&
               builderExecuteArguments.Contains("--admit-ui-launch true", StringComparison.Ordinal) &&
               builderExecuteArguments.Contains("--admit-builder-execution true", StringComparison.Ordinal) &&
               builderExecuteArguments.Contains("--unique-id \"form-1\"", StringComparison.Ordinal),
            "managed builder execution bridge should preserve explicit command, admission gates, and selection identity");

        var previousBuilderCommand = Environment.GetEnvironmentVariable("COPPERFIN_BUILDER_LAUNCH_COMMAND");
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILDER_LAUNCH_COMMAND", null);
            var unconfigured = CopperfinStudioSnapshotClient.TryExecuteBuilder(
                "form-builder",
                "form",
                "sample project.pjx",
                localization: new CopperfinLocalization("es-419"));
            Expect(!unconfigured.Success &&
                   unconfigured.Error == "La ejecución del constructor no está configurada.",
                "builder execution should fail closed and remain localized when no explicit launch command is configured");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILDER_LAUNCH_COMMAND", previousBuilderCommand);
        }
    }
}
