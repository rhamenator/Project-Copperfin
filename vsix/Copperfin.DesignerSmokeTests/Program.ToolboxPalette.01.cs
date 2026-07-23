// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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

        var arguments = CopperfinStudioHostBridge.BuildToolboxCreateArguments(
            "sample.scx",
            "textbox",
            "form");
        Expect(arguments.Contains("--toolbox-create \"textbox\"", StringComparison.Ordinal) &&
               arguments.Contains("--toolbox-context \"form\"", StringComparison.Ordinal) &&
               arguments.Contains("--path \"sample.scx\"", StringComparison.Ordinal),
            "managed toolbox create bridge should preserve stable host command tokens");
    }
}
