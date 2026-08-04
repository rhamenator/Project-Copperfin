
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
    private static void SmokeResolvedRealAssetCoverageClusterPart04()
    {
        SmokeRealAssetHostBackedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("HPOS", "9000"),
                new KeyValuePair<string, string>("WIDTH", "20000")
            },
            originalValues: new[]
            {
                new KeyValuePair<string, string>("HPOS", "8645.833"),
                new KeyValuePair<string, string>("WIDTH", "19687.500")
            },
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 96457,
                Bottom = 33436,
                Width = 96457,
                Height = 33436
            });
        SmokeRealAssetHostBackedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("HPOS", "6500"),
                new KeyValuePair<string, string>("WIDTH", "16000")
            },
            originalValues: new[]
            {
                new KeyValuePair<string, string>("HPOS", "6250.000"),
                new KeyValuePair<string, string>("WIDTH", "15104.167")
            },
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 22500,
                Bottom = 10000,
                Width = 22500,
                Height = 10000
            });
        SmokeRealAssetHostBackedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("VPOS", "7000"),
                new KeyValuePair<string, string>("HEIGHT", "3500")
            },
            originalValues: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6666.667"),
                new KeyValuePair<string, string>("HEIGHT", "3333.333")
            },
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 96457,
                Bottom = 33436,
                Width = 96457,
                Height = 33436
            });
        SmokeRealAssetHostBackedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6800"),
                new KeyValuePair<string, string>("HEIGHT", "1800")
            },
            originalValues: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6666.667"),
                new KeyValuePair<string, string>("HEIGHT", "1666.667")
            },
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 21354,
                Bottom = 10000,
                Width = 21354,
                Height = 10000
            });
        SmokeRealAssetHostBackedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "1"),
                new KeyValuePair<string, string>("FONTSIZE", "18")
            },
            originalValues: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "3"),
                new KeyValuePair<string, string>("FONTSIZE", "20")
            },
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 96457,
                Bottom = 33436,
                Width = 96457,
                Height = 33436
            });
        SmokeRealAssetHostBackedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "1"),
                new KeyValuePair<string, string>("FONTSIZE", "9")
            },
            originalValues: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "0"),
                new KeyValuePair<string, string>("FONTSIZE", "8")
            },
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 6,
            propertyName: "FONTFACE",
            originalValue: string.Empty,
            updatedValue: "Arial",
            expectedObjectTitle: "wiz_general",
            expectedSectionTitle: "Unplaced objects",
            expectedSectionCount: 4,
            expectLabel: false,
            expectUnplacedObject: true,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 75000,
                Bottom = 21666,
                Width = 75000,
                Height = 21666
            });
        SmokeRealAssetHostBackedPlacementRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 6,
            propertyName: "VPOS",
            originalValue: "11145.833",
            updatedValue: "2400",
            expectedObjectTitle: "wiz_general",
            initialSectionTitle: "Unplaced objects",
            updatedSectionTitle: "Page Header",
            expectedSectionCount: 4,
            expectLabel: false,
            expectedOriginalUnplacedObjectCount: 7,
            expectedUpdatedUnplacedObjectCount: 6);
        SmokeRealAssetHostBackedDuplicateRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            duplicateUniqueId: "DUPREAL01",
            expectedSourceObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedOriginalSectionObjectCount: 1,
            expectedUpdatedSectionObjectCount: 2);
        SmokeRealAssetHostBackedDuplicateRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            duplicateUniqueId: "LDUPREAL1",
            expectedSourceObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedOriginalSectionObjectCount: 1,
            expectedUpdatedSectionObjectCount: 2);
        SmokeRealAssetHostBackedRenameRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedOriginalUniqueId: "_RC60MC40R",
            expectedRenamedUniqueId: "RLIVEREN1",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedRenameRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedOriginalUniqueId: "_QV30QY1DL",
            expectedRenamedUniqueId: "LLIVEREN1",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedDeleteRestoreRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedUniqueId: "_RC60MC40R",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedOriginalSectionObjectCount: 1,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeleteRestoreRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedUniqueId: "_QV30QY1DL",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedOriginalSectionObjectCount: 1,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedUniqueId: "_RC60MC40R",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            propertyName: "HPOS",
            originalRawValue: "8645.833",
            updatedRawValue: "9000",
            expectedOriginalLayoutValue: 8645,
            expectedUpdatedLayoutValue: 9000,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedUniqueId: "_QV30QY1DL",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            propertyName: "HPOS",
            originalRawValue: "6250.000",
            updatedRawValue: "6500",
            expectedOriginalLayoutValue: 6250,
            expectedUpdatedLayoutValue: 6500,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedUniqueId: "_RC60MC40R",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("VPOS", "7000"),
                new KeyValuePair<string, string>("HEIGHT", "3500")
            },
            originalRawValues: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6666.667"),
                new KeyValuePair<string, string>("HEIGHT", "3333.333")
            },
            originalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("VPOS", 6666),
                new KeyValuePair<string, int?>("HEIGHT", 3333)
            },
            updatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("VPOS", 7000),
                new KeyValuePair<string, int?>("HEIGHT", 3500)
            },
            expectedSectionCount: 6,
            expectLabel: false,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedUniqueId: "_QV30QY1DL",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6800"),
                new KeyValuePair<string, string>("HEIGHT", "1800")
            },
            originalRawValues: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6666.667"),
                new KeyValuePair<string, string>("HEIGHT", "1666.667")
            },
            originalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("VPOS", 6666),
                new KeyValuePair<string, int?>("HEIGHT", 1666)
            },
            updatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("VPOS", 6800),
                new KeyValuePair<string, int?>("HEIGHT", 1800)
            },
            expectedSectionCount: 5,
            expectLabel: true,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedUniqueId: "_RC60MC40R",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "1"),
                new KeyValuePair<string, string>("FONTSIZE", "18")
            },
            originalRawValues: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "3"),
                new KeyValuePair<string, string>("FONTSIZE", "20")
            },
            originalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("FONTSTYLE", null),
                new KeyValuePair<string, int?>("FONTSIZE", null)
            },
            updatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("FONTSTYLE", null),
                new KeyValuePair<string, int?>("FONTSIZE", null)
            },
            expectedSectionCount: 6,
            expectLabel: false,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedUniqueId: "_QV30QY1DL",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "1"),
                new KeyValuePair<string, string>("FONTSIZE", "9")
            },
            originalRawValues: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "0"),
                new KeyValuePair<string, string>("FONTSIZE", "8")
            },
            originalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("FONTSTYLE", null),
                new KeyValuePair<string, int?>("FONTSIZE", null)
            },
            updatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("FONTSTYLE", null),
                new KeyValuePair<string, int?>("FONTSIZE", null)
            },
            expectedSectionCount: 5,
            expectLabel: true,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedUniqueId: "_RC60MC40R",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("HPOS", "9000"),
                new KeyValuePair<string, string>("WIDTH", "20000")
            },
            originalRawValues: new[]
            {
                new KeyValuePair<string, string>("HPOS", "8645.833"),
                new KeyValuePair<string, string>("WIDTH", "19687.500")
            },
            originalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("HPOS", 8645),
                new KeyValuePair<string, int?>("WIDTH", 19687)
            },
            updatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("HPOS", 9000),
                new KeyValuePair<string, int?>("WIDTH", 20000)
            },
            expectedSectionCount: 6,
            expectLabel: false,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedBatchPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedUniqueId: "_QV30QY1DL",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("HPOS", "6500"),
                new KeyValuePair<string, string>("WIDTH", "16000")
            },
            originalRawValues: new[]
            {
                new KeyValuePair<string, string>("HPOS", "6250.000"),
                new KeyValuePair<string, string>("WIDTH", "15104.167")
            },
            originalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("HPOS", 6250),
                new KeyValuePair<string, int?>("WIDTH", 15104)
            },
            updatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("HPOS", 6500),
                new KeyValuePair<string, int?>("WIDTH", 16000)
            },
            expectedSectionCount: 5,
            expectLabel: true,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedRenameRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedOriginalUniqueId: "_RC60MC40R",
            expectedRenamedUniqueId: "RDELREN1",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedRenameRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedOriginalUniqueId: "_QV30QY1DL",
            expectedRenamedUniqueId: "LDELREN1",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeRealAssetHostBackedDeletedDuplicateRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedOriginalUniqueId: "_RC60MC40R",
            expectedDuplicatedUniqueId: "RDELDUP1",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedDeletedDuplicateRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedOriginalUniqueId: "_QV30QY1DL",
            expectedDuplicatedUniqueId: "LDELDUP1",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedDeletedReorderRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            reorderedSourceRecordIndex: 28,
            reorderedSourceUniqueId: "_QUC0L59IE",
            reorderedSourceObjectTitle: "_QUC0L59IE",
            companionRecordIndex: 13,
            companionUniqueId: "_QEE1DSSPJ",
            companionObjectTitle: "_QEE1DSSPJ",
            expectedSectionTitle: "Group Header",
            initialSectionRecordIndex: 5,
            reorderedSectionRecordIndex: 6,
            expectedSectionCount: 6,
            expectedVisibleSectionObjectCount: 8,
            expectedReorderedRecordIndex: 0,
            expectedCompanionRecordIndex: 14,
            expectLabel: false);
        SmokeRealAssetHostBackedReorderRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE2V.FRX"),
            reorderedSourceRecordIndex: 6,
            reorderedSourceUniqueId: "_QEE1DR6KY",
            reorderedSourceObjectTitle: "DATE()",
            companionRecordIndex: 7,
            companionUniqueId: "_QEE1DUBXM",
            companionObjectTitle: "_QEE1DUBXM",
            expectedSectionTitle: "Title",
            initialSectionRecordIndex: 1,
            reorderedSectionRecordIndex: 2,
            expectedSectionCount: 4,
            expectedSectionObjectCount: 3,
            expectedInitialSectionRecordOrder: new[] { 7, 8, 6 },
            expectedReorderedSectionRecordOrder: new[] { 7, 8, 0 },
            expectedReorderedRecordIndex: 0,
            expectLabel: false);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            propertyName: "HPOS",
            updatedPropertyValue: 9000,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "9000",
            expectedUpdatedRawValue: "9000",
            expectedOriginalRawValue: "8645.833",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            propertyName: "HPOS",
            updatedPropertyValue: 6500,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "6500",
            expectedUpdatedRawValue: "6500",
            expectedOriginalRawValue: "6250.000",
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            propertyName: "VPOS",
            updatedPropertyValue: 7000,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "7000",
            expectedUpdatedRawValue: "7000",
            expectedOriginalRawValue: "6666.667",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            propertyName: "VPOS",
            updatedPropertyValue: 6800,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "6800",
            expectedUpdatedRawValue: "6800",
            expectedOriginalRawValue: "6666.667",
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            propertyName: "WIDTH",
            updatedPropertyValue: 20000,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "20000",
            expectedUpdatedRawValue: "20000",
            expectedOriginalRawValue: "19687.500",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            propertyName: "WIDTH",
            updatedPropertyValue: 16000,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "16000",
            expectedUpdatedRawValue: "16000",
            expectedOriginalRawValue: "15104.167",
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            propertyName: "HEIGHT",
            updatedPropertyValue: 3500,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "3500",
            expectedUpdatedRawValue: "3500",
            expectedOriginalRawValue: "3333.333",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            propertyName: "HEIGHT",
            updatedPropertyValue: 1800,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "1800",
            expectedUpdatedRawValue: "1800",
            expectedOriginalRawValue: "1666.667",
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            propertyName: "FONTFACE",
            updatedPropertyValue: "Arial",
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "Arial",
            expectedUpdatedRawValue: "Arial",
            expectedOriginalRawValue: "Times New Roman",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            propertyName: "FONTFACE",
            updatedPropertyValue: "Calibri",
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "Calibri",
            expectedUpdatedRawValue: "Calibri",
            expectedOriginalRawValue: "Arial",
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            propertyName: "FONTSTYLE",
            updatedPropertyValue: 1,
            expectedOriginalSelectionValue: "3",
            expectedUpdatedSelectionValue: "1",
            expectedUpdatedRawValue: "1",
            expectedOriginalRawValue: "3",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            propertyName: "FONTSTYLE",
            updatedPropertyValue: 1,
            expectedOriginalSelectionValue: "0",
            expectedUpdatedSelectionValue: "1",
            expectedUpdatedRawValue: "1",
            expectedOriginalRawValue: "0",
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            propertyName: "FONTSIZE",
            updatedPropertyValue: 18,
            expectedOriginalSelectionValue: "20",
            expectedUpdatedSelectionValue: "18",
            expectedUpdatedRawValue: "18",
            expectedOriginalRawValue: "20",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            propertyName: "FONTSIZE",
            updatedPropertyValue: 9,
            expectedOriginalSelectionValue: "8",
            expectedUpdatedSelectionValue: "9",
            expectedUpdatedRawValue: "9",
            expectedOriginalRawValue: "8",
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorDeleteRestoreRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            expectedOriginalSectionObjectCount: 1,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeleteRestoreRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            expectedOriginalSectionObjectCount: 1,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            propertyName: "HPOS",
            updatedPropertyValue: 9000,
            expectedUpdatedSelectionValue: "9000",
            expectedOriginalRawValue: "8645.833",
            expectedUpdatedRawValue: "9000",
            expectedOriginalLayoutValue: 8645,
            expectedUpdatedLayoutValue: 9000,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            propertyName: "HPOS",
            updatedPropertyValue: 6500,
            expectedUpdatedSelectionValue: "6500",
            expectedOriginalRawValue: "6250.000",
            expectedUpdatedRawValue: "6500",
            expectedOriginalLayoutValue: 6250,
            expectedUpdatedLayoutValue: 6500,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            propertyName: "FONTSTYLE",
            updatedPropertyValue: 1,
            expectedUpdatedSelectionValue: "1",
            expectedOriginalRawValue: "3",
            expectedUpdatedRawValue: "1",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            propertyName: "FONTSTYLE",
            updatedPropertyValue: 1,
            expectedUpdatedSelectionValue: "1",
            expectedOriginalRawValue: "0",
            expectedUpdatedRawValue: "1",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            propertyName: "FONTSIZE",
            updatedPropertyValue: 18,
            expectedUpdatedSelectionValue: "18",
            expectedOriginalRawValue: "20",
            expectedUpdatedRawValue: "18",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            propertyName: "FONTSIZE",
            updatedPropertyValue: 9,
            expectedUpdatedSelectionValue: "9",
            expectedOriginalRawValue: "8",
            expectedUpdatedRawValue: "9",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            propertyName: "VPOS",
            updatedPropertyValue: 7000,
            expectedUpdatedSelectionValue: "7000",
            expectedOriginalRawValue: "6666.667",
            expectedUpdatedRawValue: "7000",
            expectedOriginalLayoutValue: 6666,
            expectedUpdatedLayoutValue: 7000,
            expectedDeletedSectionVisibleObjectCount: 1);
    }
}
