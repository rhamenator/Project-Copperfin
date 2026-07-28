#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_keypress_dispatch_and_nodefault_preserve_event_loop_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_keypress_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "keypress_dispatch.prg";
        write_text(
            main_path,
            "PUBLIC nKeyFormHwnd, nKeyPressCalls, nLastKeyCode, nLastShift, nAltShift\n"
            "oForm = CREATEOBJECT('KeyForm')\n"
            "nKeyFormHwnd = oForm.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS KeyForm AS Form\n"
            "    nCalls = 0\n"
            "    nKeyCode = 0\n"
            "    nShiftAltCtrl = 0\n"
            "    cSequence = ''\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        nKeyPressCalls = nKeyPressCalls + 1\n"
            "        nLastKeyCode = tnKeyCode\n"
            "        nLastShift = tnShiftAltCtrl\n"
            "        IF tnKeyCode = 66\n"
            "            nAltShift = tnShiftAltCtrl\n"
            "        ENDIF\n"
            "        THIS.nCalls = THIS.nCalls + 1\n"
            "        THIS.nKeyCode = tnKeyCode\n"
            "        THIS.nShiftAltCtrl = tnShiftAltCtrl\n"
            "        THIS.cSequence = THIS.cSequence + '[' + TRANSFORM(tnKeyCode) + ']'\n"
            "        IF tnKeyCode = 66\n"
            "            NODEFAULT\n"
            "        ENDIF\n"
            "        IF tnKeyCode = 67\n"
            "            CLEAR EVENTS\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               std::string("KeyPress script should pause in READ EVENTS: ") + state.message);
        expect(state.waiting_for_events,
               "KeyPress script should report waiting_for_events while paused");

        const auto hwnd_it = state.globals.find("nkeyformhwnd");
        expect(hwnd_it != state.globals.end(),
               "KeyPress script should publish the form hWnd before entering READ EVENTS");
        const std::string initial_hwnd = hwnd_it == state.globals.end()
                                             ? "0"
                                             : copperfin::runtime::format_value(hwnd_it->second);
        std::intptr_t form_hwnd = 0;
        if (hwnd_it != state.globals.end())
        {
            try
            {
                form_hwnd = static_cast<std::intptr_t>(
                    std::stoll(initial_hwnd));
            }
            catch (...)
            {
                expect(false, "KeyPress form hWnd should be numeric");
            }
        }

        const auto ordinary_key = session.dispatch_windows_message(form_hwnd, 0x0100, 65, 0);
        expect(ordinary_key.has_value(),
               "WM_KEYDOWN should invoke the native KeyPress method");
        if (ordinary_key.has_value())
        {
            expect(*ordinary_key == 0,
                   "KeyPress without NODEFAULT should preserve the modeled default result");
        }

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "A handled KeyPress should restore the event loop");

        const auto alt_key = session.dispatch_windows_message(
            form_hwnd,
            0x0100,
            66,
            static_cast<std::intptr_t>(1) << 29);
        expect(alt_key.has_value(),
               "Alt WM_KEYDOWN should invoke KeyPress");
        if (alt_key.has_value())
        {
            expect(*alt_key == 1,
                   "NODEFAULT in KeyPress should return the modeled handled result");
        }

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "NODEFAULT KeyPress should not terminate the event loop");

        const auto clear_key = session.dispatch_windows_message(form_hwnd, 0x0100, 67, 0);
        expect(clear_key.has_value() && *clear_key == 0,
               "A KeyPress that clears events should still return its default result");

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("KeyPress script should complete after CLEAR EVENTS: ") + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        };
        check("nkeyformhwnd", initial_hwnd);
        check("nkeypresscalls", "3");
        check("nlastkeycode", "67");
        check("nlastshift", "0");
        check("naltshift", "4");
    }

    void test_native_keypress_dispatches_default_and_cancel_buttons()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_default_cancel_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "default_cancel_dispatch.prg";
        write_text(
            main_path,
            "PUBLIC nDefaultClicks, nCancelClicks, nDisabledClicks, lSuppressDefault, nFormHwnd\n"
            "nDefaultClicks = 0\n"
            "nCancelClicks = 0\n"
            "nDisabledClicks = 0\n"
            "lSuppressDefault = .F.\n"
            "oForm = CREATEOBJECT('DefaultCancelForm')\n"
            "nFormHwnd = oForm.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS DefaultCancelForm AS Form\n"
            "    ADD OBJECT cmdNoClick AS NoClickDefaultButton\n"
            "    ADD OBJECT cmdDisabled AS DisabledDefaultButton\n"
            "    ADD OBJECT cmdDefault AS DefaultButton\n"
            "    ADD OBJECT cmdCancel AS CancelButton\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 13 AND lSuppressDefault\n"
            "            NODEFAULT\n"
            "        ENDIF\n"
            "        IF tnKeyCode = 67\n"
            "            CLEAR EVENTS\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS NoClickDefaultButton AS CommandButton\n"
            "    Default = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DisabledDefaultButton AS CommandButton\n"
            "    Default = .T.\n"
            "    Enabled = .F.\n"
            "    FUNCTION Click\n"
            "        nDisabledClicks = nDisabledClicks + 1\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DefaultButton AS CommandButton\n"
            "    Default = .T.\n"
            "    FUNCTION Click\n"
            "        nDefaultClicks = nDefaultClicks + 1\n"
            "        lSuppressDefault = .T.\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS CommandButton\n"
            "    Cancel = .T.\n"
            "    FUNCTION Click\n"
            "        nCancelClicks = nCancelClicks + 1\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "Default/Cancel fixture should pause in READ EVENTS: " + state.message);

        const auto hwnd_it = state.globals.find("nformhwnd");
        expect(hwnd_it != state.globals.end(),
               "Default/Cancel fixture should publish the Form hWnd");
        std::intptr_t form_hwnd = 0;
        if (hwnd_it != state.globals.end())
        {
            form_hwnd = static_cast<std::intptr_t>(
                std::stoll(copperfin::runtime::format_value(hwnd_it->second)));
        }

        const auto enter_result = session.dispatch_windows_message(form_hwnd, 0x0100, 13, 0);
        expect(enter_result.has_value() && *enter_result == 0,
               "ENTER should dispatch the eligible default button Click");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "Default button Click should restore the event loop");

        const auto escape_result = session.dispatch_windows_message(form_hwnd, 0x0100, 27, 0);
        expect(escape_result.has_value() && *escape_result == 0,
               "ESC should dispatch the eligible cancel button Click");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "Cancel button Click should restore the event loop");

        const auto suppressed_result = session.dispatch_windows_message(form_hwnd, 0x0100, 13, 0);
        expect(suppressed_result.has_value() && *suppressed_result == 1,
               "Form KeyPress NODEFAULT should suppress default button activation");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "Suppressed default activation should restore the event loop");

        const auto clear_result = session.dispatch_windows_message(form_hwnd, 0x0100, 67, 0);
        expect(clear_result.has_value() && *clear_result == 0,
               "A non-action key should retain the ordinary KeyPress result");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "Default/Cancel fixture should complete after CLEAR EVENTS: " + state.message);

        const auto default_clicks = state.globals.find("ndefaultclicks");
        const auto cancel_clicks = state.globals.find("ncancelclicks");
        const auto disabled_clicks = state.globals.find("ndisabledclicks");
        expect(default_clicks != state.globals.end() &&
                   copperfin::runtime::format_value(default_clicks->second) == "1",
               "Default button should be clicked once while the no-handler candidate is skipped");
        expect(cancel_clicks != state.globals.end() &&
                   copperfin::runtime::format_value(cancel_clicks->second) == "1",
               "Cancel button should be clicked once");
        expect(disabled_clicks != state.globals.end() &&
                   copperfin::runtime::format_value(disabled_clicks->second) == "0",
               "Disabled default button should not be activated");

        std::size_t click_events = 0U;
        for (const auto &event : state.events)
        {
            if (event.category == "prg.event.click")
            {
                ++click_events;
            }
        }
        expect(click_events == 2U,
               "Default/Cancel activation should emit one invariant click event per action");
        fs::remove_all(temp_root, ignored);
    }
}
