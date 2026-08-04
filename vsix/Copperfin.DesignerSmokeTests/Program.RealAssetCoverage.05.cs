
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
    private static void SmokeResolvedRealAssetCoverageClusterPart05()
    {
        SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            propertyName: "VPOS",
            updatedPropertyValue: 6800,
            expectedUpdatedSelectionValue: "6800",
            expectedOriginalRawValue: "6666.667",
            expectedUpdatedRawValue: "6800",
            expectedOriginalLayoutValue: 6666,
            expectedUpdatedLayoutValue: 6800,
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
            propertyName: "WIDTH",
            updatedPropertyValue: 20000,
            expectedUpdatedSelectionValue: "20000",
            expectedOriginalRawValue: "19687.500",
            expectedUpdatedRawValue: "20000",
            expectedOriginalLayoutValue: 19687,
            expectedUpdatedLayoutValue: 20000,
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
            propertyName: "WIDTH",
            updatedPropertyValue: 16000,
            expectedUpdatedSelectionValue: "16000",
            expectedOriginalRawValue: "15104.167",
            expectedUpdatedRawValue: "16000",
            expectedOriginalLayoutValue: 15104,
            expectedUpdatedLayoutValue: 16000,
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
            propertyName: "HEIGHT",
            updatedPropertyValue: 3500,
            expectedUpdatedSelectionValue: "3500",
            expectedOriginalRawValue: "3333.333",
            expectedUpdatedRawValue: "3500",
            expectedOriginalLayoutValue: 3333,
            expectedUpdatedLayoutValue: 3500,
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
            propertyName: "HEIGHT",
            updatedPropertyValue: 1800,
            expectedUpdatedSelectionValue: "1800",
            expectedOriginalRawValue: "1666.667",
            expectedUpdatedRawValue: "1800",
            expectedOriginalLayoutValue: 1666,
            expectedUpdatedLayoutValue: 1800,
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
            propertyName: "FONTFACE",
            updatedPropertyValue: "Arial",
            expectedUpdatedSelectionValue: "Arial",
            expectedOriginalRawValue: "Times New Roman",
            expectedUpdatedRawValue: "Arial",
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
            propertyName: "FONTFACE",
            updatedPropertyValue: "Calibri",
            expectedUpdatedSelectionValue: "Calibri",
            expectedOriginalRawValue: "Arial",
            expectedUpdatedRawValue: "Calibri",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("VPOS", "7000"),
                new KeyValuePair<string, string>("HEIGHT", "3500")
            },
            expectedUpdatedSelectionValues: new[]
            {
                new KeyValuePair<string, string>("VPOS", "7000"),
                new KeyValuePair<string, string>("HEIGHT", "3500")
            },
            expectedOriginalRawValues: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6666.667"),
                new KeyValuePair<string, string>("HEIGHT", "3333.333")
            },
            expectedOriginalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("VPOS", 6666),
                new KeyValuePair<string, int?>("HEIGHT", 3333)
            },
            expectedUpdatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("VPOS", 7000),
                new KeyValuePair<string, int?>("HEIGHT", 3500)
            },
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6800"),
                new KeyValuePair<string, string>("HEIGHT", "1800")
            },
            expectedUpdatedSelectionValues: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6800"),
                new KeyValuePair<string, string>("HEIGHT", "1800")
            },
            expectedOriginalRawValues: new[]
            {
                new KeyValuePair<string, string>("VPOS", "6666.667"),
                new KeyValuePair<string, string>("HEIGHT", "1666.667")
            },
            expectedOriginalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("VPOS", 6666),
                new KeyValuePair<string, int?>("HEIGHT", 1666)
            },
            expectedUpdatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("VPOS", 6800),
                new KeyValuePair<string, int?>("HEIGHT", 1800)
            },
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "1"),
                new KeyValuePair<string, string>("FONTSIZE", "18")
            },
            expectedUpdatedSelectionValues: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "1"),
                new KeyValuePair<string, string>("FONTSIZE", "18")
            },
            expectedOriginalRawValues: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "3"),
                new KeyValuePair<string, string>("FONTSIZE", "20")
            },
            expectedOriginalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("FONTSTYLE", null),
                new KeyValuePair<string, int?>("FONTSIZE", null)
            },
            expectedUpdatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("FONTSTYLE", null),
                new KeyValuePair<string, int?>("FONTSIZE", null)
            },
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "1"),
                new KeyValuePair<string, string>("FONTSIZE", "9")
            },
            expectedUpdatedSelectionValues: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "1"),
                new KeyValuePair<string, string>("FONTSIZE", "9")
            },
            expectedOriginalRawValues: new[]
            {
                new KeyValuePair<string, string>("FONTSTYLE", "0"),
                new KeyValuePair<string, string>("FONTSIZE", "8")
            },
            expectedOriginalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("FONTSTYLE", null),
                new KeyValuePair<string, int?>("FONTSIZE", null)
            },
            expectedUpdatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("FONTSTYLE", null),
                new KeyValuePair<string, int?>("FONTSIZE", null)
            },
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("HPOS", "9000"),
                new KeyValuePair<string, string>("WIDTH", "20000")
            },
            expectedUpdatedSelectionValues: new[]
            {
                new KeyValuePair<string, string>("HPOS", "9000"),
                new KeyValuePair<string, string>("WIDTH", "20000")
            },
            expectedOriginalRawValues: new[]
            {
                new KeyValuePair<string, string>("HPOS", "8645.833"),
                new KeyValuePair<string, string>("WIDTH", "19687.500")
            },
            expectedOriginalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("HPOS", 8645),
                new KeyValuePair<string, int?>("WIDTH", 19687)
            },
            expectedUpdatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("HPOS", 9000),
                new KeyValuePair<string, int?>("WIDTH", 20000)
            },
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            propertyChanges: new[]
            {
                new KeyValuePair<string, string>("HPOS", "6500"),
                new KeyValuePair<string, string>("WIDTH", "16000")
            },
            expectedUpdatedSelectionValues: new[]
            {
                new KeyValuePair<string, string>("HPOS", "6500"),
                new KeyValuePair<string, string>("WIDTH", "16000")
            },
            expectedOriginalRawValues: new[]
            {
                new KeyValuePair<string, string>("HPOS", "6250.000"),
                new KeyValuePair<string, string>("WIDTH", "15104.167")
            },
            expectedOriginalLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("HPOS", 6250),
                new KeyValuePair<string, int?>("WIDTH", 15104)
            },
            expectedUpdatedLayoutValues: new[]
            {
                new KeyValuePair<string, int?>("HPOS", 6500),
                new KeyValuePair<string, int?>("WIDTH", 16000)
            },
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedRenameCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedOriginalUniqueId: "_RC60MC40R",
            expectedRenamedUniqueId: "RDELREN1",
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedRenameCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedOriginalUniqueId: "_QV30QY1DL",
            expectedRenamedUniqueId: "LDELREN1",
            expectedDeletedSectionVisibleObjectCount: 1);
        SmokeAssetEditorDeletedDuplicateCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedOriginalUniqueId: "_RC60MC40R",
            expectedDuplicatedUniqueId: "RDELDUP1");
        SmokeAssetEditorDeletedDuplicateCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedOriginalUniqueId: "_QV30QY1DL",
            expectedDuplicatedUniqueId: "LDELDUP1");
        SmokeAssetEditorDeletedReorderCommandWithRealAsset(
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
        SmokeAssetEditorReorderCommandWithRealAsset(
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
            expectedReorderedSectionRecordOrder: new[] { 7, 8, 0 },
            expectedReorderedRecordIndex: 0,
            expectLabel: false);
        SmokeAssetEditorRenameCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedOriginalUniqueId: "_RC60MC40R",
            expectedRenamedUniqueId: "RLIVEREN1");
        SmokeAssetEditorRenameCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedOriginalUniqueId: "_QV30QY1DL",
            expectedRenamedUniqueId: "LLIVEREN1");
        SmokeAssetEditorNudgeCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
            expectedSectionRecordIndex: 1,
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUniqueId: "_RC60MC40R",
            deltaHpos: 200,
            deltaVpos: 300,
            expectedOriginalRawHpos: "8645.833",
            expectedOriginalRawVpos: "6666.667",
            expectedUpdatedRawHpos: "8845.833",
            expectedUpdatedRawVpos: "6966.667",
            expectedOriginalLayoutHpos: 8645,
            expectedOriginalLayoutVpos: 6666,
            expectedUpdatedLayoutHpos: 8845,
            expectedUpdatedLayoutVpos: 6966,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 96457,
                Bottom = 33436,
                Width = 96457,
                Height = 33436
            });
        SmokeAssetEditorNudgeCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            deltaHpos: 200,
            deltaVpos: 300,
            expectedOriginalRawHpos: "6250.000",
            expectedOriginalRawVpos: "6666.667",
            expectedUpdatedRawHpos: "6450",
            expectedUpdatedRawVpos: "6966.667",
            expectedOriginalLayoutHpos: 6250,
            expectedOriginalLayoutVpos: 6666,
            expectedUpdatedLayoutHpos: 6450,
            expectedUpdatedLayoutVpos: 6966,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 21554,
                Bottom = 10000,
                Width = 21554,
                Height = 10000
            });
        SmokeAssetEditorSnapToGridCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 13,
            expectedSectionTitle: "Page Header",
            expectedSectionRecordIndex: 2,
            expectedObjectTitle: "\"TITLE\"",
            expectedSectionCount: 4,
            expectLabel: false,
            expectedUniqueId: "_QVL0O0NVK",
            expectedOriginalRawHpos: "416.667",
            expectedOriginalRawVpos: "3541.667",
            expectedUpdatedRawHpos: "420",
            expectedUpdatedRawVpos: "3540",
            expectedOriginalLayoutHpos: 416,
            expectedOriginalLayoutVpos: 3541,
            expectedUpdatedLayoutHpos: 420,
            expectedUpdatedLayoutVpos: 3540);
        SmokeAssetEditorSnapToGridCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            expectedOriginalRawHpos: "6250.000",
            expectedOriginalRawVpos: "6666.667",
            expectedUpdatedRawHpos: "6252",
            expectedUpdatedRawVpos: "6672",
            expectedOriginalLayoutHpos: 6250,
            expectedOriginalLayoutVpos: 6666,
            expectedUpdatedLayoutHpos: 6252,
            expectedUpdatedLayoutVpos: 6672);
        SmokeAssetEditorAlignLeftCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Sedna/DataExplorer/DATAEXPLORERQUICKREPORT.FRX"),
            anchorRecordIndex: 4,
            targetRecordIndex: 5,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 2,
            expectedAnchorObjectTitle: "\"Customerid\"",
            expectedTargetObjectTitle: "customerid",
            expectedSectionCount: 3,
            expectedAnchorUniqueId: "_1WB13DS3B",
            expectedTargetUniqueId: "_1WB13DS4P",
            expectedOriginalTargetRawHpos: "11979.167",
            expectedUpdatedTargetRawHpos: "0",
            expectedOriginalTargetLayoutHpos: 11979,
            expectedUpdatedTargetLayoutHpos: 0);
        SmokeAssetEditorAlignTopCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Sedna/DataExplorer/DATAEXPLORERQUICKREPORT.FRX"),
            anchorRecordIndex: 4,
            targetRecordIndex: 5,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 2,
            expectedAnchorObjectTitle: "\"Customerid\"",
            expectedTargetObjectTitle: "customerid",
            expectedSectionCount: 3,
            expectedAnchorUniqueId: "_1WB13DS3B",
            expectedTargetUniqueId: "_1WB13DS4P",
            expectedOriginalTargetRawVpos: "9583.333",
            expectedUpdatedTargetRawVpos: "9687.5",
            expectedOriginalTargetLayoutVpos: 9583,
            expectedUpdatedTargetLayoutVpos: 9687);
        SmokeAssetEditorResizeToAnchorSizeCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Sedna/DataExplorer/DATAEXPLORERQUICKREPORT.FRX"),
            anchorRecordIndex: 4,
            targetRecordIndex: 5,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 2,
            expectedAnchorUniqueId: "_1WB13DS3B",
            expectedTargetUniqueId: "_1WB13DS4P",
            expectedSectionCount: 3,
            expectedOriginalTargetRawWidth: "30312.500",
            expectedOriginalTargetRawHeight: "1875.000",
            expectedUpdatedTargetRawWidth: "7604.167",
            expectedUpdatedTargetRawHeight: "1666.667",
            expectedOriginalTargetLayoutWidth: 30312,
            expectedOriginalTargetLayoutHeight: 1875,
            expectedUpdatedTargetLayoutWidth: 7604,
            expectedUpdatedTargetLayoutHeight: 1666);
        SmokeAssetEditorResizeToAnchorWidthCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Sedna/DataExplorer/DATAEXPLORERQUICKREPORT.FRX"),
            anchorRecordIndex: 4,
            targetRecordIndex: 5,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 2,
            expectedAnchorUniqueId: "_1WB13DS3B",
            expectedTargetUniqueId: "_1WB13DS4P",
            expectedSectionCount: 3,
            expectedOriginalTargetRawWidth: "30312.500",
            expectedOriginalTargetRawHeight: "1875.000",
            expectedUpdatedTargetRawWidth: "7604.167",
            expectedUpdatedTargetRawHeight: "1875.000",
            expectedOriginalTargetLayoutWidth: 30312,
            expectedOriginalTargetLayoutHeight: 1875,
            expectedUpdatedTargetLayoutWidth: 7604,
            expectedUpdatedTargetLayoutHeight: 1875);
        SmokeAssetEditorResizeToAnchorHeightCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Sedna/DataExplorer/DATAEXPLORERQUICKREPORT.FRX"),
            anchorRecordIndex: 4,
            targetRecordIndex: 5,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 2,
            expectedAnchorUniqueId: "_1WB13DS3B",
            expectedTargetUniqueId: "_1WB13DS4P",
            expectedSectionCount: 3,
            expectedOriginalTargetRawWidth: "30312.500",
            expectedOriginalTargetRawHeight: "1875.000",
            expectedUpdatedTargetRawWidth: "30312.500",
            expectedUpdatedTargetRawHeight: "1666.667",
            expectedOriginalTargetLayoutWidth: 30312,
            expectedOriginalTargetLayoutHeight: 1875,
            expectedUpdatedTargetLayoutWidth: 30312,
            expectedUpdatedTargetLayoutHeight: 1666);
        SmokeAssetEditorDistributeHorizontallyCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            selectedRecordIndexes: new[] { 12, 13, 11 },
            focusedRecordIndex: 13,
            expectedSectionTitle: "Page Header",
            expectedSectionRecordIndex: 2,
            expectedFocusedObjectTitle: "\"TITLE\"",
            expectedSectionCount: 4,
            expectedFocusedUniqueId: "_QVL0O0NVK",
            expectedOriginalFocusedRawHpos: "416.667",
            expectedUpdatedFocusedRawHpos: "364.583",
            expectedOriginalFocusedLayoutHpos: 416,
            expectedUpdatedFocusedLayoutHpos: 364);
        SmokeAssetEditorDistributeVerticallyCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            selectedRecordIndexes: new[] { 12, 13, 11 },
            focusedRecordIndex: 13,
            expectedSectionTitle: "Page Header",
            expectedSectionRecordIndex: 2,
            expectedFocusedObjectTitle: "\"TITLE\"",
            expectedSectionCount: 4,
            expectedFocusedUniqueId: "_QVL0O0NVK",
            expectedOriginalFocusedRawVpos: "3541.667",
            expectedUpdatedFocusedRawVpos: "4427.083",
            expectedOriginalFocusedLayoutVpos: 3541,
            expectedUpdatedFocusedLayoutVpos: 4427);
        SmokeAssetEditorSnapVerticallyCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 13,
            expectedSectionTitle: "Page Header",
            expectedSectionRecordIndex: 2,
            expectedObjectTitle: "\"TITLE\"",
            expectedSectionCount: 4,
            expectLabel: false,
            expectedUniqueId: "_QVL0O0NVK",
            expectedOriginalRawHpos: "416.667",
            expectedOriginalRawVpos: "3541.667",
            expectedUpdatedRawHpos: "416.667",
            expectedUpdatedRawVpos: "3540",
            expectedOriginalLayoutHpos: 416,
            expectedOriginalLayoutVpos: 3541,
            expectedUpdatedLayoutHpos: 416,
            expectedUpdatedLayoutVpos: 3540);
        SmokeAssetEditorSnapVerticallyCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            expectedOriginalRawHpos: "6250.000",
            expectedOriginalRawVpos: "6666.667",
            expectedUpdatedRawHpos: "6250.000",
            expectedUpdatedRawVpos: "6672",
            expectedOriginalLayoutHpos: 6250,
            expectedOriginalLayoutVpos: 6666,
            expectedUpdatedLayoutHpos: 6250,
            expectedUpdatedLayoutVpos: 6672);
        SmokeAssetEditorSnapHorizontallyCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 13,
            expectedSectionTitle: "Page Header",
            expectedSectionRecordIndex: 2,
            expectedObjectTitle: "\"TITLE\"",
            expectedSectionCount: 4,
            expectLabel: false,
            expectedUniqueId: "_QVL0O0NVK",
            expectedOriginalRawHpos: "416.667",
            expectedOriginalRawVpos: "3541.667",
            expectedUpdatedRawHpos: "420",
            expectedUpdatedRawVpos: "3541.667",
            expectedOriginalLayoutHpos: 416,
            expectedOriginalLayoutVpos: 3541,
            expectedUpdatedLayoutHpos: 420,
            expectedUpdatedLayoutVpos: 3541);
        SmokeAssetEditorSnapHorizontallyCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
            expectedSectionRecordIndex: 3,
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUniqueId: "_QV30QY1DL",
            expectedOriginalRawHpos: "6250.000",
            expectedOriginalRawVpos: "6666.667",
            expectedUpdatedRawHpos: "6252",
            expectedUpdatedRawVpos: "6666.667",
            expectedOriginalLayoutHpos: 6250,
            expectedOriginalLayoutVpos: 6666,
            expectedUpdatedLayoutHpos: 6252,
            expectedUpdatedLayoutVpos: 6666);
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 1,
            expectedSectionTitle: "Title",
            propertyName: "VPOS",
            updatedPropertyValue: 500,
            expectedOriginalSelectionValue: "0",
            expectedUpdatedSelectionValue: "500",
            expectedOriginalRawValue: string.Empty,
            expectedUpdatedRawValue: "500",
            expectedOriginalLayoutValue: 0,
            expectedUpdatedLayoutValue: 500,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 1,
            expectedOriginalContainedObjects: new[]
            {
                new ExpectedSectionContainedObjectGeometry
                {
                    RecordIndex = 7,
                    Top = 6666,
                    SectionRelativeTop = 6666,
                    Bottom = 9999,
                    SectionRelativeBottom = 9999
                }
            },
            expectedUpdatedContainedObjects: new[]
            {
                new ExpectedSectionContainedObjectGeometry
                {
                    RecordIndex = 7,
                    Top = 7166,
                    SectionRelativeTop = 6666,
                    Bottom = 10499,
                    SectionRelativeBottom = 9999
                }
            });
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 3,
            expectedSectionTitle: "Detail",
            propertyName: "VPOS",
            updatedPropertyValue: 400,
            expectedOriginalSelectionValue: "0",
            expectedUpdatedSelectionValue: "400",
            expectedOriginalRawValue: string.Empty,
            expectedUpdatedRawValue: "400",
            expectedOriginalLayoutValue: 0,
            expectedUpdatedLayoutValue: 400,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedObjectCount: 1,
            expectedUntouchedSections: new[]
            {
                CreateStylelblUntouchedColumnFooterSection()
            },
            expectedOriginalContainedObjects: new[]
            {
                new ExpectedSectionContainedObjectGeometry
                {
                    RecordIndex = 6,
                    Top = 6666,
                    SectionRelativeTop = 6666,
                    Bottom = 8332,
                    SectionRelativeBottom = 8332
                }
            },
            expectedUpdatedContainedObjects: new[]
            {
                new ExpectedSectionContainedObjectGeometry
                {
                    RecordIndex = 6,
                    Top = 7066,
                    SectionRelativeTop = 6666,
                    Bottom = 8732,
                    SectionRelativeBottom = 8332
                }
            });
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Header",
            propertyName: "VPOS",
            updatedPropertyValue: 500,
            expectedOriginalSelectionValue: "0",
            expectedUpdatedSelectionValue: "500",
            expectedOriginalRawValue: string.Empty,
            expectedUpdatedRawValue: "500",
            expectedOriginalLayoutValue: 0,
            expectedUpdatedLayoutValue: 500,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 8,
            expectedExplorerSectionTitle: "Group Header - OneToMany",
            expectedGrouping: CreateBandedmGroupHeaderGrouping(),
            expectedUntouchedSections: new[]
            {
                CreateBandedmUntouchedGroupFooterSection()
            },
            expectedOriginalContainedObjects: CreateBandedmGroupHeaderContainedObjects(0),
            expectedUpdatedContainedObjects: CreateBandedmGroupHeaderContainedObjects(500));
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Header",
            propertyName: "HEIGHT",
            updatedPropertyValue: 8000,
            expectedOriginalSelectionValue: "7605",
            expectedUpdatedSelectionValue: "8000",
            expectedOriginalRawValue: "7605.000",
            expectedUpdatedRawValue: "8000",
            expectedOriginalLayoutValue: 7605,
            expectedUpdatedLayoutValue: 8000,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 8,
            expectedExplorerSectionTitle: "Group Header - OneToMany",
            expectedGrouping: CreateBandedmGroupHeaderGrouping(),
            expectedUntouchedSections: new[]
            {
                CreateBandedmUntouchedGroupFooterSection()
            },
            expectedOriginalContainedObjects: CreateBandedmGroupHeaderContainedObjects(0),
            expectedUpdatedContainedObjects: CreateBandedmGroupHeaderContainedObjects(0));
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Header",
            propertyName: "EXPR",
            updatedPropertyValue: "customer.company",
            expectedOriginalSelectionValue: "OneToMany",
            expectedUpdatedSelectionValue: "customer.company",
            expectedOriginalRawValue: "OneToMany",
            expectedUpdatedRawValue: "customer.company",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 8,
            expectedExplorerSectionTitle: "Group Header - OneToMany",
            expectedGrouping: CreateBandedmGroupHeaderGrouping(),
            expectedUntouchedSections: new[]
            {
                CreateBandedmUntouchedGroupFooterSection()
            },
            expectedUpdatedExplorerSectionTitle: "Group Header - customer.company",
            expectedUpdatedGrouping: CreateUpdatedBandedmGroupHeaderGrouping(),
            expectedUpdatedUntouchedSections: new[]
            {
                CreateBandedmUpdatedGroupFooterSection()
            },
            expectedUndoneGrouping: CreateUndoneBandedmGroupHeaderGrouping(),
            expectedUndoneUntouchedSections: new[]
            {
                CreateUndoneBandedmGroupFooterSection()
            },
            expectedOriginalLayoutTextValue: "OneToMany",
            expectedUpdatedLayoutTextValue: "customer.company",
            expectedOriginalContainedObjects: CreateBandedmGroupHeaderContainedObjects(0),
            expectedUpdatedContainedObjects: CreateBandedmGroupHeaderContainedObjects(0));
    }
}
