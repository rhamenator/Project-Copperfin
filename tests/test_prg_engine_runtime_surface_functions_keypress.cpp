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
}
