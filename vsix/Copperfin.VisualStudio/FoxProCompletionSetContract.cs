// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
