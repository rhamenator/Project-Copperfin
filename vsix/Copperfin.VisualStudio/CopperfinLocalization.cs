using System;
using System.Collections.Generic;
using System.Globalization;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinLocalization
{
    public const string DefaultLocale = "en";
    public const string SpanishLatinAmericaLocale = "es-419";
    public const string PortugueseBrazilLocale = "pt-BR";

    public static readonly IReadOnlyList<string> SupportedLocales = new[]
    {
        DefaultLocale,
        SpanishLatinAmericaLocale,
        PortugueseBrazilLocale
    };

    private static readonly IReadOnlyDictionary<string, IReadOnlyDictionary<string, string>> Catalogs =
        new Dictionary<string, IReadOnlyDictionary<string, string>>(StringComparer.OrdinalIgnoreCase)
        {
            [DefaultLocale] = new Dictionary<string, string>(StringComparer.Ordinal)
            {
                ["Studio.AppTitle"] = "Copperfin Studio",
                ["Studio.FileMenu"] = "&File",
                ["Studio.OpenMenu"] = "&Open...",
                ["Studio.ExitMenu"] = "E&xit",
                ["Studio.EmptyDocumentStatus"] = "Open one or more VFP assets to inspect and edit them in Copperfin Studio.",
                ["Studio.InitialStatus"] = "Open a VFP asset to inspect and edit it in Copperfin Studio.",
                ["Studio.MissingAssetMessage"] = "The selected asset does not exist.",
                ["Studio.OpenDialogTitle"] = "Open Copperfin Asset",
                ["Studio.OpenDialogFilter"] = "Copperfin/VFP assets|*.pjx;*.scx;*.vcx;*.frx;*.lbx;*.mnx|All files|*.*",
                ["Studio.WindowTitleWithAssetKind"] = "Copperfin Studio - {0}",
                ["Studio.OpenDocumentStatus"] = "{0}   |   {1}   |   Open tabs: {2}",
                ["Studio.AssetKind.Project"] = "Visual project",
                ["Studio.AssetKind.Form"] = "Visual form",
                ["Studio.AssetKind.ClassLibrary"] = "Visual class library",
                ["Studio.AssetKind.Report"] = "Visual report",
                ["Studio.AssetKind.Label"] = "Visual label",
                ["Studio.AssetKind.Menu"] = "Visual menu",
                ["Studio.AssetKind.Generic"] = "Copperfin asset",
                ["AssetEditor.Title"] = "Copperfin Visual Designer",
                ["AssetEditor.Subtitle"] = "This Visual Studio editor is the handoff point into Copperfin Studio. It is meant for VFP visual assets such as forms, reports, labels, menus, class libraries, and projects.",
                ["AssetEditor.Guidance"] = "This shell now pulls a structured snapshot from the native Copperfin Studio host. For VFP visual assets, that gives us a real object/property view while we work toward high-fidelity inline designers.",
                ["AssetEditor.OpenNativeStudioButton"] = "Open In Native Studio",
                ["AssetEditor.RevealInExplorerButton"] = "Reveal In Explorer",
                ["AssetEditor.RefreshButton"] = "Refresh",
                ["AssetEditor.Tab.Summary"] = "Summary",
                ["AssetEditor.Tab.Debugger"] = "Debugger",
                ["AssetEditor.Tab.TaskList"] = "Task List",
                ["AssetEditor.Tab.CodeReferences"] = "Code References",
                ["AssetEditor.Tab.DataExplorer"] = "Data Explorer",
                ["AssetEditor.Tab.ObjectBrowser"] = "Object Browser",
                ["AssetEditor.Tab.Toolbox"] = "Toolbox",
                ["AssetEditor.Tab.Builders"] = "Builders",
                ["AssetEditor.Tab.Coverage"] = "Coverage",
                ["AssetEditor.Tab.Database"] = "Database",
                ["AssetEditor.ObjectBrowser.HideProjectRecords"] = "Hide project records"
            },
            [SpanishLatinAmericaLocale] = new Dictionary<string, string>(StringComparer.Ordinal)
            {
                ["Studio.AppTitle"] = "Copperfin Studio",
                ["Studio.FileMenu"] = "&Archivo",
                ["Studio.OpenMenu"] = "&Abrir...",
                ["Studio.ExitMenu"] = "&Salir",
                ["Studio.EmptyDocumentStatus"] = "Abra uno o más activos VFP para inspeccionarlos y editarlos en Copperfin Studio.",
                ["Studio.InitialStatus"] = "Abra un activo VFP para inspeccionarlo y editarlo en Copperfin Studio.",
                ["Studio.MissingAssetMessage"] = "El activo seleccionado no existe.",
                ["Studio.OpenDialogTitle"] = "Abrir activo de Copperfin",
                ["Studio.OpenDialogFilter"] = "Activos Copperfin/VFP|*.pjx;*.scx;*.vcx;*.frx;*.lbx;*.mnx|Todos los archivos|*.*",
                ["Studio.WindowTitleWithAssetKind"] = "Copperfin Studio - {0}",
                ["Studio.OpenDocumentStatus"] = "{0}   |   {1}   |   Pestañas abiertas: {2}",
                ["Studio.AssetKind.Project"] = "Proyecto visual",
                ["Studio.AssetKind.Form"] = "Formulario visual",
                ["Studio.AssetKind.ClassLibrary"] = "Biblioteca de clases visual",
                ["Studio.AssetKind.Report"] = "Informe visual",
                ["Studio.AssetKind.Label"] = "Etiqueta visual",
                ["Studio.AssetKind.Menu"] = "Menú visual",
                ["Studio.AssetKind.Generic"] = "Activo Copperfin",
                ["AssetEditor.Title"] = "Diseñador visual de Copperfin",
                ["AssetEditor.Subtitle"] = "Este editor de Visual Studio es el punto de entrega hacia Copperfin Studio. Está pensado para activos visuales VFP como formularios, informes, etiquetas, menús, bibliotecas de clases y proyectos.",
                ["AssetEditor.Guidance"] = "Este shell ahora obtiene una instantánea estructurada desde el host nativo de Copperfin Studio. Para activos visuales VFP, eso nos da una vista real de objetos y propiedades mientras avanzamos hacia diseñadores integrados de alta fidelidad.",
                ["AssetEditor.OpenNativeStudioButton"] = "Abrir en Studio nativo",
                ["AssetEditor.RevealInExplorerButton"] = "Mostrar en Explorer",
                ["AssetEditor.RefreshButton"] = "Actualizar",
                ["AssetEditor.Tab.Summary"] = "Resumen",
                ["AssetEditor.Tab.Debugger"] = "Depurador",
                ["AssetEditor.Tab.TaskList"] = "Lista de tareas",
                ["AssetEditor.Tab.CodeReferences"] = "Referencias de código",
                ["AssetEditor.Tab.DataExplorer"] = "Explorador de datos",
                ["AssetEditor.Tab.ObjectBrowser"] = "Explorador de objetos",
                ["AssetEditor.Tab.Toolbox"] = "Herramientas",
                ["AssetEditor.Tab.Builders"] = "Constructores",
                ["AssetEditor.Tab.Coverage"] = "Cobertura",
                ["AssetEditor.Tab.Database"] = "Base de datos",
                ["AssetEditor.ObjectBrowser.HideProjectRecords"] = "Ocultar registros del proyecto"
            },
            [PortugueseBrazilLocale] = new Dictionary<string, string>(StringComparer.Ordinal)
            {
                ["Studio.AppTitle"] = "Copperfin Studio",
                ["Studio.FileMenu"] = "&Arquivo",
                ["Studio.OpenMenu"] = "&Abrir...",
                ["Studio.ExitMenu"] = "&Sair",
                ["Studio.EmptyDocumentStatus"] = "Abra um ou mais ativos VFP para inspecionar e editar no Copperfin Studio.",
                ["Studio.InitialStatus"] = "Abra um ativo VFP para inspecionar e editar no Copperfin Studio.",
                ["Studio.MissingAssetMessage"] = "O ativo selecionado não existe.",
                ["Studio.OpenDialogTitle"] = "Abrir ativo do Copperfin",
                ["Studio.OpenDialogFilter"] = "Ativos Copperfin/VFP|*.pjx;*.scx;*.vcx;*.frx;*.lbx;*.mnx|Todos os arquivos|*.*",
                ["Studio.WindowTitleWithAssetKind"] = "Copperfin Studio - {0}",
                ["Studio.OpenDocumentStatus"] = "{0}   |   {1}   |   Abas abertas: {2}",
                ["Studio.AssetKind.Project"] = "Projeto visual",
                ["Studio.AssetKind.Form"] = "Formulário visual",
                ["Studio.AssetKind.ClassLibrary"] = "Biblioteca de classes visual",
                ["Studio.AssetKind.Report"] = "Relatório visual",
                ["Studio.AssetKind.Label"] = "Etiqueta visual",
                ["Studio.AssetKind.Menu"] = "Menu visual",
                ["Studio.AssetKind.Generic"] = "Ativo Copperfin",
                ["AssetEditor.Title"] = "Designer visual do Copperfin",
                ["AssetEditor.Subtitle"] = "Este editor do Visual Studio é o ponto de entrega para o Copperfin Studio. Ele foi criado para ativos visuais VFP, como formulários, relatórios, etiquetas, menus, bibliotecas de classes e projetos.",
                ["AssetEditor.Guidance"] = "Este shell agora obtém um instantâneo estruturado do host nativo do Copperfin Studio. Para ativos visuais VFP, isso oferece uma visão real de objetos e propriedades enquanto avançamos para designers integrados de alta fidelidade.",
                ["AssetEditor.OpenNativeStudioButton"] = "Abrir no Studio nativo",
                ["AssetEditor.RevealInExplorerButton"] = "Revelar no Explorer",
                ["AssetEditor.RefreshButton"] = "Atualizar",
                ["AssetEditor.Tab.Summary"] = "Resumo",
                ["AssetEditor.Tab.Debugger"] = "Depurador",
                ["AssetEditor.Tab.TaskList"] = "Lista de tarefas",
                ["AssetEditor.Tab.CodeReferences"] = "Referências de código",
                ["AssetEditor.Tab.DataExplorer"] = "Explorador de dados",
                ["AssetEditor.Tab.ObjectBrowser"] = "Navegador de objetos",
                ["AssetEditor.Tab.Toolbox"] = "Ferramentas",
                ["AssetEditor.Tab.Builders"] = "Construtores",
                ["AssetEditor.Tab.Coverage"] = "Cobertura",
                ["AssetEditor.Tab.Database"] = "Banco de dados",
                ["AssetEditor.ObjectBrowser.HideProjectRecords"] = "Ocultar registros do projeto"
            }
        };

    public CopperfinLocalization(string? requestedLocale = null)
    {
        Locale = NormalizeLocale(requestedLocale);
    }

    public string Locale { get; }

    public static CopperfinLocalization FromEnvironment()
    {
        return new CopperfinLocalization(Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE"));
    }

    public static string NormalizeLocale(string? requestedLocale)
    {
        if (string.IsNullOrWhiteSpace(requestedLocale))
        {
            return DefaultLocale;
        }

        var normalized = (requestedLocale ?? string.Empty).Trim().Replace('_', '-');
        if (normalized.Equals(DefaultLocale, StringComparison.OrdinalIgnoreCase) ||
            normalized.StartsWith(DefaultLocale + "-", StringComparison.OrdinalIgnoreCase))
        {
            return DefaultLocale;
        }

        if (normalized.Equals("es", StringComparison.OrdinalIgnoreCase) ||
            normalized.StartsWith("es-", StringComparison.OrdinalIgnoreCase))
        {
            return SpanishLatinAmericaLocale;
        }

        if (normalized.Equals("pt", StringComparison.OrdinalIgnoreCase) ||
            normalized.StartsWith("pt-", StringComparison.OrdinalIgnoreCase))
        {
            return PortugueseBrazilLocale;
        }

        return DefaultLocale;
    }

    public string Text(string key)
    {
        if (Catalogs.TryGetValue(Locale, out var catalog) && catalog.TryGetValue(key, out var localized))
        {
            return localized;
        }

        return Catalogs[DefaultLocale].TryGetValue(key, out var fallback) ? fallback : key;
    }

    public string Format(string key, params object[] args)
    {
        return string.Format(CultureInfo.InvariantCulture, Text(key), args);
    }
}
