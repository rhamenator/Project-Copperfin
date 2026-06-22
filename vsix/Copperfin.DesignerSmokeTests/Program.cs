using System;
using System.Collections.Generic;
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
        SmokeLocalizedAssetEditorChrome();
        SmokeLocalizedHostModeSubtitles();
        SmokeLocalizedProjectWorkspaceChrome();
        SmokeLocalizedProjectCommandDebuggerChrome();
        SmokeLocalizedProjectWorkspacePlaceholders();
        SmokeLocalizedExplorerColumnHeaders();
        SmokeLocalizedAssetFamilyGuidance();
        SmokeLocalizedSnapshotUndoPropertyStatus();
        SmokeLocalizedLaunchWorkflowDialogText();
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
            }
        };

        surface.LoadReportLayout(layout, objects);
        using var bitmap = new Bitmap(surface.Width, surface.Height);
        surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        Expect(CountNonWhitePixels(bitmap) > 5000, "synthetic report layout should render visible UI content");
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
        populateSectionListMethod.Invoke(control, Array.Empty<object>());
        configureObjectColumnsMethod.Invoke(control, Array.Empty<object>());
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
