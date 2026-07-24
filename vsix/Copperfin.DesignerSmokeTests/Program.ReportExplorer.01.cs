
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
using System.Text.Json;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
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

    private static void SmokeHostShapedUnplacedReportObjectDeserialization()
    {
        const string json =
            "{\"AssetFamily\":\"report\",\"Objects\":[" +
            "{\"RecordIndex\":10,\"Title\":\"detail.line\"}," +
            "{\"RecordIndex\":12,\"Title\":\"orphan.note\"}]," +
            "\"ReportLayout\":{\"Sections\":[{\"Id\":\"detail\",\"Title\":\"Detail\",\"Objects\":[" +
            "{\"RecordIndex\":10,\"Title\":\"detail.line\",\"SectionObjectIndex\":0,\"SectionObjectCount\":1}]}]," +
            "\"UnplacedObjects\":[{\"RecordIndex\":12,\"Title\":\"orphan.note\",\"SectionObjectIndex\":null,\"SectionObjectCount\":0}]}}";
        var snapshot = JsonSerializer.Deserialize<CopperfinStudioSnapshotDocument>(
            json,
            new JsonSerializerOptions { PropertyNameCaseInsensitive = true });
        Expect(snapshot?.ReportLayout?.UnplacedObjects.Count == 1 &&
               snapshot.ReportLayout.UnplacedObjects[0].SectionObjectIndex is null,
            "host-shaped report JSON should deserialize its unplaced object with a null section index");
        if (snapshot?.ReportLayout is null)
        {
            return;
        }

        using var control = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(control, snapshot);

        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        var unplacedItem = sectionListView.Items.Cast<ListViewItem>()
            .SingleOrDefault(item => string.Equals(
                item.Text,
                new CopperfinLocalization("en-US").Text("AssetEditor.ReportSection.UnplacedObjects"),
                StringComparison.Ordinal));
        Expect(unplacedItem is not null,
            "host-shaped report JSON should expose an unplaced-object explorer row");
        if (unplacedItem is null)
        {
            return;
        }

        foreach (ListViewItem item in sectionListView.Items)
        {
            item.Selected = false;
        }
        unplacedItem.Selected = true;
        InvokeAssetEditorVoid(control, "SyncExplorerSelection");

        var unplacedRows = objectListView.Items.Cast<ListViewItem>().Select(item => item.Text).ToList();
        Expect(unplacedRows.SequenceEqual(new[] { "orphan.note" }),
            "host-shaped report JSON should route its unplaced object to the unplaced explorer surface");
    }

    private static void SmokeHostShapedReportAndLabelSectionOrdinals()
    {
        var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };
        foreach (var assetFamily in new[] { "report", "label" })
        {
            var json =
                "{\"AssetFamily\":\"" + assetFamily + "\",\"ReportLayout\":{" +
                "\"IsLabel\":" + (assetFamily == "label" ? "true" : "false") +
                ",\"Sections\":[{" +
                "\"Id\":\"detail_4\",\"SectionIndex\":2,\"SectionCount\":5," +
                "\"Title\":\"Detail\"}]," +
                "\"DeletedSections\":[{" +
                "\"Id\":\"deleted_detail\",\"Deleted\":true," +
                "\"SectionIndex\":null,\"SectionCount\":0}]}}";

            var document = JsonSerializer.Deserialize<CopperfinStudioSnapshotDocument>(json, options);
            var layout = document?.ReportLayout;
            var live = layout?.Sections.Count == 1 ? layout.Sections[0] : null;
            var deleted = layout?.DeletedSections.Count == 1 ? layout.DeletedSections[0] : null;

            Expect(document?.AssetFamily == assetFamily &&
                   layout?.IsLabel == (assetFamily == "label") &&
                   live is not null && live.SectionIndex == 2 && live.SectionCount == 5 &&
                   deleted is not null && deleted.Deleted &&
                   deleted.SectionIndex is null && deleted.SectionCount == 0,
                $"host-shaped {assetFamily} snapshots should preserve live and deleted section ordinals");
        }
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
                        PageBreak = "true",
                        ColumnBreak = "false",
                        ResetPage = "true",
                        EjectBefore = "true",
                        EjectAfter = "false",
                        Plain = "true",
                        OnEntryExpression = "DO ENTRY",
                        OnExitExpression = "DO EXIT",
                        Comment = "Band developer note",
                        DeletedObjectCount = 1,
                        Expression = "customer.company",
                        ExpressionFieldIndex = 3,
                        ExpressionMemoBlockNumber = 9,
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
                   string.Equals(topTarget, "VPOS", StringComparison.Ordinal) &&
                   string.Equals(topValue, "3200", StringComparison.Ordinal),
                "Report section property-grid selection should serialize TOP edits through the shared update path");

            TypeDescriptor.GetProperties(editableSelection)["HEIGHT"]?.SetValue(editableSelection, 6100);
            Expect(editableSelection.TryGetUpdate("HEIGHT", out var heightTarget, out var heightValue) &&
                   string.Equals(heightTarget, "HEIGHT", StringComparison.Ordinal) &&
                   string.Equals(heightValue, "6100", StringComparison.Ordinal),
                "Report section property-grid selection should serialize HEIGHT edits through the shared update path");

            ExpectSelectionUpdate(editableSelection, "PAGEBREAK", false, "false",
                "Report section property-grid selection should serialize PAGEBREAK edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COLBREAK", true, "true",
                "Report section property-grid selection should serialize COLBREAK edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "RESETPAGE", false, "false",
                "Report section property-grid selection should serialize RESETPAGE edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "EJECTBEFOR", false, "false",
                "Report section property-grid selection should serialize EJECTBEFOR edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "EJECTAFTER", true, "true",
                "Report section property-grid selection should serialize EJECTAFTER edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "PLAIN", false, "false",
                "Report section property-grid selection should serialize PLAIN edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "TAG", "UPDATED ENTRY", "UPDATED ENTRY",
                "Report section property-grid selection should serialize TAG edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "TAG2", "UPDATED EXIT", "UPDATED EXIT",
                "Report section property-grid selection should serialize TAG2 edits through the shared update path");
            ExpectSelectionUpdate(editableSelection, "COMMENT", "UPDATED BAND NOTE", "UPDATED BAND NOTE",
                "Report section property-grid selection should serialize COMMENT edits through the shared update path");

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
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Iniciar en una página nueva", StringComparison.Ordinal)) &&
               spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Iniciar en una columna nueva", StringComparison.Ordinal)) &&
               spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Restablecer número de página", StringComparison.Ordinal)),
            "Spanish report section property-grid selection should localize pagination flag labels");
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Expulsar antes", StringComparison.Ordinal)) &&
               spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Expulsar después", StringComparison.Ordinal)) &&
               spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Altura de banda constante", StringComparison.Ordinal)),
            "Spanish report section property-grid selection should localize optional-band flag labels");
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Expresión al entrar", StringComparison.Ordinal)) &&
               spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Expresión al salir", StringComparison.Ordinal)),
            "Spanish report section property-grid selection should localize band event expression labels");
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Comentarios", StringComparison.Ordinal)),
            "Spanish report section property-grid selection should localize COMMENT labels");
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
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Expresión", StringComparison.Ordinal)) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "EXPR", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "customer.company", StringComparison.Ordinal) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "GROUPINGEXPRESSION", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "customer.country", StringComparison.Ordinal),
            "Spanish report section property-grid selection should separate section expression edits from grouping-expression metadata");
        Expect(spanishSectionProperties.Any(property => string.Equals(property.DisplayName, "Campo de la expresión", StringComparison.Ordinal)) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "EXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "3", StringComparison.Ordinal) &&
               string.Equals(spanishSectionProperties.First(property => string.Equals(property.Name, "EXPRESSIONMEMO", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "9", StringComparison.Ordinal),
            "Spanish report section property-grid selection should expose section-expression provenance metadata");
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
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Iniciar em uma nova página", StringComparison.Ordinal)) &&
               portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Iniciar em uma nova coluna", StringComparison.Ordinal)) &&
               portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Redefinir número da página", StringComparison.Ordinal)),
            "Portuguese report section property-grid selection should localize pagination flag labels");
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Ejetar antes", StringComparison.Ordinal)) &&
               portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Ejetar depois", StringComparison.Ordinal)) &&
               portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Altura constante da banda", StringComparison.Ordinal)),
            "Portuguese report section property-grid selection should localize optional-band flag labels");
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Expressão ao entrar", StringComparison.Ordinal)) &&
               portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Expressão ao sair", StringComparison.Ordinal)),
            "Portuguese report section property-grid selection should localize band event expression labels");
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
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Expressão", StringComparison.Ordinal)) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "EXPR", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "customer.company", StringComparison.Ordinal) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "GROUPINGEXPRESSION", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "customer.country", StringComparison.Ordinal),
            "Portuguese report section property-grid selection should separate section expression edits from grouping-expression metadata");
        Expect(portugueseSectionProperties.Any(property => string.Equals(property.DisplayName, "Campo da expressão", StringComparison.Ordinal)) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "EXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "3", StringComparison.Ordinal) &&
               string.Equals(portugueseSectionProperties.First(property => string.Equals(property.Name, "EXPRESSIONMEMO", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "9", StringComparison.Ordinal),
            "Portuguese report section property-grid selection should expose section-expression provenance metadata");
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
        Expect(pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.PageBreak"), StringComparison.Ordinal)) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ColumnBreak"), StringComparison.Ordinal)) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ResetPage"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route pagination flag labels through the shared catalog");
        Expect(pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.EjectBefore"), StringComparison.Ordinal)) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.EjectAfter"), StringComparison.Ordinal)) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ConstantBandHeight"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route optional-band flag labels through the shared catalog");
        Expect(pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.OnEntryExpression"), StringComparison.Ordinal)) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.OnExitExpression"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route band event expression labels through the shared catalog");
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
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "EXPR", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "customer.company", StringComparison.Ordinal) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.Expression"), StringComparison.Ordinal)) &&
               string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "GROUPINGEXPRESSION", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "customer.country", StringComparison.Ordinal),
            "Pseudo-localized report section property-grid selection should keep section expression editing separate from grouped metadata");
        Expect(string.Equals(pseudoSectionProperties.First(property => string.Equals(property.Name, "EXPRESSIONFIELD", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), "3", StringComparison.Ordinal) &&
               pseudoSectionProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.ExpressionMemoBlock"), StringComparison.Ordinal)),
            "Pseudo-localized report section property-grid selection should route section-expression provenance metadata through the shared catalog");
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
        Expect(snapshot.ReportLayout.Sections[0].ExpressionFieldIndex == 3 &&
               snapshot.ReportLayout.Sections[0].ExpressionMemoBlockNumber == 9,
            "Localized report section property-grid section-expression provenance should preserve section snapshot contracts");
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

    private static void SmokeReportGroupingExplorerSelection()
    {
        var grouping = new CopperfinStudioReportGrouping
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
        };
        var groupingOnlySnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            ReportLayout = new CopperfinStudioReportLayout
            {
                Groupings = new List<CopperfinStudioReportGrouping> { grouping }
            }
        };
        var mixedSnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "report",
            ReportLayout = new CopperfinStudioReportLayout
            {
                Groupings = new List<CopperfinStudioReportGrouping> { grouping },
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
        var expectedGrouping = new ExpectedReportGroupingMetadata
        {
            GroupingIndex = 1,
            GroupingNestingDepth = 2,
            GroupingExpression = "customer.country",
            GroupingExpressionFieldIndex = 2,
            GroupingExpressionMemoBlockNumber = 7,
            HeaderSectionId = "group_header_7",
            HeaderRecordIndex = 41,
            HeaderDeleted = false,
            FooterSectionId = "group_footer_7",
            FooterRecordIndex = 47,
            FooterDeleted = true,
            HeaderStateDisplay = "Live",
            FooterStateDisplay = "Deleted"
        };

        using var control = new CopperfinAssetEditorControl();
        ApplyReportSnapshotForExplorerSmoke(control, groupingOnlySnapshot);
        var sectionListView = GetPrivateListView(control, "sectionListView");
        var objectListView = GetPrivateListView(control, "objectListView");
        InvokeAssetEditorVoid(control, "SyncExplorerSelection");

        var propertyGrid = GetPrivatePropertyGrid(control);
        Expect(propertyGrid.SelectedObject is CopperfinDesignerSelection groupingSelection &&
               groupingSelection.RecordIndex == 41 &&
               objectListView.Items.Count == 0 &&
               SelectionMatchesExpectedReportGrouping(groupingSelection, expectedGrouping),
            "Report grouping explorer selection should produce a grouping-rooted property-grid selection and clear object rows");

        if (propertyGrid.SelectedObject is CopperfinDesignerSelection editableSelection)
        {
            TypeDescriptor.GetProperties(editableSelection)["GROUPINGEXPRESSION"]?.SetValue(editableSelection, "customer.region");
            Expect(editableSelection.TryGetUpdate("GROUPINGEXPRESSION", out var exprTarget, out var exprValue) &&
                   string.Equals(exprTarget, "EXPR", StringComparison.Ordinal) &&
                   string.Equals(exprValue, "customer.region", StringComparison.Ordinal),
                "Report grouping property-grid selection should serialize grouping-expression edits through the invariant EXPR update path");
        }

        using var labelControl = new CopperfinAssetEditorControl();
        var labelSnapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "label",
            ReportLayout = new CopperfinStudioReportLayout
            {
                IsLabel = true,
                Groupings = new List<CopperfinStudioReportGrouping> { grouping }
            }
        };
        ApplyReportSnapshotForExplorerSmoke(labelControl, labelSnapshot);
        GetPrivateListView(labelControl, "sectionListView").Items.Cast<ListViewItem>()
            .First(item => string.Equals(item.Text, "Grouping 1 - customer.country", StringComparison.Ordinal))
            .Selected = true;
        InvokeAssetEditorVoid(labelControl, "SyncExplorerSelection");
        var labelPropertyGrid = GetPrivatePropertyGrid(labelControl);
        Expect(labelPropertyGrid.SelectedObject is CopperfinDesignerSelection labelSelection &&
               labelSelection.RecordIndex == 41 &&
               SelectionMatchesExpectedReportGrouping(labelSelection, expectedGrouping),
            "Label grouping explorer selection should expose the same shared grouping-rooted property-grid selection");

        if (labelPropertyGrid.SelectedObject is CopperfinDesignerSelection editableLabelSelection)
        {
            TypeDescriptor.GetProperties(editableLabelSelection)["GROUPINGEXPRESSION"]?.SetValue(editableLabelSelection, "customer.region");
            Expect(editableLabelSelection.TryGetUpdate("GROUPINGEXPRESSION", out var exprTarget, out var exprValue) &&
                   string.Equals(exprTarget, "EXPR", StringComparison.Ordinal) &&
                   string.Equals(exprValue, "customer.region", StringComparison.Ordinal),
                "Label grouping property-grid selection should preserve the shared invariant EXPR update path");
        }

        var spanishSelection = CopperfinDesignerSelection.FromReportGrouping(grouping, new CopperfinLocalization("es-419"));
        var spanishProperties = TypeDescriptor.GetProperties(spanishSelection).Cast<PropertyDescriptor>().ToList();
        Expect(spanishProperties.Any(property => string.Equals(property.DisplayName, "Índice de agrupación", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Profundidad de agrupación", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Expresión de agrupación", StringComparison.Ordinal)) &&
               spanishProperties.Any(property => string.Equals(property.DisplayName, "Estado de encabezado", StringComparison.Ordinal)) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "GROUPHEADERSTATE", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Activa", StringComparison.Ordinal) &&
               string.Equals(spanishProperties.First(property => string.Equals(property.Name, "GROUPFOOTERSTATE", StringComparison.Ordinal)).GetValue(spanishSelection)?.ToString(), "Eliminada", StringComparison.Ordinal),
            "Spanish report grouping property-grid selection should localize grouping metadata labels and state values");

        var portugueseSelection = CopperfinDesignerSelection.FromReportGrouping(grouping, new CopperfinLocalization("pt-BR"));
        var portugueseProperties = TypeDescriptor.GetProperties(portugueseSelection).Cast<PropertyDescriptor>().ToList();
        Expect(portugueseProperties.Any(property => string.Equals(property.DisplayName, "Índice de agrupamento", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Profundidade de agrupamento", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Expressão de agrupamento", StringComparison.Ordinal)) &&
               portugueseProperties.Any(property => string.Equals(property.DisplayName, "Estado do cabeçalho", StringComparison.Ordinal)) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "GROUPHEADERSTATE", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Ativa", StringComparison.Ordinal) &&
               string.Equals(portugueseProperties.First(property => string.Equals(property.Name, "GROUPFOOTERSTATE", StringComparison.Ordinal)).GetValue(portugueseSelection)?.ToString(), "Excluída", StringComparison.Ordinal),
            "Portuguese report grouping property-grid selection should localize grouping metadata labels and state values");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        var pseudoSelection = CopperfinDesignerSelection.FromReportGrouping(grouping, pseudoLocalization);
        var pseudoProperties = TypeDescriptor.GetProperties(pseudoSelection).Cast<PropertyDescriptor>().ToList();
        Expect(pseudoProperties.Any(property => string.Equals(property.DisplayName, pseudoLocalization.Text("AssetEditor.Property.GroupingHeaderState"), StringComparison.Ordinal)) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "GROUPHEADERSTATE", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), pseudoLocalization.Text("AssetEditor.State.Live"), StringComparison.Ordinal) &&
               string.Equals(pseudoProperties.First(property => string.Equals(property.Name, "GROUPFOOTERSTATE", StringComparison.Ordinal)).GetValue(pseudoSelection)?.ToString(), pseudoLocalization.Text("AssetEditor.State.Deleted"), StringComparison.Ordinal),
            "Pseudo-localized report grouping property-grid selection should route grouping metadata through the shared catalog");

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyReportSnapshotForExplorerSmoke(spanishControl, mixedSnapshot);
        Expect(GetPrivateListView(spanishControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, "Agrupación 1 - customer.country", StringComparison.Ordinal)),
            "Spanish report explorer should localize grouping rows");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyReportSnapshotForExplorerSmoke(portugueseControl, mixedSnapshot);
        Expect(GetPrivateListView(portugueseControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, "Agrupamento 1 - customer.country", StringComparison.Ordinal)),
            "Portuguese report explorer should localize grouping rows");

        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        ApplyReportSnapshotForExplorerSmoke(pseudoControl, mixedSnapshot);
        Expect(GetPrivateListView(pseudoControl, "sectionListView").Items.Cast<ListViewItem>()
                   .Any(item => string.Equals(item.Text, pseudoLocalization.Format("AssetEditor.ReportSection.GroupingWithExpression", 1, "customer.country"), StringComparison.Ordinal)),
            "Pseudo-localized report explorer should route grouping rows through the shared catalog");

        Expect(grouping.GroupingIndex == 1 &&
               grouping.NestingDepth == 2 &&
               string.Equals(grouping.Expression, "customer.country", StringComparison.Ordinal) &&
               grouping.ExpressionFieldIndex == 2 &&
               grouping.ExpressionMemoBlockNumber == 7 &&
               string.Equals(grouping.HeaderSectionId, "group_header_7", StringComparison.Ordinal) &&
               grouping.HeaderRecordIndex == 41 &&
               !grouping.HeaderDeleted &&
               string.Equals(grouping.FooterSectionId, "group_footer_7", StringComparison.Ordinal) &&
               grouping.FooterRecordIndex == 47 &&
               grouping.FooterDeleted,
            "Localized report grouping property-grid selection should preserve grouping machine contracts");
    }

}
