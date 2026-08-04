// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

namespace Copperfin.VisualStudio;

internal static class CopperfinStudioOpenDialogFilter
{
    internal const string AssetPatterns = "*.pjx;*.scx;*.vcx;*.frx;*.lbx;*.mnx;*.prg";
    internal const string AllFilesPattern = "*.*";

    internal static string Build(CopperfinLocalization localization)
    {
        return string.Join(
            "|",
            localization.Text("Studio.OpenDialogFilter.Assets"),
            AssetPatterns,
            localization.Text("Studio.OpenDialogFilter.AllFiles"),
            AllFilesPattern);
    }
}
