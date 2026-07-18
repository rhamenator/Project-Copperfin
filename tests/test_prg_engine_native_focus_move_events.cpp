// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace
{

using namespace copperfin::test_support;

void test_native_focus_and_move_events()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_native_focus_move_events";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_focus_move_events.prg";
    write_text(
        main_path,
        "oForm = CREATEOBJECT('MainForm')\n"
        "oForm.first.SetFocus()\n"
        "oForm.blocked.SetFocus()\n"
        "cActiveAfterBlocked = oForm.ActiveControl.cId\n"
        "oForm.second.SetFocus()\n"
        "cActiveAfterSuppressed = oForm.ActiveControl.cId\n"
        "oForm.second.SetFocus()\n"
        "oForm.first.Move(30, 40, 50, 60)\n"
        "oForm.override.Move(1, 2, 3, 4)\n"
        "oForm.override.SetFocus()\n"
        "cEvents = oForm.cEvents\n"
        "nFirstLeft = oForm.first.Left\n"
        "nFirstTop = oForm.first.Top\n"
        "nFirstWidth = oForm.first.Width\n"
        "nFirstHeight = oForm.first.Height\n"
        "lOverrideMove = oForm.override.lMoveCalled\n"
        "lOverrideSetFocus = oForm.override.lSetFocusCalled\n"
        "nOverrideLeft = oForm.override.Left\n"
        "RETURN\n"
        "DEFINE CLASS MainForm AS Form\n"
        "    cEvents = ''\n"
        "    ADD OBJECT first AS FocusBox WITH cId = 'first'\n"
        "    ADD OBJECT second AS FocusBox WITH cId = 'second'\n"
        "    ADD OBJECT blocked AS SuppressingFocusBox WITH cId = 'blocked'\n"
        "    ADD OBJECT override AS OverrideBox\n"
        "ENDDEFINE\n"
        "DEFINE CLASS FocusBox AS TextBox\n"
        "    cId = ''\n"
        "    PROCEDURE GotFocus\n"
        "        THISFORM.cEvents = THISFORM.cEvents + THIS.cId + ':got;'\n"
        "    ENDPROC\n"
        "    PROCEDURE LostFocus\n"
        "        THISFORM.cEvents = THISFORM.cEvents + THIS.cId + ':lost;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Moved\n"
        "        THISFORM.cEvents = THISFORM.cEvents + THIS.cId + ':moved;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS SuppressingFocusBox AS TextBox\n"
        "    cId = ''\n"
        "    nLost = 0\n"
        "    PROCEDURE GotFocus\n"
        "        THISFORM.cEvents = THISFORM.cEvents + THIS.cId + ':got;'\n"
        "    ENDPROC\n"
        "    PROCEDURE LostFocus\n"
        "        THIS.nLost = THIS.nLost + 1\n"
        "        THISFORM.cEvents = THISFORM.cEvents + THIS.cId + ':lost;'\n"
        "        IF THIS.nLost = 1\n"
        "            NODEFAULT\n"
        "        ENDIF\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS OverrideBox AS TextBox\n"
        "    lMoveCalled = .F.\n"
        "    lSetFocusCalled = .F.\n"
        "    PROCEDURE Move\n"
        "        THIS.lMoveCalled = .T.\n"
        "    ENDPROC\n"
        "    PROCEDURE SetFocus\n"
        "        THIS.lSetFocusCalled = .T.\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "native focus/move event script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present");
        if (it != state.globals.end())
        {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cevents", "first:got;first:lost;blocked:got;blocked:lost;blocked:lost;second:got;first:moved;");
    check("cactiveafterblocked", "blocked");
    check("cactiveaftersuppressed", "blocked");
    check("nfirstleft", "30");
    check("nfirsttop", "40");
    check("nfirstwidth", "50");
    check("nfirstheight", "60");
    check("loverridemove", "true");
    check("loverridesetfocus", "true");
    check("noverrideleft", "0");

    expect(has_runtime_event(state.events, "prg.object.invoke", "FocusBox.GotFocus"),
           "GotFocus should use the native method invocation event path");
    expect(has_runtime_event(state.events, "prg.object.invoke", "FocusBox.LostFocus"),
           "LostFocus should use the native method invocation event path");
    expect(has_runtime_event(state.events, "prg.object.invoke", "FocusBox.Moved"),
           "Moved should use the native method invocation event path");
    expect(!has_runtime_event(state.events, "prg.object.invoke", "OverrideBox.Moved"),
           "Move override should prevent builtin Moved dispatch");
    expect(!has_runtime_event(state.events, "prg.object.invoke", "OverrideBox.GotFocus"),
           "SetFocus override should prevent builtin GotFocus dispatch");

    fs::remove_all(temp_root, ignored);
}

} // namespace

int main()
{
    test_native_focus_and_move_events();
    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
