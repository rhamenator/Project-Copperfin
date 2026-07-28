#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_click_dispatches_on_left_button_release()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_click_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "click_dispatch.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nClicks, cSequence\n"
            "nClicks = 0\n"
            "cSequence = ''\n"
            "oForm = CREATEOBJECT('ClickForm')\n"
            "nFormHwnd = oForm.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS ClickForm AS Form\n"
            "    FUNCTION Click\n"
            "        nClicks = nClicks + 1\n"
            "        cSequence = cSequence + 'F'\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("Click fixture should pause in READ EVENTS: ") + state.message);
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
        expect(form_it != state.globals.end(),
               "Click fixture should publish the form hWnd value");
        if (form_it == state.globals.end())
        {
            return;
        }
        const std::intptr_t form_hwnd = value_to_intptr(form_it->second);

        const auto missing_handler = session.dispatch_windows_message(
            999999,
            0x0202,
            1,
            0);
        expect(!missing_handler.has_value(),
               "a target without Click should leave the modeled message unhandled");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("missing Click should restore the event loop: ") + state.message);

        const auto first_click = session.dispatch_windows_message(
            form_hwnd,
            0x0202,
            1,
            0);
        expect(first_click.has_value() && *first_click == 0,
               "a Form Click should return the modeled default result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("Click dispatch should restore the event loop: ") + state.message);

        const auto second_click = session.dispatch_windows_message(
            form_hwnd,
            0x0202,
            1,
            0);
        expect(second_click.has_value() && *second_click == 0,
               "repeated Form Click should remain handled with the same result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("repeated Click dispatch should restore the event loop: ") + state.message);

        const auto count = state.globals.find("nclicks");
        const auto sequence = state.globals.find("csequence");
        expect(count != state.globals.end() && sequence != state.globals.end(),
               "Click counters should remain visible after dispatch");
        if (count != state.globals.end())
        {
            expect(copperfin::runtime::format_value(count->second) == "2",
                   "Form Click should run once per handled left-button release");
        }
        if (sequence != state.globals.end())
        {
            expect(copperfin::runtime::format_value(sequence->second) == "FF",
                   "Form Click handlers should preserve dispatch order");
        }

        const auto click_events = std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const copperfin::runtime::RuntimeEvent &event)
            {
                return event.category == "prg.event.click" &&
                       event.detail.find("ClickForm") != std::string::npos;
            });
        expect(click_events == 2,
               "Click should emit one stable event for each handled release");
    }
}
