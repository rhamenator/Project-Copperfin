
// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
    private static int Main(string[] args)
    {
        DesignerSmokeTestRunner runner = new DesignerSmokeTestRunner(args);
        if (runner.ShouldInitializeUi)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
        }

        if (TryRunResolvedRealAssetCoveragePart(runner, args))
        {
            return runner.Finish();
        }

        runner.Run(nameof(SmokeDesignSurfaceWithSyntheticReportLayout), SmokeDesignSurfaceWithSyntheticReportLayout);
        runner.Run(nameof(SmokeInvariantReportGeometryParsing), SmokeInvariantReportGeometryParsing);
        runner.Run(nameof(SmokeLocalizedReportDesignSurfaceContext), SmokeLocalizedReportDesignSurfaceContext);
        runner.Run(nameof(SmokeLocalizedAssetEditorChrome), SmokeLocalizedAssetEditorChrome);
        runner.Run(nameof(SmokePseudoLocalizedAssetEditorChrome), SmokePseudoLocalizedAssetEditorChrome);
        runner.Run(nameof(SmokeLocalizedHostModeChromeCompaction), SmokeLocalizedHostModeChromeCompaction);
        runner.Run(nameof(SmokeVisualStudioHostSurfaceThemeContract), SmokeVisualStudioHostSurfaceThemeContract);
        runner.Run(nameof(SmokeProjectWorkflowWarningParsingLocalization), SmokeProjectWorkflowWarningParsingLocalization);
        runner.Run(nameof(SmokeManagedProjectProcessLaunchContracts), SmokeManagedProjectProcessLaunchContracts);
        runner.Run(nameof(SmokeStandaloneStudioDocumentIdentity), SmokeStandaloneStudioDocumentIdentity);
        runner.Run(nameof(SmokeStandaloneStudioRevisitingDocumentPreservesSelectors), SmokeStandaloneStudioRevisitingDocumentPreservesSelectors);
        runner.Run(nameof(SmokeStandaloneStudioCloseDocumentTabs), SmokeStandaloneStudioCloseDocumentTabs);
        runner.Run(nameof(SmokeStandaloneStudioCommandWindowDocking), SmokeStandaloneStudioCommandWindowDocking);
        runner.Run(nameof(SmokeStandaloneStudioToolWindowFloating), SmokeStandaloneStudioToolWindowFloating);
        runner.Run(nameof(SmokeStandaloneStudioCommandWindowInteraction), SmokeStandaloneStudioCommandWindowInteraction);
        runner.Run(nameof(SmokeStandaloneStudioShellLayoutPersistence), SmokeStandaloneStudioShellLayoutPersistence);
        runner.Run(nameof(SmokeStandaloneStudioFileLayoutStoreRoundTrip), SmokeStandaloneStudioFileLayoutStoreRoundTrip);
        runner.Run(nameof(SmokeStandaloneStudioTerminalShellContract), SmokeStandaloneStudioTerminalShellContract);
        runner.Run(nameof(SmokeStandaloneStudioTerminalWindow), SmokeStandaloneStudioTerminalWindow);
        runner.Run(nameof(SmokeProjectWorkspaceEntryActivation), SmokeProjectWorkspaceEntryActivation);
        runner.Run(nameof(SmokeCrossPlatformFileRevealContracts), SmokeCrossPlatformFileRevealContracts);
        runner.Run(nameof(SmokeLocalizedProjectWorkspaceChrome), SmokeLocalizedProjectWorkspaceChrome);
        runner.Run(nameof(SmokeLocalizedProjectCommandDebuggerChrome), SmokeLocalizedProjectCommandDebuggerChrome);
        runner.Run(nameof(SmokeDebuggerDetailTablesMirrorPauseState), SmokeDebuggerDetailTablesMirrorPauseState);
        runner.Run(nameof(SmokeLocalizedProjectWorkspacePlaceholders), SmokeLocalizedProjectWorkspacePlaceholders);
        runner.Run(nameof(SmokeManagedToolboxPaletteContract), SmokeManagedToolboxPaletteContract);
        runner.Run(nameof(SmokeLocalizedProjectWorkspaceBooleanValues), SmokeLocalizedProjectWorkspaceBooleanValues);
        runner.Run(nameof(SmokeLocalizedExplorerColumnHeaders), SmokeLocalizedExplorerColumnHeaders);
        runner.Run(nameof(SmokeLocalizedAssetFamilyGuidance), SmokeLocalizedAssetFamilyGuidance);
        runner.Run(nameof(SmokeLocalizedReportLayoutShellSummary), SmokeLocalizedReportLayoutShellSummary);
        runner.Run(nameof(SmokeLocalizedSnapshotUndoPropertyStatus), SmokeLocalizedSnapshotUndoPropertyStatus);
        runner.Run(nameof(SmokeLocalizedLaunchWorkflowDialogText), SmokeLocalizedLaunchWorkflowDialogText);
        runner.Run(nameof(SmokeReportSectionGroupingExplorerTitles), SmokeReportSectionGroupingExplorerTitles);
        runner.Run(nameof(SmokeReportSectionScopedObjectFiltering), SmokeReportSectionScopedObjectFiltering);
        runner.Run(nameof(SmokeHostShapedUnplacedReportObjectDeserialization), SmokeHostShapedUnplacedReportObjectDeserialization);
        runner.Run(nameof(SmokeHostShapedReportAndLabelSectionOrdinals), SmokeHostShapedReportAndLabelSectionOrdinals);
        runner.Run(nameof(SmokeReportSectionPropertyGridSelection), SmokeReportSectionPropertyGridSelection);
        runner.Run(nameof(SmokeReportGroupingExplorerSelection), SmokeReportGroupingExplorerSelection);
        runner.Run(nameof(SmokeReportSettingsExplorerSelection), SmokeReportSettingsExplorerSelection);
        runner.Run(nameof(SmokeDeletedReportSettingsExplorerSelection), SmokeDeletedReportSettingsExplorerSelection);
        runner.Run(nameof(SmokeAssetEditorReportGroupingPropertyGridHostUpdate), SmokeAssetEditorReportGroupingPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorReportSectionPropertyGridHostUpdate), SmokeAssetEditorReportSectionPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorReportSettingsPropertyGridHostUpdate), SmokeAssetEditorReportSettingsPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsPropertyGridHostUpdate), SmokeAssetEditorDeletedReportSettingsPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingOrientationHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingOrientationHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingPaperSizeHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingPaperSizeHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingGridVHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingGridVHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingGridHHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingGridHHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingPaperLengthHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingPaperLengthHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingPaperWidthHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingPaperWidthHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingLeftMarginHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingLeftMarginHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingRightMarginHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingRightMarginHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingTopBottomMarginHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingTopBottomMarginHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingTagHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingTagHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingDefaultSourceHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingDefaultSourceHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingDriverHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingDriverHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingDeviceHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingDeviceHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingOutputHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingOutputHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingPrintQualityHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingPrintQualityHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingYResolutionHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingYResolutionHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingTTOptionHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingTTOptionHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingAsciiHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingAsciiHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingCollateHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingCollateHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingCopiesHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingCopiesHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingColorHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingColorHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingColSpacingHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingColSpacingHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingColWidthHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingColWidthHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSettingsMissingColsHostUpdate), SmokeAssetEditorDeletedReportSettingsMissingColsHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedReportSectionPropertyGridHostUpdate), SmokeAssetEditorDeletedReportSectionPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorLabelSectionPropertyGridHostUpdate), SmokeAssetEditorLabelSectionPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedLabelSectionPropertyGridHostUpdate), SmokeAssetEditorDeletedLabelSectionPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorReportObjectPropertyGridHostUpdate), SmokeAssetEditorReportObjectPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorUnplacedReportObjectPropertyGridHostUpdate), SmokeAssetEditorUnplacedReportObjectPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorUnplacedReportObjectHostUpdateRefreshesShellSummary), SmokeAssetEditorUnplacedReportObjectHostUpdateRefreshesShellSummary);
        runner.Run(nameof(SmokeAssetEditorUnplacedReportObjectPlacementIntoSectionRefreshesContinuity), SmokeAssetEditorUnplacedReportObjectPlacementIntoSectionRefreshesContinuity);
        runner.Run(nameof(SmokeAssetEditorDeletedReportObjectHostUpdateRefreshesShellSummary), SmokeAssetEditorDeletedReportObjectHostUpdateRefreshesShellSummary);
        runner.Run(nameof(SmokeAssetEditorUndoRefreshesReportShellSummary), SmokeAssetEditorUndoRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorUndoRefreshesDeletedReportShellSummary), SmokeAssetEditorUndoRefreshesDeletedReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorDuplicateObjectCommandRefreshesReportShellSummary), SmokeAssetEditorDuplicateObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorRenameObjectCommandRefreshesReportShellSummary), SmokeAssetEditorRenameObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorReorderFrontObjectCommandRefreshesReportShellSummary), SmokeAssetEditorReorderFrontObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorAlignLeftObjectCommandRefreshesReportShellSummary), SmokeAssetEditorAlignLeftObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorAlignTopObjectCommandRefreshesReportShellSummary), SmokeAssetEditorAlignTopObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorMatchWidthObjectCommandRefreshesReportShellSummary), SmokeAssetEditorMatchWidthObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorMatchHeightObjectCommandRefreshesReportShellSummary), SmokeAssetEditorMatchHeightObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorMatchSizeObjectCommandRefreshesReportShellSummary), SmokeAssetEditorMatchSizeObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorDistributeHorizontallyObjectCommandRefreshesReportShellSummary), SmokeAssetEditorDistributeHorizontallyObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorDistributeVerticallyObjectCommandRefreshesReportShellSummary), SmokeAssetEditorDistributeVerticallyObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorSnapHorizontallyObjectCommandRefreshesReportShellSummary), SmokeAssetEditorSnapHorizontallyObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorSnapVerticallyObjectCommandRefreshesReportShellSummary), SmokeAssetEditorSnapVerticallyObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorSnapToGridObjectCommandRefreshesReportShellSummary), SmokeAssetEditorSnapToGridObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorDeleteObjectCommandRefreshesReportShellSummary), SmokeAssetEditorDeleteObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorRestoreObjectCommandRefreshesReportShellSummary), SmokeAssetEditorRestoreObjectCommandRefreshesReportShellSummary);
        runner.Run(nameof(SmokeAssetEditorLabelObjectPropertyGridHostUpdate), SmokeAssetEditorLabelObjectPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorLabelObjectPlacementIntoUnplacedRefreshesContinuity), SmokeAssetEditorLabelObjectPlacementIntoUnplacedRefreshesContinuity);
        runner.Run(nameof(SmokeAssetEditorUnplacedLabelObjectPropertyGridHostUpdate), SmokeAssetEditorUnplacedLabelObjectPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorUnplacedLabelObjectHostUpdateRefreshesShellSummary), SmokeAssetEditorUnplacedLabelObjectHostUpdateRefreshesShellSummary);
        runner.Run(nameof(SmokeAssetEditorDeletedLabelObjectHostUpdateRefreshesShellSummary), SmokeAssetEditorDeletedLabelObjectHostUpdateRefreshesShellSummary);
        runner.Run(nameof(SmokeAssetEditorUndoRefreshesLabelShellSummary), SmokeAssetEditorUndoRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorUndoRefreshesDeletedLabelShellSummary), SmokeAssetEditorUndoRefreshesDeletedLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorDuplicateObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorDuplicateObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorRenameObjectCommandRefreshesDeletedLabelShellSummary), SmokeAssetEditorRenameObjectCommandRefreshesDeletedLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorReorderBackObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorReorderBackObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorAlignLeftObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorAlignLeftObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorAlignTopObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorAlignTopObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorMatchWidthObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorMatchWidthObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorMatchHeightObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorMatchHeightObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorMatchSizeObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorMatchSizeObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorDistributeHorizontallyObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorDistributeHorizontallyObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorDistributeVerticallyObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorDistributeVerticallyObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorSnapHorizontallyObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorSnapHorizontallyObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorSnapVerticallyObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorSnapVerticallyObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorSnapToGridObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorSnapToGridObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorDeleteObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorDeleteObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorRestoreObjectCommandRefreshesLabelShellSummary), SmokeAssetEditorRestoreObjectCommandRefreshesLabelShellSummary);
        runner.Run(nameof(SmokeAssetEditorDeletedReportObjectPropertyGridHostUpdate), SmokeAssetEditorDeletedReportObjectPropertyGridHostUpdate);
        runner.Run(nameof(SmokeAssetEditorDeletedLabelObjectPropertyGridHostUpdate), SmokeAssetEditorDeletedLabelObjectPropertyGridHostUpdate);
        runner.Run(nameof(SmokeReportObjectPropertyGridLocalization), SmokeReportObjectPropertyGridLocalization);
        runner.Run(nameof(SmokeReportVariableInitialValuePropertyGrid), SmokeReportVariableInitialValuePropertyGrid);
        runner.Run(nameof(SmokeSharedDesignerSelectionLocalization), SmokeSharedDesignerSelectionLocalization);
        runner.Run(nameof(SmokeLocalizedCodeReferenceKindLabels), SmokeLocalizedCodeReferenceKindLabels);
        runner.Run(nameof(SmokeLocalizedProjectInsightArtifactKindLabels), SmokeLocalizedProjectInsightArtifactKindLabels);
        runner.Run(nameof(SmokeLocalizedBuilderSummaryArtifactKindLabels), SmokeLocalizedBuilderSummaryArtifactKindLabels);
        runner.Run(nameof(SmokeLocalizedWorkspaceGroupTitles), SmokeLocalizedWorkspaceGroupTitles);
        runner.Run(nameof(SmokeLocalizedProjectWorkspaceExplorerGroupTitles), SmokeLocalizedProjectWorkspaceExplorerGroupTitles);
        runner.Run(nameof(SmokeLocalizedProjectFallbackKindAndGroupLabels), SmokeLocalizedProjectFallbackKindAndGroupLabels);
        runner.Run(nameof(SmokeLocalizedReportObjectKindSubtitles), SmokeLocalizedReportObjectKindSubtitles);
        runner.Run(nameof(SmokeLocalizedReportObjectFallbackTitles), SmokeLocalizedReportObjectFallbackTitles);
        runner.Run(nameof(SmokeReportSelectionPreservedAcrossExplorerRefresh), SmokeReportSelectionPreservedAcrossExplorerRefresh);
        runner.Run(nameof(SmokeDeletedReportSectionExplorerSelection), SmokeDeletedReportSectionExplorerSelection);
        runner.Run(nameof(SmokeReportSurfaceScopeSelection), SmokeReportSurfaceScopeSelection);
        runner.Run(nameof(SmokeReportSurfaceObjectScopeAlignment), SmokeReportSurfaceObjectScopeAlignment);
        runner.Run(nameof(SmokeReportSurfaceDeletedLiveSectionObjectScopeAlignment), SmokeReportSurfaceDeletedLiveSectionObjectScopeAlignment);
        runner.Run(nameof(SmokeLabelSurfaceScopeSelection), SmokeLabelSurfaceScopeSelection);
        runner.Run(nameof(SmokeLabelSurfaceObjectScopeAlignment), SmokeLabelSurfaceObjectScopeAlignment);
        runner.Run(nameof(SmokeLabelSurfaceDeletedLiveSectionObjectScopeAlignment), SmokeLabelSurfaceDeletedLiveSectionObjectScopeAlignment);
        runner.Run(nameof(SmokeReportSurfaceObjectDragging), SmokeReportSurfaceObjectDragging);
        runner.Run(nameof(SmokeLabelSurfaceObjectDragging), SmokeLabelSurfaceObjectDragging);
        runner.Run(nameof(SmokeAssetEditorReportDragUsesBatchStudioHostUpdate), SmokeAssetEditorReportDragUsesBatchStudioHostUpdate);
        runner.Run(nameof(SmokeAssetEditorReportDragRefreshesShellSummary), SmokeAssetEditorReportDragRefreshesShellSummary);
        runner.Run(nameof(SmokeAssetEditorLabelDragRefreshesShellSummary), SmokeAssetEditorLabelDragRefreshesShellSummary);
        runner.Run(nameof(SmokeDeletedReportSectionDesignSurfaceRendering), SmokeDeletedReportSectionDesignSurfaceRendering);
        runner.Run(nameof(SmokeFocusedRealAssetEditorDeleteRestoreRoundTrip), SmokeFocusedRealAssetEditorDeleteRestoreRoundTrip);
        runner.Run(nameof(SmokeResolvedRealAssetCoverageCluster), SmokeResolvedRealAssetCoverageCluster);

        return runner.Finish();
        }

    private static bool TryRunResolvedRealAssetCoveragePart(
        DesignerSmokeTestRunner runner,
        string[] args)
    {
        if (args.Length != 2 || !string.Equals(args[0], "--exact", StringComparison.Ordinal))
        {
            return false;
        }

        switch (args[1])
        {
            case nameof(SmokeResolvedRealAssetCoverageClusterPart01):
                runner.Run(args[1], SmokeResolvedRealAssetCoverageClusterPart01);
                return true;
            case nameof(SmokeResolvedRealAssetCoverageClusterPart02):
                runner.Run(args[1], SmokeResolvedRealAssetCoverageClusterPart02);
                return true;
            case nameof(SmokeResolvedRealAssetCoverageClusterPart03):
                runner.Run(args[1], SmokeResolvedRealAssetCoverageClusterPart03);
                return true;
            case nameof(SmokeResolvedRealAssetCoverageClusterPart04):
                runner.Run(args[1], SmokeResolvedRealAssetCoverageClusterPart04);
                return true;
            case nameof(SmokeResolvedRealAssetCoverageClusterPart05):
                runner.Run(args[1], SmokeResolvedRealAssetCoverageClusterPart05);
                return true;
            case nameof(SmokeResolvedRealAssetCoverageClusterPart06):
                runner.Run(args[1], SmokeResolvedRealAssetCoverageClusterPart06);
                return true;
            case nameof(SmokeResolvedRealAssetCoverageClusterPart07):
                runner.Run(args[1], SmokeResolvedRealAssetCoverageClusterPart07);
                return true;
            default:
                return false;
        }
    }

}
