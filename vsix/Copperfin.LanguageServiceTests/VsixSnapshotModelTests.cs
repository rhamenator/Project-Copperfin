// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Text.Json;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestReportLayoutObjectPreservesPictureMetadata()
    {
        const string json =
            "{\"recordIndex\":4,\"objectKind\":\"label\",\"objectKindFieldIndex\":0," +
            "\"objectKindMemoBlockNumber\":0,\"picture\":\"@I\",\"pictureFieldIndex\":7," +
            "\"pictureMemoBlockNumber\":3,\"pictureAlignment\":\"center\"}";
        var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };
        var layoutObject = JsonSerializer.Deserialize<CopperfinStudioReportLayoutObject>(json, options);

        Expect(layoutObject is not null &&
               layoutObject.ObjectKind == "label" &&
               layoutObject.Picture == "@I" &&
               layoutObject.PictureFieldIndex == 7 &&
               layoutObject.PictureMemoBlockNumber == 3 &&
               layoutObject.PictureAlignment == "center",
            "managed report layout objects should preserve label PICTURE and provenance metadata");
    }

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

    private static void TestReportSectionsPreserveNullableOrdinalMetadata()
    {
        const string liveJson =
            "{\"id\":\"detail_4\",\"sectionIndex\":2,\"sectionCount\":5}";
        const string deletedJson =
            "{\"id\":\"deleted_detail\",\"deleted\":true,\"sectionIndex\":null,\"sectionCount\":0}";
        var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };

        var live = JsonSerializer.Deserialize<CopperfinStudioReportSection>(liveJson, options);
        Expect(live is not null &&
               live.SectionIndex == 2 &&
               live.SectionCount == 5,
            "managed report sections should preserve native section ordinal metadata");

        var deleted = JsonSerializer.Deserialize<CopperfinStudioReportSection>(deletedJson, options);
        Expect(deleted is not null &&
               deleted.Deleted &&
               deleted.SectionIndex is null &&
               deleted.SectionCount == 0,
            "managed deleted report sections should preserve null section indexes and zero counts");

        var missing = JsonSerializer.Deserialize<CopperfinStudioReportSection>(
            "{\"id\":\"legacy\"}",
            options);
        Expect(missing is not null &&
               missing.SectionIndex is null &&
               missing.SectionCount is null,
            "managed report sections should tolerate snapshots without ordinal metadata");
    }

    private static void TestHostSnapshotPreservesUnplacedReportLayoutObjects()
    {
        var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };
        foreach (var (assetFamily, isLabel) in new[]
        {
            (AssetFamily: "report", IsLabel: false),
            (AssetFamily: "label", IsLabel: true)
        })
        {
            var json = "{\"AssetFamily\":\"" + assetFamily + "\",\"Objects\":[" +
                       "{\"RecordIndex\":10,\"Title\":\"detail.line\"}," +
                       "{\"RecordIndex\":12,\"Title\":\"orphan.note\"}]," +
                       "\"ReportLayout\":{\"IsLabel\":" + (isLabel ? "true" : "false") +
                       ",\"Sections\":[{\"Id\":\"detail\",\"Title\":\"Detail\",\"Objects\":[" +
                       "{\"RecordIndex\":10,\"Title\":\"detail.line\",\"SectionObjectIndex\":0,\"SectionObjectCount\":1}]}]," +
                       "\"UnplacedObjects\":[{\"RecordIndex\":12,\"Title\":\"orphan.note\",\"SectionObjectIndex\":null,\"SectionObjectCount\":0}]}}";

            var document = JsonSerializer.Deserialize<CopperfinStudioSnapshotDocument>(json, options);
            var layout = document?.ReportLayout;
            var unplaced = layout?.UnplacedObjects.Count == 1 ? layout.UnplacedObjects[0] : null;
            var placed = layout?.Sections.Count == 1 && layout.Sections[0].Objects.Count == 1
                ? layout.Sections[0].Objects[0]
                : null;

            Expect(document?.AssetFamily == assetFamily && layout?.IsLabel == isLabel &&
                   unplaced is not null && unplaced.RecordIndex == 12 && unplaced.SectionObjectIndex is null &&
                   placed is not null && placed.RecordIndex == 10 && placed.SectionObjectIndex == 0,
                $"host-shaped {assetFamily} report layouts should preserve null unplaced and numeric placed section indexes");
        }
    }
}
