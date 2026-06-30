using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
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
        SmokeAssetEditorLabelSectionPropertyGridHostUpdate();
        SmokeAssetEditorReportObjectPropertyGridHostUpdate();
        SmokeAssetEditorLabelObjectPropertyGridHostUpdate();
        SmokeReportObjectPropertyGridLocalization();
        SmokeLocalizedReportObjectKindSubtitles();
        SmokeLocalizedReportObjectFallbackTitles();
        SmokeReportSelectionPreservedAcrossExplorerRefresh();
        SmokeDeletedReportSectionExplorerSelection();
        SmokeReportSurfaceScopeSelection();
        SmokeReportSurfaceObjectScopeAlignment();
        SmokeReportSurfaceObjectDragging();
        SmokeAssetEditorReportDragUsesBatchStudioHostUpdate();
        SmokeDeletedReportSectionDesignSurfaceRendering();
        SmokeAssetEditorWithRealAsset(
            @"C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\invoice.frx",
            expectSection: "Detail");
        SmokeAssetEditorWithRealAsset(
            @"C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\cust.lbx",
            expectSection: "Detail");
        SmokeProjectEditorWithRealAsset(
            @"C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\solution.pjx",
            expectGroup: "Forms");
        SmokeProjectDebuggerWithRealAsset(
            @"C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\solution.pjx");
        SmokeStandaloneStudioWithMultipleAssets(
            @"C:\Program Files (x86)\Microsoft Visual FoxPro 9\Wizards\Template\Books\Forms\books.scx",
            @"C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\invoice.frx");

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
               HasButtonText(spanishControl, "Actualizar"),
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
               HasButtonText(portugueseControl, "Atualizar"),
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
            SetPrivateField(control, "currentPath", Path.Combine(tempRoot, "invoice.frx"));
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
            SetPrivateField(control, "currentPath", Path.Combine(tempRoot, "invoice.frx"));
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
            SetPrivateField(control, "currentPath", Path.Combine(tempRoot, "cust.lbx"));
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
            SetPrivateField(control, "currentPath", Path.Combine(tempRoot, "cust.lbx"));
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
            SetPrivateField(control, "currentPath", Path.Combine(tempRoot, "invoice.frx"));
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

    private static void SmokeAssetEditorWithRealAsset(string path, string expectSection)
    {
        if (!File.Exists(path))
        {
            Console.WriteLine($"SKIP: {path} not found.");
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
        control.LoadDocument(path);

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

    private static void SmokeProjectEditorWithRealAsset(string path, string expectGroup)
    {
        if (!File.Exists(path))
        {
            Console.WriteLine($"SKIP: {path} not found.");
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
        control.LoadDocument(path);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(8),
            () => FindListViews(control).Any(list => list.Items.Count > 0) &&
                  FindRichTextBoxes(control).Any(box => !string.IsNullOrWhiteSpace(box.Text)));
        Expect(loaded, $"project editor should load grouped workspace data for {path}");

        var groupFound = FindListViews(control)
            .SelectMany(list => list.Items.Cast<ListViewItem>())
            .Any(item => string.Equals(item.Text, expectGroup, StringComparison.OrdinalIgnoreCase));
        Expect(groupFound, $"project editor should surface group '{expectGroup}' for {path}");

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

    private static void SmokeProjectDebuggerWithRealAsset(string path)
    {
        if (!File.Exists(path))
        {
            Console.WriteLine($"SKIP: {path} not found.");
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
        control.LoadDocument(path);

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

    private static void SmokeStandaloneStudioWithMultipleAssets(string firstPath, string secondPath)
    {
        if (!File.Exists(firstPath) || !File.Exists(secondPath))
        {
            Console.WriteLine($"SKIP: {firstPath} or {secondPath} not found.");
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
        form.OpenDocument(firstPath);
        form.OpenDocument(secondPath);

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
            form.OpenDocument(firstPath);
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

    private static string BuildBatchUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
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
