
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

    private static void SmokeInvariantReportGeometryParsing()
    {
        var previousCulture = CultureInfo.CurrentCulture;
        try
        {
            CultureInfo.CurrentCulture = new CultureInfo("pt-BR");
            var snapshotObject = new CopperfinStudioSnapshotObject
            {
                RecordIndex = 7,
                Properties = new List<CopperfinStudioSnapshotProperty>
                {
                    new() { Name = "HPOS", Value = "100.750" },
                    new() { Name = "VPOS", Value = "200.250" },
                    new() { Name = "WIDTH", Value = "6666.667" },
                    new() { Name = "HEIGHT", Value = "300.125" }
                }
            };
            var method = typeof(CopperfinDesignSurfaceControl).GetMethod(
                "TryBuildBounds",
                BindingFlags.Static | BindingFlags.NonPublic);
            Expect(method is not null, "shared design surface should retain its geometry parsing hook");
            if (method is null)
            {
                return;
            }

            var arguments = new object?[] { "report", snapshotObject, null };
            var parsed = method.Invoke(null, arguments) is true;
            var bounds = arguments[2] is RectangleF value ? value : RectangleF.Empty;
            Expect(parsed && bounds.Left == 101 && bounds.Top == 200 && bounds.Width == 6667 && bounds.Height == 300,
                "FRX/LBX geometry should parse invariant decimal text under pt-BR without culture-scaled dimensions");
        }
        finally
        {
            CultureInfo.CurrentCulture = previousCulture;
        }
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
        var spanishLocalization = new CopperfinLocalization("es-419");
        using var spanishControl = new CopperfinAssetEditorControl(spanishLocalization);
        spanishControl.EmbeddedStudioShell = true;
        Expect(HasLabelText(spanishControl, "Diseñador visual de Copperfin"),
            "Spanish editor chrome should localize the asset editor title");
        Expect(HasLabelTextContaining(spanishControl, "activos visuales VFP"),
            "Spanish editor chrome should localize the asset editor subtitle");
        Expect(HasLabelTextContaining(spanishControl, "host nativo de Copperfin Studio"),
            "Spanish editor chrome should localize the asset editor guidance text");
        Expect(HasButtonText(spanishControl, "Abrir en Studio nativo") &&
               HasButtonText(spanishControl, spanishLocalization.Text("AssetEditor.RevealInExplorerButton")) &&
               HasButtonText(spanishControl, "Actualizar") &&
               HasButtonText(spanishControl, "Duplicar objeto") &&
               HasButtonText(spanishControl, "Eliminar objeto") &&
               HasButtonText(spanishControl, "Restaurar objeto"),
            "Spanish editor chrome should localize shell command buttons");

        var portugueseLocalization = new CopperfinLocalization("pt-BR");
        using var portugueseControl = new CopperfinAssetEditorControl(portugueseLocalization);
        portugueseControl.EmbeddedStudioShell = true;
        Expect(HasLabelText(portugueseControl, "Designer visual do Copperfin"),
            "Portuguese editor chrome should localize the asset editor title");
        Expect(HasLabelTextContaining(portugueseControl, "ativos visuais VFP"),
            "Portuguese editor chrome should localize the asset editor subtitle");
        Expect(HasLabelTextContaining(portugueseControl, "host nativo do Copperfin Studio"),
            "Portuguese editor chrome should localize the asset editor guidance text");
        Expect(HasButtonText(portugueseControl, "Abrir no Studio nativo") &&
               HasButtonText(portugueseControl, portugueseLocalization.Text("AssetEditor.RevealInExplorerButton")) &&
               HasButtonText(portugueseControl, "Atualizar") &&
               HasButtonText(portugueseControl, "Duplicar objeto") &&
               HasButtonText(portugueseControl, "Excluir objeto") &&
               HasButtonText(portugueseControl, "Restaurar objeto"),
            "Portuguese editor chrome should localize shell command buttons");
    }

    private static void SmokePseudoLocalizedAssetEditorChrome()
    {
        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        pseudoControl.EmbeddedStudioShell = true;

        Expect(HasLabelText(pseudoControl, pseudoLocalization.Text("AssetEditor.Title")),
            "Pseudo-localized editor chrome should route the asset editor title through the shared catalog");
        Expect(HasLabelTextContaining(pseudoControl, pseudoLocalization.Text("AssetEditor.StandaloneSubtitle")),
            "Pseudo-localized editor chrome should route the embedded subtitle through the shared catalog");
        Expect(HasButtonText(pseudoControl, pseudoLocalization.Text("AssetEditor.RefreshButton")) &&
               HasButtonText(pseudoControl, pseudoLocalization.Text("AssetEditor.ObjectLifecycle.DuplicateButton")) &&
               HasButtonText(pseudoControl, pseudoLocalization.Text("AssetEditor.ObjectLifecycle.DeleteButton")) &&
               HasButtonText(pseudoControl, pseudoLocalization.Text("AssetEditor.ObjectLifecycle.RestoreButton")) &&
               HasTabPageText(pseudoControl, pseudoLocalization.Text("AssetEditor.Tab.Summary")) &&
               HasLabelText(pseudoControl, pseudoLocalization.Text("AssetEditor.Debugger.ReadyStatus")),
            "Pseudo-localized editor chrome should route buttons, tabs, and status labels through the shared catalog");
        Expect(HasRichTextBoxTextContaining(pseudoControl, pseudoLocalization.Text("AssetEditor.Debugger.InitialSummary")),
            "Pseudo-localized editor chrome should route debugger pane guidance through the shared catalog");
    }

    private static void SmokeLocalizedHostModeChromeCompaction()
    {
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        var spanishTitleLabel = GetPrivateLabel(spanishControl, "titleLabel");
        var spanishSubtitleLabel = GetPrivateLabel(spanishControl, "subtitleLabel");
        var spanishGuidanceLabel = GetPrivateLabel(spanishControl, "guidanceLabel");
        var spanishLaunchButton = GetPrivateButton(spanishControl, "launchButton");
        Expect(!spanishTitleLabel.Visible &&
               !spanishSubtitleLabel.Visible &&
               !spanishGuidanceLabel.Visible &&
               spanishLaunchButton.Visible &&
               spanishControl.Padding == new Padding(12, 8, 12, 12),
            "Spanish Visual Studio host mode should suppress standalone title chrome and tighten editor padding");
        spanishControl.EmbeddedStudioShell = true;
        Expect(spanishTitleLabel.Visible &&
               spanishSubtitleLabel.Visible &&
               spanishGuidanceLabel.Visible &&
               !spanishLaunchButton.Visible &&
               spanishControl.Padding == new Padding(24) &&
               spanishSubtitleLabel.Text.IndexOf("superficie de diseñador usada dentro de Visual Studio", StringComparison.Ordinal) >= 0,
            "Spanish standalone host mode should restore localized standalone title chrome");
        spanishControl.EmbeddedStudioShell = false;
        Expect(!spanishTitleLabel.Visible &&
               !spanishSubtitleLabel.Visible &&
               !spanishGuidanceLabel.Visible &&
               spanishLaunchButton.Visible &&
               spanishControl.Padding == new Padding(12, 8, 12, 12),
            "Spanish Visual Studio host mode should reapply compact chrome after toggling back");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        var portugueseTitleLabel = GetPrivateLabel(portugueseControl, "titleLabel");
        var portugueseSubtitleLabel = GetPrivateLabel(portugueseControl, "subtitleLabel");
        var portugueseGuidanceLabel = GetPrivateLabel(portugueseControl, "guidanceLabel");
        var portugueseLaunchButton = GetPrivateButton(portugueseControl, "launchButton");
        Expect(!portugueseTitleLabel.Visible &&
               !portugueseSubtitleLabel.Visible &&
               !portugueseGuidanceLabel.Visible &&
               portugueseLaunchButton.Visible &&
               portugueseControl.Padding == new Padding(12, 8, 12, 12),
            "Portuguese Visual Studio host mode should suppress standalone title chrome and tighten editor padding");
        portugueseControl.EmbeddedStudioShell = true;
        Expect(portugueseTitleLabel.Visible &&
               portugueseSubtitleLabel.Visible &&
               portugueseGuidanceLabel.Visible &&
               !portugueseLaunchButton.Visible &&
               portugueseControl.Padding == new Padding(24) &&
               portugueseSubtitleLabel.Text.IndexOf("superfície de designer usada dentro do Visual Studio", StringComparison.Ordinal) >= 0,
            "Portuguese standalone host mode should restore localized standalone title chrome");
        portugueseControl.EmbeddedStudioShell = false;
        Expect(!portugueseTitleLabel.Visible &&
               !portugueseSubtitleLabel.Visible &&
               !portugueseGuidanceLabel.Visible &&
               portugueseLaunchButton.Visible &&
               portugueseControl.Padding == new Padding(12, 8, 12, 12),
            "Portuguese Visual Studio host mode should reapply compact chrome after toggling back");
    }

    private static void SmokeVisualStudioHostSurfaceThemeContract()
    {
        using var surface = new CopperfinDesignSurfaceControl(new CopperfinLocalization("en-US"))
        {
            Size = new Size(420, 280)
        };
        var hostBackground = Color.FromArgb(30, 34, 40);
        var hostForeground = Color.FromArgb(232, 236, 240);
        surface.ApplyVisualStudioHostTheme(hostBackground, hostForeground);
        var hostTheme = CopperfinDesignSurfaceTheme.FromHostColors(hostBackground, hostForeground);
        Expect(surface.BackColor == hostBackground,
            "VSIX host theme should route the shared design-surface background through the host color boundary");
        Expect(hostTheme.PageFill != Color.FromArgb(248, 249, 252) &&
               hostTheme.SectionFill != Color.White &&
               hostTheme.SectionHeaderFill != Color.FromArgb(233, 238, 247) &&
               hostTheme.SectionHeaderText == hostForeground &&
               CopperfinDesignSurfaceTheme.Default.SectionHeaderText == Color.FromArgb(44, 52, 64),
            "VSIX host theme should replace fixed light report chrome with host-derived colors");

        using var themedBitmap = new Bitmap(surface.Width, surface.Height);
        surface.DrawToBitmap(themedBitmap, new Rectangle(Point.Empty, surface.Size));
        Expect(themedBitmap.GetPixel(0, 0) == hostBackground,
            "VSIX host theme should paint the shared design-surface chrome with the resolved host background");

        var highContrastBackground = Color.FromArgb(255, 255, 255);
        var highContrastForeground = Color.FromArgb(0, 0, 0);
        surface.ApplyVisualStudioHostTheme(highContrastBackground, highContrastForeground, true);
        var highContrastTheme = CopperfinDesignSurfaceTheme.FromHostColors(
            highContrastBackground,
            highContrastForeground,
            true);
        Expect(highContrastTheme.PageFill == highContrastBackground &&
               highContrastTheme.SectionFill == highContrastBackground &&
               highContrastTheme.SectionBorder == highContrastForeground &&
               highContrastTheme.SectionHeaderText == highContrastForeground &&
               highContrastTheme.SelectedBorder == SystemColors.Highlight,
            "VSIX host theme should use system high-contrast roles instead of blended designer colors");
        using var highContrastBitmap = new Bitmap(surface.Width, surface.Height);
        surface.DrawToBitmap(highContrastBitmap, new Rectangle(Point.Empty, surface.Size));
        Expect(highContrastBitmap.GetPixel(0, 0) == highContrastBackground,
            "VSIX host theme should paint high-contrast surfaces with the host background");

        surface.ResetVisualStudioHostTheme();
        Expect(surface.BackColor == Color.White,
            "standalone surface reset should restore the shared designer palette after VSIX host theming");

        using var editor = new CopperfinAssetEditorControl(new CopperfinLocalization("en-US"));
        var editorSurface = GetPrivateField<CopperfinDesignSurfaceControl>(editor, "designSurface")!;
        editor.EmbeddedStudioShell = true;
        Expect(editorSurface.BackColor == Color.White,
            "switching back to standalone mode should restore the original shared designer palette");
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
               HasButtonText(spanishControl, "Salir") &&
               HasButtonText(spanishControl, "Evaluar") &&
               HasButtonText(spanishControl, "Agregar") &&
               HasButtonText(spanishControl, "Quitar") &&
               HasButtonText(spanishControl, "Borrar"),
            "Spanish project command and debugger chrome should localize buttons");
        Expect(HasLabelText(spanishControl, "Cargando instantánea de Copperfin Studio...") &&
               HasLabelText(spanishControl, "Depurador listo."),
            "Spanish project command and debugger chrome should localize status labels");
        Expect(HasRichTextBoxTextContaining(spanishControl, "sesión de depuración de Copperfin"),
            "Spanish debugger chrome should localize initial debugger guidance");
        var spanishDebuggerTabs = GetPrivateField<TabControl>(spanishControl, "debuggerDetailTabs");
        var spanishDebuggerCallStack = GetPrivateListView(spanishControl, "debuggerCallStackView");
        Expect(spanishDebuggerTabs is not null &&
               spanishDebuggerTabs.TabPages.Cast<TabPage>().Select(page => page.Text).SequenceEqual(new[]
               {
                   "Pila de llamadas", "Locales", "Globales", "Eventos de ejecución", "Expresiones vigiladas", "Puntos de interrupción"
               }) &&
               spanishDebuggerCallStack.Columns[0].Text == "Rutina" &&
               spanishDebuggerCallStack.Columns[1].Text == "Ubicación",
            "Spanish debugger detail tabs and columns should localize");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(HasButtonText(portugueseControl, "Compilar projeto Copperfin") &&
               HasButtonText(portugueseControl, "Executar projeto Copperfin") &&
               HasButtonText(portugueseControl, "Depurar projeto Copperfin") &&
               HasButtonText(portugueseControl, "Iniciar sessão") &&
               HasButtonText(portugueseControl, "Continuar") &&
               HasButtonText(portugueseControl, "Passo") &&
               HasButtonText(portugueseControl, "Próximo") &&
               HasButtonText(portugueseControl, "Sair") &&
               HasButtonText(portugueseControl, "Avaliar") &&
               HasButtonText(portugueseControl, "Adicionar") &&
               HasButtonText(portugueseControl, "Remover") &&
               HasButtonText(portugueseControl, "Limpar"),
            "Portuguese project command and debugger chrome should localize buttons");
        Expect(HasLabelText(portugueseControl, "Carregando instantâneo do Copperfin Studio...") &&
               HasLabelText(portugueseControl, "Depurador pronto."),
            "Portuguese project command and debugger chrome should localize status labels");
        Expect(HasRichTextBoxTextContaining(portugueseControl, "sessão de depuração do Copperfin"),
            "Portuguese debugger chrome should localize initial debugger guidance");
        var portugueseDebuggerTabs = GetPrivateField<TabControl>(portugueseControl, "debuggerDetailTabs");
        var portugueseDebuggerCallStack = GetPrivateListView(portugueseControl, "debuggerCallStackView");
        Expect(portugueseDebuggerTabs is not null &&
               portugueseDebuggerTabs.TabPages.Cast<TabPage>().Select(page => page.Text).SequenceEqual(new[]
               {
                   "Pilha de chamadas", "Locais", "Globais", "Eventos de runtime", "Expressões observadas", "Pontos de interrupção"
               }) &&
               portugueseDebuggerCallStack.Columns[0].Text == "Rotina" &&
               portugueseDebuggerCallStack.Columns[1].Text == "Localização",
            "Portuguese debugger detail tabs and columns should localize");
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
               BuildGuidanceText(spanishControl, "program").IndexOf("programas PRG", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(spanishControl, "unknown").IndexOf("instantánea estructurada", StringComparison.Ordinal) >= 0,
            "Spanish asset-family guidance should localize all static guidance cases");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("main.prg", new CopperfinLocalization("es-419")) == "Programa visual",
            "Spanish asset-kind labels should localize PRG assets as first-class program documents");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        Expect(BuildGuidanceText(portugueseControl, "form").IndexOf("objetos de formulário", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "class_library").IndexOf("biblioteca de classes", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "report").IndexOf("bandas e objetos de relatório", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "label").IndexOf("objetos de etiqueta", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "menu").IndexOf("estruturas de menu", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "project").IndexOf("espaços de trabalho agrupados", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "program").IndexOf("programas PRG", StringComparison.Ordinal) >= 0 &&
               BuildGuidanceText(portugueseControl, "unknown").IndexOf("instantâneo estruturado", StringComparison.Ordinal) >= 0,
            "Portuguese asset-family guidance should localize all static guidance cases");
        Expect(CopperfinStudioHostBridge.DescribeAssetKind("main.prg", new CopperfinLocalization("pt-BR")) == "Programa visual",
            "Portuguese asset-kind labels should localize PRG assets as first-class program documents");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        Expect(BuildGuidanceText(pseudoControl, "program").StartsWith("[!! ", StringComparison.Ordinal) &&
               CopperfinStudioHostBridge.DescribeAssetKind("main.prg", pseudoLocalization).StartsWith("[!! ", StringComparison.Ordinal),
            "Pseudo-localized program guidance and asset-kind labels should route through the shared catalog");
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
                    DeletedSections = new List<CopperfinStudioReportSection> { new(), new() },
                    Groupings = new List<CopperfinStudioReportGrouping> { new() },
                    Settings = new List<CopperfinStudioNamedValue>
                    {
                        new() { Name = "TAG", Value = "customer.country" },
                        new() { Name = "ORIENTATION", Value = "0" },
                        new() { Name = "PAPERSIZE", Value = "1" },
                        new() { Name = "PAPERLENGTH", Value = "2794" },
                        new() { Name = "PAPERWIDTH", Value = "2159" },
                        new() { Name = "TOPMARGIN", Value = "20" },
                        new() { Name = "GRIDV", Value = "4" },
                        new() { Name = "GRIDH", Value = "8" },
                        new() { Name = "COLOR", Value = "1" },
                        new() { Name = "COPIES", Value = "2" },
                        new() { Name = "DRIVER", Value = "winspool" },
                        new() { Name = "DEVICE", Value = "FinePrint 2000" },
                        new() { Name = "OUTPUT", Value = "FPR4:" },
                        new() { Name = "DEFAULTSOURCE", Value = "15" },
                        new() { Name = "PRINTQUALITY", Value = "600" },
                        new() { Name = "YRESOLUTION", Value = "600" },
                        new() { Name = "TTOPTION", Value = "3" },
                        new() { Name = "ASCII", Value = "9" },
                        new() { Name = "COLLATE", Value = "1" },
                        new() { Name = "LEFTMARGIN", Value = "15" },
                        new() { Name = "RIGHTMARGIN", Value = "25" }
                    },
                    DeletedSettings = new List<CopperfinStudioNamedValue>
                    {
                        new() { Name = "TAG", Value = "deleted.country" },
                        new() { Name = "ORIENTATION", Value = "9" },
                        new() { Name = "DRIVER", Value = "deleted-driver" }
                    },
                    UnplacedObjects = new List<CopperfinStudioReportLayoutObject> { new() },
                    DeletedObjects = new List<CopperfinStudioReportLayoutObject> { new(), new(), new(), new() }
                }
            };

            using var englishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("en-US"));
            var englishDetails = InvokeAssetEditorString(englishControl, "BuildSnapshotDetailsText", info, snapshot);
            Expect(englishDetails.IndexOf("Sections: 3", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Deleted sections: 2", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Groupings: 1", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Settings: 21", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Deleted settings: 3", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Unplaced objects: 1", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Deleted objects: 4", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Active Sort Expression: customer.country", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Orientation: 0", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Paper Size: 1", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Paper Length: 2794", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Paper Width: 2159", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Top Margin: 20", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Vertical Grid: 4", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Horizontal Grid: 8", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Color: 1", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Copies: 2", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Printer Driver: winspool", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Printer Device: FinePrint 2000", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Printer Output: FPR4:", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Default Source: 15", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Print Quality: 600", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Y Resolution: 600", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("TrueType Option: 3", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("ASCII: 9", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Collate: 1", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Left Margin: 15", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Right Margin: 25", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("deleted.country", StringComparison.Ordinal) < 0 &&
                   englishDetails.IndexOf("Orientation: 9", StringComparison.Ordinal) < 0 &&
                   englishDetails.IndexOf("Printer Driver: deleted-driver", StringComparison.Ordinal) < 0 &&
                   englishDetails.IndexOf("Bottom Margin:", StringComparison.Ordinal) < 0 &&
                   englishDetails.IndexOf("Preview bounds:", StringComparison.Ordinal) >= 0 &&
                   englishDetails.IndexOf("Deleted preview bounds:", StringComparison.Ordinal) >= 0,
                "English report layout shell summary should include live report counts plus root sort, printer, grid, and page-setup metadata without letting deleted settings override live values or fabricating missing optional fields");

            using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
            var spanishDetails = InvokeAssetEditorString(spanishControl, "BuildSnapshotDetailsText", info, snapshot);
            Expect(spanishDetails.IndexOf("Tamaño:", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Secciones: 3", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Secciones eliminadas: 2", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Agrupaciones: 1", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Configuraciones: 21", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Configuraciones eliminadas: 3", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Objetos sin sección: 1", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Objetos eliminados: 4", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Expresión de orden activa: customer.country", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Orientación: 0", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Tamaño de papel: 1", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Largo del papel: 2794", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Ancho del papel: 2159", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Margen superior: 20", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Cuadrícula vertical: 4", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Cuadrícula horizontal: 8", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Color: 1", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Copias: 2", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Controlador de impresora: winspool", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Dispositivo de impresora: FinePrint 2000", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Salida de impresora: FPR4:", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Fuente predeterminada: 15", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Calidad de impresión: 600", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Resolución Y: 600", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Opción TrueType: 3", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("ASCII: 9", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Intercalar: 1", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Margen izquierdo: 15", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Margen derecho: 25", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Limites de vista previa:", StringComparison.Ordinal) >= 0 &&
                   spanishDetails.IndexOf("Limites de vista previa eliminada:", StringComparison.Ordinal) >= 0,
                "Spanish report layout shell summary should localize file details, report counts, and root sort/printer/grid/page-setup metadata");

            using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
            var portugueseDetails = InvokeAssetEditorString(portugueseControl, "BuildSnapshotDetailsText", info, snapshot);
            Expect(portugueseDetails.IndexOf("Tamanho:", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Seções: 3", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Seções excluídas: 2", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Agrupamentos: 1", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Configurações: 21", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Configurações excluídas: 3", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Objetos sem seção: 1", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Objetos excluídos: 4", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Expressão de ordenação ativa: customer.country", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Orientação: 0", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Tamanho do papel: 1", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Comprimento do papel: 2794", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Largura do papel: 2159", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Margem superior: 20", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Grade vertical: 4", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Grade horizontal: 8", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Cor: 1", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Cópias: 2", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Controlador da impressora: winspool", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Dispositivo da impressora: FinePrint 2000", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Saída da impressora: FPR4:", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Origem padrão: 15", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Qualidade de impressão: 600", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Resolução Y: 600", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Opção TrueType: 3", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("ASCII: 9", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Intercalar: 1", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Margem esquerda: 15", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Margem direita: 25", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Limites da visualização:", StringComparison.Ordinal) >= 0 &&
                   portugueseDetails.IndexOf("Limites da visualização excluída:", StringComparison.Ordinal) >= 0,
                "Portuguese report layout shell summary should localize file details, report counts, and root sort/printer/grid/page-setup metadata");

            var pseudoLocalization = new CopperfinLocalization("qps-ploc");
            using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
            var pseudoDetails = InvokeAssetEditorString(pseudoControl, "BuildSnapshotDetailsText", info, snapshot);
            Expect(pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Details.ReportLayoutSummary").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf("3", StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf("2", StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf("4", StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.ActiveSortExpression").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.Orientation").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.PaperLength").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.VerticalGrid").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.HorizontalGrid").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.Color").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.Copies").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.PrinterDriver").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.PrinterDevice").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.PrinterOutput").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.DefaultSource").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.PrintQuality").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.YResolution").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.TrueTypeOption").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.Ascii").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.Collate").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Property.LeftMargin").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Details.ReportPreviewBoundsSummary").Substring(0, 6), StringComparison.Ordinal) >= 0 &&
                   pseudoDetails.IndexOf(pseudoLocalization.Text("AssetEditor.Details.DeletedReportPreviewBoundsSummary").Substring(0, 6), StringComparison.Ordinal) >= 0,
                "Pseudo-localized report layout shell summary should route root sort/printer/grid/page-setup metadata through the shared catalog");

            var deletedOnlySnapshot = new CopperfinStudioSnapshotDocument
            {
                AssetFamily = "label",
                ReportLayout = new CopperfinStudioReportLayout
                {
                    Sections = new List<CopperfinStudioReportSection> { new() },
                    DeletedSections = new List<CopperfinStudioReportSection> { new() },
                    Groupings = new List<CopperfinStudioReportGrouping>(),
                    Settings = new List<CopperfinStudioNamedValue>(),
                    DeletedSettings = new List<CopperfinStudioNamedValue>
                    {
                        new() { Name = "TAG", Value = "deleted.only.tag" },
                        new() { Name = "ORIENTATION", Value = "7" },
                        new() { Name = "GRIDV", Value = "6" },
                        new() { Name = "COPIES", Value = "4" },
                        new() { Name = "DRIVER", Value = "deleted-winspool" },
                        new() { Name = "OUTPUT", Value = "DELETED:" },
                        new() { Name = "DEFAULTSOURCE", Value = "16" },
                        new() { Name = "PRINTQUALITY", Value = "1200" },
                        new() { Name = "YRESOLUTION", Value = "1200" },
                        new() { Name = "TTOPTION", Value = "2" },
                        new() { Name = "ASCII", Value = "10" },
                        new() { Name = "COLLATE", Value = "0" }
                    },
                    UnplacedObjects = new List<CopperfinStudioReportLayoutObject>(),
                    DeletedObjects = new List<CopperfinStudioReportLayoutObject>()
                }
            };

            var deletedOnlyDetails = InvokeAssetEditorString(englishControl, "BuildSnapshotDetailsText", info, deletedOnlySnapshot);
            Expect(deletedOnlyDetails.IndexOf("Sections: 1", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Deleted sections: 1", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Groupings: 0", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Settings: 0", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Deleted settings: 12", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Active Sort Expression: deleted.only.tag", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Orientation: 7", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Vertical Grid: 6", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Copies: 4", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Printer Driver: deleted-winspool", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Printer Output: DELETED:", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Default Source: 16", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Print Quality: 1200", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Y Resolution: 1200", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("TrueType Option: 2", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("ASCII: 10", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Collate: 0", StringComparison.Ordinal) >= 0 &&
                   deletedOnlyDetails.IndexOf("Preview bounds:", StringComparison.Ordinal) < 0 &&
                   deletedOnlyDetails.IndexOf("Deleted preview bounds:", StringComparison.Ordinal) < 0,
                "Deleted-settings-only report layout shell summary should surface deleted root sort and printer/grid/page-setup metadata through the managed details path without fabricating preview bounds");
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
            Expression = "customer.company",
            GroupingContextAvailable = true,
            GroupingExpression = "customer.country"
        };
        spanishPropertyGrid.SelectedObject = CopperfinDesignerSelection.FromReportSection(reportSection, new CopperfinLocalization("es-419"));
        Expect(InvokeAssetEditorString(spanishControl, "BuildPropertyApplyingStatus", "EXPR").IndexOf("Aplicando cambio de Expresión", StringComparison.Ordinal) >= 0,
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
        Expect(InvokeAssetEditorString(portugueseControl, "BuildPropertyApplyingStatus", "EXPR").IndexOf("Aplicando alteração de Expressão", StringComparison.Ordinal) >= 0,
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

    private static void SmokeLocalizedCodeReferenceKindLabels()
    {
        var insights = new CopperfinProjectInsights
        {
            ProjectRoot = @"C:\src\sample",
            DefinedSymbols = new List<CopperfinProjectCodeSymbol>
            {
                new() { Kind = "class", Name = "app.customer.editor", FilePath = @"C:\src\sample\editor.prg", Line = 3 },
                new() { Kind = "definition", Name = "SaveOrder", FilePath = @"C:\src\sample\editor.prg", Line = 19 }
            },
            RuntimeReferences = new List<CopperfinProjectCodeSymbol>
            {
                new() { Kind = "reference", Name = "SaveOrder", FilePath = @"C:\src\sample\main.prg", Line = 41 },
                new() { Kind = "call.member", Name = "oToolbar.SaveOrder", FilePath = @"C:\src\sample\main.prg", Line = 42 }
            }
        };

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        var spanishSummary = InvokeAssetEditorString(spanishControl, "BuildCodeReferenceSummary", insights);
        Expect(spanishSummary.IndexOf("[Clase] app.customer.editor", StringComparison.Ordinal) >= 0 &&
               spanishSummary.IndexOf("[Definición] SaveOrder", StringComparison.Ordinal) >= 0 &&
               spanishSummary.IndexOf("[Referencia] SaveOrder", StringComparison.Ordinal) >= 0 &&
               spanishSummary.IndexOf("[Llamada a miembro] oToolbar.SaveOrder", StringComparison.Ordinal) >= 0,
            "Spanish code-reference summaries should localize displayed symbol kinds without changing symbol names");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        var portugueseSummary = InvokeAssetEditorString(portugueseControl, "BuildCodeReferenceSummary", insights);
        Expect(portugueseSummary.IndexOf("[Classe] app.customer.editor", StringComparison.Ordinal) >= 0 &&
               portugueseSummary.IndexOf("[Definição] SaveOrder", StringComparison.Ordinal) >= 0 &&
               portugueseSummary.IndexOf("[Referência] SaveOrder", StringComparison.Ordinal) >= 0 &&
               portugueseSummary.IndexOf("[Chamada de membro] oToolbar.SaveOrder", StringComparison.Ordinal) >= 0,
            "Portuguese code-reference summaries should localize displayed symbol kinds without changing symbol names");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        var pseudoSummary = InvokeAssetEditorString(pseudoControl, "BuildCodeReferenceSummary", insights);
        Expect(pseudoSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.SymbolKind.Class"), StringComparison.Ordinal) >= 0 &&
               pseudoSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.SymbolKind.Definition"), StringComparison.Ordinal) >= 0 &&
               pseudoSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.SymbolKind.Reference"), StringComparison.Ordinal) >= 0 &&
               pseudoSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.SymbolKind.MemberCall"), StringComparison.Ordinal) >= 0 &&
               pseudoSummary.IndexOf("[class]", StringComparison.Ordinal) < 0 &&
               pseudoSummary.IndexOf("[definition]", StringComparison.Ordinal) < 0 &&
               pseudoSummary.IndexOf("[reference]", StringComparison.Ordinal) < 0 &&
               pseudoSummary.IndexOf("[call.member]", StringComparison.Ordinal) < 0,
            "Pseudo-localized code-reference summaries should route displayed symbol kinds through the shared catalog instead of leaking raw tokens");
    }

}
