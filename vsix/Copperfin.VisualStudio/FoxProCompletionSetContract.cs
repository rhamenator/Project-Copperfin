// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

namespace Copperfin.VisualStudio;

internal static class FoxProCompletionSetContract
{
    public const string Identity = "CopperfinFoxPro";
    public const string DisplayNameKey = "LanguageService.IntelliSense.CompletionSet.FoxPro";

    public static string GetDisplayName(CopperfinLocalization localization)
    {
        return localization.Text(DisplayNameKey);
    }
}
