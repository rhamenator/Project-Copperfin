
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
    private static void SmokeProjectDebugReplayWithRealAsset(string? path)
    {
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(path) ? "real project debug replay asset candidate" : path)} not found.");
            return;
        }

        #pragma warning disable VSTHRD002
        var session = CopperfinRuntimeDebugClient.StartSessionAsync(path!)
            .GetAwaiter()
            .GetResult();
        #pragma warning restore VSTHRD002
        Expect(session.Success, $"project debug replay should succeed for {path}");
        if (!session.Success)
        {
            return;
        }

        Expect(!string.IsNullOrWhiteSpace(session.ManifestPath) && File.Exists(session.ManifestPath),
            $"project debug replay should preserve the runtime manifest path for {path}");
        Expect(session.BuildWarningCount == 0,
            $"project debug replay should inherit a warning-free build for {path}");
        Expect(session.BuildWarnings.Count == 0,
            $"project debug replay should not inherit build warning lines for {path}");
        if (!string.IsNullOrWhiteSpace(session.ManifestPath) && File.Exists(session.ManifestPath))
        {
            var manifestText = File.ReadAllText(session.ManifestPath);
            Expect(manifestText.IndexOf("warning=", StringComparison.OrdinalIgnoreCase) < 0,
                $"project debug replay manifest should remain warning-free for {path}");
            Expect(manifestText.IndexOf("asset=6|wzcommon/registry.vcx|", StringComparison.Ordinal) >= 0,
                $"project debug replay manifest should stage the shared class dependency for {path}");
        }
        Expect(!string.IsNullOrWhiteSpace(session.State.Reason),
            $"project debug replay should surface a pause reason for {path}");
        Expect(session.State.Frames.Count > 0,
            $"project debug replay should surface a call stack for {path}");
        Expect(session.State.Events.Count > 0,
            $"project debug replay should surface runtime events for {path}");
    }

    private static void SmokeStandaloneStudioDocumentIdentity()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin-designer-smoke",
            "standalone-document-identity",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        var firstPath = Path.Combine(root, "first.prg");
        var secondPath = Path.Combine(root, "second.prg");
        try
        {
            File.WriteAllText(firstPath, "RETURN");
            File.WriteAllText(secondPath, "RETURN");
            SmokeStandaloneStudioWithMultipleAssets(firstPath, secondPath);
        }
        finally
        {
            try
            {
                if (Directory.Exists(root))
                {
                    Directory.Delete(root, recursive: true);
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

    private static void SmokeStandaloneStudioWithMultipleAssets(string? firstPath, string? secondPath)
    {
        if (string.IsNullOrWhiteSpace(firstPath) ||
            string.IsNullOrWhiteSpace(secondPath) ||
            !File.Exists(firstPath) ||
            !File.Exists(secondPath))
        {
            Console.WriteLine($"SKIP: {(string.IsNullOrWhiteSpace(firstPath) ? "real asset candidate" : firstPath)} or {(string.IsNullOrWhiteSpace(secondPath) ? "real asset candidate" : secondPath)} not found.");
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

        form.OpenDocument(firstPath!);
        form.Show();
        Application.DoEvents();
        form.OpenDocument(secondPath!);

        var loaded = WaitUntil(
            TimeSpan.FromSeconds(10),
            () => FindTabControls(form).Any(tab => tab.TabPages.Count >= 2) &&
                  FindTabControls(form).SelectMany(tab => tab.TabPages.Cast<TabPage>())
                      .Any(page => page.Text.Equals(Path.GetFileName(firstPath), StringComparison.OrdinalIgnoreCase)) &&
                  FindTabControls(form).SelectMany(tab => tab.TabPages.Cast<TabPage>())
                      .Any(page => page.Text.Equals(Path.GetFileName(secondPath), StringComparison.OrdinalIgnoreCase)));
        Expect(loaded, "standalone Studio should open multiple assets as separate tabs");

        var firstEditor = FindTabControls(form)
            .SelectMany(tab => tab.TabPages.Cast<TabPage>())
            .Where(page => page.Text.Equals(Path.GetFileName(firstPath), StringComparison.OrdinalIgnoreCase))
            .SelectMany(page => page.Controls.OfType<CopperfinAssetEditorControl>())
            .SingleOrDefault();
        var preHandleLoadFinished = WaitUntil(
            TimeSpan.FromSeconds(10),
            () => firstEditor?.SnapshotLoadFinished == true);
        Expect(preHandleLoadFinished,
            "standalone Studio should finish an asset snapshot that started before the form handle was created");

        var tabControl = FindTabControls(form).FirstOrDefault();
        Expect(tabControl is not null, "standalone Studio should surface a document tab control");
        if (tabControl is not null)
        {
            foreach (var expectedPath in new[]
            {
                CopperfinDocumentPathIdentity.Normalize(firstPath!),
                CopperfinDocumentPathIdentity.Normalize(secondPath!)
            })
            {
                var matchingPage = tabControl.TabPages.Cast<TabPage>()
                    .SingleOrDefault(page => string.Equals(page.ToolTipText, expectedPath, StringComparison.Ordinal));
                Expect(matchingPage is not null,
                    $"standalone Studio should retain an exact tab path binding for {expectedPath}");
                var editor = matchingPage?.Controls.OfType<CopperfinAssetEditorControl>().SingleOrDefault();
                Expect(editor is not null,
                    $"standalone Studio should retain an asset editor for {expectedPath}");
                if (editor is not null)
                {
                    Expect(
                        string.Equals(ReadPrivateStringField(editor, "currentPath"), expectedPath, StringComparison.Ordinal),
                        $"standalone Studio commands should remain bound to the exact selected document path {expectedPath}");
                }
            }

            var beforeDuplicateOpen = tabControl.TabPages.Count;
            form.OpenDocument(firstPath!);
            Application.DoEvents();
            Expect(tabControl.TabPages.Count == beforeDuplicateOpen, "opening an already open asset should not duplicate tabs");
            Expect(tabControl.SelectedTab is not null, "standalone Studio should keep a selected tab");
            Expect(tabControl.SelectedTab?.Text == Path.GetFileName(firstPath) || tabControl.SelectedTab?.Text == Path.GetFileName(secondPath),
                "standalone Studio should keep a valid selected asset tab");
        }

        TearDownForm(form);
    }

    private static void SmokeFreshRunVfpSourceStartupPaths()
    {
        var vfpSourceZipPath = ResolveFirstExistingRealAssetPath(
            ExpandUserPath(Environment.GetEnvironmentVariable("COPPERFIN_VFPSOURCE_ZIP")),
            ExpandUserPath("~/Downloads/VFPSource.zip"));
        if (string.IsNullOrWhiteSpace(vfpSourceZipPath) || !File.Exists(vfpSourceZipPath))
        {
            Console.WriteLine("SKIP: fresh-run VFPSource startup-path smoke requires a VFPSource.zip archive.");
            return;
        }

        var extractionBaseRoot = GetArchiveExtractionBaseRoot(vfpSourceZipPath!);
        try
        {
            if (Directory.Exists(extractionBaseRoot))
            {
                Directory.Delete(extractionBaseRoot, recursive: true);
            }
        }
        catch (IOException)
        {
        }
        catch (UnauthorizedAccessException)
        {
        }

        var extractedVfpSourceRoot = EnumerateResolvedRealAssetRoots()
            .FirstOrDefault(root =>
                string.Equals(
                    root,
                    Path.Combine(extractionBaseRoot, "VFPSource"),
                    StringComparison.OrdinalIgnoreCase));
        Expect(!string.IsNullOrWhiteSpace(extractedVfpSourceRoot),
            "fresh-run VFPSource startup-path smoke should materialize an extracted VFPSource root during root enumeration");
        if (string.IsNullOrWhiteSpace(extractedVfpSourceRoot))
        {
            return;
        }

        var projectPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "tasklist/tasklist.PJX");
        var programPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "tasklist/main.prg");
        var formPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "Wizards/wzapp/template/Books/Forms/books.scx");
        var classLibraryPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "EnvMgr/envmgr.vcx");
        var menuPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "ReportBuilder/handler_context.mnx");
        var reportPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "Wizards/wzapp/template/Books/Reports/by_author.FRX");
        var labelPath = TryResolveAssetUnderRoot(extractedVfpSourceRoot!, "Wizards/wzreport/STYLES/STYLELBL.LBX");

        Expect(!string.IsNullOrWhiteSpace(projectPath) && !string.IsNullOrWhiteSpace(programPath) &&
               !string.IsNullOrWhiteSpace(formPath) && !string.IsNullOrWhiteSpace(classLibraryPath) &&
               !string.IsNullOrWhiteSpace(menuPath) && !string.IsNullOrWhiteSpace(reportPath) &&
               !string.IsNullOrWhiteSpace(labelPath),
            "fresh-run VFPSource startup-path smoke should resolve project, program, form, class-library, menu, report, and label assets from the extracted root");
        if (string.IsNullOrWhiteSpace(projectPath) || string.IsNullOrWhiteSpace(programPath) ||
            string.IsNullOrWhiteSpace(formPath) || string.IsNullOrWhiteSpace(classLibraryPath) ||
            string.IsNullOrWhiteSpace(menuPath) || string.IsNullOrWhiteSpace(reportPath) ||
            string.IsNullOrWhiteSpace(labelPath))
        {
            return;
        }

        var labelSidecarPath = Path.ChangeExtension(labelPath, ".LBT");
        Expect(File.Exists(labelSidecarPath),
            "fresh-run VFPSource startup-path smoke should preserve the label sidecar beside STYLELBL.LBX");

        SmokeProjectEditorWithRealAsset(
            projectPath,
            expectGroups: new[] { "Forms", "Programs", "Class Libraries", "Classes", "Other Assets" });
        SmokeProgramEditorWithRealAsset(programPath);
        SmokeStandaloneStudioWithMultipleAssets(formPath, reportPath);
        SmokeVisualAssetEditorWithRealAsset(formPath, "form", "Visual form", "SCX/SCT assets", ".SCT");
        SmokeVisualAssetEditorWithRealAsset(classLibraryPath, "class_library", "Visual class library", "VCX/VCT assets", ".VCT");
        SmokeVisualAssetEditorWithRealAsset(menuPath, "menu", "Visual menu", "MNX/MNT assets", ".MNT");
        SmokeAssetEditorWithRealAsset(labelPath, expectSection: "Detail");
    }

    private static void SmokeConfiguredVfp9ZipAssetDiscovery()
    {
        var tempRoot = Path.Combine(Path.GetTempPath(), "CopperfinDesignerSmokeVfp9ZipDiscovery-" + Guid.NewGuid().ToString("N"));
        var zipPath = Path.Combine(tempRoot, "VFP9Samples.zip");
        var extractionBaseRoot = GetArchiveExtractionBaseRoot(zipPath);
        var previousVfp9Root = Environment.GetEnvironmentVariable("COPPERFIN_VFP9_ROOT");
        var previousVfp9Zip = Environment.GetEnvironmentVariable("COPPERFIN_VFP9_ZIP");

        Directory.CreateDirectory(tempRoot);

        try
        {
            if (Directory.Exists(extractionBaseRoot))
            {
                Directory.Delete(extractionBaseRoot, recursive: true);
            }

            using (var archive = ZipFile.Open(zipPath, ZipArchiveMode.Create))
            {
                CreateArchiveEntry(archive, "Samples/Solution/Reports/invoice.frx", "invoice");
                CreateArchiveEntry(archive, "Samples/Solution/Reports/cust.lbx", "cust");
                CreateArchiveEntry(archive, "Samples/Solution/solution.pjx", "solution");
                CreateArchiveEntry(archive, "Wizards/Template/Books/Forms/books.scx", "books");
            }

            Environment.SetEnvironmentVariable("COPPERFIN_VFP9_ROOT", null);
            Environment.SetEnvironmentVariable("COPPERFIN_VFP9_ZIP", zipPath);

            var resolvedInvoice = TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\invoice.frx");
            var resolvedLabel = TryResolveVfp9InstallAsset(@"Samples\Solution\Reports\cust.lbx");
            var resolvedSolution = TryResolveVfp9InstallAsset(@"Samples\Solution\solution.pjx");
            var resolvedBooksForm = TryResolveVfp9InstallAsset(@"Wizards\Template\Books\Forms\books.scx");

            Expect(string.Equals(
                    resolvedInvoice,
                    Path.Combine(extractionBaseRoot, "Samples", "Solution", "Reports", "invoice.frx"),
                    StringComparison.OrdinalIgnoreCase),
                "configured VFP9 zip discovery should resolve invoice.frx from the extracted sample root");
            Expect(string.Equals(
                    resolvedLabel,
                    Path.Combine(extractionBaseRoot, "Samples", "Solution", "Reports", "cust.lbx"),
                    StringComparison.OrdinalIgnoreCase),
                "configured VFP9 zip discovery should resolve cust.lbx from the extracted sample root");
            Expect(string.Equals(
                    resolvedSolution,
                    Path.Combine(extractionBaseRoot, "Samples", "Solution", "solution.pjx"),
                    StringComparison.OrdinalIgnoreCase),
                "configured VFP9 zip discovery should resolve solution.pjx from the extracted sample root");
            Expect(string.Equals(
                    resolvedBooksForm,
                    Path.Combine(extractionBaseRoot, "Wizards", "Template", "Books", "Forms", "books.scx"),
                    StringComparison.OrdinalIgnoreCase),
                "configured VFP9 zip discovery should resolve books.scx from the extracted sample root");

            var enumeratedRoots = EnumerateResolvedRealAssetRoots().ToList();
            Expect(enumeratedRoots.Any(root => string.Equals(root, extractionBaseRoot, StringComparison.OrdinalIgnoreCase)),
                "configured VFP9 zip discovery should surface the extracted sample root during real-asset root enumeration");

            var enumeratedReportAssets = EnumerateResolvedRealReportAssetPaths().ToList();
            Expect(enumeratedReportAssets.Any(path => string.Equals(path, resolvedInvoice, StringComparison.OrdinalIgnoreCase)),
                "configured VFP9 zip discovery should surface invoice.frx during report/label asset enumeration");
            Expect(enumeratedReportAssets.Any(path => string.Equals(path, resolvedLabel, StringComparison.OrdinalIgnoreCase)),
                "configured VFP9 zip discovery should surface cust.lbx during report/label asset enumeration");
        }
        finally
        {
            Environment.SetEnvironmentVariable("COPPERFIN_VFP9_ROOT", previousVfp9Root);
            Environment.SetEnvironmentVariable("COPPERFIN_VFP9_ZIP", previousVfp9Zip);

            try
            {
                if (Directory.Exists(extractionBaseRoot))
                {
                    Directory.Delete(extractionBaseRoot, recursive: true);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }

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

}
