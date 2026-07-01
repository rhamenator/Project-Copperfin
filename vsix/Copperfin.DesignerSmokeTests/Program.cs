using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal static class Program
{
    private static int failures;

    [STAThread]
    private static int Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);

        SmokeDesignSurfaceWithSyntheticReportLayout();
        SmokeLocalizedReportDesignSurfaceContext();
        SmokeLocalizedAssetEditorChrome();
        SmokePseudoLocalizedAssetEditorChrome();
        SmokeLocalizedHostModeSubtitles();
        SmokeLocalizedProjectWorkspaceChrome();
        SmokeLocalizedProjectCommandDebuggerChrome();
        SmokeLocalizedProjectWorkspacePlaceholders();
        SmokeLocalizedExplorerColumnHeaders();
        SmokeLocalizedAssetFamilyGuidance();
        SmokeLocalizedReportLayoutShellSummary();
        SmokeLocalizedSnapshotUndoPropertyStatus();
        SmokeLocalizedLaunchWorkflowDialogText();
        SmokeReportSectionGroupingExplorerTitles();
        SmokeReportSectionScopedObjectFiltering();
        SmokeReportSectionPropertyGridSelection();
        SmokeAssetEditorReportSectionPropertyGridHostUpdate();
        SmokeAssetEditorDeletedReportSectionPropertyGridHostUpdate();
        SmokeAssetEditorLabelSectionPropertyGridHostUpdate();
        SmokeAssetEditorDeletedLabelSectionPropertyGridHostUpdate();
        SmokeAssetEditorReportObjectPropertyGridHostUpdate();
        SmokeAssetEditorUnplacedReportObjectPropertyGridHostUpdate();
        SmokeAssetEditorUnplacedReportObjectHostUpdateRefreshesShellSummary();
        SmokeAssetEditorUnplacedReportObjectPlacementIntoSectionRefreshesContinuity();
        SmokeAssetEditorDeletedReportObjectHostUpdateRefreshesShellSummary();
        SmokeAssetEditorUndoRefreshesReportShellSummary();
        SmokeAssetEditorUndoRefreshesDeletedReportShellSummary();
        SmokeAssetEditorDuplicateObjectCommandRefreshesReportShellSummary();
        SmokeAssetEditorRenameObjectCommandRefreshesReportShellSummary();
        SmokeAssetEditorReorderFrontObjectCommandRefreshesReportShellSummary();
        SmokeAssetEditorDeleteObjectCommandRefreshesReportShellSummary();
        SmokeAssetEditorRestoreObjectCommandRefreshesReportShellSummary();
        SmokeAssetEditorLabelObjectPropertyGridHostUpdate();
        SmokeAssetEditorLabelObjectPlacementIntoUnplacedRefreshesContinuity();
        SmokeAssetEditorUnplacedLabelObjectPropertyGridHostUpdate();
        SmokeAssetEditorUnplacedLabelObjectHostUpdateRefreshesShellSummary();
        SmokeAssetEditorDeletedLabelObjectHostUpdateRefreshesShellSummary();
        SmokeAssetEditorUndoRefreshesLabelShellSummary();
        SmokeAssetEditorUndoRefreshesDeletedLabelShellSummary();
        SmokeAssetEditorDuplicateObjectCommandRefreshesLabelShellSummary();
        SmokeAssetEditorRenameObjectCommandRefreshesDeletedLabelShellSummary();
        SmokeAssetEditorReorderBackObjectCommandRefreshesLabelShellSummary();
        SmokeAssetEditorDeleteObjectCommandRefreshesLabelShellSummary();
        SmokeAssetEditorRestoreObjectCommandRefreshesLabelShellSummary();
        SmokeAssetEditorDeletedReportObjectPropertyGridHostUpdate();
        SmokeAssetEditorDeletedLabelObjectPropertyGridHostUpdate();
        SmokeReportObjectPropertyGridLocalization();
        SmokeSharedDesignerSelectionLocalization();
        SmokeLocalizedReportObjectKindSubtitles();
        SmokeLocalizedReportObjectFallbackTitles();
        SmokeReportSelectionPreservedAcrossExplorerRefresh();
        SmokeDeletedReportSectionExplorerSelection();
        SmokeReportSurfaceScopeSelection();
        SmokeReportSurfaceObjectScopeAlignment();
        SmokeLabelSurfaceScopeSelection();
        SmokeLabelSurfaceObjectScopeAlignment();
        SmokeReportSurfaceObjectDragging();
        SmokeLabelSurfaceObjectDragging();
        SmokeAssetEditorReportDragUsesBatchStudioHostUpdate();
        SmokeAssetEditorReportDragRefreshesShellSummary();
        SmokeAssetEditorLabelDragRefreshesShellSummary();
        SmokeDeletedReportSectionDesignSurfaceRendering();
        WithResolvedRealAssetToolchain(() =>
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
            SmokeRealAssetHostBackedPropertyRoundTrip(
                TryResolveVfpSourceAsset("VFPSource/Wizards/wzapp/template/Books/Reports/by_author.FRX"),
                recordIndex: 7,
                propertyName: "HPOS",
                originalValue: "8645.833",
                updatedValue: "9000",
                expectedObjectTitle: "\"Titles By Author\"",
                expectedSectionTitle: "Title",
                expectedSectionCount: 6,
                expectLabel: false);
            SmokeRealAssetHostBackedPropertyRoundTrip(
                TryResolveVfpSourceAsset("VFPSource/Wizards/wzreport/STYLES/STYLELBL.LBX"),
                recordIndex: 6,
                propertyName: "HPOS",
                originalValue: "6250.000",
                updatedValue: "6500",
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
                expectLabel: false);
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
                expectedObjectCount: 1);
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
                expectedObjectCount: 1);
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
                expectLabel: false);
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
                expectUnplacedObject: true);
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
                expectedObjectCount: 1);
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
            SmokeProjectEditorWithRealAsset(
                ResolveFirstExistingRealAssetPath(
                    TryResolveVfp9InstallAsset(@"Samples\Solution\solution.pjx"),
                    TryResolveVfpSourceAsset("VFPSource/addlabel/addlabel.pjx"),
                    TryResolveVfpSourceAsset("VFPSource/tasklist/tasklist.PJX")),
                expectGroups: new[] { "Forms", "Programs", "Class Libraries", "Classes", "Other Assets" });
            SmokeProjectDebuggerWithRealAsset(
                ResolveFirstExistingRealAssetPath(
                    TryResolveVfp9InstallAsset(@"Samples\Solution\solution.pjx"),
                    TryResolveVfpSourceAsset("VFPSource/addlabel/addlabel.pjx"),
                    TryResolveVfpSourceAsset("VFPSource/tasklist/tasklist.PJX")));
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
        });

        if (failures != 0)
        {
            Console.Error.WriteLine($"{failures} UI smoke test(s) failed.");
            return 1;
        }

        Console.WriteLine("All UI smoke tests passed.");
        return 0;
    }

    private static void SmokeDesignSurfaceWithSyntheticReportLayout()
    {
        using var surface = new CopperfinDesignSurfaceControl
        {
            Size = new Size(900, 700)
        };

        var objects = new List<CopperfinStudioSnapshotObject>
        {
            new CopperfinStudioSnapshotObject
            {
                RecordIndex = 6,
                Title = "customer.company",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "1200" },
                    new() { Name = "VPOS", Value = "2600" },
                    new() { Name = "WIDTH", Value = "4000" },
                    new() { Name = "HEIGHT", Value = "500" },
                    new() { Name = "EXPR", Value = "customer.company" }
                }
            },
            new CopperfinStudioSnapshotObject
            {
                RecordIndex = 9,
                Title = "orphan.note",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "800" },
                    new() { Name = "VPOS", Value = "700" },
                    new() { Name = "WIDTH", Value = "2400" },
                    new() { Name = "HEIGHT", Value = "450" },
                    new() { Name = "EXPR", Value = "orphan.note" }
                }
            }
        };

        var layout = new CopperfinStudioReportLayout
        {
            Sections = new List<CopperfinStudioReportSection>
            {
                new()
                {
                    Id = "detail_1",
                    Title = "Detail",
                    BandKind = "detail",
                    RecordIndex = 1,
                    Top = 2000,
                    Height = 5000,
                    DeletedObjectCount = 2,
                    Objects = new List<CopperfinStudioReportLayoutObject>
                    {
                        new()
                        {
                            RecordIndex = 6,
                            ObjectKind = "field",
                            Title = "customer.company",
                            Expression = "customer.company",
                            Left = 1200,
                            Top = 2600,
                            Width = 4000,
                            Height = 500
                        }
                    }
                }
            },
            UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
            {
                new()
                {
                    RecordIndex = 9,
                    ObjectKind = "field",
                    Title = "orphan.note",
                    Expression = "orphan.note",
                    Left = 800,
                    Top = 700,
                    Width = 2400,
                    Height = 450
                }
            }
        };

        surface.LoadReportLayout(layout, objects);
        var surfaceObjects = ReadPrivateListCount(surface, "objects");
        Expect(surfaceObjects == 2, "synthetic report layout should load placed and unplaced objects into the shared surface");
        Expect(ReadReportSectionProperty(surface, 0, "DeletedObjectCount") == 2,
            "synthetic report layout should preserve per-section deleted-object counts on the shared surface");
        Expect(string.Equals(ReadReportSectionPropertyText(surface, 0, "HeaderTitle"), "Detail (2 deleted objects)", StringComparison.Ordinal),
            "synthetic report layout should surface deleted-object counts in shared section headers");
        using var bitmap = new Bitmap(surface.Width, surface.Height);
        surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        Expect(CountNonWhitePixels(bitmap) > 5000, "synthetic report layout should render visible UI content");
    }

    private static void SmokeLocalizedReportDesignSurfaceContext()
    {
        using var spanishSurface = new CopperfinDesignSurfaceControl(new CopperfinLocalization("es-419"));
        Expect(string.Equals(
                InvokeDesignSurfaceString(spanishSurface, "BuildReportSectionHeaderTitle", "Detalle", 2),
                "Detalle (2 objetos eliminados)",
                StringComparison.Ordinal) &&
               string.Equals(
                   InvokeDesignSurfaceString(spanishSurface, "BuildReportBandKindDisplayText", "page_header"),
                   "Encabezado de página",
                   StringComparison.Ordinal) &&
               string.Equals(
                   InvokeDesignSurfaceString(spanishSurface, "BuildReportBandKindDisplayText", "detail_header"),
                   "Encabezado de detalle",
                   StringComparison.Ordinal) &&
               string.Equals(
                   InvokeDesignSurfaceString(spanishSurface, "BuildDeletedReportSectionHeaderTitle", "Detalle"),
                   "Detalle (eliminada)",
                   StringComparison.Ordinal) &&
               string.Equals(
                    InvokeDesignSurfaceString(spanishSurface, "BuildUnplacedTrayTitle", 1),
                    "Objetos sin sección (1)",
                    StringComparison.Ordinal),
            "Spanish design-surface report context should localize deleted-object, deleted-section, band-kind, and unplaced-object titles");

        using var portugueseSurface = new CopperfinDesignSurfaceControl(new CopperfinLocalization("pt-BR"));
        Expect(string.Equals(
                InvokeDesignSurfaceString(portugueseSurface, "BuildReportSectionHeaderTitle", "Detalhe", 2),
                "Detalhe (2 objetos excluídos)",
                StringComparison.Ordinal) &&
               string.Equals(
                   InvokeDesignSurfaceString(portugueseSurface, "BuildReportBandKindDisplayText", "group_footer"),
                   "Rodapé do grupo",
                   StringComparison.Ordinal) &&
               string.Equals(
                   InvokeDesignSurfaceString(portugueseSurface, "BuildReportBandKindDisplayText", "detail_header"),
                   "Cabeçalho do detalhe",
                   StringComparison.Ordinal) &&
               string.Equals(
                   InvokeDesignSurfaceString(portugueseSurface, "BuildDeletedReportSectionHeaderTitle", "Detalhe"),
                   "Detalhe (excluída)",
                   StringComparison.Ordinal) &&
               string.Equals(
                    InvokeDesignSurfaceString(portugueseSurface, "BuildUnplacedTrayTitle", 1),
                    "Objetos sem seção (1)",
                    StringComparison.Ordinal),
            "Portuguese design-surface report context should localize deleted-object, deleted-section, band-kind, and unplaced-object titles");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoSurface = new CopperfinDesignSurfaceControl(pseudoLocalization);
        Expect(
            InvokeDesignSurfaceString(pseudoSurface, "BuildReportSectionHeaderTitle", "Detail", 2).StartsWith("[!! ", StringComparison.Ordinal) &&
            InvokeDesignSurfaceString(pseudoSurface, "BuildReportBandKindDisplayText", "summary").StartsWith("[!! ", StringComparison.Ordinal) &&
            InvokeDesignSurfaceString(pseudoSurface, "BuildReportBandKindDisplayText", "detail_header").StartsWith("[!! ", StringComparison.Ordinal) &&
            InvokeDesignSurfaceString(pseudoSurface, "BuildDeletedReportSectionHeaderTitle", "Detail").StartsWith("[!! ", StringComparison.Ordinal) &&
            InvokeDesignSurfaceString(pseudoSurface, "BuildUnplacedTrayTitle", 1).StartsWith("[!! ", StringComparison.Ordinal),
            "Pseudo-localized design-surface report context should route deleted-object, deleted-section, band-kind, and unplaced-object titles through the shared catalog");
    }

    private static void SmokeLocalizedAssetEditorChrome()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(HasLabelText(spanishControl, "Diseñador visual de Copperfin"),
            "Spanish editor chrome should localize the asset editor title");
        Expect(HasLabelTextContaining(spanishControl, "activos visuales VFP"),
            "Spanish editor chrome should localize the asset editor subtitle");
        Expect(HasLabelTextContaining(spanishControl, "host nativo de Copperfin Studio"),
            "Spanish editor chrome should localize the asset editor guidance text");
        Expect(HasButtonText(spanishControl, "Abrir en Studio nativo") &&
               HasButtonText(spanishControl, "Mostrar en Explorer") &&
               HasButtonText(spanishControl, "Actualizar") &&
               HasButtonText(spanishControl, "Duplicar objeto") &&
               HasButtonText(spanishControl, "Eliminar objeto") &&
               HasButtonText(spanishControl, "Restaurar objeto"),
            "Spanish editor chrome should localize shell command buttons");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(HasLabelText(portugueseControl, "Designer visual do Copperfin"),
            "Portuguese editor chrome should localize the asset editor title");
        Expect(HasLabelTextContaining(portugueseControl, "ativos visuais VFP"),
            "Portuguese editor chrome should localize the asset editor subtitle");
        Expect(HasLabelTextContaining(portugueseControl, "host nativo do Copperfin Studio"),
            "Portuguese editor chrome should localize the asset editor guidance text");
        Expect(HasButtonText(portugueseControl, "Abrir no Studio nativo") &&
               HasButtonText(portugueseControl, "Revelar no Explorer") &&
               HasButtonText(portugueseControl, "Atualizar") &&
               HasButtonText(portugueseControl, "Duplicar objeto") &&
               HasButtonText(portugueseControl, "Excluir objeto") &&
               HasButtonText(portugueseControl, "Restaurar objeto"),
            "Portuguese editor chrome should localize shell command buttons");
    }

    private static void SmokePseudoLocalizedAssetEditorChrome()
    {
        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);

        Expect(HasLabelText(pseudoControl, pseudoLocalization.Text("AssetEditor.Title")),
            "Pseudo-localized editor chrome should route the asset editor title through the shared catalog");
        Expect(HasLabelTextContaining(pseudoControl, pseudoLocalization.Text("AssetEditor.Subtitle")),
            "Pseudo-localized editor chrome should route the embedded subtitle through the shared catalog");
        Expect(HasButtonText(pseudoControl, pseudoLocalization.Text("AssetEditor.RefreshButton")) &&
               HasButtonText(pseudoControl, pseudoLocalization.Text("AssetEditor.ObjectLifecycle.DuplicateButton")) &&
               HasButtonText(pseudoControl, pseudoLocalization.Text("AssetEditor.ObjectLifecycle.DeleteButton")) &&
               HasButtonText(pseudoControl, pseudoLocalization.Text("AssetEditor.ObjectLifecycle.RestoreButton")) &&
               HasTabPageText(pseudoControl, pseudoLocalization.Text("AssetEditor.Tab.Summary")) &&
               HasLabelText(pseudoControl, pseudoLocalization.Text("AssetEditor.Debugger.ReadyStatus")),
            "Pseudo-localized editor chrome should route buttons, tabs, and status labels through the shared catalog");
        Expect(HasRichTextBoxTextContaining(pseudoControl, pseudoLocalization.Text("AssetEditor.Debugger.InitialSummary")),
            "Pseudo-localized editor chrome should route debugger pane guidance through the shared catalog");
    }

    private static void SmokeLocalizedHostModeSubtitles()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(HasLabelTextContaining(spanishControl, "activos visuales VFP"),
            "Spanish embedded host mode should localize the asset editor subtitle");
        spanishControl.EmbeddedStudioShell = true;
        Expect(HasLabelTextContaining(spanishControl, "superficie de diseñador usada dentro de Visual Studio"),
            "Spanish standalone host mode should localize the asset editor subtitle");
        spanishControl.EmbeddedStudioShell = false;
        Expect(HasLabelTextContaining(spanishControl, "punto de entrega hacia Copperfin Studio"),
            "Spanish embedded host mode should restore the localized asset editor subtitle");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(HasLabelTextContaining(portugueseControl, "ativos visuais VFP"),
            "Portuguese embedded host mode should localize the asset editor subtitle");
        portugueseControl.EmbeddedStudioShell = true;
        Expect(HasLabelTextContaining(portugueseControl, "superfície de designer usada dentro do Visual Studio"),
            "Portuguese standalone host mode should localize the asset editor subtitle");
        portugueseControl.EmbeddedStudioShell = false;
        Expect(HasLabelTextContaining(portugueseControl, "ponto de entrega para o Copperfin Studio"),
            "Portuguese embedded host mode should restore the localized asset editor subtitle");
    }

    private static void SmokeLocalizedProjectWorkspaceChrome()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(HasTabPageText(spanishControl, "Resumen") &&
               HasTabPageText(spanishControl, "Depurador") &&
               HasTabPageText(spanishControl, "Lista de tareas") &&
               HasTabPageText(spanishControl, "Referencias de código") &&
               HasTabPageText(spanishControl, "Explorador de datos") &&
               HasTabPageText(spanishControl, "Explorador de objetos") &&
               HasTabPageText(spanishControl, "Herramientas") &&
               HasTabPageText(spanishControl, "Constructores") &&
               HasTabPageText(spanishControl, "Cobertura") &&
               HasTabPageText(spanishControl, "Base de datos"),
            "Spanish project workspace chrome should localize tab labels");
        Expect(HasCheckBoxText(spanishControl, "Ocultar registros del proyecto"),
            "Spanish object-browser chrome should localize the hide-project option");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(HasTabPageText(portugueseControl, "Resumo") &&
               HasTabPageText(portugueseControl, "Depurador") &&
               HasTabPageText(portugueseControl, "Lista de tarefas") &&
               HasTabPageText(portugueseControl, "Referências de código") &&
               HasTabPageText(portugueseControl, "Explorador de dados") &&
               HasTabPageText(portugueseControl, "Navegador de objetos") &&
               HasTabPageText(portugueseControl, "Ferramentas") &&
               HasTabPageText(portugueseControl, "Construtores") &&
               HasTabPageText(portugueseControl, "Cobertura") &&
               HasTabPageText(portugueseControl, "Banco de dados"),
            "Portuguese project workspace chrome should localize tab labels");
        Expect(HasCheckBoxText(portugueseControl, "Ocultar registros do projeto"),
            "Portuguese object-browser chrome should localize the hide-project option");
    }

    private static void SmokeLocalizedProjectCommandDebuggerChrome()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(HasButtonText(spanishControl, "Compilar proyecto Copperfin") &&
               HasButtonText(spanishControl, "Ejecutar proyecto Copperfin") &&
               HasButtonText(spanishControl, "Depurar proyecto Copperfin") &&
               HasButtonText(spanishControl, "Iniciar sesión") &&
               HasButtonText(spanishControl, "Continuar") &&
               HasButtonText(spanishControl, "Paso") &&
               HasButtonText(spanishControl, "Siguiente") &&
               HasButtonText(spanishControl, "Salir"),
            "Spanish project command and debugger chrome should localize buttons");
        Expect(HasLabelText(spanishControl, "Cargando instantánea de Copperfin Studio...") &&
               HasLabelText(spanishControl, "Depurador listo."),
            "Spanish project command and debugger chrome should localize status labels");
        Expect(HasRichTextBoxTextContaining(spanishControl, "sesión de depuración de Copperfin"),
            "Spanish debugger chrome should localize initial debugger guidance");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(HasButtonText(portugueseControl, "Compilar projeto Copperfin") &&
               HasButtonText(portugueseControl, "Executar projeto Copperfin") &&
               HasButtonText(portugueseControl, "Depurar projeto Copperfin") &&
               HasButtonText(portugueseControl, "Iniciar sessão") &&
               HasButtonText(portugueseControl, "Continuar") &&
               HasButtonText(portugueseControl, "Passo") &&
               HasButtonText(portugueseControl, "Próximo") &&
               HasButtonText(portugueseControl, "Sair"),
            "Portuguese project command and debugger chrome should localize buttons");
        Expect(HasLabelText(portugueseControl, "Carregando instantâneo do Copperfin Studio...") &&
               HasLabelText(portugueseControl, "Depurador pronto."),
            "Portuguese project command and debugger chrome should localize status labels");
        Expect(HasRichTextBoxTextContaining(portugueseControl, "sessão de depuração do Copperfin"),
            "Portuguese debugger chrome should localize initial debugger guidance");
    }

    private static void SmokeLocalizedProjectWorkspacePlaceholders()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(HasRichTextBoxTextContaining(spanishControl, "tareas de Copperfin") &&
               HasRichTextBoxTextContaining(spanishControl, "referencias de código de Copperfin") &&
               HasRichTextBoxTextContaining(spanishControl, "explorador de datos de Copperfin") &&
               HasRichTextBoxTextContaining(spanishControl, "explorador de objetos de Copperfin") &&
               HasRichTextBoxTextContaining(spanishControl, "herramientas de Copperfin") &&
               HasRichTextBoxTextContaining(spanishControl, "constructores de Copperfin") &&
               HasRichTextBoxTextContaining(spanishControl, "señales de cobertura") &&
               HasRichTextBoxTextContaining(spanishControl, "federación de bases de datos"),
            "Spanish project workspace placeholders should localize initial pane text");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(HasRichTextBoxTextContaining(portugueseControl, "lista de tarefas do Copperfin") &&
               HasRichTextBoxTextContaining(portugueseControl, "referências de código do Copperfin") &&
               HasRichTextBoxTextContaining(portugueseControl, "explorador de dados do Copperfin") &&
               HasRichTextBoxTextContaining(portugueseControl, "navegador de objetos do Copperfin") &&
               HasRichTextBoxTextContaining(portugueseControl, "ferramentas do Copperfin") &&
               HasRichTextBoxTextContaining(portugueseControl, "construtores do Copperfin") &&
               HasRichTextBoxTextContaining(portugueseControl, "sinais de cobertura") &&
               HasRichTextBoxTextContaining(portugueseControl, "federação de bancos de dados"),
            "Portuguese project workspace placeholders should localize initial pane text");
    }

    private static void SmokeLocalizedExplorerColumnHeaders()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(HasListViewColumnText(spanishControl, "Objeto") &&
               HasListViewColumnText(spanishControl, "Tipo") &&
               HasListViewColumnText(spanishControl, "Registro") &&
               HasListViewColumnText(spanishControl, "Sección") &&
               HasListViewColumnText(spanishControl, "Objetos") &&
               HasListViewColumnText(spanishControl, "Superior"),
            "Spanish explorer chrome should localize initial list-view column headers");
        ApplyProjectSnapshotForColumnSmoke(spanishControl);
        Expect(HasListViewColumnText(spanishControl, "Elemento") &&
               HasListViewColumnText(spanishControl, "Grupo") &&
               HasListViewColumnText(spanishControl, "Elementos") &&
               HasListViewColumnText(spanishControl, "Excluidos"),
            "Spanish explorer chrome should localize project-mode list-view column headers");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(HasListViewColumnText(portugueseControl, "Objeto") &&
               HasListViewColumnText(portugueseControl, "Tipo") &&
               HasListViewColumnText(portugueseControl, "Registro") &&
               HasListViewColumnText(portugueseControl, "Seção") &&
               HasListViewColumnText(portugueseControl, "Objetos") &&
               HasListViewColumnText(portugueseControl, "Topo"),
            "Portuguese explorer chrome should localize initial list-view column headers");
        ApplyProjectSnapshotForColumnSmoke(portugueseControl);
        Expect(HasListViewColumnText(portugueseControl, "Item") &&
               HasListViewColumnText(portugueseControl, "Grupo") &&
               HasListViewColumnText(portugueseControl, "Itens") &&
               HasListViewColumnText(portugueseControl, "Excluídos"),
            "Portuguese explorer chrome should localize project-mode list-view column headers");
    }

    private static void SmokeLocalizedAssetFamilyGuidance()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(BuildGuidanceText(spanishControl, "form").IndexOf("objetos de formulario", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(spanishControl, "class_library").IndexOf("biblioteca de clases", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(spanishControl, "report").IndexOf("bandas y objetos de informe", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(spanishControl, "label").IndexOf("objetos de etiqueta", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(spanishControl, "menu").IndexOf("estructuras de menú", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(spanishControl, "project").IndexOf("espacios de trabajo agrupados", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(spanishControl, "unknown").IndexOf("instantánea estructurada", StringComparison.Ordinal) >= 0,
            "Spanish asset-family guidance should localize all static guidance cases");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(BuildGuidanceText(portugueseControl, "form").IndexOf("objetos de formulário", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "class_library").IndexOf("biblioteca de classes", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "report").IndexOf("bandas e objetos de relatório", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "label").IndexOf("objetos de etiqueta", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "menu").IndexOf("estruturas de menu", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "project").IndexOf("espaços de trabalho agrupados", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "unknown").IndexOf("instantâneo estruturado", StringComparison.Ordinal) >= 0,
            "Portuguese asset-family guidance should localize all static guidance cases");
    }

    private static void SmokeLocalizedReportLayoutShellSummary()
    {
        var tempPath = Path.GetTempFileName();
        try
        {
            var info = new FileInfo(tempPath);
            var snapshot = new CopperfinStudioSnapshotDocument
            {
                AssetFamily = "report",
                ReportLayout = new CopperfinStudioReportLayout
                {
                    PreviewBoundsAvailable = true,
                    PreviewBoundsLeft = 0,
                    PreviewBoundsTop = 2000,
                    PreviewBoundsRight = 5200,
                    PreviewBoundsBottom = 8100,
                    PreviewBoundsWidth = 5200,
                    PreviewBoundsHeight = 6100,
                    DeletedPreviewBoundsAvailable = true,
                    DeletedPreviewBoundsLeft = 0,
                    DeletedPreviewBoundsTop = 9000,
                    DeletedPreviewBoundsRight = 1900,
                    DeletedPreviewBoundsBottom = 10400,
                    DeletedPreviewBoundsWidth = 1900,
                    DeletedPreviewBoundsHeight = 1400,
                    Sections = new List<CopperfinStudioReportSection> { new(), new(), new() },
                    Groupings = new List<CopperfinStudioReportGrouping> { new() },
                    Settings = new List<CopperfinStudioNamedValue> { new(), new() },
                    UnplacedObjects = new List<CopperfinStudioReportLayoutObject> { new() }
                }
            };

            using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
            var spanishDetails = InvokeAssetEditorString(spanishControl, "BuildSnapshotDetailsText", info, snapshot);
            Expect(spanishDetails.IndexOf("Tamaño:", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Secciones: 3", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Agrupaciones: 1", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Configuraciones: 2", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Objetos sin sección: 1", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Limites de vista previa:", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Limites de vista previa eliminada:", StringComparison.Ordinal) >= 0,
                "Spanish report layout shell summary should localize file details and report counts");

            using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
            var portugueseDetails = InvokeAssetEditorString(portugueseControl, "BuildSnapshotDetailsText", info, snapshot);
            Expect(portugueseDetails.IndexOf("Tamanho:", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Seções: 3", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Agrupamentos: 1", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Configurações: 2", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Objetos sem seção: 1", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Limites da visualização:", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Limites da visualização excluída:", StringComparison.Ordinal) >= 0,
                "Portuguese report layout shell summary should localize file details and report counts");

            var pseudoLocalization = new CopperfinLocalization("qps-ploc");
            using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
            var pseudoDetails = InvokeAssetEditorString(pseudoControl, "BuildSnapshotDetailsText", info, snapshot);
            Expect(pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Details.ReportPreviewBoundsSummary").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Details.DeletedReportPreviewBoundsSummary").Substring(0, 6), StringComparison.Ordinal) >= 0,
                "Pseudo-localized report layout shell summary should route preview bounds through the shared catalog");
        }
        finally
        {
            File.Delete(tempPath);
        }
    }

    private static void SmokeLocalizedSnapshotUndoPropertyStatus()
    {
        var snapshot = BuildStatusSmokeSnapshot();

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(string.Equals(InvokeAssetEditorString(spanishControl, "BuildUndoCommandText", "Reordenar"), "Deshacer Reordenar", StringComparison.Ordinal) &&
               InvokeAssetEditorString(spanishControl, "BuildUndoExecutingStatus", "Reordenar").IndexOf("Ejecutando deshacer del comando: Reordenar", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildUndoFailedStatus", "sin pila").IndexOf("Falló el deshacer del comando: sin pila", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildUndoCompletedStatus", "Reordenar", snapshot).IndexOf("Instantánea cargada: 2 filas de objetos, 7 campos", StringComparison.Ordinal) >= 0,
            "Spanish undo status text should localize command labels and formatted status messages");
        Expect(InvokeAssetEditorString(spanishControl, "BuildSnapshotUnavailableStatus", "host no disponible").IndexOf("Instantánea no disponible: host no disponible", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildSnapshotLoadedStatus", snapshot).IndexOf("Instantánea cargada: 2 filas de objetos, 7 campos, 3 índices complementarios. Deshacer disponible: Reordenar.", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildPropertyApplyingStatus", "WIDTH").IndexOf("Aplicando cambio de WIDTH", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildPropertyUpdateFailedStatus", "campo protegido").IndexOf("Falló la actualización de propiedad: campo protegido", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildPropertyUpdatedStatus", "WIDTH", snapshot).IndexOf("Se actualizó WIDTH. Instantánea cargada: 2 filas de objetos, 7 campos. Deshacer disponible: Reordenar.", StringComparison.Ordinal) >= 0,
            "Spanish snapshot and property status text should localize formatted messages");

        var spanishPropertyGrid = GetPrivatePropertyGrid(spanishControl);
        var reportObject = new CopperfinStudioSnapshotObject
        {
            RecordIndex = 10,
            Properties = new List<CopperfinStudioSnapshotProperty>
            {
                new() { Name = "WIDTH", Value = "4000" },
                new() { Name = "EXPR", Value = "customer.company" }
            }
        };
        spanishPropertyGrid.SelectedObject = CopperfinDesignerSelection.FromSnapshot("report", reportObject, new CopperfinLocalization("es-419"));
        Expect(InvokeAssetEditorString(spanishControl, "BuildPropertyApplyingStatus", "WIDTH").IndexOf("Aplicando cambio de Ancho", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildPropertyUpdatedStatus", "EXPR", snapshot).IndexOf("Se actualizó Expresión.", StringComparison.Ordinal) >= 0,
            "Spanish report object status text should use localized property labels when report-object selection is active");

        var reportSection = new CopperfinStudioReportSection
        {
            RecordIndex = 42,
            Title = "Detail",
            Id = "detail_1",
            BandKind = "detail",
            Top = 2000,
            Height = 5000,
            GroupingContextAvailable = true,
            GroupingExpression = "customer.country"
        };
        spanishPropertyGrid.SelectedObject = CopperfinDesignerSelection.FromReportSection(reportSection, new CopperfinLocalization("es-419"));
        Expect(InvokeAssetEditorString(spanishControl, "BuildPropertyApplyingStatus", "EXPR").IndexOf("Aplicando cambio de Expresión de agrupación", StringComparison.Ordinal) >= 0,
            "Spanish report section status text should use localized section property labels when section selection is active");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(string.Equals(InvokeAssetEditorString(portugueseControl, "BuildUndoCommandText", "Reordenar"), "Desfazer Reordenar", StringComparison.Ordinal) &&
               InvokeAssetEditorString(portugueseControl, "BuildUndoExecutingStatus", "Reordenar").IndexOf("Executando desfazer do comando: Reordenar", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildUndoFailedStatus", "sem pilha").IndexOf("Falha ao desfazer comando: sem pilha", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildUndoCompletedStatus", "Reordenar", snapshot).IndexOf("Instantâneo carregado: 2 linhas de objetos, 7 campos", StringComparison.Ordinal) >= 0,
            "Portuguese undo status text should localize command labels and formatted status messages");
        Expect(InvokeAssetEditorString(portugueseControl, "BuildSnapshotUnavailableStatus", "host indisponível").IndexOf("Instantâneo indisponível: host indisponível", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildSnapshotLoadedStatus", snapshot).IndexOf("Instantâneo carregado: 2 linhas de objetos, 7 campos, 3 índices complementares. Desfazer disponível: Reordenar.", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildPropertyApplyingStatus", "WIDTH").IndexOf("Aplicando alteração de WIDTH", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildPropertyUpdateFailedStatus", "campo protegido").IndexOf("Falha ao atualizar propriedade: campo protegido", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildPropertyUpdatedStatus", "WIDTH", snapshot).IndexOf("WIDTH atualizado. Instantâneo carregado: 2 linhas de objetos, 7 campos. Desfazer disponível: Reordenar.", StringComparison.Ordinal) >= 0,
            "Portuguese snapshot and property status text should localize formatted messages");

        var portuguesePropertyGrid = GetPrivatePropertyGrid(portugueseControl);
        portuguesePropertyGrid.SelectedObject = CopperfinDesignerSelection.FromSnapshot("report", reportObject, new CopperfinLocalization("pt-BR"));
        Expect(InvokeAssetEditorString(portugueseControl, "BuildPropertyApplyingStatus", "WIDTH").IndexOf("Aplicando alteração de Largura", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildPropertyUpdatedStatus", "EXPR", snapshot).IndexOf("Expressão atualizado.", StringComparison.Ordinal) >= 0,
            "Portuguese report object status text should use localized property labels when report-object selection is active");

        portuguesePropertyGrid.SelectedObject = CopperfinDesignerSelection.FromReportSection(reportSection, new CopperfinLocalization("pt-BR"));
        Expect(InvokeAssetEditorString(portugueseControl, "BuildPropertyApplyingStatus", "EXPR").IndexOf("Aplicando alteração de Expressão de agrupamento", StringComparison.Ordinal) >= 0,
            "Portuguese report section status text should use localized section property labels when section selection is active");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        var pseudoPropertyGrid = GetPrivatePropertyGrid(pseudoControl);
        pseudoPropertyGrid.SelectedObject = CopperfinDesignerSelection.FromSnapshot("report", reportObject, pseudoLocalization);
        Expect(InvokeAssetEditorString(pseudoControl, "BuildPropertyApplyingStatus", "WIDTH").IndexOf(pseudoLocalization.Text("AssetEditor.Property.Width"), StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(pseudoControl, "BuildPropertyUpdatedStatus", "EXPR", snapshot).IndexOf(pseudoLocalization.Text("AssetEditor.Property.Expression"), StringComparison.Ordinal) >= 0,
            "Pseudo-localized report object status text should route property labels through the shared catalog");
    }

    private static void SmokeLocalizedLaunchWorkflowDialogText()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        Expect(InvokeAssetEditorString(spanishControl, "BuildAssetPathUnavailableMessage").IndexOf("ruta del activo", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildStudioHostMissingMessage").IndexOf("No se encontró el host de Copperfin Studio", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildStudioLaunchFailedMessage").IndexOf("no se inició correctamente", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildOpenProjectFirstMessage").IndexOf("Abra primero un proyecto PJX", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(spanishControl, "BuildWorkflowLauncherMessage", "Compilación lista.", @"C:\tmp\run.exe").IndexOf("Iniciador: C:\\tmp\\run.exe", StringComparison.Ordinal) >= 0,
            "Spanish launch and workflow dialog text should localize static messages");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(InvokeAssetEditorString(portugueseControl, "BuildAssetPathUnavailableMessage").IndexOf("caminho do ativo", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildStudioHostMissingMessage").IndexOf("host do Copperfin Studio não foi encontrado", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildStudioLaunchFailedMessage").IndexOf("não iniciou corretamente", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildOpenProjectFirstMessage").IndexOf("Abra primeiro um projeto PJX", StringComparison.Ordinal) >= 0 &&
               InvokeAssetEditorString(portugueseControl, "BuildWorkflowLauncherMessage", "Compilação pronta.", @"C:\tmp\run.exe").IndexOf("Inicializador: C:\\tmp\\run.exe", StringComparison.Ordinal) >= 0,
            "Portuguese launch and workflow dialog text should localize static messages");
    }

    private static void SmokeReportSectionGroupingExplorerTitles()
    {
        using var control = new CopperfinAssetEditorControl();

        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "group_header",
                        Title = "Group Header",
                        GroupingContextAvailable = true,
                        GroupingExpression = "customer.country"
                    },
                    new()
                    {
                        Id = "detail",
                        Title = "Detail"
                    }
                }
            }
        };

        var controlType = typeof(CopperfinAssetEditorControl);
        var currentSnapshotField = controlType.GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        var populateSectionListMethod = controlType.GetMethod("PopulateSectionList", BindingFlags.Instance | BindingFlags.NonPublic);
        if (currentSnapshotField is null || populateSectionListMethod is null)
        {
            throw new InvalidOperationException("Could not find CopperfinAssetEditorControl report-grouping smoke hooks.");
        }

        currentSnapshotField.SetValue(control, snapshot);
        populateSectionListMethod.Invoke(control, new object?[] { null });

        var sectionItems = FindListViews(control)
            .SelectMany(list => list.Items.Cast<ListViewItem>())
            .Select(item => item.Text)
            .ToList();

        Expect(sectionItems.Any(text => string.Equals(text, "Group Header - customer.country", StringComparison.Ordinal)),
            "Report explorer should surface grouping expressions in grouped section titles");
        Expect(sectionItems.Any(text => string.Equals(text, "Detail", StringComparison.Ordinal)),
            "Report explorer should preserve ungrouped section titles");
    }

    private static void SmokeReportSectionScopedObjectFiltering()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 10,
                    Title = "detail.line",
                    Subtitle = "field"
                },
                new()
                {
                    RecordIndex = 11,
                    Title = "summary.total",
                    Subtitle = "field"
                },
                new()
                {
                    RecordIndex = 12,
                    Title = "orphan.note",
                    Subtitle = "field"
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail",
                        Title = "Detail",
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new() { RecordIndex = 10 }
                        }
                    },
                    new()
                    {
                        Id = "summary",
                        Title = "Summary",
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new() { RecordIndex = 11 }
                        }
                    }
                },
                UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
                {
                    new() { RecordIndex = 12 }
                }
            }
        };

        using var control = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(control, snapshot);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");

        var detailRows = objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).ToList();
        Expect(detailRows.SequenceEqual(new[] { "detail.line" }),
            "Report explorer should filter object rows to the selected section");

        sectionListView.Items[1].Selected = false;
        sectionListView.Items[0].Selected = false;
        sectionListView.Items[2].Selected = true;
        InvokeAssetEditorVoid(control, "SyncExplorerSelection");

        var unplacedRows = objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).ToList();
        Expect(unplacedRows.SequenceEqual(new[] { "orphan.note" }),
            "Report explorer should filter object rows to the unplaced-object scope");

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyReportSnapshotForExplorerSmoke(spanishControl, snapshot);
        var spanishSectionListView = GetPrivateListView(spanishControl, "sectionListView");
        Expect(spanishSectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, "Objetos sin sección", StringComparison.Ordinal)),
            "Spanish report explorer should localize the unplaced-object row");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyReportSnapshotForExplorerSmoke(portugueseControl, snapshot);
        var portugueseSectionListView = GetPrivateListView(portugueseControl, "sectionListView");
        Expect(portugueseSectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, "Objetos sem seção", StringComparison.Ordinal)),
            "Portuguese report explorer should localize the unplaced-object row");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        ApplyReportSnapshotForExplorerSmoke(pseudoControl, snapshot);
        var pseudoSectionListView = GetPrivateListView(pseudoControl, "sectionListView");
        Expect(pseudoSectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, pseudoLocalization.Text("AssetEditor.ReportSection.UnplacedObjects"), StringComparison.Ordinal)),
            "Pseudo-localized report explorer should route the unplaced-object row through the shared catalog");
    }

    private static void SmokeReportSectionPropertyGridSelection()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail",
                        Title = "Detail",
                        BandKind = "detail_header",
                        RecordIndex = 41,
                        Top = 2000,
                        Height = 5000,
                        DeletedObjectCount = 1,
                        GroupingIndex = 1,
                        GroupingNestingDepth = 2,
                        GroupRole = "header",
                        GroupPartnerSectionId = "group_footer_7",
                        GroupPartnerRecordIndex = 47,
                        GroupPartnerDeleted = true,
                        GroupingContextAvailable = true,
                        GroupingExpression = "customer.country",
                        GroupingExpressionFieldIndex = 2,
                        GroupingExpressionMemoBlockNumber = 7
                    }
                }
            }
        };

        using var control = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(control, snapshot);
        InvokeAssetEditorVoid(control, "SyncExplorerSelection");

        var propertyGrid = GetPrivatePropertyGrid(control);
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection sectionSelection &&
               sectionSelection.RecordIndex == 41,
            "Report section explorer selection should produce a section-rooted property-grid selection");

        if (propertyGrid.SelectedObject is CopperfinDesignerSelection editableSelection)
        {
            TypeDescriptor.GetProperties(editableSelection)["TOP"]?.SetValue(editableSelection, 3200);
            Expect(editableSelection.TryGetUpdate("TOP", out var topTarget, out var topValue) &&
                   string.Equals(topTarget, "TOP", StringComparison.Ordinal) &&
                   string.Equals(topValue, "3200", StringComparison.Ordinal),
                "Report section property-grid selection should serialize TOP edits through the shared update path");

            TypeDescriptor.GetProperties(editableSelection)["HEIGHT"]?.SetValue(editableSelection, 6100);
            Expect(editableSelection.TryGetUpdate("HEIGHT", out var heightTarget, out var heightValue) &&
                   string.Equals(heightTarget, "HEIGHT", StringComparison.Ordinal) &&
                   string.Equals(heightValue, "6100", StringComparison.Ordinal),
                "Report section property-grid selection should serialize HEIGHT edits through the shared update path");

            TypeDescriptor.GetProperties(editableSelection)["EXPR"]?.SetValue(editableSelection, "customer.region");
            Expect(editableSelection.TryGetUpdate("EXPR", out var exprTarget, out var exprValue) &&
                   string.Equals(exprTarget, "EXPR", StringComparison.Ordinal) &&
                   string.Equals(exprValue, "customer.region", StringComparison.Ordinal),
                "Report section property-grid selection should serialize expression edits through the shared update path");
        }

        var spanishSelection = CopperfinDesignerSelection.FromReportSection(snapshot.ReportLayout.Sections[0], new CopperfinLocalization("es-419"));
        var spanishSectionProperties = TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().ToList();
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Altura", StringComparison.Ordinal)),
            "Spanish report section property-grid selection should localize section field labels");
        Expect(string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "RECORDINDEX", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "41", StringComparison.Ordinal) &&
               spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Registro", StringComparison.Ordinal)),
            "Spanish report section property-grid selection should expose localized record metadata");
        Expect(string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "OBJECTCOUNT", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "0", StringComparison.Ordinal) &&
               spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Objetos", StringComparison.Ordinal)),
            "Spanish report section property-grid selection should expose localized object-count metadata");
        Expect(string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "SECTIONSTATE", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Activa", StringComparison.Ordinal),
            "Spanish report section property-grid selection should localize live section state values");
        Expect(string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "BANDKIND", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Encabezado de detalle", StringComparison.Ordinal),
            "Spanish report section property-grid selection should localize visible band-kind values");
        Expect(string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPROLE", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Encabezado", StringComparison.Ordinal),
            "Spanish report section property-grid selection should localize visible grouping-role values");
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Índice de agrupación", StringComparison.Ordinal)) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPINGINDEX", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "1", StringComparison.Ordinal) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPINGNESTINGDEPTH", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "2", StringComparison.Ordinal),
            "Spanish report section property-grid selection should expose localized grouping index metadata");
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Campo de la expresión de agrupación", StringComparison.Ordinal)) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPINGEXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "2", StringComparison.Ordinal) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPINGEXPRESSIONMEMO", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "7", StringComparison.Ordinal),
            "Spanish report section property-grid selection should expose grouping-expression provenance metadata");
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Id de la sección asociada", StringComparison.Ordinal)) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPPARTNERSECTIONID", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "group_footer_7", StringComparison.Ordinal) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPPARTNERRECORD", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "47", StringComparison.Ordinal) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPPARTNERSTATE", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Eliminada", StringComparison.Ordinal),
            "Spanish report section property-grid selection should expose localized grouping partner metadata");

        var portugueseSelection = CopperfinDesignerSelection.FromReportSection(snapshot.ReportLayout.Sections[0], new CopperfinLocalization("pt-BR"));
        var portugueseSectionProperties = TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().ToList();
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Altura", StringComparison.Ordinal)),
            "Portuguese report section property-grid selection should localize section field labels");
        Expect(string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "RECORDINDEX", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "41", StringComparison.Ordinal) &&
               portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Registro", StringComparison.Ordinal)),
            "Portuguese report section property-grid selection should expose localized record metadata");
        Expect(string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "OBJECTCOUNT", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "0", StringComparison.Ordinal) &&
               portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Objetos", StringComparison.Ordinal)),
            "Portuguese report section property-grid selection should expose localized object-count metadata");
        Expect(string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "SECTIONSTATE", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Ativa", StringComparison.Ordinal),
            "Portuguese report section property-grid selection should localize live section state values");
        Expect(string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "BANDKIND", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Cabeçalho do detalhe", StringComparison.Ordinal),
            "Portuguese report section property-grid selection should localize visible band-kind values");
        Expect(string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPROLE", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Cabeçalho", StringComparison.Ordinal),
            "Portuguese report section property-grid selection should localize visible grouping-role values");
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Índice de agrupamento", StringComparison.Ordinal)) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPINGINDEX", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "1", StringComparison.Ordinal) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPINGNESTINGDEPTH", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "2", StringComparison.Ordinal),
            "Portuguese report section property-grid selection should expose localized grouping index metadata");
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Campo da expressão de agrupamento", StringComparison.Ordinal)) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPINGEXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "2", StringComparison.Ordinal) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPINGEXPRESSIONMEMO", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "7", StringComparison.Ordinal),
            "Portuguese report section property-grid selection should expose grouping-expression provenance metadata");
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Id da seção parceira", StringComparison.Ordinal)) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPPARTNERSECTIONID", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "group_footer_7", StringComparison.Ordinal) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPPARTNERRECORD", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "47", StringComparison.Ordinal) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPPARTNERSTATE", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Excluída", StringComparison.Ordinal),
            "Portuguese report section property-grid selection should expose localized grouping partner metadata");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        var pseudoSelection = CopperfinDesignerSelection.FromReportSection(snapshot.ReportLayout.Sections[0], pseudoLocalization);
        var pseudoSectionProperties = TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().ToList();
        Expect(pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Height"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route new field labels through the shared catalog");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "RECORDINDEX", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "41", StringComparison.Ordinal) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Column.Record"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route record metadata through the shared catalog");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "OBJECTCOUNT", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "0", StringComparison.Ordinal) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Column.Objects"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route object-count metadata through the shared catalog");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "SECTIONSTATE", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), pseudoLocalization.Text("AssetEditor.State.Live"), StringComparison.Ordinal),
            "Pseudo-localized report section property-grid selection should route live section state values through the shared catalog");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "BANDKIND", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), pseudoLocalization.Text("AssetEditor.ReportBandKind.DetailHeader"), StringComparison.Ordinal),
            "Pseudo-localized report section property-grid selection should route visible band-kind values through the shared catalog");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "GROUPROLE", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), pseudoLocalization.Text("AssetEditor.GroupRole.Header"), StringComparison.Ordinal),
            "Pseudo-localized report section property-grid selection should route visible grouping-role values through the shared catalog");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "GROUPINGINDEX", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "1", StringComparison.Ordinal) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.GroupingNestingDepth"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route grouping index metadata through the shared catalog");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "GROUPINGEXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "2", StringComparison.Ordinal) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.GroupingExpressionMemoBlock"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route grouping-expression provenance metadata through the shared catalog");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "GROUPPARTNERSTATE", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), pseudoLocalization.Text("AssetEditor.State.Deleted"), StringComparison.Ordinal) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.GroupPartnerSectionId"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route grouping partner metadata through the shared catalog");

        Expect(string.Equals(snapshot.ReportLayout.Sections[0].BandKind, "detail_header", StringComparison.Ordinal),
            "Localized report section property-grid band-kind values should preserve section snapshot contracts");
        Expect(string.Equals(snapshot.ReportLayout.Sections[0].GroupRole, "header", StringComparison.Ordinal),
            "Localized report section property-grid grouping-role values should preserve section snapshot contracts");
        Expect(snapshot.ReportLayout.Sections[0].GroupingIndex == 1 &&
               snapshot.ReportLayout.Sections[0].GroupingNestingDepth == 2,
            "Localized report section property-grid grouping index metadata should preserve section snapshot contracts");
        Expect(snapshot.ReportLayout.Sections[0].RecordIndex == 41,
            "Localized report section property-grid record metadata should preserve section snapshot contracts");
        Expect(snapshot.ReportLayout.Sections[0].Objects.Count == 0,
            "Localized report section property-grid object-count metadata should preserve section snapshot contracts");
        Expect(snapshot.ReportLayout.Sections[0].GroupingExpressionFieldIndex == 2 &&
               snapshot.ReportLayout.Sections[0].GroupingExpressionMemoBlockNumber == 7,
            "Localized report section property-grid grouping-expression provenance should preserve section snapshot contracts");
        Expect(!snapshot.ReportLayout.Sections[0].Deleted,
            "Localized report section property-grid live state values should preserve section snapshot contracts");
        Expect(string.Equals(snapshot.ReportLayout.Sections[0].GroupPartnerSectionId, "group_footer_7", StringComparison.Ordinal) &&
               snapshot.ReportLayout.Sections[0].GroupPartnerRecordIndex == 47 &&
               snapshot.ReportLayout.Sections[0].GroupPartnerDeleted,
            "Localized report section property-grid grouping partner metadata should preserve section snapshot contracts");
    }

    private static void SmokeAssetEditorReportSectionPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor report-section host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorSectionUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildSectionUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 42,
                "A report section host-update smoke should start from a section-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                throw new InvalidOperationException("Could not read the selected report section from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(sectionSelection)["TOP"]?.SetValue(sectionSelection, 3200);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "TOP", 2000);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a report section through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("42") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("TOP") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("3200"),
                "Editing a report section through the shared asset editor should send one invariant TOP update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 42 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["TOP"]?.GetValue(refreshedSelection)?.ToString(), "3200", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a report section through the shared asset editor should preserve section-rooted selection continuity after the host-backed refresh");

            Expect(string.Equals(sectionListView.SelectedItems[0].SubItems[2].Text, "3200", StringComparison.Ordinal),
                "Editing a report section through the shared asset editor should refresh the visible section geometry from the returned snapshot");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedReportSectionPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor deleted-report-section host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeletedReportSectionUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedReportSectionUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 51,
                "A deleted report section host-update smoke should start from a section-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report section from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(sectionSelection)["TOP"]?.SetValue(sectionSelection, 9300);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "TOP", 9000);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a deleted report section through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("51") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("TOP") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("9300"),
                "Editing a deleted report section through the shared asset editor should send one invariant TOP update through the host property contract");

            var deletedSection = snapshot.ReportLayout?.DeletedSections[0]
                ?? throw new InvalidOperationException("Could not read the deleted report section from the shared smoke snapshot.");
            var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal) &&
                   objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "deleted.footer.total" }) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 51 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SECTIONSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["TOP"]?.GetValue(refreshedSelection)?.ToString(), "9300", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a deleted report section through the shared asset editor should preserve report identity, deleted section selection, and deleted-scope continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorReportObjectPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor report-object host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildObjectUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A report object host-update smoke should start from an object-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected report object from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1500);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 1200);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a report object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1500"),
                "Editing a report object through the shared asset editor should send one invariant HPOS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1500", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a report object through the shared asset editor should preserve object-rooted selection and containing section continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUnplacedReportObjectPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor unplaced-report-object host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorUnplacedReportObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildUnplacedReportObjectUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9,
                "An unplaced report object host-update smoke should start from an object-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected unplaced report object from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1100);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 800);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing an unplaced report object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("9") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1100"),
                "Editing an unplaced report object through the shared asset editor should send one invariant HPOS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1100", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing an unplaced report object through the shared asset editor should preserve unplaced object selection and unplaced-scope continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUnplacedReportObjectHostUpdateRefreshesShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor unplaced-report-object summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorUnplacedReportObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildUnplacedReportObjectPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "An unplaced report object summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected unplaced report object for the summary-refresh smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1100);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 800);
            Application.DoEvents();

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1100 T 700 R 5200 B 3100   Size: 4100 x 2400") &&
                   HasLabelTextContaining(control, "Unplaced objects: 1"),
                "Editing an unplaced report object through the shared asset editor should refresh the shell summary from the returned snapshot, including updated preview bounds and unplaced-object context");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUnplacedReportObjectPlacementIntoSectionRefreshesContinuity()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor unplaced-report placement-transition smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorUnplacedReportObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildUnplacedReportObjectPlacementIntoSectionHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9,
                "A report placement-transition smoke should start from an unplaced object-rooted property-grid selection");
            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A report placement-transition smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected unplaced report object for the placement-transition smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["VPOS"]?.SetValue(objectSelection, 2400);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "VPOS", 700);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Moving an unplaced report object into a section through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("9") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("2400"),
                "Moving an unplaced report object into a section through the shared asset editor should send one invariant VPOS update through the host property contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1100 T 2400 R 5200 B 3100   Size: 4100 x 700") &&
                   HasLabelTextContaining(control, "Unplaced objects: 0") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["VPOS"]?.GetValue(refreshedSelection)?.ToString(), "2400", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Moving an unplaced report object into a section through the shared asset editor should refresh the shell summary and preserve record-rooted continuity in the new containing-section scope");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedReportObjectHostUpdateRefreshesShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor deleted-report-object summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeletedReportObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedReportObjectPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A deleted report object summary-refresh smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report object for the summary-refresh smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1600);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 1400);
            Application.DoEvents();

            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1400 T 9400 R 5000 B 10000   Size: 3600 x 600") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Summary (deleted)", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1600", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a deleted report object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving report identity, deleted-object selection, and deleted-section continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUndoRefreshesReportShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor undo summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorUndoPreviewRefreshSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildUndoPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetCurrentSnapshot(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9 &&
                   control.CanHandleUndoCommand() &&
                   string.Equals(control.GetUndoCommandText(), "Undo Move orphan.note", StringComparison.Ordinal),
                "An undo summary-refresh smoke should start from an undo-capable unplaced report object selection");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "An undo summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            Expect(control.TryHandleUndoCommand(),
                "The shared asset editor should accept an undo command when the snapshot exposes a host-backed undo label");
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Undoing through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--undo-mode") &&
                   invocationArguments.Contains("command") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Undoing through the shared asset editor should send one invariant undo command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1100 T 700 R 5200 B 3100   Size: 4100 x 2400") &&
                   HasLabelTextContaining(control, "Unplaced objects: 1") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1100", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Undoing through the shared asset editor should refresh the shell summary from the returned snapshot while preserving unplaced object selection and unplaced-scope continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUndoRefreshesDeletedReportShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor deleted-report undo summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeletedReportUndoPreviewRefreshSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedReportUndoPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetCurrentSnapshot(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13 &&
                   control.CanHandleUndoCommand() &&
                   string.Equals(control.GetUndoCommandText(), "Undo Move deleted.footer.total", StringComparison.Ordinal),
                "A deleted report undo summary-refresh smoke should start from an undo-capable deleted report object selection");

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A deleted report undo summary-refresh smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            Expect(control.TryHandleUndoCommand(),
                "The shared asset editor should accept an undo command for a deleted report selection when the snapshot exposes a host-backed undo label");
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Undoing through the shared asset editor for a deleted report selection should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--undo-mode") &&
                   invocationArguments.Contains("command") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Undoing through the shared asset editor for a deleted report selection should send one invariant undo command through the host contract");

            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1400 T 9400 R 5000 B 10000   Size: 3600 x 600") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Summary (deleted)", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1600", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Undoing through the shared asset editor for a deleted report selection should refresh the shell summary from the returned snapshot while preserving report identity, deleted-object selection, and deleted-section continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDuplicateObjectCommandRefreshesReportShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor duplicate-object smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDuplicateReportObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        const string duplicateUniqueId = "middle-copy-guid";
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");
        var previousDuplicateUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDuplicateReportObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", duplicateUniqueId);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7,
                "A report duplicate-object smoke should start from a live object selection with duplicate and delete commands exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A report duplicate-object smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            duplicateButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Duplicating a report object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--duplicate-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--new-unique-id") &&
                   invocationArguments.Contains(duplicateUniqueId) &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Duplicating a report object through the shared asset editor should send one invariant duplicate-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1200 T 2600 R 6200 B 3200   Size: 5000 x 600") &&
                   HasLabelTextContaining(control, "Duplicated object. Snapshot loaded: 2 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.SubItems[2].Text, "10", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 10 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Live", StringComparison.Ordinal) &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 10 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Duplicating a report object through the shared asset editor should refresh the shell summary and move shared selection continuity to the duplicated report object");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", previousDuplicateUniqueId);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorRenameObjectCommandRefreshesReportShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor rename-object smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorRenameReportObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        const string renamedUniqueId = "middle-renamed-guid";
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");
        var previousRenameUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildRenameReportObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", renamedUniqueId);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var renameButton = GetPrivateButton(control, "renameObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7,
                "A report rename-object smoke should start from a live object selection with rename, duplicate, and delete commands exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A report rename-object smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            renameButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Renaming a report object identity through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--rename-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--new-unique-id") &&
                   invocationArguments.Contains(renamedUniqueId) &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Renaming a report object identity through the shared asset editor should send one invariant rename-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1200 T 2600 R 5200 B 3200   Size: 4000 x 600") &&
                   HasLabelTextContaining(control, "Regenerated object id. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "middle.value", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 7 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Live", StringComparison.Ordinal) &&
                   renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 7 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Renaming a report object identity through the shared asset editor should refresh the shell summary and preserve report selection continuity on the renamed row");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", previousRenameUniqueId);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeleteObjectCommandRefreshesReportShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor delete-object smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeleteReportObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeleteReportObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A report delete-object smoke should start from a live object selection with only the delete command exposed");

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A report delete-object smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            deleteButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Deleting a report object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--delete-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("live-detail-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Deleting a report object through the shared asset editor should send one invariant delete-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1500 T 2600 R 5500 B 3100   Size: 4000 x 500") &&
                   HasLabelTextContaining(control, "Deleted object. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail (deleted)", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   deleteButton.Visible == false &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 52 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Deleting a report object through the shared asset editor should refresh the shell summary, preserve record identity, and flip the shared command surface to restore");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorRestoreObjectCommandRefreshesReportShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor report restore-object smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorRestoreReportObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildRestoreReportObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(!deleteButton.Visible &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13,
                "A report restore-object smoke should start from a deleted object selection with only the restore command exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A report restore-object smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            restoreButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Restoring a report object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--restore-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("13") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("deleted-footer-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Restoring a report object through the shared asset editor should send one invariant restore-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1700 T 2800 R 5300 B 3400   Size: 3600 x 600") &&
                   HasLabelTextContaining(control, "Restored object. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Live", StringComparison.Ordinal) &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   restoreButton.Visible == false &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Restoring a report object through the shared asset editor should refresh the shell summary, preserve record identity, and flip the shared command surface back to delete");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorReorderFrontObjectCommandRefreshesReportShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor reorder-front smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorReorderFrontReportObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildReorderFrontReportObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value", "last.value" }) &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7,
                "A report reorder-front smoke should start from a live object selection with shared duplicate, reorder, and delete commands exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A report reorder-front smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            reorderFrontButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Reordering a report object to the front through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--reorder-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--placement") &&
                   invocationArguments.Contains("front") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Reordering a report object to the front through the shared asset editor should send one invariant reorder-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1000 T 2400 R 5900 B 3100   Size: 4900 x 700") &&
                   HasLabelTextContaining(control, "Moved object to front. Snapshot loaded: 3 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "middle.value", "first.value", "last.value" }) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "middle.value", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 7 &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 7 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Reordering a report object to the front through the shared asset editor should refresh the shell summary, reorder visible rows, and preserve report selection continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorLabelSectionPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor label-section host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorLabelSectionUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildLabelSectionUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 42,
                "A label section host-update smoke should start from a section-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                throw new InvalidOperationException("Could not read the selected label section from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(sectionSelection)["TOP"]?.SetValue(sectionSelection, 3200);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "TOP", 2000);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a label section through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("42") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("TOP") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("3200"),
                "Editing a label section through the shared asset editor should send one invariant TOP update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 42 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["TOP"]?.GetValue(refreshedSelection)?.ToString(), "3200", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a label section through the shared asset editor should preserve label identity and section-rooted continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorLabelObjectPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor label-object host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorLabelObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildLabelObjectUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A label object host-update smoke should start from an object-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected label object from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1500);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 1200);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1500"),
                "Editing a label object through the shared asset editor should send one invariant HPOS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1500", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a label object through the shared asset editor should preserve label identity, object-rooted selection, and containing section continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUnplacedLabelObjectPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor unplaced-label-object host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorUnplacedLabelObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildUnplacedLabelObjectUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9,
                "An unplaced label object host-update smoke should start from an object-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected unplaced label object from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1100);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 800);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing an unplaced label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("9") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1100"),
                "Editing an unplaced label object through the shared asset editor should send one invariant HPOS update through the host property contract");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1100", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing an unplaced label object through the shared asset editor should preserve label identity, unplaced object selection, and unplaced-scope continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorLabelObjectPlacementIntoUnplacedRefreshesContinuity()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor label placement-transition smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorLabelObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildLabelObjectPlacementIntoUnplacedHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A label placement-transition smoke should start from a section-contained object-rooted property-grid selection");
            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label placement-transition smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected label object for the placement-transition smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["VPOS"]?.SetValue(objectSelection, 9000);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "VPOS", 2600);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Moving a label object into the unplaced tray through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("9000"),
                "Moving a label object into the unplaced tray through the shared asset editor should send one invariant VPOS update through the host property contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1200 T 9000 R 5200 B 9500   Size: 4000 x 500") &&
                   HasLabelTextContaining(control, "Unplaced objects: 1") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["VPOS"]?.GetValue(refreshedSelection)?.ToString(), "9000", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Moving a label object into the unplaced tray through the shared asset editor should refresh the shell summary and preserve record-rooted continuity in the new unplaced scope");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUnplacedLabelObjectHostUpdateRefreshesShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor unplaced-label-object summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorUnplacedLabelObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildUnplacedLabelObjectPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9,
                "An unplaced label object summary-refresh smoke should start from an object-rooted property-grid selection");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "An unplaced label object summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected unplaced label object for the summary-refresh smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1100);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 800);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing an unplaced label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("9") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1100"),
                "Editing an unplaced label object through the shared asset editor should send one invariant HPOS update through the host property contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1100 T 700 R 5200 B 3100   Size: 4100 x 2400") &&
                   HasLabelTextContaining(control, "Unplaced objects: 1") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1100", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing an unplaced label object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving label identity, unplaced-object selection, and unplaced-scope continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedLabelObjectHostUpdateRefreshesShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor deleted-label-object summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeletedLabelObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedLabelObjectPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A deleted label object summary-refresh smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted label object for the summary-refresh smoke.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1600);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 1400);
            Application.DoEvents();

            var deletedSection = snapshot.ReportLayout?.DeletedSections[0]
                ?? throw new InvalidOperationException("Could not read the deleted label section from the shared smoke snapshot.");
            var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1400 T 9400 R 5000 B 10000   Size: 3600 x 600") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1600", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a deleted label object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving label identity, deleted-object selection, and deleted-section continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUndoRefreshesLabelShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor label undo summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorLabelUndoPreviewRefreshSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildLabelUndoPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetCurrentSnapshot(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 9 &&
                   control.CanHandleUndoCommand() &&
                   string.Equals(control.GetUndoCommandText(), "Undo Move orphan.note", StringComparison.Ordinal),
                "A label undo summary-refresh smoke should start from an undo-capable unplaced label object selection");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label undo summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            Expect(control.TryHandleUndoCommand(),
                "The shared asset editor should accept an undo command for an unplaced label selection when the snapshot exposes a host-backed undo label");
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Undoing through the shared label asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--undo-mode") &&
                   invocationArguments.Contains("command") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Undoing through the shared label asset editor should send one invariant undo command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1100 T 700 R 5200 B 3100   Size: 4100 x 2400") &&
                   HasLabelTextContaining(control, "Unplaced objects: 1") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 9 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1100", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                   ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Undoing through the shared label asset editor should refresh the shell summary from the returned snapshot while preserving label identity, unplaced-object selection, and unplaced-scope continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorUndoRefreshesDeletedLabelShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor deleted-label undo summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeletedLabelUndoPreviewRefreshSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedLabelUndoPreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetCurrentSnapshot(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13 &&
                   control.CanHandleUndoCommand() &&
                   string.Equals(control.GetUndoCommandText(), "Undo Move deleted.footer.total", StringComparison.Ordinal),
                "A deleted label undo summary-refresh smoke should start from an undo-capable deleted label object selection");

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A deleted label undo summary-refresh smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            Expect(control.TryHandleUndoCommand(),
                "The shared asset editor should accept an undo command for a deleted label selection when the snapshot exposes a host-backed undo label");
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Undoing through the shared asset editor for a deleted label selection should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--undo-mode") &&
                   invocationArguments.Contains("command") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Undoing through the shared asset editor for a deleted label selection should send one invariant undo command through the host contract");

            var deletedSection = snapshot.ReportLayout?.DeletedSections[0]
                ?? throw new InvalidOperationException("Could not read the deleted label section from the shared smoke snapshot.");
            var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1400 T 9400 R 5000 B 10000   Size: 3600 x 600") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1600", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Undoing through the shared asset editor for a deleted label selection should refresh the shell summary from the returned snapshot while preserving label identity, deleted-object selection, and deleted-section continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDuplicateObjectCommandRefreshesLabelShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor label duplicate-object smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDuplicateLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        const string duplicateUniqueId = "middle-copy-guid";
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");
        var previousDuplicateUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDuplicateLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", duplicateUniqueId);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 7,
                "A label duplicate-object smoke should start from a live object selection with duplicate and delete commands exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label duplicate-object smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            duplicateButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Duplicating a label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--duplicate-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("7") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("middle-field-guid") &&
                   invocationArguments.Contains("--new-unique-id") &&
                   invocationArguments.Contains(duplicateUniqueId) &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Duplicating a label object through the shared asset editor should send one invariant duplicate-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1200 T 2600 R 6200 B 3200   Size: 5000 x 600") &&
                   HasLabelTextContaining(control, "Duplicated object. Snapshot loaded: 2 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.SubItems[2].Text, "10", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 10 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Live", StringComparison.Ordinal) &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 10 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Duplicating a label object through the shared asset editor should refresh the shell summary and move shared selection continuity to the duplicated label object");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", previousDuplicateUniqueId);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorRenameObjectCommandRefreshesDeletedLabelShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor label rename-object smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorRenameDeletedLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        const string renamedUniqueId = "deleted-footer-renamed-guid";
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");
        var previousRenameUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildRenameDeletedLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", renamedUniqueId);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var renameButton = GetPrivateButton(control, "renameObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   !deleteButton.Visible &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13,
                "A deleted label rename-object smoke should start from a deleted object selection with rename, duplicate, and restore commands exposed");

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A deleted label rename-object smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            renameButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Renaming a deleted label object identity through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--rename-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("13") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("deleted-footer-guid") &&
                   invocationArguments.Contains("--new-unique-id") &&
                   invocationArguments.Contains(renamedUniqueId) &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Renaming a deleted label object identity through the shared asset editor should send one invariant rename-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1600 T 9400 R 5200 B 10000   Size: 3600 x 600") &&
                   HasLabelTextContaining(control, "Regenerated object id. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", snapshot.ReportLayout!.DeletedSections[0]), StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   !deleteButton.Visible &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Renaming a deleted label object identity through the shared asset editor should refresh the shell summary and preserve deleted label selection continuity on the renamed row");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", previousRenameUniqueId);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorRestoreObjectCommandRefreshesLabelShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor restore-object smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorRestoreLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildRestoreLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(!deleteButton.Visible &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13,
                "A label restore-object smoke should start from a deleted object selection with only the restore command exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label restore-object smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            restoreButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Restoring a label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--restore-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("13") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("deleted-footer-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Restoring a label object through the shared asset editor should send one invariant restore-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1700 T 2800 R 5300 B 3400   Size: 3600 x 600") &&
                   HasLabelTextContaining(control, "Restored object. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Live", StringComparison.Ordinal) &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   restoreButton.Visible == false &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Restoring a label object through the shared asset editor should refresh the shell summary, preserve record identity, and flip the shared command surface back to delete");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeleteObjectCommandRefreshesLabelShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor label delete-object smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeleteLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeleteLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A label delete-object smoke should start from a live object selection with only the delete command exposed");

            Expect(!HasLabelTextContaining(control, "Deleted preview bounds:"),
                "A label delete-object smoke should start without deleted preview-bounds shell text when the initial snapshot omits deleted preview bounds");

            deleteButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Deleting a label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--delete-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("live-detail-guid") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Deleting a label object through the shared asset editor should send one invariant delete-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Deleted preview bounds: L 1500 T 2600 R 5500 B 3100   Size: 4000 x 500") &&
                   HasLabelTextContaining(control, "Deleted object. Snapshot loaded: 1 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail (deleted)", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   deleteButton.Visible == false &&
                   restoreButton.Visible &&
                   restoreButton.Enabled &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 52 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Deleting a label object through the shared asset editor should refresh the shell summary, preserve record identity, and flip the shared command surface to restore");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorReorderBackObjectCommandRefreshesLabelShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor reorder-back smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorReorderBackLabelObjectSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildReorderBackLabelObjectHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var reorderFrontButton = GetPrivateButton(control, "reorderFrontObjectButton");
            var reorderBackButton = GetPrivateButton(control, "reorderBackObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "first.value", "middle.value", "last.value" }) &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 6,
                "A label reorder-back smoke should start from a live object selection with shared duplicate, reorder, and delete commands exposed");

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label reorder-back smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            reorderBackButton.PerformClick();
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Reordering a label object to the back through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--reorder-object") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--unique-id") &&
                   invocationArguments.Contains("first-field-guid") &&
                   invocationArguments.Contains("--placement") &&
                   invocationArguments.Contains("back") &&
                   invocationArguments.Contains("--path") &&
                   invocationArguments.Contains(assetPath),
                "Reordering a label object to the back through the shared asset editor should send one invariant reorder-object command through the host contract");

            Expect(HasLabelTextContaining(control, "Preview bounds: L 1400 T 2600 R 6800 B 3200   Size: 5400 x 600") &&
                   HasLabelTextContaining(control, "Moved object to back. Snapshot loaded: 3 object rows, 5 fields.") &&
                   string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "middle.value", "last.value", "first.value" }) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "first.value", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 6 &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   reorderFrontButton.Visible &&
                   reorderFrontButton.Enabled &&
                   reorderBackButton.Visible &&
                   reorderBackButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Reordering a label object to the back through the shared asset editor should refresh the shell summary, reorder visible rows, and preserve label selection continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedLabelSectionPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor deleted-label-section host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeletedLabelSectionUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedLabelSectionUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 51,
                "A deleted label section host-update smoke should start from a section-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted label section from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(sectionSelection)["TOP"]?.SetValue(sectionSelection, 9300);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "TOP", 9000);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a deleted label section through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("51") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("TOP") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("9300"),
                "Editing a deleted label section through the shared asset editor should send one invariant TOP update through the host property contract");

            var deletedSection = snapshot.ReportLayout?.DeletedSections[0]
                ?? throw new InvalidOperationException("Could not read the deleted label section from the shared smoke snapshot.");
            var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal) &&
                   objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "deleted.footer.total" }) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 51 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["SECTIONSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["TOP"]?.GetValue(refreshedSelection)?.ToString(), "9300", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a deleted label section through the shared asset editor should preserve label identity, deleted section selection, and deleted-scope continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedReportObjectPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor deleted-report-object host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeletedReportObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedReportObjectUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13,
                "A deleted report object host-update smoke should start from an object-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted report object from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1600);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 1400);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a deleted report object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("13") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1600"),
                "Editing a deleted report object through the shared asset editor should send one invariant HPOS update through the host property contract");

            var deletedSection = snapshot.ReportLayout?.DeletedSections[0]
                ?? throw new InvalidOperationException("Could not read the deleted report section from the shared smoke snapshot.");
            var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1600", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a deleted report object through the shared asset editor should preserve report identity, deleted object selection, and deleted-section continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedLabelObjectPropertyGridHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor deleted-label-object host-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorDeletedLabelObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildDeletedLabelObjectUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            sectionListView.Items[0].Selected = false;
            sectionListView.Items[1].Selected = true;
            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");

            objectListView.Items[0].Selected = true;
            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == 13,
                "A deleted label object host-update smoke should start from an object-rooted property-grid selection");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                throw new InvalidOperationException("Could not read the selected deleted label object from the shared asset editor.");
            }

            TypeDescriptor.GetProperties(objectSelection)["HPOS"]?.SetValue(objectSelection, 1600);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", "HPOS", 1400);
            Application.DoEvents();

            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Editing a deleted label object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--from-vs") &&
                   invocationArguments.Contains("--json") &&
                   invocationArguments.Contains("--set-property") &&
                   invocationArguments.Contains("--record") &&
                   invocationArguments.Contains("13") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("--property-value") &&
                   invocationArguments.Contains("1600"),
                "Editing a deleted label object through the shared asset editor should send one invariant HPOS update through the host property contract");

            var deletedSection = snapshot.ReportLayout?.DeletedSections[0]
                ?? throw new InvalidOperationException("Could not read the deleted label section from the shared smoke snapshot.");
            var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", deletedSection);
            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal) &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                   refreshedSelection.RecordIndex == 13 &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1600", StringComparison.Ordinal) &&
                   string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Editing a deleted label object through the shared asset editor should preserve label identity, deleted object selection, and deleted-section continuity after the host-backed refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeReportSelectionPreservedAcrossExplorerRefresh()
    {
        var initialSnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new() { RecordIndex = 10, Title = "detail.line", Subtitle = "field" },
                new() { RecordIndex = 11, Title = "summary.total", Subtitle = "field" }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail",
                        Title = "Detail",
                        RecordIndex = 41,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject> { new() { RecordIndex = 10 } }
                    },
                    new()
                    {
                        Id = "summary",
                        Title = "Summary",
                        RecordIndex = 42,
                        Top = 7600,
                        Height = 2800,
                        Objects = new List<CopperfinStudioReportLayoutObject> { new() { RecordIndex = 11 } }
                    }
                }
            }
        };

        var refreshedSnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new() { RecordIndex = 10, Title = "detail.line", Subtitle = "field" },
                new() { RecordIndex = 11, Title = "summary.total", Subtitle = "field" }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail",
                        Title = "Detail",
                        RecordIndex = 41,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject> { new() { RecordIndex = 10 } }
                    },
                    new()
                    {
                        Id = "summary",
                        Title = "Summary",
                        RecordIndex = 42,
                        Top = 8100,
                        Height = 3100,
                        Objects = new List<CopperfinStudioReportLayoutObject> { new() { RecordIndex = 11 } }
                    }
                }
            }
        };

        using var sectionControl = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(sectionControl, initialSnapshot);
        var sectionListView = GetPrivateListView(sectionControl, "sectionListView");
        sectionListView.Items[0].Selected = false;
        sectionListView.Items[1].Selected = true;
        InvokeAssetEditorVoid(sectionControl, "SyncExplorerSelection");
        var explorerState = InvokeAssetEditorObject(sectionControl, "CaptureExplorerSelectionState");
        SetCurrentSnapshot(sectionControl, refreshedSnapshot);
        InvokeAssetEditorVoid(sectionControl, "PopulateSectionList", explorerState);
        InvokeAssetEditorVoid(sectionControl, "SyncExplorerSelection");
        var propertyGrid = GetPrivatePropertyGrid(sectionControl);
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSectionSelection &&
               refreshedSectionSelection.RecordIndex == 42 &&
               Equals(TypeDescriptor.GetProperties(refreshedSectionSelection)["TOP"]?.GetValue(refreshedSectionSelection), 8100),
            "Report section property-grid selection should survive explorer refresh on the same section");

        using var objectControl = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(objectControl, initialSnapshot);
        var objectExplorerState = InvokeAssetEditorObject(objectControl, "CaptureExplorerSelectionState");
        SetCurrentSnapshot(objectControl, refreshedSnapshot);
        InvokeAssetEditorVoid(objectControl, "PopulateSectionList", objectExplorerState);
        InvokeAssetEditorVoid(objectControl, "SyncExplorerSelection");
        InvokeAssetEditorVoid(objectControl, "SyncSelectionFromSurface", 10);
        var objectPropertyGrid = GetPrivatePropertyGrid(objectControl);
        Expect(objectPropertyGrid.SelectedObject is CopperfinDesignerSelection refreshedObjectSelection &&
               refreshedObjectSelection.RecordIndex == 10,
            "Report object property-grid selection should remain object-rooted after explorer refresh");
    }

    private static void SmokeLocalizedReportObjectFallbackTitles()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 10,
                    Title = string.Empty,
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "<memo block 0>" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 41,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new() { RecordIndex = 10 }
                        }
                    }
                }
            }
        };

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyReportSnapshotForExplorerSmoke(spanishControl, snapshot);
        var spanishObjectListView = GetPrivateListView(spanishControl, "objectListView");
        Expect(spanishObjectListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, "Registro 10", StringComparison.Ordinal)),
            "Spanish shared report object list should localize untitled fallback titles");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyReportSnapshotForExplorerSmoke(portugueseControl, snapshot);
        var portugueseObjectListView = GetPrivateListView(portugueseControl, "objectListView");
        Expect(portugueseObjectListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, "Registro 10", StringComparison.Ordinal)),
            "Portuguese shared report object list should localize untitled fallback titles");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        Expect(InvokeAssetEditorString(pseudoControl, "BuildFallbackObjectTitle", 10).StartsWith("[!! ", StringComparison.Ordinal) &&
               InvokeAssetEditorString(pseudoControl, "BuildFallbackObjectTitle", 10).IndexOf("10", StringComparison.Ordinal) >= 0,
            "Pseudo-localized shared report object list should route untitled fallback titles through the shared catalog");

        using var pseudoSurface = new CopperfinDesignSurfaceControl(pseudoLocalization);
        Expect(InvokeDesignSurfaceString(pseudoSurface, "BuildFallbackObjectTitle", 10).StartsWith("[!! ", StringComparison.Ordinal) &&
               InvokeDesignSurfaceString(pseudoSurface, "BuildFallbackObjectTitle", 10).IndexOf("10", StringComparison.Ordinal) >= 0,
            "Pseudo-localized shared report surface should route untitled fallback captions through the shared catalog");
    }

    private static void SmokeLocalizedReportObjectKindSubtitles()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 10,
                    Title = "detail.field",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 41,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new() { RecordIndex = 10 }
                        }
                    }
                }
            }
        };

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyReportSnapshotForExplorerSmoke(spanishControl, snapshot);
        var spanishObjectListView = GetPrivateListView(spanishControl, "objectListView");
        Expect(spanishObjectListView.Items.Count == 1 &&
               string.Equals(spanishObjectListView.Items[0].SubItems[1].Text, "Campo", StringComparison.Ordinal),
            "Spanish shared report object list should localize report object kind display text");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyReportSnapshotForExplorerSmoke(portugueseControl, snapshot);
        var portugueseObjectListView = GetPrivateListView(portugueseControl, "objectListView");
        Expect(portugueseObjectListView.Items.Count == 1 &&
               string.Equals(portugueseObjectListView.Items[0].SubItems[1].Text, "Campo", StringComparison.Ordinal),
            "Portuguese shared report object list should localize report object kind display text");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        Expect(InvokeAssetEditorString(pseudoControl, "BuildObjectListSubtitle", "report", "field").StartsWith("[!! ", StringComparison.Ordinal) &&
               InvokeAssetEditorString(pseudoControl, "BuildObjectListSubtitle", "label", "group").StartsWith("[!! ", StringComparison.Ordinal),
            "Pseudo-localized shared report/label object type text should route through the shared catalog");

        Expect(string.Equals(snapshot.Objects[0].Subtitle, "field", StringComparison.Ordinal),
            "Shared report object type localization should preserve snapshot subtitle contracts");
    }

    private static void SmokeReportObjectPropertyGridLocalization()
    {
        var snapshotObject = new CopperfinStudioSnapshotObject
        {
            RecordIndex = 10,
            Title = "detail.line",
            Subtitle = "field",
            Properties = new List<CopperfinStudioSnapshotProperty>
            {
                new() { Name = "OBJTYPE", Value = "8" },
                new() { Name = "OBJCODE", Value = "53" },
                new() { Name = "EXPR", Value = "customer.company" },
                new() { Name = "HPOS", Value = "1200" },
                new() { Name = "VPOS", Value = "2600" },
                new() { Name = "WIDTH", Value = "4000" },
                new() { Name = "HEIGHT", Value = "500" },
                new() { Name = "FONTFACE", Value = "Arial" },
                new() { Name = "FONTSTYLE", Value = "1" },
                new() { Name = "FONTSIZE", Value = "10" }
            }
        };

        var spanishSelection = CopperfinDesignerSelection.FromSnapshot("report", snapshotObject, new CopperfinLocalization("es-419"));
        Expect(spanishSelection is not null &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Tipo de objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Estado del objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Expresión", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Tamaño de fuente", StringComparison.Ordinal)),
            "Spanish report object property-grid selection should localize object field labels");
        Expect(string.Equals(TypeDescriptor.GetProperties(spanishSelection)["OBJECTSTATE"]?.GetValue(spanishSelection)?.ToString(), "Activa", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(spanishSelection)["RECORDINDEX"]?.GetValue(spanishSelection)?.ToString(), "10", StringComparison.Ordinal),
            "Spanish report object property-grid selection should localize live object state values and preserve record identity");

        var portugueseSelection = CopperfinDesignerSelection.FromSnapshot("report", snapshotObject, new CopperfinLocalization("pt-BR"));
        Expect(portugueseSelection is not null &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Tipo de objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Estado do objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Expressão", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Tamanho da fonte", StringComparison.Ordinal)),
            "Portuguese report object property-grid selection should localize object field labels");
        Expect(string.Equals(TypeDescriptor.GetProperties(portugueseSelection)["OBJECTSTATE"]?.GetValue(portugueseSelection)?.ToString(), "Ativa", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(portugueseSelection)["RECORDINDEX"]?.GetValue(portugueseSelection)?.ToString(), "10", StringComparison.Ordinal),
            "Portuguese report object property-grid selection should localize live object state values and preserve record identity");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        var pseudoSelection = CopperfinDesignerSelection.FromSnapshot("report", snapshotObject, pseudoLocalization);
        Expect(pseudoSelection is not null &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ObjectType"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ObjectState"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Expression"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.FontSize"), StringComparison.Ordinal)),
            "Pseudo-localized report object property-grid selection should route object field labels through the shared catalog");

        if (pseudoSelection is not null)
        {
            TypeDescriptor.GetProperties(pseudoSelection)["EXPR"]?.SetValue(pseudoSelection, "customer.region");
            Expect(pseudoSelection.TryGetUpdate("EXPR", out var exprTarget, out var exprValue) &&
                   string.Equals(exprTarget, "EXPR", StringComparison.Ordinal) &&
                   string.Equals(exprValue, "customer.region", StringComparison.Ordinal),
                "Localized report object property-grid labels should preserve machine-readable update targets");
        }

        var liveSelection = CopperfinDesignerSelection.FromSnapshot("report", snapshotObject, new CopperfinLocalization("en-US"));
        Expect(liveSelection is not null,
            "Live report object property-grid selection should remain available for shared update-path parity checks");
        if (liveSelection is not null)
        {
            ExpectSelectionUpdate(liveSelection, "HPOS", 1300, "1300",
                "Live report object property-grid selection should serialize HPOS edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "VPOS", 2700, "2700",
                "Live report object property-grid selection should serialize VPOS edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "WIDTH", 4100, "4100",
                "Live report object property-grid selection should serialize WIDTH edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "HEIGHT", 550, "550",
                "Live report object property-grid selection should serialize HEIGHT edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "FONTFACE", "Calibri", "Calibri",
                "Live report object property-grid selection should serialize FONTFACE edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "FONTSTYLE", 2, "2",
                "Live report object property-grid selection should serialize FONTSTYLE edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "FONTSIZE", 11, "11",
                "Live report object property-grid selection should serialize FONTSIZE edits through the shared update path");
        }

        var labelSelection = CopperfinDesignerSelection.FromSnapshot("label", snapshotObject, new CopperfinLocalization("en-US"));
        Expect(labelSelection is not null,
            "Shared label object property-grid selection should expose the same update surface as report objects");
        if (labelSelection is not null)
        {
            ExpectSelectionUpdate(labelSelection, "HPOS", 1500, "1500",
                "Shared label object property-grid selection should serialize HPOS edits through the shared update path");
            ExpectSelectionUpdate(labelSelection, "FONTFACE", "Tahoma", "Tahoma",
                "Shared label object property-grid selection should serialize FONTFACE edits through the shared update path");
            ExpectSelectionUpdate(labelSelection, "FONTSIZE", 12, "12",
                "Shared label object property-grid selection should serialize FONTSIZE edits through the shared update path");
        }

        var deletedSnapshotObject = new CopperfinStudioSnapshotObject
        {
            RecordIndex = 13,
            Deleted = true,
            Title = "deleted.footer.total",
            Subtitle = "field",
            Properties = new List<CopperfinStudioSnapshotProperty>
            {
                new() { Name = "OBJTYPE", Value = "8" },
                new() { Name = "OBJCODE", Value = "53" },
                new() { Name = "EXPR", Value = "customer.deleted_total" },
                new() { Name = "HPOS", Value = "1400" },
                new() { Name = "VPOS", Value = "9200" },
                new() { Name = "WIDTH", Value = "3000" },
                new() { Name = "HEIGHT", Value = "450" },
                new() { Name = "FONTFACE", Value = "Arial" },
                new() { Name = "FONTSTYLE", Value = "0" },
                new() { Name = "FONTSIZE", Value = "9" }
            }
        };

        var deletedSelection = CopperfinDesignerSelection.FromSnapshot("report", deletedSnapshotObject, new CopperfinLocalization("en-US"));
        Expect(deletedSelection is not null &&
               string.Equals(TypeDescriptor.GetProperties(deletedSelection)["OBJECTSTATE"]?.GetValue(deletedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(deletedSelection)["RECORDINDEX"]?.GetValue(deletedSelection)?.ToString(), "13", StringComparison.Ordinal),
            "Deleted report object property-grid selection should expose deleted state and stable record identity");
        if (deletedSelection is not null)
        {
            ExpectSelectionUpdate(deletedSelection, "EXPR", "customer.deleted_region", "customer.deleted_region",
                "Deleted report object property-grid selection should preserve invariant editable update targets");
            ExpectSelectionUpdate(deletedSelection, "HPOS", 1600, "1600",
                "Deleted report object property-grid selection should serialize HPOS edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "VPOS", 9300, "9300",
                "Deleted report object property-grid selection should serialize VPOS edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "WIDTH", 3200, "3200",
                "Deleted report object property-grid selection should serialize WIDTH edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "HEIGHT", 500, "500",
                "Deleted report object property-grid selection should serialize HEIGHT edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "FONTFACE", "Consolas", "Consolas",
                "Deleted report object property-grid selection should serialize FONTFACE edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "FONTSTYLE", 1, "1",
                "Deleted report object property-grid selection should serialize FONTSTYLE edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "FONTSIZE", 10, "10",
                "Deleted report object property-grid selection should serialize FONTSIZE edits through the shared update path");
        }
    }

    private static void SmokeSharedDesignerSelectionLocalization()
    {
        var formSnapshot = new CopperfinStudioSnapshotObject
        {
            RecordIndex = 12,
            Title = "frmCustomer",
            Subtitle = "form",
            Properties = new List<CopperfinStudioSnapshotProperty>
            {
                new() { Name = "OBJNAME", Value = "frmCustomer" },
                new() { Name = "BASECLASS", Value = "form" },
                new() { Name = "PARENT", Value = "_screen" },
                new() { Name = "Left", Value = "100" },
                new() { Name = "Top", Value = "200" },
                new() { Name = "Width", Value = "600" },
                new() { Name = "Height", Value = "400" },
                new() { Name = "Caption", Value = "\"Customers\"" }
            }
        };

        var menuSnapshot = new CopperfinStudioSnapshotObject
        {
            RecordIndex = 8,
            Title = "mnuFile",
            Subtitle = "menu",
            Properties = new List<CopperfinStudioSnapshotProperty>
            {
                new() { Name = "OBJTYPE", Value = "1" },
                new() { Name = "OBJCODE", Value = "22" },
                new() { Name = "NAME", Value = "mnuFile" },
                new() { Name = "PROMPT", Value = "File" },
                new() { Name = "COMMAND", Value = "DO open" },
                new() { Name = "PROCEDURE", Value = "OpenMenu" },
                new() { Name = "MESSAGE", Value = "Open a file" },
                new() { Name = "KEYLABEL", Value = "Ctrl+O" },
                new() { Name = "LEVELNAME", Value = "Top level" },
                new() { Name = "ITEMNUM", Value = "1" }
            }
        };

        var projectSnapshot = new CopperfinStudioSnapshotObject
        {
            RecordIndex = 3,
            Title = "invoice.scx",
            Subtitle = "Form",
            Properties = new List<CopperfinStudioSnapshotProperty>
            {
                new() { Name = "NAME", Value = "invoice.scx" },
                new() { Name = "TYPE", Value = "Form" },
                new() { Name = "KEY", Value = "Invoices" },
                new() { Name = "COMMENTS", Value = "Core invoice form" },
                new() { Name = "EXCLUDE", Value = "F" },
                new() { Name = "MAINPROG", Value = "F" },
                new() { Name = "DEBUG", Value = "T" }
            }
        };

        var genericSnapshot = new CopperfinStudioSnapshotObject
        {
            RecordIndex = 1,
            Title = "shared.asset",
            Subtitle = "generic"
        };

        var spanishFormSelection = CopperfinDesignerSelection.FromSnapshot("form", formSnapshot, new CopperfinLocalization("es-419"));
        Expect(spanishFormSelection is not null &&
               TypeDescriptor.GetProperties(spanishFormSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Nombre del objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishFormSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Clase base", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishFormSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Padre", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishFormSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Título", StringComparison.Ordinal)),
            "Spanish shared form/class selection should localize object identity and caption labels");

        var spanishMenuSelection = CopperfinDesignerSelection.FromSnapshot("menu", menuSnapshot, new CopperfinLocalization("es-419"));
        Expect(spanishMenuSelection is not null &&
               TypeDescriptor.GetProperties(spanishMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Nombre", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Solicitud", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Etiqueta de tecla", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Número de elemento", StringComparison.Ordinal)),
            "Spanish shared menu selection should localize editable menu field labels");

        var spanishProjectSelection = CopperfinDesignerSelection.FromSnapshot("project", projectSnapshot, new CopperfinLocalization("es-419"));
        Expect(spanishProjectSelection is not null &&
               TypeDescriptor.GetProperties(spanishProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Elemento del proyecto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Clave", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Comentarios", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Depurar", StringComparison.Ordinal)),
            "Spanish shared project selection should localize project metadata labels");

        var portugueseFormSelection = CopperfinDesignerSelection.FromSnapshot("form", formSnapshot, new CopperfinLocalization("pt-BR"));
        Expect(portugueseFormSelection is not null &&
               TypeDescriptor.GetProperties(portugueseFormSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Nome do objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseFormSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Classe base", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseFormSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Pai", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseFormSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Legenda", StringComparison.Ordinal)),
            "Portuguese shared form/class selection should localize object identity and caption labels");

        var portugueseMenuSelection = CopperfinDesignerSelection.FromSnapshot("menu", menuSnapshot, new CopperfinLocalization("pt-BR"));
        Expect(portugueseMenuSelection is not null &&
               TypeDescriptor.GetProperties(portugueseMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Nome", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Comando", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Rótulo da tecla", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Número do item", StringComparison.Ordinal)),
            "Portuguese shared menu selection should localize editable menu field labels");

        var portugueseProjectSelection = CopperfinDesignerSelection.FromSnapshot("project", projectSnapshot, new CopperfinLocalization("pt-BR"));
        Expect(portugueseProjectSelection is not null &&
               TypeDescriptor.GetProperties(portugueseProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Item do projeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Chave", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Comentários", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Depurar", StringComparison.Ordinal)),
            "Portuguese shared project selection should localize project metadata labels");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        var pseudoMenuSelection = CopperfinDesignerSelection.FromSnapshot("menu", menuSnapshot, pseudoLocalization);
        var pseudoProjectSelection = CopperfinDesignerSelection.FromSnapshot("project", projectSnapshot, pseudoLocalization);
        var pseudoGenericSelection = CopperfinDesignerSelection.FromSnapshot("dbf", genericSnapshot, pseudoLocalization);
        Expect(pseudoMenuSelection is not null &&
               TypeDescriptor.GetProperties(pseudoMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Name"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoMenuSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.KeyLabel"), StringComparison.Ordinal)) &&
               pseudoProjectSelection is not null &&
               TypeDescriptor.GetProperties(pseudoProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ProjectItem"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoProjectSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Debug"), StringComparison.Ordinal)) &&
               pseudoGenericSelection is not null &&
               TypeDescriptor.GetProperties(pseudoGenericSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Name"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoGenericSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Type"), StringComparison.Ordinal)),
            "Pseudo-localized shared non-report selections should route menu, project, and generic labels through the shared catalog");

        if (pseudoMenuSelection is not null)
        {
            TypeDescriptor.GetProperties(pseudoMenuSelection)["PROMPT"]?.SetValue(pseudoMenuSelection, "Archivo");
            Expect(pseudoMenuSelection.TryGetUpdate("PROMPT", out var promptTarget, out var promptValue) &&
                   string.Equals(promptTarget, "PROMPT", StringComparison.Ordinal) &&
                   string.Equals(promptValue, "Archivo", StringComparison.Ordinal),
                "Localized shared menu selection labels should preserve invariant property update targets");
        }
    }

    private static void SmokeDeletedReportSectionExplorerSelection()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new() { RecordIndex = 13, Title = "deleted.footer.total", Subtitle = "field", Deleted = true }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        GroupingContextAvailable = true,
                        GroupingExpression = "customer.deleted_country",
                        GroupingExpressionFieldIndex = 4,
                        GroupingExpressionMemoBlockNumber = 12,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new() { RecordIndex = 13 }
                        }
                    }
                }
            }
        };

        using var control = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(control, snapshot);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", snapshot.ReportLayout.DeletedSections[0]);
        Expect(sectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, expectedDeletedSectionTitle, StringComparison.Ordinal)),
            "Report explorer should surface deleted section rows");

        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        var propertyGrid = GetPrivatePropertyGrid(control);
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection deletedSectionSelection &&
               deletedSectionSelection.RecordIndex == 51,
            "Deleted report section explorer selection should produce a section-rooted property-grid selection");
        if (propertyGrid.SelectedObject is CopperfinDesignerSelection deletedSelection)
        {
            TypeDescriptor.GetProperties(deletedSelection)["TOP"]?.SetValue(deletedSelection, 9100);
            Expect(deletedSelection.TryGetUpdate("TOP", out var topTarget, out var topValue) &&
                   string.Equals(topTarget, "TOP", StringComparison.Ordinal) &&
                   string.Equals(topValue, "9100", StringComparison.Ordinal),
                "Deleted report section explorer selection should serialize TOP edits through the shared update path");

            TypeDescriptor.GetProperties(deletedSelection)["HEIGHT"]?.SetValue(deletedSelection, 1600);
            Expect(deletedSelection.TryGetUpdate("HEIGHT", out var heightTarget, out var heightValue) &&
                   string.Equals(heightTarget, "HEIGHT", StringComparison.Ordinal) &&
                   string.Equals(heightValue, "1600", StringComparison.Ordinal),
                "Deleted report section explorer selection should serialize HEIGHT edits through the shared update path");

            TypeDescriptor.GetProperties(deletedSelection)["EXPR"]?.SetValue(deletedSelection, "customer.deleted_region");
            Expect(deletedSelection.TryGetUpdate("EXPR", out var exprTarget, out var exprValue) &&
                   string.Equals(exprTarget, "EXPR", StringComparison.Ordinal) &&
                   string.Equals(exprValue, "customer.deleted_region", StringComparison.Ordinal),
                "Deleted report section explorer selection should serialize expression edits through the shared update path");

            Expect(string.Equals(TypeDescriptor.GetProperties(deletedSelection)["SECTIONSTATE"]?.GetValue(deletedSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(deletedSelection)["RECORDINDEX"]?.GetValue(deletedSelection)?.ToString(), "51", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(deletedSelection)["OBJECTCOUNT"]?.GetValue(deletedSelection)?.ToString(), "1", StringComparison.Ordinal),
                "Deleted report section explorer selection should expose deleted section state, record, and object-count metadata");
        }
        Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "deleted.footer.total" }),
            "Deleted report section explorer selection should filter object rows to deleted section membership");

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyReportSnapshotForExplorerSmoke(spanishControl, snapshot);
        var spanishSectionListView = GetPrivateListView(spanishControl, "sectionListView");
        var expectedSpanishDeletedTitle = InvokeAssetEditorString(spanishControl, "BuildDeletedReportSectionListTitle", snapshot.ReportLayout.DeletedSections[0]);
        Expect(spanishSectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, expectedSpanishDeletedTitle, StringComparison.Ordinal)),
            "Spanish report explorer should localize deleted section rows");
        var spanishDeletedSelection = CopperfinDesignerSelection.FromReportSection(snapshot.ReportLayout.DeletedSections[0], new CopperfinLocalization("es-419"));
        Expect(string.Equals(TypeDescriptor.GetProperties(spanishDeletedSelection)["SECTIONSTATE"]?.GetValue(spanishDeletedSelection)?.ToString(), "Eliminada", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(spanishDeletedSelection)["RECORDINDEX"]?.GetValue(spanishDeletedSelection)?.ToString(), "51", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(spanishDeletedSelection)["OBJECTCOUNT"]?.GetValue(spanishDeletedSelection)?.ToString(), "1", StringComparison.Ordinal),
            "Spanish deleted report section property-grid selection should localize deleted section state values and preserve record/object metadata");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyReportSnapshotForExplorerSmoke(portugueseControl, snapshot);
        var portugueseSectionListView = GetPrivateListView(portugueseControl, "sectionListView");
        var expectedPortugueseDeletedTitle = InvokeAssetEditorString(portugueseControl, "BuildDeletedReportSectionListTitle", snapshot.ReportLayout.DeletedSections[0]);
        Expect(portugueseSectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, expectedPortugueseDeletedTitle, StringComparison.Ordinal)),
            "Portuguese report explorer should localize deleted section rows");
        var portugueseDeletedSelection = CopperfinDesignerSelection.FromReportSection(snapshot.ReportLayout.DeletedSections[0], new CopperfinLocalization("pt-BR"));
        Expect(string.Equals(TypeDescriptor.GetProperties(portugueseDeletedSelection)["SECTIONSTATE"]?.GetValue(portugueseDeletedSelection)?.ToString(), "Excluída", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(portugueseDeletedSelection)["RECORDINDEX"]?.GetValue(portugueseDeletedSelection)?.ToString(), "51", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(portugueseDeletedSelection)["OBJECTCOUNT"]?.GetValue(portugueseDeletedSelection)?.ToString(), "1", StringComparison.Ordinal),
            "Portuguese deleted report section property-grid selection should localize deleted section state values and preserve record/object metadata");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        ApplyReportSnapshotForExplorerSmoke(pseudoControl, snapshot);
        var pseudoSectionListView = GetPrivateListView(pseudoControl, "sectionListView");
        var expectedPseudoDeletedTitle = InvokeAssetEditorString(pseudoControl, "BuildDeletedReportSectionListTitle", snapshot.ReportLayout.DeletedSections[0]);
        Expect(pseudoSectionListView.Items.Cast<ListViewItem>().Any(item => string.Equals(item.Text, expectedPseudoDeletedTitle, StringComparison.Ordinal)),
            "Pseudo-localized report explorer should route deleted section rows through the shared catalog");
        var pseudoDeletedSelection = CopperfinDesignerSelection.FromReportSection(snapshot.ReportLayout.DeletedSections[0], pseudoLocalization);
        Expect(string.Equals(TypeDescriptor.GetProperties(pseudoDeletedSelection)["SECTIONSTATE"]?.GetValue(pseudoDeletedSelection)?.ToString(), pseudoLocalization.Text("AssetEditor.State.Deleted"), StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(pseudoDeletedSelection)["RECORDINDEX"]?.GetValue(pseudoDeletedSelection)?.ToString(), "51", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(pseudoDeletedSelection)["OBJECTCOUNT"]?.GetValue(pseudoDeletedSelection)?.ToString(), "1", StringComparison.Ordinal),
            "Pseudo-localized deleted report section property-grid selection should route deleted section state values and preserve record/object metadata");
        Expect(snapshot.ReportLayout.DeletedSections[0].Deleted,
            "Deleted report section property-grid state values should preserve deleted section snapshot contracts");
        Expect(snapshot.ReportLayout.DeletedSections[0].RecordIndex == 51,
            "Deleted report section property-grid record metadata should preserve deleted section snapshot contracts");
        Expect(snapshot.ReportLayout.DeletedSections[0].Objects.Count == 1,
            "Deleted report section property-grid object-count metadata should preserve deleted section snapshot contracts");
    }

    private static void SmokeReportSurfaceScopeSelection()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" }
                    }
                },
                new()
                {
                    RecordIndex = 13,
                    Title = "deleted.footer.total",
                    Subtitle = "field",
                    Deleted = true,
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1400" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" }
                    }
                },
                new()
                {
                    RecordIndex = 9,
                    Title = "orphan.note",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "800" },
                        new() { Name = "VPOS", Value = "700" },
                        new() { Name = "WIDTH", Value = "2400" },
                        new() { Name = "HEIGHT", Value = "450" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "field",
                                Title = "deleted.footer.total",
                                Left = 1400,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                },
                UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
                {
                    new()
                    {
                        RecordIndex = 9,
                        ObjectKind = "field",
                        Title = "orphan.note",
                        Left = 800,
                        Top = 700,
                        Width = 2400,
                        Height = 450
                    }
                }
            }
        };

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();

        ApplyReportSnapshotForExplorerSmoke(control, snapshot);
        InvokeAssetEditorVoid(control, "LoadSurface");
        Application.DoEvents();

        var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");
        using (var bitmap = new Bitmap(surface.Width, surface.Height))
        {
            surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        }

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);

        ClickDesignSurface(surface, GetCenter(ReadReportSectionRectangle(surface, 0, "HeaderBounds")));
        Application.DoEvents();
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection liveSectionSelection &&
               liveSectionSelection.RecordIndex == 42,
            "Clicking a live report section on the shared surface should produce a section-rooted property-grid selection");
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal),
            "Clicking a live report section on the shared surface should select the matching explorer row");
        Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "customer.company" }),
            "Clicking a live report section on the shared surface should scope objects to that section");

        ClickDesignSurface(surface, GetCenter(ReadReportSectionRectangle(surface, 1, "HeaderBounds")));
        Application.DoEvents();
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection deletedSectionSelection &&
               deletedSectionSelection.RecordIndex == 51,
            "Clicking a deleted report section on the shared surface should produce a deleted section-rooted property-grid selection");
        var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", snapshot.ReportLayout.DeletedSections[0]);
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal),
            "Clicking a deleted report section on the shared surface should select the matching deleted explorer row");
        Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "deleted.footer.total" }),
            "Clicking a deleted report section on the shared surface should scope objects to deleted section membership");

        ClickDesignSurface(surface, GetCenter(ReadPrivateRectangle(surface, "unplacedTrayHeaderBounds")));
        Application.DoEvents();
        Expect(propertyGrid.SelectedObject is null,
            "Clicking the unplaced-object tray on the shared surface should clear the property-grid selection");
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal),
            "Clicking the unplaced-object tray on the shared surface should select the unplaced explorer row");
        Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "orphan.note" }),
            "Clicking the unplaced-object tray on the shared surface should scope objects to unplaced rows");
    }

    private static void SmokeReportSurfaceObjectScopeAlignment()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" }
                    }
                },
                new()
                {
                    RecordIndex = 13,
                    Title = "deleted.footer.total",
                    Subtitle = "field",
                    Deleted = true,
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1400" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" }
                    }
                },
                new()
                {
                    RecordIndex = 9,
                    Title = "orphan.note",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "800" },
                        new() { Name = "VPOS", Value = "700" },
                        new() { Name = "WIDTH", Value = "2400" },
                        new() { Name = "HEIGHT", Value = "450" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "field",
                                Title = "deleted.footer.total",
                                Left = 1400,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                },
                UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
                {
                    new()
                    {
                        RecordIndex = 9,
                        ObjectKind = "field",
                        Title = "orphan.note",
                        Left = 800,
                        Top = 700,
                        Width = 2400,
                        Height = 450
                    }
                }
            }
        };

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();

        ApplyReportSnapshotForExplorerSmoke(control, snapshot);
        var sectionListView = GetPrivateListView(control, "sectionListView");
        sectionListView.Items[0].Selected = false;
        sectionListView.Items[1].Selected = true;
        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        InvokeAssetEditorVoid(control, "LoadSurface");
        Application.DoEvents();

        var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");
        using (var bitmap = new Bitmap(surface.Width, surface.Height))
        {
            surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        }

        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);

        ClickDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 0)));
        Application.DoEvents();
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal),
            "Clicking a live report object on the shared surface should select its containing live section row");
        Expect(string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal),
            "Clicking a live report object on the shared surface should select the matching object row");
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection liveObjectSelection &&
               liveObjectSelection.RecordIndex == 6,
            "Clicking a live report object on the shared surface should produce an object-rooted property-grid selection");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") == false,
            "Clicking a live report object on the shared surface should keep its containing live section highlighted");

        ClickDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 1)));
        Application.DoEvents();
        var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", snapshot.ReportLayout.DeletedSections[0]);
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal),
            "Clicking a deleted report object on the shared surface should select its containing deleted section row");
        Expect(string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal),
            "Clicking a deleted report object on the shared surface should select the matching deleted object row");
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection deletedObjectSelection &&
               deletedObjectSelection.RecordIndex == 13,
            "Clicking a deleted report object on the shared surface should keep object-rooted property-grid selection");
        if (propertyGrid.SelectedObject is CopperfinDesignerSelection deletedObjectMetadataSelection)
        {
            Expect(string.Equals(TypeDescriptor.GetProperties(deletedObjectMetadataSelection)["OBJECTSTATE"]?.GetValue(deletedObjectMetadataSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(deletedObjectMetadataSelection)["RECORDINDEX"]?.GetValue(deletedObjectMetadataSelection)?.ToString(), "13", StringComparison.Ordinal),
                "Clicking a deleted report object on the shared surface should expose deleted object state metadata");
        }
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") == false,
            "Clicking a deleted report object on the shared surface should keep its containing deleted section highlighted");

        ClickDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 2)));
        Application.DoEvents();
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal),
            "Clicking an unplaced report object on the shared surface should select the unplaced-object row");
        Expect(string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal),
            "Clicking an unplaced report object on the shared surface should select the matching unplaced object row");
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection unplacedObjectSelection &&
               unplacedObjectSelection.RecordIndex == 9,
            "Clicking an unplaced report object on the shared surface should keep object-rooted property-grid selection");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Clicking an unplaced report object on the shared surface should keep the unplaced-object tray highlighted");
    }

    private static void SmokeLabelSurfaceScopeSelection()
    {
        var snapshot = BuildLabelSurfaceInteractionSmokeSnapshot();
        var reportLayout = snapshot.ReportLayout ?? throw new InvalidOperationException("Could not build shared label surface layout snapshot.");

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();

        ApplyReportSnapshotForExplorerSmoke(control, snapshot);
        InvokeAssetEditorVoid(control, "LoadSurface");
        Application.DoEvents();

        var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");
        using (var bitmap = new Bitmap(surface.Width, surface.Height))
        {
            surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        }

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);

        ClickDesignSurface(surface, GetCenter(ReadReportSectionRectangle(surface, 0, "HeaderBounds")));
        Application.DoEvents();
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection liveSectionSelection &&
               liveSectionSelection.RecordIndex == 42,
            "Clicking a live label section on the shared surface should produce a section-rooted property-grid selection");
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal),
            "Clicking a live label section on the shared surface should select the matching explorer row");
        Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "customer.company" }),
            "Clicking a live label section on the shared surface should scope objects to that section");

        ClickDesignSurface(surface, GetCenter(ReadReportSectionRectangle(surface, 1, "HeaderBounds")));
        Application.DoEvents();
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection deletedSectionSelection &&
               deletedSectionSelection.RecordIndex == 51,
            "Clicking a deleted label section on the shared surface should produce a deleted section-rooted property-grid selection");
        var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", reportLayout.DeletedSections[0]);
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal),
            "Clicking a deleted label section on the shared surface should select the matching deleted explorer row");
        Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "deleted.footer.total" }),
            "Clicking a deleted label section on the shared surface should scope objects to deleted section membership");

        ClickDesignSurface(surface, GetCenter(ReadPrivateRectangle(surface, "unplacedTrayHeaderBounds")));
        Application.DoEvents();
        Expect(propertyGrid.SelectedObject is null,
            "Clicking the unplaced-object tray on the shared label surface should clear the property-grid selection");
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal),
            "Clicking the unplaced-object tray on the shared label surface should select the unplaced explorer row");
        Expect(objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).SequenceEqual(new[] { "orphan.note" }),
            "Clicking the unplaced-object tray on the shared label surface should scope objects to unplaced rows");
    }

    private static void SmokeLabelSurfaceObjectScopeAlignment()
    {
        var snapshot = BuildLabelSurfaceInteractionSmokeSnapshot();
        var reportLayout = snapshot.ReportLayout ?? throw new InvalidOperationException("Could not build shared label surface layout snapshot.");

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();

        ApplyReportSnapshotForExplorerSmoke(control, snapshot);
        var sectionListView = GetPrivateListView(control, "sectionListView");
        sectionListView.Items[0].Selected = false;
        sectionListView.Items[1].Selected = true;
        InvokeAssetEditorVoid(control, "SyncExplorerSelection");
        InvokeAssetEditorVoid(control, "LoadSurface");
        Application.DoEvents();

        var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");
        using (var bitmap = new Bitmap(surface.Width, surface.Height))
        {
            surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        }

        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);

        ClickDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 0)));
        Application.DoEvents();
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal),
            "Clicking a live label object on the shared surface should select its containing live section row");
        Expect(string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal),
            "Clicking a live label object on the shared surface should select the matching object row");
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection liveObjectSelection &&
               liveObjectSelection.RecordIndex == 6,
            "Clicking a live label object on the shared surface should produce an object-rooted property-grid selection");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") == false,
            "Clicking a live label object on the shared surface should keep its containing live section highlighted");

        ClickDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 1)));
        Application.DoEvents();
        var expectedDeletedSectionTitle = InvokeAssetEditorString(control, "BuildDeletedReportSectionListTitle", reportLayout.DeletedSections[0]);
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedSectionTitle, StringComparison.Ordinal),
            "Clicking a deleted label object on the shared surface should select its containing deleted section row");
        Expect(string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "deleted.footer.total", StringComparison.Ordinal),
            "Clicking a deleted label object on the shared surface should select the matching deleted object row");
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection deletedObjectSelection &&
               deletedObjectSelection.RecordIndex == 13,
            "Clicking a deleted label object on the shared surface should keep object-rooted property-grid selection");
        if (propertyGrid.SelectedObject is CopperfinDesignerSelection deletedObjectMetadataSelection)
        {
            Expect(string.Equals(TypeDescriptor.GetProperties(deletedObjectMetadataSelection)["OBJECTSTATE"]?.GetValue(deletedObjectMetadataSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(deletedObjectMetadataSelection)["RECORDINDEX"]?.GetValue(deletedObjectMetadataSelection)?.ToString(), "13", StringComparison.Ordinal),
                "Clicking a deleted label object on the shared surface should expose deleted object state metadata");
        }
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") == false,
            "Clicking a deleted label object on the shared surface should keep its containing deleted section highlighted");

        ClickDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 2)));
        Application.DoEvents();
        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Unplaced objects", StringComparison.Ordinal),
            "Clicking an unplaced label object on the shared surface should select the unplaced-object row");
        Expect(string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "orphan.note", StringComparison.Ordinal),
            "Clicking an unplaced label object on the shared surface should select the matching unplaced object row");
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection unplacedObjectSelection &&
               unplacedObjectSelection.RecordIndex == 9,
            "Clicking an unplaced label object on the shared surface should keep object-rooted property-grid selection");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Clicking an unplaced label object on the shared surface should keep the unplaced-object tray highlighted");
    }

    private static void SmokeDeletedReportSectionDesignSurfaceRendering()
    {
        using var surface = new CopperfinDesignSurfaceControl
        {
            Size = new Size(900, 700)
        };

        var objects = new List<CopperfinStudioSnapshotObject>
        {
            new()
            {
                RecordIndex = 6,
                Title = "customer.company",
                Subtitle = "field",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "1200" },
                    new() { Name = "VPOS", Value = "2600" },
                    new() { Name = "WIDTH", Value = "4000" },
                    new() { Name = "HEIGHT", Value = "500" },
                    new() { Name = "EXPR", Value = "customer.company" }
                }
            },
            new()
            {
                RecordIndex = 13,
                Title = "deleted.footer.total",
                Subtitle = "field",
                Deleted = true,
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "1400" },
                    new() { Name = "VPOS", Value = "9400" },
                    new() { Name = "WIDTH", Value = "3600" },
                    new() { Name = "HEIGHT", Value = "600" },
                    new() { Name = "EXPR", Value = "deleted.footer.total" }
                }
            }
        };

        var layout = new CopperfinStudioReportLayout
        {
            Sections = new List<CopperfinStudioReportSection>
            {
                new()
                {
                    Id = "detail_1",
                    Title = "Detail",
                    BandKind = "detail",
                    RecordIndex = 1,
                    Top = 2000,
                    Height = 5000,
                    Objects = new List<CopperfinStudioReportLayoutObject>
                    {
                        new()
                        {
                            RecordIndex = 6,
                            ObjectKind = "field",
                            Title = "customer.company",
                            Expression = "customer.company",
                            Left = 1200,
                            Top = 2600,
                            Width = 4000,
                            Height = 500
                        }
                    }
                }
            },
            DeletedSections = new List<CopperfinStudioReportSection>
            {
                new()
                {
                    Id = "deleted_footer",
                    Title = "Summary",
                    BandKind = "summary",
                    RecordIndex = 51,
                    Deleted = true,
                    Top = 9000,
                    Height = 1400,
                    DeletedObjectCount = 1,
                    Objects = new List<CopperfinStudioReportLayoutObject>
                    {
                        new()
                        {
                            RecordIndex = 13,
                            ObjectKind = "field",
                            Title = "deleted.footer.total",
                            Expression = "deleted.footer.total",
                            Left = 1400,
                            Top = 9400,
                            Width = 3600,
                            Height = 600
                        }
                    }
                }
            }
        };

        surface.LoadReportLayout(layout, objects);
        Expect(ReadPrivateListCount(surface, "reportSections") == 2,
            "shared report surface should render live and deleted sections together");
        Expect(ReadReportSectionPropertyBool(surface, 1, "Deleted"),
            "shared report surface should mark deleted section visuals");
        var expectedDeletedHeader = InvokeDesignSurfaceString(
            surface,
            "BuildDeletedReportSectionHeaderTitle",
            InvokeDesignSurfaceString(surface, "BuildReportSectionHeaderTitle", "Summary", 1));
        Expect(string.Equals(ReadReportSectionPropertyText(surface, 1, "HeaderTitle"), expectedDeletedHeader, StringComparison.Ordinal),
            "shared report surface should label deleted section headers distinctly");

        using var bitmap = new Bitmap(surface.Width, surface.Height);
        surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        Expect(CountNonWhitePixels(bitmap) > 5000, "shared report surface should render visible deleted-section UI content");
    }

    private static void SmokeReportSurfaceObjectDragging()
    {
        using var surface = new CopperfinDesignSurfaceControl
        {
            Size = new Size(1400, 1000)
        };

        var objects = new List<CopperfinStudioSnapshotObject>
        {
            new()
            {
                RecordIndex = 6,
                Title = "customer.company",
                Subtitle = "field",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "1200" },
                    new() { Name = "VPOS", Value = "2600" },
                    new() { Name = "WIDTH", Value = "4000" },
                    new() { Name = "HEIGHT", Value = "500" },
                    new() { Name = "EXPR", Value = "customer.company" }
                }
            },
            new()
            {
                RecordIndex = 13,
                Deleted = true,
                Title = "deleted.footer.total",
                Subtitle = "field",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "1400" },
                    new() { Name = "VPOS", Value = "9400" },
                    new() { Name = "WIDTH", Value = "3600" },
                    new() { Name = "HEIGHT", Value = "600" },
                    new() { Name = "EXPR", Value = "deleted.footer.total" }
                }
            },
            new()
            {
                RecordIndex = 9,
                Title = "orphan.note",
                Subtitle = "field",
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "800" },
                    new() { Name = "VPOS", Value = "700" },
                    new() { Name = "WIDTH", Value = "2400" },
                    new() { Name = "HEIGHT", Value = "450" },
                    new() { Name = "EXPR", Value = "orphan.note" }
                }
            }
        };

        var layout = new CopperfinStudioReportLayout
        {
            Sections = new List<CopperfinStudioReportSection>
            {
                new()
                {
                    Id = "detail_1",
                    Title = "Detail",
                    BandKind = "detail",
                    RecordIndex = 42,
                    Top = 2000,
                    Height = 5000,
                    Objects = new List<CopperfinStudioReportLayoutObject>
                    {
                        new()
                        {
                            RecordIndex = 6,
                            ObjectKind = "field",
                            Title = "customer.company",
                            Expression = "customer.company",
                            Left = 1200,
                            Top = 2600,
                            Width = 4000,
                            Height = 500
                        }
                    }
                }
            },
            DeletedSections = new List<CopperfinStudioReportSection>
            {
                new()
                {
                    Id = "deleted_footer",
                    Title = "Summary",
                    BandKind = "summary",
                    RecordIndex = 51,
                    Deleted = true,
                    Top = 9000,
                    Height = 1400,
                    Objects = new List<CopperfinStudioReportLayoutObject>
                    {
                        new()
                        {
                            RecordIndex = 13,
                            ObjectKind = "field",
                            Title = "deleted.footer.total",
                            Expression = "deleted.footer.total",
                            Left = 1400,
                            Top = 9400,
                            Width = 3600,
                            Height = 600
                        }
                    }
                }
            },
            UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
            {
                new()
                {
                    RecordIndex = 9,
                    ObjectKind = "field",
                    Title = "orphan.note",
                    Expression = "orphan.note",
                    Left = 800,
                    Top = 700,
                    Width = 2400,
                    Height = 450
                }
            }
        };

        surface.LoadReportLayout(layout, objects);
        RenderDesignSurface(surface);

        var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
        var moves = new List<(int RecordIndex, int Left, int Top)>();
        surface.ObjectMoved += (recordIndex, left, top) => moves.Add((recordIndex, left, top));

        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 0)), 18, 12);
        var expectedLiveLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
        var expectedLiveTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 6 &&
               moves[0].Left == expectedLiveLeft &&
               moves[0].Top == expectedLiveTop,
            "Dragging a live report object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging a live report object on the shared surface should keep the containing live section highlighted");

        moves.Clear();
        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 1)), 14, 10);
        var expectedDeletedLeft = (int)Math.Round(1400 + (14 / Math.Max(0.2F, scale)));
        var expectedDeletedTop = (int)Math.Round(9400 + (10 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 13 &&
               moves[0].Left == expectedDeletedLeft &&
               moves[0].Top == expectedDeletedTop,
            "Dragging a deleted report object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging a deleted report object on the shared surface should keep the containing deleted section highlighted");

        moves.Clear();
        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 2)), 16, 9);
        var expectedUnplacedLeft = (int)Math.Round(800 + (16 / Math.Max(0.2F, scale)));
        var expectedUnplacedTop = (int)Math.Round(700 + (9 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 9 &&
               moves[0].Left == expectedUnplacedLeft &&
               moves[0].Top == expectedUnplacedTop,
            "Dragging an unplaced report object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging an unplaced report object on the shared surface should keep the unplaced-object tray highlighted");
    }

    private static void SmokeLabelSurfaceObjectDragging()
    {
        using var surface = new CopperfinDesignSurfaceControl
        {
            Size = new Size(1400, 1000)
        };

        var snapshot = BuildLabelSurfaceInteractionSmokeSnapshot();
        var reportLayout = snapshot.ReportLayout ?? throw new InvalidOperationException("Could not build shared label surface layout snapshot.");
        surface.LoadReportLayout(reportLayout, snapshot.Objects);
        RenderDesignSurface(surface);

        var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
        var moves = new List<(int RecordIndex, int Left, int Top)>();
        surface.ObjectMoved += (recordIndex, left, top) => moves.Add((recordIndex, left, top));

        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 0)), 18, 12);
        var expectedLiveLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
        var expectedLiveTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 6 &&
               moves[0].Left == expectedLiveLeft &&
               moves[0].Top == expectedLiveTop,
            "Dragging a live label object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging a live label object on the shared surface should keep the containing live section highlighted");

        moves.Clear();
        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 1)), 14, 10);
        var expectedDeletedLeft = (int)Math.Round(1400 + (14 / Math.Max(0.2F, scale)));
        var expectedDeletedTop = (int)Math.Round(9400 + (10 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 13 &&
               moves[0].Left == expectedDeletedLeft &&
               moves[0].Top == expectedDeletedTop,
            "Dragging a deleted label object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 51 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging a deleted label object on the shared surface should keep the containing deleted section highlighted");

        moves.Clear();
        DragDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 2)), 16, 9);
        var expectedUnplacedLeft = (int)Math.Round(800 + (16 / Math.Max(0.2F, scale)));
        var expectedUnplacedTop = (int)Math.Round(700 + (9 / Math.Max(0.2F, scale)));
        Expect(moves.Count == 1 &&
               moves[0].RecordIndex == 9 &&
               moves[0].Left == expectedUnplacedLeft &&
               moves[0].Top == expectedUnplacedTop,
            "Dragging an unplaced label object on the shared surface should emit invariant HPOS/VPOS updates");
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 9 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
               ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            "Dragging an unplaced label object on the shared surface should keep the unplaced-object tray highlighted");
    }

    private static void SmokeAssetEditorReportDragUsesBatchStudioHostUpdate()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor batch-update smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorBatchUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildBatchUpdateHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");
            RenderDesignSurface(surface);

            var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
            var start = GetCenter(ReadSurfaceObjectRectangle(surface, 0));
            ClickDesignSurface(surface, start);
            Application.DoEvents();

            DragDesignSurface(surface, start, 18, 12);
            Application.DoEvents();

            var expectedLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
            var expectedTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Dragging a report object through the shared asset editor should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--visual-object-update-batch") &&
                   invocationArguments.Contains("--selected-record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains(expectedLeft.ToString()) &&
                   invocationArguments.Contains(expectedTop.ToString()) &&
                   !invocationArguments.Contains("--set-property"),
                "Dragging a report object through the shared asset editor should send one batch update with invariant HPOS/VPOS changes");

            Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                   string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                   ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6 &&
                   ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
                   !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
                "Dragging a report object through the shared asset editor should preserve section and object selection continuity after the batch refresh");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorReportDragRefreshesShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared asset-editor drag summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorBatchUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "invoice.frx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildBatchUpdatePreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetCurrentSnapshot(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface.");
            RenderDesignSurface(surface);

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A drag summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
            var start = GetCenter(ReadSurfaceObjectRectangle(surface, 0));
            ClickDesignSurface(surface, start);
            Application.DoEvents();

            DragDesignSurface(surface, start, 18, 12);
            Application.DoEvents();

            var expectedLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
            var expectedTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Dragging a report object through the shared asset editor for summary refresh should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--visual-object-update-batch") &&
                   invocationArguments.Contains("--selected-record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains(expectedLeft.ToString()) &&
                   invocationArguments.Contains(expectedTop.ToString()),
                "Dragging a report object through the shared asset editor for summary refresh should send one invariant batch update through the host contract");

            var refreshed = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                    HasLabelTextContaining(control, "Preview bounds: L 1500 T 2800 R 6100 B 6300   Size: 4600 x 3500") &&
                    string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                    string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                    propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                    refreshedSelection.RecordIndex == 6 &&
                    string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1500", StringComparison.Ordinal) &&
                    string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                    ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6);
            Expect(refreshed,
                "Dragging a report object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving report identity, object selection, and containing-section continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorLabelDragRefreshesShellSummary()
    {
        if (Path.DirectorySeparatorChar == '\\')
        {
            Console.WriteLine("SKIP: shared label asset-editor drag summary-refresh smoke requires a POSIX scriptable fake Studio host.");
            return;
        }

        var snapshot = BuildAssetEditorLabelObjectUpdateSmokeSnapshot();
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmoke-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateSmokeAssetFile(tempRoot, "cust.lbx");
        var scriptPath = Path.Combine(tempRoot, "fake-studio-host.sh");
        var logPath = Path.Combine(tempRoot, "studio-host.log");
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousLogPath = Environment.GetEnvironmentVariable("COPPERFIN_SMOKE_LOG");

        try
        {
            File.WriteAllText(logPath, string.Empty);
            CreateFakeStudioHostScript(scriptPath, BuildBatchLabelUpdatePreviewRefreshHostResponseJson());
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", scriptPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", logPath);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();

            ApplyReportSnapshotForExplorerSmoke(control, snapshot);
            SetCurrentSnapshot(control, snapshot);
            SetPrivateField(control, "currentPath", assetPath);
            GetPrivateLabel(control, "detailsLabel").Text = InvokeAssetEditorString(control, "BuildSnapshotDetailsText", new FileInfo(assetPath), snapshot);
            InvokeAssetEditorVoid(control, "LoadSurface");
            Application.DoEvents();

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared label design surface.");
            RenderDesignSurface(surface);

            Expect(!HasLabelTextContaining(control, "Preview bounds:"),
                "A label drag summary-refresh smoke should start without preview-bounds shell text when the initial snapshot omits preview bounds");

            var scale = InvokeDesignSurfaceFloat(surface, "CalculateReportScale");
            var start = GetCenter(ReadSurfaceObjectRectangle(surface, 0));
            ClickDesignSurface(surface, start);
            Application.DoEvents();

            DragDesignSurface(surface, start, 18, 12);
            Application.DoEvents();

            var expectedLeft = (int)Math.Round(1200 + (18 / Math.Max(0.2F, scale)));
            var expectedTop = (int)Math.Round(2600 + (12 / Math.Max(0.2F, scale)));
            var logLines = File.ReadAllLines(logPath);
            var invocationStartCount = logLines.Count(line => string.Equals(line, "BEGIN", StringComparison.Ordinal));
            Expect(invocationStartCount == 1,
                "Dragging a label object through the shared asset editor for summary refresh should invoke the Studio host exactly once");

            var invocationArguments = logLines.Skip(1).ToList();
            Expect(invocationArguments.Contains("--visual-object-update-batch") &&
                   invocationArguments.Contains("--selected-record") &&
                   invocationArguments.Contains("6") &&
                   invocationArguments.Contains("--property-name") &&
                   invocationArguments.Contains("HPOS") &&
                   invocationArguments.Contains("VPOS") &&
                   invocationArguments.Contains(expectedLeft.ToString()) &&
                   invocationArguments.Contains(expectedTop.ToString()),
                "Dragging a label object through the shared asset editor for summary refresh should send one invariant batch update through the host contract");

            var refreshed = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                    HasLabelTextContaining(control, "Preview bounds: L 1500 T 2800 R 6100 B 6300   Size: 4600 x 3500") &&
                    string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "Detail", StringComparison.Ordinal) &&
                    string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, "customer.company", StringComparison.Ordinal) &&
                    propertyGrid.SelectedObject is CopperfinDesignerSelection refreshedSelection &&
                    refreshedSelection.RecordIndex == 6 &&
                    string.Equals(TypeDescriptor.GetProperties(refreshedSelection)["HPOS"]?.GetValue(refreshedSelection)?.ToString(), "1500", StringComparison.Ordinal) &&
                    string.Equals(ReadPrivateStringField(surface, "assetFamily"), "label", StringComparison.Ordinal) &&
                    ReadPrivateNullableInt(surface, "selectedRecordIndex") == 6);
            Expect(refreshed,
                "Dragging a label object through the shared asset editor should refresh the shell summary from the returned snapshot while preserving label identity, object selection, and containing-section continuity");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_SMOKE_LOG", previousLogPath);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorWithRealAsset(string? path, string expectSection)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real asset candidate" : path)} not found.");
            return;
        }

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();
        control.LoadDocument(path!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => FindListViews(control).Any(list => list.Items.Count > 0));
        Expect(loaded, $"editor should load snapshot data for {path}");

        var sectionFound = FindListViews(control)
            .SelectMany(list => list.Items.Cast<ListViewItem>())
            .Any(item => string.Equals(item.Text, expectSection, StringComparison.OrdinalIgnoreCase) ||
                         item.Text.IndexOf(expectSection, StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(sectionFound, $"editor should surface section '{expectSection}' for {path}");
        Expect(HasLabelTextContaining(control, "Sections:") &&
               HasLabelTextContaining(control, "Settings:") &&
               HasLabelTextContaining(control, "Unplaced objects:"),
            $"editor should surface a report layout summary for {path}");

        var designSurface = FindDesignSurface(control);
        Expect(designSurface is not null, $"design surface should exist for {path}");
        if (designSurface is not null)
        {
            using var bitmap = new Bitmap(Math.Max(1, designSurface.Width), Math.Max(1, designSurface.Height));
            designSurface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
            Expect(CountNonWhitePixels(bitmap) > 5000, $"design surface should render visible content for {path}");
        }

        TearDownForm(hostForm);
    }

    private static void SmokeRealAssetHostBackedPropertyRoundTrip(
        string? sourcePath,
        int recordIndex,
        string propertyName,
        string originalValue,
        string updatedValue,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        bool expectUnplacedObject = false)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset write smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                propertyName,
                originalValue,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject,
                $"initial real asset snapshot should preserve {propertyName}");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                recordIndex,
                propertyName,
                updatedValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset write smoke should update {propertyName} for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                updateResult.Document,
                recordIndex,
                propertyName,
                updatedValue,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject,
                $"updated real asset snapshot should preserve {propertyName}");
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset write smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset write smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterUpdate.Document,
                recordIndex,
                propertyName,
                updatedValue,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject,
                $"reloaded updated real asset snapshot should preserve {propertyName}");
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset write smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset write smoke should reload undone snapshot data for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterUndo.Document,
                recordIndex,
                propertyName,
                originalValue,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject,
                $"reloaded undone real asset snapshot should preserve {propertyName}");
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset write smoke should clear undo after restoring {propertyName} for {sourcePath}");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeRealAssetHostBackedSectionRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        string propertyName,
        string originalRawValue,
        string updatedRawValue,
        int expectedOriginalLayoutValue,
        int expectedUpdatedLayoutValue,
        int expectedSectionCount,
        bool expectLabel,
        int expectedObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset section write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetSectionWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset section smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            AssertRealAssetSectionSnapshot(
                loaded.Document,
                recordIndex,
                expectedSectionTitle,
                propertyName,
                originalRawValue,
                expectedOriginalLayoutValue,
                expectedSectionCount,
                expectLabel,
                expectedObjectCount,
                $"initial real asset section snapshot should preserve {propertyName}");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                recordIndex,
                propertyName,
                updatedRawValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset section smoke should update {propertyName} for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetSectionSnapshot(
                updateResult.Document,
                recordIndex,
                expectedSectionTitle,
                propertyName,
                updatedRawValue,
                expectedUpdatedLayoutValue,
                expectedSectionCount,
                expectLabel,
                expectedObjectCount,
                $"updated real asset section snapshot should preserve {propertyName}");
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset section smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset section smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetSectionSnapshot(
                reloadedAfterUpdate.Document,
                recordIndex,
                expectedSectionTitle,
                propertyName,
                updatedRawValue,
                expectedUpdatedLayoutValue,
                expectedSectionCount,
                expectLabel,
                expectedObjectCount,
                $"reloaded updated real asset section snapshot should preserve {propertyName}");
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset section snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset section smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset section smoke should reload undone snapshot data for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetSectionSnapshot(
                reloadedAfterUndo.Document,
                recordIndex,
                expectedSectionTitle,
                propertyName,
                originalRawValue,
                expectedOriginalLayoutValue,
                expectedSectionCount,
                expectLabel,
                expectedObjectCount,
                $"reloaded undone real asset section snapshot should preserve {propertyName}");
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset section smoke should clear undo after restoring {propertyName} for {sourcePath}");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeRealAssetHostBackedBatchPropertyRoundTrip(
        string? sourcePath,
        int recordIndex,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges,
        IReadOnlyList<KeyValuePair<string, string>> originalValues,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset batch write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetBatchWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset batch smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            foreach (var property in originalValues)
            {
                AssertRealAssetRoundTripSnapshot(
                    loaded.Document,
                    recordIndex,
                    property.Key,
                    property.Value,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"initial real asset batch snapshot should preserve {property.Key}");
            }

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperties(assetPath, recordIndex, propertyChanges);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset batch smoke should update {propertyChanges.Count} properties for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            foreach (var property in propertyChanges)
            {
                AssertRealAssetRoundTripSnapshot(
                    updateResult.Document,
                    recordIndex,
                    property.Key,
                    property.Value,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"updated real asset batch snapshot should preserve {property.Key}");
            }
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset batch smoke should expose undo after updating {propertyChanges.Count} properties for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset batch smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            foreach (var property in propertyChanges)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    property.Key,
                    property.Value,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded updated real asset batch snapshot should preserve {property.Key}");
            }
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset batch snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset batch smoke should undo {propertyChanges.Count} properties in one command for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset batch smoke should reload restored snapshot data after the command undo for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            foreach (var property in originalValues)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    property.Key,
                    property.Value,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded undone real asset batch snapshot should preserve {property.Key}");
            }

            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset batch smoke should clear undo after restoring the batch update for {sourcePath}");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeRealAssetHostBackedPlacementRoundTrip(
        string? sourcePath,
        int recordIndex,
        string propertyName,
        string originalValue,
        string updatedValue,
        string expectedObjectTitle,
        string initialSectionTitle,
        string updatedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalUnplacedObjectCount,
        int expectedUpdatedUnplacedObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset placement candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetPlacements-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset placement smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                propertyName,
                originalValue,
                expectedObjectTitle,
                initialSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: true,
                $"initial real asset placement snapshot should preserve {propertyName}");
            AssertRealAssetUnplacedObjectCount(
                loaded.Document,
                expectedOriginalUnplacedObjectCount,
                $"initial real asset placement snapshot should preserve unplaced-object counts");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                recordIndex,
                propertyName,
                updatedValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real asset placement smoke should update {propertyName} for {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                updateResult.Document,
                recordIndex,
                propertyName,
                updatedValue,
                expectedObjectTitle,
                updatedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"updated real asset placement snapshot should preserve {propertyName}");
            AssertRealAssetUnplacedObjectCount(
                updateResult.Document,
                expectedUpdatedUnplacedObjectCount,
                $"updated real asset placement snapshot should preserve unplaced-object counts");
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real asset placement smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset placement smoke should reload updated snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterUpdate.Document,
                recordIndex,
                propertyName,
                updatedValue,
                expectedObjectTitle,
                updatedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded updated real asset placement snapshot should preserve {propertyName}");
            AssertRealAssetUnplacedObjectCount(
                reloadedAfterUpdate.Document,
                expectedUpdatedUnplacedObjectCount,
                $"reloaded updated real asset placement snapshot should preserve unplaced-object counts");
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded updated real asset placement snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real asset placement smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset placement smoke should reload undone snapshot data for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterUndo.Document,
                recordIndex,
                propertyName,
                originalValue,
                expectedObjectTitle,
                initialSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: true,
                $"reloaded undone real asset placement snapshot should preserve {propertyName}");
            AssertRealAssetUnplacedObjectCount(
                reloadedAfterUndo.Document,
                expectedOriginalUnplacedObjectCount,
                $"reloaded undone real asset placement snapshot should preserve unplaced-object counts");
            Expect(!reloadedAfterUndo.Document.CommandUndoAvailable,
                $"real asset placement smoke should clear undo after restoring {propertyName} for {sourcePath}");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeRealAssetHostBackedDuplicateRoundTrip(
        string? sourcePath,
        int recordIndex,
        string duplicateUniqueId,
        string expectedSourceObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalSectionObjectCount,
        int expectedUpdatedSectionObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset duplicate candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDuplicates-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset duplicate smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            var sourceObject = loaded.Document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
            Expect(sourceObject is not null,
                $"real asset duplicate smoke should expose source object {recordIndex} for {sourcePath}");
            if (sourceObject is null)
            {
                return;
            }

            var sourceUniqueId = TryGetSnapshotObjectPropertyValue(sourceObject, "UNIQUEID");
            Expect(!string.IsNullOrWhiteSpace(sourceUniqueId),
                $"real asset duplicate smoke should expose source UNIQUEID for {sourcePath}");
            if (string.IsNullOrWhiteSpace(sourceUniqueId))
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                "UNIQUEID",
                sourceUniqueId!,
                expectedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"initial real asset duplicate snapshot should preserve source object identity");
            AssertRealAssetSectionObjectCount(
                loaded.Document,
                expectedSectionTitle,
                expectedOriginalSectionObjectCount,
                $"initial real asset duplicate snapshot should preserve section object counts");

            var duplicateResult = CopperfinStudioSnapshotClient.TryDuplicateObject(
                assetPath,
                recordIndex,
                sourceUniqueId,
                duplicateUniqueId);
            Expect(duplicateResult.Success && duplicateResult.Document is not null,
                $"real asset duplicate smoke should duplicate record {recordIndex} for {sourcePath}");
            if (!duplicateResult.Success || duplicateResult.Document is null)
            {
                return;
            }

            var duplicatedObject = FindSnapshotObjectByUniqueId(duplicateResult.Document, duplicateUniqueId);
            Expect(duplicatedObject is not null,
                $"real asset duplicate smoke should surface the duplicated UNIQUEID for {sourcePath}");
            if (duplicatedObject is null)
            {
                return;
            }

            Expect(duplicatedObject.RecordIndex != recordIndex,
                $"real asset duplicate smoke should assign a distinct record to the duplicated object for {sourcePath}");
            AssertRealAssetRoundTripSnapshot(
                duplicateResult.Document,
                duplicatedObject.RecordIndex,
                "UNIQUEID",
                duplicateUniqueId,
                expectedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"updated real asset duplicate snapshot should preserve duplicated object identity");
            AssertRealAssetSectionObjectCount(
                duplicateResult.Document,
                expectedSectionTitle,
                expectedUpdatedSectionObjectCount,
                $"updated real asset duplicate snapshot should preserve section object counts");

            var reloadedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null,
                $"real asset duplicate smoke should reload duplicated snapshot data for {sourcePath}");
            if (!reloadedAfterDuplicate.Success || reloadedAfterDuplicate.Document is null)
            {
                return;
            }

            var reloadedDuplicatedObject = FindSnapshotObjectByUniqueId(reloadedAfterDuplicate.Document, duplicateUniqueId);
            Expect(reloadedDuplicatedObject is not null,
                $"reloaded real asset duplicate snapshot should preserve the duplicated UNIQUEID for {sourcePath}");
            if (reloadedDuplicatedObject is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterDuplicate.Document,
                reloadedDuplicatedObject.RecordIndex,
                "UNIQUEID",
                duplicateUniqueId,
                expectedSourceObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded real asset duplicate snapshot should preserve duplicated object identity");
            AssertRealAssetSectionObjectCount(
                reloadedAfterDuplicate.Document,
                expectedSectionTitle,
                expectedUpdatedSectionObjectCount,
                $"reloaded real asset duplicate snapshot should preserve section object counts");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeRealAssetHostBackedDeleteRestoreRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalSectionObjectCount,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset delete candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletes-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var loaded = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(loaded.Success && loaded.Document is not null,
                $"real asset delete smoke should load snapshot data for {sourcePath}");
            if (!loaded.Success || loaded.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                loaded.Document,
                recordIndex,
                "UNIQUEID",
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"initial real asset delete snapshot should preserve source object identity");
            AssertRealAssetSectionObjectCount(
                loaded.Document,
                expectedSectionTitle,
                expectedOriginalSectionObjectCount,
                $"initial real asset delete snapshot should preserve section object counts");

            var deleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(deleteResult.Success && deleteResult.Document is not null,
                $"real asset delete smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                deleteResult.Document,
                recordIndex,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"updated real asset delete snapshot should preserve deleted section-member continuity");

            var reloadedAfterDelete = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDelete.Success && reloadedAfterDelete.Document is not null,
                $"real asset delete smoke should reload deleted snapshot data for {sourcePath}");
            if (!reloadedAfterDelete.Success || reloadedAfterDelete.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterDelete.Document,
                recordIndex,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"reloaded real asset delete snapshot should preserve deleted section-member continuity");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real asset delete smoke should restore record {recordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                restoreResult.Document,
                recordIndex,
                "UNIQUEID",
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"restored real asset delete snapshot should preserve source object identity");
            AssertRealAssetSectionObjectCount(
                restoreResult.Document,
                expectedSectionTitle,
                expectedOriginalSectionObjectCount,
                $"restored real asset delete snapshot should preserve section object counts");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real asset delete smoke should reload restored snapshot data for {sourcePath}");
            if (!reloadedAfterRestore.Success || reloadedAfterRestore.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterRestore.Document,
                recordIndex,
                "UNIQUEID",
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"reloaded restored real asset delete snapshot should preserve source object identity");
            AssertRealAssetSectionObjectCount(
                reloadedAfterRestore.Document,
                expectedSectionTitle,
                expectedOriginalSectionObjectCount,
                $"reloaded restored real asset delete snapshot should preserve section object counts");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeRealAssetHostBackedDeletedPropertyRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string propertyName,
        string originalRawValue,
        string updatedRawValue,
        int expectedOriginalLayoutValue,
        int expectedUpdatedLayoutValue,
        int expectedSectionCount,
        bool expectLabel,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted property candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletedWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var deleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(deleteResult.Success && deleteResult.Document is not null,
                $"real deleted property smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectPropertySnapshot(
                deleteResult.Document,
                recordIndex,
                propertyName,
                originalRawValue,
                expectedOriginalLayoutValue,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"initial deleted real asset snapshot should preserve {propertyName}");

            var updateResult = CopperfinStudioSnapshotClient.TryUpdateProperty(
                assetPath,
                recordIndex,
                propertyName,
                updatedRawValue);
            Expect(updateResult.Success && updateResult.Document is not null,
                $"real deleted property smoke should update {propertyName} for deleted record {recordIndex} in {sourcePath}");
            if (!updateResult.Success || updateResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectPropertySnapshot(
                updateResult.Document,
                recordIndex,
                propertyName,
                updatedRawValue,
                expectedUpdatedLayoutValue,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"updated deleted real asset snapshot should preserve {propertyName}");
            Expect(updateResult.Document.CommandUndoAvailable,
                $"real deleted property smoke should expose undo after updating {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real deleted property smoke should reload updated deleted snapshot data for {sourcePath}");
            if (!reloadedAfterUpdate.Success || reloadedAfterUpdate.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectPropertySnapshot(
                reloadedAfterUpdate.Document,
                recordIndex,
                propertyName,
                updatedRawValue,
                expectedUpdatedLayoutValue,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"reloaded updated deleted real asset snapshot should preserve {propertyName}");
            Expect(reloadedAfterUpdate.Document.CommandUndoAvailable,
                $"reloaded deleted property snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real deleted property smoke should undo {propertyName} for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectPropertySnapshot(
                undoResult.Document,
                recordIndex,
                propertyName,
                originalRawValue,
                expectedOriginalLayoutValue,
                expectedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                $"undone deleted real asset snapshot should preserve {propertyName}");
            Expect(!undoResult.Document.CommandUndoAvailable,
                $"undone deleted property snapshot should clear command undo for {sourcePath}");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                recordIndex,
                expectedUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real deleted property smoke should restore record {recordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                restoreResult.Document,
                recordIndex,
                propertyName,
                originalRawValue,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                $"restored deleted-property real asset snapshot should preserve {propertyName}");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeRealAssetHostBackedDeletedRenameRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedOriginalUniqueId,
        string expectedRenamedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted rename candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletedRenames-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var deleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId);
            Expect(deleteResult.Success && deleteResult.Document is not null,
                $"real deleted rename smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                deleteResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "initial deleted real asset rename snapshot should preserve original identity");

            var renameResult = CopperfinStudioSnapshotClient.TryRenameObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId,
                expectedRenamedUniqueId);
            Expect(renameResult.Success && renameResult.Document is not null,
                $"real deleted rename smoke should rename record {recordIndex} for {sourcePath}");
            if (!renameResult.Success || renameResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                renameResult.Document,
                recordIndex,
                expectedRenamedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "renamed deleted real asset snapshot should preserve renamed identity");
            Expect(renameResult.Document.CommandUndoAvailable,
                $"real deleted rename smoke should expose undo after renaming {recordIndex} for {sourcePath}");

            var reloadedAfterRename = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRename.Success && reloadedAfterRename.Document is not null,
                $"real deleted rename smoke should reload renamed deleted snapshot data for {sourcePath}");
            if (!reloadedAfterRename.Success || reloadedAfterRename.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterRename.Document,
                recordIndex,
                expectedRenamedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "reloaded renamed deleted real asset snapshot should preserve renamed identity");
            Expect(reloadedAfterRename.Document.CommandUndoAvailable,
                $"reloaded deleted rename snapshot should keep undo available for {sourcePath}");

            var undoResult = CopperfinStudioSnapshotClient.TryUndoCommand(assetPath);
            Expect(undoResult.Success && undoResult.Document is not null,
                $"real deleted rename smoke should undo rename for {sourcePath}");
            if (!undoResult.Success || undoResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                undoResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "undone deleted real asset rename snapshot should preserve original identity");
            Expect(!undoResult.Document.CommandUndoAvailable,
                $"undone deleted rename snapshot should clear command undo for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real deleted rename smoke should reload restored deleted identity for {sourcePath}");
            if (!reloadedAfterUndo.Success || reloadedAfterUndo.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterUndo.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedDeletedSectionVisibleObjectCount,
                "reloaded undone deleted real asset rename snapshot should preserve original identity");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real deleted rename smoke should restore record {recordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                restoreResult.Document,
                recordIndex,
                "UNIQUEID",
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                "restored deleted-rename real asset snapshot should preserve original identity");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real deleted rename smoke should reload restored live snapshot data for {sourcePath}");
            if (!reloadedAfterRestore.Success || reloadedAfterRestore.Document is null)
            {
                return;
            }

            AssertRealAssetRoundTripSnapshot(
                reloadedAfterRestore.Document,
                recordIndex,
                "UNIQUEID",
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                "reloaded restored deleted-rename real asset snapshot should preserve original identity");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeRealAssetHostBackedDeletedDuplicateRoundTrip(
        string? sourcePath,
        int recordIndex,
        string expectedOriginalUniqueId,
        string expectedDuplicatedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real deleted duplicate candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetDeletedDuplicates-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            var deleteResult = CopperfinStudioSnapshotClient.TryDeleteObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId);
            Expect(deleteResult.Success && deleteResult.Document is not null,
                $"real deleted duplicate smoke should delete record {recordIndex} for {sourcePath}");
            if (!deleteResult.Success || deleteResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                deleteResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 1,
                "initial deleted real asset duplicate snapshot should preserve original identity");

            var duplicateResult = CopperfinStudioSnapshotClient.TryDuplicateObject(
                assetPath,
                recordIndex,
                expectedOriginalUniqueId,
                expectedDuplicatedUniqueId);
            Expect(duplicateResult.Success && duplicateResult.Document is not null,
                $"real deleted duplicate smoke should duplicate deleted record {recordIndex} for {sourcePath}");
            if (!duplicateResult.Success || duplicateResult.Document is null)
            {
                return;
            }

            var duplicatedObject = FindSnapshotObjectByUniqueId(duplicateResult.Document, expectedDuplicatedUniqueId);
            Expect(duplicatedObject is not null,
                $"real deleted duplicate smoke should surface duplicated deleted UNIQUEID {expectedDuplicatedUniqueId} for {sourcePath}");
            if (duplicatedObject is null)
            {
                return;
            }

            Expect(duplicatedObject.RecordIndex != recordIndex,
                $"real deleted duplicate smoke should assign a distinct record to the duplicated deleted row for {sourcePath}");

            AssertRealAssetDeletedObjectSnapshot(
                duplicateResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                expectedDeletedSectionObjectCount: 2,
                "duplicated deleted real asset snapshot should preserve original deleted identity");
            AssertRealAssetDeletedObjectSnapshot(
                duplicateResult.Document,
                duplicatedObject.RecordIndex,
                expectedDuplicatedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                expectedDeletedSectionObjectCount: 2,
                "duplicated deleted real asset snapshot should preserve duplicated deleted identity");
            Expect(!duplicateResult.Document.CommandUndoAvailable,
                $"real deleted duplicate smoke should not expose command undo after duplicating a deleted row for {sourcePath}");

            var reloadedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null,
                $"real deleted duplicate smoke should reload duplicated deleted snapshot data for {sourcePath}");
            if (!reloadedAfterDuplicate.Success || reloadedAfterDuplicate.Document is null)
            {
                return;
            }

            var reloadedDuplicatedObject = FindSnapshotObjectByUniqueId(reloadedAfterDuplicate.Document, expectedDuplicatedUniqueId);
            Expect(reloadedDuplicatedObject is not null,
                $"reloaded deleted duplicate snapshot should preserve duplicated UNIQUEID {expectedDuplicatedUniqueId} for {sourcePath}");
            if (reloadedDuplicatedObject is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterDuplicate.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                expectedDeletedSectionObjectCount: 2,
                "reloaded duplicated deleted real asset snapshot should preserve original deleted identity");
            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterDuplicate.Document,
                reloadedDuplicatedObject.RecordIndex,
                expectedDuplicatedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                expectedDeletedSectionObjectCount: 2,
                "reloaded duplicated deleted real asset snapshot should preserve duplicated deleted identity");
            Expect(!reloadedAfterDuplicate.Document.CommandUndoAvailable,
                $"reloaded deleted duplicate snapshot should not expose command undo for {sourcePath}");

            var restoreResult = CopperfinStudioSnapshotClient.TryRestoreObject(
                assetPath,
                duplicatedObject.RecordIndex,
                expectedDuplicatedUniqueId);
            Expect(restoreResult.Success && restoreResult.Document is not null,
                $"real deleted duplicate smoke should restore duplicated record {duplicatedObject.RecordIndex} for {sourcePath}");
            if (!restoreResult.Success || restoreResult.Document is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                restoreResult.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                "restored deleted duplicate snapshot should preserve the original deleted source row");
            AssertRealAssetRoundTripSnapshot(
                restoreResult.Document,
                duplicatedObject.RecordIndex,
                "UNIQUEID",
                expectedDuplicatedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                "restored deleted duplicate snapshot should preserve the restored duplicated identity");
            AssertRealAssetSectionObjectCount(
                restoreResult.Document,
                expectedSectionTitle,
                expectedObjectCount: 1,
                "restored deleted duplicate snapshot should preserve section live object counts");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real deleted duplicate smoke should reload restored live snapshot data for {sourcePath}");
            if (!reloadedAfterRestore.Success || reloadedAfterRestore.Document is null)
            {
                return;
            }

            var reloadedRestoredObject = FindSnapshotObjectByUniqueId(reloadedAfterRestore.Document, expectedDuplicatedUniqueId);
            Expect(reloadedRestoredObject is not null,
                $"reloaded restored deleted duplicate snapshot should preserve duplicated UNIQUEID {expectedDuplicatedUniqueId} for {sourcePath}");
            if (reloadedRestoredObject is null)
            {
                return;
            }

            AssertRealAssetDeletedObjectSnapshot(
                reloadedAfterRestore.Document,
                recordIndex,
                expectedOriginalUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionRecordIndex,
                expectedSectionCount,
                expectLabel,
                expectedVisibleSectionObjectCount: 2,
                "reloaded restored deleted duplicate snapshot should preserve the original deleted source row");
            AssertRealAssetRoundTripSnapshot(
                reloadedAfterRestore.Document,
                reloadedRestoredObject.RecordIndex,
                "UNIQUEID",
                expectedDuplicatedUniqueId,
                expectedObjectTitle,
                expectedSectionTitle,
                expectedSectionCount,
                expectLabel,
                expectUnplacedObject: false,
                "reloaded restored deleted duplicate snapshot should preserve the restored duplicated identity");
            AssertRealAssetSectionObjectCount(
                reloadedAfterRestore.Document,
                expectedSectionTitle,
                expectedObjectCount: 1,
                "reloaded restored deleted duplicate snapshot should preserve section live object counts");
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorPropertyGridRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        string propertyName,
        object updatedPropertyValue,
        string? expectedOriginalSelectionValue,
        string expectedUpdatedSelectionValue,
        string expectedUpdatedRawValue,
        string expectedOriginalRawValue,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        bool expectUnplacedObject = false)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor write candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = expectUnplacedObject
                ? FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real unplaced asset smoke.")
                : null;

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = string.Equals(item.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase);
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor smoke should start from an object-rooted property-grid selection for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                return;
            }

            var initialSelectionValue = TypeDescriptor.GetProperties(objectSelection)[propertyName]?.GetValue(objectSelection)?.ToString() ?? string.Empty;
            var expectedUndoSelectionValue = expectedOriginalSelectionValue ?? initialSelectionValue;
            if (expectedOriginalSelectionValue is not null)
            {
                Expect(string.Equals(initialSelectionValue, expectedOriginalSelectionValue, StringComparison.Ordinal),
                    $"real asset editor smoke should expose original property-grid value {propertyName} for {sourcePath}");
            }

            TypeDescriptor.GetProperties(objectSelection)[propertyName]?.SetValue(objectSelection, updatedPropertyValue);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, 0);
            Application.DoEvents();

            var updatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedObject?.RecordIndex == recordIndex &&
                           (!expectUnplacedObject ||
                            (string.Equals(ReadPrivateStringField(surface!, "assetFamily"), "report", StringComparison.Ordinal) &&
                             ReadPrivateNullableInt(surface!, "selectedRecordIndex") == recordIndex &&
                             ReadPrivateNullableInt(surface!, "selectedReportSectionRecordIndex") is null &&
                             ReadPrivateBoolField(surface!, "unplacedReportObjectsSelected"))) &&
                           string.Equals(propertyValue, expectedUpdatedSelectionValue, StringComparison.Ordinal);
                });
            Expect(updatedSelection,
                $"real asset editor smoke should preserve section/object continuity after editing {propertyName} for {sourcePath}");

            Expect(control.CanHandleUndoCommand(),
                $"real asset editor smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    propertyName,
                    expectedUpdatedRawValue,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject,
                    $"reloaded edited real asset snapshot should preserve {propertyName}");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor smoke should execute undo after editing {propertyName} for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedObject?.RecordIndex == recordIndex &&
                           (!expectUnplacedObject ||
                            (string.Equals(ReadPrivateStringField(surface!, "assetFamily"), "report", StringComparison.Ordinal) &&
                             ReadPrivateNullableInt(surface!, "selectedRecordIndex") == recordIndex &&
                             ReadPrivateNullableInt(surface!, "selectedReportSectionRecordIndex") is null &&
                             ReadPrivateBoolField(surface!, "unplacedReportObjectsSelected"))) &&
                           string.Equals(propertyValue, expectedUndoSelectionValue, StringComparison.Ordinal);
                });
            Expect(undoneSelection,
                $"real asset editor smoke should preserve section/object continuity after undoing {propertyName} for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor smoke should clear undo after restoring {propertyName} for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    propertyName,
                    expectedOriginalRawValue,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject,
                    $"reloaded undone editor real asset snapshot should preserve {propertyName}");
            }

            TearDownForm(hostForm);
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeleteRestoreRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedUniqueId,
        int expectedOriginalSectionObjectCount,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor delete candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletes-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real delete smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor delete smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == expectedSectionRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor delete smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   objectListView.Items.Count == expectedOriginalSectionObjectCount &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor delete smoke should start from a live object selection with only delete exposed for {sourcePath}");

            deleteButton.PerformClick();
            Application.DoEvents();

            var deletedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(deletedSelection,
                $"real asset editor delete smoke should preserve deleted object continuity inside the containing section for {sourcePath}");

            var reloadedAfterDelete = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDelete.Success && reloadedAfterDelete.Document is not null,
                $"real asset editor delete smoke should reload deleted on-disk state for {sourcePath}");
            if (reloadedAfterDelete.Success && reloadedAfterDelete.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterDelete.Document,
                    recordIndex,
                    expectedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    $"reloaded real asset editor delete snapshot should preserve deleted section-member continuity");
            }

            restoreButton.PerformClick();
            Application.DoEvents();

            var restoredSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 0 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedOriginalSectionObjectCount &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(restoredSelection,
                $"real asset editor delete smoke should preserve live object continuity after restore for {sourcePath}");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real asset editor delete smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterRestore.Document,
                    recordIndex,
                    "UNIQUEID",
                    expectedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded restored real asset editor delete snapshot should preserve source object identity");
                AssertRealAssetSectionObjectCount(
                    reloadedAfterRestore.Document,
                    expectedSectionTitle,
                    expectedOriginalSectionObjectCount,
                    $"reloaded restored real asset editor delete snapshot should preserve section object counts");
            }

            TearDownForm(hostForm);
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedPropertyRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedUniqueId,
        string propertyName,
        object updatedPropertyValue,
        string expectedUpdatedSelectionValue,
        string expectedOriginalRawValue,
        string expectedUpdatedRawValue,
        int expectedOriginalLayoutValue,
        int expectedUpdatedLayoutValue,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor deleted property candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletedWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real deleted-property smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor deleted-property smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == expectedSectionRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor deleted-property smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor deleted-property smoke should start from a live object selection with delete exposed for {sourcePath}");

            deleteButton.PerformClick();
            Application.DoEvents();

            var deletedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(deletedSelection,
                $"real asset editor deleted-property smoke should preserve deleted selection continuity before editing {propertyName} for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection deletedObjectSelection)
            {
                return;
            }

            var deletedSelectionValue = TypeDescriptor.GetProperties(deletedObjectSelection)[propertyName]?.GetValue(deletedObjectSelection)?.ToString() ?? string.Empty;
            var selectionProperties = TypeDescriptor.GetProperties(deletedObjectSelection);
            selectionProperties[propertyName]?.SetValue(deletedObjectSelection, updatedPropertyValue);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, 0);
            Application.DoEvents();

            var updatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           string.Equals(propertyValue, expectedUpdatedSelectionValue, StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(updatedSelection,
                $"real asset editor deleted-property smoke should preserve deleted section/object continuity after editing {propertyName} for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-property smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor deleted-property smoke should reload updated deleted on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetDeletedObjectPropertySnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    propertyName,
                    expectedUpdatedRawValue,
                    expectedUpdatedLayoutValue,
                    expectedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    $"reloaded real asset editor deleted-property snapshot should preserve {propertyName}");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor deleted-property smoke should execute undo after editing {propertyName} for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           string.Equals(propertyValue, deletedSelectionValue, StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor deleted-property smoke should preserve deleted section/object continuity after undoing {propertyName} for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor deleted-property smoke should clear undo after restoring {propertyName} for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor deleted-property smoke should reload restored deleted on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetDeletedObjectPropertySnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    propertyName,
                    expectedOriginalRawValue,
                    expectedOriginalLayoutValue,
                    expectedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    $"reloaded undone real asset editor deleted-property snapshot should preserve {propertyName}");
            }

            restoreButton.PerformClick();
            Application.DoEvents();

            var restoredSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 0 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           string.Equals(propertyValue, deletedSelectionValue, StringComparison.Ordinal) &&
                           objectListView.Items.Count == 1 &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(restoredSelection,
                $"real asset editor deleted-property smoke should preserve live continuity after restoring the deleted row for {sourcePath}");

            TearDownForm(hostForm);
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedRenameCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedOriginalUniqueId,
        string expectedRenamedUniqueId,
        int expectedDeletedSectionVisibleObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor deleted rename candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletedRenames-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var previousRenameUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID");

        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", expectedRenamedUniqueId);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var renameButton = GetPrivateButton(control, "renameObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real deleted-rename smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor deleted-rename smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == expectedSectionRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor deleted-rename smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor deleted-rename smoke should start from a live object selection with rename, duplicate, and delete exposed for {sourcePath}");

            deleteButton.PerformClick();
            Application.DoEvents();

            var deletedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedOriginalUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(deletedSelection,
                $"real asset editor deleted-rename smoke should preserve deleted selection continuity before renaming for {sourcePath}");

            renameButton.PerformClick();
            Application.DoEvents();

            var renamedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedRenamedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(renamedSelection,
                $"real asset editor deleted-rename smoke should preserve deleted section/object continuity after renaming for {sourcePath}");
            Expect(control.CanHandleUndoCommand(),
                $"real asset editor deleted-rename smoke should expose undo after renaming for {sourcePath}");

            var reloadedAfterRename = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRename.Success && reloadedAfterRename.Document is not null,
                $"real asset editor deleted-rename smoke should reload renamed deleted on-disk state for {sourcePath}");
            if (reloadedAfterRename.Success && reloadedAfterRename.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterRename.Document,
                    recordIndex,
                    expectedRenamedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    "reloaded real asset editor deleted-rename snapshot should preserve renamed identity");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor deleted-rename smoke should execute undo after renaming for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedOriginalUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == expectedDeletedSectionVisibleObjectCount &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(undoneSelection,
                $"real asset editor deleted-rename smoke should preserve deleted section/object continuity after undoing rename for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor deleted-rename smoke should clear undo after restoring original identity for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor deleted-rename smoke should reload restored deleted identity for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedDeletedSectionVisibleObjectCount,
                    "reloaded undone real asset editor deleted-rename snapshot should preserve original identity");
            }

            restoreButton.PerformClick();
            Application.DoEvents();

            var restoredSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 0 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedOriginalUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           objectListView.Items.Count == 1 &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(restoredSelection,
                $"real asset editor deleted-rename smoke should preserve live continuity after restoring the deleted row for {sourcePath}");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real asset editor deleted-rename smoke should reload restored live state for {sourcePath}");
            if (reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterRestore.Document,
                    recordIndex,
                    "UNIQUEID",
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded restored real asset editor deleted-rename snapshot should preserve original identity");
            }

            TearDownForm(hostForm);
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_RENAME_OBJECT_UNIQUE_ID", previousRenameUniqueId);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDeletedDuplicateCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        string expectedOriginalUniqueId,
        string expectedDuplicatedUniqueId)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor deleted duplicate candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDeletedDuplicates-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var previousDuplicateUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID");

        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", expectedDuplicatedUniqueId);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var renameButton = GetPrivateButton(control, "renameObjectButton");
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real deleted-duplicate smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor deleted-duplicate smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == expectedSectionRecordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor deleted-duplicate smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(renameButton.Visible &&
                   renameButton.Enabled &&
                   duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   objectListView.Items.Count == 1 &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor deleted-duplicate smoke should start from a live object selection with rename, duplicate, and delete exposed for {sourcePath}");

            deleteButton.PerformClick();
            Application.DoEvents();

            var deletedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == recordIndex &&
                           selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedOriginalUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                           objectListView.Items.Count == 1 &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           !deleteButton.Visible &&
                           restoreButton.Visible &&
                           restoreButton.Enabled &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(deletedSelection,
                $"real asset editor deleted-duplicate smoke should preserve deleted selection continuity before duplicating for {sourcePath}");

            duplicateButton.PerformClick();
            Application.DoEvents();

            int duplicatedRecordIndex = -1;
            var duplicatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex == recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    var selectedUniqueId = selectedObject is null
                        ? null
                        : TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID");
                    if (string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                        selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                        selectedSectionModel?.DeletedObjectCount == 2 &&
                        selectedObject?.RecordIndex == refreshedSelection.RecordIndex &&
                        selectedObject.Deleted &&
                        string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                        string.Equals(selectedUniqueId, expectedDuplicatedUniqueId, StringComparison.Ordinal) &&
                        string.Equals(objectState, "Deleted", StringComparison.Ordinal) &&
                        objectListView.Items.Count == 2 &&
                        renameButton.Visible &&
                        renameButton.Enabled &&
                        duplicateButton.Visible &&
                        duplicateButton.Enabled &&
                        !deleteButton.Visible &&
                        restoreButton.Visible &&
                        restoreButton.Enabled &&
                        string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") == refreshedSelection.RecordIndex &&
                        ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                        !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))
                    {
                        duplicatedRecordIndex = refreshedSelection.RecordIndex;
                        return true;
                    }

                    return false;
                });
            Expect(duplicatedSelection,
                $"real asset editor deleted-duplicate smoke should preserve deleted section/object continuity after duplicating for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor deleted-duplicate smoke should not expose undo after duplicating a deleted row for {sourcePath}");
            if (!duplicatedSelection)
            {
                return;
            }

            var reloadedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null,
                $"real asset editor deleted-duplicate smoke should reload duplicated deleted on-disk state for {sourcePath}");
            if (reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterDuplicate.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    expectedDeletedSectionObjectCount: 2,
                    "reloaded real asset editor deleted-duplicate snapshot should preserve original deleted identity");
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterDuplicate.Document,
                    duplicatedRecordIndex,
                    expectedDuplicatedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    expectedDeletedSectionObjectCount: 2,
                    "reloaded real asset editor deleted-duplicate snapshot should preserve duplicated deleted identity");
            }

            restoreButton.PerformClick();
            Application.DoEvents();

            var restoredSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != duplicatedRecordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var objectState = TypeDescriptor.GetProperties(refreshedSelection)["OBJECTSTATE"]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == expectedSectionRecordIndex &&
                           selectedSectionModel?.DeletedObjectCount == 1 &&
                           selectedObject?.RecordIndex == duplicatedRecordIndex &&
                           !selectedObject.Deleted &&
                           string.Equals(selectedObject.Title, expectedObjectTitle, StringComparison.Ordinal) &&
                           string.Equals(TryGetSnapshotObjectPropertyValue(selectedObject, "UNIQUEID"), expectedDuplicatedUniqueId, StringComparison.Ordinal) &&
                           string.Equals(objectState, "Live", StringComparison.Ordinal) &&
                           objectListView.Items.Count == 2 &&
                           renameButton.Visible &&
                           renameButton.Enabled &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == duplicatedRecordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == expectedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(restoredSelection,
                $"real asset editor deleted-duplicate smoke should preserve live continuity after restoring the duplicated row for {sourcePath}");

            var reloadedAfterRestore = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null,
                $"real asset editor deleted-duplicate smoke should reload restored live state for {sourcePath}");
            if (reloadedAfterRestore.Success && reloadedAfterRestore.Document is not null)
            {
                AssertRealAssetDeletedObjectSnapshot(
                    reloadedAfterRestore.Document,
                    recordIndex,
                    expectedOriginalUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionRecordIndex,
                    expectedSectionCount,
                    expectLabel,
                    expectedVisibleSectionObjectCount: 2,
                    "reloaded restored real asset editor deleted-duplicate snapshot should preserve the original deleted source row");
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterRestore.Document,
                    duplicatedRecordIndex,
                    "UNIQUEID",
                    expectedDuplicatedUniqueId,
                    expectedObjectTitle,
                    expectedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    "reloaded restored real asset editor deleted-duplicate snapshot should preserve duplicated live identity");
            }

            TearDownForm(hostForm);
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", previousDuplicateUniqueId);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorSectionRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        string propertyName,
        object updatedPropertyValue,
        string expectedOriginalSelectionValue,
        string expectedUpdatedSelectionValue,
        string expectedOriginalRawValue,
        string expectedUpdatedRawValue,
        int expectedOriginalLayoutValue,
        int expectedUpdatedLayoutValue,
        int expectedSectionCount,
        bool expectLabel,
        int expectedObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor section candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorSections-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared design surface for the real section asset smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor section smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioReportSection section &&
                                section.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor section smoke should start from a section-rooted property-grid selection for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection sectionSelection)
            {
                return;
            }

            var initialSelectionValue = TypeDescriptor.GetProperties(sectionSelection)[propertyName]?.GetValue(sectionSelection)?.ToString();
            Expect(string.Equals(initialSelectionValue, expectedOriginalSelectionValue, StringComparison.Ordinal),
                $"real asset editor section smoke should expose original property-grid value {propertyName} for {sourcePath}");

            TypeDescriptor.GetProperties(sectionSelection)[propertyName]?.SetValue(sectionSelection, updatedPropertyValue);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, 0);
            Application.DoEvents();

            var updatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == recordIndex &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") &&
                           objectListView.SelectedItems.Count == 0 &&
                           string.Equals(propertyValue, expectedUpdatedSelectionValue, StringComparison.Ordinal);
                });
            Expect(updatedSelection,
                $"real asset editor section smoke should preserve section-rooted continuity after editing {propertyName} for {sourcePath}");

            Expect(control.CanHandleUndoCommand(),
                $"real asset editor section smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor section smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetSectionSnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    expectedSectionTitle,
                    propertyName,
                    expectedUpdatedRawValue,
                    expectedUpdatedLayoutValue,
                    expectedSectionCount,
                    expectLabel,
                    expectedObjectCount,
                    $"reloaded edited real asset section snapshot should preserve {propertyName}");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor section smoke should execute undo after editing {propertyName} for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault();
                    var selectedSectionModel = selectedSection?.Tag as CopperfinStudioReportSection;
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection?.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedSectionModel?.RecordIndex == recordIndex &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") is null &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") &&
                           objectListView.SelectedItems.Count == 0 &&
                           string.Equals(propertyValue, expectedOriginalSelectionValue, StringComparison.Ordinal);
                });
            Expect(undoneSelection,
                $"real asset editor section smoke should preserve section-rooted continuity after undoing {propertyName} for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor section smoke should clear undo after restoring {propertyName} for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor section smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetSectionSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    expectedSectionTitle,
                    propertyName,
                    expectedOriginalRawValue,
                    expectedOriginalLayoutValue,
                    expectedSectionCount,
                    expectLabel,
                    expectedObjectCount,
                    $"reloaded undone editor real asset section snapshot should preserve {propertyName}");
            }

            TearDownForm(hostForm);
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorBatchPropertyRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string expectedSectionTitle,
        IReadOnlyList<KeyValuePair<string, string>> propertyChanges,
        IReadOnlyList<KeyValuePair<string, string>> expectedUpdatedSelectionValues,
        IReadOnlyList<KeyValuePair<string, string>> expectedOriginalRawValues,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor batch candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorBatchWrites-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real batch smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor batch smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = string.Equals(item.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase);
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor batch smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor batch smoke should start from an object-rooted property-grid selection for {sourcePath}");

            InvokeAssetEditorVoid(control, "ApplyVisualPropertyChanges", recordIndex, propertyChanges);
            Application.DoEvents();

            var updatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    if (!string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) ||
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") != recordIndex ||
                        ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))
                    {
                        return false;
                    }

                    foreach (var property in expectedUpdatedSelectionValues)
                    {
                        var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[property.Key]?.GetValue(refreshedSelection)?.ToString();
                        if (!string.Equals(propertyValue, property.Value, StringComparison.Ordinal))
                        {
                            return false;
                        }
                    }

                    return true;
                });
            Expect(updatedSelection,
                $"real asset editor batch smoke should preserve section/object continuity after applying the batch update for {sourcePath}");

            Expect(control.CanHandleUndoCommand(),
                $"real asset editor batch smoke should expose undo after applying the batch update for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor batch smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                foreach (var property in propertyChanges)
                {
                    AssertRealAssetRoundTripSnapshot(
                        reloadedAfterUpdate.Document,
                        recordIndex,
                        property.Key,
                        property.Value,
                        expectedObjectTitle,
                        expectedSectionTitle,
                        expectedSectionCount,
                        expectLabel,
                        expectUnplacedObject: false,
                        $"reloaded edited real asset batch snapshot should preserve {property.Key}");
                }
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor batch smoke should execute one command undo after applying the batch update for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    if (!string.Equals(selectedSection, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) ||
                        selectedObject?.RecordIndex != recordIndex ||
                        !string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) ||
                        ReadPrivateNullableInt(surface, "selectedRecordIndex") != recordIndex ||
                        ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"))
                    {
                        return false;
                    }

                    return true;
                });
            Expect(undoneSelection,
                $"real asset editor batch smoke should preserve section/object continuity after the command undo for {sourcePath}");

            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor batch smoke should clear undo after restoring the batch update for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor batch smoke should reload restored on-disk state after the command undo for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                foreach (var property in expectedOriginalRawValues)
                {
                    AssertRealAssetRoundTripSnapshot(
                        reloadedAfterUndo.Document,
                        recordIndex,
                        property.Key,
                        property.Value,
                        expectedObjectTitle,
                        expectedSectionTitle,
                        expectedSectionCount,
                        expectLabel,
                        expectUnplacedObject: false,
                        $"reloaded undone editor real asset batch snapshot should preserve {property.Key}");
                }
            }

            TearDownForm(hostForm);
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorPlacementRoundTripWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string initialSectionTitle,
        string updatedSectionTitle,
        int updatedSectionRecordIndex,
        string propertyName,
        object updatedPropertyValue,
        string? expectedOriginalSelectionValue,
        string expectedUpdatedSelectionValue,
        string expectedUpdatedRawValue,
        string expectedOriginalRawValue,
        string expectedObjectTitle,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalUnplacedObjectCount,
        int expectedUpdatedUnplacedObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor placement candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorPlacements-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);

        try
        {
            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report design surface for the real placement smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor placement smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = string.Equals(item.Text, initialSectionTitle, StringComparison.OrdinalIgnoreCase);
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor placement smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor placement smoke should start from an object-rooted property-grid selection for {sourcePath}");

            if (propertyGrid.SelectedObject is not CopperfinDesignerSelection objectSelection)
            {
                return;
            }

            var initialSelectionValue = TypeDescriptor.GetProperties(objectSelection)[propertyName]?.GetValue(objectSelection)?.ToString() ?? string.Empty;
            var expectedUndoSelectionValue = expectedOriginalSelectionValue ?? initialSelectionValue;
            if (expectedOriginalSelectionValue is not null)
            {
                Expect(string.Equals(initialSelectionValue, expectedOriginalSelectionValue, StringComparison.Ordinal),
                    $"real asset editor placement smoke should expose original property-grid value {propertyName} for {sourcePath}");
            }

            TypeDescriptor.GetProperties(objectSelection)[propertyName]?.SetValue(objectSelection, updatedPropertyValue);
            InvokeAssetEditorVoid(control, "ApplyPropertyGridChange", propertyName, 0);
            Application.DoEvents();

            var updatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection, updatedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedObject?.RecordIndex == recordIndex &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == updatedSectionRecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") &&
                           HasLabelTextContaining(control, $"Unplaced objects: {expectedUpdatedUnplacedObjectCount}") &&
                           string.Equals(propertyValue, expectedUpdatedSelectionValue, StringComparison.Ordinal);
                });
            Expect(updatedSelection,
                $"real asset editor placement smoke should preserve section/object continuity after editing {propertyName} for {sourcePath}");

            Expect(control.CanHandleUndoCommand(),
                $"real asset editor placement smoke should expose undo after editing {propertyName} for {sourcePath}");

            var reloadedAfterUpdate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null,
                $"real asset editor placement smoke should reload updated on-disk state for {sourcePath}");
            if (reloadedAfterUpdate.Success && reloadedAfterUpdate.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUpdate.Document,
                    recordIndex,
                    propertyName,
                    expectedUpdatedRawValue,
                    expectedObjectTitle,
                    updatedSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: false,
                    $"reloaded edited real asset placement snapshot should preserve {propertyName}");
                AssertRealAssetUnplacedObjectCount(
                    reloadedAfterUpdate.Document,
                    expectedUpdatedUnplacedObjectCount,
                    $"reloaded edited real asset placement snapshot should preserve unplaced-object counts");
            }

            var undoHandled = control.TryHandleUndoCommand();
            Expect(undoHandled,
                $"real asset editor placement smoke should execute undo after editing {propertyName} for {sourcePath}");
            Application.DoEvents();

            var undoneSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex != recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text;
                    var selectedObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var propertyValue = TypeDescriptor.GetProperties(refreshedSelection)[propertyName]?.GetValue(refreshedSelection)?.ToString();
                    return string.Equals(selectedSection, initialSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           selectedObject?.RecordIndex == recordIndex &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == recordIndex &&
                           ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") is null &&
                           ReadPrivateBoolField(surface, "unplacedReportObjectsSelected") &&
                           HasLabelTextContaining(control, $"Unplaced objects: {expectedOriginalUnplacedObjectCount}") &&
                           string.Equals(propertyValue, expectedUndoSelectionValue, StringComparison.Ordinal);
                });
            Expect(undoneSelection,
                $"real asset editor placement smoke should preserve section/object continuity after undoing {propertyName} for {sourcePath}");
            Expect(!control.CanHandleUndoCommand(),
                $"real asset editor placement smoke should clear undo after restoring {propertyName} for {sourcePath}");

            var reloadedAfterUndo = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null,
                $"real asset editor placement smoke should reload restored on-disk state for {sourcePath}");
            if (reloadedAfterUndo.Success && reloadedAfterUndo.Document is not null)
            {
                AssertRealAssetRoundTripSnapshot(
                    reloadedAfterUndo.Document,
                    recordIndex,
                    propertyName,
                    expectedOriginalRawValue,
                    expectedObjectTitle,
                    initialSectionTitle,
                    expectedSectionCount,
                    expectLabel,
                    expectUnplacedObject: true,
                    $"reloaded undone editor real asset placement snapshot should preserve {propertyName}");
                AssertRealAssetUnplacedObjectCount(
                    reloadedAfterUndo.Document,
                    expectedOriginalUnplacedObjectCount,
                    $"reloaded undone editor real asset placement snapshot should preserve unplaced-object counts");
            }

            TearDownForm(hostForm);
        }
        finally
        {
            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeAssetEditorDuplicateCommandWithRealAsset(
        string? sourcePath,
        int recordIndex,
        string duplicateUniqueId,
        string expectedSourceObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        int expectedOriginalSectionObjectCount,
        int expectedUpdatedSectionObjectCount)
    {
        if (string.IsNullOrWhiteSpace(sourcePath) || !File.Exists(sourcePath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(sourcePath) ? "real asset editor duplicate candidate" : sourcePath)} not found.");
            return;
        }

        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeRealAssetEditorDuplicates-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        var assetPath = CreateWritableAssetCopy(sourcePath!, tempRoot);
        var previousDuplicateUniqueId = Environment.GetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID");

        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", duplicateUniqueId);

            using var hostForm = new Form
            {
                Width = 1400,
                Height = 1000,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            using var control = new CopperfinAssetEditorControl
            {
                Dock = DockStyle.Fill
            };

            hostForm.Controls.Add(control);
            hostForm.Show();
            Application.DoEvents();
            control.LoadDocument(assetPath);

            var sectionListView = GetPrivateListView(control, "sectionListView");
            var objectListView = GetPrivateListView(control, "objectListView");
            var propertyGrid = GetPrivatePropertyGrid(control);
            var duplicateButton = GetPrivateButton(control, "duplicateObjectButton");
            var deleteButton = GetPrivateButton(control, "deleteObjectButton");
            var restoreButton = GetPrivateButton(control, "restoreObjectButton");
            var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared report/label design surface for the real duplicate smoke.");

            var loaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => sectionListView.Items.Count > 0);
            Expect(loaded, $"real asset editor duplicate smoke should load section data for {sourcePath}");
            if (!loaded)
            {
                return;
            }

            foreach (ListViewItem item in sectionListView.Items)
            {
                item.Selected = string.Equals(item.Text, expectedSectionTitle, StringComparison.OrdinalIgnoreCase);
            }

            InvokeAssetEditorVoid(control, "SyncExplorerSelection");
            Application.DoEvents();

            var objectLoaded = WaitUntil(
                TimeSpan.FromSeconds(8),
                () => objectListView.Items.Cast<ListViewItem>()
                    .Any(item => item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                 snapshotObject.RecordIndex == recordIndex));
            Expect(objectLoaded, $"real asset editor duplicate smoke should surface object {recordIndex} for {sourcePath}");
            if (!objectLoaded)
            {
                return;
            }

            foreach (ListViewItem item in objectListView.Items)
            {
                item.Selected = item.Tag is CopperfinStudioSnapshotObject snapshotObject &&
                                snapshotObject.RecordIndex == recordIndex;
            }

            InvokeAssetEditorVoid(control, "SyncSelectionFromList");
            Application.DoEvents();

            Expect(duplicateButton.Visible &&
                   duplicateButton.Enabled &&
                   deleteButton.Visible &&
                   deleteButton.Enabled &&
                   !restoreButton.Visible &&
                   objectListView.Items.Count == expectedOriginalSectionObjectCount &&
                   propertyGrid.SelectedObject is CopperfinDesignerSelection initialSelection &&
                   initialSelection.RecordIndex == recordIndex,
                $"real asset editor duplicate smoke should start from a live object selection with duplicate and delete commands exposed for {sourcePath}");

            duplicateButton.PerformClick();
            Application.DoEvents();

            var duplicatedSelection = WaitUntil(
                TimeSpan.FromSeconds(8),
                () =>
                {
                    if (propertyGrid.SelectedObject is not CopperfinDesignerSelection refreshedSelection ||
                        refreshedSelection.RecordIndex == recordIndex)
                    {
                        return false;
                    }

                    var selectedSection = sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text;
                    var selectedSnapshotObject = objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Tag as CopperfinStudioSnapshotObject;
                    var selectedUniqueId = selectedSnapshotObject is null
                        ? null
                        : TryGetSnapshotObjectPropertyValue(selectedSnapshotObject, "UNIQUEID");
                    return string.Equals(selectedSection, expectedSectionTitle, StringComparison.OrdinalIgnoreCase) &&
                           objectListView.Items.Count == expectedUpdatedSectionObjectCount &&
                           string.Equals(selectedUniqueId, duplicateUniqueId, StringComparison.Ordinal) &&
                           string.Equals(selectedSnapshotObject?.Title, expectedSourceObjectTitle, StringComparison.Ordinal) &&
                           duplicateButton.Visible &&
                           duplicateButton.Enabled &&
                           deleteButton.Visible &&
                           deleteButton.Enabled &&
                           !restoreButton.Visible &&
                           string.Equals(ReadPrivateStringField(surface, "assetFamily"), expectLabel ? "label" : "report", StringComparison.Ordinal) &&
                           ReadPrivateNullableInt(surface, "selectedRecordIndex") == refreshedSelection.RecordIndex &&
                           !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected");
                });
            Expect(duplicatedSelection,
                $"real asset editor duplicate smoke should preserve section/object continuity after duplicating the selected row for {sourcePath}");

            var reloadedAfterDuplicate = CopperfinStudioSnapshotClient.TryLoad(assetPath);
            Expect(reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null,
                $"real asset editor duplicate smoke should reload duplicated on-disk state for {sourcePath}");
            if (reloadedAfterDuplicate.Success && reloadedAfterDuplicate.Document is not null)
            {
                var reloadedDuplicatedObject = FindSnapshotObjectByUniqueId(reloadedAfterDuplicate.Document, duplicateUniqueId);
                Expect(reloadedDuplicatedObject is not null,
                    $"reloaded real asset editor duplicate snapshot should preserve the duplicated UNIQUEID for {sourcePath}");
                if (reloadedDuplicatedObject is not null)
                {
                    AssertRealAssetRoundTripSnapshot(
                        reloadedAfterDuplicate.Document,
                        reloadedDuplicatedObject.RecordIndex,
                        "UNIQUEID",
                        duplicateUniqueId,
                        expectedSourceObjectTitle,
                        expectedSectionTitle,
                        expectedSectionCount,
                        expectLabel,
                        expectUnplacedObject: false,
                        $"reloaded real asset editor duplicate snapshot should preserve duplicated object identity");
                }

                AssertRealAssetSectionObjectCount(
                    reloadedAfterDuplicate.Document,
                    expectedSectionTitle,
                    expectedUpdatedSectionObjectCount,
                    $"reloaded real asset editor duplicate snapshot should preserve section object counts");
            }

            TearDownForm(hostForm);
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_DUPLICATE_OBJECT_UNIQUE_ID", previousDuplicateUniqueId);

            try
            {
                if (Directory.Exists(tempRoot))
                {
                    Directory.Delete(tempRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }

    private static void SmokeProjectEditorWithRealAsset(string? path, string[] expectGroups)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real project asset candidate" : path)} not found.");
            return;
        }

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();
        control.LoadDocument(path!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => FindListViews(control).Any(list => list.Items.Count > 0) &&
                  FindRichTextBoxes(control).Any(box => !string.IsNullOrWhiteSpace(box.Text)));
        Expect(loaded, $"project editor should load grouped workspace data for {path}");

        var groupFound = FindListViews(control)
            .SelectMany(list => list.Items.Cast<ListViewItem>())
            .Any(item => expectGroups.Any(expectGroup => string.Equals(item.Text, expectGroup, StringComparison.OrdinalIgnoreCase)));
        Expect(groupFound, $"project editor should surface one of the expected groups for {path}");

        var projectButtons = FindButtons(control).Select(button => button.Text).ToList();
        Expect(projectButtons.Contains("Build Copperfin Project"), $"project editor should expose a build command for {path}");
        Expect(projectButtons.Contains("Run Copperfin Project"), $"project editor should expose a run command for {path}");
        Expect(projectButtons.Contains("Debug Copperfin Project"), $"project editor should expose a debug command for {path}");

        var summary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Project Workspace", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(summary is not null, $"project editor should surface a workspace summary for {path}");
        if (summary is not null)
        {
            Expect(summary.Text.IndexOf("Planned Output:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project workspace summary should include a build output for {path}");
            Expect(summary.Text.IndexOf("Startup Item:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project workspace summary should include a startup item for {path}");
            Expect(summary.Text.IndexOf("Native Security:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project workspace summary should include native security for {path}");
            Expect(summary.Text.IndexOf(".NET And Extensibility:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project workspace summary should include .NET/extensibility guidance for {path}");
        }

        var taskListSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Task List", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(taskListSummary is not null, $"project editor should surface a task-list pane for {path}");

        var codeReferenceSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Code References", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(codeReferenceSummary is not null, $"project editor should surface a code-references pane for {path}");

        var dataExplorerSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Data Explorer", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(dataExplorerSummary is not null, $"project editor should surface a data-explorer pane for {path}");

        var objectBrowserSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Object Browser", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(objectBrowserSummary is not null, $"project editor should surface an object-browser pane for {path}");

        var toolboxSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Toolbox And Add-ins", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(toolboxSummary is not null, $"project editor should surface a toolbox pane for {path}");

        var buildersSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Builders", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(buildersSummary is not null, $"project editor should surface a builders pane for {path}");

        var coverageSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Coverage", StringComparison.OrdinalIgnoreCase) >= 0 ||
                                   box.Text.IndexOf("runtime coverage signals", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(coverageSummary is not null, $"project editor should surface a coverage pane for {path}");

        var databaseSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Database Federation", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(databaseSummary is not null, $"project editor should surface a database pane for {path}");

        TearDownForm(hostForm);
    }

    private static void SmokeProjectDebuggerWithRealAsset(string? path)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real project debug asset candidate" : path)} not found.");
            return;
        }

        if (Path.DirectorySeparatorChar != '\\')
        {
            Console.WriteLine("SKIP: real project debugger smoke requires Windows runtime build execution support.");
            return;
        }

        using var hostForm = new Form
        {
            Width = 1400,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        using var control = new CopperfinAssetEditorControl
        {
            Dock = DockStyle.Fill
        };

        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();
        control.LoadDocument(path!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => FindButtons(control).Any(button => button.Text == "Debug Copperfin Project"));
        Expect(loaded, $"project debugger command should load for {path}");

        var debugButton = FindButtons(control).FirstOrDefault(button => button.Text == "Debug Copperfin Project");
        if (debugButton is null)
        {
            TearDownForm(hostForm);
            return;
        }

        debugButton.PerformClick();
        var debugLoaded = WaitUntil(
            TimeSpan.FromSeconds(30),
            () => FindRichTextBoxes(control)
                .Any(box => box.Text.IndexOf("Copperfin Debug Session", StringComparison.OrdinalIgnoreCase) >= 0 &&
                            box.Text.IndexOf("Pause Reason:", StringComparison.OrdinalIgnoreCase) >= 0));
        Expect(debugLoaded, $"project debugger should surface a runtime pause state for {path}");

        var debuggerSummary = FindRichTextBoxes(control)
            .FirstOrDefault(box => box.Text.IndexOf("Copperfin Debug Session", StringComparison.OrdinalIgnoreCase) >= 0);
        Expect(debuggerSummary is not null, $"project debugger should surface a debug summary for {path}");
        if (debuggerSummary is not null)
        {
            Expect(debuggerSummary.Text.IndexOf("Call Stack:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project debugger should include a call stack for {path}");
            Expect(debuggerSummary.Text.IndexOf("Runtime Events:", StringComparison.OrdinalIgnoreCase) >= 0,
                $"project debugger should include runtime events for {path}");
        }

        TearDownForm(hostForm);
    }

    private static void SmokeStandaloneStudioWithMultipleAssets(string? firstPath, string? secondPath)
    {
        if (string.IsNullOrWhiteSpace(firstPath) ||
            string.IsNullOrWhiteSpace(secondPath) ||
            !File.Exists(firstPath) ||
            !File.Exists(secondPath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(firstPath) ? "real asset candidate" : firstPath)} or {(string.IsNullOrWhiteSpace(secondPath) ? "real asset candidate" : secondPath)} not found.");
            return;
        }

        using var form = new StudioMainForm
        {
            Width = 1500,
            Height = 1000,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        form.Show();
        Application.DoEvents();
        form.OpenDocument(firstPath!);
        form.OpenDocument(secondPath!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(10),
            () => FindTabControls(form).Any(tab => tab.TabPages.Count >= 2) &&
                  FindTabControls(form).SelectMany(tab => tab.TabPages.Cast<TabPage>())
                      .Any(page => page.Text.Equals(Path.GetFileName(firstPath), StringComparison.OrdinalIgnoreCase)) &&
                  FindTabControls(form).SelectMany(tab => tab.TabPages.Cast<TabPage>())
                      .Any(page => page.Text.Equals(Path.GetFileName(secondPath), StringComparison.OrdinalIgnoreCase)));
        Expect(loaded, "standalone Studio should open multiple assets as separate tabs");

        var tabControl = FindTabControls(form).FirstOrDefault();
        Expect(tabControl is not null, "standalone Studio should surface a document tab control");
        if (tabControl is not null)
        {
            var beforeDuplicateOpen = tabControl.TabPages.Count;
            form.OpenDocument(firstPath!);
            Application.DoEvents();
            Expect(tabControl.TabPages.Count == beforeDuplicateOpen, "opening an already open asset should not duplicate tabs");
            Expect(tabControl.SelectedTab is not null, "standalone Studio should keep a selected tab");
            Expect(tabControl.SelectedTab?.Text == Path.GetFileName(firstPath) || tabControl.SelectedTab?.Text == Path.GetFileName(secondPath),
                "standalone Studio should keep a valid selected asset tab");
        }

        TearDownForm(form);
    }

    private static void TearDownForm(Form form)
    {
        if (form.IsDisposed)
        {
            return;
        }

        form.Hide();
        Application.DoEvents();
        Thread.Sleep(150);
        Application.DoEvents();
        form.Close();
        Application.DoEvents();
    }

    private static bool WaitUntil(TimeSpan timeout, Func<bool> condition)
    {
        var deadline = DateTime.UtcNow + timeout;
        while (DateTime.UtcNow < deadline)
        {
            Application.DoEvents();
            if (condition())
            {
                return true;
            }

            Thread.Sleep(50);
        }

        Application.DoEvents();
        return condition();
    }

    private static string? ResolveFirstExistingRealAssetPath(params string?[] candidates)
    {
        return candidates.FirstOrDefault(candidate => !string.IsNullOrWhiteSpace(candidate) && File.Exists(candidate));
    }

    private static void AssertRealAssetRoundTripSnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string propertyName,
        string expectedPropertyValue,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionCount,
        bool expectLabel,
        bool expectUnplacedObject,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.IsLabel == expectLabel,
            $"{failurePrefix} for {document.Path} should preserve report/label identity");
        Expect(document.ReportLayout.Sections.Count == expectedSectionCount,
            $"{failurePrefix} for {document.Path} should preserve section counts");

        CopperfinStudioReportLayoutObject? layoutObject;
        if (expectUnplacedObject)
        {
            layoutObject = document.ReportLayout.UnplacedObjects
                .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
            Expect(layoutObject is not null,
                $"{failurePrefix} for {document.Path} should preserve unplaced object {recordIndex}");
            if (layoutObject is null)
            {
                return;
            }
        }
        else
        {
            var section = document.ReportLayout.Sections
                .FirstOrDefault(candidate => string.Equals(candidate.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase));
            Expect(section is not null,
                $"{failurePrefix} for {document.Path} should preserve section '{expectedSectionTitle}'");
            if (section is null)
            {
                return;
            }

            layoutObject = section.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
            Expect(layoutObject is not null,
                $"{failurePrefix} for {document.Path} should preserve placed object {recordIndex}");
            if (layoutObject is null)
            {
                return;
            }
        }

        Expect(string.Equals(layoutObject.Title, expectedObjectTitle, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should preserve the selected object title");

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw snapshot object {recordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        var property = snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase));
        Expect(property is not null,
            $"{failurePrefix} for {document.Path} should preserve property {propertyName}");
        if (property is null)
        {
            return;
        }

        Expect(string.Equals(property.Value, expectedPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should expose {propertyName}={expectedPropertyValue}");
    }

    private static void AssertRealAssetSectionSnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string expectedSectionTitle,
        string propertyName,
        string expectedRawPropertyValue,
        int expectedLayoutPropertyValue,
        int expectedSectionCount,
        bool expectLabel,
        int expectedObjectCount,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.IsLabel == expectLabel,
            $"{failurePrefix} for {document.Path} should preserve report/label identity");
        Expect(document.ReportLayout.Sections.Count == expectedSectionCount,
            $"{failurePrefix} for {document.Path} should preserve section counts");

        var section = document.ReportLayout.Sections
            .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(section is not null,
            $"{failurePrefix} for {document.Path} should preserve section record {recordIndex}");
        if (section is null)
        {
            return;
        }

        Expect(string.Equals(section.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase),
            $"{failurePrefix} for {document.Path} should preserve section '{expectedSectionTitle}'");
        Expect(section.Objects.Count == expectedObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedObjectCount} objects in section '{expectedSectionTitle}'");

        var layoutPropertyValue = TryGetReportSectionLayoutValue(section, propertyName);
        Expect(layoutPropertyValue.HasValue,
            $"{failurePrefix} for {document.Path} should expose section layout property {propertyName}");
        if (!layoutPropertyValue.HasValue)
        {
            return;
        }

        Expect(layoutPropertyValue.Value == expectedLayoutPropertyValue,
            $"{failurePrefix} for {document.Path} should expose section {propertyName}={expectedLayoutPropertyValue}");

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw snapshot object {recordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        var property = snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase));
        Expect(property is not null,
            $"{failurePrefix} for {document.Path} should preserve property {propertyName}");
        if (property is null)
        {
            return;
        }

        Expect(string.Equals(property.Value, expectedRawPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should expose {propertyName}={expectedRawPropertyValue}");
    }

    private static void AssertRealAssetUnplacedObjectCount(
        CopperfinStudioSnapshotDocument document,
        int expectedUnplacedObjectCount,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.UnplacedObjects.Count == expectedUnplacedObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedUnplacedObjectCount} unplaced objects");
    }

    private static void AssertRealAssetSectionObjectCount(
        CopperfinStudioSnapshotDocument document,
        string expectedSectionTitle,
        int expectedObjectCount,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        var section = document.ReportLayout.Sections
            .FirstOrDefault(candidate => string.Equals(candidate.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase));
        Expect(section is not null,
            $"{failurePrefix} for {document.Path} should preserve section '{expectedSectionTitle}'");
        if (section is null)
        {
            return;
        }

        Expect(section.Objects.Count == expectedObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedObjectCount} objects in section '{expectedSectionTitle}'");
    }

    private static void AssertRealAssetDeletedObjectSnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedVisibleSectionObjectCount,
        string failurePrefix)
    {
        AssertRealAssetDeletedObjectSnapshot(
            document,
            recordIndex,
            expectedUniqueId,
            expectedObjectTitle,
            expectedSectionTitle,
            expectedSectionRecordIndex,
            expectedSectionCount,
            expectLabel,
            expectedVisibleSectionObjectCount,
            expectedDeletedSectionObjectCount: 1,
            failurePrefix);
    }

    private static void AssertRealAssetDeletedObjectSnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedVisibleSectionObjectCount,
        int expectedDeletedSectionObjectCount,
        string failurePrefix)
    {
        Expect(document.ReportLayout is not null,
            $"{failurePrefix} for {document.Path} should include a report layout");
        if (document.ReportLayout is null)
        {
            return;
        }

        Expect(document.ReportLayout.IsLabel == expectLabel,
            $"{failurePrefix} for {document.Path} should preserve report/label identity");
        Expect(document.ReportLayout.Sections.Count == expectedSectionCount,
            $"{failurePrefix} for {document.Path} should preserve section counts");
        Expect(document.ReportLayout.DeletedPreviewBoundsAvailable,
            $"{failurePrefix} for {document.Path} should preserve deleted preview bounds");

        var section = document.ReportLayout.Sections
            .FirstOrDefault(candidate => candidate.RecordIndex == expectedSectionRecordIndex);
        Expect(section is not null,
            $"{failurePrefix} for {document.Path} should preserve containing section record {expectedSectionRecordIndex}");
        if (section is null)
        {
            return;
        }

        Expect(string.Equals(section.Title, expectedSectionTitle, StringComparison.OrdinalIgnoreCase),
            $"{failurePrefix} for {document.Path} should preserve containing section '{expectedSectionTitle}'");
        Expect(section.DeletedObjectCount == expectedDeletedSectionObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedDeletedSectionObjectCount} deleted objects in section '{expectedSectionTitle}'");

        var deletedLayoutObject = document.ReportLayout.DeletedObjects
            .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(deletedLayoutObject is not null,
            $"{failurePrefix} for {document.Path} should expose deleted layout object {recordIndex}");
        if (deletedLayoutObject is null)
        {
            return;
        }

        Expect(string.Equals(deletedLayoutObject.Title, expectedObjectTitle, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should preserve the deleted object title");
        Expect(deletedLayoutObject.ContainingSectionRecordIndex == expectedSectionRecordIndex,
            $"{failurePrefix} for {document.Path} should preserve the deleted object's containing section");

        var visibleSectionObjectCount = section.Objects.Count +
                                        document.ReportLayout.DeletedObjects.Count(candidate =>
                                            candidate.ContainingSectionRecordIndex == expectedSectionRecordIndex);
        Expect(visibleSectionObjectCount == expectedVisibleSectionObjectCount,
            $"{failurePrefix} for {document.Path} should expose {expectedVisibleSectionObjectCount} visible objects in section '{expectedSectionTitle}'");

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw snapshot object {recordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        Expect(snapshotObject.Deleted,
            $"{failurePrefix} for {document.Path} should preserve deleted object state");
        Expect(string.Equals(snapshotObject.Title, expectedObjectTitle, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should preserve the raw deleted object title");
        Expect(string.Equals(TryGetSnapshotObjectPropertyValue(snapshotObject, "UNIQUEID"), expectedUniqueId, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should preserve the deleted UNIQUEID");
    }

    private static void AssertRealAssetDeletedObjectPropertySnapshot(
        CopperfinStudioSnapshotDocument document,
        int recordIndex,
        string propertyName,
        string expectedRawPropertyValue,
        int expectedLayoutPropertyValue,
        string expectedUniqueId,
        string expectedObjectTitle,
        string expectedSectionTitle,
        int expectedSectionRecordIndex,
        int expectedSectionCount,
        bool expectLabel,
        int expectedVisibleSectionObjectCount,
        string failurePrefix)
    {
        AssertRealAssetDeletedObjectSnapshot(
            document,
            recordIndex,
            expectedUniqueId,
            expectedObjectTitle,
            expectedSectionTitle,
            expectedSectionRecordIndex,
            expectedSectionCount,
            expectLabel,
            expectedVisibleSectionObjectCount,
            failurePrefix);

        if (document.ReportLayout is null)
        {
            return;
        }

        var deletedLayoutObject = document.ReportLayout.DeletedObjects
            .FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(deletedLayoutObject is not null,
            $"{failurePrefix} for {document.Path} should preserve deleted layout object {recordIndex}");
        if (deletedLayoutObject is null)
        {
            return;
        }

        var layoutPropertyValue = TryGetReportLayoutObjectValue(deletedLayoutObject, propertyName);
        Expect(layoutPropertyValue.HasValue,
            $"{failurePrefix} for {document.Path} should expose deleted layout property {propertyName}");
        if (!layoutPropertyValue.HasValue)
        {
            return;
        }

        Expect(layoutPropertyValue.Value == expectedLayoutPropertyValue,
            $"{failurePrefix} for {document.Path} should expose deleted layout {propertyName}={expectedLayoutPropertyValue}");

        var snapshotObject = document.Objects.FirstOrDefault(candidate => candidate.RecordIndex == recordIndex);
        Expect(snapshotObject is not null,
            $"{failurePrefix} for {document.Path} should preserve raw deleted snapshot object {recordIndex}");
        if (snapshotObject is null)
        {
            return;
        }

        var property = snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase));
        Expect(property is not null,
            $"{failurePrefix} for {document.Path} should preserve deleted property {propertyName}");
        if (property is null)
        {
            return;
        }

        Expect(string.Equals(property.Value, expectedRawPropertyValue, StringComparison.Ordinal),
            $"{failurePrefix} for {document.Path} should expose deleted {propertyName}={expectedRawPropertyValue}");
    }

    private static CopperfinStudioSnapshotObject? FindSnapshotObjectByUniqueId(
        CopperfinStudioSnapshotDocument document,
        string uniqueId)
    {
        return document.Objects.FirstOrDefault(candidate =>
            string.Equals(TryGetSnapshotObjectPropertyValue(candidate, "UNIQUEID"), uniqueId, StringComparison.OrdinalIgnoreCase));
    }

    private static string? TryGetSnapshotObjectPropertyValue(
        CopperfinStudioSnapshotObject snapshotObject,
        string propertyName)
    {
        return snapshotObject.Properties
            .FirstOrDefault(candidate => string.Equals(candidate.Name, propertyName, StringComparison.OrdinalIgnoreCase))
            ?.Value;
    }

    private static int? TryGetReportSectionLayoutValue(
        CopperfinStudioReportSection section,
        string propertyName)
    {
        if (string.Equals(propertyName, "TOP", StringComparison.OrdinalIgnoreCase))
        {
            return section.Top;
        }

        if (string.Equals(propertyName, "HEIGHT", StringComparison.OrdinalIgnoreCase))
        {
            return section.Height;
        }

        return null;
    }

    private static int? TryGetReportLayoutObjectValue(
        CopperfinStudioReportLayoutObject layoutObject,
        string propertyName)
    {
        if (string.Equals(propertyName, "HPOS", StringComparison.OrdinalIgnoreCase))
        {
            return layoutObject.Left;
        }

        if (string.Equals(propertyName, "VPOS", StringComparison.OrdinalIgnoreCase))
        {
            return layoutObject.Top;
        }

        if (string.Equals(propertyName, "WIDTH", StringComparison.OrdinalIgnoreCase))
        {
            return layoutObject.Width;
        }

        if (string.Equals(propertyName, "HEIGHT", StringComparison.OrdinalIgnoreCase))
        {
            return layoutObject.Height;
        }

        return null;
    }

    private static string CreateWritableAssetCopy(string sourcePath, string tempRoot)
    {
        var sourceFileInfo = new FileInfo(sourcePath);
        if (sourceFileInfo.Directory is null)
        {
            throw new InvalidOperationException($"Could not determine containing directory for {sourcePath}.");
        }

        var destinationDirectory = Path.Combine(tempRoot, sourceFileInfo.Directory.Name);
        CopyDirectoryRecursive(sourceFileInfo.Directory.FullName, destinationDirectory);
        return Path.Combine(destinationDirectory, sourceFileInfo.Name);
    }

    private static void CopyDirectoryRecursive(string sourceDirectory, string destinationDirectory)
    {
        Directory.CreateDirectory(destinationDirectory);

        foreach (var filePath in Directory.GetFiles(sourceDirectory))
        {
            var destinationPath = Path.Combine(destinationDirectory, Path.GetFileName(filePath));
            File.Copy(filePath, destinationPath, overwrite: true);
        }

        foreach (var childDirectory in Directory.GetDirectories(sourceDirectory))
        {
            var destinationChild = Path.Combine(destinationDirectory, Path.GetFileName(childDirectory));
            CopyDirectoryRecursive(childDirectory, destinationChild);
        }
    }

    private static void WithResolvedRealAssetToolchain(Action action)
    {
        var previousHostPath = Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH");
        var previousBuildHostPath = Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH");
        var previousRuntimeHostPath = Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH");
        var resolvedStudioHostPath = ResolveLocalToolPath(
            Environment.GetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH"),
            "copperfin_studio_host");
        var resolvedBuildHostPath = ResolveLocalToolPath(
            Environment.GetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH"),
            "copperfin_build_host");
        var resolvedRuntimeHostPath = ResolveLocalToolPath(
            Environment.GetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH"),
            "copperfin_runtime_host");

        if (!string.IsNullOrWhiteSpace(resolvedStudioHostPath))
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", resolvedStudioHostPath);
        }

        if (!string.IsNullOrWhiteSpace(resolvedBuildHostPath))
        {
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", resolvedBuildHostPath);
        }

        if (!string.IsNullOrWhiteSpace(resolvedRuntimeHostPath))
        {
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", resolvedRuntimeHostPath);
        }

        try
        {
            action();
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_STUDIO_HOST_PATH", previousHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_BUILD_HOST_PATH", previousBuildHostPath);
            Environment.SetEnvironmentVariable("COPPERFIN_RUNTIME_HOST_PATH", previousRuntimeHostPath);
        }
    }

    private static string? ResolveLocalToolPath(string? configuredPath, string toolName)
    {
        var candidates = new[]
        {
            ExpandUserPath(configuredPath),
            ExpandUserPath("./build/" + toolName),
            ExpandUserPath("./build/" + toolName + ".exe"),
            ExpandUserPath("./build2/" + toolName),
            ExpandUserPath("./build2/" + toolName + ".exe"),
            ExpandUserPath("./build/Release/" + toolName),
            ExpandUserPath("./build/Release/" + toolName + ".exe"),
            ExpandUserPath("./.tmp/install-localization/bin/" + toolName),
            ExpandUserPath("./.tmp/install-localization/bin/" + toolName + ".exe")
        };

        return candidates.FirstOrDefault(candidate => !string.IsNullOrWhiteSpace(candidate) && File.Exists(candidate));
    }

    private static string? TryResolveVfp9InstallAsset(string relativePath)
    {
        var configuredRoot = ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFP9_ROOT"));
        var defaultRoot = Path.DirectorySeparatorChar == '\\'
            ? @"C:\Program Files (x86)\Microsoft Visual FoxPro 9"
            : null;

        foreach (var root in new[] { configuredRoot, defaultRoot })
        {
            if (string.IsNullOrWhiteSpace(root))
            {
                continue;
            }

            var candidate = Path.Combine(root!, relativePath.Replace('\\', Path.DirectorySeparatorChar));
            if (File.Exists(candidate))
            {
                return candidate;
            }
        }

        return null;
    }

    private static string? TryResolveVfpSourceAsset(string archiveRelativePath)
    {
        var configuredRoot = ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFPSOURCE_ROOT"));
        if (!string.IsNullOrWhiteSpace(configuredRoot))
        {
            var rootedCandidate = TryResolveAssetUnderRoot(configuredRoot!, archiveRelativePath);
            if (!string.IsNullOrWhiteSpace(rootedCandidate))
            {
                return rootedCandidate;
            }
        }

        var zipPath = ResolveFirstExistingRealAssetPath(
            ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFPSOURCE_ZIP")),
            ExpandUserPath("~/Downloads/VFPSource.zip"));
        if (string.IsNullOrWhiteSpace(zipPath))
        {
            return null;
        }

        return TryExtractArchiveContainingDirectory(zipPath!, archiveRelativePath);
    }

    private static string? TryResolveAssetUnderRoot(string root, string archiveRelativePath)
    {
        var normalizedRelativePath = archiveRelativePath.Replace('/', Path.DirectorySeparatorChar);
        var candidates = new List<string>
        {
            Path.Combine(root, normalizedRelativePath)
        };

        const string vfpSourcePrefix = "VFPSource/";
        if (archiveRelativePath.StartsWith(vfpSourcePrefix, StringComparison.OrdinalIgnoreCase))
        {
            candidates.Add(Path.Combine(root, archiveRelativePath.Substring(vfpSourcePrefix.Length).Replace('/', Path.DirectorySeparatorChar)));
        }

        return candidates.FirstOrDefault(File.Exists);
    }

    private static string? TryExtractArchiveContainingDirectory(string zipPath, string archiveRelativePath)
    {
        var normalizedRelativePath = archiveRelativePath.Replace('\\', '/');
        var directorySeparatorIndex = normalizedRelativePath.LastIndexOf('/');
        if (directorySeparatorIndex < 0)
        {
            return null;
        }

        var archiveDirectory = normalizedRelativePath.Substring(0, directorySeparatorIndex + 1);
        var assetFileName = normalizedRelativePath.Substring(directorySeparatorIndex + 1);
        var extractionRoot = Path.Combine(
            Path.GetTempPath(),
            "CopperfinDesignerSmokeRealAssets",
            Path.GetFileNameWithoutExtension(zipPath),
            archiveDirectory.Replace('/', Path.DirectorySeparatorChar));
        var extractedAssetPath = Path.Combine(extractionRoot, assetFileName);
        if (File.Exists(extractedAssetPath))
        {
            return extractedAssetPath;
        }

        Directory.CreateDirectory(extractionRoot);

        using var archive = ZipFile.OpenRead(zipPath);
        var matchingEntries = archive.Entries
            .Where(entry => entry.FullName.StartsWith(archiveDirectory, StringComparison.OrdinalIgnoreCase) &&
                            !string.IsNullOrEmpty(entry.Name))
            .ToList();
        if (matchingEntries.Count == 0)
        {
            return null;
        }

        foreach (var entry in matchingEntries)
        {
            var relativeEntryPath = entry.FullName.Substring(archiveDirectory.Length).Replace('/', Path.DirectorySeparatorChar);
            var destinationPath = Path.Combine(extractionRoot, relativeEntryPath);
            var destinationDirectory = Path.GetDirectoryName(destinationPath);
            if (!string.IsNullOrWhiteSpace(destinationDirectory))
            {
                Directory.CreateDirectory(destinationDirectory!);
            }

            entry.ExtractToFile(destinationPath, overwrite: true);
        }

        return File.Exists(extractedAssetPath) ? extractedAssetPath : null;
    }

    private static string? ExpandUserPath(string? path)
    {
        if (string.IsNullOrWhiteSpace(path))
        {
            return path;
        }

        var nonNullPath = path!;

        if (nonNullPath == "~")
        {
            return Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
        }

        if (nonNullPath.StartsWith("~/", StringComparison.Ordinal) || nonNullPath.StartsWith("~\\", StringComparison.Ordinal))
        {
            var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
            return Path.Combine(home, nonNullPath.Substring(2));
        }

        return nonNullPath;
    }

    private static IEnumerable<ListView> FindListViews(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is ListView listView)
            {
                yield return listView;
            }

            foreach (var nested in FindListViews(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasListViewColumnText(Control root, string text)
    {
        foreach (var listView in FindListViews(root))
        {
            foreach (ColumnHeader column in listView.Columns)
            {
                if (string.Equals(column.Text, text, StringComparison.Ordinal))
                {
                    return true;
                }
            }
        }

        return false;
    }

    private static void ApplyProjectSnapshotForColumnSmoke(CopperfinAssetEditorControl control)
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "project",
            ProjectWorkspace = new CopperfinStudioProjectWorkspace
            {
                Groups = new List<CopperfinStudioProjectGroup>
                {
                    new()
                    {
                        Id = "forms",
                        Title = "Forms",
                        ItemCount = 1,
                        ExcludedCount = 0
                    }
                }
            }
        };

        var controlType = typeof(CopperfinAssetEditorControl);
        var currentSnapshotField = controlType.GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        var populateSectionListMethod = controlType.GetMethod("PopulateSectionList", BindingFlags.Instance | BindingFlags.NonPublic);
        var configureObjectColumnsMethod = controlType.GetMethod("ConfigureObjectColumns", BindingFlags.Instance | BindingFlags.NonPublic);
        if (currentSnapshotField is null || populateSectionListMethod is null || configureObjectColumnsMethod is null)
        {
            throw new InvalidOperationException("Could not find CopperfinAssetEditorControl project-column smoke hooks.");
        }

        currentSnapshotField?.SetValue(control, snapshot);
        populateSectionListMethod.Invoke(control, new object?[] { null });
        configureObjectColumnsMethod.Invoke(control, Array.Empty<object>());
    }

    private static void ApplyReportSnapshotForExplorerSmoke(CopperfinAssetEditorControl control, CopperfinStudioSnapshotDocument snapshot)
    {
        var controlType = typeof(CopperfinAssetEditorControl);
        var currentSnapshotField = controlType.GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        var populateSectionListMethod = controlType.GetMethod("PopulateSectionList", BindingFlags.Instance | BindingFlags.NonPublic);
        var populateObjectListMethod = controlType.GetMethod("PopulateObjectList", BindingFlags.Instance | BindingFlags.NonPublic);
        if (currentSnapshotField is null || populateSectionListMethod is null || populateObjectListMethod is null)
        {
            throw new InvalidOperationException("Could not find CopperfinAssetEditorControl report-explorer smoke hooks.");
        }

        currentSnapshotField.SetValue(control, snapshot);
        populateSectionListMethod.Invoke(control, new object?[] { null });
        populateObjectListMethod.Invoke(control, new object[] { true });
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorBatchUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorSectionUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorObjectUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorLabelSectionUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "label",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorUnplacedReportObjectUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                },
                new()
                {
                    RecordIndex = 9,
                    Title = "orphan.note",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "800" },
                        new() { Name = "VPOS", Value = "700" },
                        new() { Name = "WIDTH", Value = "2400" },
                        new() { Name = "HEIGHT", Value = "450" },
                        new() { Name = "EXPR", Value = "orphan.note" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
                {
                    new()
                    {
                        RecordIndex = 9,
                        ObjectKind = "field",
                        Title = "orphan.note",
                        Expression = "orphan.note",
                        Left = 800,
                        Top = 700,
                        Width = 2400,
                        Height = 450
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorUndoPreviewRefreshSmokeSnapshot()
    {
        var snapshot = BuildAssetEditorUnplacedReportObjectUpdateSmokeSnapshot();
        snapshot.CommandUndoAvailable = true;
        snapshot.CommandUndoLabel = "Move orphan.note";
        return snapshot;
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedReportUndoPreviewRefreshSmokeSnapshot()
    {
        var snapshot = BuildAssetEditorDeletedReportObjectUpdateSmokeSnapshot();
        snapshot.CommandUndoAvailable = true;
        snapshot.CommandUndoLabel = "Move deleted.footer.total";
        return snapshot;
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorLabelUndoPreviewRefreshSmokeSnapshot()
    {
        var snapshot = BuildAssetEditorUnplacedLabelObjectUpdateSmokeSnapshot();
        snapshot.CommandUndoAvailable = true;
        snapshot.CommandUndoLabel = "Move orphan.note";
        return snapshot;
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedLabelUndoPreviewRefreshSmokeSnapshot()
    {
        var snapshot = BuildAssetEditorDeletedLabelObjectUpdateSmokeSnapshot();
        snapshot.CommandUndoAvailable = true;
        snapshot.CommandUndoLabel = "Move deleted.footer.total";
        return snapshot;
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorLabelObjectUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "label",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildLabelSurfaceInteractionSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                },
                new()
                {
                    RecordIndex = 13,
                    Deleted = true,
                    Title = "deleted.footer.total",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1400" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "deleted.footer.total" }
                    }
                },
                new()
                {
                    RecordIndex = 9,
                    Title = "orphan.note",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "800" },
                        new() { Name = "VPOS", Value = "700" },
                        new() { Name = "WIDTH", Value = "2400" },
                        new() { Name = "HEIGHT", Value = "450" },
                        new() { Name = "EXPR", Value = "orphan.note" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "label",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "label",
                                Title = "deleted.footer.total",
                                Expression = "deleted.footer.total",
                                Left = 1400,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                },
                UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
                {
                    new()
                    {
                        RecordIndex = 9,
                        ObjectKind = "label",
                        Title = "orphan.note",
                        Expression = "orphan.note",
                        Left = 800,
                        Top = 700,
                        Width = 2400,
                        Height = 450
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorUnplacedLabelObjectUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                },
                new()
                {
                    RecordIndex = 9,
                    Title = "orphan.note",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "800" },
                        new() { Name = "VPOS", Value = "700" },
                        new() { Name = "WIDTH", Value = "2400" },
                        new() { Name = "HEIGHT", Value = "450" },
                        new() { Name = "EXPR", Value = "orphan.note" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "label",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
                {
                    new()
                    {
                        RecordIndex = 9,
                        ObjectKind = "label",
                        Title = "orphan.note",
                        Expression = "orphan.note",
                        Left = 800,
                        Top = 700,
                        Width = 2400,
                        Height = 450
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedLabelObjectUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                },
                new()
                {
                    RecordIndex = 13,
                    Deleted = true,
                    Title = "deleted.footer.total",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1400" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "deleted.footer.total" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "label",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "label",
                                Title = "deleted.footer.total",
                                Expression = "deleted.footer.total",
                                Left = 1400,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDuplicateLabelObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 7,
                    Title = "middle.value",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "middle-field-guid" },
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "middle.value" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 7,
                                ObjectKind = "label",
                                Title = "middle.value",
                                Expression = "middle.value",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorRenameDeletedLabelObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 13,
                    Deleted = true,
                    Title = "deleted.footer.total",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "deleted-footer-guid" },
                        new() { Name = "HPOS", Value = "1600" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "deleted.footer.total" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "label",
                                Title = "deleted.footer.total",
                                Expression = "deleted.footer.total",
                                Left = 1600,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorReorderBackLabelObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "first.value",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "first-field-guid" },
                        new() { Name = "HPOS", Value = "1400" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "3400" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "first.value" }
                    }
                },
                new()
                {
                    RecordIndex = 7,
                    Title = "middle.value",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "middle-field-guid" },
                        new() { Name = "HPOS", Value = "2500" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "3400" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "middle.value" }
                    }
                },
                new()
                {
                    RecordIndex = 8,
                    Title = "last.value",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "last-field-guid" },
                        new() { Name = "HPOS", Value = "3600" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "3200" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "last.value" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "label",
                                Title = "first.value",
                                Expression = "first.value",
                                Left = 1400,
                                Top = 2600,
                                Width = 3400,
                                Height = 600
                            },
                            new()
                            {
                                RecordIndex = 7,
                                ObjectKind = "label",
                                Title = "middle.value",
                                Expression = "middle.value",
                                Left = 2500,
                                Top = 2600,
                                Width = 3400,
                                Height = 600
                            },
                            new()
                            {
                                RecordIndex = 8,
                                ObjectKind = "label",
                                Title = "last.value",
                                Expression = "last.value",
                                Left = 3600,
                                Top = 2600,
                                Width = 3200,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorRestoreLabelObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 13,
                    Deleted = true,
                    Title = "deleted.footer.total",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "deleted-footer-guid" },
                        new() { Name = "HPOS", Value = "1600" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "deleted.footer.total" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "label",
                                Title = "deleted.footer.total",
                                Expression = "deleted.footer.total",
                                Left = 1600,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeleteLabelObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "live-detail-guid" },
                        new() { Name = "HPOS", Value = "1500" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "label",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1500,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedReportObjectUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                },
                new()
                {
                    RecordIndex = 13,
                    Deleted = true,
                    Title = "deleted.footer.total",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1400" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "deleted.footer.total" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "field",
                                Title = "deleted.footer.total",
                                Expression = "deleted.footer.total",
                                Left = 1400,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDuplicateReportObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 7,
                    Title = "middle.value",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "middle-field-guid" },
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "middle.value" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 7,
                                ObjectKind = "field",
                                Title = "middle.value",
                                Expression = "middle.value",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorRenameReportObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 7,
                    Title = "middle.value",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "middle-field-guid" },
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "middle.value" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 7,
                                ObjectKind = "field",
                                Title = "middle.value",
                                Expression = "middle.value",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorReorderFrontReportObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "first.value",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "first-field-guid" },
                        new() { Name = "HPOS", Value = "1000" },
                        new() { Name = "VPOS", Value = "2400" },
                        new() { Name = "WIDTH", Value = "3200" },
                        new() { Name = "HEIGHT", Value = "700" },
                        new() { Name = "EXPR", Value = "first.value" }
                    }
                },
                new()
                {
                    RecordIndex = 7,
                    Title = "middle.value",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "middle-field-guid" },
                        new() { Name = "HPOS", Value = "2100" },
                        new() { Name = "VPOS", Value = "2400" },
                        new() { Name = "WIDTH", Value = "3200" },
                        new() { Name = "HEIGHT", Value = "700" },
                        new() { Name = "EXPR", Value = "middle.value" }
                    }
                },
                new()
                {
                    RecordIndex = 8,
                    Title = "last.value",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "last-field-guid" },
                        new() { Name = "HPOS", Value = "3200" },
                        new() { Name = "VPOS", Value = "2400" },
                        new() { Name = "WIDTH", Value = "2700" },
                        new() { Name = "HEIGHT", Value = "700" },
                        new() { Name = "EXPR", Value = "last.value" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "first.value",
                                Expression = "first.value",
                                Left = 1000,
                                Top = 2400,
                                Width = 3200,
                                Height = 700
                            },
                            new()
                            {
                                RecordIndex = 7,
                                ObjectKind = "field",
                                Title = "middle.value",
                                Expression = "middle.value",
                                Left = 2100,
                                Top = 2400,
                                Width = 3200,
                                Height = 700
                            },
                            new()
                            {
                                RecordIndex = 8,
                                ObjectKind = "field",
                                Title = "last.value",
                                Expression = "last.value",
                                Left = 3200,
                                Top = 2400,
                                Width = 2700,
                                Height = 700
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeleteReportObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "live-detail-guid" },
                        new() { Name = "HPOS", Value = "1500" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1500,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorRestoreReportObjectSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 13,
                    Deleted = true,
                    Title = "deleted.footer.total",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "deleted-footer-guid" },
                        new() { Name = "HPOS", Value = "1600" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "deleted.footer.total" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "field",
                                Title = "deleted.footer.total",
                                Expression = "deleted.footer.total",
                                Left = 1600,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedReportSectionUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1200" },
                        new() { Name = "VPOS", Value = "2600" },
                        new() { Name = "WIDTH", Value = "4000" },
                        new() { Name = "HEIGHT", Value = "500" },
                        new() { Name = "EXPR", Value = "customer.company" }
                    }
                },
                new()
                {
                    RecordIndex = 13,
                    Deleted = true,
                    Title = "deleted.footer.total",
                    Subtitle = "field",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1400" },
                        new() { Name = "VPOS", Value = "9400" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "deleted.footer.total" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = "field",
                                Title = "customer.company",
                                Expression = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                DeletedSections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "deleted_footer",
                        Title = "Summary",
                        BandKind = "summary",
                        RecordIndex = 51,
                        Deleted = true,
                        Top = 9000,
                        Height = 1400,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 13,
                                ObjectKind = "field",
                                Title = "deleted.footer.total",
                                Expression = "deleted.footer.total",
                                Left = 1400,
                                Top = 9400,
                                Width = 3600,
                                Height = 600
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedLabelSectionUpdateSmokeSnapshot()
    {
        return BuildAssetEditorDeletedLabelObjectUpdateSmokeSnapshot();
    }

    private static string BuildBatchUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildBatchUpdatePreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2800"},{"Name":"WIDTH","Value":"4600"},{"Name":"HEIGHT","Value":"3500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1500,"PreviewBoundsTop":2800,"PreviewBoundsRight":6100,"PreviewBoundsBottom":6300,"PreviewBoundsWidth":4600,"PreviewBoundsHeight":3500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2800,"Width":4600,"Height":3500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildBatchLabelUpdatePreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2800"},{"Name":"WIDTH","Value":"4600"},{"Name":"HEIGHT","Value":"3500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1500,"PreviewBoundsTop":2800,"PreviewBoundsRight":6100,"PreviewBoundsBottom":6300,"PreviewBoundsWidth":4600,"PreviewBoundsHeight":3500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2800,"Width":4600,"Height":3500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildSectionUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"3800"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":3200,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":3800,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildUnplacedReportObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"field","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildUnplacedReportObjectPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":700,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":2400,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"field","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildUnplacedReportObjectPlacementIntoSectionHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":2400,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500},{"RecordIndex":9,"ObjectKind":"field","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":2400,"Width":2400,"Height":450}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildUndoPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"CommandUndoAvailable":false,"CommandUndoLabel":"","ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":700,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":2400,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"field","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildDeletedReportUndoPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"CommandUndoAvailable":false,"CommandUndoLabel":"","ReportLayout":{"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1400,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5000,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildLabelUndoPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"CommandUndoAvailable":false,"CommandUndoLabel":"","ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":700,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":2400,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"label","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildDeletedLabelUndoPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"CommandUndoAvailable":false,"CommandUndoLabel":"","ReportLayout":{"IsLabel":true,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1400,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5000,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildLabelSectionUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"3800"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":3200,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":3800,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildLabelObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildLabelObjectPlacementIntoUnplacedHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"9000"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1200,"PreviewBoundsTop":9000,"PreviewBoundsRight":5200,"PreviewBoundsBottom":9500,"PreviewBoundsWidth":4000,"PreviewBoundsHeight":500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":9000,"Width":4000,"Height":500}]}}}
""";
    }

    private static string BuildUnplacedLabelObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"label","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildUnplacedLabelObjectPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":700,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":2400,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"label","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildDeletedLabelObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedLabelObjectPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1400,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5000,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildRestoreLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":13,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"deleted-footer-guid"},{"Name":"HPOS","Value":"1700"},{"Name":"VPOS","Value":"2800"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1700,"PreviewBoundsTop":2800,"PreviewBoundsRight":5300,"PreviewBoundsBottom":3400,"PreviewBoundsWidth":3600,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1700,"Top":2800,"Width":3600,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeleteLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Deleted":true,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"live-detail-guid"},{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1500,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":5500,"DeletedPreviewBoundsBottom":3100,"DeletedPreviewBoundsWidth":4000,"DeletedPreviewBoundsHeight":500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[{"Id":"deleted_detail","Title":"Detail","BandKind":"detail","RecordIndex":52,"Deleted":true,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2600,"Width":4000,"Height":500}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildRenameDeletedLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"deleted-footer-renamed-guid"},{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1600,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5200,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildReorderBackLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3600"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"last.value"}]},{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2600,"PreviewBoundsRight":6800,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5400,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":2500,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":8,"ObjectKind":"label","Title":"last.value","Expression":"last.value","Left":3600,"Top":2600,"Width":3200,"Height":600},{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2600,"Width":3400,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDuplicateLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":10,"Title":"middle.value.copy","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-copy-guid"},{"Name":"HPOS","Value":"2200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1200,"PreviewBoundsTop":2600,"PreviewBoundsRight":6200,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5000,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":1200,"Top":2600,"Width":4000,"Height":600},{"RecordIndex":10,"ObjectKind":"label","Title":"middle.value.copy","Expression":"middle.value","Left":2200,"Top":2600,"Width":4000,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedReportObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedReportObjectPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1400,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5000,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeleteReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Deleted":true,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"live-detail-guid"},{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1500,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":5500,"DeletedPreviewBoundsBottom":3100,"DeletedPreviewBoundsWidth":4000,"DeletedPreviewBoundsHeight":500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[{"Id":"deleted_detail","Title":"Detail","BandKind":"detail","RecordIndex":52,"Deleted":true,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2600,"Width":4000,"Height":500}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildRestoreReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":13,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"deleted-footer-guid"},{"Name":"HPOS","Value":"1700"},{"Name":"VPOS","Value":"2800"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1700,"PreviewBoundsTop":2800,"PreviewBoundsRight":5300,"PreviewBoundsBottom":3400,"PreviewBoundsWidth":3600,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1700,"Top":2800,"Width":3600,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildRenameReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-renamed-guid"},{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1200,"PreviewBoundsTop":2600,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":4000,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":1200,"Top":2600,"Width":4000,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildReorderFrontReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3200"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"2700"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":5900,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4900,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":2100,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":8,"ObjectKind":"field","Title":"last.value","Expression":"last.value","Left":3200,"Top":2400,"Width":2700,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDuplicateReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":10,"Title":"middle.value.copy","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-copy-guid"},{"Name":"HPOS","Value":"2200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1200,"PreviewBoundsTop":2600,"PreviewBoundsRight":6200,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5000,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":1200,"Top":2600,"Width":4000,"Height":600},{"RecordIndex":10,"ObjectKind":"field","Title":"middle.value.copy","Expression":"middle.value","Left":2200,"Top":2600,"Width":4000,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedReportSectionUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"9700"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9300,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1400,"Top":9700,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedLabelSectionUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"9700"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9300,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1400,"Top":9700,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static void CreateFakeStudioHostScript(string scriptPath, string responseJson)
    {
        var script = string.Join(
            "\n",
            "#!/usr/bin/env bash",
            "set -e",
            "log_file=\"${COPPERFIN_SMOKE_LOG:?}\"",
            "{",
            "  printf '%s\\n' 'BEGIN'",
            "  for arg in \"$@\"; do",
            "    printf '%s\\n' \"$arg\"",
            "  done",
            "} >> \"$log_file\"",
            "cat <<'JSON'",
            responseJson,
            "JSON",
            string.Empty);

        File.WriteAllText(scriptPath, script);
        MakeExecutable(scriptPath);
    }

    private static void MakeExecutable(string path)
    {
        using var process = new Process
        {
            StartInfo = new ProcessStartInfo
            {
                FileName = "/bin/chmod",
                Arguments = $"+x \"{path}\"",
                UseShellExecute = false,
                RedirectStandardError = true,
                RedirectStandardOutput = true,
                CreateNoWindow = true
            }
        };

        if (!process.Start())
        {
            throw new InvalidOperationException("Could not start chmod for the fake Studio host script.");
        }

        var stdout = process.StandardOutput.ReadToEnd();
        var stderr = process.StandardError.ReadToEnd();
        process.WaitForExit();
        if (process.ExitCode != 0)
        {
            throw new InvalidOperationException(string.IsNullOrWhiteSpace(stderr) ? stdout : stderr);
        }
    }

    private static string BuildGuidanceText(CopperfinAssetEditorControl control, string assetFamily)
    {
        var method = typeof(CopperfinAssetEditorControl).GetMethod("BuildGuidanceText", BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException("Could not find CopperfinAssetEditorControl guidance smoke hook.");
        }

        return (string)(method.Invoke(control, new object[] { assetFamily }) ?? string.Empty);
    }

    private static CopperfinStudioSnapshotDocument BuildStatusSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            FieldCount = 7,
            IndexCount = 3,
            CommandUndoAvailable = true,
            CommandUndoLabel = "Reordenar",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new(),
                new()
            }
        };
    }

    private static string InvokeAssetEditorString(CopperfinAssetEditorControl control, string methodName, params object[] args)
    {
        var method = typeof(CopperfinAssetEditorControl).GetMethod(methodName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException($"Could not find CopperfinAssetEditorControl smoke hook {methodName}.");
        }

        return (string)(method.Invoke(control, args) ?? string.Empty);
    }

    private static void InvokeAssetEditorVoid(CopperfinAssetEditorControl control, string methodName, params object?[] args)
    {
        var method = typeof(CopperfinAssetEditorControl).GetMethod(methodName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException($"Could not find CopperfinAssetEditorControl smoke hook {methodName}.");
        }

        method.Invoke(control, args);
    }

    private static object? InvokeAssetEditorObject(CopperfinAssetEditorControl control, string methodName, params object?[] args)
    {
        var method = typeof(CopperfinAssetEditorControl).GetMethod(methodName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException($"Could not find CopperfinAssetEditorControl smoke hook {methodName}.");
        }

        return method.Invoke(control, args);
    }

    private static string InvokeDesignSurfaceString(CopperfinDesignSurfaceControl surface, string methodName, params object[] args)
    {
        var method = typeof(CopperfinDesignSurfaceControl).GetMethod(methodName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException($"Could not find CopperfinDesignSurfaceControl smoke hook {methodName}.");
        }

        return (string)(method.Invoke(surface, args) ?? string.Empty);
    }

    private static float InvokeDesignSurfaceFloat(CopperfinDesignSurfaceControl surface, string methodName, params object[] args)
    {
        var method = typeof(CopperfinDesignSurfaceControl).GetMethod(methodName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException($"Could not find CopperfinDesignSurfaceControl smoke hook {methodName}.");
        }

        return method.Invoke(surface, args) is float value
            ? value
            : throw new InvalidOperationException($"Could not read float result from CopperfinDesignSurfaceControl smoke hook {methodName}.");
    }

    private static void ClickDesignSurface(CopperfinDesignSurfaceControl surface, Point location)
    {
        var method = typeof(CopperfinDesignSurfaceControl).GetMethod("OnMouseDown", BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException("Could not find shared report design-surface mouse hook.");
        }

        method.Invoke(surface, new object[] { new MouseEventArgs(MouseButtons.Left, 1, location.X, location.Y, 0) });
    }

    private static void DragDesignSurface(CopperfinDesignSurfaceControl surface, Point start, int deltaX, int deltaY)
    {
        var mouseDown = typeof(CopperfinDesignSurfaceControl).GetMethod("OnMouseDown", BindingFlags.Instance | BindingFlags.NonPublic);
        var mouseMove = typeof(CopperfinDesignSurfaceControl).GetMethod("OnMouseMove", BindingFlags.Instance | BindingFlags.NonPublic);
        var mouseUp = typeof(CopperfinDesignSurfaceControl).GetMethod("OnMouseUp", BindingFlags.Instance | BindingFlags.NonPublic);
        if (mouseDown is null || mouseMove is null || mouseUp is null)
        {
            throw new InvalidOperationException("Could not find shared report design-surface drag hooks.");
        }

        mouseDown.Invoke(surface, new object[] { new MouseEventArgs(MouseButtons.Left, 1, start.X, start.Y, 0) });
        mouseMove.Invoke(surface, new object[] { new MouseEventArgs(MouseButtons.Left, 0, start.X + deltaX, start.Y + deltaY, 0) });
        mouseUp.Invoke(surface, new object[] { new MouseEventArgs(MouseButtons.Left, 1, start.X + deltaX, start.Y + deltaY, 0) });
    }

    private static void RenderDesignSurface(CopperfinDesignSurfaceControl surface)
    {
        using var bitmap = new Bitmap(surface.Width, surface.Height);
        surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
    }

    private static Rectangle ReadSurfaceObjectRectangle(CopperfinDesignSurfaceControl surface, int index)
    {
        var field = typeof(CopperfinDesignSurfaceControl).GetField("objects", BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(surface) is not System.Collections.IList objects || objects.Count <= index)
        {
            throw new InvalidOperationException("Could not read shared report surface objects.");
        }

        var item = objects[index]!;
        var property = item.GetType().GetProperty("PixelBounds", BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(item) is not Rectangle value)
        {
            throw new InvalidOperationException("Could not read shared report surface object bounds.");
        }

        return value;
    }

    private static int ReadPrivateListCount(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(instance) is not System.Collections.ICollection collection)
        {
            throw new InvalidOperationException($"Could not read private list field {fieldName}.");
        }

        return collection.Count;
    }

    private static int? ReadPrivateNullableInt(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field is null)
        {
            throw new InvalidOperationException($"Could not read private nullable int field {fieldName}.");
        }

        return field.GetValue(instance) as int?;
    }

    private static bool ReadPrivateBoolField(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(instance) is not bool value)
        {
            throw new InvalidOperationException($"Could not read private bool field {fieldName}.");
        }

        return value;
    }

    private static string ReadPrivateStringField(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(instance) is not string value)
        {
            throw new InvalidOperationException($"Could not read private string field {fieldName}.");
        }

        return value;
    }

    private static int ReadReportSectionProperty(CopperfinDesignSurfaceControl surface, int index, string propertyName)
    {
        var section = ReadReportSectionVisual(surface, index);
        var property = section.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(section) is not int value)
        {
            throw new InvalidOperationException($"Could not read report-section property {propertyName}.");
        }

        return value;
    }

    private static string ReadReportSectionPropertyText(CopperfinDesignSurfaceControl surface, int index, string propertyName)
    {
        var section = ReadReportSectionVisual(surface, index);
        var property = section.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(section) is not string value)
        {
            throw new InvalidOperationException($"Could not read report-section text property {propertyName}.");
        }

        return value;
    }

    private static bool ReadReportSectionPropertyBool(CopperfinDesignSurfaceControl surface, int index, string propertyName)
    {
        var section = ReadReportSectionVisual(surface, index);
        var property = section.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(section) is not bool value)
        {
            throw new InvalidOperationException($"Could not read report-section boolean property {propertyName}.");
        }

        return value;
    }

    private static Rectangle ReadReportSectionRectangle(CopperfinDesignSurfaceControl surface, int index, string propertyName)
    {
        var section = ReadReportSectionVisual(surface, index);
        var property = section.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(section) is not Rectangle value)
        {
            throw new InvalidOperationException($"Could not read report-section rectangle property {propertyName}.");
        }

        return value;
    }

    private static object ReadReportSectionVisual(CopperfinDesignSurfaceControl surface, int index)
    {
        var field = typeof(CopperfinDesignSurfaceControl).GetField("reportSections", BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(surface) is not System.Collections.IList sections || sections.Count <= index)
        {
            throw new InvalidOperationException("Could not read shared report-section visuals.");
        }

        return sections[index]!;
    }

    private static Rectangle ReadPrivateRectangle(object instance, string fieldName)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(instance) is not Rectangle rectangle)
        {
            throw new InvalidOperationException($"Could not read private rectangle field {fieldName}.");
        }

        return rectangle;
    }

    private static Point GetCenter(Rectangle rectangle)
    {
        return new Point(rectangle.Left + (rectangle.Width / 2), rectangle.Top + (rectangle.Height / 2));
    }

    private static CopperfinDesignSurfaceControl? FindDesignSurface(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is CopperfinDesignSurfaceControl surface)
            {
                return surface;
            }

            var nested = FindDesignSurface(child);
            if (nested is not null)
            {
                return nested;
            }
        }

        return null;
    }

    private static ListView GetPrivateListView(CopperfinAssetEditorControl control, string fieldName)
    {
        var field = typeof(CopperfinAssetEditorControl).GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(control) is not ListView listView)
        {
            throw new InvalidOperationException($"Could not read private list view {fieldName}.");
        }

        return listView;
    }

    private static string CreateSmokeAssetFile(string tempRoot, string fileName)
    {
        var assetPath = Path.Combine(tempRoot, fileName);
        File.WriteAllText(assetPath, string.Empty);
        return assetPath;
    }

    private static Label GetPrivateLabel(CopperfinAssetEditorControl control, string fieldName)
    {
        var field = typeof(CopperfinAssetEditorControl).GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(control) is not Label label)
        {
            throw new InvalidOperationException($"Could not read private label {fieldName}.");
        }

        return label;
    }

    private static Button GetPrivateButton(CopperfinAssetEditorControl control, string fieldName)
    {
        var field = typeof(CopperfinAssetEditorControl).GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(control) is not Button button)
        {
            throw new InvalidOperationException($"Could not read private button {fieldName}.");
        }

        return button;
    }

    private static PropertyGrid GetPrivatePropertyGrid(CopperfinAssetEditorControl control)
    {
        var field = typeof(CopperfinAssetEditorControl).GetField("propertyGrid", BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(control) is not PropertyGrid propertyGrid)
        {
            throw new InvalidOperationException("Could not read private property grid.");
        }

        return propertyGrid;
    }

    private static void SetCurrentSnapshot(CopperfinAssetEditorControl control, CopperfinStudioSnapshotDocument snapshot)
    {
        var field = typeof(CopperfinAssetEditorControl).GetField("currentSnapshot", BindingFlags.Instance | BindingFlags.NonPublic);
        if (field is null)
        {
            throw new InvalidOperationException("Could not set private currentSnapshot.");
        }

        field.SetValue(control, snapshot);
    }

    private static void SetPrivateField(object instance, string fieldName, object? value)
    {
        var field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field is null)
        {
            throw new InvalidOperationException($"Could not set private field {fieldName}.");
        }

        field.SetValue(instance, value);
    }

    private static IEnumerable<Label> FindLabels(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is Label label)
            {
                yield return label;
            }

            foreach (var nested in FindLabels(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasLabelText(Control root, string text)
    {
        foreach (var label in FindLabels(root))
        {
            if (string.Equals(label.Text, text, StringComparison.Ordinal))
            {
                return true;
            }
        }

        return false;
    }

    private static bool HasLabelTextContaining(Control root, string text)
    {
        foreach (var label in FindLabels(root))
        {
            if (label.Text.IndexOf(text, StringComparison.Ordinal) >= 0)
            {
                return true;
            }
        }

        return false;
    }

    private static IEnumerable<RichTextBox> FindRichTextBoxes(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is RichTextBox textBox)
            {
                yield return textBox;
            }

            foreach (var nested in FindRichTextBoxes(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasRichTextBoxTextContaining(Control root, string text)
    {
        foreach (var textBox in FindRichTextBoxes(root))
        {
            if (textBox.Text.IndexOf(text, StringComparison.Ordinal) >= 0)
            {
                return true;
            }
        }

        return false;
    }

    private static IEnumerable<TabControl> FindTabControls(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is TabControl tabControl)
            {
                yield return tabControl;
            }

            foreach (var nested in FindTabControls(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasTabPageText(Control root, string text)
    {
        foreach (var tabControl in FindTabControls(root))
        {
            foreach (TabPage tabPage in tabControl.TabPages)
            {
                if (string.Equals(tabPage.Text, text, StringComparison.Ordinal))
                {
                    return true;
                }
            }
        }

        return false;
    }

    private static int CountNonWhitePixels(Bitmap bitmap)
    {
        var count = 0;
        for (var y = 0; y < bitmap.Height; y += 2)
        {
            for (var x = 0; x < bitmap.Width; x += 2)
            {
                if (bitmap.GetPixel(x, y).ToArgb() != Color.White.ToArgb())
                {
                    count++;
                }
            }
        }

        return count;
    }

    private static IEnumerable<CheckBox> FindCheckBoxes(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is CheckBox checkBox)
            {
                yield return checkBox;
            }

            foreach (var nested in FindCheckBoxes(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasCheckBoxText(Control root, string text)
    {
        foreach (var checkBox in FindCheckBoxes(root))
        {
            if (string.Equals(checkBox.Text, text, StringComparison.Ordinal))
            {
                return true;
            }
        }

        return false;
    }

    private static IEnumerable<Button> FindButtons(Control root)
    {
        foreach (Control child in root.Controls)
        {
            if (child is Button button)
            {
                yield return button;
            }

            foreach (var nested in FindButtons(child))
            {
                yield return nested;
            }
        }
    }

    private static bool HasButtonText(Control root, string text)
    {
        foreach (var button in FindButtons(root))
        {
            if (string.Equals(button.Text, text, StringComparison.Ordinal))
            {
                return true;
            }
        }

        return false;
    }

    private static void ExpectSelectionUpdate(CopperfinDesignerSelection selection, string propertyName, object value, string expectedSerializedValue, string message)
    {
        TypeDescriptor.GetProperties(selection)[propertyName]?.SetValue(selection, value);
        Expect(selection.TryGetUpdate(propertyName, out var targetName, out var serializedValue) &&
               string.Equals(targetName, propertyName, StringComparison.Ordinal) &&
               string.Equals(serializedValue, expectedSerializedValue, StringComparison.Ordinal),
            message);
    }

    private static void Expect(bool condition, string message)
    {
        if (condition)
        {
            Console.WriteLine("PASS: " + message);
            return;
        }

        Console.Error.WriteLine("FAIL: " + message);
        failures++;
    }
}
