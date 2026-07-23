// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private sealed class InMemoryStudioShellLayoutStore : IStudioShellLayoutStore
    {
        internal StudioShellLayoutState? StoredState { get; set; }

        public StudioShellLayoutState? Load()
        {
            return StoredState;
        }

        public void Save(StudioShellLayoutState state)
        {
            StoredState = state;
        }
    }

    private static void SmokeStandaloneStudioCommandWindowDocking()
    {
        using var form = new StudioMainForm(
            new CopperfinLocalization("es-419"),
            new InMemoryStudioShellLayoutStore())
        {
            Width = 1200,
            Height = 800,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        form.Show();
        Application.DoEvents();

        Expect(form.IsCommandWindowVisible,
            "standalone Studio should show the docked Command window by default");
        Expect(form.CommandWindowTabTitle == "Comando de Copperfin",
            "standalone Command window should use the active locale catalog");

        form.SetCommandWindowVisible(false);
        Application.DoEvents();
        Expect(!form.IsCommandWindowVisible,
            "standalone Command window should be hideable without closing document tabs");

        form.SetCommandWindowVisible(true);
        Application.DoEvents();
        Expect(form.IsCommandWindowVisible,
            "standalone Command window should be restorable as a docked pane");

        TearDownForm(form);
    }

    private static void SmokeStandaloneStudioCommandWindowInteraction()
    {
        using var form = new StudioMainForm(
            new CopperfinLocalization("es-419"),
            new InMemoryStudioShellLayoutStore())
        {
            Width = 1200,
            Height = 800,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };

        form.Show();
        Application.DoEvents();
        var initialTranscript = form.CommandWindowTranscriptText;

        form.SubmitCommandForTest("LIST");
        Application.DoEvents();

        Expect(form.CommandWindowTranscriptText.Contains("> LIST", StringComparison.Ordinal) &&
               form.CommandWindowTranscriptText.Contains("No se pudo abrir el comando de Copperfin.", StringComparison.Ordinal),
            "standalone Command window should record submitted commands and localize an unavailable executor");
        var transcriptAfterCommand = form.CommandWindowTranscriptText;
        form.SubmitCommandForTest("   ");
        Expect(form.CommandWindowTranscriptText == transcriptAfterCommand,
            "standalone Command window should ignore empty submissions without changing the transcript");
        Expect(form.CommandWindowTranscriptText.StartsWith(initialTranscript, StringComparison.Ordinal),
            "standalone Command window should preserve its initial transcript");

        TearDownForm(form);
    }

    private static void SmokeStandaloneStudioCloseDocumentTabs()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin-designer-smoke",
            "standalone-close-document",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);

        var firstPath = Path.Combine(root, "first.prg");
        var secondPath = Path.Combine(root, "second.prg");
        try
        {
            File.WriteAllText(firstPath, "RETURN");
            File.WriteAllText(secondPath, "RETURN");

            using var form = new StudioMainForm(
                new CopperfinLocalization("es-419"),
                new InMemoryStudioShellLayoutStore())
            {
                Width = 1200,
                Height = 800,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            form.Show();
            Application.DoEvents();
            Expect(form.CloseDocumentMenuText == "&Cerrar",
                "standalone Close command should use the active locale catalog");
            form.CloseActiveDocument();
            Expect(form.DocumentTabCount == 0,
                "closing with no standalone document should be a safe no-op");

            form.OpenDocument(firstPath);
            form.OpenDocument(secondPath);
            Application.DoEvents();
            var normalizedFirstPath = CopperfinDocumentPathIdentity.Normalize(firstPath);
            var normalizedSecondPath = CopperfinDocumentPathIdentity.Normalize(secondPath);
            Expect(form.DocumentTabCount == 2 &&
                   string.Equals(form.ActiveDocumentPath, normalizedSecondPath, StringComparison.Ordinal),
                "standalone Studio should make the newest document active");

            form.CloseActiveDocument();
            Application.DoEvents();
            Expect(form.DocumentTabCount == 1 &&
                   string.Equals(form.ActiveDocumentPath, normalizedFirstPath, StringComparison.Ordinal),
                "closing the active standalone document should select the remaining tab");

            form.CloseActiveDocument();
            Application.DoEvents();
            Expect(form.DocumentTabCount == 0 &&
                   form.ActiveDocumentPath is null,
                "closing the last standalone document should restore the empty shell");

            form.OpenDocument(firstPath);
            Application.DoEvents();
            Expect(form.DocumentTabCount == 1 &&
                   string.Equals(form.ActiveDocumentPath, normalizedFirstPath, StringComparison.Ordinal),
                "a closed standalone path should be reopenable as a fresh tab");

            TearDownForm(form);
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

    private static void SmokeStandaloneStudioShellLayoutPersistence()
    {
        var store = new InMemoryStudioShellLayoutStore();
        using (var firstForm = new StudioMainForm(
                   new CopperfinLocalization("es-419"),
                   store)
               {
                   Width = 1200,
                   Height = 800,
                   ShowInTaskbar = false,
                   StartPosition = FormStartPosition.Manual,
                   Location = new Point(-32000, -32000)
               })
        {
            firstForm.Show();
            Application.DoEvents();
            firstForm.SetCommandWindowVisible(false);
            firstForm.SetTerminalWindowVisible(true);
            firstForm.SelectTerminalWindow();
            firstForm.SetShellSplitterDistanceForTest(420);
            firstForm.Close();
        }

        Expect(store.StoredState is not null &&
               !store.StoredState.CommandWindowVisible &&
               store.StoredState.TerminalWindowVisible &&
               store.StoredState.SelectedToolWindow == StudioShellLayoutState.TerminalWindowKey &&
               store.StoredState.SplitterDistance == 420,
            "standalone Studio should save invariant shell layout state");

        using (var restoredForm = new StudioMainForm(
                   new CopperfinLocalization("pt-BR"),
                   store)
               {
                   Width = 1200,
                   Height = 800,
                   ShowInTaskbar = false,
                   StartPosition = FormStartPosition.Manual,
                   Location = new Point(-32000, -32000)
               })
        {
            restoredForm.Show();
            Application.DoEvents();
            Expect(!restoredForm.IsCommandWindowVisible &&
                   restoredForm.IsTerminalWindowVisible &&
                   restoredForm.SelectedToolWindowKey == StudioShellLayoutState.TerminalWindowKey &&
                   restoredForm.ShellSplitterDistance == 420,
                "standalone Studio should restore shell layout without localized contract keys");
            restoredForm.Close();
        }

        store.StoredState = new StudioShellLayoutState
        {
            Version = StudioShellLayoutState.CurrentVersion + 1,
            CommandWindowVisible = false,
            TerminalWindowVisible = false,
            SelectedToolWindow = "invalid",
            SplitterDistance = int.MaxValue
        };
        using var fallbackForm = new StudioMainForm(
            new CopperfinLocalization("en-US"),
            store)
        {
            Width = 1200,
            Height = 800,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000)
        };
        fallbackForm.Show();
        Application.DoEvents();
        Expect(fallbackForm.IsCommandWindowVisible &&
               fallbackForm.IsTerminalWindowVisible &&
               fallbackForm.SelectedToolWindowKey == StudioShellLayoutState.CommandWindowKey &&
               fallbackForm.ShellSplitterDistance >= 160 &&
               fallbackForm.ShellSplitterDistance < int.MaxValue,
            "standalone Studio should reject corrupt shell state and use bounded defaults");
        fallbackForm.Close();
    }

    private static void SmokeStandaloneStudioFileLayoutStoreRoundTrip()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin-designer-smoke",
            "standalone-shell-layout-store",
            Guid.NewGuid().ToString("N"));
        var path = Path.Combine(root, "shell-layout.json");
        try
        {
            var store = new StudioShellLayoutFileStore(path);
            store.Save(new StudioShellLayoutState
            {
                CommandWindowVisible = false,
                TerminalWindowVisible = true,
                SelectedToolWindow = StudioShellLayoutState.TerminalWindowKey,
                SplitterDistance = 420
            });
            var loaded = store.Load();
            Expect(loaded is not null &&
                   !loaded.CommandWindowVisible &&
                   loaded.TerminalWindowVisible &&
                   loaded.SelectedToolWindow == StudioShellLayoutState.TerminalWindowKey &&
                   loaded.SplitterDistance == 420,
                "standalone Studio should round-trip its versioned JSON layout store");

            File.WriteAllText(path, "not-json");
            Expect(store.Load() is null,
                "standalone Studio should ignore corrupt persisted layout JSON");
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
}
