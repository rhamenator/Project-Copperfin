// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace
{

using namespace copperfin::test_support;

void test_native_show_hide_refresh_events()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_native_lifecycle_events";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_lifecycle_events.prg";
    write_text(
        main_path,
        "oForm = CREATEOBJECT('LifecycleForm')\n"
        "oForm.Show()\n"
        "oForm.Hide()\n"
        "oForm.Refresh()\n"
        "cLifecycleEvents = oForm.cEvents\n"
        "lLifecycleVisible = oForm.Visible\n"
        "oOverride = CREATEOBJECT('OverrideForm')\n"
        "oOverride.Show()\n"
        "oOverride.Hide()\n"
        "oOverride.Refresh()\n"
        "cOverrideEvents = oOverride.cEvents\n"
        "lOverrideVisible = oOverride.Visible\n"
        "oNodefault = CREATEOBJECT('NodefaultRefreshForm')\n"
        "oNodefault.Refresh()\n"
        "nNodefaultPaints = oNodefault.nPaint\n"
        "cNodefaultEvents = oNodefault.cEvents\n"
        "RETURN\n"
        "DEFINE CLASS LifecycleForm AS Form\n"
        "    cEvents = ''\n"
        "    PROCEDURE Activate\n"
        "        THIS.cEvents = THIS.cEvents + 'activate;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Deactivate\n"
        "        THIS.cEvents = THIS.cEvents + 'deactivate;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Paint\n"
        "        THIS.cEvents = THIS.cEvents + 'paint;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS NodefaultRefreshForm AS Form\n"
        "    cEvents = ''\n"
        "    nPaint = 0\n"
        "    PROCEDURE Paint\n"
        "        THIS.nPaint = THIS.nPaint + 1\n"
        "        THIS.cEvents = THIS.cEvents + 'paint;'\n"
        "        IF THIS.nPaint = 1\n"
        "            THIS.Refresh()\n"
        "        ENDIF\n"
        "        NODEFAULT\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS OverrideForm AS Form\n"
        "    cEvents = ''\n"
        "    PROCEDURE Show\n"
        "        THIS.cEvents = THIS.cEvents + 'show;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Hide\n"
        "        THIS.cEvents = THIS.cEvents + 'hide;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Refresh\n"
        "        THIS.cEvents = THIS.cEvents + 'refresh;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "native lifecycle event script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present");
        if (it != state.globals.end())
        {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("clifecycleevents", "activate;deactivate;paint;");
    check("llifecyclevisible", "false");
    check("coverrideevents", "show;hide;refresh;");
    check("loverridevisible", "true");
    check("nnodefaultpaints", "2");
    check("cnodefaultevents", "paint;paint;");
    expect(has_runtime_event(state.events, "prg.object.invoke", "LifecycleForm.Activate"),
           "Show should invoke Activate through the native method path");
    expect(has_runtime_event(state.events, "prg.object.invoke", "LifecycleForm.Deactivate"),
           "Hide should invoke Deactivate through the native method path");
    expect(has_runtime_event(state.events, "prg.object.invoke", "LifecycleForm.Paint"),
           "Refresh should invoke Paint through the native method path");
    expect(!has_runtime_event(state.events, "prg.object.invoke", "OverrideForm.Activate"),
           "Show override should prevent builtin Activate dispatch");
    expect(!has_runtime_event(state.events, "prg.object.invoke", "OverrideForm.Deactivate"),
           "Hide override should prevent builtin Deactivate dispatch");
    expect(!has_runtime_event(state.events, "prg.object.invoke", "OverrideForm.Paint"),
           "Refresh override should prevent builtin Paint dispatch");

    fs::remove_all(temp_root, ignored);
}

void test_native_query_unload_runs_for_quit_but_not_direct_release()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_native_query_unload";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_query_unload.prg";
    write_text(
        main_path,
        "PUBLIC cEvents\n"
        "cEvents = ''\n"
        "oDirect = CREATEOBJECT('DirectForm')\n"
        "oDirect.Release()\n"
        "cDirectEvents = cEvents\n"
        "oSet = CREATEOBJECT('QueryFormSet')\n"
        "QUIT\n"
        "RETURN\n"
        "DEFINE CLASS DirectForm AS Form\n"
        "    PROCEDURE QueryUnload\n"
        "        cEvents = cEvents + 'direct-query;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Destroy\n"
        "        cEvents = cEvents + 'direct-destroy;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Unload\n"
        "        cEvents = cEvents + 'direct-unload;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS QueryChild AS Form\n"
        "    PROCEDURE QueryUnload\n"
        "        cEvents = cEvents + 'child-query;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Destroy\n"
        "        cEvents = cEvents + 'child-destroy;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Unload\n"
        "        cEvents = cEvents + 'child-unload;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS QueryFormSet AS FormSet\n"
        "    ADD OBJECT frmChild AS QueryChild\n"
        "    PROCEDURE QueryUnload\n"
        "        cEvents = cEvents + 'set-query;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Destroy\n"
        "        cEvents = cEvents + 'set-destroy;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Unload\n"
        "        cEvents = cEvents + 'set-unload;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "native QueryUnload QUIT script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present");
        if (it != state.globals.end())
        {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cdirectevents", "direct-destroy;direct-unload;");
    check("cevents", "direct-destroy;direct-unload;child-query;set-query;child-destroy;child-unload;set-destroy;set-unload;");
    expect(has_runtime_event(state.events, "prg.object.queryunload", "QueryChild.QueryUnload"),
           "QUIT should dispatch QueryUnload on the child form before release");
    expect(has_runtime_event(state.events, "prg.object.queryunload", "QueryFormSet.QueryUnload"),
           "QUIT should dispatch QueryUnload on the formset");
    expect(!has_runtime_event(state.events, "prg.object.queryunload", "DirectForm.QueryUnload"),
           "direct Release should not dispatch QueryUnload");
    expect(has_runtime_event(state.events, "prg.object.destroy", "QueryChild.Destroy"),
           "accepted QUIT should continue to child Destroy");
    expect(has_runtime_event(state.events, "prg.object.unload", "QueryFormSet.Unload"),
           "accepted QUIT should continue to formset Unload");

    fs::remove_all(temp_root, ignored);
}

void test_native_query_unload_nodefault_vetoes_quit()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_native_query_unload_veto";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_query_unload_veto.prg";
    write_text(
        main_path,
        "PUBLIC cEvents\n"
        "cEvents = ''\n"
        "oFalse = CREATEOBJECT('FalseForm')\n"
        "QUIT\n"
        "lFalseStillAlive = PEMSTATUS(oFalse, 'cEvents', 1)\n"
        "cAfterFalse = cEvents\n"
        "oFalse.Release()\n"
        "oVeto = CREATEOBJECT('VetoForm')\n"
        "QUIT\n"
        "lStillAlive = PEMSTATUS(oVeto, 'cEvents', 1)\n"
        "cAfterQuit = cEvents\n"
        "oVeto.Release()\n"
        "RETURN\n"
        "DEFINE CLASS FalseForm AS Form\n"
        "    PROCEDURE QueryUnload\n"
        "        cEvents = cEvents + 'false-query;'\n"
        "        RETURN .F.\n"
        "    ENDPROC\n"
        "    cEvents = ''\n"
        "    PROCEDURE Destroy\n"
        "        cEvents = cEvents + 'false-destroy;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS VetoForm AS Form\n"
        "    cEvents = ''\n"
        "    PROCEDURE QueryUnload\n"
        "        cEvents = cEvents + 'query;'\n"
        "        NODEFAULT\n"
        "    ENDPROC\n"
        "    PROCEDURE Destroy\n"
        "        cEvents = cEvents + 'destroy;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "native QueryUnload veto script should continue: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present");
        if (it != state.globals.end())
        {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("lfalsestillalive", "true");
    check("cafterfalse", "false-query;");
    check("lstillalive", "true");
    check("cafterquit", "false-query;false-destroy;query;");
    expect(has_runtime_event(state.events, "prg.object.queryunload_veto", "FalseForm"),
           "false QueryUnload result should veto QUIT");
    expect(has_runtime_event(state.events, "prg.object.queryunload_veto", "VetoForm"),
           "NODEFAULT from QueryUnload should veto QUIT");
    expect(has_runtime_event(state.events, "prg.object.destroy", "VetoForm.Destroy"),
           "direct Release after a veto should still destroy the form");
    const auto query_unload_count = static_cast<std::size_t>(std::count_if(
        state.events.begin(),
        state.events.end(),
        [](const auto& event)
        {
            return event.category == "prg.object.queryunload" &&
                   event.detail == "VetoForm.QueryUnload";
        }));
    expect(query_unload_count == 1U,
           "direct Release after a veto should not dispatch QueryUnload again");

    fs::remove_all(temp_root, ignored);
}

void test_native_window_close_dispatches_query_unload_before_destroy()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_native_window_close";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_window_close.prg";
    write_text(
        main_path,
        "PUBLIC cEvents\n"
        "cEvents = ''\n"
        "oNoHandler = CREATEOBJECT('NoHandlerForm')\n"
        "nNoHandlerHwnd = oNoHandler.hWnd\n"
        "READ EVENTS\n"
        "lNoHandlerGone = !PEMSTATUS(oNoHandler, 'cMarker', 1)\n"
        "oSet = CREATEOBJECT('CloseFormSet')\n"
        "nSetHwnd = oSet.hWnd\n"
        "READ EVENTS\n"
        "RETURN\n"
        "PROCEDURE EndEvents\n"
        "    CLEAR EVENTS\n"
        "ENDPROC\n"
        "DEFINE CLASS NoHandlerForm AS Form\n"
        "    cMarker = 'no-handler'\n"
        "ENDDEFINE\n"
        "DEFINE CLASS CloseChild AS Form\n"
        "    PROCEDURE QueryUnload\n"
        "        cEvents = cEvents + 'child-query;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Destroy\n"
        "        cEvents = cEvents + 'child-destroy;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Unload\n"
        "        cEvents = cEvents + 'child-unload;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS CloseFormSet AS FormSet\n"
        "    ADD OBJECT frmChild AS CloseChild\n"
        "    PROCEDURE QueryUnload\n"
        "        cEvents = cEvents + 'set-query;'\n"
        "        CLEAR EVENTS\n"
        "    ENDPROC\n"
        "    PROCEDURE Destroy\n"
        "        cEvents = cEvents + 'set-destroy;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Unload\n"
        "        cEvents = cEvents + 'set-unload;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "native WM_CLOSE lifecycle script should enter its first event loop");

    const auto no_handler_hwnd = state.globals.find("nnohandlerhwnd");
    expect(no_handler_hwnd != state.globals.end(),
           "native WM_CLOSE lifecycle script should export the no-handler hWnd");
    if (no_handler_hwnd == state.globals.end())
    {
        fs::remove_all(temp_root, ignored);
        return;
    }
    const auto no_handler_close = session.dispatch_windows_message(
        static_cast<std::intptr_t>(std::stoll(format_value(no_handler_hwnd->second))),
        0x0010U);
    expect(no_handler_close.has_value(),
           "WM_CLOSE should be handled for a Form without QueryUnload");

    expect(session.dispatch_event_handler("EndEvents"),
           "native WM_CLOSE lifecycle script should expose an event-loop exit helper");
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "native WM_CLOSE no-handler close should continue to the FormSet event loop");
    const auto no_handler_gone = state.globals.find("lnohandlergone");
    expect(no_handler_gone != state.globals.end() && format_value(no_handler_gone->second) == "true",
           "WM_CLOSE without QueryUnload should release the Form before continuing");

    const auto set_hwnd = state.globals.find("nsethwnd");
    expect(set_hwnd != state.globals.end(),
           "native WM_CLOSE lifecycle script should export the FormSet hWnd");
    if (set_hwnd != state.globals.end())
    {
        const auto set_close = session.dispatch_windows_message(
            static_cast<std::intptr_t>(std::stoll(format_value(set_hwnd->second))),
            0x0010U);
        expect(set_close.has_value(), "WM_CLOSE should be handled for a FormSet");
    }
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "native WM_CLOSE lifecycle script should complete: " + state.message);

    const auto sequence = state.globals.find("cevents");
    expect(sequence != state.globals.end() &&
               format_value(sequence->second) ==
                   "child-query;set-query;child-destroy;child-unload;set-destroy;set-unload;",
           "WM_CLOSE should dispatch QueryUnload child-first before Destroy and Unload");
    expect(has_runtime_event(state.events, "prg.object.queryunload", "CloseChild.QueryUnload"),
           "WM_CLOSE should emit the child QueryUnload event");
    expect(has_runtime_event(state.events, "prg.object.queryunload", "CloseFormSet.QueryUnload"),
           "WM_CLOSE should emit the FormSet QueryUnload event");
    expect(has_runtime_event(state.events, "prg.object.window_close", "CloseFormSet"),
           "WM_CLOSE should emit the accepted window-close event");

    fs::remove_all(temp_root, ignored);
}

void test_native_window_close_query_unload_nodefault_vetoes_release()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_native_window_close_veto";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_window_close_veto.prg";
    write_text(
        main_path,
        "PUBLIC cEvents\n"
        "cEvents = ''\n"
        "oForm = CREATEOBJECT('VetoForm')\n"
        "nHwnd = oForm.hWnd\n"
        "READ EVENTS\n"
        "lStillAlive = PEMSTATUS(oForm, 'cMarker', 1)\n"
        "RETURN\n"
        "PROCEDURE EndEvents\n"
        "    CLEAR EVENTS\n"
        "ENDPROC\n"
        "DEFINE CLASS VetoForm AS Form\n"
        "    cMarker = 'veto'\n"
        "    PROCEDURE QueryUnload\n"
        "        cEvents = cEvents + 'query;'\n"
        "        NODEFAULT\n"
        "    ENDPROC\n"
        "    PROCEDURE Destroy\n"
        "        cEvents = cEvents + 'destroy;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto hwnd = state.globals.find("nhwnd");
    expect(hwnd != state.globals.end(), "WM_CLOSE veto script should export the Form hWnd");
    if (hwnd != state.globals.end())
    {
        const auto close_result = session.dispatch_windows_message(
            static_cast<std::intptr_t>(std::stoll(format_value(hwnd->second))),
            0x0010U);
        expect(close_result.has_value(), "WM_CLOSE veto should still be handled by the runtime");
    }
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "WM_CLOSE veto should leave the runtime in its event loop");
    expect(session.dispatch_event_handler("EndEvents"),
           "WM_CLOSE veto script should expose an event-loop exit helper");
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "WM_CLOSE veto script should complete after the veto check");

    const auto sequence = state.globals.find("cevents");
    expect(sequence != state.globals.end() && format_value(sequence->second) == "query;",
           "QueryUnload NODEFAULT should veto WM_CLOSE before Destroy");
    const auto still_alive = state.globals.find("lstillalive");
    expect(still_alive != state.globals.end() && format_value(still_alive->second) == "true",
           "QueryUnload NODEFAULT should keep the Form alive");
    expect(has_runtime_event(state.events, "prg.object.queryunload_veto", "VetoForm"),
           "WM_CLOSE veto should emit the QueryUnload veto event");
    expect(has_runtime_event(state.events, "prg.object.window_close_veto", "VetoForm"),
           "WM_CLOSE veto should emit the window-close veto event");
    expect(!has_runtime_event(state.events, "prg.object.destroy", "VetoForm.Destroy"),
           "WM_CLOSE veto should not dispatch Destroy");

    fs::remove_all(temp_root, ignored);
}

} // namespace

int main()
{
    test_native_show_hide_refresh_events();
    test_native_query_unload_runs_for_quit_but_not_direct_release();
    test_native_query_unload_nodefault_vetoes_quit();
    test_native_window_close_dispatches_query_unload_before_destroy();
    test_native_window_close_query_unload_nodefault_vetoes_release();
    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
