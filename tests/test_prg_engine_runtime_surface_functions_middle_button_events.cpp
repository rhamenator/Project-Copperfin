#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_middle_button_events_dispatch_around_middle_click()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_middle_button_events";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "middle_button_events.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nPlainHwnd, nDown, nUp, nClicks, nButton, nShift, nX, nY, cSequence\n"
            "nDown = 0\n"
            "nUp = 0\n"
            "nClicks = 0\n"
            "nButton = 0\n"
            "nShift = 0\n"
            "nX = 0\n"
            "nY = 0\n"
            "cSequence = ''\n"
            "oForm = CREATEOBJECT('MiddleEventsForm')\n"
            "oPlain = CREATEOBJECT('Form')\n"
            "nFormHwnd = oForm.hWnd\n"
            "nPlainHwnd = oPlain.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS MiddleEventsForm AS Form\n"
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
            "    FUNCTION MiddleClick\n"
            "        nClicks = nClicks + 1\n"
            "        cSequence = cSequence + 'M'\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("Middle-button fixture should pause in READ EVENTS: ") + state.message);
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
        const auto plain_it = state.globals.find("nplainhwnd");
        expect(form_it != state.globals.end() && plain_it != state.globals.end(),
               "Middle-button fixture should publish both native hWnd values");
        if (form_it == state.globals.end() || plain_it == state.globals.end())
        {
            return;
        }

        const std::intptr_t form_hwnd = value_to_intptr(form_it->second);
        const std::intptr_t plain_hwnd = value_to_intptr(plain_it->second);
        const std::intptr_t lparam =
            (static_cast<std::intptr_t>(static_cast<std::uint16_t>(-4)) << 16) |
            static_cast<std::uint16_t>(-3);

        const auto unknown = session.dispatch_windows_message(
            static_cast<std::intptr_t>(987657),
            0x0207,
            0x000c,
            lparam);
        expect(!unknown.has_value(), "Unknown middle-button targets should remain unhandled");

        const auto plain = session.dispatch_windows_message(
            plain_hwnd,
            0x0208,
            0x000c,
            lparam);
        expect(!plain.has_value(), "Objects without middle-button handlers should remain unhandled");

        const auto down = session.dispatch_windows_message(
            form_hwnd,
            0x0207,
            0x000c,
            lparam);
        expect(down.has_value() && *down == 0,
               "Middle-button MouseDown should return the modeled default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("Middle-button MouseDown should restore the event loop: ") + state.message);

        const auto up = session.dispatch_windows_message(
            form_hwnd,
            0x0208,
            0x000c,
            lparam);
        expect(up.has_value() && *up == 0,
               "Middle-button MouseUp and MiddleClick should return the modeled default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("Middle-button MouseUp dispatch should restore the event loop: ") + state.message);

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
        check("ndown", "1");
        check("nup", "1");
        check("nclicks", "1");
        check("nbutton", "4");
        check("nshift", "3");
        check("nx", "-3");
        check("ny", "-4");
        check("csequence", "DUM");

        const auto event_count = [&](const std::string &category)
        {
            return std::count_if(
                state.events.begin(),
                state.events.end(),
                [&](const copperfin::runtime::RuntimeEvent &event)
                {
                    return event.category == category &&
                           event.detail.find("MiddleEventsForm") != std::string::npos;
                });
        };
        expect(event_count("prg.event.mousedown") == 1,
               "Middle-button MouseDown should emit one stable event");
        expect(event_count("prg.event.mouseup") == 1,
               "Middle-button MouseUp should emit one stable event");
        expect(event_count("prg.event.middleclick") == 1,
               "MiddleClick should emit one stable event");
    }
}
