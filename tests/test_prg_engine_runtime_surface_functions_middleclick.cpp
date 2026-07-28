#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_middle_click_dispatches_without_mouse_arguments()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_middleclick_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "middleclick_dispatch.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nPlainHwnd, nCalls, cSequence\n"
            "nCalls = 0\n"
            "cSequence = ''\n"
            "oForm = CREATEOBJECT('MiddleClickForm')\n"
            "oPlain = CREATEOBJECT('Form')\n"
            "nFormHwnd = oForm.hWnd\n"
            "nPlainHwnd = oPlain.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS MiddleClickForm AS Form\n"
            "    FUNCTION MiddleClick\n"
            "        nCalls = nCalls + 1\n"
            "        cSequence = cSequence + 'M'\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("MiddleClick fixture should pause in READ EVENTS: ") + state.message);
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
               "MiddleClick fixture should publish both native hWnd values");
        if (form_it == state.globals.end() || plain_it == state.globals.end())
        {
            return;
        }

        const std::intptr_t form_hwnd = value_to_intptr(form_it->second);
        const std::intptr_t plain_hwnd = value_to_intptr(plain_it->second);

        const auto unknown = session.dispatch_windows_message(
            static_cast<std::intptr_t>(987656),
            0x0208,
            0x0000,
            0);
        expect(!unknown.has_value(), "Unknown MiddleClick targets should remain unhandled");

        const auto plain = session.dispatch_windows_message(
            plain_hwnd,
            0x0208,
            0x0000,
            0);
        expect(!plain.has_value(), "Objects without MiddleClick should remain unhandled");

        const auto dispatched = session.dispatch_windows_message(
            form_hwnd,
            0x0208,
            0x0000,
            0);
        expect(dispatched.has_value() && *dispatched == 0,
               "MiddleClick should return the modeled default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("MiddleClick dispatch should restore the event loop: ") + state.message);

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
        check("ncalls", "1");
        check("csequence", "M");

        const auto event_count = std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const copperfin::runtime::RuntimeEvent &event)
            {
                return event.category == "prg.event.middleclick" &&
                       event.detail.find("MiddleClickForm") != std::string::npos;
            });
        expect(event_count == 1, "MiddleClick should emit one stable event");
    }
}
