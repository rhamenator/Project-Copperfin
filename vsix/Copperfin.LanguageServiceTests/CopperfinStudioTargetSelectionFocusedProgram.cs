// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static int failures;

    private static int Main()
    {
        TestStudioTargetSelectionPrefersSelectedItemsForItemCommands();

        if (failures != 0)
        {
            Console.Error.WriteLine($"{failures} Studio target-selection test(s) failed.");
            return 1;
        }

        Console.WriteLine("All Studio target-selection tests passed.");
        return 0;
    }

    private static void Expect(bool condition, string message)
    {
        if (!condition)
        {
            Console.Error.WriteLine($"FAIL: {message}");
            failures++;
        }
    }
}
