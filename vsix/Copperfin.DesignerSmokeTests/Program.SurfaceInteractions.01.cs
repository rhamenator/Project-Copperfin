
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
                new() { Name = "SUPEXPR", Value = "customer.company > 0" },
                new() { Name = "SUPGROUP", Value = "6" },
                new() { Name = "HPOS", Value = "1200" },
                new() { Name = "VPOS", Value = "2600" },
                new() { Name = "WIDTH", Value = "4000" },
                new() { Name = "HEIGHT", Value = "500" },
                new() { Name = "FONTFACE", Value = "Arial" },
                new() { Name = "FONTSTYLE", Value = "1" },
                new() { Name = "FONTSIZE", Value = "10" },
                new() { Name = "FLOAT", Value = "true" },
                new() { Name = "NOREPEAT", Value = "false" },
                new() { Name = "STRETCH", Value = "true" },
                new() { Name = "STRETCHTOP", Value = "false" }
            }
        };

        var spanishSelection = CopperfinDesignerSelection.FromSnapshot("report", snapshotObject, new CopperfinLocalization("es-419"));
        Expect(spanishSelection is not null &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Tipo de objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Estado del objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Expresión", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Imprimir cuando", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Cuando cambia el grupo", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Tamaño de fuente", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Flotante", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "No repetir", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Extender con desbordamiento", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Extender en relación con la parte superior", StringComparison.Ordinal)),
            "Spanish report object property-grid selection should localize object field labels");
        Expect(string.Equals(TypeDescriptor.GetProperties(spanishSelection)["OBJECTSTATE"]?.GetValue(spanishSelection)?.ToString(), "Activa", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(spanishSelection)["RECORDINDEX"]?.GetValue(spanishSelection)?.ToString(), "10", StringComparison.Ordinal),
            "Spanish report object property-grid selection should localize live object state values and preserve record identity");

        var portugueseSelection = CopperfinDesignerSelection.FromSnapshot("report", snapshotObject, new CopperfinLocalization("pt-BR"));
        Expect(portugueseSelection is not null &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Tipo de objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Estado do objeto", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Expressão", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Quando imprimir", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Quando o grupo muda", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Tamanho da fonte", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Flutuar", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Não repetir", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Expandir com estouro", StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, "Expandir em relação ao topo", StringComparison.Ordinal)),
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
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PrintWhen"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PrintWhenGroup"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.FontSize"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Float"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.NoRepeat"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Stretch"), StringComparison.Ordinal)) &&
               TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.StretchTop"), StringComparison.Ordinal)),
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
            ExpectSelectionUpdate(liveSelection, "FLOAT", false, "false",
                "Live report object property-grid selection should serialize FLOAT edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "NOREPEAT", true, "true",
                "Live report object property-grid selection should serialize NOREPEAT edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "STRETCH", false, "false",
                "Live report object property-grid selection should serialize STRETCH edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "STRETCHTOP", true, "true",
                "Live report object property-grid selection should serialize STRETCHTOP edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "SUPEXPR", "customer.company > 100", "customer.company > 100",
                "Live report object property-grid selection should serialize SUPEXPR edits through the shared update path");
            ExpectSelectionUpdate(liveSelection, "SUPGROUP", 7, "7",
                "Live report object property-grid selection should serialize SUPGROUP edits through the shared update path");
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
                new() { Name = "SUPEXPR", Value = "customer.deleted_total > 0" },
                new() { Name = "SUPGROUP", Value = "6" },
                new() { Name = "HPOS", Value = "1400" },
                new() { Name = "VPOS", Value = "9200" },
                new() { Name = "WIDTH", Value = "3000" },
                new() { Name = "HEIGHT", Value = "450" },
                new() { Name = "FONTFACE", Value = "Arial" },
                new() { Name = "FONTSTYLE", Value = "0" },
                new() { Name = "FONTSIZE", Value = "9" },
                new() { Name = "FLOAT", Value = string.Empty },
                new() { Name = "NOREPEAT", Value = "false" },
                new() { Name = "STRETCH", Value = string.Empty },
                new() { Name = "STRETCHTOP", Value = "false" }
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
            ExpectSelectionUpdate(deletedSelection, "SUPEXPR", "customer.deleted_region > 0", "customer.deleted_region > 0",
                "Deleted report object property-grid selection should preserve the invariant Print When update target");
            ExpectSelectionUpdate(deletedSelection, "SUPGROUP", 7, "7",
                "Deleted report object property-grid selection should preserve the invariant Print When group update target");
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
            ExpectSelectionUpdate(deletedSelection, "FLOAT", true, "true",
                "Deleted report object property-grid selection should serialize FLOAT edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "NOREPEAT", true, "true",
                "Deleted report object property-grid selection should serialize NOREPEAT edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "STRETCH", true, "true",
                "Deleted report object property-grid selection should serialize STRETCH edits through the shared update path");
            ExpectSelectionUpdate(deletedSelection, "STRETCHTOP", true, "true",
                "Deleted report object property-grid selection should serialize STRETCHTOP edits through the shared update path");
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
                        Expression = "customer.deleted_country",
                        ExpressionFieldIndex = 4,
                        ExpressionMemoBlockNumber = 10,
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
                   string.Equals(topTarget, "VPOS", StringComparison.Ordinal) &&
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
                   string.Equals(TypeDescriptor.GetProperties(deletedSelection)["OBJECTCOUNT"]?.GetValue(deletedSelection)?.ToString(), "1", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(deletedSelection)["EXPRESSIONFIELD"]?.GetValue(deletedSelection)?.ToString(), "4", StringComparison.Ordinal) &&
                   string.Equals(TypeDescriptor.GetProperties(deletedSelection)["EXPRESSIONMEMO"]?.GetValue(deletedSelection)?.ToString(), "10", StringComparison.Ordinal),
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
               string.Equals(TypeDescriptor.GetProperties(spanishDeletedSelection)["OBJECTCOUNT"]?.GetValue(spanishDeletedSelection)?.ToString(), "1", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(spanishDeletedSelection)["EXPRESSIONFIELD"]?.GetValue(spanishDeletedSelection)?.ToString(), "4", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(spanishDeletedSelection)["EXPRESSIONMEMO"]?.GetValue(spanishDeletedSelection)?.ToString(), "10", StringComparison.Ordinal),
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
               string.Equals(TypeDescriptor.GetProperties(portugueseDeletedSelection)["OBJECTCOUNT"]?.GetValue(portugueseDeletedSelection)?.ToString(), "1", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(portugueseDeletedSelection)["EXPRESSIONFIELD"]?.GetValue(portugueseDeletedSelection)?.ToString(), "4", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(portugueseDeletedSelection)["EXPRESSIONMEMO"]?.GetValue(portugueseDeletedSelection)?.ToString(), "10", StringComparison.Ordinal),
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
               string.Equals(TypeDescriptor.GetProperties(pseudoDeletedSelection)["OBJECTCOUNT"]?.GetValue(pseudoDeletedSelection)?.ToString(), "1", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(pseudoDeletedSelection)["EXPRESSIONFIELD"]?.GetValue(pseudoDeletedSelection)?.ToString(), "4", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(pseudoDeletedSelection)["EXPRESSIONMEMO"]?.GetValue(pseudoDeletedSelection)?.ToString(), "10", StringComparison.Ordinal),
            "Pseudo-localized deleted report section property-grid selection should route deleted section state values and preserve record/object metadata");
        Expect(snapshot.ReportLayout.DeletedSections[0].Deleted,
            "Deleted report section property-grid state values should preserve deleted section snapshot contracts");
        Expect(snapshot.ReportLayout.DeletedSections[0].RecordIndex == 51,
            "Deleted report section property-grid record metadata should preserve deleted section snapshot contracts");
        Expect(snapshot.ReportLayout.DeletedSections[0].Objects.Count == 1,
            "Deleted report section property-grid object-count metadata should preserve deleted section snapshot contracts");
        Expect(snapshot.ReportLayout.DeletedSections[0].ExpressionFieldIndex == 4 &&
               snapshot.ReportLayout.DeletedSections[0].ExpressionMemoBlockNumber == 10,
            "Deleted report section property-grid section-expression provenance should preserve deleted section snapshot contracts");
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

}
