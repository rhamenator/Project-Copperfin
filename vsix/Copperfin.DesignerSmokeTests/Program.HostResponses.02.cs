
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
    private static CopperfinStudioSnapshotDocument BuildAssetEditorAlignTopLabelObjectSmokeSnapshot()
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
                        new() { Name = "VPOS", Value = "2900" },
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
                                Top = 2900,
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorResizeLabelObjectSmokeSnapshot()
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
                        new() { Name = "WIDTH", Value = "4200" },
                        new() { Name = "HEIGHT", Value = "900" },
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
                                Width = 4200,
                                Height = 900
                            }
                        }
                    }
                }
            }
        };
    }

    private static CopperfinStudioSnapshotDocument BuildAssetEditorDistributeLabelObjectSmokeSnapshot()
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
                        new() { Name = "HPOS", Value = "1900" },
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
                                Left = 1900,
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

    private static CopperfinStudioSnapshotDocument BuildAssetEditorSnapLabelObjectSmokeSnapshot()
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
                    Title = "snap.value",
                    Subtitle = "label",
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "UNIQUEID", Value = "snap-field-guid" },
                        new() { Name = "HPOS", Value = "1901" },
                        new() { Name = "VPOS", Value = "2605" },
                        new() { Name = "WIDTH", Value = "3400" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = "snap.value" }
                    }
                }
            },
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Settings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "GRIDH", Value = "12", RecordIndex = 0 },
                    new() { Name = "GRIDV", Value = "12", RecordIndex = 0 }
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
                        Height = 5000,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 7,
                                ObjectKind = "label",
                                Title = "snap.value",
                                Expression = "snap.value",
                                Left = 1901,
                                Top = 2605,
                                Width = 3400,
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

}
