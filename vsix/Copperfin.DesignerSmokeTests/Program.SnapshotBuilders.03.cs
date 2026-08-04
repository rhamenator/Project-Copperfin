
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
    private static CopperfinStudioSnapshotDocument BuildAssetEditorGroupingUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            ReportLayout = new CopperfinStudioReportLayout
            {
                Groupings = new List<CopperfinStudioReportGrouping>
                {
                    new()
                    {
                        GroupingIndex = 1,
                        NestingDepth = 2,
                        Expression = "customer.country",
                        ExpressionFieldIndex = 2,
                        ExpressionMemoBlockNumber = 7,
                        HeaderSectionId = "group_header_7",
                        HeaderRecordIndex = 41,
                        HeaderDeleted = false,
                        FooterSectionId = "group_footer_7",
                        FooterRecordIndex = 47,
                        FooterDeleted = true
                    }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorSettingsUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Customer Invoice",
                PreviewBoundsAvailable = true,
                PreviewBoundsLeft = 0,
                PreviewBoundsTop = 2000,
                PreviewBoundsRight = 5200,
                PreviewBoundsBottom = 8100,
                PreviewBoundsWidth = 5200,
                PreviewBoundsHeight = 6100,
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                Settings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "0", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 9 },
                    new() { Name = "COLS", Value = "2", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 9 },
                    new() { Name = "COLWIDTH", Value = "3600", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 9 },
                    new() { Name = "COLSPACING", Value = "120", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 9 },
                    new() { Name = "PAPERLENGTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 8 },
                    new() { Name = "PAPERWIDTH", Value = "2159", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 8 },
                    new() { Name = "TOPMARGIN", Value = "20", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "15", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "25", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 11 },
                    new() { Name = "DRIVER", Value = "winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 8 },
                    new() { Name = "DEVICE", Value = "FinePrint 2000", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 8 },
                    new() { Name = "OUTPUT", Value = "FPR4:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 8 },
                    new() { Name = "DEFAULTSOURCE", Value = "15", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 8 },
                    new() { Name = "PRINTQUALITY", Value = "600", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 8 },
                    new() { Name = "YRESOLUTION", Value = "600", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 8 },
                    new() { Name = "TTOPTION", Value = "3", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 8 },
                    new() { Name = "COLOR", Value = "1", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 8 },
                    new() { Name = "ASCII", Value = "9", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 8 },
                    new() { Name = "COLLATE", Value = "1", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 8 },
                    new() { Name = "COPIES", Value = "1", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 8 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsUpdateSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingMarginsSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingOrientationSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingPaperLengthSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingPaperSizeSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingGridVSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "PAPERSIZE", Value = "9", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 4, MemoBlockNumber = 19 },
                    new() { Name = "GRIDH", Value = "10", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 5, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingGridHSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "PAPERSIZE", Value = "9", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 4, MemoBlockNumber = 19 },
                    new() { Name = "GRIDV", Value = "6", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 5, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingPaperWidthSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingLeftMarginSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingRightMarginSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingTagSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingDefaultSourceSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DRIVER", Value = "deleted.winspool", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 10, MemoBlockNumber = 18 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingDriverSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            FieldCount = 5,
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                DeletedPreviewBoundsAvailable = true,
                DeletedPreviewBoundsLeft = 1000,
                DeletedPreviewBoundsTop = 2600,
                DeletedPreviewBoundsRight = 2200,
                DeletedPreviewBoundsBottom = 2900,
                DeletedPreviewBoundsWidth = 1200,
                DeletedPreviewBoundsHeight = 300,
                DeletedSettings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
                    new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
                    new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
                    new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
                    new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
                    new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
                    new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
                    new() { Name = "LEFTMARGIN", Value = "35", RecordIndex = 0, FieldIndex = 10, MemoBlockNumber = 0 },
                    new() { Name = "RIGHTMARGIN", Value = "45", RecordIndex = 0, FieldIndex = 11, MemoBlockNumber = 0 },
                    new() { Name = "TAG", Value = "deleted.customer.country", RecordIndex = 0, FieldIndex = 9, MemoBlockNumber = 21 },
                    new() { Name = "DEVICE", Value = "Deleted Printer", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 11, MemoBlockNumber = 18 },
                    new() { Name = "OUTPUT", Value = "DPRN:", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 12, MemoBlockNumber = 18 },
                    new() { Name = "DEFAULTSOURCE", Value = "16", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 13, MemoBlockNumber = 18 },
                    new() { Name = "PRINTQUALITY", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 14, MemoBlockNumber = 18 },
                    new() { Name = "YRESOLUTION", Value = "1200", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 15, MemoBlockNumber = 18 },
                    new() { Name = "TTOPTION", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 16, MemoBlockNumber = 18 },
                    new() { Name = "COLOR", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 17, MemoBlockNumber = 18 },
                    new() { Name = "ASCII", Value = "10", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 18, MemoBlockNumber = 18 },
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 },
                    new() { Name = "COPIES", Value = "2", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 18 }
                },
                Sections = new List<CopperfinStudioReportSection>
                {
                    new()
                    {
                        Id = "detail_1",
                        Title = "Detail",
                        BandKind = "detail",
                        RecordIndex = 42,
                        Top = 2000,
                        Height = 5000
                    }
                }
            }
        };
    }

}
