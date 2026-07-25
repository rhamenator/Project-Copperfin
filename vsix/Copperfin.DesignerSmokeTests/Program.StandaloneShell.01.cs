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

        form.SubmitCommandForTest("? x");
        Application.DoEvents();

        Expect(form.CommandWindowTranscriptText.Contains("> ? x", StringComparison.Ordinal) &&
               form.CommandWindowTranscriptText.Contains("No hay una sesion de depuracion activa de Copperfin.", StringComparison.Ordinal),
            "standalone Command window should record supported input and localize the no-session response");
        var transcriptAfterCommand = form.CommandWindowTranscriptText;
        form.SubmitCommandForTest("   ");
        Expect(form.CommandWindowTranscriptText == transcriptAfterCommand,
            "standalone Command window should ignore empty submissions without changing the transcript");
        Expect(form.CommandWindowTranscriptText.StartsWith(initialTranscript, StringComparison.Ordinal),
            "standalone Command window should preserve its initial transcript");

        TearDownForm(form);
    }

    private static void SmokeStandaloneStudioToolWindowFloating()
    {
        var store = new InMemoryStudioShellLayoutStore();
        using (var form = new StudioMainForm(
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
            form.Show();
            Application.DoEvents();
            var initialTranscript = form.CommandWindowTranscriptText;

            form.SetCommandWindowFloatingForTest(true);
            Application.DoEvents();
            var commandBounds = new Rectangle(100, 120, 640, 360);
            form.SetCommandWindowFloatingBoundsForTest(commandBounds);
            Application.DoEvents();
            Expect(form.IsCommandWindowVisible && form.IsCommandWindowFloating,
                "standalone Command window should float without changing its visibility state");
            Expect(form.CommandWindowFloatingBoundsForTest == commandBounds,
                "standalone Command window should apply its requested floating bounds");
            Expect(form.FloatCommandWindowMenuText == "Acoplar ventana de comandos",
                "floating Command window should expose the localized dock command");

            form.SubmitCommandForTest("? x");
            Application.DoEvents();
            Expect(form.CommandWindowTranscriptText.StartsWith(initialTranscript, StringComparison.Ordinal) &&
                   form.CommandWindowTranscriptText.Contains("> ? x", StringComparison.Ordinal),
                "floating Command window should preserve its existing control and transcript behavior");

            form.SetCommandWindowFloatingForTest(false);
            Application.DoEvents();
            Expect(form.IsCommandWindowVisible && !form.IsCommandWindowFloating,
                "standalone Command window should re-dock without recreating the tool");
            Expect(form.FloatCommandWindowMenuText == "Flotar ventana de comandos",
                "docked Command window should expose the localized float command");

            form.SetTerminalWindowFloatingForTest(true);
            Application.DoEvents();
            Expect(form.IsTerminalWindowVisible && form.IsTerminalWindowFloating,
                "standalone Terminal should float without changing its visibility state");
            form.CloseTerminalFloatingWindowForTest();
            Application.DoEvents();
            Expect(form.IsTerminalWindowVisible && !form.IsTerminalWindowFloating,
                "closing a floating Terminal should return it to the docked pane");
            form.Close();
        }

        Expect(store.StoredState is not null &&
               store.StoredState.CommandWindowFloating == false &&
               store.StoredState.TerminalWindowFloating == false &&
               store.StoredState.CommandWindowFloatingWidth == 640 &&
               store.StoredState.CommandWindowFloatingHeight == 360,
            "standalone Studio should save the final docked tool-window state");

        using (var firstForm = new StudioMainForm(
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
            firstForm.Show();
            Application.DoEvents();
            firstForm.SetCommandWindowFloatingForTest(true);
            firstForm.SetTerminalWindowFloatingForTest(true);
            firstForm.SetCommandWindowFloatingBoundsForTest(new Rectangle(140, 150, 660, 380));
            firstForm.SetTerminalWindowFloatingBoundsForTest(new Rectangle(220, 190, 700, 400));
            firstForm.Close();
            Expect(firstForm.IsCommandWindowFloating && firstForm.IsTerminalWindowFloating,
                "owner teardown should preserve the logical floating state after owned forms close");
        }

        Expect(store.StoredState is not null &&
               store.StoredState.CommandWindowFloating &&
               store.StoredState.TerminalWindowFloating &&
               store.StoredState.CommandWindowFloatingX == 140 &&
               store.StoredState.CommandWindowFloatingY == 150 &&
               store.StoredState.CommandWindowFloatingWidth == 660 &&
               store.StoredState.CommandWindowFloatingHeight == 380 &&
               store.StoredState.TerminalWindowFloatingX == 220 &&
               store.StoredState.TerminalWindowFloatingY == 190 &&
               store.StoredState.TerminalWindowFloatingWidth == 700 &&
               store.StoredState.TerminalWindowFloatingHeight == 400,
            "standalone Studio should persist floating state for both tool windows");

        using (var restoredForm = new StudioMainForm(
                   new CopperfinLocalization("en-US"),
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
            Expect(restoredForm.IsCommandWindowFloating && restoredForm.IsTerminalWindowFloating,
                "standalone Studio should restore floating tool-window state");
            Expect(restoredForm.CommandWindowFloatingBoundsForTest == new Rectangle(140, 150, 660, 380) &&
                   restoredForm.TerminalWindowFloatingBoundsForTest == new Rectangle(220, 190, 700, 400),
                "standalone Studio should restore floating tool-window bounds");
            restoredForm.SetCommandWindowFloatingForTest(false);
            restoredForm.SetTerminalWindowFloatingForTest(false);
            restoredForm.Close();
        }

        store.StoredState = new StudioShellLayoutState
        {
            Version = StudioShellLayoutState.CurrentVersion,
            CommandWindowVisible = true,
            TerminalWindowVisible = true,
            SelectedToolWindow = StudioShellLayoutState.CommandWindowKey,
            SplitterDistance = 420,
            CommandWindowFloating = true,
            TerminalWindowFloating = false,
            CommandWindowFloatingX = -32000,
            CommandWindowFloatingY = -32000,
            CommandWindowFloatingWidth = 640,
            CommandWindowFloatingHeight = 360
        };
        using (var fallbackForm = new StudioMainForm(
                   new CopperfinLocalization("en-US"),
                   store)
               {
                   Width = 1200,
                   Height = 800,
                   ShowInTaskbar = false,
                   StartPosition = FormStartPosition.Manual,
                   Location = new Point(-32000, -32000)
               })
        {
            fallbackForm.Show();
            Application.DoEvents();
            Expect(fallbackForm.IsCommandWindowFloating &&
                   fallbackForm.CommandWindowFloatingBoundsForTest is Rectangle fallbackBounds &&
                   fallbackBounds.X != -32000 && fallbackBounds.Y != -32000,
                "standalone Studio should ignore wholly off-screen floating bounds");
            fallbackForm.Close();
        }
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
