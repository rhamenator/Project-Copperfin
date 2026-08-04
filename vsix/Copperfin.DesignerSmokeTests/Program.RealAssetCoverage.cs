// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
    private static void SmokeResolvedRealAssetCoverageCluster()
    {
        WithResolvedRealAssetToolchain(() =>
        {
            RunRealAssetCoveragePart(nameof(SmokeResolvedRealAssetCoverageClusterPart01), SmokeResolvedRealAssetCoverageClusterPart01);
            RunRealAssetCoveragePart(nameof(SmokeResolvedRealAssetCoverageClusterPart02), SmokeResolvedRealAssetCoverageClusterPart02);
            RunRealAssetCoveragePart(nameof(SmokeResolvedRealAssetCoverageClusterPart03), SmokeResolvedRealAssetCoverageClusterPart03);
            RunRealAssetCoveragePart(nameof(SmokeResolvedRealAssetCoverageClusterPart04), SmokeResolvedRealAssetCoverageClusterPart04);
            RunRealAssetCoveragePart(nameof(SmokeResolvedRealAssetCoverageClusterPart05), SmokeResolvedRealAssetCoverageClusterPart05);
            RunRealAssetCoveragePart(nameof(SmokeResolvedRealAssetCoverageClusterPart06), SmokeResolvedRealAssetCoverageClusterPart06);
            RunRealAssetCoveragePart(nameof(SmokeResolvedRealAssetCoverageClusterPart07), SmokeResolvedRealAssetCoverageClusterPart07);
        });
    }

    private static void RunRealAssetCoveragePart(string partName, Action part)
    {
        var stopwatch = System.Diagnostics.Stopwatch.StartNew();
        Console.WriteLine($"START: {partName}");
        try
        {
            part();
        }
        finally
        {
            Console.WriteLine($"END: {partName} ({stopwatch.Elapsed.TotalSeconds:F1}s)");
        }
    }
}
