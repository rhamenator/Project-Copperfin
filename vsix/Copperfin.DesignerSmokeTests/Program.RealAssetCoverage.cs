// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
            SmokeResolvedRealAssetCoverageClusterPart01();
            SmokeResolvedRealAssetCoverageClusterPart02();
            SmokeResolvedRealAssetCoverageClusterPart03();
            SmokeResolvedRealAssetCoverageClusterPart04();
            SmokeResolvedRealAssetCoverageClusterPart05();
            SmokeResolvedRealAssetCoverageClusterPart06();
            SmokeResolvedRealAssetCoverageClusterPart07();
        });
    }
}
