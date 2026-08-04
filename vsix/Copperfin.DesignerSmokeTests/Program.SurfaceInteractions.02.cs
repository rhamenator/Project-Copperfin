
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

    private static void SmokeReportSurfaceDeletedLiveSectionObjectScopeAlignment()
    {
        SmokeSurfaceDeletedLiveSectionObjectScopeAlignment(
            assetFamily: "report",
            expectedSectionTitle: "Detail",
            expectedDeletedObjectTitle: "detail.deleted.total",
            expectedDeletedObjectKind: "field");
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

    private static void SmokeLabelSurfaceDeletedLiveSectionObjectScopeAlignment()
    {
        SmokeSurfaceDeletedLiveSectionObjectScopeAlignment(
            assetFamily: "label",
            expectedSectionTitle: "Detail",
            expectedDeletedObjectTitle: "detail.deleted.total",
            expectedDeletedObjectKind: "label");
    }

    private static void SmokeSurfaceDeletedLiveSectionObjectScopeAlignment(
        string assetFamily,
        string expectedSectionTitle,
        string expectedDeletedObjectTitle,
        string expectedDeletedObjectKind)
    {
        var snapshot = BuildDeletedLiveSectionSurfaceSmokeSnapshot(assetFamily, expectedDeletedObjectTitle, expectedDeletedObjectKind);
        var reportLayout = snapshot.ReportLayout ?? throw new InvalidOperationException("Could not build deleted live-section surface smoke snapshot.");

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

        var surface = FindDesignSurface(control) ?? throw new InvalidOperationException("Could not find shared deleted live-section design surface.");
        using (var bitmap = new Bitmap(surface.Width, surface.Height))
        {
            surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        }

        var objectListView = GetPrivateListView(control, "objectListView");
        var propertyGrid = GetPrivatePropertyGrid(control);

        ClickDesignSurface(surface, GetCenter(ReadSurfaceObjectRectangle(surface, 1)));
        Application.DoEvents();

        Expect(string.Equals(sectionListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedSectionTitle, StringComparison.Ordinal),
            $"Clicking a deleted {assetFamily} object inside a live section should select the containing live section row");
        Expect(string.Equals(objectListView.SelectedItems.Cast<ListViewItem>().FirstOrDefault()?.Text, expectedDeletedObjectTitle, StringComparison.Ordinal),
            $"Clicking a deleted {assetFamily} object inside a live section should select the matching deleted object row");
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection deletedObjectSelection &&
               deletedObjectSelection.RecordIndex == 13,
            $"Clicking a deleted {assetFamily} object inside a live section should keep object-rooted property-grid selection");
        if (propertyGrid.SelectedObject is CopperfinDesignerSelection deletedObjectMetadataSelection)
        {
            Expect(string.Equals(TypeDescriptor.GetProperties(deletedObjectMetadataSelection)["OBJECTSTATE"]?.GetValue(deletedObjectMetadataSelection)?.ToString(), "Deleted", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(deletedObjectMetadataSelection)["RECORDINDEX"]?.GetValue(deletedObjectMetadataSelection)?.ToString(), "13", StringComparison.Ordinal),
                $"Clicking a deleted {assetFamily} object inside a live section should expose deleted object state metadata");
        }
        Expect(ReadPrivateNullableInt(surface, "selectedRecordIndex") == 13 &&
               ReadPrivateNullableInt(surface, "selectedReportSectionRecordIndex") == 42 &&
               !ReadPrivateBoolField(surface, "unplacedReportObjectsSelected"),
            $"Clicking a deleted {assetFamily} object inside a live section should keep the containing live section highlighted");
        Expect(reportLayout.Sections[0].DeletedObjectCount == 1 &&
               reportLayout.DeletedObjects.Count == 1 &&
               string.Equals(reportLayout.DeletedObjects[0].Title, expectedDeletedObjectTitle, StringComparison.Ordinal),
            $"Deleted live-section {assetFamily} smoke snapshot should preserve deleted layout object membership and title contracts");
    }

    private static CopperfinStudioSnapshotDocument BuildDeletedLiveSectionSurfaceSmokeSnapshot(
        string assetFamily,
        string deletedObjectTitle,
        string deletedObjectKind)
    {
        return new CopperfinStudioSnapshotDocument
        {
            AssetFamily = assetFamily,
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 6,
                    Title = "customer.company",
                    Subtitle = deletedObjectKind,
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
                    Title = deletedObjectTitle,
                    Subtitle = deletedObjectKind,
                    Deleted = true,
                    Properties = new List<CopperfinStudioSnapshotProperty>
                    {
                        new() { Name = "HPOS", Value = "1400" },
                        new() { Name = "VPOS", Value = "3200" },
                        new() { Name = "WIDTH", Value = "3600" },
                        new() { Name = "HEIGHT", Value = "600" },
                        new() { Name = "EXPR", Value = deletedObjectTitle }
                    }
                },
                new()
                {
                    RecordIndex = 9,
                    Title = "orphan.note",
                    Subtitle = deletedObjectKind,
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
                IsLabel = string.Equals(assetFamily, "label", StringComparison.Ordinal),
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
                        DeletedObjectCount = 1,
                        Objects = new List<CopperfinStudioReportLayoutObject>
                        {
                            new()
                            {
                                RecordIndex = 6,
                                ObjectKind = deletedObjectKind,
                                Title = "customer.company",
                                Left = 1200,
                                Top = 2600,
                                Width = 4000,
                                Height = 500
                            }
                        }
                    }
                },
                DeletedObjects = new List<CopperfinStudioReportLayoutObject>
                {
                    new()
                    {
                        RecordIndex = 13,
                        Deleted = true,
                        ContainingSectionId = "detail_1",
                        ContainingSectionRecordIndex = 42,
                        SectionRelativeTop = 1200,
                        SectionRelativeBottom = 1800,
                        SectionObjectIndex = 1,
                        SectionObjectCount = 2,
                        ObjectKind = deletedObjectKind,
                        Title = deletedObjectTitle,
                        Expression = deletedObjectTitle,
                        Left = 1400,
                        Top = 3200,
                        Width = 3600,
                        Height = 600
                    }
                },
                UnplacedObjects = new List<CopperfinStudioReportLayoutObject>
                {
                    new()
                    {
                        RecordIndex = 9,
                        ObjectKind = deletedObjectKind,
                        Title = "orphan.note",
                        Left = 800,
                        Top = 700,
                        Width = 2400,
                        Height = 450
                    }
                }
            }
        };
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

}
