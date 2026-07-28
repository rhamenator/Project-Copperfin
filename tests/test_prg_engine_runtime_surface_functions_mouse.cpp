#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_mouse_down_up_dispatch_around_click()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_mouse_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "mouse_dispatch.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nMouseOnlyHwnd, nDown, nUp, nClicks, nButton, nShift, nX, nY, nOnlyDown, nOnlyUp, cSequence\n"
            "nDown = 0\n"
            "nUp = 0\n"
            "nClicks = 0\n"
            "nButton = 0\n"
            "nShift = 0\n"
            "nX = 0\n"
            "nY = 0\n"
            "nOnlyDown = 0\n"
            "nOnlyUp = 0\n"
            "cSequence = ''\n"
            "oForm = CREATEOBJECT('MouseForm')\n"
            "oMouseOnly = CREATEOBJECT('MouseOnlyForm')\n"
            "nFormHwnd = oForm.hWnd\n"
            "nMouseOnlyHwnd = oMouseOnly.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS MouseForm AS Form\n"
            "    FUNCTION MouseDown\n"
            "        LPARAMETERS tnButton, tnShiftAltCtrl, tnXCoord, tnYCoord\n"
            "        nDown = nDown + 1\n"
            "        nButton = tnButton\n"
            "        nShift = tnShiftAltCtrl\n"
            "        nX = tnXCoord\n"
            "        nY = tnYCoord\n"
            "        cSequence = cSequence + 'D'\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    FUNCTION MouseUp\n"
            "        LPARAMETERS tnButton, tnShiftAltCtrl, tnXCoord, tnYCoord\n"
            "        nUp = nUp + 1\n"
            "        nButton = tnButton\n"
            "        nShift = tnShiftAltCtrl\n"
            "        nX = tnXCoord\n"
            "        nY = tnYCoord\n"
            "        cSequence = cSequence + 'U'\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    FUNCTION Click\n"
            "        nClicks = nClicks + 1\n"
            "        cSequence = cSequence + 'C'\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MouseOnlyForm AS Form\n"
            "    FUNCTION MouseDown\n"
            "        LPARAMETERS tnButton, tnShiftAltCtrl, tnXCoord, tnYCoord\n"
            "        nOnlyDown = nOnlyDown + 1\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    FUNCTION MouseUp\n"
            "        LPARAMETERS tnButton, tnShiftAltCtrl, tnXCoord, tnYCoord\n"
            "        nOnlyUp = nOnlyUp + 1\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("Mouse fixture should pause in READ EVENTS: ") + state.message);
        if (state.reason != copperfin::runtime::DebugPauseReason::event_loop ||
            !state.waiting_for_events)
        {
            return;
        }

        const auto value_to_intptr = [](const copperfin::runtime::PrgValue &value)
        {
            try
            {
                return static_cast<std::intptr_t>(
                    std::stoll(copperfin::runtime::format_value(value)));
            }
            catch (...)
            {
                return static_cast<std::intptr_t>(0);
            }
        };
        const auto form_it = state.globals.find("nformhwnd");
        const auto mouse_only_it = state.globals.find("nmouseonlyhwnd");
        expect(form_it != state.globals.end() && mouse_only_it != state.globals.end(),
               "Mouse fixture should publish both form hWnd values");
        if (form_it == state.globals.end() || mouse_only_it == state.globals.end())
        {
            return;
        }
        const std::intptr_t form_hwnd = value_to_intptr(form_it->second);
        const std::intptr_t mouse_only_hwnd = value_to_intptr(mouse_only_it->second);
        const std::intptr_t lparam = (static_cast<std::intptr_t>(22) << 16) | 11;

        const auto only_down = session.dispatch_windows_message(
            mouse_only_hwnd,
            0x0201,
            0x000c,
            lparam);
        expect(only_down.has_value() && *only_down == 0,
               "MouseDown-only targets should handle left-button down input");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("MouseDown-only dispatch should restore the event loop: ") + state.message);

        const auto only_up = session.dispatch_windows_message(
            mouse_only_hwnd,
            0x0202,
            0x000c,
            lparam);
        expect(only_up.has_value() && *only_up == 0,
               "MouseUp-only targets should handle left-button release input");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("MouseUp-only dispatch should restore the event loop: ") + state.message);

        const auto down = session.dispatch_windows_message(form_hwnd, 0x0201, 0x000c, lparam);
        expect(down.has_value() && *down == 0,
               "MouseDown should return the modeled default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("MouseDown dispatch should restore the event loop: ") + state.message);

        const auto up = session.dispatch_windows_message(form_hwnd, 0x0202, 0x000c, lparam);
        expect(up.has_value() && *up == 0,
               "MouseUp and Click should return the modeled default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("MouseUp/Click dispatch should restore the event loop: ") + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
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
        check("nonlydown", "1");
        check("nonlyup", "1");
        check("ndown", "1");
        check("nup", "1");
        check("nclicks", "1");
        check("nbutton", "1");
        check("nshift", "3");
        check("nx", "11");
        check("ny", "22");
        check("csequence", "DUC");

        const auto event_count = [&](const std::string &category)
        {
            return std::count_if(
                state.events.begin(),
                state.events.end(),
                [&](const copperfin::runtime::RuntimeEvent &event)
                {
                    return event.category == category &&
                           event.detail.find("MouseForm") != std::string::npos;
                });
        };
        expect(event_count("prg.event.mousedown") == 1,
               "MouseDown should emit one stable event");
        expect(event_count("prg.event.mouseup") == 1,
               "MouseUp should emit one stable event");
        expect(event_count("prg.event.click") == 1,
               "Click should remain after MouseUp");
    }
}
