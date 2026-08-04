
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
    private static void SmokeResolvedRealAssetCoverageClusterPart03()
    {
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "TAG",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "label.region",
            expectedUpdatedSelectionValue: "label.region",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TAG",
                Value = "label.region",
                RecordIndex = 0,
                FieldIndex = 19,
                MemoBlockNumber = 17
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "TAG",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "customer.country",
            expectedUpdatedSelectionValue: "customer.country",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TAG",
                Value = "customer.country",
                RecordIndex = 0,
                FieldIndex = 19,
                MemoBlockNumber = 304
            },
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "TAG",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "customer.country",
            expectedUpdatedSelectionValue: "customer.country",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TAG",
                Value = "customer.country",
                RecordIndex = 0,
                FieldIndex = 19,
                MemoBlockNumber = 78
            },
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "TOPMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 7,
            expectedUpdatedSelectionValue: "7",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TOPMARGIN",
                Value = "7",
                RecordIndex = 0,
                FieldIndex = 62,
                MemoBlockNumber = 0
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "TOPMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 10,
            expectedUpdatedSelectionValue: "10",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TOPMARGIN",
                Value = "10",
                RecordIndex = 0,
                FieldIndex = 62,
                MemoBlockNumber = 0
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "BOTMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 9,
            expectedUpdatedSelectionValue: "9",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "BOTMARGIN",
                Value = "9",
                RecordIndex = 0,
                FieldIndex = 63,
                MemoBlockNumber = 0
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "BOTMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 20,
            expectedUpdatedSelectionValue: "20",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "BOTMARGIN",
                Value = "20",
                RecordIndex = 0,
                FieldIndex = 63,
                MemoBlockNumber = 0
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "TOPMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 7,
            expectedUpdatedSelectionValue: "7",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TOPMARGIN",
                Value = "7",
                RecordIndex = 0,
                FieldIndex = 62,
                MemoBlockNumber = 0
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "TOPMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 10,
            expectedUpdatedSelectionValue: "10",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TOPMARGIN",
                Value = "10",
                RecordIndex = 0,
                FieldIndex = 62,
                MemoBlockNumber = 0
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "BOTMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 9,
            expectedUpdatedSelectionValue: "9",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "BOTMARGIN",
                Value = "9",
                RecordIndex = 0,
                FieldIndex = 63,
                MemoBlockNumber = 0
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "BOTMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 20,
            expectedUpdatedSelectionValue: "20",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "BOTMARGIN",
                Value = "20",
                RecordIndex = 0,
                FieldIndex = 63,
                MemoBlockNumber = 0
            });
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "PAPERLENGTH",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 4318,
            expectedUpdatedSelectionValue: "4318",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERLENGTH",
                Value = "4318",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "PAPERWIDTH",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 2794,
            expectedUpdatedSelectionValue: "2794",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERWIDTH",
                Value = "2794",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "LEFTMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 15,
            expectedUpdatedSelectionValue: "15",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "LEFTMARGIN",
                Value = "15",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "RIGHTMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 25,
            expectedUpdatedSelectionValue: "25",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "RIGHTMARGIN",
                Value = "25",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "PAPERLENGTH",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 4318,
            expectedUpdatedSelectionValue: "4318",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERLENGTH",
                Value = "4318",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "PAPERWIDTH",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 2794,
            expectedUpdatedSelectionValue: "2794",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERWIDTH",
                Value = "2794",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "LEFTMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 15,
            expectedUpdatedSelectionValue: "15",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "LEFTMARGIN",
                Value = "15",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "RIGHTMARGIN",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 25,
            expectedUpdatedSelectionValue: "25",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "RIGHTMARGIN",
                Value = "25",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "GRIDV",
            expectedOriginalSelectionValue: "12",
            updatedPropertyValue: 14,
            expectedUpdatedSelectionValue: "14",
            expectedOriginalRawValue: "12",
            expectedUpdatedRawValue: "14",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "GRIDV",
            expectedOriginalSelectionValue: "12",
            updatedPropertyValue: 16,
            expectedUpdatedSelectionValue: "16",
            expectedOriginalRawValue: "12",
            expectedUpdatedRawValue: "16",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "GRIDV",
            expectedOriginalSelectionValue: "12",
            updatedPropertyValue: 14,
            expectedUpdatedSelectionValue: "14",
            expectedOriginalRawValue: "12",
            expectedUpdatedRawValue: "14",
            expectedSectionCount: 5,
            expectLabel: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "GRIDV",
            expectedOriginalSelectionValue: "12",
            updatedPropertyValue: 16,
            expectedUpdatedSelectionValue: "16",
            expectedOriginalRawValue: "12",
            expectedUpdatedRawValue: "16",
            expectedSectionCount: 5,
            expectLabel: true,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "GRIDH",
            expectedOriginalSelectionValue: "12",
            updatedPropertyValue: 14,
            expectedUpdatedSelectionValue: "14",
            expectedOriginalRawValue: "12",
            expectedUpdatedRawValue: "14",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "GRIDH",
            expectedOriginalSelectionValue: "12",
            updatedPropertyValue: 16,
            expectedUpdatedSelectionValue: "16",
            expectedOriginalRawValue: "12",
            expectedUpdatedRawValue: "16",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "GRIDH",
            expectedOriginalSelectionValue: "12",
            updatedPropertyValue: 14,
            expectedUpdatedSelectionValue: "14",
            expectedOriginalRawValue: "12",
            expectedUpdatedRawValue: "14",
            expectedSectionCount: 5,
            expectLabel: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "GRIDH",
            expectedOriginalSelectionValue: "12",
            updatedPropertyValue: 16,
            expectedUpdatedSelectionValue: "16",
            expectedOriginalRawValue: "12",
            expectedUpdatedRawValue: "16",
            expectedSectionCount: 5,
            expectLabel: true,
            verifyExplicitClearAfterUpdate: true);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyName: "HPOS",
            originalValue: "8645.833",
            updatedValue: "9000",
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
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyName: "HPOS",
            originalValue: "6250.000",
            updatedValue: "6500",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 21604,
                Bottom = 10000,
                Width = 21604,
                Height = 10000
            });
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyName: "VPOS",
            originalValue: "6666.667",
            updatedValue: "7000",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyName: "VPOS",
            originalValue: "6666.667",
            updatedValue: "6800",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyName: "WIDTH",
            originalValue: "19687.500",
            updatedValue: "20000",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyName: "WIDTH",
            originalValue: "15104.167",
            updatedValue: "16000",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyName: "HEIGHT",
            originalValue: "3333.333",
            updatedValue: "3500",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyName: "HEIGHT",
            originalValue: "1666.667",
            updatedValue: "1800",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyName: "FONTFACE",
            originalValue: "Times New Roman",
            updatedValue: "Arial",
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
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyName: "FONTFACE",
            originalValue: "Arial",
            updatedValue: "Calibri",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyName: "FONTSTYLE",
            originalValue: "3",
            updatedValue: "1",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyName: "FONTSTYLE",
            originalValue: "0",
            updatedValue: "1",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            propertyName: "FONTSIZE",
            originalValue: "20",
            updatedValue: "18",
            expectedObjectTitle: "\"Titles By Author\"",
            expectedSectionTitle: "Title",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedPropertyRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            propertyName: "FONTSIZE",
            originalValue: "8",
            updatedValue: "9",
            expectedObjectTitle: "wiz_field",
            expectedSectionTitle: "Detail",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 1,
            expectedSectionTitle: "Title",
            propertyName: "VPOS",
            originalRawValue: string.Empty,
            updatedRawValue: "500",
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
            },
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 96457,
                Bottom = 33436,
                Width = 96457,
                Height = 33436
            });
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 3,
            expectedSectionTitle: "Detail",
            propertyName: "VPOS",
            originalRawValue: string.Empty,
            updatedRawValue: "400",
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
            },
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 21354,
                Bottom = 10400,
                Width = 21354,
                Height = 10400
            });
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Header",
            propertyName: "VPOS",
            originalRawValue: string.Empty,
            updatedRawValue: "500",
            expectedOriginalLayoutValue: 0,
            expectedUpdatedLayoutValue: 500,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 8,
            expectedGrouping: CreateBandedmGroupHeaderGrouping(),
            expectedUntouchedSections: new[]
            {
                CreateBandedmUntouchedGroupFooterSection()
            },
            expectedOriginalContainedObjects: CreateBandedmGroupHeaderContainedObjects(0),
            expectedUpdatedContainedObjects: CreateBandedmGroupHeaderContainedObjects(500));
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Header",
            propertyName: "HEIGHT",
            originalRawValue: "7605.000",
            updatedRawValue: "8000",
            expectedOriginalLayoutValue: 7605,
            expectedUpdatedLayoutValue: 8000,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 8,
            expectedGrouping: CreateBandedmGroupHeaderGrouping(),
            expectedUntouchedSections: new[]
            {
                CreateBandedmUntouchedGroupFooterSection()
            },
            expectedOriginalContainedObjects: CreateBandedmGroupHeaderContainedObjects(0),
            expectedUpdatedContainedObjects: CreateBandedmGroupHeaderContainedObjects(0));
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/bandedm.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Header",
            propertyName: "EXPR",
            originalRawValue: "OneToMany",
            updatedRawValue: "customer.company",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 8,
            expectedGrouping: CreateBandedmGroupHeaderGrouping(),
            expectedUntouchedSections: new[]
            {
                CreateBandedmUntouchedGroupFooterSection()
            },
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
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 1,
            expectedSectionTitle: "Title",
            propertyName: "HEIGHT",
            originalRawValue: "11459.000",
            updatedRawValue: "12000",
            expectedOriginalLayoutValue: 11459,
            expectedUpdatedLayoutValue: 12000,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 1,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 96457,
                Bottom = 33436,
                Width = 96457,
                Height = 33436
            });
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 3,
            expectedSectionTitle: "Detail",
            propertyName: "HEIGHT",
            originalRawValue: "10000.000",
            updatedRawValue: "9600",
            expectedOriginalLayoutValue: 10000,
            expectedUpdatedLayoutValue: 9600,
            expectedSectionCount: 5,
            expectLabel: true,
            expectedObjectCount: 1,
            expectedUntouchedSections: new[]
            {
                CreateStylelblUntouchedColumnFooterSection()
            });
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Footer",
            propertyName: "EXPR",
            originalRawValue: string.Empty,
            updatedRawValue: "titles_by_author.last_name",
            expectedOriginalLayoutValue: null,
            expectedUpdatedLayoutValue: null,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 0,
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
            expectedUntouchedSections: new[]
            {
                CreateByAuthorUntouchedGroupHeaderSection()
            },
            expectedOriginalLayoutTextValue: string.Empty,
            expectedUpdatedLayoutTextValue: "titles_by_author.last_name");
        SmokeRealAssetHostBackedSectionRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 5,
            expectedSectionTitle: "Group Footer",
            propertyName: "HEIGHT",
            originalRawValue: "730.000",
            updatedRawValue: "900",
            expectedOriginalLayoutValue: 730,
            expectedUpdatedLayoutValue: 900,
            expectedSectionCount: 6,
            expectLabel: false,
            expectedObjectCount: 0,
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
    }
}
