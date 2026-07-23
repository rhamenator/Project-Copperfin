// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
    private static void SmokeDebuggerDetailTablesMirrorPauseState()
    {
        using var control = new CopperfinAssetEditorControl(new CopperfinLocalization("qps-ploc"));
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
                }
            }
        };

        InvokeAssetEditorVoid(control, "ApplyDebugSession", session);
        var callStack = GetPrivateListView(control, "debuggerCallStackView");
        var locals = GetPrivateListView(control, "debuggerLocalsView");
        var globals = GetPrivateListView(control, "debuggerGlobalsView");
        var events = GetPrivateListView(control, "debuggerEventsView");
        var watches = GetPrivateListView(control, "debuggerWatchesView");
        var tabs = GetPrivateField<TabControl>(control, "debuggerDetailTabs");
        Expect(tabs is not null &&
               tabs.TabPages.Count == 5 &&
               callStack.Items.Count == 1 &&
               locals.Items.Count == 2 &&
               globals.Items.Count == 1 &&
               events.Items.Count == 1 &&
               watches.Items.Count == 1 &&
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
               watches.Items.Count == 0,
            "failed debugger sessions should clear stale detail rows");
    }
}
