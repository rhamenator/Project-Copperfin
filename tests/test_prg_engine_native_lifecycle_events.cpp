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

void test_native_show_hide_refresh_events()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_native_lifecycle_events";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_lifecycle_events.prg";
    write_text(
        main_path,
        "oForm = CREATEOBJECT('LifecycleForm')\n"
        "oForm.Show()\n"
        "oForm.Hide()\n"
        "oForm.Refresh()\n"
        "cLifecycleEvents = oForm.cEvents\n"
        "lLifecycleVisible = oForm.Visible\n"
        "oOverride = CREATEOBJECT('OverrideForm')\n"
        "oOverride.Show()\n"
        "oOverride.Hide()\n"
        "oOverride.Refresh()\n"
        "cOverrideEvents = oOverride.cEvents\n"
        "lOverrideVisible = oOverride.Visible\n"
        "oNodefault = CREATEOBJECT('NodefaultRefreshForm')\n"
        "oNodefault.Refresh()\n"
        "nNodefaultPaints = oNodefault.nPaint\n"
        "cNodefaultEvents = oNodefault.cEvents\n"
        "RETURN\n"
        "DEFINE CLASS LifecycleForm AS Form\n"
        "    cEvents = ''\n"
        "    PROCEDURE Activate\n"
        "        THIS.cEvents = THIS.cEvents + 'activate;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Deactivate\n"
        "        THIS.cEvents = THIS.cEvents + 'deactivate;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Paint\n"
        "        THIS.cEvents = THIS.cEvents + 'paint;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS NodefaultRefreshForm AS Form\n"
        "    cEvents = ''\n"
        "    nPaint = 0\n"
        "    PROCEDURE Paint\n"
        "        THIS.nPaint = THIS.nPaint + 1\n"
        "        THIS.cEvents = THIS.cEvents + 'paint;'\n"
        "        IF THIS.nPaint = 1\n"
        "            THIS.Refresh()\n"
        "        ENDIF\n"
        "        NODEFAULT\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS OverrideForm AS Form\n"
        "    cEvents = ''\n"
        "    PROCEDURE Show\n"
        "        THIS.cEvents = THIS.cEvents + 'show;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Hide\n"
        "        THIS.cEvents = THIS.cEvents + 'hide;'\n"
        "    ENDPROC\n"
        "    PROCEDURE Refresh\n"
        "        THIS.cEvents = THIS.cEvents + 'refresh;'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "native lifecycle event script should complete: " + state.message);

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

    check("clifecycleevents", "activate;deactivate;paint;");
    check("llifecyclevisible", "false");
    check("coverrideevents", "show;hide;refresh;");
    check("loverridevisible", "true");
    check("nnodefaultpaints", "2");
    check("cnodefaultevents", "paint;paint;");
    expect(has_runtime_event(state.events, "prg.object.invoke", "LifecycleForm.Activate"),
           "Show should invoke Activate through the native method path");
    expect(has_runtime_event(state.events, "prg.object.invoke", "LifecycleForm.Deactivate"),
           "Hide should invoke Deactivate through the native method path");
    expect(has_runtime_event(state.events, "prg.object.invoke", "LifecycleForm.Paint"),
           "Refresh should invoke Paint through the native method path");
    expect(!has_runtime_event(state.events, "prg.object.invoke", "OverrideForm.Activate"),
           "Show override should prevent builtin Activate dispatch");
    expect(!has_runtime_event(state.events, "prg.object.invoke", "OverrideForm.Deactivate"),
           "Hide override should prevent builtin Deactivate dispatch");
    expect(!has_runtime_event(state.events, "prg.object.invoke", "OverrideForm.Paint"),
           "Refresh override should prevent builtin Paint dispatch");

    fs::remove_all(temp_root, ignored);
}

} // namespace

int main()
{
    test_native_show_hide_refresh_events();
    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
