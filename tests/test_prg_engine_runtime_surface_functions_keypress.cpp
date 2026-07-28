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

    void test_native_keypress_tab_traverses_tabstops_and_wraps()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_keypress_tab_traversal";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "keypress_tab_traversal.prg";
        write_text(
            main_path,
            "PUBLIC nFirstHwnd, nThirdHwnd, nTabCalls, nGotFocus, cLastFocus, lCancelThird\n"
            "nTabCalls = 0\n"
            "nGotFocus = 0\n"
            "cLastFocus = ''\n"
            "lCancelThird = .F.\n"
            "oForm = CREATEOBJECT('TabForm')\n"
            "oForm.first.TabIndex = 1\n"
            "oForm.skipped.TabIndex = 2\n"
            "oForm.skipped.TabStop = .F.\n"
            "oForm.disabled.TabIndex = 3\n"
            "oForm.disabled.Enabled = .F.\n"
            "oForm.third.TabIndex = 4\n"
            "oForm.first.SetFocus()\n"
            "nFirstHwnd = oForm.first.hWnd\n"
            "nThirdHwnd = oForm.third.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS TabForm AS Form\n"
            "    ADD OBJECT first AS TabBox WITH cId = 'first', TabIndex = 1\n"
            "    ADD OBJECT skipped AS TabBox WITH cId = 'skipped', TabIndex = 2, TabStop = .F.\n"
            "    ADD OBJECT disabled AS TabBox WITH cId = 'disabled', TabIndex = 3, Enabled = .F.\n"
            "    ADD OBJECT third AS TabBox WITH cId = 'third', TabIndex = 4\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TabBox AS ListBox\n"
            "    cId = ''\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 9\n"
            "            nTabCalls = nTabCalls + 1\n"
            "            IF THIS.cId = 'third' AND lCancelThird\n"
            "                NODEFAULT\n"
            "            ENDIF\n"
            "        ENDIF\n"
            "        IF tnKeyCode = 67\n"
            "            lCancelThird = .T.\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    PROCEDURE GotFocus\n"
            "        nGotFocus = nGotFocus + 1\n"
            "        cLastFocus = THIS.cId\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "Tab traversal fixture should pause in READ EVENTS: " + state.message);
        if (state.reason != copperfin::runtime::DebugPauseReason::event_loop ||
            !state.waiting_for_events)
        {
            return;
        }

        const auto read_hwnd = [&](const std::string &name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            if (found == state.globals.end())
            {
                return static_cast<std::intptr_t>(0);
            }
            return static_cast<std::intptr_t>(std::stoll(copperfin::runtime::format_value(found->second)));
        };
        const std::intptr_t first_hwnd = read_hwnd("nfirsthwnd");
        const std::intptr_t third_hwnd = read_hwnd("nthirdhwnd");

        const auto resume_event_loop = [&]()
        {
            state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                       state.waiting_for_events,
                   "Tab dispatch should restore the event loop: " + state.message);
        };
        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        const auto first_tab = session.dispatch_windows_message(first_hwnd, 0x0100, 9, 0);
        expect(first_tab.has_value() && *first_tab == 0,
               "Tab from the first control should advance focus");
        resume_event_loop();
        check("clastfocus", "third");

        const auto wrap_tab = session.dispatch_windows_message(third_hwnd, 0x0100, 9, 0);
        expect(wrap_tab.has_value() && *wrap_tab == 0,
               "Tab from the last control should wrap focus");
        resume_event_loop();
        check("clastfocus", "first");

        const auto enable_cancel = session.dispatch_windows_message(first_hwnd, 0x0100, 67, 0);
        expect(enable_cancel.has_value() && *enable_cancel == 0,
               "A normal key should reach the focused control before cancellation is enabled");
        resume_event_loop();

        const auto third_again = session.dispatch_windows_message(first_hwnd, 0x0100, 9, 0);
        expect(third_again.has_value() && *third_again == 0,
               "Tab should still reach the third control after skipped controls");
        resume_event_loop();
        check("clastfocus", "third");

        const auto cancelled_tab = session.dispatch_windows_message(third_hwnd, 0x0100, 9, 0);
        expect(cancelled_tab.has_value() && *cancelled_tab == 1,
               "NODEFAULT in the third control should cancel default Tab traversal");
        resume_event_loop();
        check("clastfocus", "third");
        check("ntabcalls", "4");
        check("ngotfocus", "4");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_keypress_tab_traverses_nested_containers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_keypress_nested_tab_traversal";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "keypress_nested_tab_traversal.prg";
        write_text(
            main_path,
            "PUBLIC nFirstHwnd, nNestedHwnd, nThirdHwnd, nGotFocus, cLastFocus\n"
            "nGotFocus = 0\n"
            "cLastFocus = ''\n"
            "oForm = CREATEOBJECT('NestedTabForm')\n"
            "oForm.first.TabIndex = 1\n"
            "oForm.host.nested.TabIndex = 2\n"
            "oForm.host.hidden.TabIndex = 3\n"
            "oForm.host.hidden.Enabled = .F.\n"
            "oForm.blockedEnabled.nested.TabIndex = 0\n"
            "oForm.blockedEnabled.Enabled = .F.\n"
            "oForm.blockedVisible.nested.TabIndex = 0\n"
            "oForm.blockedVisible.Visible = .F.\n"
            "oForm.third.TabIndex = 4\n"
            "oForm.first.SetFocus()\n"
            "nFirstHwnd = oForm.first.hWnd\n"
            "nNestedHwnd = oForm.host.nested.hWnd\n"
            "nThirdHwnd = oForm.third.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS NestedTabForm AS Form\n"
            "    ADD OBJECT first AS NestedTabBox WITH cId = 'first'\n"
            "    ADD OBJECT host AS TabContainer\n"
            "    ADD OBJECT blockedEnabled AS TabContainer\n"
            "    ADD OBJECT blockedVisible AS TabContainer\n"
            "    ADD OBJECT third AS NestedTabBox WITH cId = 'third'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TabContainer AS Container\n"
            "    ADD OBJECT nested AS NestedTabBox WITH cId = 'nested'\n"
            "    ADD OBJECT hidden AS NestedTabBox WITH cId = 'hidden'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS NestedTabBox AS ListBox\n"
            "    cId = ''\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 67 AND THIS.cId = 'third'\n"
            "            CLEAR EVENTS\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    PROCEDURE GotFocus\n"
            "        nGotFocus = nGotFocus + 1\n"
            "        cLastFocus = THIS.cId\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "Nested Tab fixture should pause in READ EVENTS: " + state.message);
        if (state.reason != copperfin::runtime::DebugPauseReason::event_loop ||
            !state.waiting_for_events)
        {
            return;
        }

        const auto read_hwnd = [&](const std::string &name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            if (found == state.globals.end())
            {
                return static_cast<std::intptr_t>(0);
            }
            return static_cast<std::intptr_t>(std::stoll(copperfin::runtime::format_value(found->second)));
        };
        const std::intptr_t first_hwnd = read_hwnd("nfirsthwnd");
        const std::intptr_t nested_hwnd = read_hwnd("nnestedhwnd");
        const std::intptr_t third_hwnd = read_hwnd("nthirdhwnd");

        const auto resume_event_loop = [&]()
        {
            state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                       state.waiting_for_events,
                   "Nested Tab dispatch should restore the event loop: " + state.message);
        };
        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        const auto first_tab = session.dispatch_windows_message(first_hwnd, 0x0100, 9, 0);
        expect(first_tab.has_value() && *first_tab == 0,
               "Tab should enter the eligible nested control");
        resume_event_loop();
        check("clastfocus", "nested");

        const auto nested_tab = session.dispatch_windows_message(nested_hwnd, 0x0100, 9, 0);
        expect(nested_tab.has_value() && *nested_tab == 0,
               "Tab should leave the nested control for the next eligible control");
        resume_event_loop();
        check("clastfocus", "third");

        const auto finish = session.dispatch_windows_message(third_hwnd, 0x0100, 67, 0);
        expect(finish.has_value() && *finish == 0,
               "The nested Tab fixture should finish through its third control");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "Nested Tab fixture should complete after CLEAR EVENTS: " + state.message);
        check("clastfocus", "third");
        check("ngotfocus", "3");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_keypress_tab_traverses_active_pageframe_page_only()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_keypress_active_pageframe_traversal";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "keypress_active_pageframe_traversal.prg";
        write_text(
            main_path,
            "PUBLIC nFirstHwnd, nActiveHwnd, nThirdHwnd, nGotFocus, cLastFocus\n"
            "nGotFocus = 0\n"
            "cLastFocus = ''\n"
            "oForm = CREATEOBJECT('PageFrameTabForm')\n"
            "oForm.first.TabIndex = 1\n"
            "oForm.activeFrame.pageOne.active.TabIndex = 2\n"
            "oForm.activeFrame.pageOne.inactive.TabStop = .F.\n"
            "oForm.activeFrame.pageOne.blocked.TabStop = .F.\n"
            "oForm.activeFrame.pageTwo.inactive.TabIndex = 0\n"
            "oForm.blockedEnabled.pageOne.blocked.TabIndex = 0\n"
            "oForm.blockedEnabled.Enabled = .F.\n"
            "oForm.blockedVisible.pageOne.blocked.TabIndex = 0\n"
            "oForm.blockedVisible.Visible = .F.\n"
            "oForm.third.TabIndex = 4\n"
            "oForm.first.SetFocus()\n"
            "nFirstHwnd = oForm.first.hWnd\n"
            "nActiveHwnd = oForm.activeFrame.pageOne.active.hWnd\n"
            "nThirdHwnd = oForm.third.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS PageFrameTabForm AS Form\n"
            "    ADD OBJECT first AS PageFrameTabBox WITH cId = 'first'\n"
            "    ADD OBJECT activeFrame AS TabPageFrame WITH ActivePage = 1\n"
            "    ADD OBJECT blockedEnabled AS TabPageFrame WITH ActivePage = 1\n"
            "    ADD OBJECT blockedVisible AS TabPageFrame WITH ActivePage = 1\n"
            "    ADD OBJECT third AS PageFrameTabBox WITH cId = 'third'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TabPageFrame AS PageFrame\n"
            "    ADD OBJECT pageOne AS TabPage\n"
            "    ADD OBJECT pageTwo AS TabPage\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TabPage AS Page\n"
            "    ADD OBJECT active AS PageFrameTabBox WITH cId = 'active'\n"
            "    ADD OBJECT inactive AS PageFrameTabBox WITH cId = 'inactive'\n"
            "    ADD OBJECT blocked AS PageFrameTabBox WITH cId = 'blocked'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PageFrameTabBox AS ListBox\n"
            "    cId = ''\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 67 AND THIS.cId = 'third'\n"
            "            CLEAR EVENTS\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    PROCEDURE GotFocus\n"
            "        nGotFocus = nGotFocus + 1\n"
            "        cLastFocus = THIS.cId\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "Active PageFrame Tab fixture should pause in READ EVENTS: " + state.message);
        if (state.reason != copperfin::runtime::DebugPauseReason::event_loop ||
            !state.waiting_for_events)
        {
            return;
        }

        const auto read_hwnd = [&](const std::string &name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            if (found == state.globals.end())
            {
                return static_cast<std::intptr_t>(0);
            }
            return static_cast<std::intptr_t>(std::stoll(copperfin::runtime::format_value(found->second)));
        };
        const std::intptr_t first_hwnd = read_hwnd("nfirsthwnd");
        const std::intptr_t active_hwnd = read_hwnd("nactivehwnd");
        const std::intptr_t third_hwnd = read_hwnd("nthirdhwnd");

        const auto resume_event_loop = [&]()
        {
            state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                       state.waiting_for_events,
                   "Active PageFrame Tab dispatch should restore the event loop: " + state.message);
        };
        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        const auto first_tab = session.dispatch_windows_message(first_hwnd, 0x0100, 9, 0);
        expect(first_tab.has_value() && *first_tab == 0,
               "Tab should enter the active PageFrame page");
        resume_event_loop();
        check("clastfocus", "active");

        const auto active_tab = session.dispatch_windows_message(active_hwnd, 0x0100, 9, 0);
        expect(active_tab.has_value() && *active_tab == 0,
               "Tab should leave the active PageFrame page for the next form control");
        resume_event_loop();
        check("clastfocus", "third");

        const auto finish = session.dispatch_windows_message(third_hwnd, 0x0100, 67, 0);
        expect(finish.has_value() && *finish == 0,
               "The active PageFrame Tab fixture should finish through its third control");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "Active PageFrame Tab fixture should complete after CLEAR EVENTS: " + state.message);
        check("clastfocus", "third");
        check("ngotfocus", "3");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_keypress_tab_traverses_commandgroup_children()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_keypress_commandgroup_traversal";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "keypress_commandgroup_traversal.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nGotFocus, cLastFocus\n"
            "nGotFocus = 0\n"
            "cLastFocus = ''\n"
            "oForm = CREATEOBJECT('CommandGroupTabForm')\n"
            "oForm.first.TabIndex = 1\n"
            "oForm.cmdGroup.TabIndex = 2\n"
            "oForm.cmdGroup.cmdFirst.TabIndex = 1\n"
            "oForm.cmdGroup.cmdSkip.TabIndex = 2\n"
            "oForm.cmdGroup.cmdSkip.TabStop = .F.\n"
            "oForm.cmdGroup.cmdDisabled.TabIndex = 3\n"
            "oForm.cmdGroup.cmdDisabled.Enabled = .F.\n"
            "oForm.cmdGroup.cmdSecond.TabIndex = 4\n"
            "oForm.blockedEnabled.TabIndex = 0\n"
            "oForm.blockedEnabled.Enabled = .F.\n"
            "oForm.blockedVisible.TabIndex = 0\n"
            "oForm.blockedVisible.Visible = .F.\n"
            "oForm.third.TabIndex = 5\n"
            "oForm.first.SetFocus()\n"
            "nFormHwnd = oForm.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS CommandGroupTabForm AS Form\n"
            "    ADD OBJECT first AS CommandGroupTabButton WITH cId = 'first'\n"
            "    ADD OBJECT cmdGroup AS TabCommandGroup\n"
            "    ADD OBJECT blockedEnabled AS TabCommandGroup WITH TabIndex = 0\n"
            "    ADD OBJECT blockedVisible AS TabCommandGroup WITH TabIndex = 0\n"
            "    ADD OBJECT third AS CommandGroupTabButton WITH cId = 'third'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TabCommandGroup AS CommandGroup\n"
            "    ADD OBJECT cmdFirst AS CommandGroupTabButton WITH cId = 'group-first'\n"
            "    ADD OBJECT cmdSkip AS CommandGroupTabButton WITH cId = 'skip'\n"
            "    ADD OBJECT cmdDisabled AS CommandGroupTabButton WITH cId = 'disabled'\n"
            "    ADD OBJECT cmdSecond AS CommandGroupTabButton WITH cId = 'group-second'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CommandGroupTabButton AS CommandButton\n"
            "    cId = ''\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 67 AND THIS.cId = 'third'\n"
            "            CLEAR EVENTS\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    PROCEDURE GotFocus\n"
            "        nGotFocus = nGotFocus + 1\n"
            "        cLastFocus = THIS.cId\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "CommandGroup Tab fixture should pause in READ EVENTS: " + state.message);
        if (state.reason != copperfin::runtime::DebugPauseReason::event_loop ||
            !state.waiting_for_events)
        {
            return;
        }

        const auto read_hwnd = [&](const std::string &name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            if (found == state.globals.end())
            {
                return static_cast<std::intptr_t>(0);
            }
            return static_cast<std::intptr_t>(std::stoll(copperfin::runtime::format_value(found->second)));
        };
        const std::intptr_t form_hwnd = read_hwnd("nformhwnd");

        const auto resume_event_loop = [&]()
        {
            state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                       state.waiting_for_events,
                   "CommandGroup Tab dispatch should restore the event loop: " + state.message);
        };
        const auto check_focus = [&](const std::string &expected)
        {
            const auto found = state.globals.find("clastfocus");
            expect(found != state.globals.end(), "clastfocus should be present");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       "clastfocus expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        const auto first_tab = session.dispatch_windows_message(form_hwnd, 0x0100, 9, 0);
        expect(first_tab.has_value() && *first_tab == 0,
               "Tab should enter the first eligible CommandGroup button");
        resume_event_loop();
        check_focus("group-first");

        const auto group_first_tab = session.dispatch_windows_message(form_hwnd, 0x0100, 9, 0);
        expect(group_first_tab.has_value() && *group_first_tab == 0,
               "Tab should advance through CommandGroup-local TabIndex order");
        resume_event_loop();
        check_focus("group-second");

        const auto group_second_tab = session.dispatch_windows_message(form_hwnd, 0x0100, 9, 0);
        expect(group_second_tab.has_value() && *group_second_tab == 0,
               "Tab should leave the CommandGroup after its eligible buttons");
        resume_event_loop();
        check_focus("third");

        const auto finish = session.dispatch_windows_message(form_hwnd, 0x0100, 67, 0);
        expect(finish.has_value() && *finish == 0,
               "The CommandGroup Tab fixture should finish through its third control");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "CommandGroup Tab fixture should complete after CLEAR EVENTS: " + state.message);
        check_focus("third");
        const auto focus_count = state.globals.find("ngotfocus");
        expect(focus_count != state.globals.end() &&
                   copperfin::runtime::format_value(focus_count->second) == "4",
               "CommandGroup Tab fixture should focus first, two group buttons, and third exactly once");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_keypress_tab_preserves_nested_parent_order()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_keypress_nested_parent_order";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "keypress_nested_parent_order.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nGotFocus, cLastFocus\n"
            "nGotFocus = 0\n"
            "cLastFocus = ''\n"
            "oForm = CREATEOBJECT('NestedParentOrderForm')\n"
            "oForm.before.TabIndex = 1\n"
            "oForm.middle.TabIndex = 2\n"
            "oForm.host.TabIndex = 3\n"
            "oForm.host.inner.TabIndex = 1\n"
            "oForm.activeFrame.TabIndex = 4\n"
            "oForm.activeFrame.pageOne.pageChild.TabIndex = 0\n"
            "oForm.after.TabIndex = 5\n"
            "oForm.before.SetFocus()\n"
            "nFormHwnd = oForm.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS NestedParentOrderForm AS Form\n"
            "    ADD OBJECT before AS NestedParentOrderBox WITH cId = 'before'\n"
            "    ADD OBJECT middle AS NestedParentOrderBox WITH cId = 'middle'\n"
            "    ADD OBJECT host AS ParentOrderContainer\n"
            "    ADD OBJECT activeFrame AS ParentOrderPageFrame WITH ActivePage = 1\n"
            "    ADD OBJECT after AS NestedParentOrderFinishButton WITH cId = 'after'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentOrderContainer AS Container\n"
            "    ADD OBJECT inner AS NestedParentOrderBox WITH cId = 'inner'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentOrderPageFrame AS PageFrame\n"
            "    ADD OBJECT pageOne AS ParentOrderPage\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentOrderPage AS Page\n"
            "    ADD OBJECT pageChild AS NestedParentOrderBox WITH cId = 'page-child'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS NestedParentOrderBox AS ListBox\n"
            "    cId = ''\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 67 AND THIS.cId = 'after'\n"
            "            CLEAR EVENTS\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    PROCEDURE GotFocus\n"
            "        nGotFocus = nGotFocus + 1\n"
            "        cLastFocus = THIS.cId\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS NestedParentOrderFinishButton AS CommandButton\n"
            "    cId = ''\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 67 AND THIS.cId = 'after'\n"
            "            CLEAR EVENTS\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    PROCEDURE GotFocus\n"
            "        nGotFocus = nGotFocus + 1\n"
            "        cLastFocus = THIS.cId\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "Nested parent-order Tab fixture should pause in READ EVENTS: " + state.message);
        if (state.reason != copperfin::runtime::DebugPauseReason::event_loop ||
            !state.waiting_for_events)
        {
            return;
        }

        const auto read_global = [&](const std::string &name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            if (found == state.globals.end())
            {
                return static_cast<std::intptr_t>(0);
            }
            return static_cast<std::intptr_t>(std::stoll(copperfin::runtime::format_value(found->second)));
        };
        const std::intptr_t form_hwnd = read_global("nformhwnd");

        const auto resume_event_loop = [&]()
        {
            state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                       state.waiting_for_events,
                   "Nested parent-order Tab dispatch should restore the event loop: " + state.message);
        };
        const auto check_focus = [&](const std::string &expected)
        {
            const auto found = state.globals.find("clastfocus");
            expect(found != state.globals.end(), "clastfocus should be present");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       "clastfocus expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        const auto dispatch_tab = [&]()
        {
            const auto result = session.dispatch_windows_message(form_hwnd, 0x0100, 9, 0);
            expect(result.has_value() && *result == 0,
                   "Nested parent-order Tab should dispatch with the modeled default result");
            resume_event_loop();
        };

        dispatch_tab();
        check_focus("middle");
        dispatch_tab();
        check_focus("inner");
        dispatch_tab();
        check_focus("page-child");
        dispatch_tab();
        check_focus("after");

        const auto finish = session.dispatch_windows_message(form_hwnd, 0x0100, 67, 0);
        expect(finish.has_value() && *finish == 0,
               "Nested parent-order fixture should finish through its final control");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "Nested parent-order fixture should complete after CLEAR EVENTS: " + state.message);
        const auto focus_count = state.globals.find("ngotfocus");
        expect(focus_count != state.globals.end() &&
                   copperfin::runtime::format_value(focus_count->second) == "5",
               "Nested parent-order fixture should focus five eligible controls exactly once");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_keypress_optiongroup_windows_navigation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
                                   "copperfin_runtime_keypress_optiongroup_windows";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "keypress_optiongroup_windows.prg";
        write_text(
            main_path,
            "PUBLIC nFormHwnd, nGotFocus, nKeyPress, nInteractive, nOptionValue, lFirst, lSecond\n"
            "nGotFocus = 0\n"
            "nKeyPress = 0\n"
            "nInteractive = 0\n"
            "nOptionValue = 0\n"
            "lFirst = .F.\n"
            "lSecond = .F.\n"
            "oForm = CREATEOBJECT('OptionNavigationForm')\n"
            "oForm.before.TabIndex = 1\n"
            "oForm.options.TabIndex = 2\n"
            "oForm.options.first.TabIndex = 2\n"
            "oForm.options.second.TabIndex = 1\n"
            "oForm.options.blocked.Enabled = .F.\n"
            "oForm.options.hidden.Visible = .F.\n"
            "oForm.options.Value = 1\n"
            "oForm.options.first.Value = .T.\n"
            "oForm.before.SetFocus()\n"
            "nFormHwnd = oForm.hWnd\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS OptionNavigationForm AS Form\n"
            "    ADD OBJECT before AS OptionNavigationBox WITH cId = 'before'\n"
            "    ADD OBJECT options AS TestOptionGroup\n"
            "    ADD OBJECT after AS OptionNavigationFinishButton WITH cId = 'after'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TestOptionGroup AS OptionGroup\n"
            "    cId = 'options'\n"
            "    ADD OBJECT first AS OptionButton WITH Caption = 'First'\n"
            "    ADD OBJECT second AS OptionButton WITH Caption = 'Second'\n"
            "    ADD OBJECT blocked AS OptionButton WITH Caption = 'Blocked'\n"
            "    ADD OBJECT hidden AS OptionButton WITH Caption = 'Hidden'\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode >= 37 AND tnKeyCode <= 40\n"
            "            nKeyPress = nKeyPress + 1\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    PROCEDURE GotFocus\n"
            "        nGotFocus = nGotFocus + 1\n"
            "    ENDPROC\n"
            "    PROCEDURE InteractiveChange\n"
            "        nInteractive = nInteractive + 1\n"
            "        nOptionValue = THIS.Value\n"
            "        lFirst = THIS.first.Value\n"
            "        lSecond = THIS.second.Value\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS OptionNavigationBox AS ListBox\n"
            "    cId = ''\n"
            "ENDDEFINE\n"
            "DEFINE CLASS OptionNavigationFinishButton AS CommandButton\n"
            "    cId = ''\n"
            "    FUNCTION KeyPress\n"
            "        LPARAMETERS tnKeyCode, tnShiftAltCtrl\n"
            "        IF tnKeyCode = 67 AND THIS.cId = 'after'\n"
            "            CLEAR EVENTS\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                   state.waiting_for_events,
               "OptionGroup navigation fixture should pause in READ EVENTS: " + state.message);
        if (state.reason != copperfin::runtime::DebugPauseReason::event_loop ||
            !state.waiting_for_events)
        {
            return;
        }

        const auto read_global_number = [&](const std::string &name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            return found == state.globals.end()
                ? 0LL
                : std::stoll(copperfin::runtime::format_value(found->second));
        };
        const auto read_global_text = [&](const std::string &name)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be present");
            return found == state.globals.end()
                ? std::string{}
                : copperfin::runtime::format_value(found->second);
        };
        const std::intptr_t form_hwnd = read_global_number("nformhwnd");
        const auto resume_event_loop = [&]()
        {
            state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                       state.waiting_for_events,
                   "OptionGroup dispatch should restore the event loop: " + state.message);
        };

        const auto enter_group = session.dispatch_windows_message(form_hwnd, 0x0100, 9, 0);
        expect(enter_group.has_value() && *enter_group == 0,
               "Tab should enter the OptionGroup as one parent-level stop");
        resume_event_loop();
        expect(read_global_number("ngotfocus") == 1,
               "Tab should focus the form control and then the OptionGroup only; got " +
                   std::to_string(read_global_number("ngotfocus")));

        const auto move_right = session.dispatch_windows_message(form_hwnd, 0x0100, 39, 0);
        expect(move_right.has_value() && *move_right == 0,
               "Right arrow should select the next eligible OptionButton");
        resume_event_loop();
        expect(read_global_number("nkeypress") == 1 &&
                   read_global_number("ninteractive") == 1 &&
                   read_global_number("noptionvalue") == 2 &&
                   read_global_text("lfirst") == "false" &&
                   read_global_text("lsecond") == "true",
               "Right arrow should update group and child selection state; keypress=" +
                   std::to_string(read_global_number("nkeypress")) +
                   " interactive=" + std::to_string(read_global_number("ninteractive")) +
                   " value=" + std::to_string(read_global_number("noptionvalue")) +
                   " first=" + read_global_text("lfirst") +
                   " second=" + read_global_text("lsecond"));

        const auto move_left = session.dispatch_windows_message(form_hwnd, 0x0100, 37, 0);
        expect(move_left.has_value() && *move_left == 0,
               "Left arrow should wrap to the prior eligible OptionButton");
        resume_event_loop();
        expect(read_global_number("nkeypress") == 2 &&
                   read_global_number("ninteractive") == 2 &&
                   read_global_number("noptionvalue") == 1 &&
                   read_global_text("lfirst") == "true" &&
                   read_global_text("lsecond") == "false",
               "Left arrow should preserve deterministic reverse selection state; keypress=" +
                   std::to_string(read_global_number("nkeypress")) +
                   " interactive=" + std::to_string(read_global_number("ninteractive")) +
                   " value=" + std::to_string(read_global_number("noptionvalue")) +
                   " first=" + read_global_text("lfirst") +
                   " second=" + read_global_text("lsecond"));

        const auto leave_group = session.dispatch_windows_message(form_hwnd, 0x0100, 9, 0);
        expect(leave_group.has_value() && *leave_group == 0,
               "Tab should leave the OptionGroup rather than entering its buttons");
        resume_event_loop();
        const auto finish = session.dispatch_windows_message(form_hwnd, 0x0100, 67, 0);
        expect(finish.has_value() && *finish == 0,
               "OptionGroup navigation fixture should finish through the next control");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "OptionGroup navigation fixture should complete after CLEAR EVENTS: " + state.message);

        fs::remove_all(temp_root, ignored);
    }
}
