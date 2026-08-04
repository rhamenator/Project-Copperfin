
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
    private static void SmokeResolvedRealAssetCoverageClusterPart02()
    {
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "PAPERSIZE",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 5,
            expectedUpdatedSelectionValue: "5",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "5",
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
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "PAPERSIZE",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 9,
            expectedUpdatedSelectionValue: "9",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "9",
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
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "PAPERSIZE",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 5,
            expectedUpdatedSelectionValue: "5",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "5",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERSIZE",
                Value = "5",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 1,
                MemoBlockNumber = 304
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERSIZE",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 1,
                MemoBlockNumber = 305
            },
            expectRawSnapshotProperty: false);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "PAPERSIZE",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 9,
            expectedUpdatedSelectionValue: "9",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "9",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERSIZE",
                Value = "9",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 1,
                MemoBlockNumber = 78
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "PAPERSIZE",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 1,
                MemoBlockNumber = 79
            },
            expectRawSnapshotProperty: false);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "COLOR",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 0,
            expectedUpdatedSelectionValue: "0",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "0",
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
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "COLOR",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 0,
            expectedUpdatedSelectionValue: "0",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "0",
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
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "COLOR",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 0,
            expectedUpdatedSelectionValue: "0",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "0",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLOR",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 2,
                MemoBlockNumber = 304
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "COLOR",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 2,
                MemoBlockNumber = 305
            },
            expectRawSnapshotProperty: false);
        SmokeAssetEditorSettingsRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "COLOR",
            expectedOriginalSelectionValue: "1",
            updatedPropertyValue: 0,
            expectedUpdatedSelectionValue: "0",
            expectedOriginalRawValue: "1",
            expectedUpdatedRawValue: "0",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLOR",
                Value = "0",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 2,
                MemoBlockNumber = 78
            },
            expectedUndoneSetting: new CopperfinStudioNamedValue
            {
                Name = "COLOR",
                Value = "1",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 2,
                MemoBlockNumber = 79
            },
            expectRawSnapshotProperty: false);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "COLS",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 3,
            expectedUpdatedSelectionValue: "3",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "COLS",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 2,
            expectedUpdatedSelectionValue: "2",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "COLS",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 2,
            expectedUpdatedSelectionValue: "2",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLS",
                Value = "2",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "COLS",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 2,
            expectedUpdatedSelectionValue: "2",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLS",
                Value = "2",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "COLWIDTH",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 3600,
            expectedUpdatedSelectionValue: "3600",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "COLWIDTH",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 2400,
            expectedUpdatedSelectionValue: "2400",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "COLWIDTH",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 4800,
            expectedUpdatedSelectionValue: "4800",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLWIDTH",
                Value = "4800",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "COLWIDTH",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 2400,
            expectedUpdatedSelectionValue: "2400",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLWIDTH",
                Value = "2400",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "COLSPACING",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 120,
            expectedUpdatedSelectionValue: "120",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "COLSPACING",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 180,
            expectedUpdatedSelectionValue: "180",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "COLSPACING",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 120,
            expectedUpdatedSelectionValue: "120",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLSPACING",
                Value = "120",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "COLSPACING",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 180,
            expectedUpdatedSelectionValue: "180",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "COLSPACING",
                Value = "180",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "DRIVER",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "cups",
            expectedUpdatedSelectionValue: "cups",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "DRIVER",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "cupslbl",
            expectedUpdatedSelectionValue: "cupslbl",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "DRIVER",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "cups",
            expectedUpdatedSelectionValue: "cups",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DRIVER",
                Value = "cups",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "DRIVER",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "cupslbl",
            expectedUpdatedSelectionValue: "cupslbl",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DRIVER",
                Value = "cupslbl",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "DEFAULTSOURCE",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 15,
            expectedUpdatedSelectionValue: "15",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "DEFAULTSOURCE",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 16,
            expectedUpdatedSelectionValue: "16",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "PRINTQUALITY",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 300,
            expectedUpdatedSelectionValue: "300",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "PRINTQUALITY",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 1200,
            expectedUpdatedSelectionValue: "1200",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "YRESOLUTION",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 600,
            expectedUpdatedSelectionValue: "600",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "YRESOLUTION",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 1200,
            expectedUpdatedSelectionValue: "1200",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "TTOPTION",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 2,
            expectedUpdatedSelectionValue: "2",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "TTOPTION",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: 3,
            expectedUpdatedSelectionValue: "3",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "OUTPUT",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "report.out",
            expectedUpdatedSelectionValue: "report.out",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "OUTPUT",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "label.out",
            expectedUpdatedSelectionValue: "label.out",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "OUTPUT",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "report.out",
            expectedUpdatedSelectionValue: "report.out",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "OUTPUT",
                Value = "report.out",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "OUTPUT",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "label.out",
            expectedUpdatedSelectionValue: "label.out",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "OUTPUT",
                Value = "label.out",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "DEVICE",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "report-device",
            expectedUpdatedSelectionValue: "report-device",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
            propertyName: "DEVICE",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "label-device",
            expectedUpdatedSelectionValue: "label-device",
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
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx"),
            propertyName: "DEVICE",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "report-device",
            expectedUpdatedSelectionValue: "report-device",
            expectedSectionCount: 5,
            expectLabel: false,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DEVICE",
                Value = "report-device",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 304
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx"),
            propertyName: "DEVICE",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "label-device",
            expectedUpdatedSelectionValue: "label-device",
            expectedSectionCount: 5,
            expectLabel: true,
            expectedUpdatedSetting: new CopperfinStudioNamedValue
            {
                Name = "DEVICE",
                Value = "label-device",
                RecordIndex = 0,
                FieldIndex = 6,
                SourceLineIndex = 3,
                MemoBlockNumber = 78
            },
            expectRawSnapshotProperty: false,
            verifyExplicitClearAfterUpdate: true);
        SmokeAssetEditorMissingSettingsStringRoundTripWithRealAsset(
            TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
            propertyName: "TAG",
            expectedOriginalSelectionValue: string.Empty,
            updatedPropertyValue: "customer.region",
            expectedUpdatedSelectionValue: "customer.region",
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
    }
}
