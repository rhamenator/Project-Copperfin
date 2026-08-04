
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
    private static void SmokeResolvedRealAssetCoverageClusterPart06()
    {
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 1,
            expectedSectionTitle: "Title",
            propertyName: "HEIGHT",
            updatedPropertyValue: 12000,
            expectedOriginalSelectionValue: "11459",
            expectedUpdatedSelectionValue: "12000",
            expectedOriginalRawValue: "11459.000",
            expectedUpdatedRawValue: "12000",
            expectedOriginalLayoutValue: 11459,
            expectedUpdatedLayoutValue: 12000,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 1);
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 3,
            expectedSectionTitle: "Detail",
            propertyName: "HEIGHT",
            updatedPropertyValue: 9600,
            expectedOriginalSelectionValue: "10000",
            expectedUpdatedSelectionValue: "9600",
            expectedOriginalRawValue: "10000.000",
            expectedUpdatedRawValue: "9600",
            expectedOriginalLayoutValue: 10000,
            expectedUpdatedLayoutValue: 9600,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedObjectCount: 1,
            expectedUntouchedSections: new[]
            {
                CreateStylelblUntouchedColumnFooterSection()
            });
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 2,
            expectedSectionTitle: "Column Header",
            propertyName: "HEIGHT",
            updatedPropertyValue: 500,
            expectedOriginalSelectionValue: "0",
            expectedUpdatedSelectionValue: "500",
            expectedOriginalRawValue: "0.000",
            expectedUpdatedRawValue: "500",
            expectedOriginalLayoutValue: 0,
            expectedUpdatedLayoutValue: 500,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedObjectCount: 0);
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 4,
            expectedSectionTitle: "Column Footer",
            propertyName: "HEIGHT",
            updatedPropertyValue: 500,
            expectedOriginalSelectionValue: "0",
            expectedUpdatedSelectionValue: "500",
            expectedOriginalRawValue: "0.000",
            expectedUpdatedRawValue: "500",
            expectedOriginalLayoutValue: 0,
            expectedUpdatedLayoutValue: 500,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedObjectCount: 0);
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 5,
            expectedSectionTitle: "Page Footer",
            propertyName: "HEIGHT",
            updatedPropertyValue: 500,
            expectedOriginalSelectionValue: "0",
            expectedUpdatedSelectionValue: "500",
            expectedOriginalRawValue: "0.000",
            expectedUpdatedRawValue: "500",
            expectedOriginalLayoutValue: 0,
            expectedUpdatedLayoutValue: 500,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedObjectCount: 0);
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 2,
            expectedSectionTitle: "Page Header",
            propertyName: "VPOS",
            updatedPropertyValue: 500,
            expectedOriginalSelectionValue: "0",
            expectedUpdatedSelectionValue: "500",
            expectedOriginalRawValue: string.Empty,
            expectedUpdatedRawValue: "500",
            expectedOriginalLayoutValue: 0,
            expectedUpdatedLayoutValue: 500,
            expectedSectionCount: 4,
            expectLabel: false,
            expectedObjectCount: 3,
            expectedOriginalContainedObjects: CreateStyle3vPageHeaderContainedObjects(0),
            expectedUpdatedContainedObjects: CreateStyle3vPageHeaderContainedObjects(500));
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 2,
            expectedSectionTitle: "Page Header",
            propertyName: "HEIGHT",
            updatedPropertyValue: 7000,
            expectedOriginalSelectionValue: "6355",
            expectedUpdatedSelectionValue: "7000",
            expectedOriginalRawValue: "6355.000",
            expectedUpdatedRawValue: "7000",
            expectedOriginalLayoutValue: 6355,
            expectedUpdatedLayoutValue: 7000,
            expectedSectionCount: 4,
            expectLabel: false,
            expectedObjectCount: 3,
            expectedOriginalContainedObjects: CreateStyle3vPageHeaderContainedObjects(0),
            expectedUpdatedContainedObjects: CreateStyle3vPageHeaderContainedObjects(0));
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 4,
            expectedSectionTitle: "Page Footer",
            propertyName: "HEIGHT",
            updatedPropertyValue: 3200,
            expectedOriginalSelectionValue: "2917",
            expectedUpdatedSelectionValue: "3200",
            expectedOriginalRawValue: "2917.000",
            expectedUpdatedRawValue: "3200",
            expectedOriginalLayoutValue: 2917,
            expectedUpdatedLayoutValue: 3200,
            expectedSectionCount: 4,
            expectLabel: false,
            expectedObjectCount: 0);
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Footer",
            propertyName: "EXPR",
            updatedPropertyValue: "titles_by_author.last_name",
            expectedOriginalSelectionValue: string.Empty,
            expectedUpdatedSelectionValue: "titles_by_author.last_name",
            expectedOriginalRawValue: string.Empty,
            expectedUpdatedRawValue: "titles_by_author.last_name",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 0,
            expectedExplorerSectionTitle: "Group Footer - titles_by_author.author_id",
            expectedGrouping: new ExpectedSectionGroupingMetadata
            {
                GroupRole = "footer",
                GroupRoleDisplay = "Footer",
                SectionExpression = string.Empty,
                GroupingIndex = 0,
                GroupingNestingDepth = 0,
                GroupingExpression = "titles_by_author.author_id",
                GroupingExpressionFieldIndex = 6,
                GroupingExpressionMemoBlockNumber = 18,
                GroupPartnerSectionId = "_RC60MBV9L",
                GroupPartnerRecordIndex = 3,
                GroupPartnerDeleted = false,
                GroupPartnerStateDisplay = "Live"
            },
            expectedUpdatedGrouping: new ExpectedSectionGroupingMetadata
            {
                GroupRole = "footer",
                GroupRoleDisplay = "Footer",
                SectionExpression = "titles_by_author.last_name",
                GroupingIndex = 0,
                GroupingNestingDepth = 0,
                GroupingExpression = "titles_by_author.author_id",
                GroupingExpressionFieldIndex = 6,
                GroupingExpressionMemoBlockNumber = 18,
                GroupPartnerSectionId = "_RC60MBV9L",
                GroupPartnerRecordIndex = 3,
                GroupPartnerDeleted = false,
                GroupPartnerStateDisplay = "Live"
            },
            expectedUntouchedSections: new[]
            {
                CreateByAuthorUntouchedGroupHeaderSection()
            },
            expectedOriginalLayoutTextValue: string.Empty,
            expectedUpdatedLayoutTextValue: "titles_by_author.last_name");
        SmokeAssetEditorSectionRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Footer",
            propertyName: "HEIGHT",
            updatedPropertyValue: 900,
            expectedOriginalSelectionValue: "730",
            expectedUpdatedSelectionValue: "900",
            expectedOriginalRawValue: "730.000",
            expectedUpdatedRawValue: "900",
            expectedOriginalLayoutValue: 730,
            expectedUpdatedLayoutValue: 900,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 0,
            expectedExplorerSectionTitle: "Group Footer - titles_by_author.author_id",
            expectedGrouping: new ExpectedSectionGroupingMetadata
            {
                GroupRole = "footer",
                GroupRoleDisplay = "Footer",
                SectionExpression = string.Empty,
                GroupingIndex = 0,
                GroupingNestingDepth = 0,
                GroupingExpression = "titles_by_author.author_id",
                GroupingExpressionFieldIndex = 6,
                GroupingExpressionMemoBlockNumber = 18,
                GroupPartnerSectionId = "_RC60MBV9L",
                GroupPartnerRecordIndex = 3,
                GroupPartnerDeleted = false,
                GroupPartnerStateDisplay = "Live"
            });
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 1,
            expectedSectionListTitle: "Title",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Title"),
                new KeyValuePair<string, string>("RECORDINDEX", "1"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "1"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "11459")
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
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 2,
            expectedSectionListTitle: "Page Header",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Page Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "2"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "3"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "6355")
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
            expectedObjectListCount: 3);
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 4,
            expectedSectionListTitle: "Page Footer",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Page Footer"),
                new KeyValuePair<string, string>("RECORDINDEX", "4"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "2917")
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
            expectedObjectListCount: 0);
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Sedna/DataExplorer/DATAEXPLORERQUICKREPORT.FRX"),
            recordIndex: 2,
            expectedSectionListTitle: "Detail",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Detail"),
                new KeyValuePair<string, string>("RECORDINDEX", "2"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "12"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "19479")
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
            expectedObjectListCount: 12);
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            recordIndex: 5,
            expectedSectionListTitle: "Group Header - OneToMany",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Group Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "5"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "8"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "7605"),
                new KeyValuePair<string, string>("GROUPROLE", "Header"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "0"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "0"),
                new KeyValuePair<string, string>("EXPR", "OneToMany"),
                new KeyValuePair<string, string>("EXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("EXPRESSIONMEMO", "25"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "OneToMany"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "25"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_RME0ORXFY"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "7"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedObjectListCount: 8);
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/foxref/foxrefresultsa4.frx"),
            recordIndex: 3,
            expectedSectionListTitle: "Group Header - RefID",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Group Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "3"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "2084"),
                new KeyValuePair<string, string>("GROUPROLE", "Header"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "1"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "1"),
                new KeyValuePair<string, string>("EXPR", "RefID"),
                new KeyValuePair<string, string>("EXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("EXPRESSIONMEMO", "23"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "RefID"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "23"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_0RV0UV0IG"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "5"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            });
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/foxref/foxrefresultsa4.frx"),
            recordIndex: 5,
            expectedSectionListTitle: "Group Footer - RefID",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Group Footer"),
                new KeyValuePair<string, string>("RECORDINDEX", "5"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "0"),
                new KeyValuePair<string, string>("GROUPROLE", "Footer"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "1"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "1"),
                new KeyValuePair<string, string>("EXPR", string.Empty),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "RefID"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "23"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_0RV0UV0HL"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "3"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedMissingProperties: new[]
            {
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO"
            });
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/foxref/foxrefresultsa4.frx"),
            recordIndex: 6,
            expectedSectionListTitle: "Group Footer - SetID",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Group Footer"),
                new KeyValuePair<string, string>("RECORDINDEX", "6"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "0"),
                new KeyValuePair<string, string>("GROUPROLE", "Footer"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "0"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "0"),
                new KeyValuePair<string, string>("EXPR", string.Empty),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "SetID"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "22"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_0LF0MKNEM"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "2"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedMissingProperties: new[]
            {
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO"
            });
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 4,
            expectedSectionListTitle: "Detail",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Detail"),
                new KeyValuePair<string, string>("RECORDINDEX", "4"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "1875")
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
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 3,
            expectedSectionListTitle: "Group Header - titles_by_author.author_id",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("EXPR", "titles_by_author.author_id"),
                new KeyValuePair<string, string>("EXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("EXPRESSIONMEMO", "18"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "titles_by_author.author_id"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "18")
            });
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 5,
            expectedSectionListTitle: "Group Footer - titles_by_author.author_id",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("EXPR", string.Empty),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "titles_by_author.author_id"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "18")
            },
            expectedMissingProperties: new[]
            {
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO"
            });
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 2,
            expectedSectionListTitle: "Column Header",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
                new KeyValuePair<string, string>("BANDKIND", "Column Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "2"),
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
            },
            expectedObjectListCount: 0);
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 3,
            expectedSectionListTitle: "Detail",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
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
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 4,
            expectedSectionListTitle: "Column Footer",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
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
            },
            expectedObjectListCount: 0);
        SmokeAssetEditorSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 5,
            expectedSectionListTitle: "Page Footer",
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Live"),
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
            },
            expectedObjectListCount: 0);
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE1H.FRX"),
            recordIndex: 1,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Title"),
                new KeyValuePair<string, string>("RECORDINDEX", "1"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "6459")
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
            TryResolveVfpSourceAsset("VFPSource/Sedna/DataExplorer/DATAEXPLORERQUICKREPORT.FRX"),
            recordIndex: 2,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Detail"),
                new KeyValuePair<string, string>("RECORDINDEX", "2"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "12"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "19479")
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
            expectedObjectListCount: 12);
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            recordIndex: 5,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Group Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "5"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "7605"),
                new KeyValuePair<string, string>("GROUPROLE", "Header"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "0"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "0"),
                new KeyValuePair<string, string>("EXPR", "OneToMany"),
                new KeyValuePair<string, string>("EXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("EXPRESSIONMEMO", "25"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "OneToMany"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "25"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_RME0ORXFY"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "7"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedObjectListCount: 0);
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/foxref/foxrefresultsa4.frx"),
            recordIndex: 2,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Group Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "2"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "2"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "5105"),
                new KeyValuePair<string, string>("GROUPROLE", "Header"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "0"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "0"),
                new KeyValuePair<string, string>("EXPR", "SetID"),
                new KeyValuePair<string, string>("EXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("EXPRESSIONMEMO", "22"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "SetID"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "22"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_0LF0MKNEW"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "6"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedObjectListCount: 2);
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/foxref/foxrefresultsa4.frx"),
            recordIndex: 3,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Group Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "3"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "2084"),
                new KeyValuePair<string, string>("GROUPROLE", "Header"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "1"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "1"),
                new KeyValuePair<string, string>("EXPR", "RefID"),
                new KeyValuePair<string, string>("EXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("EXPRESSIONMEMO", "23"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "RefID"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "23"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_0RV0UV0IG"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "5"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedObjectListCount: 0);
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/foxref/foxrefresultsa4.frx"),
            recordIndex: 5,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Group Footer"),
                new KeyValuePair<string, string>("RECORDINDEX", "5"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "0"),
                new KeyValuePair<string, string>("GROUPROLE", "Footer"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "1"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "1"),
                new KeyValuePair<string, string>("EXPR", string.Empty),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "RefID"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "23"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_0RV0UV0HL"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "3"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedMissingProperties: new[]
            {
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO"
            },
            expectedObjectListCount: 0);
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/foxref/foxrefresultsa4.frx"),
            recordIndex: 6,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Group Footer"),
                new KeyValuePair<string, string>("RECORDINDEX", "6"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "0"),
                new KeyValuePair<string, string>("GROUPROLE", "Footer"),
                new KeyValuePair<string, string>("GROUPINGINDEX", "0"),
                new KeyValuePair<string, string>("GROUPINGNESTINGDEPTH", "0"),
                new KeyValuePair<string, string>("EXPR", string.Empty),
                new KeyValuePair<string, string>("GROUPINGEXPRESSION", "SetID"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONFIELD", "6"),
                new KeyValuePair<string, string>("GROUPINGEXPRESSIONMEMO", "22"),
                new KeyValuePair<string, string>("GROUPPARTNERSECTIONID", "_0LF0MKNEM"),
                new KeyValuePair<string, string>("GROUPPARTNERRECORD", "2"),
                new KeyValuePair<string, string>("GROUPPARTNERSTATE", "Live")
            },
            expectedMissingProperties: new[]
            {
                "EXPRESSIONFIELD",
                "EXPRESSIONMEMO"
            },
            expectedObjectListCount: 0);
        SmokeAssetEditorDeletedSectionMetadataSelectionWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 2,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Page Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "2"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "6355")
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
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX"),
            recordIndex: 4,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Page Footer"),
                new KeyValuePair<string, string>("RECORDINDEX", "4"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "2917")
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
            recordIndex: 1,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Title"),
                new KeyValuePair<string, string>("RECORDINDEX", "1"),
                new KeyValuePair<string, string>("OBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("DELETEDOBJECTCOUNT", "0"),
                new KeyValuePair<string, string>("TOP", "0"),
                new KeyValuePair<string, string>("HEIGHT", "11459")
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
            recordIndex: 2,
            expectedProperties: new[]
            {
                new KeyValuePair<string, string>("SECTIONSTATE", "Deleted"),
                new KeyValuePair<string, string>("BANDKIND", "Column Header"),
                new KeyValuePair<string, string>("RECORDINDEX", "2"),
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
    }
}
