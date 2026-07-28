#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_keypreview_dispatches_before_child_keypress()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_keypreview_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "keypreview_dispatch.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nChildHwnd, nFormCalls, nChildCalls, cSequence\n"
            "nFormCalls = 0\n"
            "nChildCalls = 0\n"
            "cSequence = ''\n"
            "oForm = CREATEOBJECT('PreviewForm')\n"
            "oForm.KeyPreview = .T.\n"
            "oForm.lst.SetFocus()\n"
            "nFormHwnd = oForm.hWnd\n"
            "nChildHwnd = oForm.lst.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS PreviewForm AS Form\n"
            "    ADD OBJECT lst AS PreviewList\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        nFormCalls = nFormCalls + 1\n"
            "        cSequence = cSequence + 'F'\n"
            "        IF tnKeyCode = 66\n"
            "            NODEFAULT\n"
            "        ENDIF\n"
            "        IF tnKeyCode = 68\n"
            "            THIS.KeyPreview = .F.\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PreviewList AS ListBox\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        nChildCalls = nChildCalls + 1\n"
            "        cSequence = cSequence + 'C'\n"
            "        IF tnKeyCode = 67\n"
            "            NODEFAULT\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               std::string("KeyPreview fixture should pause in READ EVENTS: ") + state.message);
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
        const auto child_it = state.globals.find("nchildhwnd");
        expect(form_it != state.globals.end() && child_it != state.globals.end(),
               "KeyPreview fixture should publish form and child hWnd values");
        if (form_it == state.globals.end() || child_it == state.globals.end())
        {
            return;
        }
        const std::intptr_t form_hwnd = value_to_intptr(form_it->second);
        const std::intptr_t child_hwnd = value_to_intptr(child_it->second);

        const auto resume_event_loop = [&]()
        {
            state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                       state.waiting_for_events,
                   std::string("KeyPreview dispatch should restore the event loop: ") + state.message);
        };

        const auto ordinary = session.dispatch_windows_message(child_hwnd, 0x0100, 65, 0);
        expect(ordinary.has_value() && *ordinary == 0,
               "KeyPreview without NODEFAULT should preserve the modeled default result");
        resume_event_loop();

        const auto preview_nodefault = session.dispatch_windows_message(child_hwnd, 0x0100, 66, 0);
        expect(preview_nodefault.has_value() && *preview_nodefault == 1,
               "Form NODEFAULT should suppress child KeyPress and return handled result");
        resume_event_loop();

        const auto child_nodefault = session.dispatch_windows_message(child_hwnd, 0x0100, 67, 0);
        expect(child_nodefault.has_value() && *child_nodefault == 1,
               "child NODEFAULT should remain effective after Form KeyPreview");
        resume_event_loop();

        const auto disable_preview = session.dispatch_windows_message(form_hwnd, 0x0100, 68, 0);
        expect(disable_preview.has_value() && *disable_preview == 0,
               "Form-targeted input should preserve ordinary KeyPress behavior");
        resume_event_loop();

        const auto child_without_preview = session.dispatch_windows_message(child_hwnd, 0x0100, 69, 0);
        expect(child_without_preview.has_value() && *child_without_preview == 0,
               "child input should remain direct after KeyPreview is disabled");
        resume_event_loop();

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
        check("nformcalls", "4");
        check("nchildcalls", "3");
        check("csequence", "FCFFCFC");

        const auto form_events = std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const copperfin::runtime::RuntimeEvent &event)
            {
                return event.category == "prg.event.keypress" && event.detail.find("PreviewForm") != std::string::npos;
            });
        expect(form_events == 4,
               "KeyPreview should emit one stable keypress event for each form dispatch");
    }
}
