// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
    private static void SmokeDebuggerDetailTablesMirrorPauseState()
    {
        var parsedWindowsBreakpoint = CopperfinRuntimeDebugClient.ParsePauseState(
            "debug.breakpoint.count: 1\n" +
            "debug.breakpoint[0]: C:\\Samples\\invoice.prg:27\n");
        Expect(parsedWindowsBreakpoint.Breakpoints.Count == 1 &&
               parsedWindowsBreakpoint.Breakpoints[0].FilePath == "C:\\Samples\\invoice.prg" &&
               parsedWindowsBreakpoint.Breakpoints[0].Line == 27,
            "breakpoint parsing should preserve Windows drive-letter paths");

        using var control = new CopperfinAssetEditorControl(new CopperfinLocalization("qps-ploc"));
        control.SuppressDebuggerDialogs = true;
        var session = new CopperfinRuntimeDebugSession
        {
            Success = true,
            State = new CopperfinRuntimePauseState
            {
                Reason = "breakpoint",
                Location = "sample.prg:12",
                Statement = "RETURN nValue",
                Message = "paused",
                Frames =
                {
                    new CopperfinRuntimeStackFrame
                    {
                        RoutineName = "Main",
                        Location = "sample.prg:12",
                        Locals =
                        {
                            new CopperfinRuntimeNamedValue { Name = "nValue", Value = "42" },
                            new CopperfinRuntimeNamedValue { Name = "cName", Value = "Copperfin" }
                        }
                    }
                },
                Globals =
                {
                    new CopperfinRuntimeNamedValue { Name = "_SCREEN", Value = "object" }
                },
                Events =
                {
                    new CopperfinRuntimeEvent
                    {
                        Category = "runtime",
                        Detail = "prg.pause",
                        Location = "sample.prg:12"
                    }
                },
                Watches =
                {
                    new CopperfinRuntimeWatch
                    {
                        Expression = "nValue + 1",
                        Success = true,
                        Value = "43"
                    }
                },
                Breakpoints =
                {
                    new CopperfinRuntimeBreakpoint
                    {
                        FilePath = "sample.prg",
                        Line = 12,
                        ActionId = "",
                        ActionTitle = ""
                    }
                }
            }
        };

        InvokeAssetEditorVoid(control, "ApplyDebugSession", session);
        var callStack = GetPrivateListView(control, "debuggerCallStackView");
        var locals = GetPrivateListView(control, "debuggerLocalsView");
        var globals = GetPrivateListView(control, "debuggerGlobalsView");
        var events = GetPrivateListView(control, "debuggerEventsView");
        var watches = GetPrivateListView(control, "debuggerWatchesView");
        var breakpoints = GetPrivateListView(control, "debuggerBreakpointsView");
        var tabs = GetPrivateField<TabControl>(control, "debuggerDetailTabs");
        Expect(tabs is not null &&
               tabs.TabPages.Count == 6 &&
               callStack.Items.Count == 1 &&
               locals.Items.Count == 2 &&
               globals.Items.Count == 1 &&
               events.Items.Count == 1 &&
               watches.Items.Count == 1 &&
               breakpoints.Items.Count == 1 &&
               breakpoints.Items[0].SubItems[0].Text == "sample.prg" &&
               breakpoints.Items[0].SubItems[1].Text == "12" &&
               watches.Items[0].SubItems[0].Text == "nValue + 1" &&
               watches.Items[0].SubItems[1].Text == "43" &&
               locals.Items[0].SubItems[1].Text == "nValue" &&
               locals.Items[0].SubItems[2].Text == "42",
            "debugger detail tables should mirror the shared pause-state model");

        InvokeAssetEditorVoid(
            control,
            "ApplyDebugSession",
            new CopperfinRuntimeDebugSession
            {
                Success = false,
                Error = "synthetic failure"
            });
        Expect(callStack.Items.Count == 0 &&
               locals.Items.Count == 0 &&
               globals.Items.Count == 0 &&
               events.Items.Count == 0 &&
               watches.Items.Count == 0 &&
               breakpoints.Items.Count == 0,
            "failed debugger sessions should clear stale detail and breakpoint rows");

        using var hostForm = new Form
        {
            Width = 900,
            Height = 700,
            ShowInTaskbar = false,
            StartPosition = FormStartPosition.Manual,
            Location = new System.Drawing.Point(-32000, -32000)
        };
        hostForm.Controls.Add(control);
        hostForm.Show();
        Application.DoEvents();
        var threadedApply = Task.Run(() => InvokeAssetEditorVoid(control, "ApplyDebugSession", session));
        var expectedCallStackHeader = new CopperfinLocalization("qps-ploc")
            .Text("AssetEditor.Summary.CallStack");
        var debuggerSummaryBox = GetPrivateField<RichTextBox>(control, "debuggerSummaryBox")
            ?? throw new InvalidOperationException("Could not read private debugger summary box.");
        var threadedSummaryUpdated = WaitUntil(
            TimeSpan.FromSeconds(3),
            () => debuggerSummaryBox.Text.Contains(
                expectedCallStackHeader,
                StringComparison.Ordinal));
#pragma warning disable VSTHRD002 // This smoke test intentionally joins after pumping the UI loop.
        threadedApply.GetAwaiter().GetResult();
#pragma warning restore VSTHRD002
        Expect(threadedSummaryUpdated,
            "debugger session application should marshal the summary update to the WinForms control thread");
    }
}
