// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Globalization;
using System.IO;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void TestVsixEditorPaneUsesCurrentUiCulture()
    {
        var repositoryRoot = FindRepositoryRoot();
        Expect(repositoryRoot is not null,
            "VSIX editor pane localization test should locate the repository root");
        if (repositoryRoot is null)
        {
            return;
        }

        var paneSource = File.ReadAllText(Path.Combine(
            repositoryRoot,
            "vsix",
            "Copperfin.VisualStudio",
            "CopperfinAssetEditorPane.cs"));
        Expect(paneSource.Contains(
                "private const int OleCmdErrNotSupported = unchecked((int)0x80040100);",
                StringComparison.Ordinal) &&
               paneSource.Contains("return OleCmdErrNotSupported;", StringComparison.Ordinal) &&
               !paneSource.Contains("NativeMethods.OLECMDERR_E_NOTSUPPORTED", StringComparison.Ordinal),
            "VSIX editor command routing should use an accessible OLE unsupported-command HRESULT");
        Expect(paneSource.Contains("VSConstants.VSStd97CmdID.CloseDocument", StringComparison.Ordinal) &&
               paneSource.Contains("SVsShellMonitorSelection", StringComparison.Ordinal) &&
               paneSource.Contains("VSConstants.VSSELELEMID.SEID_WindowFrame", StringComparison.Ordinal) &&
               paneSource.Contains("windowFrame.CloseFrame", StringComparison.Ordinal),
            "VSIX editor command routing should close the active document frame and release its tab/RDT ownership");
        Expect(paneSource.Contains(
                "new CopperfinAssetEditorControl(CopperfinLocalization.FromCurrentUiCulture())",
                StringComparison.Ordinal),
            "VSIX editor pane should pass Visual Studio's current UI culture to the shared editor");
        Expect(paneSource.Contains("control.Dock = DockStyle.Fill;", StringComparison.Ordinal),
            "VSIX editor pane should fill the Visual Studio document frame with the shared editor");
        Expect(paneSource.Contains("public override IWin32Window Window => control;", StringComparison.Ordinal) &&
               !paneSource.Contains("public override object Content => control;", StringComparison.Ordinal),
            "VSIX editor pane should route the WinForms control through Window instead of the WPF Content host");

        var previousUiLocale = Environment.GetEnvironmentVariable("COPPERFIN_UI_LOCALE");
        var previousLocale = Environment.GetEnvironmentVariable("COPPERFIN_LOCALE");
        var previousCulture = CultureInfo.CurrentUICulture;
        try
        {
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", null);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", null);

            CultureInfo.CurrentUICulture = new CultureInfo("es-419");
            var spanish = CopperfinLocalization.FromCurrentUiCulture();
            Expect(spanish.Locale == CopperfinLocalization.SpanishLatinAmericaLocale &&
                   spanish.Text("AssetEditor.Title") == "Diseñador visual de Copperfin",
                "VSIX editor pane localization should follow a Spanish Visual Studio UI culture");

            CultureInfo.CurrentUICulture = new CultureInfo("pt-BR");
            var portuguese = CopperfinLocalization.FromCurrentUiCulture();
            Expect(portuguese.Locale == CopperfinLocalization.PortugueseBrazilLocale &&
                   portuguese.Text("AssetEditor.Title") == "Designer visual do Copperfin",
                "VSIX editor pane localization should follow a Brazilian Portuguese Visual Studio UI culture");
        }
        finally
        {
            CultureInfo.CurrentUICulture = previousCulture;
            Environment.SetEnvironmentVariable("COPPERFIN_UI_LOCALE", previousUiLocale);
            Environment.SetEnvironmentVariable("COPPERFIN_LOCALE", previousLocale);
        }
    }
}
