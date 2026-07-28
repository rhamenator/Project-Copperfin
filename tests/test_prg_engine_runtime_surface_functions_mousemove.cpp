#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_mouse_move_dispatches_with_button_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_mousemove_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "mousemove_dispatch.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nPlainHwnd, nMoves, nButton, nShift, nX, nY, cSequence\n"
            "nMoves = 0\n"
            "nButton = 0\n"
            "nShift = 0\n"
            "nX = 0\n"
            "nY = 0\n"
            "cSequence = ''\n"
            "oForm = CREATEOBJECT('MouseMoveForm')\n"
            "oPlain = CREATEOBJECT('Form')\n"
            "nFormHwnd = oForm.hWnd\n"
            "nPlainHwnd = oPlain.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS MouseMoveForm AS Form\n"
            "    FUNCTION MouseMove\n"
            "        LPARAMETERS tnButton, tnShiftAltCtrl, tnXCoord, tnYCoord\n"
            "        nMoves = nMoves + 1\n"
            "        nButton = tnButton\n"
            "        nShift = tnShiftAltCtrl\n"
            "        nX = tnXCoord\n"
            "        nY = tnYCoord\n"
            "        cSequence = cSequence + 'M'\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("MouseMove fixture should pause in READ EVENTS: ") + state.message);
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
               "MouseMove fixture should publish both form hWnd values");
        if (form_it == state.globals.end() || plain_it == state.globals.end())
        {
            return;
        }
        const std::intptr_t form_hwnd = value_to_intptr(form_it->second);
        const std::intptr_t plain_hwnd = value_to_intptr(plain_it->second);

        const auto missing_handler = session.dispatch_windows_message(
            999999,
            0x0200,
            0,
            0);
        expect(!missing_handler.has_value(),
               "an unknown target should leave MouseMove unhandled");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("unknown MouseMove should restore the event loop: ") + state.message);

        const auto missing_method = session.dispatch_windows_message(
            plain_hwnd,
            0x0200,
            0,
            0);
        expect(!missing_method.has_value(),
               "a native target without MouseMove should remain unhandled");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("missing MouseMove should restore the event loop: ") + state.message);

        const std::intptr_t first_lparam = (static_cast<std::intptr_t>(22) << 16) | 11;
        const auto first_move = session.dispatch_windows_message(
            form_hwnd,
            0x0200,
            0,
            first_lparam);
        expect(first_move.has_value() && *first_move == 0,
               "button-free MouseMove should return the modeled default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("first MouseMove should restore the event loop: ") + state.message);

        const std::uint32_t negative_coordinates =
            (static_cast<std::uint32_t>(static_cast<std::uint16_t>(-4)) << 16U) |
            static_cast<std::uint16_t>(-3);
        const auto second_move = session.dispatch_windows_message(
            form_hwnd,
            0x0200,
            0x001f,
            static_cast<std::intptr_t>(negative_coordinates));
        expect(second_move.has_value() && *second_move == 0,
               "buttoned MouseMove should return the modeled default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("second MouseMove should restore the event loop: ") + state.message);

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
        check("nmoves", "2");
        check("nbutton", "7");
        check("nshift", "3");
        check("nx", "-3");
        check("ny", "-4");
        check("csequence", "MM");

        const auto move_events = std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const copperfin::runtime::RuntimeEvent &event)
            {
                return event.category == "prg.event.mousemove" &&
                       event.detail.find("MouseMoveForm") != std::string::npos;
            });
        expect(move_events == 2,
               "handled MouseMove callbacks should emit one stable event each");
    }
}
