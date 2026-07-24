
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
    private static void SmokeReportSettingsExplorerSelection()
    {
        var settings = new List<CopperfinStudioNamedValue>
        {
            new() { Name = "ORIENTATION", Value = "0", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 9 },
            new() { Name = "COLS", Value = "2", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 9 },
            new() { Name = "COLWIDTH", Value = "3600", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 9 },
            new() { Name = "COLSPACING", Value = "120", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 9 },
            new() { Name = "PAPERLENGTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 8 },
            new() { Name = "PAPERWIDTH", Value = "2159", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 8 },
            new() { Name = "TOPMARGIN", Value = "20", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
            new() { Name = "BOTMARGIN", Value = "30", RecordIndex = 0, FieldIndex = 63, MemoBlockNumber = 0 },
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
            new() { Name = "COPIES", Value = "1", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 20, MemoBlockNumber = 8 },
            new() { Name = "GRID", Value = "true", RecordIndex = 0, FieldIndex = 12, MemoBlockNumber = 0 },
            new() { Name = "RULER", Value = "4", RecordIndex = 0, FieldIndex = 13, MemoBlockNumber = 0 },
            new() { Name = "RULERLINES", Value = "1", RecordIndex = 0, FieldIndex = 14, MemoBlockNumber = 0 },
            new() { Name = "ADDALIAS", Value = "true", RecordIndex = 0, FieldIndex = 15, MemoBlockNumber = 0 },
            new() { Name = "CURPOS", Value = "true", RecordIndex = 0, FieldIndex = 16, MemoBlockNumber = 0 },
            new() { Name = "UNIQUE", Value = "true", RecordIndex = 0, FieldIndex = 17, MemoBlockNumber = 0 },
            new() { Name = "ORDER", Value = "hex:4F524445522D4259544553", RecordIndex = 0, FieldIndex = 18, MemoBlockNumber = 22 },
            new() { Name = "COMMENT", Value = "Header developer note", RecordIndex = 0, FieldIndex = 19, MemoBlockNumber = 23 }
        };
        var settingsOnlySnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
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
                Settings = settings
            }
        };
        var mixedSnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Customer Invoice",
                Settings = settings,
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

        using var control = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(control, settingsOnlySnapshot);
        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        InvokeAssetEditorVoid(control, "SyncExplorerSelection");

        var propertyGrid = GetPrivatePropertyGrid(control);
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection settingsSelection &&
               settingsSelection.RecordIndex == 0 &&
               objectListView.Items.Count == 0 &&
               string.Equals(sectionListView.Items.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings", StringComparison.Ordinal),
            "Report settings explorer selection should produce a settings-rooted property-grid selection and clear object rows");
        Expect(string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DOCUMENTTITLE"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "Customer Invoice", StringComparison.Ordinal),
            "Report settings explorer selection should expose shared document-title metadata from the report-layout model");
        Expect(string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["SORTEXPRESSION"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "customer.country", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["SORTEXPRESSIONFIELD"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "9", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["SORTEXPRESSIONMEMO"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "11", StringComparison.Ordinal),
            "Report settings explorer selection should expose explicit sort metadata from the shared settings model");
        Expect(string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["BOTMARGIN"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "30", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["LEFTMARGIN"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "15", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLS"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "2", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLWIDTH"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "3600", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLSPACING"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "120", StringComparison.Ordinal) &&
               Convert.ToBoolean(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["GRID"]?.GetValue(propertyGrid.SelectedObject), CultureInfo.InvariantCulture) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["RULER"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "4", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["RULERLINES"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "1", StringComparison.Ordinal) &&
               Convert.ToBoolean(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["ADDALIAS"]?.GetValue(propertyGrid.SelectedObject), CultureInfo.InvariantCulture) &&
               Convert.ToBoolean(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["CURPOS"]?.GetValue(propertyGrid.SelectedObject), CultureInfo.InvariantCulture) &&
               Convert.ToBoolean(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["UNIQUE"]?.GetValue(propertyGrid.SelectedObject), CultureInfo.InvariantCulture) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["ORDER"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "hex:4F524445522D4259544553", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COMMENT"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "Header developer note", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["PAPERLENGTH"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "2794", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["PAPERWIDTH"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "2159", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DRIVER"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "winspool", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DEVICE"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "FinePrint 2000", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["OUTPUT"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "FPR4:", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DEFAULTSOURCE"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "15", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["PRINTQUALITY"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "600", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["YRESOLUTION"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "600", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["TTOPTION"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "3", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLOR"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "1", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["ASCII"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "9", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLLATE"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "1", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COPIES"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "1", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["RIGHTMARGIN"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "25", StringComparison.Ordinal),
            "Report settings explorer selection should expose column-setup, paper-dimension, printer-identity, print-profile, auxiliary print, color, and side-margin metadata from the shared settings model");
        Expect(string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["PREVIEWBOUNDS"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "L 0 T 2000 R 5200 B 8100   Size: 5200 x 6100", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DELETEDPREVIEWBOUNDS"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
            "Report settings explorer selection should expose live and deleted preview metadata from the shared report-layout model");

        if (propertyGrid.SelectedObject is CopperfinDesignerSelection editableSelection)
        {
            ExpectSelectionUpdate(editableSelection, "TOPMARGIN", 30, "30",
                "Report settings property-grid selection should serialize numeric root-setting edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "BOTMARGIN", 40, "40",
                "Report settings property-grid selection should serialize bottom-margin edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLS", 3, "3",
                "Report settings property-grid selection should serialize column-count edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLWIDTH", 4200, "4200",
                "Report settings property-grid selection should serialize column-width edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLSPACING", 180, "180",
                "Report settings property-grid selection should serialize column-spacing edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "GRID", false, "false",
                "Report settings property-grid selection should serialize grid-snapping edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "RULER", 2, "2",
                "Report settings property-grid selection should serialize ruler-unit edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "RULERLINES", 0, "0",
                "Report settings property-grid selection should serialize ruler-line edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "ADDALIAS", false, "false",
                "Report settings property-grid selection should serialize field-alias edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "CURPOS", false, "false",
                "Report settings property-grid selection should serialize show-position edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "UNIQUE", false, "false",
                "Report settings property-grid selection should serialize unique-report edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "ORDER", "hex:01A5007F", "hex:01A5007F",
                "Report settings property-grid selection should preserve binary-safe order-flag update values");
            ExpectSelectionUpdate(editableSelection, "COMMENT", "Updated header note", "Updated header note",
                "Report settings property-grid selection should serialize COMMENT edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "PAPERLENGTH", 4318, "4318",
                "Report settings property-grid selection should serialize paper-length edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "PAPERWIDTH", 2794, "2794",
                "Report settings property-grid selection should serialize paper-width edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "DRIVER", "cups", "cups",
                "Report settings property-grid selection should preserve invariant printer-driver update targets");
            ExpectSelectionUpdate(editableSelection, "DEVICE", "Office Printer", "Office Printer",
                "Report settings property-grid selection should preserve invariant printer-device update targets");
            ExpectSelectionUpdate(editableSelection, "OUTPUT", "LPT1:", "LPT1:",
                "Report settings property-grid selection should preserve invariant printer-output update targets");
            ExpectSelectionUpdate(editableSelection, "DEFAULTSOURCE", 7, "7",
                "Report settings property-grid selection should serialize default-source edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "PRINTQUALITY", 300, "300",
                "Report settings property-grid selection should serialize print-quality edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "YRESOLUTION", 300, "300",
                "Report settings property-grid selection should serialize Y-resolution edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "TTOPTION", 2, "2",
                "Report settings property-grid selection should serialize TrueType-option edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLOR", 0, "0",
                "Report settings property-grid selection should serialize color-mode edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "ASCII", 7, "7",
                "Report settings property-grid selection should serialize ASCII-mode edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLLATE", 0, "0",
                "Report settings property-grid selection should serialize collate edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COPIES", 2, "2",
                "Report settings property-grid selection should serialize copies edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "LEFTMARGIN", 35, "35",
                "Report settings property-grid selection should serialize left-margin edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "RIGHTMARGIN", 45, "45",
                "Report settings property-grid selection should serialize right-margin edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "TAG", "customer.region", "customer.region",
                "Report settings property-grid selection should preserve invariant string update targets");
        }

        using var labelControl = new CopperfinAssetEditorControl();
        var labelSnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                DocumentTitle = "Customer Invoice",
                Settings = settings
            }
        };
        ApplyReportSnapshotForExplorerSmoke(labelControl, labelSnapshot);
        var labelSectionListView = GetPrivateListView(labelControl, "sectionListView");
        var labelSettingsItem = labelSectionListView.Items.Cast<ListViewItem>()
            .First(item => string.Equals(item.Text, "Settings", StringComparison.Ordinal));
        labelSettingsItem.Selected = true;
        InvokeAssetEditorVoid(labelControl, "SyncExplorerSelection");
        var labelPropertyGrid = GetPrivatePropertyGrid(labelControl);
        Expect(labelPropertyGrid.SelectedObject is CopperfinDesignerSelection labelSelection &&
               labelSelection.RecordIndex == 0 &&
               string.Equals(labelSectionListView.Items.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings", StringComparison.Ordinal),
            "Label settings explorer selection should expose the same shared settings-rooted property-grid selection");

        var spanishSelection = CopperfinDesignerSelection.FromReportSettings(settings, new CopperfinLocalization("es-419"), reportLayout: settingsOnlySnapshot.ReportLayout);
        var spanishProperties = TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().ToList();
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Cantidad de configuraciones", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Orientación", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Columnas", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Ancho de columna", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Espaciado de columna", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Ajuste a la cuadrícula", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Unidades de regla", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Líneas de regla", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Agregar alias de campos", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Mostrar posición", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Informe único", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Largo del papel", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Ancho del papel", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Controlador de impresora", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Dispositivo de impresora", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Salida de impresora", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Fuente predeterminada", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Calidad de impresión", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Resolución Y", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Opción TrueType", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Color", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "ASCII", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Intercalar", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Copias", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Margen superior", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Margen inferior", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Margen izquierdo", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Margen derecho", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Expresión de orden", StringComparison.Ordinal)),
            "Spanish report settings property-grid selection should localize supported root-setting labels");
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Título del documento", StringComparison.Ordinal)) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "DOCUMENTTITLE", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Customer Invoice", StringComparison.Ordinal),
            "Spanish report settings property-grid selection should localize document-title metadata labels");
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Expresión de orden activa", StringComparison.Ordinal)) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "SORTEXPRESSION", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "customer.country", StringComparison.Ordinal) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "SORTEXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "9", StringComparison.Ordinal) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "SORTEXPRESSIONMEMO", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "11", StringComparison.Ordinal),
            "Spanish report settings property-grid selection should localize explicit sort metadata labels");
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Limites de vista previa", StringComparison.Ordinal)) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "PREVIEWBOUNDS", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Izq 0 Arr 2000 Der 5200 Ab 8100   Tamaño: 5200 x 6100", StringComparison.Ordinal),
            "Spanish report settings property-grid selection should localize preview metadata labels and values");

        var portugueseSelection = CopperfinDesignerSelection.FromReportSettings(settings, new CopperfinLocalization("pt-BR"), reportLayout: settingsOnlySnapshot.ReportLayout);
        var portugueseProperties = TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().ToList();
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Quantidade de configurações", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Orientação", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Colunas", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Largura da coluna", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Espaçamento da coluna", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Ajuste à grade", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Unidades da régua", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Linhas da régua", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Adicionar aliases de campos", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Mostrar posição", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Relatório único", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Comprimento do papel", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Largura do papel", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Controlador da impressora", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Dispositivo da impressora", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Saída da impressora", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Origem padrão", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Qualidade de impressão", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Resolução Y", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Opção TrueType", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Cor", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "ASCII", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Intercalar", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Cópias", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Margem superior", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Margem inferior", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Margem esquerda", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Margem direita", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Expressão de ordenação", StringComparison.Ordinal)),
            "Portuguese report settings property-grid selection should localize supported root-setting labels");
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Título do documento", StringComparison.Ordinal)) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "DOCUMENTTITLE", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Customer Invoice", StringComparison.Ordinal),
            "Portuguese report settings property-grid selection should localize document-title metadata labels");
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Expressão de ordenação ativa", StringComparison.Ordinal)) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "SORTEXPRESSION", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "customer.country", StringComparison.Ordinal) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "SORTEXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "9", StringComparison.Ordinal) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "SORTEXPRESSIONMEMO", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "11", StringComparison.Ordinal),
            "Portuguese report settings property-grid selection should localize explicit sort metadata labels");
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Limites da visualização", StringComparison.Ordinal)) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "PREVIEWBOUNDS", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "E 0 T 2000 D 5200 B 8100   Tamanho: 5200 x 6100", StringComparison.Ordinal),
            "Portuguese report settings property-grid selection should localize preview metadata labels and values");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        var pseudoSelection = CopperfinDesignerSelection.FromReportSettings(settings, pseudoLocalization, reportLayout: settingsOnlySnapshot.ReportLayout);
        var pseudoProperties = TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().ToList();
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.SettingsCount"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.TopMargin"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Columns"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ColumnWidth"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ColumnSpacing"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.GridSnapping"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.RulerUnits"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.RulerLines"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.AddAlias"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ShowPosition"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.UniqueReport"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.OrderProtectionFlags"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Comments"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PaperLength"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PaperWidth"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PrinterDriver"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PrinterDevice"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PrinterOutput"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.DefaultSource"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PrintQuality"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.YResolution"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.TrueTypeOption"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Color"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Ascii"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Collate"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Copies"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.BottomMargin"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.LeftMargin"), StringComparison.Ordinal)) &&
               pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.RightMargin"), StringComparison.Ordinal)),
            "Pseudo-localized report settings property-grid selection should route new root-setting labels through the shared catalog");
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.DocumentTitle"), StringComparison.Ordinal)) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "DOCUMENTTITLE", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "Customer Invoice", StringComparison.Ordinal),
            "Pseudo-localized report settings property-grid selection should route document-title metadata through the shared catalog");
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ActiveSortExpression"), StringComparison.Ordinal)) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "SORTEXPRESSION", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "customer.country", StringComparison.Ordinal) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "SORTEXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "9", StringComparison.Ordinal) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "SORTEXPRESSIONMEMO", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "11", StringComparison.Ordinal),
            "Pseudo-localized report settings property-grid selection should route explicit sort metadata through the shared catalog");
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PreviewBounds"), StringComparison.Ordinal)) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "PREVIEWBOUNDS", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), pseudoLocalization.Format("AssetEditor.Property.BoundsValue", 0, 2000, 5200, 8100, 5200, 6100), StringComparison.Ordinal),
            "Pseudo-localized report settings property-grid selection should route preview metadata through the shared catalog");

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyReportSnapshotForExplorerSmoke(spanishControl, mixedSnapshot);
        Expect(GetPrivateListView(spanishControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, "Configuraciones", StringComparison.Ordinal)),
            "Spanish report explorer should localize the settings scope row");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyReportSnapshotForExplorerSmoke(portugueseControl, mixedSnapshot);
        Expect(GetPrivateListView(portugueseControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, "Configurações", StringComparison.Ordinal)),
            "Portuguese report explorer should localize the settings scope row");

        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        ApplyReportSnapshotForExplorerSmoke(pseudoControl, mixedSnapshot);
        Expect(GetPrivateListView(pseudoControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, pseudoLocalization.Text("AssetEditor.ReportSection.Settings"), StringComparison.Ordinal)),
            "Pseudo-localized report explorer should route the settings scope row through the shared catalog");

        Expect(settings[0].RecordIndex == 0 &&
               string.Equals(settings[0].Name, "ORIENTATION", StringComparison.Ordinal) &&
               string.Equals(settings[1].Name, "COLS", StringComparison.Ordinal) &&
               settings[1].SourceLineIndex == 1 &&
               string.Equals(settings[2].Name, "COLWIDTH", StringComparison.Ordinal) &&
               settings[2].SourceLineIndex == 2 &&
               string.Equals(settings[3].Name, "COLSPACING", StringComparison.Ordinal) &&
               settings[3].SourceLineIndex == 3 &&
               string.Equals(settings[4].Name, "PAPERLENGTH", StringComparison.Ordinal) &&
               settings[4].FieldIndex == 6 &&
               string.Equals(settings[5].Name, "PAPERWIDTH", StringComparison.Ordinal) &&
               settings[5].FieldIndex == 6 &&
               string.Equals(settings[6].Value, "20", StringComparison.Ordinal) &&
               string.Equals(settings[7].Name, "BOTMARGIN", StringComparison.Ordinal) &&
               settings[7].FieldIndex == 63 &&
               string.Equals(settings[8].Name, "LEFTMARGIN", StringComparison.Ordinal) &&
               settings[8].FieldIndex == 10 &&
               string.Equals(settings[9].Name, "RIGHTMARGIN", StringComparison.Ordinal) &&
               settings[9].FieldIndex == 11 &&
               string.Equals(settings[10].Name, "TAG", StringComparison.Ordinal) &&
               settings[10].FieldIndex == 9 &&
               settings[10].MemoBlockNumber == 11 &&
               string.Equals(settings[11].Name, "DRIVER", StringComparison.Ordinal) &&
               settings[11].FieldIndex == 6 &&
               string.Equals(settings[12].Name, "DEVICE", StringComparison.Ordinal) &&
               settings[12].FieldIndex == 6 &&
               string.Equals(settings[13].Name, "OUTPUT", StringComparison.Ordinal) &&
               settings[13].FieldIndex == 6 &&
               string.Equals(settings[14].Name, "DEFAULTSOURCE", StringComparison.Ordinal) &&
               settings[14].FieldIndex == 6 &&
               string.Equals(settings[15].Name, "PRINTQUALITY", StringComparison.Ordinal) &&
               settings[15].FieldIndex == 6 &&
               string.Equals(settings[16].Name, "YRESOLUTION", StringComparison.Ordinal) &&
               settings[16].FieldIndex == 6 &&
               string.Equals(settings[17].Name, "TTOPTION", StringComparison.Ordinal) &&
               settings[17].FieldIndex == 6 &&
               string.Equals(settings[18].Name, "COLOR", StringComparison.Ordinal) &&
               settings[18].FieldIndex == 6 &&
               string.Equals(settings[19].Name, "ASCII", StringComparison.Ordinal) &&
               settings[19].FieldIndex == 6 &&
               string.Equals(settings[20].Name, "COLLATE", StringComparison.Ordinal) &&
               settings[20].FieldIndex == 6 &&
               string.Equals(settings[21].Name, "COPIES", StringComparison.Ordinal) &&
               settings[21].FieldIndex == 6 &&
               string.Equals(settings[22].Name, "GRID", StringComparison.Ordinal) &&
               settings[22].FieldIndex == 12 &&
               string.Equals(settings[23].Name, "RULER", StringComparison.Ordinal) &&
               settings[23].FieldIndex == 13 &&
               string.Equals(settings[24].Name, "RULERLINES", StringComparison.Ordinal) &&
               settings[24].FieldIndex == 14 &&
               string.Equals(settings[25].Name, "ADDALIAS", StringComparison.Ordinal) &&
               settings[25].FieldIndex == 15 &&
               string.Equals(settings[26].Name, "CURPOS", StringComparison.Ordinal) &&
               settings[26].FieldIndex == 16 &&
               string.Equals(settings[27].Name, "UNIQUE", StringComparison.Ordinal) &&
               settings[27].FieldIndex == 17 &&
               string.Equals(settings[28].Name, "ORDER", StringComparison.Ordinal) &&
               settings[28].FieldIndex == 18 &&
               settings[28].MemoBlockNumber == 22 &&
               string.Equals(settings[29].Name, "COMMENT", StringComparison.Ordinal) &&
               settings[29].FieldIndex == 19 &&
               settings[29].MemoBlockNumber == 23,
            "Localized report settings property-grid selection should preserve root-setting machine contracts");

        var readOnlySelection = CopperfinDesignerSelection.FromReportSettings(
            settings,
            new CopperfinLocalization("en-US"),
            documentReadOnly: true,
            reportLayout: settingsOnlySnapshot.ReportLayout);
        Expect(!readOnlySelection.TryGetUpdate("GRID", out _, out _) &&
               !readOnlySelection.TryGetUpdate("RULER", out _, out _) &&
               !readOnlySelection.TryGetUpdate("RULERLINES", out _, out _) &&
               !readOnlySelection.TryGetUpdate("ADDALIAS", out _, out _) &&
               !readOnlySelection.TryGetUpdate("CURPOS", out _, out _) &&
               !readOnlySelection.TryGetUpdate("UNIQUE", out _, out _) &&
               !readOnlySelection.TryGetUpdate("ORDER", out _, out _) &&
               !readOnlySelection.TryGetUpdate("COMMENT", out _, out _),
            "Read-only report settings property-grid selection should protect header view-setting update targets");
    }

    private static void SmokeDeletedReportSettingsExplorerSelection()
    {
        var deletedSettings = new List<CopperfinStudioNamedValue>
        {
            new() { Name = "ORIENTATION", Value = "1", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 19 },
            new() { Name = "COLS", Value = "3", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 1, MemoBlockNumber = 19 },
            new() { Name = "COLWIDTH", Value = "2400", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 2, MemoBlockNumber = 19 },
            new() { Name = "COLSPACING", Value = "180", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 3, MemoBlockNumber = 19 },
            new() { Name = "PAPERLENGTH", Value = "4318", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 4, MemoBlockNumber = 18 },
            new() { Name = "PAPERWIDTH", Value = "2794", RecordIndex = 0, FieldIndex = 6, SourceLineIndex = 5, MemoBlockNumber = 18 },
            new() { Name = "TOPMARGIN", Value = "40", RecordIndex = 0, FieldIndex = 3, MemoBlockNumber = 0 },
            new() { Name = "BOTMARGIN", Value = "55", RecordIndex = 0, FieldIndex = 63, MemoBlockNumber = 0 },
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
        };
        var deletedOnlySnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
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
                DeletedSettings = deletedSettings
            }
        };
        var mixedSnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            ReportLayout = new CopperfinStudioReportLayout
            {
                DocumentTitle = "Deleted Customer Invoice",
                Settings = new List<CopperfinStudioNamedValue>
                {
                    new() { Name = "ORIENTATION", Value = "0", RecordIndex = 0, FieldIndex = 2, SourceLineIndex = 0, MemoBlockNumber = 9 }
                },
                DeletedSettings = deletedSettings
            }
        };

        using var control = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(control, deletedOnlySnapshot);
        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        InvokeAssetEditorVoid(control, "SyncExplorerSelection");

        var propertyGrid = GetPrivatePropertyGrid(control);
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection deletedSettingsSelection &&
               deletedSettingsSelection.RecordIndex == 0 &&
               objectListView.Items.Count == 0 &&
               string.Equals(sectionListView.Items.Cast<ListViewItem>().FirstOrDefault()?.Text, "Settings (deleted)", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(deletedSettingsSelection)["SETTINGSSTATE"]?.GetValue(deletedSettingsSelection)?.ToString(), "Deleted", StringComparison.Ordinal),
            "Deleted report settings explorer selection should produce a deleted settings-rooted property-grid selection and clear object rows");
        Expect(string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DOCUMENTTITLE"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal),
            "Deleted report settings explorer selection should expose shared document-title metadata from the report-layout model");
        Expect(string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["SORTEXPRESSION"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "deleted.customer.country", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["SORTEXPRESSIONFIELD"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "9", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["SORTEXPRESSIONMEMO"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "21", StringComparison.Ordinal),
            "Deleted report settings explorer selection should expose deleted sort metadata from the shared settings model");
        Expect(string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["LEFTMARGIN"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "35", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["BOTMARGIN"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "55", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLS"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "3", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLWIDTH"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "2400", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLSPACING"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "180", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["PAPERLENGTH"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "4318", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["PAPERWIDTH"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "2794", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DRIVER"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "deleted.winspool", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DEVICE"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "Deleted Printer", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["OUTPUT"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "DPRN:", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DEFAULTSOURCE"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "16", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["PRINTQUALITY"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "1200", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["YRESOLUTION"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "1200", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["TTOPTION"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "2", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLOR"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "2", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["ASCII"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "10", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COLLATE"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "0", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["COPIES"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "2", StringComparison.Ordinal) &&
               string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["RIGHTMARGIN"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "45", StringComparison.Ordinal),
            "Deleted report settings explorer selection should expose deleted column-setup, paper-dimension, printer-identity, print-profile, auxiliary print, color, and margin metadata from the shared settings model");
        Expect(string.Equals(TypeDescriptor.GetProperties(propertyGrid.SelectedObject)["DELETEDPREVIEWBOUNDS"]?.GetValue(propertyGrid.SelectedObject)?.ToString(), "L 1000 T 2600 R 2200 B 2900   Size: 1200 x 300", StringComparison.Ordinal),
            "Deleted report settings explorer selection should expose deleted preview metadata from the shared report-layout model");

        if (propertyGrid.SelectedObject is CopperfinDesignerSelection editableSelection)
        {
            ExpectSelectionUpdate(editableSelection, "TOPMARGIN", 55, "55",
                "Deleted report settings property-grid selection should serialize numeric deleted-root-setting edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "BOTMARGIN", 65, "65",
                "Deleted report settings property-grid selection should serialize deleted bottom-margin edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLS", 4, "4",
                "Deleted report settings property-grid selection should serialize deleted column-count edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLWIDTH", 3000, "3000",
                "Deleted report settings property-grid selection should serialize deleted column-width edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLSPACING", 240, "240",
                "Deleted report settings property-grid selection should serialize deleted column-spacing edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "PAPERLENGTH", 5588, "5588",
                "Deleted report settings property-grid selection should serialize deleted paper-length edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "PAPERWIDTH", 4318, "4318",
                "Deleted report settings property-grid selection should serialize deleted paper-width edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "DRIVER", "deleted.cups", "deleted.cups",
                "Deleted report settings property-grid selection should preserve invariant deleted printer-driver update targets");
            ExpectSelectionUpdate(editableSelection, "DEVICE", "Archived Printer", "Archived Printer",
                "Deleted report settings property-grid selection should preserve invariant deleted printer-device update targets");
            ExpectSelectionUpdate(editableSelection, "OUTPUT", "DPRN2:", "DPRN2:",
                "Deleted report settings property-grid selection should preserve invariant deleted printer-output update targets");
            ExpectSelectionUpdate(editableSelection, "DEFAULTSOURCE", 17, "17",
                "Deleted report settings property-grid selection should serialize deleted default-source edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "PRINTQUALITY", 2400, "2400",
                "Deleted report settings property-grid selection should serialize deleted print-quality edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "YRESOLUTION", 2400, "2400",
                "Deleted report settings property-grid selection should serialize deleted Y-resolution edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "TTOPTION", 1, "1",
                "Deleted report settings property-grid selection should serialize deleted TrueType-option edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLOR", 1, "1",
                "Deleted report settings property-grid selection should serialize deleted color-mode edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "ASCII", 8, "8",
                "Deleted report settings property-grid selection should serialize deleted ASCII-mode edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLLATE", 1, "1",
                "Deleted report settings property-grid selection should serialize deleted collate edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COPIES", 3, "3",
                "Deleted report settings property-grid selection should serialize deleted copies edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "LEFTMARGIN", 50, "50",
                "Deleted report settings property-grid selection should serialize deleted left-margin edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "RIGHTMARGIN", 60, "60",
                "Deleted report settings property-grid selection should serialize deleted right-margin edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "TAG", "deleted.customer.region", "deleted.customer.region",
                "Deleted report settings property-grid selection should preserve invariant string update targets");
        }

        using var labelControl = new CopperfinAssetEditorControl();
        var labelSnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                DocumentTitle = "Deleted Customer Invoice",
                DeletedSettings = deletedSettings
            }
        };
        ApplyReportSnapshotForExplorerSmoke(labelControl, labelSnapshot);
        InvokeAssetEditorVoid(labelControl, "SyncExplorerSelection");
        var labelPropertyGrid = GetPrivatePropertyGrid(labelControl);
        Expect(labelPropertyGrid.SelectedObject is CopperfinDesignerSelection labelSelection &&
               labelSelection.RecordIndex == 0 &&
               string.Equals(TypeDescriptor.GetProperties(labelSelection)["SETTINGSSTATE"]?.GetValue(labelSelection)?.ToString(), "Deleted", StringComparison.Ordinal),
            "Deleted label settings explorer selection should expose the same shared deleted-settings property-grid selection");

        var spanishSelection = CopperfinDesignerSelection.FromReportSettings(deletedSettings, new CopperfinLocalization("es-419"), deleted: true, reportLayout: deletedOnlySnapshot.ReportLayout);
        var spanishProperties = TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().ToList();
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Estado de las configuraciones", StringComparison.Ordinal)) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "SETTINGSSTATE", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Eliminada", StringComparison.Ordinal),
            "Spanish deleted report settings property-grid selection should localize deleted settings state");
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Título del documento", StringComparison.Ordinal)) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "DOCUMENTTITLE", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal),
            "Spanish deleted report settings property-grid selection should localize document-title metadata labels");
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Expresión de orden activa", StringComparison.Ordinal)) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "SORTEXPRESSION", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal),
            "Spanish deleted report settings property-grid selection should localize deleted sort metadata labels");
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Margen inferior", StringComparison.Ordinal)) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "BOTMARGIN", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "55", StringComparison.Ordinal),
            "Spanish deleted report settings property-grid selection should localize deleted bottom-margin labels");
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Limites de vista previa eliminada", StringComparison.Ordinal)),
            "Spanish deleted report settings property-grid selection should localize deleted preview metadata labels");

        var portugueseSelection = CopperfinDesignerSelection.FromReportSettings(deletedSettings, new CopperfinLocalization("pt-BR"), deleted: true, reportLayout: deletedOnlySnapshot.ReportLayout);
        var portugueseProperties = TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().ToList();
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Estado das configurações", StringComparison.Ordinal)) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "SETTINGSSTATE", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Excluída", StringComparison.Ordinal),
            "Portuguese deleted report settings property-grid selection should localize deleted settings state");
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Título do documento", StringComparison.Ordinal)) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "DOCUMENTTITLE", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal),
            "Portuguese deleted report settings property-grid selection should localize document-title metadata labels");
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Expressão de ordenação ativa", StringComparison.Ordinal)) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "SORTEXPRESSION", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal),
            "Portuguese deleted report settings property-grid selection should localize deleted sort metadata labels");
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Margem inferior", StringComparison.Ordinal)) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "BOTMARGIN", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "55", StringComparison.Ordinal),
            "Portuguese deleted report settings property-grid selection should localize deleted bottom-margin labels");
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Limites da visualização excluída", StringComparison.Ordinal)),
            "Portuguese deleted report settings property-grid selection should localize deleted preview metadata labels");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        var pseudoSelection = CopperfinDesignerSelection.FromReportSettings(deletedSettings, pseudoLocalization, deleted: true, reportLayout: deletedOnlySnapshot.ReportLayout);
        var pseudoProperties = TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().ToList();
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.SettingsState"), StringComparison.Ordinal)) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "SETTINGSSTATE", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), pseudoLocalization.Text("AssetEditor.State.Deleted"), StringComparison.Ordinal),
            "Pseudo-localized deleted report settings property-grid selection should route deleted settings state through the shared catalog");
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.DocumentTitle"), StringComparison.Ordinal)) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "DOCUMENTTITLE", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "Deleted Customer Invoice", StringComparison.Ordinal),
            "Pseudo-localized deleted report settings property-grid selection should route document-title metadata through the shared catalog");
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ActiveSortExpression"), StringComparison.Ordinal)) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "SORTEXPRESSION", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "deleted.customer.country", StringComparison.Ordinal),
            "Pseudo-localized deleted report settings property-grid selection should route deleted sort metadata through the shared catalog");
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.BottomMargin"), StringComparison.Ordinal)) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "BOTMARGIN", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "55", StringComparison.Ordinal),
            "Pseudo-localized deleted report settings property-grid selection should route deleted bottom-margin metadata through the shared catalog");
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.DeletedPreviewBounds"), StringComparison.Ordinal)),
            "Pseudo-localized deleted report settings property-grid selection should route deleted preview metadata through the shared catalog");

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyReportSnapshotForExplorerSmoke(spanishControl, mixedSnapshot);
        Expect(GetPrivateListView(spanishControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, "Configuraciones (eliminada)", StringComparison.Ordinal)),
            "Spanish report explorer should localize the deleted settings scope row");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyReportSnapshotForExplorerSmoke(portugueseControl, mixedSnapshot);
        Expect(GetPrivateListView(portugueseControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, "Configurações (excluída)", StringComparison.Ordinal)),
            "Portuguese report explorer should localize the deleted settings scope row");

        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        ApplyReportSnapshotForExplorerSmoke(pseudoControl, mixedSnapshot);
        Expect(GetPrivateListView(pseudoControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, pseudoLocalization.Format("AssetEditor.ReportSection.Deleted", pseudoLocalization.Text("AssetEditor.ReportSection.Settings")), StringComparison.Ordinal)),
            "Pseudo-localized report explorer should route the deleted settings scope row through the shared catalog");

        Expect(deletedSettings[0].RecordIndex == 0 &&
               string.Equals(deletedSettings[0].Name, "ORIENTATION", StringComparison.Ordinal) &&
               string.Equals(deletedSettings[1].Name, "COLS", StringComparison.Ordinal) &&
               deletedSettings[1].SourceLineIndex == 1 &&
               string.Equals(deletedSettings[2].Name, "COLWIDTH", StringComparison.Ordinal) &&
               deletedSettings[2].SourceLineIndex == 2 &&
               string.Equals(deletedSettings[3].Name, "COLSPACING", StringComparison.Ordinal) &&
               deletedSettings[3].SourceLineIndex == 3 &&
               string.Equals(deletedSettings[4].Name, "PAPERLENGTH", StringComparison.Ordinal) &&
               deletedSettings[4].FieldIndex == 6 &&
               string.Equals(deletedSettings[5].Name, "PAPERWIDTH", StringComparison.Ordinal) &&
               deletedSettings[5].FieldIndex == 6 &&
               string.Equals(deletedSettings[6].Value, "40", StringComparison.Ordinal) &&
               string.Equals(deletedSettings[7].Name, "BOTMARGIN", StringComparison.Ordinal) &&
               deletedSettings[7].FieldIndex == 63 &&
               string.Equals(deletedSettings[8].Name, "LEFTMARGIN", StringComparison.Ordinal) &&
               deletedSettings[8].FieldIndex == 10 &&
               string.Equals(deletedSettings[9].Name, "RIGHTMARGIN", StringComparison.Ordinal) &&
               deletedSettings[9].FieldIndex == 11 &&
               string.Equals(deletedSettings[10].Name, "TAG", StringComparison.Ordinal) &&
               deletedSettings[10].FieldIndex == 9 &&
               deletedSettings[10].MemoBlockNumber == 21 &&
               string.Equals(deletedSettings[11].Name, "DRIVER", StringComparison.Ordinal) &&
               deletedSettings[11].FieldIndex == 6 &&
               string.Equals(deletedSettings[12].Name, "DEVICE", StringComparison.Ordinal) &&
               deletedSettings[12].FieldIndex == 6 &&
               string.Equals(deletedSettings[13].Name, "OUTPUT", StringComparison.Ordinal) &&
               deletedSettings[13].FieldIndex == 6 &&
               string.Equals(deletedSettings[14].Name, "DEFAULTSOURCE", StringComparison.Ordinal) &&
               deletedSettings[14].FieldIndex == 6 &&
               string.Equals(deletedSettings[15].Name, "PRINTQUALITY", StringComparison.Ordinal) &&
               deletedSettings[15].FieldIndex == 6 &&
               string.Equals(deletedSettings[16].Name, "YRESOLUTION", StringComparison.Ordinal) &&
               deletedSettings[16].FieldIndex == 6 &&
               string.Equals(deletedSettings[17].Name, "TTOPTION", StringComparison.Ordinal) &&
               deletedSettings[17].FieldIndex == 6 &&
               string.Equals(deletedSettings[18].Name, "COLOR", StringComparison.Ordinal) &&
               deletedSettings[18].FieldIndex == 6 &&
               string.Equals(deletedSettings[19].Name, "ASCII", StringComparison.Ordinal) &&
               deletedSettings[19].FieldIndex == 6 &&
               string.Equals(deletedSettings[20].Name, "COLLATE", StringComparison.Ordinal) &&
               deletedSettings[20].FieldIndex == 6 &&
               string.Equals(deletedSettings[21].Name, "COPIES", StringComparison.Ordinal) &&
               deletedSettings[21].FieldIndex == 6,
            "Localized deleted report settings property-grid selection should preserve deleted root-setting machine contracts");
    }

}
