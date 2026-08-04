
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
    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingDeviceSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingOutputSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingPrintQualitySmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingYResolutionSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingTTOptionSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingAsciiSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingCollateSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingCopiesSmokeSnapshot()
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
                    new() { Name = "COLLATE", Value = "0", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 19, MemoBlockNumber = 18 }
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingColorSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingColSpacingSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingColWidthSmokeSnapshot()
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDeletedSettingsMissingColsSmokeSnapshot()
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

}
