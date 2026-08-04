
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

}
