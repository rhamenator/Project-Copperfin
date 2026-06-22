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
                ["Studio.AssetKind.Generic"] = "Copperfin asset"
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
                ["Studio.AssetKind.Generic"] = "Activo Copperfin"
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
                ["Studio.AssetKind.Generic"] = "Ativo Copperfin"
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
