#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_control_interactivechange_dispatches_for_changed_input()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_interactivechange_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "interactivechange_dispatch.prg";
        write_text(
            main_path,
            "PUBLIC nListHwnd, nInteractiveCalls, nProgrammaticCalls\n"
            "oList = CREATEOBJECT('InteractiveList')\n"
            "oList.AddItem('Alpha')\n"
            "oList.AddItem('Beta')\n"
            "oList.ListIndex = 1\n"
            "nListHwnd = oList.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS InteractiveList AS ListBox\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 65\n"
            "            THIS.ListIndex = 2\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    PROCEDURE InteractiveChange\n"
            "        nInteractiveCalls = nInteractiveCalls + 1\n"
            "    ENDPROC\n"
            "    PROCEDURE ProgrammaticChange\n"
            "        nProgrammaticCalls = nProgrammaticCalls + 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const bool paused_in_event_loop =
            state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
            state.waiting_for_events;
        expect(paused_in_event_loop,
               std::string("InteractiveChange fixture should pause in READ EVENTS: ") + state.message);
        if (!paused_in_event_loop)
        {
            return;
        }

        const auto hwnd_it = state.globals.find("nlisthwnd");
        expect(hwnd_it != state.globals.end(),
               "InteractiveChange fixture should publish the list hWnd");
        if (hwnd_it == state.globals.end())
        {
            return;
        }

        std::intptr_t list_hwnd = 0;
        try
        {
            list_hwnd = static_cast<std::intptr_t>(
                std::stoll(copperfin::runtime::format_value(hwnd_it->second)));
        }
        catch (...)
        {
            expect(false, "InteractiveChange list hWnd should be numeric");
            return;
        }
        const auto unchanged_key = session.dispatch_windows_message(list_hwnd, 0x0100, 66, 0);
        expect(unchanged_key.has_value() && *unchanged_key == 0,
               "unchanged list input should preserve the KeyPress default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "unchanged list input should restore the event loop");

        const auto changed_key = session.dispatch_windows_message(list_hwnd, 0x0100, 65, 0);
        expect(changed_key.has_value() && *changed_key == 0,
               "changed list input should preserve the KeyPress default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "changed list input should restore the event loop");

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " should be present");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };
        check("ninteractivecalls", "1");
        check("nprogrammaticcalls", "2");
    }
}
