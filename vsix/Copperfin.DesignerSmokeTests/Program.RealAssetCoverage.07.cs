
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
    private static void SmokeResolvedRealAssetCoverageClusterPart07()
    {
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 3,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Detail"),
                new KeyValuePair<string, string>("RECORDINDEX", "3"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "1"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "10000")
            },
            expectedMissingProperties: new[]
            {
                "EXPR",
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO",
                "GROUPINGEXPRESSION",
                "GROUPINGEXPRESSIONFIELD",
                "GROUPINGEXPRESSIONMEMO",
                "GROUPPARTNERSTATE"
            },
            expectedObjectListCount: 1);
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 4,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Column Footer"),
                new KeyValuePair<string, string>("RECORDINDEX", "4"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "0")
            },
            expectedMissingProperties: new[]
            {
                "EXPR",
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO",
                "GROUPINGEXPRESSION",
                "GROUPINGEXPRESSIONFIELD",
                "GROUPINGEXPRESSIONMEMO",
                "GROUPPARTNERSTATE"
            });
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 5,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Page Footer"),
                new KeyValuePair<string, string>("RECORDINDEX", "5"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "0")
            },
            expectedMissingProperties: new[]
            {
                "EXPR",
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO",
                "GROUPINGEXPRESSION",
                "GROUPINGEXPRESSIONFIELD",
                "GROUPINGEXPRESSIONMEMO",
                "GROUPPARTNERSTATE"
            });
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 3,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("EXPR", "titles_by_author.author_id"),
                new KeyValuePair<string, string>("EXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("EXPRESSIONMEMO", "18"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "titles_by_author.author_id"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "18"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            });
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 5,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("EXPR", string.Empty),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "titles_by_author.author_id"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "18"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedMissingProperties: new[]
            {
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO"
            });
        SmokeAssetEditorBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
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
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
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
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
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
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
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
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedSectionTitle: "Title",
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
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorBatchPropertyRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedSectionTitle: "Detail",
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
            expectedObjectTitle: "wiz_field",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 6,
            expectedSectionTitle: "Unplaced objects",
            propertyName: "FONTFACE",
            updatedPropertyValue: "Arial",
            expectedOriginalSelectionValue: string.Empty,
            expectedUpdatedSelectionValue: "Arial",
            expectedUpdatedRawValue: "Arial",
            expectedOriginalRawValue: string.Empty,
            expectedObjectTitle: "wiz_general",
            expectedSectionCount: 4,
            expectLabel: false,
            expectUnplacedObject: true);
        SmokeAssetEditorPlacementRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 6,
            initialSectionTitle: "Unplaced objects",
            updatedSectionTitle: "Page Header",
            updatedSectionRecordIndex: 2,
            propertyName: "VPOS",
            updatedPropertyValue: 2400,
            expectedOriginalSelectionValue: null,
            expectedUpdatedSelectionValue: "2400",
            expectedUpdatedRawValue: "2400",
            expectedOriginalRawValue: "11145.833",
            expectedObjectTitle: "wiz_general",
            expectedSectionCount: 4,
            expectLabel: false,
            expectedOriginalUnplacedObjectCount: 7,
            expectedUpdatedUnplacedObjectCount: 6);
        SmokeAssetEditorDuplicateCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            duplicateUniqueId: "DUPREAL01",
            expectedSourceObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedOriginalSectionObjectCount: 1,
            expectedUpdatedSectionObjectCount: 2);
        SmokeAssetEditorDuplicateCommandWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            duplicateUniqueId: "LDUPREAL1",
            expectedSourceObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedOriginalSectionObjectCount: 1,
            expectedUpdatedSectionObjectCount: 2);
        SmokeConfiguredVfp9ZipAssetDiscovery();
        SmokeFreshRunVfpSourceStartupPaths();
        // These workflow assertions depend on the addlabel fixture's shared
        // class dependency and deterministic debugger startup contract. Keep
        // installed solution.pjx coverage in the dedicated VFP9 asset stages;
        // do not feed its different project shape into these expectations.
        var projectWorkflowPath = TryResolveVfpSourceAsset("VFPSource/addlabel/addlabel.pjx");
        SmokeProjectEditorWithRealAsset(
            projectWorkflowPath,
            expectGroups: new[] { "Forms", "Programs", "Class Libraries", "Classes", "Other Assets" });
        SmokeProjectBuildWorkflowWithRealAsset(
            projectWorkflowPath);
        SmokeProjectRunWorkflowWithRealAsset(
            projectWorkflowPath);
        SmokeProjectDebugReplayWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/addlabel/addlabel.pjx"));
        SmokeProgramEditorWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/tasklist/main.prg"));
        SmokeProjectDebuggerWithRealAsset(
            projectWorkflowPath);
        SmokeStandaloneStudioWithMultipleAssets(
            ResolveFirstExistingRealAssetPath(
                TryResolveVfp9InstallAsset(@"Wizards\Template\Books\Forms\books.scx"),
                TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Forms/books.scx")),
            ResolveFirstExistingRealAssetPath(
                TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
                TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX")));
        SmokeStandaloneStudioWithMultipleAssets(
            TryResolveVfpSourceAsset("VFPSource/EnvMgr/envmgr.vcx"),
            TryResolveVfpSourceAsset("VFPSource/ReportBuilder/handler_context.mnx"));
    }
}
