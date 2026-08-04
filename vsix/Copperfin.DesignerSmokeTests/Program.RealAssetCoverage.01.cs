
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
    private static void SmokeResolvedRealAssetCoverageClusterPart01()
    {
        SmokeAssetEditorWithRealAsset(
            ResolveFirstExistingRealAssetPath(
                TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
                TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX")),
            expectSection: "Detail");
        SmokeAssetEditorWithRealAsset(
            ResolveFirstExistingRealAssetPath(
                TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
                TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX")),
            expectSection: "Detail");
        SmokeRealAssetGroupingExplorerSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"));
        SmokeRealAssetSettingsSortMetadataSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"));
        SmokeRealAssetSettingsDocumentTitleSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"));
        SmokeRealAssetSettingsPageSetupSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"));
        SmokeRealAssetSettingsPaperDimensionsSelection(
            TryResolveVfpSourceAsset("VFPSource/Sedna/DataExplorer/DATAEXPLORERQUICKREPORT.FRX"));
        SmokeRealAssetSettingsPrinterIdentitySelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE1H.FRX"));
        SmokeRealAssetSettingsPrintProfileSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLE1H.FRX"));
        SmokeRealAssetSettingsColorSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"));
        SmokeRealAssetSettingsAuxiliaryPrintSelection(
            TryResolveVfpSourceAsset("VFPSource/foxref/foxrefresultsa4.frx"));
        SmokeRealAssetLabelSettingsSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"));
        SmokeRealAssetLabelSettingsDocumentTitleSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"));
        SmokeRealAssetLabelSettingsPreviewBoundsSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"));
        SmokeRealAssetSettingsPreviewBoundsSelection(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"));
        SmokeRealAssetDeletedPreviewBoundsSelectionAfterDelete(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            recordIndex: 7,
            expectedUniqueId: "_RC60MC40R",
            expectLabel: false);
        SmokeRealAssetDeletedPreviewBoundsSelectionAfterDelete(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            recordIndex: 6,
            expectedUniqueId: "_QV30QY1DL",
            expectLabel: true);
        SmokeRealAssetColumnSetupSelectionAfterExprUpdate(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            exprPayload: "COLS=3\nCOLWIDTH=3600\nCOLSPACING=120",
            expectLabel: false);
        SmokeRealAssetColumnSetupSelectionAfterExprUpdate(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            exprPayload: "COLS=2\nCOLWIDTH=2400\nCOLSPACING=180",
            expectLabel: true);
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "ORIENTATION",
            originalValue: "1",
            updatedValue: "0",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 142
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 143
            },
            expectRawSnapshotProperty: false,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 96457,
                Bottom = 33436,
                Width = 96457,
                Height = 33436
            });
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "ORIENTATION",
            originalValue: "0",
            updatedValue: "1",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 17
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 18
            },
            expectRawSnapshotProperty: false,
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 21354,
                Bottom = 10000,
                Width = 21354,
                Height = 10000
            });
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "PAPERSIZE",
            originalValue: "1",
            updatedValue: "5",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERSIZE",
                Value = "5",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 1,
                MemoBlockNumber = 142
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERSIZE",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 1,
                MemoBlockNumber = 143
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "PAPERSIZE",
            originalValue: "1",
            updatedValue: "9",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERSIZE",
                Value = "9",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 1,
                MemoBlockNumber = 17
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERSIZE",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 1,
                MemoBlockNumber = 18
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "COLOR",
            originalValue: "1",
            updatedValue: "0",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLOR",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 2,
                MemoBlockNumber = 142
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "COLOR",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 2,
                MemoBlockNumber = 143
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "COLOR",
            originalValue: "1",
            updatedValue: "0",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLOR",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 2,
                MemoBlockNumber = 17
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "COLOR",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 2,
                MemoBlockNumber = 18
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "COLS",
            updatedValue: "3",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLS",
                Value = "3",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "COLS",
            updatedValue: "2",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLS",
                Value = "2",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "COLWIDTH",
            updatedValue: "3600",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLWIDTH",
                Value = "3600",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "COLWIDTH",
            updatedValue: "2400",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLWIDTH",
                Value = "2400",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "COLSPACING",
            updatedValue: "120",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLSPACING",
                Value = "120",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "COLSPACING",
            updatedValue: "180",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLSPACING",
                Value = "180",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "DRIVER",
            updatedValue: "cups",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DRIVER",
                Value = "cups",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "DRIVER",
            updatedValue: "cupslbl",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DRIVER",
                Value = "cupslbl",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "DEFAULTSOURCE",
            updatedValue: "15",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DEFAULTSOURCE",
                Value = "15",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "DEFAULTSOURCE",
            updatedValue: "16",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DEFAULTSOURCE",
                Value = "16",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "PRINTQUALITY",
            updatedValue: "300",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PRINTQUALITY",
                Value = "300",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "PRINTQUALITY",
            updatedValue: "1200",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PRINTQUALITY",
                Value = "1200",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "YRESOLUTION",
            updatedValue: "600",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "YRESOLUTION",
                Value = "600",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "YRESOLUTION",
            updatedValue: "1200",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "YRESOLUTION",
                Value = "1200",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "TTOPTION",
            updatedValue: "2",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TTOPTION",
                Value = "2",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "TTOPTION",
            updatedValue: "3",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TTOPTION",
                Value = "3",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "OUTPUT",
            updatedValue: "report.out",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "OUTPUT",
                Value = "report.out",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "OUTPUT",
            updatedValue: "label.out",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "OUTPUT",
                Value = "label.out",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "DEVICE",
            updatedValue: "report-device",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DEVICE",
                Value = "report-device",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 142
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "DEVICE",
            updatedValue: "label-device",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DEVICE",
                Value = "label-device",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 17
            },
            expectRawSnapshotProperty: false);
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "TAG",
            updatedValue: "customer.region",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TAG",
                Value = "customer.region",
                RecordIndex = 0,
                FieldIndex = 19,
                MemoBlockNumber = 142
            });
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "TAG",
            updatedValue: "label.region",
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
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "TAG",
            updatedValue: "customer.country",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TAG",
                Value = "customer.country",
                RecordIndex = 0,
                FieldIndex = 19,
                MemoBlockNumber = 304
            });
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "TAG",
            updatedValue: "customer.country",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TAG",
                Value = "customer.country",
                RecordIndex = 0,
                FieldIndex = 19,
                MemoBlockNumber = 78
            });
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "TOPMARGIN",
            updatedValue: "7",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TOPMARGIN",
                Value = "7",
                RecordIndex = 0,
                FieldIndex = 62,
                MemoBlockNumber = 0
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
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "BOTMARGIN",
            updatedValue: "9",
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
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "TOPMARGIN",
            updatedValue: "7",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "TOPMARGIN",
                Value = "7",
                RecordIndex = 0,
                FieldIndex = 62,
                MemoBlockNumber = 0
            },
            expectedUpdatedPreviewBounds: new ExpectedPreviewBoundsGeometry
            {
                Left = 0,
                Top = 0,
                Right = 21354,
                Bottom = 10000,
                Width = 21354,
                Height = 10000
            });
        SmokeRealAssetHostBackedMissingSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "BOTMARGIN",
            updatedValue: "9",
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
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "GRIDV",
            originalValue: "12",
            updatedValue: "14",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "GRIDV",
            originalValue: "12",
            updatedValue: "16",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "GRIDH",
            originalValue: "12",
            updatedValue: "14",
            expectedSectionCount: 6,
            expectLabel: false);
        SmokeRealAssetHostBackedSettingsRoundTrip(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "GRIDH",
            originalValue: "12",
            updatedValue: "16",
            expectedSectionCount: 5,
            expectLabel: true);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "ORIENTATION",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 0,
            expectedUpdatedSelectionValue: "0",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "0",
            expectedSectionCount: 6,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 142
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 143
            },
            expectRawSnapshotProperty: false);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "ORIENTATION",
            expectedOriginalSelectionValue: "0",
            updatedPropertyValue: 1,
            expectedUpdatedSelectionValue: "1",
            expectedOriginalRawValue: "0",
            expectedUpdatedRawValue: "1",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 17
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 18
            },
            expectRawSnapshotProperty: false);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "ORIENTATION",
            expectedOriginalSelectionValue: "0",
            updatedPropertyValue: 1,
            expectedUpdatedSelectionValue: "1",
            expectedOriginalRawValue: "0",
            expectedUpdatedRawValue: "1",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 304
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 305
            },
            expectRawSnapshotProperty: false);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "ORIENTATION",
            expectedOriginalSelectionValue: "0",
            updatedPropertyValue: 1,
            expectedUpdatedSelectionValue: "1",
            expectedOriginalRawValue: "0",
            expectedUpdatedRawValue: "1",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 78
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "ORIENTATION",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 0,
                MemoBlockNumber = 79
            },
            expectRawSnapshotProperty: false);
    }
}
