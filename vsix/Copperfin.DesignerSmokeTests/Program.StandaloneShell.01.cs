// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private const uint EmReplaceSel = 0x00C2;

    [DllImport("user32.dll", CharSet = CharSet.Unicode, EntryPoint = "SendMessageW")]
    private static extern IntPtr SendMessageReplaceSelection(
        IntPtr windowHandle,
        uint message,
        IntPtr recordUndo,
        string replacement);

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

    private static void SmokeStandaloneStudioProjectCommands()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin-designer-smoke",
            "standalone-project-commands",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        var projectPath = Path.Combine(root, "commands.pjx");
        var programPath = Path.Combine(root, "commands.prg");

        try
        {
            File.WriteAllText(projectPath, string.Empty);
            File.WriteAllText(programPath, "RETURN");
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

            Expect(form.BuildProjectMenuText == "&Compilar proyecto" &&
                   form.RunProjectMenuText == "&Ejecutar sin depurar" &&
                   form.DebugProjectMenuText == "Iniciar &depuración",
                "standalone project commands should use the active locale catalog");
            Expect(form.BuildProjectShortcutKeys == (Keys.Control | Keys.Shift | Keys.B) &&
                   form.RunProjectShortcutKeys == (Keys.Control | Keys.F5) &&
                   form.DebugProjectShortcutKeys == Keys.F5,
                "standalone project commands should expose conventional Windows shortcuts");
            Expect(!form.ProjectCommandsEnabled,
                "standalone project commands should be disabled without an active project");

            form.OpenDocument(projectPath);
            form.Show();
            Application.DoEvents();
            Expect(form.ProjectCommandsEnabled,
                "standalone project commands should be enabled for an active PJX document");

            var projectEditor = FindTabControls(form)
                .SelectMany(tab => tab.TabPages.Cast<TabPage>())
                .Where(page => string.Equals(page.ToolTipText, projectPath, StringComparison.OrdinalIgnoreCase))
                .SelectMany(page => page.Controls.OfType<CopperfinAssetEditorControl>())
                .Single();
            projectEditor.SuppressProjectWorkflowDialogs = true;
            Expect(form.RunActiveProjectWorkflowForTest(CopperfinProjectOperation.Build),
                "standalone Build command should route to the active project editor");
            Expect(WaitUntil(
                    TimeSpan.FromSeconds(10),
                    () => GetPrivateField<CopperfinProjectExecutionResult>(
                        projectEditor,
                        "currentProjectWorkflowResult") is not null),
                "standalone Build command should execute the shared project workflow");

            form.OpenDocument(programPath);
            Application.DoEvents();
            Expect(!form.ProjectCommandsEnabled,
                "standalone project commands should be disabled for a non-project active document");
            Expect(!form.RunActiveProjectWorkflowForTest(CopperfinProjectOperation.Build),
                "standalone project commands should not route through a non-project active document");

            form.CloseActiveDocument();
            Application.DoEvents();
            Expect(form.ProjectCommandsEnabled,
                "standalone project commands should refresh when closing back to a PJX document");
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

    private static void SmokeStandaloneStudioUndoCommand()
    {
        var root = Path.Combine(
            Path.GetTempPath(),
            "copperfin-designer-smoke",
            "standalone-undo-command",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        var programPath = Path.Combine(root, "undo.prg");

        try
        {
            File.WriteAllText(programPath, "RETURN");
            var spanish = new CopperfinLocalization("es-419");
            using var form = new StudioMainForm(
                spanish,
                new InMemoryStudioShellLayoutStore())
            {
                Width = 1200,
                Height = 800,
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.Manual,
                Location = new Point(-32000, -32000)
            };

            Expect(form.EditMenuText == "&Editar" &&
                   form.UndoMenuText == "Deshacer" &&
                   form.UndoShortcutKeys == (Keys.Control | Keys.Z),
                "standalone Edit/Undo should use the active locale and conventional Windows shortcut");
            Expect(!form.UndoCommandEnabled && !form.TryUndoActiveDocumentForTest(),
                "standalone Undo should be disabled and safely unhandled without an active document");

            form.OpenDocument(programPath);
            form.Show();
            Application.DoEvents();
            var editor = FindTabControls(form)
                .SelectMany(tab => tab.TabPages.Cast<TabPage>())
                .Where(page => string.Equals(page.ToolTipText, programPath, StringComparison.OrdinalIgnoreCase))
                .SelectMany(page => page.Controls.OfType<CopperfinAssetEditorControl>())
                .Single();
            var editBox = new TextBox
            {
                Text = "before",
                Width = 240,
                Height = 28
            };
            editor.Controls.Add(editBox);
            editBox.BringToFront();
            editBox.Focus();
            Application.DoEvents();
            ReplaceSelectionWithUndo(editBox, "after");

            Expect(editBox.CanUndo,
                "standalone Undo smoke should create a real focused text edit in the shared editor");
            form.OpenEditMenuForTest();
            Expect(form.UndoCommandEnabled && form.UndoMenuText == "Deshacer",
                "opening standalone Edit should refresh Undo availability from the active editor state");
            Expect(form.TryUndoActiveDocumentForTest() && editBox.Text == "before",
                "standalone Undo should route to the active editor's existing focused-text undo stack");
            Expect(form.UndoCommandEnabled == editBox.CanUndo,
                "standalone Undo should refresh availability from the focused text control's native undo state");
            if (editBox.CanUndo)
            {
                Expect(form.TryUndoActiveDocumentForTest() && editBox.Text == "after",
                    "standalone Undo should preserve the native reversible single-level edit contract");
            }

            editBox.Text = "before";
            editBox.Focus();
            Application.DoEvents();
            ReplaceSelectionWithUndo(editBox, "after");
            Expect(form.ProcessCmdKeyForTest(Keys.Control | Keys.Z) &&
                   editBox.Text == "before",
                "standalone Ctrl+Z should route while focus remains inside the active document");

            SetCurrentSnapshot(editor, BuildStatusSmokeSnapshot());
            var commandInput = form.CommandWindowInputForTest;
            commandInput.Text = "before command";
            commandInput.Focus();
            Application.DoEvents();
            ReplaceSelectionWithUndo(commandInput, "after command");
            Expect(commandInput.CanUndo && editor.CanHandleUndoCommand(),
                "standalone shortcut scope smoke should cover competing tool-window and host-backed undo stacks");
            Expect(!form.ProcessCmdKeyForTest(Keys.Control | Keys.Z) &&
                   commandInput.Text == "after command" &&
                   GetCurrentSnapshot(editor).CommandUndoAvailable,
                "standalone Ctrl+Z should propagate without consuming document undo while a tool-window input has focus");
            commandInput.Undo();
            Expect(commandInput.Text == "before command",
                "propagated standalone Ctrl+Z should leave the focused tool-window undo stack available");

            form.CloseActiveDocument();
            Application.DoEvents();
            Expect(!form.UndoCommandEnabled && !form.TryUndoActiveDocumentForTest(),
                "standalone Undo should disable and safely no-op after closing the active document");

            var pseudo = new CopperfinLocalization(CopperfinLocalization.PseudoLocale);
            using var pseudoForm = new StudioMainForm(
                pseudo,
                new InMemoryStudioShellLayoutStore());
            Expect(pseudoForm.EditMenuText == pseudo.Text("Studio.EditMenu") &&
                   pseudoForm.UndoMenuText == pseudo.Text("AssetEditor.Undo.Command"),
                "standalone Undo should remain pseudo-localization ready");
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

    private static void ReplaceSelectionWithUndo(TextBoxBase textBox, string replacement)
    {
        textBox.ClearUndo();
        textBox.SelectAll();
        if (Environment.OSVersion.Platform == PlatformID.Win32NT)
        {
            SendMessageReplaceSelection(
                textBox.Handle,
                EmReplaceSel,
                new IntPtr(1),
                replacement);
            return;
        }

        textBox.SelectedText = replacement;
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

    private static void SmokeDesignerHarnessCleanupClosesOwnedForms()
    {
        using var owner = new Form
        {
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000),
            Width = 640,
            Height = 360,
            Text = "Copperfin smoke owner"
        };
        using var child = new Form
        {
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new Point(-32000, -32000),
            Width = 320,
            Height = 180,
            Text = "Copperfin smoke child",
            Owner = owner
        };

        owner.Show();
        child.Show(owner);
        Application.DoEvents();
        Expect(Application.OpenForms.Cast<Form>().Contains(owner) &&
               Application.OpenForms.Cast<Form>().Contains(child),
            "smoke cleanup regression should create an owned form tree");

        CloseHarnessForms();
        Expect(owner.IsDisposed && child.IsDisposed &&
               !Application.OpenForms.Cast<Form>().Contains(owner) &&
               !Application.OpenForms.Cast<Form>().Contains(child),
            "smoke harness cleanup should close and dispose owned Copperfin forms");
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
