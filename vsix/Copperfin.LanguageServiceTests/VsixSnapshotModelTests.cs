// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Text.Json;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestReportLayoutObjectPreservesNullableSectionIndex()
    {
        const string unplacedJson =
            "{\"recordIndex\":7,\"containingSectionId\":\"\",\"sectionObjectIndex\":null,\"sectionObjectCount\":0}";
        const string placedJson =
            "{\"recordIndex\":8,\"containingSectionId\":\"detail_4\",\"sectionObjectIndex\":2,\"sectionObjectCount\":3}";
        var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };

        var unplaced = JsonSerializer.Deserialize<CopperfinStudioReportLayoutObject>(unplacedJson, options);
        Expect(unplaced is not null &&
               unplaced.SectionObjectIndex is null &&
               unplaced.ContainingSectionRecordIndex is null,
            "unplaced report objects should preserve null section indexes during managed deserialization");

        var placed = JsonSerializer.Deserialize<CopperfinStudioReportLayoutObject>(placedJson, options);
        Expect(placed is not null &&
               placed.SectionObjectIndex == 2 &&
               placed.SectionObjectCount == 3,
            "placed report objects should preserve numeric section indexes during managed deserialization");
    }
}
